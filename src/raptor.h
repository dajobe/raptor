/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rapier.h - Redland Parser for RDF (Rapier) interfaces and definition
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



#ifndef RAPIER_H
#define RAPIER_H


#ifdef __cplusplus
extern "C" {
#endif

/* Public structure */
typedef struct rapier_parser_s rapier_parser;

typedef enum { RAPIER_SUBJECT_TYPE_RESOURCE, RAPIER_SUBJECT_TYPE_RESOURCE_EACH, RAPIER_SUBJECT_TYPE_RESOURCE_EACH_PREFIX } rapier_subject_type;
typedef enum { RAPIER_PREDICATE_TYPE_PREDICATE, RAPIER_PREDICATE_TYPE_ORDINAL, RAPIER_PREDICATE_TYPE_XML_NAME } rapier_predicate_type;
typedef enum { RAPIER_OBJECT_TYPE_RESOURCE, RAPIER_OBJECT_TYPE_LITERAL, RAPIER_OBJECT_TYPE_XML_LITERAL, RAPIER_OBJECT_TYPE_XML_NAME } rapier_object_type;

typedef enum { RAPIER_URI_SOURCE_NOT_URI, RAPIER_URI_SOURCE_ELEMENT, RAPIER_URI_SOURCE_ATTRIBUTE, RAPIER_URI_SOURCE_ID, RAPIER_URI_SOURCE_URI, RAPIER_URI_SOURCE_GENERATED } rapier_uri_source;
  

typedef struct {
#ifdef LIBRDF_INTERNAL
  librdf_uri *uri;
#else
  const char *uri;
#endif
  const char *file;
  int line;
  int column;
  int byte;  
} rapier_locator;


typedef enum {
  RAPIER_FEATURE_SCANNING,
  RAPIER_FEATURE_ALLOW_NON_NS_ATTRIBUTES,
  RAPIER_FEATURE_INTERPRET_CONTAINERS_AS_TYPEDNODE,
  RAPIER_FEATURE_ALLOW_OTHER_PARSETYPES
} rapier_feature;


/* Returned by statement_handler */
typedef struct {
  const void *subject;
  rapier_subject_type subject_type;
  const void *predicate;
  rapier_predicate_type predicate_type;
  const void *object;
  rapier_object_type object_type;
} rapier_statement;


typedef void (*rapier_message_handler)(void *user_data, rapier_locator* locator, const char *msg, ...);
typedef void (*rapier_statement_handler)(void *user_data, const rapier_statement *statement);
#ifdef LIBRDF_INTERNAL
typedef librdf_uri* (*rapier_container_test_handler)(librdf_uri *element_uri);
#else
typedef const char* (*rapier_container_test_handler)(const char *element_uri);
#endif



/* Public functions */

/* Create */
#ifdef LIBRDF_INTERNAL
rapier_parser* rapier_new(librdf_world *world);
#else
rapier_parser* rapier_new(void);
#endif

/* Destroy */
void rapier_free(rapier_parser *rdf_parser);

/* Handlers */
void rapier_set_fatal_error_handler(rapier_parser* parser, void *user_data, rapier_message_handler handler);
void rapier_set_error_handler(rapier_parser* parser, void *user_data, rapier_message_handler handler);
void rapier_set_warning_handler(rapier_parser* parser, void *user_data, rapier_message_handler handler);
void rapier_set_statement_handler(rapier_parser* parser, void *user_data, rapier_statement_handler handler);

void rapier_print_statement(const rapier_statement * const statement, FILE *stream);

/* Parsing functions */
#ifdef LIBRDF_INTERNAL
int rapier_parse_file(rapier_parser* rdf_parser,  librdf_uri *uri, librdf_uri *base_uri);
#else
int rapier_parse_file(rapier_parser* rdf_parser,  const char *filename, const char *base_uri);
#endif

/* Utility functions */
void rapier_print_locator(FILE *stream, rapier_locator* locator);

void rapier_set_feature(rapier_parser *parser, rapier_feature feature, int value);


#ifndef LIBRDF_INTERNAL

#define RAPIER_RDF_MS_URI "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define RAPIER_RDF_SCHEMA_URI "http://www.w3.org/2000/01/rdf-schema#"

#endif

#ifdef __cplusplus
}
#endif

#endif
