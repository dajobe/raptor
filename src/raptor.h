/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor.h - Redland Parser Toolkit for RDF (Raptor) interfaces and definition
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



#ifndef RAPTOR_H
#define RAPTOR_H


#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#  ifdef RAPTOR_INTERNAL
#    define RAPTOR_API _declspec(dllexport)
#  else
#    define RAPTOR_API _declspec(dllimport)
#  endif
#else
#  define RAPTOR_API
#endif

/* Use gcc 3.1+ feature to allow marking of deprecated API calls.
 * This gives a warning during compiling.
 */
#if ( __GNUC__ == 3 && __GNUC_MINOR__ > 0 ) || __GNUC__ > 3
#ifdef __APPLE_CC__
/* OSX gcc cpp-precomp is broken */
#define RAPTOR_DEPRECATED
#else
#define RAPTOR_DEPRECATED __attribute__((deprecated))
#endif
#else
#define RAPTOR_DEPRECATED
#endif

typedef void* raptor_uri;



/* Public structure */
typedef struct raptor_parser_s raptor_parser;

typedef struct raptor_www_s raptor_www;

/* OLD structure - can't deprecate a typedef */
typedef raptor_parser raptor_ntriples_parser;

typedef enum {
  RAPTOR_IDENTIFIER_TYPE_UNKNOWN,             /* Unknown type - illegal */
  RAPTOR_IDENTIFIER_TYPE_RESOURCE,            /* Resource URI (e.g. rdf:about) */
  RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,           /* _:foo N-Triples, or generated */
  RAPTOR_IDENTIFIER_TYPE_PREDICATE,           /* Predicate URI */
  RAPTOR_IDENTIFIER_TYPE_ORDINAL,             /* rdf:li, rdf:_<n> etc. */
  RAPTOR_IDENTIFIER_TYPE_LITERAL,             /* regular literal */
  RAPTOR_IDENTIFIER_TYPE_XML_LITERAL          /* rdf:parseType="Literal" */
} raptor_identifier_type;

typedef enum { RAPTOR_URI_SOURCE_UNKNOWN, RAPTOR_URI_SOURCE_NOT_URI, RAPTOR_URI_SOURCE_ELEMENT, RAPTOR_URI_SOURCE_ATTRIBUTE, RAPTOR_URI_SOURCE_ID, RAPTOR_URI_SOURCE_URI, RAPTOR_URI_SOURCE_GENERATED, RAPTOR_URI_SOURCE_BLANK_ID } raptor_uri_source;

typedef enum { RAPTOR_NTRIPLES_TERM_TYPE_URI_REF, RAPTOR_NTRIPLES_TERM_TYPE_BLANK_NODE, RAPTOR_NTRIPLES_TERM_TYPE_LITERAL } raptor_ntriples_term_type;


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
  RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES,
  RAPTOR_FEATURE_ALLOW_BAGID
} raptor_feature;


typedef enum {
  RAPTOR_GENID_TYPE_BNODEID,
  RAPTOR_GENID_TYPE_BAGID
} raptor_genid_type;


typedef struct {
  raptor_identifier_type type;
  raptor_uri *uri;
  raptor_uri_source uri_source;
  const unsigned char *id;
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
  const unsigned char *object_literal_language;
} raptor_statement;


typedef void (*raptor_message_handler)(void *user_data, raptor_locator* locator, const char *message);
typedef void (*raptor_statement_handler)(void *user_data, const raptor_statement *statement);
typedef const unsigned char* (*raptor_generate_id_handler)(void *user_data, raptor_genid_type type);
typedef raptor_uri* (*raptor_container_test_handler)(raptor_uri *element_uri);
typedef void (*raptor_www_write_bytes_handler)(raptor_www* www, void *userdata, const void *ptr, size_t size, size_t nmemb);
typedef void (*raptor_www_content_type_handler)(raptor_www* www, void *userdata, const char *content_type);


/* Public functions */

RAPTOR_API void raptor_init(void);
RAPTOR_API void raptor_finish(void);

/* Create */
RAPTOR_API raptor_parser* raptor_new_parser(const char *name);

RAPTOR_API int raptor_start_parse(raptor_parser *rdf_parser, raptor_uri *uri);

/* Destroy */
RAPTOR_API void raptor_free_parser(raptor_parser* parser);

/* Handlers */
RAPTOR_API void raptor_set_fatal_error_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API void raptor_set_error_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API void raptor_set_warning_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API void raptor_set_statement_handler(raptor_parser* parser, void *user_data, raptor_statement_handler handler);
RAPTOR_API void raptor_set_generate_id_handler(raptor_parser* parser, void *user_data, raptor_generate_id_handler handler);

RAPTOR_API void raptor_print_statement(const raptor_statement * const statement, FILE *stream);
RAPTOR_API void raptor_print_statement_as_ntriples(const raptor_statement * statement, FILE *stream);
RAPTOR_API void raptor_print_statement_detailed(const raptor_statement * statement, int detailed, FILE *stream);
RAPTOR_API char* raptor_statement_part_as_counted_string(const void *term, raptor_identifier_type type, raptor_uri* literal_datatype, const unsigned char *literal_language, size_t* len_p);
RAPTOR_API char* raptor_statement_part_as_string(const void *term, raptor_identifier_type type, raptor_uri* literal_datatype, const unsigned char *literal_language);  

RAPTOR_API raptor_locator* raptor_get_locator(raptor_parser* rdf_parser);

RAPTOR_API void raptor_set_default_generate_id_parameters(raptor_parser* rdf_parser, char *prefix, int base);

/* Parsing functions */
RAPTOR_API int raptor_parse_chunk(raptor_parser* rdf_parser, const unsigned char *buffer, size_t len, int is_end);
RAPTOR_API int raptor_parse_file(raptor_parser* rdf_parser, raptor_uri *uri, raptor_uri *base_uri);
RAPTOR_API int raptor_parse_uri(raptor_parser* rdf_parser, raptor_uri *uri, raptor_uri *base_uri);
RAPTOR_API int raptor_parse_uri_with_connection(raptor_parser* rdf_parser, raptor_uri *uri, raptor_uri *base_uri, void *connection);
RAPTOR_API void raptor_parse_abort(raptor_parser* rdf_parser);
RAPTOR_API RAPTOR_DEPRECATED void raptor_parser_abort(raptor_parser* rdf_parser, char *reason);

/* Utility functions */
RAPTOR_API void raptor_print_locator(FILE *stream, raptor_locator* locator);
RAPTOR_API int raptor_format_locator(char *buffer, size_t length, raptor_locator* locator);
 
RAPTOR_API const char* raptor_get_name(raptor_parser *rdf_parser);
RAPTOR_API const char* raptor_get_label(raptor_parser *rdf_parser);
RAPTOR_API void raptor_set_feature(raptor_parser *parser, raptor_feature feature, int value);
RAPTOR_API void raptor_set_parser_strict(raptor_parser* rdf_parser, int is_strict);

/* URI functions */
RAPTOR_API raptor_uri* raptor_new_uri(const char *uri_string);
RAPTOR_API raptor_uri* raptor_new_uri_from_uri_local_name(raptor_uri *uri, const char *local_name);
RAPTOR_API raptor_uri* raptor_new_uri_relative_to_base(raptor_uri *base_uri, const char *uri_string);
RAPTOR_API raptor_uri* raptor_new_uri_from_id(raptor_uri *base_uri, const unsigned char *id);
RAPTOR_API raptor_uri* raptor_new_uri_for_rdf_concept(const char *name);
RAPTOR_API void raptor_free_uri(raptor_uri *uri);
RAPTOR_API int raptor_uri_equals(raptor_uri* uri1, raptor_uri* uri2);
RAPTOR_API raptor_uri* raptor_uri_copy(raptor_uri *uri);
RAPTOR_API char* raptor_uri_as_string(raptor_uri *uri);
RAPTOR_API char* raptor_uri_as_counted_string(raptor_uri *uri, size_t* len_p);

/* Make an xml:base-compatible URI from an existing one */
RAPTOR_API raptor_uri* raptor_new_uri_for_xmlbase(raptor_uri* old_uri);


/* Identifier functions */
RAPTOR_API raptor_identifier* raptor_new_identifier(raptor_identifier_type type, raptor_uri *uri, raptor_uri_source uri_source, unsigned char *id);
RAPTOR_API void raptor_init_identifier(raptor_identifier *identifier, raptor_identifier_type type, raptor_uri *uri, raptor_uri_source uri_source, unsigned char *id);
RAPTOR_API int raptor_copy_identifier(raptor_identifier *dest, raptor_identifier *src);
RAPTOR_API void raptor_free_identifier(raptor_identifier *identifier);

/* Utility functions */
RAPTOR_API int raptor_print_ntriples_string(FILE *stream, const char *string, const char delim);
RAPTOR_API const char* raptor_ntriples_term_as_string (raptor_ntriples_term_type term);
RAPTOR_API size_t raptor_xml_escape_string(const unsigned char *string, size_t len, unsigned char *buffer, size_t length, char quote, raptor_message_handler error_handler, void *error_data);

/* raptor_uri.c */
RAPTOR_API void raptor_uri_resolve_uri_reference (const char *base_uri, const char *reference_uri, char *buffer, size_t length);
RAPTOR_API char *raptor_uri_filename_to_uri_string(const char *filename);
RAPTOR_API char *raptor_uri_uri_string_to_filename(const char *uri_string);
RAPTOR_API int raptor_uri_is_file_uri(const char* uri_string);
RAPTOR_API void raptor_uri_init(void);

typedef raptor_uri* (*raptor_new_uri_func) (void *context, const char *uri_string);
typedef raptor_uri* (*raptor_new_uri_from_uri_local_name_func) (void *context, raptor_uri *uri, const char *local_name);
typedef raptor_uri* (*raptor_new_uri_relative_to_base_func) (void *context, raptor_uri *base_uri, const char *uri_string);
typedef raptor_uri* (*raptor_new_uri_for_rdf_concept_func) (void *context, const char *name);
typedef void (*raptor_free_uri_func) (void *context, raptor_uri *uri);
typedef int (*raptor_uri_equals_func) (void *context, raptor_uri* uri1, raptor_uri* uri2);
typedef raptor_uri* (*raptor_uri_copy_func) (void *context, raptor_uri *uri);
typedef char* (*raptor_uri_as_string_func)(void *context, raptor_uri *uri);
typedef char* (*raptor_uri_as_counted_string_func)(void *context, raptor_uri *uri, size_t* len_p);

typedef struct {
  /* constructors */
  raptor_new_uri_func                     new_uri;
  raptor_new_uri_from_uri_local_name_func new_uri_from_uri_local_name;
  raptor_new_uri_relative_to_base_func    new_uri_relative_to_base;
  raptor_new_uri_for_rdf_concept_func     new_uri_for_rdf_concept;
  /* destructor */
  raptor_free_uri_func                    free_uri;
  /* methods */
  raptor_uri_equals_func                  uri_equals;
  raptor_uri_copy_func                    uri_copy; /* well, copy constructor */
  raptor_uri_as_string_func               uri_as_string;
  raptor_uri_as_counted_string_func       uri_as_counted_string;
  int initialised;
} raptor_uri_handler;


RAPTOR_API void raptor_uri_set_handler(raptor_uri_handler *handler, void *context);
RAPTOR_API void raptor_uri_get_handler(raptor_uri_handler **handler, void **context);

#define RAPTOR_RDF_MS_URI "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define RAPTOR_RDF_SCHEMA_URI "http://www.w3.org/2000/01/rdf-schema#"


/* raptor_www */
RAPTOR_API void raptor_www_init(void);
RAPTOR_API void raptor_www_finish(void);

RAPTOR_API void raptor_www_no_www_library_init_finish(void);

RAPTOR_API raptor_www *raptor_www_new(void);
RAPTOR_API raptor_www *raptor_www_new_with_connection(void* connection);
RAPTOR_API void raptor_www_free(raptor_www *www);
RAPTOR_API void raptor_www_set_user_agent(raptor_www *www, const char *user_agent);
RAPTOR_API void raptor_www_set_proxy(raptor_www *www, const char *proxy);
void
RAPTOR_API raptor_www_set_write_bytes_handler(raptor_www *www, raptor_www_write_bytes_handler handler, void *user_data);
RAPTOR_API void raptor_www_set_content_type_handler(raptor_www *www, raptor_www_content_type_handler handler, void *user_data);
RAPTOR_API void raptor_www_set_error_handler(raptor_www *www, raptor_message_handler error_handler, void *error_data);
RAPTOR_API int raptor_www_fetch(raptor_www *www, raptor_uri *uri);
RAPTOR_API void* raptor_www_get_connection(raptor_www *www);
RAPTOR_API void raptor_www_abort(raptor_www *www, const char *reason);
  

/* OLD RDF/XML Parser Public functions */

RAPTOR_API raptor_parser* RAPTOR_DEPRECATED raptor_new(void);
RAPTOR_API void RAPTOR_DEPRECATED raptor_free(raptor_parser *rdf_parser);

/* OLD N-Triples Parser Public functions */

RAPTOR_API raptor_parser* RAPTOR_DEPRECATED raptor_ntriples_new(void);
RAPTOR_API void RAPTOR_DEPRECATED raptor_ntriples_free(raptor_parser *parser);

RAPTOR_API void RAPTOR_DEPRECATED raptor_ntriples_set_error_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API void RAPTOR_DEPRECATED raptor_ntriples_set_fatal_error_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API void RAPTOR_DEPRECATED raptor_ntriples_set_statement_handler(raptor_parser* parser, void *user_data, raptor_statement_handler handler);

RAPTOR_API int RAPTOR_DEPRECATED raptor_ntriples_parse_file(raptor_parser* parser, raptor_uri *uri, raptor_uri *base_uri);

#ifdef __cplusplus
}
#endif

#endif
