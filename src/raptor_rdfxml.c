/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rapier_parse.c - Redland Parser for RDF (RAPIER)
 *
 * $Id$
 *
 * Copyright (C) 2000 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology, University of Bristol.
 *
 *    This package is Free Software available under either of two licenses
 *    (see FAQS.html to see why):
 * 
 * 1. The GNU Lesser General Public License (LGPL)
 * 
 *    See http://www.gnu.org/copyleft/lesser.html or COPYING.LIB for the
 *    full license text.
 *      _________________________________________________________________
 * 
 *      Copyright (C) 2000 David Beckett, Institute for Learning and
 *      Research Technology, University of Bristol. All Rights Reserved.
 * 
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public License
 *      as published by the Free Software Foundation; either version 2 of
 *      the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library; if not, write to the Free Software
 *      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 *      USA
 *      _________________________________________________________________
 * 
 *    NOTE - under Term 3 of the LGPL, you may choose to license the entire
 *    library under the GPL. See COPYING for the full license text.
 * 
 * 2. The Mozilla Public License
 * 
 *    See http://www.mozilla.org/MPL/MPL-1.1.html or MPL.html for the full
 *    license text.
 * 
 *    Under MPL section 13. I declare that all of the Covered Code is
 *    Multiple Licensed:
 *      _________________________________________________________________
 * 
 *      The contents of this file are subject to the Mozilla Public License
 *      version 1.1 (the "License"); you may not use this file except in
 *      compliance with the License. You may obtain a copy of the License
 *      at http://www.mozilla.org/MPL/
 * 
 *      Software distributed under the License is distributed on an "AS IS"
 *      basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 *      the License for the specific language governing rights and
 *      limitations under the License.
 * 
 *      The Initial Developer of the Original Code is David Beckett.
 *      Portions created by David Beckett are Copyright (C) 2000 David
 *      Beckett, Institute for Learning and Research Technology, University
 *      of Bristol. All Rights Reserved.
 * 
 *      Alternatively, the contents of this file may be used under the
 *      terms of the GNU Lesser General Public License, in which case the
 *      provisions of the LGPL License are applicable instead of those
 *      above. If you wish to allow use of your version of this file only
 *      under the terms of the LGPL License and not to allow others to use
 *      your version of this file under the MPL, indicate your decision by
 *      deleting the provisions above and replace them with the notice and
 *      other provisions required by the LGPL License. If you do not delete
 *      the provisions above, a recipient may use your version of this file
 *      under either the MPL or the LGPL License.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

extern int errno;

#define RAPIER_INTERNAL

#ifdef LIBRDF_INTERNAL
/* if inside Redland */

#ifdef RAPIER_DEBUG
#define LIBRDF_DEBUG 1
#endif

#include <librdf.h>

#include <rdf_parser.h>
#include <rdf_node.h>
#include <rdf_stream.h>
#include <rdf_statement.h>
#include <rdf_uri.h>

typedef librdf_uri rapier_uri;

#else
/* else standalone */

#define LIBRDF_MALLOC(type, size) malloc(size)
#define LIBRDF_CALLOC(type, size, count) calloc(size, count)
#define LIBRDF_FREE(type, ptr)   free((void*)ptr)

#ifdef RAPIER_DEBUG
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



typedef const char rapier_uri;

#endif


/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPIER_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#undef HAVE_STDLIB_H
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif


/* XML parser includes */
#ifdef NEED_EXPAT
#include <xmlparse.h>
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


/* Rapier includes */
#include <rapier.h>

/* Rapier structures */
/* namespace stack node */
typedef struct rapier_ns_map_s rapier_ns_map;

typedef enum {
  RAPIER_STATE_INVALID = 0,

  /* Not in RDF grammar yet - searching for a start element.
   * This can be <rdf:RDF> (goto 6.1) but since it is optional,
   * the start element can also be <Description> (goto 6.3), 
   * <rdf:Seq> (goto 6.25) <rdf:Bag> (goto 6.26) or <rdf:Alt> (goto 6.27)
   * OR from 6.3 can have ANY other element matching
   * typedNode (6.13) - goto 6.3
   * CHOICE: Search for <rdf:RDF> node before starting match
   * OR assume RDF content, hence go straight to production
   */
  RAPIER_STATE_UNKNOWN = 1000,

  /* No need for 6.1 - go straight to 6.2 */

  /* Met production 6.1 (RDF) <rdf:RDF> element or 6.17 (value) and expecting
   *   description (6.3) | container (6.4)
   * = description (6.3) | sequence (6.25) | bag (6.26) | alternative (6.27)
   */
  RAPIER_STATE_OBJ   = 6020,

  /* Met production 6.3 (description) <rdf:Description> element
   * OR 6.13 (typedNode)
   */
  RAPIER_STATE_DESCRIPTION = 6030,

  /* production 6.4 (container) - not used (pick one of sequence, bag,
   * alternative immediately in state 6.2
   */
  
  /* productions 6.5-6.11 are for attributes - not used */

  /* met production 6.12 (propertyElt)
   */
  RAPIER_STATE_PROPERTYELT = 6120,

  /* met production 6.13 (typedNode)
   */
  RAPIER_STATE_TYPED_NODE = 6130,


  /* productions 6.14-6.16 are for attributes - not used */

  /* production 6.17 (value) - not used */

  /* productions 6.18-6.24 are for attributes / values - not used */


  /* Met production 6.25 (sequence) <rdf:Seq> element seen. Goto 6.28  */
  RAPIER_STATE_SEQ = 6250,

  /* Met production 6.26 (bag) <rdf:Bag> element seen. Goto 6.28  */
  RAPIER_STATE_BAG = 6260,

  /* Met production 6.27 (alternative) <rdf:Alt> element seen. Goto 6.28 */
  RAPIER_STATE_ALT = 6270,

  /* Met production 6.28 (member) 
   * Now expect <rdf:li> element and if it empty, with resource attribute
   * goto 6.29 otherwise goto 6.30
   * CHOICE: Match rdf:resource/resource
   */
  RAPIER_STATE_MEMBER = 6280,

  /* met production 6.29 (referencedItem) after 
   * Found a container item with reference - <rdf:li (rdf:)resource=".."/> */
  RAPIER_STATE_REFERENCEDITEM = 6290,

  /* met production 6.30 (inlineItem) part 1 - plain container item */
  RAPIER_STATE_INLINEITEM = 6300,


  /* productions 6.31-6.33 are for attributes - not used */


  /* met production 6.30 (inlineItem) part 2 - container item with
   * rdf:parseType="literal" */
  RAPIER_STATE_PARSETYPE_LITERAL = 6400,

  /* met production 6.30 (inlineItem) part 3 - container item with 
   * rdf:parseType="literal" */
  RAPIER_STATE_PARSETYPE_RESOURCE = 6410,




  /* ******************************************************************* */
  /* Additional non-M&S states */

  /* Another kind of container, not Seq, Bag or Alt */
  RAPIER_STATE_CONTAINER = 6420,

  /* met production 6.30 (inlineItem) - container item 
   * with other rdf:parseType value */
  RAPIER_STATE_PARSETYPE_OTHER = 6430,


} rapier_state;


static const char * const rapier_state_names[]={
  NULL, /* No 6.0 */
  NULL, /* 6.1 not used */
  "object (6.2)",
  "description (6.3)",
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL, /* 6.4 - 6.11 not used */
  "propertyElt (6.12)",
  "typed_node (6.13)",
  NULL,NULL,NULL, NULL, NULL,NULL,NULL,NULL,NULL,NULL,NULL, /* 6.14 - 6.24 not used */
  "sequence (6.25)",
  "bag (6.26)",
  "alternative (6.27)",
  "member (6.28)",
  "referencedItem (6.29)",
  "inlineItem (6.30 part 1)",
  NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
  "parseTypeLiteral (6.30 part 2)",
  "parseTypeResource (6.30 part 3)",
  "container (not M&S)",
  "parseTypeOther (not M&S)",
};


static const char * rapier_state_as_string(rapier_state state) 
{
  int offset=(state - 6000)/10;
  if(state == RAPIER_STATE_UNKNOWN)
    return "UNKNOWN";
  if(offset<0 || offset > 43)
    return "INVALID";
  if(!rapier_state_names[offset])
    return "NOT-USED";
  return rapier_state_names[offset];
}


/* Forms:
 * 1) prefix=NULL uri=<URI>      - default namespace defined
 * 2) prefix=NULL, uri=NULL      - no default namespace
 * 3) prefix=<prefix>, uri=<URI> - regular pair defined <prefix>:<URI>
 */
struct rapier_ns_map_s {
  /* next down the stack, NULL at bottom */
  struct rapier_ns_map_s* next;
  /* NULL means is the default namespace */
  const char *prefix;
  /* needed to safely compare prefixed-names */
  int prefix_length;
  /* URI of namespace or NULL for default */
  rapier_uri *uri;
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
 * Rapier XML-namespaced name, for elements or attributes 
 */

/* There are three forms
 * namespace=NULL                                - un-namespaced name
 * namespace=defined, namespace->prefix=NULL     - (default ns) name
 * namespace=defined, namespace->prefix=defined  - ns:name
 */
typedef struct {
  /* Name - always present */
  const char *qname;
  /* Namespace or NULL if not in a namespace */
  const rapier_ns_map *namespace;
  /* URI of namespace+qname or NULL if not defined */
  rapier_uri *uri;
  /* optional value - used when name is an attribute */
  const char *value;
} rapier_ns_name;


/* These are used in the RDF/XML syntax as attributes, not
 * elements and are mostly not concepts in the RDF model (except for
 * the type attribute which is a property too).
 */
typedef enum {
  RDF_ATTR_about           = 0, /* value of rdf:about attribute */
  RDF_ATTR_aboutEach       = 1, /* " rdf:aboutEach       */
  RDF_ATTR_aboutEachPrefix = 2, /* " rdf:aboutEachPrefix */
  RDF_ATTR_ID              = 3, /* " rdf:ID */
  RDF_ATTR_bagID           = 4, /* " rdf:bagID */
  RDF_ATTR_resource        = 5, /* " rdf:resource */
  RDF_ATTR_type            = 6, /* " rdf:type -- IS a property in RDF Model */
  RDF_ATTR_parseType       = 7, /* " rdf:parseType */

  RDF_ATTR_LAST            = RDF_ATTR_parseType
} rdf_attr;

static const char * const rdf_attr_names[]={
  "about",
  "aboutEach",
  "aboutEachPrefix",
  "ID",
  "bagID",
  "resource",
  "type",
  "parseType",
};


/* For a typedNode production, what is the real production of the
 * block?  This can be typedNode (the default), sequence, bag, alternative
 * or (invented) container for other container types.
 */
typedef enum {
  RAPIER_TYPED_NODE_BLOCK_TYPE_TYPED_NODE, /* default */
  RAPIER_TYPED_NODE_BLOCK_TYPE_SEQ,
  RAPIER_TYPED_NODE_BLOCK_TYPE_BAG,
  RAPIER_TYPED_NODE_BLOCK_TYPE_ALT,
  RAPIER_TYPED_NODE_BLOCK_TYPE_CONTAINER
} rapier_typed_node_block_type;


typedef enum {
  /* undetermined yet - whitespace is stored */
  RAPIER_ELEMENT_CONTENT_TYPE_UNKNOWN,
  /* empty - no elements or cdata - whitespace is ignored */
  RAPIER_ELEMENT_CONTENT_TYPE_EMPTY,
  /* literal content (cdata) - whitespace is significant */
  RAPIER_ELEMENT_CONTENT_TYPE_LITERAL,
  /* properties (0+ elements) - whitespace is ignored,
   * any non-whitespace is error */
  RAPIER_ELEMENT_CONTENT_TYPE_PROPERTIES,
  /* resource (0 or 1 element) - whitespace is ignored,
   * any non-whitespace is error */
  RAPIER_ELEMENT_CONTENT_TYPE_RESOURCE,
  /* all content is preserved */
  RAPIER_ELEMENT_CONTENT_TYPE_PRESERVED
} rapier_element_content_type;


/*
 * Rapier Element/attributes on stack 
 */
struct rapier_element_s {
  /* NULL at bottom of stack */
  struct rapier_element_s *parent;
  rapier_ns_name *name;
  rapier_ns_name **attributes;
  unsigned int attribute_count;
  /* attributes declared in M&S */
  const char * rdf_attr[RDF_ATTR_LAST+1];
  /* how many of above seen */
  unsigned int rdf_attr_count;

  /* state that this production matches */
  rapier_state state;

  /* starting state for children of this element */
  rapier_state child_state;

  /* XML elements in RDF can contain EITHER cdata OR just sub-elements
   * Mixed content is not allowed 
   */

  rapier_element_content_type content_type;

  /* CDATA content of element and checks for mixed content */
  char *content_cdata;
  unsigned int content_cdata_length;
  /* how many cdata blocks seen */
  unsigned int content_cdata_seen;
  /* all cdata so far is whitespace */
  unsigned int content_cdata_all_whitespace;
  /* how many contained elements seen */
  unsigned int content_element_seen;

  /* Identifies for the resource described by this element ID and/or URI */
  const char *id;
  int id_is_generated;
  rapier_uri *uri;
  /* Type of uri */
  rapier_subject_type subject_type;

  /* ID of Bag */
  const char *bag_id;
  int bagid_is_generated;
  rapier_uri *bag_uri;

  /* URI of a contained object */
  rapier_uri *object_uri;

  /* URI of type of container */
  rapier_uri *container_type;

  /* last ordinal used, so initialising to 0 works, emitting rdF:_1 first */
  int last_ordinal;
};

typedef struct rapier_element_s rapier_element;


/*
 * Rapier parser object
 */
struct rapier_parser_s {
  /* XML parser specific stuff */
#ifdef NEED_EXPAT
  XML_Parser xp;
#endif
#ifdef NEED_LIBXML
  /* structure holding sax event handlers */
  xmlSAXHandler sax;
  /* parser context */
  xmlParserCtxtPtr xc;
#endif  

  /* element depth */
  int depth;

  /* stack of namespaces, most recently added at top */
  rapier_ns_map *namespaces;

  /* can be filled with error location information */
  rapier_locator locator;

  /* stack of elements - elements add after current_element */
  rapier_element *root_element;
  rapier_element *current_element;

  /* non 0 if parser had fatal error and cannot continue */
  int failed;

  /* typedNode block type */
  rapier_typed_node_block_type typed_node_block_type;

  /* generated ID counter */
  int genid;

  /* base URI of RDF/XML */
  rapier_uri *base_uri;


  /* FEATURE: 
   * non 0 if scanning for <rdf:RDF> element, else assume doc is RDF */
  int feature_scanning_for_rdf_RDF;

  /* FEATURE:
   * non 0 to allow non-namespaced resource, ID etc attributes
   * on RDF namespaced-elements
   */
  int feature_allow_non_ns_attributes;

  /* FEATURE:
   * non 0 to interpret Seq, Bag, Alt, other containers using the
   * typedNode production
   */
  int feature_interpret_containers_as_typedNode;

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

  rapier_message_handler fatal_error_handler;
  rapier_message_handler error_handler;
  rapier_message_handler warning_handler;

  rapier_container_test_handler container_test_handler;

  /* parser callbacks */
  rapier_statement_handler statement_handler;
};




/* static variables */

#ifdef LIBRDF_INTERNAL
#define RAPIER_RDF_type_URI LIBRDF_MS_type_URI
#define RAPIER_RDF_subject_URI LIBRDF_MS_subject_URI
#define RAPIER_RDF_predicate_URI LIBRDF_MS_predicate_URI
#define RAPIER_RDF_object_URI LIBRDF_MS_object_URI
#define RAPIER_RDF_Statement_URI LIBRDF_MS_Statement_URI

#define RAPIER_RDF_Seq_URI LIBRDF_MS_Seq_URI
#define RAPIER_RDF_Bag_URI LIBRDF_MS_Bag_URI
#define RAPIER_RDF_Alt_URI LIBRDF_MS_Alt_URI

#else

static const char * const rapier_rdf_ms_uri=RAPIER_RDF_MS_URI;
static const char * const rapier_rdf_schema_uri=RAPIER_RDF_SCHEMA_URI;
static const char * const rapier_rdf_type_uri="http://www.w3.org/1999/02/22-rdf-syntax-ns#type";
static const char * const rapier_rdf_subject_uri="http://www.w3.org/1999/02/22-rdf-syntax-ns#subject";
static const char * const rapier_rdf_predicate_uri="http://www.w3.org/1999/02/22-rdf-syntax-ns#predicate";
static const char * const rapier_rdf_object_uri="http://www.w3.org/1999/02/22-rdf-syntax-ns#object";
static const char * const rapier_rdf_Statement_uri="http://www.w3.org/1999/02/22-rdf-syntax-ns#Statement";

static const char * const rapier_rdf_Seq_uri="http://www.w3.org/1999/02/22-rdf-syntax-ns#Seq";
static const char * const rapier_rdf_Bag_uri="http://www.w3.org/1999/02/22-rdf-syntax-ns#Bag";
static const char * const rapier_rdf_Alt_uri="http://www.w3.org/1999/02/22-rdf-syntax-ns#Alt";
#define RAPIER_RDF_type_URI rapier_rdf_type_uri
#define RAPIER_RDF_subject_URI rapier_rdf_subject_uri
#define RAPIER_RDF_predicate_URI rapier_rdf_predicate_uri
#define RAPIER_RDF_object_URI rapier_rdf_object_uri
#define RAPIER_RDF_Statement_URI rapier_rdf_Statement_uri

#define RAPIER_RDF_Seq_URI rapier_rdf_Seq_uri
#define RAPIER_RDF_Bag_URI rapier_rdf_Bag_uri
#define RAPIER_RDF_Alt_URI rapier_rdf_Alt_uri
#endif


static const char * const rapier_xml_uri="http://www.w3.org/XML/1998/namespace";


/* Prototypes for common expat/libxml parsing event-handling functions */
static void rapier_xml_start_element_handler(void *user_data, const XML_Char *name, const XML_Char **atts);
static void rapier_xml_end_element_handler(void *user_data, const XML_Char *name);
/* s is not 0 terminated. */
static void rapier_xml_cdata_handler(void *user_data, const XML_Char *s, int len);

#ifdef HAVE_XML_SetNamespaceDeclHandler
static void rapier_start_namespace_decl_handler(void *user_data, const XML_Char *prefix, const XML_Char *uri);
static void rapier_end_namespace_decl_handler(void *user_data, const XML_Char *prefix);
#endif


/* libxml-only prototypes */
#ifdef NEED_LIBXML
static void rapier_xml_warning(void *context, const char *msg, ...);
static void rapier_xml_error(void *context, const char *msg, ...);
static void rapier_xml_fatal_error(void *context, const char *msg, ...);
static void rapier_xml_validation_error(void *context, const char *msg, ...);
static void rapier_xml_validation_warning(void *context, const char *msg, ...);
static void rapier_xml_set_document_locator (void *ctx, xmlSAXLocatorPtr loc);
#endif


/* Prototypes for local functions */
static char * rapier_file_uri_to_filename(const char *uri);
static void rapier_parser_fatal_error(rapier_parser* parser, const char *message, ...);
static void rapier_parser_error(rapier_parser* parser, const char *message, ...);
static void rapier_parser_warning(rapier_parser* parser, const char *message, ...);



/* prototypes for namespace and name/qname functions */
static void rapier_init_namespaces(rapier_parser *rdf_parser);
static void rapier_start_namespace(rapier_parser *rdf_parser, const char *prefix, const char *namespace, int depth);
static void rapier_free_namespace(rapier_parser *rdf_parser,  rapier_ns_map* namespace);
static void rapier_end_namespace(rapier_parser *rdf_parser, const char *prefix, const char *namespace);
static void rapier_end_namespaces_for_depth(rapier_parser *rdf_parser);
static rapier_ns_name* rapier_make_namespaced_name(rapier_parser *rdf_parser, const char *name, const char *value, int is_element);
#ifdef RAPIER_DEBUG
static void rapier_print_ns_name(FILE *stream, rapier_ns_name* name);
#endif
static void rapier_free_ns_name(rapier_ns_name* name);
static int rapier_ns_names_equal(rapier_ns_name *name1, rapier_ns_name *name2);


/* prototypes for element functions */
static rapier_element* rapier_element_pop(rapier_parser *rdf_parser);
static void rapier_element_push(rapier_parser *rdf_parser, rapier_element* element);
static void rapier_free_element(rapier_element *element);
#ifdef RAPIER_DEBUG
static void rapier_print_element(rapier_element *element, FILE* stream);
#endif


/* prototypes for grammar functions */
static void rapier_start_element_grammar(rapier_parser *parser, rapier_element *element);
static void rapier_end_element_grammar(rapier_parser *parser, rapier_element *element);


/* prototype for statement related functions */
static void rapier_generate_statement(rapier_parser *rdf_parser, const void *subject, const rapier_subject_type subject_type, const void *predicate, const rapier_predicate_type predicate_type, const void *object, const rapier_object_type object_type, rapier_uri *bag);
static void rapier_print_statement_detailed(const rapier_statement *statement, int detailed, FILE *stream);


#ifdef LIBRDF_INTERNAL
#define IS_RDF_MS_CONCEPT(name, uri, qname) librdf_uri_equals(uri, librdf_concept_uris[LIBRDF_CONCEPT_MS_##qname])
#define RAPIER_URI_AS_STRING(uri) (librdf_uri_as_string(uri))
#define RAPIER_FREE_URI(uri) librdf_free_uri(uri)
#else
#define IS_RDF_MS_CONCEPT(name, uri, qname) !strcmp(name, #qname)
#define RAPIER_URI_AS_STRING(uri) ((const char*)uri)
#define RAPIER_FREE_URI(uri) LIBRDF_FREE(cstring, uri)
#endif




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
rapier_init_namespaces(rapier_parser *rdf_parser) {
  /* defined at level -1 since always 'present' when inside the XML world */
  rapier_start_namespace(rdf_parser, "xml", rapier_xml_uri, -1);
}


static void
rapier_start_namespace(rapier_parser *rdf_parser, 
                       const char *prefix, const char *namespace, int depth)
{
  int prefix_length=0;
#ifdef LIBRDF_INTERNAL
#else
  int uri_length=0;
#endif
  int len;
  rapier_ns_map *map;
  void *p;

  LIBRDF_DEBUG4(rapier_start_namespace,
                "namespace prefix %s uri %s depth %d\n",
                prefix ? prefix : "(default)", namespace, depth);

  /* Convert an empty namespace string "" to a NULL pointer */
  if(!*namespace)
    namespace=NULL;

  len=sizeof(rapier_ns_map);
#ifdef LIBRDF_INTERNAL
#else
  if(namespace) {
    uri_length=strlen(namespace);
    len+=uri_length+1;
  }
#endif
  if(prefix) {
    prefix_length=strlen(prefix);
    len+=prefix_length+1;
  }

  /* Just one malloc for map structure + namespace (maybe) + prefix (maybe)*/
  map=(rapier_ns_map*)LIBRDF_CALLOC(rapier_ns_map, len, 1);
  if(!map) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  }

  p=(void*)map+sizeof(rapier_ns_map);
#ifdef LIBRDF_INTERNAL
  map->uri=librdf_new_uri(namespace);
  if(!map->uri) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
    LIBRDF_FREE(rapier_ns_map, map);
    return;
  }
#else
  if(namespace) {
    map->uri=strcpy((char*)p, namespace);
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
  if(namespace) {
#ifdef LIBRDF_INTERNAL
    if(librdf_uri_equals(map->uri, librdf_concept_ms_namespace_uri))
      map->is_rdf_ms=1;
    else if(librdf_uri_equals(map->uri, librdf_concept_schema_namespace_uri))
      map->is_rdf_schema=1;
#else
    if(!strcmp(namespace, rapier_rdf_ms_uri))
      map->is_rdf_ms=1;
    else if(!strcmp(namespace, rapier_rdf_schema_uri))
      map->is_rdf_schema=1;
#endif
  }

  if(rdf_parser->namespaces)
    map->next=rdf_parser->namespaces;
  rdf_parser->namespaces=map;
}


static void 
rapier_free_namespace(rapier_parser *rdf_parser,  rapier_ns_map* namespace)
{
#ifdef LIBRDF_INTERNAL
  if(namespace->uri)
    librdf_free_uri(namespace->uri);
#endif
  LIBRDF_FREE(rapier_ns_map, namespace);
}


static void 
rapier_end_namespace(rapier_parser *rdf_parser, 
                     const char *prefix, const char *namespace) 
{
  LIBRDF_DEBUG3(rapier_end_namespace, "prefix %s uri \"%s\"\n", 
                prefix ? prefix : "(default)", namespace);
}


static void 
rapier_end_namespaces_for_depth(rapier_parser *rdf_parser) 
{
  while(rdf_parser->namespaces &&
        rdf_parser->namespaces->depth == rdf_parser->depth) {
    rapier_ns_map* ns=rdf_parser->namespaces;
    rapier_ns_map* next=ns->next;

#ifdef LIBRDF_INTERNAL
    rapier_end_namespace(rdf_parser, ns->prefix, 
                         librdf_uri_as_string(ns->uri));
#else  
    rapier_end_namespace(rdf_parser, ns->prefix, ns->uri);
#endif
    rapier_free_namespace(rdf_parser, ns);

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

static rapier_ns_name*
rapier_make_namespaced_name(rapier_parser *rdf_parser, const char *name,
                            const char *value, int is_element) 
{
  rapier_ns_name* ns_name;
  const char *p;
  char *new_value=NULL;
  rapier_ns_map* ns;
  char* new_name;
  int prefix_length;
  int qname_length=0;

#if RAPIER_DEBUG > 1
  LIBRDF_DEBUG2(rapier_make_namespaced_name,
                "name %s\n", name);
#endif  

  ns_name=(rapier_ns_name*)LIBRDF_CALLOC(rapier_ns_name, sizeof(rapier_ns_name), 1);
  if(!ns_name) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
    return NULL;
  }

  if(value) {
    new_value=(char*)LIBRDF_MALLOC(cstring, strlen(value)+1);
    if(!new_value) {
      rapier_parser_fatal_error(rdf_parser, "Out of memory");
      LIBRDF_FREE(rapier_ns_name, ns_name);
      return NULL;
    } 
    strcpy(new_value, value);
    ns_name->value=new_value;
  }

  /* Find : */
  for(p=name; *p && *p != ':'; p++)
    ;

  if(!*p) {
    qname_length=p-name;

    /* No : - pick up default namespace, if there is one */
    new_name=(char*)LIBRDF_MALLOC(cstring, qname_length+1);
    if(!new_name) {
      rapier_parser_fatal_error(rdf_parser, "Out of memory");
      rapier_free_ns_name(ns_name);
      return NULL;
    }
    strcpy(new_name, name);
    ns_name->qname=new_name;

    /* Find a default namespace */
    for(ns=rdf_parser->namespaces; ns && ns->prefix; ns=ns->next)
      ;

    if(ns) {
      ns_name->namespace=ns;
#if RAPIER_DEBUG > 1
    LIBRDF_DEBUG2(rapier_make_namespaced_name,
                  "Found default namespace %s\n", ns->uri);
#endif
    } else {
      /* failed to find namespace - now what? FIXME */
      /* rapier_parser_warning(rdf_parser, "No default namespace defined - cannot expand %s", name); */
#if RAPIER_DEBUG > 1
    LIBRDF_DEBUG1(rapier_make_namespaced_name,
                  "No default namespace defined\n");
#endif
    }

  } else {
    /* There is a namespace prefix */

    prefix_length=p-name;
    p++; 

    /* p now is at start of qname */
    qname_length=strlen(p);
    new_name=(char*)LIBRDF_MALLOC(cstring, qname_length+1);
    if(!new_name) {
      rapier_parser_fatal_error(rdf_parser, "Out of memory");
      rapier_free_ns_name(ns_name);
      return NULL;
    }
    strcpy(new_name, p);
    ns_name->qname=new_name;

    /* Find the namespace */
    for(ns=rdf_parser->namespaces; ns ; ns=ns->next)
      if(ns->prefix && prefix_length == ns->prefix_length && 
         !strncmp(name, ns->prefix, prefix_length))
        break;

    if(!ns) {
      /* failed to find namespace - now what? */
      rapier_parser_error(rdf_parser, "Failed to find namespace in %s", name);
      rapier_free_ns_name(ns_name);
      return NULL;
    }

#if RAPIER_DEBUG > 1
    LIBRDF_DEBUG3(rapier_make_namespaced_name,
                  "Found namespace prefix %s URI %s\n", ns->prefix, ns->uri);
#endif
    ns_name->namespace=ns;
  }



  /* If namespace has a URI and a qname is defined, create the URI
   * for this element 
   */
  if(ns_name->namespace && ns_name->namespace->uri && qname_length) {
#ifdef LIBRDF_INTERNAL
    librdf_uri* uri=librdf_new_uri_from_uri_qname(ns_name->namespace->uri,
                                                  new_name);
    if(!uri) {
      rapier_free_ns_name(ns_name);
      return NULL;
    }
    ns_name->uri=uri;
#else
    char *uri_string=(char*)LIBRDF_MALLOC(cstring, 
                                          ns_name->namespace->uri_length + 
                                          qname_length + 1);
    if(!uri_string) {
      rapier_free_ns_name(ns_name);
      return NULL;
    }
    strcpy(uri_string, ns_name->namespace->uri);
    strcpy(uri_string+ns_name->namespace->uri_length, new_name);
    ns_name->uri=uri_string;
#endif
  }


  return ns_name;
}


#ifdef RAPIER_DEBUG
static void
rapier_print_ns_name(FILE *stream, rapier_ns_name* name) 
{
  if(name->namespace) {
    if(name->namespace->prefix)
      fprintf(stream, "%s:%s", name->namespace->prefix, name->qname);
    else
      fprintf(stream, "(default):%s", name->qname);
  } else
    fputs(name->qname, stream);
}
#endif

static void
rapier_free_ns_name(rapier_ns_name* name) 
{
  if(name->qname)
    LIBRDF_FREE(cstring, (void*)name->qname);

  if(name->uri)
    RAPIER_FREE_URI(name->uri);

  if(name->value)
    LIBRDF_FREE(cstring, (void*)name->value);
  LIBRDF_FREE(rapier_ns_name, name);
}


static int
rapier_ns_names_equal(rapier_ns_name *name1, rapier_ns_name *name2)
{
#ifdef LIBRDF_INTERNAL
  if(name1->uri && name2->uri)
    return librdf_uri_equals(name1->uri, name2->uri);
#else
  if(name1->namespace != name2->namespace)
    return 0;
#endif
  if(strcmp(name1->qname, name2->qname))
    return 0;
  return 1;
}


static rapier_element*
rapier_element_pop(rapier_parser *rdf_parser) 
{
  rapier_element *element=rdf_parser->current_element;

  if(!element)
    return NULL;

  rdf_parser->current_element=element->parent;
  if(rdf_parser->root_element == element) /* just deleted root */
    rdf_parser->root_element=NULL;

  return element;
}


static void
rapier_element_push(rapier_parser *rdf_parser, rapier_element* element) 
{
  element->parent=rdf_parser->current_element;
  rdf_parser->current_element=element;
  if(!rdf_parser->root_element)
    rdf_parser->root_element=element;
}


static void
rapier_free_element(rapier_element *element)
{
  int i;

  for (i=0; i < element->attribute_count; i++)
    if(element->attributes[i])
      rapier_free_ns_name(element->attributes[i]);

  if(element->attributes)
    LIBRDF_FREE(rapier_ns_name_array, element->attributes);

  /* Free special RDF M&S attributes */
  for(i=0; i<= RDF_ATTR_LAST; i++) 
    if(element->rdf_attr[i])
      LIBRDF_FREE(cstring, (void*)element->rdf_attr[i]);

  if(element->content_cdata_length)
    LIBRDF_FREE(rapier_ns_name_array, element->content_cdata);

  if(element->id_is_generated)
    LIBRDF_FREE(cstring, (char*)element->id);

  if(element->uri)
    RAPIER_FREE_URI(element->uri);

  if(element->bag_uri)
    RAPIER_FREE_URI(element->bag_uri);

  if(element->object_uri)
    RAPIER_FREE_URI(element->object_uri);

  rapier_free_ns_name(element->name);
  LIBRDF_FREE(rapier_element, element);
}



#ifdef RAPIER_DEBUG
static void
rapier_print_element(rapier_element *element, FILE* stream)
{
  int i;

  rapier_print_ns_name(stream, element->name);
  fputc('\n', stream);

  if(element->attribute_count) {
    fputs(" attributes: ", stream);
    for (i = 0; i < element->attribute_count; i++) {
      if(i)
        fputc(' ', stream);
      rapier_print_ns_name(stream, element->attributes[i]);
      fprintf(stream, "='%s'", element->attributes[i]->value);
    }
    fputc('\n', stream);
  }
}
#endif


static char *
rapier_format_element(rapier_element *element, int *length_p, int is_end)
{
  int length;
  int l;
  char *buffer;
  char *ptr;
  int i;

  /* get length of element name (and namespace-prefix: if there is one) */
  length=strlen(element->name->qname);
  if(element->name->namespace)
    length += element->name->namespace->prefix_length + 1;

  if(is_end)
    length++; /* / */

  if (!is_end && element->attributes) {
    /* space after element name */
    length++;
    
    for(i=0; element->attributes[i];) {
      if(!i)
        (length)++; /* ' ' between attributes */
      length += strlen(element->attributes[i]->qname) + 2; /* =" */
      if(element->attributes[i]->namespace)
        length += element->attributes[i]->namespace->prefix_length + 1;

      i++;
      length += strlen(element->attributes[i]->value) + 1; /* " */
      i++;
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
  if(element->name->namespace) {
    strncpy(ptr, element->name->namespace->prefix,
            element->name->namespace->prefix_length);
    ptr+= element->name->namespace->prefix_length;
    *ptr++=':';
  }
  strcpy(ptr, element->name->qname);
  ptr += strlen(element->name->qname);

  if(!is_end && element->attributes) {
    *ptr++ = ' ';
    for(i=0; element->attributes && element->attributes[i];) {
      if(!i)
        *ptr++ =' ';
      
      if(element->attributes[i]->namespace) {
        length += element->attributes[i]->namespace->prefix_length + 1;
        strncpy(ptr, element->attributes[i]->namespace->prefix,
                element->attributes[i]->namespace->prefix_length);
        ptr+= element->attributes[i]->namespace->prefix_length;
        *ptr++=':';
      }
    
      strcpy(ptr, element->attributes[i]->qname);
      ptr += strlen(element->attributes[i]->qname);
      
      *ptr++ ='=';
      *ptr++ ='"';
      
      i++;
      
      l=strlen(element->attributes[i]->value);
      strcpy(ptr, element->attributes[i]->value);
      ptr += l;
      *ptr++ ='"';
      
      i++;
    }
  }
  
  *ptr++ = '>';
  *ptr='\0';

  return buffer;
}



static void
rapier_xml_start_element_handler(void *user_data,
                                 const XML_Char *name, const XML_Char **atts)
{
  rapier_parser* rdf_parser=(rapier_parser*)user_data;
  int all_atts_count=0;
  int ns_attributes_count=0;
  rapier_ns_name** named_attrs=NULL;
  int i;
  rapier_ns_name* element_name;
  rapier_element* element=NULL;
#ifdef NEED_EXPAT
  /* for storing error info */
  rapier_locator *locator=&rdf_parser->locator; 
#endif

#ifdef RAPIER_DEBUG
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

        rapier_start_namespace(user_data, prefix, atts[i+1],
                               rdf_parser->depth);
        /* Is it ok to zap XML parser array things?
         * FIXME
         */
        atts[i]=NULL; 
        continue;
      }

      ns_attributes_count++;
    }
  }


  /* Now can recode element name with a namespace */

  element_name=rapier_make_namespaced_name(rdf_parser, name, NULL, 1);
  if(!element_name) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  }


  /* Create new element structure */
  element=(rapier_element*)LIBRDF_CALLOC(rapier_element, 
                                         sizeof(rapier_element), 1);
  if(!element) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
    rapier_free_ns_name(element_name);
    return;
  } 


  element->name=element_name;

  /* Prepare for possible element content */
  element->content_element_seen=0;
  element->content_cdata_seen=0;
  element->content_cdata_length=0;




  if(ns_attributes_count) {
    int offset = 0;

    /* Round 2 - turn attributes into namespaced-attributes */

    /* Allocate new array to hold namespaced-attributes */
    named_attrs=(rapier_ns_name**)LIBRDF_CALLOC(rapier_ns_name-array, sizeof(rapier_ns_name*), ns_attributes_count);
    if(!named_attrs) {
      rapier_parser_fatal_error(rdf_parser, "Out of memory");
      LIBRDF_FREE(rapier_element, element);
      rapier_free_ns_name(element_name);
      return;
    }

    for (i = 0; i < all_atts_count; i++) {
      rapier_ns_name* attribute;

      /* Skip previously processed attributes */
      if(!atts[i<<1])
        continue;

      /* namespace-name[i] stored in named_attrs[i] */
      attribute=rapier_make_namespaced_name(rdf_parser, atts[i<<1],
                                            atts[(i<<1)+1], 0);
      if(!attribute) { /* failed - tidy up and return */
        int j;

        for (j=0; j < i; j++)
          LIBRDF_FREE(rapier_ns_name, named_attrs[j]);
        LIBRDF_FREE(rapier_ns_name_array, named_attrs);
        LIBRDF_FREE(rapier_element, element);
        rapier_free_ns_name(element_name);
        return;
      }

      /* Save pointers to some RDF M&S attributes */

      /* If RDF M&S namespace-prefixed attributes */
      if(attribute->namespace && attribute->namespace->is_rdf_ms) {
        const char *attr_name=attribute->qname;
        int j;

        for(j=0; j<= RDF_ATTR_LAST; j++)
          if(!strcmp(attr_name, rdf_attr_names[j])) {
            element->rdf_attr[j]=attribute->value;
            element->rdf_attr_count++;
            /* Delete it if it was stored elsewhere */
#if RAPIER_DEBUG
            LIBRDF_DEBUG3(rapier_xml_start_element_handler,
                          "Found RDF M&S attribute %s URI %s\n",
                          attr_name, attribute->value);
#endif
            /* make sure value isn't deleted from ns_name structure */
            attribute->value=NULL;
            rapier_free_ns_name(attribute);
            attribute=NULL;
          }
      } /* end if RDF M&S namespaced-prefixed attributes */


      /* If non namespace-prefixed RDF M&S attributes found on an element */
      if(rdf_parser->feature_allow_non_ns_attributes && attribute) {
        const char *attr_name=attribute->qname;
        int j;

        for(j=0; j<= RDF_ATTR_LAST; j++)
          if(!strcmp(attr_name, rdf_attr_names[j])) {
            element->rdf_attr[j]=attribute->value;
            /* Delete it if it was stored elsewhere */
#if RAPIER_DEBUG
            LIBRDF_DEBUG3(rapier_xml_start_element_handler,
                          "Found non-namespaced RDF M&S attribute %s URI %s\n",
                          attr_name, attribute->value);
#endif
            /* make sure value isn't deleted from ns_name structure */
            attribute->value=NULL;
            rapier_free_ns_name(attribute);
            attribute=NULL;
        }
      } /* end if non-namespace prefixed RDF M&S attributes */


      if(attribute)
        named_attrs[offset++]=attribute;
    }

    /* set actual count from attributes that haven't been skipped */
    ns_attributes_count=offset;
    if(!offset && named_attrs) {
      /* all attributes were RDF M&S or other specials and deleted
       * so delete array and don't store pointer */
      LIBRDF_FREE(rapier_ns_name_array, named_attrs);
      named_attrs=NULL;
    }

  } /* end if ns_attributes_count */

  element->attributes=named_attrs;
  element->attribute_count=ns_attributes_count;


  rapier_element_push(rdf_parser, element);


  if(element->parent) {
    element->content_type=element->parent->content_type;

    element->parent->content_element_seen++;
    
    if(element->parent->content_element_seen == 1 &&
       element->parent->content_cdata_seen == 1) {
      /* Uh oh - mixed content, the parent element has cdata too */
      rapier_parser_warning(rdf_parser, "element %s has mixed content.", 
                            element->parent->name->qname);
    }
    
    /* If there is some existing all-whitespace content cdata
       * (which is probably before the first element) delete it
       */
    if(element->parent->content_element_seen &&
       element->parent->content_cdata_all_whitespace &&
       element->parent->content_cdata) {
      LIBRDF_FREE(rapier_ns_name_array, element->parent->content_cdata);
      element->parent->content_cdata=NULL;
      element->parent->content_cdata_length=0;
    }

  } /* end if element->parent */


#ifdef RAPIER_DEBUG
  fprintf(stderr, "rapier_xml_start_element_handler: Start ns-element: ");
  rapier_print_element(element, stderr);
#endif


  /* Right, now ready to enter the grammar */
  rapier_start_element_grammar(rdf_parser, element);

}


static void
rapier_xml_end_element_handler(void *user_data, const XML_Char *name)
{
  rapier_parser* rdf_parser=(rapier_parser*)user_data;
  rapier_element* element;
  rapier_ns_name *element_name;
#ifdef NEED_EXPAT
  /* for storing error info */
  rapier_locator *locator=&rdf_parser->locator;
#endif

#ifdef NEED_EXPAT
  locator->line=XML_GetCurrentLineNumber(rdf_parser->xp);
  locator->column=XML_GetCurrentColumnNumber(rdf_parser->xp);
  locator->byte=XML_GetCurrentByteIndex(rdf_parser->xp);
#endif

  /* recode element name */

  element_name=rapier_make_namespaced_name(rdf_parser, name, NULL, 1);
  if(!element_name) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  }


#ifdef RAPIER_DEBUG
  fprintf(stderr, "\nrapier_xml_end_element_handler: End ns-element: ");
  rapier_print_ns_name(stderr, element_name);
  fputc('\n', stderr);
#endif

  element=rapier_element_pop(rdf_parser);
  if(!rapier_ns_names_equal(element->name, element_name)) {
    /* Hmm, unexpected name - FIXME, should do something! */
    rapier_parser_warning(rdf_parser, 
                          "Element %s ended, expected end of element %s\n",
                          name, element->name->qname);
    return;
  }

  rapier_end_element_grammar(rdf_parser, element);

  rapier_free_ns_name(element_name);

  rapier_end_namespaces_for_depth(rdf_parser);

  rapier_free_element(element);

  rdf_parser->depth--;

#ifdef RAPIER_DEBUG
  fputc('\n', stderr);
#endif

}



/* cdata (and ignorable whitespace for libxml). 
 * s is not 0 terminated for expat, is for libxml - grrrr.
 */
static void
rapier_xml_cdata_handler(void *user_data, const XML_Char *s, int len)
{
  rapier_parser* rdf_parser=(rapier_parser*)user_data;
  rapier_element* element;
  rapier_state state;
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

  /* cdata never changes the parser state */

  state=element->state;
  LIBRDF_DEBUG3(rapier_xml_cdata_handler, "in state %d - %s\n", state,
                rapier_state_as_string(state));
  switch(state) {
    case RAPIER_STATE_UNKNOWN:
      /* Ignore all cdata if still looking for RDF */
      if(rdf_parser->feature_scanning_for_rdf_RDF)
        return;

      /* Ignore all whitespace cdata before first element */
      if(all_whitespace)
        return;

      /* This probably will never happen since that would make the
       * XML not be well-formed
       */
      rapier_parser_warning(rdf_parser, "Found cdata before RDF element.");
      break;

    case RAPIER_STATE_PARSETYPE_OTHER:
      /* FIXME */

      /* FALLTHROUGH */

    case RAPIER_STATE_PARSETYPE_LITERAL:
      element->content_type=RAPIER_ELEMENT_CONTENT_TYPE_LITERAL;
      break;

    default:
      break;
    } /* end switch */


  if(element->content_type != RAPIER_ELEMENT_CONTENT_TYPE_LITERAL &&
     element->content_type != RAPIER_ELEMENT_CONTENT_TYPE_PRESERVED) {

    /* Whitespace is ignored except for literal or preserved content types */
    if(all_whitespace) {
      LIBRDF_DEBUG2(rapier_xml_cdata_handler, "Ignoring whitespace cdata inside element %s\n", element->name->qname);
      return;
    }

    if(++element->content_cdata_seen == 1 &&
       element->content_element_seen == 1) {
      /* Uh oh - mixed content, this element has elements too */
      rapier_parser_warning(rdf_parser, "element %s has mixed content.", 
                            element->name->qname);
    }
  }


  buffer=(char*)LIBRDF_MALLOC(cstring, element->content_cdata_length + len + 1);
  if(!buffer) {
    rapier_parser_fatal_error(rdf_parser, "Out of memory");
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

  LIBRDF_DEBUG3(rapier_xml_cdata_handler, 
                "content cdata now: '%s' (%d bytes)\n", 
                buffer, element->content_cdata_length);

  LIBRDF_DEBUG3(rapier_xml_cdata_handler, 
                "ending in state %d - %s\n",
                state, rapier_state_as_string(state));

}


#ifdef HAVE_XML_SetNamespaceDeclHandler
static void
rapier_start_namespace_decl_handler(void *user_data,
                                    const XML_Char *prefix, const XML_Char *uri)
{
  rapier_parser* rdf_parser=(rapier_parser*)user_data;

#ifdef RAPIER_DEBUG
  fprintf(stderr_parser->locator, "saw namespace %s URI %s\n", prefix, uri);
#endif
}


static void
rapier_end_namespace_decl_handler(void *user_data, const XML_Char *prefix)
{
  rapier_parser* rdf_parser=(rapier_parser*)user_data;

#ifdef RAPIER_DEBUG
  fprintf(stderr_parser->locator, "saw end namespace prefix %s\n", prefix);
#endif
}
#endif


#ifdef NEED_EXPAT
/* This is called for a declaration of an unparsed (NDATA) entity */
static void
rapier_xml_unparsed_entity_decl_handler(void *user_data,
                                        const XML_Char *entityName,
                                        const XML_Char *base,
                                        const XML_Char *systemId,
                                        const XML_Char *publicId,
                                        const XML_Char *notationName) 
{
/*  rapier_parser* rdf_parser=(rapier_parser*)user_data; */
  fprintf(stderr,
          "rapier_xml_unparsed_entity_decl_handler: entityName %s base %s systemId %s publicId %s notationName %s\n",
          entityName, (base ? base : "(None)"), 
          systemId, (publicId ? publicId: "(None)"),
          notationName);
}


static int 
rapier_xml_external_entity_ref_handler(void *user_data,
                                       const XML_Char *context,
                                       const XML_Char *base,
                                       const XML_Char *systemId,
                                       const XML_Char *publicId)
{
/*  rapier_parser* rdf_parser=(rapier_parser*)user_data; */
  fprintf(stderr,
          "rapier_xml_external_entity_ref_handler: base %s systemId %s publicId %s\n",
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
rapier_xml_warning(void *ctx, const char *msg, ...) 
{
  va_list args;
  rapier_parser* rdf_parser=(rapier_parser*)ctx;
  int length;
  char *nmsg;

  va_start(args, msg);
  length=strlen(xml_warning_prefix)+strlen(msg)+1;
  nmsg=(char*)LIBRDF_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    rapier_parser_warning(rdf_parser, msg, args);
  } else {
    strcpy(nmsg, xml_warning_prefix);
    strcat(nmsg, msg);
    rapier_parser_warning(rdf_parser, nmsg, args);
    LIBRDF_FREE(cstring,nmsg);
  }
  va_end(args);
}


static void
rapier_xml_error(void *ctx, const char *msg, ...) 
{
  va_list args;
  rapier_parser* rdf_parser=(rapier_parser*)ctx;
  int length;
  char *nmsg;

  va_start(args, msg);
  length=strlen(xml_error_prefix)+strlen(msg)+1;
  nmsg=(char*)LIBRDF_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    rapier_parser_error(rdf_parser, nmsg, args);
  } else {
    strcpy(nmsg, xml_error_prefix);
    strcat(nmsg, msg);
    rapier_parser_error(rdf_parser, nmsg, args);
    LIBRDF_FREE(cstring,nmsg);
  }
  va_end(args);
}


static void
rapier_xml_fatal_error(void *ctx, const char *msg, ...) 
{
  va_list args;
  rapier_parser* rdf_parser=(rapier_parser*)ctx;
  int length;
  char *nmsg;

  va_start(args, msg);
  length=strlen(xml_fatal_error_prefix)+strlen(msg)+1;
  nmsg=(char*)LIBRDF_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    rapier_parser_fatal_error(rdf_parser, nmsg, args);
  } else {
    strcpy(nmsg, xml_error_prefix);
    strcat(nmsg, msg);
    rapier_parser_fatal_error(rdf_parser, nmsg, args);
    LIBRDF_FREE(cstring,nmsg);
  }
  va_end(args);
}


static void
rapier_xml_validation_error(void *ctx, const char *msg, ...) 
{
  va_list args;
  rapier_parser* rdf_parser=(rapier_parser*)ctx;
  int length;
  char *nmsg;

  va_start(args, msg);
  length=strlen(xml_validation_error_prefix)+strlen(msg)+1;
  nmsg=(char*)LIBRDF_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    rapier_parser_fatal_error(rdf_parser, nmsg, args);
  } else {
    strcpy(nmsg, xml_validation_error_prefix);
    strcat(nmsg, msg);
    rapier_parser_fatal_error(rdf_parser, nmsg, args);
    LIBRDF_FREE(cstring,nmsg);
  }
  va_end(args);
}


static void
rapier_xml_validation_warning(void *ctx, const char *msg, ...) 
{
  va_list args;
  rapier_parser* rdf_parser=(rapier_parser*)ctx;
  int length;
  char *nmsg;

  va_start(args, msg);
  length=strlen(xml_validation_warning_prefix)+strlen(msg)+1;
  nmsg=(char*)LIBRDF_MALLOC(cstring, length);
  if(!nmsg) {
    /* just pass on, might be out of memory error */
    rapier_parser_warning(rdf_parser, nmsg, args);
  } else {
    strcpy(nmsg, xml_validation_warning_prefix);
    strcat(nmsg, msg);
    rapier_parser_fatal_error(rdf_parser, nmsg, args);
    LIBRDF_FREE(cstring,nmsg);
  }
  va_end(args);
}


static void
rapier_xml_set_document_locator (void *ctx, xmlSAXLocatorPtr loc) 
{
  rapier_parser* rdf_parser=(rapier_parser*)ctx;
  /* for storing error info */
  rapier_locator *locator=&rdf_parser->locator;

  locator->line=loc->getLineNumber(rdf_parser->xc);
  locator->column=loc->getColumnNumber(rdf_parser->xc);
}
  

#endif



/**
 * rapier_file_uri_to_filename - Convert a URI representing a file (starting file:) to a filename
 * @uri: URI of string
 * 
 * Return value: the filename or NULL on failure
 **/
static char *
rapier_file_uri_to_filename(const char *uri) 
{
  int length;
  char *filename;

  if (strncmp(uri, "file:", 5))
    return NULL;

  /* FIXME: unix version of URI -> filename conversion */
  length=strlen(uri) -5 +1;
  filename=LIBRDF_MALLOC(cstring, length);
  if(!filename)
    return NULL;

  strcpy(filename, uri+5);
  return filename;
}


/*
 * rapier_parser_fatal_error - Error from a parser - Internal
 **/
static void
rapier_parser_fatal_error(rapier_parser* parser, const char *message, ...)
{
  va_list arguments;

  parser->failed=1;

  if(parser->fatal_error_handler) {
    parser->fatal_error_handler(parser->fatal_error_user_data, 
                                &parser->locator, message);
    abort();
  }

  va_start(arguments, message);

  rapier_print_locator(stderr, &parser->locator);
  fprintf(stderr, " rapier fatal error - ");
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);

  va_end(arguments);

  abort();
}


/*
 * rapier_parser_error - Error from a parser - Internal
 **/
static void
rapier_parser_error(rapier_parser* parser, const char *message, ...)
{
  va_list arguments;

  if(parser->error_handler) {
    parser->error_handler(parser->error_user_data, 
                          &parser->locator, message);
    return;
  }

  va_start(arguments, message);

  rapier_print_locator(stderr, &parser->locator);
  fprintf(stderr, " rapier error - ");
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);

  va_end(arguments);
}


/*
 * rapier_parser_warning - Warning from a parser - Internal
 **/
static void
rapier_parser_warning(rapier_parser* parser, const char *message, ...)
{
  va_list arguments;

  if(parser->warning_handler) {
    parser->warning_handler(parser->warning_user_data,
                            &parser->locator, message);
    return;
  }

  va_start(arguments, message);

  rapier_print_locator(stderr, &parser->locator);
  fprintf(stderr, " rapier warning - ");
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);

  va_end(arguments);
}



/* PUBLIC FUNCTIONS */

/**
 * rapier_new - Initialise the Rapier RDF parser
 *
 * Return value: non 0 on failure
 **/
rapier_parser*
rapier_new(rapier_uri *base_uri)
{
  rapier_parser* rdf_parser;
#ifdef NEED_EXPAT
  XML_Parser xp;
#endif

  rdf_parser=LIBRDF_CALLOC(rapier_parser, 1, sizeof(rapier_parser));

  if(!rdf_parser)
    return NULL;

  /* Initialise default feature values */
  rdf_parser->feature_scanning_for_rdf_RDF=0;
  rdf_parser->feature_allow_non_ns_attributes=1;
  rdf_parser->feature_interpret_containers_as_typedNode=0;


#ifdef NEED_EXPAT
  xp=XML_ParserCreate(NULL);

  /* create a new parser in the specified encoding */
  XML_SetUserData(xp, rdf_parser);

  /* XML_SetEncoding(xp, "..."); */

  XML_SetElementHandler(xp, rapier_xml_start_element_handler,
                        rapier_xml_end_element_handler);
  XML_SetCharacterDataHandler(xp, rapier_xml_cdata_handler);

  XML_SetUnparsedEntityDeclHandler(xp,
                                   rapier_xml_unparsed_entity_decl_handler);

  XML_SetExternalEntityRefHandler(xp,
                                  rapier_xml_external_entity_ref_handler);


#ifdef HAVE_XML_SetNamespaceDeclHandler
  XML_SetNamespaceDeclHandler(xp,
                              rapier_start_namespace_decl_handler,
                              rapier_end_namespace_decl_handler);
#endif
  rdf_parser->xp=xp;
#endif

#ifdef NEED_LIBXML
  xmlDefaultSAXHandlerInit();
  rdf_parser->sax.startElement=rapier_xml_start_element_handler;
  rdf_parser->sax.endElement=rapier_xml_end_element_handler;

  rdf_parser->sax.characters=rapier_xml_cdata_handler;
  rdf_parser->sax.ignorableWhitespace=rapier_xml_cdata_handler;

  rdf_parser->sax.warning=rapier_xml_warning;
  rdf_parser->sax.error=rapier_xml_error;
  rdf_parser->sax.fatalError=rapier_xml_fatal_error;

  rdf_parser->sax.setDocumentLocator=rapier_xml_set_document_locator;

  /* xmlInitParserCtxt(&rdf_parser->xc); */
#endif

  rdf_parser->base_uri=base_uri;

  rapier_init_namespaces(rdf_parser);

  return rdf_parser;
}




/**
 * rapier_free - Free the Rapier RDF parser
 * @rdf_parser: parser object
 * 
 **/
void
rapier_free(rapier_parser *rdf_parser) 
{
  rapier_element* element;
  rapier_ns_map* ns;

  ns=rdf_parser->namespaces;
  while(ns) {
    rapier_ns_map* next_ns=ns->next;

    rapier_free_namespace(rdf_parser, ns);
    ns=next_ns;
  }

  while((element=rapier_element_pop(rdf_parser))) {
    rapier_free_element(element);
  }

  LIBRDF_FREE(rapier_parser, rdf_parser);
}


/**
 * rapier_set_fatal_error_handler - Set the parser error handling function
 * @parser: the parser
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 * 
 * The function will receive callbacks when the parser fails.
 * 
 **/
void
rapier_set_fatal_error_handler(rapier_parser* parser, void *user_data,
                               rapier_message_handler handler)
{
  parser->fatal_error_user_data=user_data;
  parser->fatal_error_handler=handler;
}


/**
 * rapier_set_error_handler - Set the parser error handling function
 * @parser: the parser
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 * 
 * The function will receive callbacks when the parser fails.
 * 
 **/
void
rapier_set_error_handler(rapier_parser* parser, void *user_data,
                         rapier_message_handler handler)
{
  parser->error_user_data=user_data;
  parser->error_handler=handler;
}


/**
 * rapier_set_warning_handler - Set the parser warning handling function
 * @parser: the parser
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 * 
 * The function will receive callbacks when the parser gives a warning.
 * 
 **/
void
rapier_set_warning_handler(rapier_parser* parser, void *user_data,
                           rapier_message_handler handler)
{
  parser->warning_user_data=user_data;
  parser->warning_handler=handler;
}


void
rapier_set_statement_handler(rapier_parser* parser,
                          void *user_data, 
                          rapier_statement_handler handler)
{
  parser->user_data=user_data;
  parser->statement_handler=handler;
}





/**
 * rapier_parse_file - Retrieve the RDF/XML content at URI
 * @rdf_parser: parser
 * @uri: URI of RDF content
 * @base_uri: the base URI to use (or NULL if the same)
 * 
 * Return value: non 0 on failure
 **/
int
rapier_parse_file(rapier_parser* rdf_parser,  const char *uri,
                  const char *base_uri) 
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
  rapier_locator *locator=&rdf_parser->locator;

  /* initialise fields */
  rdf_parser->depth=0;
  rdf_parser->root_element= rdf_parser->current_element=NULL;
  rdf_parser->failed=0;



#ifdef NEED_EXPAT
  xp=rdf_parser->xp;

  XML_SetBase(xp, base_uri);
#endif


  filename=rapier_file_uri_to_filename(uri);
  if(!filename)
    return 1;

  locator->file=filename;
  locator->uri=base_uri;

  fh=fopen(filename, "r");
  if(!fh) {
    rapier_parser_error(rdf_parser, "file open failed - %s", strerror(errno));
#ifdef NEED_EXPAT
    XML_ParserFree(xp);
#endif /* EXPAT */
    LIBRDF_FREE(cstring, (void*)filename);
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
      LIBRDF_FREE(cstring, (void*)filename);
      return 1;
    }
    xc->userData = rdf_parser;
    xc->vctxt.userData = rdf_parser;
    xc->vctxt.error=rapier_xml_validation_error;
    xc->vctxt.warning=rapier_xml_validation_warning;
    
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

    rapier_parser_error(rdf_parser, "XML Parsing failed - %s",
                        XML_ErrorString(xe));
    rc=1;
  } else
    rc=0;

  XML_ParserFree(xp);
#endif /* EXPAT */
#ifdef NEED_LIBXML
  if(rc)
    rapier_parser_error(rdf_parser, "XML Parsing failed");

  xmlFreeParserCtxt(xc);

#endif

  LIBRDF_FREE(cstring, (void*)filename);

  return (rc != 0);
}


void
rapier_print_locator(FILE *stream, rapier_locator* locator) 
{
  if(!locator)
    return;

  if(locator->uri)
    fprintf(stream, "URI %s", locator->uri);
  else if (locator->file)
    fprintf(stream, "file %s", locator->file);
  else
    return;
  if(locator->line) {
    fprintf(stream, ":%d", locator->line);
    if(locator->column)
      fprintf(stream, " column %d", locator->column);
  }
}



void
rapier_set_feature(rapier_parser *parser, rapier_feature feature, int value) {
  switch(feature) {
    case RAPIER_FEATURE_SCANNING:
      parser->feature_scanning_for_rdf_RDF=value;
      break;

    case RAPIER_FEATURE_ALLOW_NON_NS_ATTRIBUTES:
      parser->feature_allow_non_ns_attributes=value;
      break;

    case RAPIER_FEATURE_INTERPRET_CONTAINERS_AS_TYPEDNODE:
      parser->feature_interpret_containers_as_typedNode=value;
      break;

    case RAPIER_FEATURE_ALLOW_OTHER_PARSETYPES:
      parser->feature_allow_other_parseTypes=value;
      break;

    default:
      break;
  }
}


static void
rapier_generate_statement(rapier_parser *rdf_parser, 
                          const void *subject,
                          const rapier_subject_type subject_type,
                          const void *predicate,
                          const rapier_predicate_type predicate_type,
                          const void *object,
                          const rapier_object_type object_type,
                          rapier_uri *bag)
{
  rapier_statement statement;

  statement.subject=subject;
  statement.subject_type=subject_type;
  statement.predicate=predicate;
  statement.predicate_type=predicate_type;
  statement.object=object;
  statement.object_type=object_type;

#ifdef RAPIER_DEBUG
  fprintf(stderr, "rapier_generate_statement: Generating statement: ");
  rapier_print_statement_detailed(&statement, 1, stderr);
  fputc('\n', stderr);

  if(!subject)
    LIBRDF_FATAL1(rapier_generate_statement, "Statement has no subject\n");
  
  if(!predicate)
    LIBRDF_FATAL1(rapier_generate_statement, "Statement has no predicate\n");
  
  if(!object)
    LIBRDF_FATAL1(rapier_generate_statement, "Statement has no object\n");
  
#endif

  if(!rdf_parser->statement_handler)
    return;

  /* Generate the statement; or is it fact? */
  (*rdf_parser->statement_handler)(rdf_parser->user_data, &statement);

  if(!bag)
    return;

  /* generate reified statements */
  statement.subject_type=RAPIER_SUBJECT_TYPE_RESOURCE;
  statement.predicate_type=RAPIER_PREDICATE_TYPE_PREDICATE;

  statement.subject=bag;
  statement.object_type=RAPIER_OBJECT_TYPE_RESOURCE;

  statement.predicate=RAPIER_RDF_type_URI;
  statement.object=RAPIER_RDF_Statement_URI;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, &statement);

  statement.predicate=RAPIER_RDF_subject_URI;
  statement.object=subject;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, &statement);

  statement.predicate=RAPIER_RDF_predicate_URI;
  statement.object=predicate;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, &statement);

  statement.predicate=RAPIER_RDF_object_URI;
  statement.object=object;
  statement.object_type=object_type;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, &statement);
}


static void
rapier_generate_named_statement(rapier_parser *rdf_parser, 
                                const void *subject,
                                const rapier_subject_type subject_type,
                                rapier_ns_name *predicate,
                                const void *object,
                                const rapier_object_type object_type,
                                rapier_uri *bag)
{
    if(predicate->uri)
      rapier_generate_statement(rdf_parser, 
                                subject,
                                subject_type,
                                predicate->uri,
                                RAPIER_PREDICATE_TYPE_PREDICATE,
                                object,
                                object_type,
                                bag);
    else
      rapier_generate_statement(rdf_parser, 
                                subject,
                                subject_type,
                                predicate->qname,
                                RAPIER_PREDICATE_TYPE_XML_NAME,
                                object,
                                object_type,
                                bag);
}


static void
rapier_print_statement_detailed(const rapier_statement * const statement,
                                int detailed, FILE *stream) 
{
  fprintf(stream, "[%s, ",
          RAPIER_URI_AS_STRING((rapier_uri*)statement->subject));

  if(statement->predicate_type == RAPIER_PREDICATE_TYPE_ORDINAL)
    fprintf(stream, "[rdf:_%d]", *((int*)statement->predicate));
  else if(statement->predicate_type == RAPIER_PREDICATE_TYPE_XML_NAME)
    fputs((const char*)statement->predicate, stream);
  else
    fputs(RAPIER_URI_AS_STRING((rapier_uri*)statement->predicate), stream);

  if(statement->object_type == RAPIER_OBJECT_TYPE_LITERAL || 
     statement->object_type == RAPIER_OBJECT_TYPE_XML_LITERAL)
    fprintf(stream, ", \"%s\"]",  (const char*)statement->object);
  else if(statement->object_type == RAPIER_OBJECT_TYPE_XML_NAME)
    fprintf(stream, ", %s]", (const char*)statement->object);
  else
    fprintf(stream, ", %s]", 
            RAPIER_URI_AS_STRING((rapier_uri*)statement->object));
}


void
rapier_print_statement(const rapier_statement * const statement, FILE *stream) 
{
  rapier_print_statement_detailed(statement, 0, stream);
}



static const char *
rapier_generate_id(rapier_parser *rdf_parser, const int id_for_bag)
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

  return (const char*)buffer;
}


static rapier_uri*
rapier_make_uri_from_id(rapier_uri *base_uri, const char *id) 
{
#ifdef LIBRDF_INTERNAL
  librdf_uri *new_uri;
  char *qname;
  int len;
#else
  char *new_uri;
  int len;
#endif

#if defined(RAPIER_DEBUG) && RAPIER_DEBUG > 1
  LIBRDF_DEBUG3(rapier_make_uri_from_id, 
                "Using base URI %s and ID %s\n", 
                base_uri, id);
#endif

#ifdef LIBRDF_INTERNAL
  /* "#id\0" */
  len=1+strlen(id)+1;
  qname=LIBRDF_MALLOC(cstring, len);
  if(!qname)
    return NULL;
  *qname='#';
  strcpy(qname+1, id);
  new_uri=librdf_new_uri_from_uri_qname(base_uri, qname);
  LIBRDF_FREE(cstring, qname);
  return new_uri;
#else
  len=strlen(base_uri)+1+strlen(id)+1;
  new_uri=LIBRDF_MALLOC(cstring, len);
  if(!new_uri)
    return NULL;
  sprintf(new_uri, "%s#%s", base_uri, id);
  return new_uri;
#endif
}


static rapier_uri*
rapier_make_uri(rapier_uri *base_uri, const char *uri_string) 
{
#ifdef LIBRDF_INTERNAL
#else
  char *new_uri;
  const char *p;
  int base_uri_len=strlen(base_uri);
#endif

#if defined(RAPIER_DEBUG) && RAPIER_DEBUG > 1
  LIBRDF_DEBUG3(rapier_make_uri, 
                "Using base URI %s and URI string '%s'\n", 
                base_uri, uri_string);
#endif

#ifdef LIBRDF_INTERNAL
  return librdf_new_uri_relative_to_base(base_uri, uri_string);
#else
  /* If URI string is empty, just copy base URI */
  if(!*uri_string) {
    new_uri=LIBRDF_MALLOC(cstring, base_uri_len+1);
    if(!new_uri)
      return NULL;
    strcpy(new_uri, base_uri);
    return new_uri;
  }

  /* If URI string is a fragment #foo, append to base URI */
  if(*uri_string == '#') {
    new_uri=LIBRDF_MALLOC(cstring, base_uri_len+1+strlen(uri_string)+1);
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
   * FIXME what a crock
   */
  if(*p && *p == ':') {
    new_uri=LIBRDF_MALLOC(cstring, strlen(uri_string)+1);
    if(!new_uri)
      return NULL;
    strcpy(new_uri, uri_string);
    return new_uri;
  }

  /* Otherwise is a general URI relative to base URI */

  /* FIXME do this properly */

  /* Move p to the last / or : char in the base URI */
  for(p=base_uri+base_uri_len-1; p > base_uri && *p != '/' && *p != ':'; p--)
    ;

  new_uri=LIBRDF_MALLOC(cstring, (p-base_uri)+1+strlen(uri_string)+1);
  if(!new_uri)
    return NULL;
  strncpy(new_uri, base_uri, p-base_uri+1);
  strcpy(new_uri+(p-base_uri)+1, uri_string);
  return new_uri;
#endif

}


static rapier_uri*
rapier_copy_uri(rapier_uri *uri) 
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


static void 
rapier_process_property_attributes(rapier_parser *rdf_parser, 
                                   rapier_element *element)
{
  int i;

  /* Process remaining attributes as propAttr* =
   * (propName="string")*
   */
  for(i=0; i<element->attribute_count; i++) {
    rapier_ns_name* attribute=element->attributes[i];
    const char *value = attribute->value;
    
    /* Generate the property - either with a namespaced predicate
     * (attribute has a URI) or non-namespaced predicate (attribute
     * is just the XML name)
     */
    if(attribute->namespace && attribute->namespace->is_rdf_ms && 
       IS_RDF_MS_CONCEPT(attribute->qname, attribute->uri, li)) {
      /* recognise rdf:li attribute */
      element->last_ordinal++;
      rapier_generate_statement(rdf_parser,
                                element->uri,
                                RAPIER_SUBJECT_TYPE_RESOURCE,
                                (void*)&element->last_ordinal,
                                RAPIER_PREDICATE_TYPE_ORDINAL,
                                (void*)value,
                                RAPIER_OBJECT_TYPE_LITERAL,
                                element->bag_uri);
    } else
      rapier_generate_named_statement(rdf_parser, 
                                      element->uri,
                                      RAPIER_SUBJECT_TYPE_RESOURCE,
                                      attribute,
                                      (void*)value,
                                      RAPIER_OBJECT_TYPE_LITERAL,
                                      element->bag_uri);

  } /* end for ... attributes */
}


static void
rapier_start_element_grammar(rapier_parser *rdf_parser,
                             rapier_element *element) 
{
  int finished;
  rapier_state state;
  int i;
  const char *el_name=element->name->qname;
  int element_in_rdf_ns=(element->name->namespace && 
                         element->name->namespace->is_rdf_ms);


  if(element->parent) {
    state=element->parent->child_state;
    if(!state) {
      state=element->parent->state;
      LIBRDF_DEBUG3(rapier_start_element_grammar,
                    "NO CHILD STATE set - taking parent state %d - %s\n",
                    state, rapier_state_as_string(state));
    }
    
  } else
    state=RAPIER_STATE_UNKNOWN;
  LIBRDF_DEBUG3(rapier_start_element_grammar, "starting in state %d - %s\n",
                state, rapier_state_as_string(state));

  finished= 0;
  while(!finished) {
    switch(state) {
      case RAPIER_STATE_UNKNOWN:
        /* found <rdf:RDF> ? */
        if(element_in_rdf_ns && 
          IS_RDF_MS_CONCEPT(el_name, element->name->uri, RDF)) {
          element->child_state=RAPIER_STATE_OBJ;
          /* Yes - need more content before can continue,
           * so wait for another element
           */
          finished=1;
          break;
        }

        /* If scanning for element, can continue */
        if(rdf_parser->feature_scanning_for_rdf_RDF) {
          finished=1;
          break;
        }

        /* Otherwise the choice of the next state can be made
         * from the current element by the IN_RDF state
         */
        state=RAPIER_STATE_OBJ;
        break;


      case RAPIER_STATE_OBJ:
      case RAPIER_STATE_INLINEITEM:
        /* Handling either 6.2 (obj) or 6.17 (value) (inside
         * rdf:RDF, propertyElt or rdf:li) and expecting
         * description (6.3) | sequence (6.25) | bag (6.26) | alternative (6.27)
         * [|other container (not M&S)]
         *
         * CHOICES:
         *   <rdf:Description> (goto 6.3)
         *   <rdf:Seq> (goto 6.25) (or typedNode CHOICE)
         *   <rdf:Bag> (goto 6.26) (or typedNode CHOICE)
         *   <rdf:Alt> (goto 6.27) (or typedNode CHOICE)
         * OR from 6.3 can have ANY other element matching
         * typedNode (6.13) - goto 6.3
         */

        if(element_in_rdf_ns) {
          if(IS_RDF_MS_CONCEPT(el_name, element->name->uri, Description)) {
            state=RAPIER_STATE_DESCRIPTION;
            break;
          } 

          if(IS_RDF_MS_CONCEPT(el_name, element->name->uri, Seq)) {
            /* CHOICE - can choose either sequence or typedNode productions */
            if(rdf_parser->feature_interpret_containers_as_typedNode) {
              rdf_parser->typed_node_block_type=RAPIER_TYPED_NODE_BLOCK_TYPE_SEQ;
              state=RAPIER_STATE_TYPED_NODE;
            } else {
              /* However, if there are non rdf:ID attributes, it must
               * be interpreted as a typedNode
               */
              if(element->rdf_attr_count -
                 (element->rdf_attr[RDF_ATTR_ID] != NULL))
                state=RAPIER_STATE_TYPED_NODE;
              else {
                state=RAPIER_STATE_SEQ;
                element->container_type=RAPIER_RDF_Seq_URI;
              }
            }

            break;
          }

          if(IS_RDF_MS_CONCEPT(el_name, element->name->uri, Bag)) {
            /* CHOICE - can choose either sequence or typedNode productions */
            if(rdf_parser->feature_interpret_containers_as_typedNode) {
              rdf_parser->typed_node_block_type=RAPIER_TYPED_NODE_BLOCK_TYPE_BAG;
              state=RAPIER_STATE_TYPED_NODE;
            } else {
              /* However, if there are non rdf:ID attributes, it must
               * be interpreted as a typedNode
               */
              if(element->rdf_attr_count -
                 (element->rdf_attr[RDF_ATTR_ID] != NULL))
                state=RAPIER_STATE_TYPED_NODE;
              else {
                state=RAPIER_STATE_BAG;
                element->container_type=RAPIER_RDF_Bag_URI;
              }
            }

            break;
          }

          if(IS_RDF_MS_CONCEPT(el_name, element->name->uri, Alt)) {
            /* CHOICE - can choose either sequence or typedNode productions */
            if(rdf_parser->feature_interpret_containers_as_typedNode) {
              rdf_parser->typed_node_block_type=RAPIER_TYPED_NODE_BLOCK_TYPE_ALT;
              state=RAPIER_STATE_TYPED_NODE;
            } else {
              /* However, if there are non rdf:ID attributes, it must
               * be interpreted as a typedNode
               */
              if(element->rdf_attr_count -
                 (element->rdf_attr[RDF_ATTR_ID] != NULL))
                state=RAPIER_STATE_TYPED_NODE;
              else {
                state=RAPIER_STATE_ALT;
                element->container_type=RAPIER_RDF_Alt_URI;
              }
            }

            break;
          }

          if(rdf_parser->container_test_handler) {
            rapier_uri *container_type=rdf_parser->container_test_handler(element->name->uri);
            if(container_type) {
              /* CHOICE - can choose either 'container' or typedNode productions */
              if(rdf_parser->feature_interpret_containers_as_typedNode) {
                rdf_parser->typed_node_block_type=RAPIER_TYPED_NODE_BLOCK_TYPE_CONTAINER;
                state=RAPIER_STATE_TYPED_NODE;
              } else {
                state=RAPIER_STATE_CONTAINER;
                element->container_type=container_type;
              }

              break;
            }
          }

          /* Other rdf: elements at this layer are taken as matching
           * the typedNode production - just fallthrough
           */
        }

        /* Default to typedNode */
        state=RAPIER_STATE_TYPED_NODE;
        break;


      /* No need for 6.2 - already chose 6.3, 6.25, 6.26 or 6.27 */


      case RAPIER_STATE_DESCRIPTION:
      case RAPIER_STATE_TYPED_NODE:
      case RAPIER_STATE_PARSETYPE_RESOURCE:
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
         * 6.3, 6.25, 6.26, 6.27.  OR when 
         * feature_interpret_containers_as_typedNode is set,
         * we can be handling rdf:Seq, rdf:Bag, rdf:Alt or other container
         * and this can be determined from the value of 
         * rdf_parser->typed_node_block_type
         *
         * Expect here from production 6.13 (typedNode)
         * <typeName idAboutAttr? bagIdAttr? propAttr* />
         *   Attributes: (ID|about|aboutEach|aboutEachPrefix)? bagID? propAttr*
         *
         * CHOICE (description): Create a bag here (always? even if
         * no bagId given?) FIXME - not implemented yet.
         */


        /* lets add booleans - isn't C wonderful! */
        if((element->rdf_attr[RDF_ATTR_ID] != NULL) +
           (element->rdf_attr[RDF_ATTR_about] != NULL) +
           (element->rdf_attr[RDF_ATTR_aboutEach] != NULL) +
           (element->rdf_attr[RDF_ATTR_aboutEachPrefix] != NULL) > 1) {
          rapier_parser_warning(rdf_parser, "More than one of RDF ID, about, aboutEach or aboutEachPrefix attributes on element %s - from productions 6.5, 6.6, 6.7 and 6.8 expect at most one.", el_name);
        }

        if(element->rdf_attr[RDF_ATTR_ID]) {
          element->id=element->rdf_attr[RDF_ATTR_ID];
          element->uri=rapier_make_uri_from_id(rdf_parser->base_uri, element->id);
          element->subject_type=RAPIER_SUBJECT_TYPE_RESOURCE;
        } else if (element->rdf_attr[RDF_ATTR_about]) {
          element->uri=rapier_make_uri(rdf_parser->base_uri, element->rdf_attr[RDF_ATTR_about]);
          element->subject_type=RAPIER_SUBJECT_TYPE_RESOURCE;
        } else if (element->rdf_attr[RDF_ATTR_aboutEach]) {
          element->uri=rapier_make_uri(rdf_parser->base_uri, element->rdf_attr[RDF_ATTR_aboutEach]);
          element->subject_type=RAPIER_SUBJECT_TYPE_RESOURCE_EACH;
        } else if (element->rdf_attr[RDF_ATTR_aboutEachPrefix]) {
          element->uri=rapier_make_uri(rdf_parser->base_uri, element->rdf_attr[RDF_ATTR_aboutEachPrefix]);
          element->subject_type=RAPIER_SUBJECT_TYPE_RESOURCE_EACH_PREFIX;
        } else {
          element->id=rapier_generate_id(rdf_parser, 0);
          element->id_is_generated=1;
          element->uri=rapier_make_uri_from_id(rdf_parser->base_uri, element->id);
        }


        if(element->rdf_attr[RDF_ATTR_bagID]) {
          element->bag_id=element->rdf_attr[RDF_ATTR_bagID];
          element->bag_uri=rapier_make_uri_from_id(rdf_parser->base_uri, element->bag_id);
        }

        /* If there is a parent element (property) containing this
         * element (node)
         */
        if(element->parent) {
          
          /* Free any existing object URI still around */
          if(element->parent->object_uri)
            RAPIER_FREE_URI(element->parent->object_uri);

          /* Store our URI in our parent as the object URI */
          element->parent->object_uri=rapier_copy_uri(element->uri);

          element->parent->content_type = RAPIER_ELEMENT_CONTENT_TYPE_RESOURCE;
        }


        /* If this is a typed node, generate the rdf:type statement
         * from this node
         */
        if(state == RAPIER_STATE_TYPED_NODE) {
          rapier_object_type object_type=(element->name->uri) ? RAPIER_OBJECT_TYPE_RESOURCE : RAPIER_OBJECT_TYPE_XML_NAME;
          void *object=(element->name->uri) ? (void*)element->name->uri : (void*)element->name->qname;

          rapier_generate_statement(rdf_parser, 
                                    element->uri,
                                    RAPIER_SUBJECT_TYPE_RESOURCE,
                                    RAPIER_RDF_type_URI,
                                    RAPIER_PREDICATE_TYPE_PREDICATE,
                                    object,
                                    object_type,
                                    element->bag_uri);

        }


        rapier_process_property_attributes(rdf_parser, element);

        /* for both productions now need some more content or
         * property elements before can do any more work.
         */

        element->child_state=RAPIER_STATE_PROPERTYELT;
        element->content_type=RAPIER_ELEMENT_CONTENT_TYPE_PROPERTIES;
        finished=1;
        break;


      case RAPIER_STATE_SEQ:
      case RAPIER_STATE_BAG:
      case RAPIER_STATE_ALT:
      case RAPIER_STATE_CONTAINER:
        /* choices here from production 6.25 (sequence), 6.26 (bag)
         * and 6.27 (alternative)
         * <rdf:Seq idAttr?>...</rdf:Seq> (attribute productions: 6.6)
         *   Attributes: ID?
         * <rdf:Seq idAttr? memberAttr* /> (attribute productions: 6.6, 6.31)
         *   Attributes: ID? rdf:_<n> (where n is an integer)
         *
         * Note: The second case implies the element must have no content.
         */

        /* Find the ID for this container from the rdf:ID attribute
         * or generate an ID
         */
        if(element->rdf_attr[RDF_ATTR_ID]) {
          element->id=element->rdf_attr[RDF_ATTR_ID];
        } else {
          element->id=rapier_generate_id(rdf_parser, 0);
          element->id_is_generated=1;
        }

        element->uri=rapier_make_uri_from_id(rdf_parser->base_uri, element->id);

        /* Store URI of new thing in our parent */
        if(element->parent) {
          /* Free any existing object URI still around */
          if(element->parent->object_uri)
            RAPIER_FREE_URI(element->parent->object_uri);

          /* Store our URI in our parent as the object URI */
          element->parent->object_uri=rapier_copy_uri(element->uri);

          element->parent->content_type = RAPIER_ELEMENT_CONTENT_TYPE_RESOURCE;
        }


        /* Generate the rdf:type statement for the container node */
        rapier_generate_statement(rdf_parser, 
                                  element->uri,
                                  RAPIER_SUBJECT_TYPE_RESOURCE,
                                  RAPIER_RDF_type_URI,
                                  RAPIER_PREDICATE_TYPE_PREDICATE,
                                  element->container_type,
                                  RAPIER_OBJECT_TYPE_RESOURCE,
                                  NULL);


        /* Process container member attributes which match:
         * 6.31 (memberAttr) = member* = (rdf:_n="string")*
         */
        for(i=0; i<element->attribute_count; i++) {
          rapier_ns_name* attribute=element->attributes[i];
          const char *name=attribute->qname;
          const char *value = attribute->value;
          int ordinal = 0;
          int attribute_problems=0;

          if(!attribute->namespace || !attribute->namespace->is_rdf_ms) {
            /* No namespace or not rdf: namespace */
            attribute_problems++;
          } else if (*name != '_') {
            /* not rdf:_ */
            attribute_problems++;
          } else {
            /* rdf:_ something */

            name++;
            while(*name >= '0' && *name <= '9') {
              ordinal *= 10;
              ordinal += (*name++ - '0');
            }

            if(ordinal < 1) {
              rapier_parser_warning(rdf_parser, "Illegal ordinal value %d in attribute %s seen on container element %s.", ordinal, attribute->qname, el_name);
              attribute_problems+=2;
            }
          }

          if(attribute->namespace && attribute->namespace->is_rdf_ms &&
             attribute_problems == 1)
            /* Warn if unexpected or bad rdf: attributes seen and not
             * already warned
             */
            rapier_parser_warning(rdf_parser, "Unexpected rdf: attribute %s seen on container element %s - from production 6.25, 6.26 and 6.27 expected rdf:ID or rdf:_n only.", name, el_name);


          /* Always generate a property - either an ordinal,
           * one when the attribute is OK, otherwise just a
           * regular one using the attribute value
           */
          if(!attribute_problems)
            rapier_generate_statement(rdf_parser, 
                                      element->uri,
                                      RAPIER_SUBJECT_TYPE_RESOURCE,
                                      (void*)&ordinal,
                                      RAPIER_PREDICATE_TYPE_ORDINAL,
                                      (void*)value,
                                      RAPIER_OBJECT_TYPE_LITERAL,
                                      NULL);

          else
            rapier_generate_named_statement(rdf_parser, 
                                            element->uri,
                                            RAPIER_SUBJECT_TYPE_RESOURCE,
                                            attribute,
                                            (void*)value,
                                            RAPIER_OBJECT_TYPE_LITERAL,
                                            NULL);


        } /* end for ... attributes */


        element->child_state=RAPIER_STATE_MEMBER;
        finished=1;
        break;

        /* Expect to meet the production member - 
         * referencedItem (6.29) | inlineItem (6.30) =
         *   '<rdf:li' resourceAttr '/>'  | 
         *   '<rdf:li' '>' value </rdf:li>' |
         *   '<rdf:li' parseLiteral '>' literal </rdf:li>' |
         *   '<rdf:li' parseResource '>' propertyElt* </rdf:li>' 
         * 
         * Taking just the start element work:
         *   1) <rdf:li rdf:resource="..."/>
         *   2) <rdf:li>
         *   3) <rdf:li rdf:parseType="literal">
         *   4) <rdf:li rdf:parseType="resource">
         *
         * 1) moves to RAPIER_STATE_REFERENCEDITEM
         * 2) moves to RAPIER_STATE_INLINEITEM to handle contents of rdf:li
         * 3) moves to RAPIER_STATE_PARSETYPE_LITERAL
         * 4) moves to RAPIER_STATE_PARSETYPE_RESOURCE
         * Other parseType content moves to RAPIER_STATE_PARSETYPE_OTHER
         */
      case RAPIER_STATE_MEMBER:
        if(element->rdf_attr[RDF_ATTR_resource]) {
          element->uri=rapier_make_uri(rdf_parser->base_uri,
                                       element->rdf_attr[RDF_ATTR_resource]);
          state=RAPIER_STATE_REFERENCEDITEM;
        } else if (element->rdf_attr[RDF_ATTR_parseType]) {
          const char *parse_type=element->rdf_attr[RDF_ATTR_parseType];

          if(!strcasecmp(parse_type, "literal")) {
            element->child_state=RAPIER_STATE_PARSETYPE_LITERAL;
          } else if (!strcasecmp(parse_type, "resource")) {
            element->child_state=RAPIER_STATE_PARSETYPE_RESOURCE;
          } else {
            if(rdf_parser->feature_allow_other_parseTypes)
              element->child_state=RAPIER_STATE_PARSETYPE_OTHER;
            else
              element->child_state=RAPIER_STATE_PARSETYPE_LITERAL;
          }
        } else
          element->child_state=RAPIER_STATE_INLINEITEM;

        finished=1;
        break;

      case RAPIER_STATE_REFERENCEDITEM:
        /* FIXME - need to generate warning for unexpected content */
        finished=1;
        break;

      case RAPIER_STATE_PARSETYPE_OTHER:
        /* FIXME */

        /* FALLTHROUGH */

      case RAPIER_STATE_PARSETYPE_LITERAL:
        {
          char *fmt_buffer;
          int fmt_length;
          fmt_buffer=rapier_format_element(element, &fmt_length,0);
          if(fmt_buffer && fmt_length) {
            /* Append cdata content content */
            if(element->content_cdata) {
              /* Append */
              char *new_cdata=LIBRDF_MALLOC(cstring, element->content_cdata_length + fmt_length + 1);
              if(new_cdata) {
                strncpy(new_cdata, element->content_cdata,
                        element->content_cdata_length);
                strcpy(new_cdata+element->content_cdata_length, fmt_buffer);
                LIBRDF_FREE(cstring, element->content_cdata);
                element->content_cdata=new_cdata;
              }
            } else {
              /* Copy - is empty */
              element->content_cdata       =fmt_buffer;
              element->content_cdata_length=fmt_length;
            }
          }
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
      case RAPIER_STATE_PROPERTYELT:

        /* Handle rdf:li as a property in member state above */ 
        if(element_in_rdf_ns && 
           IS_RDF_MS_CONCEPT(el_name, element->name->uri, li)) {
          state=RAPIER_STATE_MEMBER;
          break;
        }


        /* M&S says: "The value of the ID attribute, if specified, is
         * the identifier for the resource that represents the
         * reification of the statement." */
        element->bag_id=element->rdf_attr[RDF_ATTR_ID];

        if (element->rdf_attr[RDF_ATTR_parseType]) {
          const char *parse_type=element->rdf_attr[RDF_ATTR_parseType];

          if(!strcasecmp(parse_type, "literal")) {
            element->child_state=RAPIER_STATE_PARSETYPE_LITERAL;
          } else if (!strcasecmp(parse_type, "resource")) {
            element->child_state=RAPIER_STATE_PARSETYPE_RESOURCE;
          } else {
            if(rdf_parser->feature_allow_other_parseTypes)
              element->child_state=RAPIER_STATE_PARSETYPE_OTHER;
            else
              element->child_state=RAPIER_STATE_PARSETYPE_LITERAL;
          }
        } else if(element->rdf_attr[RDF_ATTR_resource]) {
          /* Can only be last case */
          element->uri=rapier_make_uri(rdf_parser->base_uri,
                                       element->rdf_attr[RDF_ATTR_resource]);

          /* Store URI of new thing in our parent */
          if(element->parent) {
            /* Free any existing object URI still around */
            if(element->parent->object_uri)
              RAPIER_FREE_URI(element->parent->object_uri);
            
            /* Store our URI in our parent as the object URI *FIXME*  Needed? */
            element->parent->object_uri=rapier_copy_uri(element->uri);
            
            element->parent->content_type = RAPIER_ELEMENT_CONTENT_TYPE_RESOURCE;
          }

          if(element->rdf_attr[RDF_ATTR_bagID]) {
            element->bag_id=element->rdf_attr[RDF_ATTR_bagID];
            element->bag_uri=rapier_make_uri_from_id(rdf_parser->base_uri, element->bag_id);
          }

          rapier_process_property_attributes(rdf_parser, element);

          rapier_generate_named_statement(rdf_parser, 
                                          element->parent->uri,
                                          RAPIER_SUBJECT_TYPE_RESOURCE,
                                          element->name,
                                          (void*)element->uri, 
                                          RAPIER_OBJECT_TYPE_RESOURCE,
                                          element->bag_uri);
          
          /* Done - wait for end of this element to end in order to 
           * check the element was empty as expected */
          element->content_type = RAPIER_ELEMENT_CONTENT_TYPE_EMPTY;
        } else {
          /* Otherwise process content in obj (value) state */
          element->child_state=RAPIER_STATE_OBJ;
          element->content_type = RAPIER_ELEMENT_CONTENT_TYPE_UNKNOWN;
        } /* end if parseType or resource attribute seen */

        finished=1;

        break;


      default:
        rapier_parser_fatal_error(rdf_parser, "rapier_start_element_grammar - unexpected parser state %d - %s\n", state, rapier_state_as_string(state));
        finished=1;

    } /* end switch */

    if(state != element->state) {
      element->state=state;
      LIBRDF_DEBUG3(rapier_start_element_grammar, 
                    "moved to state %d - %s\n",
                    state, rapier_state_as_string(state));
    }

  } /* end while */

  LIBRDF_DEBUG3(rapier_start_element_grammar, 
                "ending in state %d - %s\n",
                state, rapier_state_as_string(state));
}


static void
rapier_end_element_grammar(rapier_parser *rdf_parser,
                           rapier_element *element) 
{
  rapier_state state;
  int finished;
  const char *el_name=element->name->qname;
  int element_in_rdf_ns=(element->name->namespace && 
                         element->name->namespace->is_rdf_ms);


  state=element->state;
  LIBRDF_DEBUG3(rapier_end_element_grammar, "starting in state %d - %s\n",
                state, rapier_state_as_string(state));

  finished= 0;
  while(!finished) {
    switch(state) {
      case RAPIER_STATE_UNKNOWN:
        finished=1;
        break;

      case RAPIER_STATE_OBJ:
        if(element_in_rdf_ns && 
          IS_RDF_MS_CONCEPT(el_name, element->name->uri,RDF)) {
          /* end of RDF - boo hoo */
          state=RAPIER_STATE_UNKNOWN;
          finished=1;
          break;
        }
        /* When scanning, another element ending is outside the RDF
         * world so this can happen without further work
         */
        if(rdf_parser->feature_scanning_for_rdf_RDF) {
          state=RAPIER_STATE_UNKNOWN;
          finished=1;
          break;
        }
        /* otherwise found some junk after RDF content in an RDF-only 
         * document (probably never get here since this would be
         * a mismatched XML tag and cause an error earlier)
         */
        rapier_parser_warning(rdf_parser, "Element %s ended, expected end of RDF element\n", el_name);
        state=RAPIER_STATE_UNKNOWN;
        finished=1;
        break;

      /* No need for 6.2 - already chose 6.3, 6.25, 6.26 or 6.27 */

      case RAPIER_STATE_DESCRIPTION:
      case RAPIER_STATE_TYPED_NODE:
      case RAPIER_STATE_PARSETYPE_RESOURCE:

        /* If there is a parent element containing this element and
         * the parent isn't a description, create the statement
         * between this node using parent property
         */
        if(state != RAPIER_STATE_DESCRIPTION && 
           element->parent && 
           element->parent->uri &&
           element->parent->state != RAPIER_STATE_DESCRIPTION) {

          rapier_generate_named_statement(rdf_parser, 
                                          element->parent->uri,
                                          RAPIER_SUBJECT_TYPE_RESOURCE,
                                          element->parent->name,
                                          element->uri,
                                          RAPIER_OBJECT_TYPE_RESOURCE,
                                          NULL);
        }

        finished=1;
        break;

      case RAPIER_STATE_SEQ:
      case RAPIER_STATE_BAG:
      case RAPIER_STATE_ALT:
      case RAPIER_STATE_CONTAINER:

        finished=1;
        break;

      case RAPIER_STATE_REFERENCEDITEM:
        if((element->content_element_seen + element->content_cdata_seen))
          rapier_parser_warning(rdf_parser, "Unexpected content in %s with resource attribute - from production 6.29 expected an empty element.", el_name);

        element->parent->last_ordinal++;
        rapier_generate_statement(rdf_parser, 
                                  element->parent->uri,
                                  RAPIER_SUBJECT_TYPE_RESOURCE,
                                  (void*)&element->parent->last_ordinal,
                                  RAPIER_PREDICATE_TYPE_ORDINAL,
                                  (void*)element->uri,
                                  RAPIER_OBJECT_TYPE_RESOURCE,
                                  NULL);

        finished=1;
        break;

      case RAPIER_STATE_INLINEITEM:
        element->parent->last_ordinal++;
        rapier_generate_statement(rdf_parser, 
                                  element->parent->uri,
                                  RAPIER_SUBJECT_TYPE_RESOURCE,
                                  (void*)&element->parent->last_ordinal,
                                  RAPIER_PREDICATE_TYPE_ORDINAL,
                                  (void*)element->content_cdata,
                                  RAPIER_OBJECT_TYPE_LITERAL,
                                  NULL);
        finished=1;
        break;


      case RAPIER_STATE_PARSETYPE_OTHER:
        /* FIXME */

        /* FALLTHROUGH */

      case RAPIER_STATE_PARSETYPE_LITERAL:
        element->parent->content_type=RAPIER_ELEMENT_CONTENT_TYPE_PRESERVED;

        /* Append closing element */
        {
          char *fmt_buffer;
          int fmt_length;
          fmt_buffer=rapier_format_element(element, &fmt_length,1);
          if(fmt_buffer && fmt_length) {
            /* Append cdata content content */
            char *new_cdata=LIBRDF_MALLOC(cstring, element->content_cdata_length + fmt_length + 1);
            if(new_cdata) {
              strncpy(new_cdata, element->content_cdata,
                      element->content_cdata_length);
              strcpy(new_cdata+element->content_cdata_length, fmt_buffer);
              LIBRDF_FREE(cstring, element->content_cdata);
              element->content_cdata=new_cdata;
            }
          }
        }

        /* Append this cdata content to parent element cdata content */
        if(element->parent->content_cdata) {
          /* Append */
          char *new_cdata=LIBRDF_MALLOC(cstring, element->parent->content_cdata_length + element->content_cdata_length + 1);
          if(new_cdata) {
            strncpy(new_cdata, element->parent->content_cdata,
                    element->parent->content_cdata_length);
            strcpy(new_cdata+element->parent->content_cdata_length, 
                   element->content_cdata);
            LIBRDF_FREE(cstring, element->parent->content_cdata);
            element->parent->content_cdata=new_cdata;
          }
        } else {
          /* Copy - parent is empty */
          element->parent->content_cdata       =element->content_cdata;
          element->parent->content_cdata_length=element->content_cdata_length;
        }

        element->content_cdata=NULL;
        element->content_cdata_length=0;

        finished=1;
        break;


      case RAPIER_STATE_PROPERTYELT:
      case RAPIER_STATE_MEMBER:
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

        if(element->content_type == RAPIER_ELEMENT_CONTENT_TYPE_UNKNOWN) {
          if(element->content_cdata_seen) 
            element->content_type= RAPIER_ELEMENT_CONTENT_TYPE_LITERAL;
          else if (element->content_element_seen) 
            element->content_type= RAPIER_ELEMENT_CONTENT_TYPE_PROPERTIES;
          else
            element->content_type= RAPIER_ELEMENT_CONTENT_TYPE_EMPTY;
        }

        if(element->content_type == RAPIER_ELEMENT_CONTENT_TYPE_EMPTY) {
          char *object_id=(char*)rapier_generate_id(rdf_parser, 0);
          if(object_id) {
            element->object_uri=rapier_make_uri_from_id(rdf_parser->base_uri, 
                                                        (const char*)object_id);
            LIBRDF_FREE(cstring, object_id);
            element->content_type = RAPIER_ELEMENT_CONTENT_TYPE_RESOURCE;
          }
        }

        if(element->parent && 
           element->parent->state == RAPIER_STATE_DESCRIPTION) {
          int object_is_literal=(element->content_cdata != NULL);
          rapier_object_type object_type=object_is_literal ? RAPIER_OBJECT_TYPE_LITERAL : RAPIER_OBJECT_TYPE_RESOURCE;
          void *object=object_is_literal ? (void*)element->content_cdata : (void*)element->object_uri;

          /* If there is a parent Description element (node), generate
           * the statement:
           *   subject  : parent
           *   predicate: ordinal / this property
           *   object   : child object (node)
           */
          if(state == RAPIER_STATE_MEMBER) {
            element->parent->last_ordinal++;
            rapier_generate_statement(rdf_parser, 
                                      element->parent->uri,
                                      RAPIER_SUBJECT_TYPE_RESOURCE,
                                      (void*)&element->parent->last_ordinal,
                                      RAPIER_PREDICATE_TYPE_ORDINAL,
                                      object,
                                      object_type,
                                      NULL);
          } else
            rapier_generate_named_statement(rdf_parser, 
                                            element->parent->uri,
                                            RAPIER_SUBJECT_TYPE_RESOURCE,
                                            element->name,
                                            object,
                                            object_type,
                                            NULL);
          
        } else {
          int object_is_literal=(element->content_cdata != NULL);
          void *object=object_is_literal ? (void*)element->content_cdata : (void*)element->object_uri;
          rapier_object_type object_type=object_is_literal ? RAPIER_OBJECT_TYPE_LITERAL : RAPIER_OBJECT_TYPE_RESOURCE;

          /* Not child of Description, so process as child of propertyElt */
          switch(element->content_type) {
            case RAPIER_ELEMENT_CONTENT_TYPE_LITERAL:
            case RAPIER_ELEMENT_CONTENT_TYPE_RESOURCE:
              if(state == RAPIER_STATE_MEMBER) {
                element->parent->last_ordinal++;
                rapier_generate_statement(rdf_parser, 
                                          element->parent->uri,
                                          RAPIER_SUBJECT_TYPE_RESOURCE,
                                          (void*)&element->parent->last_ordinal,
                                          RAPIER_PREDICATE_TYPE_ORDINAL,
                                          object,
                                          object_type,
                                          NULL);
              } else
                rapier_generate_named_statement(rdf_parser, 
                                                element->parent->uri,
                                                RAPIER_SUBJECT_TYPE_RESOURCE,
                                                element->name,
                                                object, 
                                                object_type,
                                                NULL);
              
              break;
              
            case RAPIER_ELEMENT_CONTENT_TYPE_EMPTY:
              /* empty property of form
               * <property rdf:resource="object-URI"/> 
               */
              if(state == RAPIER_STATE_MEMBER) {
                element->parent->last_ordinal++;
                rapier_generate_statement(rdf_parser, 
                                          element->parent->uri,
                                          RAPIER_SUBJECT_TYPE_RESOURCE,
                                          (void*)&element->parent->last_ordinal,
                                          RAPIER_PREDICATE_TYPE_ORDINAL,
                                          (void*)element->uri,
                                          RAPIER_OBJECT_TYPE_RESOURCE,
                                          NULL);
              } else
                rapier_generate_named_statement(rdf_parser, 
                                                element->parent->uri,
                                                RAPIER_SUBJECT_TYPE_RESOURCE,
                                                element->name,
                                                (void*)element->uri,
                                                RAPIER_OBJECT_TYPE_RESOURCE,
                                                NULL);
              break;

          case RAPIER_ELEMENT_CONTENT_TYPE_PRESERVED:
              if(state == RAPIER_STATE_MEMBER) {
                element->parent->last_ordinal++;
                rapier_generate_statement(rdf_parser, 
                                          element->parent->uri,
                                          RAPIER_SUBJECT_TYPE_RESOURCE,
                                          (void*)&element->parent->last_ordinal,
                                          RAPIER_PREDICATE_TYPE_ORDINAL,
                                          element->content_cdata,
                                          RAPIER_OBJECT_TYPE_XML_LITERAL,
                                          NULL);
              } else
                rapier_generate_named_statement(rdf_parser, 
                                                element->parent->uri,
                                                RAPIER_SUBJECT_TYPE_RESOURCE,
                                                element->name,
                                                element->content_cdata, 
                                                RAPIER_OBJECT_TYPE_XML_LITERAL,
                                                NULL);
              
            break;
            
            default:
              rapier_parser_fatal_error(rdf_parser, "rapier_end_element_grammar state RAPIER_STATE_PROPERTYELT - unexpected content type %d\n", element->content_type);
          } /* end switch */
          
        }

        finished=1;
        break;


      default:
        rapier_parser_fatal_error(rdf_parser, "rapier_end_element_grammar - unexpected parser state %d - %s\n", state, rapier_state_as_string(state));
        finished=1;

    } /* end switch */

    if(state != element->state) {
      element->state=state;
      LIBRDF_DEBUG3(rapier_end_element_grammar, "moved to state %d - %s\n",
                    state, rapier_state_as_string(state));
    }

  } /* end while */

  LIBRDF_DEBUG3(rapier_end_element_grammar, 
                "ending in state %d - %s\n",
                state, rapier_state_as_string(state));

}
