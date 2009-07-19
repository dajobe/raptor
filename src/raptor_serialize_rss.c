/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_serialize_rss.c - Raptor RSS 1.0 and Atom 1.0 serializers
 *
 * Copyright (C) 2003-2009, David Beckett http://www.dajobe.org/
 * Copyright (C) 2003-2005, University of Bristol, UK http://www.bristol.ac.uk/
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
#include "raptor_rss.h"


typedef struct {
  raptor_world* world;
  /* owned by this: URI OR bnode if starts with _: */
  raptor_uri* uri;
  /* shared pointer */
  raptor_rss_item* item;
} raptor_rss_group_map;



/*
 * Raptor 'RSS 1.0' serializer object
 */
typedef struct {
  raptor_world* world;

  /* static rss model */
  raptor_rss_model model;

  /* Triples with no assigned type node */
  raptor_sequence *triples;

  /* Sequence of raptor_rss_item* : rdf:Seq items rdf:_<n> at offset n */
  raptor_sequence *items;

  /* Sequence of raptor_rss_item* (?x rdf:type rss:Enclosure) */
  raptor_sequence *enclosures;

  /* URI of rdf:Seq node */
  raptor_uri *seq_uri;

  /* Namespace stack for serializing */
  raptor_namespace_stack *nstack;

  /* the default namespace (rdf: or atom:) - 
   * this is destroyed when nstack above is deleted 
   */
  raptor_namespace* default_nspace;

  /* the xml: namespace */
  raptor_namespace *xml_nspace;

  /* the root element (rdf:RDF or atom:feed) */
  raptor_xml_element* root_element;

  /* where the xml is being written */
  raptor_xml_writer *xml_writer;

  /* non-0 if this is an atom 1.0 serializer */
  int is_atom;

  /* 0 = none
   * 1 = existing rss:item item containg rdf/xml encoding of any extra
   *     triples about URI (rss-1.0 serializer only)
   * 2 = at:md element containing rdf/xml property elements encoding
   *     of any extra triples about URI (atom serializer only)
   */
  int rss_triples_mode;
  
  /* namespaces declared here */
  raptor_namespace* nspaces[RAPTOR_RSS_NAMESPACES_SIZE];

  /* Map of group URI (key, owned) : rss item object (value, shared) */
  raptor_avltree *group_map;

  /* User declared namespaces */
  raptor_sequence *user_namespaces;

  /* URI of XML Literal datatype */
  raptor_uri* xml_literal_dt;

  int free_default_nspace;
} raptor_rss10_serializer_context;


static void
raptor_free_group_map(raptor_rss_group_map* gm) 
{
  if(gm->uri)
    raptor_free_uri_v2(gm->world, gm->uri);
  RAPTOR_FREE(raptor_rss_group_map, gm);
}


static int
raptor_rss_group_map_compare(raptor_rss_group_map* gm1,
                             raptor_rss_group_map* gm2)
{
  return raptor_uri_compare_v2(gm1->world, gm1->uri, gm2->uri);
}


static raptor_rss_item*
raptor_rss10_get_group_item(raptor_rss10_serializer_context *rss_serializer,
                            raptor_uri* uri)
{
  raptor_rss_group_map search_gm;
  raptor_rss_group_map* gm;

  search_gm.world=rss_serializer->world;
  search_gm.uri=uri;
  gm=(raptor_rss_group_map*)raptor_avltree_search(rss_serializer->group_map,
                                                  (void*)&search_gm);

  return gm ? gm->item : NULL;
}


static int
raptor_rss10_set_item_group(raptor_rss10_serializer_context *rss_serializer,
                            raptor_uri* uri, raptor_rss_item *item)
{
  raptor_rss_group_map* gm;

  if(raptor_rss10_get_group_item(rss_serializer, uri))
    return 0;
 
  gm=(raptor_rss_group_map*)RAPTOR_CALLOC(raptor_rss_group_map, 1,
                                          sizeof(raptor_rss_group_map));
  gm->world=rss_serializer->world;
  gm->uri=raptor_uri_copy_v2(rss_serializer->world, uri);
  gm->item=item;
  
  raptor_avltree_add(rss_serializer->group_map, gm);
  return 0;
}


/**
 * raptor_rss10_serialize_init:
 * @serializer: serializer object
 * @name: serializer name
 *
 * INTERNAL (raptor_serializer_factory API) - create a new serializer 
 *
 * Return value: non-0 on failure
 */
static int
raptor_rss10_serialize_init(raptor_serializer* serializer, const char *name)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;

  rss_serializer->world=serializer->world;

  raptor_rss_common_init(serializer->world);
  raptor_rss_model_init(serializer->world, &rss_serializer->model);

  rss_serializer->triples=raptor_new_sequence((raptor_sequence_free_handler*)raptor_free_statement_v2, (raptor_sequence_print_handler*)raptor_print_statement_v2);

  rss_serializer->items=raptor_new_sequence((raptor_sequence_free_handler*)raptor_free_rss_item, (raptor_sequence_print_handler*)NULL);

  rss_serializer->enclosures=raptor_new_sequence((raptor_sequence_free_handler*)raptor_free_rss_item, (raptor_sequence_print_handler*)NULL);

  rss_serializer->group_map=raptor_new_avltree(serializer->world,
                                               (raptor_data_compare_function)raptor_rss_group_map_compare,
                                               (raptor_data_free_function)raptor_free_group_map, 0);

  rss_serializer->user_namespaces=raptor_new_sequence((raptor_sequence_free_handler*)raptor_free_namespace, NULL);

  rss_serializer->is_atom=!(strcmp(name,"atom"));

  rss_serializer->nstack=raptor_new_namespaces_v2(serializer->world,
                                                  (raptor_simple_message_handler)raptor_serializer_simple_error,
                                                  serializer,
                                                  1);

  rss_serializer->xml_literal_dt=raptor_new_uri_v2(serializer->world, raptor_xml_literal_datatype_uri_string);

  return 0;
}
  

/**
 * raptor_rss10_serialize_terminate:
 * @serializer: serializer object
 *
 * INTERNAL (raptor_serializer_factory API) - destroy a serializer
 */
static void
raptor_rss10_serialize_terminate(raptor_serializer* serializer)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  int i;
  raptor_world* world=serializer->world;
  
  raptor_rss_model_clear(&rss_serializer->model);
  raptor_rss_common_terminate(world);

  if(rss_serializer->triples)
    raptor_free_sequence(rss_serializer->triples);

  if(rss_serializer->items)
    raptor_free_sequence(rss_serializer->items);

  if(rss_serializer->enclosures)
    raptor_free_sequence(rss_serializer->enclosures);

  if(rss_serializer->seq_uri)
    raptor_free_uri_v2(rss_serializer->world, rss_serializer->seq_uri);

  if(rss_serializer->xml_writer)
    raptor_free_xml_writer(rss_serializer->xml_writer);

  for(i=0; i<RAPTOR_RSS_NAMESPACES_SIZE;i++) {
    if(rss_serializer->nspaces[i])
      raptor_free_namespace(rss_serializer->nspaces[i]);
  }
  
  if(rss_serializer->free_default_nspace && rss_serializer->default_nspace)
    raptor_free_namespace(rss_serializer->default_nspace);

  if(rss_serializer->xml_nspace)
    raptor_free_namespace(rss_serializer->xml_nspace);

  if(rss_serializer->user_namespaces)
    raptor_free_sequence(rss_serializer->user_namespaces);

  /* all raptor_namespace* objects must be freed BEFORE the stack
   * they are attached to here: */
  if(rss_serializer->nstack)
    raptor_free_namespaces(rss_serializer->nstack);

  if(rss_serializer->group_map)
    raptor_free_avltree(rss_serializer->group_map);
  
  if(world->rss_fields_info_qnames) {
    for(i=0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
      if(world->rss_fields_info_qnames[i])
        raptor_free_qname(world->rss_fields_info_qnames[i]);
    }
    RAPTOR_FREE(raptor_qname* array, world->rss_fields_info_qnames);
    world->rss_fields_info_qnames=NULL;
  }

  if(world->rss_types_info_qnames) {
    for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
      if(world->rss_types_info_qnames[i])
        raptor_free_qname(world->rss_types_info_qnames[i]);
    }
    RAPTOR_FREE(raptor_wname* array, world->rss_types_info_qnames);
    world->rss_types_info_qnames=NULL;
  }

  if(rss_serializer->xml_literal_dt)
    raptor_free_uri_v2(rss_serializer->world, rss_serializer->xml_literal_dt);
}
  

/**
 * raptor_rss10_move_statements:
 * @rss_serializer: serializer object
 * @type: item type
 * @item: item object
 *
 * INTERNAL - Move statements from the stored triples into item @item
 * that match @item's URI as subject.
 *
 * Return value: count of number of triples moved
 */
static int
raptor_rss10_move_statements(raptor_rss10_serializer_context *rss_serializer,
                             raptor_rss_type type,
                             raptor_rss_item *item)
{
  int t;
  int count=0;
  int is_atom=rss_serializer->is_atom;
  
  for(t=0; t< raptor_sequence_size(rss_serializer->triples); t++) {
    raptor_statement_v2* s;
    int f;

    s=(raptor_statement_v2*)raptor_sequence_get_at(rss_serializer->triples, t);
    if(!s)
      continue;

    if(s->s->subject_type != RAPTOR_IDENTIFIER_TYPE_RESOURCE ||
       !raptor_uri_equals_v2(rss_serializer->world, (raptor_uri*)s->s->subject, item->uri))
       continue;
    
    /* now we know this triple is associated with the item URI
     * and can count the relevant triples */
    count++;
    
    /* add triples with anonymous object to the general triples sequence
     * for this item, and to the group map (blank node closure)
     */
    if(s->s->object_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
      raptor_uri* fake_uri=raptor_new_uri_v2(rss_serializer->world, (const unsigned char*)s->s->object);
      raptor_rss10_set_item_group(rss_serializer, fake_uri, item);
      raptor_free_uri_v2(rss_serializer->world, fake_uri);

      RAPTOR_DEBUG4("Moved anonymous value property URI <%s> for typed node %i - %s\n",
                    raptor_uri_as_string_v2(rss_serializer->world, (raptor_uri*)s->s->predicate),
                    type, raptor_rss_items_info[type].name);
      s=(raptor_statement_v2*)raptor_sequence_delete_at(rss_serializer->triples,
                                                        t);
      raptor_sequence_push(item->triples, s);
      continue;
    }


    /* otherwise process object value types resource or literal */
    for(f=0; f < RAPTOR_RSS_FIELDS_SIZE; f++) {
      if(!rss_serializer->world->rss_fields_info_uris[f])
        continue;
      
      if((s->s->predicate_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE ||
          s->s->predicate_type == RAPTOR_IDENTIFIER_TYPE_PREDICATE) &&
          s->s->object_type != RAPTOR_IDENTIFIER_TYPE_ANONYMOUS &&
         raptor_uri_equals_v2(rss_serializer->world, 
                              (raptor_uri*)s->s->predicate,
                              rss_serializer->world->rss_fields_info_uris[f])) {
         raptor_rss_field* field=raptor_rss_new_field(rss_serializer->world);

        /* found field this triple to go in 'item' so move the
         * object value over 
         */
        if(s->s->object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
          field->uri=(raptor_uri*)s->s->object;
        else {
          field->value=(unsigned char*)s->s->object;
          if(s->s->object_literal_datatype &&
             raptor_uri_equals_v2(rss_serializer->world,
                                  s->s->object_literal_datatype,
                                  rss_serializer->xml_literal_dt))
             field->is_xml=1;
          if(f == RAPTOR_RSS_FIELD_CONTENT_ENCODED)
             field->is_xml=1;
          if(f == RAPTOR_RSS_FIELD_ATOM_SUMMARY && *field->value == '<')
            field->is_xml=1;
        }
        s->s->object=NULL;

        if(is_atom) { 
          int i;
          
          /* Rewrite item fields rss->atom */
          for(i=0; raptor_atom_to_rss[i].from != RAPTOR_RSS_FIELD_UNKNOWN; i++) {
            int from_f=raptor_atom_to_rss[i].to;
            int to_f=raptor_atom_to_rss[i].from;
            
            /* Do not rewrite to atom0.3 terms */
            if(raptor_rss_fields_info[to_f].nspace == ATOM0_3_NS)
              continue;

            if(f == from_f &&
               !(item->fields[to_f] && item->fields[to_f]->value)) {
              f= to_f;
              if(to_f == RAPTOR_RSS_FIELD_ATOM_SUMMARY && *field->value == '<')
                field->is_xml=1;
              field->is_mapped=1;
              RAPTOR_DEBUG5("Moved field %d - %s to field %d - %s\n", from_f, raptor_rss_fields_info[from_f].name, to_f, raptor_rss_fields_info[to_f].name);
              break;
            }
          }
        } /* end is atom field to map */

        RAPTOR_DEBUG1("Adding field\n");
        raptor_rss_item_add_field(item, f, field);
        raptor_sequence_set_at(rss_serializer->triples, t, NULL);
        break;
      }
    } /* end for field loop */

    /* loop ended early so triple was assocated with a field - continue */
    if(f < RAPTOR_RSS_FIELDS_SIZE)
      continue;


    /* otherwise triple was not found as a field so store in triples
     * sequence 
     */
    RAPTOR_DEBUG4("UNKNOWN property URI <%s> for typed node %i - %s\n",
                  raptor_uri_as_string_v2(rss_serializer->world, (raptor_uri*)s->s->predicate),
                  type, raptor_rss_items_info[type].name);
    s=(raptor_statement_v2*)raptor_sequence_delete_at(rss_serializer->triples,
                                                      t);
    raptor_sequence_push(item->triples, s);

  } /* end for all triples */

#ifdef RAPTOR_DEBUG
  if(count > 0)
    RAPTOR_DEBUG5("Moved %d triples to typed node %i - %s with uri <%s>\n",
                  count, type, raptor_rss_items_info[type].name,
                  raptor_uri_as_string_v2(rss_serializer->world, (raptor_uri*)item->uri));
#endif

  return count;
}


/**
 * raptor_rss10_move_anonymous_statements:
 * @rss_serializer: serializer object
 *
 * INTERNAL - Move statements with a blank node subject to the appropriate item
 *
 */
static int
raptor_rss10_move_anonymous_statements(raptor_rss10_serializer_context *rss_serializer)
{
  int t;
  int handled=1;
  int round=0;
#ifdef RAPTOR_DEBUG
  int moved_count=0;
#endif

  for(round=0; handled; round++) {
    handled=0;
    
    for(t=0; t< raptor_sequence_size(rss_serializer->triples); t++) {
      raptor_statement_v2* s;
      raptor_uri* fake_uri;
      raptor_rss_item* item;
      
      s=(raptor_statement_v2*)raptor_sequence_get_at(rss_serializer->triples, t);
      if(!s)
        continue;
      
      if(s->s->subject_type != RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
        continue;
      
      fake_uri=raptor_new_uri_v2(rss_serializer->world, (const unsigned char*)s->s->subject);
      item=raptor_rss10_get_group_item(rss_serializer, fake_uri);
      raptor_free_uri_v2(rss_serializer->world, fake_uri);
      
      if(item) {
        /* triple matched an existing item */
        s=(raptor_statement_v2*)raptor_sequence_delete_at(rss_serializer->triples,
                                                          t);
        raptor_sequence_push(item->triples, s);
#ifdef RAPTOR_DEBUG
        moved_count++;
#endif

        if(s->s->object_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
          fake_uri=raptor_new_uri_v2(rss_serializer->world, (const unsigned char*)s->s->object);
          raptor_rss10_set_item_group(rss_serializer, fake_uri, item);
          raptor_free_uri_v2(rss_serializer->world, fake_uri);
        }
        

        handled=1;
      }
    } /* end for all triples */
    
#ifdef RAPTOR_DEBUG
    if(moved_count > 0)
      RAPTOR_DEBUG3("Round %d: Moved %d triples\n", round, moved_count);
#endif
  }
  
  return 0;
}


/**
 * raptor_rss10_move_leftover_statements:
 * @rss_serializer: serializer object
 *
 * INTERNAL - Move any statements in the serializer pool to items or channel
 *
 */
static int
raptor_rss10_move_leftover_statements(raptor_rss10_serializer_context *rss_serializer) {
  raptor_rss_model* rss_model;
  int i;
  int type;
  raptor_rss_item* item;

  rss_model=&rss_serializer->model;

  type=RAPTOR_RSS_ITEM;
  for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
    item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
    raptor_rss10_move_statements(rss_serializer, (raptor_rss_type)type, item);
  }
  
  type=RAPTOR_RSS_CHANNEL;
  if(rss_model->common[type]) {
    item=rss_model->common[type];
    raptor_rss10_move_statements(rss_serializer, (raptor_rss_type)type, item);
  }

  return 0;
}


/**
 * raptor_rss10_remove_mapped_item_fields:
 * @rss_serializer: serializer object
 * @item: rss item
 * @type: item type
 *
 * INTERNAL - Remove mapped fields for an item
 *
 */
static int
raptor_rss10_remove_mapped_item_fields(raptor_rss10_serializer_context *rss_serializer,
                                       raptor_rss_item* item, int type)
{
  int f;
  
  if(!item->fields_count)
    return 0;
      
  for(f=0; f < RAPTOR_RSS_FIELDS_SIZE; f++) {
    raptor_rss_field* field;
    int saw_mapped=0;
    int saw_non_mapped=0;
        
    for (field=item->fields[f]; field; field=field->next) {
      if(field->is_mapped)
        saw_mapped++;
      else
        saw_non_mapped++;
    }
    
    if(saw_mapped && saw_non_mapped) {
      raptor_rss_field* last_field=NULL;
      RAPTOR_DEBUG6("Item %p Field %d - %s: %d mapped %d non-mapped\n", item, f, raptor_rss_fields_info[f].name, saw_mapped, saw_non_mapped);
    
      field=item->fields[f];
      while(field) {
        raptor_rss_field* next=field->next;
        field->next=NULL;
        if(field->is_mapped)
          raptor_rss_field_free(field);
        else {
          if(!last_field)
            item->fields[f]=field;
          else
            last_field->next=field;
          last_field=field;
        }
        field=next;
      }
    }

  }

  return 0;
}


/**
 * raptor_rss10_remove_mapped_fields:
 * @rss_serializer: serializer object
 *
 * INTERNAL - Move statements with a blank node subject to the appropriate item
 *
 */
static int
raptor_rss10_remove_mapped_fields(raptor_rss10_serializer_context *rss_serializer)
{
  raptor_rss_model* rss_model;
  int is_atom;
  int i;
  
  rss_model=&rss_serializer->model;
  is_atom=rss_serializer->is_atom;

  if(!is_atom)
    return 0;

  if(rss_model->items_count) {
    for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
      raptor_rss_item* item;
      item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
      raptor_rss10_remove_mapped_item_fields(rss_serializer, item,
                                             RAPTOR_RSS_ITEM);
    }
  }
  
  for(i=RAPTOR_RSS_CHANNEL; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    raptor_rss_item* item;
    for (item=rss_model->common[i]; item; item=item->next) {
      raptor_rss10_remove_mapped_item_fields(rss_serializer, item, i);
    }
  }

  return 0;
}

/**
 * raptor_rss10_store_statement:
 * @rss_serializer: serializer object
 * @s: statement
 *
 * INTERNAL - decide where to store a statement in an item or keep pending
 *
 * Return value: non-0 if handled (stored)
 */
static int
raptor_rss10_store_statement(raptor_rss10_serializer_context *rss_serializer,
                             raptor_statement_v2 *s)
{
  raptor_rss_item *item=NULL;
  int handled=0;
  int is_atom=rss_serializer->is_atom;
  raptor_uri* fake_uri;
  
  fake_uri=raptor_new_uri_v2(rss_serializer->world, (const unsigned char*)s->s->subject);
  item=raptor_rss10_get_group_item(rss_serializer, fake_uri);
  raptor_free_uri_v2(rss_serializer->world, fake_uri);

  if(item && s->s->object_type != RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
    int f;

    for(f=0; f < RAPTOR_RSS_FIELDS_SIZE; f++) {
      raptor_rss_field* field;
      if(!rss_serializer->world->rss_fields_info_uris[f])
        continue;

      if((s->s->predicate_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE ||
          s->s->predicate_type == RAPTOR_IDENTIFIER_TYPE_PREDICATE) &&
         raptor_uri_equals_v2(rss_serializer->world, 
                              (raptor_uri*)s->s->predicate,
                              rss_serializer->world->rss_fields_info_uris[f])) {
        /* found field this triple to go in 'item' so move the
         * object value over 
         */
        field=raptor_rss_new_field(rss_serializer->world);
        if(s->s->object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE) {
          field->uri=(raptor_uri*)s->s->object;
        } else {
          field->value=(unsigned char*)s->s->object;
          if(s->s->object_literal_datatype &&
             raptor_uri_equals_v2(rss_serializer->world,
                                  s->s->object_literal_datatype,
                                  rss_serializer->xml_literal_dt))
             field->is_xml=1;
          if(f == RAPTOR_RSS_FIELD_CONTENT_ENCODED)
            field->is_xml=1;
          if(f == RAPTOR_RSS_FIELD_ATOM_SUMMARY && *field->value == '<')
            field->is_xml=1;
        }
        s->s->object=NULL;

        if(is_atom) { 
          int i;
          
          /* Rewrite item fields rss->atom */
          for(i=0; raptor_atom_to_rss[i].from != RAPTOR_RSS_FIELD_UNKNOWN; i++) {
            int from_f=raptor_atom_to_rss[i].to;
            int to_f=raptor_atom_to_rss[i].from;
            
            /* Do not rewrite to atom0.3 terms */
            if(raptor_rss_fields_info[to_f].nspace == ATOM0_3_NS)
              continue;
            
            if(f == from_f &&
               !(item->fields[to_f] && item->fields[to_f]->value)) {
              f= to_f;
              if(to_f == RAPTOR_RSS_FIELD_ATOM_SUMMARY && *field->value == '<')
                field->is_xml=1;
              field->is_mapped=1;
              RAPTOR_DEBUG5("Moved field %d - %s to field %d - %s\n", from_f, raptor_rss_fields_info[from_f].name, to_f, raptor_rss_fields_info[to_f].name);
              break;
            }
          }
        }

        RAPTOR_DEBUG1("Adding field\n");
        raptor_rss_item_add_field(item, f, field);
        raptor_free_statement_v2(s);
#if RAPTOR_DEBUG > 1
        RAPTOR_DEBUG2("Stored statement under typed node %p\n", item);
#endif

        handled=1;
        break;
      }
    }
  }
  
  if(!handled) {
    raptor_sequence_push(rss_serializer->triples, s);
#if RAPTOR_DEBUG > 1
    fprintf(stderr,"Stored statement: ");
    raptor_print_statement_as_ntriples_v2(s, stderr);
    fprintf(stderr,"\n");
#endif
    handled=1;
  }

  return handled;
}


static int
raptor_rss10_serialize_start(raptor_serializer* serializer)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;

  if(serializer->feature_rss_triples) {
    if(!strcmp((const char*)serializer->feature_rss_triples,
               "none"))
      rss_serializer->rss_triples_mode=0;
    else if(!strcmp((const char*)serializer->feature_rss_triples,
               "rdf-xml"))
      rss_serializer->rss_triples_mode=1;
    else if(!strcmp((const char*)serializer->feature_rss_triples, 
                    "atom-triples"))
      rss_serializer->rss_triples_mode=2;
    else
      rss_serializer->rss_triples_mode=0;
  }

  return 0;
}


/**
 * raptor_rss10_serialize_statement:
 * @serializer: serializer object
 * @statement: statement
 *
 * INTERNAL (raptor_serializer_factory API) - Serialize a statement
 *
 * Return value: non-0 on failure
 */
static int
raptor_rss10_serialize_statement(raptor_serializer* serializer, 
                                 const raptor_statement *statement)
{
  raptor_rss10_serializer_context *rss_serializer;
  raptor_rss_model *rss_model;
  int handled = 0;
  int i;
  raptor_rss_type type;
  raptor_rss_item *item = NULL;

  rss_serializer = (raptor_rss10_serializer_context*)serializer->context;
  rss_model = &rss_serializer->model;

#if RAPTOR_DEBUG > 1
  if(1) {
    raptor_statement_v2 s2;
    RAPTOR_DEBUG1("Processing statement\n  ");
    s2.s = (raptor_statement*)statement;
    s2.world = rss_serializer->world;
    raptor_print_statement_as_ntriples_v2(&s2, stderr);
    fputc('\n', stderr);
  }
#endif

  if(raptor_uri_equals_v2(rss_serializer->world,
                          (raptor_uri*)statement->predicate, 
                          RAPTOR_RSS_RSS_items_URI(rss_model))) {
    /* ignore any triple (? rss:items ?) - is infered */
    return 0;
  }

  if(!raptor_uri_equals_v2(rss_serializer->world,
                           (raptor_uri*)statement->predicate, 
                           RAPTOR_RSS_RDF_type_URI(rss_model))) 
    goto savetriple;
  

  /* Look for triple (?resource rdf:type rdf:Seq) */
  if(statement->object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE &&
     raptor_uri_equals_v2(rss_serializer->world,
                          (raptor_uri*)statement->object, 
                          RAPTOR_RSS_RDF_Seq_URI(rss_model))) {
    
    if(statement->subject_type==RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
      RAPTOR_DEBUG2("Saw rdf:Seq with blank node %s\n",
                    (char*)statement->subject);
      rss_serializer->seq_uri = raptor_new_uri_v2(rss_serializer->world,
                                                  (unsigned char*)statement->subject);
    } else {
      RAPTOR_DEBUG2("Saw rdf:Seq with URI <%s>\n",
                    raptor_uri_as_string_v2(rss_serializer->world,
                                            (raptor_uri*)statement->subject));
      rss_serializer->seq_uri=raptor_uri_copy_v2(rss_serializer->world,
                                                 rss_serializer->seq_uri);
    }
    
    handled = 1;
    goto savetriple;
  }


  /* look for triple: (? rdf:type ?) to find containers and blocks */
  type = RAPTOR_RSS_NONE;
  for(i = 0; i < RAPTOR_RSS_COMMON_SIZE; i++) {
    raptor_uri *item_uri = serializer->world->rss_types_info_uris[i];
    if(item_uri &&
       raptor_uri_equals_v2(rss_serializer->world,
                            (raptor_uri*)statement->object, item_uri)) {
      type = (raptor_rss_type)i;
      RAPTOR_DEBUG4("Found typed node %i - %s with URI <%s>\n", type,
                    raptor_rss_items_info[type].name,
                    raptor_uri_as_string_v2(rss_serializer->world,
                                            (raptor_uri*)statement->subject));
      break;
    }
  }

  if(type == RAPTOR_RSS_NONE) {
    RAPTOR_DEBUG2("UNKNOWN typed node with type URI <%s>\n",
                  raptor_uri_as_string_v2(rss_serializer->world,
                                          (raptor_uri*)statement->object));
    goto savetriple;
  }


  if(type == RAPTOR_RSS_ITEM) {
    for(i = 0; i < raptor_sequence_size(rss_serializer->items); i++) {
      item = (raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
      if(raptor_rss_item_equals_statement_subject(item, statement))
        break;
      
    }
    if(i < raptor_sequence_size(rss_serializer->items)) {
      RAPTOR_DEBUG2("Found RSS item at entry %d in sequence of items\n", i);
    } else {
      RAPTOR_DEBUG2("RSS item URI <%s> is not in sequence of items\n",
                    raptor_uri_as_string_v2(rss_serializer->world,
                                            (raptor_uri*)statement->subject));
      item = NULL;
    }
  } else if(type == RAPTOR_RSS_ENCLOSURE) {
    for(i = 0; i < raptor_sequence_size(rss_serializer->enclosures); i++) {
      item = (raptor_rss_item*)raptor_sequence_get_at(rss_serializer->enclosures, i);
      if(raptor_rss_item_equals_statement_subject(item, statement))
        break;
      
    }
    if(i < raptor_sequence_size(rss_serializer->items)) {
      RAPTOR_DEBUG2("Found enclosure at entry %d in sequence of enclosures\n", i);
    } else {
      RAPTOR_DEBUG2("Add new enclosure to sequence with URI <%s>\n",
                    raptor_uri_as_string_v2(rss_serializer->world,
                                            (raptor_uri*)statement->subject));
      
      item = raptor_new_rss_item(rss_serializer->world);
      raptor_sequence_push(rss_serializer->enclosures, item);
    }
  } else {
    item=raptor_rss_model_add_common(rss_model, type);
  }
  

  if(item) {
    raptor_rss_item_set_uri(item, (raptor_uri*)statement->subject);
    
    /* Move any existing statements to the newly discovered item */
    raptor_rss10_move_statements(rss_serializer, type, item);
    
    raptor_rss10_set_item_group(rss_serializer, item->uri, item);
    
    handled = 1;
  }


  savetriple:
  if(!handled) {
    raptor_statement_v2 *t;
    t = raptor_statement_copy_v2_from_v1(rss_serializer->world, statement);
    if(t)
      handled = raptor_rss10_store_statement(rss_serializer, t);
  }

  return 0;
}


static void
raptor_rss10_build_items(raptor_rss10_serializer_context *rss_serializer)
{
  raptor_rss_model* rss_model=&rss_serializer->model;
  int i;

  if(!rss_serializer->seq_uri)
    return;
  
  for(i=0; i < raptor_sequence_size(rss_serializer->triples); i++) {
    int ordinal= -1;
    raptor_uri* fake_uri=NULL;
    raptor_statement_v2* s;

    s=(raptor_statement_v2*)raptor_sequence_get_at(rss_serializer->triples, i);
    if(!s)
      continue;
    
#if RAPTOR_DEBUG > 1
    RAPTOR_DEBUG1("Processing statement\n  ");
    raptor_print_statement_as_ntriples_v2(s, stderr);
    fputc('\n', stderr);
#endif
    
    /* skip triples that are not ? ? <uri> */
    if(s->s->object_type != RAPTOR_IDENTIFIER_TYPE_RESOURCE) {
      RAPTOR_DEBUG1("Not ? ? <uri> - continuing\n");
      continue;
    }
  

    if(s->s->subject_type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
      fake_uri=raptor_new_uri_v2(rss_serializer->world, (unsigned char*)s->s->subject);
    else
      fake_uri=raptor_uri_copy_v2(rss_serializer->world, (raptor_uri*)s->s->subject);
      
    if(raptor_uri_equals_v2(rss_serializer->world, fake_uri, rss_serializer->seq_uri)) {
      /* found <seq URI> <some predicate> <some URI> triple */

      if(s->s->predicate_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL)
        ordinal= *((int*)s->s->predicate);
      else { /* predicate is a resource */
        const unsigned char* uri_str;
        uri_str= raptor_uri_as_string_v2(rss_serializer->world, (raptor_uri*)s->s->predicate);
        if(!strncmp((const char*)uri_str, "http://www.w3.org/1999/02/22-rdf-syntax-ns#_", 44))
          ordinal= raptor_check_ordinal(uri_str+44);
      }
      RAPTOR_DEBUG3("Found RSS 1.0 item %d with URI <%s>\n", ordinal,
                    raptor_uri_as_string_v2(rss_serializer->world, (raptor_uri*)s->s->object));

      if(ordinal >= 0) {
        raptor_rss_item *item;
        
        item = raptor_new_rss_item(rss_serializer->world);

        raptor_rss_item_set_uri(item, (raptor_uri*)s->s->object);

        raptor_sequence_set_at(rss_serializer->items, ordinal-1, item);

        raptor_sequence_set_at(rss_serializer->triples, i, NULL);

        /* Move any existing statements to the newly discovered item */
        raptor_rss10_move_statements(rss_serializer, RAPTOR_RSS_ITEM, item);

        raptor_rss10_set_item_group(rss_serializer, item->uri, item);
      }
    }

    raptor_free_uri_v2(rss_serializer->world, fake_uri);
  }

  rss_model->items_count=raptor_sequence_size(rss_serializer->items);
}


static void
raptor_rss10_build_xml_names(raptor_serializer *serializer, int is_entry)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_rss_model* rss_model=&rss_serializer->model;
  raptor_uri *base_uri=serializer->base_uri;
  raptor_xml_element *element;
  raptor_qname *qname;
  const unsigned char*root_local_name;
  int i;
  int is_atom=rss_serializer->is_atom;
  const raptor_rss_item_info *item_node_type;
  int item_node_typei;
  const unsigned char* ns_uri;
  raptor_world* world=serializer->world;
  int default_ns_id;
  const unsigned char *default_prefix;
  
  if(is_atom) {
    default_ns_id = ATOM1_0_NS;
    ns_uri = raptor_atom_namespace_uri;
    root_local_name = (is_entry ? (const unsigned char*)"entry" :
                                  (const unsigned char*)"feed");
    item_node_typei = RAPTOR_ATOM_ENTRY;
  } else {
    default_ns_id = RSS1_0_NS;
    ns_uri = raptor_rdf_namespace_uri;
    root_local_name = (const unsigned char*)"RDF";
    item_node_typei = RAPTOR_RSS_ITEM;
  }
  item_node_type = &raptor_rss_items_info[item_node_typei];

  if(serializer->feature_prefix_elements)
    /* declare this NS with standard prefix */
    default_prefix = (const unsigned char*)raptor_rss_namespaces_info[default_ns_id].prefix;
  else
    default_prefix = NULL;

  rss_serializer->default_nspace = raptor_new_namespace(rss_serializer->nstack,
                                                        default_prefix, ns_uri,
                                                        0);
  rss_serializer->free_default_nspace = 1;
  if(serializer->feature_prefix_elements) {
    rss_serializer->nspaces[default_ns_id] = rss_serializer->default_nspace;
    rss_serializer->free_default_nspace = 0;
  }

  rss_serializer->xml_nspace = raptor_new_namespace(rss_serializer->nstack,
                                                    (const unsigned char*)"xml",
                                                    (const unsigned char*)raptor_xml_namespace_uri,
                                                    0);


  /* Now we have a namespace stack, declare the namespaces */
  for(i = 0; i < RAPTOR_RSS_NAMESPACES_SIZE; i++) {
    raptor_uri* uri = serializer->world->rss_namespaces_info_uris[i];
    const unsigned char *prefix;

    prefix = (const unsigned char*)raptor_rss_namespaces_info[i].prefix;
    if(!prefix)
      continue;
    
    if(i == default_ns_id) {
      if(serializer->feature_prefix_elements)
        prefix = NULL;
    }
    
    if(uri) {
      raptor_namespace* nspace;
      nspace = raptor_new_namespace_from_uri(rss_serializer->nstack, prefix,
                                             uri, 0);
      rss_serializer->nspaces[i] = nspace;
    }
  }


  qname = raptor_new_qname_from_namespace_local_name_v2(serializer->world,
                                                        rss_serializer->nspaces[default_ns_id],
                                                        root_local_name,
                                                        NULL);
  if(base_uri)
    base_uri = raptor_uri_copy_v2(rss_serializer->world, base_uri);
  element = raptor_new_xml_element(qname, NULL, base_uri);
  rss_serializer->root_element = element;


  /* Declare the namespaces on the root element */
  raptor_xml_element_declare_namespace(element, rss_serializer->default_nspace);

  for(i = 0; i < RAPTOR_RSS_NAMESPACES_SIZE; i++) {
    const unsigned char *prefix;

    prefix = (const unsigned char*)raptor_rss_namespaces_info[i].prefix;
    if(!prefix && i != default_ns_id)
      continue;

    if(rss_serializer->nspaces[i])
      raptor_xml_element_declare_namespace(element, rss_serializer->nspaces[i]);
  }
  for(i = 0; i < raptor_sequence_size(rss_serializer->user_namespaces); i++) {
    raptor_namespace* nspace;
    nspace = (raptor_namespace*)raptor_sequence_get_at(rss_serializer->user_namespaces, i);

    /* Ignore user setting default namespace prefix */
    if(!nspace->prefix)
      continue;
    
    raptor_xml_element_declare_namespace(element, nspace);
  }


  world->rss_fields_info_qnames=(raptor_qname**)RAPTOR_CALLOC(raptor_qname* array, RAPTOR_RSS_FIELDS_SIZE, sizeof(raptor_qname*));
  if(!world->rss_fields_info_qnames)
    return;
  for(i=0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
    int n=raptor_rss_fields_info[i].nspace;
    raptor_namespace* nspace=rss_serializer->nspaces[n];
    world->rss_fields_info_qnames[i]=raptor_new_qname_from_namespace_local_name_v2(serializer->world,
                                                                                   nspace,
                                                                                   (const unsigned char*)raptor_rss_fields_info[i].name,
                                                                                   NULL);
    if(!world->rss_fields_info_qnames[i])
      return;
  }

  world->rss_types_info_qnames=(raptor_qname**)RAPTOR_CALLOC(raptor_qname* array, RAPTOR_RSS_COMMON_SIZE, sizeof(raptor_qname*));
  if(!world->rss_types_info_qnames)
    return;
  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    int n=raptor_rss_items_info[i].nspace;
    raptor_namespace* nspace=rss_serializer->nspaces[n];
    if(nspace) {
      world->rss_types_info_qnames[i]=raptor_new_qname_from_namespace_local_name_v2(serializer->world,
                                                                                    nspace,
                                                                                    (const unsigned char*)raptor_rss_items_info[i].name,
                                                                                    NULL);
      if(!world->rss_types_info_qnames[i])
        return;
    }
  }
  
  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    raptor_rss_item* item;
    for (item=rss_model->common[i]; item; item=item->next) {
      int typei=i;
      if(!item->fields_count)
        continue;
      if(is_atom) {
        if(typei == RAPTOR_RSS_CHANNEL)
          typei=RAPTOR_ATOM_FEED;
        else if(typei == RAPTOR_RSS_ITEM)
          typei=RAPTOR_ATOM_ENTRY;
      }
      item->node_type=&raptor_rss_items_info[typei];
      item->node_typei=typei;
    }
  }

  for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
    raptor_rss_item* item;
    item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
    item->node_type=item_node_type;
    item->node_typei=item_node_typei;
  }

  for(i=0; i < raptor_sequence_size(rss_serializer->enclosures); i++) {
    raptor_rss_item* item;
    item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->enclosures, i);
    item->node_type=&raptor_rss_items_info[RAPTOR_RSS_ENCLOSURE];
    item->node_typei=RAPTOR_RSS_ENCLOSURE;
  }

}


static void
raptor_rss10_emit_atom_triples_map(raptor_serializer *serializer, int is_feed,
                                   const unsigned char* map_element_name)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_xml_writer* xml_writer;
  raptor_uri *base_uri=serializer->base_uri;
  raptor_uri* base_uri_copy=NULL;
  raptor_namespace* at_nspace=rss_serializer->nspaces[ATOMTRIPLES_NS];
  raptor_xml_element* at_map_root_element;
  raptor_qname *at_map_root_qname;
  int i;
  
  xml_writer=rss_serializer->xml_writer;

  at_map_root_qname=raptor_new_qname_from_namespace_local_name_v2(serializer->world, at_nspace,
                                                                  (const unsigned char*)map_element_name,  NULL);
  base_uri_copy=base_uri ? raptor_uri_copy_v2(rss_serializer->world, base_uri) : NULL;
  at_map_root_element=raptor_new_xml_element(at_map_root_qname, NULL,
                                             base_uri_copy);
  
  raptor_xml_writer_start_element(xml_writer, at_map_root_element);
  
  /* Walk list of fields mapped atom to rss */
  for(i=0; raptor_atom_to_rss[i].from != RAPTOR_RSS_FIELD_UNKNOWN; i++) {
    int from_f=raptor_atom_to_rss[i].from;
    int to_f=raptor_atom_to_rss[i].to;
    const raptor_rss_field_info* from_field_info = &raptor_rss_fields_info[from_f];
    const raptor_rss_field_info* to_field_info = &raptor_rss_fields_info[to_f];
    raptor_xml_element* at_map_element;
    raptor_qname *at_map_qname;
    raptor_qname** at_map_attrs;
    const char* predicate_prefix;
    unsigned char* ruri_string;
    
    /* Do not rewrite to atom0.3 terms */
    if(to_field_info->nspace == ATOM0_3_NS)
      continue;

    /* atom:feed only contains some fields that are mapped */
    if(is_feed && !(from_f == RAPTOR_RSS_FIELD_ATOM_ID ||
                    from_f == RAPTOR_RSS_FIELD_ATOM_UPDATED ||
                    from_f == RAPTOR_RSS_FIELD_ATOM_RIGHTS ||
                    from_f == RAPTOR_RSS_FIELD_ATOM_TITLE))
      continue;
    
    predicate_prefix=raptor_rss_namespaces_info[from_field_info->nspace].prefix;
    if(!predicate_prefix)
      continue;
    
    /* <at:map property="{property URI}">{atom element}</at:map> */
    at_map_qname=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                               at_nspace,
                                                               map_element_name,
                                                               NULL);
    base_uri_copy=base_uri ? raptor_uri_copy_v2(rss_serializer->world, base_uri) : NULL;
    at_map_element=raptor_new_xml_element(at_map_qname, NULL, base_uri_copy);
    
    
    at_map_attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1,
                                                sizeof(raptor_qname*));
    ruri_string=raptor_uri_to_relative_uri_string_v2(serializer->world, base_uri, serializer->world->rss_fields_info_uris[to_f]);
    at_map_attrs[0]=raptor_new_qname(rss_serializer->nstack, 
                                     (const unsigned char*)"property", 
                                     ruri_string,
                                     NULL, NULL); /* errors */
    raptor_free_memory(ruri_string);
    raptor_xml_element_set_attributes(at_map_element, at_map_attrs, 1);
    
    raptor_xml_writer_start_element(xml_writer, at_map_element);
    raptor_xml_writer_cdata(xml_writer, (const unsigned char*)predicate_prefix);
    raptor_xml_writer_cdata_counted(xml_writer, (const unsigned char*)":", 1);
    raptor_xml_writer_cdata(xml_writer,
                            (const unsigned char*)from_field_info->name);
    raptor_xml_writer_end_element(xml_writer, at_map_element);
    
    raptor_free_xml_element(at_map_element);
  }
  
  raptor_xml_writer_end_element(xml_writer, at_map_root_element);
    
  raptor_free_xml_element(at_map_root_element);
}



/* atom-specific feed XML elements */
static void
raptor_rss10_emit_atom_feed(raptor_serializer *serializer,
                            raptor_rss_item *item)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_xml_writer* xml_writer;
  raptor_uri *base_uri=serializer->base_uri;
  raptor_uri* base_uri_copy=NULL;
  raptor_xml_element* atom_link_element;
  raptor_qname *atom_link_qname;
  raptor_qname** atom_link_attrs;
  raptor_namespace* atom_nspace=rss_serializer->nspaces[ATOM1_0_NS];
  unsigned char* ruri_string;

  xml_writer=rss_serializer->xml_writer;

  atom_link_qname=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                                atom_nspace,
                                                                (const unsigned char*)"link",
                                                                NULL);
  base_uri_copy=base_uri ? raptor_uri_copy_v2(rss_serializer->world, base_uri) : NULL;
  atom_link_element=raptor_new_xml_element(atom_link_qname, NULL, base_uri_copy);

  atom_link_attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 2, sizeof(raptor_qname*));
  ruri_string = raptor_uri_to_relative_uri_string_v2(rss_serializer->world,
                                                     base_uri, item->uri);

  atom_link_attrs[0]=raptor_new_qname(rss_serializer->nstack, 
                                      (const unsigned char*)"href", 
                                      ruri_string,
                                      NULL, NULL); /* errors */
  raptor_free_memory(ruri_string);
  atom_link_attrs[1]=raptor_new_qname(rss_serializer->nstack, 
                                      (const unsigned char*)"rel", 
                                      (const unsigned char*)"self",
                                      NULL, NULL); /* errors */
  raptor_xml_element_set_attributes(atom_link_element, atom_link_attrs, 2);
  
  raptor_xml_writer_empty_element(xml_writer, atom_link_element);

  raptor_free_xml_element(atom_link_element);

  if(rss_serializer->rss_triples_mode == 2) {
    raptor_rss10_emit_atom_triples_map(serializer, 1,
                                       (const unsigned char*)"feedmap");
    raptor_rss10_emit_atom_triples_map(serializer, 0,
                                       (const unsigned char*)"entrymap");
  }
}


/* emit the RSS 1.0-specific  rdf:Seq and rss:item XML elements */
static void
raptor_rss10_emit_rss_items(raptor_serializer *serializer)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_xml_writer* xml_writer;
  raptor_uri *base_uri=serializer->base_uri;
  raptor_uri* base_uri_copy=NULL;
  raptor_xml_element* rss_items_predicate;
  int i;
  raptor_qname *rdf_Seq_qname;
  raptor_xml_element *rdf_Seq_element;
  
  if(!raptor_sequence_size(rss_serializer->items))
    return;
  
  xml_writer=rss_serializer->xml_writer;

  rdf_Seq_qname=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                              rss_serializer->default_nspace,
                                                              (const unsigned char*)"Seq",
                                                              NULL);
  
  base_uri_copy=base_uri ? raptor_uri_copy_v2(rss_serializer->world, base_uri) : NULL;
  rdf_Seq_element=raptor_new_xml_element(rdf_Seq_qname, NULL, base_uri_copy);
  
  /* make the <rss:items><rdf:Seq><rdf:li /> .... </rdf:Seq></rss:items> */
  
  base_uri_copy=base_uri ? raptor_uri_copy_v2(rss_serializer->world, base_uri) : NULL;
  rss_items_predicate=raptor_new_xml_element(raptor_qname_copy(serializer->world->rss_fields_info_qnames[RAPTOR_RSS_FIELD_ITEMS]), NULL, base_uri_copy);
  
  raptor_xml_writer_start_element(xml_writer, rss_items_predicate);
  
  raptor_xml_writer_start_element(xml_writer, rdf_Seq_element);
  
  for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
    raptor_rss_item* item_item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
    raptor_qname *rdf_li_qname;
    raptor_xml_element *rdf_li_element;
    raptor_qname **attrs;
    unsigned char* ruri_string;
    
    rdf_li_qname=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                               rss_serializer->default_nspace,
                                                               (const unsigned char*)"li",
                                                               NULL);
    base_uri_copy=base_uri ? raptor_uri_copy_v2(rss_serializer->world, base_uri) : NULL;
    rdf_li_element=raptor_new_xml_element(rdf_li_qname, NULL, base_uri_copy);
    attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname*));
    ruri_string=raptor_uri_to_relative_uri_string_v2(rss_serializer->world, base_uri, item_item->uri);
    attrs[0]=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                           rss_serializer->default_nspace,
                                                           (const unsigned char*)"resource",
                                                           ruri_string);
    raptor_free_memory(ruri_string);
    raptor_xml_element_set_attributes(rdf_li_element, attrs, 1);
    
    raptor_xml_writer_empty_element(xml_writer, rdf_li_element);
    
    raptor_xml_writer_newline(xml_writer);
    
    raptor_free_xml_element(rdf_li_element);
  }
  
  raptor_xml_writer_end_element(xml_writer, rdf_Seq_element);
  
  raptor_free_xml_element(rdf_Seq_element);
  
  raptor_xml_writer_end_element(xml_writer, rss_items_predicate);
  
  raptor_free_xml_element(rss_items_predicate);
}


/* emit a block of RDF/XML depending on the rssTriples feature mode */
static void
raptor_rss10_emit_rdfxml_item_triples(raptor_serializer *serializer,
                                      raptor_rss_item *item)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_xml_writer* xml_writer;
  raptor_qname* root_qname=NULL;
  raptor_xml_element* root_element=NULL;
  raptor_serializer* ser=NULL;
  raptor_uri* base_uri=NULL;
  int t_max_count=raptor_sequence_size(item->triples);
  int t_count;
  int t;
  int is_atom;
  
  if(rss_serializer->rss_triples_mode == 0 || !item->triples)
    return;

  xml_writer=rss_serializer->xml_writer;
  is_atom=rss_serializer->is_atom;

  /* can only use atom-triples with atom serializer */
  if(rss_serializer->rss_triples_mode == 2 && !is_atom)
    return;
  
  /* can only use rdf-xml with rss-1.0 serializer */
  if(rss_serializer->rss_triples_mode == 1 && is_atom)
    return;
  
  t_count=0;
  for(t=0; t < t_max_count; t++) {
    if(raptor_sequence_get_at(item->triples, t))
      t_count++;
  }
  if(!t_count)
    return;

  RAPTOR_DEBUG2("Serializing %d triples\n", t_count);
  
  if(is_atom) {
    raptor_namespace* at_nspace=rss_serializer->nspaces[ATOMTRIPLES_NS];

    /* atom:md with no attribute */
    root_qname=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                             at_nspace,
                                                             (const unsigned char*)"md",
                                                             NULL);
    if(!root_qname)
      goto oom;
    
    base_uri=serializer->base_uri;
    if(base_uri)
      base_uri=raptor_uri_copy_v2(rss_serializer->world, base_uri);
    
    /* after this root_element owns root_qname and (this copy of) base_uri */
    root_element=raptor_new_xml_element(root_qname, NULL, base_uri);
    if(!root_element) {
      if(base_uri)
        raptor_free_uri_v2(rss_serializer->world, base_uri);
      raptor_free_qname(root_qname); root_qname=NULL;
      goto oom;
    }
    root_qname=NULL;

    raptor_xml_writer_start_element(xml_writer, root_element);
  }
  
  ser=raptor_new_serializer_v2(rss_serializer->world, "rdfxml-abbrev");
  if(!ser)
    goto oom;
  
  raptor_rdfxmla_serialize_set_xml_writer(ser, xml_writer,
                                          rss_serializer->nstack);
  raptor_rdfxmla_serialize_set_write_rdf_RDF(ser, 0);
  raptor_rdfxmla_serialize_set_single_node(ser, item->uri);
  if(rss_serializer->rss_triples_mode == 2) {
    /* raptor_rdfxmla_serialize_set_write_typed_nodes(ser, 0); */
  }
  
  if(base_uri)
    base_uri=raptor_uri_copy_v2(rss_serializer->world, base_uri);

  /* after this call, ser owns (this copy of) base_uri and does
   * NOT own serializer->iostream and will not destroy it
   * when raptor_free_serializer(ser) is called.
   */
  raptor_serialize_start_to_iostream(ser, base_uri, serializer->iostream);
  
  for(t=0; t < t_max_count; t++) {
    raptor_statement_v2* s;
    s=(raptor_statement_v2*)raptor_sequence_get_at(item->triples, t);
    if(s)
      raptor_serialize_statement(ser, s->s);
  }
  
  raptor_serialize_end(ser);
  
  raptor_free_serializer(ser); ser=NULL;

  if(is_atom)
    raptor_xml_writer_end_element(xml_writer, root_element);
  
  oom:
  if(ser)
    raptor_free_serializer(ser);
  if(root_qname)
    raptor_free_qname(root_qname);
  if(root_element)
    raptor_free_xml_element(root_element);
}


/**
 * raptor_rss10_ensure_atom_field_zero_one:
 * @item: RSS item object
 * @f: ATOM field type
 *
 * INTERNAL - Check that the given item @field appears 0 or 1 times
 */
static void
raptor_rss10_ensure_atom_field_zero_one(raptor_rss_item* item, 
                                        raptor_rss_fields_type f)
{
  raptor_rss_field* field=item->fields[f];
  if(!field)
    return;

  if(field->next) {
    /* more than 1 value so delete rest of values */
    raptor_rss_field* next=field->next;
    field->next=NULL;

    do {
      field=next;

      next=field->next;
      field->next=NULL;
      raptor_rss_field_free(field);
    } while(next);
  }

}


/**
 * raptor_rss10_ensure_atom_feed_valid:
 * @rss_serializer: serializer object
 *
 * INTERNAL - Ensure the atom items have all the fields they need:
 *   <id> & <title> & <updated>
 * plus:
 *   <link rel='alternate' ...> OR <content>..
 *
 */
static int
raptor_rss10_ensure_atom_feed_valid(raptor_rss10_serializer_context *rss_serializer)
{
  int is_atom;
  int i;
  raptor_rss_item* item;
  raptor_rss_model* rss_model;
  struct timeval tv;
  time_t now = 0;

#ifdef HAVE_GETTIMEOFDAY
  if(!gettimeofday(&tv, NULL))
    now = tv.tv_sec;
#endif
  
  is_atom=rss_serializer->is_atom;
  rss_model=&rss_serializer->model;

  if(!is_atom)
    return 0;

  item=rss_model->common[RAPTOR_RSS_CHANNEL];
  if(item) {
    int f;

    /* atom:id is required */
    f=RAPTOR_RSS_FIELD_ATOM_ID;
    if(!item->fields[f]) {
      raptor_rss_field* field=raptor_rss_new_field(rss_serializer->world);
      field->uri=raptor_uri_copy_v2(rss_serializer->world, item->uri);
      raptor_rss_item_add_field(item, f, field);
    }

    /* atom:updated is required */
    f=RAPTOR_RSS_FIELD_ATOM_UPDATED;
    if(!item->fields[f]) {
      raptor_rss_field* field=raptor_rss_new_field(rss_serializer->world);
      raptor_rss_set_date_field(field, now);
      raptor_rss_item_add_field(item, f, field);
    }

    /* atom:content is forbidden in feed */
    f=RAPTOR_RSS_FIELD_ATOM_CONTENT;
    if(item->fields[f]) {
      raptor_rss_field_free(item->fields[f]);
      item->fields[f]=NULL;
    }

    /* atom:summary is forbidden in feed */
    f=RAPTOR_RSS_FIELD_ATOM_SUMMARY;
    if(item->fields[f]) {
      raptor_rss_field_free(item->fields[f]);
      item->fields[f]=NULL;
    }

    /* These fields can appear 0 or 1 times on a feed */ 
    raptor_rss10_ensure_atom_field_zero_one(item,
                                            RAPTOR_RSS_FIELD_ATOM_ICON);
    raptor_rss10_ensure_atom_field_zero_one(item,
                                            RAPTOR_RSS_FIELD_ATOM_LOGO);
    raptor_rss10_ensure_atom_field_zero_one(item,
                                            RAPTOR_RSS_FIELD_ATOM_RIGHTS);
    raptor_rss10_ensure_atom_field_zero_one(item,
                                            RAPTOR_RSS_FIELD_ATOM_SUBTITLE);
  }
  

  for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
    item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);

    /* atom:id - defaults to item URI */
    if(!item->fields[RAPTOR_RSS_FIELD_ATOM_ID]) {
      raptor_rss_field* field=raptor_rss_new_field(rss_serializer->world);
      field->uri=raptor_uri_copy_v2(rss_serializer->world, item->uri);
      raptor_rss_item_add_field(item, RAPTOR_RSS_FIELD_ATOM_ID, field);
    }

    /* atom:title - defaults to "untitled" */
    if(!item->fields[RAPTOR_RSS_FIELD_ATOM_TITLE]) {
      raptor_rss_field* field=raptor_rss_new_field(rss_serializer->world);
      field->value=(unsigned char*)RAPTOR_MALLOC(cstring, 9);
      strncpy((char*)field->value, "untitled", 9);
      raptor_rss_item_add_field(item, RAPTOR_RSS_FIELD_ATOM_TITLE, field);
    }
    
    /* atom:updated - defaults to now time */
    if(!item->fields[RAPTOR_RSS_FIELD_ATOM_UPDATED]) {
      raptor_rss_field* field=raptor_rss_new_field(rss_serializer->world);
      raptor_rss_set_date_field(field, now);
      raptor_rss_item_add_field(item, RAPTOR_RSS_FIELD_ATOM_UPDATED, field);
    }
    
    /* enforce there is either an atom:content OR atom:link (rel=alternate) 
     * by adding a link to {item URI} if missing
     */
    if(!item->fields[RAPTOR_RSS_FIELD_ATOM_CONTENT] &&
       !item->fields[RAPTOR_RSS_FIELD_ATOM_LINK]) {
      raptor_rss_field* field=raptor_rss_new_field(rss_serializer->world);
      field->uri=raptor_uri_copy_v2(rss_serializer->world, item->uri);
      raptor_rss_item_add_field(item, RAPTOR_RSS_FIELD_ATOM_LINK, field);
    }

    /* These fields can appear 0 or 1 times on an entry */ 
    raptor_rss10_ensure_atom_field_zero_one(item,
                                            RAPTOR_RSS_FIELD_ATOM_PUBLISHED);
    raptor_rss10_ensure_atom_field_zero_one(item,
                                            RAPTOR_RSS_FIELD_ATOM_RIGHTS);
    raptor_rss10_ensure_atom_field_zero_one(item,
                                            RAPTOR_RSS_FIELD_ATOM_SOURCE);
    raptor_rss10_ensure_atom_field_zero_one(item,
                                            RAPTOR_RSS_FIELD_ATOM_SUMMARY);
  }
  
  return 0;
}


static void
raptor_rss10_emit_item(raptor_serializer* serializer,
                       raptor_rss_item *item, int item_type,
                       int emit_container) 
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_xml_writer* xml_writer;
  raptor_rss_model* rss_model;
  raptor_uri *base_uri=serializer->base_uri;
  raptor_xml_element *element=NULL;
  raptor_qname **attrs=NULL;
  raptor_uri* base_uri_copy=NULL;
  int fi;
  int is_atom;

#ifdef RAPTOR_DEBUG
  if(!item) {
    RAPTOR_FATAL3("Tried to emit NULL item of type %d - %s\n", item_type,
                  raptor_rss_items_info[item_type].name);
  }
#endif

  xml_writer=rss_serializer->xml_writer;
  is_atom=rss_serializer->is_atom;
  rss_model=&rss_serializer->model;

  if(!item->fields_count) {
    int i;
    for(i=0; i < raptor_sequence_size(rss_serializer->enclosures); i++) {
      raptor_rss_item *enclosure_item;
      enclosure_item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->enclosures, i);
      /* If the item and enclosure item have the same URI, move the
       * enclosure fields to the item.  Assumed that they got conflated
       * previously such as when the enclosure url = the guid
       */
      if(enclosure_item->uri &&
         raptor_uri_equals_v2(rss_serializer->world, item->uri, enclosure_item->uri)) {
        int j;
        for (j=0; j < RAPTOR_RSS_FIELDS_SIZE;j++) {
          if (j != RAPTOR_RSS_RDF_ENCLOSURE_TYPE &&
              j != RAPTOR_RSS_RDF_ENCLOSURE_LENGTH &&
              j != RAPTOR_RSS_RDF_ENCLOSURE_URL) {
            item->fields[j]=enclosure_item->fields[j];
            enclosure_item->fields[j]=NULL;
            item->fields_count++;
            enclosure_item->fields_count--;
          }
        }
        break;
      }
    }
  }

  if(!item->fields_count)
    return;

  if(emit_container) {
    base_uri_copy=base_uri ? raptor_uri_copy_v2(rss_serializer->world, base_uri) : NULL;
    element=raptor_new_xml_element(raptor_qname_copy(serializer->world->rss_types_info_qnames[item->node_typei]), NULL, base_uri_copy);
    if(!is_atom && item->uri) {
      unsigned char* ruri_string;
      attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname*));
      ruri_string=raptor_uri_to_relative_uri_string_v2(rss_serializer->world, base_uri, item->uri);
      attrs[0]=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                             rss_serializer->default_nspace,
                                                             (const unsigned char*)"about",
                                                             ruri_string);
      raptor_free_memory(ruri_string);
      raptor_xml_element_set_attributes(element, attrs, 1);
    }

    raptor_xml_writer_start_element(xml_writer, element);
  }
  

  for(fi = 0; fi < RAPTOR_RSS_FIELDS_SIZE; fi++) {
    raptor_rss_fields_type f = (raptor_rss_fields_type)fi;
    raptor_rss_field* field;

    if(f == RAPTOR_RSS_FIELD_ITEMS)
      /* emitting the RSS items rdf:Seq block is done after this loop */
      continue;

    if(!serializer->world->rss_fields_info_uris[f])
      continue;
    
    if(f == RAPTOR_RSS_FIELD_ATOM_AUTHOR) {
      int typei;
      
      if(!is_atom)
        continue;

      if(item_type != RAPTOR_RSS_CHANNEL)
        continue;
      
      typei=RAPTOR_ATOM_AUTHOR;
      if(!rss_model->common[typei]) {
        raptor_rss_item* author_item;
        raptor_identifier* identifier;

        /* No atom author was present so make a new atom:author item
         * then either promote the string to an atom:name field OR
         * use "unknown"
         */
        author_item=raptor_rss_model_add_common(rss_model, (raptor_rss_type)typei);
        identifier=&(author_item->identifier);

        author_item->node_type=&raptor_rss_items_info[typei];
        author_item->node_typei=typei;
        /* FIXME - uses _:author as bnode name - should make a new
         * genid for each author node.  This is OK because there
         * is a check above that there is only 1 author per FEED.
         */
        identifier->id=(const unsigned char*)RAPTOR_MALLOC(cstring, 7);
        strncpy((char*)identifier->id, "author", 7);

        identifier->type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
        identifier->uri_source=RAPTOR_URI_SOURCE_GENERATED;

        /* Move atom:name author field, or create a dummy one */
        f=RAPTOR_RSS_FIELD_ATOM_NAME;
        if(item->fields[f]) {
          field=item->fields[f];
          item->fields[f]=NULL;
        } else {
          field=raptor_rss_new_field(serializer->world);
          field->value=(unsigned char*)RAPTOR_MALLOC(cstring, 8);
          strncpy((char*)field->value, "unknown", 8);
        }
        raptor_rss_item_add_field(author_item, RAPTOR_RSS_FIELD_ATOM_NAME, field);

        /* Move atom author fields if found: atom:uri and atom:email
         * are only used inside Person constructs
         */
        f=RAPTOR_RSS_FIELD_ATOM_URI;
        if(item->fields[f]) {
          field=item->fields[f];
          raptor_rss_item_add_field(author_item, f, field);
          item->fields[f]=NULL;
        }
        f=RAPTOR_RSS_FIELD_ATOM_EMAIL;
        if(item->fields[f]) {
          field=item->fields[f];
          raptor_rss_item_add_field(author_item, f, field);
          item->fields[f]=NULL;
        }
      }
      
      RAPTOR_DEBUG3("Emitting type %i - %s\n", typei, 
                    raptor_rss_items_info[typei].name);
      raptor_rss10_emit_item(serializer, rss_model->common[typei], typei, 
                             1);
      continue;
    }
    

    for (field=item->fields[f]; field; field=field->next) {
      raptor_xml_element* predicate;

      base_uri_copy=base_uri ? raptor_uri_copy_v2(rss_serializer->world, base_uri) : NULL;
      predicate=raptor_new_xml_element(raptor_qname_copy(serializer->world->rss_fields_info_qnames[f]), NULL, base_uri_copy);

      /* Use atom:summary in preference */
      if(is_atom && f == RAPTOR_RSS_FIELD_DESCRIPTION)
        continue;

      if(is_atom && field->uri) {
        unsigned char* ruri_string;
        size_t len;
        raptor_uri* my_base_uri=base_uri;
        
        if(f == RAPTOR_RSS_FIELD_ATOM_ID)
          my_base_uri=NULL;
        
        ruri_string=raptor_uri_to_relative_counted_uri_string_v2(rss_serializer->world,
                                                                 my_base_uri, 
                                                                 field->uri, &len);

        if(f == RAPTOR_RSS_FIELD_ATOM_LINK &&
           !item->fields[RAPTOR_RSS_FIELD_ATOM_CONTENT]) {
          /* atom:link to URI and there is no atom:content */
          raptor_qname **predicate_attrs=NULL;
          predicate_attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 2, 
                                                         sizeof(raptor_qname*));
          predicate_attrs[0]=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                                           NULL,
                                                                           (const unsigned char*)"href",
                                                                           ruri_string);
          predicate_attrs[1]=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                                           NULL, 
                                                                           (const unsigned char*)"rel", 
                                                                           (const unsigned char*)"alternate");
          field->value=NULL;
          raptor_xml_element_set_attributes(predicate, predicate_attrs, 2);
          raptor_xml_writer_empty_element(xml_writer, predicate);
        } else if(f == RAPTOR_RSS_FIELD_ATOM_CONTENT) {
          /* <atom:content src="{uri value}" type="{type}" /> */
          raptor_qname **predicate_attrs=NULL;
          const unsigned char* content_type;
          raptor_rss_field* content_type_field;

          /* get the type */
          content_type_field=item->fields[RAPTOR_RSS_FIELD_AT_CONTENT_TYPE];
          if(content_type_field && content_type_field->value)
            content_type=content_type_field->value;
          else
            /* FIXME - default content type */
            content_type=(const unsigned char*)"text/html";

          predicate_attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 2, 
                                                         sizeof(raptor_qname*));
          predicate_attrs[0]=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                                           NULL,
                                                                           (const unsigned char*)"src",
                                                                           ruri_string);
          predicate_attrs[1]=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                                           NULL, 
                                                                           (const unsigned char*)"type", 
                                                                           (const unsigned char*)content_type);
          /* free at:contentType field - no need to emit it */
          if(content_type_field) {
            raptor_rss_field_free(content_type_field);
            item->fields[RAPTOR_RSS_FIELD_AT_CONTENT_TYPE]=NULL;
          }

          field->value=NULL;
          raptor_xml_element_set_attributes(predicate, predicate_attrs, 2);
          raptor_xml_writer_empty_element(xml_writer, predicate);
        } else {
          raptor_xml_writer_start_element(xml_writer, predicate);
          raptor_xml_writer_cdata_counted(xml_writer, ruri_string, len);
          raptor_xml_writer_end_element(xml_writer, predicate);
        }
        raptor_free_memory(ruri_string);
        
      } else if (field->uri) {
        raptor_uri* enclosure_uri=field->uri;
        raptor_rss_item *enclosure_item=NULL;
        int i;
        if (f == RAPTOR_RSS_FIELD_ENCLOSURE && item_type == RAPTOR_RSS_ITEM) {
          for(i=0; i < raptor_sequence_size(rss_serializer->enclosures); i++) {
            enclosure_item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->enclosures, i);
            if(enclosure_item->uri && raptor_uri_equals_v2(rss_serializer->world, enclosure_uri, enclosure_item->uri))
              break;
          }
          if (enclosure_item) {
            int attr_count=0;
            unsigned char* ruri_string;

            attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 3, sizeof(raptor_qname*));
            ruri_string=raptor_uri_to_relative_uri_string_v2(rss_serializer->world, base_uri, field->uri);
            attrs[attr_count]=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                                            rss_serializer->default_nspace,
                                                                            (const unsigned char*)"resource",
                                                                            ruri_string);
            raptor_free_memory(ruri_string);
            attr_count++;
            if (enclosure_item->fields[RAPTOR_RSS_RDF_ENCLOSURE_TYPE] && enclosure_item->fields[RAPTOR_RSS_RDF_ENCLOSURE_TYPE]->value) {
              attrs[attr_count]=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                                              rss_serializer->nspaces[RSS2_0_ENC_NS],
                                                                              (const unsigned char*)raptor_rss_fields_info[RAPTOR_RSS_RDF_ENCLOSURE_TYPE].name,
                                                                              (const unsigned char*)enclosure_item->fields[RAPTOR_RSS_RDF_ENCLOSURE_TYPE]->value);
              attr_count++;
            }
            if (enclosure_item->fields[RAPTOR_RSS_RDF_ENCLOSURE_LENGTH] && enclosure_item->fields[RAPTOR_RSS_RDF_ENCLOSURE_LENGTH]->value) {
              attrs[attr_count]=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                                              rss_serializer->nspaces[RSS2_0_ENC_NS],
                                                                              (const unsigned char*)raptor_rss_fields_info[RAPTOR_RSS_RDF_ENCLOSURE_LENGTH].name,
                                                                              (const unsigned char*)enclosure_item->fields[RAPTOR_RSS_RDF_ENCLOSURE_LENGTH]->value);
              attr_count++;
            }
            raptor_xml_element_set_attributes(predicate, attrs, attr_count);
          } else {
            RAPTOR_DEBUG2("Enclosure item with URI %s could not be found in list of enclosures\n", raptor_uri_as_string_v2(rss_serializer->world, enclosure_uri));
          }
        } else {
          unsigned char* ruri_string;

          /* not an rss:item with an rss:enclosure field */
          attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname*));
          ruri_string=raptor_uri_to_relative_uri_string_v2(rss_serializer->world, base_uri, field->uri);
          attrs[0]=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                                 rss_serializer->default_nspace,
                                                                 (const unsigned char*)"resource",
                                                                 ruri_string);
          raptor_free_memory(ruri_string);
          raptor_xml_element_set_attributes(predicate, attrs, 1);
        }
        raptor_xml_writer_empty_element(xml_writer, predicate);		
      } else if(field->value) {
        /* not a URI, must be a literal */
        int is_xhtml_content=field->is_xml;
        int prefer_cdata=(!is_atom && f == RAPTOR_RSS_FIELD_CONTENT_ENCODED);
        
        if(is_xhtml_content && !prefer_cdata) {
          raptor_qname **predicate_attrs=NULL;
          predicate_attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname*));
          if(is_atom)
            predicate_attrs[0]=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                                             NULL,
                                                                             (const unsigned char*)"type",
                                                                             (const unsigned char*)"xhtml");
          else
            predicate_attrs[0]=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                                             rss_serializer->default_nspace,
                                                                             (const unsigned char*)"parseType",
                                                                             (const unsigned char*)"Literal");
          raptor_xml_element_set_attributes(predicate, predicate_attrs, 1);
        }

        raptor_xml_writer_start_element(xml_writer, predicate);
        if(is_xhtml_content) {
          if(prefer_cdata)
            raptor_xml_writer_raw_counted(xml_writer,
                                          (const unsigned char*)"<![CDATA[", 9);
          raptor_xml_writer_raw(xml_writer, (const unsigned char*)field->value);
          if(prefer_cdata)
            raptor_xml_writer_raw_counted(xml_writer, 
                                          (const unsigned char*)"]]>", 3);
        } else
          raptor_xml_writer_cdata(xml_writer, (const unsigned char*)field->value);
        raptor_xml_writer_end_element(xml_writer, predicate);
      } else {
        RAPTOR_DEBUG3("Field %d - %s had no URI or literal value\n",
                      f, raptor_rss_fields_info[f].name);
      }
      raptor_free_xml_element(predicate);
    }
  }


  if(item_type == RAPTOR_RSS_CHANNEL) {
    if(is_atom)
      raptor_rss10_emit_atom_feed(serializer, item);

    if(!is_atom)
      raptor_rss10_emit_rss_items(serializer);
  }
    
  /* Add an RDF/XML block with remaining triples if Atom */
  if(item->triples && raptor_sequence_size(item->triples))
    raptor_rss10_emit_rdfxml_item_triples(serializer, item);

  if(emit_container) {
    raptor_xml_writer_end_element(xml_writer, element);
    raptor_free_xml_element(element);
  }

}


/**
 * raptor_rss10_serialize_end:
 * @serializer: serializer object
 *
 * INTERNAL (raptor_serializer_factory API) - End a serializing
 *
 * Return value: non-0 on failure
 */
static int
raptor_rss10_serialize_end(raptor_serializer* serializer) {
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_rss_model* rss_model;
  int i;
  raptor_xml_writer* xml_writer;
#ifdef RAPTOR_DEBUG
  int triple_count=0;
#endif
  int is_atom;
  raptor_qname **attrs=NULL;
  int attrs_count=0;
  raptor_uri* entry_uri=NULL;
  raptor_rss_item* entry_item=NULL;
  
  rss_model=&rss_serializer->model;
  is_atom=rss_serializer->is_atom;

  raptor_rss10_build_items(rss_serializer);

  raptor_rss10_move_leftover_statements(rss_serializer);

  raptor_rss10_move_anonymous_statements(rss_serializer);

  if(is_atom) {
    raptor_rss10_ensure_atom_feed_valid(rss_serializer);

    raptor_rss10_remove_mapped_fields(rss_serializer);

    if(serializer->feature_atom_entry_uri) {
      entry_uri=raptor_new_uri_v2(rss_serializer->world, serializer->feature_atom_entry_uri);
      for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
        raptor_rss_item* item;
        item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
        if(raptor_uri_equals_v2(rss_serializer->world, item->uri, entry_uri)) {
          entry_item=item;
          break;
        }
      }
      if(!entry_item) {
        RAPTOR_DEBUG2("Entry URI %s was not found in list of items\n",
                      raptor_uri_as_string_v2(rss_serializer->world, entry_uri));
        raptor_free_uri_v2(rss_serializer->world, entry_uri);
        entry_uri=NULL;
      }
    }

  }

#ifdef RAPTOR_DEBUG
  for(i=0; i < raptor_sequence_size(rss_serializer->triples); i++) {
    raptor_statement_v2* t=(raptor_statement_v2*)raptor_sequence_get_at(rss_serializer->triples, i);
    if(t) {
      fprintf(stderr, " %d: ", i);
      raptor_print_statement_v2(t, stderr);
      fputc('\n', stderr);
      triple_count++;
    }
  }
  RAPTOR_DEBUG2("Starting with %d stored triples\n", triple_count);
#endif

  if(!rss_model->common[RAPTOR_RSS_CHANNEL]) {
    raptor_serializer_error(serializer, "No RSS channel found");
    return 1;
  }
  
  
  if(rss_serializer->xml_writer)
    raptor_free_xml_writer(rss_serializer->xml_writer);

  xml_writer=raptor_new_xml_writer_v2(rss_serializer->world,
                                      rss_serializer->nstack,
                                      serializer->iostream,
                                      NULL, NULL, /* errors */
                                      1);
  rss_serializer->xml_writer=xml_writer;
  raptor_xml_writer_set_feature(xml_writer,
                                RAPTOR_FEATURE_WRITER_AUTO_INDENT, 1);
  raptor_xml_writer_set_feature(xml_writer,
                                RAPTOR_FEATURE_WRITER_AUTO_EMPTY, 1);

  raptor_rss10_build_xml_names(serializer, (is_atom && entry_uri));

  if(serializer->base_uri && serializer->feature_write_base_uri) {
    const unsigned char* base_uri_string;

    attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname*));

    base_uri_string=raptor_uri_as_string_v2(rss_serializer->world, serializer->base_uri);
    attrs[attrs_count++]=raptor_new_qname_from_namespace_local_name_v2(rss_serializer->world,
                                                                       rss_serializer->xml_nspace,
                                                                       (const unsigned char*)"base",
                                                                       base_uri_string);
  }

  if(attrs_count)
    raptor_xml_element_set_attributes(rss_serializer->root_element, attrs, 
                                      attrs_count);
  else
    raptor_xml_element_set_attributes(rss_serializer->root_element, NULL, 0);

  raptor_xml_writer_start_element(xml_writer, rss_serializer->root_element);


  if(entry_item) {
    RAPTOR_DEBUG1("Emitting entry\n");
    raptor_rss10_emit_item(serializer, entry_item, RAPTOR_RSS_ITEM, 0);
    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
  } else {
    i=RAPTOR_RSS_CHANNEL;
    RAPTOR_DEBUG3("Emitting type %i - %s\n", i, raptor_rss_items_info[i].name);
    raptor_rss10_emit_item(serializer, rss_model->common[i], i, !is_atom);
    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);

    if(rss_model->items_count) {
      for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
        raptor_rss_item* item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
        raptor_rss10_emit_item(serializer, item, RAPTOR_RSS_ITEM, 1);
        raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
      }
      
    }

    for(i=RAPTOR_RSS_CHANNEL+1; i< RAPTOR_RSS_COMMON_SIZE; i++) {
      raptor_rss_item* item;

      if(is_atom) {
        /* atom 1.0 only serializes rss:item (channel is done above) */
        if(i != RAPTOR_RSS_ITEM)
          continue;
      } else {
        /* rss 1.0 ignores atom:author for now - FIXME */
        if(i == RAPTOR_ATOM_AUTHOR)
          continue;
      }

      for (item=rss_model->common[i]; item; item=item->next) {
        RAPTOR_DEBUG3("Emitting type %i - %s\n", i, raptor_rss_items_info[i].name);
        raptor_rss10_emit_item(serializer, item, i, 1);
      }
    }
  }


  raptor_xml_writer_end_element(xml_writer, rss_serializer->root_element);

  raptor_free_xml_element(rss_serializer->root_element);

  raptor_xml_writer_newline(xml_writer);

  raptor_xml_writer_flush(xml_writer);

  return 0;
}


/* add a namespace */
static int
raptor_rss10_serialize_declare_namespace_from_namespace(raptor_serializer* serializer,
                                                        raptor_namespace *nspace)
{
  raptor_rss10_serializer_context* rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  int i;

  for(i=0; i< raptor_sequence_size(rss_serializer->user_namespaces); i++) {
    raptor_namespace* ns;
    ns=(raptor_namespace*)raptor_sequence_get_at(rss_serializer->user_namespaces, i);

    /* If prefix is already declared, ignore it */
    if(!ns->prefix && !nspace->prefix)
      return 1;

    if(ns->prefix && nspace->prefix &&
       !strcmp((const char*)ns->prefix, (const char*)nspace->prefix))
      return 1;

    if(ns->uri && nspace->uri &&
       raptor_uri_equals_v2(rss_serializer->world, ns->uri, nspace->uri))
      return 1;
  }

  nspace=raptor_new_namespace_from_uri(rss_serializer->nstack,
                                       nspace->prefix, nspace->uri,
                                       0);
  if(!nspace)
    return 1;

  raptor_sequence_push(rss_serializer->user_namespaces, nspace);
  return 0;
}


/* add a namespace */
static int
raptor_rss10_serialize_declare_namespace(raptor_serializer* serializer,
                                         raptor_uri *uri,
                                         const unsigned char *prefix)
{
  raptor_rss10_serializer_context* rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_namespace *ns;
  int rc;

  ns=raptor_new_namespace_from_uri(rss_serializer->nstack, prefix, uri, 0);
  rc=raptor_rss10_serialize_declare_namespace_from_namespace(serializer, ns);
  raptor_free_namespace(ns);

  return rc;
}



/**
 * raptor_rss10_serialize_finish_factory:
 * @factory: serializer factory
 *
 * INTERNAL (raptor_serializer_factory API) - finish the serializer factory
 */
static void
raptor_rss10_serialize_finish_factory(raptor_serializer_factory* factory)
{

}


static int
raptor_rss10_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->context_length     = sizeof(raptor_rss10_serializer_context);
  
  factory->init                = raptor_rss10_serialize_init;
  factory->terminate           = raptor_rss10_serialize_terminate;
  factory->declare_namespace   = raptor_rss10_serialize_declare_namespace;
  factory->declare_namespace_from_namespace   = raptor_rss10_serialize_declare_namespace_from_namespace;
  factory->serialize_start     = raptor_rss10_serialize_start;
  factory->serialize_statement = raptor_rss10_serialize_statement;
  factory->serialize_end       = raptor_rss10_serialize_end;
  factory->finish_factory      = raptor_rss10_serialize_finish_factory;

  return 0;
}



int
raptor_init_serializer_rss10(raptor_world* world) {
  return raptor_serializer_register_factory(world,
                                            "rss-1.0", "RSS 1.0",
                                            NULL, 
                                            NULL,
                                            (const unsigned char*)"http://purl.org/rss/1.0/spec",
                                            &raptor_rss10_serializer_register_factory);
}

int
raptor_init_serializer_atom(raptor_world* world) {
  return raptor_serializer_register_factory(world,
                                            "atom", "Atom 1.0",
                                            "application/atom+xml", 
                                            NULL,
                                            NULL,
                                            &raptor_rss10_serializer_register_factory);
}

