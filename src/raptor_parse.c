/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_parse.c - Redland Parser for RDF (RAPTOR)
 *
 * $Id$
 *
 * Copyright (C) 2000-2002 David Beckett - http://purl.org/net/dajobe/
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
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
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


/* Raptor structures */

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
  RAPTOR_STATE_PARSETYPE_COLLECTION = 6430,

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
  "parseTypeCollection (revised syntax)", /* 6.44 */
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

  RDF_ATTR_LAST            = RDF_ATTR_List
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
} rdf_attr_info[]={
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
  { "List",            RAPTOR_IDENTIFIER_TYPE_LITERAL , 0 }
};

/* In above 'Useless' indicates it generates parts of a reified statement
 * in which in the current RDF model, those parts are not allowed - i.e.
 * literals in the subject and predicate positions
 */


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
  const char * rdf_attr[RDF_ATTR_LAST+1];
  /* how many of above seen */
  int rdf_attr_count;

  /* value of xml:lang attribute on this element or NULL */
  const char *xml_language;

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
  const char *tail_id;
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
};

typedef struct raptor_xml_parser_s raptor_xml_parser;



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


#ifdef HAVE_XML_SetNamespaceDeclHandler
static void raptor_start_namespace_decl_handler(void *user_data, const XML_Char *prefix, const XML_Char *uri);
static void raptor_end_namespace_decl_handler(void *user_data, const XML_Char *prefix);
#endif



/* prototypes for element functions */
static raptor_element* raptor_element_pop(raptor_xml_parser *rdf_parser);
static void raptor_element_push(raptor_xml_parser *rdf_parser, raptor_element* element);
static void raptor_free_element(raptor_element *element);
#ifdef RAPTOR_DEBUG
static void raptor_print_element(raptor_element *element, FILE* stream);
#endif


/* prototypes for grammar functions */
static void raptor_start_element_grammar(raptor_parser *parser, raptor_element *element);
static void raptor_end_element_grammar(raptor_parser *parser, raptor_element *element);


/* prototype for statement related functions */
static void raptor_generate_statement(raptor_parser *rdf_parser, raptor_uri *subject_uri, const char *subject_id, const raptor_identifier_type subject_type, const raptor_uri_source subject_uri_source, raptor_uri *predicate_uri, const char *predicate_id, const raptor_identifier_type predicate_type, const raptor_uri_source predicate_uri_source, raptor_uri *object_uri, const char *object_id, const raptor_identifier_type object_type, const raptor_uri_source object_uri_source, raptor_uri *literal_datatype, raptor_uri *bag);



/* Prototypes for parsing data functions */
static int raptor_xml_parse_init(raptor_parser* rdf_parser, const char *name);
static void raptor_xml_parse_terminate(raptor_parser *rdf_parser);
static int raptor_xml_parse_start(raptor_parser* rdf_parser, raptor_uri *uri);
static int raptor_xml_parse_chunk(raptor_parser* rdf_parser, const char *buffer, size_t len, int is_end);





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
    RAPTOR_FREE(cstring, (char*)element->object_literal_datatype);

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
raptor_format_element(raptor_element *element, int *length_p, int is_end)
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
  buffer=(char*)RAPTOR_MALLOC(cstring, length + 1);
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



void
raptor_xml_start_element_handler(void *user_data,
                                 const XML_Char *name, const XML_Char **atts)
{
  raptor_parser* rdf_parser;
  raptor_xml_parser* rdf_xml_parser;
  int all_atts_count=0;
  int ns_attributes_count=0;
  raptor_qname** named_attrs=NULL;
  int i;
  raptor_element* element=NULL;
  int non_nspaced_count=0;
  char *xml_language=NULL;
  raptor_uri *xml_base=NULL;
  
  rdf_parser=(raptor_parser*)user_data;
  rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;

#ifdef RAPTOR_XML_EXPAT
#ifdef EXPAT_UTF8_BOM_CRASH
  rdf_parser->tokens_count++;
#endif
#endif

#ifdef RAPTOR_DEBUG
  fputc('\n', stderr);
#endif

  raptor_update_document_locator(rdf_parser);

  rdf_xml_parser->depth++;

  if (atts) {
    /* Round 1 - process XML attributes */
    for (i = 0; atts[i]; i+=2) {
      all_atts_count++;

      /* synthesise the XML namespace events */
      if(!strncmp(atts[i], "xmlns", 5)) {
        /* there is more i.e. xmlns:foo */
        const char *prefix=atts[i][5] ? &atts[i][6] : NULL;

        if(raptor_namespaces_start_namespace(&rdf_parser->namespaces,
                                             prefix, atts[i+1],
                                             rdf_xml_parser->depth)) {
          raptor_parser_fatal_error(rdf_parser, "Out of memory");
          return;
        }
        
        /* Is it ok to zap XML parser array things? */
        atts[i]=NULL; 
        continue;
      }

      if(!strcmp(atts[i], "xml:lang")) {
        xml_language=(char*)RAPTOR_MALLOC(cstring, strlen(atts[i+1])+1);
        if(!xml_language) {
          raptor_parser_fatal_error(rdf_parser, "Out of memory");
          return;
        }
        strcpy(xml_language, atts[i+1]);
        continue;
      }
      
      if(!strcmp(atts[i], "xml:base")) {
        xml_base=raptor_new_uri_relative_to_base(raptor_inscope_base_uri(rdf_parser), atts[i+1]);
        atts[i]=NULL; 
        continue;
      }

      /* delete other xml attributes - not used */
      if(!strncmp(atts[i], "xml", 3)) {
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

        /* If RDF M&S namespace-prefixed attributes */
        if(attr->nspace && attr->nspace->is_rdf_ms) {
          const char *attr_name=attr->local_name;
          int j;

          for(j=0; j<= RDF_ATTR_LAST; j++)
            if(!strcmp(attr_name, rdf_attr_info[j].name)) {
              element->rdf_attr[j]=attr->value;
              element->rdf_attr_count++;
              /* Delete it if it was stored elsewhere */
#if RAPTOR_DEBUG
              RAPTOR_DEBUG3(raptor_xml_start_element_handler,
                            "Found RDF M&S attribute %s URI %s\n",
                            attr_name, attr->value);
#endif
              /* make sure value isn't deleted from qname structure */
              attr->value=NULL;
              raptor_free_qname(attr);
              attr=NULL;
            }
        } /* end if RDF M&S namespaced-prefixed attributes */

        if(!attr)
          continue;

        /* If non namespace-prefixed RDF M&S attributes found on an element */
        if(rdf_parser->feature_allow_non_ns_attributes &&
           !attr->nspace) {
          const char *attr_name=attr->local_name;
          int j;

          for(j=0; j<= RDF_ATTR_LAST; j++)
            if(!strcmp(attr_name, rdf_attr_info[j].name)) {
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
        } /* end if non-namespace prefixed RDF M&S attributes */

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
      /* all attributes were RDF M&S or other specials and deleted
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
      raptor_parser_warning(rdf_parser, "property element %s has multiple object node elements, skipping.", 
                            element->parent->name->local_name);
      element->state=RAPTOR_STATE_SKIPPING;
      element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PRESERVED;

    } else {
      if(!element->parent->child_state)
        raptor_parser_fatal_error(rdf_parser, "raptor_xml_start_element_handler - no parent element child_state set");      

      element->state=element->parent->child_state;
      element->parent->content_element_seen++;
    
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
    element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PRESERVED;
  }
  

  if (element->rdf_attr[RDF_ATTR_aboutEach] || 
      element->rdf_attr[RDF_ATTR_aboutEachPrefix]) {
    raptor_parser_warning(rdf_parser, "element %s has aboutEach / aboutEachPrefix, skipping.", 
                          element->name->local_name);
    element->state=RAPTOR_STATE_SKIPPING;
    element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PRESERVED;
  }
  
  /* Right, now ready to enter the grammar */
  raptor_start_element_grammar(rdf_parser, element);

}


void
raptor_xml_end_element_handler(void *user_data, const XML_Char *name)
{
  raptor_parser* rdf_parser;
  raptor_xml_parser* rdf_xml_parser;
  raptor_element* element;
  raptor_qname *element_name;

  rdf_parser=(raptor_parser*)user_data;
  rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;

#ifdef RAPTOR_XML_EXPAT
#ifdef EXPAT_UTF8_BOM_CRASH
  rdf_parser->tokens_count++;
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
  if(!raptor_qname_equal(element->name, element_name)) {
    /* Hmm, unexpected name - FIXME, should do something! */
    raptor_parser_warning(rdf_parser, 
                          "Element %s ended, expected end of element %s",
                          name, element->name->local_name);
    return;
  }

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
    if(element->state != RAPTOR_STATE_MEMBER &&
       element->state != RAPTOR_STATE_PARSETYPE_RESOURCE)
      element->parent->child_state=element->state;
  }
  

  raptor_free_element(element);

  rdf_xml_parser->depth--;

#ifdef RAPTOR_DEBUG
  fputc('\n', stderr);
#endif

}



/* cdata (and ignorable whitespace for libxml). 
 * s is not 0 terminated for expat, is for libxml - grrrr.
 */
void
raptor_xml_cdata_handler(void *user_data, const XML_Char *s, int len)
{
  raptor_parser* rdf_parser;
  raptor_xml_parser* rdf_xml_parser;
  raptor_element* element;
  raptor_state state;
  char *buffer;
  char *ptr;
  int all_whitespace=1;
  int i;

  rdf_parser=(raptor_parser*)user_data;
  rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;

#ifdef RAPTOR_XML_EXPAT
#ifdef EXPAT_UTF8_BOM_CRASH
  rdf_parser->tokens_count++;
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
  
#ifdef RAPTOR_DEBUG
  fputc('\n', stderr);
#endif

  raptor_update_document_locator(rdf_parser);

  /* cdata never changes the parser state 
   * and the containing element state always determines what to do.
   * Use the child_state first if there is one, since that applies
   */
  state=element->child_state;
  RAPTOR_DEBUG3(raptor_xml_cdata_handler, "Working in state %d - %s\n", state,
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
    raptor_parser_warning(rdf_parser, "Found cdata before RDF element.");
  }


  if(element->child_content_type == RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTIES) {
    /* If found non-whitespace content, move to literal content */
    if(!all_whitespace)
      element->child_content_type = RAPTOR_ELEMENT_CONTENT_TYPE_LITERAL; 
  }


  if(!rdf_content_type_info[element->child_content_type].whitespace_significant) {

    /* Whitespace is ignored except for literal or preserved content types */
    if(all_whitespace) {
      RAPTOR_DEBUG2(raptor_xml_cdata_handler, "Ignoring whitespace cdata inside element %s\n", element->name->local_name);
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
  strncpy(ptr, s, len);
  ptr += len;
  *ptr = '\0';

  RAPTOR_DEBUG3(raptor_xml_cdata_handler, 
                "Content cdata now: '%s' (%d bytes)\n", 
                buffer, element->content_cdata_length);

  RAPTOR_DEBUG3(raptor_xml_cdata_handler, 
                "Ending in state %d - %s\n",
                state, raptor_state_as_string(state));

}


#ifdef HAVE_XML_SetNamespaceDeclHandler
static void
raptor_start_namespace_decl_handler(void *user_data,
                                    const XML_Char *prefix, const XML_Char *uri)
{
  RAPTOR_DEBUG3(raptor_start_namespace_decl_handler,
                "saw namespace %s URI %s\n", prefix, uri);
}


static void
raptor_end_namespace_decl_handler(void *user_data, const XML_Char *prefix)
{
  RAPTOR_DEBUG2(raptor_start_namespace_decl_handler,
                "saw end namespace prefix %s\n", prefix);
}
#endif


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

  rdf_xml_parser->xp=xp;
#endif

  return 0;
}


static int
raptor_xml_parse_start(raptor_parser* rdf_parser, raptor_uri *uri)
{
#ifdef RAPTOR_XML_EXPAT
  XML_Parser xp;
#endif
  raptor_xml_parser* rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;

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
}


/* Internal - handle XML parser errors and pass them on to app */
static void
raptor_xml_parse_handle_errors(raptor_parser* rdf_parser) {
#ifdef RAPTOR_XML_EXPAT
  XML_Parser xp=((raptor_xml_parser*)rdf_parser->context)->xp;
  int xe=XML_GetErrorCode(xp);
  raptor_locator *locator=&rdf_parser->locator;

#ifdef EXPAT_UTF8_BOM_CRASH
  if(rdf_parser->tokens_count) {
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
raptor_xml_parse_chunk_(raptor_parser* rdf_parser, const char *buffer,
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
    xc = xmlCreatePushParserCtxt(&rdf_xml_parser->sax, rdf_parser,
                                 buffer, len, 
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
      len= -1;
    else
      return 0;
  }
#endif

  if(len <= 0) {
#ifdef RAPTOR_XML_EXPAT
    rc=XML_Parse(xp, buffer, 0, 1);
    if(!rc) /* expat: 0 is failure */
      return 1;
#endif
#ifdef RAPTOR_XML_LIBXML
    xmlParseChunk(xc, buffer, 0, 1);
#endif
    return 0;
  }


#ifdef RAPTOR_XML_EXPAT
  rc=XML_Parse(xp, buffer, len, is_end);
  if(!rc) /* expat: 0 is failure */
    return 1;
  if(is_end)
    return 0;
#endif

#ifdef RAPTOR_XML_LIBXML
  /* Work around some libxml versions that fail to work
   * if the buffer size is larger than the entire file
   * and thus the entire parsing is done in one operation.
   */
  if(rdf_xml_parser->first_read && is_end) {
    /* parse all but the last character */
    rc=xmlParseChunk(xc, buffer, len-1, 0);
    if(rc)
      return 1;
    /* last character */
    rc=xmlParseChunk(xc, buffer + (len-1), 1, 0);
    if(rc)
      return 1;
    /* end */
    len= -1; /* pretend to be EOF */
    xmlParseChunk(xc, buffer, 0, 1);
    return 0;
  }

  rdf_xml_parser->first_read=0;
    
  rc=xmlParseChunk(xc, buffer, len, is_end);
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
raptor_xml_parse_chunk(raptor_parser* rdf_parser, const char *buffer,
                       size_t len, int is_end) 
{
  int rc=raptor_xml_parse_chunk_(rdf_parser, buffer, len, is_end);
  if(rc)    
    raptor_xml_parse_handle_errors(rdf_parser);
  return rc;
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
                          raptor_uri *literal_datatype,
                          raptor_uri *reified)
{
  raptor_statement *statement=&rdf_parser->statement;
  const char *language=NULL;
  static const char empty_literal[1]="";
  raptor_xml_parser *rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;

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

  if(!reified)
    return;

  /* generate reified statements */
  statement->subject_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
  statement->predicate_type=RAPTOR_IDENTIFIER_TYPE_PREDICATE;

  statement->object_literal_language=NULL;

  statement->subject=reified;
  statement->object_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;

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
}



/**
 * raptor_element_has_property_attributes: Return true if the element has at least one property attribute
 * @rdf_parser: Raptor parser object
 * @element: element with the property attributes 
 * 
 **/
static int
raptor_element_has_property_attributes(raptor_parser *rdf_parser, 
                                       raptor_element *element) 
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
    const char *name=attr->local_name;
    const char *value = attr->value;
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
        raptor_parser_warning(rdf_parser, "Found unknown RDF M&S attribute %s.", 
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
                                  
                                  NULL);
        handled=1;
      }
      
    } /* end is RDF M&S property */


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
                                
                                resource_element->reified.uri);

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

    property_uri=raptor_new_uri_for_rdf_concept(rdf_attr_info[i].name);
    
    object_uri=object_is_literal ? (raptor_uri*)value : raptor_new_uri_relative_to_base(raptor_inscope_base_uri(rdf_parser), value);
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

                              resource_element->reified.uri);
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
  const char *el_name=element->name->local_name;
  int element_in_rdf_ns=(element->name->nspace && 
                         element->name->nspace->is_rdf_ms);
  raptor_xml_parser *rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;


  state=element->state;
  RAPTOR_DEBUG3(raptor_start_element_grammar, "Starting in state %d - %s\n",
                state, raptor_state_as_string(state));

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
            element->child_state=RAPTOR_STATE_OBJ;
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
        state=RAPTOR_STATE_OBJ;
        element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_NODES;
        break;


      case RAPTOR_STATE_OBJ:
        /* Handling either 6.2 (obj) or 6.17 (value) (inside
         * rdf:RDF, propertyElt) and expecting
         * description (6.3) | sequence (6.25) | bag (6.26) | alternative (6.27)
         * [|other container (not M&S)]
         *
         * Everything goes to typedNode (6.13) now
         */

        state=RAPTOR_STATE_TYPED_NODE;

        element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTIES;

        break;


      /* No need for 6.2 - already chose 6.3, 6.25, 6.26 or 6.27 */


      case RAPTOR_STATE_DESCRIPTION:
      case RAPTOR_STATE_TYPED_NODE:
      case RAPTOR_STATE_PARSETYPE_RESOURCE:
      case RAPTOR_STATE_PARSETYPE_COLLECTION:
        /* Handling 6.3 (description), 6.13 (typedNode), contents
         * of a property (propertyElt or member) with parseType="Resource"
         * and rdf:Seq, rdf:Bag, rdf:Alt which are just typedNodes now
         *
         * Expect here from merge of production 6.3, 6.13, 6.25-6.27
         * <typeName (ID|about)? bagID? propAttr* />
         *
         * Only create a bag if bagID given
         */

        if(element->content_type !=RAPTOR_ELEMENT_CONTENT_TYPE_COLLECTION &&
           element->content_type !=RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION && 
           element->parent && 
           (element->parent->state == RAPTOR_STATE_PROPERTYELT ||
            element->parent->state == RAPTOR_STATE_MEMBER) &&
           element->parent->content_element_seen > 1) {
          raptor_update_document_locator(rdf_parser);
          raptor_parser_error(rdf_parser, "The enclosing property already has an object");
          state=RAPTOR_STATE_SKIPPING;
          element->child_state=RAPTOR_STATE_SKIPPING;
          finished=1;
          break;
        }

        if(state == RAPTOR_STATE_TYPED_NODE || 
           state == RAPTOR_STATE_DESCRIPTION || 
           state == RAPTOR_STATE_PARSETYPE_COLLECTION) {
          if(element_in_rdf_ns &&
             raptor_uri_equals(element->name->uri, RAPTOR_RDF_Description_URI(rdf_xml_parser)))
            state=RAPTOR_STATE_DESCRIPTION;
          else
            state=RAPTOR_STATE_TYPED_NODE;
        }
        

        if((element->rdf_attr[RDF_ATTR_ID]!=NULL) +
           (element->rdf_attr[RDF_ATTR_about]!=NULL) +
           (element->rdf_attr[RDF_ATTR_nodeID]!=NULL)>1) {
          raptor_update_document_locator(rdf_parser);
          raptor_parser_warning(rdf_parser, "Found multiple attribute of rdf:ID, rdf:about and rdf:nodeID attrs on element %s - expected only one.", el_name);
        }

        if(element->rdf_attr[RDF_ATTR_ID]) {
          element->subject.id=(char*)element->rdf_attr[RDF_ATTR_ID];
          element->rdf_attr[RDF_ATTR_ID]=NULL;
          element->subject.uri=raptor_new_uri_from_id(raptor_inscope_base_uri(rdf_parser), element->subject.id);
          element->subject.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          element->subject.uri_source=RAPTOR_URI_SOURCE_ID;
        } else if (element->rdf_attr[RDF_ATTR_about]) {
          element->subject.uri=raptor_new_uri_relative_to_base(raptor_inscope_base_uri(rdf_parser), element->rdf_attr[RDF_ATTR_about]);
          element->subject.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          element->subject.uri_source=RAPTOR_URI_SOURCE_URI;
        } else if (element->rdf_attr[RDF_ATTR_nodeID]) {
          element->subject.id=element->rdf_attr[RDF_ATTR_nodeID];
          element->rdf_attr[RDF_ATTR_nodeID]=NULL;
          element->subject.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
          element->subject.uri_source=RAPTOR_URI_SOURCE_BLANK_ID;
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
          element->bag.id=(char*)element->rdf_attr[RDF_ATTR_bagID];
          element->rdf_attr[RDF_ATTR_bagID]=NULL;
          element->bag.uri=raptor_new_uri_from_id(raptor_inscope_base_uri(rdf_parser), element->bag.id);
          element->bag.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          element->bag.uri_source=RAPTOR_URI_SOURCE_GENERATED;
        }


        if(element->parent) {

          /* In a rdf:parseType="Collection" the resources are appended
           * to the list at the genid element->parent->tail_id
           */
          if (element->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_COLLECTION ||
              element->content_type == RAPTOR_ELEMENT_CONTENT_TYPE_DAML_COLLECTION) {
            const char * idList = raptor_generate_id(rdf_parser, 0);
            
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

                                      NULL);

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

                                      NULL);
            
            /* If there is no rdf:parseType="Collection" */
            if (!element->parent->tail_id) {
              int len;
              char *new_id;
              
              /* Free any existing object URI still around
               * I suspect this can never happen.
               */
              if(element->parent->object.uri) {
                abort();
                raptor_free_uri(element->parent->object.uri);
              }

              len=strlen(idList);
              new_id=(char*)RAPTOR_MALLOC(cstring, len+1);
              if(!len) {
                if(new_id)
                  RAPTOR_FREE(cstring, new_id);
                return;
              }
              strncpy(new_id, idList, len+1);

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
        

        /* If this is a typed node, generate the rdf:type statement
         * from this node
         */
        if(state == RAPTOR_STATE_TYPED_NODE)
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

                                    element->reified.uri);

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
          fmt_buffer=raptor_format_element(element, &fmt_length, 0);
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

              RAPTOR_DEBUG3(raptor_start_element_grammar,
                            "content cdata appended, now: '%s' (%d bytes)\n", 
                            element->content_cdata,
                            element->content_cdata_length);

            } else {
              /* Copy - is empty */
              element->content_cdata       =fmt_buffer;
              element->content_cdata_length=fmt_length;

              RAPTOR_DEBUG3(raptor_start_element_grammar,
                            "content cdata copied, now: '%s' (%d bytes)\n", 
                            element->content_cdata,
                            element->content_cdata_length);

            }
          }

          element->child_state = RAPTOR_STATE_PARSETYPE_LITERAL;
          element->child_content_type = RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL;
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

        /* Handling rdf:li as a property, noting special processing */ 
        if(element_in_rdf_ns && 
           raptor_uri_equals(element->name->uri, RAPTOR_RDF_li_URI(rdf_xml_parser))) {
          state=RAPTOR_STATE_MEMBER;
        }


        /* M&S says: "The value of the ID attribute, if specified, is
         * the identifier for the resource that represents the
         * reification of the statement." */

        /* Later on it implies something different
         * FIXME 1 - reference to this
         * FIXME 2 - pick one of these interpretations
         */

        if(element->rdf_attr[RDF_ATTR_ID]) {
          element->reified.id=element->rdf_attr[RDF_ATTR_ID];
          element->rdf_attr[RDF_ATTR_ID]=NULL;
          element->reified.uri=raptor_new_uri_from_id(raptor_inscope_base_uri(rdf_parser), element->reified.id);
          element->reified.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          element->reified.uri_source=RAPTOR_URI_SOURCE_GENERATED;
        }
        
        if (element->rdf_attr[RDF_ATTR_datatype]) {
          element->object_literal_datatype=raptor_new_uri_relative_to_base(raptor_inscope_base_uri(rdf_parser), element->rdf_attr[RDF_ATTR_datatype]);
          element->rdf_attr[RDF_ATTR_datatype]=NULL; 
        }

        element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTY_CONTENT;

        if (element->rdf_attr[RDF_ATTR_parseType]) {
          const char *parse_type=element->rdf_attr[RDF_ATTR_parseType];

          if(!raptor_strcasecmp(parse_type, "literal")) {
            element->child_state=RAPTOR_STATE_PARSETYPE_LITERAL;
            element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL;
            element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL;
          } else if (!raptor_strcasecmp(parse_type, "resource")) {
            state=RAPTOR_STATE_PARSETYPE_RESOURCE;
            element->child_state=RAPTOR_STATE_PROPERTYELT;
            element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_PROPERTIES;

            /* create a node for the subject of the contained properties */
            element->subject.id=raptor_generate_id(rdf_parser, 0);
            element->subject.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
            element->subject.uri_source=RAPTOR_URI_SOURCE_GENERATED;
          } else if(!strcmp(parse_type, "Collection")) {
            /* An rdf:parseType="Collection" appears as a single node */
            element->content_type=RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
            element->child_state=RAPTOR_STATE_PARSETYPE_COLLECTION;
            element->child_content_type=RAPTOR_ELEMENT_CONTENT_TYPE_COLLECTION;
          } else {
            if(rdf_parser->feature_allow_other_parseTypes) {
              if(!raptor_strcasecmp(parse_type, "daml:collection")) {
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

          /* Can only be last case */

          /* rdf:resource attribute checked at element close time */

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
            element->child_state=RAPTOR_STATE_OBJ;
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

  RAPTOR_DEBUG3(raptor_start_element_grammar, 
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
  raptor_xml_parser *rdf_xml_parser=(raptor_xml_parser*)rdf_parser->context;


  state=element->state;
  RAPTOR_DEBUG3(raptor_end_element_grammar, "Starting in state %d - %s\n",
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
        if(state == RAPTOR_STATE_TYPED_NODE && 
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

                                    NULL);
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

                                      element->reified.uri);
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

                                      element->reified.uri);
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
          fmt_buffer=raptor_format_element(element, &fmt_length,1);
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
        
        RAPTOR_DEBUG3(raptor_end_element_grammar,
                      "content cdata now: '%s' (%d bytes)\n", 
                       element->content_cdata, element->content_cdata_length);
         
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

          RAPTOR_DEBUG3(raptor_end_element_grammar,
                        "content cdata appended to parent, now: '%s' (%d bytes)\n", 
                        element->parent->content_cdata,
                        element->parent->content_cdata_length);

        } else {
          /* Copy - parent is empty */
          element->parent->content_cdata       =element->content_cdata;
          element->parent->content_cdata_length=element->content_cdata_length+1;
          RAPTOR_DEBUG3(raptor_end_element_grammar,
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
                                                    element->rdf_attr[RDF_ATTR_resource]);
                element->object.type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
                element->object.uri_source=RAPTOR_URI_SOURCE_URI;
                element->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
              } else if(element->rdf_attr[RDF_ATTR_nodeID]) {
                element->object.id=element->rdf_attr[RDF_ATTR_nodeID];
                element->rdf_attr[RDF_ATTR_nodeID]=NULL;
                element->object.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
                element->object.uri_source=RAPTOR_URI_SOURCE_BLANK_ID;
                element->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
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
              /* If there is empty literal content with properties
               * generate a node to hang properties off 
               */
              if(element->object.type == RAPTOR_IDENTIFIER_TYPE_LITERAL &&
                 raptor_element_has_property_attributes(rdf_parser, element) &&
                 !element->object.uri) {
                element->object.id=raptor_generate_id(rdf_parser, 0);
                element->object.type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
                element->object.uri_source=RAPTOR_URI_SOURCE_GENERATED;
                element->content_type = RAPTOR_ELEMENT_CONTENT_TYPE_RESOURCE;
              }
              
              raptor_process_property_attributes(rdf_parser, element, 
                                                 element, &element->object);
            }
            

            if(state == RAPTOR_STATE_MEMBER) {
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

                                        element->reified.uri);
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

                                        element->reified.uri);
            }
          
            break;

        case RAPTOR_ELEMENT_CONTENT_TYPE_PRESERVED:
        case RAPTOR_ELEMENT_CONTENT_TYPE_XML_LITERAL:
          /* FIXME: undetermined yet if XML literals have a datatype URI 
           * If they do gain it, change FIXME XML1 and FIXME XML2 below
           */
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
                                        NULL, /* FIXME XML1 */

                                        element->reified.uri);
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
                                        NULL, /* FIXME XML2 */

                                        element->reified.uri);
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

  RAPTOR_DEBUG3(raptor_end_element_grammar, 
                "Ending in state %d - %s\n",
                state, raptor_state_as_string(state));

}


/**
 * raptor_inscope_xml_language - Return the in-scope xml:lang
 * @rdf_parser: Raptor parser object
 * 
 * Looks for the innermost xml:lang on an element
 * 
 * Return value: The xml:lang value or NULL if none is in scope. 
 **/
const char*
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
  raptor_parser_register_factory("rdfxml", &raptor_xml_parser_register_factory);
}
