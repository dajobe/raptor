/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_internal.h - Redland Parser Toolkit for RDF (Raptor) internals
 *
 * $Id$
 *
 * Copyright (C) 2002 David Beckett - http://purl.org/net/dajobe/
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



#ifndef RAPTOR_INTERNAL_H
#define RAPTOR_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RAPTOR_INTERNAL

#ifdef RAPTOR_IN_REDLAND
#define LIBRDF_INTERNAL
#endif
  
#ifdef LIBRDF_INTERNAL
#ifdef LIBRDF_DEBUG
#define RAPTOR_DEBUG 1
#endif

#define IS_RDF_MS_CONCEPT(name, uri, local_name) librdf_uri_equals(uri, librdf_concept_uris[LIBRDF_CONCEPT_MS_##local_name])
#define RAPTOR_URI_AS_STRING(uri) (librdf_uri_as_string(uri))
#define RAPTOR_URI_TO_FILENAME(uri) (librdf_uri_to_filename(uri))
#define RAPTOR_FREE_URI(uri) librdf_free_uri(uri)
#else
/* else standalone */

#define LIBRDF_MALLOC(type, size) malloc(size)
#define LIBRDF_CALLOC(type, size, count) calloc(size, count)
#define LIBRDF_FREE(type, ptr)   free((void*)ptr)

#define IS_RDF_MS_CONCEPT(name, uri, local_name) !strcmp(name, #local_name)
#define RAPTOR_URI_AS_STRING(uri) ((const char*)uri)
#define RAPTOR_URI_TO_FILENAME(uri) (raptor_uri_uri_string_to_filename(uri))
#define RAPTOR_FREE_URI(uri) LIBRDF_FREE(cstring, uri)

#ifdef RAPTOR_DEBUG
/* Debugging messages */
#define LIBRDF_DEBUG1(function, msg) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function); } while(0)
#define LIBRDF_DEBUG2(function, msg, arg1) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1);} while(0)
#define LIBRDF_DEBUG3(function, msg, arg1, arg2) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2);} while(0)
#define LIBRDF_DEBUG4(function, msg, arg1, arg2, arg3) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2, arg3);} while(0)
#define LIBRDF_DEBUG4(function, msg, arg1, arg2, arg3) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2, arg3);} while(0)

#else
/* DEBUGGING TURNED OFF */

/* No debugging messages */
#define LIBRDF_DEBUG1(function, msg)
#define LIBRDF_DEBUG2(function, msg, arg1)
#define LIBRDF_DEBUG3(function, msg, arg1, arg2)
#define LIBRDF_DEBUG4(function, msg, arg1, arg2, arg3)

#endif


/* Fatal errors - always happen */
#define LIBRDF_FATAL1(function, msg) do {fprintf(stderr, "%s:%d:%s: fatal error: " msg, __FILE__, __LINE__ , #function); abort();} while(0)
#define LIBRDF_FATAL2(function, msg,arg) do {fprintf(stderr, "%s:%d:%s: fatal error: " msg, __FILE__, __LINE__ , #function, arg); abort();} while(0)

#endif


/* XML parser includes */
#ifdef RAPTOR_XML_EXPAT
#ifdef HAVE_EXPAT_H
#include <expat.h>
#endif
#ifdef HAVE_XMLPARSE_H
#include <xmlparse.h>
#endif
#endif


#ifdef RAPTOR_XML_LIBXML

#ifdef HAVE_LIBXML_PARSER_H
#include <libxml/parser.h>
#else
#ifdef HAVE_GNOME_XML_PARSER_H
#include <gnome-xml/parser.h>
#else
DIE
#endif
#endif

#define XML_Char xmlChar


/*
 * Raptor entity expansion list
 * (libxml only)
 */
#ifdef RAPTOR_LIBXML_MY_ENTITIES

struct raptor_xml_entity_t {
  xmlEntity entity;
#ifndef RAPTOR_LIBXML_ENTITY_NAME_LENGTH
  int name_length;
#endif

  struct raptor_xml_entity_t *next;
};
typedef struct raptor_xml_entity_t raptor_xml_entity;
#ifdef RAPTOR_LIBXML_ENTITY_NAME_LENGTH
#define RAPTOR_ENTITY_NAME_LENGTH(ent) ent->entity.name_length
#else
#define RAPTOR_ENTITY_NAME_LENGTH(ent) ent->name_length
#endif

#endif


/* libxml-only prototypes */


/* raptor_libxml.c exports */
extern void raptor_libxml_init(xmlSAXHandler *sax);
#ifdef RAPTOR_LIBXML_MY_ENTITIES
extern void raptor_libxml_free_entities(raptor_parser *rdf_parser);
#endif
extern void raptor_libxml_validation_error(void *context, const char *msg, ...);
extern void raptor_libxml_validation_warning(void *context, const char *msg, ...);

/* raptor_parse.c - exported to libxml part */
extern void raptor_libxml_update_document_locator (raptor_parser *rdf_parser);

extern xmlParserCtxtPtr raptor_get_libxml_context(raptor_parser *rdf_parser);
extern void raptor_set_libxml_document_locator(raptor_parser *rdf_parser, xmlSAXLocatorPtr loc);
extern xmlSAXLocatorPtr raptor_get_libxml_document_locator(raptor_parser *rdf_parser);

#ifdef RAPTOR_LIBXML_MY_ENTITIES
extern raptor_xml_entity* raptor_get_libxml_entities(raptor_parser *rdf_parser);
extern void raptor_set_libxml_entities(raptor_parser *rdf_parser, raptor_xml_entity* entities);
#endif
/* end of libxml-only */
#endif



typedef struct raptor_parser_factory_s raptor_parser_factory;
typedef struct raptor_namespace_s raptor_namespace;



/* Raptor Namespace Stack node */
typedef struct {
  raptor_namespace* top;
#ifdef RAPTOR_IN_REDLAND
  librdf_world* world;
#endif
} raptor_namespace_stack;


/* Forms:
 * 1) prefix=NULL uri=<URI>      - default namespace defined
 * 2) prefix=NULL, uri=NULL      - no default namespace
 * 3) prefix=<prefix>, uri=<URI> - regular pair defined <prefix>:<URI>
 */
struct raptor_namespace_s {
  /* next down the stack, NULL at bottom */
  struct raptor_namespace_s* next;
  /* NULL means is the default namespace */
  const char *prefix;
  /* needed to safely compare prefixed-names */
  int prefix_length;
  /* URI of namespace or NULL for default */
  raptor_uri *uri;
#ifdef RAPTOR_IN_REDLAND
#else
  /* When implementing URIs as char*, need this for efficiency */
  int uri_length;
#endif
  /* parsing depth that this ns was added.  It will
   * be deleted when the parser leaves this depth 
   */
  int depth;
  /* Non 0 if is xml: prefixed name */
  int is_xml;
  /* Non 0 if is RDF M&S Namespace */
  int is_rdf_ms;
  /* Non 0 if is RDF Schema Namespace */
  int is_rdf_schema;
};


/*
 * Raptor parser object
 */
struct raptor_parser_s {

  /* FIXME temporary redland stuff */
  void *world;
  
#ifdef RAPTOR_IN_REDLAND
  /* DAML collection URIs */
  librdf_uri *raptor_daml_oil_uri;
  librdf_uri *raptor_daml_List_uri;
  librdf_uri *raptor_daml_first_uri;
  librdf_uri *raptor_daml_rest_uri;
  librdf_uri *raptor_daml_nil_uri;
#endif

  /* stack of namespaces, most recently added at top */
  raptor_namespace_stack namespaces;

  /* can be filled with error location information */
  raptor_locator locator;

  /* non 0 if parser had fatal error and cannot continue */
  int failed;

  /* generated ID counter */
  int genid;

  /* base URI of RDF/XML */
  raptor_uri *base_uri;

  /* Reading from this file */
  FILE *fh;

  /* static statement for use in passing to user code */
  raptor_statement statement;

  /* FEATURE: 
   * non 0 if scanning for <rdf:RDF> element inside the XML,
   * else require rdf:RDF as first element. See also feature_assume_is_rdf
   */
  int feature_scanning_for_rdf_RDF;

  /* FEATURE: 
   * non 0 if assume document is rdf/xml, thus rdf:RDF is optional
   * but still allowed.  See also feature_scanning_for_rdf:RDF.
   */
  int feature_assume_is_rdf;

  /* FEATURE:
   * non 0 to allow non-namespaced resource, ID etc attributes
   * on RDF namespaced-elements
   */
  int feature_allow_non_ns_attributes;


  /* FEATURE:
   * non 0 to handle other rdf:parseType values that are not
   * literal or resource
   */
  int feature_allow_other_parseTypes;


  /* stuff for our user */
  void *user_data;

  void *fatal_error_user_data;
  void *error_user_data;
  void *warning_user_data;

  raptor_message_handler fatal_error_handler;
  raptor_message_handler error_handler;
  raptor_message_handler warning_handler;

  raptor_container_test_handler container_test_handler;

  /* parser callbacks */
  raptor_statement_handler statement_handler;


  /* parser specific stuff */
  void *context;

  struct raptor_parser_factory_s* factory;
};


/** A Storage Factory */
struct raptor_parser_factory_s {
  struct raptor_parser_factory_s* next;
  char* name;
  
  /* the rest of this structure is populated by the
     parser-specific register function */
  size_t context_length;
  
  /* create a new parser */
  int (*init)(raptor_parser* parser, const char *name);
  
  /* destroy a parser */
  void (*terminate)(raptor_parser* parser);
  
  /* start a parse */
  int (*start)(raptor_parser* parser, raptor_uri *uri);
  
  /* parse a chunk of memory */
  int (*chunk)(raptor_parser* parser, const char *buffer, size_t len, int is_end);

};




/* raptor_general.c */

void raptor_parser_register_factory(const char *name, void (*factory) (raptor_parser_factory*));

const char* raptor_generate_id(raptor_parser *rdf_parser, const int id_for_bag);
const char* raptor_inscope_xml_language(raptor_parser *rdf_parser);
raptor_uri* raptor_inscope_base_uri(raptor_parser *rdf_parser);


/* raptor_parse.c */

typedef void (*raptor_internal_message_handler)(raptor_parser* parser, const char *message, ...);

extern void raptor_parser_fatal_error(raptor_parser* parser, const char *message, ...);
extern void raptor_parser_error(raptor_parser* parser, const char *message, ...);
extern void raptor_parser_warning(raptor_parser* parser, const char *message, ...);
extern void raptor_parser_fatal_error_varargs(raptor_parser* parser, const char *message, va_list arguments);
extern void raptor_parser_error_varargs(raptor_parser* parser, const char *message, va_list arguments);
extern void raptor_parser_warning_varargs(raptor_parser* parser, const char *message, va_list arguments);


/* raptor_parse.c */

#ifdef RAPTOR_IN_REDLAND
raptor_parser* raptor_xml_new(raptor_parser *rdf_parser, librdf_world *world);
#else
raptor_parser* raptor_xml_new(raptor_parser *rdf_parser);
#endif
void raptor_xml_free(raptor_parser *rdf_parser);

/* Prototypes for common expat/libxml parsing event-handling functions */
extern void raptor_xml_start_element_handler(void *user_data, const XML_Char *name, const XML_Char **atts);
extern void raptor_xml_end_element_handler(void *user_data, const XML_Char *name);
/* s is not 0 terminated. */
extern void raptor_xml_cdata_handler(void *user_data, const XML_Char *s, int len);

/* raptor_general.c */
extern void raptor_expat_update_document_locator (raptor_parser *rdf_parser);


/* raptor_locator.c */
extern void raptor_update_document_locator (raptor_parser *rdf_parser);


#ifdef HAVE_STRCASECMP
#define raptor_strcasecmp strcasecmp
#define raptor_strncasecmp strncasecmp
#else
#ifdef HAVE_STRICMP
#define raptor_strcasecmp stricmp
#define raptor_strncasecmp strnicmp
#endif
#endif


/* raptor_nspace.c */
#ifdef RAPTOR_IN_REDLAND
void raptor_namespaces_init(raptor_namespace_stack *nstack, librdf_world *world);
#else
void raptor_namespaces_init(raptor_namespace_stack *nstack);
#endif
void raptor_namespaces_free(raptor_namespace_stack *nstack);
int raptor_namespaces_start_namespace(raptor_namespace_stack *nstack, const char *prefix, const char *nspace, int depth);
void raptor_namespaces_end_namespace(raptor_namespace_stack *nstack);
void raptor_namespaces_end_for_depth(raptor_namespace_stack *nstack, int depth);
raptor_namespace* raptor_namespaces_get_default_namespace(raptor_namespace_stack *nstack);
raptor_namespace *raptor_namespaces_find_namespace(raptor_namespace_stack *nstack, const char *prefix, int prefix_length);

#ifdef RAPTOR_IN_REDLAND
raptor_namespace* raptor_namespace_new(const char *prefix, const char *ns_uri_string, int depth, librdf_world *world);
#else
raptor_namespace* raptor_namespace_new(const char *prefix, const char *ns_uri_string, int depth);
#endif
void raptor_namespace_free(raptor_namespace *ns);
raptor_uri* raptor_namespace_get_uri(const raptor_namespace *ns);
const char* raptor_namespace_get_prefix(const raptor_namespace *ns);
raptor_uri* raptor_namespace_local_name_to_uri(const raptor_namespace *ns, const char *local_name);

#ifdef RAPTOR_DEBUG
void raptor_namespace_print(FILE *stream, raptor_namespace* ns);
#endif


/* 
 * Raptor XML-namespace qualified name (qname), for elements or attributes 
 *
 * namespace is only defined when the XML name has a namespace and
 * only then is uri also given.
 */
typedef struct {
  /* Name - always present */
  const char *local_name;
  int local_name_length;
  /* Namespace or NULL if not in a namespace */
  const raptor_namespace *nspace;
  /* URI of namespace+local_name or NULL if not defined */
  raptor_uri *uri;
  /* optional value - used when name is an attribute */
  const char *value;
  int value_length;
} raptor_qname;



/* raptor_qname.c */
raptor_qname* raptor_new_qname(raptor_namespace_stack *nstack, const char *name, const char *value, raptor_internal_message_handler error_handler, void *error_data);
#ifdef RAPTOR_DEBUG
void raptor_qname_print(FILE *stream, raptor_qname* name);
#endif
void raptor_free_qname(raptor_qname* name);
int raptor_qname_equal(raptor_qname *name1, raptor_qname *name2);


/* raptor_uri.c */
void raptor_uri_init_default_handler(raptor_uri_handler *handler);


void raptor_init_parser_rdfxml(void);
void raptor_init_parser_ntriples(void);
void raptor_uri_init(void);

/* end of RAPTOR_INTERNAL */
#endif


#ifdef __cplusplus
}
#endif

#endif
