/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_set.c - Sets for checking IDs (based on Redland memory hash)
 *
 * $Id$
 *
 * Copyright (C) 2003 David Beckett - http://purl.org/net/dajobe/
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
#include <stdarg.h>
#include <sys/types.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for abort() as used in errors */
#endif

/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"



/*
 * The only methods needed here are:
 *  Create Set
 *  Destroy Set
 *  Check an ID present add it if not, return if added/not
 *
 */

struct raptor_set_node_s
{
  struct raptor_set_node_s* next;
  char *item;
  size_t item_len;
  unsigned long hash;
};
typedef struct raptor_set_node_s raptor_set_node;


struct raptor_set_s
{
  /* An array pointing to a list of nodes (buckets) */
  raptor_set_node** nodes;

  /* this many buckets used (out of capacity) */
  int size;
  /* this many items */
  int items;
  /* size of the buckets array 'nodes' above */
  int capacity;

  /* array load factor expressed out of 1000.
   * Always true: (size/capacity * 1000) < load_factor,
   * or in the code: size * 1000 < load_factor * capacity
   */
  int load_factor;
#ifdef RAPTOR_DEBUG
  int hits;
  int misses;
#endif
};


/* default load_factor out of 1000 */
static const int raptor_set_default_load_factor=750;

/* starting capacity - MUST BE POWER OF 2 */
static const int raptor_set_initial_capacity=8;


/* prototypes for local functions */
static raptor_set_node* raptor_set_find_node(raptor_set* set, char *item, size_t item_len, unsigned long hash);
static void raptor_free_set_node(raptor_set_node* node);
static int raptor_set_expand_size(raptor_set* set);


/*
 * perldelta 5.8.0 says under *Performance Enhancements*
 *
 *   Hashes now use Bob Jenkins "One-at-a-Time" hashing key algorithm
 *   http://burtleburtle.net/bob/set/doobs.html  This algorithm is
 *   reasonably fast while producing a much better spread of values
 *   than the old hashing algorithm ...
 *
 * Changed here to set the string backwards to help do URIs better
 *
 */

#define ONE_AT_A_TIME_SET(set,str,len) \
     do { \
        register const unsigned char *c_oneat = str+len-1; \
        register int i_oneat = len; \
        register unsigned long set_oneat = 0; \
        while (i_oneat--) { \
            set_oneat += *c_oneat--; \
            set_oneat += (set_oneat << 10); \
            set_oneat ^= (set_oneat >> 6); \
        } \
        set_oneat += (set_oneat << 3); \
        set_oneat ^= (set_oneat >> 11); \
        (set) = (set_oneat + (set_oneat << 15)); \
    } while(0)



/* helper functions */


/**
 * raptor_set_find_node - Find the node for the given item with optional hash
 * @set: the memory set context
 * @item: item string
 * @item_len: item string length
 * @hash: hash value or 0 if it should be computed here
 * 
 * Return value: &raptor_set_node of content or NULL on failure
 **/
static raptor_set_node*
raptor_set_find_node(raptor_set* set, 
                     char *item, size_t item_len,
                     unsigned long hash) 
{
  raptor_set_node* node;
  int bucket;

  /* empty set */
  if(!set->capacity)
    return NULL;

  if(!hash)
    ONE_AT_A_TIME_SET(hash, item, item_len);
  

  /* find slot in table */
  bucket=hash & (set->capacity - 1);

  /* check if there is a list present */ 
  node=set->nodes[bucket];
  if(!node)
    /* no list there */
    return NULL;
    
  /* walk the list */
  while(node) {
    if(item_len == node->item_len && !memcmp(item, node->item, item_len))
      break;
    node=node->next;
  }

  return node;
}


static void
raptor_free_set_node(raptor_set_node* node) 
{
  if(node->item)
    RAPTOR_FREE(cstring, node->item);
  RAPTOR_FREE(raptor_set_node, node);
}


static int
raptor_set_expand_size(raptor_set* set) {
  int required_capacity=0;
  raptor_set_node **new_nodes;
  int i;

  if (set->capacity) {
    /* big enough */
    if((1000 * set->items) < (set->load_factor * set->capacity))
      return 0;
    /* grow set (keeping it a power of two) */
    required_capacity=set->capacity << 1;
  } else {
    required_capacity=raptor_set_initial_capacity;
  }

  /* allocate new table */
  new_nodes=(raptor_set_node**)RAPTOR_CALLOC(raptor_set_nodes, 
                                             required_capacity,
                                             sizeof(raptor_set_node*));
  if(!new_nodes)
    return 1;


  /* it is a new set empty set - we are done */
  if(!set->size) {
    set->capacity=required_capacity;
    set->nodes=new_nodes;
    return 0;
  }
  

  for(i=0; i<set->capacity; i++) {
    raptor_set_node *node=set->nodes[i];
      
    /* walk all attached nodes */
    while(node) {
      raptor_set_node *next;
      int bucket;

      next=node->next;
      /* find slot in new table */
      bucket=node->hash & (required_capacity - 1);
      node->next=new_nodes[bucket];
      new_nodes[bucket]=node;

      node=next;
    }
  }

  /* now free old table */
  RAPTOR_FREE(raptor_set_nodes, set->nodes);

  /* attach new one */
  set->capacity=required_capacity;
  set->nodes=new_nodes;

  return 0;
}



/* functions implementing the set api */

/**
 * raptor_new_set - Create a new set
 * @set: &raptor set
 * @context: memory set contxt
 * 
 * Return value: non 0 on failure
 **/
raptor_set*
raptor_new_set(void) 
{
  raptor_set* set=(raptor_set*)RAPTOR_CALLOC(raptor_set, 1, sizeof(raptor_set));
  if(!set)
    return NULL;

  set->load_factor=raptor_set_default_load_factor;
  if(raptor_set_expand_size(set)) {
    RAPTOR_FREE(raptor_set, set);
    set=NULL;
  }
  return set;
}


/**
 * raptor_free_set - Destroy a set
 * @context: memory set context
 * 
 * Return value: non 0 on failure
 **/
int
raptor_free_set(raptor_set *set) 
{
  if(set->nodes) {
    int i;
  
    for(i=0; i<set->capacity; i++) {
      raptor_set_node *node=set->nodes[i];
      
      /* this entry is used */
      if(node) {
	raptor_set_node *next;
	/* free all attached nodes */
	while(node) {
	  next=node->next;
	  raptor_free_set_node(node);
	  node=next;
	}
      }
    }
    RAPTOR_FREE(raptor_set_nodes, set->nodes);
  }

  RAPTOR_FREE(raptor_set, set);

  return 0;
}



/**
 * raptor_set_add: - Add an item to the set
 * @context: memory set context
 * @item: pointer to item to store
 * 
 * Return value: <0 on failure, 0 on success, 1 if already present
 **/
int
raptor_set_add(raptor_set* set, char *item, size_t item_len)
{
  raptor_set_node *node;
  unsigned long hash;
  char *new_item=NULL;
  int bucket= (-1);

  /* ensure there is enough space in the set */
  if (raptor_set_expand_size(set))
    return -1;

  ONE_AT_A_TIME_SET(hash, item, item_len);
  
  /* find node for item */
  node=raptor_set_find_node(set, item, item_len, hash);

  /* if already there, error */
  if(node) {
#ifdef RAPTOR_DEBUG
    set->misses++;
#endif
    return 1;
  }

#ifdef RAPTOR_DEBUG
  set->hits++;
#endif
  
  /* always a new node */

  bucket=hash & (set->capacity - 1);
  
  /* allocate new node */
  node=(raptor_set_node*)RAPTOR_CALLOC(raptor_set_node, 1,
                                       sizeof(raptor_set_node));
  if(!node)
    return 1;
  
  node->hash=hash;
  
  /* allocate item for new node */
  new_item=(char*)RAPTOR_MALLOC(cstring, item_len);
  if(!new_item) {
    RAPTOR_FREE(raptor_set_node, node);
    return -1;
  }
  
  /* copy new item */
  memcpy(new_item, item, item_len);
  node->item=new_item;
  node->item_len=item_len;

  node->next=set->nodes[bucket];
  set->nodes[bucket]=node;
  
  set->items++;

  /* Only increase bucket count use when previous value was NULL */
  if(!node->next)
    set->size++;

  return 0;
}


#ifdef RAPTOR_DEBUG
void
raptor_set_stats_print(raptor_set* set, FILE *stream) {
  fprintf(stream, "hits: %d misses: %d\n", set->hits, set->misses);
}
#endif


#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);


#ifdef RAPTOR_IN_REDLAND
#include <librdf.h>
#endif

int
main(int argc, char *argv[]) 
{
  const char *program=argv[0];
  char *items[8] = { "ron", "amy", "jen", "bij", "jib", "daj", "jim", NULL };
  raptor_set *set;
  int i=0;
  
  fprintf(stderr, "%s: Creating set\n", program);

  set=raptor_new_set();
  if(!set) {
    fprintf(stderr, "%s: Failed to create set\n", program);
    exit(1);
  }

  for(i=0; items[i]; i++) {
    size_t len=strlen(items[i]);
    int rc;

    fprintf(stderr, "%s: Adding set item '%s'\n", program, items[i]);
  
    rc=raptor_set_add(set, items[i], len);
if(rc) {
      fprintf(stderr, "%s: Adding set item %d '%s' failed, returning error %d\n",
              program, i, items[i], rc);
      exit(1);
    }
  }

  for(i=0; items[i]; i++) {
    size_t len=strlen(items[i]);
    int rc;

    fprintf(stderr, "%s: Adding duplicate set item '%s'\n", program, items[i]);

    rc=raptor_set_add(set, items[i], len);
    if(rc <= 0) {
      fprintf(stderr, "%s: Adding duplicate set item %d '%s' succeeded, should have failed, returning error %d\n",
              program, i, items[i], rc);
      exit(1);
    }
  }

  raptor_set_stats_print(set, stderr);

  fprintf(stderr, "%s: Freeing set\n", program);
  raptor_free_set(set);

  /* keep gcc -Wall happy */
  return(0);
}

#endif
