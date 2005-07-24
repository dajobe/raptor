/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_serialize_rss.c - Raptor RSS 1.0 serializer
 *
 * $Id$
 *
 * Copyright (C) 2003-2005, David Beckett http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology http://www.ilrt.bristol.ac.uk/
 * University of Bristol, UK http://www.bristol.ac.uk/
 * 
 * Contributions:
 *   Copyright (C) 2004-2005, Suzan Foster <su@islief.nl>
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


/*
 * Raptor 'RSS 1.0' serializer object
 */
typedef struct {
  /* static rss model */
  raptor_rss_model model;

  /* Triples with no assigned type node */
  raptor_sequence *triples;

  /* URIs of rdf:Seq items rdf:_<n> at offset n */
  raptor_sequence *items;

  /* URIs of raptor_rss_item* (?x rdf:type rss:Enclosure) */
  raptor_sequence *enclosures;

  /* URI of rdf:Seq node */
  raptor_uri *seq_uri;

  /* Namespace stack for serializing */
  raptor_namespace_stack *nstack;

  /* the rdf: namespace - this is destroyed when nstack above is deleted */
  raptor_namespace* rdf_nspace;

  /* the rdf:RDF element */
  raptor_xml_element* rdf_RDF_element;

  /* where the xml is being written */
  raptor_xml_writer *xml_writer;
} raptor_rss10_serializer_context;



/* create a new serializer */
static int
raptor_rss10_serialize_init(raptor_serializer* serializer, const char *name)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;

  raptor_rss_common_init();
  raptor_rss_model_init(&rss_serializer->model);

  rss_serializer->triples=raptor_new_sequence((raptor_sequence_free_handler*)raptor_free_statement, (raptor_sequence_print_handler*)raptor_print_statement);

  rss_serializer->items=raptor_new_sequence((raptor_sequence_free_handler*)raptor_free_rss_item, (raptor_sequence_print_handler*)NULL);

  rss_serializer->enclosures=raptor_new_sequence((raptor_sequence_free_handler*)raptor_free_rss_item, (raptor_sequence_print_handler*)NULL);

  return 0;
}
  

/* destroy a serializer */
static void
raptor_rss10_serialize_terminate(raptor_serializer* serializer)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  int i;
  
  raptor_rss_model_clear(&rss_serializer->model);
  raptor_rss_common_terminate();

  if(rss_serializer->triples)
    raptor_free_sequence(rss_serializer->triples);

  if(rss_serializer->items)
    raptor_free_sequence(rss_serializer->items);

  if(rss_serializer->enclosures)
    raptor_free_sequence(rss_serializer->enclosures);

  if(rss_serializer->seq_uri)
    raptor_free_uri(rss_serializer->seq_uri);

  if(rss_serializer->xml_writer)
    raptor_free_xml_writer(rss_serializer->xml_writer);

  for(i=0; i<RAPTOR_RSS_NAMESPACES_SIZE;i++) {
    if(raptor_rss_namespaces_info[i].nspace)
      raptor_free_namespace(raptor_rss_namespaces_info[i].nspace);
  }
  
  if(rss_serializer->rdf_nspace)
    raptor_free_namespace(rss_serializer->rdf_nspace);

  if(rss_serializer->nstack)
    raptor_free_namespaces(rss_serializer->nstack);

  for(i=0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
    if(raptor_rss_fields_info[i].qname)
      raptor_free_qname(raptor_rss_fields_info[i].qname);
  }

  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    if(raptor_rss_types_info[i].qname)
      raptor_free_qname(raptor_rss_types_info[i].qname);
  }

}
  

static int
raptor_rss10_move_statements(raptor_rss10_serializer_context *rss_serializer,
                             raptor_rss_type type,
                             raptor_rss_item *item)
{
  int t;
  int handled=0;
#ifdef RAPTOR_DEBUG
  int moved_count=0;
#endif
  
  for(t=0; t< raptor_sequence_size(rss_serializer->triples); t++) {
    raptor_statement* s=(raptor_statement*)raptor_sequence_get_at(rss_serializer->triples, t);
    if(!s)
      continue;
    
    if(s->subject_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE &&
       raptor_uri_equals((raptor_uri*)s->subject, item->uri)) {
      /* subject is item URI */
      int f;

      for(f=0; f < RAPTOR_RSS_FIELDS_SIZE; f++) {
        if(!raptor_rss_fields_info[f].uri)
          continue;
        
        if((s->predicate_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE ||
            s->predicate_type == RAPTOR_IDENTIFIER_TYPE_PREDICATE) &&
           raptor_uri_equals((raptor_uri*)s->predicate,
                             raptor_rss_fields_info[f].uri)) {
           raptor_rss_field* field=raptor_rss_new_field();

          /* found field this triple to go in 'item' so move the
           * object value over 
           */
          if(s->object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
            field->uri=(raptor_uri*)s->object;
          else
            field->value=(char*)s->object;
          s->object=NULL;

          RAPTOR_DEBUG1("fa4 - ");
          raptor_rss_item_add_field(item, f, field);
          break;
        }
      }
      
      if(f < RAPTOR_RSS_FIELDS_SIZE) {
        raptor_sequence_set_at(rss_serializer->triples, t, NULL);
#ifdef RAPTOR_DEBUG
        moved_count++;
#endif
        handled=1;
      } else
        RAPTOR_DEBUG4("UNKNOWN property URI <%s> for typed node %i - %s\n",
                      raptor_uri_as_string((raptor_uri*)s->predicate),
                      type, raptor_rss_types_info[type].name);
      
    } /* end if subject matched item URI */
    
  } /* end for all triples */

#ifdef RAPTOR_DEBUG
  if(moved_count > 0)
    RAPTOR_DEBUG5("Moved %d triples to typed node %i - %s with uri <%s>\n",
                  moved_count, type, raptor_rss_types_info[type].name,
                  raptor_uri_as_string((raptor_uri*)item->uri));
#endif

  return handled;
}


static int
raptor_rss10_store_statement(raptor_rss10_serializer_context *rss_serializer,
                             raptor_statement *s)
{
  raptor_rss_model* rss_model=&rss_serializer->model;
  raptor_rss_item *item=NULL;
  int type;
  int handled=0;
  
  for(type=0; type< RAPTOR_RSS_COMMON_SIZE; type++) {
    int found=0;
    for(item=rss_model->common[type]; item; item=item->next) {
      raptor_uri *item_uri=item->uri;
      if(item_uri && raptor_uri_equals((raptor_uri*)s->subject, item_uri)) {
        found=1;
        break;
      }
    }

    if (found) 
      break;	
  }

  if(!item) {
    int i;
    
    for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
      item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
      if(item->uri && raptor_uri_equals((raptor_uri*)s->subject, item->uri))
        break;
    }
    if(i < raptor_sequence_size(rss_serializer->items))
      type=RAPTOR_RSS_ITEM;
    else {
      for(i=0; i < raptor_sequence_size(rss_serializer->enclosures); i++) {
        item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->enclosures, i);
        if(item->uri &&
           raptor_uri_equals((raptor_uri*)s->subject, item->uri))
          break;
        
      }
      if(i < raptor_sequence_size(rss_serializer->enclosures))
        type=RAPTOR_RSS_ENCLOSURE;
      else
        item=NULL;
    }
  }
  

  if(item) {
    int f;

    for(f=0; f < RAPTOR_RSS_FIELDS_SIZE; f++) {
      raptor_rss_field* field;
      if(!raptor_rss_fields_info[f].uri)
        continue;
      if((s->predicate_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE ||
          s->predicate_type == RAPTOR_IDENTIFIER_TYPE_PREDICATE) &&
         raptor_uri_equals((raptor_uri*)s->predicate,
                           raptor_rss_fields_info[f].uri)) {
        /* found field this triple to go in 'item' so move the
         * object value over 
         */
        field=raptor_rss_new_field();
        if(s->object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE) {
          field->uri=(raptor_uri*)s->object;
          s->object=NULL;
        } else {
          field->value=(char*)s->object;
          s->object=NULL;
        }

        RAPTOR_DEBUG1("fa5 - ");
        raptor_rss_item_add_field(item, f, field);
        raptor_free_statement(s);
        RAPTOR_DEBUG3("Stored statement under typed node %i - %s\n",
                      type, raptor_rss_types_info[type].name);
        handled=1;
        break;
      }
    }
  }
  
  if(!handled) {
    raptor_sequence_push(rss_serializer->triples, s);
#ifdef RAPTOR_DEBUG
    fprintf(stderr,"Stored statement: ");
    raptor_print_statement_as_ntriples(s, stderr);
    fprintf(stderr,"\n");
#endif
    handled=1;
  }

  return handled;
}


/* serialize a statement */
static int
raptor_rss10_serialize_statement(raptor_serializer* serializer, 
                                 const raptor_statement *statement)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_rss_model* rss_model=&rss_serializer->model;
  int handled=0;
  
  if(raptor_uri_equals((raptor_uri*)statement->predicate, 
                       RAPTOR_RSS_RDF_type_URI(rss_model))) {

    if(raptor_uri_equals((raptor_uri*)statement->object, 
                         RAPTOR_RSS_RDF_Seq_URI(rss_model))) {

      /* triple (?resource rdf:type rdf:Seq) */
      RAPTOR_DEBUG2("Saw rdf:Seq with URI <%s>\n",
                    raptor_uri_as_string((raptor_uri*)statement->subject));
      if(statement->subject_type==RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
        rss_serializer->seq_uri=raptor_new_uri((unsigned char*)statement->subject);
      else
        rss_serializer->seq_uri=raptor_uri_copy(rss_serializer->seq_uri);
      
      handled=1;
    } else {
      int i;
      raptor_rss_type type=RAPTOR_RSS_NONE;
      
      for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
        raptor_uri *item_uri=raptor_rss_types_info[i].uri;
        if(item_uri &&
           raptor_uri_equals((raptor_uri*)statement->object, item_uri)) {
          type=(raptor_rss_type)i;
          RAPTOR_DEBUG4("Found RSS 1.0 typed node %i - %s with URI <%s>\n", type, raptor_rss_types_info[type].name,
                        raptor_uri_as_string((raptor_uri*)statement->subject));
          break;
        }
      }

      if(type != RAPTOR_RSS_NONE) {
        raptor_rss_item *item=NULL;

        if(type == RAPTOR_RSS_ITEM) {
          for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
            item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
            if(item->uri &&
               raptor_uri_equals((raptor_uri*)statement->subject, item->uri))
              break;
            
          }
          if(i < raptor_sequence_size(rss_serializer->items)) {
            RAPTOR_DEBUG2("Found RSS item at entry %d in sequence of items\n", i);
          } else {
            RAPTOR_DEBUG2("RSS item URI <%s> is not in sequence of items\n",
                          raptor_uri_as_string((raptor_uri*)statement->subject));
            item=NULL;
          }
        } else if(type == RAPTOR_RSS_ENCLOSURE) {
          for(i=0; i < raptor_sequence_size(rss_serializer->enclosures); i++) {
            item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->enclosures, i);
            if(item->uri &&
               raptor_uri_equals((raptor_uri*)statement->subject, item->uri))
              break;
            
          }
          if(i < raptor_sequence_size(rss_serializer->items)) {
            RAPTOR_DEBUG2("Found enclosure at entry %d in sequence of enclosures\n", i);
          } else {
            RAPTOR_DEBUG2("Add new enclosure to sequence with URI <%s>\n",
                          raptor_uri_as_string((raptor_uri*)statement->subject));

            item=(raptor_rss_item*)RAPTOR_CALLOC(raptor_rss_item, 1, sizeof(raptor_rss_item));
            raptor_sequence_push(rss_serializer->enclosures, item);
          }
        } else {
          item=raptor_rss_model_add_common(rss_model, type);
        }

        if(item) {
          raptor_identifier* identifier=&(item->identifier);

          item->uri=raptor_uri_copy((raptor_uri*)statement->subject);
          identifier->uri=raptor_uri_copy(item->uri);
          identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          identifier->uri_source=RAPTOR_URI_SOURCE_URI;

          raptor_rss10_move_statements(rss_serializer, type, item);

          handled=1;
        }
      } else
        RAPTOR_DEBUG2("UNKNOWN RSS 1.0 typed node with type URI <%s>\n",
                      raptor_uri_as_string((raptor_uri*)statement->object));

    }
  }

  if(!handled) {
    raptor_statement *t=raptor_statement_copy(statement);

    /* outside RDF land we don't need to distinguish URIs and blank nodes */
    if(t->subject_type==RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
      unsigned char *blank=(unsigned char*)t->subject;
      t->subject=raptor_new_uri(blank);
      t->subject_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
      RAPTOR_FREE(cstring, blank);
    }
    if(t->object_type==RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
      unsigned char *blank=(unsigned char*)t->object;
      t->object=raptor_new_uri(blank);
      t->object_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
      RAPTOR_FREE(cstring, blank);
    }

    raptor_rss10_store_statement(rss_serializer, t);

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
    raptor_statement* s=(raptor_statement*)raptor_sequence_get_at(rss_serializer->triples, i);
    if(!s)
      continue;

    if(raptor_uri_equals((raptor_uri*)s->subject, rss_serializer->seq_uri) &&
       s->predicate_type == RAPTOR_IDENTIFIER_TYPE_ORDINAL) {
      int* p=(int*)s->predicate;

      RAPTOR_DEBUG3("Found RSS 1.0 item %d with URI <%s>\n", *p,
                    raptor_uri_as_string((raptor_uri*)s->object));

      if(*p > 0) {
        raptor_rss_item* item=(raptor_rss_item*)RAPTOR_CALLOC(raptor_rss_item, 1, sizeof(raptor_rss_item));
        raptor_identifier* identifier=&item->identifier;

        item->uri=(raptor_uri*)s->object;
        s->object=NULL;
        identifier->uri=raptor_uri_copy(item->uri);
        identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
        identifier->uri_source=RAPTOR_URI_SOURCE_URI;

        raptor_sequence_set_at(rss_serializer->items, (*p)-1, item);

        raptor_sequence_set_at(rss_serializer->triples, i, NULL);

        raptor_rss10_move_statements(rss_serializer, RAPTOR_RSS_ITEM, item);
      }
    }
  }

  rss_model->items_count=raptor_sequence_size(rss_serializer->items);
}


static void
raptor_rss10_build_xml_names(raptor_serializer *serializer)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_rss_model* rss_model=&rss_serializer->model;
  raptor_uri *base_uri=serializer->base_uri;
  raptor_xml_element *element;
  raptor_qname *qname;
  int i;

  rss_serializer->rdf_nspace=raptor_new_namespace(rss_serializer->nstack,
                                                  (const unsigned char*)"rdf",
                                                  (const unsigned char*)raptor_rdf_namespace_uri,
                                                  0);
  qname=raptor_new_qname_from_namespace_local_name(rss_serializer->rdf_nspace, (const unsigned char*)"RDF",  NULL);
  if(base_uri)
    base_uri=raptor_uri_copy(base_uri);
  element=raptor_new_xml_element(qname, NULL, base_uri);
  rss_serializer->rdf_RDF_element=element;

  raptor_xml_element_declare_namespace(element, rss_serializer->rdf_nspace);

  /* Now we have a namespace stack, declare the namespaces */
  for(i=0; i<RAPTOR_RSS_NAMESPACES_SIZE;i++) {
    raptor_uri* uri=raptor_rss_namespaces_info[i].uri;
    const unsigned char *prefix=(const unsigned char*)raptor_rss_namespaces_info[i].prefix;
    if((prefix && uri) || i == RSS1_0_NS) {
      raptor_namespace* nspace=raptor_new_namespace(rss_serializer->nstack, prefix, raptor_uri_as_string(uri), 0);
      raptor_rss_namespaces_info[i].nspace=nspace;
      raptor_xml_element_declare_namespace(element, nspace);
    }
  }

  for(i=0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
    int n=raptor_rss_fields_info[i].nspace;
    raptor_namespace* nspace=raptor_rss_namespaces_info[n].nspace;
    raptor_rss_fields_info[i].qname=raptor_new_qname_from_namespace_local_name(nspace,
                                                                               (const unsigned char*)raptor_rss_fields_info[i].name, NULL);
  }

  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    int n=raptor_rss_types_info[i].nspace;
    raptor_namespace* nspace=raptor_rss_namespaces_info[n].nspace;
    if(nspace)
      raptor_rss_types_info[i].qname=raptor_new_qname_from_namespace_local_name(nspace, (const unsigned char*)raptor_rss_types_info[i].name, NULL);
  }
  
  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    raptor_rss_item* item;
    for (item=rss_model->common[i]; item; item=item->next) {
      if(!item->fields_count)
        continue;
      item->node_type=&raptor_rss_types_info[i];
    }
  }

  for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
    raptor_rss_item* item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
    item->node_type=&raptor_rss_types_info[RAPTOR_RSS_ITEM];
  }

  for(i=0; i < raptor_sequence_size(rss_serializer->enclosures); i++) {
    raptor_rss_item* item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->enclosures, i);
    item->node_type=&raptor_rss_types_info[RAPTOR_RSS_ENCLOSURE];
  }

}


static const unsigned char *raptor_rss10_spaces=(const unsigned char*)"          ";

static void
raptor_rss10_emit_item(raptor_serializer* serializer,
                       raptor_rss_item *item,
                       int item_type,
                       int indent) 
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_xml_writer* xml_writer=rss_serializer->xml_writer;
  raptor_uri *base_uri=serializer->base_uri;
  raptor_xml_element *element;
  raptor_qname **attrs=NULL;
  raptor_uri* base_uri_copy=NULL;
  int f;

  if (!item->fields_count) {
    int i;
    for(i=0; i < raptor_sequence_size(rss_serializer->enclosures); i++) {
      raptor_rss_item *enclosure_item;
      enclosure_item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->enclosures, i);
      /* If the item and enclosure item have the same URI, move the
       * enclosure fields to the item.  Assumed that they got conflated
       * previously such as when the enclosure url = the guid
       */
      if(enclosure_item->uri &&
         raptor_uri_equals(item->uri, enclosure_item->uri)) {
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

  base_uri_copy=base_uri ? raptor_uri_copy(base_uri) : NULL;
  element=raptor_new_xml_element(raptor_qname_copy(item->node_type->qname), NULL, base_uri_copy);
  if(item->uri) {
    attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname*));
    attrs[0]=raptor_new_qname_from_namespace_local_name(rss_serializer->rdf_nspace, (const unsigned char*)"about",  raptor_uri_as_string(item->uri));
    raptor_xml_element_set_attributes(element, attrs, 1);
  }

  raptor_xml_writer_raw_counted(xml_writer, raptor_rss10_spaces, indent);
  raptor_xml_writer_start_element(xml_writer, element);
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);


  for(f=0; f < RAPTOR_RSS_FIELDS_SIZE; f++) {
    raptor_rss_field* field;

    if(f == RAPTOR_RSS_FIELD_ITEMS)
      /* Done after loop */
      continue;

    if(f == RAPTOR_RSS_FIELD_ATOM_AUTHOR)
      continue;
    
    if(!raptor_rss_fields_info[f].uri)
      continue;
    
    for (field=item->fields[f]; field; field=field->next) {
      raptor_xml_element* predicate;

      base_uri_copy=base_uri ? raptor_uri_copy(base_uri) : NULL;
      predicate=raptor_new_xml_element(raptor_qname_copy(raptor_rss_fields_info[f].qname), NULL, base_uri_copy);    
      raptor_xml_writer_raw_counted(xml_writer, raptor_rss10_spaces, indent+2);
      if (field->uri) {
        raptor_uri* enclosure_uri=field->uri;
        raptor_rss_item *enclosure_item=NULL;
        int i;
        if (f == RAPTOR_RSS_RDF_ENCLOSURE && item_type == RAPTOR_RSS_ITEM) {
          for(i=0; i < raptor_sequence_size(rss_serializer->enclosures); i++) {
            enclosure_item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->enclosures, i);
            if(enclosure_item->uri && raptor_uri_equals(enclosure_uri, enclosure_item->uri))
              break;
          }
          if (enclosure_item) {
            int attr_count=0;

            attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 3, sizeof(raptor_qname*));
            attrs[attr_count]=raptor_new_qname_from_namespace_local_name(rss_serializer->rdf_nspace, (const unsigned char*)"resource",  raptor_uri_as_string(field->uri));
            attr_count++;
            if (enclosure_item->fields[RAPTOR_RSS_RDF_ENCLOSURE_TYPE] && enclosure_item->fields[RAPTOR_RSS_RDF_ENCLOSURE_TYPE]->value) {
              attrs[attr_count]=raptor_new_qname_from_namespace_local_name(raptor_rss_namespaces_info[RSS2_0_ENC_NS].nspace, (const unsigned char*)raptor_rss_fields_info[RAPTOR_RSS_RDF_ENCLOSURE_TYPE].name, (const unsigned char*)enclosure_item->fields[RAPTOR_RSS_RDF_ENCLOSURE_TYPE]->value);
              attr_count++;
            }
            if (enclosure_item->fields[RAPTOR_RSS_RDF_ENCLOSURE_LENGTH] && enclosure_item->fields[RAPTOR_RSS_RDF_ENCLOSURE_LENGTH]->value) {
              attrs[attr_count]=raptor_new_qname_from_namespace_local_name(raptor_rss_namespaces_info[RSS2_0_ENC_NS].nspace, (const unsigned char*)raptor_rss_fields_info[RAPTOR_RSS_RDF_ENCLOSURE_LENGTH].name, (const unsigned char*)enclosure_item->fields[RAPTOR_RSS_RDF_ENCLOSURE_LENGTH]->value);
              attr_count++;
            }
            raptor_xml_element_set_attributes(predicate, attrs, attr_count);
          } else {
            RAPTOR_DEBUG2("Enclosure item with URI %s could not be found in list of enclosures\n", raptor_uri_as_string(enclosure_uri));
          }
        } else {
          attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname*));
          attrs[0]=raptor_new_qname_from_namespace_local_name(rss_serializer->rdf_nspace, (const unsigned char*)"resource",  raptor_uri_as_string(field->uri));
          raptor_xml_element_set_attributes(predicate, attrs, 1);
        }
        raptor_xml_writer_empty_element(xml_writer, predicate);		
      } else {
        /* not a URI, must be a literal */
        raptor_xml_writer_start_element(xml_writer, predicate);
        if(f == RAPTOR_RSS_FIELD_CONTENT_ENCODED) {
          raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"<![CDATA[", 9);
          raptor_xml_writer_raw(xml_writer, (const unsigned char*)field->value);
          raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"]]>", 3);
        } else
          raptor_xml_writer_cdata(xml_writer, (const unsigned char*)field->value);
        raptor_xml_writer_end_element(xml_writer, predicate);
      }
      raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
      raptor_free_xml_element(predicate);
    }
  }

  if(item_type == RAPTOR_RSS_CHANNEL && item->fields[RAPTOR_RSS_FIELD_ITEMS]) {
    raptor_xml_element* rss_items_predicate;
    int i;
    raptor_qname *rdf_Seq_qname;
    raptor_xml_element *rdf_Seq_element;
    
    rdf_Seq_qname=raptor_new_qname_from_namespace_local_name(rss_serializer->rdf_nspace, (const unsigned char*)"Seq",  NULL);

    base_uri_copy=base_uri ? raptor_uri_copy(base_uri) : NULL;
    rdf_Seq_element=raptor_new_xml_element(rdf_Seq_qname, NULL, base_uri_copy);

    /* make the <rss:items><rdf:Seq><rdf:li /> .... </rdf:Seq></rss:items> */

    base_uri_copy=base_uri ? raptor_uri_copy(base_uri) : NULL;
    rss_items_predicate=raptor_new_xml_element(raptor_qname_copy(raptor_rss_fields_info[RAPTOR_RSS_FIELD_ITEMS].qname), NULL, base_uri_copy);

    raptor_xml_writer_raw_counted(xml_writer, raptor_rss10_spaces, indent+2);
    raptor_xml_writer_start_element(xml_writer, rss_items_predicate);
    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);

    raptor_xml_writer_raw_counted(xml_writer, raptor_rss10_spaces, indent+4);
    raptor_xml_writer_start_element(xml_writer, rdf_Seq_element);
    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
    
    for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
      raptor_rss_item* item_item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
      raptor_qname *rdf_li_qname;
      raptor_xml_element *rdf_li_element;
      
      rdf_li_qname=raptor_new_qname_from_namespace_local_name(rss_serializer->rdf_nspace, (const unsigned char*)"li",  NULL);
      base_uri_copy=base_uri ? raptor_uri_copy(base_uri) : NULL;
      rdf_li_element=raptor_new_xml_element(rdf_li_qname, NULL, base_uri_copy);
      attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname*));
      attrs[0]=raptor_new_qname_from_namespace_local_name(rss_serializer->rdf_nspace, (const unsigned char*)"resource",  raptor_uri_as_string(item_item->uri));
      raptor_xml_element_set_attributes(rdf_li_element, attrs, 1);
      
      raptor_xml_writer_raw_counted(xml_writer, raptor_rss10_spaces, indent+6);
      raptor_xml_writer_empty_element(xml_writer, rdf_li_element);
      raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
      
      raptor_free_xml_element(rdf_li_element);
    }
    
    raptor_xml_writer_raw_counted(xml_writer, raptor_rss10_spaces, indent+4);
    raptor_xml_writer_end_element(xml_writer, rdf_Seq_element);
    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
    
    raptor_free_xml_element(rdf_Seq_element);

    raptor_xml_writer_raw_counted(xml_writer, raptor_rss10_spaces, indent+2);
    raptor_xml_writer_end_element(xml_writer, rss_items_predicate);

    raptor_free_xml_element(rss_items_predicate);

    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
  }

  raptor_xml_writer_raw_counted(xml_writer, raptor_rss10_spaces, indent);
  raptor_xml_writer_end_element(xml_writer, element);
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);

  raptor_free_xml_element(element);
}


static int
raptor_rss10_serialize_end(raptor_serializer* serializer) {
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_rss_model* rss_model=&rss_serializer->model;
  int i;
  raptor_xml_writer* xml_writer;
  raptor_uri_handler *uri_handler;
  void *uri_context;
#ifdef RAPTOR_DEBUG
  int triple_count=0;
#endif

  raptor_rss10_build_items(rss_serializer);

#ifdef RAPTOR_DEBUG
  for(i=0; i < raptor_sequence_size(rss_serializer->triples); i++) {
    raptor_statement* t=(raptor_statement*)raptor_sequence_get_at(rss_serializer->triples, i);
    if(t) {
      fprintf(stderr, " %d: ", i);
      raptor_print_statement(t, stderr);
      fputc('\n', stderr);
      triple_count++;
    }
  }
  RAPTOR_DEBUG2("Starting with %d stored triples\n", triple_count);
#endif

  raptor_uri_get_handler(&uri_handler, &uri_context);

  rss_serializer->nstack=raptor_new_namespaces(uri_handler, uri_context,
                                               NULL, NULL, /* errors */
                                               1);

  xml_writer=raptor_new_xml_writer(rss_serializer->nstack,
                                   uri_handler, uri_context,

                                   serializer->iostream,
                                   NULL, NULL, /* errors */
                                   1);
  rss_serializer->xml_writer=xml_writer;

  raptor_xml_writer_raw(xml_writer,
                        (const unsigned char*)"<?xml version=\"1.0\" encoding=\"utf-8\"?>\n");

  raptor_rss10_build_xml_names(serializer);

  raptor_xml_writer_start_element(xml_writer, rss_serializer->rdf_RDF_element);

  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);


  i=RAPTOR_RSS_CHANNEL;
  RAPTOR_DEBUG3("Emitting type %i - %s\n", i, raptor_rss_types_info[i].name);
  raptor_rss10_emit_item(serializer, rss_model->common[i], i, 2);
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);


  if(rss_model->items_count) {
    for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
      raptor_rss_item* item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
      raptor_rss10_emit_item(serializer, item, RAPTOR_RSS_ITEM, 2);
      raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
    }

  }

  for(i=RAPTOR_RSS_CHANNEL+1; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    if(i == RAPTOR_ATOM_AUTHOR)
      continue;

    raptor_rss_item* item;
    for (item=rss_model->common[i]; item; item=item->next) {
      RAPTOR_DEBUG3("Emitting type %i - %s\n", i, raptor_rss_types_info[i].name);
      raptor_rss10_emit_item(serializer, item, i, 2);
    }
  }

  raptor_xml_writer_end_element(xml_writer, rss_serializer->rdf_RDF_element);
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);

  raptor_free_xml_element(rss_serializer->rdf_RDF_element);

  return 0;
}


  
/* finish the serializer factory */
static void
raptor_rss10_serialize_finish_factory(raptor_serializer_factory* factory)
{

}


static void
raptor_rss10_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->context_length     = sizeof(raptor_rss10_serializer_context);
  
  factory->init                = raptor_rss10_serialize_init;
  factory->terminate           = raptor_rss10_serialize_terminate;
  factory->declare_namespace   = NULL;
  factory->serialize_start     = NULL;
  factory->serialize_statement = raptor_rss10_serialize_statement;
  factory->serialize_end       = raptor_rss10_serialize_end;
  factory->finish_factory      = raptor_rss10_serialize_finish_factory;
}



void
raptor_init_serializer_rss10(void) {
  raptor_serializer_register_factory("rss-1.0",  "RSS 1.0",
                                     NULL, 
                                     NULL,
                                     (const unsigned char*)"http://purl.org/rss/1.0/spec",
                                     &raptor_rss10_serializer_register_factory);
}

