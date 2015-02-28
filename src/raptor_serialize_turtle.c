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
  raptor_namespace *mkr_nspace;         /* the mkr: namespace */
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

  /* Flags for turtle writer */
  int turtle_writer_flags;

  /* non zero if header is finished being written
   * (and thus no new namespaces can be declared).
   */
  int written_header;


  /* for labeling namespaces */

  int namespace_count;

  /***** variables for mKR serializer *****/
  /* Non 0 for mKR serializer */
  int emit_mkr;

  /* Non 0 to emit mKR blank subject */
  int mkr_blank;

  /* Non 0 for rs:ResultSet */
  int resultset;

  /* URI of mkr:has */
  raptor_uri* mkr_has_uri;

  /* URI of mkr:isu */
  raptor_uri* mkr_isu_uri;

  /* URI of mkr:do */
  raptor_uri* mkr_do_uri;

  /* URI of mkr:is */
  raptor_uri* mkr_is_uri;

  /* URI of rs:ResultSet */
  raptor_uri* rs_ResultSet_uri;

  /* URI of rs:resultVariable */
  raptor_uri* rs_resultVariable_uri;

  /* Non 0 if "begin relation result ;" has been written */
  int written_begin;

  /* state for raptor_mkr_emit_subject_collection_items()
           and mkr_new_sequence_from_rdflist() */
  mkr_type mkrtype;

  /* state for raptor_mkr_emit_subject_properties()
           and raptor_mkr_emit_po() */
  int has_count;
  int indent_count;
  int curly_count;

  /* state for raptor_mkr_emit_subject_resultset() */
  int mkr_rs_size;
  int mkr_rs_arity;
  int mkr_rs_ntuple;
  int mkr_rs_nvalue;
  int mkr_rs_processing_value;
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

static int raptor_mkr_emit_subject_collection_items(raptor_serializer* serializer,
                                                    raptor_abbrev_subject* subject,
                                                    int depth);
mkr_type mkr_get_gtype(raptor_serializer* serializer, raptor_term* gterm);
raptor_sequence* mkr_new_sequence_from_rdflist(raptor_serializer* serializer,
                                               raptor_abbrev_subject* subject,
                                               int depth);
static int raptor_mkr_emit_subject_properties(raptor_serializer *serializer,
                                              raptor_abbrev_subject* subject,
                                              int depth);
static int raptor_mkr_emit_po(raptor_serializer* serializer,
                              raptor_abbrev_node* predicate,
                              raptor_sequence* objectSequence,
                              int depth);
static int raptor_mkr_emit_subject_resultset(raptor_serializer* serializer,
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

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("\n##### DEBUG: raptor_turtle_is_legal_turtle_qname: ");
#endif

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

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("prefix_name = (%s) ok", prefix_name);
#endif

  local_name = (const char*)qname->local_name;
  if(local_name) {
    /* nameStartChar: must have leading [A-Z][a-z][0-9]_  */
    /* nameChar: no . anywhere */
    if(!(isalpha((int)*local_name) || isdigit((int)*local_name) || *local_name == '_') ||
       strchr(local_name, '.'))
      return 0;
  }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf(" local_name = (%s) ok\n", local_name);
#endif

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
  int emit_mkr = context->emit_mkr;

  raptor_qname* qname = NULL;

  RAPTOR_DEBUG_ABBREV_NODE("Emitting resource node", node);

  if(node->term->type != RAPTOR_TERM_TYPE_URI)
    return 1;

  if(raptor_uri_equals(node->term->value.uri, context->rdf_nil_uri)) {
    raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"( )", 3);
    return 0;
  }

  if(emit_mkr) {
    qname = mkr_new_qname_from_namespace_uri(context->nstack,
                                                node->term->value.uri, 10);
  } else {
    qname = raptor_new_qname_from_namespace_uri(context->nstack,
                                                node->term->value.uri, 10);
    /* XML Names allow leading '_' and '.' anywhere but Turtle does not */
    if(qname && !raptor_turtle_is_legal_turtle_qname(qname)) {
      raptor_free_qname(qname);
      qname = NULL;
    }
  }

  if(raptor_uri_equals(node->term->value.uri, context->rdf_nil_uri)) {
    raptor_turtle_writer_raw_counted(turtle_writer ,(const unsigned char*)"( )", 3);
    return 0;
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
                                   NULL, "Triple has unsupported term type %u",
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
  int emit_mkr = context->emit_mkr;
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
    predicate = nodes[0];
    object = nodes[1];


    if(!raptor_uri_equals(predicate->term->value.uri,
                          context->rdf_first_uri)) {
      raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                       "Malformed collection - first predicate is not rdf:first");
      raptor_free_avltree_iterator(iter);
      return 1;
    }

    if(!object)
      continue;

    if(i > 0) {
      if(emit_mkr)
        raptor_turtle_writer_raw_counted(context->turtle_writer,
                                         (const unsigned char*)", ", 2);
      else
        raptor_turtle_writer_newline(context->turtle_writer);
    }

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
                                   NULL, "Triple has unsupported term type %u",
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
      if((object->term->type != RAPTOR_TERM_TYPE_URI) ||
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
 * mkr_get_gtype
 * @serializer: #raptor_serializer object
 * @gterm: object term of isu or rdf:type
 *
 * get mkr_type for this group
 *
 * Return value: MKR_UNKNOWN on failure
 **/
mkr_type
mkr_get_gtype(raptor_serializer* serializer, raptor_term* gterm)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  mkr_type mkrtype = MKR_UNKNOWN;
  raptor_qname* qname = NULL;
  char* local = NULL;

  if(!gterm)
    return mkrtype;

  qname = mkr_new_qname_from_term(context->nstack, gterm);
  if(!qname)
    return mkrtype;
  local = (char*)qname->local_name;
  if(!local)
    return mkrtype;

  if(!strcmp(local, "list"))
    mkrtype = MKR_LIST;
  else if(!strcmp(local, "set"))
    mkrtype = MKR_SET;
  else if(!strcmp(local, "view"))
    mkrtype = MKR_VIEW;
  else if(!strcmp(local, "graph"))
    mkrtype = MKR_GRAPH;
  else if(!strcmp(local, "hierarchy"))
    mkrtype = MKR_HIERARCHY;
  else if(!strcmp(local, "relation"))
    mkrtype = MKR_RELATION;

  return mkrtype;
}

/*
 * mkr_new_sequence_from_rdflist
 * @serializer: #raptor_serializer object
 * @subject: subject node
 * @depth: depth into tree
 *
 * create a raptor_sequence of terms
 * from rdf collection of items (rdf:first, rdf:rest) about a subject node.
 *
 * Return value: NULL on failure
 **/
raptor_sequence*
mkr_new_sequence_from_rdflist(raptor_serializer* serializer,
                              raptor_abbrev_subject* subject,
                              int depth)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_avltree_iterator* iter = NULL;
  int rv = 0;
  int i;
  int is_new_subject = 0;

  raptor_sequence* seq = NULL;

#if defined(RAPTOR_DEBUG)
  raptor_turtle_writer* turtle_writer = context->turtle_writer;
  raptor_turtle_writer_raw(turtle_writer, (unsigned char*)"##### DEBUG: ");
#endif

  RAPTOR_DEBUG_ABBREV_NODE("creating raptor_sequence from subject collection items", subject->node);

  seq = raptor_new_sequence((raptor_data_free_handler)raptor_free_abbrev_node,
                            (raptor_data_print_handler)NULL);

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
    predicate = nodes[0];
    object = nodes[1];

    if(!raptor_uri_equals(predicate->term->value.uri, context->mkr_isu_uri)) {
      context->mkrtype = mkr_get_gtype(serializer, object->term);
    } else if(!raptor_uri_equals(predicate->term->value.uri, context->rdf_type->term->value.uri)) {
      context->mkrtype = mkr_get_gtype(serializer, object->term);
    } else if(!raptor_uri_equals(predicate->term->value.uri, context->rdf_first_uri)) {
      raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                       "Malformed collection - first predicate is not rdf:first");
      raptor_free_avltree_iterator(iter);
      return NULL;
    }

    if(!object)
      continue;


    switch(object->term->type) {
      case RAPTOR_TERM_TYPE_URI:
        raptor_sequence_push(seq, object);
        /* rv = raptor_turtle_emit_resource(serializer, object, depth+1); */
        break;

      case RAPTOR_TERM_TYPE_LITERAL:
        raptor_sequence_push(seq, object);
        /* rv = raptor_turtle_emit_literal(serializer, object, depth+1); */
        break;

      case RAPTOR_TERM_TYPE_BLANK:
        raptor_sequence_push(seq, object);
        /* rv = raptor_turtle_emit_blank(serializer, object, depth+1); */
        break;

      case RAPTOR_TERM_TYPE_UNKNOWN:
      default:
        raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR,
                                   NULL, "Triple has unsupported term type %u",
                                   object->term->type);
        break;
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
      return NULL;
    }

    if(object->term->type == RAPTOR_TERM_TYPE_BLANK) {
      subject = raptor_abbrev_subject_find(context->blanks, object->term);

      if(!subject) {
        raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                         "Malformed collection - could not find subject for rdf:rest");
        raptor_free_avltree_iterator(iter);
        return NULL;
      }

      /* got a <(old)subject> rdf:rest <(new)subject> triple so know
       * subject has changed and should reset the properties iterator
       */
      if(iter)
        raptor_free_avltree_iterator(iter);
      iter = raptor_new_avltree_iterator(subject->properties, NULL, NULL, 1);
      is_new_subject = 1;

    } else {
      if((object->term->type != RAPTOR_TERM_TYPE_URI) ||
         !raptor_uri_equals(object->term->value.uri, context->rdf_nil_uri)) {
        raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                         "Malformed collection - last rdf:rest resource is not rdf:nil");
        raptor_free_avltree_iterator(iter);
        return NULL;
      }
      break;
    }
  }

  if(iter)
    raptor_free_avltree_iterator(iter);

#if defined(RAPTOR_DEBUG)
  raptor_sequence_print(seq, stdout);
#endif
  return seq;
}


/*
 * raptor_mkr_emit_subject_collection_items:
 * @serializer: #raptor_serializer object
 * @subject: subject node
 * @depth: depth into tree
 *
 * Emit an abbreviated rdf collection of items (rdf:first, rdf:rest) about a subject node.
 *
 * Return value: non-0 on failure
 **/
static int
raptor_mkr_emit_subject_collection_items(raptor_serializer* serializer,
                                            raptor_abbrev_subject* subject,
                                            int depth)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_turtle_writer* turtle_writer = context->turtle_writer;
  raptor_qname* qname = NULL;
  int rv = 0;
  raptor_avltree_iterator* iter = NULL;
  int i;
  int is_new_subject = 0;

  mkr_type mkrtype = MKR_UNKNOWN;
  raptor_sequence* seq = NULL;
  int size = 0;

  RAPTOR_DEBUG_ABBREV_NODE("Emitting subject collection items", subject->node);

  seq = mkr_new_sequence_from_rdflist(serializer, subject, depth);
  size = raptor_sequence_size(seq);

  context->mkrtype = subject->node->term->mkrtype;

  /* write type begin string */
  if(size && (size > 1))
    switch(context->mkrtype) {
      case MKR_LIST:  raptor_turtle_writer_raw_counted(turtle_writer, (unsigned char*)"(", 1); break;
      case MKR_SET:   raptor_turtle_writer_raw_counted(turtle_writer, (unsigned char*)"[", 1); break;
      case MKR_VIEW:  raptor_turtle_writer_raw_counted(turtle_writer, (unsigned char*)"{(", 2); break;
      case MKR_GRAPH: raptor_turtle_writer_raw_counted(turtle_writer, (unsigned char*)"{[", 2); break;

      default:  raptor_turtle_writer_raw_counted(turtle_writer, (unsigned char*)"(", 1); break;
    }

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
    predicate = nodes[0];
    object = nodes[1];

    if(!raptor_uri_equals(predicate->term->value.uri, context->mkr_isu_uri)) {
      context->mkrtype = mkr_get_gtype(serializer, object->term);
    } else if(!raptor_uri_equals(predicate->term->value.uri, context->rdf_type->term->value.uri)) {
      context->mkrtype = mkr_get_gtype(serializer, object->term);
    } else if(!raptor_uri_equals(predicate->term->value.uri, context->rdf_first_uri)) {
      raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                       "Malformed collection - first predicate is not rdf:first");
      raptor_free_avltree_iterator(iter);
      return NULL;
    }

    if(!object)
      continue;

    if(i > 0)
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
                                   NULL, "Triple has unsupported term type %u",
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
      if((object->term->type != RAPTOR_TERM_TYPE_URI) ||
         !raptor_uri_equals(object->term->value.uri, context->rdf_nil_uri)) {
        raptor_log_error(serializer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                         "Malformed collection - last rdf:rest resource is not rdf:nil");
        raptor_free_avltree_iterator(iter);
        return 1;
      }
      break;
    }


    /* last item */
  }

  context->mkrtype = subject->node->term->mkrtype;

  /* write type end string */
  if(size && (size > 1))
    switch(context->mkrtype) {
      case MKR_LIST:  raptor_turtle_writer_raw_counted(turtle_writer, (unsigned char*)")", 1); break;
      case MKR_SET:   raptor_turtle_writer_raw_counted(turtle_writer, (unsigned char*)"]", 1); break;
      case MKR_VIEW:  raptor_turtle_writer_raw_counted(turtle_writer, (unsigned char*)")}", 2); break;
      case MKR_GRAPH: raptor_turtle_writer_raw_counted(turtle_writer, (unsigned char*)"]}", 2); break;

      default:  raptor_turtle_writer_raw_counted(turtle_writer, (unsigned char*)")", 1); break;
    }

  if(qname)
    raptor_free_qname(qname);

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
    predicate = nodes[0];
    object = nodes[1];

    if(!last_predicate ||
       !raptor_abbrev_node_equals(predicate, last_predicate)) {
      /* first predicate or same predicate as last time */

      /* no object list abbreviation possible, terminate last object */
      if(last_predicate) {
        raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" ;", 2);
        raptor_turtle_writer_newline(turtle_writer);
      }

      qname = raptor_new_qname_from_namespace_uri(context->nstack,
                                                  predicate->term->value.uri,
                                                  10);

      if(raptor_abbrev_node_equals(predicate, context->rdf_type)) {
        raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"a", 1);
      } else if(qname) {
        raptor_turtle_writer_qname(turtle_writer, qname);
      } else {
        raptor_turtle_writer_reference(turtle_writer, predicate->term->value.uri);
      }
      raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" ", 1);


      if(qname)
        raptor_free_qname(qname);
    } else { /* not last object for this predicate */
      raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)", ", 2);
    }


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
                                   NULL, "Triple has unsupported term type %u",
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


/*********************************************************************************/

/*
 * raptor_mkr_emit_subject_properties:
 * @serializer: #raptor_serializer object
 * @subject: subject node
 * @depth: depth into tree
 *
 * Emit the properties about a subject node.
 * use object sequence
 *
 * Return value: non-0 on failure
 **/
static int
raptor_mkr_emit_subject_properties(raptor_serializer* serializer,
                                      raptor_abbrev_subject* subject,
                                      int depth)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_turtle_writer *turtle_writer = context->turtle_writer;
  raptor_avltree_iterator* iter = NULL;
  raptor_abbrev_node* last_predicate = NULL;
  raptor_sequence* objectSequence = NULL;
  int numpred = 0;
  int numobj = 0;
  int ri = 0;
  int rv = 0;
  int i;

  RAPTOR_DEBUG_ABBREV_NODE("Emitting mkr subject properties", subject->node);

  /* Emit any rdf:_n properties collected */
  if(raptor_sequence_size(subject->list_items) > 0)
    rv = raptor_turtle_emit_subject_list_items(serializer, subject, depth+1);

#if defined(RAPTOR_DEBUG)
  printf("\n##### DEBUG: raptor_mkr_emit_subject_properties: subject = ");
  raptor_term_print_as_ntriples(subject->node->term, stdout);
  printf("\n");
#endif

  /* context->indent_count++; */
  for(i = 0, (iter = raptor_new_avltree_iterator(subject->properties, NULL, NULL, 1));
      iter && !ri;
      i++, (ri = raptor_avltree_iterator_next(iter))) {
    raptor_abbrev_node** nodes;
    raptor_abbrev_node* predicate;
    raptor_abbrev_node* object;

    nodes = (raptor_abbrev_node**)raptor_avltree_iterator_get(iter);
    if(!nodes)
      break;
    predicate = nodes[0];
    object = nodes[1];

    /* create object sequence for each property */
    if(!last_predicate) {
      /* start first predicate */
      context->has_count = 0;
      numpred = 1;
      numobj = 1;
      objectSequence = raptor_new_sequence((raptor_data_free_handler)raptor_free_abbrev_node, NULL);
      raptor_sequence_push(objectSequence, object);

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
      printf("i=%u: numpred=%u: ", i, numpred); raptor_term_print_as_ntriples(predicate->term, stdout);
      printf(" numobj=%u: ", numobj); raptor_term_print_as_ntriples(object->term, stdout);
      printf("\n");
#endif

    } else if(raptor_abbrev_node_equals(last_predicate, predicate)) {
      numobj++;
      raptor_sequence_push(objectSequence, object);

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
      printf("i=%u: numpred=%u: ", i, numpred); raptor_term_print_as_ntriples(predicate->term, stdout);
      printf(" numobj=%u: ", numobj); raptor_term_print_as_ntriples(object->term, stdout);
      printf("\n");
#endif

    } else {

      /* emit next last_predicate */
      rv = raptor_mkr_emit_po(serializer, last_predicate, objectSequence, depth+1);

      if(!ri) { /* more predicates */
        raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)", ", 2);
        raptor_turtle_writer_newline(turtle_writer);
      }
      /* Return error if emitting something failed */
      if(rv)
        return rv;

      /* start next predicate */
      numpred++;
      objectSequence = raptor_new_sequence((raptor_data_free_handler)raptor_free_abbrev_node, NULL);
      numobj = 1;
      raptor_sequence_push(objectSequence, object);

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
      printf("i=%u: numpred=%u: ", i, numpred); raptor_term_print_as_ntriples(predicate->term, stdout);
      printf(" numobj=%u: ", numobj); raptor_term_print_as_ntriples(object->term, stdout);
      printf("\n");
#endif

    }
    last_predicate = predicate;
    /* continue processing this predicate */
  }

  /* emit the last last_predicate */
  rv = raptor_mkr_emit_po(serializer, last_predicate, objectSequence, depth+1);

  raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" ;", 2);

  if(context->curly_count > 0)
    raptor_turtle_writer_decrease_indent(turtle_writer);
  raptor_turtle_writer_newline(turtle_writer);

  /* Return error if emitting something failed */
  if(rv)
    return rv;

  if(iter)
    raptor_free_avltree_iterator(iter);
/*
  if(objectSequence)
    raptor_free_sequence(objectSequence); objectSequence = NULL;
*/

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("exit raptor_mkr_emit_subject_properties\n");
#endif
  return rv;
}

/*********************************************************************************/

static int
raptor_mkr_emit_po(raptor_serializer* serializer,
                   raptor_abbrev_node* predicate,
                   raptor_sequence* objectSequence,
                   int depth)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_turtle_writer *turtle_writer = context->turtle_writer;
  raptor_abbrev_node* object = NULL;
  raptor_qname *qname = NULL;
  char* local = NULL;
  int rv = 0;
  int size = 0;
  int j;
 
  if(!predicate) {
    printf("##### ERROR: raptor_mkr_emit_po: NULL predicate\n");
    rv = 1;
    return rv;
  } else if(predicate->term->type == RAPTOR_TERM_TYPE_UNKNOWN) {
    printf("##### ERROR: raptor_mkr_emit_po: predicate type RAPTOR_TERM_TYPE_UNKNOWN\n");
    rv = 2;
    return rv;
  }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: raptor_mkr_emit_po: predicate = ");
  raptor_term_print_as_ntriples(predicate->term, stdout);
  printf("\n");
#endif

  /* emit predicate */
  qname = mkr_new_qname_from_term(context->nstack, predicate->term);

  if(raptor_abbrev_node_equals(predicate, context->rdf_type)) {
    raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"isu", 3);

  } else if(qname) {
    local = (char*)qname->local_name;
    if(strcmp(local, "has") && strcmp(local, "do") && strcmp(local, "is") &&
       strcmp(local, "isu") && strcmp(local, "iss") && strcmp(local, "isa") &&
       strcmp(local, "isp") && strcmp(local, "isg") && strcmp(local, "isc") &&
       strcmp(local, "isa*") && strcmp(local, "isc*") &&
       strcmp(local, "at") && strcmp(local, "of") && strcmp(local, "with") &&
       strcmp(local, "od") && strcmp(local, "from") && strcmp(local, "to") &&
       strcmp(local, "in") && strcmp(local, "out") && strcmp(local, "where") &&
       strcmp(local, "ho") && strcmp(local, "rel") && strcmp(local, ":=")
       ) {
      /* not one of the above */ 
      context->has_count++;
      if(context->has_count == 1) {
        raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"has ", 4);
        /* raptor_turtle_writer_increase_indent(turtle_writer); */
      } else {
        raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"has ", 4);
      }
      raptor_turtle_writer_qname(turtle_writer, qname);
      raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" =", 2);
    } else {
      raptor_turtle_writer_qname(turtle_writer, qname);
    }

  } else {
    raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"has ", 4);
    raptor_turtle_writer_reference(turtle_writer, predicate->term->value.uri);
    raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" =", 2);
  }
  raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" ", 1);

  if(qname)
    raptor_free_qname(qname);
  
  /* emit object sequence for predicate */
  size = raptor_sequence_size(objectSequence);
  if((size > 1) && !context->curly_count)
    raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"[", 1);
  for(j = 0; j < size; j++) {
    object = (raptor_abbrev_node*)raptor_sequence_get_at(objectSequence, j);
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
                                   NULL, "Triple has unsupported term type %u",
                                   object->term->type);
        break;
    }
    if(j < size - 1)
      raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)", ", 2);
      
  }
  if((size > 1) && !context->curly_count)
    raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"]", 1);

/*
    if(objectSequence)
      raptor_free_sequence(objectSequence); objectSequence = NULL;
*/

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("exit raptor_mkr_emit_po\n");
#endif
  return rv;
}

/********************************************************************************/

/*
 * raptor_mkr_emit_subject_resultset:
 * @serializer: #raptor_serializer object
 * @subject: subject node
 * @depth: depth into tree
 *
 * Emit rs:ResultSet as CSV relation.
 *
 * Return value: non-0 on failure
 **/
static int
raptor_mkr_emit_subject_resultset(raptor_serializer* serializer,
                                  raptor_abbrev_subject* subject,
                                  int depth)
{
  raptor_turtle_context* context = (raptor_turtle_context*)serializer->context;
  raptor_turtle_writer *turtle_writer = context->turtle_writer;
  raptor_abbrev_node* last_predicate = NULL;
  raptor_avltree_iterator* iter = NULL;
  int skip_object;
  int rv = 0;
  int i;


  RAPTOR_DEBUG_ABBREV_NODE("Emitting subject resultset", subject->node);

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

    predicate = nodes[0];
    object = nodes[1];

    if(!last_predicate ||
       !raptor_abbrev_node_equals(predicate, last_predicate)) {
      /* first predicate or same predicate as last time */

      /* no object list abbreviation possible, terminate last object */
      if(last_predicate) {
        if(!context->mkr_rs_arity) {
          /* last variable in first row */
          raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" ;", 2);
          raptor_turtle_writer_newline(turtle_writer);
          context->mkr_rs_ntuple++; /* start count after variables */
	} else if(!context->mkr_rs_nvalue) {
          /* size not emitted */
        } else if(context->mkr_rs_processing_value &&
                  (context->mkr_rs_nvalue == context->mkr_rs_arity)) {
          /* previous value was last value of row */
          context->mkr_rs_processing_value = 0;
          raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" ;", 2);
          if(context->mkr_rs_ntuple == 1) {
            raptor_turtle_writer_decrease_indent(turtle_writer);
            raptor_turtle_writer_decrease_indent(turtle_writer);
          }
          raptor_turtle_writer_newline(turtle_writer);
          context->mkr_rs_nvalue = 0;
          context->mkr_rs_ntuple++;
          if(context->mkr_rs_ntuple > context->mkr_rs_size) {
            /* previous row was last row of table */
            raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"end relation result ;", 21);
            raptor_turtle_writer_newline(turtle_writer);
            break;
          }
        }
      }

      qname = raptor_new_qname_from_namespace_uri(context->nstack,
                                                  predicate->term->value.uri,
                                                  10);
      if(raptor_abbrev_node_equals(predicate, context->rdf_type)) {
        skip_object = 1;  /* all values have been written */
      } else if(qname) {
        /* check predicate name */
        if(!strcmp((const char*)qname->local_name, (const char*)"resultVariable")) {

          /* emit mKR relation header */
          raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" is result ;", 12);
          raptor_turtle_writer_newline(turtle_writer);
          raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"result is relation with format = csv ;", 38);
          raptor_turtle_writer_newline(turtle_writer);
          raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"begin relation result ;", 23);
          raptor_turtle_writer_newline(turtle_writer);
          skip_object = 0;

        } else if(!strcmp((const char*)qname->local_name, (const char*)"size")) {
          context->mkr_rs_arity = context->mkr_rs_nvalue;
          context->mkr_rs_nvalue = 0;
          skip_object = 0;
        } else if(!strcmp((const char*)qname->local_name, (const char*)"solution")) {
          skip_object = 0; /* get values */
        } else if(!strcmp((const char*)qname->local_name, (const char*)"binding")) {
          skip_object = 0; /* get values */
        } else if(!strcmp((const char*)qname->local_name, (const char*)"variable")) {
          skip_object = 1;
        } else if(!strcmp((const char*)qname->local_name, (const char*)"value")) {
          context->mkr_rs_processing_value = 1;
          context->mkr_rs_nvalue++;
          skip_object = 0;
        } else {
          skip_object = 1;
        }

      } else {
        /* not qname */
        raptor_turtle_writer_reference(turtle_writer, predicate->term->value.uri);
        skip_object = 0;
      } /* end predicate */

      if(qname)
        raptor_free_qname(qname);
    } else { /* predicate was skipped */
      if(!context->mkr_rs_arity)
        /* not last variable */
        raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)", ", 2);
    }

    if(!skip_object) {
      /* do not skip object */
      switch(object->term->type) {
        case RAPTOR_TERM_TYPE_URI:
          rv = raptor_turtle_emit_resource(serializer, object, depth+1);
          break;

        case RAPTOR_TERM_TYPE_LITERAL:
          if(!context->mkr_rs_arity) {
            /* variables */
            context->mkr_rs_nvalue++;
            if(context->mkr_rs_nvalue == 1)
              raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"rel ", 4);
            raptor_turtle_writer_csv_string(turtle_writer, object->term->value.literal.string);
          } else if(!context->mkr_rs_nvalue) {
            /* size */
            context->mkr_rs_size = atoi((const char*)object->term->value.literal.string);
          } else {
            /* values */
            if(context->mkr_rs_nvalue == 1)
              raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"rel ", 4);
            raptor_turtle_writer_csv_string(turtle_writer, object->term->value.literal.string);
            if(context->mkr_rs_nvalue < context->mkr_rs_arity)
              /* not last value */
              raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)", ", 2);
          }
          break;

        case RAPTOR_TERM_TYPE_BLANK:
          rv = raptor_turtle_emit_blank(serializer, object, depth+1);
          break;

        case RAPTOR_TERM_TYPE_UNKNOWN:
        default:
          raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR,
                                     NULL, "Triple has unsupported term type %u",
                                     object->term->type);
          break;
      }
    } /* end object */

    /* Return error if emitting something failed above */
    if(rv)
      return rv;

    last_predicate = predicate;
  } /* end iteration i */

  if(iter)
    raptor_free_avltree_iterator(iter);

  return rv;
}


/*********************************************************************************/

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
  int emit_mkr = context->emit_mkr;
  int mkr_blank = context->mkr_blank;
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

  /* check if we can do object abbreviation */
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

    /* check for collection (includes MKR_LIST, MKR_SET, MKR_VIEW, MKR_GRAPH) */
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
      subject->node->term->mkrtype = MKR_LIST;

    /* check for rs:ResultSet */
    } else if(pred1->term->type == RAPTOR_TERM_TYPE_URI &&
       raptor_uri_equals(pred1->term->value.uri, context->rs_resultVariable_uri)) {
        context->resultset = 1;
        subject->node->term->mkrtype = MKR_RELATION;
    }
  }

  /* emit the subject node */
  if(subject->node->term->type == RAPTOR_TERM_TYPE_URI) {
    rc = raptor_turtle_emit_resource(serializer, subject->node, depth+1);
    if(rc)
      return rc;
    blank = 0;
    collection = 0;

  } else if(subject->node->term->type == RAPTOR_TERM_TYPE_BLANK) {
    if((subject->node->count_as_subject == 1 &&
        subject->node->count_as_object == 0) && depth > 1) {
      blank = 1;
    } else if(subject->node->count_as_object == 0) {
      raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"[]", 2);
      blank = 0;
    } else if(!collection && subject->node->count_as_object > 1) {
      /* Referred to (used as an object), so needs a nodeID */
      raptor_turtle_writer_bnodeid(turtle_writer,
                                   subject->node->term->value.blank.string,
                                   subject->node->term->value.blank.string_len);
    }
  }

  /* emit subject properties */
  if(collection) {
#if defined(RAPTOR_DEBUG)
    printf("collection: ");
#endif
    if(emit_mkr) {
      raptor_turtle_writer_increase_indent(turtle_writer);
  
      rc = raptor_mkr_emit_subject_collection_items(serializer, subject, depth+1);
  
      raptor_turtle_writer_decrease_indent(turtle_writer);

    } else {
      raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"(", 1);
      raptor_turtle_writer_increase_indent(turtle_writer);
  
      rc = raptor_turtle_emit_subject_collection_items(serializer, subject, depth+1);
  
      raptor_turtle_writer_decrease_indent(turtle_writer);
      raptor_turtle_writer_newline(turtle_writer);
      raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)")", 1);
    }

  } else if(context->resultset && emit_mkr) {
#if defined(RAPTOR_DEBUG)
    printf("resultset: ");
#endif
    raptor_turtle_writer_increase_indent(turtle_writer);

    raptor_mkr_emit_subject_resultset(serializer, subject, depth+1);

    raptor_turtle_writer_decrease_indent(turtle_writer);

  } else {
#if defined(RAPTOR_DEBUG)
    printf("other: ");
#endif
    if(blank && depth > 1) {
      if(emit_mkr) {
        if(!mkr_blank) { /* write mkr blank as object */
          raptor_turtle_writer_bnodeid(turtle_writer,
                                       subject->node->term->value.blank.string,
                                       subject->node->term->value.blank.string_len);
          raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" ;", 2);
          raptor_turtle_writer_newline(turtle_writer);
        }

        raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"{", 1);
        context->curly_count++;

        if(mkr_blank) { /* write mkr blank as subject */
          raptor_turtle_writer_increase_indent(turtle_writer);
          raptor_turtle_writer_newline(turtle_writer);
          raptor_turtle_writer_bnodeid(turtle_writer,
                                       subject->node->term->value.blank.string,
                                       subject->node->term->value.blank.string_len);
        }
      } else {
        raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"[", 1);
      }
    }

    raptor_turtle_writer_increase_indent(turtle_writer);
    raptor_turtle_writer_newline(turtle_writer); /* no property on subject line */

    if(emit_mkr)
      raptor_mkr_emit_subject_properties(serializer, subject, depth+1);
    else
      raptor_turtle_emit_subject_properties(serializer, subject, depth+1);

    raptor_turtle_writer_decrease_indent(turtle_writer);

    if(blank && depth > 1) {
      if(emit_mkr) {
        raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"}", 1);
        context->curly_count--;
      } else {
        raptor_turtle_writer_newline(turtle_writer);
        raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)"]", 1);
      }
    }
  }

  if(depth == 0) {
    /* NOTE: the space before the . here MUST be there or statements
     * that end in a numeric literal will be interpreted incorrectly
     * (the "." will be parsed as part of the literal and statement
     * left unterminated)
     */
#if defined(RAPTOR_DEBUG)
  printf("depth 0: ");
#endif
    if(emit_mkr) {
      if(!context->resultset) {
        raptor_turtle_writer_newline(turtle_writer);
      }
      context->resultset = 0;
      context->written_begin = 0;
      context->indent_count = 0;
      context->curly_count = 0;
    } else {
      raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" .", 2);
      raptor_turtle_writer_newline(turtle_writer);
      raptor_turtle_writer_newline(turtle_writer);
    }
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

  context->turtle_writer_flags = 0;
  if(!strcmp(name,(const char*)"mkr")) {
    context->emit_mkr = 1;
    context->mkr_blank = 1;
    context->curly_count = 0;
    context->indent_count = 0;
    context->turtle_writer_flags |= TURTLE_WRITER_FLAG_MKR;
    context->mkrtype = MKR_LIST;
  } else {
    context->emit_mkr = 0;
  }
  context->resultset = 0;
  context->written_begin = 0;

  context->nstack = raptor_new_namespaces(serializer->world, 1);
  if(!context->nstack)
    return 1;
  context->rdf_nspace = raptor_new_namespace(context->nstack,
                                             (const unsigned char*)"rdf",
                                             (const unsigned char*)raptor_rdf_namespace_uri,
                                              0);
  context->mkr_nspace = raptor_new_namespace(context->nstack,
                                             (const unsigned char*)"mkr",
                                             (const unsigned char*)raptor_mkr_namespace_uri,
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

  /* for mKR serializer */
  context->mkr_has_uri = raptor_new_uri(serializer->world, (const unsigned char*)"http://mkrmke.net/ns/has");
  context->mkr_isu_uri = raptor_new_uri(serializer->world, (const unsigned char*)"http://mkrmke.net/ns/isu");
  context->mkr_do_uri = raptor_new_uri(serializer->world, (const unsigned char*)"http://mkrmke.net/ns/do");
  context->mkr_is_uri = raptor_new_uri(serializer->world, (const unsigned char*)"http://mkrmke.net/ns/is");

  context->rs_ResultSet_uri = raptor_new_uri(serializer->world, (const unsigned char*)"http://jena.hpl.hp.com/2003/03/result-set#ResultSet");
  context->rs_resultVariable_uri = raptor_new_uri(serializer->world, (const unsigned char*)"http://jena.hpl.hp.com/2003/03/result-set#resultVariable");

  if(!context->rdf_nspace || !context->mkr_nspace || !context->namespaces ||
     !context->subjects || !context->blanks || !context->nodes ||
     !context->rdf_xml_literal_uri || !context->rdf_first_uri ||
     !context->rdf_rest_uri || !context->rdf_nil_uri || !context->rdf_type ||
     !context->mkr_has_uri || !context->mkr_isu_uri || !context->mkr_do_uri || !context->mkr_is_uri ||
     !context->rs_ResultSet_uri || !context->rs_resultVariable_uri)
  {
    raptor_turtle_serialize_terminate(serializer);
    return 1;
  }

  /* Note: item 0 in the list is rdf:RDF's namespace */
  if(raptor_sequence_push(context->namespaces, context->rdf_nspace)) {
    raptor_turtle_serialize_terminate(serializer);
    return 1;
  }

  /* Note: item 1 in the list is mkr:mKR's namespace */
  if(raptor_sequence_push(context->namespaces, context->mkr_nspace)) {
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

  if(context->mkr_nspace) {
    raptor_free_namespace(context->mkr_nspace);
    context->mkr_nspace = NULL;
  }

  if(context->namespaces) {
    int i;

    /* Note: item 0 in the list is rdf:RDF's namespace and freed above */
    /* Note: item 1 in the list is mkr:mKR's namespace and freed above */
    for(i = 2; i< raptor_sequence_size(context->namespaces); i++) {
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

  if(context->mkr_has_uri) {
    raptor_free_uri(context->mkr_has_uri);
    context->mkr_has_uri = NULL;
  }

  if(context->mkr_isu_uri) {
    raptor_free_uri(context->mkr_isu_uri);
    context->mkr_isu_uri = NULL;
  }

  if(context->mkr_do_uri) {
    raptor_free_uri(context->mkr_do_uri);
    context->mkr_do_uri = NULL;
  }

  if(context->mkr_is_uri) {
    raptor_free_uri(context->mkr_is_uri);
    context->mkr_is_uri = NULL;
  }

  if(context->rs_ResultSet_uri) {
    raptor_free_uri(context->rs_ResultSet_uri);
    context->rs_ResultSet_uri = NULL;
  }

  if(context->rs_resultVariable_uri) {
    raptor_free_uri(context->rs_resultVariable_uri);
    context->rs_resultVariable_uri = NULL;
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
                                           serializer->iostream,
                                           context->turtle_writer_flags);
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
  int emit_mkr = context->emit_mkr;
  raptor_turtle_writer* turtle_writer = context->turtle_writer;

  if(context->written_header)
    return;

  if(!context->turtle_writer)
    return;

  for(i = 0; i< raptor_sequence_size(context->namespaces); i++) {
    raptor_namespace* ns;
    if(!emit_mkr && (i == 1)) /* skip mKR namespace */
      continue;
    ns = (raptor_namespace*)raptor_sequence_get_at(context->namespaces, i);
    raptor_turtle_writer_namespace_prefix(turtle_writer, ns);
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
                               "Do not know how to serialize node type %u",
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
                               "Cannot serialize a triple with object node type %u",
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
                               "Do not know how to serialize node type %u",
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
static const char* const    mkr_names[2] = { "mkr", NULL};

static const char* const turtle_uri_strings[3] = {
  "http://www.w3.org/ns/formats/Turtle",
  "http://www.dajobe.org/2004/01/turtle/",
  NULL
};

static const char* const mkr_uri_strings[5] = {
  "http://www.w3.org/ns/formats/Turtle",
  "http://www.dajobe.org/2004/01/turtle/",
  "http://mkrmke.net/ns/",
  "http://mkrmke.net/mkb/",
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
#define MKR_TYPES_COUNT 6
static const raptor_type_q mkr_types[TURTLE_TYPES_COUNT + 1] = {
  { "text/mkr", 8, 10},
  { "application/mkr", 15, 10},
  { "application/x-mkr", 17, 8},
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

static int
raptor_mkr_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->desc.names = mkr_names;
  factory->desc.mime_types = mkr_types;

  factory->desc.label = "mKR my Knowledge Representation Language";
  factory->desc.uri_strings   = mkr_uri_strings;

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

int
raptor_init_serializer_mkr(raptor_world* world)
{
  return !raptor_serializer_register_factory(world,
                                             &raptor_mkr_serializer_register_factory);
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

  turtle_writer = raptor_new_turtle_writer(world, base_uri, 0, nstack, iostr, 0);
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
                                           iostr, 0);
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

  turtle_writer = raptor_new_turtle_writer(world, base_uri, 0, nstack, iostr, 0);
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

