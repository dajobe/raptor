/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_serialize_rdfxmla.c - RDF/XML with abbreviations serializer
 *
 * Copyright (C) 2004-2010, David Beckett http://www.dajobe.org/
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


/*
 * Raptor rdfxml-abbrev serializer object
 */
typedef struct {
  raptor_namespace_stack *nstack;       /* Namespace stack */
  raptor_namespace *xml_nspace;         /* the xml: namespace */
  raptor_namespace *rdf_nspace;         /* the rdf: namespace */
  raptor_xml_element* rdf_RDF_element;  /* the rdf:RDF element */
  raptor_xml_writer *xml_writer;        /* where the xml is being written */
  raptor_sequence *namespaces;          /* User declared namespaces */
  raptor_avltree *subjects;             /* subject items */
  raptor_avltree *blanks;               /* blank subject items */
  raptor_avltree *nodes;                /* nodes */
  raptor_abbrev_node *rdf_type;         /* rdf:type uri */

  /* non-zero if is Adobe XMP abbreviated form */
  int is_xmp;

  /* non zero if rdf:RDF has been written (and thus no new namespaces
   * can be declared).
   */
  int written_header;

  /* for labeling namespaces */
  int namespace_count;

  /* xml_writer was passed in and not owned by us */
  int external_xml_writer;

  /* true if should write rdf:RDF */
  int write_rdf_RDF;

  /* starting namespace stack depth */
  int starting_depth;

  /* namespaces stack was passed in andn not owned by us */
  int external_nstack;

  /* If not NULL, the URI of the single node to serialize - starting
   * from property elements */
  raptor_uri* single_node;

  /* If non-0, emit typed nodes */
  int write_typed_nodes;
} raptor_rdfxmla_context;


/* prototypes for functions */

static int raptor_rdfxmla_emit_resource(raptor_serializer *serializer,
                                        raptor_xml_element *element,
                                        raptor_abbrev_node *node,
                                        int depth);

static int raptor_rdfxmla_emit_literal(raptor_serializer *serializer,
                                       raptor_xml_element *element,
                                       raptor_abbrev_node *node,
                                       int depth);
static int raptor_rdfxmla_emit_blank(raptor_serializer *serializer,
                                     raptor_xml_element *element,
                                     raptor_abbrev_node* node,
                                     int depth);
static int raptor_rdfxmla_emit_subject_list_items(raptor_serializer* serializer,
                                                  raptor_abbrev_subject* subject,
                                                  int depth);
static int raptor_rdfxmla_emit_subject_properties(raptor_serializer *serializer,
                                                  raptor_abbrev_subject* subject,
                                                  int depth);
static int raptor_rdfxmla_emit_subject(raptor_serializer *serializer,
                                       raptor_abbrev_subject* subject,
                                       int depth);
static int raptor_rdfxmla_emit(raptor_serializer *serializer);

static int raptor_rdfxmla_serialize_init(raptor_serializer* serializer,
                                         const char *name);
static void raptor_rdfxmla_serialize_terminate(raptor_serializer* serializer);
static int raptor_rdfxmla_serialize_declare_namespace(raptor_serializer* serializer, 
                                                      raptor_uri *uri,
                                                      const unsigned char *prefix);
static int raptor_rdfxmla_serialize_start(raptor_serializer* serializer);
static int raptor_rdfxmla_serialize_statement(raptor_serializer* serializer, 
                                              raptor_statement *statement);

static int raptor_rdfxmla_serialize_end(raptor_serializer* serializer);
static void raptor_rdfxmla_serialize_finish_factory(raptor_serializer_factory* factory);


/* helper functions */


/*
 * raptor_rdfxmla_emit_resource_uri:
 * @serializer: #raptor_serializer object
 * @element: XML Element
 * @uri: URI object
 * @depth: depth into tree
 * 
 * Emit a description of a resource using an XML Element
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_rdfxmla_emit_resource_uri(raptor_serializer *serializer,
                                 raptor_xml_element *element,
                                 raptor_uri* uri,
                                 int depth) 
{
  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;
  raptor_xml_writer *xml_writer = context->xml_writer;
  raptor_qname **attrs;
  unsigned char *attr_name;
  unsigned char *attr_value;
  
  RAPTOR_DEBUG2("Emitting resource predicate URI %s\n",
                raptor_uri_as_string(uri));

  attrs = RAPTOR_CALLOC(raptor_qname**, 1, sizeof(raptor_qname*));
  if(!attrs)
    return 1;
    
  attr_name = (unsigned char *)"resource";

  if(RAPTOR_OPTIONS_GET_NUMERIC(serializer, RAPTOR_OPTION_RELATIVE_URIS))
    /* newly allocated string */
    attr_value = raptor_uri_to_relative_uri_string(serializer->base_uri, uri);
  else
    attr_value = raptor_uri_as_string(uri);

  attrs[0] = raptor_new_qname_from_namespace_local_name(serializer->world,
                                                        context->rdf_nspace,
                                                        attr_name, 
                                                        attr_value);
      
  if(RAPTOR_OPTIONS_GET_NUMERIC(serializer, RAPTOR_OPTION_RELATIVE_URIS))
    RAPTOR_FREE(char*, attr_value);

  if(!attrs[0]) {
    RAPTOR_FREE(qnamearray, attrs);
    return 1;
  }

  raptor_xml_element_set_attributes(element, attrs, 1);

  raptor_xml_writer_start_element(xml_writer, element);
  raptor_xml_writer_end_element(context->xml_writer, element);

  RAPTOR_DEBUG2("Emitted resource predicate URI %s\n",
                raptor_uri_as_string(uri));
  
  return 0;
}


/*
 * raptor_rdfxmla_emit_resource:
 * @serializer: #raptor_serializer object
 * @element: XML Element
 * @node: resource node
 * @depth: depth into tree
 * 
 * Emit a description of a resource using an XML Element
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_rdfxmla_emit_resource(raptor_serializer *serializer,
                             raptor_xml_element *element,
                             raptor_abbrev_node* node,
                             int depth) 
{
  int rc;
  
  RAPTOR_DEBUG_ABBREV_NODE("Emitting resource node", node);

  if(node->term->type != RAPTOR_TERM_TYPE_URI)
    return 1;

  rc = raptor_rdfxmla_emit_resource_uri(serializer, element, 
                                        node->term->value.uri, depth);
  
  RAPTOR_DEBUG_ABBREV_NODE("Emitted resource node", node);

  return rc;
}


/*
 * raptor_rdfxmla_emit_literal:
 * @serializer: #raptor_serializer object
 * @element: XML Element
 * @node: literal node
 * @depth: depth into tree
 * 
 * Emit a description of a literal using an XML Element
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_rdfxmla_emit_literal(raptor_serializer *serializer,
                            raptor_xml_element *element,
                            raptor_abbrev_node* node,
                            int depth)
{
  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;
  raptor_xml_writer *xml_writer = context->xml_writer;
  raptor_qname **attrs;
  int attrs_count;
  
  RAPTOR_DEBUG_ABBREV_NODE("Emitting literal node", node);

  if(node->term->type != RAPTOR_TERM_TYPE_LITERAL)
    return 1;
  
  if(node->term->value.literal.language || node->term->value.literal.datatype) {
          
    attrs_count = 0;
    attrs = RAPTOR_CALLOC(raptor_qname**, 2, sizeof(raptor_qname*));
    if(!attrs)
      return 1;

    if(node->term->value.literal.language) {
      attrs[attrs_count] = raptor_new_qname(context->nstack,
                                            (unsigned char*)"xml:lang",
                                            node->term->value.literal.language);
      if(!attrs[attrs_count])
        goto attrs_oom;
      attrs_count++;
    }

    if(node->term->value.literal.datatype) {
      unsigned char *datatype_value;
      datatype_value = raptor_uri_as_string(node->term->value.literal.datatype);
      attrs[attrs_count] = raptor_new_qname_from_namespace_local_name(serializer->world,
                                                                      context->rdf_nspace,
                                                                      (const unsigned char*)"datatype",
                                                                      datatype_value);
      if(!attrs[attrs_count])
        goto attrs_oom;
      attrs_count++;

      /* SJS Note: raptor_default_uri_as_string simply returns a
       * pointer to the string. Hope this is also true of alternate
       * uri implementations. */
      /* RAPTOR_FREE(char*, datatype_value); */

    }

    raptor_xml_element_set_attributes(element, attrs, attrs_count);

  }
      
  raptor_xml_writer_start_element(xml_writer, element);
  raptor_xml_writer_cdata(xml_writer, node->term->value.literal.string);
  raptor_xml_writer_end_element(xml_writer, element);

  RAPTOR_DEBUG_ABBREV_NODE("Emitted literal node", node);
  
  return 0;

  attrs_oom:

  raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_FATAL, NULL,
                   "Out of memory");

  /* attrs_count has not been incremented yet
   * and it points to the qname the allocation of which failed */
  attrs_count--;
  while(attrs_count>=0)
    raptor_free_qname(attrs[attrs_count--]);

  RAPTOR_FREE(qnamearray, attrs);

  return 1;
}


/*
 * raptor_rdfxmla_emit_blank:
 * @serializer: #raptor_serializer object
 * @element: XML Element
 * @node: blank node
 * @depth: depth into tree
 * 
 * Emit a description of a blank node using an XML Element
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_rdfxmla_emit_blank(raptor_serializer *serializer,
                          raptor_xml_element *element, raptor_abbrev_node* node,
                          int depth) 
{
  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;

  RAPTOR_DEBUG_ABBREV_NODE("Emitting blank node", node);

  if(node->term->type != RAPTOR_TERM_TYPE_BLANK)
    return 1;
  
  if((node->count_as_subject == 1 && node->count_as_object == 1)) {
    /* If this is only used as a 1 subject and object or never
     * used as a subject or never used as an object, it never need
     * be referenced with an explicit name */
    raptor_abbrev_subject* blank;
    
    raptor_xml_writer_start_element(context->xml_writer, element);
    
    blank = raptor_abbrev_subject_find(context->blanks, node->term);
          
    if(blank) {
      raptor_rdfxmla_emit_subject(serializer, blank, depth + 1);
      raptor_abbrev_subject_invalidate(blank);
    }
          
  } else {
    unsigned char *attr_name = (unsigned char*)"nodeID";
    unsigned char *attr_value = node->term->value.blank.string;
    raptor_qname **attrs;

    attrs = RAPTOR_CALLOC(raptor_qname**, 1, sizeof(raptor_qname*));
    if(!attrs)
      return 1;

    attrs[0] = raptor_new_qname_from_namespace_local_name(serializer->world,
                                                          context->rdf_nspace,
                                                          attr_name,
                                                          attr_value);

    raptor_xml_element_set_attributes(element, attrs, 1);
    raptor_xml_writer_start_element(context->xml_writer, element);

  }

  raptor_xml_writer_end_element(context->xml_writer, element);

  RAPTOR_DEBUG_ABBREV_NODE("Emitted blank node", node);
  
  return 0;
}


/*
 * raptor_rdfxmla_emit_subject_list_items:
 * @serializer: #raptor_serializer object
 * @subject: subject node
 * @depth: depth into tree
 * 
 * Emit an rdf list of items (rdf:li) about a subject node.
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_rdfxmla_emit_subject_list_items(raptor_serializer* serializer,
                                       raptor_abbrev_subject* subject,
                                       int depth)
{
  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;
  int rv = 0;
  int i = 0;
  raptor_uri* base_uri = NULL;

  RAPTOR_DEBUG_ABBREV_NODE("Emitting subject list items", subject->node);

  while(!rv && i < raptor_sequence_size(subject->list_items)) {

    raptor_abbrev_node* object;
    raptor_qname *qname;
    raptor_xml_element *element;
    
    object = (raptor_abbrev_node*)raptor_sequence_get_at(subject->list_items, i++);
    if(!object)
      continue;
    
    qname = raptor_new_qname_from_namespace_local_name(serializer->world,
                                                       context->rdf_nspace,
                                                       (unsigned char *)"li",
                                                       NULL);
    
    if(serializer->base_uri)
      base_uri = raptor_uri_copy(serializer->base_uri);
    element = raptor_new_xml_element(qname, NULL, base_uri);
    if(!element) {
      raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_FATAL, NULL,
                       "Out of memory");
      raptor_free_qname(qname);
      rv = 1; /* error */
      break;
    }

    switch (object->term->type) {
      
      case RAPTOR_TERM_TYPE_URI:
        rv = raptor_rdfxmla_emit_resource(serializer, element, object,
                                          depth + 1);
        break;
          
      case RAPTOR_TERM_TYPE_LITERAL:
        rv = raptor_rdfxmla_emit_literal(serializer, element, object,
                                         depth + 1);
        break;
          
      case RAPTOR_TERM_TYPE_BLANK:
        rv = raptor_rdfxmla_emit_blank(serializer, element, object,
                                       depth + 1);
        break;

      case RAPTOR_TERM_TYPE_UNKNOWN:
      default:
        raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR,
                                   NULL, "Triple has unsupported term type %d", 
                                   object->term->type);
        break;

    }
    
    raptor_free_xml_element(element);

  }
  
  return rv;
}


/*
 * raptor_rdfxmla_emit_subject_properties:
 * @serializer: #raptor_serializer object
 * @subject: subject node
 * @depth: depth into tree
 * 
 * Emit the properties about a subject node.
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_rdfxmla_emit_subject_properties(raptor_serializer* serializer,
                                       raptor_abbrev_subject* subject,
                                       int depth)
{
  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;
  int rv = 0;
  int i;
  raptor_avltree_iterator* iter = NULL;
  raptor_term* subject_term = subject->node->term;

  RAPTOR_DEBUG_ABBREV_NODE("Emitting subject properties", subject->node);

  /* Emit any rdf:_n properties collected */
  if(raptor_sequence_size(subject->list_items) > 0) {
    rv = raptor_rdfxmla_emit_subject_list_items(serializer, subject, depth + 1);
    if(rv)
      return rv;
  }

  
  if(subject->node_type && !context->write_typed_nodes) {
    raptor_uri *base_uri = NULL;
    raptor_qname *qname = NULL;
    raptor_xml_element *element = NULL;

    /* if rdf:type was associated with this subject and do not want
     * a typed node, emit it as a property element 
     */
    qname = raptor_new_qname_from_resource(context->namespaces,
                                           context->nstack,
                                           &context->namespace_count,
                                           context->rdf_type);
    if(!qname)
      goto oom;
      
    if(serializer->base_uri)
      base_uri = raptor_uri_copy(serializer->base_uri);

    element = raptor_new_xml_element(qname, NULL, base_uri);
    if(!element) {
      if(base_uri)
        raptor_free_uri(base_uri);
      raptor_free_qname(qname);
      goto oom;
    }

    rv = raptor_rdfxmla_emit_resource_uri(serializer, element, 
                                          subject_term->value.uri,
                                          depth + 1);
    raptor_free_xml_element(element);
  }


  for(i = 0, iter = raptor_new_avltree_iterator(subject->properties, NULL, NULL, 1);
      iter && !rv;
      i++, (rv = raptor_avltree_iterator_next(iter))) {
    raptor_uri *base_uri = NULL;
    raptor_qname *qname;
    raptor_xml_element *element;
    raptor_abbrev_node** nodes;
    raptor_abbrev_node* predicate;
    raptor_abbrev_node* object;

    nodes = (raptor_abbrev_node**)raptor_avltree_iterator_get(iter);
    if(!nodes)
      break;
    predicate= nodes[0];
    object= nodes[1];
    
    qname = raptor_new_qname_from_resource(context->namespaces,
                                           context->nstack,
                                           &context->namespace_count,
                                           predicate);
    if(!qname) {
      raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                                 "Cannot split URI '%s' into an XML qname",
                                 raptor_uri_as_string(predicate->term->value.uri));
      continue;
    }
    
    if(serializer->base_uri)
      base_uri = raptor_uri_copy(serializer->base_uri);
    element = raptor_new_xml_element(qname, NULL, base_uri);
    if(!element) {
      if(base_uri)
        raptor_free_uri(base_uri);
      raptor_free_qname(qname);
      goto oom;
    }

    switch (object->term->type) {
      
      case RAPTOR_TERM_TYPE_URI:
        rv = raptor_rdfxmla_emit_resource(serializer, element, object,
                                          depth + 1);
        break;
          
      case RAPTOR_TERM_TYPE_LITERAL:
        rv = raptor_rdfxmla_emit_literal(serializer, element, object,
                                         depth + 1);
        break;
          
      case RAPTOR_TERM_TYPE_BLANK:
        rv = raptor_rdfxmla_emit_blank(serializer, element, object,
                                       depth + 1);
        break;
          
      case RAPTOR_TERM_TYPE_UNKNOWN:
      default:
        raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR,
                                   NULL, "Triple has unsupported term type %d", 
                                   object->term->type);
        break;
    }    

    /* Return error if emitting something failed above */
    if(rv)
      return rv;

    raptor_free_xml_element(element);
    
  }
  if(iter)
    raptor_free_avltree_iterator(iter);
  
  return rv;

  oom:
  if(iter)
    raptor_free_avltree_iterator(iter);
  raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_FATAL, NULL,
                   "Out of memory");
  return 1;
}


/*
 * raptor_rdfxmla_emit_subject:
 * @serializer: #raptor_serializer object
 * @subject: subject node
 * @depth: depth into tree
 * 
 * Emit a subject node
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_rdfxmla_emit_subject(raptor_serializer *serializer,
                            raptor_abbrev_subject* subject,
                            int depth) 
{
  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;  
  raptor_qname *qname = NULL;    
  raptor_xml_element *element = NULL;
  raptor_qname **attrs;
  unsigned char *attr_name;
  unsigned char *attr_value;
  raptor_uri *base_uri = NULL;
  int subject_is_single_node;
  raptor_term *subject_term = subject->node->term;
  
  if(!raptor_abbrev_subject_valid(subject))
    return 0;

  subject_is_single_node = (context->single_node &&
                            subject_term->type == RAPTOR_TERM_TYPE_URI &&
                            raptor_uri_equals(subject_term->value.uri,
                                              context->single_node));
  

  RAPTOR_DEBUG_ABBREV_NODE("Emitting subject node", subject->node);
  
  if(!depth &&
     subject_term->type == RAPTOR_TERM_TYPE_BLANK &&
     subject->node->count_as_subject == 1 &&
     subject->node->count_as_object == 1) {
    RAPTOR_DEBUG_ABBREV_NODE("Skipping subject node", subject->node);
    return 0;
  }
  

  if(subject->node_type && context->write_typed_nodes) {
    /* if rdf:type was associated with this subject */
    qname = raptor_new_qname_from_resource(context->namespaces,
                                           context->nstack,
                                           &context->namespace_count,
                                           subject->node_type);
    
    if(!qname) {
      raptor_log_error_formatted(serializer->world,
                                 RAPTOR_LOG_LEVEL_ERROR, NULL,
                                 "Cannot split URI '%s' into an XML qname",
                                 raptor_uri_as_string(subject->node_type->term->value.uri));
      return 1;
    }
    
  } else {
    qname = raptor_new_qname_from_namespace_local_name(serializer->world, 
                                                       context->rdf_nspace,
                                                       (unsigned const char*)"Description",
                                                       NULL);
    if(!qname)
      goto oom;    
  }

  if(serializer->base_uri)
    base_uri = raptor_uri_copy(serializer->base_uri);
  element = raptor_new_xml_element(qname, NULL, base_uri);
  if(!element) {
    if(base_uri)
      raptor_free_uri(base_uri);
    raptor_free_qname(qname);
    goto oom;
  }
    
  attrs = RAPTOR_CALLOC(raptor_qname**, 1, sizeof(raptor_qname*));
  if(!attrs)
    goto oom;
    
  attr_name = NULL;
  attr_value = NULL;
    
  /* emit the subject node */
  if(subject_term->type == RAPTOR_TERM_TYPE_URI) {
    attr_name = (unsigned char*)"about";
    if(context->is_xmp) {
      /* XML rdf:about value is always "" */
      attr_value = RAPTOR_CALLOC(unsigned char*, 1, sizeof(unsigned char));
    } else if(RAPTOR_OPTIONS_GET_NUMERIC(serializer,
                                         RAPTOR_OPTION_RELATIVE_URIS))
      attr_value = raptor_uri_to_relative_uri_string(serializer->base_uri,
                                                     subject_term->value.uri);
    else
      attr_value = raptor_uri_to_string(subject_term->value.uri);
    
  } else if(subject_term->type == RAPTOR_TERM_TYPE_BLANK) {
    if(subject->node->count_as_subject &&
       subject->node->count_as_object &&
       !(subject->node->count_as_subject == 1 && 
         subject->node->count_as_object == 1)) {
      /* No need for nodeID if this node is never used as a subject
       * or object OR if it is used exactly once as subject and object.
       */
      attr_name = (unsigned char*)"nodeID";
      attr_value = subject_term->value.blank.string;
    }
  } 
    
  if(attr_name) {
    attrs[0] = raptor_new_qname_from_namespace_local_name(serializer->world,
                                                          context->rdf_nspace,
                                                          attr_name,
                                                          attr_value);
    
    if(subject_term->type != RAPTOR_TERM_TYPE_BLANK)
      RAPTOR_FREE(char*, attr_value);
    
    if(!attrs[0]) {
      RAPTOR_FREE(qnamearray, attrs);
      goto oom;  
    }

    /* Note: if we were willing to track the in-scope rdf:lang, we
     * could do the "2.5 Property Attributes" abbreviation here */
    raptor_xml_element_set_attributes(element, attrs, 1);
  } else {
    RAPTOR_FREE(qnamearray, attrs);
  }
    
  if(!subject_is_single_node) {
    raptor_xml_writer_start_element(context->xml_writer, element);
    raptor_rdfxmla_emit_subject_properties(serializer, subject, depth + 1);
    raptor_xml_writer_end_element(context->xml_writer, element);
  } else
    raptor_rdfxmla_emit_subject_properties(serializer, subject, depth);
    
  raptor_free_xml_element(element);
    
  return 0;

  oom:
  if(element)
    raptor_free_xml_element(element);
  raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                   "Out of memory");
  return 1;
}


/*
 * raptor_rdfxmla_emit - 
 * @serializer: #raptor_serializer object
 * 
 * Emit RDF/XML for all stored triples.
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_rdfxmla_emit(raptor_serializer *serializer)
{
  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;
  raptor_abbrev_subject* subject;
  raptor_abbrev_subject* blank;
  raptor_avltree_iterator* iter = NULL;

  iter = raptor_new_avltree_iterator(context->subjects, NULL, NULL, 1);
  while(iter) {
    subject = (raptor_abbrev_subject*)raptor_avltree_iterator_get(iter);
    if(subject) {
      raptor_rdfxmla_emit_subject(serializer, subject, context->starting_depth);
    }
    if(raptor_avltree_iterator_next(iter))
      break;
  }
  if(iter) 
    raptor_free_avltree_iterator(iter);
  
  if(!context->single_node) {
    /* Emit any remaining blank nodes */
    iter = raptor_new_avltree_iterator(context->blanks, NULL, NULL, 1);
    while(iter) {
      blank = (raptor_abbrev_subject*)raptor_avltree_iterator_get(iter);
      if(blank) {
	raptor_rdfxmla_emit_subject(serializer, blank, context->starting_depth);
      }
      if(raptor_avltree_iterator_next(iter))
	break;
    }
    if(iter) 
      raptor_free_avltree_iterator(iter);
  }
    
  return 0;
}


/*
 * raptor serializer rdfxml-abbrev implementation
 */


static void
raptor_rdfxmla_serialize_init_nstack(raptor_serializer* serializer,
                                     raptor_namespace_stack *nstack)
{
  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;
  
  context->xml_nspace = raptor_new_namespace(context->nstack,
                                             (const unsigned char*)"xml",
                                             raptor_xml_namespace_uri,
                                             context->starting_depth);
  
  context->rdf_nspace = raptor_new_namespace(context->nstack,
                                             (const unsigned char*)"rdf",
                                             raptor_rdf_namespace_uri,
                                             context->starting_depth);
}



/* create a new serializer */
static int
raptor_rdfxmla_serialize_init(raptor_serializer* serializer, const char *name)
{
  raptor_rdfxmla_context* context;
  raptor_term* type_term;

  context = (raptor_rdfxmla_context*)serializer->context;

  context->nstack = raptor_new_namespaces(serializer->world, 1);
  if(!context->nstack)
    return 1;

  raptor_rdfxmla_serialize_init_nstack(serializer, context->nstack);

  context->namespaces = raptor_new_sequence(NULL, NULL);

  context->subjects =
    raptor_new_avltree((raptor_data_compare_handler)raptor_abbrev_subject_compare,
                       (raptor_data_free_handler)raptor_free_abbrev_subject, 0);

  context->blanks =
    raptor_new_avltree((raptor_data_compare_handler)raptor_abbrev_subject_compare,
                       (raptor_data_free_handler)raptor_free_abbrev_subject, 0);
  
  context->nodes =
    raptor_new_avltree((raptor_data_compare_handler)raptor_abbrev_node_compare,
                       (raptor_data_free_handler)raptor_free_abbrev_node, 0);

  type_term = RAPTOR_RDF_type_term(serializer->world);
  context->rdf_type = raptor_new_abbrev_node(serializer->world, type_term);

  if(!context->xml_nspace || !context->rdf_nspace || !context->namespaces ||
     !context->subjects || !context->blanks || !context->nodes ||
     !context->rdf_type) {
    raptor_rdfxmla_serialize_terminate(serializer);
    return 1;
  }

  context->is_xmp=!strncmp(name, "rdfxml-xmp", 10);
  if(context->is_xmp)
    RAPTOR_OPTIONS_SET_NUMERIC(serializer,
                               RAPTOR_OPTION_WRITER_XML_DECLARATION, 0);

  /* Note: item 0 in the list is rdf:RDF's namespace */
  if(raptor_sequence_push(context->namespaces, context->rdf_nspace)) {
    raptor_rdfxmla_serialize_terminate(serializer);
    return 1;
  }

  context->write_rdf_RDF = 1;
  context->starting_depth = 0;
  context->single_node = NULL;
  context->write_typed_nodes = 1;
  
  return 0;
}
  

/* destroy a serializer */
static void
raptor_rdfxmla_serialize_terminate(raptor_serializer* serializer)
{
  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;

  if(context->xml_writer) {
    if(!context->external_xml_writer)
      raptor_free_xml_writer(context->xml_writer);
    context->xml_writer = NULL;
    context->external_xml_writer = 0;
  }

  if(context->rdf_RDF_element) {
    raptor_free_xml_element(context->rdf_RDF_element);
    context->rdf_RDF_element = NULL;
  }

  if(context->rdf_nspace) {
    raptor_free_namespace(context->rdf_nspace);
    context->rdf_nspace = NULL;
  }

  if(context->xml_nspace) {
    raptor_free_namespace(context->xml_nspace);
    context->xml_nspace = NULL;
  }

  if(context->namespaces) {
    int i;
    
    /* Note: item 0 in the list is rdf:RDF's namespace and freed above */
    for(i = 1; i< raptor_sequence_size(context->namespaces); i++) {
      raptor_namespace* ns;
      ns = (raptor_namespace*)raptor_sequence_get_at(context->namespaces, i);
      if(ns)
        raptor_free_namespace(ns);
    }
    raptor_free_sequence(context->namespaces);
    context->namespaces = NULL;
  }

  if(context->subjects) {
    raptor_free_avltree(context->subjects);
    context->subjects = NULL;
  }
  
  if(context->blanks) {
    raptor_free_avltree(context->blanks);
    context->blanks = NULL;
  }
  
  if(context->nodes) {
    raptor_free_avltree(context->nodes);
    context->nodes = NULL;
  }
  
  /* always free raptor_namespace* before stack */
  if(context->nstack) {
    if(!context->external_nstack)
      raptor_free_namespaces(context->nstack);
    context->nstack = NULL;
  }

  if(context->rdf_type) {
    raptor_free_abbrev_node(context->rdf_type);
    context->rdf_type = NULL;
  }
}


#define RDFXMLA_NAMESPACE_DEPTH 0

/* add a namespace */
static int
raptor_rdfxmla_serialize_declare_namespace_from_namespace(raptor_serializer* serializer, 
                                                          raptor_namespace *nspace)
{
  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;
  int i;
  
  if(context->written_header)
    return 1;
  
  for(i = 0; i< raptor_sequence_size(context->namespaces); i++) {
    raptor_namespace* ns;
    ns = (raptor_namespace*)raptor_sequence_get_at(context->namespaces, i);

    /* If prefix is already declared, ignore it */
    if(!ns->prefix && !nspace->prefix)
      return 1;
    
    if(ns->prefix && nspace->prefix && 
       !strcmp((const char*)ns->prefix, (const char*)nspace->prefix))
      return 1;

    if(ns->uri && nspace->uri &&
       raptor_uri_equals(ns->uri, nspace->uri))
      return 1;
  }

  nspace = raptor_new_namespace_from_uri(context->nstack,
                                         nspace->prefix, nspace->uri,
                                         context->starting_depth +
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
  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;
  raptor_namespace *ns;
  int rc;
  
  ns = raptor_new_namespace_from_uri(context->nstack, prefix, uri, 
                                     context->starting_depth +
                                       RDFXMLA_NAMESPACE_DEPTH);

  rc = raptor_rdfxmla_serialize_declare_namespace_from_namespace(serializer,
                                                                 ns);
  raptor_free_namespace(ns);
  
  return rc;
}


/*
 * raptor_rdfxmla_serialize_set_write_rdf_RDF:
 * @serializer: serializer object
 * @value: value
 *
 * INTERNAL - Set flag to write rdf:RDF root element
 *
 * Return value: non-0 on failure
 */
int
raptor_rdfxmla_serialize_set_write_rdf_RDF(raptor_serializer* serializer,
                                           int value)
{
  raptor_rdfxmla_context* context;

  if(strcmp(serializer->factory->desc.names[0], "rdfxml-abbrev"))
    return 1;
  
  context = (raptor_rdfxmla_context*)serializer->context;

  context->write_rdf_RDF = value;

  return 0;
}


/*
 * raptor_rdfxmla_serialize_set_xml_writer:
 * @serializer: serializer object
 * @xml_writer: XML writer
 * @nstack: namespace stack
 *
 * INTERNAL - Set an existing created XML writer to write the serializing to
 *
 * Return value: non-0 on failure
 */
int
raptor_rdfxmla_serialize_set_xml_writer(raptor_serializer* serializer,
                                        raptor_xml_writer* xml_writer,
                                        raptor_namespace_stack *nstack)
{
  raptor_rdfxmla_context* context;

  if(strcmp(serializer->factory->desc.names[0], "rdfxml-abbrev"))
    return 1;
  
  context = (raptor_rdfxmla_context*)serializer->context;

  context->xml_writer = xml_writer;
  context->starting_depth = xml_writer ? (raptor_xml_writer_get_depth(xml_writer) + 1) : -1;
  context->external_xml_writer = (xml_writer != NULL);

  if(context->xml_nspace)
    raptor_free_namespace(context->xml_nspace);
  if(context->rdf_nspace)
    raptor_free_namespace(context->rdf_nspace);
  /* always free raptor_namespace* before stack */
  if(context->nstack)
    raptor_free_namespaces(context->nstack);

  context->nstack = nstack;
  context->external_nstack = 1;
  raptor_rdfxmla_serialize_init_nstack(serializer, context->nstack);

  return 0;
}


/*
 * raptor_rdfxmla_serialize_set_single_node:
 * @serializer:
 * @uri:
 *
 * INTERNAL - Set a single node to serialize the contents
 *
 * The outer node element with this URI is not serialized, the inner
 * property elements are written.  @uri is copied
 *
 * Return value: non-0 on failure
 */
int
raptor_rdfxmla_serialize_set_single_node(raptor_serializer* serializer,
                                         raptor_uri* uri)
{
  raptor_rdfxmla_context* context;

  if(strcmp(serializer->factory->desc.names[0], "rdfxml-abbrev"))
    return 1;
  
  context = (raptor_rdfxmla_context*)serializer->context;

  if(context->single_node)
    raptor_free_uri(context->single_node);
  
  context->single_node = raptor_uri_copy(uri);

  return 0;
}


/*
 * raptor_rdfxmla_serialize_set_write_typed_nodes:
 * @serializer:
 * @value:
 *
 * INTERNAL - Set flag to write typed node elements
 *
 * Return value: non-0 on failure
 */
int
raptor_rdfxmla_serialize_set_write_typed_nodes(raptor_serializer* serializer,
                                               int value)
{
  raptor_rdfxmla_context* context;

  if(strcmp(serializer->factory->desc.names[0], "rdfxml-abbrev"))
    return 1;
  
  context = (raptor_rdfxmla_context*)serializer->context;

  context->write_typed_nodes = value;

  return 0;
}


/* start a serialize */
static int
raptor_rdfxmla_serialize_start(raptor_serializer* serializer)
{
  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;

  if(!context->external_xml_writer) {
    raptor_xml_writer* xml_writer;
    raptor_option option;

    if(context->xml_writer)
      raptor_free_xml_writer(context->xml_writer);

    xml_writer = raptor_new_xml_writer(serializer->world, context->nstack,
                                       serializer->iostream);
    if(!xml_writer)
      return 1;

    raptor_xml_writer_set_option(xml_writer,
                                 RAPTOR_OPTION_WRITER_AUTO_INDENT, NULL,1);
    raptor_xml_writer_set_option(xml_writer,
                                 RAPTOR_OPTION_WRITER_AUTO_EMPTY, NULL, 1);
    raptor_xml_writer_set_option(xml_writer,
                                 RAPTOR_OPTION_WRITER_INDENT_WIDTH, NULL, 2);
    option = RAPTOR_OPTION_WRITER_XML_VERSION;
    raptor_xml_writer_set_option(xml_writer, option, NULL,
                                 RAPTOR_OPTIONS_GET_NUMERIC(serializer, option));
    option = RAPTOR_OPTION_WRITER_XML_DECLARATION;
    raptor_xml_writer_set_option(xml_writer, option, NULL,
                                 RAPTOR_OPTIONS_GET_NUMERIC(serializer, option));

    context->xml_writer = xml_writer;
  }
  
  return 0;
}


static int
raptor_rdfxmla_ensure_writen_header(raptor_serializer* serializer,
                                    raptor_rdfxmla_context* context) 
{
  raptor_xml_writer* xml_writer;
  raptor_qname *qname;
  raptor_uri *base_uri;
  int i;
  raptor_qname **attrs = NULL;
  int attrs_count = 0;

  if(context->written_header)
    return 0; /* already succeeded */

  if(!context->write_rdf_RDF) {
    context->written_header = 1;
    return 0;
  }
  
  xml_writer = context->xml_writer;
  if(context->is_xmp)
    raptor_xml_writer_raw(xml_writer,
                          (const unsigned char*)"<?xpacket begin='﻿' id='W5M0MpCehiHzreSzNTczkc9d'?>\n<x:xmpmeta xmlns:x='adobe:ns:meta/'>\n");
  
  qname = raptor_new_qname_from_namespace_local_name(serializer->world,
                                                     context->rdf_nspace,
                                                     (const unsigned char*)"RDF",
                                                     NULL);
  if(!qname)
    goto oom;
  base_uri = serializer->base_uri;
  if(base_uri)
    base_uri = raptor_uri_copy(base_uri);
  context->rdf_RDF_element = raptor_new_xml_element(qname, NULL, base_uri);
  if(!context->rdf_RDF_element) {
    if(base_uri)
      raptor_free_uri(base_uri);
    raptor_free_qname(qname);
    goto oom;
  }
  
  /* NOTE: Starts at item 1 as item 0 is the element's namespace (rdf) 
   * and does not need to be declared
   */
  for(i = 1; i< raptor_sequence_size(context->namespaces); i++) {
    raptor_namespace* ns;
    ns = (raptor_namespace*)raptor_sequence_get_at(context->namespaces, i);
    raptor_xml_element_declare_namespace(context->rdf_RDF_element, ns);
  }

  if(base_uri &&
     RAPTOR_OPTIONS_GET_NUMERIC(serializer, RAPTOR_OPTION_WRITE_BASE_URI)) {
    const unsigned char* base_uri_string;

    attrs = RAPTOR_CALLOC(raptor_qname**, 1, sizeof(raptor_qname*));
    if(!attrs)
      goto oom;

    base_uri_string = raptor_uri_as_string(base_uri);
    attrs[attrs_count] = raptor_new_qname_from_namespace_local_name(serializer->world,
                                                                    context->xml_nspace,
                                                                    (const unsigned char*)"base",
                                                                    base_uri_string);
    if(!attrs[attrs_count]) {
      RAPTOR_FREE(qnamearray, attrs);
      goto oom;
    }
    attrs_count++;
  }

  if(attrs_count)
    raptor_xml_element_set_attributes(context->rdf_RDF_element, attrs, 
                                      attrs_count);
  else
    raptor_xml_element_set_attributes(context->rdf_RDF_element, NULL, 0);



  raptor_xml_writer_start_element(xml_writer, context->rdf_RDF_element);
  
  context->written_header = 1;

  return 0;

  oom:
  raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                   "Out of memory");
  return 1;
}
  

/* serialize a statement */
static int
raptor_rdfxmla_serialize_statement(raptor_serializer* serializer, 
                                   raptor_statement *statement)
{
  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;
  raptor_abbrev_subject* subject = NULL;
  raptor_abbrev_node* predicate = NULL;
  raptor_abbrev_node* object = NULL;
  int rv = 0;
  raptor_term_type object_type;
  
  if(!(statement->subject->type == RAPTOR_TERM_TYPE_URI ||
       statement->subject->type == RAPTOR_TERM_TYPE_BLANK)) {
    raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                               "Cannot serialize a triple with subject node type %d",
                               statement->subject->type);
    return 1;
  }  

  subject = raptor_abbrev_subject_lookup(context->nodes, context->subjects,
                                         context->blanks,
                                         statement->subject);
  if(!subject)
    return 1;
  
  object_type = statement->object->type;

  if(!(object_type == RAPTOR_TERM_TYPE_URI ||
       object_type == RAPTOR_TERM_TYPE_BLANK ||
       object_type == RAPTOR_TERM_TYPE_LITERAL)) {
    raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                               "Cannot serialize a triple with object node type %d",
                               object_type);
    return 1;
  }
  
  object = raptor_abbrev_node_lookup(context->nodes,
                                     statement->object);
  if(!object)
    return 1;          


  if(statement->predicate->type == RAPTOR_TERM_TYPE_URI) {
    predicate = raptor_abbrev_node_lookup(context->nodes, statement->predicate);
    if(!predicate)
      return 1;

    if(!subject->node_type && 
       raptor_abbrev_node_equals(predicate, context->rdf_type) &&
       statement->object->type == RAPTOR_TERM_TYPE_URI) {

      /* Store the first one as the type for abbreviation 2.14
       * purposes. Note that it is perfectly legal to have
       * multiple type definitions.  All definitions after the
       * first go in the property list */
      subject->node_type = raptor_abbrev_node_lookup(context->nodes,
                                                     statement->object);
      if(!subject->node_type)
        return 1;
      subject->node_type->ref_count++;
      return 0;
    
    } else {
      int add_property = 1;

      if(context->is_xmp && predicate->ref_count > 1) {
        raptor_avltree_iterator* iter = NULL;
        int i;
        for(i = 0, (iter = raptor_new_avltree_iterator(subject->properties, NULL, NULL, 1));
            iter && !rv;
            i++, (rv = raptor_avltree_iterator_next(iter))) {
          raptor_abbrev_node** nodes;
          raptor_abbrev_node* node;

          nodes = (raptor_abbrev_node**)raptor_avltree_iterator_get(iter);
          if(!nodes)
            break;
          node= nodes[0];
          
          if(node == predicate) {
            add_property = 0;
            if(object->term->type == RAPTOR_TERM_TYPE_BLANK) {
              /* look for any generated blank node associated with this
               * statement and free it
               */
              raptor_abbrev_subject *blank = 
                raptor_abbrev_subject_find(context->blanks,
                                           statement->object);
              if(subject) raptor_avltree_delete(context->blanks, blank);
            }
            break;
          }
        }
        if(iter)
          raptor_free_avltree_iterator(iter);
      }

      if(add_property) {
        rv = raptor_abbrev_subject_add_property(subject, predicate, object);
        if(rv < 0) {
          raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                                     "Unable to add properties to subject %p",
                                     subject);
          return rv;
        }
      }
    }
  
  } else {
    raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                               "Cannot serialize a triple with predicate node type %d",
                               statement->predicate->type);
    return 1;
  }
  
  if(object_type == RAPTOR_TERM_TYPE_URI ||
     object_type == RAPTOR_TERM_TYPE_BLANK)
    object->count_as_object++;
    

  return 0;

}


/* end a serialize */
static int
raptor_rdfxmla_serialize_end(raptor_serializer* serializer)
{

  raptor_rdfxmla_context* context = (raptor_rdfxmla_context*)serializer->context;
  raptor_xml_writer* xml_writer = context->xml_writer;

  if(xml_writer) {
    if(!raptor_rdfxmla_ensure_writen_header(serializer, context)) {

      raptor_rdfxmla_emit(serializer);  

      if(context->write_rdf_RDF) {
        /* ensure_writen_header() returned success, can assume context->rdf_RDF_element is non-NULL */
        raptor_xml_writer_end_element(xml_writer, context->rdf_RDF_element);
        
        raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
      }
    }
  }

  if(context->rdf_RDF_element) {
    raptor_free_xml_element(context->rdf_RDF_element);
    context->rdf_RDF_element = NULL;
  }

  if(context->is_xmp && xml_writer)
    raptor_xml_writer_raw(xml_writer, 
                          (const unsigned char*)"</x:xmpmeta>\n<?xpacket end='r'?>\n");

  if(xml_writer)
    raptor_xml_writer_flush(xml_writer);

  if(context->single_node)
    raptor_free_uri(context->single_node);

  context->written_header = 0;
  
  return 0;
}


/* finish the serializer factory */
static void
raptor_rdfxmla_serialize_finish_factory(raptor_serializer_factory* factory)
{
  /* NOP */
}


static const char* const rdfxml_xmp_names[2] = { "rdfxml-xmp", NULL};

static const char* const rdfxml_xmp_uri_strings[2] = {
  "http://www.w3.org/TR/rdf-syntax-grammar",
  NULL
};
  
#define RDFXML_XMP_TYPES_COUNT 1
static const raptor_type_q rdfxml_xmp_types[RDFXML_XMP_TYPES_COUNT + 1] = {
  { "application/rdf+xml", 19, 0},
  { NULL, 0, 0}
};

static int
raptor_rdfxml_xmp_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->desc.names = rdfxml_xmp_names;
  factory->desc.mime_types = rdfxml_xmp_types;

  factory->desc.label = "RDF/XML (XMP Profile)";
  factory->desc.uri_strings = rdfxml_xmp_uri_strings;

  factory->context_length     = sizeof(raptor_rdfxmla_context);
  
  factory->init                = raptor_rdfxmla_serialize_init;
  factory->terminate           = raptor_rdfxmla_serialize_terminate;
  factory->declare_namespace   = raptor_rdfxmla_serialize_declare_namespace;
  factory->declare_namespace_from_namespace   = raptor_rdfxmla_serialize_declare_namespace_from_namespace;
  factory->serialize_start     = raptor_rdfxmla_serialize_start;
  factory->serialize_statement = raptor_rdfxmla_serialize_statement;
  factory->serialize_end       = raptor_rdfxmla_serialize_end;
  factory->finish_factory      = raptor_rdfxmla_serialize_finish_factory;

  return 0;
}


static const char* const rdfxmla_names[2] = { "rdfxml-abbrev", NULL};

static const char* const rdfxml_uri_strings[3] = {
  "http://www.w3.org/ns/formats/RDF_XML",
  "http://www.w3.org/TR/rdf-syntax-grammar",
  NULL
};

#define RDFXMLA_TYPES_COUNT 1
static const raptor_type_q rdfxmla_types[RDFXMLA_TYPES_COUNT + 1] = {
  { "application/rdf+xml", 19, 0},
  { NULL, 0, 0}
};

static int
raptor_rdfxmla_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->desc.names = rdfxmla_names;
  factory->desc.mime_types = rdfxmla_types;

  factory->desc.label = "RDF/XML (Abbreviated)";
  factory->desc.uri_strings = rdfxml_uri_strings;

  factory->context_length     = sizeof(raptor_rdfxmla_context);
  
  factory->init                = raptor_rdfxmla_serialize_init;
  factory->terminate           = raptor_rdfxmla_serialize_terminate;
  factory->declare_namespace   = raptor_rdfxmla_serialize_declare_namespace;
  factory->declare_namespace_from_namespace   = raptor_rdfxmla_serialize_declare_namespace_from_namespace;
  factory->serialize_start     = raptor_rdfxmla_serialize_start;
  factory->serialize_statement = raptor_rdfxmla_serialize_statement;
  factory->serialize_end       = raptor_rdfxmla_serialize_end;
  factory->finish_factory      = raptor_rdfxmla_serialize_finish_factory;

  return 0;
}


int
raptor_init_serializer_rdfxmla(raptor_world* world)
{
  int rc;
  
  rc = !raptor_serializer_register_factory(world,
                                           &raptor_rdfxml_xmp_serializer_register_factory);
  if(rc)
    return rc;
  
  rc = !raptor_serializer_register_factory(world,
                                          &raptor_rdfxmla_serializer_register_factory);

  return rc;
}

