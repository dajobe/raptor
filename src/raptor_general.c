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


/**
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
}


/**
 * raptor_finish - Terminate the raptor library
 *
 * Cleans up state of the library.
 **/
void
raptor_finish(void) 
{
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


/*
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



/**
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


int
raptor_start_parse(raptor_parser *rdf_parser, raptor_uri *uri) {
  raptor_uri_handler *uri_handler;
  void *uri_context;

  rdf_parser->base_uri=uri;
  rdf_parser->locator.uri=uri;
  rdf_parser->locator.line= rdf_parser->locator.column = 0;

  raptor_namespaces_free(&rdf_parser->namespaces);

  raptor_uri_get_handler(&uri_handler, &uri_context);
  raptor_namespaces_init(&rdf_parser->namespaces,
                         uri_handler, uri_context,
                         raptor_parser_error, rdf_parser);

  return rdf_parser->factory->start(rdf_parser, uri);
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
  raptor_www *www=raptor_www_new();

  if(!www)
    return 1;

  raptor_www_set_write_bytes_handler(www, raptor_parse_uri_write_bytes, 
                                     rdf_parser);

  if(raptor_start_parse(rdf_parser, base_uri))
    return 1;

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
raptor_parser_abort(raptor_parser *parser, const char *reason) {
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


void
raptor_print_statement_as_ntriples(const raptor_statement * statement,
                                   FILE *stream) 
{
  if(statement->subject_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
    fprintf(stream, "_:%s", (const char*)statement->subject);
  else { /* must be URI */
    fputc('<', stream);
    raptor_print_ntriples_string(stream, raptor_uri_as_string((raptor_uri*)statement->subject), '\0');
    fputc('>', stream);
  }
  fputc(' ', stream);

  if(statement->predicate_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL)
    fprintf(stream, "<http://www.w3.org/1999/02/22-rdf-syntax-ns#_%d>",
            *((int*)statement->predicate));
  else { /* must be URI */
    fputc('<', stream);
    raptor_print_ntriples_string(stream, raptor_uri_as_string((raptor_uri*)statement->predicate), '\0');
    fputc('>', stream);
  }
  fputc(' ', stream);

  if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
    fputc('"', stream);
    raptor_print_ntriples_string(stream, (const char*)statement->object, '"');
    fputc('"', stream);
    if(statement->object_literal_language)
      fprintf(stream, "@%s",  (const char*)statement->object_literal_language);
    if(statement->object_literal_datatype)
      fprintf(stream, "^^<%s>", 
              raptor_uri_as_string((raptor_uri*)statement->object_literal_datatype));
  } else if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL) {
    fputc('"', stream);
    raptor_print_ntriples_string(stream, (const char*)statement->object, '"');
    fputc('"', stream);
    if(statement->object_literal_language)
      fprintf(stream, "@%s",  (const char*)statement->object_literal_language);
    fputs("^^<http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral>", stream);
  } else if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
    fprintf(stream, "_:%s", (const char*)statement->object);
  else if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL)
    fprintf(stream, "<http://www.w3.org/1999/02/22-rdf-syntax-ns#_%d>",
            *((int*)statement->object));
  else { /* must be URI */
    fputc('<', stream);
    raptor_print_ntriples_string(stream, raptor_uri_as_string((raptor_uri*)statement->object), '\0');
    fputc('>', stream);
  }

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
