/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_parse.c - Redland Parser for RDF (RAPTOR)
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


#ifdef HAVE_CONFIG_H
#ifdef LIBRDF_INTERNAL
#include <rdf_config.h>
#else
#include <config.h>
#endif
#endif

#ifdef WIN32
#define WIN32_LEAD_AND_MEAN 1
#define HAVE_STDLIB_H 1
#define HAVE_STDARG_H 1

/* use expat on win32 */
#define NEED_EXPAT 1
#define HAVE_XMLPARSE_H 1

#include <windows.h>

#define strcasecmp(X,Y) strcmpi(X,Y)

/* dll entry point */
BOOL APIENTRY
DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
  return TRUE;
}
#endif


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifndef WIN32
extern int errno;
#endif

#ifdef LIBRDF_INTERNAL
/* if inside Redland */

#ifdef LIBRDF_DEBUG
#define RAPTOR_DEBUG 1
#endif

#include <librdf.h>

#include <rdf_parser.h>
#include <rdf_node.h>
#include <rdf_stream.h>
#include <rdf_statement.h>
#include <rdf_uri.h>

#else
/* else standalone */

#define LIBRDF_MALLOC(type, size) malloc(size)
#define LIBRDF_CALLOC(type, size, count) calloc(size, count)
#define LIBRDF_FREE(type, ptr)   free((void*)ptr)

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


#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif

/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#undef HAVE_STDLIB_H
#endif


/* XML parser includes */
#ifdef NEED_EXPAT
#ifdef HAVE_EXPAT_H
#include <expat.h>
#endif
#ifdef HAVE_XMLPARSE_H
#include <xmlparse.h>
#endif
#endif

#ifdef NEED_LIBXML

#ifdef HAVE_PARSER_H
#include <parser.h>
#else
#ifdef HAVE_GNOME_XML_PARSER_H
#include <gnome-xml/parser.h>
#else
DIE
#endif

#endif

/* translate names from expat to libxml */
#define XML_Char xmlChar
#endif


/* Raptor includes */
#include "raptor.h"

/* Raptor structures */
/* namespace stack node */
typedef struct raptor_ns_map_s raptor_ns_map;

typedef enum {
  RAPTOR_STATE_INVALID = 0,

  /* Skipping current tree of elements - used to recover finding
   * illegal content, when parsling permissively.
   */
  RAPTOR_STATE_SKIPPING = 1,

  /* Not in RDF grammar yet - searching for a start element.
   * This can be <rdf:RDF> (goto 6.1) but since it is optional,
   * the start element can also be <Description> (goto 6.3), 
   * <rdf:Seq> (goto 6.25) <rdf:Bag> (goto 6.26) or <rdf:Alt> (goto 6.27)
   * OR from 6.3 can have ANY other element matching
   * typedNode (6.13) - goto 6.3
   * CHOICE: Search for <rdf:RDF> node before starting match
   * OR assume RDF content, hence go straight to production
   */
  RAPTOR_STATE_UNKNOWN = 1000,

  /* No need for 6.1 - go straight to 6.2 */

  /* Met production 6.1 (RDF) <rdf:RDF> element or 6.17 (value) and expecting
   *   description (6.3) | container (6.4)
   * = description (6.3) | sequence (6.25) | bag (6.26) | alternative (6.27)
   */
  RAPTOR_STATE_OBJ   = 6020,

  /* Met production 6.3 (description) <rdf:Description> element
   * OR 6.13 (typedNode)
   */
  RAPTOR_STATE_DESCRIPTION = 6030,

  /* production 6.4 (container) - not used (pick one of sequence, bag,
   * alternative immediately in state 6.2
   */
  
  /* productions 6.5-6.11 are for attributes - not used */

  /* met production 6.12 (propertyElt)
   */
  RAPTOR_STATE_PROPERTYELT = 6120,

  /* met production 6.13 (typedNode)
   */
  RAPTOR_STATE_TYPED_NODE = 6130,


  /* productions 6.14-6.16 are for attributes - not used */

  /* production 6.17 (value) - not used */

  /* productions 6.18-6.24 are for attributes / values - not used */


  /* Met production 6.25 (sequence) <rdf:Seq> element seen. Goto 6.28  */
  /* RAPTOR_STATE_SEQ = 6250, */

  /* Met production 6.26 (bag) <rdf:Bag> element seen. Goto 6.28  */
  /* RAPTOR_STATE_BAG = 6260, */

  /* Met production 6.27 (alternative) <rdf:Alt> element seen. Goto 6.28 */
  /* RAPTOR_STATE_ALT = 6270, */

  /* Met production 6.28 (member) 
   * Now expect <rdf:li> element and if it empty, with resource attribute
   * goto 6.29 otherwise goto 6.30
   * CHOICE: Match rdf:resource/resource
   */
  RAPTOR_STATE_MEMBER = 6280,

  /* met production 6.29 (referencedItem) after 
   * Found a container item with reference - <rdf:li (rdf:)resource=".."/> */
  /* RAPTOR_STATE_REFERENCEDITEM = 6290, */

  /* met production 6.30 (inlineItem) part 1 - plain container item */
  /* RAPTOR_STATE_INLINEITEM = 6300, */


  /* productions 6.31-6.33 are for attributes - not used */


  /* met production 6.30 (inlineItem) part 2 - container item with
   * rdf:parseType="literal" */
  RAPTOR_STATE_PARSETYPE_LITERAL = 6400,

  /* met production 6.30 (inlineItem) part 3 - container item with 
   * rdf:parseType="literal" */
  RAPTOR_STATE_PARSETYPE_RESOURCE = 6410,




  /* ******************************************************************* */
  /* Additional non-M&S states */

  /* met production 6.30 (inlineItem) - container item 
   * with other rdf:parseType value */
  RAPTOR_STATE_PARSETYPE_OTHER = 6420,

  /* met production 6.30 (inlineItem) - container item 
   * with rdf:parseType value "daml:collection" */
  RAPTOR_STATE_PARSETYPE_DAML_COLLECTION = 6430,

} raptor_state;


static const char * const raptor_state_names[]={
  NULL, /* No 6.0 */
  NULL, /* 6.1 not used */
  "object (6.2)",
  "description (6.3)",
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, /* 6.4 - 6.11 not used */
  "propertyElt (6.12)",
  "typedNode (6.13)",
  NULL,NULL,NULL, NULL, NULL,NULL,NULL,NULL,NULL,NULL,NULL, /* 6.14 - 6.24 not used */
  NULL, /* was sequence (6.25) */
  NULL, /* was bag (6.26) */
  NULL, /* was alternative (6.27) */
  "member (6.28)",
  NULL, /* was referencedItem (6.29) */
  NULL, /* was inlineItem (6.30 part 1) */
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  "parseTypeLiteral (6.30 part 2)",
  "parseTypeResource (6.30 part 3)",
  "parseTypeOther (not M&S)", /* 6.43 */
  "parseTypeDamlCollection (not M&S)", /* 6.44 */
};


static const char * raptor_state_as_string(raptor_state state) 
{
  int offset=(state - 6000)/10;
  if(state == RAPTOR_STATE_UNKNOWN)
    return "UNKNOWN";
  if(state == RAPTOR_STATE_SKIPPING)
    return "SKIPPING";
  if(offset<0 || offset > 43)
    return "INVALID";
  if(!raptor_state_names[offset])
    return "NOT-USED";
  return raptor_state_names[offset];
}


/* Forms:
 * 1) prefix=NULL uri=<URI>      - default namespace defined
 * 2) prefix=NULL, uri=NULL      - no default namespace
 * 3) prefix=<prefix>, uri=<URI> - regular pair defined <prefix>:<URI>
 */
struct raptor_ns_map_s {
  /* next down the stack, NULL at bottom */
  struct raptor_ns_map_s* next;
  /* NULL means is the default namespace */
  const char *prefix;
  /* needed to safely compare prefixed-names */
  int prefix_length;
  /* URI of namespace or NULL for default */
  raptor_uri *uri;
#ifdef LIBRDF_INTERNAL
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
 * Raptor XML-namespaced name, for elements or attributes 
 */

/* There are three forms
 * namespace=NULL                                - un-namespaced name
 * namespace=defined, namespace->prefix=NULL     - (default ns) name
 * namespace=defined, namespace->prefix=defined  - ns:name
 */
typedef struct {
  /* Name - always present */
  const char *local_name;
  int local_name_length;
  /* Namespace or NULL if not in a namespace */
  const raptor_ns_map *nspace;
  /* URI of namespace+local_name or NULL if not defined */
  raptor_uri *uri;
  /* optional value - used when name is an attribute */
  const char *value;
  int value_length;
} raptor_ns_name;


/* These are used in the RDF/XML syntax as attributes, not
 * elements and are mostly not concepts in the RDF model (except for
 * the type and value attributes which are properties too).
 */
typedef enum {
  RDF_ATTR_about           = 0, /* value of rdf:about attribute */
  RDF_ATTR_aboutEach       = 1, /* " rdf:aboutEach */
  RDF_ATTR_aboutEachPrefix = 2, /* " rdf:aboutEachPrefix */
  RDF_ATTR_ID              = 3, /* " rdf:ID */
  RDF_ATTR_bagID           = 4, /* " rdf:bagID */
  RDF_ATTR_resource        = 5, /* " rdf:resource */
  RDF_ATTR_parseType       = 6, /* " rdf:parseType */
  RDF_ATTR_type            = 7, /* " rdf:type -- a property in RDF Model */
  RDF_ATTR_value           = 8, /* " rdf:value -- a property in RDF model */

  RDF_ATTR_LAST            = RDF_ATTR_value
} rdf_attr;


/* Information about rdf attributes
 * type: Set when the attribute is a property rather than just syntax
 *   NOTE: raptor_process_property_attributes() expects only 
 *     RAPTOR_IDENTIFIER_TYPE_NONE,
 *     RAPTOR_IDENTIFIER_TYPE_LITERAL or RAPTOR_IDENTIFIER_TYPE_RESOURCE
 *      
 * name: name of property
 */

static const struct { 
  const char * const name;            /* attribute name */
  const raptor_identifier_type type;  /* statement value */
} rdf_attr_info[]={
  { "about",           RAPTOR_IDENTIFIER_TYPE_UNKNOWN  },
  { "aboutEach",       RAPTOR_IDENTIFIER_TYPE_UNKNOWN  },
  { "aboutEachPrefix", RAPTOR_IDENTIFIER_TYPE_UNKNOWN  },
  { "ID",              RAPTOR_IDENTIFIER_TYPE_UNKNOWN  },
  { "bagID",           RAPTOR_IDENTIFIER_TYPE_UNKNOWN  },
  { "resource",        RAPTOR_IDENTIFIER_TYPE_UNKNOWN  },
  { "parseType",       RAPTOR_IDENTIFIER_TYPE_UNKNOWN  },
  { "type",            RAPTOR_IDENTIFIER_TYPE_RESOURCE },
  { "value",           RAPTOR_IDENTIFIER_TYPE_LITERAL  }
};


typedef enum {
  /* undetermined yet - whitespace is stored */
  RAPTOR_ELEMENT_CONTENT_TYPE_UNKNOWN,

  /* literal content - no elements, cdata allowed, whitespace significant 
   * <propElement> blah </propElement>
   */
  RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL,

  /* parseType literal content (WF XML) - all content preserved
   * <propElement rdf:parseType="Literal"><em>blah</em></propElement> 
   */
  RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL,

  /* top-level nodes - 0+ elements expected, no cdata, whitespace ignored,
   * any non-whitespace cdata is error
   * only used for <rdf:RDF> or implict <rdf:RDF>
   */
  RAPTOR_ELEMENT_CONTENT_TYPE_NODES,

  /* properties - 0+ elements expected, no cdata, whitespace ignored,
   * any non-whitespace cdata is error
   * <nodeElement><prop1>blah</prop1> <prop2>blah</prop2> </nodeElement>
   */
  RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTIES,

  /* resource URI given - no element, no cdata, whitespace ignored,
   * any non-whitespace cdata is error 
   * <propElement rdf:resource="uri"/>
   * <propElement rdf:resource="uri"></propElement>
   */
  RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE,

  /* skipping content - all content is preserved 
   * Used when skipping content for unknown parseType-s,
   * error recovery, some other reason
   */
  RAPTOR_ELEMENT_CONTENT_TYPE_PRESERVED,

  /* parseType daml:collection - all content preserved
   * Parsing of this determined by DAML collection rules
   * <propElement rdf:parseType="daml:collection">...</propElement>
   */
  RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION,

  /* dummy for use in strings below */
  RAPTOR_ELEMENT_CONTENT_TYPE_LAST,
} raptor_element_content_type;


static const struct {
  const char * const name;
  const int whitespace_significant;
  /* non-blank cdata */
  const int cdata_allowed;
  /* XML element content */
  const int element_allowed;
  /* Do RDF-specific processing? (property attributes, rdf: attributes, ...) */
  const int rdf_processing;
} rdf_content_type_info[]={
  {"Unknown",         1, 1, 1, 0 },
  {"Literal",         1, 1, 0, 0 },
  {"XML Literal",     1, 1, 1, 0 },
  {"Nodes",           0, 0, 1, 1 },
  {"Properties",      0, 1, 1, 1 },
  {"Resource",        0, 0, 0, 0 },
  {"Preserved",       1, 1, 1, 0 },
  {"DAML Collection", 1, 1, 1, 1 },
};



static const char * raptor_element_content_type_as_string(raptor_element_content_type type) 
{
  if(type<0 || type > RAPTOR_ELEMENT_CONTENT_TYPE_LAST)
    return "INVALID";
  return rdf_content_type_info[type].name;
}



/*
 * Raptor Element/attributes on stack 
 */
struct raptor_element_s {
  /* NULL at bottom of stack */
  struct raptor_element_s *parent;
  raptor_ns_name *name;
  raptor_ns_name **attributes;
  int attribute_count;
  /* attributes declared in M&S */
  const char * rdf_attr[RDF_ATTR_LAST+1];
  /* how many of above seen */
  int rdf_attr_count;

  /* state that this production matches */
  raptor_state state;

  /* how to handle the content inside this XML element */
  raptor_element_content_type content_type;


  /* starting state for children of this element */
  raptor_state child_state;

  /* starting content type for children of this element */
  raptor_element_content_type child_content_type;


  /* CDATA content of element and checks for mixed content */
  char *content_cdata;
  unsigned int content_cdata_length;
  /* how many cdata blocks seen */
  unsigned int content_cdata_seen;
  /* all cdata so far is whitespace */
  unsigned int content_cdata_all_whitespace;
  /* how many contained elements seen */
  unsigned int content_element_seen;

  /* STATIC Bag identifier */
  raptor_identifier bag;


  /* STATIC Subject identifier (URI/anon ID), type, source
   *
   * When the XML element represents a node, this is the identifier 
   */
  raptor_identifier subject;
  
  /* STATIC Predicate URI, source is either
   * RAPTOR_URI_SOURCE_ELEMENT or RAPTOR_URI_SOURCE_ATTRIBUTE
   *
   * When the XML element represents a node or predicate,
   * this is the identifier of the predicate 
   */
  raptor_identifier predicate;

  /* STATIC Object identifier (URI/anon ID), type, source
   *
   * When this XML element generates a statement that needs an object,
   * possibly from a child element, this is the identifier of the object 
   */
  raptor_identifier object;


  /* URI of type of container */
  raptor_uri *container_type;

  /* last ordinal used, so initialising to 0 works, emitting rdf:_1 first */
  int last_ordinal;

  /* If this element's parseType is a daml:collection 
   * this identifies the current tail of the collection(list). 
   */
  raptor_uri *tail_uri;
};

typedef struct raptor_element_s raptor_element;


/*
 * Raptor parser object
 */
struct raptor_parser_s {

#ifdef LIBRDF_INTERNAL
  librdf_world *world;

  /* DAML collection URIs */
  librdf_uri *raptor_daml_oil_uri;
  librdf_uri *raptor_daml_List_uri;
  librdf_uri *raptor_daml_first_uri;
  librdf_uri *raptor_daml_rest_uri;
  librdf_uri *raptor_daml_nil_uri;
#endif

  /* XML parser specific stuff */
#ifdef NEED_EXPAT
  XML_Parser xp;
#endif
#ifdef NEED_LIBXML
  /* structure holding sax event handlers */
  xmlSAXHandler sax;
  /* parser context */
  xmlParserCtxtPtr xc;
  /* pointer to SAX document locator */
  xmlSAXLocatorPtr loc;
#endif  

  /* element depth */
  int depth;

  /* stack of namespaces, most recently added at top */
  raptor_ns_map *namespaces;

  /* can be filled with error location information */
  raptor_locator locator;

  /* stack of elements - elements add after current_element */
  raptor_element *root_element;
  raptor_element *current_element;

  /* non 0 if parser had fatal error and cannot continue */
  int failed;

  /* generated ID counter */
  int genid;

  /* base URI of RDF/XML */
  raptor_uri *base_uri;


  /* static statement for use in passing to user code */
  raptor_statement statement;


  /* FEATURE: 
   * non 0 if scanning for <rdf:RDF> element, else assume doc is RDF
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
};




/* static variables */

#ifdef LIBRDF_INTERNAL
#define RAPTOR_RDF_type_URI LIBRDF_MS_type_URI
#define RAPTOR_RDF_value_URI LIBRDF_MS_value_URI
#define RAPTOR_RDF_subject_URI LIBRDF_MS_subject_URI
#define RAPTOR_RDF_predicate_URI LIBRDF_MS_predicate_URI
#define RAPTOR_RDF_object_URI LIBRDF_MS_object_URI
#define RAPTOR_RDF_Statement_URI LIBRDF_MS_Statement_URI

#define RAPTOR_RDF_Seq_URI LIBRDF_MS_Seq_URI
#define RAPTOR_RDF_Bag_URI LIBRDF_MS_Bag_URI
#define RAPTOR_RDF_Alt_URI LIBRDF_MS_Alt_URI

#define RAPTOR_DAML_LIST_URI(rdf_parser) rdf_parser->raptor_daml_List_uri
#define RAPTOR_DAML_FIRST_URI(rdf_parser) rdf_parser->raptor_daml_first_uri
#define RAPTOR_DAML_REST_URI(rdf_parser) rdf_parser->raptor_daml_rest_uri
#define RAPTOR_DAML_NIL_URI(rdf_parser) rdf_parser->raptor_daml_nil_uri

#else

static const char * const raptor_rdf_ms_uri=RAPTOR_RDF_MS_URI;
static const char * const raptor_rdf_schema_uri=RAPTOR_RDF_SCHEMA_URI;
static const char * const raptor_rdf_type_uri=RAPTOR_RDF_MS_URI "type";
static const char * const raptor_rdf_value_uri=RAPTOR_RDF_MS_URI "value";
static const char * const raptor_rdf_subject_uri=RAPTOR_RDF_MS_URI "subject";
static const char * const raptor_rdf_predicate_uri=RAPTOR_RDF_MS_URI "predicate";
static const char * const raptor_rdf_object_uri=RAPTOR_RDF_MS_URI "object";
static const char * const raptor_rdf_Statement_uri=RAPTOR_RDF_MS_URI "Statement";

static const char * const raptor_rdf_Seq_uri=RAPTOR_RDF_MS_URI "Seq";
static const char * const raptor_rdf_Bag_uri=RAPTOR_RDF_MS_URI "Bag";
static const char * const raptor_rdf_Alt_uri=RAPTOR_RDF_MS_URI "Alt";

static const char * const raptor_daml_List_uri=RAPTOR_DAML_OIL_URI "List";
static const char * const raptor_daml_first_uri=RAPTOR_DAML_OIL_URI "first";
static const char * const raptor_daml_rest_uri=RAPTOR_DAML_OIL_URI "rest";
static const char * const raptor_daml_nil_uri=RAPTOR_DAML_OIL_URI "nil";


#define RAPTOR_RDF_type_URI raptor_rdf_type_uri
#define RAPTOR_RDF_value_URI raptor_rdf_value_uri
#define RAPTOR_RDF_subject_URI raptor_rdf_subject_uri
#define RAPTOR_RDF_predicate_URI raptor_rdf_predicate_uri
#define RAPTOR_RDF_object_URI raptor_rdf_object_uri
#define RAPTOR_RDF_Statement_URI raptor_rdf_Statement_uri

#define RAPTOR_RDF_Seq_URI raptor_rdf_Seq_uri
#define RAPTOR_RDF_Bag_URI raptor_rdf_Bag_uri
#define RAPTOR_RDF_Alt_URI raptor_rdf_Alt_uri

#define RAPTOR_DAML_LIST_URI(rdf_parser) raptor_daml_List_uri
#define RAPTOR_DAML_FIRST_URI(rdf_parser) raptor_daml_first_uri
#define RAPTOR_DAML_REST_URI(rdf_parser) raptor_daml_rest_uri
#define RAPTOR_DAML_NIL_URI(rdf_parser) raptor_daml_nil_uri

#endif


static const char * const raptor_xml_uri="http://www.w3.org/XML/1998/namespace";



/* Prototypes for common expat/libxml parsing event-handling functions */
static void raptor_xml_start_element_handler(void *user_data, const XML_Char *name, const XML_Char **atts);
static void raptor_xml_end_element_handler(void *user_data, const XML_Char *name);
/* s is not 0 terminated. */
static void raptor_xml_cdata_handler(void *user_data, const XML_Char *s, int len);

#ifdef HAVE_XML_SetNamespaceDeclHandler
static void raptor_start_namespace_decl_handler(void *user_data, const XML_Char *prefix, const XML_Char *uri);
static void raptor_end_namespace_decl_handler(void *user_data, const XML_Char *prefix);
#endif


/* libxml-only prototypes */
#ifdef NEED_LIBXML
static void raptor_xml_warning(void *context, const char *msg, ...);
static void raptor_xml_error(void *context, const char *msg, ...);
static void raptor_xml_fatal_error(void *context, const char *msg, ...);
static void raptor_xml_validation_error(void *context, const char *msg, ...);
static void raptor_xml_validation_warning(void *context, const char *msg, ...);
static void raptor_xml_set_document_locator (void *ctx, xmlSAXLocatorPtr loc);
#endif


/* Prototypes for local functions */
#ifndef LIBRDF_INTERNAL
static char * raptor_file_uri_to_filename(const char *uri);
#endif
static void raptor_parser_fatal_error(raptor_parser* parser, const char *message, ...);
static void raptor_parser_error(raptor_parser* parser, const char *message, ...);
static void raptor_parser_warning(raptor_parser* parser, const char *message, ...);



/* prototypes for namespace and name/local_name functions */
static void raptor_init_namespaces(raptor_parser *rdf_parser);
static void raptor_start_namespace(raptor_parser *rdf_parser, const char *prefix, const char *nspace, int depth);
static void raptor_free_namespace(raptor_parser *rdf_parser,  raptor_ns_map *nspace);
static void raptor_end_namespace(raptor_parser *rdf_parser, const char *prefix, const char *nspace);
static void raptor_end_namespaces_for_depth(raptor_parser *rdf_parser);
static raptor_ns_name* raptor_make_namespaced_name(raptor_parser *rdf_parser, const char *name, const char *value, int is_element);
#ifdef RAPTOR_DEBUG
static void raptor_print_ns_name(FILE *stream, raptor_ns_name* name);
#endif
static void raptor_free_ns_name(raptor_ns_name* name);
static int raptor_ns_names_equal(raptor_ns_name *name1, raptor_ns_name *name2);


/* prototypes for element functions */
static raptor_element* raptor_element_pop(raptor_parser *rdf_parser);
static void raptor_element_push(raptor_parser *rdf_parser, raptor_element* element);
static void raptor_free_element(raptor_element *element);
#ifdef RAPTOR_DEBUG
static void raptor_print_element(raptor_element *element, FILE* stream);
#endif


/* prototypes for grammar functions */
static void raptor_start_element_grammar(raptor_parser *parser, raptor_element *element);
static void raptor_end_element_grammar(raptor_parser *parser, raptor_element *element);


/* prototype for statement related functions */
static void raptor_generate_statement(raptor_parser *rdf_parser, raptor_uri *subject_uri, const char *subject_id, const raptor_identifier_type subject_type, const raptor_uri_source subject_uri_source, raptor_uri *predicate_uri, const char *predicate_id, const raptor_identifier_type predicate_type, const raptor_uri_source predicate_uri_source, raptor_uri *object_uri, const char *object_id, const raptor_identifier_type object_type, const raptor_uri_source object_uri_source, raptor_uri *bag);
static void raptor_print_statement_detailed(const raptor_statement *statement, int detailed, FILE *stream);


/*
 * Namespaces in XML
 * http://www.w3.org/TR/1999/REC-xml-names-19990114/#nsc-NSDeclared
 * (section 4) says:
 *
 * --------------------------------------------------------------------
 *   The prefix xml is by definition bound to the namespace name 
 *   http://www.w3.org/XML/1998/namespace
 * --------------------------------------------------------------------
 *
 * Thus should define it in the table of namespaces before we start.
 *
 * We *can* also define others, but let's not.
 *
 */
static void
raptor_init_namespaces(raptor_parser *rdf_parser) {
  /* defined at level -1 since always 'present' when inside the XML world */
  raptor_start_namespace(rdf_parser, "xml", raptor_xml_uri, -1);
}


static void
raptor_start_namespace(raptor_parser *rdf_parser, 
                       const char *prefix, const char *nspace, int depth)
{
  int prefix_length=0;
#ifdef LIBRDF_INTERNAL
#else
  int uri_length=0;
#endif
  int len;
  raptor_ns_map *map;
  char *p;

  LIBRDF_DEBUG4(raptor_start_namespace,
                "namespace prefix %s uri %s depth %d\n",
                prefix ? prefix : "(default)", nspace, depth);

  /* Convert an empty namespace string "" to a NULL pointer */
  if(!*nspace)
    nspace=NULL;

  len=sizeof(raptor_ns_map);
#ifdef LIBRDF_INTERNAL
#else
  if(nspace) {
    uri_length=strlen(nspace);
    len+=uri_length+1;
  }
#endif
  if(prefix) {
    prefix_length=strlen(prefix);
    len+=prefix_length+1;
  }

  /* Just one malloc for map structure + namespace (maybe) + prefix (maybe)*/
  map=(raptor_ns_map*)LIBRDF_CALLOC(raptor_ns_map, len, 1);
  if(!map) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  }

  p=(char*)map+sizeof(raptor_ns_map);
#ifdef LIBRDF_INTERNAL
  map->uri=librdf_new_uri(rdf_parser->world, nspace);
  if(!map->uri) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    LIBRDF_FREE(raptor_ns_map, map);
    return;
  }
#else
  if(nspace) {
    map->uri=strcpy((char*)p, nspace);
    map->uri_length=uri_length;
    p+= uri_length+1;
  }
#endif
  if(prefix) {
    map->prefix=strcpy((char*)p, prefix);
    map->prefix_length=prefix_length;

    if(!strcmp(map->prefix, "xml"))
      map->is_xml=1;
  }
  map->depth=depth;

  /* set convienience flags when there is a defined namespace URI */
  if(nspace) {
#ifdef LIBRDF_INTERNAL
    if(librdf_uri_equals(map->uri, librdf_concept_ms_namespace_uri))
      map->is_rdf_ms=1;
    else if(librdf_uri_equals(map->uri, librdf_concept_schema_namespace_uri))
      map->is_rdf_schema=1;
#else
    if(!strcmp(nspace, raptor_rdf_ms_uri))
      map->is_rdf_ms=1;
    else if(!strcmp(nspace, raptor_rdf_schema_uri))
      map->is_rdf_schema=1;
#endif
  }

  if(rdf_parser->namespaces)
    map->next=rdf_parser->namespaces;
  rdf_parser->namespaces=map;
}


static void 
raptor_free_namespace(raptor_parser *rdf_parser,  raptor_ns_map* nspace)
{
#ifdef LIBRDF_INTERNAL
  if(nspace->uri)
    librdf_free_uri(nspace->uri);
#endif
  LIBRDF_FREE(raptor_ns_map, nspace);
}


static void 
raptor_end_namespace(raptor_parser *rdf_parser, 
                     const char *prefix, const char *nspace)
{
  LIBRDF_DEBUG3(raptor_end_namespace, "prefix %s uri \"%s\"\n", 
                prefix ? prefix : "(default)", nspace);
}


static void 
raptor_end_namespaces_for_depth(raptor_parser *rdf_parser) 
{
  while(rdf_parser->namespaces &&
        rdf_parser->namespaces->depth == rdf_parser->depth) {
    raptor_ns_map* ns=rdf_parser->namespaces;
    raptor_ns_map* next=ns->next;

#ifdef LIBRDF_INTERNAL
    raptor_end_namespace(rdf_parser, ns->prefix, 
                         librdf_uri_as_string(ns->uri));
#else  
    raptor_end_namespace(rdf_parser, ns->prefix, ns->uri);
#endif
    raptor_free_namespace(rdf_parser, ns);

    rdf_parser->namespaces=next;
  }

}



/*
 * Namespaces in XML
 * http://www.w3.org/TR/1999/REC-xml-names-19990114/#defaulting
 * says:
 *
 * --------------------------------------------------------------------
 *  5.2 Namespace Defaulting
 *
 *  A default namespace is considered to apply to the element where it
 *  is declared (if that element has no namespace prefix), and to all
 *  elements with no prefix within the content of that element. 
 *
 *  If the URI reference in a default namespace declaration is empty,
 *  then unprefixed elements in the scope of the declaration are not
 *  considered to be in any namespace.
 *
 *  Note that default namespaces do not apply directly to attributes.
 *
 * [...]
 *
 *  5.3 Uniqueness of Attributes
 *
 *  In XML documents conforming to this specification, no tag may
 *  contain two attributes which:
 *
 *    1. have identical names, or 
 *
 *    2. have qualified names with the same local part and with
 *    prefixes which have been bound to namespace names that are
 *    identical.
 * --------------------------------------------------------------------
 */

static raptor_ns_name*
raptor_make_namespaced_name(raptor_parser *rdf_parser, const char *name,
                            const char *value, int is_element) 
{
  raptor_ns_name* ns_name;
  const char *p;
  raptor_ns_map* ns;
  char* new_name;
  int prefix_length;
  int local_name_length=0;

#if RAPTOR_DEBUG > 1
  LIBRDF_DEBUG2(raptor_make_namespaced_name,
                "name %s\n", name);
#endif  

  ns_name=(raptor_ns_name*)LIBRDF_CALLOC(raptor_ns_name, sizeof(raptor_ns_name), 1);
  if(!ns_name) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    return NULL;
  }

  if(value) {
    int value_length=strlen(value);
    char* new_value=(char*)LIBRDF_MALLOC(cstring, value_length+1);

    if(!new_value) {
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      LIBRDF_FREE(raptor_ns_name, ns_name);
      return NULL;
    } 
    strcpy(new_value, value);
    ns_name->value=new_value;
    ns_name->value_length=value_length;
  }

  /* Find : */
  for(p=name; *p && *p != ':'; p++)
    ;

  if(!*p) {
    local_name_length=p-name;

    /* No : - pick up default namespace, if there is one */
    new_name=(char*)LIBRDF_MALLOC(cstring, local_name_length+1);
    if(!new_name) {
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      raptor_free_ns_name(ns_name);
      return NULL;
    }
    strcpy(new_name, name);
    ns_name->local_name=new_name;
    ns_name->local_name_length=local_name_length;
    
    /* Find a default namespace */
    for(ns=rdf_parser->namespaces; ns && ns->prefix; ns=ns->next)
      ;

    if(ns) {
      ns_name->nspace=ns;
#if RAPTOR_DEBUG > 1
    LIBRDF_DEBUG2(raptor_make_namespaced_name,
                  "Found default namespace %s\n", ns->uri);
#endif
    } else {
      /* failed to find namespace - now what? FIXME */
      /* raptor_parser_warning(rdf_parser, "No default namespace defined - cannot expand %s", name); */
#if RAPTOR_DEBUG > 1
      LIBRDF_DEBUG1(raptor_make_namespaced_name,
                    "No default namespace defined\n");
#endif
    }

  } else {
    /* There is a namespace prefix */

    prefix_length=p-name;
    p++; 

    /* p now is at start of local_name */
    local_name_length=strlen(p);
    new_name=(char*)LIBRDF_MALLOC(cstring, local_name_length+1);
    if(!new_name) {
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      raptor_free_ns_name(ns_name);
      return NULL;
    }
    strcpy(new_name, p);
    ns_name->local_name=new_name;
    ns_name->local_name_length=local_name_length;

    /* Find the namespace */
    for(ns=rdf_parser->namespaces; ns ; ns=ns->next)
      if(ns->prefix && prefix_length == ns->prefix_length && 
         !strncmp(name, ns->prefix, prefix_length))
        break;

    if(!ns) {
      /* failed to find namespace - now what? */
      raptor_parser_error(rdf_parser, "Failed to find namespace in %s", name);
      raptor_free_ns_name(ns_name);
      return NULL;
    }

#if RAPTOR_DEBUG > 1
    LIBRDF_DEBUG3(raptor_make_namespaced_name,
                  "Found namespace prefix %s URI %s\n", ns->prefix, ns->uri);
#endif
    ns_name->nspace=ns;
  }



  /* If namespace has a URI and a local_name is defined, create the URI
   * for this element 
   */
  if(ns_name->nspace && ns_name->nspace->uri && local_name_length) {
#ifdef LIBRDF_INTERNAL
    librdf_uri* uri=librdf_new_uri_from_uri_local_name(ns_name->nspace->uri,
                                                       new_name);
    if(!uri) {
      raptor_free_ns_name(ns_name);
      return NULL;
    }
    ns_name->uri=uri;
#else
    char *uri_string=(char*)LIBRDF_MALLOC(cstring, 
                                          ns_name->nspace->uri_length + 
                                          local_name_length + 1);
    if(!uri_string) {
      raptor_free_ns_name(ns_name);
      return NULL;
    }
    strcpy(uri_string, ns_name->nspace->uri);
    strcpy(uri_string + ns_name->nspace->uri_length, new_name);
    ns_name->uri=uri_string;
#endif
  }


  return ns_name;
}


#ifdef RAPTOR_DEBUG
static void
raptor_print_ns_name(FILE *stream, raptor_ns_name* name) 
{
  if(name->nspace) {
    if(name->nspace->prefix)
      fprintf(stream, "%s:%s", name->nspace->prefix, name->local_name);
    else
      fprintf(stream, "(default):%s", name->local_name);
  } else
    fputs(name->local_name, stream);
}
#endif

static void
raptor_free_ns_name(raptor_ns_name* name) 
{
  if(name->local_name)
    LIBRDF_FREE(cstring, (void*)name->local_name);

  if(name->uri)
    RAPTOR_FREE_URI(name->uri);

  if(name->value)
    LIBRDF_FREE(cstring, (void*)name->value);
  LIBRDF_FREE(raptor_ns_name, name);
}


static int
raptor_ns_names_equal(raptor_ns_name *name1, raptor_ns_name *name2)
{
#ifdef LIBRDF_INTERNAL
  if(name1->uri && name2->uri)
    return librdf_uri_equals(name1->uri, name2->uri);
#else
  if(name1->nspace != name2->nspace)
    return 0;
#endif
  if(name1->local_name_length != name2->local_name_length)
    return 0;
  if(strcmp(name1->local_name, name2->local_name))
    return 0;
  return 1;
}


static raptor_element*
raptor_element_pop(raptor_parser *rdf_parser) 
{
  raptor_element *element=rdf_parser->current_element;

  if(!element)
    return NULL;

  rdf_parser->current_element=element->parent;
  if(rdf_parser->root_element == element) /* just deleted root */
    rdf_parser->root_element=NULL;

  return element;
}


static void
raptor_element_push(raptor_parser *rdf_parser, raptor_element* element) 
{
  element->parent=rdf_parser->current_element;
  rdf_parser->current_element=element;
  if(!rdf_parser->root_element)
    rdf_parser->root_element=element;
}


static void
raptor_free_element(raptor_element *element)
{
  unsigned int i;

  for (i=0; i < element->attribute_count; i++)
    if(element->attributes[i])
      raptor_free_ns_name(element->attributes[i]);

  if(element->attributes)
    LIBRDF_FREE(raptor_ns_name_array, element->attributes);

  /* Free special RDF M&S attributes */
  for(i=0; i<= RDF_ATTR_LAST; i++) 
    if(element->rdf_attr[i])
      LIBRDF_FREE(cstring, (void*)element->rdf_attr[i]);

  if(element->content_cdata_length)
    LIBRDF_FREE(raptor_ns_name_array, element->content_cdata);

  raptor_free_identifier(&element->subject);
  raptor_free_identifier(&element->predicate);
  raptor_free_identifier(&element->object);
  raptor_free_identifier(&element->bag);

  if(element->tail_uri)
    RAPTOR_FREE_URI(element->tail_uri);

  raptor_free_ns_name(element->name);
  LIBRDF_FREE(raptor_element, element);
}



#ifdef RAPTOR_DEBUG
static void
raptor_print_element(raptor_element *element, FILE* stream)
{
  raptor_print_ns_name(stream, element->name);
  fputc('\n', stream);

  if(element->attribute_count) {
    unsigned int i;

    fputs(" attributes: ", stream);
    for (i = 0; i < element->attribute_count; i++) {
      if(i)
        fputc(' ', stream);
      raptor_print_ns_name(stream, element->attributes[i]);
      fprintf(stream, "='%s'", element->attributes[i]->value);
    }
    fputc('\n', stream);
  }
}
#endif


static char *
raptor_format_element(raptor_element *element, int *length_p, int is_end)
{
  int length;
  char *buffer;
  char *ptr;
  unsigned int i;

  /* get length of element name (and namespace-prefix: if there is one) */
  length=element->name->local_name_length + 1; /* < */
  if(element->name->nspace &&
     element->name->nspace->prefix_length > 0)
    length += element->name->nspace->prefix_length + 1; /* : */

  if(is_end)
    length++; /* / */

  if (!is_end && element->attributes) {
    for(i=0; i < element->attribute_count; i++) {
      length++; /* ' ' between attributes and after element name */

      /* qname */
      length += element->attributes[i]->local_name_length;
      if(element->attributes[i]->nspace &&
         element->attributes[i]->nspace->prefix_length > 0)
         /* prefix: */
        length += element->attributes[i]->nspace->prefix_length + 1;

      /* ="value" */
      length += element->attributes[i]->value_length + 3;
    }
  }
  
  length++; /* > */
  
  if(length_p)
    *length_p=length;

  /* +1 here is for \0 at end */
  buffer=(char*)LIBRDF_MALLOC(cstring, length + 1);
  if(!buffer)
    return NULL;

  ptr=buffer;

  *ptr++ = '<';
  if(is_end)
    *ptr++ = '/';
  if(element->name->nspace && element->name->nspace->prefix_length > 0) {
    strncpy(ptr, element->name->nspace->prefix,
            element->name->nspace->prefix_length);
    ptr+= element->name->nspace->prefix_length;
    *ptr++=':';
  }
  strcpy(ptr, element->name->local_name);
  ptr += element->name->local_name_length;

  if(!is_end && element->attributes) {
    for(i=0; i < element->attribute_count; i++) {
      *ptr++ =' ';
      
      if(element->attributes[i]->nspace && 
         element->attributes[i]->nspace->prefix_length > 0) {
        strncpy(ptr, element->attributes[i]->nspace->prefix,
                element->attributes[i]->nspace->prefix_length);
        ptr+= element->attributes[i]->nspace->prefix_length;
        *ptr++=':';
      }
    
      strcpy(ptr, element->attributes[i]->local_name);
      ptr += element->attributes[i]->local_name_length;
      
      *ptr++ ='=';
      *ptr++ ='"';
      
      strcpy(ptr, element->attributes[i]->value);
      ptr += element->attributes[i]->value_length;
      *ptr++ ='"';
    }
  }
  
  *ptr++ = '>';
  *ptr='\0';

  return buffer;
}



static void
raptor_xml_start_element_handler(void *user_data,
                                 const XML_Char *name, const XML_Char **atts)
{
  raptor_parser* rdf_parser;
  int all_atts_count=0;
  int ns_attributes_count=0;
  raptor_ns_name** named_attrs=NULL;
  int i;
  raptor_ns_name* element_name;
  raptor_element* element=NULL;
#ifdef NEED_EXPAT
  /* for storing error info */
  raptor_locator *locator;
#endif
  int non_nspaced_count=0;
  
  rdf_parser=(raptor_parser*)user_data;
#ifdef NEED_EXPAT
  locator=&rdf_parser->locator; 
#endif

#ifdef RAPTOR_DEBUG
  fputc('\n', stderr);
#endif

#ifdef NEED_EXPAT
  locator->line=XML_GetCurrentLineNumber(rdf_parser->xp);
  locator->column=XML_GetCurrentColumnNumber(rdf_parser->xp);
  locator->byte=XML_GetCurrentByteIndex(rdf_parser->xp);
#endif

  rdf_parser->depth++;

  if (atts) {
    /* Round 1 - find special attributes, at present just namespaces */
    for (i = 0; atts[i]; i+=2) {
      all_atts_count++;

      /* synthesise the XML NS events */
      if(!strncmp(atts[i], "xmlns", 5)) {
        /* there is more i.e. xmlns:foo */
        const char *prefix=atts[i][5] ? &atts[i][6] : NULL;

        raptor_start_namespace(rdf_parser, prefix, atts[i+1],
                               rdf_parser->depth);
        /* Is it ok to zap XML parser array things? */
        atts[i]=NULL; 
        continue;
      }

      ns_attributes_count++;
    }
  }


  /* Now can recode element name with a namespace */

  element_name=raptor_make_namespaced_name(rdf_parser, name, NULL, 1);
  if(!element_name) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  }


  /* Create new element structure */
  element=(raptor_element*)LIBRDF_CALLOC(raptor_element, 
                                         sizeof(raptor_element), 1);
  if(!element) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    raptor_free_ns_name(element_name);
    return;
  } 


  element->name=element_name;

  /* Prepare for possible element content */
  element->content_element_seen=0;
  element->content_cdata_seen=0;
  element->content_cdata_length=0;


  if(!element_name->nspace)
    non_nspaced_count++;


  if(ns_attributes_count) {
    int offset = 0;

    /* Round 2 - turn attributes into namespaced-attributes */

    /* Allocate new array to hold namespaced-attributes */
    named_attrs=(raptor_ns_name**)LIBRDF_CALLOC(raptor_ns_name-array, sizeof(raptor_ns_name*), ns_attributes_count);
    if(!named_attrs) {
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      LIBRDF_FREE(raptor_element, element);
      raptor_free_ns_name(element_name);
      return;
    }

    for (i = 0; i < all_atts_count; i++) {
      raptor_ns_name* attribute;

      /* Skip previously processed attributes */
      if(!atts[i<<1])
        continue;

      /* namespace-name[i] stored in named_attrs[i] */
      attribute=raptor_make_namespaced_name(rdf_parser, atts[i<<1],
                                            atts[(i<<1)+1], 0);
      if(!attribute) { /* failed - tidy up and return */
        int j;

        for (j=0; j < i; j++)
          LIBRDF_FREE(raptor_ns_name, named_attrs[j]);
        LIBRDF_FREE(raptor_ns_name_array, named_attrs);
        LIBRDF_FREE(raptor_element, element);
        raptor_free_ns_name(element_name);
        return;
      }

      
      /* leave literal XML alone */
      if (!(rdf_parser->current_element &&
            rdf_parser->current_element->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL)) {

        /* Save pointers to some RDF M&S attributes */

        /* If RDF M&S namespace-prefixed attributes */
        if(attribute->nspace && attribute->nspace->is_rdf_ms) {
          const char *attr_name=attribute->local_name;
          int j;

          for(j=0; j<= RDF_ATTR_LAST; j++)
            if(!strcmp(attr_name, rdf_attr_info[j].name)) {
              element->rdf_attr[j]=attribute->value;
              element->rdf_attr_count++;
              /* Delete it if it was stored elsewhere */
#if RAPTOR_DEBUG
              LIBRDF_DEBUG3(raptor_xml_start_element_handler,
                            "Found RDF M&S attribute %s URI %s\n",
                            attr_name, attribute->value);
#endif
              /* make sure value isn't deleted from ns_name structure */
              attribute->value=NULL;
              raptor_free_ns_name(attribute);
              attribute=NULL;
            }
        } /* end if RDF M&S namespaced-prefixed attributes */

        if(!attribute)
          continue;

        /* If non namespace-prefixed RDF M&S attributes found on an element */
        if(rdf_parser->feature_allow_non_ns_attributes) {
          const char *attr_name=attribute->local_name;
          int j;

          for(j=0; j<= RDF_ATTR_LAST; j++)
            if(!strcmp(attr_name, rdf_attr_info[j].name)) {
              element->rdf_attr[j]=attribute->value;
              element->rdf_attr_count++;
              /* Delete it if it was stored elsewhere */
#if RAPTOR_DEBUG
              LIBRDF_DEBUG3(raptor_xml_start_element_handler,
                            "Found non-namespaced RDF M&S attribute %s URI %s\n",
                            attr_name, attribute->value);
#endif
              /* make sure value isn't deleted from ns_name structure */
              attribute->value=NULL;
              raptor_free_ns_name(attribute);
              attribute=NULL;
            }
        } /* end if non-namespace prefixed RDF M&S attributes */

        if(!attribute)
          continue;

        if(!attribute->nspace)
          non_nspaced_count++;

      } /* end if leave literal XML alone */

      if(attribute)
        named_attrs[offset++]=attribute;
    }

    /* set actual count from attributes that haven't been skipped */
    ns_attributes_count=offset;
    if(!offset && named_attrs) {
      /* all attributes were RDF M&S or other specials and deleted
       * so delete array and don't store pointer */
      LIBRDF_FREE(raptor_ns_name_array, named_attrs);
      named_attrs=NULL;
    }

  } /* end if ns_attributes_count */

  element->attributes=named_attrs;
  element->attribute_count=ns_attributes_count;


  raptor_element_push(rdf_parser, element);


  /* start from unknown; if we have a parent, it may set this */
  element->state=RAPTOR_STATE_UNKNOWN;

  if(element->parent) {
    element->content_type=element->parent->content_type;

    element->parent->content_element_seen++;
    
    /* leave literal XML alone */
    if (rdf_parser->current_element->content_type != RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL) {
      if(element->parent->content_element_seen == 1 &&
         element->parent->content_cdata_seen == 1) {
        /* Uh oh - mixed content, the parent element has cdata too */
        raptor_parser_warning(rdf_parser, "element %s has mixed content.", 
                              element->parent->name->local_name);
      }
    
      /* If there is some existing all-whitespace content cdata
       * (which is probably before the first element) delete it
       */
      if(element->parent->content_element_seen &&
         element->parent->content_cdata_all_whitespace &&
         element->parent->content_cdata) {
        LIBRDF_FREE(raptor_ns_name_array, element->parent->content_cdata);
        element->parent->content_cdata=NULL;
        element->parent->content_cdata_length=0;
      }

    } /* end if leave literal XML alone */

    if(element->parent->child_state)
      element->state=element->parent->child_state;
    else
      raptor_parser_fatal_error(rdf_parser, "raptor_xml_start_element_handler - no parent element child_state set\n");      
    
  } /* end if element->parent */


#ifdef RAPTOR_DEBUG
  LIBRDF_DEBUG2(raptor_xml_start_element_handler, "Using content type %s\n",
                rdf_content_type_info[element->content_type].name);

  fprintf(stderr, "raptor_xml_start_element_handler: Start ns-element: ");
  raptor_print_element(element, stderr);
#endif

  
  /* Check for non namespaced stuff */
  if(non_nspaced_count)
    element->state=RAPTOR_STATE_SKIPPING;

  /* Right, now ready to enter the grammar */
  raptor_start_element_grammar(rdf_parser, element);

}


static void
raptor_xml_end_element_handler(void *user_data, const XML_Char *name)
{
  raptor_parser* rdf_parser=(raptor_parser*)user_data;
  raptor_element* element;
  raptor_ns_name *element_name;
#ifdef NEED_EXPAT
  /* for storing error info */
  raptor_locator *locator=&rdf_parser->locator;
#endif

#ifdef NEED_EXPAT
  locator->line=XML_GetCurrentLineNumber(rdf_parser->xp);
  locator->column=XML_GetCurrentColumnNumber(rdf_parser->xp);
  locator->byte=XML_GetCurrentByteIndex(rdf_parser->xp);
#endif

  /* recode element name */

  element_name=raptor_make_namespaced_name(rdf_parser, name, NULL, 1);
  if(!element_name) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  }


#ifdef RAPTOR_DEBUG
  fprintf(stderr, "\nraptor_xml_end_element_handler: End ns-element: ");
  raptor_print_ns_name(stderr, element_name);
  fputc('\n', stderr);
#endif

  element=raptor_element_pop(rdf_parser);
  if(!raptor_ns_names_equal(element->name, element_name)) {
    /* Hmm, unexpected name - FIXME, should do something! */
    raptor_parser_warning(rdf_parser, 
                          "Element %s ended, expected end of element %s\n",
                          name, element->name->local_name);
    return;
  }

  raptor_end_element_grammar(rdf_parser, element);

  raptor_free_ns_name(element_name);

  raptor_end_namespaces_for_depth(rdf_parser);

  raptor_free_element(element);

  rdf_parser->depth--;

#ifdef RAPTOR_DEBUG
  fputc('\n', stderr);
#endif

}



/* cdata (and ignorable whitespace for libxml). 
 * s is not 0 terminated for expat, is for libxml - grrrr.
 */
static void
raptor_xml_cdata_handler(void *user_data, const XML_Char *s, int len)
{
  raptor_parser* rdf_parser=(raptor_parser*)user_data;
  raptor_element* element;
  raptor_state state;
  char *buffer;
  char *ptr;
  int all_whitespace=1;
  int i;

  for(i=0; i<len; i++)
    if(!isspace(s[i])) {
      all_whitespace=0;
      break;
    }

  element=rdf_parser->current_element;

#ifdef RAPTOR_DEBUG
  fputc('\n', stderr);
#endif

  /* cdata never changes the parser state 
   * and the containing element state always determines what to do.
   * Use the child_state first if there is one, since that applies
   */
  state=element->child_state;
  LIBRDF_DEBUG3(raptor_xml_cdata_handler, "Working in state %d - %s\n", state,
                raptor_state_as_string(state));


  LIBRDF_DEBUG3(raptor_xml_cdata_handler,
                "Content type %s (%d)\n", raptor_element_content_type_as_string(element->content_type), element->content_type);
  


  switch(state) {
    case RAPTOR_STATE_SKIPPING:
      return;

    case RAPTOR_STATE_UNKNOWN:
      /* Ignore all cdata if still looking for RDF */
      if(rdf_parser->feature_scanning_for_rdf_RDF)
        return;

      /* Ignore all whitespace cdata before first element */
      if(all_whitespace)
        return;

      /* This probably will never happen since that would make the
       * XML not be well-formed
       */
      raptor_parser_warning(rdf_parser, "Found cdata before RDF element.");
      break;

    case RAPTOR_STATE_PARSETYPE_OTHER:
      /* FIXME */

      /* FALLTHROUGH */

    case RAPTOR_STATE_PARSETYPE_LITERAL:
      element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL;
      break;

    case RAPTOR_STATE_PARSETYPE_DAML_COLLECTION:
      element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION;
      break;

    default:
      break;
    } /* end switch */


  if(element->content_type != RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL &&
     element->content_type != RAPTOR_ELEMENT_CONTENT_TYPE_PRESERVED) {

    /* Whitespace is ignored except for literal or preserved content types */
    if(all_whitespace) {
      LIBRDF_DEBUG2(raptor_xml_cdata_handler, "Ignoring whitespace cdata inside element %s\n", element->name->local_name);
      return;
    }

    if(++element->content_cdata_seen == 1 &&
       element->content_element_seen == 1) {
      /* Uh oh - mixed content, this element has elements too */
      raptor_parser_warning(rdf_parser, "element %s has mixed content.", 
                            element->name->local_name);
    }
  }


  buffer=(char*)LIBRDF_MALLOC(cstring, element->content_cdata_length + len + 1);
  if(!buffer) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  }

  if(element->content_cdata_length) {
    strncpy(buffer, element->content_cdata, element->content_cdata_length);
    LIBRDF_FREE(cstring, element->content_cdata);
    element->content_cdata_all_whitespace &= all_whitespace;
  } else
    element->content_cdata_all_whitespace = all_whitespace;

  element->content_cdata=buffer;

  /* move pointer to end of cdata buffer */
  ptr=buffer+element->content_cdata_length;

  /* adjust stored length */
  element->content_cdata_length += len;

  /* now write new stuff at end of cdata buffer */
  strncpy(ptr, s, len);
  ptr += len;
  *ptr = '\0';

  LIBRDF_DEBUG3(raptor_xml_cdata_handler, 
                "Content cdata now: '%s' (%d bytes)\n", 
                buffer, element->content_cdata_length);

  LIBRDF_DEBUG3(raptor_xml_cdata_handler, 
                "Ending in state %d - %s\n",
                state, raptor_state_as_string(state));

}


#ifdef HAVE_XML_SetNamespaceDeclHandler
static void
raptor_start_namespace_decl_handler(void *user_data,
                                    const XML_Char *prefix, const XML_Char *uri)
{
  raptor_parser* rdf_parser=(raptor_parser*)user_data;

#ifdef RAPTOR_DEBUG
  fprintf(stderr, "saw namespace %s URI %s\n", prefix, uri);
#endif
}


static void
raptor_end_namespace_decl_handler(void *user_data, const XML_Char *prefix)
{
  raptor_parser* rdf_parser=(raptor_parser*)user_data;

#ifdef RAPTOR_DEBUG
  fprintf(stderr, "saw end namespace prefix %s\n", prefix);
#endif
}
#endif


#ifdef NEED_EXPAT
/* This is called for a declaration of an unparsed (NDATA) entity */
static void
raptor_xml_unparsed_entity_decl_handler(void *user_data,
                                        const XML_Char *entityName,
                                        const XML_Char *base,
                                        const XML_Char *systemId,
                                        const XML_Char *publicId,
                                        const XML_Char *notationName) 
{
/*  raptor_parser* rdf_parser=(raptor_parser*)user_data; */
  fprintf(stderr,
          "raptor_xml_unparsed_entity_decl_handler: entityName %s base %s systemId %s publicId %s notationName %s\n",
          entityName, (base ? base : "(None)"), 
          systemId, (publicId ? publicId: "(None)"),
          notationName);
}


static int 
raptor_xml_external_entity_ref_handler(void *user_data,
                                       const XML_Char *context,
                                       const XML_Char *base,
                                       const XML_Char *systemId,
                                       const XML_Char *publicId)
{
/*  raptor_parser* rdf_parser=(raptor_parser*)user_data; */
  fprintf(stderr,
          "raptor_xml_external_entity_ref_handler: base %s systemId %s publicId %s\n",
          (base ? base : "(None)"), 
          systemId, (publicId ? publicId: "(None)"));

  /* "The handler should return 0 if processing should not continue
   * because of a fatal error in the handling of the external entity."
   */
  return 1;
}
#endif


#ifdef NEED_LIBXML
#include <stdarg.h>

static const char* xml_warning_prefix="XML parser warning - ";
static const char* xml_error_prefix="XML parser error - ";
static const char* xml_fatal_error_prefix="XML parser fatal error - ";
static const char* xml_validation_error_prefix="XML parser validation error - ";
static const char* xml_validation_warning_prefix="XML parser validation warning - ";

static void
raptor_xml_set_document_locator (void *ctx, xmlSAXLocatorPtr loc) 
{
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  rdf_parser->loc=loc;
}

static void
raptor_xml_update_document_locator (raptor_parser *rdf_parser) {
  /* for storing error info */
  raptor_locator *locator=&rdf_parser->locator;

  locator->line=rdf_parser->loc->getLineNumber(rdf_parser->xc);
  locator->column=rdf_parser->loc->getColumnNumber(rdf_parser->xc);
}
  

static void
raptor_xml_warning(void *ctx, const char *msg, ...) 
{
  va_list args;
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  int length;
  char *nmsg;

  va_start(args, msg);

  raptor_xml_update_document_locator(rdf_parser);
  
  length=strlen(xml_warning_prefix)+strlen(msg)+1;
  nmsg=(char*)LIBRDF_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    raptor_parser_warning(rdf_parser, msg, args);
  } else {
    strcpy(nmsg, xml_warning_prefix);
    strcat(nmsg, msg);
    raptor_parser_warning(rdf_parser, nmsg, args);
    LIBRDF_FREE(cstring,nmsg);
  }
  va_end(args);
}


static void
raptor_xml_error(void *ctx, const char *msg, ...) 
{
  va_list args;
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  int length;
  char *nmsg;

  va_start(args, msg);

  raptor_xml_update_document_locator(rdf_parser);

  length=strlen(xml_error_prefix)+strlen(msg)+1;
  nmsg=(char*)LIBRDF_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    raptor_parser_error(rdf_parser, nmsg, args);
  } else {
    strcpy(nmsg, xml_error_prefix);
    strcat(nmsg, msg);
    raptor_parser_error(rdf_parser, nmsg, args);
    LIBRDF_FREE(cstring,nmsg);
  }
  va_end(args);
}


static void
raptor_xml_fatal_error(void *ctx, const char *msg, ...) 
{
  va_list args;
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  int length;
  char *nmsg;

  va_start(args, msg);

  raptor_xml_update_document_locator(rdf_parser);

  length=strlen(xml_fatal_error_prefix)+strlen(msg)+1;
  nmsg=(char*)LIBRDF_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    raptor_parser_fatal_error(rdf_parser, nmsg, args);
  } else {
    strcpy(nmsg, xml_error_prefix);
    strcat(nmsg, msg);
    raptor_parser_fatal_error(rdf_parser, nmsg, args);
    LIBRDF_FREE(cstring,nmsg);
  }
  va_end(args);
}


static void
raptor_xml_validation_error(void *ctx, const char *msg, ...) 
{
  va_list args;
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  int length;
  char *nmsg;

  va_start(args, msg);

  raptor_xml_update_document_locator(rdf_parser);

  length=strlen(xml_validation_error_prefix)+strlen(msg)+1;
  nmsg=(char*)LIBRDF_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    raptor_parser_fatal_error(rdf_parser, nmsg, args);
  } else {
    strcpy(nmsg, xml_validation_error_prefix);
    strcat(nmsg, msg);
    raptor_parser_fatal_error(rdf_parser, nmsg, args);
    LIBRDF_FREE(cstring,nmsg);
  }
  va_end(args);
}


static void
raptor_xml_validation_warning(void *ctx, const char *msg, ...) 
{
  va_list args;
  raptor_parser* rdf_parser=(raptor_parser*)ctx;
  int length;
  char *nmsg;

  va_start(args, msg);

  raptor_xml_update_document_locator(rdf_parser);

  length=strlen(xml_validation_warning_prefix)+strlen(msg)+1;
  nmsg=(char*)LIBRDF_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    raptor_parser_warning(rdf_parser, nmsg, args);
  } else {
    strcpy(nmsg, xml_validation_warning_prefix);
    strcat(nmsg, msg);
    raptor_parser_fatal_error(rdf_parser, nmsg, args);
    LIBRDF_FREE(cstring,nmsg);
  }
  va_end(args);
}


#endif



#ifndef LIBRDF_INTERNAL
/**
 * raptor_file_uri_to_filename - Convert a URI representing a file (starting file:) to a filename
 * @uri: URI of string
 * 
 * Return value: the filename or NULL on failure
 **/
static char *
raptor_file_uri_to_filename(const char *uri) 
{
  char *filename;
#ifndef WIN32
  int length;
#endif

#ifdef WIN32
  filename=LIBRDF_MALLOC(cstring, MAX_PATH);
  if (S_OK != URLDownloadToCacheFile(NULL, uri, filename, URLOSTRM_GETNEWESTVERSION, 0, NULL))
    return NULL;
#else
  if (strncmp(uri, "file:", 5))
    return NULL;

  /* FIXME: unix version of URI -> filename conversion */
  length=strlen(uri) -5 +1;
  filename=(char*)LIBRDF_MALLOC(cstring, length);
  if(!filename)
    return NULL;

  strcpy(filename, uri+5);
#endif

  return filename;
}
#endif


/*
 * raptor_parser_fatal_error - Error from a parser - Internal
 **/
static void
raptor_parser_fatal_error(raptor_parser* parser, const char *message, ...)
{
  va_list arguments;

  parser->failed=1;

  va_start(arguments, message);

  if(parser->fatal_error_handler) {
    parser->fatal_error_handler(parser->fatal_error_user_data, 
                                &parser->locator, message, arguments);
    abort();
  }

  raptor_print_locator(stderr, &parser->locator);
  fprintf(stderr, " raptor fatal error - ");
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);

  va_end(arguments);

  abort();
}


/*
 * raptor_parser_error - Error from a parser - Internal
 **/
static void
raptor_parser_error(raptor_parser* parser, const char *message, ...)
{
  va_list arguments;

  va_start(arguments, message);

  if(parser->error_handler) {
    parser->error_handler(parser->error_user_data, 
                          &parser->locator, message, arguments);
    return;
  }

  raptor_print_locator(stderr, &parser->locator);
  fprintf(stderr, " raptor error - ");
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);

  va_end(arguments);
}


/*
 * raptor_parser_warning - Warning from a parser - Internal
 **/
static void
raptor_parser_warning(raptor_parser* parser, const char *message, ...)
{
  va_list arguments;

  va_start(arguments, message);

  if(parser->warning_handler) {
    parser->warning_handler(parser->warning_user_data,
                            &parser->locator, message, arguments);
    return;
  }

  raptor_print_locator(stderr, &parser->locator);
  fprintf(stderr, " raptor warning - ");
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);

  va_end(arguments);
}



/* PUBLIC FUNCTIONS */

/**
 * raptor_new - Initialise the Raptor RDF parser
 *
 * Return value: non 0 on failure
 **/
raptor_parser*
raptor_new(
#ifdef LIBRDF_INTERNAL
  librdf_world *world
#else
  void
#endif
)
{
  raptor_parser* rdf_parser;
#ifdef NEED_EXPAT
  XML_Parser xp;
#endif

  rdf_parser=(raptor_parser*)LIBRDF_CALLOC(raptor_parser, 1, sizeof(raptor_parser));

  if(!rdf_parser)
    return NULL;

  /* Initialise default feature values */
  rdf_parser->feature_scanning_for_rdf_RDF=0;
  rdf_parser->feature_allow_non_ns_attributes=1;
  rdf_parser->feature_allow_other_parseTypes=1;

#ifdef NEED_EXPAT
  xp=XML_ParserCreate(NULL);

  /* create a new parser in the specified encoding */
  XML_SetUserData(xp, rdf_parser);

  /* XML_SetEncoding(xp, "..."); */

  XML_SetElementHandler(xp, raptor_xml_start_element_handler,
                        raptor_xml_end_element_handler);
  XML_SetCharacterDataHandler(xp, raptor_xml_cdata_handler);

  XML_SetUnparsedEntityDeclHandler(xp,
                                   raptor_xml_unparsed_entity_decl_handler);

  XML_SetExternalEntityRefHandler(xp,
                                  raptor_xml_external_entity_ref_handler);


#ifdef HAVE_XML_SetNamespaceDeclHandler
  XML_SetNamespaceDeclHandler(xp,
                              raptor_start_namespace_decl_handler,
                              raptor_end_namespace_decl_handler);
#endif
  rdf_parser->xp=xp;
#endif

#ifdef NEED_LIBXML
  xmlDefaultSAXHandlerInit();
  rdf_parser->sax.startElement=raptor_xml_start_element_handler;
  rdf_parser->sax.endElement=raptor_xml_end_element_handler;

  rdf_parser->sax.characters=raptor_xml_cdata_handler;
  rdf_parser->sax.ignorableWhitespace=raptor_xml_cdata_handler;

  rdf_parser->sax.warning=raptor_xml_warning;
  rdf_parser->sax.error=raptor_xml_error;
  rdf_parser->sax.fatalError=raptor_xml_fatal_error;

  rdf_parser->sax.setDocumentLocator=raptor_xml_set_document_locator;

  /* xmlInitParserCtxt(&rdf_parser->xc); */
#endif

#ifdef LIBRDF_INTERNAL
  rdf_parser->world=world;

  rdf_parser->raptor_daml_oil_uri=librdf_new_uri(world, "http://www.daml.org/2001/03/daml+oil#");
  rdf_parser->raptor_daml_List_uri=librdf_new_uri_from_uri_local_name(rdf_parser->raptor_daml_oil_uri, "List");
  rdf_parser->raptor_daml_first_uri=librdf_new_uri_from_uri_local_name(rdf_parser->raptor_daml_oil_uri, "first");
  rdf_parser->raptor_daml_rest_uri=librdf_new_uri_from_uri_local_name(rdf_parser->raptor_daml_oil_uri, "rest");
  rdf_parser->raptor_daml_nil_uri=librdf_new_uri_from_uri_local_name(rdf_parser->raptor_daml_oil_uri, "nil");

#endif

  raptor_init_namespaces(rdf_parser);

  return rdf_parser;
}




/**
 * raptor_free - Free the Raptor RDF parser
 * @rdf_parser: parser object
 * 
 **/
void
raptor_free(raptor_parser *rdf_parser) 
{
  raptor_element* element;
  raptor_ns_map* ns;

  ns=rdf_parser->namespaces;
  while(ns) {
    raptor_ns_map* next_ns=ns->next;

    raptor_free_namespace(rdf_parser, ns);
    ns=next_ns;
  }

  while((element=raptor_element_pop(rdf_parser))) {
    raptor_free_element(element);
  }

#ifdef LIBRDF_INTERNAL
  if(rdf_parser->raptor_daml_oil_uri)
    librdf_free_uri(rdf_parser->raptor_daml_oil_uri);
  if(rdf_parser->raptor_daml_List_uri)
    librdf_free_uri(rdf_parser->raptor_daml_List_uri);
  if(rdf_parser->raptor_daml_first_uri)
    librdf_free_uri(rdf_parser->raptor_daml_first_uri);
  if(rdf_parser->raptor_daml_rest_uri)
    librdf_free_uri(rdf_parser->raptor_daml_rest_uri);
  if(rdf_parser->raptor_daml_nil_uri)
    librdf_free_uri(rdf_parser->raptor_daml_nil_uri);
#endif

  LIBRDF_FREE(raptor_parser, rdf_parser);
}


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


void
raptor_set_statement_handler(raptor_parser* parser,
                          void *user_data, 
                          raptor_statement_handler handler)
{
  parser->user_data=user_data;
  parser->statement_handler=handler;
}





/**
 * raptor_parse_file - Retrieve the RDF/XML content at URI
 * @rdf_parser: parser
 * @uri: URI of RDF content
 * @base_uri: the base URI to use (or NULL if the same)
 * 
 * Return value: non 0 on failure
 **/
int
raptor_parse_file(raptor_parser* rdf_parser,  raptor_uri *uri,
                  raptor_uri *base_uri) 
{
#ifdef NEED_EXPAT
  XML_Parser xp;
#endif
#ifdef NEED_LIBXML
  /* parser context */
  xmlParserCtxtPtr xc=NULL;
#endif
#define RBS 1024
  FILE *fh;
  char buffer[RBS];
  int rc=1;
  int len;
  const char *filename;
  /* for storing error info */
  raptor_locator *locator=&rdf_parser->locator;

  /* initialise fields */
  rdf_parser->depth=0;
  rdf_parser->root_element= rdf_parser->current_element=NULL;
  rdf_parser->failed=0;

  rdf_parser->base_uri=base_uri;


#ifdef NEED_EXPAT
  xp=rdf_parser->xp;

  XML_SetBase(xp, RAPTOR_URI_AS_STRING(base_uri));
#endif


#ifdef RAPTOR_URI_AS_FILENAME
  filename=RAPTOR_URI_AS_FILENAME(uri);
#else
  filename=RAPTOR_URI_TO_FILENAME(uri);
#endif
  if(!filename)
    return 1;

  locator->file=filename;
  locator->uri=base_uri;

  fh=fopen(filename, "r");
  if(!fh) {
    raptor_parser_error(rdf_parser, "file open failed - %s", strerror(errno));
#ifdef NEED_EXPAT
    XML_ParserFree(xp);
#endif /* EXPAT */
#ifdef RAPTOR_URI_TO_FILENAME
    LIBRDF_FREE(cstring, (void*)filename);
#endif
    return 1;
  }

#ifdef NEED_LIBXML
  /* libxml needs at least 4 bytes from the XML content to allow
   * content encoding detection to work */
  len=fread(buffer, 1, 4, fh);
  if(len>0) {
    xc = xmlCreatePushParserCtxt(&rdf_parser->sax, rdf_parser,
                                 buffer, len, filename);
    if(!xc) {
      fclose(fh);
#ifdef RAPTOR_URI_TO_FILENAME
      LIBRDF_FREE(cstring, (void*)filename);
#endif
      return 1;
    }
    xc->userData = rdf_parser;
    xc->vctxt.userData = rdf_parser;
    xc->vctxt.error=raptor_xml_validation_error;
    xc->vctxt.warning=raptor_xml_validation_warning;
    
    rdf_parser->xc = xc;
    
  } else {
    fclose(fh);
    fh=NULL;
  }

#endif

  while(fh && !feof(fh)) {
    len=fread(buffer, 1, RBS, fh);
    if(len <= 0) {
#ifdef NEED_EXPAT
      XML_Parse(xp, buffer, 0, 1);
#endif
#ifdef NEED_LIBXML
      xmlParseChunk(xc, buffer, 0, 1);
#endif
      break;
    }
#ifdef NEED_EXPAT
    rc=XML_Parse(xp, buffer, len, (len < RBS));
    if(len < RBS)
      break;
    if(!rc) /* expat: 0 is failure */
      break;
#endif
#ifdef NEED_LIBXML
    rc=xmlParseChunk(xc, buffer, len, 0);
    if(len < RBS)
      break;
    if(rc) /* libxml: non 0 is failure */
      break;
#endif
  }
  fclose(fh);


#ifdef NEED_EXPAT
  if(!rc) {
    int xe=XML_GetErrorCode(xp);

    locator->line=XML_GetCurrentLineNumber(xp);
    locator->column=XML_GetCurrentColumnNumber(xp);
    locator->byte=XML_GetCurrentByteIndex(xp);

    raptor_parser_error(rdf_parser, "XML Parsing failed - %s",
                        XML_ErrorString(xe));
    rc=1;
  } else
    rc=0;

  XML_ParserFree(xp);
#endif /* EXPAT */
#ifdef NEED_LIBXML
  if(rc)
    raptor_parser_error(rdf_parser, "XML Parsing failed");

  xmlFreeParserCtxt(xc);

#endif

#ifdef RAPTOR_URI_TO_FILENAME
  LIBRDF_FREE(cstring, (void*)filename);
#endif

  return (rc != 0);
}


void
raptor_print_locator(FILE *stream, raptor_locator* locator) 
{
  if(!locator)
    return;

  if(locator->uri)
    fprintf(stream, "URI %s", RAPTOR_URI_AS_STRING(locator->uri));
  else if (locator->file)
    fprintf(stream, "file %s", locator->file);
  else
    return;
  if(locator->line) {
    fprintf(stream, ":%d", locator->line);
    if(locator->column >= 0)
      fprintf(stream, " column %d", locator->column);
  }
}



void
raptor_set_feature(raptor_parser *parser, raptor_feature feature, int value) {
  switch(feature) {
    case RAPTOR_FEATURE_SCANNING:
      parser->feature_scanning_for_rdf_RDF=value;
      break;

    case RAPTOR_FEATURE_ALLOW_NON_NS_ATTRIBUTES:
      parser->feature_allow_non_ns_attributes=value;
      break;

    case RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES:
      parser->feature_allow_other_parseTypes=value;
      break;

    default:
      break;
  }
}


static void
raptor_generate_statement(raptor_parser *rdf_parser, 
                          raptor_uri *subject_uri,
                          const char *subject_id,
                          const raptor_identifier_type subject_type,
                          const raptor_uri_source subject_uri_source,
                          raptor_uri *predicate_uri,
                          const char *predicate_id,
                          const raptor_identifier_type predicate_type,
                          const raptor_uri_source predicate_uri_source,
                          raptor_uri *object_uri,
                          const char *object_id,
                          const raptor_identifier_type object_type,
                          const raptor_uri_source object_uri_source,
                          raptor_uri *bag)
{
  raptor_statement *statement=&rdf_parser->statement;

  statement->subject=subject_uri ? (void*)subject_uri : (void*)subject_id;
  statement->subject_type=subject_type;

  statement->predicate=predicate_uri ? (void*)predicate_uri : (void*)predicate_id;
  statement->predicate_type=predicate_type;

  statement->object=object_uri ? (void*)object_uri : (void*)object_id;
  statement->object_type=object_type;

#ifdef RAPTOR_DEBUG
  fprintf(stderr, "raptor_generate_statement: Generating statement: ");
  raptor_print_statement_detailed(statement, 1, stderr);
  fputc('\n', stderr);

  if(!(subject_uri||subject_id))
    LIBRDF_FATAL1(raptor_generate_statement, "Statement has no subject\n");
  
  if(!(predicate_uri||predicate_id))
    LIBRDF_FATAL1(raptor_generate_statement, "Statement has no predicate\n");
  
  if(!(object_uri||object_id))
    LIBRDF_FATAL1(raptor_generate_statement, "Statement has no object\n");
  
#endif

  if(!rdf_parser->statement_handler)
    return;

  /* Generate the statement; or is it fact? */
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);

  if(!bag)
    return;

  /* generate reified statements */
  statement->subject_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
  statement->predicate_type=RAPTOR_IDENTIFIER_TYPE_PREDICATE;

  statement->subject=bag;
  statement->object_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;

  statement->predicate=RAPTOR_RDF_type_URI;
  statement->object=RAPTOR_RDF_Statement_URI;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);

  statement->predicate=RAPTOR_RDF_subject_URI;
  statement->object=subject_uri ? (void*)subject_uri : (void*)subject_id;
  statement->object_type=subject_type;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);

  statement->predicate=RAPTOR_RDF_predicate_URI;
  statement->object=predicate_uri ? (void*)predicate_uri : (void*)predicate_id;
  statement->object_type=(predicate_type == RAPTOR_IDENTIFIER_TYPE_PREDICATE) ? RAPTOR_IDENTIFIER_TYPE_RESOURCE : predicate_type;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);

  statement->predicate=RAPTOR_RDF_object_URI;
  statement->object=object_uri ? (void*)object_uri : (void*)object_id;
  statement->object_type=object_type;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);
}



static void
raptor_print_statement_detailed(const raptor_statement * statement,
                                int detailed, FILE *stream) 
{
  if(statement->subject_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
    fprintf(stream, "[%s, ", (const char*)statement->subject);
  else {
#ifdef RAPTOR_DEBUG
    if(!statement->subject)
      LIBRDF_FATAL1(raptor_print_statement_detailed, "Statement has NULL subject URI\n");
#endif
    fprintf(stream, "[%s, ",
            RAPTOR_URI_AS_STRING((raptor_uri*)statement->subject));
  }

  if(statement->predicate_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL)
    fprintf(stream, "[rdf:_%d]", *((int*)statement->predicate));
  else {
#ifdef RAPTOR_DEBUG
    if(!statement->predicate)
      LIBRDF_FATAL1(raptor_print_statement_detailed, "Statement has NULL predicate URI\n");
#endif
    fputs(RAPTOR_URI_AS_STRING((raptor_uri*)statement->predicate), stream);
  }

  if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_LITERAL || 
     statement->object_type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL)
    fprintf(stream, ", \"%s\"]",  (const char*)statement->object);
  else if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
    fprintf(stream, ", %s]", (const char*)statement->object);
  else {
#ifdef RAPTOR_DEBUG
    if(!statement->object)
      LIBRDF_FATAL1(raptor_generate_statement, "Statement has NULL object URI\n");
#endif
    fprintf(stream, ", %s]", 
            RAPTOR_URI_AS_STRING((raptor_uri*)statement->object));
  }
}


void
raptor_print_statement(const raptor_statement * const statement, FILE *stream) 
{
  raptor_print_statement_detailed(statement, 0, stream);
}


void
raptor_print_statement_as_ntriples(const raptor_statement * statement,
                                   FILE *stream) 
{
  if(statement->subject_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
    fprintf(stream, "_:%s", (const char*)statement->subject);
  else
    fprintf(stream, "<%s>",
            RAPTOR_URI_AS_STRING((raptor_uri*)statement->subject));
  fputc(' ', stream);

  if(statement->predicate_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL)
    fprintf(stream, "<http://www.w3.org/1999/02/22-rdf-syntax-ns#_%d>",
            *((int*)statement->predicate));
  else /* must be URI */
    fprintf(stream, "<%s>",
            RAPTOR_URI_AS_STRING((raptor_uri*)statement->predicate));
  fputc(' ', stream);

  if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_LITERAL || 
     statement->object_type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL)
    fprintf(stream, "\"%s\"",  (const char*)statement->object);
  else if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
    fprintf(stream, "_:%s", (const char*)statement->object);
  else /* must be URI */
    fprintf(stream, "<%s>", 
            RAPTOR_URI_AS_STRING((raptor_uri*)statement->object));

  fputs(" .", stream);
}




static const char *
raptor_generate_id(raptor_parser *rdf_parser, const int id_for_bag)
{
  char *buffer;
  /* "genid" + min length 1 + \0 */
  int length=7;
  int id=++rdf_parser->genid;
  int tmpid=id;

  while(tmpid/=10)
    length++;
  buffer=(char*)LIBRDF_MALLOC(cstring, length);
  if(!buffer)
    return NULL;
  sprintf(buffer, "genid%d", id);

  return buffer;
}


static raptor_uri*
raptor_make_uri_from_id(raptor_uri *base_uri, const char *id) 
{
#ifdef LIBRDF_INTERNAL
  librdf_uri *new_uri;
  char *local_name;
  int len;
#else
  char *new_uri;
  int len;
  int base_uri_len=strlen(base_uri);
#endif

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  LIBRDF_DEBUG2(raptor_make_uri_from_id, "Using ID %s\n", id);
#endif

#ifdef LIBRDF_INTERNAL
  /* "#id\0" */
  len=1+strlen(id)+1;
  local_name=(char*)LIBRDF_MALLOC(cstring, len);
  if(!local_name)
    return NULL;
  *local_name='#';
  strcpy(local_name+1, id);
  new_uri=librdf_new_uri_relative_to_base(base_uri, local_name);
  LIBRDF_FREE(cstring, local_name);
  return new_uri;
#else
  len=base_uri_len+1+strlen(id)+1;
  new_uri=(char*)LIBRDF_MALLOC(cstring, len);
  if(!new_uri)
    return NULL;
  strncpy(new_uri, base_uri, base_uri_len);
  new_uri[base_uri_len]='#';
  strcpy(new_uri+base_uri_len+1, id);
  return new_uri;
#endif
}


raptor_uri*
raptor_make_uri(raptor_uri *base_uri, const char *uri_string) 
{
#ifdef LIBRDF_INTERNAL
#else
  char *new_uri;
  const char *p;
  int base_uri_len=strlen(base_uri);
#endif

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  LIBRDF_DEBUG3(raptor_make_uri, 
                "Using base URI %s and URI string '%s'\n", 
                base_uri, uri_string);
#endif

#ifdef LIBRDF_INTERNAL
  return librdf_new_uri_relative_to_base(base_uri, uri_string);
#else
  /* If URI string is empty, just copy base URI */
  if(!*uri_string) {
    new_uri=(char*)LIBRDF_MALLOC(cstring, base_uri_len+1);
    if(!new_uri)
      return NULL;
    strcpy(new_uri, base_uri);
    return new_uri;
  }

  /* If URI string is a fragment #foo, append to base URI */
  if(*uri_string == '#') {
    new_uri=(char*)LIBRDF_MALLOC(cstring, base_uri_len+1+strlen(uri_string)+1);
    if(!new_uri)
      return NULL;
    strncpy(new_uri, base_uri, base_uri_len);
    strcpy(new_uri+base_uri_len, uri_string);
    return new_uri;
  }

  /* If URI string is an absolute URI, just copy it */
  for(p=uri_string;*p; p++)
    if(!isalnum(*p))
       break;
  /* If first non-alphanumeric char is a ':' then probably a absolute URI
   * Need to check URI spec - FIXME
   */
  if(*p && *p == ':') {
    new_uri=(char*)LIBRDF_MALLOC(cstring, strlen(uri_string)+1);
    if(!new_uri)
      return NULL;
    strcpy(new_uri, uri_string);
    return new_uri;
  }

  /* Otherwise is a general URI relative to base URI */

  /* FIXME do this properly */

  /* Move p to the last /, : or # char in the base URI */
  for(p=base_uri+base_uri_len-1;
      p > base_uri && *p != '/' && *p != ':' && *p != '#' ;
      p--)
    ;

  new_uri=(char*)LIBRDF_MALLOC(cstring, (p-base_uri)+1+strlen(uri_string)+1);
  if(!new_uri)
    return NULL;
  strncpy(new_uri, base_uri, p-base_uri+1);
  strcpy(new_uri+(p-base_uri)+1, uri_string);
  return new_uri;
#endif

}


raptor_uri*
raptor_copy_uri(raptor_uri *uri) 
{
#ifdef LIBRDF_INTERNAL
  return librdf_new_uri_from_uri(uri);
#else
  char *new_uri;
  new_uri=(char*)LIBRDF_MALLOC(cstring, strlen(uri)+1);
  if(!new_uri)
    return NULL;
  strcpy(new_uri, uri);
  return new_uri;
#endif
}


/**
 * raptor_new_identifier - Constructor - create a raptor_identifier
 * @type: raptor_identifier_type of identifier
 * @uri: URI of identifier (if relevant)
 * @uri_source: raptor_uri_source of URI (if relevnant)
 * @id: string for ID or genid (if relevant)
 * 
 * Constructs a new identifier copying the URI, ID fields.
 * 
 * Return value: a new raptor_identifier object or NULL on failure
 **/
raptor_identifier*
raptor_new_identifier(raptor_identifier_type type,
                      raptor_uri *uri,
                      raptor_uri_source uri_source,
                      char *id)
{
  raptor_identifier *identifier;
  raptor_uri *new_uri=NULL;
  char *new_id=NULL;

  identifier=(raptor_identifier*)LIBRDF_CALLOC(raptor_identifier, 1,
                                                sizeof(raptor_identifier));
  if(!identifier)
    return NULL;

  if(uri) {
    new_uri=raptor_copy_uri(uri);
    if(!new_uri) {
      LIBRDF_FREE(raptor_identifier, identifier);
      return NULL;
    }
  }

  if(id) {
    int len=strlen(id);
    
    new_id=LIBRDF_MALLOC(cstring, len+1);
    if(!len) {
      if(new_uri)
        LIBRDF_FREE(cstring, new_uri);
      LIBRDF_FREE(raptor_identifier, identifier);
      return NULL;
    }
    strncpy(new_id, id, len+1);
  }
  

  identifier->is_malloced=1;
  raptor_init_identifier(identifier, type, new_uri, uri_source, new_id);
  return identifier;
}


/**
 * raptor_init_identifier - Initialise a pre-allocated raptor_identifier object
 * @identifier: Existing object
 * @type: raptor_identifier_type of identifier
 * @uri: URI of identifier (if relevant)
 * @uri_source: raptor_uri_source of URI (if relevnant)
 * @id: string for ID or genid (if relevant)
 * 
 * Fills in the fields of an existing allocated raptor_identifier object.
 * DOES NOT copy any of the option arguments.
 **/
void
raptor_init_identifier(raptor_identifier *identifier,
                       raptor_identifier_type type,
                       raptor_uri *uri,
                       raptor_uri_source uri_source,
                       char *id) 
{
  identifier->is_malloced=0;
  identifier->type=type;
  identifier->uri=uri;
  identifier->uri_source=uri_source;
  identifier->id=id;
}


/**
 * raptor_copy_identifier - Copy raptor_identifiers
 * @dest: destination &raptor_identifier (previously created)
 * @src:  source &raptor_identifier
 * 
 * Return value: Non 0 on failure
 **/
int
raptor_copy_identifier(raptor_identifier *dest, raptor_identifier *src)
{
  raptor_uri *new_uri=NULL;
  char *new_id=NULL;
  
  raptor_free_identifier(dest);
  raptor_init_identifier(dest, src->type, new_uri, src->uri_source, new_id);

  if(src->uri) {
    new_uri=raptor_copy_uri(src->uri);
    if(!new_uri)
      return 0;
  }

  if(src->id) {
    int len=strlen(src->id);
    
    new_id=LIBRDF_MALLOC(cstring, len+1);
    if(!len) {
      if(new_uri)
        LIBRDF_FREE(cstring, new_uri);
      return 0;
    }
    strncpy(new_id, src->id, len+1);
  }
  dest->uri=new_uri;
  dest->id=new_id;

  dest->type=src->type;
  dest->uri_source=src->uri_source;
  dest->ordinal=src->ordinal;

  return 0;
}


/**
 * raptor_free_identifier - Destructor - destroy a raptor_identifier object
 * @identifier: &raptor_identifier object
 * 
 * Does not free an object initialised by raptor_init_identifier()
 **/
void
raptor_free_identifier(raptor_identifier *identifier) 
{
  if(identifier->uri)
    RAPTOR_FREE_URI(identifier->uri);

  if(identifier->id)
    LIBRDF_FREE(cstring, (void*)identifier->id);

  if(identifier->is_malloced)
    LIBRDF_FREE(identifier, (void*)identifier);
}



/**
 * raptor_process_property_attributes: Process the property attributes for an element for a given resource
 * @rdf_parser: Raptor parser object
 * @attributes_element: element with the property attributes 
 * @resource_element: element that defines the resource URI 
 *                    subject_uri, subject_uri_source etc.
 * 
 **/
static void 
raptor_process_property_attributes(raptor_parser *rdf_parser, 
                                   raptor_element *attributes_element,
                                   raptor_element *resource_element)
{
  unsigned int i;

  /* Process attributes as propAttr* = * (propName="string")*
   */
  for(i=0; i<attributes_element->attribute_count; i++) {
    raptor_ns_name* attribute=attributes_element->attributes[i];
    const char *name=attribute->local_name;
    const char *value = attribute->value;
    
    /* Generate the property statement using one of these properties:
     * 1) rdf:li -> rdf:_n
     * 2) rdf:_n
     * 3) the URI from the attribute
     */
    if(attribute->nspace && attribute->nspace->is_rdf_ms) {
      /* is rdf: namespace */
      int ordinal=0;
        
      if(IS_RDF_MS_CONCEPT(attribute->local_name, attribute->uri, li)) {
        /* recognise rdf:li attribute */
        ordinal=++resource_element->last_ordinal;
      } else if(*name == '_') {
        /* recognise rdf:_ */
        name++;
        while(*name >= '0' && *name <= '9') {
          ordinal *= 10;
          ordinal += (*name++ - '0');
        }
        
        if(ordinal < 1) {
          raptor_parser_warning(rdf_parser, "Illegal ordinal value %d in attribute %s seen on container element %s.", ordinal, attribute->local_name, name);
        }
      }

      if(ordinal >= 1)
        /* Generate an ordinal property when there are no problems */
        raptor_generate_statement(rdf_parser, 
                                  resource_element->subject.uri,
                                  resource_element->subject.id,
                                  resource_element->subject.type,
                                  resource_element->subject.uri_source,
                                  
                                  (raptor_uri*)&ordinal,
                                  NULL,
                                  RAPTOR_IDENTIFIER_TYPE_ORDINAL,
                                  RAPTOR_URI_SOURCE_NOT_URI,
                                  
                                  (raptor_uri*)value,
                                  NULL,
                                  RAPTOR_IDENTIFIER_TYPE_LITERAL,
                                  RAPTOR_URI_SOURCE_NOT_URI,
                                  
                                  NULL);
    } else
      /* else not rdf: namespace - generate statement */
      raptor_generate_statement(rdf_parser, 
                                resource_element->subject.uri,
                                resource_element->subject.id,
                                resource_element->subject.type,
                                resource_element->subject.uri_source,

                                attribute->uri,
                                NULL,
                                RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                RAPTOR_URI_SOURCE_ATTRIBUTE,

                                (raptor_uri*)value,
                                NULL,
                                RAPTOR_IDENTIFIER_TYPE_LITERAL,
                                RAPTOR_URI_SOURCE_NOT_URI,

                                resource_element->bag.uri);

  } /* end for ... attributes */


  /* Handle rdf property attributes
   * (only rdf:type and rdf:value at present) 
   */
  for(i=0; i<= RDF_ATTR_LAST; i++) {
    const char *value=attributes_element->rdf_attr[i];
    int object_is_literal=(rdf_attr_info[i].type == RAPTOR_IDENTIFIER_TYPE_LITERAL);
    raptor_uri *property_uri, *object_uri;
    raptor_identifier_type object_type;
    
    if(!value || rdf_attr_info[i].type == RAPTOR_IDENTIFIER_TYPE_UNKNOWN)
      continue;

#ifdef LIBRDF_INTERNAL
    librdf_get_concept_by_name(rdf_parser->world, 1, rdf_attr_info[i].name,
                               &property_uri, NULL);
#else    
    property_uri=raptor_make_uri(RAPTOR_RDF_MS_URI, rdf_attr_info[i].name);
#endif
    
    object_uri=object_is_literal ? (raptor_uri*)value : raptor_make_uri(rdf_parser->base_uri, value);
    object_type=object_is_literal ? RAPTOR_IDENTIFIER_TYPE_LITERAL : RAPTOR_IDENTIFIER_TYPE_RESOURCE;
    
    raptor_generate_statement(rdf_parser, 
                              resource_element->subject.uri,
                              resource_element->subject.id,
                              resource_element->subject.type,
                              resource_element->subject.uri_source,
                              
                              property_uri,
                              NULL,
                              RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                              RAPTOR_URI_SOURCE_ATTRIBUTE,
                              
                              object_uri,
                              NULL,
                              object_type,
                              RAPTOR_URI_SOURCE_NOT_URI,
                              
                              resource_element->bag.uri);
    if(!object_is_literal)
      RAPTOR_FREE_URI(object_uri);
#ifdef LIBRDF_INTERNAL
    /* noop */
#else    
    RAPTOR_FREE_URI(property_uri);
#endif
    
  } /* end for rdf:property values */

}


static void
raptor_start_element_grammar(raptor_parser *rdf_parser,
                             raptor_element *element) 
{
  int finished;
  raptor_state state;
  const char *el_name=element->name->local_name;
  int element_in_rdf_ns=(element->name->nspace && 
                         element->name->nspace->is_rdf_ms);


  state=element->state;
  LIBRDF_DEBUG3(raptor_start_element_grammar, "Starting in state %d - %s\n",
                state, raptor_state_as_string(state));

  finished= 0;
  while(!finished) {
    switch(state) {
      case RAPTOR_STATE_SKIPPING:
        element->child_state=state;
        finished=1;
        break;
        
      case RAPTOR_STATE_UNKNOWN:
        /* found <rdf:RDF> ? */
        if(element_in_rdf_ns) {
          if(IS_RDF_MS_CONCEPT(el_name, element->name->uri, RDF)) {
            element->child_state=RAPTOR_STATE_OBJ;
            /* Yes - need more content before can continue,
             * so wait for another element
             */
            finished=1;
            break;
          }
          if(IS_RDF_MS_CONCEPT(el_name, element->name->uri, Description)) {
            state=RAPTOR_STATE_OBJ;
            /* Yes - found something so move immediately to obj */
            break;
          }
        }

        /* If scanning for element, can continue */
        if(rdf_parser->feature_scanning_for_rdf_RDF) {
          finished=1;
          break;
        }

        /* Otherwise the choice of the next state can be made
         * from the current element by the IN_RDF state
         */
        state=RAPTOR_STATE_OBJ;
        break;


      case RAPTOR_STATE_OBJ:
        /* Handling either 6.2 (obj) or 6.17 (value) (inside
         * rdf:RDF, propertyElt) and expecting
         * description (6.3) | sequence (6.25) | bag (6.26) | alternative (6.27)
         * [|other container (not M&S)]
         *
         * Everything goes to typedNode (6.13) now
         */

        if(element_in_rdf_ns) {
          if(IS_RDF_MS_CONCEPT(el_name, element->name->uri, Description)) {
            state=RAPTOR_STATE_DESCRIPTION;
            break;
          } 

          if(IS_RDF_MS_CONCEPT(el_name, element->name->uri, Seq)) {
            state=RAPTOR_STATE_TYPED_NODE;
            break;
          }

          if(IS_RDF_MS_CONCEPT(el_name, element->name->uri, Bag)) {
            state=RAPTOR_STATE_TYPED_NODE;
            break;
          }

          if(IS_RDF_MS_CONCEPT(el_name, element->name->uri, Alt)) {
            state=RAPTOR_STATE_TYPED_NODE;
            break;
          }

          if(rdf_parser->container_test_handler) {
            raptor_uri *container_type=rdf_parser->container_test_handler(element->name->uri);
            if(container_type) {
              state=RAPTOR_STATE_TYPED_NODE;
              break;
            }
          }

          /* Other rdf: elements at this layer are taken as matching
           * the typedNode production - just fallthrough
           */
        }

        /* Default to typedNode */
        state=RAPTOR_STATE_TYPED_NODE;
        break;


      /* No need for 6.2 - already chose 6.3, 6.25, 6.26 or 6.27 */


      case RAPTOR_STATE_DESCRIPTION:
      case RAPTOR_STATE_TYPED_NODE:
      case RAPTOR_STATE_PARSETYPE_RESOURCE:
      case RAPTOR_STATE_PARSETYPE_DAML_COLLECTION:
        /* Handling 6.3 (description), 6.13 (typedNode) or contents
         * of a property (propertyElt or member) with parseType="resource"
         *
         * 6.3 (description):
         * <rdf:Description idAboutAttr? bagIdAttr? propAttr* >
         *   Attributes: (ID|about|aboutEach|aboutEachPrefix)? bagID? propAttr*
         * (may have no content, that is tested in the end element code)
         *
         * 6.13 (typedNode):
         * haved fallen through and not chosen other productions -
         * 6.3, 6.25, 6.26, 6.27.
         * and handling rdf:Seq, rdf:Bag, rdf:Alt or other container
         * and this can be determined from the value of 
         * rdf_parser->typed_node_block_type
         *
         * Expect here from production 6.13 (typedNode)
         * <typeName idAboutAttr? bagIdAttr? propAttr* />
         *   Attributes: (ID|about|aboutEach|aboutEachPrefix)? bagID? propAttr*
         *
         * CHOICE (description): Create a bag here (always? even if
         * no bagId given?) FIXME - not decided / implemented yet.
         */


        /* lets add booleans - isn't C wonderful! */
        if((element->rdf_attr[RDF_ATTR_ID] != NULL) +
           (element->rdf_attr[RDF_ATTR_about] != NULL) +
           (element->rdf_attr[RDF_ATTR_aboutEach] != NULL) +
           (element->rdf_attr[RDF_ATTR_aboutEachPrefix] != NULL) > 1) {
          raptor_parser_warning(rdf_parser, "More than one of RDF ID, about, aboutEach or aboutEachPrefix attributes on element %s - from productions 6.5, 6.6, 6.7 and 6.8 expect at most one.", el_name);
        }

        if(element->rdf_attr[RDF_ATTR_ID]) {
          element->subject.id=(char*)element->rdf_attr[RDF_ATTR_ID];
          element->rdf_attr[RDF_ATTR_ID]=NULL;
          element->subject.uri=raptor_make_uri_from_id(rdf_parser->base_uri, element->subject.id);
          element->subject.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          element->subject.uri_source=RAPTOR_URI_SOURCE_ID;
        } else if (element->rdf_attr[RDF_ATTR_about]) {
          element->subject.uri=raptor_make_uri(rdf_parser->base_uri, element->rdf_attr[RDF_ATTR_about]);
          element->subject.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          element->subject.uri_source=RAPTOR_URI_SOURCE_URI;
        } else if (element->rdf_attr[RDF_ATTR_aboutEach]) {
          element->subject.uri=raptor_make_uri(rdf_parser->base_uri, element->rdf_attr[RDF_ATTR_aboutEach]);
          element->subject.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE_EACH;
          element->subject.uri_source=RAPTOR_URI_SOURCE_URI;
        } else if (element->rdf_attr[RDF_ATTR_aboutEachPrefix]) {
          element->subject.uri=raptor_make_uri(rdf_parser->base_uri, element->rdf_attr[RDF_ATTR_aboutEachPrefix]);
          element->subject.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE_EACH_PREFIX;
          element->subject.uri_source=RAPTOR_URI_SOURCE_URI;
        } else if (element->parent && 
                   (element->parent->object.uri || element->parent->object.id)) {
          /* copy from parent, it has a URI for us */
          raptor_copy_identifier(&element->subject, &element->parent->object);
        } else {
          element->subject.id=raptor_generate_id(rdf_parser, 0);
          element->subject.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
          element->subject.uri_source=RAPTOR_URI_SOURCE_GENERATED;
        }


        if(element->rdf_attr[RDF_ATTR_bagID]) {
          element->bag.id=(char*)element->rdf_attr[RDF_ATTR_bagID];
          element->rdf_attr[RDF_ATTR_bagID]=NULL;
          element->bag.uri=raptor_make_uri_from_id(rdf_parser->base_uri,  element->bag.id);
          element->bag.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          element->bag.uri_source=RAPTOR_URI_SOURCE_GENERATED;
        }


        if(element->parent) {

          /* In a rdf:parseType daml:collection the resources are appended
           * to the list at element->parent->tail_uri 
           */
          if (state == RAPTOR_STATE_PARSETYPE_DAML_COLLECTION) {
            const char * idList = raptor_generate_id(rdf_parser, 0);
            raptor_uri* uriList = raptor_make_uri_from_id(rdf_parser->base_uri, idList);
            LIBRDF_FREE(cstring, (char*)idList);
            
            /* <uriList> rdf:type daml:List */
            raptor_generate_statement(rdf_parser, 
                                      uriList,
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                      RAPTOR_URI_SOURCE_URI,

                                      RAPTOR_RDF_type_URI,
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                      RAPTOR_URI_SOURCE_URI,

                                      RAPTOR_DAML_LIST_URI(rdf_parser),
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                      RAPTOR_URI_SOURCE_URI,
                                      NULL);

            /* <uriList> daml:first <element->uri> */
            raptor_generate_statement(rdf_parser, 
                                      uriList,
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                      RAPTOR_URI_SOURCE_URI,

                                      RAPTOR_DAML_FIRST_URI(rdf_parser),
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                      RAPTOR_URI_SOURCE_URI,

                                      element->subject.uri,
                                      element->subject.id,
                                      element->subject.type,
                                      RAPTOR_URI_SOURCE_URI,
                                      NULL);
            
            /* If there is no daml:collection */
            if (!element->parent->tail_uri) {
              /* Free any existing object URI still around
               * I suspect this can never happen.
               */
              if(element->parent->object.uri) {
                abort();
                RAPTOR_FREE_URI(element->parent->object.uri);
              }
              
              element->parent->object.uri=raptor_copy_uri(uriList);
              element->parent->object.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
              element->parent->object.uri_source=RAPTOR_URI_SOURCE_URI;
            } else {
              /* <tail_uri> daml:rest <uriListRest> */
              raptor_generate_statement(rdf_parser, 
                                        element->parent->tail_uri,
                                        NULL,
                                        RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                        RAPTOR_URI_SOURCE_URI,

                                        RAPTOR_DAML_REST_URI(rdf_parser),
                                        NULL,
                                        RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                        RAPTOR_URI_SOURCE_URI,

                                        uriList,
                                        NULL,
                                        RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                        RAPTOR_URI_SOURCE_URI,

                                        NULL);
            }

            /* update new tail */
            if(element->parent->tail_uri)
              RAPTOR_FREE_URI(element->parent->tail_uri);

            element->parent->tail_uri=uriList;
            
          } else if(!element->parent->object.uri &&
                    element->parent->state != RAPTOR_STATE_UNKNOWN) {
            /* If there is a parent element (property) containing this
             * element (node) and it has no object, set it from this subject
             */
            
            /* Store URI of this node in our parent as the property object */
            raptor_copy_identifier(&element->parent->object, &element->subject);
            element->parent->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
          }
        }
        

        /* If this is a typed node, generate the rdf:type statement
         * from this node
         */
        if(state == RAPTOR_STATE_TYPED_NODE)
          raptor_generate_statement(rdf_parser, 
                                    element->subject.uri,
                                    element->subject.id,
                                    element->subject.type,
                                    element->subject.uri_source,

                                    RAPTOR_RDF_type_URI,
                                    NULL,
                                    RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                    RAPTOR_URI_SOURCE_URI,

                                    element->name->uri,  /* FIXME: element->container_type, ??? */
                                    NULL,
                                    RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                    element->object.uri_source,

                                    element->bag.uri);

        raptor_process_property_attributes(rdf_parser, element, element);

        /* for both productions now need some more content or
         * property elements before can do any more work.
         */

        element->child_state=RAPTOR_STATE_PROPERTYELT;
        element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTIES;
        finished=1;
        break;


      case RAPTOR_STATE_PARSETYPE_OTHER:
        /* FIXME */

        /* FALLTHROUGH */

      case RAPTOR_STATE_PARSETYPE_LITERAL:
        {
          char *fmt_buffer;
          int fmt_length;
          fmt_buffer=raptor_format_element(element, &fmt_length, 0);
          if(fmt_buffer && fmt_length) {
            /* Append cdata content content */
            if(element->content_cdata) {
              /* Append */
              char *new_cdata=(char*)LIBRDF_MALLOC(cstring, element->content_cdata_length + fmt_length + 1); /* DAML merge used 10 - why? */
              if(new_cdata) {
                strncpy(new_cdata, element->content_cdata,
                        element->content_cdata_length);
                strcpy(new_cdata+element->content_cdata_length, fmt_buffer);
                LIBRDF_FREE(cstring, element->content_cdata);
                element->content_cdata=new_cdata;
              }
              LIBRDF_FREE(cstring, fmt_buffer);

              LIBRDF_DEBUG3(raptor_start_element_grammar,
                            "content cdata appended, now: '%s' (%d bytes)\n", 
                            element->content_cdata,
                            element->content_cdata_length);

            } else {
              /* Copy - is empty */
              element->content_cdata       =fmt_buffer;
              element->content_cdata_length=fmt_length;

              LIBRDF_DEBUG3(raptor_start_element_grammar,
                            "content cdata copied, now: '%s' (%d bytes)\n", 
                            element->content_cdata,
                            element->content_cdata_length);

            }
          }

          element->child_state = RAPTOR_STATE_PARSETYPE_LITERAL;
        }
        
        finished=1;
        break;

        /* choices here from production 6.12 (propertyElt)
         *   <propName idAttr?> value </propName>
         *     Attributes: ID?
         *   <propName idAttr? parseLiteral> literal </propName>
         *     Attributes: ID? parseType="literal"
         *   <propName idAttr? parseResource> propertyElt* </propName>
         *     Attributes: ID? parseType="resource"
         *   <propName idRefAttr? bagIdAttr? propAttr* />
         *     Attributes: (ID|resource)? bagIdAttr? propAttr*
         *
         * The only one that must be handled here is the last one which
         * uses only attributes and no content.
         */
      case RAPTOR_STATE_MEMBER:
      case RAPTOR_STATE_PROPERTYELT:

        /* Handle rdf:li as a property in member state above */ 
        if(element_in_rdf_ns && 
           IS_RDF_MS_CONCEPT(el_name, element->name->uri, li)) {
          state=RAPTOR_STATE_MEMBER;
          break;
        }


        /* M&S says: "The value of the ID attribute, if specified, is
         * the identifier for the resource that represents the
         * reification of the statement." */

        /* Later on it implies something different
         * FIXME 1 - reference to this
         * FIXME 2 - pick one of these interpretations
         */

        element->bag.id=element->rdf_attr[RDF_ATTR_ID];
        element->rdf_attr[RDF_ATTR_ID]=NULL;

        if (element->rdf_attr[RDF_ATTR_parseType]) {
          const char *parse_type=element->rdf_attr[RDF_ATTR_parseType];

          if(!strcasecmp(parse_type, "literal")) {
            element->child_state=RAPTOR_STATE_PARSETYPE_LITERAL;
          } else if (!strcasecmp(parse_type, "resource")) {
            state=RAPTOR_STATE_PARSETYPE_RESOURCE;
            element->child_state=RAPTOR_STATE_PROPERTYELT;
            element->subject.id=raptor_generate_id(rdf_parser, 0);
            element->subject.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
            element->subject.uri_source=RAPTOR_URI_SOURCE_GENERATED;
          } else {
            if(rdf_parser->feature_allow_other_parseTypes) {
              if(!strcasecmp(parse_type, "daml:collection")) {
                element->child_state=RAPTOR_STATE_PARSETYPE_DAML_COLLECTION;
              } else {
                element->child_state=RAPTOR_STATE_PARSETYPE_OTHER;
              }
            } else {
              element->child_state=RAPTOR_STATE_PARSETYPE_LITERAL;
            }
          }
        } else {

          /* Can only be last case */
          if(element->rdf_attr[RDF_ATTR_resource]) {
            element->subject.uri=raptor_make_uri(rdf_parser->base_uri,
                                                 element->rdf_attr[RDF_ATTR_resource]);
            element->subject.uri_source=RAPTOR_URI_SOURCE_URI;
            element->subject.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          }
          

          /* FIXME - what should be done here */
#if 0
          if(element->rdf_attr[RDF_ATTR_bagID]) {
            element->bag.id=element->rdf_attr[RDF_ATTR_bagID];
            element->rdf_attr[RDF_ATTR_bagID]=NULL;
            element->bag.uri=raptor_make_uri_from_id(rdf_parser->base_uri,  element->bag.id);
            element->bag.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
            element->bag.uri_source=RAPTOR_URI_SOURCE_GENERATED;
          }
#endif
          
          /* Assign the properties on this element to the resource
           * URI held in our parent element
           */
          raptor_process_property_attributes(rdf_parser, element, 
                                             element->parent);

          /*
           * Assign bag URI here so we don't reify property attributes using 
           * this bagID 
           */
          if(element->bag.id) {
            element->bag.uri=raptor_make_uri_from_id(rdf_parser->base_uri,  element->bag.id);
            element->bag.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
            element->bag.uri_source=RAPTOR_URI_SOURCE_GENERATED;
          }

          if(element->rdf_attr[RDF_ATTR_resource]) {
            /* Done - wait for end of this element to end in order to 
             * check the element was empty as expected */
            element->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
          } else {
            /* Otherwise process content in obj (value) state */
            element->child_state=RAPTOR_STATE_OBJ;
            element->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_UNKNOWN;
          }
        }

        finished=1;

        break;


      default:
        raptor_parser_fatal_error(rdf_parser, "raptor_start_element_grammar - unexpected parser state %d - %s\n", state, raptor_state_as_string(state));
        finished=1;

    } /* end switch */

    if(state != element->state) {
      element->state=state;
      LIBRDF_DEBUG3(raptor_start_element_grammar, 
                    "Moved to state %d - %s\n",
                    state, raptor_state_as_string(state));
    }

  } /* end while */

  LIBRDF_DEBUG3(raptor_start_element_grammar, 
                "Ending in state %d - %s\n",
                state, raptor_state_as_string(state));
}


static void
raptor_end_element_grammar(raptor_parser *rdf_parser,
                           raptor_element *element) 
{
  raptor_state state;
  int finished;
  const char *el_name=element->name->local_name;
  int element_in_rdf_ns=(element->name->nspace && 
                         element->name->nspace->is_rdf_ms);


  state=element->state;
  LIBRDF_DEBUG3(raptor_end_element_grammar, "Starting in state %d - %s\n",
                state, raptor_state_as_string(state));

  finished= 0;
  while(!finished) {
    switch(state) {
      case RAPTOR_STATE_SKIPPING:
        finished=1;
        break;

      case RAPTOR_STATE_UNKNOWN:
        finished=1;
        break;

      case RAPTOR_STATE_OBJ:
        if(element_in_rdf_ns && 
          IS_RDF_MS_CONCEPT(el_name, element->name->uri,RDF)) {
          /* end of RDF - boo hoo */
          state=RAPTOR_STATE_UNKNOWN;
          finished=1;
          break;
        }
        /* When scanning, another element ending is outside the RDF
         * world so this can happen without further work
         */
        if(rdf_parser->feature_scanning_for_rdf_RDF) {
          state=RAPTOR_STATE_UNKNOWN;
          finished=1;
          break;
        }
        /* otherwise found some junk after RDF content in an RDF-only 
         * document (probably never get here since this would be
         * a mismatched XML tag and cause an error earlier)
         */
        raptor_parser_warning(rdf_parser, "Element %s ended, expected end of RDF element\n", el_name);
        state=RAPTOR_STATE_UNKNOWN;
        finished=1;
        break;

      /* No need for 6.2 - already chose 6.3, 6.25, 6.26 or 6.27 */

      case RAPTOR_STATE_DESCRIPTION:
      case RAPTOR_STATE_TYPED_NODE:
      case RAPTOR_STATE_PARSETYPE_RESOURCE:

        /* If there is a parent element containing this element and
         * the parent isn't a description, has an identifier,
         * create the statement between this node using parent property
         * (Need to check for identifier so that top-level typed nodes
         * don't get connect to <rdf:RDF> parent element)
         */
        if(state != RAPTOR_STATE_DESCRIPTION && 
           element->parent &&
           (element->parent->subject.uri || element->parent->subject.id))
          raptor_generate_statement(rdf_parser, 
                                    element->parent->subject.uri,
                                    element->parent->subject.id,
                                    element->parent->subject.type,
                                    element->parent->subject.uri_source,

                                    element->parent->name->uri,
                                    NULL,
                                    RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                    RAPTOR_URI_SOURCE_ELEMENT,

                                    element->subject.uri,
                                    element->subject.id,
                                    element->subject.type,
                                    element->subject.uri_source,

                                    NULL);

        finished=1;
        break;

      case RAPTOR_STATE_PARSETYPE_DAML_COLLECTION:

        finished=1;
        break;

      case RAPTOR_STATE_PARSETYPE_OTHER:
        /* FIXME */

        /* FALLTHROUGH */

      case RAPTOR_STATE_PARSETYPE_LITERAL:
        element->parent->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PRESERVED;

        /* Append closing element */
        {
          char *fmt_buffer;
          int fmt_length;
          fmt_buffer=raptor_format_element(element, &fmt_length,1);
          if(fmt_buffer && fmt_length) {
            /* Append cdata content content */
            char *new_cdata=(char*)LIBRDF_MALLOC(cstring, element->content_cdata_length + fmt_length + 1);
            if(new_cdata) {
              strncpy(new_cdata, element->content_cdata,
                      element->content_cdata_length); /* DAML merge had +1 - why? */
              strcpy(new_cdata+element->content_cdata_length, fmt_buffer);
              LIBRDF_FREE(cstring, element->content_cdata);
              element->content_cdata=new_cdata;
              element->content_cdata_length += fmt_length;
            }
            LIBRDF_FREE(cstring, fmt_buffer);
          }
        }
        
        LIBRDF_DEBUG3(raptor_end_element_grammar,
                      "content cdata now: '%s' (%d bytes)\n", 
                       element->content_cdata, element->content_cdata_length);
         
        /* Append this cdata content to parent element cdata content */
        if(element->parent->content_cdata) {
          /* Append */
          char *new_cdata=(char*)LIBRDF_MALLOC(cstring, element->parent->content_cdata_length + element->content_cdata_length + 1);
          if(new_cdata) {
            strncpy(new_cdata, element->parent->content_cdata,
                    element->parent->content_cdata_length);
            strncpy(new_cdata+element->parent->content_cdata_length, 
                    element->content_cdata, element->content_cdata_length+1);
            LIBRDF_FREE(cstring, element->parent->content_cdata);
            element->parent->content_cdata=new_cdata;
            element->parent->content_cdata_length += element->content_cdata_length;
            /* Done with our cdata - free it before pointer is zapped */
            LIBRDF_FREE(cstring, element->content_cdata);
          }

          LIBRDF_DEBUG3(raptor_end_element_grammar,
                        "content cdata appended to parent, now: '%s' (%d bytes)\n", 
                        element->parent->content_cdata,
                        element->parent->content_cdata_length);

        } else {
          /* Copy - parent is empty */
          element->parent->content_cdata       =element->content_cdata;
          element->parent->content_cdata_length=element->content_cdata_length+1;
          LIBRDF_DEBUG3(raptor_end_element_grammar,
                        "content cdata copied to parent, now: '%s' (%d bytes)\n",
                        element->parent->content_cdata,
                        element->parent->content_cdata_length);

        }

        element->content_cdata=NULL;
        element->content_cdata_length=0;

        finished=1;
        break;


      case RAPTOR_STATE_PROPERTYELT:
      case RAPTOR_STATE_MEMBER:
        /* This is used inside these: productions
         *   6.12 (propertyElt) part 1 (element OR literal content),
         *                      part 4 (expect no content),
         *   6.30 (inlineItem) part 1 (element OR literal content)
         *
         * and similar parts of 6.28, 6.29 and 6.30 for rdf:li property
         *
         * The literal content part is handled here.
         * The element content is handled in the internal states
         * Empty content is checked here.
         */

        if(element->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_UNKNOWN) {
          if(element->content_cdata_seen) 
            element->content_type= RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL;
          else if (element->content_element_seen) 
            element->content_type= RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTIES;
          else
            element->content_type= RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
        }

        LIBRDF_DEBUG3(raptor_end_element_grammar,
                      "Content type %s (%d)\n", raptor_element_content_type_as_string(element->content_type), element->content_type);

        if(element->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE) {
          if(element->rdf_attr[RDF_ATTR_resource]) {
            element->object.uri=raptor_make_uri(rdf_parser->base_uri,
                                                 element->rdf_attr[RDF_ATTR_resource]);
            element->object.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
            element->object.uri_source=RAPTOR_URI_SOURCE_URI;
            element->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
          } else {
            element->object.id=raptor_generate_id(rdf_parser, 0);
            element->object.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
            element->object.uri_source=RAPTOR_URI_SOURCE_GENERATED;
            element->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
          }
        }


        /* If there is a parent resource node (Description,
         * typedNode or parseTypeResource)
         */
        if(element->parent->state == RAPTOR_STATE_DESCRIPTION ||
           element->parent->state == RAPTOR_STATE_TYPED_NODE ||
           element->parent->state == RAPTOR_STATE_PARSETYPE_RESOURCE) {
          int object_is_literal=(element->content_cdata != NULL); /* FIXME */
          raptor_identifier_type object_type=object_is_literal ? RAPTOR_IDENTIFIER_TYPE_LITERAL : element->object.type;
          raptor_uri *object_uri=object_is_literal ? (raptor_uri*)element->content_cdata : element->object.uri;

          /* then generate the statement:
           *   subject  : parent
           *   predicate: ordinal / this property
           *   object   : child object (node)
           */
          if(state == RAPTOR_STATE_MEMBER) {
            element->parent->last_ordinal++;
            raptor_generate_statement(rdf_parser, 
                                      element->parent->subject.uri,
                                      element->parent->subject.id,
                                      element->parent->subject.type,
                                      RAPTOR_URI_SOURCE_ELEMENT,

                                      (raptor_uri*)&element->parent->last_ordinal,
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_ORDINAL,
                                      RAPTOR_URI_SOURCE_NOT_URI,
                                      
                                      object_uri,
                                      element->object.id,
                                      object_type,
                                      element->object.uri_source,

                                      element->bag.uri);
          } else
            raptor_generate_statement(rdf_parser, 
                                      element->parent->subject.uri,
                                      element->parent->subject.id,
                                      element->parent->subject.type,
                                      RAPTOR_URI_SOURCE_ELEMENT,

                                      element->name->uri,
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                      RAPTOR_URI_SOURCE_ELEMENT,

                                      object_uri,
                                      element->object.id,
                                      object_type,
                                      element->object.uri_source,

                                      element->bag.uri);
          
        } else {
          int object_is_literal=(element->content_cdata != NULL); /* FIXME */
          raptor_uri *object_uri=object_is_literal ? (raptor_uri*)element->content_cdata : element->object.uri;
          raptor_identifier_type object_type=object_is_literal ? RAPTOR_IDENTIFIER_TYPE_LITERAL : element->object.type;

          /* Not child of Description, so process as child of propertyElt */
          switch(element->content_type) {
            case RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL:
              if(state == RAPTOR_STATE_MEMBER) {
                element->parent->last_ordinal++;
                raptor_generate_statement(rdf_parser, 
                                          element->parent->subject.uri,
                                          element->parent->subject.id,
                                          element->parent->subject.type,
                                          RAPTOR_URI_SOURCE_ELEMENT,

                                          (raptor_uri*)&element->parent->last_ordinal,
                                          NULL,
                                          RAPTOR_IDENTIFIER_TYPE_ORDINAL,
                                          RAPTOR_URI_SOURCE_NOT_URI,
                                          
                                          object_uri,
                                          element->object.id,
                                          object_type,
                                          element->object.uri_source,

                                          element->bag.uri);
              } else
                raptor_generate_statement(rdf_parser, 
                                          element->parent->subject.uri,
                                          element->parent->subject.id,
                                          element->parent->subject.type,
                                          RAPTOR_URI_SOURCE_ELEMENT,

                                          element->name->uri,
                                          NULL,
                                          RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                          RAPTOR_URI_SOURCE_NOT_URI,

                                          object_uri,
                                          element->object.id,
                                          object_type,
                                          element->object.uri_source,

                                          element->bag.uri);
              
              break;
              
            case RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE:
              /* empty property of form
               * <property rdf:resource="object-URI"/> 
               */
              if(state == RAPTOR_STATE_MEMBER) {
                element->parent->last_ordinal++;
                raptor_generate_statement(rdf_parser, 
                                          element->parent->subject.uri,
                                          element->parent->subject.id,
                                          element->parent->subject.type,
                                          RAPTOR_URI_SOURCE_ELEMENT,

                                          (raptor_uri*)&element->parent->last_ordinal,
                                          NULL,
                                          RAPTOR_IDENTIFIER_TYPE_ORDINAL,
                                          RAPTOR_URI_SOURCE_NOT_URI,

                                          element->subject.uri,
                                          element->subject.id,
                                          element->subject.type,
                                          element->subject.uri_source,

                                          element->bag.uri);
              } else
                raptor_generate_statement(rdf_parser, 
                                          element->parent->subject.uri,
                                          element->parent->subject.id,
                                          element->parent->subject.type,
                                          RAPTOR_URI_SOURCE_ELEMENT,

                                          element->name->uri,
                                          NULL,
                                          RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                          RAPTOR_URI_SOURCE_ELEMENT,

                                          element->subject.uri,
                                          element->subject.id,
                                          element->subject.type,
                                          element->subject.uri_source,

                                          element->bag.uri);
              break;

          case RAPTOR_ELEMENT_CONTENT_TYPE_PRESERVED:
              if(state == RAPTOR_STATE_MEMBER) {
                element->parent->last_ordinal++;
                raptor_generate_statement(rdf_parser, 
                                          element->parent->subject.uri,
                                          element->parent->subject.id,
                                          element->parent->subject.type,
                                          element->parent->subject.uri_source,

                                          (raptor_uri*)&element->parent->last_ordinal,
                                          NULL,
                                          RAPTOR_IDENTIFIER_TYPE_ORDINAL,
                                          RAPTOR_URI_SOURCE_NOT_URI,
                                          
                                          (raptor_uri*)element->content_cdata,
                                          NULL,
                                          RAPTOR_IDENTIFIER_TYPE_XML_LITERAL,
                                          RAPTOR_URI_SOURCE_NOT_URI,

                                          element->bag.uri);
              } else {
                raptor_generate_statement(rdf_parser, 
                                          element->parent->subject.uri,
                                          element->parent->subject.id,
                                          element->parent->subject.type,
                                          element->parent->subject.uri_source,
                                          
                                          element->name->uri,
                                          NULL,
                                          RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                          RAPTOR_URI_SOURCE_ELEMENT,

                                          (raptor_uri*)element->content_cdata,
                                          NULL,
                                          RAPTOR_IDENTIFIER_TYPE_XML_LITERAL,
                                          RAPTOR_URI_SOURCE_NOT_URI,

                                          element->bag.uri);
              }

            break;
            
            case RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION:
              if (!element->tail_uri) {
                /* If No List: set parents object to daml:nil */
                raptor_generate_statement(rdf_parser, 
                                          element->parent->subject.uri,
                                          element->parent->subject.id,
                                          element->parent->subject.type,
                                          RAPTOR_URI_SOURCE_ELEMENT,

                                          element->subject.uri,
                                          element->subject.id,
                                          RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                          RAPTOR_URI_SOURCE_URI,

                                          RAPTOR_DAML_NIL_URI(rdf_parser),
                                          NULL,
                                          RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                          RAPTOR_URI_SOURCE_URI,

                                          NULL);
              } else {
                /* If List: set parents object to the list */
                raptor_generate_statement(rdf_parser, 
                                          element->parent->subject.uri,
                                          element->parent->subject.id,
                                          element->parent->subject.type,
                                          RAPTOR_URI_SOURCE_ELEMENT,

                                          element->name->uri,
                                          NULL,
                                          RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                          RAPTOR_URI_SOURCE_ELEMENT,

                                          element->object.uri,
                                          element->object.id,
                                          element->object.type,
                                          RAPTOR_URI_SOURCE_URI,

                                          NULL);
                /* and terminate the list */
                raptor_generate_statement(rdf_parser, 
                                          element->tail_uri,
                                          NULL,
                                          RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                          RAPTOR_URI_SOURCE_URI,

                                          RAPTOR_DAML_REST_URI(rdf_parser),
                                          NULL,
                                          RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                          RAPTOR_URI_SOURCE_URI,

                                          RAPTOR_DAML_NIL_URI(rdf_parser),
                                          NULL,
                                          RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                          RAPTOR_URI_SOURCE_URI,

                                          NULL);
              }
              break;
              
            default:
              raptor_parser_fatal_error(rdf_parser, "raptor_end_element_grammar state RAPTOR_STATE_PROPERTYELT - unexpected content type %d\n", element->content_type);
          } /* end switch */
          
        }

        finished=1;
        break;


      default:
        raptor_parser_fatal_error(rdf_parser, "raptor_end_element_grammar - unexpected parser state %d - %s\n", state, raptor_state_as_string(state));
        finished=1;

    } /* end switch */

    if(state != element->state) {
      element->state=state;
      LIBRDF_DEBUG3(raptor_end_element_grammar, "Moved to state %d - %s\n",
                    state, raptor_state_as_string(state));
    }

  } /* end while */

  LIBRDF_DEBUG3(raptor_end_element_grammar, 
                "Ending in state %d - %s\n",
                state, raptor_state_as_string(state));

}
