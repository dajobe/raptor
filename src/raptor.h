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


/* Public structure */
typedef struct raptor_parser_s raptor_parser;

typedef enum { RAPTOR_SUBJECT_TYPE_RESOURCE, RAPTOR_SUBJECT_TYPE_RESOURCE_EACH, RAPTOR_SUBJECT_TYPE_RESOURCE_EACH_PREFIX, RAPTOR_SUBJECT_TYPE_ANONYMOUS } raptor_subject_type;
typedef enum { RAPTOR_PREDICATE_TYPE_PREDICATE, RAPTOR_PREDICATE_TYPE_ORDINAL, RAPTOR_PREDICATE_TYPE_XML_NAME } raptor_predicate_type;
typedef enum { RAPTOR_OBJECT_TYPE_RESOURCE, RAPTOR_OBJECT_TYPE_LITERAL, RAPTOR_OBJECT_TYPE_XML_LITERAL, RAPTOR_OBJECT_TYPE_XML_NAME, RAPTOR_OBJECT_TYPE_ANONYMOUS } raptor_object_type;

typedef enum { RAPTOR_URI_SOURCE_NOT_URI, RAPTOR_URI_SOURCE_ELEMENT, RAPTOR_URI_SOURCE_ATTRIBUTE, RAPTOR_URI_SOURCE_ID, RAPTOR_URI_SOURCE_URI, RAPTOR_URI_SOURCE_GENERATED } raptor_uri_source;
  
#ifdef LIBRDF_INTERNAL
/* if inside Redland */
typedef librdf_uri raptor_uri;
#else
typedef const char raptor_uri;
#endif

#ifdef LIBRDF_INTERNAL
#define IS_RDF_MS_CONCEPT(name, uri, local_name) librdf_uri_equals(uri, librdf_concept_uris[LIBRDF_CONCEPT_MS_##local_name])
#define RAPTOR_URI_AS_STRING(uri) (librdf_uri_as_string(uri))
#define RAPTOR_URI_AS_FILENAME(uri) (librdf_uri_as_filename(uri))
#undef RAPTOR_URI_TO_FILENAME
#define RAPTOR_FREE_URI(uri) librdf_free_uri(uri)
#else
#define IS_RDF_MS_CONCEPT(name, uri, local_name) !strcmp(name, #local_name)
#define RAPTOR_URI_AS_STRING(uri) ((const char*)uri)
#undef RAPTOR_URI_AS_FILENAME
#define RAPTOR_URI_TO_FILENAME(uri) (raptor_file_uri_to_filename(uri))
#define RAPTOR_FREE_URI(uri) LIBRDF_FREE(cstring, uri)
#endif


typedef struct {
  raptor_uri *uri;
  const char *file;
  int line;
  int column;
  int byte;  
} raptor_locator;


typedef enum {
  RAPTOR_FEATURE_SCANNING,
  RAPTOR_FEATURE_ALLOW_NON_NS_ATTRIBUTES,
  RAPTOR_FEATURE_INTERPRET_CONTAINERS_AS_TYPEDNODE,
  RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES
} raptor_feature;


/* Returned by statement_handler */
typedef struct {
  const void *subject;
  raptor_subject_type subject_type;
  const void *predicate;
  raptor_predicate_type predicate_type;
  const void *object;
  raptor_object_type object_type;
} raptor_statement;


typedef void (*raptor_message_handler)(void *user_data, raptor_locator* locator, const char *msg, ...);
typedef void (*raptor_statement_handler)(void *user_data, const raptor_statement *statement);
typedef raptor_uri* (*raptor_container_test_handler)(raptor_uri *element_uri);



/* Public functions */

/* Create */
#ifdef LIBRDF_INTERNAL
RAPTOR_API raptor_parser* raptor_new(librdf_world *world);
#else
RAPTOR_API raptor_parser* raptor_new(void);
#endif

/* Destroy */
RAPTOR_API void raptor_free(raptor_parser *rdf_parser);

/* Handlers */
RAPTOR_API void raptor_set_fatal_error_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API void raptor_set_error_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API void raptor_set_warning_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API void raptor_set_statement_handler(raptor_parser* parser, void *user_data, raptor_statement_handler handler);

RAPTOR_API void raptor_print_statement(const raptor_statement * const statement, FILE *stream);

/* Parsing functions */
#ifdef LIBRDF_INTERNAL
RAPTOR_API int raptor_parse_file(raptor_parser* rdf_parser,  librdf_uri *uri, librdf_uri *base_uri);
#else
RAPTOR_API int raptor_parse_file(raptor_parser* rdf_parser,  const char *filename, const char *base_uri);
#endif

/* Utility functions */
RAPTOR_API void raptor_print_locator(FILE *stream, raptor_locator* locator);

RAPTOR_API void raptor_set_feature(raptor_parser *parser, raptor_feature feature, int value);

/* URI functions */
RAPTOR_API raptor_uri* raptor_make_uri(raptor_uri *base_uri, const char *uri_string);
RAPTOR_API raptor_uri* raptor_copy_uri(raptor_uri *uri);


#ifndef LIBRDF_INTERNAL

#define RAPTOR_RDF_MS_URI "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define RAPTOR_RDF_SCHEMA_URI "http://www.w3.org/2000/01/rdf-schema#"
#define RAPTOR_DAML_OIL_URI "http://www.daml.org/2001/03/daml+oil#"

#endif

#ifdef __cplusplus
}
#endif

#endif
