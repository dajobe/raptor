/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_general.c - Raptor general routines
 *
 * $Id$
 *
 * Copyright (C) 2000-2004 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology - http://www.ilrt.bris.ac.uk/
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
#include <raptor_config.h>
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
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


/* prototypes for helper functions */
static void raptor_delete_parser_factories(void);
static raptor_parser_factory* raptor_get_parser_factory(const char *name);
static void raptor_print_statement_part_as_ntriples(FILE* stream, const void *term, raptor_identifier_type type, raptor_uri* literal_datatype, const unsigned char *literal_language);


/* statics */

/* list of parser factories */
static raptor_parser_factory* parsers=NULL;

const char * const raptor_short_copyright_string = "Copyright (C) 2000-2004 David Beckett, ILRT, University of Bristol";

const char * const raptor_copyright_string = "Copyright (C) 2000-2004 David Beckett - http://purl.org/net/dajobe/\nInstitute for Learning and Research Technology - http://www.ilrt.bristol.ac.uk/,\nUniversity of Bristol - http://www.bristol.ac.uk/";

const char * const raptor_version_string = VERSION;

const unsigned int raptor_version_major = RAPTOR_VERSION_MAJOR;
const unsigned int raptor_version_minor = RAPTOR_VERSION_MINOR;
const unsigned int raptor_version_release = RAPTOR_VERSION_RELEASE;

const unsigned int raptor_version_decimal = RAPTOR_VERSION_DECIMAL;



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
#ifdef RAPTOR_PARSER_RSS
  raptor_init_parser_rss();
#endif
  raptor_init_parser_turtle();
  raptor_init_parser_ntriples();
  raptor_init_parser_rdfxml();

  raptor_uri_init();
  raptor_www_init();
}


/**
 * raptor_finish - Terminate the raptor library
 *
 * Cleans up state of the library.
 **/
void
raptor_finish(void) 
{
  raptor_www_finish();
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

    if(factory->finish_factory)
      factory->finish_factory(factory);

    RAPTOR_FREE(raptor_parser_factory, factory->name);
    RAPTOR_FREE(raptor_parser_factory, factory->label);
    if(factory->alias)
      RAPTOR_FREE(raptor_parser_factory, factory->alias);
    if(factory->mime_type)
      RAPTOR_FREE(raptor_parser_factory, factory->mime_type);
    if(factory->uri_string)
      RAPTOR_FREE(raptor_parser_factory, factory->uri_string);

    RAPTOR_FREE(raptor_parser_factory, factory);
  }
  parsers=NULL;
}


/* class methods */

/*
 * raptor_parser_register_factory - Register a syntax handled by a parser factory
 * @name: the short syntax name
 * @label: readable label for syntax
 * @mime_type: MIME type of the syntax handled by the parser (or NULL)
 * @uri_string: URI string of the syntax (or NULL)
 * @factory: pointer to function to call to register the factory
 * 
 * INTERNAL
 *
 **/
void
raptor_parser_register_factory(const char *name, const char *label,
                               const char *mime_type,
                               const char *alias,
                               const unsigned char *uri_string,
                               void (*factory) (raptor_parser_factory*)) 
{
  raptor_parser_factory *parser, *h;
  char *name_copy, *label_copy, *mime_type_copy, *alias_copy;
  unsigned char *uri_string_copy;
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG4("Received registration for syntax %s '%s' with alias '%s'\n", 
                name, label, (alias ? alias : "none"));
  RAPTOR_DEBUG4(raptor_parser_register_factory,
                "MIME type %s, URI %s\n", 
                (mime_type ? mime_type : "none"),
                (uri_string ? uri_string : "none"));
#endif
  
  parser=(raptor_parser_factory*)RAPTOR_CALLOC(raptor_parser_factory, 1,
                                               sizeof(raptor_parser_factory));
  if(!parser)
    RAPTOR_FATAL1("Out of memory\n");

  for(h = parsers; h; h = h->next ) {
    if(!strcmp(h->name, name) ||
       (alias && !strcmp(h->name, alias))) {
      RAPTOR_FATAL2("parser %s already registered\n", h->name);
    }
  }
  
  name_copy=(char*)RAPTOR_CALLOC(cstring, strlen(name)+1, 1);
  if(!name_copy) {
    RAPTOR_FREE(raptor_parser, parser);
    RAPTOR_FATAL1("Out of memory\n");
  }
  strcpy(name_copy, name);
  parser->name=name_copy;
        
  label_copy=(char*)RAPTOR_CALLOC(cstring, strlen(label)+1, 1);
  if(!label_copy) {
    RAPTOR_FREE(raptor_parser, parser);
    RAPTOR_FATAL1("Out of memory\n");
  }
  strcpy(label_copy, label);
  parser->label=label_copy;

  if(mime_type) {
    mime_type_copy=(char*)RAPTOR_CALLOC(cstring, strlen(mime_type)+1, 1);
    if(!mime_type_copy) {
      RAPTOR_FREE(raptor_parser, parser);
      RAPTOR_FATAL1("Out of memory\n");
    }
    strcpy(mime_type_copy, mime_type);
    parser->mime_type=mime_type_copy;
  }

  if(uri_string) {
    uri_string_copy=(unsigned char*)RAPTOR_CALLOC(cstring, strlen((const char*)uri_string)+1, 1);
    if(!uri_string_copy) {
    RAPTOR_FREE(raptor_parser, parser);
    RAPTOR_FATAL1("Out of memory\n");
    }
    strcpy((char*)uri_string_copy, (const char*)uri_string);
    parser->uri_string=uri_string_copy;
  }
        
  if(alias) {
    alias_copy=(char*)RAPTOR_CALLOC(cstring, strlen(alias)+1, 1);
    if(!alias_copy) {
      RAPTOR_FREE(raptor_parser, parser);
      RAPTOR_FATAL1("Out of memory\n");
    }
    strcpy(alias_copy, alias);
    parser->alias=alias_copy;
  }

  /* Call the parser registration function on the new object */
  (*factory)(parser);
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG3("%s has context size %d\n", name, parser->context_length);
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
      RAPTOR_DEBUG1("No (default) parsers registered\n");
      return NULL;
    }
  } else {
    for(factory=parsers; factory; factory=factory->next) {
      if(!strcmp(factory->name, name) ||
         (factory->alias && !strcmp(factory->alias, name)))
        break;
    }
    /* else FACTORY name not found */
    if(!factory) {
      RAPTOR_DEBUG2("No parser with name %s found\n", name);
      return NULL;
    }
  }
        
  return factory;
}


/**
 * raptor_syntaxes_enumerate - Get information on syntaxes
 * @counter: index into the list of syntaxes
 * @name: pointer to store the name of the syntax (or NULL)
 * @label: pointer to store syntax readable label (or NULL)
 * @mime_type: pointer to store syntax MIME Type (or NULL)
 * @uri_string: pointer to store syntax URI string (or NULL)
 * 
 * Return value: non 0 on failure of if counter is out of range
 **/
int
raptor_syntaxes_enumerate(const unsigned int counter,
                          const char **name, const char **label,
                          const char **mime_type,
                          const unsigned char **uri_string)
{
  unsigned int i;
  raptor_parser_factory *factory=parsers;

  if(!factory || counter < 0)
    return 1;

  for(i=0; factory && i<=counter ; i++, factory=factory->next) {
    if(i == counter) {
      if(name)
        *name=factory->name;
      if(label)
        *label=factory->label;
      if(mime_type)
        *mime_type=factory->mime_type;
      if(uri_string)
        *uri_string=factory->uri_string;
      return 0;
    }
  }
        
  return 1;
}


/**
 * raptor_parsers_enumerate - Get list of syntax parsers
 * @counter: index to list of parsers
 * @name: pointer to store syntax name (or NULL)
 * @label: pointer to store syntax label (or NULL)
 * 
 * Return value: non 0 on failure of if counter is out of range
 **/
int
raptor_parsers_enumerate(const unsigned int counter,
                         const char **name, const char **label)
{
  return raptor_syntaxes_enumerate(counter, name, label, NULL, NULL);
}


/**
 * raptor_syntax_name_check -  Check name of a parser
 * @name: the syntax name
 *
 * Return value: non 0 if name is a known syntax name
 */
int
raptor_syntax_name_check(const char *name) {
  return (raptor_get_parser_factory(name) != NULL);
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
  
#ifdef RAPTOR_XML_LIBXML
  rdf_parser->magic=RAPTOR_LIBXML_MAGIC;
#endif  
  rdf_parser->factory=factory;

  rdf_parser->failed=0;

  /* Initialise default (lax) feature values */
  raptor_set_parser_strict(rdf_parser, 0);

  if(factory->init(rdf_parser, name)) {
    raptor_free_parser(rdf_parser);
    return NULL;
  }
  
  return rdf_parser;
}


/**
 * raptor_new_parser_for_content - Constructor - create a new raptor_parser
 * @uri: URI identifying the syntax (or NULL)
 * @mime_type: mime type identifying the content (or NULL)
 * @buffer: buffer of content to guess (or NULL)
 * @len: length of buffer
 * @identifier: identifier of content (or NULL)
 * 
 * Users &raptor_guess_parser_name to find a parser by scoring
 * recognition of the syntax by a block of characters, the content
 * identifier or a mime type.  The content identifier is typically a
 * filename or URI or some other identifier.
 * 
 * Return value: a new &raptor_parser object or NULL on failure
 **/
raptor_parser*
raptor_new_parser_for_content(raptor_uri *uri, const char *mime_type,
                              const unsigned char *buffer, size_t len,
                              const unsigned char *identifier)
{
  return raptor_new_parser(raptor_guess_parser_name(uri, mime_type, buffer, len, identifier));
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
  if(uri)
    uri=raptor_uri_copy(uri);
  
  if(rdf_parser->base_uri)
    raptor_free_uri(rdf_parser->base_uri);

  rdf_parser->base_uri=uri;
  rdf_parser->locator.uri=uri;
  rdf_parser->locator.line= rdf_parser->locator.column = 0;

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

  if(rdf_parser->default_generate_id_handler_prefix)
    RAPTOR_FREE(cstring, rdf_parser->default_generate_id_handler_prefix);

  RAPTOR_FREE(raptor_parser, rdf_parser);
}


/* Size of XML buffer to use when reading from a file */
#define RAPTOR_READ_BUFFER_SIZE 1024


/**
 * raptor_parse_file_stream - Retrieve the RDF/XML content from a FILE*
 * @rdf_parser: parser
 * @stream: FILE* of RDF content
 * @filename: filename of content or NULL if it has no name
 * @base_uri: the base URI to use
 *
 * After draining the stream, fclose is not called on it internally.
 *
 * Return value: non 0 on failure
 **/
int
raptor_parse_file_stream(raptor_parser* rdf_parser,
                         FILE *stream, const char* filename,
                         raptor_uri *base_uri)
{
  /* Read buffer */
  unsigned char buffer[RAPTOR_READ_BUFFER_SIZE];
  int rc=0;
  raptor_locator *locator=&rdf_parser->locator;

  if(!stream || !base_uri)
    return 1;

  locator->line= locator->column = -1;
  locator->file= filename;

  if(raptor_start_parse(rdf_parser, base_uri))
    return 1;
  
  while(!feof(stream)) {
    int len=fread(buffer, 1, RAPTOR_READ_BUFFER_SIZE, stream);
    int is_end=(len < RAPTOR_READ_BUFFER_SIZE);
    rc=raptor_parse_chunk(rdf_parser, buffer, len, is_end);
    if(rc || is_end)
      break;
  }

  return (rc != 0);
}


/**
 * raptor_parse_file - Retrieve the RDF/XML content at a file URI
 * @rdf_parser: parser
 * @uri: URI of RDF content or NULL to read from standard input
 * @base_uri: the base URI to use (or NULL if the same)
 *
 * If uri is NULL (source is stdin), then the base_uri is required.
 * 
 * Return value: non 0 on failure
 **/
int
raptor_parse_file(raptor_parser* rdf_parser, raptor_uri *uri,
                  raptor_uri *base_uri) 
{
  int rc=0;
  int free_base_uri=0;
  const char *filename=NULL;
  FILE *fh=NULL;
#if defined(HAVE_UNISTD_H) && defined(HAVE_SYS_STAT_H)
  struct stat buf;
#endif

  if(uri) {
    filename=raptor_uri_uri_string_to_filename(raptor_uri_as_string(uri));
    if(!filename)
      return 1;

#if defined(HAVE_UNISTD_H) && defined(HAVE_SYS_STAT_H)
    if(!stat(filename, &buf) && S_ISDIR(buf.st_mode)) {
      raptor_parser_error(rdf_parser, "Cannot read from a directory '%s'",
                          filename);
      goto cleanup;
    }
#endif

    fh=fopen(filename, "r");
    if(!fh) {
      raptor_parser_error(rdf_parser, "file '%s' open failed - %s",
                          filename, strerror(errno));
      goto cleanup;
    }
    if(!base_uri) {
      base_uri=raptor_uri_copy(uri);
      free_base_uri=1;
    }
  } else {
    if(!base_uri)
      return 1;
    fh=stdin;
  }

  rc=raptor_parse_file_stream(rdf_parser, fh, filename, base_uri);

  cleanup:
  if(uri) {
    if(fh)
      fclose(fh);
    RAPTOR_FREE(cstring, (void*)filename);
  }
  if(free_base_uri)
    raptor_free_uri(base_uri);

  return rc;
}


static void
raptor_parse_uri_write_bytes(raptor_www* www,
                             void *userdata, const void *ptr, size_t size, size_t nmemb)
{
  raptor_parser* rdf_parser=(raptor_parser*)userdata;
  int len=size*nmemb;

  if(raptor_parse_chunk(rdf_parser, (unsigned char*)ptr, len, 0))
    raptor_www_abort(www, "Parsing failed");
}


/**
 * raptor_parse_uri - Retrieve the RDF/XML content at URI
 * @rdf_parser: parser
 * @uri: URI of RDF content
 * @base_uri: the base URI to use (or NULL if the same)
 * 
 * Sends an HTTP Accept: header whent the URI is of the HTTP protocol,
 * see raptor_parse_uri_with_connection for details.
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
 * @connection: connection object pointer or NULL to create a new one
 * 
 * When @connection is NULL and a MIME Type exists for the parser
 * type - such as returned by raptor_get_mime_type(parser) - this
 * type is sent in an HTTP Accept: header in the form
 * Accept: MIME-TYPE along with a wildcard of 0.1 quality, so MIME-TYPE is
 * prefered rather than the sole answer.  The latter part may not be
 * necessary but should ensure an HTTP 200 response.
 *
 * Return value: non 0 on failure
 **/
int
raptor_parse_uri_with_connection(raptor_parser* rdf_parser, raptor_uri *uri,
                                 raptor_uri *base_uri, void *connection)
{
  raptor_www *www;
  const char* mime_type;
  
  if(!base_uri)
    base_uri=uri;
 
  if(connection) {
    www=raptor_www_new_with_connection(connection);
    if(!www)
      return 1;
  } else {
    www=raptor_www_new();
    if(!www)
      return 1;
    if((mime_type=raptor_get_mime_type(rdf_parser))) {
      char *accept_h=(char*)RAPTOR_MALLOC(cstring, strlen(mime_type)+11);
      strcpy(accept_h, mime_type);
      strcat(accept_h, ",*/*;q=0.1"); /* strlen()=4 chars */
      raptor_www_set_http_accept(www, accept_h);
      RAPTOR_FREE(cstring, accept_h);
    }
  }
  
  raptor_www_set_write_bytes_handler(www, raptor_parse_uri_write_bytes, 
                                     rdf_parser);

  if(raptor_start_parse(rdf_parser, base_uri)) {
    raptor_www_free(www);
    return 1;
  }

  if(raptor_www_fetch(www, uri)) {
    raptor_www_free(www);
    return 1;
  }

  raptor_parse_chunk(rdf_parser, NULL, 0, 1);

  raptor_www_free(www);

  return rdf_parser->failed;
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


/* 
 * Thanks to the patch in this Debian bug for the solution
 * to the crash inside vsnprintf on some architectures.
 *
 * "reuse of args inside the while(1) loop is in violation of the
 * specs and only happens to work by accident on other systems."
 *
 * http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=104325 
 */

#ifndef va_copy
#ifdef __va_copy
#define va_copy(dest,src) __va_copy(dest,src)
#else
#define va_copy(dest,src) (dest) = (src)
#endif
#endif

/* Compatiblity wrapper */
char*
raptor_vsnprintf(const char *message, va_list arguments) 
{
  char empty_buffer[1];
  int len;
  char *buffer=NULL;
  va_list args_copy;

#ifdef HAVE_C99_VSNPRINTF
  /* copy for re-use */
  va_copy(args_copy, arguments);
  len=vsnprintf(empty_buffer, 1, message, args_copy)+1;
  va_end(args_copy);

  if(len<=0)
    return NULL;
  
  buffer=(char*)RAPTOR_MALLOC(cstring, len);
  if(buffer) {
    /* copy for re-use */
    va_copy(args_copy, arguments);
    vsnprintf(buffer, len, message, args_copy);
    va_end(args_copy);
  }
#else
  /* This vsnprintf doesn't return number of bytes required */
  int size=2;
      
  while(1) {
    buffer=(char*)RAPTOR_MALLOC(cstring, size+1);
    if(!buffer)
      break;
    
    /* copy for re-use */
    va_copy(args_copy, arguments);
    len=vsnprintf(buffer, size, message, args_copy);
    va_end(args_copy);

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
 * raptor_parser_simple_error - Error from a parser - Internal
 *
 * Matches the raptor_simple_message_handler API but same as
 * raptor_parser_error 
 **/
void
raptor_parser_simple_error(void* parser, const char *message, ...)
{
  va_list arguments;

  va_start(arguments, message);

  raptor_parser_error_varargs((raptor_parser*)parser, message, arguments);
  
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
    size_t length;
    if(!buffer) {
      fprintf(stderr, "raptor_parser_error_varargs: Out of memory\n");
      return;
    }
    length=strlen(buffer);
    if(buffer[length-1]=='\n')
      buffer[length-1]='\0';
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
    size_t length;
    if(!buffer) {
      fprintf(stderr, "raptor_parser_warning_varargs: Out of memory\n");
      return;
    }
    length=strlen(buffer);
    if(buffer[length-1]=='\n')
      buffer[length-1]='\0';
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


/**
 * raptor_set_statement_handler - Set the statement handler function for the parser
 * @parser: &raptor_parser parser object
 * @user_data: user data pointer for callback
 * @handler: new statement callback function
 * 
 **/
void
raptor_set_statement_handler(raptor_parser* parser,
                             void *user_data,
                             raptor_statement_handler handler)
{
  parser->user_data=user_data;
  parser->statement_handler=handler;
}


/**
 * raptor_set_generate_id_handler - Set the generate ID handler function for the parser
 * @parser: &raptor_parser parser object
 * @user_data: user data pointer for callback
 * @handler: generate ID callback function
 *
 * Sets the function to generate IDs for the parser.  The handler is
 * called with the @user_data parameter and an ID type of either
 * RAPTOR_GENID_TYPE_BNODEID or RAPTOR_GENID_TYPE_BAGID (latter is deprecated).
 *
 * The final argument of the callback method is user_bnodeid, the value of
 * the rdf:nodeID attribute that the user provided if any (or NULL).
 * It can either be returned directly as the generated value when present or
 * modified.  The passed in value must be free()d if it is not used.
 *
 * If handler is NULL, the default method is used
 * 
 **/
void
raptor_set_generate_id_handler(raptor_parser* parser,
                               void *user_data,
                               raptor_generate_id_handler handler)
{
  parser->generate_id_handler_user_data=user_data;
  parser->generate_id_handler=handler;
}


static struct
{
  raptor_feature feature;
  const char *name;
  const char *label;
} raptor_features_list [RAPTOR_FEATURE_LAST+1]= {
  { RAPTOR_FEATURE_SCANNING                ,"scanForRDF", "Scan for rdf:RDF in XML content" },
  { RAPTOR_FEATURE_ASSUME_IS_RDF           ,"assumeIsRDF", "Assume content is RDF/XML, don't require rdf:RDF" },
  { RAPTOR_FEATURE_ALLOW_NON_NS_ATTRIBUTES ,"allowNonNsAttributes", "Allow bare 'name' rather than namespaced 'rdf:name'" },
  { RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES  ,"allowOtherParsetypes", "Allow user-defined rdf:parseType values" },
  { RAPTOR_FEATURE_ALLOW_BAGID             ,"allowBagID", "Allow rdf:bagID" },
  { RAPTOR_FEATURE_ALLOW_RDF_TYPE_RDF_LIST ,"allowRDFtypeRDFlist", "Generate the collection rdf:type rdf:List triple" },
  { RAPTOR_FEATURE_NORMALIZE_LANGUAGE      ,"normalizeLanguage", "Normalize xml:lang values to lowercase" },
  { RAPTOR_FEATURE_NON_NFC_FATAL           ,"nonNFCfatal", "Make non-NFC literals cause a fatal error" },
  { RAPTOR_FEATURE_WARN_OTHER_PARSETYPES   ,"warnOtherParseTypes", "Warn about unknown rdf:parseType values" }
};


static const char *raptor_feature_uri_prefix="http://feature.librdf.org/raptor-";
/* NOTE: this is strlen(raptor_feature_uri_prefix) */
#define RAPTOR_FEATURE_URI_PREFIX_LEN 33

/**
 * raptor_features_enumerate - Get list of syntax features
 * @counter: feature enumeration (0+)
 * @name: pointer to store feature short name (or NULL)
 * @uri: pointer to store feature URI (or NULL)
 * @label: pointer to feature label (or NULL)
 * 
 * If uri is not NULL, a pointer toa new raptor_uri is returned
 * that must be freed by the caller with raptor_free_uri().
 *
 * Return value: non 0 on failure or if the feature is unknown
 **/
int
raptor_features_enumerate(const raptor_feature feature,
                          const char **name, 
                          raptor_uri **uri, const char **label)
{
  int i;

  for(i=0; i <= RAPTOR_FEATURE_LAST; i++)
    if(raptor_features_list[i].feature == feature) {
      if(name)
        *name=raptor_features_list[i].name;
      
      if(uri) {
        raptor_uri *base_uri=raptor_new_uri((const unsigned char*)raptor_feature_uri_prefix);
        if(!base_uri)
          return 1;
        
        *uri=raptor_new_uri_from_uri_local_name(base_uri,
                                                (const unsigned char*)raptor_features_list[i].name);
        raptor_free_uri(base_uri);
      }
      if(label)
        *label=raptor_features_list[i].label;
      return 0;
    }

  return 1;
}



/**
 * raptor_set_feature - Set various parser features
 * @parser: &raptor_parser parser object
 * @feature: feature to set from enumerated &raptor_feature values
 * @value: integer feature value (0 or larger)
 * 
 * The allowed features are available via raptor_features_enumerate().
 *
 * Return value: non 0 on failure or if the feature is unknown
 **/
int
raptor_set_feature(raptor_parser *parser, raptor_feature feature, int value)
{
  if(value < 0)
    return -1;
  
  switch(feature) {
    case RAPTOR_FEATURE_SCANNING:
      parser->feature_scanning_for_rdf_RDF=value;
      break;

    case RAPTOR_FEATURE_ASSUME_IS_RDF:
      break;

    case RAPTOR_FEATURE_ALLOW_NON_NS_ATTRIBUTES:
      parser->feature_allow_non_ns_attributes=value;
      break;

    case RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES:
      parser->feature_allow_other_parseTypes=value;
      break;

    case RAPTOR_FEATURE_ALLOW_BAGID:
      parser->feature_allow_bagID=value;
      break;

    case RAPTOR_FEATURE_ALLOW_RDF_TYPE_RDF_LIST:
      parser->feature_allow_rdf_type_rdf_List=value;
      break;

    case RAPTOR_FEATURE_NORMALIZE_LANGUAGE:
      parser->feature_normalize_language=value;
      break;

    case RAPTOR_FEATURE_NON_NFC_FATAL:
      parser->feature_non_nfc_fatal=value;
      break;

    case RAPTOR_FEATURE_WARN_OTHER_PARSETYPES:
      parser->feature_warn_other_parseTypes=value;
      break;

    default:
      return -1;
      break;
  }

  return 0;
}


/**
 * raptor_get_feature - Get various parser features
 * @parser: &raptor_parser parser object
 * 
 * The allowed features are available via raptor_features_enumerate().
 *
 * Note: no feature value is negative
 *
 * Return value: feature value or < 0 for an illegal feature
 **/
int
raptor_get_feature(raptor_parser *parser, raptor_feature feature)
{
  int result= -1;
  
  switch(feature) {
    case RAPTOR_FEATURE_SCANNING:
      result=(parser->feature_scanning_for_rdf_RDF != 0);
      break;

    case RAPTOR_FEATURE_ASSUME_IS_RDF:
      result=0;
      break;

    case RAPTOR_FEATURE_ALLOW_NON_NS_ATTRIBUTES:
      result=(parser->feature_allow_non_ns_attributes != 0);
      break;

    case RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES:
      result=(parser->feature_allow_other_parseTypes != 0);
      break;

    case RAPTOR_FEATURE_ALLOW_BAGID:
      result=(parser->feature_allow_bagID != 0);
      break;

    case RAPTOR_FEATURE_ALLOW_RDF_TYPE_RDF_LIST:
      result=(parser->feature_allow_rdf_type_rdf_List != 0);
      break;

    case RAPTOR_FEATURE_NORMALIZE_LANGUAGE:
      result=(parser->feature_normalize_language != 0);
      break;

    case RAPTOR_FEATURE_NON_NFC_FATAL:
      result=(parser->feature_non_nfc_fatal != 0);
      break;
      
    case RAPTOR_FEATURE_WARN_OTHER_PARSETYPES:
      result=(parser->feature_warn_other_parseTypes != 0);
      break;

    default:
      break;
  }
  
  return result;
}


/**
 * raptor_feature_from_uri - Turn a parser feature URI into an enum
 * @parser: &raptor_parser parser object
 * @uri: feature URI
 * 
 * The allowed feature URIs are available via raptor_features_enumerate().
 *
 * Return value: < 0 if the feature is unknown
 **/
raptor_feature
raptor_feature_from_uri(raptor_uri *uri)
{
  unsigned char *uri_string;
  int i;
  raptor_feature feature= (raptor_feature)-1;
  
  if(!uri)
    return feature;
  
  uri_string=raptor_uri_as_string(uri);
  if(strncmp((const char*)uri_string, raptor_feature_uri_prefix,
             RAPTOR_FEATURE_URI_PREFIX_LEN))
    return feature;

  uri_string += RAPTOR_FEATURE_URI_PREFIX_LEN;

  for(i=0; i <= RAPTOR_FEATURE_LAST; i++)
    if(!strcmp(raptor_features_list[i].name, (const char*)uri_string)) {
      feature=(raptor_feature)i;
      break;
    }

  return feature;
}


/**
 * raptor_set_parser_strict - Set parser to strict / lax mode
 * @rdf_parser: &raptor_parse object
 * @is_strict: Non 0 for strict parsing
 * 
 **/
void
raptor_set_parser_strict(raptor_parser* rdf_parser, int is_strict)
{
  is_strict=(is_strict) ? 1 : 0;

  /* Initialise default parser mode */
  rdf_parser->feature_scanning_for_rdf_RDF=0;

  rdf_parser->feature_allow_non_ns_attributes=!is_strict;
  rdf_parser->feature_allow_other_parseTypes=!is_strict;
  rdf_parser->feature_allow_bagID=!is_strict;
  rdf_parser->feature_allow_rdf_type_rdf_List=0;
  rdf_parser->feature_normalize_language=1;
  rdf_parser->feature_non_nfc_fatal=is_strict;
  rdf_parser->feature_warn_other_parseTypes=!is_strict;
}


/**
 * raptor_set_default_generate_id_parameters - Set default ID generation parameters
 * @rdf_parser: &raptor_parse object
 * @prefix: prefix string
 * @base: integer base identifier
 *
 * Sets the parameters for the default algoriothm used to generate IDs.
 * The default algorithm uses both @prefix and @base to generate a new
 * identifier.   The exact identifier generated is not guaranteed to
 * be a strict concatenation of @prefix and @base but will use both
 * parts.
 *
 * For finer control of the generated identifiers, use
 * &raptor_set_default_generate_id_handler.
 *
 * If prefix is NULL, the default prefix is used (currently "genid")
 * If base is less than 1, it is initialised to 1.
 * 
 **/
void
raptor_set_default_generate_id_parameters(raptor_parser* rdf_parser, 
                                          char *prefix, int base)
{
  char *prefix_copy=NULL;
  size_t length=0;

  if(--base<0)
    base=0;

  if(prefix) {
    length=strlen(prefix);
    
    prefix_copy=(char*)RAPTOR_MALLOC(cstring, length+1);
    if(!prefix_copy)
      return;
    strcpy(prefix_copy, prefix);
  }
  
  if(rdf_parser->default_generate_id_handler_prefix)
    RAPTOR_FREE(cstring, rdf_parser->default_generate_id_handler_prefix);

  rdf_parser->default_generate_id_handler_prefix=prefix_copy;
  rdf_parser->default_generate_id_handler_prefix_length=length;
  rdf_parser->default_generate_id_handler_base=base;
}


/**
 * raptor_get_name: Return the short name for the parser
 * @parser: &raptor_parser parser object
 **/
const char*
raptor_get_name(raptor_parser *rdf_parser) 
{
  return rdf_parser->factory->name;
}


/**
 * raptor_get_label: Return a readable label for the parser
 * @parser: &raptor_parser parser object
 **/
const char*
raptor_get_label(raptor_parser *rdf_parser) 
{
  return rdf_parser->factory->label;
}


/**
 * raptor_get_mime_type: Return MIME type for the parser
 * @parser: &raptor_parser parser object
 *
 * Return value: MIME type or NULL if none available
 **/
const char*
raptor_get_mime_type(raptor_parser *rdf_parser) 
{
  return rdf_parser->factory->mime_type;
}


/**
 * raptor_parse_abort - Abort an ongoing parse
 * @parser: &raptor_parser parser object
 * 
 * Causes any ongoing generation of statements by a parser to be
 * terminated and the parser to return controlto the application
 * as soon as draining any existing buffers.
 *
 * Most useful inside raptor_parse_file or raptor_parse_uri when
 * the Raptor library is directing the parsing and when one of the
 * callback handlers such as as set by raptor_set_statement_handler
 * requires to return to the main application code.
 **/
void
raptor_parse_abort(raptor_parser *parser)
{
  parser->failed=1;
}


const char * const raptor_xml_literal_datatype_uri_string="http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral";
const unsigned int raptor_xml_literal_datatype_uri_string_len=53;


/**
 * raptor_print_statement - Print a raptor_statement to a stream
 * @statement: &raptor_statement object to print
 * @stream: &FILE* stream
 *
 **/
void
raptor_print_statement(const raptor_statement * statement, FILE *stream) 
{
  fputc('[', stream);

  if(statement->subject_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
    fputs((const char*)statement->subject, stream);
  } else {
#ifdef RAPTOR_DEBUG
    if(!statement->subject)
      RAPTOR_FATAL1("Statement has NULL subject URI\n");
#endif
    fputs((const char*)raptor_uri_as_string((raptor_uri*)statement->subject), stream);
  }

  fputs(", ", stream);

  if(statement->predicate_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL)
    fprintf(stream, "[rdf:_%d]", *((int*)statement->predicate));
  else {
#ifdef RAPTOR_DEBUG
    if(!statement->predicate)
      RAPTOR_FATAL1("Statement has NULL predicate URI\n");
#endif
    fputs((const char*)raptor_uri_as_string((raptor_uri*)statement->predicate), stream);
  }

  fputs(", ", stream);

  if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_LITERAL || 
     statement->object_type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL) {
    if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL) {
      fputc('<', stream);
      fputs(raptor_xml_literal_datatype_uri_string, stream);
      fputc('>', stream);
    } else if(statement->object_literal_datatype) {
      fputc('<', stream);
      fputs((const char*)raptor_uri_as_string((raptor_uri*)statement->object_literal_datatype), stream);
      fputc('>', stream);
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
      RAPTOR_FATAL1("Statement has NULL object URI\n");
#endif
    fputs((const char*)raptor_uri_as_string((raptor_uri*)statement->object), stream);
  }

  fputc(']', stream);
}


void
raptor_print_statement_detailed(const raptor_statement * statement, 
                                int detailed, FILE *stream) 
{
  raptor_print_statement(statement, stream);
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
                             const unsigned char *string,
                             const char delim) 
{
  unsigned char c;
  size_t len=strlen((const char*)string);
  int unichar_len;
  unsigned long unichar;
  
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
    
    unichar_len=raptor_utf8_to_unicode_char(NULL, string, len);
    if(unichar_len < 0 || unichar_len > (int)len)
      /* UTF-8 encoding had an error or ended in the middle of a string */
      return 1;

    unichar_len=raptor_utf8_to_unicode_char(&unichar, string, len);
    
    if(unichar < 0x10000)
      fprintf(stream, "\\u%04lX", unichar);
    else
      fprintf(stream, "\\U%08lX", unichar);
    
    unichar_len--; /* since loop does len-- */
    string += unichar_len; len -= unichar_len;

  }

  return 0;
}



/**
 * raptor_statement_part_as_counted_string - Turns part of raptor statement into a N-Triples format counted string
 * @term: &raptor_statement part (subject, predicate, object)
 * @type: &raptor_statement part type
 * @literal_datatype: &raptor_statement part datatype
 * @literal_language: &raptor_statement part language
 * @len_p: Pointer to location to store length of new string (if not NULL)
 * 
 * Turns the given @term into an N-Triples escaped string using all the
 * escapes as defined in http://www.w3.org/TR/rdf-testcases/#ntriples
 *
 * The part (subject, predicate, object) of the raptor_statement is
 * typically passed in as @term, the part type (subject_type,
 * predicate_type, object_type) is passed in as @type.  When the part
 * is a literal, the @literal_datatype and @literal_language fields
 * are set, otherwise NULL (usually object_datatype,
 * object_literal_language).
 *
 * Return value: the new string or NULL on failure.  The length of
 * the new string is returned in *&len_p if len_p is not NULL.
 **/
unsigned char*
raptor_statement_part_as_counted_string(const void *term, 
                                        raptor_identifier_type type,
                                        raptor_uri* literal_datatype,
                                        const unsigned char *literal_language,
                                        size_t* len_p)
{
  size_t len, term_len, language_len, uri_len;
  unsigned char *s, *buffer, *uri_string;
  
  switch(type) {
    case RAPTOR_IDENTIFIER_TYPE_LITERAL:
    case RAPTOR_IDENTIFIER_TYPE_XML_LITERAL:
      term_len=strlen((const char*)term);
      len=2+term_len;
      if(literal_language && type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
        language_len=strlen((const char*)literal_language);
        len+= language_len+1;
      }
      if(type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL)
        len += 4+raptor_xml_literal_datatype_uri_string_len;
      else if(literal_datatype) {
        uri_string=raptor_uri_as_counted_string((raptor_uri*)literal_datatype, &uri_len);
        len += 4+uri_len;
      }
  
      buffer=(unsigned char*)RAPTOR_MALLOC(cstring, len+1);
      if(!buffer)
        return NULL;

      s=buffer;
      *s++ ='"';
      /* raptor_print_ntriples_string(stream, (const char*)term, '"'); */
      strcpy((char*)s, (const char*)term);
      s+= term_len;
      *s++ ='"';
      if(literal_language && type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
        *s++ ='@';
        strcpy((char*)s, (const char*)literal_language);
        s+= language_len;
      }

      if(type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL) {
        *s++ ='^';
        *s++ ='^';
        *s++ ='<';
        strcpy((char*)s, raptor_xml_literal_datatype_uri_string);
        s+= raptor_xml_literal_datatype_uri_string_len;
        *s++ ='>';
      } else if(literal_datatype) {
        *s++ ='^';
        *s++ ='^';
        *s++ ='<';
        strcpy((char*)s, (const char*)uri_string);
        s+= uri_len;
        *s++ ='>';
      }
      *s++ ='\0';
      
      break;
      
    case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
      len=2+strlen((const char*)term);
      buffer=(unsigned char*)RAPTOR_MALLOC(cstring, len+1);
      if(!buffer)
        return NULL;
      s=buffer;
      *s++ ='_';
      *s++ =':';
      strcpy((char*)s, (const char*)term);
      break;
      
    case RAPTOR_IDENTIFIER_TYPE_ORDINAL:
      /* FIXME - 46 for "<http://www.w3.org/1999/02/22-rdf-syntax-ns#_>" */
      len=46 + 13; 
      buffer=(unsigned char*)RAPTOR_MALLOC(cstring, len+1);
      if(!buffer)
        return NULL;

      sprintf((char*)buffer,
              "<http://www.w3.org/1999/02/22-rdf-syntax-ns#_%d>",
              *((int*)term));
      break;
  
    case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
    case RAPTOR_IDENTIFIER_TYPE_PREDICATE:
      uri_string=raptor_uri_as_counted_string((raptor_uri*)term, &uri_len);
      len=2+uri_len;
      buffer=(unsigned char*)RAPTOR_MALLOC(cstring, len+1);
      if(!buffer)
        return NULL;

      s=buffer;
      *s++ ='<';
      /* raptor_print_ntriples_string(stream, raptor_uri_as_string((raptor_uri*)term), '\0'); */
      strcpy((char*)s, (const char*)uri_string);
      s+= uri_len;
      *s++ ='>';
      *s++ ='\0';
      break;
      
    default:
      RAPTOR_FATAL2("Unknown type %d", type);
  }

  if(len_p)
    *len_p=len;
  
 return buffer;
}


/**
 * raptor_statement_part_as_string - Turns part of raptor statement into a N-Triples format string
 * @term: &raptor_statement part (subject, predicate, object)
 * @type: &raptor_statement part type
 * @literal_datatype: &raptor_statement part datatype
 * @literal_language: &raptor_statement part language
 * 
 * Turns the given @term into an N-Triples escaped string using all the
 * escapes as defined in http://www.w3.org/TR/rdf-testcases/#ntriples
 *
 * The part (subject, predicate, object) of the raptor_statement is
 * typically passed in as @term, the part type (subject_type,
 * predicate_type, object_type) is passed in as @type.  When the part
 * is a literal, the @literal_datatype and @literal_language fields
 * are set, otherwise NULL (usually object_datatype,
 * object_literal_language).
 *
 * Return value: the new string or NULL on failure.
 **/
unsigned char*
raptor_statement_part_as_string(const void *term, 
                                raptor_identifier_type type,
                                raptor_uri* literal_datatype,
                                const unsigned char *literal_language) {
     return raptor_statement_part_as_counted_string(term, type,
                                                    literal_datatype,
                                                    literal_language,
                                                    NULL);
}


static void
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
      raptor_print_ntriples_string(stream, (const unsigned char*)term, '"');
      fputc('"', stream);
      if(literal_language && type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
        fputc('@', stream);
        fputs((const char*)literal_language, stream);
      }
      if(type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL) {
        fputs("^^<", stream);
        fputs(raptor_xml_literal_datatype_uri_string, stream);
        fputc('>', stream);
      } else if(literal_datatype) {
        fputs("^^<", stream);
        fputs((const char*)raptor_uri_as_string((raptor_uri*)literal_datatype), stream);
        fputc('>', stream);
      }

      break;
      
    case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
      fputs("_:", stream);
      fputs((const char*)term, stream);
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
      RAPTOR_FATAL2("Unknown type %d", type);
  }
}


/**
 * raptor_print_statement_as_ntriples - Print a raptor_statement in N-Triples form
 * @statement: &raptor_statement to print
 * @stream: &FILE* stream
 * 
 **/
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




static unsigned char*
raptor_default_generate_id_handler(void *user_data, raptor_genid_type type,
                                   unsigned char *user_bnodeid) 
{
  raptor_parser *rdf_parser=(raptor_parser *)user_data;
  int id;
  unsigned char *buffer;
  int length;
  int tmpid;

  if(user_bnodeid)
    return user_bnodeid;

  id=++rdf_parser->default_generate_id_handler_base;

  tmpid=id;
  length=2; /* min length 1 + \0 */
  while(tmpid/=10)
    length++;

  if(rdf_parser->default_generate_id_handler_prefix)
    length += rdf_parser->default_generate_id_handler_prefix_length;
  else
    length += 5; /* genid */
  
  buffer=(unsigned char*)RAPTOR_MALLOC(cstring, length);
  if(!buffer)
    return NULL;
  if(rdf_parser->default_generate_id_handler_prefix) {
    strncpy((char*)buffer, rdf_parser->default_generate_id_handler_prefix,
            rdf_parser->default_generate_id_handler_prefix_length);
    sprintf((char*)buffer+rdf_parser->default_generate_id_handler_prefix_length,
            "%d", id);
  } else 
    sprintf((char*)buffer, "genid%d", id);

  return buffer;
}


/**
 * raptor_generate_id - Default generate id - internal
 **/
unsigned char *
raptor_generate_id(raptor_parser *rdf_parser, const int id_for_bag,
                   unsigned char *user_bnodeid)
{
  raptor_genid_type type=id_for_bag ? RAPTOR_GENID_TYPE_BNODEID :
                                      RAPTOR_GENID_TYPE_BAGID;
  if(rdf_parser->generate_id_handler)
    return rdf_parser->generate_id_handler(rdf_parser->generate_id_handler_user_data,
                                           type, user_bnodeid);
  else
    return raptor_default_generate_id_handler(rdf_parser, type, user_bnodeid);
}


/**
 * raptor_get_locator: Get the current raptor locator object
 * @rdf_parser: raptor parser
 * 
 * Return value: raptor locator
 **/
raptor_locator*
raptor_get_locator(raptor_parser *rdf_parser) 
{
  return &rdf_parser->locator;
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


int
raptor_check_ordinal(const unsigned char *name) {
  int ordinal= -1;
  unsigned char c;

  while((c=*name++)) {
    if(c < '0' || c > '9')
      return -1;
    if(ordinal <0)
      ordinal=0;
    ordinal *= 10;
    ordinal += (c - '0');
  }
  return ordinal;
}


struct syntax_score
{
  int score;
  raptor_parser_factory* factory;
};


static int
compare_syntax_score(const void *a, const void *b) {
  return ((struct syntax_score*)b)->score - ((struct syntax_score*)a)->score;
}
  

/**
 * raptor_guess_parser_name - guess a parser name for content
 * @uri: URI identifying the syntax (or NULL)
 * @mime_type: mime type identifying the content (or NULL)
 * @buffer: buffer of content to guess (or NULL)
 * @len: length of buffer
 * @identifier: identifier of content (or NULL)
 * 
 * Find a parser by scoring recognition of the syntax by a block of
 * characters, the content identifier or a mime type.  The content
 * identifier is typically a filename or URI or some other identifier.
 * 
 * Return value: a parser name or NULL if no guess could be made
 **/
const char*
raptor_guess_parser_name(raptor_uri *uri, const char *mime_type,
                         const unsigned char *buffer, size_t len,
                         const unsigned char *identifier)
{
  unsigned int i;
  raptor_parser_factory *factory=parsers;
  unsigned char *suffix=NULL;
  struct syntax_score scores[10]; /* FIXME - up to 10 parsers :) */

  if(identifier) {
    unsigned char *p=(unsigned char*)strrchr((const char*)identifier, '.');
    if(p) {
      unsigned char *from, *to;
      p++;
      suffix=(unsigned char*)RAPTOR_MALLOC(cstring, strlen((const char*)p)+1);
      if(!suffix)
        return NULL;
      for(from=p, to=suffix; *from; ) {
        unsigned char c=*from++;
        *to++=isupper((const char)c) ? (unsigned char)tolower((const char)c): c;
      }
      *to='\0';
    }
  }

  for(i=0; factory; i++, factory=factory->next) {
    int score= -1;
    
    if(mime_type && factory->mime_type &&
       !strcmp(mime_type, factory->mime_type))
      break;
    
    if(uri && factory->uri_string &&
       !strcmp((const char*)raptor_uri_as_string(uri), 
               (const char*)factory->uri_string))
      break;

    if(factory->recognise_syntax)
      score=factory->recognise_syntax(factory, buffer, len, 
                                      identifier, suffix, 
                                      mime_type);

    scores[i].score=score < 10 ? score : 10; scores[i].factory=factory;
    RAPTOR_DEBUG3("Score %15s : %d\n", factory->name, score);
  }
  
  if(!factory) {
    /* sort the scores and pick a factory */
    qsort(scores, i, sizeof(struct syntax_score), compare_syntax_score);
    if(scores[0].score >= 0)
      factory=scores[0].factory;
  }

  if(suffix)
    RAPTOR_FREE(cstring, suffix);

  return factory ? factory->name : NULL;
}


/**
 * raptor_free_memory - Free memory allocated inside raptor.
 * @ptr: memory pointer
 * 
 * Some systems require memory allocated in a library to
 * be deallocated in that library.  This function allows
 * memory allocated by raptor to be freed.
 *
 * Examples include the result of the '_to_' methods that returns
 * allocated memory such as raptor_uri_to_filename() or
 * raptor_uri_to_string().
 *
 **/
void
raptor_free_memory(void *ptr)
{
  RAPTOR_FREE(void, ptr);
}


#if defined (RAPTOR_DEBUG) && defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)

#undef malloc
void*
raptor_system_malloc(size_t size)
{
  return malloc(size);
}

#undef free
void
raptor_system_free(void *ptr)
{
  free(ptr);
}

#endif



#ifdef STANDALONE
#include <stdio.h>

int main(int argc, char *argv[]);


char *program;

int
main(int argc, char *argv[])
{
  int i;

  program=argv[0];

  raptor_init();
  
#ifdef RAPTOR_DEBUG
  fprintf(stderr, "%s: Known features:\n", program);
#endif

  for(i=0; 1; i++) {
    const char *feature_name;
    const char *feature_label;
    raptor_uri *feature_uri;
    int fn;
    
    if(raptor_features_enumerate((raptor_feature)i,
                                 &feature_name, &feature_uri, &feature_label))
      break;

#ifdef RAPTOR_DEBUG
    fprintf(stderr, " %2d %-20s %s\n", i, feature_name, feature_label);
#endif
    fn=raptor_feature_from_uri(feature_uri);
    if(fn != i) {
      fprintf(stderr, "raptor_feature_from_uri returned %d expected %d\n", fn, i);
      return 1;
    }
    raptor_free_uri(feature_uri);
  }

  raptor_finish();
  
  return 0;
}

#endif
