/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_serialize_turtle.c - Turtle serializer
 *
 * Copyright (C) 2006,2008 Dave Robillard
 * Copyright (C) 2004-2013 David Beckett http://www.dajobe.org/
 * Copyright (C) 2004-2005 University of Bristol, UK http://www.bristol.ac.uk/
 * Copyright (C) 2005 Steve Shepard steveshep@gmail.com
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


#define MAX_ASCII_INT_SIZE 13


/*
 * Raptor turtle serializer object
 */
typedef struct {
  raptor_namespace_stack *nstack;       /* Namespace stack */
  raptor_namespace *rdf_nspace;         /* the rdf: namespace */
  raptor_turtle_writer *turtle_writer;  /* where the xml is being written */
  raptor_sequence *namespaces;          /* User declared namespaces */
  raptor_avltree *subjects;             /* subject items */
  raptor_avltree *blanks;               /* blank subject items */
  raptor_avltree *nodes;                /* nodes */
  raptor_abbrev_node *rdf_type;         /* rdf:type uri */

  /* URI of rdf:XMLLiteral */
  raptor_uri* rdf_xml_literal_uri;

  /* URI of rdf:first */
  raptor_uri* rdf_first_uri;
  
  /* URI of rdf:rest */
  raptor_uri* rdf_rest_uri;

  /* URI of rdf:nil */
  raptor_uri* rdf_nil_uri;

  /* non zero if header is finished being written
   * (and thus no new namespaces can be declared).
   */
  int written_header;

  /* for labeling namespaces */
  int namespace_count;
} raptor_turtle_context;


/* prototypes for functions */

static int raptor_turtle_emit_resource(raptor_serializer *serializer,
                                       raptor_abbrev_node* node,
                                       int depth);

static int raptor_turtle_emit_literal(raptor_serializer *serializer,
                                      raptor_abbrev_node* node,
                                      int depth);
static int raptor_turtle_emit_blank(raptor_serializer *serializer,
                                    raptor_abbrev_node* node,
                                    int depth);
static int raptor_turtle_emit_subject_list_items(raptor_serializer* serializer,
                                                 raptor_abbrev_subject* subject,
                                                 int depth);
static int raptor_turtle_emit_subject_collection_items(raptor_serializer* serializer,
                                                       raptor_abbrev_subject* subject,
                                                       int depth);
static int raptor_turtle_emit_subject_properties(raptor_serializer *serializer,
                                                 raptor_abbrev_subject* subject,
                                                 int depth);
static int raptor_turtle_emit_subject(raptor_serializer *serializer,
                                      raptor_abbrev_subject* subject,
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
                                             raptor_statement *statement);

static int raptor_turtle_serialize_end(raptor_serializer* serializer);
static void raptor_turtle_serialize_finish_factory(raptor_serializer_factory* factory);


int
raptor_turtle_is_legal_turtle_qname(raptor_qname* qname)
{
  const char* prefix_name;
  const char* local_name;
  
  if(!qname)
    return 0;
  
  prefix_name = qname->nspace ? (const char*)qname->nspace->prefix : NULL;
  if(prefix_name) {
    /* prefixName: must have leading [A-Z][a-z][0-9] (nameStartChar - '_')  */
    /* prefixName: no . anywhere */
    if(!(isalpha((int)*prefix_name) || isdigit((int)*prefix_name)) ||
       strchr(prefix_name, '.'))
      return 0;
  }

  local_name = (const char*)qname->local_name;
  if(local_name) {
    /* nameStartChar: must have leading [A-Z][a-z][0-9]_  */
    /* nameChar: no . anywhere */
    if(!(isalpha((int)*local_name) || isdigit((int)*local_name) || *local_name == '_') ||
       strchr(local_name, '.'))
      return 0;
  }
  
  return 1;
}
    
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
                            raptor_abbrev_node* node,
                            int depth) 
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_turtle_writer *turtle_writer = context->turtle_writer;

  raptor_qname* qname = NULL;

  RAPTOR_DEBUG_ABBREV_NODE("Emitting resource node", node);

  if(node->term->type != RAPTOR_TERM_TYPE_URI)
    return 1;

  if(raptor_uri_equals(node->term->value.uri, context->rdf_nil_uri)) {
      raptor_turtle_writer_raw_counted(turtle_writer,(const unsigned char *)"( )", 3);
      return 0;
  }

  qname = raptor_new_qname_from_namespace_uri(context->nstack,
                                              node->term->value.uri, 10);

  /* XML Names allow leading '_' and '.' anywhere but Turtle does not */
  if(qname && !raptor_turtle_is_legal_turtle_qname(qname)) {
    raptor_free_qname(qname);
    qname = NULL;
  }
  
  if(qname) {
    raptor_turtle_writer_qname(turtle_writer, qname);
    raptor_free_qname(qname);
  } else {
    raptor_turtle_writer_reference(turtle_writer, node->term->value.uri);
  }

  RAPTOR_DEBUG_ABBREV_NODE("Emitted", node);
  
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
                           raptor_abbrev_node* node,
                           int depth)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_turtle_writer *turtle_writer = context->turtle_writer;
  int rc = 0;
  
  RAPTOR_DEBUG_ABBREV_NODE("Emitting literal node", node);

  if(node->term->type != RAPTOR_TERM_TYPE_LITERAL)
    return 1;
  
  rc = raptor_turtle_writer_literal(turtle_writer, context->nstack,
                                    node->term->value.literal.string,
                                    node->term->value.literal.language, 
                                    node->term->value.literal.datatype);

  RAPTOR_DEBUG_ABBREV_NODE("Emitted literal node", node);
  
  return rc;
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
                         raptor_abbrev_node* node,
                         int depth) 
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  int rc = 0;
  
  RAPTOR_DEBUG_ABBREV_NODE("Emitting blank node", node);
  
  if(node->term->type != RAPTOR_TERM_TYPE_BLANK)
    return 1;
  
  if((node->count_as_subject == 1 && node->count_as_object == 1)) {
    /* If this is only used as a 1 subject and object or never
     * used as a subject or never used as an object, it never need
     * be referenced with an explicit name */
    raptor_abbrev_subject* blank;

    blank = raptor_abbrev_subject_find(context->blanks, node->term);
    if(blank) {
      rc = raptor_turtle_emit_subject(serializer, blank, depth+1);
      raptor_abbrev_subject_invalidate(blank);
    }
          
  } else {
    /* Blank node that needs an explicit name */
    raptor_turtle_writer_bnodeid(context->turtle_writer,
                                 node->term->value.blank.string,
                                 node->term->value.blank.string_len);
  }

  RAPTOR_DEBUG_ABBREV_NODE("Emitted blank node", node);
  
  return rc;
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
                                      raptor_abbrev_subject* subject,
                                      int depth)
{
  int rv = 0;
  int i = 0;

  RAPTOR_DEBUG_ABBREV_NODE("Emitting subject list items", subject->node);

  while(!rv && i < raptor_sequence_size(subject->list_items)) {
    raptor_abbrev_node* object;
    
    object = (raptor_abbrev_node*)raptor_sequence_get_at(subject->list_items,
                                                          i++);
    if(!object)
      continue;
    
    switch(object->term->type) {
      case RAPTOR_TERM_TYPE_URI:
        rv = raptor_turtle_emit_resource(serializer, object, depth+1);
        break;
          
      case RAPTOR_TERM_TYPE_LITERAL:
        rv = raptor_turtle_emit_literal(serializer, object, depth+1);
        break;
          
      case RAPTOR_TERM_TYPE_BLANK:
        rv = raptor_turtle_emit_blank(serializer, object, depth+1);
        break;

      case RAPTOR_TERM_TYPE_UNKNOWN:
      default:
        raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR,
                                   NULL, "Triple has unsupported term type %d", 
                                   object->term->type);
        break;

    }
    
  }
  
  return rv;
}


/*
 * raptor_turtle_emit_subject_collection_items:
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
                                            raptor_abbrev_subject* subject,
                                            int depth)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  int rv = 0;
  raptor_avltree_iterator* iter = NULL;
  int i;
  int is_new_subject = 0;

  RAPTOR_DEBUG_ABBREV_NODE("Emitting subject collection items", subject->node);

  /* if just saw a new subject (is_new_subject is true) then there is no need
   * to advance the iterator - it was just reset
   */
  for(i = 0, (iter = raptor_new_avltree_iterator(subject->properties, NULL, NULL, 1));
      iter && !rv;
      i++, (rv = is_new_subject ? 0 : raptor_avltree_iterator_next(iter))) {
    raptor_abbrev_node** nodes;
    raptor_abbrev_node* predicate;
    raptor_abbrev_node* object;

    is_new_subject = 0;

    nodes = (raptor_abbrev_node**)raptor_avltree_iterator_get(iter);
    if(!nodes)
      break;
    predicate= nodes[0];
    object= nodes[1];
    
    if(!raptor_uri_equals(predicate->term->value.uri,
                          context->rdf_first_uri)) {
      raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                       "Malformed collection - first predicate is not rdf:first");
      raptor_free_avltree_iterator(iter);
      return 1;
    }
    
    if(!object)
      continue;
    
    if(i > 0)
      raptor_turtle_writer_newline(context->turtle_writer);
    
    switch(object->term->type) {
      case RAPTOR_TERM_TYPE_URI:
        rv = raptor_turtle_emit_resource(serializer, object, depth+1);
        break;
          
      case RAPTOR_TERM_TYPE_LITERAL:
        rv = raptor_turtle_emit_literal(serializer, object, depth+1);
        break;
          
      case RAPTOR_TERM_TYPE_BLANK:
        rv = raptor_turtle_emit_blank(serializer, object, depth+1);
        break;

      case RAPTOR_TERM_TYPE_UNKNOWN:
      default:
        raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR,
                                   NULL, "Triple has unsupported term type %d", 
                                   object->term->type);
        break;
    }

    /* Return error if emitting something failed above */
    if(rv) {
      raptor_free_avltree_iterator(iter);
      return rv;
    }

    /* last item */
    rv = raptor_avltree_iterator_next(iter);
    if(rv)
      break;

    nodes = (raptor_abbrev_node**)raptor_avltree_iterator_get(iter);
    predicate = nodes[0];
    object = nodes[1];

    if(!raptor_uri_equals(predicate->term->value.uri, context->rdf_rest_uri)) {
      raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                       "Malformed collection - second predicate is not rdf:rest");
      raptor_free_avltree_iterator(iter);
      return 1;
    }
    
    if(object->term->type == RAPTOR_TERM_TYPE_BLANK) {
      subject = raptor_abbrev_subject_find(context->blanks, object->term);

      if(!subject) {
        raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                         "Malformed collection - could not find subject for rdf:rest");
        raptor_free_avltree_iterator(iter);
        return 1;
      }

      /* got a <(old)subject> rdf:rest <(new)subject> triple so know
       * subject has changed and should reset the properties iterator
       */
      if(iter)
        raptor_free_avltree_iterator(iter);
      iter = raptor_new_avltree_iterator(subject->properties, NULL, NULL, 1);
      is_new_subject = 1;

    } else {
      if(object->term->type != RAPTOR_TERM_TYPE_URI ||
         !raptor_uri_equals(object->term->value.uri, context->rdf_nil_uri)) {
        raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                         "Malformed collection - last rdf:rest resource is not rdf:nil");
        raptor_free_avltree_iterator(iter);
        return 1;
      }
      break;
    }
  }
  if(iter)
    raptor_free_avltree_iterator(iter);
  
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
                                      raptor_abbrev_subject* subject,
                                      int depth)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_turtle_writer *turtle_writer = context->turtle_writer;
  raptor_abbrev_node* last_predicate = NULL;
  int rv = 0;  
  raptor_avltree_iterator* iter = NULL;
  int i;

  RAPTOR_DEBUG_ABBREV_NODE("Emitting subject properties", subject->node);

  /* Emit any rdf:_n properties collected */
  if(raptor_sequence_size(subject->list_items) > 0)
    rv = raptor_turtle_emit_subject_list_items(serializer, subject, depth+1);

  for(i = 0, (iter = raptor_new_avltree_iterator(subject->properties, NULL, NULL, 1));
      iter && !rv;
      i++, (rv = raptor_avltree_iterator_next(iter))) {
    raptor_abbrev_node** nodes;
    raptor_abbrev_node* predicate;
    raptor_abbrev_node* object;
    raptor_qname *qname;

    nodes = (raptor_abbrev_node**)raptor_avltree_iterator_get(iter);
    if(!nodes)
      break;
    predicate= nodes[0];
    object= nodes[1];

    if(!(last_predicate && raptor_abbrev_node_equals(predicate, last_predicate))) {
      /* no object list abbreviation possible, terminate last object */
      if(last_predicate) {
        raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" ;", 2);
        raptor_turtle_writer_newline(turtle_writer);
      }

      qname = raptor_new_qname_from_namespace_uri(context->nstack,
                                                  predicate->term->value.uri,
                                                  10);

      if(raptor_abbrev_node_equals(predicate, context->rdf_type))
        raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"a", 1);
      else if(qname)
        raptor_turtle_writer_qname(turtle_writer, qname);
      else
        raptor_turtle_writer_reference(turtle_writer, predicate->term->value.uri);

      raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" ", 1);
    
      if(qname)
        raptor_free_qname(qname);
    } else
      raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)", ", 2);


    switch(object->term->type) {
      case RAPTOR_TERM_TYPE_URI:
        rv = raptor_turtle_emit_resource(serializer, object, depth+1);
        break;
          
      case RAPTOR_TERM_TYPE_LITERAL:
        rv = raptor_turtle_emit_literal(serializer, object, depth+1);
        break;
          
      case RAPTOR_TERM_TYPE_BLANK:
        rv = raptor_turtle_emit_blank(serializer, object, depth+1);
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

    last_predicate = predicate;
  }

  if(iter)
    raptor_free_avltree_iterator(iter);
         
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
                           raptor_abbrev_subject* subject,
                           int depth) 
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_turtle_writer* turtle_writer = context->turtle_writer;
  int blank = 1;
  int collection = 0;
  int rc = 0;
  
  if(!raptor_abbrev_subject_valid(subject)) return 0;

  RAPTOR_DEBUG_ABBREV_NODE("Emitting subject node", subject->node);

  if(!depth &&
     subject->node->term->type == RAPTOR_TERM_TYPE_BLANK &&
     subject->node->count_as_subject == 1 &&
     subject->node->count_as_object == 1) {
    RAPTOR_DEBUG_ABBREV_NODE("Skipping subject node - subj & obj count 1", subject->node);
    return 0;
  }
  
  if(raptor_avltree_size(subject->properties) == 0) {
    RAPTOR_DEBUG_ABBREV_NODE("Skipping subject node - no props", subject->node);
    return 0;
  }

  /* check if we can do collection abbreviation */
  if(raptor_avltree_size(subject->properties) >= 2) {
    raptor_avltree_iterator* iter = NULL;
    raptor_abbrev_node* pred1;
    raptor_abbrev_node* pred2;

    iter = raptor_new_avltree_iterator(subject->properties, NULL, NULL, 1);
    if(!iter)
      return 1;
    pred1 = ((raptor_abbrev_node**)raptor_avltree_iterator_get(iter))[0];
    if(raptor_avltree_iterator_next(iter)) {
      raptor_free_avltree_iterator(iter);
      return 1;
    }
    pred2 = ((raptor_abbrev_node**)raptor_avltree_iterator_get(iter))[0];
    raptor_free_avltree_iterator(iter);

    if(pred1->term->type == RAPTOR_TERM_TYPE_URI &&
       pred2->term->type == RAPTOR_TERM_TYPE_URI &&
       (
        (raptor_uri_equals(pred1->term->value.uri, context->rdf_first_uri) &&
         raptor_uri_equals(pred2->term->value.uri, context->rdf_rest_uri))
        ||
        (raptor_uri_equals(pred2->term->value.uri, context->rdf_first_uri) &&
         raptor_uri_equals(pred1->term->value.uri, context->rdf_rest_uri))
        )
       ) {
      collection = 1;
    }
  }

  /* emit the subject node */
  if(subject->node->term->type == RAPTOR_TERM_TYPE_URI) {
    rc= raptor_turtle_emit_resource(serializer, subject->node, depth+1);
    if(rc)
      return rc;
    blank = 0;
    collection = 0;
    
  } else if(subject->node->term->type == RAPTOR_TERM_TYPE_BLANK) {
    if((subject->node->count_as_subject == 1 && 
        subject->node->count_as_object == 0) && depth > 1) { 
      blank = 1;
    } else if(subject->node->count_as_object == 0) {
      raptor_turtle_writer_raw_counted(turtle_writer,
                                       (const unsigned char*)"[]", 2);
      blank = 0;
    } else if(!collection && subject->node->count_as_object > 1) {
      /* Referred to (used as an object), so needs a nodeID */
      raptor_turtle_writer_raw_counted(turtle_writer,
                                       (const unsigned char*)"_:", 2);
      raptor_turtle_writer_raw_counted(turtle_writer,
                                       subject->node->term->value.blank.string,
                                       subject->node->term->value.blank.string_len);
    }
  } 

  if(collection) {
    raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"(", 1);
    
    raptor_turtle_writer_increase_indent(turtle_writer);
    
    rc = raptor_turtle_emit_subject_collection_items(serializer, subject, depth+1);
    
    raptor_turtle_writer_decrease_indent(turtle_writer);
    raptor_turtle_writer_newline(turtle_writer);
    
    raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)")", 1);

  } else {
    if(blank && depth > 1)
      raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"[", 1);
    
    raptor_turtle_writer_increase_indent(turtle_writer);
    raptor_turtle_writer_newline(turtle_writer);

    raptor_turtle_emit_subject_properties(serializer, subject, depth+1);
    
    raptor_turtle_writer_decrease_indent(turtle_writer);
    
    if(blank && depth > 1) {
      raptor_turtle_writer_newline(turtle_writer);
      raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"]", 1);
    }
  }

  if(depth == 0) {
    /* NOTE: the space before the . here MUST be there or statements
     * that end in a numeric literal will be interpreted incorrectly
     * (the "." will be parsed as part of the literal and statement
     * left unterminated) 
     */
    raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" .", 2);
    raptor_turtle_writer_newline(turtle_writer);
    raptor_turtle_writer_newline(turtle_writer);
  }

  return rc;
}


/*
 * raptor_turtle_emit:
 * @serializer: #raptor_serializer object
 * 
 * Emit Turtle for all stored triples.
 * 
 * Return value: non-0 on failure
 **/
static int
raptor_turtle_emit(raptor_serializer *serializer)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_abbrev_subject* subject;
  raptor_abbrev_subject* blank;
  int rc;
  raptor_avltree_iterator* iter = NULL;

  iter = raptor_new_avltree_iterator(context->subjects, NULL, NULL, 1);
  while(iter) {
    subject = (raptor_abbrev_subject *)raptor_avltree_iterator_get(iter);
    if(subject) {
      rc = raptor_turtle_emit_subject(serializer, subject, 0);
      if(rc) {
        raptor_free_avltree_iterator(iter);
        return rc;
      }
    }
    if(raptor_avltree_iterator_next(iter)) break;
  }
  if(iter) raptor_free_avltree_iterator(iter);

  /* Emit any remaining blank nodes. */
  iter = raptor_new_avltree_iterator(context->blanks, NULL, NULL, 1);
  while(iter) {
    blank = (raptor_abbrev_subject *)raptor_avltree_iterator_get(iter);
    if(blank) {
      rc = raptor_turtle_emit_subject(serializer, blank, 0);
      if(rc) {
        raptor_free_avltree_iterator(iter);
        return rc;
      }
    }
    if(raptor_avltree_iterator_next(iter)) break;
  }
  if(iter) raptor_free_avltree_iterator(iter);

  return 0;
}


/*
 * raptor serializer Turtle implementation
 */


/* create a new serializer */
static int
raptor_turtle_serialize_init(raptor_serializer* serializer, const char *name)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_uri *rdf_type_uri;

  context->nstack = raptor_new_namespaces(serializer->world, 1);
  if(!context->nstack)
    return 1;
  context->rdf_nspace = raptor_new_namespace(context->nstack,
                                             (const unsigned char*)"rdf",
                                             (const unsigned char*)raptor_rdf_namespace_uri,
                                              0);

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

  rdf_type_uri = raptor_new_uri_for_rdf_concept(serializer->world,
                                                (const unsigned char*)"type");
  if(rdf_type_uri) {
    raptor_term* uri_term;
    uri_term = raptor_new_term_from_uri(serializer->world,
                                        rdf_type_uri);
    raptor_free_uri(rdf_type_uri);
    context->rdf_type = raptor_new_abbrev_node(serializer->world, uri_term);
    raptor_free_term(uri_term);
  } else
    context->rdf_type = NULL;

  context->rdf_xml_literal_uri = raptor_new_uri(serializer->world, raptor_xml_literal_datatype_uri_string);
  context->rdf_first_uri = raptor_new_uri(serializer->world, (const unsigned char*)"http://www.w3.org/1999/02/22-rdf-syntax-ns#first");
  context->rdf_rest_uri = raptor_new_uri(serializer->world, (const unsigned char*)"http://www.w3.org/1999/02/22-rdf-syntax-ns#rest");
  context->rdf_nil_uri = raptor_new_uri(serializer->world, (const unsigned char*)"http://www.w3.org/1999/02/22-rdf-syntax-ns#nil");

  if(!context->rdf_nspace || !context->namespaces ||
     !context->subjects || !context->blanks || !context->nodes ||
     !context->rdf_xml_literal_uri || !context->rdf_first_uri ||
     !context->rdf_rest_uri || !context->rdf_nil_uri || !context->rdf_type)
  {
    raptor_turtle_serialize_terminate(serializer);
    return 1;
  }

  /* Note: item 0 in the list is rdf:RDF's namespace */
  if(raptor_sequence_push(context->namespaces, context->rdf_nspace)) {
    raptor_turtle_serialize_terminate(serializer);
    return 1;
  }

  return 0;
}


/* destroy a serializer */
static void
raptor_turtle_serialize_terminate(raptor_serializer* serializer)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;

  if(context->turtle_writer) {
    raptor_free_turtle_writer(context->turtle_writer);
    context->turtle_writer = NULL;
  }

  if(context->rdf_nspace) {
    raptor_free_namespace(context->rdf_nspace);
    context->rdf_nspace = NULL;
  }

  if(context->namespaces) {
    int i;

    /* Note: item 0 in the list is rdf:RDF's namespace and freed above */
    for(i = 1; i< raptor_sequence_size(context->namespaces); i++) {
      raptor_namespace* ns;
      ns =(raptor_namespace*)raptor_sequence_get_at(context->namespaces, i);
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

  if(context->nstack) {
    raptor_free_namespaces(context->nstack);
    context->nstack = NULL;
  }

  if(context->rdf_type) {
    raptor_free_abbrev_node(context->rdf_type);
    context->rdf_type = NULL;
  }

  if(context->rdf_xml_literal_uri) {
    raptor_free_uri(context->rdf_xml_literal_uri);
    context->rdf_xml_literal_uri = NULL;
  }

  if(context->rdf_first_uri) {
    raptor_free_uri(context->rdf_first_uri);
    context->rdf_first_uri = NULL;
  }

  if(context->rdf_rest_uri) {
    raptor_free_uri(context->rdf_rest_uri);
    context->rdf_rest_uri = NULL;
  }

  if(context->rdf_nil_uri) {
    raptor_free_uri(context->rdf_nil_uri);
    context->rdf_nil_uri = NULL;
  }
}


#define TURTLE_NAMESPACE_DEPTH 0

/* add a namespace */
static int
raptor_turtle_serialize_declare_namespace_from_namespace(raptor_serializer* serializer, 
                                                         raptor_namespace *nspace)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
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
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_namespace *ns;
  int rc;
  
  ns = raptor_new_namespace_from_uri(context->nstack, prefix, uri, 
                                   TURTLE_NAMESPACE_DEPTH);

  rc = raptor_turtle_serialize_declare_namespace_from_namespace(serializer, ns);
  raptor_free_namespace(ns);
  
  return rc;
}


/* start a serialize */
static int
raptor_turtle_serialize_start(raptor_serializer* serializer)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_turtle_writer* turtle_writer;
  int flag;
  
  if(context->turtle_writer)
    raptor_free_turtle_writer(context->turtle_writer);

  flag = RAPTOR_OPTIONS_GET_NUMERIC(serializer, RAPTOR_OPTION_WRITE_BASE_URI);
  turtle_writer = raptor_new_turtle_writer(serializer->world,
                                           serializer->base_uri,
                                           flag,
                                           context->nstack,
                                           serializer->iostream);
  if(!turtle_writer)
    return 1;

  raptor_turtle_writer_set_option(turtle_writer,
                                  RAPTOR_OPTION_WRITER_AUTO_INDENT, 1);
  raptor_turtle_writer_set_option(turtle_writer,
                                  RAPTOR_OPTION_WRITER_INDENT_WIDTH, 2);
  
  context->turtle_writer = turtle_writer;

  return 0;
}

static void
raptor_turtle_ensure_writen_header(raptor_serializer* serializer,
                                   raptor_turtle_context* context) 
{
  int i;

  if(context->written_header)
    return;
  
  if(!context->turtle_writer)
    return;

  for(i = 0; i< raptor_sequence_size(context->namespaces); i++) {
    raptor_namespace* ns;
    ns = (raptor_namespace*)raptor_sequence_get_at(context->namespaces, i);
    raptor_turtle_writer_namespace_prefix(context->turtle_writer, ns);
    raptor_namespace_stack_start_namespace(context->nstack, ns, 0);
  }
  
  raptor_turtle_writer_newline(context->turtle_writer);

  context->written_header = 1;
}

/* serialize a statement */
static int
raptor_turtle_serialize_statement(raptor_serializer* serializer, 
                                  raptor_statement *statement)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_abbrev_subject* subject = NULL;
  raptor_abbrev_node* predicate = NULL;
  raptor_abbrev_node* object = NULL;
  int rv;
  raptor_term_type object_type;

  if(!(statement->subject->type == RAPTOR_TERM_TYPE_URI ||
       statement->subject->type == RAPTOR_TERM_TYPE_BLANK)) {
    raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                               "Do not know how to serialize node type %d",
                               statement->subject->type);
    return 1;
  }  

  subject = raptor_abbrev_subject_lookup(context->nodes, context->subjects,
                                         context->blanks,
                                         statement->subject);
  if(!subject) {
    return 1;
  }

  object_type = statement->object->type;

  if(!(object_type == RAPTOR_TERM_TYPE_URI ||
       object_type == RAPTOR_TERM_TYPE_BLANK ||
       object_type == RAPTOR_TERM_TYPE_LITERAL)) {
    raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                               "Cannot serialize a triple with object node type %d",
                               object_type);
    return 1;
  }

  object = raptor_abbrev_node_lookup(context->nodes, statement->object);
  if(!object)
    return 1;          


  if(statement->predicate->type == RAPTOR_TERM_TYPE_URI) {
    predicate = raptor_abbrev_node_lookup(context->nodes, statement->predicate);
    if(!predicate)
      return 1;
	      
    rv = raptor_abbrev_subject_add_property(subject, predicate, object);
    if(rv < 0) {
      raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                                 "Unable to add properties to subject %p",
                                 subject);
      return rv;
    }
  
  } else {
    raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                               "Do not know how to serialize node type %d",
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
raptor_turtle_serialize_end(raptor_serializer* serializer)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  
  raptor_turtle_ensure_writen_header(serializer, context);
  
  raptor_turtle_emit(serializer);  

  /* reset serializer for reuse */
  context->written_header = 0;

  return 0;
}


/* finish the serializer factory */
static void
raptor_turtle_serialize_finish_factory(raptor_serializer_factory* factory)
{
  /* NOP */
}


static const char* const turtle_names[2] = { "turtle", NULL};

static const char* const turtle_uri_strings[3] = {
  "http://www.w3.org/ns/formats/Turtle",
  "http://www.dajobe.org/2004/01/turtle/",
  NULL
};
  
#define TURTLE_TYPES_COUNT 6
static const raptor_type_q turtle_types[TURTLE_TYPES_COUNT + 1] = {
  { "text/turtle", 11, 10},
  { "application/turtle", 18, 10},
  { "application/x-turtle", 20, 8},
  { "text/n3", 7, 3},
  { "text/rdf+n3", 11, 3},
  { "application/rdf+n3", 18, 3},
  { NULL, 0, 0}
};

static int
raptor_turtle_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->desc.names = turtle_names;
  factory->desc.mime_types = turtle_types;

  factory->desc.label = "Turtle Terse RDF Triple Language";
  factory->desc.uri_strings = turtle_uri_strings;
  
  factory->context_length     = sizeof(raptor_turtle_context);
  
  factory->init                = raptor_turtle_serialize_init;
  factory->terminate           = raptor_turtle_serialize_terminate;
  factory->declare_namespace   = raptor_turtle_serialize_declare_namespace;
  factory->declare_namespace_from_namespace   = raptor_turtle_serialize_declare_namespace_from_namespace;
  factory->serialize_start     = raptor_turtle_serialize_start;
  factory->serialize_statement = raptor_turtle_serialize_statement;
  factory->serialize_end       = raptor_turtle_serialize_end;
  factory->finish_factory      = raptor_turtle_serialize_finish_factory;

  return 0;
}


int
raptor_init_serializer_turtle(raptor_world* world)
{
  return !raptor_serializer_register_factory(world,
                                             &raptor_turtle_serializer_register_factory);
}



/**
 * raptor_uri_turtle_write:
 * @world: world
 * @iostr: iostream for writing
 * @uri: uri
 * @nstack: namespace stack
 * @base_uri: base URI
 *
 * Write #raptor_uri to a stream in turtle syntax (using QNames).
 *
 * Note: This creates and destroys several internal objects for each
 * call so for more efficient writing, create a turtle serializer.
 *
 * Return value: non-0 on failure
 */
int
raptor_uri_turtle_write(raptor_world *world,
                        raptor_iostream* iostr,
                        raptor_uri* uri, 
                        raptor_namespace_stack *nstack,
                        raptor_uri *base_uri)
{
  int rc;
  raptor_turtle_writer* turtle_writer;
  
  turtle_writer = raptor_new_turtle_writer(world, base_uri, 0, nstack, iostr);
  if(!turtle_writer)
    return 1;

  rc = raptor_turtle_writer_uri(turtle_writer, uri);
  
  raptor_free_turtle_writer(turtle_writer);
  
  return rc;
}



/**
 * raptor_term_turtle_write:
 * @iostr: iostream for writing
 * @term: term
 * @nstack: namespace stack
 * @base_uri: base URI
 *
 * Write #raptor_term to a stream in turtle syntax (using QNames).
 *
 * Note: This creates and destroys several internal objects for each
 * call so for more efficient writing, create a turtle serializer.
 *
 * Return value: non-0 on failure
 */
int
raptor_term_turtle_write(raptor_iostream* iostr,
                         raptor_term* term, 
                         raptor_namespace_stack *nstack,
                         raptor_uri *base_uri)
{
  int rc;
  raptor_turtle_writer* turtle_writer;
  
  turtle_writer = raptor_new_turtle_writer(term->world, base_uri, 0, nstack,
                                           iostr);
  if(!turtle_writer)
    return 1;

  rc = raptor_turtle_writer_term(turtle_writer, term);
  
  raptor_free_turtle_writer(turtle_writer);
  
  return rc;
}



/**
 * raptor_uri_to_turtle_counted_string:
 * @world: world
 * @uri: uri
 * @nstack: namespace stack
 * @base_uri: base URI
 * @len_p: Pointer to location to store length of new string (if not NULL)
 *
 * Convert #raptor_uri to a string.
 * Caller has responsibility to free the string.
 *
 * Note: This creates and destroys several internal objects for each
 * call so for more efficient writing, create a turtle serializer.
 *
 * Return value: the new string or NULL on failure.  The length of
 * the new string is returned in *@len_p if len_p is not NULL.
 */
unsigned char*
raptor_uri_to_turtle_counted_string(raptor_world *world,
                                    raptor_uri* uri, 
                                    raptor_namespace_stack *nstack,
                                    raptor_uri *base_uri,
                                    size_t *len_p)
{
  int rc = 1;
  raptor_iostream* iostr;
  unsigned char *s = NULL;
  raptor_turtle_writer* turtle_writer;

  iostr = raptor_new_iostream_to_string(world,
                                        (void**)&s, len_p, malloc);
  if(!iostr)
    return NULL;
  
  turtle_writer = raptor_new_turtle_writer(world, base_uri, 0, nstack, iostr);
  if(!turtle_writer)
    goto tidy;

  rc = raptor_turtle_writer_uri(turtle_writer, uri);

  raptor_free_turtle_writer(turtle_writer);

  tidy:
  raptor_free_iostream(iostr);

  if(rc) {
    free(s);
    s = NULL;
  }
  
  return s;
}

/**
 * raptor_uri_to_turtle_string:
 * @world: world
 * @uri: uri
 * @nstack: namespace stack
 * @base_uri: base URI
 *
 * Convert #raptor_uri to a string.
 * Caller has responsibility to free the string.
 *
 * Note: This creates and destroys several internal objects for each
 * call so for more efficient writing, create a turtle serializer.
 *
 * Return value: the new string or NULL on failure.
 */
unsigned char*
raptor_uri_to_turtle_string(raptor_world *world,
                            raptor_uri* uri, 
                            raptor_namespace_stack *nstack,
                            raptor_uri *base_uri)
{
  return raptor_uri_to_turtle_counted_string(world, uri, nstack, base_uri, NULL);
}



/**
 * raptor_term_to_turtle_counted_string:
 * @term: term
 * @nstack: namespace stack
 * @base_uri: base URI
 * @len_p: Pointer to location to store length of new string (if not NULL)
 *
 * Convert #raptor_term to a string.
 * Caller has responsibility to free the string.
 *
 * Note: This creates and destroys several internal objects for each
 * call so for more efficient writing, create a turtle serializer.
 *
 * See also raptor_term_to_counted_string() which writes in simpler
 * N-Triples with no Turtle abbreviated forms, and is quicker.
 *
 * Return value: the new string or NULL on failure.  The length of
 * the new string is returned in *@len_p if len_p is not NULL.
 */
unsigned char*
raptor_term_to_turtle_counted_string(raptor_term* term, 
                                     raptor_namespace_stack *nstack,
                                     raptor_uri *base_uri,
                                     size_t *len_p)
{
  int rc;
  raptor_iostream* iostr;
  unsigned char *s;
  iostr = raptor_new_iostream_to_string(term->world,
                                        (void**)&s, len_p, malloc);
  if(!iostr)
    return NULL;

  rc = raptor_term_turtle_write(iostr, term, nstack, base_uri);

  raptor_free_iostream(iostr);
  if(rc) {
    free(s);
    s = NULL;
  }
  
  return s;
}

/**
 * raptor_term_to_turtle_string:
 * @term: term
 * @nstack: namespace stack
 * @base_uri: base URI
 *
 * Convert #raptor_term to a string.
 * Caller has responsibility to free the string.
 *
 * See also raptor_term_to_counted_string() which writes in simpler
 * N-Triples with no Turtle abbreviated forms, and is quicker.
 *
 * Return value: the new string or NULL on failure.
 */
unsigned char*
raptor_term_to_turtle_string(raptor_term* term, 
                             raptor_namespace_stack *nstack,
                             raptor_uri *base_uri)
{
  return raptor_term_to_turtle_counted_string(term, nstack, base_uri, NULL);
}

