/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_v2.h - INTERNAL raptor experimental V2 functions
 *
 * Copyright (C) 2009, David Beckett http://www.dajobe.org/
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


#ifndef RAPTOR_V2_H
#define RAPTOR_V2_H

#undef RAPTOR_V2_AVAILABLE
#define RAPTOR_V2_AVAILABLE 1


/**
 * raptor_statement_v2:
 * @world: raptor_world object
 * @s: #raptor_statement
 *
 * An RDF triple wrapper for raptor v2 statement API.
 *
 * See #raptor_statement.
 */
typedef struct {
  raptor_world* world;
  raptor_statement *s;
} raptor_statement_v2;

RAPTOR_API
int raptor_parsers_enumerate_v2(raptor_world* world, const unsigned int counter, const char **name, const char **label);
RAPTOR_API
int raptor_syntaxes_enumerate_v2(raptor_world* world, const unsigned int counter, const char **name, const char **label, const char **mime_type, const unsigned char **uri_string);
RAPTOR_API
int raptor_syntax_name_check_v2(raptor_world* world, const char *name);
RAPTOR_API
const char* raptor_guess_parser_name_v2(raptor_world* world, raptor_uri *uri, const char *mime_type, const unsigned char *buffer, size_t len, const unsigned char *identifier);

RAPTOR_API
raptor_parser* raptor_new_parser_v2(raptor_world* world, const char *name);
RAPTOR_API
raptor_parser* raptor_new_parser_for_content_v2(raptor_world* world, raptor_uri *uri, const char *mime_type, const unsigned char *buffer, size_t len, const unsigned char *identifier);

RAPTOR_API
void raptor_print_statement_v2(const raptor_statement_v2 * statement, FILE *stream);
RAPTOR_API
void raptor_print_statement_as_ntriples_v2(const raptor_statement_v2 * statement, FILE *stream);

RAPTOR_API
unsigned char* raptor_statement_part_as_counted_string_v2(raptor_world* world, const void *term, raptor_identifier_type type, raptor_uri* literal_datatype, const unsigned char *literal_language, size_t* len_p);
RAPTOR_API
unsigned char* raptor_statement_part_as_string_v2(raptor_world* world, const void *term, raptor_identifier_type type, raptor_uri* literal_datatype, const unsigned char *literal_language);  
RAPTOR_API
int raptor_statement_compare_v2(const raptor_statement_v2 *s1, const raptor_statement_v2 *s2);

RAPTOR_API
void raptor_print_locator_v2(raptor_world* world, FILE *stream, raptor_locator* locator);
RAPTOR_API
int raptor_format_locator_v2(raptor_world* world, char *buffer, size_t length, raptor_locator* locator);

RAPTOR_API
const char * raptor_locator_uri_v2(raptor_world* world, raptor_locator *locator);

RAPTOR_API
int raptor_features_enumerate_v2(raptor_world* world, const raptor_feature feature, const char **name, raptor_uri **uri, const char **label);

RAPTOR_API
int raptor_serializers_enumerate_v2(raptor_world* world, const unsigned int counter, const char **name, const char **label, const char **mime_type, const unsigned char **uri_string);
RAPTOR_API
int raptor_serializer_syntax_name_check_v2(raptor_world* world, const char *name);

RAPTOR_API
raptor_serializer* raptor_new_serializer_v2(raptor_world* world, const char *name);

RAPTOR_API
int raptor_serializer_features_enumerate_v2(raptor_world* world, const raptor_feature feature, const char **name,  raptor_uri **uri, const char **label);

RAPTOR_API
raptor_uri* raptor_new_uri_v2(raptor_world* world, const unsigned char *uri_string);
RAPTOR_API
raptor_uri* raptor_new_uri_from_uri_local_name_v2(raptor_world* world, raptor_uri *uri, const unsigned char *local_name);
RAPTOR_API
raptor_uri* raptor_new_uri_relative_to_base_v2(raptor_world* world, raptor_uri *base_uri, const unsigned char *uri_string);
RAPTOR_API
raptor_uri* raptor_new_uri_from_id_v2(raptor_world* world, raptor_uri *base_uri, const unsigned char *id);
RAPTOR_API
raptor_uri* raptor_new_uri_for_rdf_concept_v2(raptor_world* world, const char *name);
RAPTOR_API
void raptor_free_uri_v2(raptor_world* world, raptor_uri *uri);
RAPTOR_API
int raptor_uri_equals_v2(raptor_world* world, raptor_uri* uri1, raptor_uri* uri2);
RAPTOR_API
int raptor_uri_compare_v2(raptor_world* world, raptor_uri* uri1, raptor_uri* uri2);
RAPTOR_API
raptor_uri* raptor_uri_copy_v2(raptor_world* world, raptor_uri *uri);
RAPTOR_API
unsigned char* raptor_uri_as_string_v2(raptor_world* world, raptor_uri *uri);
RAPTOR_API
unsigned char* raptor_uri_as_counted_string_v2(raptor_world* world, raptor_uri *uri, size_t* len_p);

RAPTOR_API
raptor_uri* raptor_new_uri_for_xmlbase_v2(raptor_world* world, raptor_uri* old_uri);

RAPTOR_API
raptor_uri* raptor_new_uri_for_retrieval_v2(raptor_world* world, raptor_uri* old_uri);

RAPTOR_API
raptor_identifier* raptor_new_identifier_v2(raptor_world* world, raptor_identifier_type type, raptor_uri *uri, raptor_uri_source uri_source, const unsigned char *id, const unsigned char *literal, raptor_uri *literal_datatype, const unsigned char *literal_language);

void raptor_iostream_write_statement_ntriples_v2(raptor_world* world, raptor_iostream* iostr, const raptor_statement *statement);

RAPTOR_API
unsigned char* raptor_uri_to_relative_counted_uri_string_v2(raptor_world* world, raptor_uri *base_uri, raptor_uri *reference_uri, size_t *length_p);
RAPTOR_API
unsigned char* raptor_uri_to_relative_uri_string_v2(raptor_world* world, raptor_uri *base_uri,  raptor_uri *reference_uri);

RAPTOR_API
void raptor_uri_print_v2(raptor_world* world, const raptor_uri* uri, FILE *stream);
RAPTOR_API
unsigned char* raptor_uri_to_counted_string_v2(raptor_world* world, raptor_uri *uri, size_t *len_p);
RAPTOR_API
unsigned char* raptor_uri_to_string_v2(raptor_world* world, raptor_uri *uri);

RAPTOR_API
void raptor_uri_set_handler_v2(raptor_world* world, const raptor_uri_handler *handler, void *context);
RAPTOR_API
void raptor_uri_get_handler_v2(raptor_world* world, const raptor_uri_handler **handler, void **context);

RAPTOR_API
int raptor_www_init_v2(raptor_world* world);
RAPTOR_API
void raptor_www_finish_v2(raptor_world* world);
RAPTOR_API
void raptor_www_no_www_library_init_finish_v2(raptor_world* world);

RAPTOR_API
raptor_www *raptor_www_new_v2(raptor_world* world);
RAPTOR_API
raptor_www *raptor_www_new_with_connection_v2(raptor_world* world, void* connection);

RAPTOR_API
raptor_qname* raptor_new_qname_from_namespace_local_name_v2(raptor_world* world, raptor_namespace *ns, const unsigned char *local_name, const unsigned char *value);

RAPTOR_API
raptor_namespace_stack* raptor_new_namespaces_v2(raptor_world* world, raptor_simple_message_handler error_handler, void *error_data, int defaults);
RAPTOR_API
int raptor_namespaces_init_v2(raptor_world* world, raptor_namespace_stack *nstack, raptor_simple_message_handler error_handler, void *error_data, int defaults);

/**
 * raptor_sequence_free_handler_v2:
 * @context: context data for the free handler
 * @object: object to free
 *
 * Handler function for freeing a sequence item.
 *
 * Set by raptor_new_sequence_v2().
*/
typedef void (raptor_sequence_free_handler_v2(void* context, void* object));

/**
 * raptor_sequence_print_handler_v2:
 * @context: context data for the print handler
 * @object: object to print
 * @fh: FILE* to print to
 *
 * Handler function for printing a sequence item.
 *
 * Set by raptor_new_sequence_v2() or raptor_sequence_set_print_handler_v2().
 */
typedef void (raptor_sequence_print_handler_v2(void *context, void *object, FILE *fh));

RAPTOR_API
raptor_sequence* raptor_new_sequence_v2(raptor_sequence_free_handler_v2* free_handler, raptor_sequence_print_handler_v2* print_handler, void* handler_context);

RAPTOR_API
void raptor_sequence_set_print_handler_v2(raptor_sequence *seq, raptor_sequence_print_handler_v2 *print_handler);

RAPTOR_API
int raptor_iostream_write_uri_v2(raptor_world* world, raptor_iostream *iostr,  raptor_uri *uri);

RAPTOR_API
raptor_feature raptor_feature_from_uri_v2(raptor_world* world, raptor_uri *uri);

RAPTOR_API
raptor_xml_writer* raptor_new_xml_writer_v2(raptor_world* world, raptor_namespace_stack *nstack, raptor_iostream* iostr, raptor_simple_message_handler error_handler, void *error_data, int canonicalize);

RAPTOR_API
int raptor_xml_writer_features_enumerate_v2(raptor_world* world, const raptor_feature feature, const char **name,  raptor_uri **uri, const char **label);

RAPTOR_API
void raptor_error_handlers_init_v2(raptor_world* world, raptor_error_handlers* error_handlers);

#endif
