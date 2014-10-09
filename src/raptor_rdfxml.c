/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_rdfxml.c - Raptor RDF/XML Parser
 *
 * Copyright (C) 2000-2008, David Beckett http://www.dajobe.org/
 * Copyright (C) 2000-2005, University of Bristol, UK http://www.bristol.ac.uk/
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


#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


/* Define these for far too much output */
#undef RAPTOR_DEBUG_VERBOSE
#undef RAPTOR_DEBUG_CDATA


/* Raptor structures */

typedef enum {
  /* Catch uninitialised state */
  RAPTOR_STATE_INVALID = 0,

  /* Skipping current tree of elements - used to recover finding
   * illegal content, when parsling permissively.
   */
  RAPTOR_STATE_SKIPPING,

  /* Not in RDF grammar yet - searching for a start element.
   *
   * This can be <rdf:RDF> (goto NODE_ELEMENT_LIST) but since it is optional,
   * the start element can also be one of  
   *   http://www.w3.org/TR/rdf-syntax-grammar/#nodeElementURIs
   * 
   * If RDF content is assumed, go straight to OBJ
   */
  RAPTOR_STATE_UNKNOWN,

  /* A list of node elements
   *   http://www.w3.org/TR/rdf-syntax-grammar/#nodeElementList 
   */
  RAPTOR_STATE_NODE_ELEMENT_LIST,

  /* Found an <rdf:Description> */
  RAPTOR_STATE_DESCRIPTION,

  /* Found a property element
   *   http://www.w3.org/TR/rdf-syntax-grammar/#propertyElt
   */
  RAPTOR_STATE_PROPERTYELT,

  /* A property element that is an ordinal - rdf:li, rdf:_n
   */
  RAPTOR_STATE_MEMBER_PROPERTYELT,

  /* Found a node element
   *   http://www.w3.org/TR/rdf-syntax-grammar/#nodeElement
   */
  RAPTOR_STATE_NODE_ELEMENT,

  /* A property element with rdf:parseType="Literal"
   *   http://www.w3.org/TR/rdf-syntax-grammar/#parseTypeLiteralPropertyElt
   */
  RAPTOR_STATE_PARSETYPE_LITERAL,

  /* A property element with rdf:parseType="Resource"
   *   http://www.w3.org/TR/rdf-syntax-grammar/#parseTypeResourcePropertyElt
   */
  RAPTOR_STATE_PARSETYPE_RESOURCE,

  /* A property element with rdf:parseType="Collection"
   *  http://www.w3.org/TR/rdf-syntax-grammar/#parseTypeCollectionPropertyElt
   *
   * (This also handles daml:Collection)
   */
  RAPTOR_STATE_PARSETYPE_COLLECTION,

  /* A property element with a rdf:parseType attribute and a value
   * not "Literal" or "Resource"
   *   http://www.w3.org/TR/rdf-syntax-grammar/#parseTypeOtherPropertyElt
   */
  RAPTOR_STATE_PARSETYPE_OTHER,

  RAPTOR_STATE_PARSETYPE_LAST = RAPTOR_STATE_PARSETYPE_OTHER


} raptor_state;


static const char* const raptor_state_names[RAPTOR_STATE_PARSETYPE_LAST+2] = {
  "INVALID",
  "SKIPPING",
  "UNKNOWN",
  "nodeElementList",
  "propertyElt",
  "Description",
  "propertyElt",
  "memberPropertyElt",
  "nodeElement",
  "parseTypeLiteral",
  "parseTypeResource",
  "parseTypeCollection",
  "parseTypeOther"
};


static const char * raptor_rdfxml_state_as_string(raptor_state state) 
{
  if(state < 1 || state > RAPTOR_STATE_PARSETYPE_LAST)
    state = (raptor_state)0;
  return raptor_state_names[(int)state];
}


/*
 * raptor_rdfxml_check_propertyElement_name:
 * @name: rdf namespace term
 *
 * Check if an rdf namespace name is allowed to be used as a Node Element.
 *
 * Return value: < 0 if unknown rdf namespace term, 0 if known and not allowed, > 0 if known and allowed
 */
static int
raptor_rdfxml_check_nodeElement_name(const char *name) 
{
  int i;

  if(*name == '_')
    return 1;
  
  for(i = 0; raptor_rdf_ns_terms_info[i].name; i++)
    if(!strcmp(raptor_rdf_ns_terms_info[i].name, name))
      return raptor_rdf_ns_terms_info[i].allowed_as_nodeElement;

  return -1;
}


/*
 * raptor_rdfxml_check_propertyElement_name:
 * @name: rdf namespace term
 *
 * Check if an rdf namespace name is allowed to be used as a Property Element.
 *
 * Return value: < 0 if unknown rdf namespace term, 0 if known and not allowed, > 0 if known and allowed
 */
static int
raptor_rdfxml_check_propertyElement_name(const char *name) 
{
  int i;

  if(*name == '_')
    return 1;
  
  for(i = 0; raptor_rdf_ns_terms_info[i].name; i++)
    if(!strcmp(raptor_rdf_ns_terms_info[i].name, (const char*)name))
      return raptor_rdf_ns_terms_info[i].allowed_as_propertyElement;

  return -1;
}


static int
raptor_rdfxml_check_propertyAttribute_name(const char *name) 
{
  int i;

  if(*name == '_')
    return 1;
  
  for(i = 0; raptor_rdf_ns_terms_info[i].name; i++)
    if(!strcmp(raptor_rdf_ns_terms_info[i].name, (const char*)name))
      return raptor_rdf_ns_terms_info[i].allowed_as_propertyAttribute;

  return -1;
}


typedef enum {
  /* undetermined yet - whitespace is stored */
  RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_UNKNOWN,

  /* literal content - no elements, cdata allowed, whitespace significant 
   * <propElement> blah </propElement>
   */
  RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LITERAL,

  /* parseType literal content (WF XML) - all content preserved
   * <propElement rdf:parseType="Literal"><em>blah</em></propElement> 
   */
  RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_XML_LITERAL,

  /* top-level nodes - 0+ elements expected, no cdata, whitespace ignored,
   * any non-whitespace cdata is error
   * only used for <rdf:RDF> or implict <rdf:RDF>
   */
  RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_NODES,

  /* properties - 0+ elements expected, no cdata, whitespace ignored,
   * any non-whitespace cdata is error
   * <nodeElement><prop1>blah</prop1> <prop2>blah</prop2> </nodeElement>
   */
  RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTIES,

  /* property content - all content preserved
   * any content type changes when first non-whitespace found
   * <propElement>...
   */
  RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTY_CONTENT,

  /* resource URI given - no element, no cdata, whitespace ignored,
   * any non-whitespace cdata is error 
   * <propElement rdf:resource="uri"/>
   * <propElement rdf:resource="uri"></propElement>
   */
  RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_RESOURCE,

  /* skipping content - all content is preserved 
   * Used when skipping content for unknown parseType-s,
   * error recovery, some other reason
   */
  RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PRESERVED,

  /* parseType Collection - all content preserved
   * Parsing of this determined by RDF/XML (Revised) closed collection rules
   * <propElement rdf:parseType="Collection">...</propElement>
   */
  RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_COLLECTION,

  /* Like above but handles "daml:collection" */
  RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION,

  /* dummy for use in strings below */
  RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LAST

} raptor_rdfxml_element_content_type;


static const struct {
  const char * name;
  int whitespace_significant;
  /* non-blank cdata */
  int cdata_allowed;
  /* XML element content */
  int element_allowed;
  /* Do RDF-specific processing? (property attributes, rdf: attributes, ...) */
  int rdf_processing;
} rdf_content_type_info[RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LAST]={
  {"Unknown",         1, 1, 1, 0 },
  {"Literal",         1, 1, 0, 0 },
  {"XML Literal",     1, 1, 1, 0 },
  {"Nodes",           0, 0, 1, 1 },
  {"Properties",      0, 1, 1, 1 },
  {"Property Content",1, 1, 1, 1 },
  {"Resource",        0, 0, 0, 0 },
  {"Preserved",       1, 1, 1, 0 },
  {"Collection",      1, 1, 1, 1 },
  {"DAML Collection", 1, 1, 1, 1 },
};



static const char *
raptor_rdfxml_element_content_type_as_string(raptor_rdfxml_element_content_type type) 
{
  if(type >= RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LAST)
    return "INVALID";

  return rdf_content_type_info[type].name;
}





/*
 * Raptor Element/attributes on stack 
 */
struct raptor_rdfxml_element_s {
  raptor_world* world;

  raptor_xml_element *xml_element;

  /* NULL at bottom of stack */
  struct raptor_rdfxml_element_s *parent;

  /* attributes declared in M&S */
  const unsigned char * rdf_attr[RDF_NS_LAST + 1];
  /* how many of above seen */
  int rdf_attr_count;

  /* state that this production matches */
  raptor_state state;

  /* how to handle the content inside this XML element */
  raptor_rdfxml_element_content_type content_type;


  /* starting state for children of this element */
  raptor_state child_state;

  /* starting content type for children of this element */
  raptor_rdfxml_element_content_type child_content_type;


  /* Reified statement identifier */
  raptor_term* reified;

  unsigned const char* reified_id;

  /* Bag identifier */
  raptor_term* bag;
  int last_bag_ordinal; /* starts at 0, so first predicate is rdf:_1 */

  /* Subject identifier (URI/anon ID), type, source
   *
   * When the XML element represents a node, this is the identifier 
   */
  raptor_term* subject;
  
  /* Predicate URI
   *
   * When the XML element represents a node or predicate,
   * this is the identifier of the predicate 
   */
  raptor_term* predicate;

  /* Object identifier (URI/anon ID), type, source
   *
   * When this XML element generates a statement that needs an object,
   * possibly from a child element, this is the identifier of the object 
   */
  raptor_term* object;

  /* URI of datatype of literal */
  raptor_uri *object_literal_datatype;

  /* last ordinal used, so initialising to 0 works, emitting rdf:_1 first */
  int last_ordinal;

  /* If this element's parseType is a Collection 
   * this identifies the anon node of current tail of the collection(list). 
   */
  const unsigned char *tail_id;

  /* RDF/XML specific checks */

  /* all cdata so far is whitespace */
  unsigned int content_cdata_all_whitespace;
};

typedef struct raptor_rdfxml_element_s raptor_rdfxml_element;


#define RAPTOR_RDFXML_N_CONCEPTS 5

/*
 * Raptor parser object
 */
struct raptor_rdfxml_parser_s {
  raptor_sax2 *sax2;
  
  /* stack of elements - elements add after current_element */
  raptor_rdfxml_element *root_element;
  raptor_rdfxml_element *current_element;

  raptor_uri* concepts[RAPTOR_RDFXML_N_CONCEPTS];

  /* set of seen rdf:ID / rdf:bagID values (with in-scope base URI) */
  raptor_id_set* id_set;

  void *xml_content;
  size_t xml_content_length;
  raptor_iostream* iostream;

  /* writer for building parseType="Literal" content */
  raptor_xml_writer* xml_writer;
};




/* static variables */

#define RAPTOR_DAML_NS_URI(rdf_xml_parser)   rdf_xml_parser->concepts[0]

#define RAPTOR_DAML_List_URI(rdf_xml_parser)  rdf_xml_parser->concepts[1]
#define RAPTOR_DAML_first_URI(rdf_xml_parser) rdf_xml_parser->concepts[2]
#define RAPTOR_DAML_rest_URI(rdf_xml_parser)  rdf_xml_parser->concepts[3]
#define RAPTOR_DAML_nil_URI(rdf_xml_parser)   rdf_xml_parser->concepts[4]

/* RAPTOR_RDFXML_N_CONCEPTS defines size of array */


/* prototypes for element functions */
static raptor_rdfxml_element* raptor_rdfxml_element_pop(raptor_rdfxml_parser *rdf_parser);
static void raptor_rdfxml_element_push(raptor_rdfxml_parser *rdf_parser, raptor_rdfxml_element* element);

static int raptor_rdfxml_record_ID(raptor_parser *rdf_parser, raptor_rdfxml_element *element, const unsigned char *id);

/* prototypes for grammar functions */
static void raptor_rdfxml_start_element_grammar(raptor_parser *parser, raptor_rdfxml_element *element);
static void raptor_rdfxml_end_element_grammar(raptor_parser *parser, raptor_rdfxml_element *element);
static void raptor_rdfxml_cdata_grammar(raptor_parser *parser, const unsigned char *s, int len, int is_cdata);


/* prototype for statement related functions */
static void raptor_rdfxml_generate_statement(raptor_parser *rdf_parser, raptor_term *subject,  raptor_uri *predicate_uri, raptor_term *object, raptor_term *reified, raptor_rdfxml_element *bag_element);



/* Prototypes for parsing data functions */
static int raptor_rdfxml_parse_init(raptor_parser* rdf_parser, const char *name);
static void raptor_rdfxml_parse_terminate(raptor_parser *rdf_parser);
static int raptor_rdfxml_parse_start(raptor_parser* rdf_parser);
static int raptor_rdfxml_parse_chunk(raptor_parser* rdf_parser, const unsigned char *buffer, size_t len, int is_end);
static void raptor_rdfxml_update_document_locator(raptor_parser *rdf_parser);

static raptor_uri* raptor_rdfxml_inscope_base_uri(raptor_parser *rdf_parser);


static raptor_rdfxml_element*
raptor_rdfxml_element_pop(raptor_rdfxml_parser *rdf_xml_parser) 
{
  raptor_rdfxml_element *element = rdf_xml_parser->current_element;

  if(!element)
    return NULL;

  rdf_xml_parser->current_element = element->parent;
  if(rdf_xml_parser->root_element == element) /* just deleted root */
    rdf_xml_parser->root_element = NULL;

  return element;
}


static void
raptor_rdfxml_element_push(raptor_rdfxml_parser *rdf_xml_parser, raptor_rdfxml_element* element) 
{
  element->parent = rdf_xml_parser->current_element;
  rdf_xml_parser->current_element = element;
  if(!rdf_xml_parser->root_element)
    rdf_xml_parser->root_element = element;
}


static void
raptor_free_rdfxml_element(raptor_rdfxml_element *element)
{
  int i;
  
  /* Free special RDF M&S attributes */
  for(i = 0; i <= RDF_NS_LAST; i++) 
    if(element->rdf_attr[i])
      RAPTOR_FREE(char*, element->rdf_attr[i]);

  if(element->subject)
    raptor_free_term(element->subject);
  if(element->predicate)
    raptor_free_term(element->predicate);
  if(element->object)
    raptor_free_term(element->object);
  if(element->bag)
    raptor_free_term(element->bag);
  if(element->reified)
    raptor_free_term(element->reified);

  if(element->tail_id)
    RAPTOR_FREE(char*, (char*)element->tail_id);
  if(element->object_literal_datatype)
    raptor_free_uri(element->object_literal_datatype);

  if(element->reified_id)
    RAPTOR_FREE(char*, (char*)element->reified_id);

  RAPTOR_FREE(raptor_rdfxml_element, element);
}


static void
raptor_rdfxml_sax2_new_namespace_handler(void *user_data,
                                         raptor_namespace* nspace)
{
  raptor_parser* rdf_parser;
  const unsigned char* namespace_name;
  size_t namespace_name_len;
  raptor_uri* uri = raptor_namespace_get_uri(nspace);

  rdf_parser = (raptor_parser*)user_data;
  raptor_parser_start_namespace(rdf_parser, nspace);

  if(!uri)
    return;
  
  namespace_name = raptor_uri_as_counted_string(uri, &namespace_name_len);
  
  if(namespace_name_len == raptor_rdf_namespace_uri_len-1 && 
     !strncmp((const char*)namespace_name, 
              (const char*)raptor_rdf_namespace_uri, 
              namespace_name_len)) {
    const unsigned char *prefix = raptor_namespace_get_prefix(nspace);
    raptor_parser_warning(rdf_parser,
                          "Declaring a namespace with prefix %s to URI %s - one letter short of the RDF namespace URI and probably a mistake.",
                          prefix, namespace_name);
  } 

  if(namespace_name_len > raptor_rdf_namespace_uri_len && 
     !strncmp((const char*)namespace_name,
              (const char*)raptor_rdf_namespace_uri,
              raptor_rdf_namespace_uri_len)) {
    raptor_parser_error(rdf_parser,
                        "Declaring a namespace URI %s to which the RDF namespace URI is a prefix is forbidden.",
                        namespace_name);
  }
}



static void
raptor_rdfxml_start_element_handler(void *user_data,
                                    raptor_xml_element* xml_element)
{
  raptor_parser* rdf_parser;
  raptor_rdfxml_parser* rdf_xml_parser;
  raptor_rdfxml_element* element;
  int ns_attributes_count = 0;
  raptor_qname** named_attrs = NULL;
  int i;
  int count_bumped = 0;
  
  rdf_parser = (raptor_parser*)user_data;
  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;
  
  if(rdf_parser->failed)
    return;

  raptor_rdfxml_update_document_locator(rdf_parser);

  /* Create new element structure */
  element = RAPTOR_CALLOC(raptor_rdfxml_element*, 1, sizeof(*element));
  if(!element) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    rdf_parser->failed = 1;
    return;
  }
  element->world = rdf_parser->world;
  element->xml_element = xml_element;

  raptor_rdfxml_element_push(rdf_xml_parser, element);

  named_attrs = raptor_xml_element_get_attributes(xml_element);
  ns_attributes_count = raptor_xml_element_get_attributes_count(xml_element);

  /* RDF-specific processing of attributes */
  if(ns_attributes_count) {
    raptor_qname** new_named_attrs;
    int offset = 0;
    raptor_rdfxml_element* parent_element;

    parent_element = element->parent;

    /* Allocate new array to move namespaced-attributes to if
     * rdf processing is performed
     */
    new_named_attrs = RAPTOR_CALLOC(raptor_qname**, ns_attributes_count, 
                                    sizeof(raptor_qname*));
    if(!new_named_attrs) {
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      rdf_parser->failed = 1;
      return;
    }

    for(i = 0; i < ns_attributes_count; i++) {
      raptor_qname* attr = named_attrs[i];

      /* If:
       *  1 We are handling RDF content and RDF processing is allowed on
       *    this element
       * OR
       *  2 We are not handling RDF content and 
       *    this element is at the top level (top level Desc. / typedNode)
       *    i.e. we have no parent
       * then handle the RDF attributes
       */
      if((parent_element &&
          rdf_content_type_info[parent_element->child_content_type].rdf_processing) ||
         !parent_element) {

        /* Save pointers to some RDF M&S attributes */
        
        /* If RDF namespace-prefixed attributes */
        if(attr->nspace && attr->nspace->is_rdf_ms) {
          const unsigned char *attr_name = attr->local_name;
          int j;

          for(j = 0; j <= RDF_NS_LAST; j++)
            if(!strcmp((const char*)attr_name,
                       raptor_rdf_ns_terms_info[j].name)) {
              element->rdf_attr[j] = attr->value;
              element->rdf_attr_count++;
              /* Delete it if it was stored elsewhere */
#ifdef RAPTOR_DEBUG_VERBOSE
              RAPTOR_DEBUG3("Found RDF namespace attribute '%s' URI %s\n",
                            (char*)attr_name, attr->value);
#endif
              /* make sure value isn't deleted from qname structure */
              attr->value = NULL;
              raptor_free_qname(attr);
              attr = NULL;
              break;
            }
        } /* end if RDF namespaced-prefixed attributes */

        if(!attr)
          continue;

        /* If non namespace-prefixed RDF attributes found on an element */
        if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_ALLOW_NON_NS_ATTRIBUTES) &&
           !attr->nspace) {
          const unsigned char *attr_name = attr->local_name;
          int j;

          for(j = 0; j <= RDF_NS_LAST; j++)
            if(!strcmp((const char*)attr_name,
                       raptor_rdf_ns_terms_info[j].name)) {
              element->rdf_attr[j] = attr->value;
              element->rdf_attr_count++;
              if(!raptor_rdf_ns_terms_info[j].allowed_unprefixed_on_attribute)
                raptor_parser_warning(rdf_parser,
                                      "Using rdf attribute '%s' without the RDF namespace has been deprecated.",
                                      attr_name);

              /* Delete it if it was stored elsewhere */
              /* make sure value isn't deleted from qname structure */
              attr->value = NULL;
              raptor_free_qname(attr);
              attr = NULL;
              break;
            }
        } /* end if non-namespace prefixed RDF attributes */

        if(!attr)
          continue;

      } /* end if leave literal XML alone */

      if(attr)
        new_named_attrs[offset++] = attr;
    }

    /* new attribute count is set from attributes that haven't been skipped */
    ns_attributes_count = offset;
    if(!ns_attributes_count) {
      /* all attributes were deleted so delete the new array */
      RAPTOR_FREE(raptor_qname_array, new_named_attrs);
      new_named_attrs = NULL;
    }

    RAPTOR_FREE(raptor_qname_array, named_attrs);
    named_attrs = new_named_attrs;
    raptor_xml_element_set_attributes(xml_element, 
                                      named_attrs, ns_attributes_count);
  } /* end if ns_attributes_count */


  /* start from unknown; if we have a parent, it may set this */
  element->state = RAPTOR_STATE_UNKNOWN;
  element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_UNKNOWN;

  if(element->parent && 
     element->parent->child_content_type != RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_UNKNOWN) {
    element->content_type = element->parent->child_content_type;
      
    if(element->parent->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_RESOURCE &&
       element->content_type != RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_COLLECTION &&
       element->content_type != RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) {
      raptor_qname* parent_el_name;
      parent_el_name = raptor_xml_element_get_name(element->parent->xml_element);
      /* If parent has an rdf:resource, this element should not be here */
      raptor_parser_error(rdf_parser,
                          "property element '%s' has multiple object node elements, skipping.", 
                          parent_el_name->local_name);
      element->state = RAPTOR_STATE_SKIPPING;
      element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PRESERVED;

    } else {
      if(!element->parent->child_state) {
        raptor_parser_fatal_error(rdf_parser,
                                  "%s: Internal error: no parent element child_state set",
                                  __FUNCTION__);
        return;
      }

      element->state = element->parent->child_state;
      element->parent->xml_element->content_element_seen++;
      count_bumped++;
    
      /* leave literal XML alone */
      if(!rdf_content_type_info[element->content_type].cdata_allowed) {
        if(element->parent->xml_element->content_element_seen &&
           element->parent->xml_element->content_cdata_seen) {
          raptor_qname* parent_el_name;

          parent_el_name = raptor_xml_element_get_name(element->parent->xml_element);
          /* Uh oh - mixed content, the parent element has cdata too */
          raptor_parser_warning(rdf_parser, "element '%s' has mixed content.", 
                                parent_el_name->local_name);
        }
        
        /* If there is some existing all-whitespace content cdata
         * before this node element, delete it
         */
        if(element->parent->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTIES &&
           element->parent->xml_element->content_element_seen &&
           element->parent->content_cdata_all_whitespace &&
           element->parent->xml_element->content_cdata_length) {
          
          element->parent->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_RESOURCE;
          
          raptor_free_stringbuffer(element->parent->xml_element->content_cdata_sb);
          element->parent->xml_element->content_cdata_sb = NULL;
          element->parent->xml_element->content_cdata_length = 0;
        }
        
      } /* end if leave literal XML alone */
      
    } /* end if parent has no rdf:resource */

  } /* end if element->parent */


#ifdef RAPTOR_DEBUG_VERBOSE
  RAPTOR_DEBUG2("Using content type %s\n",
                rdf_content_type_info[element->content_type].name);

  fprintf(stderr, "raptor_rdfxml_start_element_handler: Start ns-element: ");
  raptor_print_xml_element(xml_element, stderr);
#endif

  
  /* Check for non namespaced stuff when not in a parseType literal, other */
  if(rdf_content_type_info[element->content_type].rdf_processing) {
    const raptor_namespace* ns;

    ns = raptor_xml_element_get_name(xml_element)->nspace;
    /* The element */

    /* If has no namespace or the namespace has no name (xmlns="") */
    if((!ns || (ns && !raptor_namespace_get_uri(ns))) && element->parent) {
      raptor_qname* parent_el_name;

      parent_el_name = raptor_xml_element_get_name(element->parent->xml_element);

      raptor_parser_error(rdf_parser,
                          "Using an element '%s' without a namespace is forbidden.", 
                          parent_el_name->local_name);
      element->state = RAPTOR_STATE_SKIPPING;
      /* Remove count above so that parent thinks this is empty */
      if(count_bumped)
        element->parent->xml_element->content_element_seen--;
      element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PRESERVED;
    }


    /* Check for any remaining non-namespaced attributes */
    if(named_attrs) {
      for(i = 0; i < ns_attributes_count; i++) {
        raptor_qname *attr = named_attrs[i];
        /* Check if any attributes are non-namespaced */
        if(!attr->nspace ||
           (attr->nspace && !raptor_namespace_get_uri(attr->nspace))) {
          raptor_parser_error(rdf_parser,
                              "Using an attribute '%s' without a namespace is forbidden.", 
                              attr->local_name);
          raptor_free_qname(attr);
          named_attrs[i] = NULL;
        }
      }
    }
  }
  

  if(element->rdf_attr[RDF_NS_aboutEach] || 
     element->rdf_attr[RDF_NS_aboutEachPrefix]) {
    raptor_parser_warning(rdf_parser,
                          "element '%s' has aboutEach / aboutEachPrefix, skipping.", 
                          raptor_xml_element_get_name(xml_element)->local_name);
    element->state = RAPTOR_STATE_SKIPPING;
    /* Remove count above so that parent thinks this is empty */
    if(count_bumped)
      element->parent->xml_element->content_element_seen--;
    element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PRESERVED;
  }
  
  /* Right, now ready to enter the grammar */
  raptor_rdfxml_start_element_grammar(rdf_parser, element);

  return;
}


static void
raptor_rdfxml_end_element_handler(void *user_data, 
                                  raptor_xml_element* xml_element)
{
  raptor_parser* rdf_parser;
  raptor_rdfxml_parser* rdf_xml_parser;
  raptor_rdfxml_element* element;

  rdf_parser = (raptor_parser*)user_data;
  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;

  if(!rdf_parser->failed) {
    raptor_rdfxml_update_document_locator(rdf_parser);

    raptor_rdfxml_end_element_grammar(rdf_parser,
                                      rdf_xml_parser->current_element);
  }
  
  element = raptor_rdfxml_element_pop(rdf_xml_parser);
  if(element) {
    if(element->parent) {
      /* Do not change this; PROPERTYELT will turn into MEMBER if necessary
       * See the switch case for MEMBER / PROPERTYELT where the test is done.
       *
       * PARSETYPE_RESOURCE should never be propogated up since it
       * will turn the next child (node) element into a property
       */
      if(element->state != RAPTOR_STATE_MEMBER_PROPERTYELT &&
         element->state != RAPTOR_STATE_PARSETYPE_RESOURCE)
        element->parent->child_state = element->state;
    }
  
    raptor_free_rdfxml_element(element);
  }
}


/* cdata (and ignorable whitespace for libxml). 
 * s 0 terminated is for libxml
 */
static void
raptor_rdfxml_characters_handler(void *user_data, 
                                 raptor_xml_element* xml_element,
                                 const unsigned char *s, int len)
{
  raptor_parser* rdf_parser = (raptor_parser*)user_data;

  raptor_rdfxml_cdata_grammar(rdf_parser, s, len, 0);
}


/* cdata (and ignorable whitespace for libxml). 
 * s is 0 terminated for libxml2
 */
static void
raptor_rdfxml_cdata_handler(void *user_data, raptor_xml_element* xml_element,
                            const unsigned char *s, int len)
{
  raptor_parser* rdf_parser = (raptor_parser*)user_data;

  raptor_rdfxml_cdata_grammar(rdf_parser, s, len, 1);
}


/* comment handler
 * s is 0 terminated
 */
static void
raptor_rdfxml_comment_handler(void *user_data, raptor_xml_element* xml_element,
                              const unsigned char *s)
{
  raptor_parser* rdf_parser = (raptor_parser*)user_data;
  raptor_rdfxml_parser* rdf_xml_parser;
  raptor_rdfxml_element* element;

  if(rdf_parser->failed || !xml_element)
    return;

  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;
  element = rdf_xml_parser->current_element;

  if(element) {
    if(element->child_content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_XML_LITERAL)
      raptor_xml_writer_comment(rdf_xml_parser->xml_writer, s);
  }
  

#ifdef RAPTOR_DEBUG_VERBOSE
  RAPTOR_DEBUG2("XML Comment '%s'\n", s);
#endif
}


static const unsigned char* const daml_namespace_uri_string = (const unsigned char*)"http://www.daml.org/2001/03/daml+oil#";
static const int daml_namespace_uri_string_len = 37;


static int
raptor_rdfxml_parse_init(raptor_parser* rdf_parser, const char *name)
{
  raptor_rdfxml_parser* rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;
  raptor_sax2* sax2;
  raptor_world* world = rdf_parser->world;

  /* Allocate sax2 object */
  sax2 = raptor_new_sax2(rdf_parser->world, &rdf_parser->locator, rdf_parser);
  rdf_xml_parser->sax2 = sax2;
  if(!sax2)
    return 1;

  /* Initialize sax2 element handlers */
  raptor_sax2_set_start_element_handler(sax2, raptor_rdfxml_start_element_handler);
  raptor_sax2_set_end_element_handler(sax2, raptor_rdfxml_end_element_handler);
  raptor_sax2_set_characters_handler(sax2, raptor_rdfxml_characters_handler);
  raptor_sax2_set_cdata_handler(sax2, raptor_rdfxml_cdata_handler);
  raptor_sax2_set_comment_handler(sax2, raptor_rdfxml_comment_handler);
  raptor_sax2_set_namespace_handler(sax2, raptor_rdfxml_sax2_new_namespace_handler);

  /* Allocate uris */  
  RAPTOR_DAML_NS_URI(rdf_xml_parser) = raptor_new_uri_from_counted_string(world,
                                                                          daml_namespace_uri_string,
                                                                          daml_namespace_uri_string_len);

  RAPTOR_DAML_List_URI(rdf_xml_parser) = raptor_new_uri_from_uri_local_name(world, RAPTOR_DAML_NS_URI(rdf_xml_parser), (const unsigned char *)"List");
  RAPTOR_DAML_first_URI(rdf_xml_parser) = raptor_new_uri_from_uri_local_name(world, RAPTOR_DAML_NS_URI(rdf_xml_parser) ,(const unsigned char *)"first");
  RAPTOR_DAML_rest_URI(rdf_xml_parser) = raptor_new_uri_from_uri_local_name(world, RAPTOR_DAML_NS_URI(rdf_xml_parser), (const unsigned char *)"rest");
  RAPTOR_DAML_nil_URI(rdf_xml_parser) = raptor_new_uri_from_uri_local_name(world, RAPTOR_DAML_NS_URI(rdf_xml_parser), (const unsigned char *)"nil");

  /* Check for uri allocation failures */
  if(!RAPTOR_DAML_NS_URI(rdf_xml_parser) ||
     !RAPTOR_DAML_List_URI(rdf_xml_parser) ||
     !RAPTOR_DAML_first_URI(rdf_xml_parser) ||
     !RAPTOR_DAML_rest_URI(rdf_xml_parser) ||
     !RAPTOR_DAML_nil_URI(rdf_xml_parser))
    return 1;

  /* Everything succeeded */
  return 0;
}


static int
raptor_rdfxml_parse_start(raptor_parser* rdf_parser)
{
  raptor_uri *uri = rdf_parser->base_uri;
  raptor_rdfxml_parser* rdf_xml_parser;

  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;

  /* base URI required for RDF/XML */
  if(!uri)
    return 1;

  /* Optionally normalize language to lowercase
   * http://www.w3.org/TR/rdf-concepts/#dfn-language-identifier
   */
  raptor_sax2_set_option(rdf_xml_parser->sax2,
                         RAPTOR_OPTION_NORMALIZE_LANGUAGE, NULL,
                         RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_NORMALIZE_LANGUAGE));

  /* Optionally forbid internal network and file requests in the XML parser */
  raptor_sax2_set_option(rdf_xml_parser->sax2, 
                         RAPTOR_OPTION_NO_NET, NULL,
                         RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_NO_NET));
  raptor_sax2_set_option(rdf_xml_parser->sax2, 
                         RAPTOR_OPTION_NO_FILE, NULL,
                         RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_NO_FILE));
  raptor_sax2_set_option(rdf_xml_parser->sax2, 
                         RAPTOR_OPTION_LOAD_EXTERNAL_ENTITIES, NULL,
                         RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_LOAD_EXTERNAL_ENTITIES));
  if(rdf_parser->uri_filter)
    raptor_sax2_set_uri_filter(rdf_xml_parser->sax2, rdf_parser->uri_filter,
                               rdf_parser->uri_filter_user_data);

  raptor_sax2_parse_start(rdf_xml_parser->sax2, uri);

  /* Delete any existing id_set */
  if(rdf_xml_parser->id_set) {
    raptor_free_id_set(rdf_xml_parser->id_set);
    rdf_xml_parser->id_set = NULL;
  }
  
  /* Create a new id_set if needed */
  if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_CHECK_RDF_ID)) {
    rdf_xml_parser->id_set = raptor_new_id_set(rdf_parser->world);
    if(!rdf_xml_parser->id_set)
      return 1;
  }
  
  return 0;
}


static void
raptor_rdfxml_parse_terminate(raptor_parser *rdf_parser) 
{
  raptor_rdfxml_parser* rdf_xml_parser;
  raptor_rdfxml_element* element;
  int i;

  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;

  if(rdf_xml_parser->sax2) {
    raptor_free_sax2(rdf_xml_parser->sax2);
    rdf_xml_parser->sax2 = NULL;
  }
  
  while( (element = raptor_rdfxml_element_pop(rdf_xml_parser)) )
    raptor_free_rdfxml_element(element);


  for(i = 0; i < RAPTOR_RDFXML_N_CONCEPTS; i++) {
    raptor_uri* concept_uri = rdf_xml_parser->concepts[i];
    if(concept_uri) {
      raptor_free_uri(concept_uri);
      rdf_xml_parser->concepts[i] = NULL;
    }
  }
  
  if(rdf_xml_parser->id_set) {
    raptor_free_id_set(rdf_xml_parser->id_set);
    rdf_xml_parser->id_set = NULL;
  }

  if (rdf_xml_parser->xml_writer) {
    raptor_free_xml_writer(rdf_xml_parser->xml_writer);
    rdf_xml_parser->xml_writer = NULL;
  }

  if (rdf_xml_parser->iostream) {
    raptor_free_iostream(rdf_xml_parser->iostream);
    rdf_xml_parser->iostream = NULL;
  }

  if (rdf_xml_parser->xml_content) {
    RAPTOR_FREE(char*, rdf_xml_parser->xml_content);
    rdf_xml_parser->xml_content = NULL;
    rdf_xml_parser->xml_content_length = 0;
  }
}


static int
raptor_rdfxml_parse_recognise_syntax(raptor_parser_factory* factory, 
                                     const unsigned char *buffer, size_t len,
                                     const unsigned char *identifier, 
                                     const unsigned char *suffix, 
                                     const char *mime_type)
{
  int score = 0;
  
  if(suffix) {
    if(!strcmp((const char*)suffix, "rdf") || 
       !strcmp((const char*)suffix, "rdfs") ||
       !strcmp((const char*)suffix, "foaf") ||
       !strcmp((const char*)suffix, "doap") ||
       !strcmp((const char*)suffix, "owl") ||
       !strcmp((const char*)suffix, "daml"))
      score = 9;
    if(!strcmp((const char*)suffix, "rss"))
      score = 3;
  }
  
  if(identifier) {
    if(strstr((const char*)identifier, "rss1"))
      score += 5;
    else if(!suffix && strstr((const char*)identifier, "rss"))
      score += 3;
    else if(!suffix && strstr((const char*)identifier, "rdf"))
      score += 2;
    else if(!suffix && strstr((const char*)identifier, "RDF"))
      score += 2;
  }
  
  if(mime_type) {
    if(strstr((const char*)mime_type, "html"))
      score -= 4;
    else if(!strcmp((const char*)mime_type, "text/rdf"))
      score += 7;
    else if(!strcmp((const char*)mime_type, "application/xml"))
      score += 5;
  }

  if(buffer && len) {
    /* Check it's an XML namespace declared and not N3 or Turtle which
     * mention the namespace URI but not in this form.
     */
#define  HAS_RDF_XMLNS1 (raptor_memstr((const char*)buffer, len, "xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#") != NULL)
#define  HAS_RDF_XMLNS2 (raptor_memstr((const char*)buffer, len, "xmlns:rdf='http://www.w3.org/1999/02/22-rdf-syntax-ns#") != NULL)
#define  HAS_RDF_XMLNS3 (raptor_memstr((const char*)buffer, len, "xmlns=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#") != NULL)
#define  HAS_RDF_XMLNS4 (raptor_memstr((const char*)buffer, len, "xmlns='http://www.w3.org/1999/02/22-rdf-syntax-ns#") != NULL)
#define  HAS_RDF_ENTITY1 (raptor_memstr((const char*)buffer, len, "!ENTITY rdf 'http://www.w3.org/1999/02/22-rdf-syntax-ns#'") != NULL)
#define  HAS_RDF_ENTITY2 (raptor_memstr((const char*)buffer, len, "!ENTITY rdf \"http://www.w3.org/1999/02/22-rdf-syntax-ns#\"") != NULL)
#define  HAS_RDF_ENTITY3 (raptor_memstr((const char*)buffer, len, "xmlns:rdf=\"&rdf;\"") != NULL)
#define  HAS_RDF_ENTITY4 (raptor_memstr((const char*)buffer, len, "xmlns:rdf='&rdf;'") != NULL)
#define  HAS_HTML_NS (raptor_memstr((const char*)buffer, len, "http://www.w3.org/1999/xhtml") != NULL)
#define  HAS_HTML_ROOT (raptor_memstr((const char*)buffer, len, "<html") != NULL)

    if(!HAS_HTML_NS && !HAS_HTML_ROOT &&
       (HAS_RDF_XMLNS1 || HAS_RDF_XMLNS2 || HAS_RDF_XMLNS3 || HAS_RDF_XMLNS4 ||
        HAS_RDF_ENTITY1 || HAS_RDF_ENTITY2 || HAS_RDF_ENTITY3 || HAS_RDF_ENTITY4)
      ) {
      int has_rdf_RDF = (raptor_memstr((const char*)buffer, len, "<rdf:RDF") != NULL);
      int has_rdf_Description = (raptor_memstr((const char*)buffer, len, "rdf:Description") != NULL);
      int has_rdf_about = (raptor_memstr((const char*)buffer, len, "rdf:about") != NULL);

      score += 7;
      if(has_rdf_RDF)
        score++;
      if(has_rdf_Description)
        score++;
      if(has_rdf_about)
        score++;
    }
  }
  
  return score;
}



static int
raptor_rdfxml_parse_chunk(raptor_parser* rdf_parser,
                          const unsigned char *buffer,
                          size_t len, int is_end) 
{
  raptor_rdfxml_parser* rdf_xml_parser;
  int rc;
  
  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;
  if(rdf_parser->failed)
    return 1;

  rc = raptor_sax2_parse_chunk(rdf_xml_parser->sax2, buffer, len, is_end);

  if(is_end) {
    if(rdf_parser->emitted_default_graph) {
      raptor_parser_end_graph(rdf_parser, NULL, 0);
      rdf_parser->emitted_default_graph--;
    }
  }

  return rc;
}


static void
raptor_rdfxml_generate_statement(raptor_parser *rdf_parser, 
                                 raptor_term *subject_term,
                                 raptor_uri *predicate_uri,
                                 raptor_term *object_term,
                                 raptor_term *reified_term,
                                 raptor_rdfxml_element* bag_element)
{
  raptor_statement *statement = &rdf_parser->statement;
  raptor_term* predicate_term = NULL;
  int free_reified_term = 0;
  
  if(rdf_parser->failed)
    return;

#ifdef RAPTOR_DEBUG_VERBOSE
  if(!subject_term)
    RAPTOR_FATAL1("Statement has no subject\n");
  
  if(!predicate_uri)
    RAPTOR_FATAL1("Statement has no predicate\n");
  
  if(!object_term)
    RAPTOR_FATAL1("Statement has no object\n");
  
#endif

  predicate_term = raptor_new_term_from_uri(rdf_parser->world, predicate_uri);
  if(!predicate_term)
    return;

  statement->subject = subject_term;
  statement->predicate = predicate_term;
  statement->object = object_term;

#ifdef RAPTOR_DEBUG_VERBOSE
  fprintf(stderr, "raptor_rdfxml_generate_statement: Generating statement: ");
  raptor_statement_print(statement, stderr);
  fputc('\n', stderr);
#endif

  if(!rdf_parser->emitted_default_graph) {
    raptor_parser_start_graph(rdf_parser, NULL, 0);
    rdf_parser->emitted_default_graph++;
  }

  if(!rdf_parser->statement_handler)
    goto generate_tidy;

  /* Generate the statement; or is it a fact? */
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);


  /* the bagID mess */
  if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_ALLOW_BAGID) &&
     bag_element && bag_element->bag) {
    raptor_term* bag = bag_element->bag;
    raptor_uri* bag_predicate_uri = NULL;
    raptor_term* bag_predicate_term = NULL;
    
    statement->subject = bag;

    bag_element->last_bag_ordinal++;

    /* new URI object */
    bag_predicate_uri = raptor_new_uri_from_rdf_ordinal(rdf_parser->world,
                                                        bag_element->last_bag_ordinal);
    if(!bag_predicate_uri)
      goto generate_tidy;

    bag_predicate_term = raptor_new_term_from_uri(rdf_parser->world,
                                                  bag_predicate_uri);
    raptor_free_uri(bag_predicate_uri);

    if(!bag_predicate_term)
      goto generate_tidy;
    
    statement->predicate = bag_predicate_term;

    if(!reified_term || !reified_term->value.blank.string) {
      unsigned char *reified_id = NULL;

      /* reified_term is NULL so generate a bag ID */
      reified_id = raptor_world_generate_bnodeid(rdf_parser->world);
      if(!reified_id)
        goto generate_tidy;

      reified_term = raptor_new_term_from_blank(rdf_parser->world, reified_id);
      RAPTOR_FREE(char*, reified_id);

      if(!reified_term)
        goto generate_tidy;
      free_reified_term = 1;
    }
    
    statement->object = reified_term;
    (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);

    if(bag_predicate_term)
      raptor_free_term(bag_predicate_term);
  }


  /* return if is there no reified ID (that is valid) */
  if(!reified_term || !reified_term->value.blank.string)
    goto generate_tidy;


  /* otherwise generate reified statements */

  statement->subject = reified_term;
  statement->predicate = RAPTOR_RDF_type_term(rdf_parser->world);
  statement->object = RAPTOR_RDF_Statement_term(rdf_parser->world);
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);

  /* statement->subject = reified_term; */
  statement->predicate = RAPTOR_RDF_subject_term(rdf_parser->world);
  statement->object = subject_term;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);


  /* statement->subject = reified_term; */
  statement->predicate = RAPTOR_RDF_predicate_term(rdf_parser->world);
  statement->object = predicate_term;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);

  /* statement->subject = reified_term; */
  statement->predicate = RAPTOR_RDF_object_term(rdf_parser->world);
  statement->object = object_term;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);


 generate_tidy:
  /* Tidy up things allocated here */
  if(predicate_term)
    raptor_free_term(predicate_term);
  if(free_reified_term && reified_term)
    raptor_free_term(reified_term);
}



/**
 * raptor_rdfxml_element_has_property_attributes:
 * @element: element with the property attributes 
 *
 * Return true if the element has at least one property attribute.
 * 
 **/
static int
raptor_rdfxml_element_has_property_attributes(raptor_rdfxml_element *element) 
{
  int i;
  
  if(element->xml_element->attribute_count > 0)
    return 1;

  /* look for rdf: properties */
  for(i = 0; i <= RDF_NS_LAST; i++) {
    if(element->rdf_attr[i] &&
       raptor_rdf_ns_terms_info[i].type != RAPTOR_TERM_TYPE_UNKNOWN)
      return 1;
  }
  return 0;
}


/**
 * raptor_rdfxml_process_property_attributes:
 * @rdf_parser: Raptor parser object
 * @attributes_element: element with the property attributes 
 * @resource_element: element that defines the resource URI 
 *                    subject->value etc.
 * @property_node_identifier: Use this identifier for the resource URI
 *   and count any ordinals for it locally
 *
 * Process the property attributes for an element for a given resource.
 * 
 **/
static int
raptor_rdfxml_process_property_attributes(raptor_parser *rdf_parser, 
                                          raptor_rdfxml_element *attributes_element,
                                          raptor_rdfxml_element *resource_element,
                                          raptor_term *property_node_identifier)
{
  unsigned int i;
  raptor_term *resource_identifier;

  resource_identifier = property_node_identifier ? property_node_identifier : resource_element->subject;
  

  /* Process attributes as propAttr* = * (propName="string")*
   */
  for(i = 0; i < attributes_element->xml_element->attribute_count; i++) {
    raptor_qname* attr = attributes_element->xml_element->attributes[i];
    const unsigned char *name;
    const unsigned char *value;
    int handled = 0;

    if(!attr)
      continue;
    
    name = attr->local_name;
    value = attr->value;

    if(!attr->nspace) {
      raptor_rdfxml_update_document_locator(rdf_parser);
      raptor_parser_error(rdf_parser,
                          "Using property attribute '%s' without a namespace is forbidden.",
                          name);
      continue;
    }


    if(!raptor_unicode_check_utf8_nfc_string(value, strlen((const char*)value),
                                             NULL)) {
      const char *message;

      message = "Property attribute '%s' has a string not in Unicode Normal Form C: %s";
      raptor_rdfxml_update_document_locator(rdf_parser);
      if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_NON_NFC_FATAL))
        raptor_parser_error(rdf_parser, message, name, value);
      else
        raptor_parser_warning(rdf_parser, message, name, value);
      continue;
    }
    

    /* Generate the property statement using one of these properties:
     * 1) rdf:_n
     * 2) the URI from the rdf:* attribute where allowed
     * 3) otherwise forbidden (including rdf:li)
     */
    if(attr->nspace->is_rdf_ms) {
      /* is rdf: namespace */
        
      if(*name == '_') {
        int ordinal;

        /* recognise rdf:_ */
        name++;
        ordinal = raptor_check_ordinal(name);
        if(ordinal < 1) {
          raptor_rdfxml_update_document_locator(rdf_parser);
          raptor_parser_error(rdf_parser,
                              "Illegal ordinal value %d in property attribute '%s' seen on containing element '%s'.",
                              ordinal, attr->local_name, name);
        }
      } else {
        int rc;

        raptor_rdfxml_update_document_locator(rdf_parser);

        rc = raptor_rdfxml_check_propertyAttribute_name((const char*)name);
        if(!rc)
          raptor_parser_error(rdf_parser,
                              "RDF term %s is forbidden as a property attribute.",
                              name);
        else if(rc < 0)
          raptor_parser_warning(rdf_parser,
                                "Unknown RDF namespace property attribute '%s'.", 
                                name);
      }

    } /* end is RDF namespace property */


    if(!handled) {
      raptor_term* object_term;

      object_term = raptor_new_term_from_literal(rdf_parser->world,
                                                 (unsigned char*)value,
                                                 NULL, NULL);
    
      /* else not rdf: namespace or unknown in rdf: namespace so
       * generate a statement with a literal object
       */
      raptor_rdfxml_generate_statement(rdf_parser, 
                                       resource_identifier,
                                       attr->uri,
                                       object_term,
                                       NULL, /* Property attributes are never reified*/
                                       resource_element);

      raptor_free_term(object_term);
    }
    
  } /* end for ... attributes */


  /* Handle rdf property attributes
   * (only rdf:type and rdf:value at present) 
   */
  for(i = 0; i <= RDF_NS_LAST; i++) {
    const unsigned char *value = attributes_element->rdf_attr[i];
    size_t value_len;
    int object_is_literal;
    raptor_uri *property_uri;
    raptor_term* object_term;
      
    if(!value)
      continue;

    value_len = strlen((const char*)value);
    
    object_is_literal = (raptor_rdf_ns_terms_info[i].type == RAPTOR_TERM_TYPE_LITERAL);

    if(raptor_rdf_ns_terms_info[i].type == RAPTOR_TERM_TYPE_UNKNOWN) {
      const char *name = raptor_rdf_ns_terms_info[i].name;
      int rc = raptor_rdfxml_check_propertyAttribute_name(name);
      if(!rc) {
        raptor_rdfxml_update_document_locator(rdf_parser);
        raptor_parser_error(rdf_parser,
                            "RDF term %s is forbidden as a property attribute.",
                            name);
        continue;
      } else if(rc < 0)
        raptor_parser_warning(rdf_parser,
                              "Unknown RDF namespace property attribute '%s'.", 
                              name);
    }

    if(object_is_literal &&
       !raptor_unicode_check_utf8_nfc_string(value, value_len, NULL)) {
      const char *message;
      message = "Property attribute '%s' has a string not in Unicode Normal Form C: %s";
      raptor_rdfxml_update_document_locator(rdf_parser);
      if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_NON_NFC_FATAL))
        raptor_parser_error(rdf_parser, message,
                            raptor_rdf_ns_terms_info[i].name, value);
      else
        raptor_parser_warning(rdf_parser, message,
                              raptor_rdf_ns_terms_info[i].name, value);
      continue;
    }

    property_uri = raptor_new_uri_for_rdf_concept(rdf_parser->world, 
                                                  (const unsigned char*)raptor_rdf_ns_terms_info[i].name);
    
    if(object_is_literal) {
      object_term = raptor_new_term_from_literal(rdf_parser->world,
                                                 (unsigned char*)value,
                                                 NULL, NULL);
    } else {
      raptor_uri *base_uri;
      raptor_uri *object_uri;
      base_uri = raptor_rdfxml_inscope_base_uri(rdf_parser);
      object_uri = raptor_new_uri_relative_to_base(rdf_parser->world, 
                                                   base_uri, value);
      object_term = raptor_new_term_from_uri(rdf_parser->world, object_uri);
      raptor_free_uri(object_uri);
    }
    
    raptor_rdfxml_generate_statement(rdf_parser,
                                     resource_identifier,
                                     property_uri,
                                     object_term,
                                     NULL, /* Property attributes are never reified*/
                                     resource_element);

    raptor_free_term(object_term);

    raptor_free_uri(property_uri);
    
  } /* end for rdf:property values */
  
  return 0;
}


static void
raptor_rdfxml_start_element_grammar(raptor_parser *rdf_parser,
                                    raptor_rdfxml_element *element) 
{
  raptor_rdfxml_parser *rdf_xml_parser;
  int finished;
  raptor_state state;
  raptor_xml_element* xml_element;
  raptor_qname* el_qname;
  const unsigned char *el_name;
  int element_in_rdf_ns;
  int rc = 0;
  raptor_uri* base_uri;
  raptor_uri* element_name_uri;

  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;

  xml_element = element->xml_element;
  el_qname = raptor_xml_element_get_name(xml_element);
  el_name = el_qname->local_name;
  element_in_rdf_ns = (el_qname->nspace && el_qname->nspace->is_rdf_ms);
  base_uri = raptor_rdfxml_inscope_base_uri(rdf_parser);
  element_name_uri = el_qname->uri;
  
  state = element->state;
#ifdef RAPTOR_DEBUG_VERBOSE
  RAPTOR_DEBUG2("Starting in state %s\n", raptor_rdfxml_state_as_string(state));
#endif

  finished = 0;
  while(!finished) {

    switch(state) {
      case RAPTOR_STATE_SKIPPING:
        element->child_state = state;
        element->child_content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PRESERVED;
        finished = 1;
        break;
        
      case RAPTOR_STATE_UNKNOWN:
        /* found <rdf:RDF> ? */

        if(element_in_rdf_ns) {
          if(raptor_uri_equals(element_name_uri,
                               RAPTOR_RDF_RDF_URI(rdf_parser->world))) {
            element->child_state = RAPTOR_STATE_NODE_ELEMENT_LIST;
            element->child_content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_NODES;
            /* Yes - need more content before can continue,
             * so wait for another element
             */
            finished = 1;
            break;
          }
          if(raptor_uri_equals(element_name_uri,
                               RAPTOR_RDF_Description_URI(rdf_parser->world))) {
            state = RAPTOR_STATE_DESCRIPTION;
            element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTIES;
            /* Yes - found something so move immediately to description */
            break;
          }

          if(element_in_rdf_ns) {
            rc = raptor_rdfxml_check_nodeElement_name((const char*)el_name);
            if(!rc) {
              raptor_parser_error(rdf_parser,
                                  "rdf:%s is forbidden as a node element.",
                                  el_name);
              state = RAPTOR_STATE_SKIPPING;
              element->child_state = RAPTOR_STATE_SKIPPING;
              finished = 1;
              break;
            } else if(rc < 0) {
              raptor_parser_warning(rdf_parser, 
                                    "rdf:%s is an unknown RDF namespaced element.", 
                                    el_name);
            }
          }
        }

        /* If scanning for element, can continue */
        if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_SCANNING)) {
          finished = 1;
          break;
        }

        /* Otherwise the choice of the next state can be made
         * from the current element by the OBJ state
         */
        state = RAPTOR_STATE_NODE_ELEMENT_LIST;
        element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_NODES;
        break;


      case RAPTOR_STATE_NODE_ELEMENT_LIST:
        /* Handling
         *   http://www.w3.org/TR/rdf-syntax-grammar/#nodeElementList 
         *
         * Everything goes to nodeElement
         */

        state = RAPTOR_STATE_NODE_ELEMENT;

        element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTIES;

        break;



      case RAPTOR_STATE_DESCRIPTION:
      case RAPTOR_STATE_NODE_ELEMENT:
      case RAPTOR_STATE_PARSETYPE_RESOURCE:
      case RAPTOR_STATE_PARSETYPE_COLLECTION:
        /* Handling <rdf:Description> or other node element
         *   http://www.w3.org/TR/rdf-syntax-grammar/#nodeElement
         *
         * or a property element acting as a node element for
         * rdf:parseType="Resource"
         *   http://www.w3.org/TR/rdf-syntax-grammar/#parseTypeResourcePropertyElt
         * or rdf:parseType="Collection" (and daml:Collection)
         *   http://www.w3.org/TR/rdf-syntax-grammar/#parseTypeCollectionPropertyElt
         *
         * Only create a bag if bagID given
         */

        if(!element_name_uri) {
          /* We cannot handle this */
          raptor_parser_warning(rdf_parser, "Using node element '%s' without a namespace is forbidden.", 
                                el_qname->local_name);
          raptor_rdfxml_update_document_locator(rdf_parser);
          element->state = RAPTOR_STATE_SKIPPING;
          element->child_state = RAPTOR_STATE_SKIPPING;
          finished = 1;
          break;
        }

        if(element_in_rdf_ns) {
          rc = raptor_rdfxml_check_nodeElement_name((const char*)el_name);
          if(!rc) {
            raptor_parser_error(rdf_parser,
                                "rdf:%s is forbidden as a node element.",
                                el_name);
            state = RAPTOR_STATE_SKIPPING;
            element->state = RAPTOR_STATE_SKIPPING;
            element->child_state = RAPTOR_STATE_SKIPPING;
            finished = 1;
            break;
          } else if(rc < 0) {
            raptor_parser_warning(rdf_parser,
                                  "rdf:%s is an unknown RDF namespaced element.", 
                                  el_name);
          }
        }

        if(element->content_type != RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_COLLECTION &&
           element->content_type != RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION && 
           element->parent && 
           (element->parent->state == RAPTOR_STATE_PROPERTYELT ||
            element->parent->state == RAPTOR_STATE_MEMBER_PROPERTYELT) &&
           element->parent->xml_element->content_element_seen > 1) {
          raptor_rdfxml_update_document_locator(rdf_parser);
          raptor_parser_error(rdf_parser, "The enclosing property already has an object");
          state = RAPTOR_STATE_SKIPPING;
          element->child_state = RAPTOR_STATE_SKIPPING;
          finished = 1;
          break;
        }

        if(state == RAPTOR_STATE_NODE_ELEMENT || 
           state == RAPTOR_STATE_DESCRIPTION || 
           state == RAPTOR_STATE_PARSETYPE_COLLECTION) {
          if(element_in_rdf_ns &&
             raptor_uri_equals(element_name_uri,
                               RAPTOR_RDF_Description_URI(rdf_parser->world)))
            state = RAPTOR_STATE_DESCRIPTION;
          else
            state = RAPTOR_STATE_NODE_ELEMENT;
        }
        

        if((element->rdf_attr[RDF_NS_ID]!=NULL) +
           (element->rdf_attr[RDF_NS_about]!=NULL) +
           (element->rdf_attr[RDF_NS_nodeID]!=NULL) > 1) {
          raptor_rdfxml_update_document_locator(rdf_parser);
          raptor_parser_error(rdf_parser, "Multiple attributes of rdf:ID, rdf:about and rdf:nodeID on element '%s' - only one allowed.", el_name);
        }

        if(element->rdf_attr[RDF_NS_ID]) {
          unsigned char* subject_id;
          raptor_uri* subject_uri;
          
          subject_id = (unsigned char*)element->rdf_attr[RDF_NS_ID];

          if(!raptor_valid_xml_ID(rdf_parser, subject_id)) {
            raptor_parser_error(rdf_parser, "Illegal rdf:ID value '%s'", 
                                subject_id);
            state = RAPTOR_STATE_SKIPPING;
            element->child_state = RAPTOR_STATE_SKIPPING;
            finished = 1;
            break;
          }
          if(raptor_rdfxml_record_ID(rdf_parser, element, subject_id)) {
            raptor_parser_error(rdf_parser, "Duplicated rdf:ID value '%s'",
                                subject_id);
            state = RAPTOR_STATE_SKIPPING;
            element->child_state = RAPTOR_STATE_SKIPPING;
            finished = 1;
            break;
          }

          /* after this, subject_id is the owner of the ID string */
          element->rdf_attr[RDF_NS_ID] = NULL;

          subject_uri = raptor_new_uri_from_id(rdf_parser->world, base_uri,
                                               subject_id);
          RAPTOR_FREE(char*, subject_id);

          if(!subject_uri)
            goto oom;
          element->subject = raptor_new_term_from_uri(rdf_parser->world,
                                                      subject_uri);
          raptor_free_uri(subject_uri);

          if(!element->subject)
            goto oom;

        } else if(element->rdf_attr[RDF_NS_about]) {
          raptor_uri* subject_uri;

          subject_uri = raptor_new_uri_relative_to_base(rdf_parser->world,
                                                        base_uri,
                                                        (const unsigned char*)element->rdf_attr[RDF_NS_about]);
          if(!subject_uri)
            goto oom;
          
          element->subject = raptor_new_term_from_uri(rdf_parser->world,
                                                      subject_uri);
          raptor_free_uri(subject_uri);

          RAPTOR_FREE(char*, element->rdf_attr[RDF_NS_about]);
          element->rdf_attr[RDF_NS_about] = NULL;
          if(!element->subject)
            goto oom;

        } else if(element->rdf_attr[RDF_NS_nodeID]) {
          unsigned char* subject_id;
          subject_id = raptor_world_internal_generate_id(rdf_parser->world,
                                                         (unsigned char*)element->rdf_attr[RDF_NS_nodeID]);
          if(!subject_id)
            goto oom;
          
          element->subject = raptor_new_term_from_blank(rdf_parser->world,
                                                        subject_id);
          RAPTOR_FREE(char*, subject_id);

          element->rdf_attr[RDF_NS_nodeID] = NULL;
          if(!element->subject)
            goto oom;

          if(!raptor_valid_xml_ID(rdf_parser, element->subject->value.blank.string)) {
            raptor_parser_error(rdf_parser, "Illegal rdf:nodeID value '%s'", 
                                (const char*)element->subject->value.blank.string);
            state = RAPTOR_STATE_SKIPPING;
            element->child_state = RAPTOR_STATE_SKIPPING;
            finished = 1;
            break;
          }
        } else if(element->parent && 
                   element->parent->child_content_type != RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_COLLECTION &&
                   element->parent->child_content_type != RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION &&
                   element->parent->object) {
          /* copy from parent (property element), it has a URI for us */
          element->subject = raptor_term_copy(element->parent->object);
        } else {
          unsigned char* subject_id;
          subject_id = raptor_world_generate_bnodeid(rdf_parser->world);
          if(!subject_id)
            goto oom;

          element->subject = raptor_new_term_from_blank(rdf_parser->world,
                                                        subject_id);
          RAPTOR_FREE(char*, subject_id);

          if(!element->subject)
            goto oom;
        }


        if(element->rdf_attr[RDF_NS_bagID]) {
          if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_ALLOW_BAGID)) {
            unsigned char* bag_id;
            raptor_uri* bag_uri = NULL;

            bag_id = (unsigned char*)element->rdf_attr[RDF_NS_bagID];
            element->rdf_attr[RDF_NS_bagID] = NULL;

            bag_uri = raptor_new_uri_from_id(rdf_parser->world, 
                                             base_uri, bag_id);
            if(!bag_uri) {
              RAPTOR_FREE(char*, bag_id);
              goto oom;
            }

            element->bag = raptor_new_term_from_uri(rdf_parser->world, bag_uri);
            raptor_free_uri(bag_uri);

            if(!raptor_valid_xml_ID(rdf_parser, bag_id)) {
              raptor_parser_error(rdf_parser, "Illegal rdf:bagID value '%s'", 
                                  bag_id);
              state = RAPTOR_STATE_SKIPPING;
              element->child_state = RAPTOR_STATE_SKIPPING;
              finished = 1;
              RAPTOR_FREE(char*, bag_id);
              break;
            }
            if(raptor_rdfxml_record_ID(rdf_parser, element, bag_id)) {
              raptor_parser_error(rdf_parser, "Duplicated rdf:bagID value '%s'",
                                  bag_id);
              state = RAPTOR_STATE_SKIPPING;
              element->child_state = RAPTOR_STATE_SKIPPING;
              finished = 1;
              RAPTOR_FREE(char*, bag_id);
              break;
            }

            RAPTOR_FREE(char*, bag_id);
            raptor_parser_warning(rdf_parser, "rdf:bagID is deprecated.");

            
            raptor_rdfxml_generate_statement(rdf_parser, 
                                             element->bag,
                                             RAPTOR_RDF_type_URI(rdf_parser->world),
                                             RAPTOR_RDF_Bag_term(rdf_parser->world),
                                             NULL,
                                             NULL);
          } else {
            /* bagID forbidden */
            raptor_parser_error(rdf_parser, "rdf:bagID is forbidden.");
            state = RAPTOR_STATE_SKIPPING;
            element->child_state = RAPTOR_STATE_SKIPPING;
            finished = 1;
            break;
          }
        }


        if(element->parent) {

          /* In a rdf:parseType="Collection" the resources are appended
           * to the list at the genid element->parent->tail_id
           */
          if(element->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_COLLECTION ||
             element->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) {
            /* <idList> rdf:type rdf:List */
            const unsigned char * idList;
            raptor_uri *predicate_uri;
            raptor_term* idList_term;
            raptor_term* object_term;
              
            idList = raptor_world_generate_bnodeid(rdf_parser->world);
            if(!idList)
              goto oom;
            /* idList string is saved below in element->parent->tail_id */

            idList_term = raptor_new_term_from_blank(rdf_parser->world, idList);
            if(!idList_term) {
              RAPTOR_FREE(char*, idList);
              goto oom;
            }

            if((element->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) ||
               RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_ALLOW_RDF_TYPE_RDF_LIST)) {
              raptor_uri* class_uri = NULL;

              if(element->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) {
                class_uri = RAPTOR_DAML_List_URI(rdf_xml_parser);
                object_term = raptor_new_term_from_uri(rdf_parser->world,
                                                       class_uri);
              } else
                object_term = raptor_term_copy(RAPTOR_RDF_List_term(rdf_parser->world));
              
              raptor_rdfxml_generate_statement(rdf_parser, 
                                               idList_term,
                                               RAPTOR_RDF_type_URI(rdf_parser->world),
                                               object_term,
                                               NULL,
                                               element);
              raptor_free_term(object_term);
            }

            predicate_uri = (element->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) ? RAPTOR_DAML_first_URI(rdf_xml_parser) : RAPTOR_RDF_first_URI(rdf_parser->world);

            /* <idList> rdf:first <element->uri> */
            raptor_rdfxml_generate_statement(rdf_parser, 
                                             idList_term,
                                             predicate_uri,
                                             element->subject,
                                             NULL,
                                             NULL);
            
            /* If there is no rdf:parseType="Collection" */
            if(!element->parent->tail_id) {
              /* Free any existing object still around.
               * I suspect this can never happen.
               */
              if(element->parent->object)
                raptor_free_term(element->parent->object);

              element->parent->object = raptor_new_term_from_blank(rdf_parser->world,
                                                                   idList);
            } else {
              raptor_term* tail_id_term;

              tail_id_term = raptor_new_term_from_blank(rdf_parser->world, 
                                                        element->parent->tail_id);
              
              predicate_uri = (element->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) ? RAPTOR_DAML_rest_URI(rdf_xml_parser) : RAPTOR_RDF_rest_URI(rdf_parser->world);

              /* _:tail_id rdf:rest _:listRest */
              raptor_rdfxml_generate_statement(rdf_parser, 
                                               tail_id_term,
                                               predicate_uri,
                                               idList_term,
                                               NULL,
                                               NULL);

              raptor_free_term(tail_id_term);
            }

            /* update new tail */
            if(element->parent->tail_id)
              RAPTOR_FREE(char*, (char*)element->parent->tail_id);

            element->parent->tail_id = idList;
            
            raptor_free_term(idList_term);
          } else if(element->parent->state != RAPTOR_STATE_UNKNOWN &&
                    element->state != RAPTOR_STATE_PARSETYPE_RESOURCE) {
            /* If there is a parent element (property) containing this
             * element (node) and it has no object, set it from this subject
             */
            
            if(element->parent->object) {
              raptor_rdfxml_update_document_locator(rdf_parser);
              raptor_parser_error(rdf_parser,
                                  "Tried to set multiple objects of a statement");
            } else {
              /* Store URI of this node in our parent as the property object */
              element->parent->object = raptor_term_copy(element->subject);
              element->parent->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_RESOURCE;
            }

          }
        }
        

        /* If this is a node element, generate the rdf:type statement
         * from this node
         */
        if(state == RAPTOR_STATE_NODE_ELEMENT) {
          raptor_term* el_name_term;
              
          el_name_term = raptor_new_term_from_uri(rdf_parser->world,
                                                  element_name_uri);

          raptor_rdfxml_generate_statement(rdf_parser, 
                                           element->subject,
                                           RAPTOR_RDF_type_URI(rdf_parser->world),
                                           el_name_term,
                                           element->reified,
                                           element);

          raptor_free_term(el_name_term);
        }

        if(raptor_rdfxml_process_property_attributes(rdf_parser, element,
                                                     element, NULL))
          goto oom;

        /* for both productions now need some more content or
         * property elements before can do any more work.
         */

        element->child_state = RAPTOR_STATE_PROPERTYELT;
        element->child_content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTIES;
        finished = 1;
        break;


      case RAPTOR_STATE_PARSETYPE_OTHER:
        /* FALLTHROUGH */

      case RAPTOR_STATE_PARSETYPE_LITERAL:
        raptor_xml_writer_start_element(rdf_xml_parser->xml_writer, xml_element);
        element->child_state = RAPTOR_STATE_PARSETYPE_LITERAL;
        element->child_content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_XML_LITERAL;
        
        finished = 1;
        break;

        /* Handle all the detail of the various options of property element
         *   http://www.w3.org/TR/rdf-syntax-grammar/#propertyElt
         *
         * All the attributes must be scanned here to see what additional
         * property element work is needed.  No triples are generated
         * until the end of this element, until it is clear if the
         * element was empty.
         */
      case RAPTOR_STATE_MEMBER_PROPERTYELT:
      case RAPTOR_STATE_PROPERTYELT:

        if(!element_name_uri) {
          raptor_parser_error(rdf_parser, "Using property element '%s' without a namespace is forbidden.", 
                              raptor_xml_element_get_name(element->parent->xml_element)->local_name);
          raptor_rdfxml_update_document_locator(rdf_parser);
          element->state = RAPTOR_STATE_SKIPPING;
          element->child_state = RAPTOR_STATE_SKIPPING;
          finished = 1;
          break;
        }

        /* Handling rdf:li as a property, noting special processing */ 
        if(element_in_rdf_ns && 
           raptor_uri_equals(element_name_uri,
                             RAPTOR_RDF_li_URI(rdf_parser->world))) {
          state = RAPTOR_STATE_MEMBER_PROPERTYELT;
        }


        if(element_in_rdf_ns) {
          rc = raptor_rdfxml_check_propertyElement_name((const char*)el_name);
          if(!rc) {
            raptor_parser_error(rdf_parser, 
                                "rdf:%s is forbidden as a property element.",
                                el_name);
            state = RAPTOR_STATE_SKIPPING;
            element->child_state = RAPTOR_STATE_SKIPPING;
            finished = 1;
            break;
          } else if(rc < 0) {
            raptor_parser_warning(rdf_parser,
                                  "rdf:%s is an unknown RDF namespaced element.",
                                  el_name);
          }
        }
          

        /* rdf:ID on a property element - reify a statement. 
         * Allowed on all property element forms
         */
        if(element->rdf_attr[RDF_NS_ID]) {
          raptor_uri *reified_uri;
          
          element->reified_id = element->rdf_attr[RDF_NS_ID];
          element->rdf_attr[RDF_NS_ID] = NULL;
          reified_uri = raptor_new_uri_from_id(rdf_parser->world, base_uri,
                                               element->reified_id);
          if(!reified_uri)
            goto oom;

          element->reified = raptor_new_term_from_uri(rdf_parser->world,
                                                      reified_uri);
          raptor_free_uri(reified_uri);

          if(!element->reified)
            goto oom;

          if(!raptor_valid_xml_ID(rdf_parser, element->reified_id)) {
            raptor_parser_error(rdf_parser, "Illegal rdf:ID value '%s'",
                                element->reified_id);
            state = RAPTOR_STATE_SKIPPING;
            element->child_state = RAPTOR_STATE_SKIPPING;
            finished = 1;
            break;
          }
          if(raptor_rdfxml_record_ID(rdf_parser, element, element->reified_id)) {
            raptor_parser_error(rdf_parser, "Duplicated rdf:ID value '%s'",
                                element->reified_id);
            state = RAPTOR_STATE_SKIPPING;
            element->child_state = RAPTOR_STATE_SKIPPING;
            finished = 1;
            break;
          }
        }
        
        /* rdf:datatype on a property element.  
         * Only allowed for
         *   http://www.w3.org/TR/rdf-syntax-grammar/#literalPropertyElt 
         */
        if(element->rdf_attr[RDF_NS_datatype]) {
          raptor_uri *datatype_uri;
          
          datatype_uri = raptor_new_uri_relative_to_base(rdf_parser->world, 
                                                         base_uri,
                                                         (const unsigned char*)element->rdf_attr[RDF_NS_datatype]);
          element->object_literal_datatype = datatype_uri;
          RAPTOR_FREE(char*, element->rdf_attr[RDF_NS_datatype]);
          element->rdf_attr[RDF_NS_datatype] = NULL; 
          if(!element->object_literal_datatype)
            goto oom;
        }

        if(element->rdf_attr[RDF_NS_bagID]) {

          if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_ALLOW_BAGID)) {

            if(element->rdf_attr[RDF_NS_resource] ||
               element->rdf_attr[RDF_NS_parseType]) {
              
              raptor_parser_error(rdf_parser, "rdf:bagID is forbidden on property element '%s' with an rdf:resource or rdf:parseType attribute.", el_name);
              /* prevent this being used later either */
              RAPTOR_FREE(char*, element->rdf_attr[RDF_NS_bagID]);
              element->rdf_attr[RDF_NS_bagID] = NULL;
            } else {
              unsigned char* bag_id;
              raptor_uri* bag_uri;
              
              bag_id = (unsigned char*)element->rdf_attr[RDF_NS_bagID];
              element->rdf_attr[RDF_NS_bagID] = NULL;
              bag_uri = raptor_new_uri_from_id(rdf_parser->world, base_uri,
                                               bag_id);
              if(!bag_uri) {
                RAPTOR_FREE(char*, bag_id);
                goto oom;
              }

              element->bag = raptor_new_term_from_uri(rdf_parser->world,
                                                      bag_uri);
              raptor_free_uri(bag_uri);

              if(!element->bag) {
                RAPTOR_FREE(char*, bag_id);
                goto oom;
              }
              
              if(!raptor_valid_xml_ID(rdf_parser, bag_id)) {
                raptor_parser_error(rdf_parser, "Illegal rdf:bagID value '%s'",
                                    bag_id);
                state = RAPTOR_STATE_SKIPPING;
                element->child_state = RAPTOR_STATE_SKIPPING;
                finished = 1;
                RAPTOR_FREE(char*, bag_id);
                break;
              }
              if(raptor_rdfxml_record_ID(rdf_parser, element, bag_id)) {
                raptor_parser_error(rdf_parser,
                                    "Duplicated rdf:bagID value '%s'", bag_id);
                state = RAPTOR_STATE_SKIPPING;
                element->child_state = RAPTOR_STATE_SKIPPING;
                RAPTOR_FREE(char*, bag_id);
                finished = 1;
                break;
              }

              RAPTOR_FREE(char*, bag_id);
              raptor_parser_warning(rdf_parser, "rdf:bagID is deprecated.");
            }
          } else {
            /* bagID forbidden */
            raptor_parser_error(rdf_parser, "rdf:bagID is forbidden.");
            state = RAPTOR_STATE_SKIPPING;
            element->child_state = RAPTOR_STATE_SKIPPING;
            finished = 1;
            break;
          }
        } /* if rdf:bagID on property element */
        

        element->child_content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTY_CONTENT;

        if(element->rdf_attr[RDF_NS_parseType]) {
          const unsigned char *parse_type;
          int i;
          int is_parseType_Literal = 0;

          parse_type = element->rdf_attr[RDF_NS_parseType];

          if(raptor_rdfxml_element_has_property_attributes(element)) {
            raptor_parser_error(rdf_parser, "Property attributes cannot be used with rdf:parseType='%s'", parse_type);
            state = RAPTOR_STATE_SKIPPING;
            element->child_state = RAPTOR_STATE_SKIPPING;
            finished = 1;
            break;
          }

          /* Check for bad combinations of things with parseType */
          for(i = 0; i <= RDF_NS_LAST; i++)
            if(element->rdf_attr[i] && i != RDF_NS_parseType) {
              raptor_parser_error(rdf_parser, "Attribute '%s' cannot be used with rdf:parseType='%s'", raptor_rdf_ns_terms_info[i].name, parse_type);
              state = RAPTOR_STATE_SKIPPING;
              element->child_state = RAPTOR_STATE_SKIPPING;
              break;
            }


          if(!strcmp((char*)parse_type, "Literal"))
            is_parseType_Literal = 1;
          else if(!strcmp((char*)parse_type, "Resource")) {
            unsigned char* subject_id;
            
            state = RAPTOR_STATE_PARSETYPE_RESOURCE;
            element->child_state = RAPTOR_STATE_PROPERTYELT;
            element->child_content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTIES;

            /* create a node for the subject of the contained properties */
            subject_id = raptor_world_generate_bnodeid(rdf_parser->world);
            if(!subject_id)
              goto oom;

            element->subject = raptor_new_term_from_blank(rdf_parser->world,
                                                          subject_id);
            RAPTOR_FREE(char*, subject_id);

            if(!element->subject)
              goto oom;
          } else if(!strcmp((char*)parse_type, "Collection")) {
            /* An rdf:parseType="Collection" appears as a single node */
            element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_RESOURCE;
            element->child_state = RAPTOR_STATE_PARSETYPE_COLLECTION;
            element->child_content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_COLLECTION;
          } else {
            if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_ALLOW_OTHER_PARSETYPES) &&
               !raptor_strcasecmp((char*)parse_type, "daml:collection")) {
                /* A DAML collection appears as a single node */
                element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_RESOURCE;
                element->child_state = RAPTOR_STATE_PARSETYPE_COLLECTION;
                element->child_content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION;
            } else {
              if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_WARN_OTHER_PARSETYPES)) {
                raptor_parser_warning(rdf_parser, "Unknown rdf:parseType value '%s' taken as 'Literal'", parse_type);
              }
              is_parseType_Literal = 1;
            }
            
          }
          
          if(is_parseType_Literal) {
            raptor_xml_writer* xml_writer;

            /* rdf:parseType="Literal" - explicitly or default
             * if the parseType value is not recognised
             */
            rdf_xml_parser->xml_content = NULL;
            rdf_xml_parser->xml_content_length = 0;
            rdf_xml_parser->iostream =
              raptor_new_iostream_to_string(rdf_parser->world,
                                            &rdf_xml_parser->xml_content,
                                            &rdf_xml_parser->xml_content_length,
                                            raptor_alloc_memory);
            if(!rdf_xml_parser->iostream)
              goto oom;
            xml_writer = raptor_new_xml_writer(rdf_parser->world, NULL,
                                               rdf_xml_parser->iostream);
            rdf_xml_parser->xml_writer = xml_writer;
            if(!rdf_xml_parser->xml_writer)
              goto oom;
            
            raptor_xml_writer_set_option(rdf_xml_parser->xml_writer, 
                                         RAPTOR_OPTION_WRITER_XML_DECLARATION,
                                         NULL, 0);

            element->child_state = RAPTOR_STATE_PARSETYPE_LITERAL;
            element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_XML_LITERAL;
            element->child_content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_XML_LITERAL;
          }
        } else {

          /* Can only be the empty property element case
           *   http://www.w3.org/TR/rdf-syntax-grammar/#emptyPropertyElt
           */

          /* The presence of the rdf:resource or rdf:nodeID
           * attributes is checked at element close time
           */

          /*
           * Assign reified URI here so we don't reify property attributes
           * using this id
           */
          if(element->reified_id && !element->reified) {
            raptor_uri* reified_uri;
            reified_uri = raptor_new_uri_from_id(rdf_parser->world, base_uri,
                                                 element->reified_id);
            if(!reified_uri)
              goto oom;
            element->reified = raptor_new_term_from_uri(rdf_parser->world,
                                                        reified_uri);
            raptor_free_uri(reified_uri);

            if(!element->reified)
              goto oom;
          }

          if(element->rdf_attr[RDF_NS_resource] ||
             element->rdf_attr[RDF_NS_nodeID]) {
            /* Done - wait for end of this element to end in order to 
             * check the element was empty as expected */
            element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_RESOURCE;
          } else {
            /* Otherwise process content in obj (value) state */
            element->child_state = RAPTOR_STATE_NODE_ELEMENT_LIST;
            element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTY_CONTENT;
          }
        }

        finished = 1;

        break;


      case RAPTOR_STATE_INVALID:
      default:
        raptor_parser_fatal_error(rdf_parser,
                                  "%s Internal error - unexpected parser state %d - %s",
                                  __FUNCTION__,
                                  state, raptor_rdfxml_state_as_string(state));
        finished = 1;

    } /* end switch */

    if(state != element->state) {
      element->state = state;
#ifdef RAPTOR_DEBUG_VERBOSE
      RAPTOR_DEBUG3("Moved to state %d - %s\n", state,
                    raptor_rdfxml_state_as_string(state));
#endif
    }

  } /* end while */

#ifdef RAPTOR_DEBUG_VERBOSE
  RAPTOR_DEBUG2("Ending in state %s\n", raptor_rdfxml_state_as_string(state));
#endif

  return;

  oom:
  raptor_parser_fatal_error(rdf_parser, "Out of memory, skipping");
  element->state = RAPTOR_STATE_SKIPPING;
}


static void
raptor_rdfxml_end_element_grammar(raptor_parser *rdf_parser,
                                  raptor_rdfxml_element *element) 
{
  raptor_rdfxml_parser *rdf_xml_parser;
  raptor_state state;
  int finished;
  raptor_xml_element* xml_element = element->xml_element;
  raptor_qname* el_qname;
  const unsigned char *el_name;
  int element_in_rdf_ns;
  raptor_uri* element_name_uri;

  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;

  el_qname = raptor_xml_element_get_name(xml_element);
  el_name = el_qname->local_name;
  element_in_rdf_ns= (el_qname->nspace && el_qname->nspace->is_rdf_ms);
  element_name_uri = el_qname->uri;


  state = element->state;
#ifdef RAPTOR_DEBUG_VERBOSE
  RAPTOR_DEBUG2("Starting in state %s\n", raptor_rdfxml_state_as_string(state));
#endif

  finished= 0;
  while(!finished) {
    switch(state) {
      case RAPTOR_STATE_SKIPPING:
        finished = 1;
        break;

      case RAPTOR_STATE_UNKNOWN:
        finished = 1;
        break;

      case RAPTOR_STATE_NODE_ELEMENT_LIST:
        if(element_in_rdf_ns && 
           raptor_uri_equals(element_name_uri,
                             RAPTOR_RDF_RDF_URI(rdf_parser->world))) {
          /* end of RDF - boo hoo */
          state = RAPTOR_STATE_UNKNOWN;
          finished = 1;
          break;
        }
        /* When scanning, another element ending is outside the RDF
         * world so this can happen without further work
         */
        if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_SCANNING)) {
          state = RAPTOR_STATE_UNKNOWN;
          finished = 1;
          break;
        }
        /* otherwise found some junk after RDF content in an RDF-only 
         * document (probably never get here since this would be
         * a mismatched XML tag and cause an error earlier)
         */
        raptor_rdfxml_update_document_locator(rdf_parser);
        raptor_parser_warning(rdf_parser,
                              "Element '%s' ended, expected end of RDF element",
                              el_name);
        state = RAPTOR_STATE_UNKNOWN;
        finished = 1;
        break;


      case RAPTOR_STATE_DESCRIPTION:
      case RAPTOR_STATE_NODE_ELEMENT:
      case RAPTOR_STATE_PARSETYPE_RESOURCE:

        /* If there is a parent element containing this element and
         * the parent isn't a description, has an identifier,
         * create the statement between this node using parent property
         * (Need to check for identifier so that top-level typed nodes
         * don't get connect to <rdf:RDF> parent element)
         */
        if(state == RAPTOR_STATE_NODE_ELEMENT && 
           element->parent && element->parent->subject) {
          raptor_rdfxml_generate_statement(rdf_parser, 
                                           element->parent->subject,
                                           element_name_uri,
                                           element->subject,
                                           NULL,
                                           element);
        } else if(state == RAPTOR_STATE_PARSETYPE_RESOURCE && 
                  element->parent && element->parent->subject) {
          /* Handle rdf:li as the rdf:parseType="resource" property */
          if(element_in_rdf_ns && 
             raptor_uri_equals(element_name_uri,
                               RAPTOR_RDF_li_URI(rdf_parser->world))) {
            raptor_uri* ordinal_predicate_uri;
            
            element->parent->last_ordinal++;
            ordinal_predicate_uri = raptor_new_uri_from_rdf_ordinal(rdf_parser->world, element->parent->last_ordinal);

            raptor_rdfxml_generate_statement(rdf_parser, 
                                             element->parent->subject,
                                             ordinal_predicate_uri,
                                             element->subject,
                                             element->reified,
                                             element->parent);
            raptor_free_uri(ordinal_predicate_uri);
          } else {
            raptor_rdfxml_generate_statement(rdf_parser, 
                                             element->parent->subject,
                                             element_name_uri,
                                             element->subject,
                                             element->reified,
                                             element->parent);
          }
        }
        finished = 1;
        break;

      case RAPTOR_STATE_PARSETYPE_COLLECTION:

        finished = 1;
        break;

      case RAPTOR_STATE_PARSETYPE_OTHER:
        /* FALLTHROUGH */

      case RAPTOR_STATE_PARSETYPE_LITERAL:
        element->parent->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_XML_LITERAL;

        raptor_xml_writer_end_element(rdf_xml_parser->xml_writer, xml_element);

        finished = 1;
        break;


      case RAPTOR_STATE_PROPERTYELT:
      case RAPTOR_STATE_MEMBER_PROPERTYELT:
        /* A property element
         *   http://www.w3.org/TR/rdf-syntax-grammar/#propertyElt
         *
         * Literal content part is handled here.
         * The element content is handled in the internal states
         * Empty content is checked here.
         */

        if(element->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTY_CONTENT) {
          if(xml_element->content_cdata_seen) 
            element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LITERAL;
          else if(xml_element->content_element_seen) 
            element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTIES;
          else {
            /* Empty Literal */
            element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LITERAL;
          }
          
        }


        /* Handle terminating a rdf:parseType="Collection" list */
        if(element->child_content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_COLLECTION ||
           element->child_content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) {
          raptor_term* nil_term;

          if(element->child_content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) {
            raptor_uri* nil_uri = RAPTOR_DAML_nil_URI(rdf_xml_parser);
            nil_term = raptor_new_term_from_uri(rdf_parser->world, nil_uri);
          } else {
            nil_term = raptor_term_copy(RAPTOR_RDF_nil_term(rdf_parser->world));
          }
          
          if(!element->tail_id) {
            /* If No List: set object of statement to rdf:nil */
            element->object = raptor_term_copy(nil_term);
          } else {
            raptor_uri* rest_uri = NULL;
            raptor_term* tail_id_term;
            
            if(element->child_content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION)
              rest_uri =  RAPTOR_DAML_rest_URI(rdf_xml_parser);
            else
              rest_uri = RAPTOR_RDF_rest_URI(rdf_parser->world);

            tail_id_term = raptor_new_term_from_blank(rdf_parser->world, 
                                                      element->tail_id);

            /* terminate the list */
            raptor_rdfxml_generate_statement(rdf_parser, 
                                             tail_id_term,
                                             rest_uri,
                                             nil_term,
                                             NULL,
                                             NULL);

            raptor_free_term(tail_id_term);
          }

          raptor_free_term(nil_term);
          
        } /* end rdf:parseType="Collection" termination */
        

#ifdef RAPTOR_DEBUG_VERBOSE
        RAPTOR_DEBUG3("Content type %s (%d)\n",
                      raptor_rdfxml_element_content_type_as_string(element->content_type),
                      element->content_type);
#endif

        switch(element->content_type) {
          case RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_RESOURCE:

            if(raptor_rdfxml_element_has_property_attributes(element) &&
               element->child_state == RAPTOR_STATE_DESCRIPTION) {
              raptor_parser_error(rdf_parser,
                                  "Property element '%s' has both property attributes and a node element content",
                                  el_name);
              state = RAPTOR_STATE_SKIPPING;
              element->child_state = RAPTOR_STATE_SKIPPING;
              break;
            }

            if(!element->object) {
              if(element->rdf_attr[RDF_NS_resource]) {
                raptor_uri* resource_uri;
                resource_uri = raptor_new_uri_relative_to_base(rdf_parser->world,
                                                               raptor_rdfxml_inscope_base_uri(rdf_parser),
                                                               (const unsigned char*)element->rdf_attr[RDF_NS_resource]);
                if(!resource_uri)
                  goto oom;
                
                element->object = raptor_new_term_from_uri(rdf_parser->world,
                                                           resource_uri);
                raptor_free_uri(resource_uri);

                RAPTOR_FREE(char*, element->rdf_attr[RDF_NS_resource]);
                element->rdf_attr[RDF_NS_resource] = NULL;
                if(!element->object)
                  goto oom;
                element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_RESOURCE;
              } else if(element->rdf_attr[RDF_NS_nodeID]) {
                unsigned char* resource_id;
                resource_id = raptor_world_internal_generate_id(rdf_parser->world,
                                                                (unsigned char*)element->rdf_attr[RDF_NS_nodeID]);
                if(!resource_id)
                  goto oom;
                
                element->object = raptor_new_term_from_blank(rdf_parser->world,
                                                             resource_id);
                RAPTOR_FREE(char*, resource_id);
                element->rdf_attr[RDF_NS_nodeID] = NULL;
                if(!element->object)
                  goto oom;

                element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_RESOURCE;
                if(!raptor_valid_xml_ID(rdf_parser,
                                        element->object->value.blank.string)) {
                  raptor_parser_error(rdf_parser, "Illegal rdf:nodeID value '%s'", (const char*)element->object->value.blank.string);
                  state = RAPTOR_STATE_SKIPPING;
                  element->child_state = RAPTOR_STATE_SKIPPING;
                  break;
                }
              } else {
                unsigned char* resource_id;
                resource_id = raptor_world_generate_bnodeid(rdf_parser->world);
                if(!resource_id)
                  goto oom;
                
                element->object = raptor_new_term_from_blank(rdf_parser->world,
                                                             resource_id);
                RAPTOR_FREE(char*, resource_id);

                if(!element->object)
                  goto oom;
                element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_RESOURCE;
              }

              if(raptor_rdfxml_process_property_attributes(rdf_parser, element, 
                                                           element->parent, 
                                                           element->object))
                 goto oom;

            }

            /* We know object is a resource, so delete any unsignficant
             * whitespace so that FALLTHROUGH code below finds the object.
             */
            if(xml_element->content_cdata_length) {
              raptor_free_stringbuffer(xml_element->content_cdata_sb);
              xml_element->content_cdata_sb = NULL;
              xml_element->content_cdata_length = 0;
            }

            /* FALLTHROUGH */
          case RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LITERAL:

            if(element->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LITERAL) {

              if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_ALLOW_BAGID)) {
                /* Only an empty literal can have a rdf:bagID */
                if(element->bag) {
                  if(xml_element->content_cdata_length > 0) {
                    raptor_parser_error(rdf_parser,
                                        "rdf:bagID is forbidden on a literal property element '%s'.",
                                        el_name);

                    /* prevent this being used later either */
                    element->rdf_attr[RDF_NS_bagID] = NULL;
                  } else {
                    raptor_rdfxml_generate_statement(rdf_parser, 
                                                     element->bag,
                                                     RAPTOR_RDF_type_URI(rdf_parser->world),
                                                     RAPTOR_RDF_Bag_term(rdf_parser->world),
                                                     NULL,
                                                     NULL);
                  }
                }
              } /* if rdf:bagID */

              /* If there is empty literal content with properties
               * generate a node to hang properties off 
               */
              if(raptor_rdfxml_element_has_property_attributes(element) &&
                 xml_element->content_cdata_length > 0) {
                raptor_parser_error(rdf_parser,
                                    "Literal property element '%s' has property attributes", 
                                    el_name);
                state = RAPTOR_STATE_SKIPPING;
                element->child_state = RAPTOR_STATE_SKIPPING;
                break;
              }

              if(element->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LITERAL &&
                 raptor_rdfxml_element_has_property_attributes(element) &&
                 !element->object) {
                unsigned char* object_id;
                object_id = raptor_world_generate_bnodeid(rdf_parser->world);
                if(!object_id)
                  goto oom;
                
                element->object = raptor_new_term_from_blank(rdf_parser->world,
                                                             object_id);
                RAPTOR_FREE(char*, object_id);

                if(!element->object)
                  goto oom;
                element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_RESOURCE;
              }
              
              if(raptor_rdfxml_process_property_attributes(rdf_parser, element, 
                                                           element,
                                                           element->object))
                 goto oom;
            }
            

            /* just be friendly to older compilers and don't declare
             * variables in the middle of a block
             */
            if(1) {
              raptor_uri *predicate_uri = NULL;
              int predicate_ordinal = -1;
              raptor_term* object_term = NULL;
              
              if(state == RAPTOR_STATE_MEMBER_PROPERTYELT) {
                predicate_ordinal = ++element->parent->last_ordinal;
                predicate_uri = raptor_new_uri_from_rdf_ordinal(rdf_parser->world,
                                                                predicate_ordinal);

              } else {
                predicate_uri = element_name_uri;
              }


              if(element->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LITERAL) {
                unsigned char* literal = NULL;
                raptor_uri* literal_datatype;
                unsigned char* literal_language = NULL;

                /* an empty stringbuffer - empty CDATA - is OK */
                if(raptor_stringbuffer_length(xml_element->content_cdata_sb)) {
                  literal = raptor_stringbuffer_as_string(xml_element->content_cdata_sb);
                  if(!literal)
                    goto oom;
                }
                
                literal_datatype = element->object_literal_datatype;
                if(!literal_datatype)
                  literal_language = (unsigned char*)raptor_sax2_inscope_xml_language(rdf_xml_parser->sax2);

                if(!literal_datatype && literal &&
                   !raptor_unicode_check_utf8_nfc_string(literal,
                                                         xml_element->content_cdata_length,
                                                         NULL)) {
                  const char *message;
                  message = "Property element '%s' has a string not in Unicode Normal Form C: %s";
                  raptor_rdfxml_update_document_locator(rdf_parser);
                  if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_NON_NFC_FATAL))
                    raptor_parser_error(rdf_parser, message, el_name, literal);
                  else
                    raptor_parser_warning(rdf_parser, message, el_name, literal);
                }

                object_term = raptor_new_term_from_literal(rdf_parser->world,
                                                           literal,
                                                           literal_datatype,
                                                           literal_language);
              } else {
                object_term = raptor_term_copy(element->object);
              }

              raptor_rdfxml_generate_statement(rdf_parser, 
                                               element->parent->subject,
                                               predicate_uri,
                                               object_term,
                                               element->reified,
                                               element->parent);

              if(predicate_ordinal >= 0)
                raptor_free_uri(predicate_uri);

              raptor_free_term(object_term);
            }
            
            break;

        case RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PRESERVED:
        case RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_XML_LITERAL:
            {
              unsigned char *buffer;
              size_t length;
              raptor_term* xmlliteral_term = NULL;
              
              if(rdf_xml_parser->xml_writer) {
                raptor_xml_writer_flush(rdf_xml_parser->xml_writer);

                raptor_free_iostream(rdf_xml_parser->iostream);
                rdf_xml_parser->iostream = NULL;
                
                buffer = (unsigned char*)rdf_xml_parser->xml_content;
                length = rdf_xml_parser->xml_content_length;
              } else {
                buffer = raptor_stringbuffer_as_string(xml_element->content_cdata_sb);
                length = xml_element->content_cdata_length;
              }

              if(!raptor_unicode_check_utf8_nfc_string(buffer, length, NULL)) {
                const char *message;
                message = "Property element '%s' has XML literal content not in Unicode Normal Form C: %s";
                raptor_rdfxml_update_document_locator(rdf_parser);
                if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_NON_NFC_FATAL))
                  raptor_parser_error(rdf_parser, message, el_name, buffer);
                else
                  raptor_parser_warning(rdf_parser, message, el_name, buffer);
              }

              xmlliteral_term = raptor_new_term_from_literal(rdf_parser->world,
                                                             buffer,
                                                             RAPTOR_RDF_XMLLiteral_URI(rdf_parser->world),
                                                             NULL);
              
              if(state == RAPTOR_STATE_MEMBER_PROPERTYELT) {
                raptor_uri* predicate_uri;
                
                element->parent->last_ordinal++;
                predicate_uri = raptor_new_uri_from_rdf_ordinal(rdf_parser->world, element->parent->last_ordinal);

                raptor_rdfxml_generate_statement(rdf_parser, 
                                                 element->parent->subject,
                                                 predicate_uri,
                                                 xmlliteral_term,
                                                 element->reified,
                                                 element->parent);

                raptor_free_uri(predicate_uri);
              } else {
                raptor_rdfxml_generate_statement(rdf_parser, 
                                                 element->parent->subject,
                                                 element_name_uri,
                                                 xmlliteral_term,
                                                 element->reified,
                                                 element->parent);
              }
              
              raptor_free_term(xmlliteral_term);

              /* Finish the xml writer iostream for parseType="Literal" */
              if(rdf_xml_parser->xml_writer) {
                raptor_free_xml_writer(rdf_xml_parser->xml_writer);
                rdf_xml_parser->xml_writer = NULL;
                RAPTOR_FREE(char*, rdf_xml_parser->xml_content);
                rdf_xml_parser->xml_content = NULL;
                rdf_xml_parser->xml_content_length = 0;
              }
            }
            
          break;

          case RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_COLLECTION:
          case RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_DAML_COLLECTION:

          case RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_NODES:
          case RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTIES:
          case RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTY_CONTENT:
            
          case RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_UNKNOWN:
          case RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LAST:
          default:
            raptor_parser_fatal_error(rdf_parser,
                                      "%s: Internal error in state RAPTOR_STATE_PROPERTYELT - got unexpected content type %s (%d)",
                                      __FUNCTION__,
                                      raptor_rdfxml_element_content_type_as_string(element->content_type),
                                      element->content_type);
        } /* end switch */

      finished = 1;
      break;

      case RAPTOR_STATE_INVALID:
      default:
        raptor_parser_fatal_error(rdf_parser,
                                  "%s: Internal error - unexpected parser state %d - %s",
                                  __FUNCTION__,
                                  state,
                                  raptor_rdfxml_state_as_string(state));
        finished = 1;

    } /* end switch */

    if(state != element->state) {
      element->state = state;
#ifdef RAPTOR_DEBUG_VERBOSE
      RAPTOR_DEBUG3("Moved to state %d - %s\n", state,
                    raptor_rdfxml_state_as_string(state));
#endif
    }

  } /* end while */

#ifdef RAPTOR_DEBUG_VERBOSE
  RAPTOR_DEBUG2("Ending in state %s\n", raptor_rdfxml_state_as_string(state));
#endif

  return;

  oom:
  raptor_parser_fatal_error(rdf_parser, "Out of memory, skipping");
  element->state = RAPTOR_STATE_SKIPPING;
}



static void
raptor_rdfxml_cdata_grammar(raptor_parser *rdf_parser,
                            const unsigned char *s, int len,
                            int is_cdata)
{
  raptor_rdfxml_parser* rdf_xml_parser;
  raptor_rdfxml_element* element;
  raptor_xml_element* xml_element;
  raptor_state state;
  int all_whitespace = 1;
  int i;

  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;

  if(rdf_parser->failed)
    return;

#ifdef RAPTOR_DEBUG_CDATA
  RAPTOR_DEBUG2("Adding characters (is_cdata=%d): '", is_cdata);
  (void)fwrite(s, 1, len, stderr);
  fprintf(stderr, "' (%d bytes)\n", len);
#endif

  for(i = 0; i < len; i++)
    if(!isspace(s[i])) {
      all_whitespace = 0;
      break;
    }

  element = rdf_xml_parser->current_element;

  /* this file is very broke - probably not XML, whatever */
  if(!element)
    return;

  xml_element = element->xml_element;
  
  raptor_rdfxml_update_document_locator(rdf_parser);

  /* cdata never changes the parser state 
   * and the containing element state always determines what to do.
   * Use the child_state first if there is one, since that applies
   */
  state = element->child_state;
#ifdef RAPTOR_DEBUG_VERBOSE
  RAPTOR_DEBUG2("Working in state %s\n", raptor_rdfxml_state_as_string(state));
#endif


#ifdef RAPTOR_DEBUG_VERBOSE
  RAPTOR_DEBUG3("Content type %s (%d)\n",
                raptor_rdfxml_element_content_type_as_string(element->content_type),
                element->content_type);
#endif
  


  if(state == RAPTOR_STATE_SKIPPING)
    return;

  if(state == RAPTOR_STATE_UNKNOWN) {
    /* Ignore all cdata if still looking for RDF */
    if(RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_SCANNING))
      return;

    /* Ignore all whitespace cdata before first element */
    if(all_whitespace)
      return;
    
    /* This probably will never happen since that would make the
     * XML not be well-formed
     */
    raptor_parser_warning(rdf_parser, "Character data before RDF element.");
  }


  if(element->child_content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTIES) {
    /* If found non-whitespace content, move to literal content */
    if(!all_whitespace)
      element->child_content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LITERAL; 
  }


  if(!rdf_content_type_info[element->child_content_type].whitespace_significant) {

    /* Whitespace is ignored except for literal or preserved content types */
    if(all_whitespace) {
#ifdef RAPTOR_DEBUG_CDATA
      RAPTOR_DEBUG2("Ignoring whitespace cdata inside element '%s'\n",
                    raptor_xml_element_get_name(element->parent->xml_element)->local_name);
#endif
      return;
    }

    if(xml_element->content_cdata_seen && xml_element->content_element_seen) {
      raptor_qname* parent_el_name;

      parent_el_name = raptor_xml_element_get_name(element->parent->xml_element);
      /* Uh oh - mixed content, this element has elements too */
      raptor_parser_warning(rdf_parser, "element '%s' has mixed content.", 
                            parent_el_name->local_name);
    }
  }


  if(element->content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_PROPERTY_CONTENT) {
    element->content_type = RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_LITERAL;
#ifdef RAPTOR_DEBUG_VERBOSE
    RAPTOR_DEBUG3("Content type changed to %s (%d)\n",
                  raptor_rdfxml_element_content_type_as_string(element->content_type),
                  element->content_type);
#endif
  }

  if(element->child_content_type == RAPTOR_RDFXML_ELEMENT_CONTENT_TYPE_XML_LITERAL)
    raptor_xml_writer_cdata_counted(rdf_xml_parser->xml_writer, s, len);
  else {
    raptor_stringbuffer_append_counted_string(xml_element->content_cdata_sb,
                                              s, len, 1);
    element->content_cdata_all_whitespace &= all_whitespace;
    
    /* adjust stored length */
    xml_element->content_cdata_length += len;
  }


#ifdef RAPTOR_DEBUG_CDATA
  RAPTOR_DEBUG3("Content cdata now: %d bytes\n",
                xml_element->content_cdata_length);
#endif
#ifdef RAPTOR_DEBUG_VERBOSE
  RAPTOR_DEBUG2("Ending in state %s\n", raptor_rdfxml_state_as_string(state));
#endif
}



/**
 * raptor_rdfxml_inscope_base_uri:
 * @rdf_parser: Raptor parser object
 *
 * Return the in-scope base URI.
 * 
 * Looks for the innermost xml:base on an element or document URI
 * 
 * Return value: The URI string value or NULL on failure.
 **/
static raptor_uri*
raptor_rdfxml_inscope_base_uri(raptor_parser *rdf_parser)
{
  raptor_rdfxml_parser* rdf_xml_parser;
  raptor_uri* base_uri;

  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;

  base_uri = raptor_sax2_inscope_base_uri(rdf_xml_parser->sax2);
  if(!base_uri)
    base_uri = rdf_parser->base_uri;

  return base_uri;
}


/**
 * raptor_rdfxml_record_ID:
 * @rdf_parser: Raptor parser object
 * @element: Current element
 * @id: ID string
 *
 * Record an rdf:ID / rdf:bagID value (with xml base) and check it hasn't been seen already.
 * 
 * Record and check the ID values, if they have been seen already.
 * per in-scope-base URI.
 * 
 * Return value: non-zero if already seen, or failure
 **/
static int
raptor_rdfxml_record_ID(raptor_parser *rdf_parser,
                        raptor_rdfxml_element *element,
                        const unsigned char *id)
{
  raptor_rdfxml_parser *rdf_xml_parser;
  raptor_uri* base_uri;
  size_t id_len;
  int rc;
  
  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;

  if(!RAPTOR_OPTIONS_GET_NUMERIC(rdf_parser, RAPTOR_OPTION_CHECK_RDF_ID))
    return 0;

  base_uri = raptor_rdfxml_inscope_base_uri(rdf_parser);

  id_len = strlen((const char*)id);

  rc = raptor_id_set_add(rdf_xml_parser->id_set, base_uri, id, id_len);

  return (rc != 0);
}



static void
raptor_rdfxml_update_document_locator(raptor_parser *rdf_parser)
{
  raptor_rdfxml_parser *rdf_xml_parser;

  rdf_xml_parser = (raptor_rdfxml_parser*)rdf_parser->context;

  raptor_sax2_update_document_locator(rdf_xml_parser->sax2,
                                      &rdf_parser->locator);
}



static void
raptor_rdfxml_parse_finish_factory(raptor_parser_factory* factory)
{
}


static const char* const rdfxml_names[3] = { "rdfxml", "raptor", NULL};

static const char* const rdfxml_uri_strings[3] = {
  "http://www.w3.org/ns/formats/RDF_XML",
  "http://www.w3.org/TR/rdf-syntax-grammar",
  NULL
};

#define RDFXML_TYPES_COUNT 2
static const raptor_type_q rdfxml_types[RDFXML_TYPES_COUNT + 1] = {
  { "application/rdf+xml", 19, 10}, 
  { "text/rdf", 8, 6},
  { NULL, 0, 0}
};

static int
raptor_rdfxml_parser_register_factory(raptor_parser_factory *factory) 
{
  int rc = 0;

  factory->desc.names = rdfxml_names;

  factory->desc.mime_types = rdfxml_types;

  factory->desc.label = "RDF/XML";
  factory->desc.uri_strings = rdfxml_uri_strings;

  factory->desc.flags = RAPTOR_SYNTAX_NEED_BASE_URI;
  
  factory->context_length     = sizeof(raptor_rdfxml_parser);
  
  factory->init      = raptor_rdfxml_parse_init;
  factory->terminate = raptor_rdfxml_parse_terminate;
  factory->start     = raptor_rdfxml_parse_start;
  factory->chunk     = raptor_rdfxml_parse_chunk;
  factory->finish_factory = raptor_rdfxml_parse_finish_factory;
  factory->recognise_syntax = raptor_rdfxml_parse_recognise_syntax;

  return rc;
}


int
raptor_init_parser_rdfxml(raptor_world* world)
{
  return !raptor_world_register_parser_factory(world,
                                               &raptor_rdfxml_parser_register_factory);
}


#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
void
raptor_rdfxml_parser_stats_print(raptor_rdfxml_parser* rdf_xml_parser, 
                                 FILE *stream)
{
  fputs("rdf:ID set ", stream);
  raptor_id_set_stats_print(rdf_xml_parser->id_set, stream);
}
#endif
