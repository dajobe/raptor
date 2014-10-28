/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_parse.c - Raptor Parser API
 *
 * Copyright (C) 2000-2010, David Beckett http://www.dajobe.org/
 * Copyright (C) 2000-2005, University of Bristol, UK http://www.bristol.ac.uk/
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
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


#ifndef STANDALONE

/* prototypes for helper functions */
static void raptor_parser_set_strict(raptor_parser* rdf_parser, int is_strict);

/* helper methods */

static void
raptor_free_parser_factory(raptor_parser_factory* factory)
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN(factory, raptor_parser_factory);
  
  if(factory->finish_factory)
    factory->finish_factory(factory);
  
  RAPTOR_FREE(raptor_parser_factory, factory);
}


/* class methods */

int
raptor_parsers_init(raptor_world *world)
{
  int rc = 0;

  world->parsers = raptor_new_sequence((raptor_data_free_handler)raptor_free_parser_factory, NULL);
  if(!world->parsers)
    return 1;
  
#ifdef RAPTOR_PARSER_RDFXML
  rc+= raptor_init_parser_rdfxml(world) != 0;
#endif

#ifdef RAPTOR_PARSER_NTRIPLES
  rc+= raptor_init_parser_ntriples(world) != 0;
#endif

#ifdef RAPTOR_PARSER_N3
  rc+= raptor_init_parser_n3(world) != 0;
#endif

#ifdef RAPTOR_PARSER_TURTLE
  rc+= raptor_init_parser_turtle(world) != 0;
#endif

#ifdef RAPTOR_PARSER_TRIG
  rc+= raptor_init_parser_trig(world) != 0;
#endif

#ifdef RAPTOR_PARSER_RSS
  rc+= raptor_init_parser_rss(world) != 0;
#endif

#if defined(RAPTOR_PARSER_GRDDL)
  rc+= raptor_init_parser_grddl_common(world) != 0;

#ifdef RAPTOR_PARSER_GRDDL
  rc+= raptor_init_parser_grddl(world) != 0;
#endif

#endif

#ifdef RAPTOR_PARSER_GUESS
  rc+= raptor_init_parser_guess(world) != 0;
#endif

#ifdef RAPTOR_PARSER_RDFA
  rc+= raptor_init_parser_rdfa(world) != 0;
#endif

#ifdef RAPTOR_PARSER_JSON
  rc+= raptor_init_parser_json(world) != 0;
#endif

#ifdef RAPTOR_PARSER_NQUADS
  rc+= raptor_init_parser_nquads(world) != 0;
#endif

  return rc;
}


/*
 * raptor_finish_parsers - delete all the registered parsers
 */
void
raptor_parsers_finish(raptor_world *world)
{
  if(world->parsers) {
    raptor_free_sequence(world->parsers);
    world->parsers = NULL;
  }
#if defined(RAPTOR_PARSER_GRDDL)
  raptor_terminate_parser_grddl_common(world);
#endif
}


/*
 * raptor_world_register_parser_factory:
 * @world: raptor world
 * @factory: pointer to function to call to register the factory
 * 
 * Internal - Register a parser via parser factory.
 *
 * All strings set in the @factory method are shared with the
 * #raptor_parser_factory
 *
 * Return value: new factory object or NULL on failure
 **/
RAPTOR_EXTERN_C
raptor_parser_factory*
raptor_world_register_parser_factory(raptor_world* world,
                                     int (*factory) (raptor_parser_factory*)) 
{
  raptor_parser_factory *parser = NULL;
  
  parser = RAPTOR_CALLOC(raptor_parser_factory*, 1, sizeof(*parser));
  if(!parser)
    return NULL;

  parser->world = world;

  parser->desc.mime_types = NULL;
  
  if(raptor_sequence_push(world->parsers, parser))
    return NULL; /* on error, parser is already freed by the sequence */
  
  /* Call the parser registration function on the new object */
  if(factory(parser))
    return NULL; /* parser is owned and freed by the parsers sequence */
  
  if(raptor_syntax_description_validate(&parser->desc)) {
    raptor_log_error(world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                     "Parser description failed to validate\n");
    goto tidy;
  }
  


#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2("Registered parser %s\n", parser->desc.names[0]);
#endif

  return parser;

  /* Clean up on failure */
  tidy:
  raptor_free_parser_factory(parser);
  return NULL;
}


/*
 * raptor_world_get_parser_factory:
 * @world: world object
 * @name: the factory name or NULL for the default factory
 *
 * INTERNAL - Get a parser factory by name.
 * 
 * Return value: the factory object or NULL if there is no such factory
 **/
raptor_parser_factory*
raptor_world_get_parser_factory(raptor_world *world, const char *name) 
{
  raptor_parser_factory *factory = NULL;

  /* return 1st parser if no particular one wanted - why? */
  if(!name) {
    factory = (raptor_parser_factory *)raptor_sequence_get_at(world->parsers, 0);
    if(!factory) {
      RAPTOR_DEBUG1("No (default) parsers registered\n");
      return NULL;
    }
  } else {
    int i;
    
    for(i = 0;
        (factory = (raptor_parser_factory*)raptor_sequence_get_at(world->parsers, i));
        i++) {
      int namei;
      const char* fname;
      
      for(namei = 0; (fname = factory->desc.names[namei]); namei++) {
        if(!strcmp(fname, name))
          break;
      }
      if(fname)
        break;
    }
  }
        
  return factory;
}


/**
 * raptor_world_get_parsers_count:
 * @world: world object
 *
 * Get number of parsers
 *
 * Return value: number of parsers
 **/
int
raptor_world_get_parsers_count(raptor_world* world)
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, NULL);

  raptor_world_open(world);

  return raptor_sequence_size(world->parsers);
}


/**
 * raptor_world_get_parser_description:
 * @world: world object
 * @counter: index into the list of parsers
 *
 * Get parser descriptive syntax information
 * 
 * Return value: description or NULL if counter is out of range
 **/
const raptor_syntax_description*
raptor_world_get_parser_description(raptor_world* world, 
                                    unsigned int counter)
{
  raptor_parser_factory *factory;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, NULL);

  raptor_world_open(world);
  
  factory = (raptor_parser_factory*)raptor_sequence_get_at(world->parsers,
                                                           counter);

  if(!factory)
    return NULL;

  return &factory->desc;
}


/**
 * raptor_world_is_parser_name:
 * @world: world object
 * @name: the syntax name
 *
 * Check the name of a parser is known.
 *
 * Return value: non 0 if name is a known syntax name
 */
int
raptor_world_is_parser_name(raptor_world* world, const char *name)
{
  if(!name)
    return 0;
  
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, 0);

  raptor_world_open(world);
  
  return (raptor_world_get_parser_factory(world, name) != NULL);
}


/**
 * raptor_new_parser:
 * @world: world object
 * @name: the parser name or NULL for default parser
 *
 * Constructor - create a new raptor_parser object.
 *
 * Return value: a new #raptor_parser object or NULL on failure
 */
raptor_parser*
raptor_new_parser(raptor_world* world, const char *name)
{
  raptor_parser_factory* factory;
  raptor_parser* rdf_parser;

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  raptor_world_open(world);
  
  factory = raptor_world_get_parser_factory(world, name);
  if(!factory)
    return NULL;

  rdf_parser = RAPTOR_CALLOC(raptor_parser*, 1, sizeof(*rdf_parser));
  if(!rdf_parser)
    return NULL;

  rdf_parser->world = world;
  raptor_statement_init(&rdf_parser->statement, world);
  
  rdf_parser->context = RAPTOR_CALLOC(void*, 1, factory->context_length);
  if(!rdf_parser->context) {
    raptor_free_parser(rdf_parser);
    return NULL;
  }
  
#ifdef RAPTOR_XML_LIBXML
  rdf_parser->magic = RAPTOR_LIBXML_MAGIC;
#endif  
  rdf_parser->factory = factory;

  /* Bit flags */
  rdf_parser->failed = 0;
  rdf_parser->emit_graph_marks = 1;
  rdf_parser->emitted_default_graph = 0;
  
  raptor_object_options_init(&rdf_parser->options, RAPTOR_OPTION_AREA_PARSER);

  /* set parsing strictness from default value */
  raptor_parser_set_strict(rdf_parser, 
                           RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_STRICT));

  if(factory->init(rdf_parser, name)) {
    raptor_free_parser(rdf_parser);
    return NULL;
  }
  
  return rdf_parser;
}


/**
 * raptor_new_parser_for_content:
 * @world: world object
 * @uri: URI identifying the syntax (or NULL)
 * @mime_type: mime type identifying the content (or NULL)
 * @buffer: buffer of content to guess (or NULL)
 * @len: length of buffer
 * @identifier: identifier of content (or NULL)
 * 
 * Constructor - create a new raptor_parser.
 *
 * Uses raptor_world_guess_parser_name() to find a parser by scoring
 * recognition of the syntax by a block of characters, the content
 * identifier or a mime type.  The content identifier is typically a
 * filename or URI or some other identifier.
 * 
 * Return value: a new #raptor_parser object or NULL on failure
 **/
raptor_parser*
raptor_new_parser_for_content(raptor_world* world,
                              raptor_uri *uri, const char *mime_type,
                              const unsigned char *buffer, size_t len,
                              const unsigned char *identifier)
{
  const char* name;

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  raptor_world_open(world);

  name = raptor_world_guess_parser_name(world, uri, mime_type,
                                        buffer, len, identifier);
  return name ? raptor_new_parser(world, name) : NULL;
}


/**
 * raptor_parser_parse_start:
 * @rdf_parser: RDF parser
 * @uri: base URI or may be NULL if no base URI is required
 *
 * Start a parse of content with base URI.
 * 
 * Parsers that need a base URI can be identified using a syntax
 * description returned by raptor_world_get_parser_description()
 * statically or raptor_parser_get_description() on a constructed
 * parser.
 * 
 * Return value: non-0 on failure, <0 if a required base URI was missing
 **/
int
raptor_parser_parse_start(raptor_parser *rdf_parser, raptor_uri *uri) 
{
  if((rdf_parser->factory->desc.flags & RAPTOR_SYNTAX_NEED_BASE_URI) && !uri) {
    raptor_parser_error(rdf_parser, "Missing base URI for %s parser.",
                        rdf_parser->factory->desc.names[0]);
    return -1;
  }

  if(uri)
    uri = raptor_uri_copy(uri);
  
  if(rdf_parser->base_uri)
    raptor_free_uri(rdf_parser->base_uri);
  rdf_parser->base_uri = uri;

  rdf_parser->locator.uri    = uri;
  rdf_parser->locator.line   = -1;
  rdf_parser->locator.column = -1;
  rdf_parser->locator.byte   = -1;

  if(rdf_parser->factory->start)
    return rdf_parser->factory->start(rdf_parser);
  else
    return 0;
}




/**
 * raptor_parser_parse_chunk:
 * @rdf_parser: RDF parser
 * @buffer: content to parse
 * @len: length of buffer
 * @is_end: non-0 if this is the end of the content (such as EOF)
 *
 * Parse a block of content into triples.
 * 
 * This method can only be called after raptor_parser_parse_start() has
 * initialised the parser.
 * 
 * Return value: non-0 on failure.
 **/
int
raptor_parser_parse_chunk(raptor_parser* rdf_parser,
                          const unsigned char *buffer, size_t len, int is_end) 
{
  if(rdf_parser->sb)
    raptor_stringbuffer_append_counted_string(rdf_parser->sb, buffer, len, 1);
    
  return rdf_parser->factory->chunk(rdf_parser, buffer, len, is_end);
}


/**
 * raptor_free_parser:
 * @parser: #raptor_parser object
 *
 * Destructor - destroy a raptor_parser object.
 * 
 **/
void
raptor_free_parser(raptor_parser* rdf_parser)
{
  if(!rdf_parser)
    return;

  if(rdf_parser->factory)
    rdf_parser->factory->terminate(rdf_parser);

  if(rdf_parser->www)
    raptor_free_www(rdf_parser->www);

  if(rdf_parser->context)
    RAPTOR_FREE(raptor_parser_context, rdf_parser->context);

  if(rdf_parser->base_uri)
    raptor_free_uri(rdf_parser->base_uri);

  if(rdf_parser->sb)
    raptor_free_stringbuffer(rdf_parser->sb);

  raptor_object_options_clear(&rdf_parser->options);

  RAPTOR_FREE(raptor_parser, rdf_parser);
}


/**
 * raptor_parser_parse_file_stream:
 * @rdf_parser: parser
 * @stream: FILE* of RDF content
 * @filename: filename of content or NULL if it has no name
 * @base_uri: the base URI to use
 *
 * Parse RDF content from a FILE*.
 *
 * After draining the FILE* stream (EOF), fclose is not called on it.
 *
 * Return value: non 0 on failure
 **/
int
raptor_parser_parse_file_stream(raptor_parser* rdf_parser,
                                FILE *stream, const char* filename,
                                raptor_uri *base_uri)
{
  int rc = 0;
  raptor_locator *locator = &rdf_parser->locator;

  if(!stream || !base_uri)
    return 1;

  locator->line= locator->column = -1;
  locator->file= filename;

  if(raptor_parser_parse_start(rdf_parser, base_uri))
    return 1;
  
  while(!feof(stream)) {
    size_t len = fread(rdf_parser->buffer, 1, RAPTOR_READ_BUFFER_SIZE, stream);
    int is_end = (len < RAPTOR_READ_BUFFER_SIZE);
    rdf_parser->buffer[len] = '\0';
    rc = raptor_parser_parse_chunk(rdf_parser, rdf_parser->buffer, len, is_end);
    if(rc || is_end)
      break;
  }

  return (rc != 0);
}


/**
 * raptor_parser_parse_file:
 * @rdf_parser: parser
 * @uri: URI of RDF content or NULL to read from standard input
 * @base_uri: the base URI to use (or NULL if the same)
 *
 * Parse RDF content at a file URI.
 *
 * If @uri is NULL (source is stdin), then the @base_uri is required.
 * 
 * Return value: non 0 on failure
 **/
int
raptor_parser_parse_file(raptor_parser* rdf_parser, raptor_uri *uri,
                         raptor_uri *base_uri) 
{
  int rc = 0;
  int free_base_uri = 0;
  const char *filename = NULL;
  FILE *fh = NULL;
#if defined(HAVE_UNISTD_H) && defined(HAVE_SYS_STAT_H)
  struct stat buf;
#endif

  if(uri) {
    filename = raptor_uri_uri_string_to_filename(raptor_uri_as_string(uri));
    if(!filename)
      return 1;

#if defined(HAVE_UNISTD_H) && defined(HAVE_SYS_STAT_H)
    if(!stat(filename, &buf) && S_ISDIR(buf.st_mode)) {
      raptor_parser_error(rdf_parser, "Cannot read from a directory '%s'",
                          filename);
      goto cleanup;
    }
#endif

    fh = fopen(filename, "r");
    if(!fh) {
      raptor_parser_error(rdf_parser, "file '%s' open failed - %s",
                          filename, strerror(errno));
      goto cleanup;
    }
    if(!base_uri) {
      base_uri = raptor_uri_copy(uri);
      free_base_uri = 1;
    }
  } else {
    if(!base_uri)
      return 1;
    fh = stdin;
  }

  rc = raptor_parser_parse_file_stream(rdf_parser, fh, filename, base_uri);

  cleanup:
  if(uri) {
    if(fh)
      fclose(fh);
    RAPTOR_FREE(char*, filename);
  }
  if(free_base_uri)
    raptor_free_uri(base_uri);

  return rc;
}


void
raptor_parser_parse_uri_write_bytes(raptor_www* www,
                                    void *userdata, const void *ptr, 
                                    size_t size, size_t nmemb)
{
  raptor_parse_bytes_context* rpbc = (raptor_parse_bytes_context*)userdata;
  size_t len = size * nmemb;

  if(!rpbc->started) {
    raptor_uri* base_uri = rpbc->base_uri;
    
    if(!base_uri) {
      rpbc->final_uri = raptor_www_get_final_uri(www);
      /* base URI after URI resolution is finally chosen */
      base_uri = rpbc->final_uri ? rpbc->final_uri : www->uri;
    }

    if(raptor_parser_parse_start(rpbc->rdf_parser, base_uri))
      raptor_www_abort(www, "Parsing failed");
    rpbc->started = 1;
  }

  if(raptor_parser_parse_chunk(rpbc->rdf_parser, (unsigned char*)ptr, len, 0))
    raptor_www_abort(www, "Parsing failed");
}


static void
raptor_parser_parse_uri_content_type_handler(raptor_www* www, void* userdata, 
                                             const char* content_type)
{
  raptor_parser* rdf_parser = (raptor_parser*)userdata;
  if(rdf_parser->factory->content_type_handler)
    rdf_parser->factory->content_type_handler(rdf_parser, content_type);
}


int
raptor_parser_set_uri_filter_no_net(void *user_data, raptor_uri* uri)
{
  unsigned char* uri_string = raptor_uri_as_string(uri);
  
  if(raptor_uri_uri_string_is_file_uri(uri_string))
    return 0;

  raptor_parser_error((raptor_parser*)user_data, 
                      "Network fetch of URI '%s' denied", uri_string);
  return 1;
}


/**
 * raptor_parser_parse_uri:
 * @rdf_parser: parser
 * @uri: URI of RDF content
 * @base_uri: the base URI to use (or NULL if the same)
 *
 * Parse the RDF content at URI.
 * 
 * Sends an HTTP Accept: header whent the URI is of the HTTP protocol,
 * see raptor_parser_parse_uri_with_connection() for details including
 * how the @base_uri is used.
 *
 * Return value: non 0 on failure
 **/
int
raptor_parser_parse_uri(raptor_parser* rdf_parser, raptor_uri *uri,
                        raptor_uri *base_uri)
{
  return raptor_parser_parse_uri_with_connection(rdf_parser, uri, base_uri,
                                                 NULL);
}


/**
 * raptor_parser_parse_uri_with_connection:
 * @rdf_parser: parser
 * @uri: URI of RDF content
 * @base_uri: the base URI to use (or NULL if the same)
 * @connection: connection object pointer or NULL to create a new one
 *
 * Parse RDF content at URI using existing WWW connection.
 * 
 * If @base_uri is not given and during resolution of the URI, a
 * protocol redirection occurs, the final resolved URI will be
 * used as the base URI.  If redirection does not occur, the
 * base URI will be @uri.
 *
 * If @base_uri is given, it overrides the process above.
 *
 * When @connection is NULL and a MIME Type exists for the parser
 * type, this type is sent in an HTTP Accept: header in the form
 * Accept: MIME-TYPE along with a wildcard of 0.1 quality, so MIME-TYPE is
 * prefered rather than the sole answer.  The latter part may not be
 * necessary but should ensure an HTTP 200 response.
 *
 * Return value: non 0 on failure
 **/
int
raptor_parser_parse_uri_with_connection(raptor_parser* rdf_parser,
                                        raptor_uri *uri,
                                        raptor_uri *base_uri, void *connection)
{
  int ret = 0;
  raptor_parse_bytes_context rpbc;
  char* ua = NULL;
  char* cert_filename = NULL;
  char* cert_type = NULL;
  char* cert_passphrase = NULL;
  int ssl_verify_peer;
  int ssl_verify_host;

  if(connection) {
    if(rdf_parser->www)
      raptor_free_www(rdf_parser->www);
    rdf_parser->www = raptor_new_www_with_connection(rdf_parser->world,
                                                     connection);
    if(!rdf_parser->www)
      return 1;
  } else {
    const char *accept_h;
    
    if(rdf_parser->www)
      raptor_free_www(rdf_parser->www);
    rdf_parser->www = raptor_new_www(rdf_parser->world);
    if(!rdf_parser->www)
      return 1;

    accept_h = raptor_parser_get_accept_header(rdf_parser);
    if(accept_h) {
      raptor_www_set_http_accept(rdf_parser->www, accept_h);
      RAPTOR_FREE(char*, accept_h);
    }
  }

  rpbc.rdf_parser = rdf_parser;
  rpbc.base_uri = base_uri;
  rpbc.final_uri = NULL;
  rpbc.started = 0;
  
  if(rdf_parser->uri_filter)
    raptor_www_set_uri_filter(rdf_parser->www, rdf_parser->uri_filter,
                              rdf_parser->uri_filter_user_data);
  else if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_NO_NET))
    raptor_www_set_uri_filter(rdf_parser->www,
                              raptor_parser_set_uri_filter_no_net, rdf_parser);
  
  raptor_www_set_write_bytes_handler(rdf_parser->www,
                                     raptor_parser_parse_uri_write_bytes, 
                                     &rpbc);

  raptor_www_set_content_type_handler(rdf_parser->www,
                                      raptor_parser_parse_uri_content_type_handler,
                                      rdf_parser);

  raptor_www_set_http_cache_control(rdf_parser->www, 
                                    RAPTOR_OPTIONS_GET_STRING(rdf_parser, 
                                                              RAPTOR_OPTION_WWW_HTTP_CACHE_CONTROL));

  ua = RAPTOR_OPTIONS_GET_STRING(rdf_parser, RAPTOR_OPTION_WWW_HTTP_USER_AGENT);
  if(ua)
    raptor_www_set_user_agent(rdf_parser->www, ua);

  cert_filename = RAPTOR_OPTIONS_GET_STRING(rdf_parser,
                                            RAPTOR_OPTION_WWW_CERT_FILENAME);
  cert_type = RAPTOR_OPTIONS_GET_STRING(rdf_parser,
                                        RAPTOR_OPTION_WWW_CERT_TYPE);
  cert_passphrase = RAPTOR_OPTIONS_GET_STRING(rdf_parser,
                                              RAPTOR_OPTION_WWW_CERT_PASSPHRASE);
  if(cert_filename || cert_type || cert_passphrase)
    raptor_www_set_ssl_cert_options(rdf_parser->www, cert_filename,
                                    cert_type, cert_passphrase);
  
  ssl_verify_peer = RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser,
                                               RAPTOR_OPTION_WWW_SSL_VERIFY_PEER);
  ssl_verify_host = RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser,
                                               RAPTOR_OPTION_WWW_SSL_VERIFY_HOST);
  raptor_www_set_ssl_verify_options(rdf_parser->www, ssl_verify_peer,
                                    ssl_verify_host);

  ret = raptor_www_fetch(rdf_parser->www, uri);
  
  if(!rpbc.started && !ret)
    ret = raptor_parser_parse_start(rdf_parser, base_uri);

  if(rpbc.final_uri)
    raptor_free_uri(rpbc.final_uri);

  if(ret) {
    raptor_free_www(rdf_parser->www);
    rdf_parser->www = NULL;
    return 1;
  }

  if(raptor_parser_parse_chunk(rdf_parser, NULL, 0, 1))
    rdf_parser->failed = 1;

  raptor_free_www(rdf_parser->www);
  rdf_parser->www = NULL;

  return rdf_parser->failed;
}


/*
 * raptor_parser_fatal_error - Fatal Error from a parser - Internal
 */
void
raptor_parser_fatal_error(raptor_parser* parser, const char *message, ...)
{
  va_list arguments;

  va_start(arguments, message);
  if(parser) {
    parser->failed = 1;
    raptor_log_error_varargs(parser->world,
                             RAPTOR_LOG_LEVEL_FATAL,
                             &parser->locator,
                             message, arguments);
  } else
    raptor_log_error_varargs(NULL,
                             RAPTOR_LOG_LEVEL_FATAL, NULL,
                             message, arguments); 
  va_end(arguments);
}


/*
 * raptor_parser_error - Error from a parser - Internal
 */
void
raptor_parser_error(raptor_parser* parser, const char *message, ...)
{
  va_list arguments;

  va_start(arguments, message);

  raptor_parser_log_error_varargs(parser, RAPTOR_LOG_LEVEL_ERROR,
                                  message, arguments);
  
  va_end(arguments);
}


/**
 * raptor_parser_log_error_varargs:
 * @parser: parser (or NULL)
 * @level: log level
 * @message: error format message
 * @arguments: varargs for message
 *
 * Error from a parser - Internal.
 */  
void  
raptor_parser_log_error_varargs(raptor_parser* parser,
                                raptor_log_level level,
                                const char *message, va_list arguments)
{
  if(parser)
    raptor_log_error_varargs(parser->world,
                             level,
                             &parser->locator,
                             message, arguments);
  else
    raptor_log_error_varargs(NULL,
                             level,
                             NULL,
                             message, arguments);
}


/*
 * raptor_parser_warning - Warning from a parser - Internal
 */
void
raptor_parser_warning(raptor_parser* parser, const char *message, ...)
{
  va_list arguments;

  va_start(arguments, message);

  if(parser)
    raptor_log_error_varargs(parser->world,
                             RAPTOR_LOG_LEVEL_WARN,
                             &parser->locator,
                             message, arguments);
  else
    raptor_log_error_varargs(NULL,
                             RAPTOR_LOG_LEVEL_WARN,
                             NULL,
                             message, arguments);
  
  va_end(arguments);
}



/* PUBLIC FUNCTIONS */

/**
 * raptor_parser_set_statement_handler:
 * @parser: #raptor_parser parser object
 * @user_data: user data pointer for callback
 * @handler: new statement callback function
 *
 * Set the statement handler function for the parser.
 *
 * Use this to set the function to receive statements as the parsing
 * proceeds. The statement argument to @handler is shared and must be
 * copied by the caller with raptor_statement_copy().
 **/
void
raptor_parser_set_statement_handler(raptor_parser* parser,
                                    void *user_data,
                                    raptor_statement_handler handler)
{
  parser->user_data = user_data;
  parser->statement_handler = handler;
}


/**
 * raptor_parser_set_graph_mark_handler:
 * @parser: #raptor_parser parser object
 * @user_data: user data pointer for callback
 * @handler: new graph callback function
 *
 * Set the graph mark handler function for the parser.
 *
 * See #raptor_graph_mark_handler and #raptor_graph_mark_flags for
 * the marks that may be returned by the handler.
 * 
 **/
void
raptor_parser_set_graph_mark_handler(raptor_parser* parser,
                                     void *user_data,
                                     raptor_graph_mark_handler handler)
{
  parser->user_data = user_data;
  parser->graph_mark_handler = handler;
}


/**
 * raptor_parser_set_namespace_handler:
 * @parser: #raptor_parser parser object
 * @user_data: user data pointer for callback
 * @handler: new namespace callback function
 *
 * Set the namespace handler function for the parser.
 *
 * When a prefix/namespace is seen in a parser, call the given
 * @handler with the prefix string and the #raptor_uri namespace URI.
 * Either can be NULL for the default prefix or default namespace.
 *
 * The handler function does not deal with duplicates so any
 * namespace may be declared multiple times.
 * 
 **/
void
raptor_parser_set_namespace_handler(raptor_parser* parser,
                                    void *user_data,
                                    raptor_namespace_handler handler)
{
  parser->namespace_handler = handler;
  parser->namespace_handler_user_data = user_data;
}


/**
 * raptor_parser_set_uri_filter:
 * @parser: parser object
 * @filter: URI filter function
 * @user_data: User data to pass to filter function
 * 
 * Set URI filter function for WWW retrieval.
 **/
void
raptor_parser_set_uri_filter(raptor_parser* parser, 
                             raptor_uri_filter_func filter,
                             void *user_data)
{
  parser->uri_filter = filter;
  parser->uri_filter_user_data = user_data;
}


/**
 * raptor_parser_set_option:
 * @parser: #raptor_parser parser object
 * @option: option to set from enumerated #raptor_option values
 * @string: string option value (or NULL)
 * @integer: integer option value
 *
 * Set parser option.
 * 
 * If @string is not NULL and the option type is numeric, the string
 * value is converted to an integer and used in preference to @integer.
 *
 * If @string is NULL and the option type is not numeric, an error is
 * returned.
 *
 * The @string values used are copied.
 *
 * The allowed options are available via
 * raptor_world_get_option_description().
 *
 * Return value: non 0 on failure or if the option is unknown
 **/
int
raptor_parser_set_option(raptor_parser *parser, raptor_option option,
                         const char* string, int integer)
{
  int rc;
  
  rc = raptor_object_options_set_option(&parser->options, option,
                                        string, integer);
  if(option == RAPTOR_OPTION_STRICT && !rc) {
    int is_strict = RAPTOR_OPTIONS_GET_NUMERIC(parser, RAPTOR_OPTION_STRICT);
    raptor_parser_set_strict(parser, is_strict);
  }

  return rc;
}


/**
 * raptor_parser_get_option:
 * @parser: #raptor_parser parser object
 * @option: option to get value
 * @string_p: pointer to where to store string value
 * @integer_p: pointer to where to store integer value
 *
 * Get parser option.
 * 
 * Any string value returned in *@string_p is shared and must
 * be copied by the caller.
 *
 * The allowed options are available via
 * raptor_world_get_option_description().
 *
 * Return value: option value or < 0 for an illegal option
 **/
int
raptor_parser_get_option(raptor_parser *parser, raptor_option option,
                         char** string_p, int* integer_p)
{
  return raptor_object_options_get_option(&parser->options, option,
                                          string_p, integer_p);
}


/**
 * raptor_parser_set_strict:
 * @rdf_parser: #raptor_parser object
 * @is_strict: Non 0 for strict parsing
 *
 * INTERNAL - Set parser to strict / lax mode.
 * 
 **/
static void
raptor_parser_set_strict(raptor_parser* rdf_parser, int is_strict)
{
  is_strict = (is_strict) ? 1 : 0;

  /* Initialise default parser mode */
  RAPTOR_OPTIONS_SET_NUMERIC(rdf_parser, RAPTOR_OPTION_SCANNING, 0);

  RAPTOR_OPTIONS_SET_NUMERIC(rdf_parser, RAPTOR_OPTION_ALLOW_NON_NS_ATTRIBUTES, !is_strict);
  RAPTOR_OPTIONS_SET_NUMERIC(rdf_parser, RAPTOR_OPTION_ALLOW_OTHER_PARSETYPES, !is_strict);
  RAPTOR_OPTIONS_SET_NUMERIC(rdf_parser, RAPTOR_OPTION_ALLOW_BAGID, !is_strict);
  RAPTOR_OPTIONS_SET_NUMERIC(rdf_parser, RAPTOR_OPTION_ALLOW_RDF_TYPE_RDF_LIST, 0);
  RAPTOR_OPTIONS_SET_NUMERIC(rdf_parser, RAPTOR_OPTION_NORMALIZE_LANGUAGE, 1);
  RAPTOR_OPTIONS_SET_NUMERIC(rdf_parser, RAPTOR_OPTION_NON_NFC_FATAL, is_strict);
  RAPTOR_OPTIONS_SET_NUMERIC(rdf_parser, RAPTOR_OPTION_WARN_OTHER_PARSETYPES, !is_strict);
  RAPTOR_OPTIONS_SET_NUMERIC(rdf_parser, RAPTOR_OPTION_CHECK_RDF_ID, 1);
  RAPTOR_OPTIONS_SET_NUMERIC(rdf_parser, RAPTOR_OPTION_HTML_TAG_SOUP, !is_strict);
  RAPTOR_OPTIONS_SET_NUMERIC(rdf_parser, RAPTOR_OPTION_MICROFORMATS, !is_strict);
  RAPTOR_OPTIONS_SET_NUMERIC(rdf_parser, RAPTOR_OPTION_HTML_LINK, !is_strict);
}


/**
 * raptor_parser_get_name:
 * @rdf_parser: #raptor_parser parser object
 *
 * Get the name of a parser.
 *
 * Use raptor_parser_get_description() to get the alternate names and
 * aliases as well as other descriptive values.
 *
 * Return value: the short name for the parser.
 **/
const char*
raptor_parser_get_name(raptor_parser *rdf_parser)
{
  if(rdf_parser->factory->get_name)
    return rdf_parser->factory->get_name(rdf_parser);
  else
    return rdf_parser->factory->desc.names[0];
}


/**
 * raptor_parser_get_description:
 * @rdf_parser: #raptor_parser parser object
 *
 * Get description of the syntaxes of the parser.
 *
 * The returned description is static and lives as long as the raptor
 * library (raptor world).
 *
 * Return value: description of syntax
 **/
const raptor_syntax_description*
raptor_parser_get_description(raptor_parser *rdf_parser)
{
  if(rdf_parser->factory->get_description)
    return rdf_parser->factory->get_description(rdf_parser);
  else
    return &rdf_parser->factory->desc;
}



/**
 * raptor_parser_parse_abort:
 * @rdf_parser: #raptor_parser parser object
 *
 * Abort an ongoing parsing.
 * 
 * Causes any ongoing generation of statements by a parser to be
 * terminated and the parser to return controlto the application
 * as soon as draining any existing buffers.
 *
 * Most useful inside raptor_parser_parse_file() or
 * raptor_parser_parse_uri() when the Raptor library is directing the
 * parsing and when one of the callback handlers such as as set by
 * raptor_parser_set_statement_handler() requires to return to the main
 * application code.
 **/
void
raptor_parser_parse_abort(raptor_parser *rdf_parser)
{
  rdf_parser->failed = 1;
}


/**
 * raptor_parser_get_locator:
 * @rdf_parser: raptor parser
 *
 * Get the current raptor locator object.
 * 
 * Return value: raptor locator
 **/
raptor_locator*
raptor_parser_get_locator(raptor_parser *rdf_parser)
{
  if(rdf_parser->factory->get_locator)
    return rdf_parser->factory->get_locator(rdf_parser);
  else
    return &rdf_parser->locator;
}


#ifdef RAPTOR_DEBUG
void
raptor_stats_print(raptor_parser *rdf_parser, FILE *stream)
{
#ifdef RAPTOR_PARSER_RDFXML
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  if(!strcmp(rdf_parser->factory->desc.names[0], "rdfxml")) {
    raptor_rdfxml_parser *rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;
    fputs("raptor parser stats\n  ", stream);
    raptor_rdfxml_parser_stats_print(rdf_xml_parser, stream);
  }
#endif
#endif
}
#endif


struct syntax_score
{
  int score;
  raptor_parser_factory* factory;
};


static int
compare_syntax_score(const void *a, const void *b) {
  return ((struct syntax_score*)b)->score - ((struct syntax_score*)a)->score;
}

#define RAPTOR_MIN_GUESS_SCORE 2

/**
 * raptor_world_guess_parser_name:
 * @world: world object
 * @uri: URI identifying the syntax (or NULL)
 * @mime_type: mime type identifying the content (or NULL)
 * @buffer: buffer of content to guess (or NULL)
 * @len: length of buffer
 * @identifier: identifier of content (or NULL)
 *
 * Guess a parser name for content.
 * 
 * Find a parser by scoring recognition of the syntax by a block of
 * characters, the content identifier or a mime type.  The content
 * identifier is typically a filename or URI or some other identifier.
 *
 * If the guessing finds only low scores, NULL will be returned.
 * 
 * Return value: a parser name or NULL if no guess could be made
 **/
const char*
raptor_world_guess_parser_name(raptor_world* world,
                               raptor_uri *uri, const char *mime_type,
                               const unsigned char *buffer, size_t len,
                               const unsigned char *identifier)
{
  unsigned int i;
  raptor_parser_factory *factory;
  unsigned char *suffix = NULL;
  struct syntax_score* scores;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, NULL);

  raptor_world_open(world);

  scores = RAPTOR_CALLOC(struct syntax_score*,
                         raptor_sequence_size(world->parsers),
                         sizeof(struct syntax_score));
  if(!scores)
    return NULL;
  
  if(identifier) {
    unsigned char *p = (unsigned char*)strrchr((const char*)identifier, '.');
    if(p) {
      unsigned char *from, *to;

      p++;
      suffix = RAPTOR_MALLOC(unsigned char*, strlen((const char*)p) + 1);
      if(!suffix) {
        RAPTOR_FREE(syntax_scores, scores);
        return NULL;
      }

      for(from = p, to = suffix; *from; ) {
        unsigned char c = *from++;
        /* discard the suffix if it wasn't '\.[a-zA-Z0-9]+$' */
        if(!isalpha(c) && !isdigit(c)) {
          RAPTOR_FREE(char*, suffix);
          suffix = NULL;
          to = NULL;
          break;
        }
        *to++ = isupper(c) ? (unsigned char)tolower(c): c;
      }
      if(to)
        *to = '\0';
    }
  }

  for(i = 0;
      (factory = (raptor_parser_factory*)raptor_sequence_get_at(world->parsers, i));
      i++) {
    int score = -1;
    const raptor_type_q* type_q = NULL;
    
    if(mime_type && factory->desc.mime_types) {
      int j;
      type_q = NULL;
      for(j = 0; 
          (type_q = &factory->desc.mime_types[j]) && type_q->mime_type;
          j++) {
        if(!strcmp(mime_type, type_q->mime_type))
          break;
      }
      /* got an exact match mime type - score it via the Q */
      if(type_q)
        score = type_q->q;
    }
    /* mime type match has high Q - return factory as result */
    if(score >= 10)
      break;
    
    if(uri && factory->desc.uri_strings) {
      int j;
      const char* uri_string = (const char*)raptor_uri_as_string(uri);
      const char* factory_uri_string = NULL;
      
      for(j = 0;
          (factory_uri_string = factory->desc.uri_strings[j]);
          j++) {
        if(!strcmp(uri_string, factory_uri_string))
          break;
      }
      if(factory_uri_string)
        /* got an exact match syntax for URI - return factory as result */
        break;
    }
    
    if(factory->recognise_syntax) {
      int c = -1;
    
      /* Only use first N bytes to avoid HTML documents that contain
       * RDF/XML examples
       */
#define FIRSTN 1024
#if FIRSTN > RAPTOR_READ_BUFFER_SIZE
#error "RAPTOR_READ_BUFFER_SIZE is not large enough"
#endif
      if(buffer && len && len > FIRSTN) {
        c = buffer[FIRSTN];
        ((char*)buffer)[FIRSTN] = '\0';
      }

      score += factory->recognise_syntax(factory, buffer, len, 
                                         identifier, suffix, 
                                         mime_type);

      if(c >= 0)
        ((char*)buffer)[FIRSTN] = c;
    }

    scores[i].score = score < 10 ? score : 10; 
    scores[i].factory = factory;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 2
    RAPTOR_DEBUG3("Score %15s : %d\n", factory->desc.names[0], score);
#endif
  }
  
  if(!factory) {
    /* sort the scores and pick a factory if score is good enough */
    qsort(scores, i, sizeof(struct syntax_score), compare_syntax_score);

    if(scores[0].score >= RAPTOR_MIN_GUESS_SCORE)
      factory = scores[0].factory;
  }

  if(suffix)
    RAPTOR_FREE(char*, suffix);

  RAPTOR_FREE(syntax_scores, scores);
  
  return factory ? factory->desc.names[0] : NULL;
}


/*
 * raptor_parser_copy_flags_state:
 * @to_parser: destination parser
 * @from_parser: source parser
 * 
 * Copy status flags between parsers - INTERNAL.
 **/
void
raptor_parser_copy_flags_state(raptor_parser *to_parser, 
                               raptor_parser *from_parser)
{
  to_parser->failed = from_parser->failed;
  to_parser->emit_graph_marks = from_parser->emit_graph_marks;
  to_parser->emitted_default_graph = from_parser->emitted_default_graph;
}



/*
 * raptor_parser_copy_user_state:
 * @to_parser: destination parser
 * @from_parser: source parser
 * 
 * Copy user state between parsers - INTERNAL.
 *
 * Return value: non-0 on failure
 **/
int
raptor_parser_copy_user_state(raptor_parser *to_parser, 
                              raptor_parser *from_parser)
{
  int rc = 0;
  
  to_parser->user_data = from_parser->user_data;
  to_parser->statement_handler = from_parser->statement_handler;
  to_parser->namespace_handler = from_parser->namespace_handler;
  to_parser->namespace_handler_user_data = from_parser->namespace_handler_user_data;
  to_parser->uri_filter = from_parser->uri_filter;
  to_parser->uri_filter_user_data = from_parser->uri_filter_user_data;

  /* copy bit flags */
  raptor_parser_copy_flags_state(to_parser, from_parser);

  /* copy options */
  if(!rc)
    rc = raptor_object_options_copy_state(&to_parser->options, 
                                          &from_parser->options);

  return rc;
}


/*
 * raptor_parser_start_namespace:
 * @rdf_parser: parser
 * @nspace: namespace starting
 * 
 * Internal - Invoke start namespace handler
 **/
void
raptor_parser_start_namespace(raptor_parser* rdf_parser, 
                              raptor_namespace* nspace)
{
  if(!rdf_parser->namespace_handler)
    return;

  (*rdf_parser->namespace_handler)(rdf_parser->namespace_handler_user_data, 
                                   nspace);
}


/**
 * raptor_parser_get_accept_header:
 * @rdf_parser: parser
 * 
 * Get an HTTP Accept value for the parser.
 *
 * The returned string must be freed by the caller such as with
 * raptor_free_memory().
 *
 * Return value: a new Accept: header string or NULL on failure
 **/
const char*
raptor_parser_get_accept_header(raptor_parser* rdf_parser)
{
  raptor_parser_factory *factory = rdf_parser->factory;
  char *accept_header = NULL;
  size_t len;
  char *p;
  int i;
  const raptor_type_q* type_q;
  
  if(factory->accept_header)
    return factory->accept_header(rdf_parser);

  if(!factory->desc.mime_types)
    return NULL;

  len = 0;
  for(i = 0; 
      (type_q = &factory->desc.mime_types[i]) && type_q->mime_type;
      i++) {
    len += type_q->mime_type_len + 2; /* ", " */
    if(type_q->q < 10)
      len += 6; /* ";q=X.Y" */
  }
  
  /* 9 = strlen("\*\/\*;q=0.1") */
#define ACCEPT_HEADER_LEN 9
  accept_header = RAPTOR_MALLOC(char*, len + ACCEPT_HEADER_LEN + 1);
  if(!accept_header)
    return NULL;

  p = accept_header;
  for(i = 0; 
      (type_q = &factory->desc.mime_types[i]) && type_q->mime_type;
      i++) {
    memcpy(p, type_q->mime_type, type_q->mime_type_len);
    p += type_q->mime_type_len;
    if(type_q->q < 10) {
      *p++ = ';';
      *p++ = 'q';
      *p++ = '=';
      *p++ = '0';
      *p++ = '.';
      *p++ = '0' + (type_q->q);
    }
    
    *p++ = ',';
    *p++ = ' ';
  }

  memcpy(p, "*/*;q=0.1", ACCEPT_HEADER_LEN + 1);

  return accept_header;
}


const char*
raptor_parser_get_accept_header_all(raptor_world* world)
{
  raptor_parser_factory *factory;
  char *accept_header = NULL;
  size_t len;
  char *p;
  int i;
  
  len = 0;
  for(i = 0;
      (factory = (raptor_parser_factory*)raptor_sequence_get_at(world->parsers, i));
      i++) {
    const raptor_type_q* type_q;
    int j;
    
    for(j = 0;
        (type_q = &factory->desc.mime_types[j]) && type_q->mime_type;
        j++) {
      len += type_q->mime_type_len + 2; /* ", " */
      if(type_q->q < 10)
        len += 6; /* ";q=X.Y" */
    }
  }
  
  /* 9 = strlen("\*\/\*;q=0.1") */
#define ACCEPT_HEADER_LEN 9
  accept_header = RAPTOR_MALLOC(char*, len + ACCEPT_HEADER_LEN + 1);
  if(!accept_header)
    return NULL;

  p = accept_header;
  for(i = 0;
      (factory = (raptor_parser_factory*)raptor_sequence_get_at(world->parsers, i));
      i++) {
    const raptor_type_q* type_q;
    int j;
    
    for(j = 0; 
        (type_q = &factory->desc.mime_types[j]) && type_q->mime_type;
        j++) {
      memcpy(p, type_q->mime_type, type_q->mime_type_len);
      p+= type_q->mime_type_len;
      if(type_q->q < 10) {
        *p++ = ';';
        *p++ = 'q';
        *p++ = '=';
        *p++ = '0';
        *p++ = '.';
        *p++ = '0' + (type_q->q);
      }
      
      *p++ = ',';
      *p++ = ' ';
    }
    
  }
  
  memcpy(p, "*/*;q=0.1", ACCEPT_HEADER_LEN + 1);
  
  return accept_header;
}


void
raptor_parser_save_content(raptor_parser* rdf_parser, int save)
{
  if(rdf_parser->sb)
    raptor_free_stringbuffer(rdf_parser->sb);

  rdf_parser->sb= save ? raptor_new_stringbuffer() : NULL;
}


const unsigned char*
raptor_parser_get_content(raptor_parser* rdf_parser, size_t* length_p)
{
  unsigned char* buffer;
  size_t len;
  
  if(!rdf_parser->sb)
    return NULL;
  
  len = raptor_stringbuffer_length(rdf_parser->sb);
  buffer = RAPTOR_MALLOC(unsigned char*, len + 1);
  if(!buffer)
    return NULL;

  raptor_stringbuffer_copy_to_string(rdf_parser->sb, buffer, len);

  if(length_p)
    *length_p=len;

  return buffer;
}


void 
raptor_parser_start_graph(raptor_parser* parser, raptor_uri* uri,
                          int is_declared)
{
  int flags = RAPTOR_GRAPH_MARK_START;
  if(is_declared)
    flags |= RAPTOR_GRAPH_MARK_DECLARED;

  if(!parser->emit_graph_marks)
    return;
  
  if(parser->graph_mark_handler)
    (*parser->graph_mark_handler)(parser->user_data, uri, flags);
}


void 
raptor_parser_end_graph(raptor_parser* parser, raptor_uri* uri, int is_declared)
{
  int flags = 0;
  if(is_declared)
    flags |= RAPTOR_GRAPH_MARK_DECLARED;
  
  if(!parser->emit_graph_marks)
    return;
  
  if(parser->graph_mark_handler)
    (*parser->graph_mark_handler)(parser->user_data, uri, flags);
}


/**
 * raptor_parser_get_world:
 * @rdf_parser: parser
 * 
 * Get the #raptor_world object associated with a parser.
 *
 * Return value: raptor_world* pointer
 **/
raptor_world *
raptor_parser_get_world(raptor_parser* rdf_parser)
{
  return rdf_parser->world;
}


/**
 * raptor_parser_get_graph:
 * @rdf_parser: parser
 * 
 * Get the current graph for the parser
 *
 * The returned URI is owned by the caller and must be freed with
 * raptor_free_uri()
 *
 * Return value: raptor_uri* graph name or NULL for the default graph
 **/
raptor_uri*
raptor_parser_get_graph(raptor_parser* rdf_parser)
{
  if(rdf_parser->factory->get_graph)
    return rdf_parser->factory->get_graph(rdf_parser);
  return NULL;
}


/**
 * raptor_parser_parse_iostream:
 * @rdf_parser: parser
 * @iostr: iostream to read from
 * @base_uri: the base URI to use (or NULL)
 *
 * Parse content from an iostream
 *
 * If the parser requires a base URI and @base_uri is NULL, an error
 * will be generated and the function will fail.
 * 
 * Return value: non 0 on failure, <0 if a required base URI was missing
 **/
int
raptor_parser_parse_iostream(raptor_parser* rdf_parser, raptor_iostream *iostr,
                             raptor_uri *base_uri)
{
  int rc = 0;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(rdf_parser, raptor_parser, 1);
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(iostr, raptor_iostr, 1);

  rc = raptor_parser_parse_start(rdf_parser, base_uri);
  if(rc)
    return rc;
  
  while(!raptor_iostream_read_eof(iostr)) {
    int ilen;
    size_t len;
    int is_end;

    ilen = raptor_iostream_read_bytes(rdf_parser->buffer, 1,
                                      RAPTOR_READ_BUFFER_SIZE, iostr);
    if(ilen < 0)
      break;
    len = RAPTOR_GOOD_CAST(size_t, ilen);
    is_end = (len < RAPTOR_READ_BUFFER_SIZE);

    rc = raptor_parser_parse_chunk(rdf_parser, rdf_parser->buffer, len, is_end);
    if(rc || is_end)
      break;
  }
  
  return rc;
}


/* end not STANDALONE */
#endif


#ifdef STANDALONE
#include <stdio.h>

int main(int argc, char *argv[]);


int
main(int argc, char *argv[])
{
  raptor_world *world;
  const char *program = raptor_basename(argv[0]);
  int i;
  const char *s;

  world = raptor_new_world();
  if(!world || raptor_world_open(world))
    exit(1);
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  fprintf(stderr, "%s: Known options:\n", program);
#endif

  for(i = 0; i <= (int)raptor_option_get_count(); i++) {
    raptor_option_description *od;
    int fn;
    
    od = raptor_world_get_option_description(world,
                                             RAPTOR_DOMAIN_PARSER,
                                             (raptor_option)i);
    if(!od)
      continue;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    fprintf(stderr, " %2d %-20s %s <%s>\n", i, od->name, od->label,
            (od->uri ? (const char*)raptor_uri_as_string(od->uri) : ""));
#endif
    fn = raptor_world_get_option_from_uri(world, od->uri);
    if(fn != i) {
      fprintf(stderr,
              "%s: raptor_option_from_uri() returned %d expected %d\n",
              program, fn, i);
      return 1;
    }
    raptor_free_option_description(od);
  }

  s = raptor_parser_get_accept_header_all(world);
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  fprintf(stderr, "Default HTTP accept header: '%s'\n", s);
#endif
  if(!s) {
    fprintf(stderr, "%s: raptor_parser_get_accept_header_all() failed\n",
            program);
    return 1;
  }
  RAPTOR_FREE(char*, s);

  raptor_free_world(world);
  
  return 0;
}

#endif
