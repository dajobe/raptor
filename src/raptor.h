/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor.h - Redland Parser Toolkit for RDF (Raptor) interfaces and definition
 *
 * $Id$
 *
 * Copyright (C) 2000-2001 David Beckett - http://purl.org/net/dajobe/
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



#ifndef RAPTOR_H
#define RAPTOR_H


#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
  #ifdef RAPTOR_INTERNAL
    #define RAPTOR_API _declspec(dllexport)
  #else
    #define RAPTOR_API _declspec(dllimport)
  #endif
#else
  #define RAPTOR_API
#endif

#ifdef RAPTOR_IN_REDLAND
#include <librdf.h>
#include <rdf_uri.h>

typedef librdf_uri raptor_uri;
#else
typedef const char raptor_uri;
#endif



/* Public structure */
typedef struct raptor_parser_s raptor_parser;

typedef enum {
  RAPTOR_IDENTIFIER_TYPE_UNKNOWN,             /* Unknown type - illegal */
  RAPTOR_IDENTIFIER_TYPE_RESOURCE,            /* Resource URI (e.g. rdf:about) */
  RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,           /* _:foo N-Triples, or generated */
  RAPTOR_IDENTIFIER_TYPE_PREDICATE,           /* Predicate URI */
  RAPTOR_IDENTIFIER_TYPE_ORDINAL,             /* rdf:li, rdf:_<n> etc. */
  RAPTOR_IDENTIFIER_TYPE_LITERAL,             /* regular literal */
  RAPTOR_IDENTIFIER_TYPE_XML_LITERAL,         /* rdf:parseType="Literal" */
} raptor_identifier_type;

typedef enum { RAPTOR_URI_SOURCE_UNKNOWN, RAPTOR_URI_SOURCE_NOT_URI, RAPTOR_URI_SOURCE_ELEMENT, RAPTOR_URI_SOURCE_ATTRIBUTE, RAPTOR_URI_SOURCE_ID, RAPTOR_URI_SOURCE_URI, RAPTOR_URI_SOURCE_GENERATED, RAPTOR_URI_SOURCE_BLANK_ID } raptor_uri_source;


typedef struct {
  raptor_uri *uri;
  const char *file;
  int line;
  int column;
  int byte;  
} raptor_locator;


typedef enum {
  RAPTOR_FEATURE_SCANNING,
  RAPTOR_FEATURE_ASSUME_IS_RDF,
  RAPTOR_FEATURE_ALLOW_NON_NS_ATTRIBUTES,
  RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES
} raptor_feature;


typedef struct {
  raptor_identifier_type type;
  raptor_uri *uri;
  raptor_uri_source uri_source;
  const char *id;
  int ordinal;
  int is_malloced;
} raptor_identifier;


/* Returned by statement_handler */
typedef struct {
  const void *subject;
  raptor_identifier_type subject_type;
  const void *predicate;
  raptor_identifier_type predicate_type;
  const void *object;
  raptor_identifier_type object_type;
  raptor_uri *object_literal_datatype;
  const char *object_literal_language;
} raptor_statement;


typedef void (*raptor_message_handler)(void *user_data, raptor_locator* locator, const char *message);
typedef void (*raptor_statement_handler)(void *user_data, const raptor_statement *statement);
typedef raptor_uri* (*raptor_container_test_handler)(raptor_uri *element_uri);



/* Public functions */

RAPTOR_API void raptor_init(void);
RAPTOR_API void raptor_finish(void);

/* Create */
/* OLD API */
#ifdef RAPTOR_IN_REDLAND
RAPTOR_API raptor_parser* raptor_new(librdf_world *world);
#else
RAPTOR_API raptor_parser* raptor_new(void);
#endif
/* NEW API */
RAPTOR_API raptor_parser* raptor_new_parser(const char *name);

RAPTOR_API int raptor_start_parse(raptor_parser *rdf_parser, raptor_uri *uri);
RAPTOR_API int raptor_start_parse_file(raptor_parser *rdf_parser, const char *filename, raptor_uri *uri);


/* Destroy */
/* OLD API */
RAPTOR_API void raptor_free(raptor_parser *rdf_parser);
/* NEW API */
void raptor_free_parser(raptor_parser* parser);

/* Handlers */
RAPTOR_API void raptor_set_fatal_error_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API void raptor_set_error_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API void raptor_set_warning_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API void raptor_set_statement_handler(raptor_parser* parser, void *user_data, raptor_statement_handler handler);
#ifdef RAPTOR_IN_REDLAND
RAPTOR_API void raptor_set_redland_world(raptor_parser* parser, librdf_world *world);
#endif

RAPTOR_API void raptor_print_statement(const raptor_statement * const statement, FILE *stream);
RAPTOR_API void raptor_print_statement_as_ntriples(const raptor_statement * statement, FILE *stream);
RAPTOR_API void raptor_print_statement_detailed(const raptor_statement * statement, int detailed, FILE *stream);

RAPTOR_API raptor_locator* raptor_get_locator(raptor_parser* rdf_parser);


/* Parsing functions */
int raptor_parse_chunk(raptor_parser* rdf_parser, const char *buffer, size_t len, int is_end);
#ifdef RAPTOR_IN_REDLAND
RAPTOR_API int raptor_parse_file(raptor_parser* rdf_parser,  librdf_uri *uri, librdf_uri *base_uri);
#else
RAPTOR_API int raptor_parse_file(raptor_parser* rdf_parser,  const char *filename, const char *base_uri);
#endif

/* Utility functions */
RAPTOR_API void raptor_print_locator(FILE *stream, raptor_locator* locator);
RAPTOR_API int raptor_format_locator(char *buffer, size_t length, raptor_locator* locator);
 
RAPTOR_API void raptor_set_feature(raptor_parser *parser, raptor_feature feature, int value);

/* URI functions */
RAPTOR_API raptor_uri* raptor_make_uri(raptor_uri *base_uri, const char *uri_string);
RAPTOR_API raptor_uri* raptor_copy_uri(raptor_uri *uri);

/* Identifier functions */
RAPTOR_API raptor_identifier* raptor_new_identifier(raptor_identifier_type type, raptor_uri *uri, raptor_uri_source uri_source, char *id);
RAPTOR_API void raptor_init_identifier(raptor_identifier *identifier, raptor_identifier_type type, raptor_uri *uri, raptor_uri_source uri_source, char *id);
RAPTOR_API int raptor_copy_identifier(raptor_identifier *dest, raptor_identifier *src);
RAPTOR_API void raptor_free_identifier(raptor_identifier *identifier);

RAPTOR_API int raptor_print_ntriples_string(FILE *stream, const char *string, const char delim);

/* raptor_uri.c */
RAPTOR_API void raptor_uri_resolve_uri_reference (const char *base_uri, const char *reference_uri, char *buffer, size_t length);
RAPTOR_API char *raptor_uri_filename_to_uri_string(const char *filename);
RAPTOR_API char *raptor_uri_uri_string_to_filename(const char *uri_string);
RAPTOR_API int raptor_uri_is_file_uri(const char* uri_string);

/* FIXME temporary redland stuff */
void raptor_set_world(raptor_parser *rdf_parser, void *world);

#ifndef RAPTOR_IN_REDLAND

#define RAPTOR_RDF_MS_URI "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define RAPTOR_RDF_SCHEMA_URI "http://www.w3.org/2000/01/rdf-schema#"
#define RAPTOR_DAML_OIL_URI "http://www.daml.org/2001/03/daml+oil#"

#endif

#ifdef __cplusplus
}
#endif

#endif
