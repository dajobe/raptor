/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_general.c - Raptor general routines
 *
 * $Id$
 *
 * Copyright (C) 2000-2003 David Beckett - http://purl.org/net/dajobe/
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
#include <stdarg.h>
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


/* prototypes for helper functions */
static void raptor_delete_parser_factories(void);
static raptor_parser_factory* raptor_get_parser_factory(const char *name);


/* statics */

/* list of parser factories */
static raptor_parser_factory* parsers=NULL;


/*
 * raptor_init - Initialise the raptor library
 * 
 * Initialises the library.
 *
 * MUST be called before using any of the raptor APIs.
 **/
void
raptor_init(void) 
{
  if(parsers)
    return;
  /* FIXME */
  raptor_init_parser_rdfxml();
  raptor_init_parser_ntriples();

  raptor_uri_init();
  raptor_www_init();
}


/*
 * raptor_finish - Terminate the raptor library
 *
 * Cleans up state of the library.
 **/
void
raptor_finish(void) 
{
  raptor_www_finish();
  raptor_terminate_parser_rdfxml();
  raptor_delete_parser_factories();
}


/* helper functions */


/*
 * raptor_delete_parser_factories - helper function to delete all the registered parser factories
 */
static void
raptor_delete_parser_factories(void)
{
  raptor_parser_factory *factory, *next;
  
  for(factory=parsers; factory; factory=next) {
    next=factory->next;
    RAPTOR_FREE(raptor_parser_factory, factory->name);
    RAPTOR_FREE(raptor_parser_factory, factory);
  }
  parsers=NULL;
}


/* class methods */

/*
 * raptor_parser_register_factory - Register a parser factory
 * @name: the parser factory name
 * @factory: pointer to function to call to register the factory
 * 
 **/
void
raptor_parser_register_factory(const char *name,
                               void (*factory) (raptor_parser_factory*)) 
{
  raptor_parser_factory *parser, *h;
  char *name_copy;
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2(raptor_parser_register_factory,
                "Received registration for parser %s\n", name);
#endif
  
  parser=(raptor_parser_factory*)RAPTOR_CALLOC(raptor_parser_factory, 1,
                                               sizeof(raptor_parser_factory));
  if(!parser)
    RAPTOR_FATAL1(raptor_parser_register_factory, "Out of memory\n");

  name_copy=(char*)RAPTOR_CALLOC(cstring, strlen(name)+1, 1);
  if(!name_copy) {
    RAPTOR_FREE(raptor_parser, parser);
    RAPTOR_FATAL1(raptor_parser_register_factory, "Out of memory\n");
  }
  strcpy(name_copy, name);
  parser->name=name_copy;
        
  for(h = parsers; h; h = h->next ) {
    if(!strcmp(h->name, name_copy)) {
      RAPTOR_FATAL2(raptor_parser_register_factory,
                    "parser %s already registered\n", h->name);
    }
  }
  
  /* Call the parser registration function on the new object */
  (*factory)(parser);
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG3(raptor_parser_register_factory, "%s has context size %d\n",
                name, parser->context_length);
#endif
  
  parser->next = parsers;
  parsers = parser;
}


/**
 * raptor_get_parser_factory - Get a parser factory by name
 * @name: the factory name or NULL for the default factory
 * 
 * Return value: the factory object or NULL if there is no such factory
 **/
static raptor_parser_factory*
raptor_get_parser_factory (const char *name) 
{
  raptor_parser_factory *factory;

  /* return 1st parser if no particular one wanted - why? */
  if(!name) {
    factory=parsers;
    if(!factory) {
      RAPTOR_DEBUG1(raptor_get_parser_factory, 
                    "No (default) parsers registered\n");
      return NULL;
    }
  } else {
    for(factory=parsers; factory; factory=factory->next) {
      if(!strcmp(factory->name, name)) {
        break;
      }
    }
    /* else FACTORY name not found */
    if(!factory) {
      RAPTOR_DEBUG2(raptor_get_parser_factory,
                    "No parser with name %s found\n",
                    name);
      return NULL;
    }
  }
        
  return factory;
}



/*
 * raptor_new_parser - Constructor - create a new raptor_parser object
 * @name: the parser name
 *
 * Return value: a new &raptor_parser object or NULL on failure
 */
raptor_parser*
raptor_new_parser(const char *name) {
  raptor_parser_factory* factory;
  raptor_parser* rdf_parser;

  factory=raptor_get_parser_factory(name);
  if(!factory)
    return NULL;

  rdf_parser=(raptor_parser*)RAPTOR_CALLOC(raptor_parser, 1,
                                           sizeof(raptor_parser));
  if(!rdf_parser)
    return NULL;
  
  rdf_parser->context=(char*)RAPTOR_CALLOC(raptor_parser_context, 1,
                                           factory->context_length);
  if(!rdf_parser->context) {
    raptor_free_parser(rdf_parser);
    return NULL;
  }
  
  rdf_parser->factory=factory;

  rdf_parser->failed=0;

  /* Initialise default feature values */
  rdf_parser->feature_scanning_for_rdf_RDF=0;
  rdf_parser->feature_assume_is_rdf=0;
  rdf_parser->feature_allow_non_ns_attributes=1;
  rdf_parser->feature_allow_other_parseTypes=1;

  if(factory->init(rdf_parser, name)) {
    raptor_free_parser(rdf_parser);
    return NULL;
  }
  
  return rdf_parser;
}



/**
 * raptor_start_parse: Start a parse of content with base URI
 * @rdf_parser: 
 * @uri: base URI or NULL if no base URI is required
 * 
 * Only the N-Triples parser has an optional base URI.
 * 
 * Return value: non-0 on failure.
 **/
int
raptor_start_parse(raptor_parser *rdf_parser, raptor_uri *uri) 
{
  raptor_uri_handler *uri_handler;
  void *uri_context;

  if(uri)
    uri=raptor_uri_copy(uri);
  
  if(rdf_parser->base_uri)
    raptor_free_uri(rdf_parser->base_uri);

  rdf_parser->base_uri=uri;
  rdf_parser->locator.uri=uri;
  rdf_parser->locator.line= rdf_parser->locator.column = 0;

  raptor_namespaces_free(&rdf_parser->namespaces);

  raptor_uri_get_handler(&uri_handler, &uri_context);
  raptor_namespaces_init(&rdf_parser->namespaces,
                         uri_handler, uri_context,
                         raptor_parser_error, rdf_parser);

  return rdf_parser->factory->start(rdf_parser);
}




int
raptor_parse_chunk(raptor_parser* rdf_parser,
                   const unsigned char *buffer, size_t len, int is_end) 
{
  return rdf_parser->factory->chunk(rdf_parser, buffer, len, is_end);
}


/**
 * raptor_free_parser - Destructor - destroy a raptor_parser object
 * @parser: &raptor_parser object
 * 
 **/
void
raptor_free_parser(raptor_parser* rdf_parser) 
{
  if(rdf_parser->factory)
    rdf_parser->factory->terminate(rdf_parser);

  if(rdf_parser->context)
    RAPTOR_FREE(raptor_parser_context, rdf_parser->context);

  if(rdf_parser->base_uri)
    raptor_free_uri(rdf_parser->base_uri);

  raptor_namespaces_free(&rdf_parser->namespaces);

  RAPTOR_FREE(raptor_parser, rdf_parser);
}


/* Size of XML buffer to use when reading from a file */
#define RAPTOR_READ_BUFFER_SIZE 1024

/**
 * raptor_parse_file - Retrieve the RDF/XML content at URI
 * @rdf_parser: parser
 * @uri: URI of RDF content
 * @base_uri: the base URI to use (or NULL if the same)
 * 
 * Return value: non 0 on failure
 **/
int
raptor_parse_file(raptor_parser* rdf_parser, raptor_uri *uri,
                  raptor_uri *base_uri) 
{
  /* Read buffer */
  unsigned char buffer[RAPTOR_READ_BUFFER_SIZE];
  int rc=0;
  const char *filename=raptor_uri_uri_string_to_filename(raptor_uri_as_string(uri));
  raptor_locator *locator=&rdf_parser->locator;

  if(!filename)
    return 1;

  locator->file=filename;
  locator->line= locator->column = -1;

  if(!strcmp(filename, "-"))
    rdf_parser->fh=stdin;
  else {
    rdf_parser->fh=fopen(filename, "r");
    if(!rdf_parser->fh) {
      raptor_parser_error(rdf_parser, "file '%s' open failed - %s",
                          filename, strerror(errno));
      RAPTOR_FREE(cstring, (void*)filename);
      return 1;
    }
  }

  if(raptor_start_parse(rdf_parser, base_uri)) {
    RAPTOR_FREE(cstring, (void*)filename);
    return 1;
  }
  
  while(rdf_parser->fh && !feof(rdf_parser->fh)) {
    int len=fread(buffer, 1, RAPTOR_READ_BUFFER_SIZE, rdf_parser->fh);
    int is_end=(len < RAPTOR_READ_BUFFER_SIZE);
    rc=raptor_parse_chunk(rdf_parser, buffer, len, is_end);
    if(rc || is_end)
      break;
  }

  RAPTOR_FREE(cstring, (void*)filename);

  return (rc != 0);
}


static void
raptor_parse_uri_write_bytes(raptor_www* www,
                             void *userdata, const void *ptr, size_t size, size_t nmemb)
{
  raptor_parser* rdf_parser=(raptor_parser*)userdata;
  int len=size*nmemb;

  if(raptor_parse_chunk(rdf_parser, ptr, len, 0))
    raptor_www_abort(www, "Parsing failed");
}


/**
 * raptor_parse_uri - Retrieve the RDF/XML content at URI
 * @rdf_parser: parser
 * @uri: URI of RDF content
 * @base_uri: the base URI to use (or NULL if the same)
 * 
 * Return value: non 0 on failure
 **/
int
raptor_parse_uri(raptor_parser* rdf_parser, raptor_uri *uri,
                 raptor_uri *base_uri)
{
  return raptor_parse_uri_with_connection(rdf_parser, uri, base_uri, NULL);
}


/**
 * raptor_parse_uri_with_connection - Retrieve the RDF/XML content at URI using existing WWW connection
 * @rdf_parser: parser
 * @uri: URI of RDF content
 * @base_uri: the base URI to use (or NULL if the same)
 * @connection: connection object pointer
 * 
 * Return value: non 0 on failure
 **/
int
raptor_parse_uri_with_connection(raptor_parser* rdf_parser, raptor_uri *uri,
                                 raptor_uri *base_uri, void *connection)
{
  raptor_www *www=raptor_www_new_with_connection(connection);

  if(!www)
    return 1;

  if(!base_uri)
    base_uri=uri;
  
  raptor_www_set_write_bytes_handler(www, raptor_parse_uri_write_bytes, 
                                     rdf_parser);

  if(raptor_start_parse(rdf_parser, base_uri)) {
    raptor_www_free(www);
    return 1;
  }

  raptor_www_fetch(www, uri);

  raptor_parse_chunk(rdf_parser, NULL, 0, 1);

  raptor_www_free(www);

  return 0;
}


/*
 * raptor_parser_fatal_error - Fatal Error from a parser - Internal
 **/
void
raptor_parser_fatal_error(raptor_parser* parser, const char *message, ...)
{
  va_list arguments;

  va_start(arguments, message);

  raptor_parser_fatal_error_varargs(parser, message, arguments);
  
  va_end(arguments);
}


/* Compatiblity wrapper */
char*
raptor_vsnprintf(const char *message, va_list arguments) 
{
  char empty_buffer[1];
  int len;
  char *buffer=NULL;

#ifdef HAVE_C99_VSNPRINTF
  len=vsnprintf(empty_buffer, 1, message, arguments)+1;
  if(len<=0)
    return NULL;
  
  buffer=(char*)RAPTOR_MALLOC(cstring, len);
  if(!buffer)
    return NULL;
  vsnprintf(buffer, len, message, arguments);
#else
  /* This vsnprintf doesn't return number of bytes required */
  int size=2;
      
  while(1) {
    buffer=(char*)RAPTOR_MALLOC(cstring, size+1);
    if(!buffer)
      return NULL;
    
    len=vsnprintf(buffer, size, message, arguments);
    if(len>=0)
      break;
    RAPTOR_FREE(cstring, buffer);
    size+=4;
  }
#endif

  return buffer;
}


/*
 * raptor_parser_fatal_error_varargs - Fatal Error from a parser - Internal
 **/
void
raptor_parser_fatal_error_varargs(raptor_parser* parser, const char *message,
                                  va_list arguments)
{
  parser->failed=1;

  if(parser->fatal_error_handler) {
    char *buffer=raptor_vsnprintf(message, arguments);
    if(!buffer) {
      fprintf(stderr, "raptor_parser_fatal_error_varargs: Out of memory\n");
      return;
    }

    parser->fatal_error_handler(parser->fatal_error_user_data, 
                                &parser->locator, buffer); 
    RAPTOR_FREE(cstring, buffer);
    abort();
  }

  raptor_print_locator(stderr, &parser->locator);
  fprintf(stderr, " raptor fatal error - ");
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);

  abort();
}


/*
 * raptor_parser_error - Error from a parser - Internal
 **/
void
raptor_parser_error(raptor_parser* parser, const char *message, ...)
{
  va_list arguments;

  va_start(arguments, message);

  raptor_parser_error_varargs(parser, message, arguments);
  
  va_end(arguments);
}


/*
 * raptor_parser_error_varargs - Error from a parser - Internal
 **/
void
raptor_parser_error_varargs(raptor_parser* parser, const char *message, 
                            va_list arguments)
{
  if(parser->error_handler) {
    char *buffer=raptor_vsnprintf(message, arguments);
    if(!buffer) {
      fprintf(stderr, "raptor_parser_error_varargs: Out of memory\n");
      return;
    }
    parser->error_handler(parser->error_user_data, 
                          &parser->locator, buffer);
    RAPTOR_FREE(cstring, buffer);
    return;
  }

  raptor_print_locator(stderr, &parser->locator);
  fprintf(stderr, " raptor error - ");
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);
}


/*
 * raptor_parser_warning - Warning from a parser - Internal
 **/
void
raptor_parser_warning(raptor_parser* parser, const char *message, ...)
{
  va_list arguments;

  va_start(arguments, message);

  raptor_parser_warning_varargs(parser, message, arguments);

  va_end(arguments);
}


/*
 * raptor_parser_warning - Warning from a parser - Internal
 **/
void
raptor_parser_warning_varargs(raptor_parser* parser, const char *message, 
                              va_list arguments)
{

  if(parser->warning_handler) {
    char *buffer=raptor_vsnprintf(message, arguments);
    if(!buffer) {
      fprintf(stderr, "raptor_parser_warning_varargs: Out of memory\n");
      return;
    }
    parser->warning_handler(parser->warning_user_data,
                            &parser->locator, buffer);
    RAPTOR_FREE(cstring, buffer);
    return;
  }

  raptor_print_locator(stderr, &parser->locator);
  fprintf(stderr, " raptor warning - ");
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);
}



/* PUBLIC FUNCTIONS */

/**
 * raptor_new - Initialise the Raptor RDF parser
 *
 * OLD API - use raptor_new_parser("rdfxml")
 *
 * Return value: non 0 on failure
 **/
raptor_parser*
raptor_new(void)
{
  return raptor_new_parser("rdfxml");
}




/**
 * raptor_free - Free the Raptor RDF parser
 * @rdf_parser: parser object
 *
 * OLD API - use raptor_free_parser
 * 
 **/
void
raptor_free(raptor_parser *rdf_parser) 
{
  raptor_free_parser(rdf_parser);
}


/**
 * raptor_set_fatal_error_handler - Set the parser error handling function
 * @parser: the parser
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 * 
 * The function will receive callbacks when the parser fails.
 * 
 **/
void
raptor_set_fatal_error_handler(raptor_parser* parser, void *user_data,
                               raptor_message_handler handler)
{
  parser->fatal_error_user_data=user_data;
  parser->fatal_error_handler=handler;
}


/**
 * raptor_set_error_handler - Set the parser error handling function
 * @parser: the parser
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 * 
 * The function will receive callbacks when the parser fails.
 * 
 **/
void
raptor_set_error_handler(raptor_parser* parser, void *user_data,
                         raptor_message_handler handler)
{
  parser->error_user_data=user_data;
  parser->error_handler=handler;
}


/**
 * raptor_set_warning_handler - Set the parser warning handling function
 * @parser: the parser
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 * 
 * The function will receive callbacks when the parser gives a warning.
 * 
 **/
void
raptor_set_warning_handler(raptor_parser* parser, void *user_data,
                           raptor_message_handler handler)
{
  parser->warning_user_data=user_data;
  parser->warning_handler=handler;
}


void
raptor_set_statement_handler(raptor_parser* parser,
                             void *user_data,
                             raptor_statement_handler handler)
{
  parser->user_data=user_data;
  parser->statement_handler=handler;
}


void
raptor_set_feature(raptor_parser *parser, raptor_feature feature, int value) {
  switch(feature) {
    case RAPTOR_FEATURE_SCANNING:
      parser->feature_scanning_for_rdf_RDF=value;
      break;

    case RAPTOR_FEATURE_ASSUME_IS_RDF:
      parser->feature_assume_is_rdf=value;
      break;

    case RAPTOR_FEATURE_ALLOW_NON_NS_ATTRIBUTES:
      parser->feature_allow_non_ns_attributes=value;
      break;

    case RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES:
      parser->feature_allow_other_parseTypes=value;
      break;

    default:
      break;
  }
}


void
raptor_parse_abort(raptor_parser *parser) {
  parser->failed=1;
}


/* 0.9.9 added */
void
raptor_parser_abort(raptor_parser *parser, char *reason) {
  parser->failed=1;
}


void
raptor_print_statement_detailed(const raptor_statement * statement,
                                int detailed, FILE *stream) 
{
  if(statement->subject_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
    fprintf(stream, "[%s, ", (const char*)statement->subject);
  else {
#ifdef RAPTOR_DEBUG
    if(!statement->subject)
      RAPTOR_FATAL1(raptor_print_statement_detailed, "Statement has NULL subject URI\n");
#endif
    fprintf(stream, "[%s, ",
            raptor_uri_as_string((raptor_uri*)statement->subject));
  }

  if(statement->predicate_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL)
    fprintf(stream, "[rdf:_%d]", *((int*)statement->predicate));
  else {
#ifdef RAPTOR_DEBUG
    if(!statement->predicate)
      RAPTOR_FATAL1(raptor_print_statement_detailed, "Statement has NULL predicate URI\n");
#endif
    fputs(raptor_uri_as_string((raptor_uri*)statement->predicate), stream);
  }

  fputs(", ", stream);
  if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_LITERAL || 
     statement->object_type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL) {
    if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL) {
      fputs("<http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral>", stream);
    } else if(statement->object_literal_datatype) {
      fputc('<', stream);
      fputs(raptor_uri_as_string((raptor_uri*)statement->object_literal_datatype), stream);
      fputc('<', stream);
    }
    fputc('"', stream);
    fputs((const char*)statement->object, stream);
    fputc('"', stream);
  } else if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
    fputs((const char*)statement->object, stream);
  else if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL)
    fprintf(stream, "[rdf:_%d]", *((int*)statement->object));
  else {
#ifdef RAPTOR_DEBUG
    if(!statement->object)
      RAPTOR_FATAL1(raptor_print_statement_detailed, "Statement has NULL object URI\n");
#endif
    fprintf(stream, "%s", 
            raptor_uri_as_string((raptor_uri*)statement->object));
  }
  fputc(']', stream);
}


void
raptor_print_statement(const raptor_statement * const statement, FILE *stream) 
{
  raptor_print_statement_detailed(statement, 0, stream);
}


/**
 * raptor_print_ntriples_string - Print an UTF-8 string using N-Triples escapes
 * @stream: FILE* stream to print to
 * @string: UTF-8 string to print
 * @delim: Delimiter character for string (such as ") or \0 for no delim
 * escaping.
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
    if((delim && c == delim) || c == '\\') {
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
    
    unichar_len=raptor_utf8_to_unicode_char(NULL, (const unsigned char *)string, len);
    if(unichar_len < 0 || unichar_len > len)
      /* UTF-8 encoding had an error or ended in the middle of a string */
      return 1;

    unichar_len=raptor_utf8_to_unicode_char(&unichar,
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




void
raptor_print_statement_part_as_ntriples(FILE* stream,
                                        const void *term, 
                                        raptor_identifier_type type,
                                        raptor_uri* literal_datatype,
                                        const unsigned char *literal_language) 
{
  switch(type) {
    case RAPTOR_IDENTIFIER_TYPE_LITERAL:
    case RAPTOR_IDENTIFIER_TYPE_XML_LITERAL:
      fputc('"', stream);
      raptor_print_ntriples_string(stream, (const char*)term, '"');
      fputc('"', stream);
      if(literal_language)
        fprintf(stream, "@%s",  (const char*)literal_language);
      if(type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL)
        fputs("^^<http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral>", stream);
      else if(literal_datatype)
        fprintf(stream, "^^<%s>", raptor_uri_as_string((raptor_uri*)literal_datatype));

      break;
      
    case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
      fprintf(stream, "_:%s", (const char*)term);
      break;
      
    case RAPTOR_IDENTIFIER_TYPE_ORDINAL:
      fprintf(stream, "<http://www.w3.org/1999/02/22-rdf-syntax-ns#_%d>",
              *((int*)term));
      break;
  
    case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
    case RAPTOR_IDENTIFIER_TYPE_PREDICATE:
      fputc('<', stream);
      raptor_print_ntriples_string(stream, raptor_uri_as_string((raptor_uri*)term), '\0');
      fputc('>', stream);
      break;
      
    default:
      RAPTOR_FATAL2(raptor_statement_part_as_string, "Unknown type %d", type);
  }
}


void
raptor_print_statement_as_ntriples(const raptor_statement * statement,
                                   FILE *stream) 
{
  raptor_print_statement_part_as_ntriples(stream,
                                          statement->subject,
                                          statement->subject_type,
                                          NULL, NULL);
  fputc(' ', stream);
  raptor_print_statement_part_as_ntriples(stream,
                                          statement->predicate,
                                          statement->predicate_type,
                                          NULL, NULL);
  fputc(' ', stream);
  raptor_print_statement_part_as_ntriples(stream,
                                          statement->object,
                                          statement->object_type,
                                          statement->object_literal_datatype,
                                          statement->object_literal_language);
  fputs(" .", stream);
}




const unsigned char *
raptor_generate_id(raptor_parser *rdf_parser, const int id_for_bag)
{
  static int myid=0;
  unsigned char *buffer;
  /* "genid" + min length 1 + \0 */
  int length=7;
  int id=++myid;
  int tmpid=id;

  while(tmpid/=10)
    length++;
  buffer=(unsigned char*)RAPTOR_MALLOC(cstring, length);
  if(!buffer)
    return NULL;
  sprintf((char*)buffer, "genid%d", id);

  return buffer;
}


/**
 * raptor_new_identifier - Constructor - create a raptor_identifier
 * @type: raptor_identifier_type of identifier
 * @uri: URI of identifier (if relevant)
 * @uri_source: raptor_uri_source of URI (if relevnant)
 * @id: string for ID or genid (if relevant)
 * 
 * Constructs a new identifier copying the URI, ID fields.
 * 
 * Return value: a new raptor_identifier object or NULL on failure
 **/
raptor_identifier*
raptor_new_identifier(raptor_identifier_type type,
                      raptor_uri *uri,
                      raptor_uri_source uri_source,
                      unsigned char *id)
{
  raptor_identifier *identifier;
  raptor_uri *new_uri=NULL;
  unsigned char *new_id=NULL;

  identifier=(raptor_identifier*)RAPTOR_CALLOC(raptor_identifier, 1,
                                                sizeof(raptor_identifier));
  if(!identifier)
    return NULL;

  if(uri) {
    new_uri=raptor_uri_copy(uri);
    if(!new_uri) {
      RAPTOR_FREE(raptor_identifier, identifier);
      return NULL;
    }
  }

  if(id) {
    int len=strlen((char*)id);
    
    new_id=(unsigned char*)RAPTOR_MALLOC(cstring, len+1);
    if(!len) {
      if(new_uri)
        RAPTOR_FREE(cstring, new_uri);
      RAPTOR_FREE(raptor_identifier, identifier);
      return NULL;
    }
    strncpy((char*)new_id, (char*)id, len+1);
  }
  

  identifier->is_malloced=1;
  raptor_init_identifier(identifier, type, new_uri, uri_source, new_id);
  return identifier;
}


/**
 * raptor_init_identifier - Initialise a pre-allocated raptor_identifier object
 * @identifier: Existing object
 * @type: raptor_identifier_type of identifier
 * @uri: URI of identifier (if relevant)
 * @uri_source: raptor_uri_source of URI (if relevnant)
 * @id: string for ID or genid (if relevant)
 * 
 * Fills in the fields of an existing allocated raptor_identifier object.
 * DOES NOT copy any of the option arguments.
 **/
void
raptor_init_identifier(raptor_identifier *identifier,
                       raptor_identifier_type type,
                       raptor_uri *uri,
                       raptor_uri_source uri_source,
                       unsigned char *id) 
{
  identifier->is_malloced=0;
  identifier->type=type;
  identifier->uri=uri;
  identifier->uri_source=uri_source;
  identifier->id=id;
}


/**
 * raptor_copy_identifier - Copy raptor_identifiers
 * @dest: destination &raptor_identifier (previously created)
 * @src:  source &raptor_identifier
 * 
 * Return value: Non 0 on failure
 **/
int
raptor_copy_identifier(raptor_identifier *dest, raptor_identifier *src)
{
  raptor_uri *new_uri=NULL;
  unsigned char *new_id=NULL;
  
  raptor_free_identifier(dest);
  raptor_init_identifier(dest, src->type, new_uri, src->uri_source, new_id);

  if(src->uri) {
    new_uri=raptor_uri_copy(src->uri);
    if(!new_uri)
      return 0;
  }

  if(src->id) {
    int len=strlen((char*)src->id);
    
    new_id=(unsigned char*)RAPTOR_MALLOC(cstring, len+1);
    if(!len) {
      if(new_uri)
        RAPTOR_FREE(cstring, new_uri);
      return 0;
    }
    strncpy((char*)new_id, (char*)src->id, len+1);
  }
  dest->uri=new_uri;
  dest->id=new_id;

  dest->type=src->type;
  dest->uri_source=src->uri_source;
  dest->ordinal=src->ordinal;

  return 0;
}


/**
 * raptor_free_identifier - Destructor - destroy a raptor_identifier object
 * @identifier: &raptor_identifier object
 * 
 * Does not free an object initialised by raptor_init_identifier()
 **/
void
raptor_free_identifier(raptor_identifier *identifier) 
{
  if(identifier->uri)
    raptor_free_uri(identifier->uri);

  if(identifier->id)
    RAPTOR_FREE(cstring, (void*)identifier->id);

  if(identifier->is_malloced)
    RAPTOR_FREE(identifier, (void*)identifier);
}


raptor_locator*
raptor_get_locator(raptor_parser *rdf_parser) 
{
  return &rdf_parser->locator;
}


/**
 * raptor_validate_xml_ID - check the string matches the xml:ID value constraints
 * @rdf_parser: RDF parser
 * @string: The string to check.
 *
 * This checks the syntax part of the xml:ID validity constraint,
 * that it matches [ VC: Name Token ] as amended by XML Namespaces:
 *
 *   http://www.w3.org/TR/REC-xml-names/#NT-NCName
 * 
 * Returns non-zero if the ID string is valid
 **/
int
raptor_valid_xml_ID(raptor_parser *rdf_parser, const unsigned char *string)
{
  unsigned char c;
  int len=strlen((const char*)string);
  int unichar_len;
  long unichar;
  int pos;
  
  for(pos=0; (c=*string); string++, len--, pos++) {

    /* It is unicode */
    
    unichar_len=raptor_utf8_to_unicode_char(NULL, (const unsigned char *)string, len);
    if(unichar_len < 0 || unichar_len > len) {
      raptor_parser_error(rdf_parser, "Bad UTF-8 encoding missing.");
      return 0;
    }
  
    unichar_len=raptor_utf8_to_unicode_char(&unichar,
                                            (const unsigned char *)string, len);
    if(!pos) {
      /* start of xml:ID name */
      if(!raptor_unicode_is_namestartchar(unichar))
        return 0;
    } else {
      /* rest of xml:ID name */
      if(!raptor_unicode_is_namechar(unichar))
        return 0;
    }

    unichar_len--; /* since loop does len-- */
    string += unichar_len; len -= unichar_len;
  }
  return 1;
}



/**
 * raptor_xml_escape_string - return an XML-escaped version of the current string
 * @string: string to XML escape
 * @len: length of string
 * @new_len_p: pointer to store new string length
 * @quote: optional quote character to escape, or 0
 * 
 * Replaces each &, < and > with &amp; &lt; &gt; respectively
 * and turns characters 0x7e to 0xff inclusive into the 
 * escaped &#nnn form.
 *
 * If quote is given it can be either of '\'' or '\"'
 * which will be turned into &apos; or &quot; respectively.
 * ASCII NUL ('\0') or any other character will not be escaped.
 * 
 * Return value: a new string or NULL on failure
 **/
char*
raptor_xml_escape_string(raptor_parser *rdf_parser, 
                         const unsigned char *string, size_t len,
                         size_t* new_len_p,
                         char quote)
{
  int l;
  int new_len=0;
  char *new_string;
  const unsigned char *p;
  char *q;
  int unichar_len;
  unsigned long unichar;

  if(quote != '\"' && quote != '\'')
    quote='\0';

  for(l=len, p=string; l; p++, l--) {
    unichar_len=raptor_utf8_to_unicode_char(&unichar, p, l);
    if(unichar_len < 0 || unichar_len > l) {
      raptor_parser_error(rdf_parser, "Bad UTF-8 encoding missing.");
      return 0;
    }
  
    if(unichar == '&')
      /* &amp; */
      new_len+=5;
    else if(unichar == '<' || unichar == '>')
      /* &lt; or &gt; */
      new_len+=4;
    else if (quote && unichar == quote)
      /* &apos; or &quot; */
      new_len+= 6;
    else if (unichar == 0x09 || unichar == 0x0d || unichar == 0x0a)
      /* XML whitespace excluding 0x20, since next condition skips it */
      new_len++;
    else if (unichar < 0x20 || unichar > 0x7e) {
      /* &#xXX; (0/0x00 to 255/0xff) */
      new_len += 6;
      if(unichar > 0xff)
        /* &#xXXXX; */
        new_len+=2;
      if(unichar > 0xffff)
        /* &#xXXXXX; */
        new_len++;
      if(unichar > 0xfffff)
        /* &#xXXXXXX; */
        new_len++;
    } else
      new_len++;

    unichar_len--; /* since loop does len-- */
    p += unichar_len; l -= unichar_len;
  }

  new_string=RAPTOR_MALLOC(cstring, new_len+1);
  if(!new_string)
    return NULL;

  if(new_len_p)
    *new_len_p=new_len;
  
  for(l=len, p=string, q=new_string; l; p++, l--) {
    unichar_len=raptor_utf8_to_unicode_char(&unichar, p, l);

    if(unichar == '&') {
      strncpy(q, "&amp;", 5);
      q+= 5;
    } else if (unichar == '<') {
      strncpy(q, "&lt;", 4);
      q+= 4;
    } else if (unichar == '>') {
      strncpy(q, "&gt;", 4);
      q+= 4;
    } else if (quote && unichar == quote) {
      if(quote == '\'')  
        strncpy(q, "&apos;", 6);
      else
        strncpy(q, "&quot;", 6);
      q+= 6;
    } else if (unichar == 0x09 || unichar == 0x0d || unichar == 0x0a)
      /* XML whitespace excluding 0x20, since next condition skips it */
      *q++ = unichar;
    else if (unichar < 0x20 || unichar > 0x7e) {
      if(unichar < 0x100) {
        /* &#xXX; */
        sprintf(q, "&#x%02lX;", unichar);
        q+= 6;
      } else if (unichar < 0x10000) {
        /* &#xXXXX; */
        sprintf(q, "&#x%04lX;", unichar);
        q+= 8;
      } else if (unichar < 0x100000) {
        /* &#xXXXXX; */
        sprintf(q, "&#x%05lX;", unichar);
        q+= 9;
      } else {
        /* &#xXXXXXX; */
        sprintf(q, "&#x%06lX;", unichar);
        q+= 10;
      }
    } else
      *q++ = unichar;

    unichar_len--; /* since loop does len-- */
    p += unichar_len; l -= unichar_len;
  }

  /* Terminate new string */
  *q = '\0';

  return new_string;
}


#ifdef RAPTOR_DEBUG
void
raptor_stats_print(raptor_parser *rdf_parser, FILE *stream)
{
  if(!strcmp(rdf_parser->factory->name, "rdfxml")) {
    raptor_xml_parser *rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;
    fputs("raptor parser stats\n  ", stream);
    raptor_xml_parser_stats_print(rdf_xml_parser, stream);
  }
}
#endif





#ifdef STANDALONE

/* static prototypes */
void raptor_bad_string_print(const unsigned char *input, FILE *stream);
int main(int argc, char *argv[]);

void
raptor_bad_string_print(const unsigned char *input, FILE *stream)
{
  while(*input) {
    char c=*input;
    if(isprint(c))
      fputc(c, stream);
    else
      fprintf(stream, "\\x%02X", (c & 0xff));
    input++;
  }
}


int
main(int argc, char *argv[]) 
{
  char *program=argv[0];
  struct tv {
    const unsigned char *string;
    const char quote;
    const unsigned char *result;
  };
  struct tv *t;
  struct tv test_values[]={
    {"&", 0, "&amp;"},
    {"<", 0, "&lt;"},
    {">", 0, "&gt;"},

    {"'&'", '\'', "&apos;&amp;&apos;"},
    {"'<'", '\'', "&apos;&lt;&apos;"},
    {"'>'", '\'', "&apos;&gt;&apos;"},

    {"\"&\"", '\"', "&quot;&amp;&quot;"},
    {"\"<\"", '\"', "&quot;&lt;&quot;"},
    {"\">\"", '\"', "&quot;&gt;&quot;"},

    {"&amp;", 0, "&amp;amp;"},
    {"<foo>", 0, "&lt;foo&gt;"},

    {"\x1f", 0, "&#x1F;"},
    {"\xc2\x80", 0, "&#x80;"},
    {"\xe0\xa0\x80", 0, "&#x0800;"},
    {"\xf0\x90\x80\x80", 0, "&#x10000;"},

    {"\x7f", 0, "&#x7F;"},
    {"\xdf\xbf", 0, "&#x07FF;"},
    {"\xef\xbf\xbd", 0, "&#xFFFD;"},
    {"\xf4\x8f\xbf\xbf", 0, "&#x10FFFF;"},

    {"\xc3\xbf", 0, "&#xFF;"},
    {"\xf0\x8f\xbf\xbf", 0, "&#xFFFF;"},

    {NULL, 0, 0}
  };
  int i;
  int failures=0;

  for(i=0; (t=&test_values[i]) && t->string; i++) {
    const unsigned char *utf8_string=t->string;
    int quote=t->quote;
    size_t utf8_string_len=strlen(utf8_string);
    unsigned char *xml_string;

    xml_string=raptor_xml_escape_string(NULL, utf8_string, utf8_string_len,
                                        NULL, quote);
    if(!xml_string) {
      fprintf(stderr, "%s: raptor_xml_escape_string FAILED to escape string '",
              program);
      raptor_bad_string_print(utf8_string, stderr);
      fputs("'\n", stderr);
      failures++;
      continue;
    }
    if(strcmp(xml_string, t->result)) {
      fprintf(stderr, "%s: raptor_xml_escape_string FAILED to escape string '",
              program);
      raptor_bad_string_print(utf8_string, stderr);
      fprintf(stderr, "', expected '%s', result was '%s'\n",
              t->result, xml_string);
      failures++;
      continue;
    }
    
    fprintf(stderr, "%s: raptor_xml_escape_string escaped string to '%s' ok\n",
            program, xml_string);
    RAPTOR_FREE(cstring, xml_string);
  }

  if(!failures)
    fprintf(stderr, "%s: raptor_xml_escape_string all tests OK\n", program);

  return failures;
}

#endif
