/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_serialize_rdfxmla.c - RDF/XML with abbreviations serializer
 *
 * Copyright (C) 2004-2006, David Beckett http://purl.org/net/dajobe/
 * Copyright (C) 2004-2005, University of Bristol, UK http://www.bristol.ac.uk/
 * Copyright (C) 2005, Steve Shepard steveshep@gmail.com
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
 */

#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#ifdef WIN32
#include <win32_raptor_config.h>
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
#include "raptor.h"
#include "raptor_internal.h"


/*
 * FIXME Duplicate code
 *
 * Parts of this is taken from redland librdf_node.h and librdf_node.c
 */

#define MAX_ASCII_INT_SIZE 13

typedef struct {
  int ref_count;                /* count of references */
  raptor_identifier_type type;  /* node type */
  union {

    struct {
      raptor_uri *uri;
    } resource;

    struct {
      unsigned char *string;
      raptor_uri *datatype;
      unsigned char *language;
    } literal;

    struct {
      int ordinal;
    } ordinal;

    struct {
      unsigned char *string;
    } blank;
    
  } value;
  
} raptor_node;


typedef struct {
  raptor_node *node;             /* node representing the subject of
                                  * this resource */
  raptor_node *node_type;        /* the rdf:type of this resource */
  raptor_sequence *properties;   /* list of properties
                                  * (predicate/object pair) of this
                                  * subject */
  raptor_sequence *list_items;   /* list of container elements if
                                  * is rdf container */
} raptor_subject;

  
/*
 * Raptor rdfxml-abbrev serializer object
 */
typedef struct {
  raptor_namespace_stack *nstack;       /* Namespace stack */
  raptor_namespace *rdf_nspace;         /* the rdf: namespace */
  raptor_xml_element* rdf_RDF_element;  /* the rdf:RDF element */
  raptor_xml_writer *xml_writer;        /* where the xml is being written */
  raptor_sequence *namespaces;          /* User declared namespaces */
  raptor_sequence *subjects;            /* subject items */
  raptor_sequence *blanks;              /* blank subject items */
  raptor_sequence *nodes;               /* nodes */
  raptor_node *rdf_type;                /* rdf:type uri */

  /* URI of rdf:XMLLiteral */
  raptor_uri* rdf_xml_literal_uri;

  /* non-zero if is Adobe XMP abbreviated form */
  int is_xmp;

  /* non zero if rdf:RDF has been written (and thus no new namespaces
   * can be declared).
   */
  int written_header;
} raptor_rdfxmla_context;


/* prototypes for functions */
static unsigned char *raptor_unique_id(unsigned char *base);

static raptor_qname *raptor_new_qname_from_resource(raptor_serializer *serializer,
                                                    raptor_node *node);

static int raptor_rdfxmla_emit_resource(raptor_serializer *serializer,
                                        raptor_xml_element *element,
                                        raptor_node *node);

static int raptor_rdfxmla_emit_literal(raptor_serializer *serializer,
                                       raptor_xml_element *element,
                                       raptor_node *node);
static int raptor_rdfxmla_emit_xml_literal(raptor_serializer *serializer,
                                           raptor_xml_element *element,
                                           raptor_node *node);
static int raptor_rdfxmla_emit_blank(raptor_serializer *serializer,
                                     raptor_xml_element *element,
                                     raptor_node *node);
static int raptor_rdfxmla_emit_subject_list_items(raptor_serializer* serializer,
                                                  raptor_subject *subject);
static int raptor_rdfxmla_emit_subject_properties(raptor_serializer *serializer,
                                                  raptor_subject *subject);
static int raptor_rdfxmla_emit_subject(raptor_serializer *serializer,
                                       raptor_subject *subject);
static int raptor_rdfxmla_emit(raptor_serializer *serializer);

static raptor_node *raptor_new_node(raptor_identifier_type node_type,
                                    const void *node_data,
                                    raptor_uri *datatype,
                                    const unsigned char *language);
static void raptor_free_node(raptor_node *node);
static int raptor_node_equals(raptor_node *node1, raptor_node *node2);
static int raptor_node_matches(raptor_node *node,
                               raptor_identifier_type node_type,
                               const void *node_data,
                               raptor_uri *datatype,
                               const unsigned char *language);

static raptor_subject *raptor_new_subject(raptor_node *node);
static void raptor_free_subject(raptor_subject *subject);
static int raptor_subject_add_property(raptor_subject *subject,
                                       raptor_node *predicate,
                                       raptor_node *object);
static int raptor_subject_add_list_element(raptor_subject *subject, int ordinal,
                                           raptor_node *object);

static raptor_node *raptor_rdfxmla_lookup_node(raptor_rdfxmla_context* context,
                                               raptor_identifier_type node_type,
                                               const void *node_value,
                                               raptor_uri *datatype,
                                               const unsigned char *language);

static raptor_subject *raptor_rdfxmla_find_subject(raptor_sequence *sequence,
                                                   raptor_identifier_type node_type,
                                                   const void *node_data, int *idx);


static raptor_subject *raptor_rdfxmla_lookup_subject(raptor_rdfxmla_context* context,
                                                     raptor_identifier_type node_type,
                                                     const void *node_data);

static int raptor_rdfxmla_serialize_init(raptor_serializer* serializer,
                                         const char *name);
static void raptor_rdfxmla_serialize_terminate(raptor_serializer* serializer);
static int raptor_rdfxmla_serialize_declare_namespace(raptor_serializer* serializer, 
                                                      raptor_uri *uri,
                                                      const unsigned char *prefix);
static int raptor_rdfxmla_serialize_start(raptor_serializer* serializer);
static int raptor_rdfxmla_serialize_statement(raptor_serializer* serializer, 
                                              const raptor_statement *statement);

static int raptor_rdfxmla_serialize_end(raptor_serializer* serializer);
static void raptor_rdfxmla_serialize_finish_factory(raptor_serializer_factory* factory);

/* static vars */
static int raptor_namespace_count = 0;


/* helper functions */

static unsigned char *
raptor_unique_id(unsigned char *base) 
{
  /* Raptor doesn't check that blank IDs it generates are unique
   * from any specified by rdf:nodeID. Here, we need to emit IDs that
   * are different from the ones the parser generates so that there
   * is no collision. For now, just prefix a '_' to the parser
   * generated name.
   */
    
  const char *prefix = "_";
  int prefix_len = strlen(prefix);
  int base_len = strlen((const char*)base);
  int len = prefix_len + base_len + 1;
  unsigned char *unique_id;

  unique_id= (unsigned char *)RAPTOR_MALLOC(cstring, len);
  strncpy((char*)unique_id, prefix, prefix_len);
  strncpy((char*)unique_id+prefix_len, (char *)base, base_len);
  unique_id[len-1]='\0';
    
  return unique_id;
}


static raptor_qname *
raptor_new_qname_from_resource(raptor_serializer* serializer,
                               raptor_node *node)
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;
  unsigned char* name=NULL;  /* where to split predicate name */
  size_t name_len=1;
  unsigned char *uri_string;
  size_t uri_len;
  unsigned char c;
  unsigned char *p;
  raptor_uri *ns_uri;
  raptor_namespace *ns;
  raptor_qname *qname;
  
  if(node->type != RAPTOR_IDENTIFIER_TYPE_RESOURCE) {
    RAPTOR_FATAL1("Node must be a resource\n");
    return NULL;
  }

  qname=raptor_namespaces_qname_from_uri(context->nstack, 
                                         node->value.resource.uri, 10);
  if(qname)
    return qname;
  
  uri_string = raptor_uri_as_counted_string(node->value.resource.uri, &uri_len);

  p= uri_string;
  name_len=uri_len;
  while(name_len >0) {
    if(raptor_xml_name_check(p, name_len, 10)) {
      name=p;
      break;
    }
    p++; name_len--;
  }
      
  if(!name || (name == uri_string))
    return NULL;

  c=*name; *name='\0';
  ns_uri=raptor_new_uri(uri_string);
  *name=c;
  
  ns = raptor_namespaces_find_namespace_by_uri(context->nstack, ns_uri);
  if(!ns) {
    /* The namespace was not declared, so create one */
    unsigned char prefix[2 + MAX_ASCII_INT_SIZE + 1];
    sprintf((char *)prefix, "ns%d", raptor_namespace_count++);

    ns = raptor_new_namespace_from_uri(context->nstack, prefix, ns_uri, 0);

    /* We'll most likely need this namespace again. Push it on our
     * stack.  It will be deleted in
     * raptor_rdfxmla_serialize_terminate
     */
    raptor_sequence_push(context->namespaces, ns);
  }

  qname = raptor_new_qname_from_namespace_local_name(ns, name,  NULL);
  
  raptor_free_uri(ns_uri);

  return qname;
}


static int
raptor_rdfxmla_emit_resource(raptor_serializer *serializer,
                             raptor_xml_element *element, raptor_node *node) 
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;
  raptor_xml_writer *xml_writer = context->xml_writer;
  raptor_qname **attrs;
  unsigned char *attr_name;
  unsigned char *attr_value;
  
  if(node->type != RAPTOR_IDENTIFIER_TYPE_RESOURCE)
    return 1;

  attrs = (raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname *));
  if(!attrs)
    return 1;
    
  attr_name = (unsigned char *)"resource";

  if(serializer->feature_relative_uris)
    /* newly allocated string */
    attr_value = raptor_uri_to_relative_uri_string(serializer->base_uri,
                                                   node->value.resource.uri);
  else
    attr_value = raptor_uri_as_string(node->value.resource.uri);

  attrs[0] = raptor_new_qname_from_namespace_local_name(context->rdf_nspace,
                                                        attr_name, 
                                                        attr_value);
      
  if(serializer->feature_relative_uris)
    RAPTOR_FREE(cstring, attr_value);

  raptor_xml_element_set_attributes(element, attrs, 1);

  raptor_xml_writer_start_element(xml_writer, element);
  raptor_xml_writer_end_element(context->xml_writer, element);

  return 0;
  
}


static int
raptor_rdfxmla_emit_literal(raptor_serializer *serializer,
                            raptor_xml_element *element, raptor_node *node)
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;
  raptor_xml_writer *xml_writer = context->xml_writer;
  raptor_qname **attrs;
  int attrs_count;
  
  if(node->type != RAPTOR_IDENTIFIER_TYPE_LITERAL)
    return 1;
  
  if(node->value.literal.language || node->value.literal.datatype) {
          
    attrs_count = 0;
    attrs = (raptor_qname **)RAPTOR_CALLOC(qnamearray,2,sizeof(raptor_qname *));
    if(!attrs)
      return 1;

    if(node->value.literal.language) {
      attrs[attrs_count++] = raptor_new_qname(context->nstack,
                                              (unsigned char*)"xml:lang",
                                              (unsigned char*)node->value.literal.language,
                                              raptor_serializer_simple_error,
                                              serializer);
    }

    if(node->value.literal.datatype) {
      unsigned char *datatype_value;
      datatype_value = raptor_uri_as_string(node->value.literal.datatype);
      attrs[attrs_count++] = raptor_new_qname_from_namespace_local_name(context->rdf_nspace, (const unsigned char*)"datatype",
                                                                        datatype_value);
      /* SJS Note: raptor_default_uri_as_string simply returns a
       * pointer to the string. Hope this is also true of alternate
       * uri implementations. */
      /* RAPTOR_FREE(cstring, datatype_value); */
          
    }

    raptor_xml_element_set_attributes(element, attrs, attrs_count);
        
  }
      
  raptor_xml_writer_start_element(xml_writer, element);
  raptor_xml_writer_cdata(xml_writer, node->value.literal.string);
  raptor_xml_writer_end_element(xml_writer, element);

  return 0;
}

static int
raptor_rdfxmla_emit_xml_literal(raptor_serializer *serializer,
                                raptor_xml_element *element, raptor_node *node) 
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;
  raptor_xml_writer *xml_writer = context->xml_writer;
  raptor_qname **attrs;
  
  if(node->type != RAPTOR_IDENTIFIER_TYPE_XML_LITERAL)
    return 1;
  
  attrs = (raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname *));
  if(!attrs)
    return 1;
  
  attrs[0] = raptor_new_qname_from_namespace_local_name(context->rdf_nspace,
                                                        (const unsigned char*)"parseType",
                                                        (const unsigned char*)"Literal");
  raptor_xml_element_set_attributes(element, attrs, 1);
  raptor_xml_writer_start_element(xml_writer, element);
  raptor_xml_writer_raw(xml_writer, node->value.literal.string);
  raptor_xml_writer_end_element(xml_writer, element);

  return 0;
  
}


static int
raptor_rdfxmla_emit_blank(raptor_serializer *serializer,
                          raptor_xml_element *element, raptor_node *node) 
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;

  if(node->type != RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
    return 1;
  
  if(node->ref_count == 3) {
    /* If ref_count == 3, we know that this subject is only one
     * that refers to the blank resource. It holds one reference
     * because it is the subject of a statement, one because it
     * is in the nodes array and one because it is in the
     * properties sequence. */
    int idx;
    raptor_subject *blank;

    raptor_xml_writer_start_element(context->xml_writer, element);

    blank = raptor_rdfxmla_find_subject(context->blanks, node->type,
                                        node->value.blank.string, &idx);
          
    if(blank) {
      raptor_rdfxmla_emit_subject(serializer, blank);
      raptor_sequence_set_at(context->blanks, idx, NULL);
    }
          
  } else {
    unsigned char *attr_name = (unsigned char*)"nodeID";
    unsigned char *attr_value = raptor_unique_id(node->value.blank.string);
    raptor_qname **attrs;

    attrs = (raptor_qname **)RAPTOR_CALLOC(qnamearray,1,sizeof(raptor_qname *));
    if(!attrs)
      return 1;

    attrs[0] = raptor_new_qname_from_namespace_local_name(context->rdf_nspace,
                                                          attr_name,
                                                          attr_value);

    RAPTOR_FREE(cstring, attr_value);
    
    raptor_xml_element_set_attributes(element, attrs, 1);
    raptor_xml_writer_start_element(context->xml_writer, element);

  }

  raptor_xml_writer_end_element(context->xml_writer, element);

  return 0;

}


static int
raptor_rdfxmla_emit_subject_list_items(raptor_serializer* serializer,
                                       raptor_subject *subject)
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;
  int rv = 0;
  int i=0;
  raptor_uri* base_uri=NULL;
  
  while (!rv && i < raptor_sequence_size(subject->list_items)) {

    raptor_node *object;
    raptor_qname *qname;
    raptor_xml_element *element;
    
    object = (raptor_node *)raptor_sequence_get_at(subject->list_items, i++);
    if(!object)
      continue;
    
    qname = raptor_new_qname_from_namespace_local_name(context->rdf_nspace,
                                                       (unsigned char *)"li",
                                                       NULL);
    
    if(serializer->base_uri)
      base_uri=raptor_uri_copy(serializer->base_uri);
    element = raptor_new_xml_element(qname, NULL, base_uri);

    switch (object->type) {
      
      case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
        rv = raptor_rdfxmla_emit_resource(serializer, element, object);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_LITERAL:
        rv = raptor_rdfxmla_emit_literal(serializer, element, object);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_XML_LITERAL:
        rv = raptor_rdfxmla_emit_xml_literal(serializer, element, object);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
        rv = raptor_rdfxmla_emit_blank(serializer, element, object);
        break;

      case RAPTOR_IDENTIFIER_TYPE_ORDINAL:
        /* ordinals should never appear as an object with current parsers */
      case RAPTOR_IDENTIFIER_TYPE_PREDICATE:
        /* predicates should never appear as an object */
      case RAPTOR_IDENTIFIER_TYPE_UNKNOWN:
      default:
        RAPTOR_FATAL1("Unsupported identifier type\n");
        break;

    }
    
    raptor_free_xml_element(element);

  }
  
  return rv;

}

static int
raptor_rdfxmla_emit_subject_properties(raptor_serializer* serializer,
                                       raptor_subject *subject)
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;
  int rv = 0;  
  int i;
  
  /* Emit any container props we've collected */
  if(raptor_sequence_size(subject->list_items) > 0)
    rv = raptor_rdfxmla_emit_subject_list_items(serializer, subject);

  i=0;
  while (!rv && i < raptor_sequence_size(subject->properties)) {
    raptor_uri *base_uri=NULL;
    raptor_node *predicate;
    raptor_node *object;
    raptor_qname *qname;
    raptor_xml_element *element;
    
    predicate = (raptor_node *)raptor_sequence_get_at(subject->properties, i++);
    object = (raptor_node *)raptor_sequence_get_at(subject->properties, i++);
    
    if(predicate->type == RAPTOR_IDENTIFIER_TYPE_ORDINAL) {
      /* we should only get here in rare cases -- usually when there
       * are multiple ordinals with the same value. */

      unsigned char uri_string[MAX_ASCII_INT_SIZE + 2];

      sprintf((char*)uri_string, "_%d", predicate->value.ordinal.ordinal);

      qname = raptor_new_qname_from_namespace_local_name(context->rdf_nspace,
                                                         uri_string, NULL);
      
    } else
      qname = raptor_new_qname_from_resource(serializer, predicate);
    
    if(serializer->base_uri)
      base_uri=raptor_uri_copy(serializer->base_uri);
    element = raptor_new_xml_element(qname, NULL, base_uri);

    switch (object->type) {
      
      case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
        rv = raptor_rdfxmla_emit_resource(serializer, element, object);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_LITERAL:
        rv = raptor_rdfxmla_emit_literal(serializer, element, object);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
        rv = raptor_rdfxmla_emit_blank(serializer, element, object);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_XML_LITERAL:
        rv = raptor_rdfxmla_emit_xml_literal(serializer, element, object);
        break;

      case RAPTOR_IDENTIFIER_TYPE_ORDINAL:
        /* ordinals should never appear as an object with current parsers */
      case RAPTOR_IDENTIFIER_TYPE_PREDICATE:
        /* predicates should never appear as an object */
      case RAPTOR_IDENTIFIER_TYPE_UNKNOWN:
      default:
        RAPTOR_FATAL1("Unsupported identifier type\n");
        break;
    }    

    raptor_free_xml_element(element);
    
  }
         
  return rv;
}


static int
raptor_rdfxmla_emit_subject(raptor_serializer *serializer,
                            raptor_subject *subject) 
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;  
  raptor_qname *qname = NULL;    
  raptor_xml_element *element;
  raptor_qname **attrs;
  unsigned char *attr_name;
  unsigned char *attr_value;
  raptor_uri *base_uri=NULL;
  
  if(subject->node_type) { /* if rdf:type was associated with this subject */
    qname = raptor_new_qname_from_resource(serializer, subject->node_type);
    
    if(!qname) {
      raptor_serializer_error(serializer,
                              "Cannot split URI %s into an XML qname",
                              raptor_uri_as_string(subject->node_type->value.resource.uri));
      return 1;
    }
    
  } else
    qname = raptor_new_qname_from_namespace_local_name(context->rdf_nspace,
                                                       (unsigned const char*)"Description",  NULL);
    

  if(serializer->base_uri)
    base_uri=raptor_uri_copy(serializer->base_uri);
  element = raptor_new_xml_element(qname, NULL, base_uri);
    
  attrs = (raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname *));
  if(!attrs)
    return 1;
    
  attr_name = NULL;
  attr_value = NULL;
    
  /* emit the subject node */
  if(subject->node->type == RAPTOR_IDENTIFIER_TYPE_RESOURCE) {
    attr_name = (unsigned char*)"about";
    if(context->is_xmp) {
      /* XML rdf:about value is always "" */
      attr_value = (unsigned char *)RAPTOR_CALLOC(string, 1, sizeof(unsigned char*));
    } else if(serializer->feature_relative_uris)
      attr_value = raptor_uri_to_relative_uri_string(serializer->base_uri,
                                                     subject->node->value.resource.uri);
    else
      attr_value = raptor_uri_to_string(subject->node->value.resource.uri);
    
  } else if(subject->node->type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
    if(subject->node->ref_count != 2 &&
        subject->node->ref_count != 3) {
      /* If ref_count == 2, we know that this is a top-level
       * node. There is one reference because it is the subject of a
       * statement and one because it is in the nodes array. If
       * ref_count == 3, we know it is a "private" node. The
       * additional ref_count is due to the fact that it is the object
       * of a statement.*/
      attr_name = (unsigned char*)"nodeID";
      attr_value = raptor_unique_id(subject->node->value.blank.string);
    }
  } else if(subject->node->type == RAPTOR_IDENTIFIER_TYPE_ORDINAL) {
    attr_name = (unsigned char*)"about";
    attr_value = (unsigned char *)RAPTOR_MALLOC(string,
                                                raptor_rdf_namespace_uri_len + MAX_ASCII_INT_SIZE + 2);
    sprintf((char*)attr_value, "%s_%d", raptor_rdf_namespace_uri,
            subject->node->value.ordinal.ordinal);
  } 
    
  if(attr_name) {
    attrs[0] = raptor_new_qname_from_namespace_local_name(context->rdf_nspace,
                                                          attr_name,
                                                          attr_value);
    RAPTOR_FREE(cstring, attr_value);
    
    /* Note: if we were willing to track the in-scope rdf:lang, we
     * could do the "2.5 Property Attributes" abbreviation here */
    raptor_xml_element_set_attributes(element, attrs, 1);
  } else {
    RAPTOR_FREE(qnamearray, attrs);
  }
    
  raptor_xml_writer_start_element(context->xml_writer, element);
    
  raptor_rdfxmla_emit_subject_properties(serializer, subject);    
    
  raptor_xml_writer_end_element(context->xml_writer, element);
    
  raptor_free_xml_element(element);
    
  return 0;
}


static int
raptor_rdfxmla_emit(raptor_serializer *serializer)
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;
  raptor_subject *subject;
  raptor_subject *blank;
  int i;
  
  for(i=0; i < raptor_sequence_size(context->subjects); i++) {
  
    subject = (raptor_subject *)raptor_sequence_get_at(context->subjects, i);
  
    if(subject) {
      raptor_rdfxmla_emit_subject(serializer, subject);
      /*raptor_sequence_set_at(context->subjects, i, NULL); */
    }
  
  }
    
  /* Emit any remaining anonymous nodes */
  for(i=0; i < raptor_sequence_size(context->blanks); i++) {
    blank = (raptor_subject *)raptor_sequence_get_at(context->blanks, i);
    if(blank)
      raptor_rdfxmla_emit_subject(serializer, blank);
  }
    
  return 0;
    
}


/*
 * raptor_node implementation
 *
 **/

static raptor_node *
raptor_new_node(raptor_identifier_type node_type, const void *node_data,
                raptor_uri *datatype, const unsigned char *language)
{
  unsigned char *string;
  raptor_node* node;
  
  if(node_type == RAPTOR_IDENTIFIER_TYPE_UNKNOWN)
    return 0;

  node = (raptor_node *)RAPTOR_CALLOC(raptor_node, 1, sizeof(raptor_node));

  if(node) {

    node->type = node_type;
    
    switch (node_type) {
        case RAPTOR_IDENTIFIER_TYPE_PREDICATE:
          node->type = RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          /* intentional fall through */
        case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
          node->value.resource.uri = raptor_uri_copy((raptor_uri*)node_data);
          break;
          
        case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
          string=(unsigned char*)RAPTOR_MALLOC(blank,strlen((char*)node_data)+1);
          strcpy((char*)string, (const char*) node_data);
          node->value.blank.string = string;
          break;
          
        case RAPTOR_IDENTIFIER_TYPE_ORDINAL:
          node->value.ordinal.ordinal = *(int *)node_data;
          break;
          
        case RAPTOR_IDENTIFIER_TYPE_LITERAL:
        case RAPTOR_IDENTIFIER_TYPE_XML_LITERAL:
          string = (unsigned char*)RAPTOR_MALLOC(literal,
                                                 strlen((char*)node_data)+1);
          strcpy((char*)string, (const char*)node_data);
          node->value.literal.string = string;

          if(datatype) {
            node->value.literal.datatype = raptor_uri_copy(datatype);
          }

          if(language) {
            unsigned char *lang;
            lang =(unsigned char*)RAPTOR_MALLOC(language,
                                                strlen((const char*)language)+1);
            strcpy((char*)lang, (const char*)language);
            node->value.literal.language = lang;
          }
          break;
          
        case RAPTOR_IDENTIFIER_TYPE_UNKNOWN: 
        default:
          RAPTOR_FREE(raptor_node, node);
    }
    
  }

  return node;
  
}

static void
raptor_free_node(raptor_node *node)
{
  if(!node)
    return;

  node->ref_count--;
  
  if(node->ref_count > 0)
    return;
  
  switch (node->type) {
      case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
      case RAPTOR_IDENTIFIER_TYPE_PREDICATE:
        raptor_free_uri(node->value.resource.uri);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
        RAPTOR_FREE(blank, node->value.blank.string);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_LITERAL:
      case RAPTOR_IDENTIFIER_TYPE_XML_LITERAL:
        RAPTOR_FREE(literal, node->value.literal.string);

        if(node->value.literal.datatype)
          raptor_free_uri(node->value.literal.datatype);

        if(node->value.literal.language)
          RAPTOR_FREE(language, node->value.literal.language);

        break;
          
      case RAPTOR_IDENTIFIER_TYPE_ORDINAL:
      case RAPTOR_IDENTIFIER_TYPE_UNKNOWN: 
      default:
        /* Nothing to do */
        break;
  }

  RAPTOR_FREE(raptor_node, node);
  
}

static int
raptor_node_equals(raptor_node *node1, raptor_node *node2)
{
  int rv = 0;  

  if(node1->type != node2->type)
    return 0;

  switch (node1->type) {
      case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
      case RAPTOR_IDENTIFIER_TYPE_PREDICATE:
        rv = raptor_uri_equals(node1->value.resource.uri,
                               node2->value.resource.uri);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
        rv = !strcmp((const char*)node1->value.blank.string, (const char*)node2->value.blank.string);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_LITERAL:
      case RAPTOR_IDENTIFIER_TYPE_XML_LITERAL:

        if((char *)node1->value.literal.string != NULL &&
            (char *)node2->value.literal.string != NULL) {

          /* string */
          rv = (strcmp((char *)node1->value.literal.string,
                       (char *)node2->value.literal.string) == 0);

          /* language */
          if((char *)node1->value.literal.language != NULL &&
              (char *)node2->value.literal.language != NULL) {
            rv &= (strcmp((char *)node1->value.literal.language,
                          (char *)node2->value.literal.language) == 0);
          } else if((char *)node1->value.literal.language != NULL ||
                     (char *)node2->value.literal.language != NULL) {
            rv = 0;
          }

          /* datatype */
          if(node1->value.literal.datatype != NULL &&
              node2->value.literal.datatype != NULL) {
            rv &= (raptor_uri_equals(node1->value.literal.datatype,
                                     node2->value.literal.datatype) != 0);
          } else if(node1->value.literal.datatype != NULL ||
                     node2->value.literal.datatype != NULL) {
            rv = 0;
          }
          
        } else {
          RAPTOR_FATAL1("string must be non-NULL for literal or xml literal\n");
          rv = 0;
        }        

        break;
          
      case RAPTOR_IDENTIFIER_TYPE_ORDINAL:
        rv = (node1->value.ordinal.ordinal == node2->value.ordinal.ordinal);
        break;
        
      case RAPTOR_IDENTIFIER_TYPE_UNKNOWN: 
      default:
        /* Nothing to do */
        break;
  }

  return rv;
  
}


static int
raptor_node_matches(raptor_node *node, raptor_identifier_type node_type,
                    const void *node_data, raptor_uri *datatype,
                    const unsigned char *language)
{
  int rv = 0;
  
  if(node->type != node_type)
    return 0;

  switch (node->type) {
      case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
      case RAPTOR_IDENTIFIER_TYPE_PREDICATE:
        rv = raptor_uri_equals(node->value.resource.uri,
                               (raptor_uri *)node_data);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
        rv = !strcmp((const char*)node->value.blank.string, (const char *)node_data);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_LITERAL:
      case RAPTOR_IDENTIFIER_TYPE_XML_LITERAL:

        if((char *)node->value.literal.string != NULL &&
            (char *)node_data != NULL) {

          /* string */
          rv = (strcmp((char *)node->value.literal.string,
                       (char *)node_data) == 0);

          /* language */
          if((char *)node->value.literal.language != NULL &&
              (char *)language != NULL) {

            rv &= (strcmp((char *)node->value.literal.language,
                          (char *)language) == 0);
          } else if((char *)node->value.literal.language != NULL ||
                     (char *)language != NULL) {
            rv = 0;
          }

          /* datatype */
          if(node->value.literal.datatype != NULL && datatype != NULL) {
            rv &= (raptor_uri_equals(node->value.literal.datatype,datatype) !=0);
          } else if(node->value.literal.datatype != NULL || datatype != NULL) {
            rv = 0;
          }
          
        } else {
          RAPTOR_FATAL1("string must be non-NULL for literal or xml literal\n");
          rv = 0;
        }        
        
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_ORDINAL:
        rv = (node->value.ordinal.ordinal == *(int *)node_data);
        break;
        
      case RAPTOR_IDENTIFIER_TYPE_UNKNOWN: 
      default:
        /* Nothing to do */
        break;
  }

  return rv;
}


/*
 * raptor_subject implementation
 **/

static raptor_subject *
raptor_new_subject(raptor_node *node)
{
  raptor_subject *subject;
  
  if(!(node->type == RAPTOR_IDENTIFIER_TYPE_RESOURCE ||
        node->type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS ||
        node->type == RAPTOR_IDENTIFIER_TYPE_ORDINAL)) {
    RAPTOR_FATAL1("Subject node must be a resource, blank, or ordinal\n");
    return NULL;
  }  
  
  subject = (raptor_subject *)RAPTOR_CALLOC(raptor_subject, 1,
                                            sizeof(raptor_subject));

  if(subject) {
    subject->node = node;
    subject->node->ref_count++;
    
    subject->node_type = NULL;
    subject->properties =
      raptor_new_sequence((raptor_sequence_free_handler *)raptor_free_node, NULL);
    subject->list_items =
      raptor_new_sequence((raptor_sequence_free_handler *)raptor_free_node, NULL);

    if(subject->node == NULL ||
        subject->properties == NULL ||
        subject->list_items == NULL) {
      raptor_free_subject(subject);
      subject = NULL;
    }
  
  }

  return subject;

}


static void
raptor_free_subject(raptor_subject *subject) 
{
  if(subject) {

    if(subject->node)
      raptor_free_node(subject->node);
    
    if(subject->node_type)
      raptor_free_node(subject->node_type);
    
    if(subject->properties)
      raptor_free_sequence(subject->properties);

    if(subject->list_items)
      raptor_free_sequence(subject->list_items);

    RAPTOR_FREE(raptor_subject, subject);
    
  }
  
}


static int
raptor_subject_add_property(raptor_subject *subject, raptor_node *predicate,
                            raptor_node *object) 
{
  /* insert predicate/object pair in properties array */
  int err;
  
  err = raptor_sequence_push(subject->properties, predicate);
  if(err) {
    return err;
  }
  
  err = raptor_sequence_push(subject->properties, object);
  if(err) {
    raptor_sequence_pop(subject->properties);
    return err;
  }
  
  predicate->ref_count++;
  object->ref_count++;
  
  return 0;
  
}


static int
raptor_subject_add_list_element(raptor_subject *subject, int ordinal,
                                raptor_node *object)
{
  int rv = 1;
  raptor_node *node;

  node = (raptor_node*)raptor_sequence_get_at(subject->list_items, ordinal);
  if(!node) {
    /* If there isn't already an entry */
    rv = raptor_sequence_set_at(subject->list_items, ordinal, object);
    if(!rv)
      object->ref_count++;

  }
  
  return rv;
}


#ifdef RDFXMLA_DEBUG
static void
raptor_print_subject(raptor_subject *subject) 
{
  int i;
  unsigned char *subj;
  unsigned char *pred;
  unsigned char *obj;

  /* Note: The raptor_node field passed as the first argument for
   * raptor_statement_part_as_string() is somewhat arbitrary, since as
   * the data structure is designed, the first word in the value union
   * is what was passed as the subject/predicate/object of the
   * statement.
   */
  subj = raptor_statement_part_as_string(subject->node->value.resource.uri,
                                         subject->node->type, NULL, NULL);

  if(subject->type) {
      obj=raptor_statement_part_as_string(subject->type->value.resource.uri,
                                          subject->type->type,
                                          subject->type->value.literal.datatype,
                                          subject->type->value.literal.language);
      fprintf(stderr,"[%s, http://www.w3.org/1999/02/22-rdf-syntax-ns#type, %s]\n", subj, obj);      
      RAPTOR_FREE(cstring, obj);
  }
  
  for(i=0; i < raptor_sequence_size(subject->elements); i++) {

    raptor_node *o = raptor_sequence_get_at(subject->elements, i);
    if(o) {
      obj = raptor_statement_part_as_string(o->value.literal.string,
                                            o->type,
                                            o->value.literal.datatype,
                                            o->value.literal.language);
      fprintf(stderr,"[%s, [rdf:_%d], %s]\n", subj, i, obj);      
      RAPTOR_FREE(cstring, obj);
    }
    
  }

  i=0;
  while (i < raptor_sequence_size(subject->properties)) {

    raptor_node *p = raptor_sequence_get_at(subject->properties, i++);
    raptor_node *o = raptor_sequence_get_at(subject->properties, i++);

    if(p && o) {
      pred = raptor_statement_part_as_string(p->value.resource.uri, p->type,
                                             NULL, NULL);
      obj = raptor_statement_part_as_string(o->value.literal.string,
                                            o->type,
                                            o->value.literal.datatype,
                                            o->value.literal.language);
      fprintf(stderr,"[%s, %s, %s]\n", subj, pred, obj);      
      RAPTOR_FREE(cstring, pred);
      RAPTOR_FREE(cstring, obj);
    }
    
  }
  
  RAPTOR_FREE(cstring, subj);
  
}
#endif

/*
 * raptor rdfxml-abbrev implementation
 */

static raptor_node *
raptor_rdfxmla_lookup_node(raptor_rdfxmla_context* context,
                           raptor_identifier_type node_type,
                           const void *node_value, raptor_uri *datatype,
                           const unsigned char *language)
{
  raptor_node *rv_node = 0;
  int i;
  
  /* Search for specified node in array. TODO: this should really be a
   * hash, not a list. */
  for(i=0; i < raptor_sequence_size(context->nodes); i++) {
    raptor_node *node = (raptor_node*)raptor_sequence_get_at(context->nodes, i);

    if(raptor_node_matches(node, node_type, node_value, datatype, language)) {
      rv_node = node;
      break;
    }
  }
  
  /* If not found, create one and insert it */
  if(!rv_node) {
    rv_node = raptor_new_node(node_type, node_value, datatype, language);
    
    if(rv_node) {
      if(raptor_sequence_push(context->nodes, rv_node) == 0) {
        rv_node->ref_count++;
      } else {
        raptor_free_node(rv_node);
        rv_node = 0;
      }
      
    }
    
  }
  
  return rv_node;
  
}

static raptor_subject *
raptor_rdfxmla_find_subject(raptor_sequence *sequence,
                            raptor_identifier_type node_type,
                            const void *node_data, int *idx)
{
  raptor_subject *rv_subject = 0;
  int i;
  
  for(i=0; i < raptor_sequence_size(sequence); i++) {
    raptor_subject *subject=(raptor_subject*)raptor_sequence_get_at(sequence, i);

    if(subject && raptor_node_matches(subject->node, node_type, node_data, 0, 0)) {
      rv_subject = subject;
      break;
    }
    
  }

  if(idx)
    *idx = i;
  
  return rv_subject;
  
}


static raptor_subject *
raptor_rdfxmla_lookup_subject(raptor_rdfxmla_context* context,
                              raptor_identifier_type node_type,
                              const void *node_data)
{
  /* Search for specified resource in resources array. TODO: this
   * should really be a hash, not a list. */
  raptor_sequence *sequence = (node_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) ?
    context->blanks : context->subjects;
  
  raptor_subject *rv_subject = raptor_rdfxmla_find_subject(sequence, node_type,
                                                           node_data, NULL);

  /* If not found, create one and insert it */
  if(!rv_subject) {
    raptor_node *node = raptor_rdfxmla_lookup_node(context, node_type,
                                                   node_data, NULL, NULL);
    if(node) {      
      rv_subject = raptor_new_subject(node);
      if(rv_subject) {
        if(raptor_sequence_push(sequence, rv_subject)) {
          raptor_free_subject(rv_subject);
          rv_subject = NULL;
        }      
      }
    }
  }
  
  return rv_subject;
  
}

/* create a new serializer */
static int
raptor_rdfxmla_serialize_init(raptor_serializer* serializer, const char *name)
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;
  raptor_uri_handler *uri_handler;
  raptor_uri *rdf_type_uri;
  void *uri_context;
  
  raptor_uri_get_handler(&uri_handler, &uri_context);
  context->nstack=raptor_new_namespaces(uri_handler, uri_context,
                                        raptor_serializer_simple_error,
                                        serializer,
                                        1);
  context->rdf_nspace=raptor_new_namespace(context->nstack,
                                           (const unsigned char*)"rdf",
                                           (const unsigned char*)raptor_rdf_namespace_uri,
                                           0);

  context->namespaces=raptor_new_sequence(NULL, NULL);
  /* Note: item 0 in the list is rdf:RDF's namespace */
  raptor_sequence_push(context->namespaces, context->rdf_nspace);

  context->subjects =
    raptor_new_sequence((raptor_sequence_free_handler *)raptor_free_subject,NULL);

  context->blanks =
    raptor_new_sequence((raptor_sequence_free_handler *)raptor_free_subject,NULL);
  
  context->nodes =
    raptor_new_sequence((raptor_sequence_free_handler *)raptor_free_node, NULL);

  rdf_type_uri = raptor_new_uri_for_rdf_concept("type");
  if(rdf_type_uri) {    
    context->rdf_type = raptor_new_node(RAPTOR_IDENTIFIER_TYPE_RESOURCE,
                                        rdf_type_uri, NULL, NULL);
    raptor_free_uri(rdf_type_uri);
  }
  
  if(context->nstack == NULL || context->rdf_nspace == NULL ||
      context->namespaces == NULL || context->subjects == NULL ||
      context->blanks == NULL || context->nodes == NULL ||
      context->rdf_type == NULL) {
    raptor_rdfxmla_serialize_terminate(serializer);
    return 1;
  }

  context->rdf_xml_literal_uri=raptor_new_uri(raptor_xml_literal_datatype_uri_string);

  context->is_xmp=!strncmp(name, "rdfxml-xmp", 10);
  if(context->is_xmp)
    serializer->feature_write_xml_declaration=0;

  return 0;
}
  

/* destroy a serializer */
static void
raptor_rdfxmla_serialize_terminate(raptor_serializer* serializer)
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;

  if(context->xml_writer)
    raptor_free_xml_writer(context->xml_writer);

  if(context->rdf_RDF_element)
    raptor_free_xml_element(context->rdf_RDF_element);

  if(context->rdf_nspace)
    raptor_free_namespace(context->rdf_nspace);

  if(context->namespaces) {
    int i;
    
    /* Note: item 0 in the list is rdf:RDF's namespace and freed above */
    for(i=1; i< raptor_sequence_size(context->namespaces); i++) {
      raptor_namespace* ns;
      ns =(raptor_namespace*)raptor_sequence_get_at(context->namespaces, i);
      if(ns)
        raptor_free_namespace(ns);
    }
    raptor_free_sequence(context->namespaces);
  }

  if(context->subjects)
    raptor_free_sequence(context->subjects);
  
  if(context->blanks)
    raptor_free_sequence(context->blanks);
  
  if(context->nodes)
    raptor_free_sequence(context->nodes);
  
  if(context->nstack)
    raptor_free_namespaces(context->nstack);

  if(context->rdf_type)
    raptor_free_node(context->rdf_type);
  
  if(context->rdf_xml_literal_uri)
    raptor_free_uri(context->rdf_xml_literal_uri);

}
  

#define RDFXMLA_NAMESPACE_DEPTH 0

/* add a namespace */
static int
raptor_rdfxmla_serialize_declare_namespace_from_namespace(raptor_serializer* serializer, 
                                                          raptor_namespace *nspace)
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;
  int i;
  
  if(context->written_header)
    return 1;
  
  for(i=0; i< raptor_sequence_size(context->namespaces); i++) {
    raptor_namespace* ns;
    ns=(raptor_namespace*)raptor_sequence_get_at(context->namespaces, i);

    /* If prefix is already declared, ignore ig */
    if(!ns->prefix && !nspace->prefix)
      return 1;
    
    if(ns->prefix && nspace->prefix && 
       !strcmp((const char*)ns->prefix, (const char*)nspace->prefix))
      return 1;
  }

  nspace=raptor_new_namespace_from_uri(context->nstack,
                                       nspace->prefix, nspace->uri,
                                       RDFXMLA_NAMESPACE_DEPTH);
  if(!nspace)
    return 1;
  
  raptor_sequence_push(context->namespaces, nspace);
  return 0;
}


/* add a namespace */
static int
raptor_rdfxmla_serialize_declare_namespace(raptor_serializer* serializer, 
                                          raptor_uri *uri,
                                          const unsigned char *prefix)
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;
  raptor_namespace *ns;
  int rc;
  
  ns=raptor_new_namespace_from_uri(context->nstack, prefix, uri, 
                                   RDFXMLA_NAMESPACE_DEPTH);

  rc=raptor_rdfxmla_serialize_declare_namespace_from_namespace(serializer, 
                                                               ns);
  raptor_free_namespace(ns);
  
  return rc;
}


/* start a serialize */
static int
raptor_rdfxmla_serialize_start(raptor_serializer* serializer)
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;
  raptor_xml_writer* xml_writer;
  raptor_uri_handler *uri_handler;
  void *uri_context;

  raptor_uri_get_handler(&uri_handler, &uri_context);

  xml_writer=raptor_new_xml_writer(context->nstack,
                                   uri_handler, uri_context,
                                   serializer->iostream,
                                   raptor_serializer_simple_error,
                                   serializer,
                                   1);
  if(!xml_writer)
    return 1;

  raptor_xml_writer_set_feature(xml_writer,RAPTOR_FEATURE_WRITER_AUTO_INDENT,1);
  raptor_xml_writer_set_feature(xml_writer,RAPTOR_FEATURE_WRITER_AUTO_EMPTY, 1);
  raptor_xml_writer_set_feature(xml_writer,RAPTOR_FEATURE_WRITER_INDENT_WIDTH,2);
  raptor_xml_writer_set_feature(xml_writer, RAPTOR_FEATURE_WRITER_XML_VERSION,
                                serializer->xml_version);
  raptor_xml_writer_set_feature(xml_writer, 
                                RAPTOR_FEATURE_WRITER_XML_DECLARATION, 
                                serializer->feature_write_xml_declaration);
  
  context->xml_writer=xml_writer;

  return 0;
}


static void
raptor_rdfxmla_ensure_writen_header(raptor_serializer* serializer,
                                    raptor_rdfxmla_context* context) 
{
  raptor_xml_writer* xml_writer;
  raptor_qname *qname;
  raptor_uri *base_uri;
  int i;

  if(context->written_header)
    return;
  
  xml_writer=context->xml_writer;
  if(context->is_xmp)
    raptor_xml_writer_raw(xml_writer,
                          (const unsigned char*)"<?xpacket begin='﻿' id='W5M0MpCehiHzreSzNTczkc9d'?>\n<x:xmpmeta xmlns:x='adobe:ns:meta/'>");
  
  qname=raptor_new_qname_from_namespace_local_name(context->rdf_nspace,
                                                   (const unsigned char*)"RDF",
                                                   NULL);
  base_uri=serializer->base_uri;
  if(base_uri)
    base_uri=raptor_uri_copy(base_uri);
  context->rdf_RDF_element=raptor_new_xml_element(qname, NULL, base_uri);
  
  /* NOTE: Starts it item 1 as item 0 is the element's namespace (rdf) 
   * and does not need to be declared
   */
  for(i=1; i< raptor_sequence_size(context->namespaces); i++) {
    raptor_namespace* ns;
    ns=(raptor_namespace*)raptor_sequence_get_at(context->namespaces, i);
    raptor_xml_element_declare_namespace(context->rdf_RDF_element, ns);
  }
  raptor_xml_writer_start_element(xml_writer, context->rdf_RDF_element);
  
  context->written_header=1;
}
  

/* serialize a statement */
static int
raptor_rdfxmla_serialize_statement(raptor_serializer* serializer, 
                                   const raptor_statement *statement)
{
  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;
  raptor_subject *subject = 0;
  raptor_node *predicate = 0;
  raptor_node *object = 0;
  int rv;
  raptor_identifier_type object_type;

  if(statement->subject_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE ||
     statement->subject_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS ||
     statement->subject_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL) {

    subject = raptor_rdfxmla_lookup_subject(context, statement->subject_type,
                                            statement->subject);
    if(!subject)
      return 1;

  } else {
    raptor_serializer_error(serializer, "Do not know how to serialize node type %d\n", statement->subject_type);
    return 1;
  }  
  
  object_type=statement->object_type;
  if(object_type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
    if(statement->object_literal_datatype &&
       raptor_uri_equals(statement->object_literal_datatype, 
                         context->rdf_xml_literal_uri))
      object_type = RAPTOR_IDENTIFIER_TYPE_XML_LITERAL;
  }

  if(object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE ||
     object_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS ||
     object_type == RAPTOR_IDENTIFIER_TYPE_LITERAL ||
     object_type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL || 
     object_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL) {

    object = raptor_rdfxmla_lookup_node(context, object_type,
                                        statement->object,
                                        statement->object_literal_datatype,
                                        statement->object_literal_language);
    if(!object)
      return 1;          
    
  } else {
    raptor_serializer_error(serializer, "Do not know how to serialize node type %d\n", object_type);
    return 1;
  }

  if((statement->predicate_type == RAPTOR_IDENTIFIER_TYPE_PREDICATE) ||
     (statement->predicate_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)) {
    predicate = raptor_rdfxmla_lookup_node(context, statement->predicate_type,
                                           statement->predicate, NULL, NULL);

    if(subject->node_type == NULL &&
        raptor_node_equals(predicate, context->rdf_type)) {

      /* Store the first one as the type for abbreviation 2.14
       * purposes. Note that it is perfectly legal to have
       * multiple type definitions.  All definitions after the
       * first go in the property list */
      subject->node_type = raptor_rdfxmla_lookup_node(context,
                                                      object_type,
                                                      statement->object, NULL,
                                                      NULL);
      subject->node_type->ref_count++;
      return 0;
    
    } else {
      int add_property=1;

      if(context->is_xmp && predicate->ref_count > 1) {
        int i;
        for(i=0; i < raptor_sequence_size(subject->properties); i++) {
          raptor_node *node = (raptor_node*)raptor_sequence_get_at(subject->properties, i);
          if(node == predicate) {
            add_property=0;
            
            if(object->type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
              /* look for any generated blank node associated with this
               * statement and free it
               */
              int idx=0;
              if(raptor_rdfxmla_find_subject(context->blanks, object_type,
                                             statement->object, &idx))
                raptor_sequence_set_at(context->blanks, idx, NULL);
            }
            
            break;
          }
        }
      }

      if(add_property) {
	rv = raptor_subject_add_property(subject, predicate, object);
	if(rv) {
	  raptor_serializer_error(serializer,
				  "Unable to add properties to subject 0x%x\n",
				  subject);
	}
      }
    }
  
  } else if(statement->predicate_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL) {
    int idx = *(int*)statement->predicate;
    rv = raptor_subject_add_list_element(subject, idx, object);
    if(rv) {
      /* An ordinal might already exist at that location, the fallback
       * is to just put in the properties list */
      predicate = raptor_rdfxmla_lookup_node(context, statement->predicate_type,
                                             statement->predicate, NULL, NULL);
      rv = raptor_subject_add_property(subject, predicate, object);
      if(rv) {
        raptor_serializer_error(serializer,
                                "Unable to add properties to subject 0x%x\n",
                                subject);
      }
    }

    if(rv != 0)
      return rv;
    
  } else {
    raptor_serializer_error(serializer,
                            "Do not know how to serialize node type %d\n",
                            statement->predicate_type);
    return 1;
  }
  
  return 0;

}


/* end a serialize */
static int
raptor_rdfxmla_serialize_end(raptor_serializer* serializer)
{

  raptor_rdfxmla_context* context=(raptor_rdfxmla_context*)serializer->context;
  raptor_xml_writer* xml_writer;
  
  raptor_rdfxmla_ensure_writen_header(serializer, context);
  
  raptor_rdfxmla_emit(serializer);  

  xml_writer=context->xml_writer;

  raptor_xml_writer_end_element(xml_writer, context->rdf_RDF_element);

  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
  
  raptor_free_xml_element(context->rdf_RDF_element);
  context->rdf_RDF_element=NULL;

  if(context->is_xmp)
    raptor_xml_writer_raw(xml_writer, 
                          (const unsigned char*)"</x:xmpmeta>\n<?xpacket end='r'?>\n");
  
  return 0;
}


/* finish the serializer factory */
static void
raptor_rdfxmla_serialize_finish_factory(raptor_serializer_factory* factory)
{
  /* NOP */
}


static void
raptor_rdfxmla_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->context_length     = sizeof(raptor_rdfxmla_context);
  
  factory->init                = raptor_rdfxmla_serialize_init;
  factory->terminate           = raptor_rdfxmla_serialize_terminate;
  factory->declare_namespace   = raptor_rdfxmla_serialize_declare_namespace;
  factory->declare_namespace_from_namespace   = raptor_rdfxmla_serialize_declare_namespace_from_namespace;
  factory->serialize_start     = raptor_rdfxmla_serialize_start;
  factory->serialize_statement = raptor_rdfxmla_serialize_statement;
  factory->serialize_end       = raptor_rdfxmla_serialize_end;
  factory->finish_factory      = raptor_rdfxmla_serialize_finish_factory;
}


void
raptor_init_serializer_rdfxmla(void)
{
  raptor_serializer_register_factory("rdfxml-abbrev", "RDF/XML (Abbreviated)", 
                                     "application/rdf+xml",
                                     NULL,
                                     (const unsigned char*)"http://www.w3.org/TR/rdf-syntax-grammar",
                                     &raptor_rdfxmla_serializer_register_factory);
  raptor_serializer_register_factory("rdfxml-xmp", "RDF/XML (XMP Profile)", 
                                     "application/rdf+xml",
                                     NULL,
                                     (const unsigned char*)"http://www.w3.org/TR/rdf-syntax-grammar",
                                     &raptor_rdfxmla_serializer_register_factory);
}

