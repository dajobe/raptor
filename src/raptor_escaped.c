/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_escaped.c - Raptor escaped writing utilities
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
 * 
 */


#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#include <stdio.h>
#include <string.h>

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


/**
 * raptor_string_escaped_write:
 * @string: UTF-8 string to write
 * @len: length of UTF-8 string
 * @delim: Terminating delimiter character for string (such as " or >) or \0 for no escaping.
 * @flags: bit flags - see #raptor_escaped_write_bitflags
 * @iostr: #raptor_iostream to write to
 *
 * Write a UTF-8 string formatted using different escapes to a #raptor_iostream
 *
 * Supports writing escapes in the Python, N-Triples, Turtle, JSON,
 * SPARQL styles to an iostream.
 * 
 * Return value: non-0 on failure such as bad UTF-8 encoding.
 **/
int
raptor_string_escaped_write(const unsigned char *string,
                            size_t len,
                            const char delim,
                            unsigned int flags,
                            raptor_iostream *iostr)
{
  unsigned char c;
  int unichar_len;
  raptor_unichar unichar;

  if(!string)
    return 1;
  
  for(; (c=*string); string++, len--) {
    if((delim && c == delim && (delim == '\'' || delim == '"')) ||
       c == '\\') {
      raptor_iostream_write_byte('\\', iostr);
      raptor_iostream_write_byte(c, iostr);
      continue;
    }

    if(delim && c == delim) {
      raptor_iostream_counted_string_write("\\u", 2, iostr);
      raptor_iostream_hexadecimal_write(c, 4, iostr);
      continue;
    }
    
    if(flags & RAPTOR_ESCAPED_WRITE_BITFLAG_SPARQL_URI_ESCAPES) {
      /* Must escape #x00-#x20<>\"{}|^` */
      if(c <= 0x20 ||
         c == '<' || c == '>' || c == '\\' || c == '"' || 
         c == '{' || c == '}' || c == '|' || c == '^' || c == '`') {
        raptor_iostream_counted_string_write("\\u", 2, iostr);
        raptor_iostream_hexadecimal_write(c, 4, iostr);
        continue;
      } else if(c < 0x7f) {
        raptor_iostream_write_byte(c, iostr);
        continue;
      }
    }

    if(flags & RAPTOR_ESCAPED_WRITE_BITFLAG_BS_ESCAPES_TNRU) {
      if(c == 0x09) {
        raptor_iostream_counted_string_write("\\t", 2, iostr);
        continue;
      } else if(c == 0x0a) {
        raptor_iostream_counted_string_write("\\n", 2, iostr);
        continue;
      } else if(c == 0x0d) {
        raptor_iostream_counted_string_write("\\r", 2, iostr);
        continue;
      } else if(c < 0x20 || c == 0x7f) {
        raptor_iostream_counted_string_write("\\u", 2, iostr);
        raptor_iostream_hexadecimal_write(c, 4, iostr);
        continue;
      }
    }
    
    if(flags & RAPTOR_ESCAPED_WRITE_BITFLAG_BS_ESCAPES_BF) {
      if(c == 0x08) {
        /* JSON has \b for backspace */
        raptor_iostream_counted_string_write("\\b", 2, iostr);
        continue;
      } else if(c == 0x0b) {
        /* JSON has \f for formfeed */
        raptor_iostream_counted_string_write("\\f", 2, iostr);
        continue;
      }
    }

    /* Just format remaining characters */
    if(c < 0x7f) {
      raptor_iostream_write_byte(c, iostr);
      continue;
    } 
    
    /* It is unicode */    
    unichar_len = raptor_unicode_utf8_string_get_char(string, len, &unichar);
    if(unichar_len < 0 || RAPTOR_GOOD_CAST(size_t, unichar_len) > len)
      /* UTF-8 encoding had an error or ended in the middle of a string */
      return 1;

    if(flags & RAPTOR_ESCAPED_WRITE_BITFLAG_UTF8) {
      /* UTF-8 is allowed so no need to escape */
      raptor_iostream_counted_string_write(string, unichar_len, iostr);
    } else {
      if(unichar < 0x10000) {
        raptor_iostream_counted_string_write("\\u", 2, iostr);
        raptor_iostream_hexadecimal_write(RAPTOR_GOOD_CAST(unsigned int, unichar), 4, iostr);
      } else {
        raptor_iostream_counted_string_write("\\U", 2, iostr);
        raptor_iostream_hexadecimal_write(RAPTOR_GOOD_CAST(unsigned int, unichar), 8, iostr);
      }
    }
    
    unichar_len--; /* since loop does len-- */
    string += unichar_len; len -= unichar_len;

  }

  return 0;
}


/**
 * raptor_string_python_write:
 * @string: UTF-8 string to write
 * @len: length of UTF-8 string
 * @delim: Terminating delimiter character for string (such as " or >)
 * or \0 for no escaping.
 * @mode: mode 0=N-Triples mode, 1=Turtle (allow raw UTF-8), 2=Turtle long string (allow raw UTF-8), 3=JSON
 * @iostr: #raptor_iostream to write to
 *
 * Write a UTF-8 string using Python-style escapes (N-Triples, Turtle, JSON) to a #raptor_iostream
 *
 * @Deprecated: use raptor_string_escaped_write() where the features
 * requested are bits that can be individually chosen.
 * 
 * Return value: non-0 on failure such as bad UTF-8 encoding.
 **/
int
raptor_string_python_write(const unsigned char *string,
                           size_t len,
                           const char delim,
                           unsigned int mode,
                           raptor_iostream *iostr)
{
  unsigned int flags = 0;

  switch(mode) {
    case 0:
      flags = RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL;
      break;

    case 1:
      flags = RAPTOR_ESCAPED_WRITE_TURTLE_LITERAL;
      break;

    case 2:
      flags = RAPTOR_ESCAPED_WRITE_TURTLE_LONG_LITERAL;
      break;

    case 3:
      flags = RAPTOR_ESCAPED_WRITE_JSON_LITERAL;
      break;

    default:
      return 1;
  }

  return raptor_string_escaped_write(string, len, delim, flags, iostr);
}



/**
 * raptor_term_escaped_write:
 * @term: term to write
 * @flags: bit flags - see #raptor_escaped_write_bitflags
 * @iostr: raptor iostream
 * 
 * Write a #raptor_term formatted with escapes to a #raptor_iostream
 * 
 * Return value: non-0 on failure
 **/
int
raptor_term_escaped_write(const raptor_term *term,
                          unsigned int flags,
                          raptor_iostream* iostr)
{
  const char* quotes="\"\"\"\"";

  if(!term)
    return 1;

  switch(term->type) {
    case RAPTOR_TERM_TYPE_LITERAL:
      if(flags == RAPTOR_ESCAPED_WRITE_TURTLE_LONG_LITERAL) 
        raptor_iostream_counted_string_write(quotes, 3, iostr);
      else
        raptor_iostream_write_byte('"', iostr);
      raptor_string_escaped_write(term->value.literal.string,
                                  term->value.literal.string_len,
                                  '"',
                                  flags,
                                  iostr);
      if(flags == RAPTOR_ESCAPED_WRITE_TURTLE_LONG_LITERAL) 
        raptor_iostream_counted_string_write(quotes, 3, iostr);
      else
        raptor_iostream_write_byte('"', iostr);

      if(term->value.literal.language) {
        raptor_iostream_write_byte('@', iostr);
        raptor_iostream_counted_string_write(term->value.literal.language,
                                             term->value.literal.language_len,
                                             iostr);
      }
      if(term->value.literal.datatype) {
        if(flags == RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL)
          flags = RAPTOR_ESCAPED_WRITE_NTRIPLES_URI;
        else if(flags == RAPTOR_ESCAPED_WRITE_TURTLE_LITERAL)
          flags = RAPTOR_ESCAPED_WRITE_TURTLE_URI;

        raptor_iostream_counted_string_write("^^", 2, iostr);
        raptor_uri_escaped_write(term->value.literal.datatype, NULL,
                                 flags, iostr);
      }

      break;
      
    case RAPTOR_TERM_TYPE_BLANK:
      raptor_iostream_counted_string_write("_:", 2, iostr);

      raptor_iostream_counted_string_write(term->value.blank.string,
                                           term->value.blank.string_len,
                                           iostr);
      break;
      
    case RAPTOR_TERM_TYPE_URI:
      if(flags == RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL)
        flags = RAPTOR_ESCAPED_WRITE_NTRIPLES_URI;
      else if(flags == RAPTOR_ESCAPED_WRITE_TURTLE_LITERAL)
        flags = RAPTOR_ESCAPED_WRITE_TURTLE_URI;

      raptor_uri_escaped_write(term->value.uri, NULL, flags, iostr);
      break;
      
    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
      raptor_log_error_formatted(term->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                                 "Triple has unsupported term type %d", 
                                 term->type);
      return 1;
  }

  return 0;
}
