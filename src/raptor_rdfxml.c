/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_parse.c - Redland Parser for RDF (RAPTOR)
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


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <win32_config.h>
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

/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


/* Define this for far too much output */
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


static const char * const raptor_state_names[RAPTOR_STATE_PARSETYPE_LAST+2]={
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


static const char * raptor_state_as_string(raptor_state state) 
{
  if(state<1 || state > RAPTOR_STATE_PARSETYPE_LAST)
    state=(raptor_state)0;
  return raptor_state_names[(int)state];
}


/* These are used in the RDF/XML syntax as attributes, not
 * elements and are mostly not concepts in the RDF model 
 * except where indicated - either rdf:Property or rdfs:Class
 */
typedef enum {
  RDF_ATTR_about           = 0, /* value of rdf:about attribute */
  RDF_ATTR_aboutEach       = 1, /* " rdf:aboutEach */
  RDF_ATTR_aboutEachPrefix = 2, /* " rdf:aboutEachPrefix */
  RDF_ATTR_ID              = 3, /* " rdf:ID */
  RDF_ATTR_bagID           = 4, /* " rdf:bagID */
  RDF_ATTR_resource        = 5, /* " rdf:resource */
  RDF_ATTR_parseType       = 6, /* " rdf:parseType */
  RDF_ATTR_nodeID          = 7, /* " rdf:nodeID */
  RDF_ATTR_datatype        = 8, /* " rdf:datatype */
  /* rdf:Property-s */
  RDF_ATTR_type            = 9, /* " rdf:type -- a property in RDF Model */
  RDF_ATTR_value           = 10, /* " rdf:value -- a property in RDF model */
  RDF_ATTR_subject         = 11, /* " rdf:subject -- a property in RDF model */
  RDF_ATTR_predicate       = 12, /* " rdf:predicate -- a property in RDF model */
  RDF_ATTR_object          = 13, /* " rdf:object -- a property in RDF model */
  RDF_ATTR_first           = 14, /* " rdf:first -- a property in RDF model */
  RDF_ATTR_rest            = 15, /* " rdf:rest -- a property in RDF model */
  /* rdfs:Class-s */
  RDF_ATTR_Seq             = 16, /* " rdf:Seq -- a class in RDF Model */
  RDF_ATTR_Bag             = 17, /* " rdf:Bag -- a class in RDF model */
  RDF_ATTR_Alt             = 18, /* " rdf:Alt -- a class in RDF model */
  RDF_ATTR_Statement       = 19, /* " rdf:Statement -- a class in RDF model */
  RDF_ATTR_Property        = 20, /* " rdf:Property -- a class in RDF model */
  RDF_ATTR_List            = 21, /* " rdf:List -- a class in RDF model */
  RDF_ATTR_XMLLiteral      = 22, /* " rdf:XMLLiteral - a cless in RDF graph */
  /* rdfs:Resource-s */
  RDF_ATTR_nil             = 23, /* " rdf:nil -- a resource in RDF graph */

  RDF_ATTR_LAST            = RDF_ATTR_nil
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
  int allowed_unprefixed_on_attribute;
} rdf_attr_info[RDF_ATTR_LAST+1]={
  { "about",           RAPTOR_IDENTIFIER_TYPE_UNKNOWN , 1 },
  { "aboutEach",       RAPTOR_IDENTIFIER_TYPE_UNKNOWN , 0 },
  { "aboutEachPrefix", RAPTOR_IDENTIFIER_TYPE_UNKNOWN , 0 },
  { "ID",              RAPTOR_IDENTIFIER_TYPE_UNKNOWN , 1 },
  { "bagID",           RAPTOR_IDENTIFIER_TYPE_UNKNOWN , 1 },
  { "resource",        RAPTOR_IDENTIFIER_TYPE_UNKNOWN , 1 },
  { "parseType",       RAPTOR_IDENTIFIER_TYPE_UNKNOWN , 1 },
  { "nodeID",          RAPTOR_IDENTIFIER_TYPE_UNKNOWN , 0 },
  { "datatype",        RAPTOR_IDENTIFIER_TYPE_UNKNOWN , 0 },
  /* rdf:Property-s */
  { "type",            RAPTOR_IDENTIFIER_TYPE_RESOURCE, 1 },
  { "value",           RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 },
  { "subject",         RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 },
  { "predicate",       RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 },
  { "object",          RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 },
  { "first",           RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 },
  { "rest",            RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 },
  /* rdfs:Class-s */
  { "Seq",             RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 },
  { "Bag",             RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 },
  { "Alt",             RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 },
  { "Statement",       RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 },
  { "Property",        RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 },
  { "List",            RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 },
  { "XMLLiteral",      RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 },
  /* rdfs:Resource-s */
  { "nil",             RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 }
};


/*
 * http://www.w3.org/TR/rdf-syntax-grammar/#section-grammar-summary
 * coreSyntaxTerms := rdf:RDF | rdf:ID | rdf:about | rdf:bagID | 
                      rdf:parseType | rdf:resource | rdf:nodeID | rdf:datatype
 * syntaxTerms     := coreSyntaxTerms | rdf:Description | rdf:li
 * oldTerms        := rdf:aboutEach | rdf:aboutEachPrefix
 *
 * nodeElementURIs       := anyURI - ( coreSyntaxTerms | rdf:li | oldTerms )
 * propertyElementURIs   := anyURI - ( coreSyntaxTerms | rdf:Description | oldTerms )
 * propertyAttributeURIs := anyURI - ( coreSyntaxTerms | rdf:Description | rdf:li | oldTerms )
 *
 * So, forbidden terms in the RDF namespace are:
 * nodeElements 
 *   RDF | ID | about | bagID | parseType | resource | nodeID | datatype |
 *   li | aboutEach | aboutEachPrefix
 *
 * propertyElements
 *   RDF | ID | about | bagID | parseType | resource | nodeID | datatype |
 *   Description | aboutEach | aboutEachPrefix
 *
 * propertyAttributes
 *   RDF | ID | about | bagID | parseType | resource | nodeID | datatype |
 *   Description | li | aboutEach | aboutEachPrefix
 *
 * Later: bagID removed
*/


static const struct { 
  const char * const name;            /* term name */
  int forbidden_as_nodeElement;
  int forbidden_as_propertyElement;
  int forbidden_as_propertyAttribute;
} rdf_syntax_terms_info[]={
  { "RDF",             1, 1, 1 },
  { "ID",              1, 1, 1 },
  { "about",           1, 1, 1 },
  { "bagID",           1, 1, 1 },
  { "parseType",       1, 1, 1 },
  { "resource",        1, 1, 1 },
  { "nodeID",          1, 1, 1 },
  { "datatype",        1, 1, 1 },
  { "Description",     0, 1, 1 },
  { "li",              1, 0, 0 },
  { "aboutEach",       1, 1, 1 },
  { "aboutEachPrefix", 1, 1, 1 },
  /* Other names are allowed anywhere */
  { NULL,              0, 0, 0 },
};


static int
raptor_forbidden_nodeElement_name(const char *name) 
{
  int i;
  for(i=0; rdf_syntax_terms_info[i].name; i++)
    if(!strcmp(rdf_syntax_terms_info[i].name, name))
      return rdf_syntax_terms_info[i].forbidden_as_nodeElement;
  return 0;
}


static int
raptor_forbidden_propertyElement_name(const char *name) 
{
  int i;
  for(i=0; rdf_syntax_terms_info[i].name; i++)
    if(!strcmp(rdf_syntax_terms_info[i].name, name))
      return rdf_syntax_terms_info[i].forbidden_as_propertyElement;
  return 0;
}

#if 0
/* This is probably not needed; covered by rdf_attr_info */
static int
raptor_forbidden_propertyAttribute_name(const char *name) 
{
  int i;
  for(i=0; rdf_syntax_terms_info[i].name; i++)
    if(!strcmp(rdf_syntax_terms_info[i].name, name))
      return rdf_syntax_terms_info[i].forbidden_as_propertyAttribute;
  return 0;
}
#endif


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

  /* property content - all content preserved
   * any content type changes when first non-whitespace found
   * <propElement>...
   */
  RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTY_CONTENT,

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

  /* parseType Collection - all content preserved
   * Parsing of this determined by RDF/XML (Revised) closed collection rules
   * <propElement rdf:parseType="Collection">...</propElement>
   */
  RAPTOR_ELEMENT_CONTENT_TYPE_COLLECTION,

  /* Like above but handles "daml:collection" */
  RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION,

  /* dummy for use in strings below */
  RAPTOR_ELEMENT_CONTENT_TYPE_LAST

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
} rdf_content_type_info[RAPTOR_ELEMENT_CONTENT_TYPE_LAST]={
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
  raptor_qname *name;
  raptor_qname **attributes;
  int attribute_count;
  /* attributes declared in M&S */
  const unsigned char * rdf_attr[RDF_ATTR_LAST+1];
  /* how many of above seen */
  int rdf_attr_count;

  /* value of xml:lang attribute on this element or NULL */
  const unsigned char *xml_language;

  /* URI of xml:base attribute value on this element or NULL */
  raptor_uri *base_uri;


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

  /* STATIC Reified statement identifier */
  raptor_identifier reified;

  /* STATIC Bag identifier */
  raptor_identifier bag;
  int last_bag_ordinal; /* starts at 0, so first predicate is rdf:_1 */

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

  /* URI of datatype of literal */
  raptor_uri *object_literal_datatype;

  /* last ordinal used, so initialising to 0 works, emitting rdf:_1 first */
  int last_ordinal;

  /* If this element's parseType is a Collection 
   * this identifies the anon node of current tail of the collection(list). 
   */
  const unsigned char *tail_id;
};

typedef struct raptor_element_s raptor_element;


#define RAPTOR_N_CONCEPTS 21

/*
 * Raptor parser object
 */
struct raptor_xml_parser_s {
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

#endif  

  /* element depth */
  int depth;

  /* stack of elements - elements add after current_element */
  raptor_element *root_element;
  raptor_element *current_element;

#ifdef RAPTOR_XML_LIBXML
  /* flag for some libxml eversions*/
  int first_read;
#endif

  raptor_uri* concepts[RAPTOR_N_CONCEPTS];

  /* set of seen rdf:ID / rdf:bagID values (with in-scope base URI) */
  raptor_set* id_set;
};




/* static variables */

#define RAPTOR_RDF_type_URI(rdf_xml_parser)      rdf_xml_parser->concepts[0]
#define RAPTOR_RDF_value_URI(rdf_xml_parser)     rdf_xml_parser->concepts[1]
#define RAPTOR_RDF_subject_URI(rdf_xml_parser)   rdf_xml_parser->concepts[2]
#define RAPTOR_RDF_predicate_URI(rdf_xml_parser) rdf_xml_parser->concepts[3]
#define RAPTOR_RDF_object_URI(rdf_xml_parser)    rdf_xml_parser->concepts[4]
#define RAPTOR_RDF_Statement_URI(rdf_xml_parser) rdf_xml_parser->concepts[5]

#define RAPTOR_RDF_Seq_URI(rdf_xml_parser) rdf_xml_parser->concepts[6]
#define RAPTOR_RDF_Bag_URI(rdf_xml_parser) rdf_xml_parser->concepts[7]
#define RAPTOR_RDF_Alt_URI(rdf_xml_parser) rdf_xml_parser->concepts[8]

#define RAPTOR_RDF_List_URI(rdf_xml_parser)  rdf_xml_parser->concepts[9]
#define RAPTOR_RDF_first_URI(rdf_xml_parser) rdf_xml_parser->concepts[10]
#define RAPTOR_RDF_rest_URI(rdf_xml_parser)  rdf_xml_parser->concepts[11]
#define RAPTOR_RDF_nil_URI(rdf_xml_parser)   rdf_xml_parser->concepts[12]

#define RAPTOR_DAML_NS_URI(rdf_xml_parser)   rdf_xml_parser->concepts[13]

#define RAPTOR_DAML_List_URI(rdf_xml_parser)  rdf_xml_parser->concepts[14]
#define RAPTOR_DAML_first_URI(rdf_xml_parser) rdf_xml_parser->concepts[15]
#define RAPTOR_DAML_rest_URI(rdf_xml_parser)  rdf_xml_parser->concepts[16]
#define RAPTOR_DAML_nil_URI(rdf_xml_parser)   rdf_xml_parser->concepts[17]

#define RAPTOR_RDF_RDF_URI(rdf_xml_parser)         rdf_xml_parser->concepts[18]
#define RAPTOR_RDF_Description_URI(rdf_xml_parser) rdf_xml_parser->concepts[19]
#define RAPTOR_RDF_li_URI(rdf_xml_parser)          rdf_xml_parser->concepts[20]

/* RAPTOR_N_CONCEPTS defines size of array */


/* prototypes for element functions */
static raptor_element* raptor_element_pop(raptor_xml_parser *rdf_parser);
static void raptor_element_push(raptor_xml_parser *rdf_parser, raptor_element* element);
static void raptor_free_element(raptor_element *element);
#ifdef RAPTOR_DEBUG
static void raptor_print_element(raptor_element *element, FILE* stream);
#endif

static int raptor_record_ID(raptor_parser *rdf_parser, raptor_element *element, const unsigned char *id);

/* prototypes for grammar functions */
static void raptor_start_element_grammar(raptor_parser *parser, raptor_element *element);
static void raptor_end_element_grammar(raptor_parser *parser, raptor_element *element);


/* prototype for statement related functions */
static void raptor_generate_statement(raptor_parser *rdf_parser, raptor_uri *subject_uri, const unsigned char *subject_id, const raptor_identifier_type subject_type, const raptor_uri_source subject_uri_source, raptor_uri *predicate_uri, const unsigned char *predicate_id, const raptor_identifier_type predicate_type, const raptor_uri_source predicate_uri_source, raptor_uri *object_uri, const unsigned char *object_id, const raptor_identifier_type object_type, const raptor_uri_source object_uri_source, raptor_uri *literal_datatype, raptor_identifier *reified, raptor_element *bag_element);



/* Prototypes for parsing data functions */
static int raptor_xml_parse_init(raptor_parser* rdf_parser, const char *name);
static void raptor_xml_parse_terminate(raptor_parser *rdf_parser);
static int raptor_xml_parse_start(raptor_parser* rdf_parser);
static int raptor_xml_parse_chunk(raptor_parser* rdf_parser, const unsigned char *buffer, size_t len, int is_end);





static raptor_element*
raptor_element_pop(raptor_xml_parser *rdf_parser) 
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
raptor_element_push(raptor_xml_parser *rdf_parser, raptor_element* element) 
{
  element->parent=rdf_parser->current_element;
  rdf_parser->current_element=element;
  if(!rdf_parser->root_element)
    rdf_parser->root_element=element;
}


static void
raptor_free_element(raptor_element *element)
{
  int i;

  for (i=0; i < element->attribute_count; i++)
    if(element->attributes[i])
      raptor_free_qname(element->attributes[i]);

  if(element->attributes)
    RAPTOR_FREE(raptor_qname_array, element->attributes);

  /* Free special RDF M&S attributes */
  for(i=0; i<= RDF_ATTR_LAST; i++) 
    if(element->rdf_attr[i])
      RAPTOR_FREE(cstring, (void*)element->rdf_attr[i]);

  if(element->content_cdata_length)
    RAPTOR_FREE(raptor_qname_array, element->content_cdata);

  raptor_free_identifier(&element->subject);
  raptor_free_identifier(&element->predicate);
  raptor_free_identifier(&element->object);
  raptor_free_identifier(&element->bag);
  raptor_free_identifier(&element->reified);

  if(element->tail_id)
    RAPTOR_FREE(cstring, (char*)element->tail_id);

  if(element->base_uri)
    raptor_free_uri(element->base_uri);

  if(element->xml_language)
    RAPTOR_FREE(cstring, element->xml_language);

  if(element->object_literal_datatype)
    raptor_free_uri(element->object_literal_datatype);

  raptor_free_qname(element->name);
  RAPTOR_FREE(raptor_element, element);
}



#ifdef RAPTOR_DEBUG
static void
raptor_print_element(raptor_element *element, FILE* stream)
{
  raptor_qname_print(stream, element->name);
  fputc('\n', stream);

  if(element->attribute_count) {
    int i;

    fputs(" attributes: ", stream);
    for (i = 0; i < element->attribute_count; i++) {
      if(i)
        fputc(' ', stream);
      raptor_qname_print(stream, element->attributes[i]);
      fprintf(stream, "='%s'", element->attributes[i]->value);
    }
    fputc('\n', stream);
  }
}
#endif


static char *
raptor_format_element(raptor_parser *rdf_parser,
                      raptor_element *element, int *length_p, int is_end)
{
  int length;
  char *buffer;
  char *ptr;
  int i;

  /* get length of element name (and namespace-prefix: if there is one) */
  length=element->name->local_name_length + 1; /* < */
  if(element->name->nspace &&
     element->name->nspace->prefix_length > 0)
    length += element->name->nspace->prefix_length + 1; /* : */

  if(is_end)
    length++; /* / */

  if (!is_end && element->attributes) {
    for(i=0; i < element->attribute_count; i++) {
      size_t escaped_attr_val_len;
      
      length++; /* ' ' between attributes and after element name */

      /* qname */
      length += element->attributes[i]->local_name_length;
      if(element->attributes[i]->nspace &&
         element->attributes[i]->nspace->prefix_length > 0)
         /* prefix: */
        length += element->attributes[i]->nspace->prefix_length + 1;

      /* XML escaped value */
      escaped_attr_val_len=raptor_xml_escape_string(rdf_parser,
                                                    element->attributes[i]->value, element->attributes[i]->value_length,
                                                  NULL, 0, '"');
      /* ="XML-escaped-value" */
      length += 3+escaped_attr_val_len;
    }
  }
  
  length++; /* > */
  
  if(length_p)
    *length_p=length;

  /* +1 here is for \0 at end */
  buffer=(char*)RAPTOR_MALLOC(cstring, length + 1);
  if(!buffer)
    return NULL;

  ptr=buffer;

  *ptr++ = '<';
  if(is_end)
    *ptr++ = '/';
  if(element->name->nspace && element->name->nspace->prefix_length > 0) {
    strncpy(ptr, (char*)element->name->nspace->prefix,
            element->name->nspace->prefix_length);
    ptr+= element->name->nspace->prefix_length;
    *ptr++=':';
  }
  strcpy(ptr, (char*)element->name->local_name);
  ptr += element->name->local_name_length;

  if(!is_end && element->attributes) {
    for(i=0; i < element->attribute_count; i++) {
      size_t escaped_attr_val_len;

      *ptr++ =' ';
      
      if(element->attributes[i]->nspace && 
         element->attributes[i]->nspace->prefix_length > 0) {
        strncpy(ptr, (char*)element->attributes[i]->nspace->prefix,
                element->attributes[i]->nspace->prefix_length);
        ptr+= element->attributes[i]->nspace->prefix_length;
        *ptr++=':';
      }
    
      strcpy(ptr, (char*)element->attributes[i]->local_name);
      ptr += element->attributes[i]->local_name_length;
      
      *ptr++ ='=';
      *ptr++ ='"';
      
      escaped_attr_val_len=raptor_xml_escape_string(rdf_parser,
                                                    element->attributes[i]->value, element->attributes[i]->value_length,
                                                    NULL, 0, '"');
      if(escaped_attr_val_len == element->attributes[i]->value_length) {
        /* save a malloc/free when there is no escaping */
        strcpy(ptr, element->attributes[i]->value);
        ptr += element->attributes[i]->value_length;
      } else {
        unsigned char *escaped_attr_val=(char*)RAPTOR_MALLOC(cstring,escaped_attr_val_len+1);
        raptor_xml_escape_string(rdf_parser,
                                 element->attributes[i]->value, element->attributes[i]->value_length,
                                 escaped_attr_val, escaped_attr_val_len, '"');
        
        strcpy(ptr, escaped_attr_val);
        RAPTOR_FREE(cstring,escaped_attr_val);
        ptr += escaped_attr_val_len;
      }
      *ptr++ ='"';
    }
  }
  
  *ptr++ = '>';
  *ptr='\0';

  return buffer;
}



void
raptor_xml_start_element_handler(void *user_data,
                                 const unsigned char *name, 
                                 const unsigned char **atts)
{
  raptor_parser* rdf_parser;
  raptor_xml_parser* rdf_xml_parser;
  unsigned char **xml_atts_copy=NULL;
  size_t xml_atts_size;
  int all_atts_count=0;
  int ns_attributes_count=0;
  raptor_qname** named_attrs=NULL;
  int i;
  raptor_element* element=NULL;
  int non_nspaced_count=0;
  unsigned char *xml_language=NULL;
  raptor_uri *xml_base=NULL;
  int count_bumped=0;
  
  rdf_parser=(raptor_parser*)user_data;
  rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;

#ifdef RAPTOR_XML_EXPAT
#ifdef EXPAT_UTF8_BOM_CRASH
  rdf_xml_parser->tokens_count++;
#endif
#endif

  if(rdf_parser->failed)
    return;

  raptor_update_document_locator(rdf_parser);

  rdf_xml_parser->depth++;

  if(atts) {
#ifdef RAPTOR_XML_LIBXML
    /* Round 0 - do XML attribute value normalization */
    for (i = 0; atts[i]; i+=2) {
      unsigned char *value=(unsigned char*)atts[i+1];
      unsigned char *src = value;
      unsigned char *dst = xmlStrdup(value);

      if (!dst) {
        raptor_parser_fatal_error(rdf_parser, "Out of memory");
	return;
      }

      atts[i+1]=dst;

      while (*src == 0x20 || *src == 0x0d || *src == 0x0a || *src == 0x09) 
        src++;
      while (*src) {
	if (*src == 0x20 || *src == 0x0d || *src == 0x0a || *src == 0x09) {
          while (*src == 0x20 || *src == 0x0d || *src == 0x0a || *src == 0x09)
            src++;
          if (*src)
            *dst++ = 0x20;
	} else {
          *dst++ = *src++;
	}
      }
      *dst = '\0';
      xmlFree(value);
    }
#endif

    /* Save passed in XML attributes pointers so we can 
     * NULL the pointers when they get handled below (various atts[i]=NULL)
     */
    for (i = 0; atts[i]; i++);
    xml_atts_size=sizeof(unsigned char*) * i;
    if(xml_atts_size) {
      xml_atts_copy=(unsigned char**)RAPTOR_MALLOC(cstringpointer,xml_atts_size);
      memcpy(xml_atts_copy, atts, xml_atts_size);
    }

    /* Round 1 - process XML attributes */
    for (i = 0; atts[i]; i+=2) {
      all_atts_count++;

      /* synthesise the XML namespace events */
      if(!memcmp((const char*)atts[i], "xmlns", 5)) {
        /* there is more i.e. xmlns:foo */
        const unsigned char *prefix=atts[i][5] ? &atts[i][6] : NULL;

        raptor_namespaces_start_namespace(&rdf_parser->namespaces,
                                          prefix, atts[i+1],
                                          rdf_xml_parser->depth,
                                          raptor_parser_error, rdf_parser);
        
        atts[i]=NULL; 
        continue;
      }

      if(!strcmp((char*)atts[i], "xml:lang")) {
        xml_language=(unsigned char*)RAPTOR_MALLOC(cstring, strlen((char*)atts[i+1])+1);
        if(!xml_language) {
          raptor_parser_fatal_error(rdf_parser, "Out of memory");
          return;
        }
        strcpy((char*)xml_language, (char*)atts[i+1]);
        atts[i]=NULL; 
        continue;
      }
      
      if(!strcmp((char*)atts[i], "xml:base")) {
        raptor_uri* xuri=raptor_new_uri_relative_to_base(raptor_inscope_base_uri(rdf_parser), (char*)atts[i+1]);

        xml_base=raptor_new_uri_for_xmlbase(xuri);
        raptor_free_uri(xuri);
        atts[i]=NULL; 
        continue;
      }

      /* delete other xml attributes - not used */
      if(!strncmp((char*)atts[i], "xml", 3)) {
        atts[i]=NULL; 
        continue;
      }
      

      ns_attributes_count++;
    }
  }


  /* Create new element structure */
  element=(raptor_element*)RAPTOR_CALLOC(raptor_element, 
                                         sizeof(raptor_element), 1);
  if(!element) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  } 


  /* Prepare for possible element content */
  element->content_element_seen=0;
  element->content_cdata_seen=0;
  element->content_cdata_length=0;

  element->xml_language=xml_language;
  element->base_uri=xml_base;


  /* Now can recode element name with a namespace */
  element->name=raptor_new_qname(&rdf_parser->namespaces, name, NULL,
                                 raptor_parser_error, rdf_parser);
  if(!element->name) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    RAPTOR_FREE(raptor_element, element);
    return;
  } 

  if(!element->name->nspace)
    non_nspaced_count++;


  if(ns_attributes_count) {
    int offset = 0;

    /* Round 2 - turn string attributes into namespaced-attributes */

    /* Allocate new array to hold namespaced-attributes */
    named_attrs=(raptor_qname**)RAPTOR_CALLOC(raptor_qname-array, sizeof(raptor_qname*), ns_attributes_count);
    if(!named_attrs) {
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      RAPTOR_FREE(raptor_element, element);
      raptor_free_qname(element->name);
      return;
    }

    for (i = 0; i < all_atts_count; i++) {
      raptor_qname* attr;

      /* Skip previously processed attributes */
      if(!atts[i<<1])
        continue;

      /* namespace-name[i] stored in named_attrs[i] */
      attr=raptor_new_qname(&rdf_parser->namespaces,
                            atts[i<<1], atts[(i<<1)+1],
                            raptor_parser_error, rdf_parser);
      if(!attr) { /* failed - tidy up and return */
        int j;

        for (j=0; j < i; j++)
          RAPTOR_FREE(raptor_qname, named_attrs[j]);
        RAPTOR_FREE(raptor_qname_array, named_attrs);
        raptor_free_qname(element->name);
        RAPTOR_FREE(raptor_element, element);
        return;
      }

      
      /* If:
       *  1 We are handling RDF content and RDF processing is allowed on
       *    this element
       * OR
       *  2 We are not handling RDF content and 
       *    this element is at the top level (top level Desc. / typedNode)
       *    FIXME  - is this correct!
       * then handle the RDF attributes
       */
      if ((rdf_xml_parser->current_element &&
           rdf_content_type_info[rdf_xml_parser->current_element->child_content_type].rdf_processing) ||
          !rdf_xml_parser->current_element) {

        /* Save pointers to some RDF M&S attributes */

        /* If RDF namespace-prefixed attributes */
        if(attr->nspace && attr->nspace->is_rdf_ms) {
          const unsigned char *attr_name=attr->local_name;
          int j;

          for(j=0; j<= RDF_ATTR_LAST; j++)
            if(!strcmp((char*)attr_name, rdf_attr_info[j].name)) {
              element->rdf_attr[j]=attr->value;
              element->rdf_attr_count++;
              /* Delete it if it was stored elsewhere */
#if RAPTOR_DEBUG
              RAPTOR_DEBUG3(raptor_xml_start_element_handler,
                            "Found RDF namespace attribute %s URI %s\n",
                            (char*)attr_name, attr->value);
#endif
              /* make sure value isn't deleted from qname structure */
              attr->value=NULL;
              raptor_free_qname(attr);
              attr=NULL;
              break;
            }
        } /* end if RDF namespaced-prefixed attributes */

        if(!attr)
          continue;

        /* If non namespace-prefixed RDF attributes found on an element */
        if(rdf_parser->feature_allow_non_ns_attributes &&
           !attr->nspace) {
          const unsigned char *attr_name=attr->local_name;
          int j;

          for(j=0; j<= RDF_ATTR_LAST; j++)
            if(!strcmp((char*)attr_name, rdf_attr_info[j].name)) {
              element->rdf_attr[j]=attr->value;
              element->rdf_attr_count++;
              if(!rdf_attr_info[i].allowed_unprefixed_on_attribute)
                raptor_parser_warning(rdf_parser, "Unqualified use of rdf:%s has been deprecated.", attr_name);
              /* Delete it if it was stored elsewhere */
              /* make sure value isn't deleted from qname structure */
              attr->value=NULL;
              raptor_free_qname(attr);
              attr=NULL;
              break;
            }
        } /* end if non-namespace prefixed RDF attributes */

        if(!attr)
          continue;

        if(!attr->nspace)
          non_nspaced_count++;

      } /* end if leave literal XML alone */

      if(attr)
        named_attrs[offset++]=attr;
    }

    /* set actual count from attributes that haven't been skipped */
    ns_attributes_count=offset;
    if(!offset && named_attrs) {
      /* all attributes were RDF namespace or other specials and deleted
       * so delete array and don't store pointer */
      RAPTOR_FREE(raptor_qname_array, named_attrs);
      named_attrs=NULL;
    }

  } /* end if ns_attributes_count */

  element->attributes=named_attrs;
  element->attribute_count=ns_attributes_count;


  raptor_element_push(rdf_xml_parser, element);


  /* start from unknown; if we have a parent, it may set this */
  element->state=RAPTOR_STATE_UNKNOWN;
  element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_UNKNOWN;

  if(!rdf_parser->feature_scanning_for_rdf_RDF && element->parent) {
    element->content_type=element->parent->child_content_type;
      
    if(element->parent->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE &&
       element->content_type != RAPTOR_ELEMENT_CONTENT_TYPE_COLLECTION &&
       element->content_type != RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) {
      /* If parent has an rdf:resource, this element should not be here */
      raptor_parser_error(rdf_parser, "property element %s has multiple object node elements, skipping.", 
                            element->parent->name->local_name);
      element->state=RAPTOR_STATE_SKIPPING;
      element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PRESERVED;

    } else {
      if(!element->parent->child_state)
        raptor_parser_fatal_error(rdf_parser, "raptor_xml_start_element_handler - no parent element child_state set");      

      element->state=element->parent->child_state;
      element->parent->content_element_seen++;
      count_bumped++;
    
      /* leave literal XML alone */
      if (!rdf_content_type_info[element->content_type].cdata_allowed) {
        if(element->parent->content_element_seen == 1 &&
           element->parent->content_cdata_seen == 1) {
          /* Uh oh - mixed content, the parent element has cdata too */
          raptor_parser_warning(rdf_parser, "element %s has mixed content.", 
                                element->parent->name->local_name);
        }
        
        /* If there is some existing all-whitespace content cdata
         * before this node element, delete it
         */
        if(element->parent->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTIES &&
           element->parent->content_element_seen &&
           element->parent->content_cdata_all_whitespace &&
           element->parent->content_cdata) {
          
          element->parent->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
          
          RAPTOR_FREE(raptor_qname_array, element->parent->content_cdata);
          element->parent->content_cdata=NULL;
          element->parent->content_cdata_length=0;
        }
        
      } /* end if leave literal XML alone */
      
    } /* end if parent has no rdf:resource */

  } /* end if element->parent */


#ifdef RAPTOR_DEBUG
  RAPTOR_DEBUG2(raptor_xml_start_element_handler, "Using content type %s\n",
                rdf_content_type_info[element->content_type].name);

  fprintf(stderr, "raptor_xml_start_element_handler: Start ns-element: ");
  raptor_print_element(element, stderr);
#endif

  
  /* Check for non namespaced stuff when not in a
   * parseType literal, other
   */
  if (rdf_content_type_info[element->content_type].rdf_processing &&
      non_nspaced_count) {
    raptor_parser_warning(rdf_parser, "element %s has non-namespaced parts, skipping.", 
                          element->name->local_name);
    element->state=RAPTOR_STATE_SKIPPING;
    /* Remove count above so that parent thinks this is empty */
    if(count_bumped)
      element->parent->content_element_seen--;
    element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PRESERVED;
  }
  

  if (element->rdf_attr[RDF_ATTR_aboutEach] || 
      element->rdf_attr[RDF_ATTR_aboutEachPrefix]) {
    raptor_parser_warning(rdf_parser, "element %s has aboutEach / aboutEachPrefix, skipping.", 
                          element->name->local_name);
    element->state=RAPTOR_STATE_SKIPPING;
    /* Remove count above so that parent thinks this is empty */
    if(count_bumped)
      element->parent->content_element_seen--;
    element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PRESERVED;
  }
  
  /* Right, now ready to enter the grammar */
  raptor_start_element_grammar(rdf_parser, element);

  if(xml_atts_copy) {
    /* Restore passed in XML attributes, free the copy */
    memcpy(atts, xml_atts_copy, xml_atts_size);
    RAPTOR_FREE(cstringpointer, xml_atts_copy);
  }
    
}


void
raptor_xml_end_element_handler(void *user_data, const unsigned char *name)
{
  raptor_parser* rdf_parser;
  raptor_xml_parser* rdf_xml_parser;
  raptor_element* element;
  raptor_qname *element_name;

  rdf_parser=(raptor_parser*)user_data;
  rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;

  if(rdf_parser->failed)
    return;

#ifdef RAPTOR_XML_EXPAT
#ifdef EXPAT_UTF8_BOM_CRASH
  rdf_xml_parser->tokens_count++;
#endif
#endif

  raptor_update_document_locator(rdf_parser);

  /* recode element name */

  element_name=raptor_new_qname(&rdf_parser->namespaces, name, NULL,
                                raptor_parser_error, rdf_parser);
  if(!element_name) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  }


#ifdef RAPTOR_DEBUG
  fprintf(stderr, "\nraptor_xml_end_element_handler: End ns-element: ");
  raptor_qname_print(stderr, element_name);
  fputc('\n', stderr);
#endif

  element=rdf_xml_parser->current_element;

  raptor_end_element_grammar(rdf_parser, element);

  element=raptor_element_pop(rdf_xml_parser);

  raptor_free_qname(element_name);

  raptor_namespaces_end_for_depth(&rdf_parser->namespaces, rdf_xml_parser->depth);

  if(element->parent) {
    /* Do not change this; PROPERTYELT will turn into MEMBER if necessary
     * See the switch case for MEMBER / PROPERTYELT where the test is done.
     *
     * PARSETYPE_RESOURCE should never be propogated up since it
     * will turn the next child (node) element into a property
     */
    if(element->state != RAPTOR_STATE_MEMBER_PROPERTYELT &&
       element->state != RAPTOR_STATE_PARSETYPE_RESOURCE)
      element->parent->child_state=element->state;
  }
  

  raptor_free_element(element);

  rdf_xml_parser->depth--;

}



/* cdata (and ignorable whitespace for libxml). 
 * s is not 0 terminated for expat, is for libxml - grrrr.
 */
void
raptor_xml_cdata_handler(void *user_data, const unsigned char *s, int len)
{
  raptor_parser* rdf_parser;
  raptor_xml_parser* rdf_xml_parser;
  raptor_element* element;
  raptor_state state;
  char *buffer;
  char *ptr;
  char *escaped_buffer=NULL;
  int all_whitespace=1;
  int i;

  rdf_parser=(raptor_parser*)user_data;
  rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;

  if(rdf_parser->failed)
    return;

#ifdef RAPTOR_XML_EXPAT
#ifdef EXPAT_UTF8_BOM_CRASH
  rdf_xml_parser->tokens_count++;
#endif
#endif

  for(i=0; i<len; i++)
    if(!isspace(s[i])) {
      all_whitespace=0;
      break;
    }

  element=rdf_xml_parser->current_element;

  /* this file is very broke - probably not XML, whatever */
  if(!element)
    return;
  
  raptor_update_document_locator(rdf_parser);

  /* cdata never changes the parser state 
   * and the containing element state always determines what to do.
   * Use the child_state first if there is one, since that applies
   */
  state=element->child_state;
  RAPTOR_DEBUG2(raptor_xml_cdata_handler, "Working in state %s\n",
                raptor_state_as_string(state));


  RAPTOR_DEBUG3(raptor_xml_cdata_handler,
                "Content type %s (%d)\n", raptor_element_content_type_as_string(element->content_type), element->content_type);
  


  if(state == RAPTOR_STATE_SKIPPING)
    return;

  if(state == RAPTOR_STATE_UNKNOWN) {
    /* Ignore all cdata if still looking for RDF */
    if(rdf_parser->feature_scanning_for_rdf_RDF)
      return;

    /* Ignore all whitespace cdata before first element */
    if(all_whitespace)
      return;
    
    /* This probably will never happen since that would make the
     * XML not be well-formed
     */
    raptor_parser_warning(rdf_parser, "Character data before RDF element.");
  }


  if(element->child_content_type == RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTIES) {
    /* If found non-whitespace content, move to literal content */
    if(!all_whitespace)
      element->child_content_type = RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL; 
  }


  if(!rdf_content_type_info[element->child_content_type].whitespace_significant) {

    /* Whitespace is ignored except for literal or preserved content types */
    if(all_whitespace) {
#ifdef RAPTOR_DEBUG_CDATA
      RAPTOR_DEBUG2(raptor_xml_cdata_handler, "Ignoring whitespace cdata inside element %s\n", element->name->local_name);
#endif
      return;
    }

    if(++element->content_cdata_seen == 1 &&
       element->content_element_seen == 1) {
      /* Uh oh - mixed content, this element has elements too */
      raptor_parser_warning(rdf_parser, "element %s has mixed content.", 
                            element->name->local_name);
    }
  }


  if(element->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTY_CONTENT) {
    element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL;
    RAPTOR_DEBUG3(raptor_xml_cdata_handler,
                  "Content type changed to %s (%d)\n", raptor_element_content_type_as_string(element->content_type), element->content_type);
  }

  if(element->child_content_type == RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL) {
    size_t escaped_buffer_len=raptor_xml_escape_string(rdf_parser,
                                                s, len,
                                                NULL, 0, '\0');
    /* save a malloc/free when there is no escaping */
    if(escaped_buffer_len != len) {
      escaped_buffer=(char*)RAPTOR_MALLOC(cstring, escaped_buffer_len+1);
      if(!escaped_buffer) {
        raptor_parser_fatal_error(rdf_parser, "Out of memory");
        return;
      }
      raptor_xml_escape_string(rdf_parser,
                               s, len,
                               escaped_buffer, escaped_buffer_len, '\0');
      len=escaped_buffer_len;
    }
  }

  buffer=(char*)RAPTOR_MALLOC(cstring, element->content_cdata_length + len + 1);
  if(!buffer) {
    raptor_parser_fatal_error(rdf_parser, "Out of memory");
    return;
  }

  if(element->content_cdata_length) {
    strncpy(buffer, element->content_cdata, element->content_cdata_length);
    RAPTOR_FREE(cstring, element->content_cdata);
    element->content_cdata_all_whitespace &= all_whitespace;
  } else
    element->content_cdata_all_whitespace = all_whitespace;

  element->content_cdata=buffer;

  /* move pointer to end of cdata buffer */
  ptr=buffer+element->content_cdata_length;

  /* adjust stored length */
  element->content_cdata_length += len;

  /* now write new stuff at end of cdata buffer */
  if((element->child_content_type == RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL)
     && escaped_buffer) {
    strncpy(ptr, escaped_buffer, len);
    RAPTOR_FREE(cstring, escaped_buffer);
  } else
    strncpy(ptr, (char*)s, len);
  ptr += len;
  *ptr = '\0';

#ifdef RAPTOR_DEBUG_CDATA
  RAPTOR_DEBUG3(raptor_xml_cdata_handler, 
                "Content cdata now: '%s' (%d bytes)\n", 
                buffer, element->content_cdata_length);
#endif
  RAPTOR_DEBUG2(raptor_xml_cdata_handler, 
                "Ending in state %s\n", raptor_state_as_string(state));

}


#ifdef RAPTOR_XML_EXPAT
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
/*  raptor_xml_parser* rdf_parser=(raptor_xml_parser*)user_data; */
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


/* comment handler
 * s is 0 terminated
 */
void
raptor_xml_comment_handler(void *user_data, const unsigned char *s)
{
  raptor_parser* rdf_parser=(raptor_parser*)user_data;

  if(rdf_parser->failed)
    return;

  /* nop */
  RAPTOR_DEBUG2(raptor_xml_comment_handler, "XML Comment '%s'\n", s);
}



static int
raptor_xml_parse_init(raptor_parser* rdf_parser, const char *name)
{
#ifdef RAPTOR_XML_EXPAT
  XML_Parser xp;
#endif
  raptor_xml_parser* rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;

  RAPTOR_RDF_type_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("type");
  RAPTOR_RDF_value_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("value");
  RAPTOR_RDF_subject_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("subject");
  RAPTOR_RDF_predicate_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("predicate");
  RAPTOR_RDF_object_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("object");
  RAPTOR_RDF_Statement_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("Statement");

  RAPTOR_RDF_Seq_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("Seq");
  RAPTOR_RDF_Bag_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("Bag");
  RAPTOR_RDF_Alt_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("Alt");

  RAPTOR_RDF_List_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("List");
  RAPTOR_RDF_first_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("first");
  RAPTOR_RDF_rest_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("rest");
  RAPTOR_RDF_nil_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("nil");

  RAPTOR_DAML_NS_URI(rdf_xml_parser)=raptor_new_uri("http://www.daml.org/2001/03/daml+oil#");

  RAPTOR_DAML_List_URI(rdf_xml_parser)=raptor_new_uri_from_uri_local_name(RAPTOR_DAML_NS_URI(rdf_xml_parser), "List");
  RAPTOR_DAML_first_URI(rdf_xml_parser)=raptor_new_uri_from_uri_local_name(RAPTOR_DAML_NS_URI(rdf_xml_parser) ,"first");
  RAPTOR_DAML_rest_URI(rdf_xml_parser)=raptor_new_uri_from_uri_local_name(RAPTOR_DAML_NS_URI(rdf_xml_parser), "rest");
  RAPTOR_DAML_nil_URI(rdf_xml_parser)=raptor_new_uri_from_uri_local_name(RAPTOR_DAML_NS_URI(rdf_xml_parser), "nil");

  RAPTOR_RDF_RDF_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("RDF");
  RAPTOR_RDF_Description_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("Description");
  RAPTOR_RDF_li_URI(rdf_xml_parser)=raptor_new_uri_for_rdf_concept("li");

#ifdef RAPTOR_XML_EXPAT
  xp=XML_ParserCreate(NULL);

  /* create a new parser in the specified encoding */
  XML_SetUserData(xp, rdf_parser);

  /* XML_SetEncoding(xp, "..."); */

  XML_SetElementHandler(xp, 
                        (XML_StartElementHandler)raptor_xml_start_element_handler,
                        (XML_EndElementHandler)raptor_xml_end_element_handler);
  XML_SetCharacterDataHandler(xp, 
                              (XML_CharacterDataHandler)raptor_xml_cdata_handler);

  XML_SetCommentHandler(xp,
                        (XML_CommentHandler)raptor_xml_comment_handler);


  XML_SetUnparsedEntityDeclHandler(xp, raptor_xml_unparsed_entity_decl_handler);

  XML_SetExternalEntityRefHandler(xp, (XML_ExternalEntityRefHandler)raptor_xml_external_entity_ref_handler);

  rdf_xml_parser->xp=xp;
#endif

  rdf_xml_parser->id_set=raptor_new_set();

  return 0;
}


static int
raptor_xml_parse_start(raptor_parser* rdf_parser)
{
  raptor_uri *uri=rdf_parser->base_uri;
#ifdef RAPTOR_XML_EXPAT
  XML_Parser xp;
#endif
  raptor_xml_parser* rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;

  /* base URI required for RDF/XML */
  if(!uri)
    return 1;

  /* initialise fields */
  rdf_xml_parser->depth=0;
  rdf_xml_parser->root_element= rdf_xml_parser->current_element=NULL;

#ifdef RAPTOR_XML_EXPAT
  xp=rdf_xml_parser->xp;

  XML_SetBase(xp, raptor_uri_as_string(uri));
#endif

#ifdef RAPTOR_XML_LIBXML
  raptor_libxml_init(&rdf_xml_parser->sax);

  rdf_xml_parser->first_read=1;
#endif

  return 0;
}


static void
raptor_xml_parse_terminate(raptor_parser *rdf_parser) 
{
  raptor_xml_parser* rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;
  raptor_element* element;
  int i;
  
#ifdef RAPTOR_XML_EXPAT
  if(rdf_xml_parser->xp) {
    XML_ParserFree(rdf_xml_parser->xp);
    rdf_xml_parser->xp=NULL;
  }
#endif

#ifdef RAPTOR_XML_LIBXML
  if(rdf_xml_parser->xc) {
    xmlFreeParserCtxt(rdf_xml_parser->xc);
    rdf_xml_parser->xc=NULL;
  }
#endif

  if(rdf_parser->fh) {
    fclose(rdf_parser->fh);
    rdf_parser->fh=NULL;
  }
  while((element=raptor_element_pop(rdf_xml_parser))) {
    raptor_free_element(element);
  }


  for(i=0; i< RAPTOR_N_CONCEPTS; i++) {
    raptor_uri* concept_uri=rdf_xml_parser->concepts[i];
    if(concept_uri) {
      raptor_free_uri(concept_uri);
      rdf_xml_parser->concepts[i]=NULL;
    }
  }
  

#ifdef RAPTOR_XML_LIBXML
#ifdef RAPTOR_LIBXML_MY_ENTITIES
  raptor_xml_libxml_free_entities(rdf_parser);
#endif
#endif

  raptor_free_set(rdf_xml_parser->id_set);
}


/* Internal - handle XML parser errors and pass them on to app */
static void
raptor_xml_parse_handle_errors(raptor_parser* rdf_parser) {
#ifdef RAPTOR_XML_EXPAT
  XML_Parser xp=((raptor_xml_parser*)rdf_parser->context)->xp;
  int xe=XML_GetErrorCode(xp);
  raptor_locator *locator=&rdf_parser->locator;

#ifdef EXPAT_UTF8_BOM_CRASH
  raptor_xml_parser* rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;
  if(rdf_xml_parser->tokens_count) {
#endif
    /* Work around a bug with the expat 1.95.1 shipped with RedHat 7.2
     * which dies here if the error is before <?xml?...
     * The expat 1.95.1 source release version works fine.
     */
    locator->line=XML_GetCurrentLineNumber(xp);
    locator->column=XML_GetCurrentColumnNumber(xp);
    locator->byte=XML_GetCurrentByteIndex(xp);
#ifdef EXPAT_UTF8_BOM_CRASH
  }
#endif
      
  raptor_update_document_locator(rdf_parser);
  raptor_parser_error(rdf_parser, "XML Parsing failed - %s",
                      XML_ErrorString(xe));
#endif /* EXPAT */

#ifdef RAPTOR_XML_LIBXML
  raptor_update_document_locator(rdf_parser);
  raptor_parser_error(rdf_parser, "XML Parsing failed");
#endif
}


/* Internal - no XML parser error handing */
static int
raptor_xml_parse_chunk_(raptor_parser* rdf_parser, const unsigned char *buffer,
                        size_t len, int is_end) 
{
  raptor_xml_parser* rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;
#ifdef RAPTOR_XML_EXPAT
  XML_Parser xp=rdf_xml_parser->xp;
#endif
#ifdef RAPTOR_XML_LIBXML
  /* parser context */
  xmlParserCtxtPtr xc=rdf_xml_parser->xc;
#endif
  int rc;
  
#ifdef RAPTOR_XML_LIBXML
  if(!xc) {
    if(!len) {
      /* no data given at all - emit a similar message to expat */
      raptor_update_document_locator(rdf_parser);
      raptor_parser_error(rdf_parser, "XML Parsing failed - %s",
                          "no element found");
      return 1;
    }

    xc = xmlCreatePushParserCtxt(&rdf_xml_parser->sax, rdf_parser,
                                 (char*)buffer, len, 
                                 NULL);
    if(!xc)
      return 1;

    xc->userData = rdf_parser;
    xc->vctxt.userData = rdf_parser;
    xc->vctxt.error=raptor_libxml_validation_error;
    xc->vctxt.warning=raptor_libxml_validation_warning;
    xc->replaceEntities = 1;
    
    rdf_xml_parser->xc = xc;

    if(is_end)
      len=0;
    else
      return 0;
  }
#endif

  if(!len) {
#ifdef RAPTOR_XML_EXPAT
    rc=XML_Parse(xp, (char*)buffer, 0, 1);
    if(!rc) /* expat: 0 is failure */
      return 1;
#endif
#ifdef RAPTOR_XML_LIBXML
    xmlParseChunk(xc, (char*)buffer, 0, 1);
#endif
    return 0;
  }


#ifdef RAPTOR_XML_EXPAT
  rc=XML_Parse(xp, (char*)buffer, len, is_end);
  if(!rc) /* expat: 0 is failure */
    return 1;
  if(is_end)
    return 0;
#endif

#ifdef RAPTOR_XML_LIBXML

  /* This works around some libxml versions that fail to work
   * if the buffer size is larger than the entire file
   * and thus the entire parsing is done in one operation.
   *
   * The code below:
   *   2.4.19 (oldest tested) to 2.4.24 - required
   *   2.4.25                           - works with or without it
   *   2.4.26 or later                  - fails with this code
   */

#if LIBXML_VERSION < 20425
  if(rdf_xml_parser->first_read && is_end) {
    /* parse all but the last character */
    rc=xmlParseChunk(xc, (char*)buffer, len-1, 0);
    if(rc)
      return 1;
    /* last character */
    rc=xmlParseChunk(xc, (char*)buffer + (len-1), 1, 0);
    if(rc)
      return 1;
    /* end */
    xmlParseChunk(xc, (char*)buffer, 0, 1);
    return 0;
  }
#endif

  rdf_xml_parser->first_read=0;
    
  rc=xmlParseChunk(xc, (char*)buffer, len, is_end);
  if(rc) /* libxml: non 0 is failure */
    return 1;
  if(is_end)
    return 0;
#endif

  return 0;
}



/**
 * raptor_xml_parse_chunk - Parse a chunk of memory
 * @rdf_parser: RDF Parser object
 * @buffer: memory to parse
 * @len: size of memory
 * @is_end: non-zero if this is the last chunk to parse
 * 
 * Parse the content in the given buffer, emitting RDF statements
 * as a side-effect, to the registered statement handler if they
 * are generated, and errors to the appropriate error handlers.
 * 
 * Return value: Non zero on failure.
 **/
static int
raptor_xml_parse_chunk(raptor_parser* rdf_parser, const unsigned char *buffer,
                       size_t len, int is_end) 
{
  int rc=0;
  if(rdf_parser->failed)
    return 1;

  rc=raptor_xml_parse_chunk_(rdf_parser, buffer, len, is_end);
  if(rc)    
    raptor_xml_parse_handle_errors(rdf_parser);
  return rc;
}


static void
raptor_generate_statement(raptor_parser *rdf_parser, 
                          raptor_uri *subject_uri,
                          const unsigned char *subject_id,
                          const raptor_identifier_type subject_type,
                          const raptor_uri_source subject_uri_source,
                          raptor_uri *predicate_uri,
                          const unsigned char *predicate_id,
                          const raptor_identifier_type predicate_type,
                          const raptor_uri_source predicate_uri_source,
                          raptor_uri *object_uri,
                          const unsigned char *object_id,
                          const raptor_identifier_type object_type,
                          const raptor_uri_source object_uri_source,
                          raptor_uri *literal_datatype,
                          raptor_identifier *reified,
                          raptor_element* bag_element)
{
  raptor_statement *statement=&rdf_parser->statement;
  const unsigned char *language=NULL;
  static const char empty_literal[1]="";
  raptor_xml_parser *rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;
  char *reified_id=NULL;

  if(rdf_parser->failed)
    return;

  if(object_type == RAPTOR_IDENTIFIER_TYPE_LITERAL ||
     object_type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL) {
    language=raptor_inscope_xml_language(rdf_parser);
    if(!object_uri)
      object_uri=(raptor_uri*)empty_literal;
  }
  
  statement->subject=subject_uri ? (void*)subject_uri : (void*)subject_id;
  statement->subject_type=subject_type;

  statement->predicate=predicate_uri ? (void*)predicate_uri : (void*)predicate_id;
  statement->predicate_type=predicate_type;

  statement->object=object_uri ? (void*)object_uri : (void*)object_id;
  statement->object_type=object_type;

  statement->object_literal_language=language;
  statement->object_literal_datatype=literal_datatype;


#ifdef RAPTOR_DEBUG
  fprintf(stderr, "raptor_generate_statement: Generating statement: ");
  raptor_print_statement_detailed(statement, 1, stderr);
  fputc('\n', stderr);

  if(!(subject_uri||subject_id))
    RAPTOR_FATAL1(raptor_generate_statement, "Statement has no subject\n");
  
  if(!(predicate_uri||predicate_id))
    RAPTOR_FATAL1(raptor_generate_statement, "Statement has no predicate\n");
  
  if(!(object_uri||object_id))
    RAPTOR_FATAL1(raptor_generate_statement, "Statement has no object\n");
  
#endif

  if(!rdf_parser->statement_handler)
    return;

  /* Generate the statement; or is it fact? */
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);


  /* the bagID mess */
  if(rdf_parser->feature_allow_bagID &&
     bag_element && (bag_element->bag.uri || bag_element->bag.id)) {
    raptor_identifier* bag=&bag_element->bag;
    
    statement->subject=bag->uri ? (void*)bag->uri : (void*)bag->id;
    statement->subject_type=bag->type;

    bag_element->last_bag_ordinal++;

    statement->predicate=(void*)&bag_element->last_bag_ordinal;
    statement->predicate_type=RAPTOR_IDENTIFIER_TYPE_ORDINAL;

    if(reified && (reified->uri || reified->id)) {
      statement->object=reified->uri ? (void*)reified->uri : (void*)reified->id;
      statement->object_type=reified->type;
    } else {
      statement->object=reified_id=(char*)raptor_generate_id(rdf_parser, 0);
      statement->object_type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
    }
    
    (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);
    
  } else if(!reified || (!reified->uri && !reified->id))
    return;

  /* generate reified statements */
  statement->subject_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
  statement->predicate_type=RAPTOR_IDENTIFIER_TYPE_PREDICATE;
  statement->object_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;

  statement->object_literal_language=NULL;

  if(reified_id) {
    statement->subject=reified_id;
    statement->subject_type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
  } else {
    statement->subject=reified->uri ? (void*)reified->uri : (void*)reified->id;
    statement->subject_type=reified->type;
  }
  

  statement->predicate=RAPTOR_RDF_type_URI(rdf_xml_parser);
  statement->object=RAPTOR_RDF_Statement_URI(rdf_xml_parser);
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);

  statement->predicate=RAPTOR_RDF_subject_URI(rdf_xml_parser);
  statement->object=subject_uri ? (void*)subject_uri : (void*)subject_id;
  statement->object_type=subject_type;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);

  statement->predicate=RAPTOR_RDF_predicate_URI(rdf_xml_parser);
  statement->object=predicate_uri ? (void*)predicate_uri : (void*)predicate_id;
  statement->object_type=(predicate_type == RAPTOR_IDENTIFIER_TYPE_PREDICATE) ? RAPTOR_IDENTIFIER_TYPE_RESOURCE : predicate_type;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);

  statement->predicate=RAPTOR_RDF_object_URI(rdf_xml_parser);
  statement->object=object_uri ? (void*)object_uri : (void*)object_id;
  statement->object_type=object_type;
  statement->object_literal_language=language;

  (*rdf_parser->statement_handler)(rdf_parser->user_data, statement);

  if(reified_id)
    RAPTOR_FREE(cstring, reified_id);
}



/**
 * raptor_element_has_property_attributes: Return true if the element has at least one property attribute
 * @element: element with the property attributes 
 * 
 **/
static int
raptor_element_has_property_attributes(raptor_element *element) 
{
  int i;
  
  if(element->attribute_count >0)
    return 1;

  /* look for rdf: properties */
  for(i=0; i<= RDF_ATTR_LAST; i++) {
    if(element->rdf_attr[i] &&
       rdf_attr_info[i].type != RAPTOR_IDENTIFIER_TYPE_UNKNOWN)
      return 1;
  }
  return 0;
}


/**
 * raptor_process_property_attributes: Process the property attributes for an element for a given resource
 * @rdf_parser: Raptor parser object
 * @attributes_element: element with the property attributes 
 * @resource_element: element that defines the resource URI 
 *                    subject_uri, subject_uri_source etc.
 * @property_node_identifier: Use this identifier for the resource URI
 *   and count any ordinals for it locally
 * 
 **/
static void 
raptor_process_property_attributes(raptor_parser *rdf_parser, 
                                   raptor_element *attributes_element,
                                   raptor_element *resource_element,
                                   raptor_identifier *property_node_identifier)
{
  int i;
  int local_last_ordinal=0;
  raptor_identifier *resource_identifier;
  raptor_xml_parser *rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;

  resource_identifier=property_node_identifier ? property_node_identifier : &resource_element->subject;
  

  /* Process attributes as propAttr* = * (propName="string")*
   */
  for(i=0; i<attributes_element->attribute_count; i++) {
    raptor_qname* attr=attributes_element->attributes[i];
    const unsigned char *name=attr->local_name;
    const unsigned char *value = attr->value;
    int handled=0;
    
    if(!attr->nspace) {
      raptor_update_document_locator(rdf_parser);
      raptor_parser_warning(rdf_parser, "Unqualified use of attribute %s is deprecated.", name);
      continue;
    }

    /* Generate the property statement using one of these properties:
     * 1) rdf:li -> rdf:_n
     * 2) rdf:_n
     * 3) the URI from the attribute
     */
    if(attr->nspace->is_rdf_ms) {
      /* is rdf: namespace */
      int ordinal=0;
        
      if(raptor_uri_equals(attr->uri, RAPTOR_RDF_li_URI(rdf_xml_parser))) {
        /* recognise rdf:li attribute */
        if(property_node_identifier)
          ordinal= ++local_last_ordinal;
        else
          ordinal= ++resource_element->last_ordinal;
      } else if(*name == '_') {
        /* recognise rdf:_ */
        name++;
        while(*name >= '0' && *name <= '9') {
          ordinal *= 10;
          ordinal += (*name++ - '0');
        }
        
        if(ordinal < 1) {
          raptor_update_document_locator(rdf_parser);
          raptor_parser_warning(rdf_parser, "Illegal ordinal value %d in attribute %s seen on container element %s.", ordinal, attr->local_name, name);
        }
      } else {
        raptor_update_document_locator(rdf_parser);
        raptor_parser_warning(rdf_parser, "Unknown RDF namespace attribute %s.", 
                              name);
      }

      if(ordinal >= 1) {
        /* Generate an ordinal property when there are no problems */
        raptor_generate_statement(rdf_parser, 
                                  resource_identifier->uri,
                                  resource_identifier->id,
                                  resource_identifier->type,
                                  resource_identifier->uri_source,
                                  
                                  (raptor_uri*)&ordinal,
                                  NULL,
                                  RAPTOR_IDENTIFIER_TYPE_ORDINAL,
                                  RAPTOR_URI_SOURCE_NOT_URI,
                                  
                                  (raptor_uri*)value,
                                  NULL,
                                  RAPTOR_IDENTIFIER_TYPE_LITERAL,
                                  RAPTOR_URI_SOURCE_NOT_URI,
                                  NULL,
                                  
                                  NULL, /* Property attributes are never reified*/
                                  resource_element);
        handled=1;
      }
      
    } /* end is RDF namespace property */


    if(!handled)
      /* else not rdf: namespace or unknown in rdf: namespace so
       * generate a statement with a literal object
       */
      raptor_generate_statement(rdf_parser, 
                                resource_identifier->uri,
                                resource_identifier->id,
                                resource_identifier->type,
                                resource_identifier->uri_source,
                                
                                attr->uri,
                                NULL,
                                RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                RAPTOR_URI_SOURCE_ATTRIBUTE,
                                
                                (raptor_uri*)value,
                                NULL,
                                RAPTOR_IDENTIFIER_TYPE_LITERAL,
                                RAPTOR_URI_SOURCE_NOT_URI,
                                NULL,
                                
                                NULL, /* Property attributes are never reified*/
                                resource_element);

  } /* end for ... attributes */


  /* Handle rdf property attributes
   * (only rdf:type and rdf:value at present) 
   */
  for(i=0; i<= RDF_ATTR_LAST; i++) {
    const unsigned char *value=attributes_element->rdf_attr[i];
    int object_is_literal=(rdf_attr_info[i].type == RAPTOR_IDENTIFIER_TYPE_LITERAL);
    raptor_uri *property_uri, *object_uri;
    raptor_identifier_type object_type;
    
    if(!value || rdf_attr_info[i].type == RAPTOR_IDENTIFIER_TYPE_UNKNOWN)
      continue;

    property_uri=raptor_new_uri_for_rdf_concept(rdf_attr_info[i].name);
    
    object_uri=object_is_literal ? (raptor_uri*)value : raptor_new_uri_relative_to_base(raptor_inscope_base_uri(rdf_parser), (char*)value);
    object_type=object_is_literal ? RAPTOR_IDENTIFIER_TYPE_LITERAL : RAPTOR_IDENTIFIER_TYPE_RESOURCE;
    
    raptor_generate_statement(rdf_parser, 
                              resource_identifier->uri,
                              resource_identifier->id,
                              resource_identifier->type,
                              resource_identifier->uri_source,
                              
                              property_uri,
                              NULL,
                              RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                              RAPTOR_URI_SOURCE_ATTRIBUTE,
                              
                              object_uri,
                              NULL,
                              object_type,
                              RAPTOR_URI_SOURCE_NOT_URI,
                              NULL,

                              NULL, /* Property attributes are never reified*/
                              resource_element);
    if(!object_is_literal)
      raptor_free_uri(object_uri);

    raptor_free_uri(property_uri);
    
  } /* end for rdf:property values */

}


static void
raptor_start_element_grammar(raptor_parser *rdf_parser,
                             raptor_element *element) 
{
  int finished;
  raptor_state state;
  const unsigned char *el_name=element->name->local_name;
  int element_in_rdf_ns=(element->name->nspace && 
                         element->name->nspace->is_rdf_ms);
  raptor_xml_parser *rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;


  state=element->state;
  RAPTOR_DEBUG2(raptor_start_element_grammar, "Starting in state %s\n",
                raptor_state_as_string(state));

  finished= 0;
  while(!finished) {
    switch(state) {
      case RAPTOR_STATE_SKIPPING:
        element->child_state=state;
        element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PRESERVED;
        finished=1;
        break;
        
      case RAPTOR_STATE_UNKNOWN:
        /* found <rdf:RDF> ? */

        if(element_in_rdf_ns) {
          if(raptor_uri_equals(element->name->uri, RAPTOR_RDF_RDF_URI(rdf_xml_parser))) {
            element->child_state=RAPTOR_STATE_NODE_ELEMENT_LIST;
            element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_NODES;
            /* Yes - need more content before can continue,
             * so wait for another element
             */
            finished=1;
            break;
          }
          if(raptor_uri_equals(element->name->uri, RAPTOR_RDF_Description_URI(rdf_xml_parser))) {
	    state=RAPTOR_STATE_DESCRIPTION;
	    element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTIES;
            /* Yes - found something so move immediately to description */
            break;
          }
          if(element_in_rdf_ns && raptor_forbidden_nodeElement_name((const char*)el_name)) {
            raptor_parser_error(rdf_parser, "rdf:%s is forbidden as a node element.", el_name);
            state=RAPTOR_STATE_SKIPPING;
            element->child_state=RAPTOR_STATE_SKIPPING;
            finished=1;
            break;
          }
        }

        /* If scanning for element, can continue */
        if(rdf_parser->feature_scanning_for_rdf_RDF) {
          finished=1;
          break;
        }

        /* If cannot assume document is rdf/xml, must have rdf:RDF at root */
        if(!rdf_parser->feature_assume_is_rdf) {
          raptor_parser_error(rdf_parser, "Document element rdf:RDF missing.");
          state=RAPTOR_STATE_SKIPPING;
          element->child_state=RAPTOR_STATE_SKIPPING;
          finished=1;
          break;
        }
        
        /* Otherwise the choice of the next state can be made
         * from the current element by the OBJ state
         */
        state=RAPTOR_STATE_NODE_ELEMENT_LIST;
        element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_NODES;
        break;


      case RAPTOR_STATE_NODE_ELEMENT_LIST:
        /* Handling
         *   http://www.w3.org/TR/rdf-syntax-grammar/#nodeElementList 
         *
         * Everything goes to nodeElement
         */

        state=RAPTOR_STATE_NODE_ELEMENT;

        element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTIES;

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

        if(element->content_type !=RAPTOR_ELEMENT_CONTENT_TYPE_COLLECTION &&
           element->content_type !=RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION && 
           element->parent && 
           (element->parent->state == RAPTOR_STATE_PROPERTYELT ||
            element->parent->state == RAPTOR_STATE_MEMBER_PROPERTYELT) &&
           element->parent->content_element_seen > 1) {
          raptor_update_document_locator(rdf_parser);
          raptor_parser_error(rdf_parser, "The enclosing property already has an object");
          state=RAPTOR_STATE_SKIPPING;
          element->child_state=RAPTOR_STATE_SKIPPING;
          finished=1;
          break;
        }

        if(state == RAPTOR_STATE_NODE_ELEMENT || 
           state == RAPTOR_STATE_DESCRIPTION || 
           state == RAPTOR_STATE_PARSETYPE_COLLECTION) {
          if(element_in_rdf_ns &&
             raptor_uri_equals(element->name->uri, RAPTOR_RDF_Description_URI(rdf_xml_parser)))
            state=RAPTOR_STATE_DESCRIPTION;
          else
            state=RAPTOR_STATE_NODE_ELEMENT;
        }
        

        if(element_in_rdf_ns && raptor_forbidden_nodeElement_name((const char*)el_name)) {
          raptor_parser_error(rdf_parser, "rdf:%s is forbidden as a node element.", el_name);
          state=RAPTOR_STATE_SKIPPING;
          element->child_state=RAPTOR_STATE_SKIPPING;
          finished=1;
          break;
        }

        if((element->rdf_attr[RDF_ATTR_ID]!=NULL) +
           (element->rdf_attr[RDF_ATTR_about]!=NULL) +
           (element->rdf_attr[RDF_ATTR_nodeID]!=NULL)>1) {
          raptor_update_document_locator(rdf_parser);
          raptor_parser_error(rdf_parser, "Multiple attributes of rdf:ID, rdf:about and rdf:nodeID on element %s - only one allowed.", el_name);
        }

        if(element->rdf_attr[RDF_ATTR_ID]) {
          element->subject.id=element->rdf_attr[RDF_ATTR_ID];
          element->rdf_attr[RDF_ATTR_ID]=NULL;
          element->subject.uri=raptor_new_uri_from_id(raptor_inscope_base_uri(rdf_parser), element->subject.id);
          element->subject.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          element->subject.uri_source=RAPTOR_URI_SOURCE_ID;
          if(!raptor_valid_xml_ID(rdf_parser, element->subject.id)) {
            raptor_parser_error(rdf_parser, "Illegal rdf:ID value '%s'", element->subject.id);
            state=RAPTOR_STATE_SKIPPING;
            element->child_state=RAPTOR_STATE_SKIPPING;
            finished=1;
            break;
          }
          if(raptor_record_ID(rdf_parser, element, element->subject.id)) {
            raptor_parser_error(rdf_parser, "Duplicated rdf:ID value '%s'", element->subject.id);
            state=RAPTOR_STATE_SKIPPING;
            element->child_state=RAPTOR_STATE_SKIPPING;
            finished=1;
            break;
          }
        } else if (element->rdf_attr[RDF_ATTR_about]) {
          element->subject.uri=raptor_new_uri_relative_to_base(raptor_inscope_base_uri(rdf_parser), (char*)element->rdf_attr[RDF_ATTR_about]);
          element->subject.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          element->subject.uri_source=RAPTOR_URI_SOURCE_URI;
        } else if (element->rdf_attr[RDF_ATTR_nodeID]) {
          element->subject.id=element->rdf_attr[RDF_ATTR_nodeID];
          element->rdf_attr[RDF_ATTR_nodeID]=NULL;
          element->subject.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
          element->subject.uri_source=RAPTOR_URI_SOURCE_BLANK_ID;
          if(!raptor_valid_xml_ID(rdf_parser, element->subject.id)) {
            raptor_parser_error(rdf_parser, "Illegal rdf:nodeID value '%s'", element->subject.id);
            state=RAPTOR_STATE_SKIPPING;
            element->child_state=RAPTOR_STATE_SKIPPING;
            finished=1;
            break;
          }
        } else if (element->parent && 
                   element->parent->child_content_type != RAPTOR_ELEMENT_CONTENT_TYPE_COLLECTION &&
                   element->parent->child_content_type != RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION &&
                   (element->parent->object.uri || element->parent->object.id)) {
          /* copy from parent (property element), it has a URI for us */
          raptor_copy_identifier(&element->subject, &element->parent->object);
        } else {
          element->subject.id=raptor_generate_id(rdf_parser, 0);
          element->subject.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
          element->subject.uri_source=RAPTOR_URI_SOURCE_GENERATED;
        }


        if(element->rdf_attr[RDF_ATTR_bagID]) {
          if(rdf_parser->feature_allow_bagID) {
            element->bag.id=element->rdf_attr[RDF_ATTR_bagID];
            element->rdf_attr[RDF_ATTR_bagID]=NULL;
            element->bag.uri=raptor_new_uri_from_id(raptor_inscope_base_uri(rdf_parser), element->bag.id);
            element->bag.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
            element->bag.uri_source=RAPTOR_URI_SOURCE_GENERATED;
            
            if(!raptor_valid_xml_ID(rdf_parser, element->bag.id)) {
              raptor_parser_error(rdf_parser, "Illegal rdf:bagID value '%s'", element->bag.id);
              state=RAPTOR_STATE_SKIPPING;
              element->child_state=RAPTOR_STATE_SKIPPING;
              finished=1;
              break;
            }
            if(raptor_record_ID(rdf_parser, element, element->bag.id)) {
              raptor_parser_error(rdf_parser, "Duplicated rdf:bagID value '%s'", element->bag.id);
              state=RAPTOR_STATE_SKIPPING;
              element->child_state=RAPTOR_STATE_SKIPPING;
              finished=1;
              break;
            }

            raptor_parser_warning(rdf_parser, "rdf:bagID is deprecated.");

            raptor_generate_statement(rdf_parser, 
                                      element->bag.uri,
                                      element->bag.id,
                                      element->bag.type,
                                      element->bag.uri_source,
                                      
                                      RAPTOR_RDF_type_URI(rdf_xml_parser),
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                      RAPTOR_URI_SOURCE_URI,
                                      
                                      RAPTOR_RDF_Bag_URI(rdf_xml_parser),
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                      RAPTOR_URI_SOURCE_NOT_URI,
                                      NULL,
                                      
                                      NULL,
                                      NULL);
          } else {
            /* bagID forbidden */
            raptor_parser_error(rdf_parser, "rdf:bagID is forbidden.");
            state=RAPTOR_STATE_SKIPPING;
            element->child_state=RAPTOR_STATE_SKIPPING;
            finished=1;
            break;
          }
        }


        if(element->parent) {

          /* In a rdf:parseType="Collection" the resources are appended
           * to the list at the genid element->parent->tail_id
           */
          if (element->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_COLLECTION ||
              element->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) {
            const unsigned char * idList = raptor_generate_id(rdf_parser, 0);
            
            /* <idList> rdf:type rdf:List */
            raptor_uri *collection_uri=(element->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) ? RAPTOR_DAML_List_URI(rdf_xml_parser) : RAPTOR_RDF_List_URI(rdf_xml_parser);

            raptor_generate_statement(rdf_parser, 
                                      NULL,
                                      idList,
                                      RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
                                      RAPTOR_URI_SOURCE_ID,

                                      RAPTOR_RDF_type_URI(rdf_xml_parser),
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                      RAPTOR_URI_SOURCE_URI,

                                      collection_uri,
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                      RAPTOR_URI_SOURCE_URI,
                                      NULL,

                                      NULL,
                                      element);

            collection_uri=(element->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) ? RAPTOR_DAML_first_URI(rdf_xml_parser) : RAPTOR_RDF_first_URI(rdf_xml_parser);

            /* <idList> rdf:first <element->uri> */
            raptor_generate_statement(rdf_parser, 
                                      NULL,
                                      idList,
                                      RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
                                      RAPTOR_URI_SOURCE_ID,

                                      collection_uri,
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                      RAPTOR_URI_SOURCE_URI,

                                      element->subject.uri,
                                      element->subject.id,
                                      element->subject.type,
                                      element->subject.uri_source,
                                      NULL,

                                      NULL,
                                      NULL);
            
            /* If there is no rdf:parseType="Collection" */
            if (!element->parent->tail_id) {
              int len;
              unsigned char *new_id;
              
              /* Free any existing object URI still around
               * I suspect this can never happen.
               */
              if(element->parent->object.uri) {
                abort();
                raptor_free_uri(element->parent->object.uri);
              }

              len=strlen((char*)idList);
              new_id=(unsigned char*)RAPTOR_MALLOC(cstring, len+1);
              if(!len) {
                if(new_id)
                  RAPTOR_FREE(cstring, new_id);
                return;
              }
              strncpy((char*)new_id, (char*)idList, len+1);

              element->parent->object.id=new_id;
              element->parent->object.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
              element->parent->object.uri_source=RAPTOR_URI_SOURCE_ID;
            } else {
              collection_uri=(element->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) ? RAPTOR_DAML_rest_URI(rdf_xml_parser) : RAPTOR_RDF_rest_URI(rdf_xml_parser);
              /* _:tail_id rdf:rest _:listRest */
              raptor_generate_statement(rdf_parser, 
                                        NULL,
                                        element->parent->tail_id,
                                        RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
                                        RAPTOR_URI_SOURCE_ID,

                                        collection_uri,
                                        NULL,
                                        RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                        RAPTOR_URI_SOURCE_URI,

                                        NULL,
                                        idList,
                                        RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
                                        RAPTOR_URI_SOURCE_ID,
                                        NULL,

                                        NULL,
                                        NULL);
            }

            /* update new tail */
            if(element->parent->tail_id)
              RAPTOR_FREE(cstring, (char*)element->parent->tail_id);

            element->parent->tail_id=idList;
            
          } else if(element->parent->state != RAPTOR_STATE_UNKNOWN &&
                    element->state != RAPTOR_STATE_PARSETYPE_RESOURCE) {
            /* If there is a parent element (property) containing this
             * element (node) and it has no object, set it from this subject
             */
            
            if(element->parent->object.uri) {
              raptor_update_document_locator(rdf_parser);
              raptor_parser_error(rdf_parser, "Tried to set multiple objects of a statement");
            } else {
              /* Store URI of this node in our parent as the property object */
              raptor_copy_identifier(&element->parent->object, &element->subject);
              element->parent->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
            }

          }
        }
        

        /* If this is a node element, generate the rdf:type statement
         * from this node
         */
        if(state == RAPTOR_STATE_NODE_ELEMENT)
          raptor_generate_statement(rdf_parser, 
                                    element->subject.uri,
                                    element->subject.id,
                                    element->subject.type,
                                    element->subject.uri_source,

                                    RAPTOR_RDF_type_URI(rdf_xml_parser),
                                    NULL,
                                    RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                    RAPTOR_URI_SOURCE_URI,

                                    element->name->uri,
                                    NULL,
                                    RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                    element->object.uri_source,
                                    NULL,

                                    &element->reified,
                                    element);

        raptor_process_property_attributes(rdf_parser, element, element, 0);

        /* for both productions now need some more content or
         * property elements before can do any more work.
         */

        element->child_state=RAPTOR_STATE_PROPERTYELT;
        element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTIES;
        finished=1;
        break;


      case RAPTOR_STATE_PARSETYPE_OTHER:
        /* FIXME */

        /* FALLTHROUGH */

      case RAPTOR_STATE_PARSETYPE_LITERAL:
        {
          char *fmt_buffer;
          int fmt_length;
          fmt_buffer=raptor_format_element(rdf_parser, element, &fmt_length, 0);
          if(fmt_buffer && fmt_length) {
            /* Append cdata content content */
            if(element->content_cdata) {
              /* Append */
              char *new_cdata=(char*)RAPTOR_MALLOC(cstring, element->content_cdata_length + fmt_length + 1);
              if(new_cdata) {
                strncpy(new_cdata, element->content_cdata,
                        element->content_cdata_length);
                strcpy(new_cdata+element->content_cdata_length, fmt_buffer);
                RAPTOR_FREE(cstring, element->content_cdata);
                element->content_cdata=new_cdata;
              }
              RAPTOR_FREE(cstring, fmt_buffer);

#ifdef RAPTOR_DEBUG_CDATA
              RAPTOR_DEBUG3(raptor_start_element_grammar,
                            "content cdata appended, now: '%s' (%d bytes)\n", 
                            element->content_cdata,
                            element->content_cdata_length);
#endif

            } else {
              /* Copy - is empty */
              element->content_cdata       =fmt_buffer;
              element->content_cdata_length=fmt_length;

#ifdef RAPTOR_DEBUG_CDATA
              RAPTOR_DEBUG3(raptor_start_element_grammar,
                            "content cdata copied, now: '%s' (%d bytes)\n", 
                            element->content_cdata,
                            element->content_cdata_length);
#endif

            }
          }

          element->child_state = RAPTOR_STATE_PARSETYPE_LITERAL;
          element->child_content_type = RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL;
        }
        
        finished=1;
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

        /* Handling rdf:li as a property, noting special processing */ 
        if(element_in_rdf_ns && 
           raptor_uri_equals(element->name->uri, RAPTOR_RDF_li_URI(rdf_xml_parser))) {
          state=RAPTOR_STATE_MEMBER_PROPERTYELT;
        }


        if(element_in_rdf_ns && raptor_forbidden_propertyElement_name((const char*)el_name)) {
          raptor_parser_error(rdf_parser, "rdf:%s is forbidden as a property element.", el_name);
          state=RAPTOR_STATE_SKIPPING;
          element->child_state=RAPTOR_STATE_SKIPPING;
          finished=1;
          break;
        }
          

        /* rdf:ID on a property element - reify a statement. 
         * Allowed on all property element forms
         */
        if(element->rdf_attr[RDF_ATTR_ID]) {
          element->reified.id=element->rdf_attr[RDF_ATTR_ID];
          element->rdf_attr[RDF_ATTR_ID]=NULL;
          element->reified.uri=raptor_new_uri_from_id(raptor_inscope_base_uri(rdf_parser), element->reified.id);
          element->reified.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          element->reified.uri_source=RAPTOR_URI_SOURCE_GENERATED;

          if(!raptor_valid_xml_ID(rdf_parser, element->reified.id)) {
            raptor_parser_error(rdf_parser, "Illegal rdf:ID value '%s'", element->reified.id);
            state=RAPTOR_STATE_SKIPPING;
            element->child_state=RAPTOR_STATE_SKIPPING;
            finished=1;
            break;
          }
          if(raptor_record_ID(rdf_parser, element, element->reified.id)) {
            raptor_parser_error(rdf_parser, "Duplicated rdf:ID value '%s'", element->reified.id);
            state=RAPTOR_STATE_SKIPPING;
            element->child_state=RAPTOR_STATE_SKIPPING;
            finished=1;
            break;
          }
        }
        
        /* rdf:datatype on a property element.  
         * Only allowed for
         *   http://www.w3.org/TR/rdf-syntax-grammar/#literalPropertyElt 
         */
        if (element->rdf_attr[RDF_ATTR_datatype]) {
          element->object_literal_datatype=raptor_new_uri_relative_to_base(raptor_inscope_base_uri(rdf_parser), (char*)element->rdf_attr[RDF_ATTR_datatype]);
          element->rdf_attr[RDF_ATTR_datatype]=NULL; 
        }

        if(element->rdf_attr[RDF_ATTR_bagID]) {

          if(rdf_parser->feature_allow_bagID) {

            if(element->rdf_attr[RDF_ATTR_resource] ||
               element->rdf_attr[RDF_ATTR_parseType]) {
              
              raptor_parser_error(rdf_parser, "rdf:bagID is forbidden on property element %s with an rdf:resource or rdf:parseType attribute.", el_name);
              /* prevent this being used later either */
              element->rdf_attr[RDF_ATTR_bagID]=NULL;
            } else {
              element->bag.id=element->rdf_attr[RDF_ATTR_bagID];
              element->rdf_attr[RDF_ATTR_bagID]=NULL;
              element->bag.uri=raptor_new_uri_from_id(raptor_inscope_base_uri(rdf_parser), element->bag.id);
              element->bag.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
              element->bag.uri_source=RAPTOR_URI_SOURCE_GENERATED;
              
              if(!raptor_valid_xml_ID(rdf_parser, element->bag.id)) {
                raptor_parser_error(rdf_parser, "Illegal rdf:bagID value '%s'", element->bag.id);
                state=RAPTOR_STATE_SKIPPING;
                element->child_state=RAPTOR_STATE_SKIPPING;
                finished=1;
                break;
              }
              if(raptor_record_ID(rdf_parser, element, element->bag.id)) {
                raptor_parser_error(rdf_parser, "Duplicated rdf:bagID value '%s'", element->bag.id);
                state=RAPTOR_STATE_SKIPPING;
                element->child_state=RAPTOR_STATE_SKIPPING;
                finished=1;
                break;
              }

              raptor_parser_warning(rdf_parser, "rdf:bagID is deprecated.");
            }
          } else {
            /* bagID forbidden */
            raptor_parser_error(rdf_parser, "rdf:bagID is forbidden.");
            state=RAPTOR_STATE_SKIPPING;
            element->child_state=RAPTOR_STATE_SKIPPING;
            finished=1;
            break;
          }
        } /* if rdf:bagID on property element */
        

        element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTY_CONTENT;

        if (element->rdf_attr[RDF_ATTR_parseType]) {
          const unsigned char *parse_type=element->rdf_attr[RDF_ATTR_parseType];

          if(!strcmp((char*)parse_type, "Literal")) {
            element->child_state=RAPTOR_STATE_PARSETYPE_LITERAL;
            element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL;
            element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL;
          } else if (!strcmp((char*)parse_type, "Resource")) {
            state=RAPTOR_STATE_PARSETYPE_RESOURCE;
            element->child_state=RAPTOR_STATE_PROPERTYELT;
            element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTIES;

            /* create a node for the subject of the contained properties */
            element->subject.id=raptor_generate_id(rdf_parser, 0);
            element->subject.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
            element->subject.uri_source=RAPTOR_URI_SOURCE_GENERATED;
          } else if(!strcmp((char*)parse_type, "Collection")) {
            /* An rdf:parseType="Collection" appears as a single node */
            element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
            element->child_state=RAPTOR_STATE_PARSETYPE_COLLECTION;
            element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_COLLECTION;
          } else {
            if(rdf_parser->feature_allow_other_parseTypes) {
              if(!raptor_strcasecmp((char*)parse_type, "daml:collection")) {
                /* A DAML collection appears as a single node */
                element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
                element->child_state=RAPTOR_STATE_PARSETYPE_COLLECTION;
                element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION;
              } else {
                element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL;
                element->child_state=RAPTOR_STATE_PARSETYPE_OTHER;
                element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL;
              }
            } else {
              element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL;
              element->child_state=RAPTOR_STATE_PARSETYPE_LITERAL;
              element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL;
            }
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
          if(element->reified.id) {
            element->reified.uri=raptor_new_uri_from_id(raptor_inscope_base_uri(rdf_parser), element->reified.id);
            element->reified.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
            element->reified.uri_source=RAPTOR_URI_SOURCE_GENERATED;
          }

          if(element->rdf_attr[RDF_ATTR_resource] ||
             element->rdf_attr[RDF_ATTR_nodeID]) {
            /* Done - wait for end of this element to end in order to 
             * check the element was empty as expected */
            element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
          } else {
            /* Otherwise process content in obj (value) state */
            element->child_state=RAPTOR_STATE_NODE_ELEMENT_LIST;
            element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTY_CONTENT;
          }
        }

        finished=1;

        break;


      default:
        raptor_parser_fatal_error(rdf_parser, "raptor_start_element_grammar - unexpected parser state %d - %s", state, raptor_state_as_string(state));
        finished=1;

    } /* end switch */

    if(state != element->state) {
      element->state=state;
      RAPTOR_DEBUG3(raptor_start_element_grammar, 
                    "Moved to state %d - %s\n",
                    state, raptor_state_as_string(state));
    }

  } /* end while */

  RAPTOR_DEBUG2(raptor_start_element_grammar, 
                "Ending in state %s\n",
                raptor_state_as_string(state));
}


static void
raptor_end_element_grammar(raptor_parser *rdf_parser,
                           raptor_element *element) 
{
  raptor_state state;
  int finished;
  const unsigned char *el_name=element->name->local_name;
  int element_in_rdf_ns=(element->name->nspace && 
                         element->name->nspace->is_rdf_ms);
  raptor_xml_parser *rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;


  state=element->state;
  RAPTOR_DEBUG2(raptor_end_element_grammar, "Starting in state %s\n",
                raptor_state_as_string(state));

  finished= 0;
  while(!finished) {
    switch(state) {
      case RAPTOR_STATE_SKIPPING:
        finished=1;
        break;

      case RAPTOR_STATE_UNKNOWN:
        finished=1;
        break;

      case RAPTOR_STATE_NODE_ELEMENT_LIST:
        if(element_in_rdf_ns && 
           raptor_uri_equals(element->name->uri, RAPTOR_RDF_RDF_URI(rdf_xml_parser))) {
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
        raptor_update_document_locator(rdf_parser);
        raptor_parser_warning(rdf_parser, "Element %s ended, expected end of RDF element", el_name);
        state=RAPTOR_STATE_UNKNOWN;
        finished=1;
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
                                    NULL,

                                    NULL,
                                    element);
        else if(state == RAPTOR_STATE_PARSETYPE_RESOURCE && 
                element->parent &&
                (element->parent->subject.uri || element->parent->subject.id)) {
          /* Handle rdf:li as the rdf:parseType="resource" property */
          if(element_in_rdf_ns && 
             raptor_uri_equals(element->name->uri, RAPTOR_RDF_li_URI(rdf_xml_parser))) {
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
                                      
                                      element->subject.uri,
                                      element->subject.id,
                                      element->subject.type,
                                      element->subject.uri_source,
                                      NULL,

                                      &element->reified,
                                      element->parent);
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
                                      
                                      element->subject.uri,
                                      element->subject.id,
                                      element->subject.type,
                                      element->subject.uri_source,
                                      NULL,

                                      &element->reified,
                                      element->parent);
          }
        }
        finished=1;
        break;

      case RAPTOR_STATE_PARSETYPE_COLLECTION:

        finished=1;
        break;

      case RAPTOR_STATE_PARSETYPE_OTHER:
        /* FIXME */

        /* FALLTHROUGH */

      case RAPTOR_STATE_PARSETYPE_LITERAL:
        element->parent->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL;

        /* Append closing element */
        {
          char *fmt_buffer;
          int fmt_length;
          fmt_buffer=raptor_format_element(rdf_parser, element, &fmt_length,1);
          if(fmt_buffer && fmt_length) {
            /* Append cdata content content */
            char *new_cdata=(char*)RAPTOR_MALLOC(cstring, element->content_cdata_length + fmt_length + 1);
            if(new_cdata) {
              strncpy(new_cdata, element->content_cdata,
                      element->content_cdata_length);
              strcpy(new_cdata+element->content_cdata_length, fmt_buffer);
              RAPTOR_FREE(cstring, element->content_cdata);
              element->content_cdata=new_cdata;
              element->content_cdata_length += fmt_length;
            }
            RAPTOR_FREE(cstring, fmt_buffer);
          }
        }
        
#ifdef RAPTOR_DEBUG_CDATA
        RAPTOR_DEBUG3(raptor_end_element_grammar,
                      "content cdata now: '%s' (%d bytes)\n", 
                       element->content_cdata, element->content_cdata_length);
#endif
         
        /* Append this cdata content to parent element cdata content */
        if(element->parent->content_cdata) {
          /* Append */
          char *new_cdata=(char*)RAPTOR_MALLOC(cstring, element->parent->content_cdata_length + element->content_cdata_length + 1);
          if(new_cdata) {
            strncpy(new_cdata, element->parent->content_cdata,
                    element->parent->content_cdata_length);
            strncpy(new_cdata+element->parent->content_cdata_length, 
                    element->content_cdata, element->content_cdata_length+1);
            RAPTOR_FREE(cstring, element->parent->content_cdata);
            element->parent->content_cdata=new_cdata;
            element->parent->content_cdata_length += element->content_cdata_length;
            /* Done with our cdata - free it before pointer is zapped */
            RAPTOR_FREE(cstring, element->content_cdata);
          }

#ifdef RAPTOR_DEBUG_CDATA
          RAPTOR_DEBUG3(raptor_end_element_grammar,
                        "content cdata appended to parent, now: '%s' (%d bytes)\n", 
                        element->parent->content_cdata,
                        element->parent->content_cdata_length);
#endif

        } else {
          /* Copy - parent is empty */
          element->parent->content_cdata       =element->content_cdata;
          element->parent->content_cdata_length=element->content_cdata_length+1;
#ifdef RAPTOR_DEBUG_CDATA
          RAPTOR_DEBUG3(raptor_end_element_grammar,
                        "content cdata copied to parent, now: '%s' (%d bytes)\n",
                        element->parent->content_cdata,
                        element->parent->content_cdata_length);
#endif

        }

        element->content_cdata=NULL;
        element->content_cdata_length=0;

        finished=1;
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

        if(element->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTY_CONTENT) {
          if(element->content_cdata_seen) 
            element->content_type= RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL;
          else if (element->content_element_seen) 
            element->content_type= RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTIES;
          else { /* Empty Literal */
            element->object.type= RAPTOR_IDENTIFIER_TYPE_LITERAL;
            element->content_type= RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL;
          }
          
        }


        /* Handle terminating a rdf:parseType="Collection" list */
        if(element->child_content_type == RAPTOR_ELEMENT_CONTENT_TYPE_COLLECTION ||
           element->child_content_type == RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) {
          raptor_uri* nil_uri=(element->child_content_type == RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) ? RAPTOR_DAML_nil_URI(rdf_xml_parser) : RAPTOR_RDF_nil_URI(rdf_xml_parser);
          if (!element->tail_id) {
            /* If No List: set object of statement to rdf:nil */
            element->object.uri= raptor_uri_copy(nil_uri);
            element->object.id= NULL;
            element->object.type= RAPTOR_IDENTIFIER_TYPE_RESOURCE;
            element->object.uri_source= RAPTOR_URI_SOURCE_URI;
          } else {
            raptor_uri* rest_uri=(element->child_content_type == RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) ? RAPTOR_DAML_rest_URI(rdf_xml_parser) : RAPTOR_RDF_rest_URI(rdf_xml_parser);
            /* terminate the list */
            raptor_generate_statement(rdf_parser, 
                                      NULL,
                                      element->tail_id,
                                      RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
                                      RAPTOR_URI_SOURCE_ID,
                                      
                                      rest_uri,
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                      RAPTOR_URI_SOURCE_URI,
                                      
                                      nil_uri,
                                      NULL,
                                      RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                      RAPTOR_URI_SOURCE_URI,
                                      NULL,

                                      NULL,
                                      NULL);
          }

        } /* end rdf:parseType="Collection" termination */
        

        RAPTOR_DEBUG3(raptor_end_element_grammar,
                      "Content type %s (%d)\n", raptor_element_content_type_as_string(element->content_type), element->content_type);

        switch(element->content_type) {
          case RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE:

            if(element->object.type == RAPTOR_IDENTIFIER_TYPE_UNKNOWN) {
              if(element->rdf_attr[RDF_ATTR_resource]) {
                element->object.uri=raptor_new_uri_relative_to_base(raptor_inscope_base_uri(rdf_parser),
                                                    (char*)element->rdf_attr[RDF_ATTR_resource]);
                element->object.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
                element->object.uri_source=RAPTOR_URI_SOURCE_URI;
                element->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
              } else if(element->rdf_attr[RDF_ATTR_nodeID]) {
                element->object.id=element->rdf_attr[RDF_ATTR_nodeID];
                element->rdf_attr[RDF_ATTR_nodeID]=NULL;
                element->object.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
                element->object.uri_source=RAPTOR_URI_SOURCE_BLANK_ID;
                element->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
                if(!raptor_valid_xml_ID(rdf_parser, element->object.id)) {
                  raptor_parser_error(rdf_parser, "Illegal rdf:nodeID value '%s'", element->object.id);
                  state=RAPTOR_STATE_SKIPPING;
                  element->child_state=RAPTOR_STATE_SKIPPING;
                  finished=1;
                  break;
                }
              } else {
                element->object.id=raptor_generate_id(rdf_parser, 0);
                element->object.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
                element->object.uri_source=RAPTOR_URI_SOURCE_GENERATED;
                element->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
              }

              raptor_process_property_attributes(rdf_parser, element, 
                                                 element->parent, 
                                                 &element->object);

            }

            /* We know object is a resource, so delete any unsignficant
             * whitespace so that FALLTHROUGH code below finds the object.
             */
            if(element->content_cdata) {
              RAPTOR_FREE(raptor_qname_array, element->content_cdata);
              element->content_cdata=NULL;
              element->content_cdata_length=0;
            }

            /* FALLTHROUGH */
          case RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL:

            if(element->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL) {

              if(rdf_parser->feature_allow_bagID) {
                /* Only an empty literal can have a rdf:bagID */
                if(element->bag.uri || element->bag.id) {
                  if(element->content_cdata_length > 0) {
                    raptor_parser_error(rdf_parser, "rdf:bagID is forbidden on a literal property element %s.", el_name);
                    /* prevent this being used later either */
                    element->rdf_attr[RDF_ATTR_bagID]=NULL;
                  } else
                    raptor_generate_statement(rdf_parser, 
                                              element->bag.uri,
                                              element->bag.id,
                                              element->bag.type,
                                              element->bag.uri_source,
                                              
                                              RAPTOR_RDF_type_URI(rdf_xml_parser),
                                              NULL,
                                              RAPTOR_IDENTIFIER_TYPE_PREDICATE,
                                              RAPTOR_URI_SOURCE_URI,
                                              
                                              RAPTOR_RDF_Bag_URI(rdf_xml_parser),
                                              NULL,
                                              RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                              RAPTOR_URI_SOURCE_NOT_URI,
                                              NULL,
                                              
                                              NULL,
                                              NULL);
                }
              } /* if rdf:bagID */

              /* If there is empty literal content with properties
               * generate a node to hang properties off 
               */
              if(raptor_element_has_property_attributes(element) &&
                 element->content_cdata_length > 0) {
                raptor_parser_error(rdf_parser, "Literal property element %s has property attributes", el_name);
                state=RAPTOR_STATE_SKIPPING;
                element->child_state=RAPTOR_STATE_SKIPPING;
                finished=1;
                break;
              }

              if(element->object.type == RAPTOR_IDENTIFIER_TYPE_LITERAL &&
                 raptor_element_has_property_attributes(element) &&
                 !element->object.uri) {
                element->object.id=raptor_generate_id(rdf_parser, 0);
                element->object.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
                element->object.uri_source=RAPTOR_URI_SOURCE_GENERATED;
                element->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
              }
              
              raptor_process_property_attributes(rdf_parser, element, 
                                                 element, &element->object);
            }
            

            if(state == RAPTOR_STATE_MEMBER_PROPERTYELT) {
              int object_is_literal=(element->content_cdata != NULL); /* FIXME */
              raptor_uri *object_uri=object_is_literal ? (raptor_uri*)element->content_cdata : element->object.uri;
              raptor_identifier_type object_type=object_is_literal ? RAPTOR_IDENTIFIER_TYPE_LITERAL : element->object.type;
              raptor_uri *literal_datatype=object_is_literal ? element->object_literal_datatype: NULL;

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
                                        literal_datatype,

                                        &element->reified,
                                        element->parent);
            } else {
              int object_is_literal=(element->content_cdata != NULL); /* FIXME */
              raptor_uri *object_uri=object_is_literal ? (raptor_uri*)element->content_cdata : element->object.uri;
              raptor_identifier_type object_type=object_is_literal ? RAPTOR_IDENTIFIER_TYPE_LITERAL : element->object.type;
              raptor_uri *literal_datatype=object_is_literal ? element->object_literal_datatype: NULL;
              
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
                                        literal_datatype,

                                        &element->reified,
                                        element->parent);
            }
          
            break;

        case RAPTOR_ELEMENT_CONTENT_TYPE_PRESERVED:
        case RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL:
            if(state == RAPTOR_STATE_MEMBER_PROPERTYELT) {
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
                                        NULL,

                                        &element->reified,
                                        element->parent);
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
                                        NULL,

                                        &element->reified,
                                        element->parent);
            }

          break;

          case RAPTOR_ELEMENT_CONTENT_TYPE_COLLECTION:
          case RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION:
            abort();
            
            break;

          default:
            raptor_parser_fatal_error(rdf_parser, "raptor_end_element_grammar state RAPTOR_STATE_PROPERTYELT - unexpected content type %s (%d)", raptor_element_content_type_as_string(element->content_type), element->content_type);
        } /* end switch */

      finished=1;
      break;


      default:
        raptor_parser_fatal_error(rdf_parser, "raptor_end_element_grammar - unexpected parser state %d - %s", state, raptor_state_as_string(state));
        finished=1;

    } /* end switch */

    if(state != element->state) {
      element->state=state;
      RAPTOR_DEBUG3(raptor_end_element_grammar, "Moved to state %d - %s\n",
                    state, raptor_state_as_string(state));
    }

  } /* end while */

  RAPTOR_DEBUG2(raptor_end_element_grammar, 
                "Ending in state %s\n", raptor_state_as_string(state));

}


/**
 * raptor_inscope_xml_language - Return the in-scope xml:lang
 * @rdf_parser: Raptor parser object
 * 
 * Looks for the innermost xml:lang on an element
 * 
 * Return value: The xml:lang value or NULL if none is in scope. 
 **/
const unsigned char*
raptor_inscope_xml_language(raptor_parser *rdf_parser)
{
  raptor_xml_parser *rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;
  raptor_element *element;
  
  for(element=rdf_xml_parser->current_element; element; element=element->parent)
    if(element->xml_language)
      return element->xml_language;
    
  return NULL;
}


/**
 * raptor_inscope_base_uri - Return the in-scope base URI
 * @rdf_parser: Raptor parser object
 * 
 * Looks for the innermost xml:base on an element or document URI
 * 
 * Return value: The URI string value or NULL on failure.
 **/
raptor_uri*
raptor_inscope_base_uri(raptor_parser *rdf_parser)
{
  raptor_xml_parser *rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;
  raptor_element *element;
  
  for(element=rdf_xml_parser->current_element; element; element=element->parent)
    if(element->base_uri)
      return element->base_uri;
    
  return rdf_parser->base_uri;
}


/**
 * raptor_record_ID - Record an rdf:ID / rdf:bagID value (with xml base) and check it hasn't been seen already
 * @rdf_parser: Raptor parser object
 * @element: Current element
 * @id: ID string
 * 
 * Record and check the ID values, if they have been seen already.
 * per in-scope-base URI.
 * 
 * Return value: non-zero if already seen, or failure
 **/
static int
raptor_record_ID(raptor_parser *rdf_parser, raptor_element *element,
                 const unsigned char *id)
{
  raptor_xml_parser *rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;
  raptor_uri* base_uri=raptor_inscope_base_uri(rdf_parser);
  char *item;
  char *base_uri_string;
  size_t base_uri_string_len;
  size_t id_len;
  size_t len;
  int rc;
  
  base_uri_string=raptor_uri_as_counted_string(base_uri, &base_uri_string_len);
  id_len=strlen(id);
  
  /* Storing ID + ' ' + base-uri-string + '\0' */
  len=id_len+1+strlen(base_uri_string)+1;

  item=(char*)RAPTOR_MALLOC(cstring, len);
  if(!item)
    return 1;

  strcpy(item, id);
  item[id_len]=' ';
  strcpy(item+id_len+1, base_uri_string); /* strcpy for end '\0' */
  
  rc=raptor_set_add(rdf_xml_parser->id_set, item, len);
  RAPTOR_FREE(cstring, item);

  return (rc != 0);
}




#ifdef RAPTOR_XML_LIBXML
xmlParserCtxtPtr
raptor_get_libxml_context(raptor_parser *rdf_parser) {
  return ((raptor_xml_parser*)rdf_parser->context)->xc;
}

void
raptor_set_libxml_document_locator(raptor_parser *rdf_parser,
                                   xmlSAXLocatorPtr loc) {
  ((raptor_xml_parser*)rdf_parser->context)->loc=loc;
}

xmlSAXLocatorPtr
raptor_get_libxml_document_locator(raptor_parser *rdf_parser) {
  return ((raptor_xml_parser*)rdf_parser->context)->loc;
}

#ifdef RAPTOR_LIBXML_MY_ENTITIES
raptor_xml_entity*
raptor_get_libxml_entities(raptor_parser *rdf_parser) {
  return ((raptor_xml_parser*)rdf_parser->context)->entities;
}

void
raptor_set_libxml_entities(raptor_parser *rdf_parser, 
                           raptor_xml_entity* entities) {
  ((raptor_xml_parser*)rdf_parser->context)->entities=entities;
}
#endif

#endif


#ifdef RAPTOR_XML_EXPAT
void
raptor_expat_update_document_locator (raptor_parser *rdf_parser) {
  raptor_locator *locator=&rdf_parser->locator;
  raptor_xml_parser *rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;

  locator->line=XML_GetCurrentLineNumber(rdf_xml_parser->xp);
  locator->column=XML_GetCurrentColumnNumber(rdf_xml_parser->xp);
  locator->byte=XML_GetCurrentByteIndex(rdf_xml_parser->xp);
}
#endif


static void
raptor_xml_parser_register_factory(raptor_parser_factory *factory) 
{
  factory->context_length     = sizeof(raptor_xml_parser);
  
  factory->init      = raptor_xml_parse_init;
  factory->terminate = raptor_xml_parse_terminate;
  factory->start     = raptor_xml_parse_start;
  factory->chunk     = raptor_xml_parse_chunk;
}


void
raptor_init_parser_rdfxml (void) {
#ifdef RAPTOR_XML_LIBXML
  xmlInitParser();
#endif
  raptor_parser_register_factory("rdfxml", "RDF/XML",
                                 &raptor_xml_parser_register_factory);
}

void
raptor_terminate_parser_rdfxml (void) {
#ifdef RAPTOR_XML_LIBXML
  xmlCleanupParser();
#endif
}

#ifdef RAPTOR_DEBUG
void
raptor_xml_parser_stats_print(raptor_xml_parser* rdf_xml_parser, FILE *stream) {
  fputs("rdf:ID set ", stream);
  raptor_set_stats_print(rdf_xml_parser->id_set, stream);
}
#endif
