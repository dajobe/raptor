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
#ifdef LIBRDF_INTERNAL
#include <rdf_config.h>
#else
#include <config.h>
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

extern int errno;

#ifdef LIBRDF_INTERNAL
/* if inside Redland */

#ifdef RAPTOR_DEBUG
#define LIBRDF_DEBUG 1
#endif

#include <librdf.h>

#include <rdf_parser.h>
#include <rdf_node.h>
#include <rdf_stream.h>
#include <rdf_statement.h>
#include <rdf_uri.h>

#else
/* else standalone */

#define LIBRDF_MALLOC(type, size) malloc(size)
#define LIBRDF_CALLOC(type, size, count) calloc(size, count)
#define LIBRDF_FREE(type, ptr)   free((void*)ptr)

#ifdef RAPTOR_DEBUG
/* Debugging messages */
#define LIBRDF_DEBUG1(function, msg) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function); } while(0)
#define LIBRDF_DEBUG2(function, msg, arg1) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1);} while(0)
#define LIBRDF_DEBUG3(function, msg, arg1, arg2) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2);} while(0)
#define LIBRDF_DEBUG4(function, msg, arg1, arg2, arg3) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2, arg3);} while(0)
#define LIBRDF_DEBUG4(function, msg, arg1, arg2, arg3) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2, arg3);} while(0)

#else
/* DEBUGGING TURNED OFF */

/* No debugging messages */
#define LIBRDF_DEBUG1(function, msg)
#define LIBRDF_DEBUG2(function, msg, arg1)
#define LIBRDF_DEBUG3(function, msg, arg1, arg2)
#define LIBRDF_DEBUG4(function, msg, arg1, arg2, arg3)

#endif

/* Fatal errors - always happen */
#define LIBRDF_FATAL1(function, msg) do {fprintf(stderr, "%s:%d:%s: fatal error: " msg, __FILE__, __LINE__ , #function); abort();} while(0)
#define LIBRDF_FATAL2(function, msg,arg) do {fprintf(stderr, "%s:%d:%s: fatal error: " msg, __FILE__, __LINE__ , #function, arg); abort();} while(0)


#endif


/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#undef HAVE_STDLIB_H
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif


/* Raptor includes */
#include <raptor.h>
#include <ntriples.h>




/* Prototypes for local functions */
static void raptor_ntriples_generate_statement(raptor_ntriples_parser *parser, const char *subject, const raptor_ntriples_term_type subject_type, const char *predicate, const raptor_ntriples_term_type predicate_type, const void *object, const raptor_ntriples_term_type object_type);
static void raptor_ntriples_parser_fatal_error(raptor_ntriples_parser* parser, const char *message, ...);
static int raptor_ntriples_parse(raptor_ntriples_parser* parser, char *buffer, int length, int is_end);




/*
 * NTriples parser object
 */
struct raptor_ntriples_parser_s {
#ifdef LIBRDF_INTERNAL
  librdf_world *world;
#endif

  /* is filled with current line, column, byte location information */
  raptor_locator locator;

  /* current line */
  char *line;
  /* current line length */
  int line_length;
  /* current char in line buffer */
  int offset;
  
  /* non 0 if parser had fatal error and cannot continue */
  int failed;

  /* base URI of RDF/XML */
  raptor_uri *base_uri;

  /* static statement for use in passing to user code */
  raptor_statement statement;

  /* stuff for our user */
  void *user_data;

  void *fatal_error_user_data;

  raptor_message_handler fatal_error_handler;

  /* parser callbacks */
  raptor_statement_handler statement_handler;
};




/* PUBLIC FUNCTIONS */

/**
 * raptor_ntriples_new - Initialise the Raptor NTriples parser
 *
 * Return value: non 0 on failure
 **/

raptor_ntriples_parser*
raptor_ntriples_new(
#ifdef LIBRDF_INTERNAL
  librdf_world *world
#else
  void
#endif
)
{
  raptor_ntriples_parser* parser;

  parser=(raptor_ntriples_parser*)LIBRDF_CALLOC(raptor_ntriples_parser, 1, sizeof(raptor_ntriples_parser));

  if(!parser)
    return NULL;

  return parser;
}




/**
 * raptor_ntriples_free - Free the Raptor NTriples parser
 * @rdf_parser: parser object
 * 
 **/
void
raptor_ntriples_free(raptor_ntriples_parser *parser) 
{
  if(parser->line_length)
    LIBRDF_FREE(cdata, parser->line);
  
  LIBRDF_FREE(raptor_ntriples_parser, parser);
}


/**
 * raptor_ntriples_set_fatal_error_handler - Set the parser error handling function
 * @parser: the parser
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 * 
 * The function will receive callbacks when the parser fails.
 * 
 **/
void
raptor_ntriples_set_fatal_error_handler(raptor_ntriples_parser* parser,
                                        void *user_data,
                                        raptor_message_handler handler)
{
  parser->fatal_error_user_data=user_data;
  parser->fatal_error_handler=handler;
}


void
raptor_ntriples_set_statement_handler(raptor_ntriples_parser* parser,
                                      void *user_data, 
                                      raptor_statement_handler handler) 
{
  parser->user_data=user_data;
  parser->statement_handler=handler;
}


static const char *term_type_strings[]={
  "URIref",
  "AnonNode",
  "Literal"
};


const char *
raptor_ntriples_term_as_string (raptor_ntriples_term_type term) {
  return term_type_strings[(int)term];
}



static void
raptor_ntriples_generate_statement(raptor_ntriples_parser *parser, 
                                   const char *subject,
                                   const raptor_ntriples_term_type subject_type,
                                   const char *predicate,
                                   const raptor_ntriples_term_type predicate_type,
                                   const void *object,
                                   const raptor_ntriples_term_type object_type)
{
  raptor_statement *statement=&parser->statement;
  raptor_uri *subject_uri=NULL;
  raptor_uri *predicate_uri; /* always freed, no need for a sentinel */
  raptor_uri *object_uri=NULL;

  /* Two choices for subject from N-Triples */
  if(subject_type == RAPTOR_NTRIPLES_TERM_TYPE_ANON_NODE) {
    statement->subject=subject;
    statement->subject_type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
  } else {
    subject_uri=raptor_make_uri(parser->base_uri, subject);
    statement->subject=subject_uri;
    statement->subject_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
  }


  /* One choice for predicate from N-Triples */
  predicate_uri=raptor_make_uri(parser->base_uri, predicate);
  statement->predicate=predicate_uri;
  statement->predicate_type=RAPTOR_IDENTIFIER_TYPE_PREDICATE;

  /* Three choices for object from N-Triples */
  if(object_type == RAPTOR_NTRIPLES_TERM_TYPE_URI_REF) {
    object_uri=raptor_make_uri(parser->base_uri, object);
    statement->object=object_uri;
    statement->object_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
  } else if(object_type == RAPTOR_NTRIPLES_TERM_TYPE_ANON_NODE) {
    statement->object=object;
    statement->object_type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
  } else { 
    statement->object_type=RAPTOR_IDENTIFIER_TYPE_LITERAL;
    statement->object=object;
  }

  if(!parser->statement_handler)
    return;

  /* Generate the statement; or is it fact? */
  (*parser->statement_handler)(parser->user_data, statement);

  if(subject_uri)
    RAPTOR_FREE_URI(subject_uri);
  RAPTOR_FREE_URI(predicate_uri);
  if(object_uri)
    RAPTOR_FREE_URI(object_uri);
}


static int
raptor_ntriples_parse_line (raptor_ntriples_parser* parser, char *buffer, 
                            int len) 
{
  int i;
  char *p;
  char *start=NULL; /* keeps gcc -Wall happy */
  char *dest;
  char c = '\0';
  char *terms[3];
  int term_lengths[3];
  raptor_ntriples_term_type term_types[3];
  int backslash=0;

  /* ASSERTION:
   * p always points to first char we are considering
   * p[len-1] always points to last char
   */
  
  /* Handle empty  lines */
  if(!len)
    return 0;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  LIBRDF_DEBUG3(raptor_ntriples_parse_line,
                "handling line '%s' (%d bytes)\n", 
                buffer, len);
#endif
  
  p=buffer;

  while(len>0 && isspace(*p)) {
    p++;
    parser->locator.column++;
    parser->locator.byte++;
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
    raptor_ntriples_parser_fatal_error(parser, "Missing . at end of line");
    return 1;
  }

  p[len-1]='\0';
  len--;


  /* Must be triple */

  for(i=0; i<3; i++) {
    if(!len) {
      raptor_ntriples_parser_fatal_error(parser, "Unexpected end of line");
      return 1;
    }
    
    /* Expect either <anonURI> or _:name */
    if(i == 2) {
      if(*p != '<' && *p != '_' && *p != '"') {
        raptor_ntriples_parser_fatal_error(parser, "Saw '%c', expected <URIref>, _:anonNode or \"literal\"", *p);
        return 1;
      }
    } else {
      if(*p != '<' && *p != '_') {
        raptor_ntriples_parser_fatal_error(parser, "Saw '%c', expected <URIref> or _:anonNode", *p);
        return 1;
      }
    }

    switch(*p) {
      case '<':
        term_types[i]= RAPTOR_NTRIPLES_TERM_TYPE_URI_REF;
        
        p++;
        len--;
        parser->locator.column++;
        parser->locator.byte++;

        start=p;
        while(len > 0 && *p != '>') {
          p++;
          len--;
          parser->locator.column++;
          parser->locator.byte++;
        }

        if(!len) {
          raptor_ntriples_parser_fatal_error(parser, "Missing end > for URI");
          return 1;
        }

        break;

      case '"':
        term_types[i]= RAPTOR_NTRIPLES_TERM_TYPE_LITERAL;
        
        start=p;
        dest=p;

        p++;
        len--;
        parser->locator.column++;
        parser->locator.byte++;

        /* find end of string, fixing backslashed characters on the way */
        while(len > 0) {
          c = *p;

          p++;
          len--;
          parser->locator.column++;
          parser->locator.byte++;

          if(c == '\\') {
            if(backslash) {
              *dest++='\\';
              backslash=0;
            } else
              backslash=1;
            continue;
          }
          if(backslash) {
            switch(c) {
              case '"':
                *dest++='"';
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
              default:
                raptor_ntriples_parser_fatal_error(parser, "Illegal string escape \\%c in \"%s\"", c, start);
                break;
            }
            backslash=0;
            continue;
          }

          /* finish at non-backslashed " */
          if(c == '"') {
            /* terminate dest string, can be shorter than source */
            *dest='\0';
            break;
          }

          /* otherwise store and move on */
          *dest++=c;
        } /* end while */

        if(c != '"') {
          raptor_ntriples_parser_fatal_error(parser, "Missing end \" for literal");
          return 1;
        }
        
        break;


      case '_':
        term_types[i]= RAPTOR_NTRIPLES_TERM_TYPE_ANON_NODE;

        p++;
        len--;
        parser->locator.column++;
        parser->locator.byte++;

        if(!len || (len > 0 && *p != ':')) {
          raptor_ntriples_parser_fatal_error(parser, "Illegal anonNode _ not followed by :");
          return 1;
        }
        /* Found ':' - move on */

        p++;
        len--;
        parser->locator.column++;
        parser->locator.byte++;

        /* start after _: */
        start=p;

        while(len>0 && isalnum(*p)) {
          p++;
          len--;
          parser->locator.column++;
          parser->locator.byte++;
        }

        break;

      default:
        raptor_ntriples_parser_fatal_error(parser, "Unknown term type");
        return 1;
    }


    /* Replace
     *   end '>' for <URIref>
     *   whitespace after _:anonNode
     * with '\0' to terminate string
     * and move to char after delimiter
     */
    if(len>0 && term_types[i] != RAPTOR_NTRIPLES_TERM_TYPE_LITERAL) {
      *p='\0';
      p++;
      len--;
      parser->locator.column++;
      parser->locator.byte++;
    }
    
    /* Store term */
    terms[i]=start; term_lengths[i]=(p-start);

    
    /* Skip whitespace between parts */
    while(len>0 && isspace(*p)) {
      p++;
      len--;
      parser->locator.column++;
      parser->locator.byte++;
    }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    fprintf(stderr, "item %d: term '%s' len %d type %s\n",
            i, terms[i], term_lengths[i],
            raptor_ntriples_term_as_string(term_types[i]));
#endif    
  }

  if(len) {
    raptor_ntriples_parser_fatal_error(parser, "Extra junk before .: '%s' (%d bytes)", p, len);
    return 1;
  }
  

  raptor_ntriples_generate_statement(parser, 
                                     terms[0], term_types[0],
                                     terms[1], term_types[1],
                                     terms[2], term_types[2]);

  parser->locator.byte += len;

  return 0;
}


int
raptor_ntriples_parse(raptor_ntriples_parser* parser, char *s, int len,
                      int is_end)
{
  char *buffer;
  char *ptr;
  char *start;
  char last_nl;
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  LIBRDF_DEBUG2(raptor_ntriples_parse, "adding %d bytes to line buffer\n", len);
#endif

  buffer=(char*)LIBRDF_MALLOC(cstring, parser->line_length + len + 1);
  if(!buffer) {
    raptor_ntriples_parser_fatal_error(parser, "Out of memory");
    return 1;
  }

  if(parser->line_length) {
    strncpy(buffer, parser->line, parser->line_length);
    LIBRDF_FREE(cstring, parser->line);
  }

  parser->line=buffer;

  /* move pointer to end of cdata buffer */
  ptr=buffer+parser->line_length;

  /* adjust stored length */
  parser->line_length += len;

  /* now write new stuff at end of cdata buffer */
  strncpy(ptr, s, len);
  ptr += len;
  *ptr = '\0';

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  LIBRDF_DEBUG3(raptor_ntriples_parse,
                "line buffer now '%s' (%d bytes)\n", 
                parser->line, parser->line_length);
#endif

  last_nl='\n';  /* last newline character - \r triggers check */

  ptr=buffer+parser->offset;
  start=ptr;
  while(*ptr) {
    /* skip \n when just seen \r - i.e. \r\n or CR LF */
    if(last_nl == '\r' && *ptr == '\n') {
      ptr++;
      parser->locator.byte++;
    }
    
    while(*ptr && *ptr != '\n' && *ptr != '\r')
      ptr++;

    /* keep going - no newline yet */
    if(!*ptr && !is_end)
      break;

    last_nl=*ptr;

    len=ptr-start;
    parser->locator.column=0;

    *ptr='\0';
    if(raptor_ntriples_parse_line(parser,start,len))
      return 1;
    
    parser->locator.line++;

    /* go past newline */
    ptr++;
    parser->locator.byte++;

    start=ptr;
  }

  /* exit now, no more input */
  if(is_end)
    return 0;
    
  parser->offset=start-buffer;

  len=parser->line_length - parser->offset;
    
  if(len) {
    /* collapse buffer */

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    LIBRDF_DEBUG3(raptor_ntriples_parse,
                  "collapsing line buffer from %d to %d bytes\n", 
                  parser->line_length, len);
#endif
    buffer=(char*)LIBRDF_MALLOC(cstring, len + 1);
    if(!buffer) {
      raptor_ntriples_parser_fatal_error(parser, "Out of memory");
      return 1;
    }

    strncpy(buffer, parser->line+parser->line_length-len, len);
    buffer[len]='\0';

    LIBRDF_FREE(cstring, parser->line);

    parser->line=buffer;
    parser->line_length -= parser->offset;
    parser->offset=0;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    LIBRDF_DEBUG3(raptor_ntriples_parse,
                  "line buffer now '%s' (%d bytes)\n", 
                  parser->line, parser->line_length);
#endif    
  }
  
  return 0;
}


/*
 * raptor_ntriples_parser_fatal_error - Error from a parser - Internal
 **/
static void
raptor_ntriples_parser_fatal_error(raptor_ntriples_parser* parser,
                                   const char *message, ...)
{
  va_list arguments;

  parser->failed=1;

  if(parser->fatal_error_handler) {
    parser->fatal_error_handler(parser->fatal_error_user_data, 
                                &parser->locator, message);
    abort();
  }

  va_start(arguments, message);

  raptor_print_locator(stderr, &parser->locator);
  fprintf(stderr, " NTriples fatal error - ");
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);

  va_end(arguments);

  abort();
}


/* COPIED FROM raptor_parse.c */

#ifndef LIBRDF_INTERNAL
/**
 * raptor_file_uri_to_filename - Convert a URI representing a file (starting file:) to a filename
 * @uri: URI of string
 * 
 * Return value: the filename or NULL on failure
 **/
static char *
raptor_file_uri_to_filename(const char *uri) 
{
  int length;
  char *filename;

  if (strncmp(uri, "file:", 5))
    return NULL;

  /* FIXME: unix version of URI -> filename conversion */
  length=strlen(uri) -5 +1;
  filename=(char*)LIBRDF_MALLOC(cstring, length);
  if(!filename)
    return NULL;

  strcpy(filename, uri+5);
  return filename;
}
#endif



/**
 * raptor_ntriples_parse_file - Retrieve the Ntriples content at URI
 * @ntriples_parser: parser
 * @uri: URI of NTriples content
 * @base_uri: the base URI to use (or NULL if the same)
 * 
 * Return value: non 0 on failure
 **/
int
raptor_ntriples_parse_file(raptor_ntriples_parser* parser, raptor_uri *uri,
                           raptor_uri *base_uri) 
{
#define RBS 1024
  FILE *fh;
  char buffer[RBS];
  int rc=1;
  int len;
  const char *filename;
  /* for storing error info */
  raptor_locator *locator=&parser->locator;

  /* initialise fields */
  parser->base_uri=base_uri;

  locator->line=1;
  locator->column=0;
  locator->byte=0;

#ifdef RAPTOR_URI_AS_FILENAME
  filename=RAPTOR_URI_AS_FILENAME(uri);
#else
  filename=RAPTOR_URI_TO_FILENAME(uri);
#endif
  if(!filename)
    return 1;

  locator->file=filename;
  locator->uri=base_uri;

  fh=fopen(filename, "r");
  if(!fh) {
    raptor_ntriples_parser_fatal_error(parser, "file open failed - %s", strerror(errno));
#ifdef RAPTOR_URI_TO_FILENAME
    LIBRDF_FREE(cstring, (void*)filename);
#endif
    return 1;
  }


  while(fh && !feof(fh)) {
    len=fread(buffer, 1, RBS, fh);
    if(len <= 0) {
      break;
    }
    rc=raptor_ntriples_parse(parser, buffer, len, (len < RBS));
    if(len < RBS)
      break;
    if(rc) /* 0 is success */
      break;
  }
  fclose(fh);

#ifdef RAPTOR_URI_TO_FILENAME
  LIBRDF_FREE(cstring, (void*)filename);
#endif

  return (rc != 0);
}
