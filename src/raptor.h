/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor.h - Redland Parser Toolkit for RDF (Raptor) interfaces and definition
 *
 * $Id$
 *
 * Copyright (C) 2000-2005, David Beckett http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology http://www.ilrt.bristol.ac.uk/
 * University of Bristol, UK http://www.bristol.ac.uk/
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



#ifndef RAPTOR_H
#define RAPTOR_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/* Required for va_list in raptor_vsnprintf */
#include <stdarg.h>

#ifdef WIN32
#  ifdef RAPTOR_STATIC
#    define RAPTOR_API
#  else
#    ifdef RAPTOR_INTERNAL
#      define RAPTOR_API _declspec(dllexport)
#    else
#      define RAPTOR_API _declspec(dllimport)
#    endif
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


/* Public statics */
RAPTOR_API
extern const char * const raptor_short_copyright_string;
RAPTOR_API
extern const char * const raptor_copyright_string;
RAPTOR_API
extern const char * const raptor_version_string;
RAPTOR_API
extern const unsigned int raptor_version_major;
RAPTOR_API
extern const unsigned int raptor_version_minor;
RAPTOR_API
extern const unsigned int raptor_version_release;
RAPTOR_API
extern const unsigned int raptor_version_decimal;

RAPTOR_API
extern const unsigned char * const raptor_xml_namespace_uri;
RAPTOR_API
extern const unsigned char * const raptor_rdf_namespace_uri;
RAPTOR_API
extern const unsigned char * const raptor_rdf_schema_namespace_uri;
RAPTOR_API
extern const unsigned char * const raptor_xmlschema_datatypes_namespace_uri;
RAPTOR_API
extern const unsigned char * const raptor_owl_namespace_uri;

/**
 * raptor_rdf_namespace_uri_len:
 *
 * Length of #raptor_rdf_namespace_uri string
 */
RAPTOR_API
extern const unsigned int raptor_rdf_namespace_uri_len;

RAPTOR_API
extern const unsigned char * const raptor_xml_literal_datatype_uri_string;
RAPTOR_API
extern const unsigned int raptor_xml_literal_datatype_uri_string_len;


/* Public structure */
typedef struct raptor_parser_s raptor_parser;
typedef struct raptor_serializer_s raptor_serializer;

typedef struct raptor_www_s raptor_www;
typedef struct raptor_iostream_s raptor_iostream;

typedef struct raptor_xml_element_s raptor_xml_element;
typedef struct raptor_xml_writer_s raptor_xml_writer;

typedef struct raptor_qname_s raptor_qname;
typedef struct raptor_namespace_s raptor_namespace;
typedef struct raptor_namespace_stack_s raptor_namespace_stack;

/**
 * raptor_ntriples_parser:
 *
 * @Deprecated: use #raptor_parser.
 *
 * old structure - use #raptor_parser instead.
*/
typedef raptor_parser raptor_ntriples_parser;

/**
 * raptor_identifier_type:
 * @RAPTOR_IDENTIFIER_TYPE_RESOURCE:    Resource URI (e.g. <literal>rdf:about</literal>)
 * @RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:   <literal>_:foo</literal> N-Triples, or generated
 * @RAPTOR_IDENTIFIER_TYPE_PREDICATE:   predicate URI
 * @RAPTOR_IDENTIFIER_TYPE_ORDINAL:     <literal>rdf:li</literal>, <literal>rdf:_</literal><emphasis>n</emphasis>
 * @RAPTOR_IDENTIFIER_TYPE_LITERAL:     regular literal
 * @RAPTOR_IDENTIFIER_TYPE_XML_LITERAL: <literal>rdf:parseType="Literal"</literal>
 * @RAPTOR_IDENTIFIER_TYPE_UNKNOWN:     Internal
 *
 * Type of identifier in a #raptor_statement
 *
 */
typedef enum {
  RAPTOR_IDENTIFIER_TYPE_UNKNOWN,
  RAPTOR_IDENTIFIER_TYPE_RESOURCE,
  RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
  RAPTOR_IDENTIFIER_TYPE_PREDICATE,
  RAPTOR_IDENTIFIER_TYPE_ORDINAL,
  RAPTOR_IDENTIFIER_TYPE_LITERAL,
  RAPTOR_IDENTIFIER_TYPE_XML_LITERAL
} raptor_identifier_type;


/**
 * raptor_uri_source:
 * @RAPTOR_URI_SOURCE_UNKNOWN: Internal
 * @RAPTOR_URI_SOURCE_NOT_URI: Internal
 * @RAPTOR_URI_SOURCE_ELEMENT: Internal
 * @RAPTOR_URI_SOURCE_ATTRIBUTE: Internal
 * @RAPTOR_URI_SOURCE_ID: Internal
 * @RAPTOR_URI_SOURCE_URI: Internal
 * @RAPTOR_URI_SOURCE_GENERATED: Internal
 * @RAPTOR_URI_SOURCE_BLANK_ID: Internal
 *
 * Internal: Where a URI or identifier was derived from
 *
 * Likely to be deprecated in future releases.
 */

typedef enum { RAPTOR_URI_SOURCE_UNKNOWN, RAPTOR_URI_SOURCE_NOT_URI, RAPTOR_URI_SOURCE_ELEMENT, RAPTOR_URI_SOURCE_ATTRIBUTE, RAPTOR_URI_SOURCE_ID, RAPTOR_URI_SOURCE_URI, RAPTOR_URI_SOURCE_GENERATED, RAPTOR_URI_SOURCE_BLANK_ID } raptor_uri_source;

/**
 * raptor_ntriples_term_type:
 * @RAPTOR_NTRIPLES_TERM_TYPE_URI_REF: Internal
 * @RAPTOR_NTRIPLES_TERM_TYPE_BLANK_NODE: Internal
 * @RAPTOR_NTRIPLES_TERM_TYPE_LITERAL: I
 *
 * N-Triples term types
 * 
 * Used for the deprecated function raptor_ntriples_term_as_string() only.
 *
 */
typedef enum { RAPTOR_NTRIPLES_TERM_TYPE_URI_REF, RAPTOR_NTRIPLES_TERM_TYPE_BLANK_NODE, RAPTOR_NTRIPLES_TERM_TYPE_LITERAL } raptor_ntriples_term_type;


/**
 * raptor_locator:
 * @uri: URI of location (or NULL)
 * @file: Filename of location (or NULL)
 * @line: Line number of location (or <0 for no line)
 * @column: Column number of location (or <0 for no column)
 * @byte: Byte number of location (or <0 for no byte)
 *
 * Location information for an error, warning or information message.
 */
typedef struct {
  raptor_uri *uri;
  const char *file;
  int line;
  int column;
  int byte;  
} raptor_locator;

/**
 * raptor_feature:
 * @RAPTOR_FEATURE_SCANNING: If true (default false), the RDF/XML
 *   parser will look for embedded rdf:RDF elements inside the XML
 *   content, and not require that the XML start with an rdf:RDF root
 *   element.
 * @RAPTOR_FEATURE_ASSUME_IS_RDF: If true (default false) then the
 *   RDF/XML parser will assume the content is RDF/XML, not require
 *   that rdf:RDF root element, and immediately interpret the content
 *   as RDF/XML.
 * @RAPTOR_FEATURE_ALLOW_NON_NS_ATTRIBUTES: If true (default true)
 *   then the RDF/XML parser will allow non-XML namespaced attributes
 *   to be accepted as well as rdf: namespaced ones.  For example,
 *   'about' and 'ID' will be interpreted as if they were rdf:about
 *   and rdf:ID respectively.
 * @RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES: If true (default true)
 *   then the RDF/XML parser will allow unknown parsetypes to be
 *   present and will pass them on to the user.  Unimplemented at
 *   present.
 * @RAPTOR_FEATURE_ALLOW_BAGID: If true (default true) then the
 *   RDF/XML parser will support the rdf:bagID attribute that was
 *   removed from the RDF/XML language when it was revised.  This
 *   support may be removed in future.
 * @RAPTOR_FEATURE_ALLOW_RDF_TYPE_RDF_LIST: If true (default false)
 *   then the RDF/XML parser will generate the idList rdf:type
 *   rdf:List triple in the handling of rdf:parseType="Collection".
 *   This triple was removed during the revising of RDF/XML after
 *   collections were initially added.
 * @RAPTOR_FEATURE_NORMALIZE_LANGUAGE: If true (default true) then
 *   XML language values such as from xml:lang will be normalized to
 *   lowercase.
 * @RAPTOR_FEATURE_NON_NFC_FATAL: If true (default false) then
 *  illegal Unicode Normal Form C in literals will give a fatal
 *  error, otherwise just a warning.
 * @RAPTOR_FEATURE_WARN_OTHER_PARSETYPES: If true (default true) then 
 *   the RDF/XML parser will warn about unknown rdf:parseType values.
 * @RAPTOR_FEATURE_CHECK_RDF_ID: If true (default true) then the
 *   RDF/XML will check rdf:ID attribute values for duplicates and
 *   cause an error if any are found.
 * @RAPTOR_FEATURE_RELATIVE_URIS: If true (default true) then
 *   relative URIs will be used wherever possible when serializing.
 * @RAPTOR_FEATURE_START_URI: Set the start URI for serlalizing to use.
 * @RAPTOR_FEATURE_WRITER_AUTO_INDENT: Automatically indent elements when
 *   seriailizing.
 * @RAPTOR_FEATURE_WRITER_AUTO_EMPTY: Automatically detect and
 *   abbreviate empty elements when serializing.
 * @RAPTOR_FEATURE_WRITER_INDENT_WIDTH: Integer number of spaces to use
 *   for each indent level when serializing with auto indent.
 * @RAPTOR_FEATURE_LAST: Internal
 *
 * Raptor parser, serializer or XML writer features.
 */
typedef enum {
  RAPTOR_FEATURE_SCANNING,
  RAPTOR_FEATURE_ASSUME_IS_RDF,
  RAPTOR_FEATURE_ALLOW_NON_NS_ATTRIBUTES,
  RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES,
  RAPTOR_FEATURE_ALLOW_BAGID,
  RAPTOR_FEATURE_ALLOW_RDF_TYPE_RDF_LIST,
  RAPTOR_FEATURE_NORMALIZE_LANGUAGE,
  RAPTOR_FEATURE_NON_NFC_FATAL,
  RAPTOR_FEATURE_WARN_OTHER_PARSETYPES,
  RAPTOR_FEATURE_CHECK_RDF_ID,
  RAPTOR_FEATURE_RELATIVE_URIS,
  RAPTOR_FEATURE_START_URI,
  RAPTOR_FEATURE_WRITER_AUTO_INDENT,
  RAPTOR_FEATURE_WRITER_AUTO_EMPTY,
  RAPTOR_FEATURE_WRITER_INDENT_WIDTH,
  RAPTOR_FEATURE_LAST=RAPTOR_FEATURE_WRITER_INDENT_WIDTH
} raptor_feature;


/**
 * raptor_genid_type:
 * @RAPTOR_GENID_TYPE_BNODEID: Generated ID is for a blank node
 * @RAPTOR_GENID_TYPE_BAGID: Generated ID is for rdf:bagID
 *
 * Intended type for a generated identifier asked for by the handler
 * registered with raptor_set_generate_id_handler().
 */
typedef enum {
  RAPTOR_GENID_TYPE_BNODEID,
  RAPTOR_GENID_TYPE_BAGID
} raptor_genid_type;


/**
 * raptor_identifier:
 * @type: Type of identifier
 * @uri: URI of identifier for types %RAPTOR_IDENTIFIER_TYPE_RESOURCE and
 *   %RAPTOR_IDENTIFIER_TYPE_PREDICATE
 * @uri_source: where the identifier (URI or blank node) came from
 * @id: blank node identifier for type %RAPTOR_IDENTIFIER_TYPE_ANONYMOUS
 * @ordinal: integer ordinal for type %RAPTOR_IDENTIFIER_TYPE_ORDINAL
 * @is_malloced: internal
 * @literal: literal string for types %RAPTOR_IDENTIFIER_TYPE_LITERAL and
 *    %RAPTOR_IDENTIFIER_TYPE_XML_LITERAL
 * @literal_datatype: RDF literal datatype URI for types
 *   %RAPTOR_IDENTIFIER_TYPE_LITERAL and %RAPTOR_IDENTIFIER_TYPE_XML_LITERAL
 * @literal_language: RDF literal language for type
 *   %RAPTOR_IDENTIFIER_TYPE_LITERAL
 *
 * Raptor RDF term identifier.
*/
typedef struct {
  raptor_identifier_type type;
  raptor_uri *uri;
  raptor_uri_source uri_source;
  const unsigned char *id;
  int ordinal;
  int is_malloced;
  const unsigned char *literal;
  raptor_uri *literal_datatype;
  const unsigned char *literal_language;
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


/**
 * raptor_new_uri_func:
 * @context: URI context data
 * @uri_string: URI string
 *
 * Handler function for implementing raptor_new_uri().
 *
 * Return value: new URI object or NULL on failure
 */
typedef raptor_uri* (*raptor_new_uri_func) (void *context, const unsigned char *uri_string);

/**
 * raptor_new_uri_from_uri_local_name_func:
 * @context: URI context data
 * @uri: URI object
 * @local_name: local name string
 *
 * Handler function for implementing raptor_new_uri_from_uri_local_name().
 *
 * Return value: new URI object or NULL on failure
 */
typedef raptor_uri* (*raptor_new_uri_from_uri_local_name_func) (void *context, raptor_uri *uri, const unsigned char *local_name);

/**
 * raptor_new_uri_relative_to_base_func:
 * @context: URI context data
 * @base_uri: base URI object
 * @uri_string: relative URI string
 *
 * Handler function for implementing raptor_new_uri_relative_to_base().
 *
 * Return value: new URI object or NULL on failure
 */
typedef raptor_uri* (*raptor_new_uri_relative_to_base_func) (void *context, raptor_uri *base_uri, const unsigned char *uri_string);

/**
 * raptor_new_uri_for_rdf_concept_func:
 * @context: URI context data
 * @name: RDF term
 *
 * Handler function for implementing raptor_new_uri_for_rdf_concept().
 *
 * Return value: new URI object or NULL on failure
 */
typedef raptor_uri* (*raptor_new_uri_for_rdf_concept_func) (void *context, const char *name);

/**
 * raptor_free_uri_func:
 * @context: URI context data
 * @uri: URI object
 *
 * Handler function for implementing raptor_free_uri().
 */
typedef void (*raptor_free_uri_func) (void *context, raptor_uri *uri);

/**
 * raptor_uri_equals_func:
 * @context: URI context data
 * @uri1: URI object 1
 * @uri2: URI object 2
 *
 * Handler function for implementing raptor_uri_equals().
 *
 * Return value: non-0 if the URIs are equal
 */
typedef int (*raptor_uri_equals_func) (void *context, raptor_uri* uri1, raptor_uri* uri2);

/**
 * raptor_uri_copy_func:
 * @context: URI context data
 * @uri: URI object
 *
 * Handler function for implementing raptor_uri_copy().
 *
 * Return value: new URI object or NULL on failure
 */
typedef raptor_uri* (*raptor_uri_copy_func) (void *context, raptor_uri *uri);

/**
 * raptor_uri_as_string_func:
 * @context: URI context data
 * @uri: URI object
 *
 * Handler function for implementing raptor_uri_as_string().
 *
 * Return value: shared string representation of the URI
 */
typedef unsigned char* (*raptor_uri_as_string_func)(void *context, raptor_uri *uri);

/**
 * raptor_uri_as_counted_string_func:
 * @context: URI context data
 * @uri: URI object
 * @len_p: length
 *
 * Handler function for implementing raptor_uri_as_counted_string().
 *
 * Return value: shared string representation of the URI
 */
typedef unsigned char* (*raptor_uri_as_counted_string_func)(void *context, raptor_uri *uri, size_t* len_p);


/**
 * raptor_uri_handler:
 * @new_uri: function for raptor_new_uri()
 * @new_uri_from_uri_local_name: function for raptor_new_uri_from_uri_local_name()
 * @new_uri_relative_to_base: function for raptor_new_uri_relative_to_base()
 * @new_uri_for_rdf_concept: function for raptor_new_uri_for_rdf_concept()
 * @free_uri: function for raptor_free_uri()
 * @uri_equals: function for raptor_uri_equals()
 * @uri_copy: function for raptor_uri_copy()
 * @uri_as_string: function for raptor_uri_as_string()
 * @uri_as_counted_string: function for raptor_uri_as_counted_string()
 * @initialised: Internal
 *
 * URI implementation handler structure.
 */
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


/**
 * raptor_simple_message_handler:
 * @user_data: user data
 * @message: message to report
 * @...: arguments for message
 *
 * Simple message handler function.
 *
 * Used by multiple functions including raptor_xml_escape_string(),
 * raptor_iostream_write_xml_escaped_string(), raptor_new_qname(),
 * raptor_qname_string_to_uri(), raptor_new_namespaces(),
 * raptor_namespaces_init(), raptor_iostream_write_xml_element(),
 * raptor_new_xml_writer().
 */
typedef void (*raptor_simple_message_handler)(void *user_data, const char *message, ...);

/**
 * raptor_message_handler:
 * @user_data: user data
 * @locator: location associated with message or NULL
 * @message: message to report
 *
 * Message with location handler function.
 *
 * Used during parsing and serializing for errors and warnings that
 * may include location information. Multiple handlers may be set for
 * parsers and serializers by raptor_set_fatal_error_handler(),
 * raptor_set_error_handler(), raptor_set_warning_handler(),
 * raptor_serializer_set_error_handler() and
 * raptor_serializer_set_warning_handler().
 *
 * Also used by raptor_www_set_error_handler() for location-based errors
 * in WWW retrieval.
 */
typedef void (*raptor_message_handler)(void *user_data, raptor_locator* locator, const char *message);

/**
 * raptor_statement_handler:
 * @user_data: user data
 * @statement: statement to report
 *
 * Statement (triple) reporting handler function.
 */
typedef void (*raptor_statement_handler)(void *user_data, const raptor_statement *statement);

/**
 * raptor_generate_id_handler:
 * @user_data: user data
 * @type: type of ID to create
 * @user_bnodeid: a user-specified ID or NULL if none available.
 *
 * Generate an identifier handler function.
 *
 * Return value: new ID to use
 */
typedef unsigned char* (*raptor_generate_id_handler)(void *user_data, raptor_genid_type type, unsigned char* user_bnodeid);

/**
 * raptor_namespace_handler:
 * @user_data: user data
 * @nspace: #raptor_namespace declared
 *
 * XML Namespace declaration reporting handler set by 
 * raptor_set_namespace_handler().
 */
typedef void (*raptor_namespace_handler)(void* user_data, raptor_namespace *nspace);

/**
 * raptor_www_write_bytes_handler:
 * @www: WWW object
 * @userdata: user data
 * @ptr: data pointer
 * @size: size of individual item
 * @nmemb: number of items
 *
 * Receiving bytes of data from WWW retrieval handler.
 *
 * Set by raptor_www_set_write_bytes_handler().
 */
typedef void (*raptor_www_write_bytes_handler)(raptor_www* www, void *userdata, const void *ptr, size_t size, size_t nmemb);

/**
 * raptor_www_content_type_handler:
 * @www: WWW object
 * @userdata: user data
 * @content_type: content type seen
 *
 * Receiving Content-Type: header from WWW retrieval handler.
 *
 * Set by raptor_www_set_content_type_handler().
 */
typedef void (*raptor_www_content_type_handler)(raptor_www* www, void *userdata, const char *content_type);


/* Public functions */

RAPTOR_API
void raptor_init(void);
RAPTOR_API
void raptor_finish(void);

/* Get parser names */
RAPTOR_API
int raptor_parsers_enumerate(const unsigned int counter, const char **name, const char **label);
RAPTOR_API
int raptor_syntaxes_enumerate(const unsigned int counter, const char **name, const char **label, const char **mime_type, const unsigned char **uri_string);
RAPTOR_API
int raptor_syntax_name_check(const char *name);
RAPTOR_API
const char* raptor_guess_parser_name(raptor_uri *uri, const char *mime_type, const unsigned char *buffer, size_t len, const unsigned char *identifier);

/* Create */
RAPTOR_API
raptor_parser* raptor_new_parser(const char *name);
RAPTOR_API
raptor_parser* raptor_new_parser_for_content(raptor_uri *uri, const char *mime_type, const unsigned char *buffer, size_t len, const unsigned char *identifier);

RAPTOR_API
int raptor_start_parse(raptor_parser *rdf_parser, raptor_uri *uri);

/* Destroy */
RAPTOR_API
void raptor_free_parser(raptor_parser* parser);

/* Handlers */
RAPTOR_API
void raptor_set_fatal_error_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API
void raptor_set_error_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API
void raptor_set_warning_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
RAPTOR_API
void raptor_set_statement_handler(raptor_parser* parser, void *user_data, raptor_statement_handler handler);
RAPTOR_API
void raptor_set_generate_id_handler(raptor_parser* parser, void *user_data, raptor_generate_id_handler handler);
RAPTOR_API
void raptor_set_namespace_handler(raptor_parser* parser, void *user_data, raptor_namespace_handler handler);

RAPTOR_API
void raptor_print_statement(const raptor_statement * statement, FILE *stream);
RAPTOR_API
void raptor_print_statement_as_ntriples(const raptor_statement * statement, FILE *stream);
RAPTOR_API RAPTOR_DEPRECATED
void raptor_print_statement_detailed(const raptor_statement * statement, int detailed, FILE *stream);
RAPTOR_API
unsigned char* raptor_statement_part_as_counted_string(const void *term, raptor_identifier_type type, raptor_uri* literal_datatype, const unsigned char *literal_language, size_t* len_p);
RAPTOR_API
unsigned char* raptor_statement_part_as_string(const void *term, raptor_identifier_type type, raptor_uri* literal_datatype, const unsigned char *literal_language);  

RAPTOR_API
raptor_locator* raptor_get_locator(raptor_parser* rdf_parser);

RAPTOR_API
void raptor_set_default_generate_id_parameters(raptor_parser* rdf_parser, char *prefix, int base);

/* Parsing functions */
RAPTOR_API
int raptor_parse_chunk(raptor_parser* rdf_parser, const unsigned char *buffer, size_t len, int is_end);
RAPTOR_API
int raptor_parse_file_stream(raptor_parser* rdf_parser, FILE *stream, const char *filename, raptor_uri *base_uri);
RAPTOR_API
int raptor_parse_file(raptor_parser* rdf_parser, raptor_uri *uri, raptor_uri *base_uri);
RAPTOR_API
int raptor_parse_uri(raptor_parser* rdf_parser, raptor_uri *uri, raptor_uri *base_uri);
RAPTOR_API
int raptor_parse_uri_with_connection(raptor_parser* rdf_parser, raptor_uri *uri, raptor_uri *base_uri, void *connection);
RAPTOR_API
void raptor_parse_abort(raptor_parser* rdf_parser);

/* Utility functions */
RAPTOR_API
void raptor_print_locator(FILE *stream, raptor_locator* locator);
RAPTOR_API
int raptor_format_locator(char *buffer, size_t length, raptor_locator* locator);
RAPTOR_API
int raptor_locator_line(raptor_locator *locator);
RAPTOR_API
int raptor_locator_column(raptor_locator *locator);
RAPTOR_API
int raptor_locator_byte(raptor_locator *locator);
RAPTOR_API
const char * raptor_locator_file(raptor_locator *locator);
RAPTOR_API
const char * raptor_locator_uri(raptor_locator *locator);


RAPTOR_API
const char* raptor_get_name(raptor_parser *rdf_parser);
RAPTOR_API
const char* raptor_get_label(raptor_parser *rdf_parser);
RAPTOR_API
const char* raptor_get_mime_type(raptor_parser *rdf_parser);

RAPTOR_API
int raptor_features_enumerate(const raptor_feature feature, const char **name, raptor_uri **uri, const char **label);
RAPTOR_API
int raptor_set_feature(raptor_parser *parser, raptor_feature feature, int value);
RAPTOR_API
int raptor_parser_set_feature_string(raptor_parser *parser, raptor_feature feature, const unsigned char *value);
RAPTOR_API
int raptor_get_feature(raptor_parser *parser, raptor_feature feature);
RAPTOR_API
const unsigned char* raptor_parser_get_feature_string(raptor_parser *parser, raptor_feature feature);
RAPTOR_API
void raptor_set_parser_strict(raptor_parser* rdf_parser, int is_strict);

/* Get serializer names */
RAPTOR_API
int raptor_serializers_enumerate(const unsigned int counter, const char **name, const char **label, const char **mime_type, const unsigned char **uri_string);
RAPTOR_API
int raptor_serializer_syntax_name_check(const char *name);

/* Serializing */
RAPTOR_API
raptor_serializer* raptor_new_serializer(const char *name);
RAPTOR_API
void raptor_free_serializer(raptor_serializer* rdf_serializer);

RAPTOR_API
int raptor_serialize_start(raptor_serializer *rdf_serializer, raptor_uri *uri, raptor_iostream *iostream);
RAPTOR_API
int raptor_serialize_start_to_filename(raptor_serializer *rdf_serializer, const char *filename);
RAPTOR_API
int raptor_serialize_start_to_string(raptor_serializer *rdf_serializer, raptor_uri *uri, void **string_p, size_t *length_p);
RAPTOR_API
int raptor_serialize_start_to_file_handle(raptor_serializer *rdf_serializer, raptor_uri *uri, FILE *fh);
RAPTOR_API
int raptor_serialize_set_namespace(raptor_serializer* rdf_serializer, raptor_uri *uri, const unsigned char *prefix);
RAPTOR_API
int raptor_serialize_statement(raptor_serializer* rdf_serializer, const raptor_statement *statement);
RAPTOR_API
int raptor_serialize_end(raptor_serializer *rdf_serializer);
RAPTOR_API
raptor_iostream* raptor_serializer_get_iostream(raptor_serializer *serializer);
RAPTOR_API
void raptor_serializer_set_error_handler(raptor_serializer* serializer, void *user_data, raptor_message_handler handler);
RAPTOR_API
void raptor_serializer_set_warning_handler(raptor_serializer* serializer, void *user_data, raptor_message_handler handler);
RAPTOR_API
raptor_locator* raptor_serializer_get_locator(raptor_serializer *rdf_serializer);
RAPTOR_API
int raptor_serializer_features_enumerate(const raptor_feature feature, const char **name,  raptor_uri **uri, const char **label);
RAPTOR_API
int raptor_serializer_set_feature(raptor_serializer *serializer, raptor_feature feature, int value);
RAPTOR_API
int raptor_serializer_set_feature_string(raptor_serializer *serializer, raptor_feature feature, const unsigned char *value);
RAPTOR_API
int raptor_serializer_get_feature(raptor_serializer *serializer, raptor_feature feature);
RAPTOR_API
const unsigned char *raptor_serializer_get_feature_string(raptor_serializer *serializer, raptor_feature feature);

/* memory functions */
RAPTOR_API
void raptor_free_memory(void *ptr);
RAPTOR_API
void* raptor_alloc_memory(size_t size);
RAPTOR_API
void* raptor_calloc_memory(size_t nmemb, size_t size);

/* URI functions */
RAPTOR_API
raptor_uri* raptor_new_uri(const unsigned char *uri_string);
RAPTOR_API
raptor_uri* raptor_new_uri_from_uri_local_name(raptor_uri *uri, const unsigned char *local_name);
RAPTOR_API
raptor_uri* raptor_new_uri_relative_to_base(raptor_uri *base_uri, const unsigned char *uri_string);
RAPTOR_API
raptor_uri* raptor_new_uri_from_id(raptor_uri *base_uri, const unsigned char *id);
RAPTOR_API
raptor_uri* raptor_new_uri_for_rdf_concept(const char *name);
RAPTOR_API
void raptor_free_uri(raptor_uri *uri);
RAPTOR_API
int raptor_uri_equals(raptor_uri* uri1, raptor_uri* uri2);
RAPTOR_API
raptor_uri* raptor_uri_copy(raptor_uri *uri);
RAPTOR_API
unsigned char* raptor_uri_as_string(raptor_uri *uri);
RAPTOR_API
unsigned char* raptor_uri_as_counted_string(raptor_uri *uri, size_t* len_p);

/* Make an xml:base-compatible URI from an existing one */
RAPTOR_API
raptor_uri* raptor_new_uri_for_xmlbase(raptor_uri* old_uri);
/* Make a URI suitable for retrieval (no fragment, has path) from an existing one */
RAPTOR_API
raptor_uri* raptor_new_uri_for_retrieval(raptor_uri* old_uri);

/* Identifier functions */
RAPTOR_API
raptor_identifier* raptor_new_identifier(raptor_identifier_type type, raptor_uri *uri, raptor_uri_source uri_source, const unsigned char *id, const unsigned char *literal, raptor_uri *literal_datatype, const unsigned char *literal_language);
RAPTOR_API
int raptor_copy_identifier(raptor_identifier *dest, raptor_identifier *src);
RAPTOR_API
void raptor_free_identifier(raptor_identifier *identifier);

/* Utility functions */
RAPTOR_API
int raptor_print_ntriples_string(FILE *stream, const unsigned char *string, const char delim);
RAPTOR_API RAPTOR_DEPRECATED
unsigned char* raptor_ntriples_string_as_utf8_string(raptor_parser* rdf_parser, const unsigned char *src, int len, size_t *dest_lenp);
RAPTOR_API RAPTOR_DEPRECATED
const char* raptor_ntriples_term_as_string(raptor_ntriples_term_type term);
RAPTOR_API
int raptor_iostream_write_string_ntriples(raptor_iostream *iostr, const unsigned char *string, size_t len, const char delim);
RAPTOR_API
void raptor_iostream_write_statement_ntriples(raptor_iostream* iostr, const raptor_statement *statement);
RAPTOR_API
int raptor_xml_escape_string(const unsigned char *string, size_t len, unsigned char *buffer, size_t length, char quote, raptor_simple_message_handler error_handler, void *error_data);
RAPTOR_API
int raptor_iostream_write_xml_escaped_string(raptor_iostream* iostr, const unsigned char *string, size_t len, char quote, raptor_simple_message_handler error_handler, void *error_data);

RAPTOR_API
char* raptor_vsnprintf(const char *message, va_list arguments);

RAPTOR_API
int raptor_xml_name_check(const unsigned char *string, size_t length, int xml_version);

/* raptor_rfc2396.c */
RAPTOR_API
void raptor_uri_resolve_uri_reference(const unsigned char *base_uri, const unsigned char *reference_uri, unsigned char* buffer, size_t length);

/* raptor_uri.c */
RAPTOR_API
unsigned char *raptor_uri_filename_to_uri_string(const char *filename);
RAPTOR_API
char *raptor_uri_uri_string_to_filename(const unsigned char *uri_string);
RAPTOR_API
char *raptor_uri_uri_string_to_filename_fragment(const unsigned char *uri_string, unsigned char **fragment_p);
RAPTOR_API
int raptor_uri_is_file_uri(const unsigned char* uri_string);
RAPTOR_API
unsigned char* raptor_uri_to_relative_counted_uri_string(raptor_uri *base_uri, raptor_uri *reference_uri, size_t *length_p);
RAPTOR_API
unsigned char* raptor_uri_to_relative_uri_string(raptor_uri *base_uri,  raptor_uri *reference_uri);
RAPTOR_API
void raptor_uri_print(const raptor_uri* uri, FILE *stream);
RAPTOR_API
unsigned char* raptor_uri_to_counted_string(raptor_uri *uri, size_t *len_p);
RAPTOR_API
unsigned char* raptor_uri_to_string(raptor_uri *uri);

RAPTOR_API
void raptor_uri_set_handler(raptor_uri_handler *handler, void *context);
RAPTOR_API
void raptor_uri_get_handler(raptor_uri_handler **handler, void **context);

/**
 * RAPTOR_RDF_MS_URI:
 *
 * RDF Namespace URI (rdf:).
 *
 * Copy with raptor_uri_copy() to use.
 */
#define RAPTOR_RDF_MS_URI raptor_rdf_namespace_uri

/**
 * RAPTOR_RDF_sCHEMA_URI:
 *
 * RDF Schema Namespace URI (rdfs:).
 *
 * Copy with raptor_uri_copy() to use.
 */
#define RAPTOR_RDF_SCHEMA_URI raptor_rdf_schema_namespace_uri

/**
 * RAPTOR_XML_sCHEMA_DATATYPES_URI:
 *
 * XML Schema Datatypes URI (xsd:).
 *
 * Copy with raptor_uri_copy() to use.
 */
#define RAPTOR_XMLSCHEMA_DATATYPES_URI raptor_xmlschema_datatypes_namespace_uri

/**
 * RAPTOR_OWL_URI:
 *
 * OWL Namespace URI (owl:).
 *
 * Copy with raptor_uri_copy() to use.
 */
#define RAPTOR_OWL_URI raptor_owl_namespace_uri


/* raptor_www */
RAPTOR_API
void raptor_www_init(void);
RAPTOR_API
void raptor_www_finish(void);

RAPTOR_API
void raptor_www_no_www_library_init_finish(void);

RAPTOR_API
raptor_www *raptor_www_new(void);
RAPTOR_API
raptor_www *raptor_www_new_with_connection(void* connection);
RAPTOR_API
void raptor_www_free(raptor_www *www);
RAPTOR_API
void raptor_www_set_user_agent(raptor_www *www, const char *user_agent);
RAPTOR_API
void raptor_www_set_proxy(raptor_www *www, const char *proxy);
RAPTOR_API
void raptor_www_set_http_accept(raptor_www *www, const char *value);
RAPTOR_API
void raptor_www_set_write_bytes_handler(raptor_www *www, raptor_www_write_bytes_handler handler, void *user_data);
RAPTOR_API
void raptor_www_set_content_type_handler(raptor_www *www, raptor_www_content_type_handler handler, void *user_data);
RAPTOR_API
void raptor_www_set_error_handler(raptor_www *www, raptor_message_handler error_handler, void *error_data);
RAPTOR_API
int raptor_www_fetch(raptor_www *www, raptor_uri *uri);
RAPTOR_API
int raptor_www_fetch_to_string(raptor_www *www, raptor_uri *uri, void **string_p, size_t *length_p, void *(*malloc_handler)(size_t size));
RAPTOR_API
void* raptor_www_get_connection(raptor_www *www);
RAPTOR_API
void raptor_www_abort(raptor_www *www, const char *reason);


/* raptor_qname - XML qnames */
RAPTOR_API
raptor_qname* raptor_new_qname(raptor_namespace_stack *nstack, const unsigned char *name, const unsigned char *value, raptor_simple_message_handler error_handler, void *error_data);
RAPTOR_API
raptor_qname* raptor_new_qname_from_namespace_local_name(raptor_namespace *ns, const unsigned char *local_name, const unsigned char *value);
RAPTOR_API
raptor_qname* raptor_qname_copy(raptor_qname *qname);
RAPTOR_API
void raptor_free_qname(raptor_qname* name);
RAPTOR_API
int raptor_qname_equal(raptor_qname *name1, raptor_qname *name2);
/* utility function */
RAPTOR_API
raptor_uri* raptor_qname_string_to_uri(raptor_namespace_stack *nstack,  const unsigned char *name, size_t name_len, raptor_simple_message_handler error_handler, void *error_data);
RAPTOR_API
int raptor_iostream_write_qname(raptor_iostream* iostr, raptor_qname *qname);

/* raptor_namespace_stack - stack of XML namespaces */
RAPTOR_API
raptor_namespace* raptor_new_namespace_from_uri(raptor_namespace_stack *nstack, const unsigned char *prefix,  raptor_uri* ns_uri, int depth);
RAPTOR_API
raptor_namespace_stack* raptor_new_namespaces(raptor_uri_handler *uri_handler, void *uri_context, raptor_simple_message_handler error_handler, void *error_data, int defaults);

RAPTOR_API
void raptor_namespaces_init(raptor_namespace_stack *nstack, raptor_uri_handler *uri_handler, void *uri_context, raptor_simple_message_handler error_handler, void *error_data, int defaults);
RAPTOR_API
void raptor_namespaces_clear(raptor_namespace_stack *nstack);
RAPTOR_API
void raptor_free_namespaces(raptor_namespace_stack *nstack);
RAPTOR_API
void raptor_namespaces_start_namespace(raptor_namespace_stack *nstack, raptor_namespace *nspace);
RAPTOR_API
int raptor_namespaces_start_namespace_full(raptor_namespace_stack *nstack, const unsigned char *prefix, const unsigned char *ns_uri_string, int depth);
RAPTOR_API
void raptor_namespaces_end_for_depth(raptor_namespace_stack *nstack, int depth);
RAPTOR_API
raptor_namespace* raptor_namespaces_get_default_namespace(raptor_namespace_stack *nstack);
RAPTOR_API
raptor_namespace *raptor_namespaces_find_namespace(raptor_namespace_stack *nstack, const unsigned char *prefix, int prefix_length);
RAPTOR_API
raptor_namespace* raptor_namespaces_find_namespace_by_uri(raptor_namespace_stack *nstack, raptor_uri *ns_uri);
RAPTOR_API
int raptor_namespaces_namespace_in_scope(raptor_namespace_stack *nstack, const raptor_namespace *nspace);

/* raptor_namespace - XML namespace */
RAPTOR_API
raptor_namespace* raptor_new_namespace(raptor_namespace_stack *nstack, const unsigned char *prefix, const unsigned char *ns_uri_string, int depth);
RAPTOR_API
void raptor_free_namespace(raptor_namespace *ns);
RAPTOR_API
int raptor_namespace_copy(raptor_namespace_stack *nstack, raptor_namespace *ns, int new_depth);
RAPTOR_API
raptor_uri* raptor_namespace_get_uri(const raptor_namespace *ns);
RAPTOR_API
const unsigned char* raptor_namespace_get_prefix(const raptor_namespace *ns);
RAPTOR_API
unsigned char *raptor_namespaces_format(const raptor_namespace *ns, size_t *length_p);
RAPTOR_API
int raptor_iostream_write_namespace(raptor_iostream* iostr, raptor_namespace *ns);
RAPTOR_API
int raptor_new_namespace_parts_from_string(const unsigned char *string, unsigned char **prefix, unsigned char **uri_string);

typedef struct raptor_stringbuffer_s raptor_stringbuffer;

/* Sequence class */
typedef struct raptor_sequence_s raptor_sequence;

/**
 * raptor_sequence_free_handler:
 * @object: object to free
 *
 * Handler function for freeing a sequence item.
 *
 * Set by raptor_new_sequence().
*/
typedef void (raptor_sequence_free_handler(void* object));
/**
 * raptor_sequence_print_handler:
 * @object: object to print
 * @fh: FILE* to print to
 *
 * Handler function for printing a sequence item.
 *
 * Set by raptor_new_sequence() or raptor_sequence_set_print_handler().
 */
typedef void (raptor_sequence_print_handler(void *object, FILE *fh));

/* Create */
RAPTOR_API
raptor_sequence* raptor_new_sequence(raptor_sequence_free_handler* free_handler, raptor_sequence_print_handler* print_handler);
/* Destroy */
RAPTOR_API
void raptor_free_sequence(raptor_sequence* seq);
/* Methods */
RAPTOR_API
int raptor_sequence_size(raptor_sequence* seq);
RAPTOR_API
int raptor_sequence_set_at(raptor_sequence* seq, int idx, void *data);
RAPTOR_API
int raptor_sequence_push(raptor_sequence* seq, void *data);
RAPTOR_API
int raptor_sequence_shift(raptor_sequence* seq, void *data);
RAPTOR_API
void* raptor_sequence_get_at(raptor_sequence* seq, int idx);
RAPTOR_API
void* raptor_sequence_pop(raptor_sequence* seq);
RAPTOR_API
void* raptor_sequence_unshift(raptor_sequence* seq);

RAPTOR_API
int raptor_compare_strings(const void *a, const void *b);

RAPTOR_API
void raptor_sequence_sort(raptor_sequence* seq, int(*compare)(const void *, const void *));

/* helper for printing sequences of strings */ 
RAPTOR_API
void raptor_sequence_print_string(char *data, FILE *fh);
RAPTOR_API
void raptor_sequence_print_uri(char *data, FILE *fh);
RAPTOR_API
void raptor_sequence_set_print_handler(raptor_sequence *seq, raptor_sequence_print_handler *print_handler);
RAPTOR_API
void raptor_sequence_print(raptor_sequence* seq, FILE* fh);
RAPTOR_API
int raptor_sequence_join(raptor_sequence* dest, raptor_sequence *src);

/* Unicode and UTF8 */
RAPTOR_API
int raptor_unicode_char_to_utf8(unsigned long c, unsigned char *output);
RAPTOR_API
int raptor_utf8_to_unicode_char(unsigned long *output, const unsigned char *input, int length);
RAPTOR_API
int raptor_unicode_is_xml11_namestartchar(long c);
RAPTOR_API
int raptor_unicode_is_xml10_namestartchar(long c);
RAPTOR_API
int raptor_unicode_is_xml11_namechar(long c);
RAPTOR_API
int raptor_unicode_is_xml10_namechar(long c);
RAPTOR_API
int raptor_utf8_check(const unsigned char *string, size_t length);

/* raptor_stringbuffer */
RAPTOR_API
raptor_stringbuffer* raptor_new_stringbuffer(void);
RAPTOR_API
void raptor_free_stringbuffer(raptor_stringbuffer *stringbuffer);
RAPTOR_API
int raptor_stringbuffer_append_counted_string(raptor_stringbuffer* stringbuffer, const unsigned char *string, size_t length, int do_copy);
RAPTOR_API
int raptor_stringbuffer_append_string(raptor_stringbuffer* stringbuffer, const unsigned char *string, int do_copy);
RAPTOR_API
int raptor_stringbuffer_append_decimal(raptor_stringbuffer* stringbuffer, int integer);
RAPTOR_API
int raptor_stringbuffer_append_stringbuffer(raptor_stringbuffer* stringbuffer, raptor_stringbuffer* append);
RAPTOR_API
int raptor_stringbuffer_prepend_counted_string(raptor_stringbuffer* stringbuffer, const unsigned char *string, size_t length, int do_copy);
RAPTOR_API
int raptor_stringbuffer_prepend_string(raptor_stringbuffer* stringbuffer, const unsigned char *string, int do_copy);
RAPTOR_API
unsigned char * raptor_stringbuffer_as_string(raptor_stringbuffer* stringbuffer);
RAPTOR_API
size_t raptor_stringbuffer_length(raptor_stringbuffer* stringbuffer);
RAPTOR_API
int raptor_stringbuffer_copy_to_string(raptor_stringbuffer* stringbuffer, unsigned char *string, size_t length);

/**
 * raptor_iostream_init_func:
 * @context: stream context data
 *
 * Handler function for #raptor_iostream initialising.
 *
 * Return value: non-0 on failure.
 */
typedef int (*raptor_iostream_init_func) (void *context);

/**
 * raptor_iostream_finish_func:
 * @context: stream context data
 *
 * Handler function for #raptor_iostream terminating.
 *
 */
typedef void (*raptor_iostream_finish_func) (void *context);

/**
 * raptor_iostream_write_byte_func
 * @context: stream context data
 * @byte: byte to write
 *
 * Handler function for implementing raptor_iostream_write_byte().
 *
 * Return value: non-0 on failure.
 */
typedef int (*raptor_iostream_write_byte_func) (void *context, const int byte);

/**
 * raptor_iostream_write_bytes_func:
 * @context: stream context data
 * @ptr: pointer to bytes to write
 * @size: size of item
 * @nmemb: number of items
 *
 * Handler function for implementing raptor_iostream_write_bytes().
 *
 * Return value: non-0 on failure.
 */
typedef int (*raptor_iostream_write_bytes_func) (void *context, const void *ptr, size_t size, size_t nmemb);

/**
 * raptor_iostream_write_end_func:
 * @context: stream context data
 *
 * Handler function for implementing raptor_iostream_write_end().
 *
 */
typedef void (*raptor_iostream_write_end_func) (void *context);

/**
 * raptor_iostream_handler:
 * @init:  initialisation handler - optional, called at most once
 * @finish: finishing handler -  optional, called at most once
 * @write_byte: write byte handler - required
 * @write_bytes: write bytes handler - required
 * @write_end: write end handler - optional, called at most once
 *
 * I/O stream implementation handler structure.
 * 
 */
typedef struct {
  raptor_iostream_init_func         init;
  raptor_iostream_finish_func       finish;
  raptor_iostream_write_byte_func   write_byte;
  raptor_iostream_write_bytes_func  write_bytes;
  raptor_iostream_write_end_func    write_end;
} raptor_iostream_handler;


RAPTOR_API
raptor_iostream* raptor_new_iostream_from_handler(void *context, raptor_iostream_handler *handler);
RAPTOR_API
raptor_iostream* raptor_new_iostream_to_sink(void);
RAPTOR_API
raptor_iostream* raptor_new_iostream_to_filename(const char *filename);
RAPTOR_API
raptor_iostream* raptor_new_iostream_to_file_handle(FILE *handle);
RAPTOR_API
raptor_iostream* raptor_new_iostream_to_string(void **string_p, size_t *length_p, void *(*malloc_handler)(size_t size));
RAPTOR_API
void raptor_free_iostream(raptor_iostream *iostr);
RAPTOR_API
int raptor_iostream_write_bytes(raptor_iostream *iostr, const void *ptr, size_t size, size_t nmemb);
RAPTOR_API
int raptor_iostream_write_byte(raptor_iostream *iostr, const int byte);
RAPTOR_API
void raptor_iostream_write_end(raptor_iostream *iostr);
RAPTOR_API
int raptor_iostream_write_string(raptor_iostream *iostr, const void *string);
RAPTOR_API
int raptor_iostream_write_counted_string(raptor_iostream *iostr, const void *string, size_t len);
RAPTOR_API
size_t raptor_iostream_get_bytes_written_count(raptor_iostream *iostr);
RAPTOR_API
int raptor_iostream_write_decimal(raptor_iostream* iostr, int integer);
RAPTOR_API
int raptor_iostream_format_hexadecimal(raptor_iostream* iostr, unsigned int integer, int width);
RAPTOR_API
int raptor_iostream_write_stringbuffer(raptor_iostream* iostr, raptor_stringbuffer *sb);

/* Parser and Serializer features */
RAPTOR_API
raptor_feature raptor_feature_from_uri(raptor_uri *uri);
RAPTOR_API
int raptor_feature_value_type(const raptor_feature feature);

/* SAX2 element Class (raptor_xml_element) */
RAPTOR_API
raptor_xml_element* raptor_new_xml_element(raptor_qname* name, const unsigned char* xml_language, raptor_uri* xml_base);
RAPTOR_API
void raptor_free_xml_element(raptor_xml_element *element);
RAPTOR_API
raptor_qname* raptor_xml_element_get_name(raptor_xml_element *xml_element);
RAPTOR_API
void raptor_xml_element_set_attributes(raptor_xml_element* xml_element, raptor_qname **attributes, int count);
RAPTOR_API
void raptor_xml_element_declare_namespace(raptor_xml_element* xml_element, raptor_namespace *nspace);
RAPTOR_API
int raptor_iostream_write_xml_element(raptor_iostream *iostr, raptor_xml_element *element, raptor_namespace_stack *nstack, int is_empty, int is_end, raptor_simple_message_handler error_handler, void *error_data, int depth);

/* XML Writer Class (raptor_xml_writer) */
RAPTOR_API
raptor_xml_writer* raptor_new_xml_writer(raptor_namespace_stack *nstack, raptor_uri_handler *uri_handler, void *uri_context, raptor_iostream* iostr, raptor_simple_message_handler error_handler, void *error_data, int canonicalize);
RAPTOR_API
void raptor_free_xml_writer(raptor_xml_writer* xml_writer);
RAPTOR_API
void raptor_xml_writer_empty_element(raptor_xml_writer* xml_writer, raptor_xml_element *element);
RAPTOR_API
void raptor_xml_writer_start_element(raptor_xml_writer* xml_writer, raptor_xml_element *element);
RAPTOR_API
void raptor_xml_writer_end_element(raptor_xml_writer* xml_writer, raptor_xml_element *element);
RAPTOR_API
void raptor_xml_writer_cdata(raptor_xml_writer* xml_writer, const unsigned char *s);
RAPTOR_API
void raptor_xml_writer_cdata_counted(raptor_xml_writer* xml_writer, const unsigned char *s, unsigned int len);
RAPTOR_API
void raptor_xml_writer_raw(raptor_xml_writer* xml_writer, const unsigned char *s);
RAPTOR_API
void raptor_xml_writer_raw_counted(raptor_xml_writer* xml_writer, const unsigned char *s, unsigned int len);
RAPTOR_API
void raptor_xml_writer_comment(raptor_xml_writer* xml_writer, const unsigned char *s);
RAPTOR_API
void raptor_xml_writer_comment_counted(raptor_xml_writer* xml_writer, const unsigned char *s, unsigned int len);
RAPTOR_API
int raptor_xml_writer_features_enumerate(const raptor_feature feature, const char **name,  raptor_uri **uri, const char **label);
RAPTOR_API
int raptor_xml_writer_set_feature(raptor_xml_writer *xml_writer, raptor_feature feature, int value);
RAPTOR_API
int raptor_xml_writer_set_feature_string(raptor_xml_writer *xml_writer, raptor_feature feature, const unsigned char *value);
RAPTOR_API
int raptor_xml_writer_get_feature(raptor_xml_writer *xml_writer, raptor_feature feature);
RAPTOR_API
const unsigned char *raptor_xml_writer_get_feature_string(raptor_xml_writer *xml_writer, raptor_feature feature);

#ifdef __cplusplus
}
#endif

#endif
