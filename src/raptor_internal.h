/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_internal.h - Redland Parser Toolkit for RDF (Raptor) internals
 *
 * $Id$
 *
 * Copyright (C) 2002-2003 David Beckett - http://purl.org/net/dajobe/
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

/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif
#include <dmalloc.h>
#endif


#ifdef LIBRDF_DEBUG
#define RAPTOR_DEBUG 1
#endif

#define RAPTOR_MALLOC(type, size) malloc(size)
#define RAPTOR_CALLOC(type, size, count) calloc(size, count)
#define RAPTOR_FREE(type, ptr)   free((void*)ptr)

#ifdef RAPTOR_DEBUG
/* Debugging messages */
#define RAPTOR_DEBUG1(function, msg) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function); } while(0)
#define RAPTOR_DEBUG2(function, msg, arg1) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1);} while(0)
#define RAPTOR_DEBUG3(function, msg, arg1, arg2) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2);} while(0)
#define RAPTOR_DEBUG4(function, msg, arg1, arg2, arg3) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2, arg3);} while(0)
#define RAPTOR_DEBUG5(function, msg, arg1, arg2, arg3, arg4) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2, arg3, arg4);} while(0)
#define RAPTOR_DEBUG6(function, msg, arg1, arg2, arg3, arg4, arg5) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2, arg3, arg4, arg5);} while(0)

#else
/* DEBUGGING TURNED OFF */

/* No debugging messages */
#define RAPTOR_DEBUG1(function, msg)
#define RAPTOR_DEBUG2(function, msg, arg1)
#define RAPTOR_DEBUG3(function, msg, arg1, arg2)
#define RAPTOR_DEBUG4(function, msg, arg1, arg2, arg3)
#define RAPTOR_DEBUG5(function, msg, arg1, arg2, arg3, arg4)
#define RAPTOR_DEBUG6(function, msg, arg1, arg2, arg3, arg4, arg5)

#endif


/* Fatal errors - always happen */
#define RAPTOR_FATAL1(function, msg) do {fprintf(stderr, "%s:%d:%s: fatal error: " msg, __FILE__, __LINE__ , #function); abort();} while(0)
#define RAPTOR_FATAL2(function, msg,arg) do {fprintf(stderr, "%s:%d:%s: fatal error: " msg, __FILE__, __LINE__ , #function, arg); abort();} while(0)



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

#include <libxml/parser.h>


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
typedef struct raptor_set_s raptor_set;


/* Raptor Namespace Stack node */
struct raptor_namespace_stack_s {
  raptor_namespace* top;
  raptor_uri_handler *uri_handler;
  void *uri_context;
  raptor_simple_message_handler error_handler;
  void *error_data;

  raptor_uri *rdf_ms_uri;
  raptor_uri *rdf_schema_uri;
};


/* Forms:
 * 1) prefix=NULL uri=<URI>      - default namespace defined
 * 2) prefix=NULL, uri=NULL      - no default namespace
 * 3) prefix=<prefix>, uri=<URI> - regular pair defined <prefix>:<URI>
 */
struct raptor_namespace_s {
  /* next down the stack, NULL at bottom */
  struct raptor_namespace_s* next;

  raptor_namespace_stack *nstack;

  /* NULL means is the default namespace */
  const unsigned char *prefix;
  /* needed to safely compare prefixed-names */
  int prefix_length;
  /* URI of namespace or NULL for default */
  raptor_uri *uri;
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

#ifdef RAPTOR_XML_LIBXML
#define RAPTOR_LIBXML_MAGIC 0x8AF108
#endif

/*
 * Raptor parser object
 */
struct raptor_parser_s {
#ifdef RAPTOR_XML_LIBXML
  int magic;
#endif

  /* can be filled with error location information */
  raptor_locator locator;

  /* non 0 if parser had fatal error and cannot continue */
  int failed;

  /* generated ID counter */
  int genid;

  /* base URI of RDF/XML */
  raptor_uri *base_uri;

  /* static statement for use in passing to user code */
  raptor_statement statement;

  /* FEATURE: 
   * non 0 if scanning for <rdf:RDF> element inside the XML,
   * else require rdf:RDF as first element. See also feature_assume_is_rdf
   */
  int feature_scanning_for_rdf_RDF;

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


  /* FEATURE: 
   * non 0 if rdf:bagID is allowed.
   * RDF/XML Syntax (Revised) 2003 forbids this.
   */
  int feature_allow_bagID;

  /* FEATURE: 
   * non 0 if rdf:Collection generates the <idList> rdf:type rdf:List triple.
   * post-last call WD of RDF/XML Syntax (Revised) 2003 forbids amended
   * the rules to not generate this.
   */
  int feature_allow_rdf_type_rdf_List;

  /* FEATURE: 
   * non 0 if normalizing xml:lang attribute values to lowercase.
   * http://www.w3.org/TR/rdf-concepts/#dfn-language-identifier
   * says this is done when language-tagged literals are in the graph.
   */
  int feature_normalize_language;

  /* FEATURE
   * non 0 if non-NFC literals cause a fatal error rather than
   * a warning.
   */
  int feature_non_nfc_fatal;

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

  void *generate_id_handler_user_data;
  raptor_generate_id_handler generate_id_handler;

  int default_generate_id_handler_base;
  char *default_generate_id_handler_prefix;
  size_t default_generate_id_handler_prefix_length;

  /* parser specific stuff */
  void *context;

  struct raptor_parser_factory_s* factory;
};


/** A Parser Factory for a syntax */
struct raptor_parser_factory_s {
  struct raptor_parser_factory_s* next;

  /* syntax name */
  const char* name;
  /* syntax readable label */
  const char* label;
  /* syntax MIME type (or NULL) */
  const char* mime_type;
  /* syntax URI (or NULL) */
  const unsigned char* uri_string;
  
  /* the rest of this structure is populated by the
     parser-specific register function */
  size_t context_length;
  
  /* create a new parser */
  int (*init)(raptor_parser* parser, const char *name);
  
  /* destroy a parser */
  void (*terminate)(raptor_parser* parser);
  
  /* start a parse */
  int (*start)(raptor_parser* parser);
  
  /* parse a chunk of memory */
  int (*chunk)(raptor_parser* parser, const unsigned char *buffer, size_t len, int is_end);

};




/* raptor_general.c */

extern const char *raptor_xml_literal_datatype_uri_string;

void raptor_parser_register_factory(const char *name, const char *label, const char *mime_type, const unsigned char *uri_string, void (*factory) (raptor_parser_factory*));

unsigned char* raptor_generate_id(raptor_parser *rdf_parser, const int id_for_bag, unsigned char* user_bnodeid);
const unsigned char* raptor_inscope_xml_language(raptor_parser *rdf_parser);
raptor_uri* raptor_inscope_base_uri(raptor_parser *rdf_parser);
#ifdef RAPTOR_DEBUG
void raptor_stats_print(raptor_parser *rdf_parser, FILE *stream);
#endif


/* raptor_parse.c */

extern void raptor_parser_fatal_error(raptor_parser* parser, const char *message, ...);
extern void raptor_parser_error(raptor_parser* parser, const char *message, ...);
extern void raptor_parser_simple_error(void* parser, const char *message, ...);
extern void raptor_parser_warning(raptor_parser* parser, const char *message, ...);
extern void raptor_parser_fatal_error_varargs(raptor_parser* parser, const char *message, va_list arguments);
extern void raptor_parser_error_varargs(raptor_parser* parser, const char *message, va_list arguments);
extern void raptor_parser_warning_varargs(raptor_parser* parser, const char *message, va_list arguments);


/* raptor_parse.c */

typedef struct raptor_xml_parser_s raptor_xml_parser;

/* Prototypes for common expat/libxml parsing event-handling functions */
extern void raptor_xml_start_element_handler(void *user_data, const unsigned char *name, const unsigned char **atts);
extern void raptor_xml_end_element_handler(void *user_data, const unsigned char *name);
/* s is not 0 terminated. */
extern void raptor_xml_characters_handler(void *user_data, const unsigned char *s, int len);
extern void raptor_xml_cdata_handler(void *user_data, const unsigned char *s, int len);
void raptor_xml_comment_handler(void *user_data, const unsigned char *s);

#ifdef RAPTOR_DEBUG
void raptor_xml_parser_stats_print(raptor_xml_parser* rdf_xml_parser, FILE *stream);
#endif

/* raptor_general.c */
extern void raptor_expat_update_document_locator (raptor_parser *rdf_parser);
extern int raptor_valid_xml_ID(raptor_parser *rdf_parser, const unsigned char *string);
char* raptor_vsnprintf(const char *message, va_list arguments);
void raptor_print_statement_part_as_ntriples(FILE* stream, const void *term, raptor_identifier_type type, raptor_uri* literal_datatype, const unsigned char *literal_language);
int raptor_check_ordinal(const unsigned char *name);

/* raptor_identifier.c */
#ifdef RAPTOR_DEBUG
void raptor_identifier_print(FILE *stream, raptor_identifier* identifier);
#endif
  
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

#ifdef RAPTOR_DEBUG
void raptor_namespace_print(FILE *stream, raptor_namespace* ns);
#endif


/* 
 * Raptor XML-namespace qualified name (qname), for elements or attributes 
 *
 * namespace is only defined when the XML name has a namespace and
 * only then is uri also given.
 */
struct raptor_qname_s {
  /* Name - always present */
  const unsigned char *local_name;
  int local_name_length;
  /* Namespace or NULL if not in a namespace */
  const raptor_namespace *nspace;
  /* URI of namespace+local_name or NULL if not defined */
  raptor_uri *uri;
  /* optional value - used when name is an attribute */
  const unsigned char *value;
  unsigned int value_length;
};



/* raptor_qname.c */
#ifdef RAPTOR_DEBUG
void raptor_qname_print(FILE *stream, raptor_qname* name);
#endif


/* raptor_uri.c */
void raptor_uri_init_default_handler(raptor_uri_handler *handler);

/* raptor_parse.c */
void raptor_init_parser_rdfxml(void);
void raptor_init_parser_ntriples(void);
void raptor_init_parser_n3(void);
void raptor_init_parser_rss(void);

void raptor_terminate_parser_rdfxml (void);


/* raptor_utf8.c */
int raptor_unicode_char_to_utf8(unsigned long c, unsigned char *output);
int raptor_utf8_to_unicode_char(unsigned long *output, const unsigned char *input, int length);
int raptor_unicode_is_namestartchar(long c);
int raptor_unicode_is_namechar(long c);
int raptor_utf8_is_nfc(const unsigned char *input, size_t length);

/* raptor_www*.c */
#ifdef RAPTOR_WWW_LIBXML
#include <libxml/parser.h>
#include <libxml/xmlerror.h>
#include <libxml/nanohttp.h>
#endif

#ifdef RAPTOR_WWW_LIBCURL
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#endif

/* Size of buffer used in various raptor_www places for I/O  */
#define RAPTOR_WWW_BUFFER_SIZE 256

/* WWW library state */
struct  raptor_www_s {
  char *type;
  int free_type;
  int total_bytes;
  int failed;
  int status_code;

  raptor_uri *uri;

#ifdef RAPTOR_WWW_LIBCURL
  CURL* curl_handle;
  CURLcode status;
  char error_buffer[CURL_ERROR_SIZE];
  int curl_init_here;
#endif

#ifdef RAPTOR_WWW_LIBXML
  void *ctxt;
  xmlGenericErrorFunc old_handler;
  char buffer[RAPTOR_WWW_BUFFER_SIZE];
  int is_end;
#endif

#ifdef RAPTOR_WWW_LIBFETCH
  char buffer[RAPTOR_WWW_BUFFER_SIZE];
#endif

  char *user_agent;

  /* proxy URL string or NULL for none */
  char *proxy;
  
  void *write_bytes_userdata;
  raptor_www_write_bytes_handler write_bytes;
  void *content_type_userdata;
  raptor_www_content_type_handler content_type;

  void *error_data;
  raptor_message_handler error_handler;

  /* can be filled with error location information */
  raptor_locator locator;
};



/* internal */
void raptor_www_libxml_init(raptor_www *www);
void raptor_www_libxml_free(raptor_www *www);
int raptor_www_libxml_fetch(raptor_www *www);

void raptor_www_error_varargs(raptor_www *www, const char *message, va_list arguments);
void raptor_www_error(raptor_www *www, const char *message, ...);

void raptor_www_curl_init(raptor_www *www);
void raptor_www_curl_free(raptor_www *www);
int raptor_www_curl_fetch(raptor_www *www);

void raptor_www_libwww_init(raptor_www *www);
void raptor_www_libwww_free(raptor_www *www);
int raptor_www_libwww_fetch(raptor_www *www);

void raptor_www_libfetch_init(raptor_www *www);
void raptor_www_libfetch_free(raptor_www *www);
int raptor_www_libfetch_fetch(raptor_www *www);

  /* raptor_set.c */
raptor_set* raptor_new_set(void);
int raptor_free_set(raptor_set* set);
int raptor_set_add(raptor_set* set, char *item, size_t item_len);
#ifdef RAPTOR_DEBUG
void raptor_set_stats_print(raptor_set* set, FILE *stream);
#endif

/* raptor_sax2.c */
typedef struct raptor_sax2_s raptor_sax2;
/*
 * SAX2 elements/attributes on stack 
 */
struct raptor_sax2_element_s {
  /* NULL at bottom of stack */
  struct raptor_sax2_element_s *parent;
  raptor_qname *name;
  raptor_qname **attributes;
  unsigned int attribute_count;

  /* value of xml:lang attribute on this element or NULL */
  const unsigned char *xml_language;

  /* URI of xml:base attribute value on this element or NULL */
  raptor_uri *base_uri;

  /* CDATA content of element and checks for mixed content */
  unsigned char *content_cdata;
  unsigned int content_cdata_length;
  /* how many cdata blocks seen */
  unsigned int content_cdata_seen;
  /* how many contained elements seen */
  unsigned int content_element_seen;
};


struct raptor_sax2_s {
#ifdef RAPTOR_XML_EXPAT
  XML_Parser xp;
#ifdef EXPAT_UTF8_BOM_CRASH
  int tokens_count; /* used to see if trying to get location info is safe */
#endif
#endif
#ifdef RAPTOR_XML_LIBXML
  /* structure holding sax event handlers */
  xmlSAXHandler sax;
  /* parser context */
  xmlParserCtxtPtr xc;
  /* pointer to SAX document locator */
  xmlSAXLocatorPtr loc;

#ifdef RAPTOR_LIBXML_MY_ENTITIES
  /* for xml entity resolution */
  raptor_xml_entity* entities;
#endif

#if LIBXML_VERSION < 20425
  /* flag for some libxml eversions*/
  int first_read;
#endif

#endif  

  /* element depth */
  int depth;

  /* stack of elements - elements add after current_element */
  raptor_sax2_element *root_element;
  raptor_sax2_element *current_element;
};

raptor_sax2_element* raptor_sax2_element_pop(raptor_sax2 *sax2);
void raptor_sax2_element_push(raptor_sax2 *sax2, raptor_sax2_element* element);
raptor_sax2_element* raptor_new_sax2_element(raptor_qname* name, const unsigned char* xml_language, raptor_uri* xml_base);
void raptor_free_sax2_element(raptor_sax2_element *element);
#ifdef RAPTOR_DEBUG
void raptor_print_sax2_element(raptor_sax2_element *element, FILE* stream);
#endif
unsigned char *raptor_format_sax2_element(raptor_sax2_element *element, raptor_namespace_stack* nstack, size_t *length_p, int is_end, raptor_simple_message_handler error_handler, void *error_data, int depth);

/* raptor_xml_writer.c */
/* FIXME: NOT PUBLIC YET - should be in raptor.h with RAPTOR_API added */
raptor_xml_writer* raptor_new_xml_writer(raptor_uri_handler *uri_handler, void *uri_context, raptor_simple_message_handler error_handler, void *error_data, int canonicalize);
void raptor_free_xml_writer(raptor_xml_writer* xml_writer);
void raptor_xml_writer_start_element(raptor_xml_writer* xml_writer, raptor_sax2_element *element);
void raptor_xml_writer_end_element(raptor_xml_writer* xml_writer, raptor_sax2_element *element);
void raptor_xml_writer_cdata(raptor_xml_writer* xml_writer, const unsigned char *str, unsigned int length);
void raptor_xml_writer_comment(raptor_xml_writer* xml_writer, const unsigned char *str, unsigned int length);
unsigned char* raptor_xml_writer_as_string(raptor_xml_writer* xml_writer, unsigned int *length_p);


/* n3_parser.y and n3_lexer.l */
typedef struct raptor_n3_parser_s raptor_n3_parser;

typedef struct {
  raptor_identifier *subject;
  raptor_identifier *predicate;
  raptor_identifier *object;
} raptor_triple;


/* Sequence class */
typedef struct raptor_sequence_s raptor_sequence;
typedef void* (raptor_free_handler(void*));
typedef void (raptor_print_handler(void *object, FILE *fh));

/* Create */
RAPTOR_API raptor_sequence* raptor_new_sequence(raptor_free_handler* free_handler, raptor_print_handler* print_handler);
/* Destroy */
RAPTOR_API void raptor_free_sequence(raptor_sequence* seq);
/* Methods */
RAPTOR_API int raptor_sequence_size(raptor_sequence* seq);
RAPTOR_API int raptor_sequence_set_at(raptor_sequence* seq, int idx, void *data);
RAPTOR_API int raptor_sequence_push(raptor_sequence* seq, void *data);
RAPTOR_API int raptor_sequence_shift(raptor_sequence* seq, void *data);
RAPTOR_API void* raptor_sequence_get_at(raptor_sequence* seq, int idx);
RAPTOR_API void* raptor_sequence_pop(raptor_sequence* seq);
RAPTOR_API void* raptor_sequence_unshift(raptor_sequence* seq);

RAPTOR_API int raptor_compare_strings(const void *a, const void *b);

RAPTOR_API void raptor_sequence_sort(raptor_sequence* seq, int(*compare)(const void *, const void *));

/* helper for printing sequences of strings */ 
RAPTOR_API void raptor_sequence_print_string(char *data, FILE *fh);
RAPTOR_API void raptor_sequence_print(raptor_sequence* seq, FILE* fh);

/* raptor_stringbuffer.c - NOT PUBLIC */
raptor_stringbuffer* raptor_new_stringbuffer(void);
void raptor_free_stringbuffer(raptor_stringbuffer *stringbuffer);
int raptor_stringbuffer_append_counted_string(raptor_stringbuffer* stringbuffer, const unsigned char *string, size_t length, int do_copy);
int raptor_stringbuffer_append_string(raptor_stringbuffer* stringbuffer, const unsigned char *string, int do_copy);
int raptor_stringbuffer_append_decimal(raptor_stringbuffer* stringbuffer, int integer);
unsigned char * raptor_stringbuffer_as_string(raptor_stringbuffer* stringbuffer);
size_t raptor_stringbuffer_length(raptor_stringbuffer* stringbuffer);

/* end of RAPTOR_INTERNAL */
#endif


#ifdef __cplusplus
}
#endif

#endif
