/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_abbrev.c - Code common to abbreviating serializers (ttl/rdfxmla)
 *
 * Copyright (C) 2006, Dave Robillard
 * Copyright (C) 2004-2011, David Beckett http://www.dajobe.org/
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
 * raptor_abbrev_node implementation
 *
 */


static raptor_abbrev_subject* raptor_new_abbrev_subject(raptor_abbrev_node* node);

/**
 * raptor_new_abbrev_node:
 * @world: raptor world
 * @term: term to use
 *
 * INTERNAL - Constructor for raptor_abbrev_node
 *
 * The @term is copied by the constructor.
 *
 * Return value: new raptor abbrev node or NULL on failure
 **/
raptor_abbrev_node* 
raptor_new_abbrev_node(raptor_world* world, raptor_term *term)
{
  raptor_abbrev_node* node = NULL;
  
  if(term->type == RAPTOR_TERM_TYPE_UNKNOWN)
    return NULL;

  node = RAPTOR_CALLOC(raptor_abbrev_node*, 1, sizeof(*node));
  if(node) {
    node->world = world;
    node->ref_count = 1;
    node->term = raptor_term_copy(term);
  }

  return node;
}


/**
 * raptor_new_abbrev_node:
 * @node: raptor abbrev node
 *
 * INTERNAL - Destructor for raptor_abbrev_node
 */
void
raptor_free_abbrev_node(raptor_abbrev_node* node)
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN(node, raptor_abbrev_node);

  if(--node->ref_count)
    return;

  if(node->term)
    raptor_free_term(node->term);

  RAPTOR_FREE(raptor_abbrev_node, node);
}


/**
 * raptor_abbrev_node_compare:
 * @node1: node 1
 * @node2: node 2
 *
 * INTERNAL - compare two raptor_abbrev_nodes.
 *
 * This needs to be a strong ordering for use by raptor_avltree.
 * This is very performance critical, anything to make it faster is worth it.
 *
 * Return value: <0, 0 or 1 if @node1 less than, equal or greater
 * than @node2 respectively
 */
int
raptor_abbrev_node_compare(raptor_abbrev_node* node1, raptor_abbrev_node* node2)
{
  if(node1 == node2)
    return 0;

  return raptor_term_compare(node1->term, node2->term);
}


/**
 * raptor_abbrev_node_equals:
 * @node1: node 1
 * @node2: node 2
 *
 * INTERNAL - compare two raptor_abbrev_nodes for equality
 *
 * Return value: non-0 if nodes are equal
 */
int
raptor_abbrev_node_equals(raptor_abbrev_node* node1, raptor_abbrev_node* node2)
{
  return raptor_term_equals(node1->term, node2->term);
}


/**
 * raptor_abbrev_node_lookup:
 * @nodes: Tree of nodes to search
 * @node: Node value to search for
 *
 * INTERNAL - Look in an avltree of nodes for a node described by parameters
 *   and if present create it, add it and return it
 *
 * Return value: the node found/created or NULL on failure
 */
raptor_abbrev_node* 
raptor_abbrev_node_lookup(raptor_avltree* nodes, raptor_term* term)
{
  raptor_abbrev_node *lookup_node;
  raptor_abbrev_node *rv_node;

  /* Create a temporary node for search comparison. */
  lookup_node = raptor_new_abbrev_node(term->world, term);
  
  if(!lookup_node)
    return NULL;

  rv_node = (raptor_abbrev_node*)raptor_avltree_search(nodes, lookup_node);
  
  /* If not found, insert/return a new one */
  if(!rv_node) {
    
    if(raptor_avltree_add(nodes, lookup_node))
      return NULL;
    else
      return lookup_node;

  /* Found */
  } else {
    raptor_free_abbrev_node(lookup_node);
    return rv_node;
  }
}


static raptor_abbrev_node**
raptor_new_abbrev_po(raptor_abbrev_node* predicate, raptor_abbrev_node* object)
{
  raptor_abbrev_node** nodes = NULL;
  nodes = RAPTOR_CALLOC(raptor_abbrev_node**, 2, sizeof(raptor_abbrev_node*));
  if(!nodes)
    return NULL;
  
  nodes[0] = predicate;
  nodes[1] = object;
  
  return nodes;
}


static void
raptor_free_abbrev_po(raptor_abbrev_node** nodes)
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN(nodes, raptor_abbrev_node_pair);
  
  if(nodes[0])
    raptor_free_abbrev_node(nodes[0]);
  if(nodes[1])
    raptor_free_abbrev_node(nodes[1]);

  RAPTOR_FREE(raptor_abbrev_nodes, nodes);
}


static int
raptor_compare_abbrev_po(raptor_abbrev_node** nodes1,
                         raptor_abbrev_node** nodes2)
{
  int d;
  d = raptor_abbrev_node_compare(nodes1[0], nodes2[0]);
  if(!d)
    d = raptor_abbrev_node_compare(nodes1[1], nodes2[1]);

  return d;
}


#ifdef RAPTOR_DEBUG
static void
raptor_print_abbrev_po(raptor_abbrev_node** nodes, FILE* handle)
{
  raptor_abbrev_node* p = nodes[0];
  raptor_abbrev_node* o = nodes[1];
  
  if(p && o) {
    fputc('[', handle);
    raptor_term_print_as_ntriples(p->term, handle);
    fputs(" : ", handle);
    raptor_term_print_as_ntriples(o->term, handle);
    fputs("]\n", handle);
  }
}
#endif


/*
 * raptor_abbrev_subject implementation
 *
 * The subject of triples, with all predicates and values
 * linked from them.
 *
 **/


static raptor_abbrev_subject*
raptor_new_abbrev_subject(raptor_abbrev_node* node)
{
  raptor_abbrev_subject* subject;
  
  if(!(node->term->type == RAPTOR_TERM_TYPE_URI ||
       node->term->type == RAPTOR_TERM_TYPE_BLANK)) {
    raptor_log_error(node->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                     "Subject node is type %d not a uri or blank node");
    return NULL;
  }  
  
  subject = RAPTOR_CALLOC(raptor_abbrev_subject*, 1, sizeof(*subject));

  if(subject) {
    subject->node = node;
    subject->node->ref_count++;
    subject->node->count_as_subject++;
    
    subject->node_type = NULL;

    subject->valid = 1;

    subject->properties =
      raptor_new_avltree((raptor_data_compare_handler)raptor_compare_abbrev_po,
                         (raptor_data_free_handler)raptor_free_abbrev_po,
                         0);
#ifdef RAPTOR_DEBUG
    if(subject->properties)
      raptor_avltree_set_print_handler(subject->properties,
                                       (raptor_data_print_handler)raptor_print_abbrev_po);
#endif

    subject->list_items =
      raptor_new_sequence((raptor_data_free_handler)raptor_free_abbrev_node, NULL);

    if(!subject->properties || !subject->list_items) {
      raptor_free_abbrev_subject(subject);
      subject = NULL;
    }
  
  }

  return subject;
}


void
raptor_free_abbrev_subject(raptor_abbrev_subject* subject) 
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN(subject, raptor_abbrev_subject);
  
  if(subject->node)
    raptor_free_abbrev_node(subject->node);
  
  if(subject->node_type)
    raptor_free_abbrev_node(subject->node_type);
  
  if(subject->properties)
    raptor_free_avltree(subject->properties);
  
  if(subject->list_items)
    raptor_free_sequence(subject->list_items);
  
  RAPTOR_FREE(raptor_subject, subject);
}


int
raptor_abbrev_subject_valid(raptor_abbrev_subject *subject)
{
  return subject->valid;
}


int
raptor_abbrev_subject_invalidate(raptor_abbrev_subject *subject)
{
  subject->valid = 0;
  return 0;
}
  
  


/**
 * raptor_subject_add_property:
 * @subject: subject node to add to
 * @predicate: predicate node
 * @object: object node
 *
 * INTERNAL - Add predicate/object pair into properties array of a subject node.
 *
 * The subject node takes ownership of the predicate/object nodes.
 * On error, predicate/object are freed immediately.
 * 
 * Return value: <0 on failure, >0 if pair is a duplicate and it was not added
 **/
int
raptor_abbrev_subject_add_property(raptor_abbrev_subject* subject,
                                   raptor_abbrev_node* predicate,
                                   raptor_abbrev_node* object) 
{
  int err;
  raptor_abbrev_node** nodes;
  
  nodes = raptor_new_abbrev_po(predicate, object);
  if(!nodes)
    return -1;

  predicate->ref_count++;
  object->ref_count++;

  if(raptor_avltree_search(subject->properties, nodes)) {
    /* Already present - do not add a duplicate triple (s->[p o]) */
    raptor_free_abbrev_po(nodes);
    return 1;
  }
  
#if 0
  fprintf(stderr, "Adding P,O ");
  raptor_print_abbrev_po(stderr, nodes);

  raptor_avltree_dump(subject->properties, stderr);
#endif
  err = raptor_avltree_add(subject->properties, nodes);
  if(err)
    return -1;
#if 0
  fprintf(stderr, "Result ");
  raptor_avltree_print(subject->properties, stderr);
  
  raptor_avltree_dump(subject->properties, stderr);

  raptor_avltree_check(subject->properties);

  fprintf(stderr, "\n\n");
#endif

  return 0;
}


int
raptor_abbrev_subject_compare(raptor_abbrev_subject* subject1,
                              raptor_abbrev_subject* subject2)
{
  return raptor_abbrev_node_compare(subject1->node, subject2->node);
}


/**
 * raptor_abbrev_subject_find:
 * @subjects: AVL-Tree of subject nodes
 * @term: node to find
 *
 * INTERNAL - Find a subject node in an AVL-Tree of subject nodes
 *
 * Return value: node or NULL if not found or failure
 */
raptor_abbrev_subject*
raptor_abbrev_subject_find(raptor_avltree *subjects, raptor_term* node)
{
  raptor_abbrev_subject* rv_subject = NULL;
  raptor_abbrev_node* lookup_node = NULL;
  raptor_abbrev_subject* lookup = NULL;

  /* datatype and language are both NULL for a subject node */
  
  lookup_node = raptor_new_abbrev_node(node->world, node);
  if(!lookup_node)
    return NULL;
  
  lookup = raptor_new_abbrev_subject(lookup_node);
  if(!lookup) {
    raptor_free_abbrev_node(lookup_node);
    return NULL;
  }

  rv_subject = (raptor_abbrev_subject*) raptor_avltree_search(subjects, lookup);

  raptor_free_abbrev_subject(lookup);
  raptor_free_abbrev_node(lookup_node);

  return rv_subject;
}


/**
 * raptor_abbrev_subject_lookup:
 * @nodes: AVL-Tree of subject nodes
 * @subjects: AVL-Tree of URI-subject nodes
 * @blanks: AVL-Tree of blank-subject nodes
 * @term: node to find
 *
 * INTERNAL - Find a subject node in the appropriate uri/blank AVL-Tree of subject nodes or add it
 *
 * Return value: node or NULL on failure
 */
raptor_abbrev_subject* 
raptor_abbrev_subject_lookup(raptor_avltree* nodes,
                             raptor_avltree* subjects, raptor_avltree* blanks,
                             raptor_term* term)
{
  raptor_avltree *tree;
  raptor_abbrev_subject* rv_subject;

  /* Search for specified resource. */
  tree = (term->type == RAPTOR_TERM_TYPE_BLANK) ? blanks : subjects;
  rv_subject = raptor_abbrev_subject_find(tree, term);

  /* If not found, create one and insert it */
  if(!rv_subject) {
    raptor_abbrev_node* node = raptor_abbrev_node_lookup(nodes, term);
    if(node) {      
      rv_subject = raptor_new_abbrev_subject(node);
      if(rv_subject) {
        if(raptor_avltree_add(tree, rv_subject)) {
          rv_subject = NULL;
        }
      }
    }
  }

  return rv_subject;
}


#ifdef ABBREV_DEBUG
void
raptor_print_subject(raptor_abbrev_subject* subject) 
{
  int i;
  unsigned char *subj;
  unsigned char *pred;
  unsigned char *obj;
  raptor_avltree_iterator* iter = NULL;

  /* Note: The raptor_abbrev_node field passed as the first argument for
   * raptor_term_to_string() is somewhat arbitrary, since as
   * the data structure is designed, the first word in the value union
   * is what was passed as the subject/predicate/object of the
   * statement.
   */
  subj = raptor_term_to_string(subject);

  if(subject->type) {
    obj = raptor_term_to_string(subject);
    fprintf(stderr,"[%s, http://www.w3.org/1999/02/22-rdf-syntax-ns#type, %s]\n", subj, obj);      
    RAPTOR_FREE(char*, obj);
  }
  
  for(i = 0; i < raptor_sequence_size(subject->elements); i++) {

    raptor_abbrev_node* o = raptor_sequence_get_at(subject->elements, i);
    if(o) {
      obj = raptor_term_to_string(o);
      fprintf(stderr,"[%s, [rdf:_%d], %s]\n", subj, i, obj);      
      RAPTOR_FREE(char*, obj);
    }
    
  }


  iter = raptor_new_avltree_iterator(subject->properties, NULL, NULL, 1);
  while(iter) {
    raptor_abbrev_node** nodes;
    nodes = (raptor_abbrev_node**)raptor_avltree_iterator_get(iter);
    if(!nodes)
      break;
    raptor_print_abbrev_po(stderr, nodes);

    if(raptor_avltree_iterator_next(iter))
      break;
  }
  if(iter)
    raptor_free_avltree_iterator(iter);
  
  RAPTOR_FREE(char*, subj);
  
}
#endif


/* helper functions */

/**
 * raptor_new_qname_from_resource:
 * @namespaces: sequence of namespaces (corresponding to nstack)
 * @nstack: #raptor_namespace_stack to use/update
 * @namespace_count: size of nstack (may be modified)
 * @node: #raptor_abbrev_node to use 
 * 
 * INTERNAL - Make an XML QName from the URI associated with the node.
 * 
 * Return value: the QName or NULL on failure
 **/
raptor_qname*
raptor_new_qname_from_resource(raptor_sequence* namespaces,
                               raptor_namespace_stack* nstack,
                               int* namespace_count,
                               raptor_abbrev_node* node)
{
  unsigned char* name = NULL;  /* where to split predicate name */
  size_t name_len = 1;
  unsigned char *uri_string;
  size_t uri_len;
  unsigned char *p;
  raptor_uri *ns_uri;
  raptor_namespace *ns;
  raptor_qname *qname;
  unsigned char *ns_uri_string;
  size_t ns_uri_string_len;
  
  if(node->term->type != RAPTOR_TERM_TYPE_URI) {
#ifdef RAPTOR_DEBUG
    RAPTOR_FATAL1("Node must be a URI\n");
#endif
    return NULL;
  }

  qname = raptor_new_qname_from_namespace_uri(nstack, node->term->value.uri, 10);
  if(qname)
    return qname;
  
  uri_string = raptor_uri_as_counted_string(node->term->value.uri, &uri_len);

  p= uri_string;
  name_len = uri_len;
  while(name_len >0) {
    if(raptor_xml_name_check(p, name_len, 10)) {
      name = p;
      break;
    }
    p++; name_len--;
  }
      
  if(!name || (name == uri_string))
    return NULL;

  ns_uri_string_len = uri_len - name_len;
  ns_uri_string = RAPTOR_MALLOC(unsigned char*, ns_uri_string_len + 1);
  if(!ns_uri_string)
    return NULL;
  memcpy(ns_uri_string, (const char*)uri_string, ns_uri_string_len);
  ns_uri_string[ns_uri_string_len] = '\0';
  
  ns_uri = raptor_new_uri_from_counted_string(node->world, ns_uri_string,
                                              ns_uri_string_len);
  RAPTOR_FREE(char*, ns_uri_string);
  
  if(!ns_uri)
    return NULL;

  ns = raptor_namespaces_find_namespace_by_uri(nstack, ns_uri);
  if(!ns) {
    /* The namespace was not declared, so create one */
    unsigned char prefix[2 + MAX_ASCII_INT_SIZE + 1];
    (*namespace_count)++;
    prefix[0] = 'n';
    prefix[1] = 's';
    (void)raptor_format_integer(RAPTOR_GOOD_CAST(char*,&prefix[2]),
                                MAX_ASCII_INT_SIZE + 1, *namespace_count,
                                /* base */ 10, -1, '\0');

    ns = raptor_new_namespace_from_uri(nstack, prefix, ns_uri, 0);

    /* We'll most likely need this namespace again. Push it on our
     * stack.  It will be deleted in raptor_rdfxmla_serialize_terminate()
     */
    if(raptor_sequence_push(namespaces, ns)) {
      /* namespaces sequence has no free handler so we have to free
       * the ns ourselves on error
       */
      raptor_free_namespace(ns);
      raptor_free_uri(ns_uri);
      return NULL;
    }
  }

  qname = raptor_new_qname_from_namespace_local_name(node->world, ns, name,
                                                     NULL);
  
  raptor_free_uri(ns_uri);

  return qname;
}
