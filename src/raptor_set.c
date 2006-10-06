/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_set.c - Sets for checking IDs (based on Redland memory hash)
 *
 * Copyright (C) 2003-2006, David Beckett http://purl.org/net/dajobe/
 * Copyright (C) 2003-2004, University of Bristol, UK http://www.bristol.ac.uk/
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
#include <stdarg.h>
#include <sys/types.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for abort() as used in errors */
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


#ifndef STANDALONE

/*
 * The only methods needed here are:
 *  Create Set
 *  Destroy Set
 *  Check a (base, ID) pair present add it if not, return if added/not
 *
 */

struct raptor_id_set_node_s
{
  struct raptor_id_set_node_s* next;
  char *item;
  size_t item_len;
  unsigned long hash;
};
typedef struct raptor_id_set_node_s raptor_id_set_node;


struct raptor_base_id_set_s
{
  /* The base URI of this set of IDs */
  raptor_uri *uri;
  
  /* neighbour ID sets */
  struct raptor_base_id_set_s* prev;
  struct raptor_base_id_set_s* next;

  /* An array pointing to a list of nodes (buckets) */
  raptor_id_set_node** nodes;

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
};
typedef struct raptor_base_id_set_s raptor_base_id_set;


struct raptor_id_set_s
{
  /* start of sets, 1 per base URI */
  struct raptor_base_id_set_s* first;

#ifdef RAPTOR_DEBUG
  int hits;
  int misses;
#endif
};


/* default load_factor out of 1000 */
static const int raptor_id_set_default_load_factor=750;

/* starting capacity - MUST BE POWER OF 2 */
static const int raptor_id_set_initial_capacity=8;


/* prototypes for local functions */
static raptor_id_set_node* raptor_base_id_set_find_node(raptor_base_id_set* set, unsigned char *item, size_t item_len, unsigned long hash);
static void raptor_free_id_set_node(raptor_id_set_node* node);
static int raptor_base_id_set_expand_size(raptor_base_id_set* set);


/*
 * perldelta 5.8.0 says under *Performance Enhancements*
 *
 *   Hashes now use Bob Jenkins "One-at-a-Time" hashing key algorithm
 *   http://burtleburtle.net/bob/hash/doobs.html  This algorithm is
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
 * raptor_id_set_find_node:
 * @set: the memory set context
 * @item: item string
 * @item_len: item string length
 * @hash: hash value or 0 if it should be computed here
 *
 * Find the node for the given item with optional hash.
 * 
 * Return value: #raptor_id_set_node of content or NULL on failure
 **/
static raptor_id_set_node*
raptor_base_id_set_find_node(raptor_base_id_set* base, 
                             unsigned char *item, size_t item_len,
                             unsigned long hash) 
{
  raptor_id_set_node* node;
  int bucket;

  /* empty set */
  if(!base->capacity)
    return NULL;

  if(!hash)
    ONE_AT_A_TIME_SET(hash, (unsigned char*)item, item_len);
  

  /* find slot in table */
  bucket=hash & (base->capacity - 1);

  /* check if there is a list present */ 
  node=base->nodes[bucket];
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
raptor_free_id_set_node(raptor_id_set_node* node) 
{
  if(node->item)
    RAPTOR_FREE(cstring, node->item);
  RAPTOR_FREE(raptor_id_set_node, node);
}


static int
raptor_base_id_set_expand_size(raptor_base_id_set* base) {
  int required_capacity=0;
  raptor_id_set_node **new_nodes;
  int i;

  if (base->capacity) {
    /* big enough */
    if((1000 * base->items) < (base->load_factor * base->capacity))
      return 0;
    /* grow set (keeping it a power of two) */
    required_capacity=base->capacity << 1;
  } else {
    required_capacity=raptor_id_set_initial_capacity;
  }

  /* allocate new table */
  new_nodes=(raptor_id_set_node**)RAPTOR_CALLOC(raptor_id_set_nodes, 
                                                required_capacity,
                                                sizeof(raptor_id_set_node*));
  if(!new_nodes)
    return 1;


  /* it is a new set empty set - we are done */
  if(!base->size) {
    base->capacity=required_capacity;
    base->nodes=new_nodes;
    return 0;
  }
  

  for(i=0; i<base->capacity; i++) {
    raptor_id_set_node *node=base->nodes[i];
      
    /* walk all attached nodes */
    while(node) {
      raptor_id_set_node *next;
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
  RAPTOR_FREE(raptor_id_set_nodes, base->nodes);

  /* attach new one */
  base->capacity=required_capacity;
  base->nodes=new_nodes;

  return 0;
}



/* functions implementing the ID set api */

/**
 * raptor_new_id_set:
 *
 * Constructor - create a new ID set.
 * 
 * Return value: non 0 on failure
 **/
raptor_id_set*
raptor_new_id_set(void) 
{
  raptor_id_set* set=(raptor_id_set*)RAPTOR_CALLOC(raptor_id_set, 1, 
                                                   sizeof(raptor_id_set));
  if(!set)
    return NULL;
  
  return set;
}


/**
 * raptor_free_base_id_set:
 * @set: #raptor_base_id_set
 *
 * Destructor - destroy an raptor_base_id_set.
 *
 **/
static void
raptor_free_base_id_set(raptor_base_id_set *base) 
{
  if(base->nodes) {
    int i;
  
    for(i=0; i<base->capacity; i++) {
      raptor_id_set_node *node=base->nodes[i];
      
      /* this entry is used */
      if(node) {
	raptor_id_set_node *next;
	/* free all attached nodes */
	while(node) {
	  next=node->next;
	  raptor_free_id_set_node(node);
	  node=next;
	}
      }
    }
    RAPTOR_FREE(raptor_id_set_nodes, base->nodes);
  }

  if(base->uri)
    raptor_free_uri(base->uri);

  RAPTOR_FREE(raptor_base_id_set, base);

  return;
}



/**
 * raptor_free_id_set:
 * @set: #raptor_id_set
 *
 * Destroy a set.
 *
 **/
void
raptor_free_id_set(raptor_id_set *set) 
{
  raptor_base_id_set *base=set->first;
  while(base) {
    raptor_base_id_set *next=base->next;
    raptor_free_base_id_set(base);
    base=next;
  }
  RAPTOR_FREE(raptor_id_set, set);
}



/**
 * raptor_id_set_add:
 * @set: #raptor_id_set
 * @base_uri: base #raptor_uri of identifier
 * @id: identifier name
 * @id_len: length of identifier
 *
 * Add an item to the set.
 * 
 * Return value: <0 on failure, 0 on success, 1 if already present
 **/
int
raptor_id_set_add(raptor_id_set* set, raptor_uri *base_uri,
                  const unsigned char *id, size_t id_len)
{
  raptor_base_id_set *base;
  unsigned char *base_uri_string;
  size_t base_uri_string_len;
  size_t item_len;
  unsigned char *item;

  raptor_id_set_node *node;
  unsigned long hash;
  char *new_item=NULL;
  int bucket= (-1);

  if(!base_uri || !id || !id_len)
    return -1;

  base=set->first;
  while(base) {
    if(raptor_uri_equals(base->uri, base_uri))
      break;
    base=base->next;
  }

  if(!base) {
    /* a set for this base_uri not found */
    base=(raptor_base_id_set*)RAPTOR_CALLOC(raptor_base_id_set, 1, 
                                            sizeof(raptor_base_id_set));
    if(!base)
      return -1;

    base->load_factor=raptor_id_set_default_load_factor;
    if(raptor_base_id_set_expand_size(base)) {
      RAPTOR_FREE(raptor_base_id_set, base);
      return -1;
    }

    base->uri=raptor_uri_copy(base_uri);

    /* Add to the start of the list */
    if(set->first)
      set->first->prev=base;
    /* base->prev=NULL; */
    base->next=set->first;

    set->first=base;
  } else {
    /* If not at the start of the list, move there */
    if(base != set->first) {
      /* remove from the list */
      base->prev->next=base->next;
      if(base->next)
        base->next->prev=base->prev;
      /* add at the start of the list */
      set->first->prev=base;
      base->prev=NULL;
      base->next=set->first;
    }
  }
  
  
  
  /* ensure there is enough space in the set */
  if (raptor_base_id_set_expand_size(base))
    return -1;

  /* Storing ID + ' ' + base-uri-string + '\0' */
  base_uri_string=raptor_uri_as_counted_string(base_uri, &base_uri_string_len);
  item_len=id_len+1+strlen((const char*)base_uri_string)+1;

  item=(unsigned char*)RAPTOR_MALLOC(cstring, item_len);
  if(!item)
    return 1;

  strcpy((char*)item, (const char*)id);
  item[id_len]=' ';
  strcpy((char*)item+id_len+1, (const char*)base_uri_string); /* strcpy for end '\0' */
  

  ONE_AT_A_TIME_SET(hash, (unsigned char*)item, item_len);
  
  /* find node for item */
  node=raptor_base_id_set_find_node(base, item, item_len, hash);

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

  bucket=hash & (base->capacity - 1);
  
  /* allocate new node */
  node=(raptor_id_set_node*)RAPTOR_CALLOC(raptor_id_set_node, 1,
                                          sizeof(raptor_id_set_node));
  if(!node)
    return 1;
  
  node->hash=hash;
  
  /* allocate item for new node */
  new_item=(char*)RAPTOR_MALLOC(cstring, item_len);
  if(!new_item) {
    RAPTOR_FREE(raptor_id_set_node, node);
    return -1;
  }
  
  /* copy new item */
  memcpy(new_item, item, item_len);
  node->item=new_item;
  node->item_len=item_len;

  node->next=base->nodes[bucket];
  base->nodes[bucket]=node;
  
  base->items++;

  /* Only increase bucket count use when previous value was NULL */
  if(!node->next)
    base->size++;

  RAPTOR_FREE(cstring, item);

  return 0;
}


#ifdef RAPTOR_DEBUG
void
raptor_id_set_stats_print(raptor_id_set* set, FILE *stream) {
  fprintf(stream, "set hits: %d misses: %d\n", set->hits, set->misses);
}
#endif

#endif


#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);


int
main(int argc, char *argv[]) 
{
  const char *program=raptor_basename(argv[0]);
  const char *items[8] = { "ron", "amy", "jen", "bij", "jib", "daj", "jim", NULL };
  raptor_id_set *set;
  raptor_uri *base_uri;
  int i=0;
  
  raptor_init();
  
  base_uri=raptor_new_uri((const unsigned char*)"http://example.org/base#");

  fprintf(stderr, "%s: Creating set\n", program);

  set=raptor_new_id_set();
  if(!set) {
    fprintf(stderr, "%s: Failed to create set\n", program);
    exit(1);
  }

  for(i=0; items[i]; i++) {
    size_t len=strlen(items[i]);
    int rc;

    fprintf(stderr, "%s: Adding set item '%s'\n", program, items[i]);
  
    rc=raptor_id_set_add(set, base_uri, (const unsigned char*)items[i], len);
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

    rc=raptor_id_set_add(set, base_uri, (const unsigned char*)items[i], len);
    if(rc <= 0) {
      fprintf(stderr, "%s: Adding duplicate set item %d '%s' succeeded, should have failed, returning error %d\n",
              program, i, items[i], rc);
      exit(1);
    }
  }

#ifdef RAPTOR_DEBUG
  raptor_id_set_stats_print(set, stderr);
#endif

  fprintf(stderr, "%s: Freeing set\n", program);
  raptor_free_id_set(set);

  raptor_free_uri(base_uri);
  
  raptor_finish();
  
  /* keep gcc -Wall happy */
  return(0);
}

#endif
