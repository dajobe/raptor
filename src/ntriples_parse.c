/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * ntriples_parse.c - Raptor N-Triples Parser implementation
 *
 * $Id$
 *
 * N-Triples
 * http://www.w3.org/2001/sw/RDFCore/ntriples/
 *
 * Copyright (C) 2001 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology - http://www.ilrt.org/
 * University of Bristol - http://www.bristol.ac.uk/
 * 
 * This package is Free Software or Open Source available under the
 * following licenses (these are alternatives):
 *   1. GNU Lesser General Public License (LGPL)
 *   2. GNU General Public License (GPL)
 *   3. Mozilla Public License (MPL)
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * full license terms.
 * 
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <win32_config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#endif


/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"
#include "ntriples.h"

/* Prototypes for local functions */
static void raptor_ntriples_generate_statement(raptor_parser *parser, const char *subject, const raptor_ntriples_term_type subject_type, const char *predicate, const raptor_ntriples_term_type predicate_type, const void *object, const raptor_ntriples_term_type object_type, int object_literal_is_XML, char *object_literal_language, char *object_literal_datatype);

static int raptor_ntriples_unicode_char_to_utf8(unsigned long c, char *output);
static int raptor_ntriples_utf8_to_unicode_char(long *output, const unsigned char *input, int length);



/*
 * NTriples parser object
 */
struct raptor_ntriples_parser_context_s {
  /* current line */
  char *line;
  /* current line length */
  int line_length;
  /* current char in line buffer */
  int offset;
  
  /* static statement for use in passing to user code */
  raptor_statement statement;
};


typedef struct raptor_ntriples_parser_context_s raptor_ntriples_parser_context;



/**
 * raptor_ntriples_parse_init - Initialise the Raptor NTriples parser
 *
 * Return value: non 0 on failure
 **/

static int
raptor_ntriples_parse_init(raptor_parser* rdf_parser, const char *name) {
  return 0;
}


/* PUBLIC FUNCTIONS */

/**
 * raptor_ntriples_new - Initialise the Raptor NTriples parser
 *
 * OLD API - use raptor_new_parser("ntriples")
 *
 * Return value: non 0 on failure
 **/

raptor_parser*
raptor_ntriples_new(void)
{
  return raptor_new_parser("ntriples");
}


/**
 * raptor_ntriples_free - Free the Raptor NTriples parser
 * @rdf_parser: parser object
 * 
 * OLD API - use raptor_free_parser
 **/
void
raptor_ntriples_free(raptor_parser *rdf_parser)
{
  raptor_free_parser(rdf_parser);
}


/**
 * raptor_ntriples_free - Free the Raptor NTriples parser
 * @rdf_parser: parser object
 * 
 **/
static void
raptor_ntriples_parse_terminate(raptor_parser *rdf_parser) {
  raptor_ntriples_parser_context *ntriples_parser=(raptor_ntriples_parser_context*)rdf_parser->context;
  if(ntriples_parser->line_length)
    RAPTOR_FREE(cdata, ntriples_parser->line);
}


/**
 * raptor_ntriples_set_error_handler - Set the parser error handling function
 * @parser: the parser
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 * 
 * The function will receive callbacks when the parser fails.
 *
 * OLD API - use raptor_set_error_handler
 * 
 **/
void
raptor_ntriples_set_error_handler(raptor_parser* parser,
                                  void *user_data,
                                  raptor_message_handler handler)
{
  raptor_set_error_handler(parser, user_data, handler);
}


/**
 * raptor_ntriples_set_fatal_error_handler - Set the parser error handling function
 * @parser: the parser
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 * 
 * The function will receive callbacks when the parser fails.
 * 
 * OLD API - use raptor_set_fatal_error_handler
 **/
void
raptor_ntriples_set_fatal_error_handler(raptor_parser* parser,
                                        void *user_data,
                                        raptor_message_handler handler)
{
  raptor_set_fatal_error_handler(parser, user_data, handler);
}


/**
 * raptor_ntriples_set_statement_handler - set the statement handler callback
 * @parser: ntriples parser
 * @user_data: user data for callback
 * @handler: callback function
 * 
 * OLD API - use raptor_set_statement_handler
 **/
void
raptor_ntriples_set_statement_handler(raptor_parser* parser,
                                      void *user_data, 
                                      raptor_statement_handler handler) 
{
  raptor_set_statement_handler(parser, user_data, handler);
}


static const char *term_type_strings[]={
  "URIref",
  "bnodeID",
  "Literal"
};


const char *
raptor_ntriples_term_as_string (raptor_ntriples_term_type term) {
  return term_type_strings[(int)term];
}



static void
raptor_ntriples_generate_statement(raptor_parser *parser, 
                                   const char *subject,
                                   const raptor_ntriples_term_type subject_type,
                                   const char *predicate,
                                   const raptor_ntriples_term_type predicate_type,
                                   const void *object,
                                   const raptor_ntriples_term_type object_type,
                                   int object_literal_is_XML,
                                   char *object_literal_language,
                                   char *object_literal_datatype)
{
  raptor_statement *statement=&parser->statement;
  raptor_uri *subject_uri=NULL;
  raptor_uri *predicate_uri; /* always freed, no need for a sentinel */
  raptor_uri *object_uri=NULL;
  raptor_uri *datatype_uri=NULL;

  /* Two choices for subject from N-Triples */
  if(subject_type == RAPTOR_NTRIPLES_TERM_TYPE_BLANK_NODE) {
    statement->subject=subject;
    statement->subject_type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
  } else {
    subject_uri=raptor_new_uri_relative_to_base(parser->base_uri, subject);
    statement->subject=subject_uri;
    statement->subject_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
  }

  if(object_literal_datatype)
    datatype_uri=raptor_new_uri_relative_to_base(parser->base_uri, (const char*)object_literal_datatype);

  /* One choice for predicate from N-Triples */
  predicate_uri=raptor_new_uri_relative_to_base(parser->base_uri, predicate);
  statement->predicate=predicate_uri;
  statement->predicate_type=RAPTOR_IDENTIFIER_TYPE_PREDICATE;

  /* Three choices for object from N-Triples */
  if(object_type == RAPTOR_NTRIPLES_TERM_TYPE_URI_REF) {
    object_uri=raptor_new_uri_relative_to_base(parser->base_uri, (const char*)object);
    statement->object=object_uri;
    statement->object_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
  } else if(object_type == RAPTOR_NTRIPLES_TERM_TYPE_BLANK_NODE) {
    statement->object=object;
    statement->object_type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
  } else { 
    statement->object_type=object_literal_is_XML ?
                           RAPTOR_IDENTIFIER_TYPE_XML_LITERAL :
                           RAPTOR_IDENTIFIER_TYPE_LITERAL;
    statement->object=object;
    statement->object_literal_language=object_literal_language;
    statement->object_literal_datatype=datatype_uri;
  }

  if(!parser->statement_handler)
    return;

  /* Generate the statement; or is it fact? */
  (*parser->statement_handler)(parser->user_data, statement);

  if(subject_uri)
    raptor_free_uri(subject_uri);
  raptor_free_uri(predicate_uri);
  if(object_uri)
    raptor_free_uri(object_uri);
  if(datatype_uri)
    raptor_free_uri(datatype_uri);
}


/*
 * Based on librdf_unicode_char_to_utf8
 * with no need to calculate length since the encoded character is
 * always copied into a buffer with sufficient size.
 * 
 * Return value: bytes encoded to output buffer or <0 on failure
 **/
static int
raptor_ntriples_unicode_char_to_utf8(unsigned long c, char *output)
{
  int size=0;
  
  if      (c < 0x00000080)
    size=1;
  else if (c < 0x00000800)
    size=2;
  else if (c < 0x00010000)
    size=3;
  else if (c < 0x00200000)
    size=4;
  else if (c < 0x04000000)
    size=5;
  else if (c < 0x80000000)
    size=6;
  else
    return -1;

  switch(size) {
    case 6:
      output[5]=0x80 | (c & 0x3F);
      c= c >> 6;
       /* set bit 2 (bits 7,6,5,4,3,2 less 7,6,5,4,3 set below) on last byte */
      c |= 0x4000000; /* 0x10000 = 0x04 << 24 */
      /* FALLTHROUGH */
    case 5:
      output[4]=0x80 | (c & 0x3F);
      c= c >> 6;
       /* set bit 3 (bits 7,6,5,4,3 less 7,6,5,4 set below) on last byte */
      c |= 0x200000; /* 0x10000 = 0x08 << 18 */
      /* FALLTHROUGH */
    case 4:
      output[3]=0x80 | (c & 0x3F);
      c= c >> 6;
       /* set bit 4 (bits 7,6,5,4 less 7,6,5 set below) on last byte */
      c |= 0x10000; /* 0x10000 = 0x10 << 12 */
      /* FALLTHROUGH */
    case 3:
      output[2]=0x80 | (c & 0x3F);
      c= c >> 6;
      /* set bit 5 (bits 7,6,5 less 7,6 set below) on last byte */
      c |= 0x800; /* 0x800 = 0x20 << 6 */
      /* FALLTHROUGH */
    case 2:
      output[1]=0x80 | (c & 0x3F);
      c= c >> 6;
      /* set bits 7,6 on last byte */
      c |= 0xc0; 
      /* FALLTHROUGH */
    case 1:
      output[0]=c;
  }

  return size;
}

/*
 * Based on librdf_utf8_to_unicode_char
 * replacing librdf_unicode_char with long
 *
 * Return value: number of bytes used or <0 on failure
 */

static int
raptor_ntriples_utf8_to_unicode_char(long *output,
                                     const unsigned char *input, int length)
{
  unsigned char in;
  int size;
  long c=0;
  
  if(length < 1)
    return -1;

  in=*input++;
  if((in & 0x80) == 0) {
    size=1;
    c= in & 0x7f;
  } else if((in & 0xe0) == 0xc0) {
    size=2;
    c= in & 0x1f;
  } else if((in & 0xf0) == 0xe0) {
    size=3;
    c= in & 0x0f;
  } else if((in & 0xf8) == 0xf0) {
    size=4;
    c = in & 0x07;
  } else if((in & 0xfc) == 0xf8) {
    size=5;
    c = in & 0x03;
  } else if((in & 0xfe) == 0xfc) {
    size=6;
    c = in & 0x01;
  } else
    return -1;


  if(!output)
    return size;

  if(length < size)
    return -1;

  switch(size) {
    case 6:
      in=*input++ & 0x3f;
      c= c << 6;
      c |= in;
      /* FALLTHROUGH */
    case 5:
      in=*input++ & 0x3f;
      c= c << 6;
      c |= in;
      /* FALLTHROUGH */
    case 4:
      in=*input++ & 0x3f;
      c= c << 6;
      c |= in;
      /* FALLTHROUGH */
    case 3:
      in=*input++ & 0x3f;
      c= c << 6;
      c |= in;
      /* FALLTHROUGH */
    case 2:
      in=*input++ & 0x3f;
      c= c << 6;
      c |= in;
      /* FALLTHROUGH */
    default:
      *output=c;
  }

  return size;
}

/* These are for 7-bit ASCII and not locale-specific */
#define IS_ASCII_ALPHA(c) (((c)>0x40 && (c)<0x5B) || ((c)>0x60 && (c)<0x7B))
#define IS_ASCII_DIGIT(c) ((c)>0x30 && (c)<0x3A)
#define IS_ASCII_PRINT(c) ((c)>0x1F && (c)<0x7F)

/**
 * raptor_ntriples_string - Parse an N-Triples string/URI with escapes
 * @parser: NTriples parser
 * @start: pointer to starting character of string (in)
 * @dest: destination of string (in)
 * @lenp: pointer to length of string (in/out)
 * @dest_lenp: pointer to length of destination string (out)
 * @end_char: string ending character
 * @is_uri: string is a URI
 * 
 * N-Triples strings/URIs are written in ASCII at present; characters
 * outside the printable ASCII range are discarded with a warning.
 * See the grammar for full details of the allowed ranges.
 *
 * Return value: Non 0 on failure
 **/
static int
raptor_ntriples_string(raptor_parser* rdf_parser, 
                       char **start, char *dest, 
                       int *lenp, int *dest_lenp,
                       char end_char, int is_uri)
{
  char *p=*start;
  unsigned char c='\0';
  int ulen=0;
  unsigned long unichar=0;
 
  /* find end of string, fixing backslashed characters on the way */
  while(*lenp > 0) {
    c = *p;

    p++;
    (*lenp)--;
    rdf_parser->locator.column++;
    rdf_parser->locator.byte++;

    /* This is an ASCII check, not a printable character check 
     * so isprint() is not appropriate, since that is a locale check.
     */
    if(!IS_ASCII_PRINT(c)) {
      raptor_parser_error(rdf_parser, "Non-printable ASCII character %d (0x%02X) found.", c, c);
      continue;
    }
    
    if(c != '\\') {
      /* finish at non-backslashed end_char */
      if(c == end_char) {
        /* terminate dest, can be shorter than source */
        *dest='\0';
        break;
      }
      
      /* otherwise store and move on */
      *dest++=c;
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
        *dest++=c;
        break;
      case 'n':
        *dest++='\n';
        break;
      case 'r':
        *dest++='\r';
        break;
      case 't':
        *dest++='\t';
        break;
      case 'u':
      case 'U':
        ulen=(c == 'u') ? 4 : 8;
        
        if(*lenp < ulen) {
          raptor_parser_error(rdf_parser, "%c over end of line", c);
          return 0;
        }
        
        sscanf(p, ((ulen == 4) ? "%04x" : "%08x"), &unichar);

        p+=ulen;
        (*lenp)-=ulen;
        rdf_parser->locator.column+=ulen;
        rdf_parser->locator.byte+=ulen;
        
        dest+=raptor_ntriples_unicode_char_to_utf8(unichar, dest);
        break;

      default:
        raptor_parser_error(rdf_parser, "Illegal string escape \\%c in \"%s\"", c, start);
        return 0;
    }
    
  } /* end while */


  if(c != end_char)
    raptor_parser_error(rdf_parser, "Missing terminating '%c'", end_char);

  if(dest_lenp)
    *dest_lenp=p-*start;

  *start=p;

  return 0;
}


static int
raptor_ntriples_parse_line (raptor_parser* rdf_parser, char *buffer, int len) 
{
  int i;
  char *p;
  char *dest;
  char *terms[3];
  int term_lengths[3];
  raptor_ntriples_term_type term_types[3];
  int term_length= 0;
  int object_literal_is_XML=0;
  char *object_literal_language=NULL;
  char *object_literal_datatype=NULL;


  /* ASSERTION:
   * p always points to first char we are considering
   * p[len-1] always points to last char
   */
  
  /* Handle empty  lines */
  if(!len)
    return 0;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG3(raptor_ntriples_parse_line,
                "handling line '%s' (%d bytes)\n", 
                buffer, len);
#endif
  
  p=buffer;

  while(len>0 && isspace(*p)) {
    p++;
    rdf_parser->locator.column++;
    rdf_parser->locator.byte++;
    len--;
  }

  /* Handle empty - all whitespace lines */
  if(!len)
    return 0;
  
  /* Handle comment lines */
  if(*p == '#')
    return 0;
  
  /* Remove trailing spaces */
  while(len>0 && isspace(p[len-1])) {
    p[len-1]='\0';
    len--;
  }

  /* can't be empty now - that would have been caught above */
  
  /* Check for terminating '.' */
  if(p[len-1] != '.') {
    /* Move current location to point to problem */
    rdf_parser->locator.column += len-2;
    rdf_parser->locator.byte += len-2;
    raptor_parser_error(rdf_parser, "Missing . at end of line");
    return 0;
  }

  p[len-1]='\0';
  len--;


  /* Must be triple */

  for(i=0; i<3; i++) {
    if(!len) {
      raptor_parser_error(rdf_parser, "Unexpected end of line");
      return 0;
    }
    
    /* Expect either <URI> or _:name */
    if(i == 2) {
      if(*p != '<' && *p != '_' && *p != '"' && *p != 'x') {
        raptor_parser_error(rdf_parser, "Saw '%c', expected <URIref>, _:bnodeID or \"literal\"", *p);
        return 0;
      }
      if(*p == 'x') {
        if(len < 4 || strncmp(p, "xml\"", 4)) {
          raptor_parser_error(rdf_parser, "Saw '%c', expected xml\"...\")", *p);
          return 0;
        }
      }
    } else {
      if(*p != '<' && *p != '_') {
        raptor_parser_error(rdf_parser, "Saw '%c', expected <URIref> or _:bnodeID", *p);
        return 0;
      }
    }

    switch(*p) {
      case '<':
        term_types[i]= RAPTOR_NTRIPLES_TERM_TYPE_URI_REF;
        
        dest=p;

        p++;
        len--;
        rdf_parser->locator.column++;
        rdf_parser->locator.byte++;

        if(raptor_ntriples_string(rdf_parser,
                                  &p, dest, &len, &term_length, '>', 1))
          return 1;
        break;

      case '"':
        term_types[i]= RAPTOR_NTRIPLES_TERM_TYPE_LITERAL;
        
        dest=p;

        p++;
        len--;
        rdf_parser->locator.column++;
        rdf_parser->locator.byte++;

        if(raptor_ntriples_string(rdf_parser,
                                  &p, dest, &len, &term_length, '"', 0))
          return 1;
        
        if(len && (*p == '-' || *p == '@')) {
          if(*p == '-')
            raptor_parser_error(rdf_parser, "Old N-Triples language syntax using \"string\"-lang rather than \"string\"@lang.");

          object_literal_language=p;

          /* Skip - */
          p++;
          len--;
          rdf_parser->locator.column++;
          rdf_parser->locator.byte++;

          if(!len) {
            raptor_parser_error(rdf_parser, "Missing language in xml\"string\"-language after -");
            return 0;
          }
          

          if(raptor_ntriples_string(rdf_parser,
                                    &p, object_literal_language, &len,
                                    NULL, ' ', 0))
            return 1;
        }

        if(len >1 && *p == '^' && p[1] == '^') {

          object_literal_datatype=p;

          /* Skip ^^ */
          p+= 2;
          len-= 2;
          rdf_parser->locator.column+= 2;
          rdf_parser->locator.byte+= 2;

          if(!len || (len && *p != '<')) {
            raptor_parser_error(rdf_parser, "Missing datatype URI-ref in\"string\"^^<URI-ref> after ^^");
            return 0;
          }

          p++;
          len--;
          rdf_parser->locator.column++;
          rdf_parser->locator.byte++;

          if(raptor_ntriples_string(rdf_parser,
                                    &p, object_literal_datatype, &len,
                                    NULL, '>', 1))
            return 1;
          
        }


        break;


      case '_':
        term_types[i]= RAPTOR_NTRIPLES_TERM_TYPE_BLANK_NODE;

        p++;
        len--;
        rdf_parser->locator.column++;
        rdf_parser->locator.byte++;

        if(!len || (len > 0 && *p != ':')) {
          raptor_parser_error(rdf_parser, "Illegal bNodeID - _ not followed by :");
          return 0;
        }
        /* Found ':' - move on */

        p++;
        len--;
        rdf_parser->locator.column++;
        rdf_parser->locator.byte++;

        if(len>0 && !IS_ASCII_ALPHA(*p)) {
          raptor_parser_error(rdf_parser, "Illegal bnodeID - does not start with an ASCII alphabetic character.");
          return 0;
        }

        /* start after _: */
        dest=p;

        while(len>0 && (IS_ASCII_ALPHA(*p) || IS_ASCII_DIGIT(*p))) {
          p++;
          len--;
          rdf_parser->locator.column++;
          rdf_parser->locator.byte++;
        }

        term_length=p-dest;

        break;

      case 'x':
        /* already know we have 'xml"' coming up */
        term_types[i]= RAPTOR_NTRIPLES_TERM_TYPE_LITERAL;
        
        /* 3=strlen("xml") */
        p+=3;
        len-=3;

        dest=p;

        p++;
        len--;
        rdf_parser->locator.column++;
        rdf_parser->locator.byte++;

        if(raptor_ntriples_string(rdf_parser,
                                  &p, dest, &len, &term_length, '"', 0))
          return 1;

        /* got XML literal string */
        object_literal_is_XML=1;

        if(len && (*p == '-' || *p == '@')) {
          if(*p == '-')
            raptor_parser_error(rdf_parser, "Old N-Triples language syntax using xml\"string\"-lang rather than xml\"string\"@lang.");

          object_literal_language=p;

          /* Skip - */
          p++;
          len--;
          rdf_parser->locator.column++;
          rdf_parser->locator.byte++;

          if(!len) {
            raptor_parser_error(rdf_parser, "Missing language in xml\"string\"-language after -");
            return 0;
          }

          if(raptor_ntriples_string(rdf_parser,
                                    &p, object_literal_language, &len,
                                    NULL, ' ', 0))
            return 1;
          
        }

        if(len >1 && *p == '^' && p[1] == '^') {

          object_literal_datatype=p;

          /* Skip ^^ */
          p+= 2;
          len-= 2;
          rdf_parser->locator.column+= 2;
          rdf_parser->locator.byte+= 2;

          if(!len || (len && *p != '<')) {
            raptor_parser_error(rdf_parser, "Missing datatype URI-ref in xml\"string\"^^<URI-ref> after ^^");
            return 0;
          }

          p++;
          len--;
          rdf_parser->locator.column++;
          rdf_parser->locator.byte++;

          if(raptor_ntriples_string(rdf_parser,
                                    &p, object_literal_datatype, &len,
                                    NULL, '>', 1))
            return 1;
          
        }

        if(len) {
          if(*p != ' ') {
            raptor_parser_error(rdf_parser, "Missing terminating ' '");
            return 0;
          }

          p++;
          len--;
          rdf_parser->locator.column++;
          rdf_parser->locator.byte++;
        }
        
        break;


      default:
        raptor_parser_fatal_error(rdf_parser, "Unknown term type");
        return 1;
    }


    /* Store term */
    terms[i]=dest; term_lengths[i]=term_length;

    /* Replace
     *   end '>' for <URIref>
     *   whitespace after _:bnodeID
     * with '\0' to terminate string
     * and move to char after delimiter
     */
    if(len>0 && term_types[i] != RAPTOR_NTRIPLES_TERM_TYPE_LITERAL) {
      *p='\0';
      p++;
      len--;
      rdf_parser->locator.column++;
      rdf_parser->locator.byte++;
    }
    
    
    /* Skip whitespace between parts */
    while(len>0 && isspace(*p)) {
      p++;
      len--;
      rdf_parser->locator.column++;
      rdf_parser->locator.byte++;
    }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    fprintf(stderr, "item %d: term '%s' len %d type %s\n",
            i, terms[i], term_lengths[i],
            raptor_ntriples_term_as_string(term_types[i]));
#endif    
  }

  if(len) {
    raptor_parser_error(rdf_parser, "Junk before terminating \".\"");
    return 0;
  }
  

  raptor_ntriples_generate_statement(rdf_parser, 
                                     terms[0], term_types[0],
                                     terms[1], term_types[1],
                                     terms[2], term_types[2],
                                     object_literal_is_XML,
                                     object_literal_language,
                                     object_literal_datatype);

  rdf_parser->locator.byte += len;

  return 0;
}


static int
raptor_ntriples_parse_chunk(raptor_parser* rdf_parser, 
                            const char *s, size_t len,
                            int is_end)
{
  char *buffer;
  char *ptr;
  char *start;
  char last_nl;
  raptor_ntriples_parser_context *ntriples_parser=(raptor_ntriples_parser_context*)rdf_parser->context;
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2(raptor_ntriples_parse_chunk, "adding %d bytes to line buffer\n", len);
#endif

  buffer=(char*)RAPTOR_MALLOC(cstring, ntriples_parser->line_length + len + 1);
  if(!buffer) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    return 1;
  }

  if(ntriples_parser->line_length) {
    strncpy(buffer, ntriples_parser->line, ntriples_parser->line_length);
    RAPTOR_FREE(cstring, ntriples_parser->line);
  }

  ntriples_parser->line=buffer;

  /* move pointer to end of cdata buffer */
  ptr=buffer+ntriples_parser->line_length;

  /* adjust stored length */
  ntriples_parser->line_length += len;

  /* now write new stuff at end of cdata buffer */
  strncpy(ptr, s, len);
  ptr += len;
  *ptr = '\0';

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG3(raptor_ntriples_parse_chunk,
                "line buffer now '%s' (%d bytes)\n", 
                ntriples_parser->line, ntriples_parser->line_length);
#endif

  last_nl='\n';  /* last newline character - \r triggers check */

  ptr=buffer+ntriples_parser->offset;
  start=ptr;
  while(*ptr) {
    /* skip \n when just seen \r - i.e. \r\n or CR LF */
    if(last_nl == '\r' && *ptr == '\n') {
      ptr++;
      rdf_parser->locator.byte++;
    }
    
    while(*ptr && *ptr != '\n' && *ptr != '\r')
      ptr++;

    /* keep going - no newline yet */
    if(!*ptr && !is_end)
      break;

    last_nl=*ptr;

    len=ptr-start;
    rdf_parser->locator.column=0;

    *ptr='\0';
    if(raptor_ntriples_parse_line(rdf_parser,start,len))
      return 1;
    
    rdf_parser->locator.line++;

    /* go past newline */
    ptr++;
    rdf_parser->locator.byte++;

    start=ptr;
  }

  /* exit now, no more input */
  if(is_end)
    return 0;
    
  ntriples_parser->offset=start-buffer;

  len=ntriples_parser->line_length - ntriples_parser->offset;
    
  if(len) {
    /* collapse buffer */

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    RAPTOR_DEBUG3(raptor_ntriples_parse_chunk,
                  "collapsing line buffer from %d to %d bytes\n", 
                  ntriples_parser->line_length, len);
#endif
    buffer=(char*)RAPTOR_MALLOC(cstring, len + 1);
    if(!buffer) {
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      return 1;
    }

    strncpy(buffer, ntriples_parser->line+ntriples_parser->line_length-len, len);
    buffer[len]='\0';

    RAPTOR_FREE(cstring, ntriples_parser->line);

    ntriples_parser->line=buffer;
    ntriples_parser->line_length -= ntriples_parser->offset;
    ntriples_parser->offset=0;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    RAPTOR_DEBUG3(raptor_ntriples_parse_chunk,
                  "line buffer now '%s' (%d bytes)\n", 
                  ntriples_parser->line, ntriples_parser->line_length);
#endif    
  }
  
  return 0;
}


static int
raptor_ntriples_parse_start(raptor_parser *rdf_parser, raptor_uri *uri) 
{
  raptor_locator *locator=&rdf_parser->locator;

  locator->line=1;
  locator->column=0;
  locator->byte=0;

  return 0;
}


/**
 * raptor_ntriples_parse_file - Retrieve the Ntriples content at URI
 * @rdf_parser: parser
 * @uri: URI of NTriples content
 * @base_uri: the base URI to use (or NULL if the same)
 * 
 * OLD API - use raptor_parse_file
 *
 * Return value: non 0 on failure
 **/
int
raptor_ntriples_parse_file(raptor_parser* rdf_parser, raptor_uri *uri,
                           raptor_uri *base_uri) 
{
  return raptor_parse_file(rdf_parser, uri, base_uri);
}


/**
 * raptor_print_ntriples_string - Print an UTF-8 string using N-Triples escapes
 * @stream: FILE* stream to print to
 * @string: UTF-8 string to print
 * @delim: Delimiter character for string (such as ")
 * 
 * Return value: non-0 on failure such as bad UTF-8 encoding.
 **/
int
raptor_print_ntriples_string(FILE *stream,
                             const char *string,
                             const char delim) 
{
  unsigned char c;
  int len=strlen(string);
  int unichar_len;
  long unichar;
  
  for(; (c=*string); string++, len--) {
    if(c == delim || c == '\\') {
      fprintf(stream, "\\%c", c);
      continue;
    }
    
    /* Note: NTriples is ASCII */
    if(c == 0x09) {
      fputs("\\t", stream);
      continue;
    } else if(c == 0x0a) {
      fputs("\\n", stream);
      continue;
    } else if(c == 0x0d) {
      fputs("\\r", stream);
      continue;
    } else if(c < 0x20|| c == 0x7f) {
      fprintf(stream, "\\u%04X", c);
      continue;
    } else if(c < 0x80) {
      fputc(c, stream);
      continue;
    }
    
    /* It is unicode */
    
    unichar_len=raptor_ntriples_utf8_to_unicode_char(NULL, (const unsigned char *)string, len);
    if(unichar_len < 0 || unichar_len > len)
      /* UTF-8 encoding had an error or ended in the middle of a string */
      return 1;

    unichar_len=raptor_ntriples_utf8_to_unicode_char(&unichar,
                                                     (const unsigned char *)string, len);
    
    if(unichar < 0x10000)
      fprintf(stream, "\\u%04lX", unichar);
    else
      fprintf(stream, "\\U%08lX", unichar);
    
    unichar_len--; /* since loop does len-- */
    string += unichar_len; len -= unichar_len;

  }

  return 0;
}




static void
raptor_ntriples_parser_register_factory(raptor_parser_factory *factory) 
{
  factory->context_length     = sizeof(raptor_ntriples_parser_context);
  
  factory->init      = raptor_ntriples_parse_init;
  factory->terminate = raptor_ntriples_parse_terminate;
  factory->start     = raptor_ntriples_parse_start;
  factory->chunk     = raptor_ntriples_parse_chunk;
}


void
raptor_init_parser_ntriples (void) {
  raptor_parser_register_factory("ntriples", 
                                 &raptor_ntriples_parser_register_factory);
}
