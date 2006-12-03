/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_serialize_turtle.c - Turtle serializer
 *
 * Copyright (C) 2006, Dave Robillard
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


#define MAX_ASCII_INT_SIZE 13


/*
 * Raptor turtle serializer object
 */
typedef struct {
  raptor_namespace_stack *nstack;       /* Namespace stack */
  raptor_namespace *rdf_nspace;         /* the rdf: namespace */
  raptor_turtle_writer *turtle_writer;  /* where the xml is being written */
  raptor_sequence *namespaces;          /* User declared namespaces */
  raptor_sequence *subjects;            /* subject items */
  raptor_sequence *blanks;              /* blank subject items */
  raptor_sequence *nodes;               /* nodes */
  raptor_node *rdf_type;                /* rdf:type uri */

  /* URI of rdf:XMLLiteral */
  raptor_uri* rdf_xml_literal_uri;

  /* URI of rdf:first */
  raptor_uri* rdf_first_uri;
  
  /* URI of rdf:rest */
  raptor_uri* rdf_rest_uri;

  /* non zero if rdf:RDF has been written (and thus no new namespaces
   * can be declared).
   */
  int written_header;

  /* for labeling namespaces */
  int namespace_count;
} raptor_turtle_context;


/* prototypes for functions */

static int raptor_turtle_emit_resource(raptor_serializer *serializer,
                                        raptor_node *node,
                                        int depth);

static int raptor_turtle_emit_literal(raptor_serializer *serializer,
                                       raptor_node *node,
                                       int depth);
static int raptor_turtle_emit_xml_literal(raptor_serializer *serializer,
                                           raptor_node *node,
                                           int depth);
static int raptor_turtle_emit_blank(raptor_serializer *serializer,
                                     raptor_node *node,
                                     int depth);
static int raptor_turtle_emit_subject_list_items(raptor_serializer* serializer,
                                                  raptor_subject *subject,
                                                  int depth);
static int raptor_turtle_emit_subject_collection_items(raptor_serializer* serializer,
                                                       raptor_subject *subject,
                                                       int depth);
static int raptor_turtle_emit_subject_properties(raptor_serializer *serializer,
                                                  raptor_subject *subject,
                                                  int depth);
static int raptor_turtle_emit_subject(raptor_serializer *serializer,
                                       raptor_subject *subject,
                                       int depth);
static int raptor_turtle_emit(raptor_serializer *serializer);

static int raptor_turtle_serialize_init(raptor_serializer* serializer,
                                         const char *name);
static void raptor_turtle_serialize_terminate(raptor_serializer* serializer);
static int raptor_turtle_serialize_declare_namespace(raptor_serializer* serializer, 
                                                      raptor_uri *uri,
                                                      const unsigned char *prefix);
static int raptor_turtle_serialize_start(raptor_serializer* serializer);
static int raptor_turtle_serialize_statement(raptor_serializer* serializer, 
                                              const raptor_statement *statement);

static int raptor_turtle_serialize_end(raptor_serializer* serializer);
static void raptor_turtle_serialize_finish_factory(raptor_serializer_factory* factory);


/*
 * raptor_turtle_emit_resource:
 * @serializer: #raptor_serializer object
 * @node: resource node
 * @depth: depth into tree
 * 
 * Emit a description of a resource using an XML Element
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_turtle_emit_resource(raptor_serializer *serializer,
                            raptor_node *node,
                            int depth) 
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  raptor_turtle_writer *turtle_writer = context->turtle_writer;

  raptor_qname* qname = NULL;

  RAPTOR_DEBUG5("Emitting resource node %p refcount %d subject %d object %d\n",
                node, 
                node->ref_count, node->count_as_subject, node->count_as_object);

  if(node->type != RAPTOR_IDENTIFIER_TYPE_RESOURCE)
    return 1;

  qname = raptor_namespaces_qname_from_uri(context->nstack,
      node->value.resource.uri, 10);

  if(qname) {
    raptor_turtle_writer_qname(turtle_writer, qname);
    raptor_free_qname(qname);
  } else {
    raptor_turtle_writer_reference(turtle_writer, node->value.resource.uri);
  }

  RAPTOR_DEBUG2("Emitted %p\n", node);
  
  return 0;
}


/*
 * raptor_turtle_emit_literal:
 * @serializer: #raptor_serializer object
 * @node: literal node
 * @depth: depth into tree
 * 
 * Emit a description of a literal (object).
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_turtle_emit_literal(raptor_serializer *serializer,
                           raptor_node *node,
                           int depth)
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  raptor_turtle_writer *turtle_writer = context->turtle_writer;
  
  RAPTOR_DEBUG5("Emitting literal node %p refcount %d subject %d object %d\n",
                node, 
                node->ref_count, node->count_as_subject, node->count_as_object);

  if(node->type != RAPTOR_IDENTIFIER_TYPE_LITERAL)
    return 1;
  
  raptor_turtle_writer_literal(turtle_writer, node->value.literal.string,
                               node->value.literal.language, 
                               node->value.literal.datatype);

  RAPTOR_DEBUG2("Emitted %p\n", node);
  
  return 0;
}

/*
 * raptor_turtle_emit_xml_literal:
 * @serializer: #raptor_serializer object
 * @node: XML literal node
 * @depth: depth into tree
 * 
 * Emit a description of a literal using an XML Element
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_turtle_emit_xml_literal(raptor_serializer *serializer,
                               raptor_node *node,
                               int depth) 
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  raptor_turtle_writer *turtle_writer = context->turtle_writer;
  raptor_uri* type_uri;

  RAPTOR_DEBUG5("Emitting XML literal node %p refcount %d subject %d object %d\n",
                node, 
                node->ref_count, node->count_as_subject, node->count_as_object);

  if(node->type != RAPTOR_IDENTIFIER_TYPE_XML_LITERAL)
    return 1;

  type_uri = raptor_new_uri((const unsigned char*)
    "http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral");
  
  raptor_turtle_writer_literal(turtle_writer, node->value.literal.string, NULL, type_uri);

  raptor_free_uri(type_uri);

  return 0;  
}

/*
 * raptor_turtle_emit_blank:
 * @serializer: #raptor_serializer object
 * @node: blank node
 * @depth: depth into tree
 * 
 * Emit a description of a blank node
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_turtle_emit_blank(raptor_serializer *serializer,
                         raptor_node *node,
                         int depth) 
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  RAPTOR_DEBUG5("Emitting blank node %p refcount %d subject %d object %d\n",
                node, 
                node->ref_count, node->count_as_subject, node->count_as_object);
  
  if(node->type != RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
    return 1;
  
  if((node->count_as_subject == 1 && node->count_as_object == 1)) {
    /* If this is only used as a 1 subject and object or never
     * used as a subject or never used as an object, it never need
     * be referenced with an explicit name */
    int idx;
    raptor_subject *blank = raptor_find_subject(context->blanks, node->type,
                                node->value.blank.string, &idx);
          
    if(blank) {
      raptor_turtle_emit_subject(serializer, blank, depth+1);
      raptor_sequence_set_at(context->blanks, idx, NULL);
    }
          
  } else {
    /* Blank node that needs an explicit name */
    const unsigned char *node_id = node->value.blank.string;

    raptor_turtle_writer_raw(context->turtle_writer, (const unsigned char*)"_:");
    raptor_turtle_writer_raw(context->turtle_writer, node_id);
  }

  RAPTOR_DEBUG2("Emitted %p\n", node);
  
  return 0;
}


/*
 * raptor_turtle_emit_subject_list_items:
 * @serializer: #raptor_serializer object
 * @subject: subject node
 * @depth: depth into tree
 * 
 * Emit an rdf list of items (rdf:li) about a subject node.
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_turtle_emit_subject_list_items(raptor_serializer* serializer,
                                      raptor_subject *subject,
                                      int depth)
{
  int rv = 0;
  int i=0;
  
  RAPTOR_DEBUG5("Emitting subject list items for node %p refcount %d subject %d object %d\n", 
                subject->node,
                subject->node->ref_count, subject->node->count_as_subject, 
                subject->node->count_as_object);

  while (!rv && i < raptor_sequence_size(subject->list_items)) {

    raptor_node *object;
    
    object = (raptor_node *)raptor_sequence_get_at(subject->list_items, i++);
    if(!object)
      continue;
    
    switch (object->type) {
      
      case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
        rv = raptor_turtle_emit_resource(serializer, object,
                                          depth+1);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_LITERAL:
        rv = raptor_turtle_emit_literal(serializer, object,
                                         depth+1);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_XML_LITERAL:
        rv = raptor_turtle_emit_xml_literal(serializer, object,
                                             depth+1);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
        rv = raptor_turtle_emit_blank(serializer, object, depth+1);
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
    
  }
  
  return rv;
}


/*
 * raptor_turtle_emit_subject_collection:
 * @serializer: #raptor_serializer object
 * @subject: subject node
 * @depth: depth into tree
 * 
 * Emit an abbreviated rdf collection of items (rdf:first, rdf:rest) about a subject node.
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_turtle_emit_subject_collection_items(raptor_serializer* serializer,
                                            raptor_subject *subject,
                                            int depth)
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  int rv = 0;
  int idx;

  RAPTOR_DEBUG5("Emitting subject collection items for node %p refcount %d subject %d object %d\n", 
                subject->node,
                subject->node->ref_count, subject->node->count_as_subject, 
                subject->node->count_as_object);

  while (!rv && raptor_sequence_size(subject->properties) >= 2) {

    raptor_node *predicate;
    raptor_node *object;
    
    int i=0;
    predicate = (raptor_node *)raptor_sequence_get_at(subject->properties, i++);
    object = (raptor_node *)raptor_sequence_get_at(subject->properties, i++);
    
    if (!raptor_uri_equals(predicate->value.resource.uri, 
                           context->rdf_first_uri)) {
      raptor_serializer_error(serializer,
                              "Malformed collection (first predicate is not rdf:first)");
      return 1;
    }
    
    if(!object)
      continue;
    
    switch (object->type) {
      
      case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
        rv = raptor_turtle_emit_resource(serializer, object,
                                          depth+1);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_LITERAL:
        rv = raptor_turtle_emit_literal(serializer, object,
                                         depth+1);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_XML_LITERAL:
        rv = raptor_turtle_emit_xml_literal(serializer, object,
                                             depth+1);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
        rv = raptor_turtle_emit_blank(serializer, object, depth+1);
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
    
    //raptor_turtle_writer_raw(context->turtle_writer, (const unsigned char*)" ");
   
    /* last item */
    if (i >= raptor_sequence_size(subject->properties))
      break;

    predicate = (raptor_node *)raptor_sequence_get_at(subject->properties, i++);
    object = (raptor_node *)raptor_sequence_get_at(subject->properties, i++);

    if (!raptor_uri_equals(predicate->value.resource.uri, context->rdf_rest_uri)) {
      raptor_serializer_error(serializer,
                            "Malformed collection (second predicate is not rdf:rest)");
      return 1;
    }
 
    subject = raptor_find_subject(context->blanks, object->type,
                                  object->value.blank.string, &idx);

    if (!subject) {
      raptor_serializer_error(serializer,
                            "Malformed collection (could not find subject for rdf:rest)");
      return 1;
    } else {
      raptor_turtle_writer_newline(context->turtle_writer);
    }
    
  }
  
  return rv;
}



/*
 * raptor_turtle_emit_subject_properties:
 * @serializer: #raptor_serializer object
 * @subject: subject node
 * @depth: depth into tree
 * 
 * Emit the properties about a subject node.
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_turtle_emit_subject_properties(raptor_serializer* serializer,
                                       raptor_subject *subject,
                                       int depth)
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  raptor_turtle_writer *turtle_writer = context->turtle_writer;
  raptor_node *last_predicate=NULL;
  int rv = 0;  
  int i;

  RAPTOR_DEBUG5("Emitting subject properties for node %p refcount %d subject %d object %d\n", 
                subject->node, subject->node->ref_count, 
                subject->node->count_as_subject,
                subject->node->count_as_object);

  /* Emit any rdf:_n properties collected */
  if(raptor_sequence_size(subject->list_items) > 0)
    rv = raptor_turtle_emit_subject_list_items(serializer, subject, depth+1);

  i=0;
  while (!rv && i < raptor_sequence_size(subject->properties)) {
    raptor_node *predicate;
    raptor_node *object;
    raptor_qname *qname;

    predicate = (raptor_node *)raptor_sequence_get_at(subject->properties, i++);
    object = (raptor_node *)raptor_sequence_get_at(subject->properties, i++);

    if( ! (last_predicate && raptor_node_equals(predicate, last_predicate))) {
      /* no object list abbreviation possible, terminate last object */
      if(last_predicate) {
        raptor_turtle_writer_raw(turtle_writer, (const unsigned char*)";");
        raptor_turtle_writer_newline(turtle_writer);
      }
      if(predicate->type == RAPTOR_IDENTIFIER_TYPE_ORDINAL) {
        /* we should only get here in rare cases -- usually when there
         * are multiple ordinals with the same value. */

        unsigned char uri_string[MAX_ASCII_INT_SIZE + 2];

        sprintf((char*)uri_string, "_%d", predicate->value.ordinal.ordinal);

        qname = raptor_new_qname_from_namespace_local_name(context->rdf_nspace,
            uri_string, NULL);

      } else {
  
        qname = raptor_namespaces_qname_from_uri(context->nstack,
          predicate->value.resource.uri, 10);
      
      }

      if(raptor_node_equals(predicate, context->rdf_type))
        raptor_turtle_writer_raw(turtle_writer, (const unsigned char*)"a");
      else if (qname)
        raptor_turtle_writer_qname(turtle_writer, qname);
      else
        raptor_turtle_writer_reference(turtle_writer, predicate->value.resource.uri);

      raptor_turtle_writer_raw(turtle_writer, (const unsigned char*)" ");
    
      if(qname)
        raptor_free_qname(qname);
    } else {
      raptor_turtle_writer_raw(turtle_writer, (const unsigned char*)", ");
    }

    switch (object->type) {
      
      case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
        rv = raptor_turtle_emit_resource(serializer, object, depth+1);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_LITERAL:
        rv = raptor_turtle_emit_literal(serializer, object, depth+1);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
        rv = raptor_turtle_emit_blank(serializer, object, depth+1);
        break;
          
      case RAPTOR_IDENTIFIER_TYPE_XML_LITERAL:
        rv = raptor_turtle_emit_xml_literal(serializer, object, depth+1);
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
    
    last_predicate = predicate;
  }
         
  return rv;
}

/*
 * raptor_turtle_emit_subject:
 * @serializer: #raptor_serializer object
 * @subject: subject node
 * @depth: depth into tree
 * 
 * Emit a subject node
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_turtle_emit_subject(raptor_serializer *serializer,
                           raptor_subject *subject,
                           int depth) 
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  raptor_turtle_writer* turtle_writer=context->turtle_writer;
  int blank = 1;
  int collection = 0;

  RAPTOR_DEBUG5("Emitting subject node %p refcount %d subject %d object %d\n", 
                subject->node,
                subject->node->ref_count, 
                subject->node->count_as_subject,
                subject->node->count_as_object);

  if(!depth &&
     subject->node->type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS &&
     subject->node->count_as_subject == 1 &&
     subject->node->count_as_object == 1) {
    RAPTOR_DEBUG2("Skipping subject node %p\n", subject->node);
    return 0;
  }
  
  if(raptor_sequence_size(subject->properties) == 0) {
    RAPTOR_DEBUG2("Skipping subject node %p\n", subject->node);
    return 0;
  }

  /* check if we can do collection abbreviation */
  if (raptor_sequence_size(subject->properties) >= 4) {
    raptor_node* pred1 = (raptor_node *)raptor_sequence_get_at(subject->properties, 0);
    raptor_node* pred2 = (raptor_node *)raptor_sequence_get_at(subject->properties, 2);

    if(pred1->type == RAPTOR_IDENTIFIER_TYPE_RESOURCE
          && pred2->type == RAPTOR_IDENTIFIER_TYPE_RESOURCE
          && (
              (raptor_uri_equals(pred1->value.resource.uri, context->rdf_first_uri)
                && raptor_uri_equals(pred2->value.resource.uri, context->rdf_rest_uri))
            || (raptor_uri_equals(pred2->value.resource.uri, context->rdf_first_uri)
             && raptor_uri_equals(pred1->value.resource.uri, context->rdf_rest_uri)))) {
      collection = 1;
    }
  }

  /* emit the subject node */
  if(subject->node->type == RAPTOR_IDENTIFIER_TYPE_RESOURCE) {
    
    raptor_turtle_emit_resource(serializer, subject->node, depth+1);
    blank = 0;
    
  } else if(subject->node->type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
    if((subject->node->count_as_subject == 1 && 
         subject->node->count_as_object == 0) && depth > 1) { 
      blank = 1;
    } else if(subject->node->count_as_object == 0) {
      raptor_turtle_writer_raw(turtle_writer, (const unsigned char*)"[]");
      blank = 0;
    } else if(!collection && subject->node->count_as_object > 1) {
      /* Referred to (used as an object), so needs a nodeID */
      const unsigned char* genid = subject->node->value.blank.string;
      size_t len = strlen((const char*)genid);
      unsigned char* subject_str = (unsigned char *)RAPTOR_MALLOC(cstring, len+3);
      snprintf((char*)subject_str, len+3, "_:%s", (const char*)genid);
      raptor_turtle_writer_raw(turtle_writer, subject_str);
      RAPTOR_FREE(cstring, subject_str);
    }
  } else if(subject->node->type == RAPTOR_IDENTIFIER_TYPE_ORDINAL) {
    unsigned char* subject_str = (unsigned char *)RAPTOR_MALLOC(string,
                                                raptor_rdf_namespace_uri_len + MAX_ASCII_INT_SIZE + 2);
    sprintf((char*)subject, "%s_%d", raptor_rdf_namespace_uri,
            subject->node->value.ordinal.ordinal);
      
    raptor_turtle_writer_raw(turtle_writer, subject_str);
    RAPTOR_FREE(cstring, subject_str);
    return blank = 0;
  } 

  if(collection) {
    raptor_turtle_writer_raw(turtle_writer, (const unsigned char*)"(");
    
    raptor_turtle_writer_increase_indent(turtle_writer);
    raptor_turtle_writer_newline(turtle_writer);
    
    raptor_turtle_emit_subject_collection_items(serializer, subject, depth+1);
    
    raptor_turtle_writer_decrease_indent(turtle_writer);
    raptor_turtle_writer_newline(turtle_writer);
    
    raptor_turtle_writer_raw(turtle_writer, (const unsigned char*)")");
  } else {
    if(blank && depth > 1)
      raptor_turtle_writer_raw(turtle_writer, (const unsigned char*)"[");
    
    raptor_turtle_writer_increase_indent(turtle_writer);
    raptor_turtle_writer_newline(turtle_writer);

    raptor_turtle_emit_subject_properties(serializer, subject, depth+1);
    
    raptor_turtle_writer_decrease_indent(turtle_writer);

    
    if(blank && depth > 1) {
      raptor_turtle_writer_newline(turtle_writer);
      raptor_turtle_writer_raw(turtle_writer, (const unsigned char*)"]");
    }
  }

  if(depth == 0) {
    /* NOTE: the space before the . here MUST be there or statements that end in
       a numeric literal will be interpreted incorrectly (the "." will be parsed
       as part of the literal and statement left unterminated) */
    raptor_turtle_writer_raw(turtle_writer, (const unsigned char*)" .");
    raptor_turtle_writer_newline(turtle_writer);
    raptor_turtle_writer_newline(turtle_writer);
  }

  return 0;
}


/*
 * raptor_turtle_emit - 
 * @serializer: #raptor_serializer object
 * 
 * Emit RDF/XML for all stored triples.
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_turtle_emit(raptor_serializer *serializer)
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  raptor_subject *subject;
  raptor_subject *blank;
  int i;
  
  for(i=0; i < raptor_sequence_size(context->subjects); i++) {
    subject = (raptor_subject *)raptor_sequence_get_at(context->subjects, i);
    if(subject)
      raptor_turtle_emit_subject(serializer, subject, 0);
  }
  
  /* Emit any remaining blank nodes */
  for(i=0; i < raptor_sequence_size(context->blanks); i++) {
    blank = (raptor_subject *)raptor_sequence_get_at(context->blanks, i);
    if(blank)
      raptor_turtle_emit_subject(serializer, blank, 0);
  }
  
  return 0;
}


/*
 * raptor serializer turtle implementation
 */


/* create a new serializer */
static int
raptor_turtle_serialize_init(raptor_serializer* serializer, const char *name)
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  raptor_uri_handler *uri_handler;
  raptor_uri *rdf_type_uri;
  void *uri_context;
  
  raptor_uri_get_handler(&uri_handler, &uri_context);
  context->nstack=raptor_new_namespaces(uri_handler, uri_context,
                                        (raptor_simple_message_handler)raptor_serializer_simple_error,
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
  } else {
    context->rdf_type = NULL;
  }
  
  if(!context->nstack || !context->rdf_nspace || !context->namespaces ||
     !context->subjects || !context->blanks || !context->nodes ||
     !context->rdf_type) {
    raptor_turtle_serialize_terminate(serializer);
    return 1;
  }

  context->rdf_xml_literal_uri=raptor_new_uri(raptor_xml_literal_datatype_uri_string);
  context->rdf_first_uri=raptor_new_uri((const unsigned char*)"http://www.w3.org/1999/02/22-rdf-syntax-ns#first");
  context->rdf_rest_uri=raptor_new_uri((const unsigned char*)"http://www.w3.org/1999/02/22-rdf-syntax-ns#rest");

  return 0;
}
  

/* destroy a serializer */
static void
raptor_turtle_serialize_terminate(raptor_serializer* serializer)
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;

  if(context->turtle_writer)
    raptor_free_turtle_writer(context->turtle_writer);

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
  
  if(context->rdf_first_uri)
    raptor_free_uri(context->rdf_first_uri);
  
  if(context->rdf_rest_uri)
    raptor_free_uri(context->rdf_rest_uri);
}
  

#define TURTLE_NAMESPACE_DEPTH 0

/* add a namespace */
static int
raptor_turtle_serialize_declare_namespace_from_namespace(raptor_serializer* serializer, 
                                                          raptor_namespace *nspace)
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  int i;
 
  if(context->written_header)
    return 1;
  
  if (!nspace->prefix)
    return 1;
  
  for(i=0; i< raptor_sequence_size(context->namespaces); i++) {
    raptor_namespace* ns;
    ns=(raptor_namespace*)raptor_sequence_get_at(context->namespaces, i);

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

  nspace=raptor_new_namespace_from_uri(context->nstack,
                                       nspace->prefix, nspace->uri,
                                       TURTLE_NAMESPACE_DEPTH);
  if(!nspace)
    return 1;
  
  raptor_sequence_push(context->namespaces, nspace);
  return 0;
}


/* add a namespace */
static int
raptor_turtle_serialize_declare_namespace(raptor_serializer* serializer, 
                                          raptor_uri *uri,
                                          const unsigned char *prefix)
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  raptor_namespace *ns;
  int rc;
  
  ns=raptor_new_namespace_from_uri(context->nstack, prefix, uri, 
                                   TURTLE_NAMESPACE_DEPTH);

  rc=raptor_turtle_serialize_declare_namespace_from_namespace(serializer, 
                                                               ns);
  raptor_free_namespace(ns);
  
  return rc;
}


/* start a serialize */
static int
raptor_turtle_serialize_start(raptor_serializer* serializer)
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  raptor_turtle_writer* turtle_writer;
  raptor_uri_handler *uri_handler;
  void *uri_context;

  raptor_uri_get_handler(&uri_handler, &uri_context);

  if(context->turtle_writer)
    raptor_free_turtle_writer(context->turtle_writer);

  turtle_writer=raptor_new_turtle_writer(serializer->base_uri, context->nstack,
                                   uri_handler, uri_context,
                                   serializer->iostream,
                                   (raptor_simple_message_handler)raptor_serializer_simple_error,
                                   serializer);
  if(!turtle_writer)
    return 1;

  raptor_turtle_writer_set_feature(turtle_writer,RAPTOR_FEATURE_WRITER_AUTO_INDENT,1);
  raptor_turtle_writer_set_feature(turtle_writer,RAPTOR_FEATURE_WRITER_INDENT_WIDTH,2);
  
  context->turtle_writer=turtle_writer;

  return 0;
}

static void
raptor_turtle_ensure_writen_header(raptor_serializer* serializer,
                                    raptor_turtle_context* context) 
{
  int i;

  if(context->written_header)
    return;
  
  for(i=0; i< raptor_sequence_size(context->namespaces); i++) {
    raptor_namespace* ns;
    ns=(raptor_namespace*)raptor_sequence_get_at(context->namespaces, i);
    raptor_turtle_writer_namespace_prefix(context->turtle_writer, ns);
    raptor_namespace_copy(context->nstack, ns, 0);
  }
  
  raptor_turtle_writer_raw(context->turtle_writer, (const unsigned char*)"\n");

  context->written_header=1;
}

/* serialize a statement */
static int
raptor_turtle_serialize_statement(raptor_serializer* serializer, 
                                   const raptor_statement *statement)
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  raptor_subject *subject = NULL;
  raptor_node *predicate = NULL;
  raptor_node *object = NULL;
  int rv;
  raptor_identifier_type object_type;

  if(statement->subject_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE ||
     statement->subject_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS ||
     statement->subject_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL) {

    subject = raptor_lookup_subject(context->nodes, context->subjects,
	                                context->blanks, statement->subject_type,
                                    statement->subject);
    if(!subject)
      return 1;

  } else {
    raptor_serializer_error(serializer,
                            "Do not know how to serialize node type %d\n",
                            statement->subject_type);
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

    object = raptor_lookup_node(context->nodes, object_type,
                                statement->object,
                                statement->object_literal_datatype,
                                statement->object_literal_language);
    if(!object)
      return 1;          

    if(object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE ||
       object_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
      object->count_as_object++;
    
  } else {
    raptor_serializer_error(serializer,
                            "Cannot serialize a triple with object node type %d\n",
                            object_type);
    return 1;
  }

  if((statement->predicate_type == RAPTOR_IDENTIFIER_TYPE_PREDICATE) ||
     (statement->predicate_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)) {
    predicate = raptor_lookup_node(context->nodes, statement->predicate_type,
                                   statement->predicate, NULL, NULL);
	      
    rv = raptor_subject_add_property(subject, predicate, object);
    if(rv) {
      raptor_serializer_error(serializer,
          "Unable to add properties to subject 0x%p\n",
          subject);
    }
  
  } else if(statement->predicate_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL) {
    int idx = *(int*)statement->predicate;
    rv = raptor_subject_add_list_element(subject, idx, object);
    if(rv) {
      /* An ordinal might already exist at that location, the fallback
       * is to just put in the properties list */
      predicate = raptor_lookup_node(context->nodes, statement->predicate_type,
                                     statement->predicate, NULL, NULL);
      rv = raptor_subject_add_property(subject, predicate, object);
      if(rv) {
        raptor_serializer_error(serializer,
                                "Unable to add properties to subject 0x%p\n",
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
raptor_turtle_serialize_end(raptor_serializer* serializer)
{
  raptor_turtle_context* context=(raptor_turtle_context*)serializer->context;
  
  raptor_turtle_ensure_writen_header(serializer, context);
  
  raptor_turtle_emit(serializer);  

  return 0;
}


/* finish the serializer factory */
static void
raptor_turtle_serialize_finish_factory(raptor_serializer_factory* factory)
{
  /* NOP */
}


static void
raptor_turtle_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->context_length     = sizeof(raptor_turtle_context);
  
  factory->init                = raptor_turtle_serialize_init;
  factory->terminate           = raptor_turtle_serialize_terminate;
  factory->declare_namespace   = raptor_turtle_serialize_declare_namespace;
  factory->declare_namespace_from_namespace   = raptor_turtle_serialize_declare_namespace_from_namespace;
  factory->serialize_start     = raptor_turtle_serialize_start;
  factory->serialize_statement = raptor_turtle_serialize_statement;
  factory->serialize_end       = raptor_turtle_serialize_end;
  factory->finish_factory      = raptor_turtle_serialize_finish_factory;
}


void
raptor_init_serializer_turtle(void)
{
  raptor_serializer_register_factory("turtle", "Turtle", 
                                     "application/x-turtle",
                                     NULL,
                                     (const unsigned char*)"http://www.dajobe.org/2004/01/turtle",
                                     &raptor_turtle_serializer_register_factory);
}

