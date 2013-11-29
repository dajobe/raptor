/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_ntriples.c - Raptor N-Triples parsing utilities
 *
 * Copyright (C) 2013, David Beckett http://www.dajobe.org/
 *
 * This package is Free Software and part of Redland http://librdf.org/
 *
 * It is licensed under the following three licenses as alternatives:
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 *
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 *
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * complete terms and further detail along with the license texts for
 * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
 *
 */


#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif


/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


/* These are for 7-bit ASCII and not locale-specific */
#define IS_ASCII_ALPHA(c) (((c) > 0x40 && (c) < 0x5B) || ((c) > 0x60 && (c) < 0x7B))
#define IS_ASCII_UPPER(c) ((c) > 0x40 && (c) < 0x5B)
#define IS_ASCII_DIGIT(c) ((c) > 0x2F && (c) < 0x3A)
#define IS_ASCII_PRINT(c) ((c) > 0x1F && (c) < 0x7F)
#define TO_ASCII_LOWER(c) ((c)+0x20)

typedef enum {
  RAPTOR_TERM_CLASS_URI,      /* ends on > */
  RAPTOR_TERM_CLASS_BNODEID,  /* ends on first non [A-Za-z][A-Za-z0-9]* */
  RAPTOR_TERM_CLASS_STRING,   /* ends on non-escaped " */
  RAPTOR_TERM_CLASS_LANGUAGE  /* ends on first non [a-z0-9]+ ('-' [a-z0-9]+ )? */
} raptor_ntriples_term_class;


static int
raptor_ntriples_term_valid(raptor_parser* rdf_parser,
                           unsigned char c, int position,
                           raptor_ntriples_term_class term_class)
{
  int result = 0;

  switch(term_class) {
    case RAPTOR_TERM_CLASS_URI:
      /* ends on > */
      result = (c != '>');
      break;

    case RAPTOR_TERM_CLASS_BNODEID:
      /* ends on first non [A-Za-z0-9_:][-.A-Za-z0-9]* */
      result = IS_ASCII_ALPHA(c) || IS_ASCII_DIGIT(c) || c == '_' || c == ':';
      if(position)
        /* FIXME
         * This isn't correct; '.' is allowed in positions 1..N-1 but
         * this calling convention of character-by-character cannot
         * check this.
         */
        result = (result || c == '-');
      break;

    case RAPTOR_TERM_CLASS_STRING:
      /* ends on " */
      result = (c != '"');
      break;

    case RAPTOR_TERM_CLASS_LANGUAGE:
      /* ends on first non [a-zA-Z]+ ('-' [a-zA-Z0-9]+ )? */
      result = IS_ASCII_ALPHA(c);
      if(position)
        result = (result || IS_ASCII_DIGIT(c) || c == '-');
      break;

    default:
      raptor_parser_error(rdf_parser, "Unknown N-Triples term class %d",
                          term_class);
  }

  return result;
}


/*
 * raptor_ntriples_term:
 * @parser: NTriples parser
 * @start: pointer to starting character of string (in)
 * @dest: destination of string (in)
 * @lenp: pointer to length of string (in/out)
 * @dest_lenp: pointer to length of destination string (out)
 * @end_char: string ending character
 * @class: string class
 *
 * Parse an N-Triples term with escapes.
 *
 * Relies that @dest is long enough; it need only be as large as the
 * input string @start since when UTF-8 encoding, the escapes are
 * removed and the result is always less than or equal to length of
 * input.
 *
 * N-Triples strings/URIs are written in ASCII at present; characters
 * outside the printable ASCII range are discarded with a warning.
 * See the grammar for full details of the allowed ranges.
 *
 * UTF-8 and the \u and \U esapes are both allowed.
 *
 * Return value: Non 0 on failure
 **/
static int
raptor_ntriples_term(raptor_parser* rdf_parser,
                     const unsigned char **start, unsigned char *dest,
                     size_t *lenp, size_t *dest_lenp,
                     char end_char,
                     raptor_ntriples_term_class term_class)
{
  const unsigned char *p = *start;
  unsigned char c = '\0';
  size_t ulen = 0;
  unsigned long unichar = 0;
  unsigned int position = 0;
  int end_char_seen = 0;

  /* find end of string, fixing backslashed characters on the way */
  while(*lenp > 0) {
    int unichar_width;

    c = *p;

    p++;
    (*lenp)--;
    rdf_parser->locator.column++;
    rdf_parser->locator.byte++;

    if(c > 0x7f) {
      /* just copy the UTF-8 bytes through */
      int unichar_len;
      unichar_len = raptor_unicode_utf8_string_get_char(p - 1, 1 + *lenp, NULL);
      if(unichar_len < 0 || RAPTOR_GOOD_CAST(size_t, unichar_len) > *lenp) {
        raptor_parser_error(rdf_parser, "UTF-8 encoding error at character %d (0x%02X) found.", c, c);
        /* UTF-8 encoding had an error or ended in the middle of a string */
        return 1;
      }
      memmove(dest, p-1, unichar_len);
      dest += unichar_len;

      unichar_len--; /* p, *lenp were moved on by 1 earlier */

      p += unichar_len;
      (*lenp) -= unichar_len;
      rdf_parser->locator.column += unichar_len;
      rdf_parser->locator.byte += unichar_len;
      continue;
    }

    if(c != '\\') {
      /* finish at non-backslashed end_char */
      if(end_char && c == end_char) {
        end_char_seen = 1;
        break;
      }

      if(!raptor_ntriples_term_valid(rdf_parser, c, position, term_class)) {
        if(end_char) {
          /* end char was expected, so finding an invalid thing is an error */
          raptor_parser_error(rdf_parser, "Missing terminating '%c' (found '%c')", end_char, c);
          return 0;
        } else {
          /* it's the end - so rewind 1 to save next char */
          p--;
          (*lenp)++;
          rdf_parser->locator.column--;
          rdf_parser->locator.byte--;
          break;
        }
      }

      /* otherwise store and move on */
      *dest++ = c;
      position++;
      continue;
    }

    if(!*lenp) {
      raptor_parser_error(rdf_parser, "\\ at end of line");
      return 0;
    }

    c = *p;

    p++;
    (*lenp)--;
    rdf_parser->locator.column++;
    rdf_parser->locator.byte++;

    switch(c) {
      case '"':
      case '\\':
        *dest++ = c;
        break;
      case 'b':
        *dest++ = '\b';
        break;
      case 'f':
        *dest++ = '\f';
        break;
      case 'n':
        *dest++ = '\n';
        break;
      case 'r':
        *dest++ = '\r';
        break;
      case 't':
        *dest++ = '\t';
        break;
      case '<':
      case '>':
      case '{':
      case '}':
      case '|':
      case '^':
      case '`':
        /* Turtle 2013 allows these in URIs (as well as \" and \\) */
        *dest++ = c;
        break;

      case 'u':
      case 'U':
        ulen = (c == 'u') ? 4 : 8;

        if(*lenp < ulen) {
          raptor_parser_error(rdf_parser, "%c over end of line", c);
          return 0;
        }

        if(1) {
          unsigned int ii;
          int n = 0;

          for(ii = 0; ii < ulen; ii++) {
            char cc = p[ii];
            if(!isxdigit(RAPTOR_GOOD_CAST(char, cc))) {
              raptor_parser_error(rdf_parser,
                            "N-Triples string error - illegal hex digit %c in Unicode escape '%c%s...'",
                            cc, c, p);
              n = 1;
              break;
            }
          }

          if(n)
            break;

          n = sscanf((const char*)p, ((ulen == 4) ? "%04lx" : "%08lx"), &unichar);
          if(n != 1) {
            raptor_parser_error(rdf_parser, "Illegal Uncode escape '%c%s...'", c, p);
            break;
          }
        }

        p += ulen;
        (*lenp) -= ulen;
        rdf_parser->locator.column += RAPTOR_GOOD_CAST(int, ulen);
        rdf_parser->locator.byte += RAPTOR_GOOD_CAST(int, ulen);

        if(unichar > raptor_unicode_max_codepoint) {
          raptor_parser_error(rdf_parser,
                              "Illegal Unicode character with code point #x%lX (max #x%lX).",
                              unichar, raptor_unicode_max_codepoint);
          break;
        }

        unichar_width = raptor_unicode_utf8_string_put_char(unichar, dest, 4);
        if(unichar_width < 0) {
          raptor_parser_error(rdf_parser,
                              "Illegal Unicode character with code point #x%lX.",
                              unichar);
          break;
        }

        /* The destination length is set here to 4 since we know that in
         * all cases, the UTF-8 encoded output sequence is always shorter
         * than the input sequence, and the buffer is edited in place.
         *   \uXXXX: 6 bytes input - UTF-8 max 3 bytes output
         *   \uXXXXXXXX: 10 bytes input - UTF-8 max 4 bytes output
         */
        dest += (int)unichar_width;
        break;

      default:
        raptor_parser_error(rdf_parser,
                            "Illegal string escape \\%c in \"%s\"", c,
                            (char*)start);
        return 0;
    }

    position++;
  } /* end while */


  if(end_char && !end_char_seen) {
    raptor_parser_error(rdf_parser, "Missing terminating '%c' before end of line.", end_char);
    return 1;
  }

  /* terminate dest, can be shorter than source */
  *dest = '\0';

  if(dest_lenp)
    *dest_lenp = p - *start;

  *start = p;

  return 0;
}


/*
 * raptor_ntriples_parse_term:
 * @rdf_parser: parser
 * @string: string input (in)
 * @len_p: pointer to length of @string (in/out)
 * @term_p: pointer to store term (out)
 *
 * INTERNAL - Parse a string into a #raptor_term
 *
 * Return value: number of bytes processed or 0 on failure
 */
int
raptor_ntriples_parse_term(raptor_parser* rdf_parser,
                           unsigned char *string, size_t *len_p,
                           raptor_term** term_p)
{
  unsigned char *p = string;
  unsigned char *dest;
  size_t term_length = 0;

  switch(*p) {
    case '<':
      dest = p;

      p++;
      (*len_p)--;
      rdf_parser->locator.column++;
      rdf_parser->locator.byte++;

      if(raptor_ntriples_term(rdf_parser,
                              (const unsigned char**)&p,
                              dest, len_p, &term_length,
                              '>', RAPTOR_TERM_CLASS_URI)) {
        goto fail;
      }

      if(!raptor_turtle_check_uri_string(dest)) {
        raptor_parser_error(rdf_parser, "URI '%s' contains bad character(s)",
                            dest);
        goto fail;
      }

      if(1) {
        raptor_uri *uri;

        /* Check for bad ordinal predicate */
        if(!strncmp((const char*)dest,
                    "http://www.w3.org/1999/02/22-rdf-syntax-ns#_", 44)) {
          int ordinal = raptor_check_ordinal(dest + 44);
          if(ordinal <= 0)
            raptor_parser_error(rdf_parser, "Illegal ordinal value %d in property '%s'.", ordinal, dest);
        }
        if(raptor_uri_uri_string_is_absolute(dest) <= 0) {
          raptor_parser_error(rdf_parser, "URI '%s' is not absolute.", dest);
          goto fail;
        }

        uri = raptor_new_uri(rdf_parser->world, dest);
        if(!uri) {
          raptor_parser_error(rdf_parser, "Could not create URI for '%s'", (const char *)dest);
          goto fail;
        }

        *term_p = raptor_new_term_from_uri(rdf_parser->world, uri);
        raptor_free_uri(uri);
      }
      break;

    case '"':
      dest = p;

      p++;
      (*len_p)--;
      rdf_parser->locator.column++;
      rdf_parser->locator.byte++;

      if(raptor_ntriples_term(rdf_parser,
                              (const unsigned char**)&p,
                              dest, len_p, &term_length,
                              '"', RAPTOR_TERM_CLASS_STRING)) {
        goto fail;
      }

      if(1) {
        unsigned char *object_literal_language = NULL;
        unsigned char *object_literal_datatype = NULL;
        raptor_uri* datatype_uri = NULL;

        if(*len_p && *p == '@') {
          unsigned char *q;
          size_t lang_len;

          object_literal_language = p;

          /* Skip - */
          p++;
          (*len_p)--;
          rdf_parser->locator.column++;
          rdf_parser->locator.byte++;

          if(!*len_p) {
            raptor_parser_error(rdf_parser, "Missing language after \"string\"-");
            goto fail;
          }

          if(raptor_ntriples_term(rdf_parser,
                                  (const unsigned char**)&p,
                                  object_literal_language, len_p, &lang_len,
                                  '\0', RAPTOR_TERM_CLASS_LANGUAGE)) {
            goto fail;
          }

          if(!lang_len) {
            raptor_parser_error(rdf_parser, "Invalid language tag at @%s", p);
            goto fail;
          }

          /* Normalize language to lowercase
           * http://www.w3.org/TR/rdf-concepts/#dfn-language-identifier
           */
          for(q = object_literal_language; *q; q++) {
            if(IS_ASCII_UPPER(*q))
              *q = TO_ASCII_LOWER(*q);
          }

        }

        if(*len_p > 1 && *p == '^' && p[1] == '^') {

          object_literal_datatype = p;

          /* Skip ^^ */
          p += 2;
          *len_p -= 2;
          rdf_parser->locator.column += 2;
          rdf_parser->locator.byte += 2;

          if(!*len_p || (*len_p && *p != '<')) {
            raptor_parser_error(rdf_parser, "Missing datatype URI-ref in\"string\"^^<URI-ref> after ^^");
            goto fail;
          }

          p++;
          (*len_p)--;
          rdf_parser->locator.column++;
          rdf_parser->locator.byte++;

          if(raptor_ntriples_term(rdf_parser,
                                  (const unsigned char**)&p,
                                  object_literal_datatype, len_p, NULL,
                                  '>', RAPTOR_TERM_CLASS_URI)) {
            goto fail;
          }

          if(raptor_uri_uri_string_is_absolute(object_literal_datatype) <= 0) {
            raptor_parser_error(rdf_parser, "Datatype URI '%s' is not absolute.", object_literal_datatype);
            goto fail;
          }

        }

        if(object_literal_datatype && object_literal_language) {
          raptor_parser_warning(rdf_parser,
                                "Typed literal used with a language - ignoring the language");
          object_literal_language = NULL;
        }

        if(object_literal_datatype) {
          datatype_uri = raptor_new_uri(rdf_parser->world,
                                        object_literal_datatype);
          if(!datatype_uri) {
            raptor_parser_error(rdf_parser,
                                "Could not create literal datatype uri '%s'",
                                object_literal_datatype);
            goto fail;
          }
          object_literal_language = NULL;
        }

        *term_p = raptor_new_term_from_literal(rdf_parser->world,
                                               dest,
                                               datatype_uri,
                                               object_literal_language);
      }

      break;


      case '_':
        /* store where _ was */
        dest = p;

        p++;
        (*len_p)--;
        rdf_parser->locator.column++;
        rdf_parser->locator.byte++;

        if(!*len_p || (*len_p > 0 && *p != ':')) {
          raptor_parser_error(rdf_parser,
                              "Illegal bNodeID - _ not followed by :");
          goto fail;
        }

        /* Found ':' - move on */

        p++;
        (*len_p)--;
        rdf_parser->locator.column++;
        rdf_parser->locator.byte++;

        if(raptor_ntriples_term(rdf_parser,
                                (const unsigned char**)&p,
                                dest, len_p, &term_length,
                                '\0', RAPTOR_TERM_CLASS_BNODEID)) {
          goto fail;
        }

        if(!term_length) {
          raptor_parser_error(rdf_parser, "Bad or missing bNodeID after _:");
          goto fail;
        }

        *term_p = raptor_new_term_from_blank(rdf_parser->world, dest);

        break;

      default:
        raptor_parser_fatal_error(rdf_parser, "Unknown term type");
        goto fail;
    }

  fail:

  return p - string;
}
