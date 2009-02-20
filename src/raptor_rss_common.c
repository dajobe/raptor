/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_rss_common.c - Raptor RSS common code
 *
 * Copyright (C) 2003-2008, David Beckett http://www.dajobe.org/
 * Copyright (C) 2003-2005, University of Bristol, UK http://www.bristol.ac.uk/
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


/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"
#include "raptor_rss.h"


const raptor_rss_namespace_info raptor_rss_namespaces_info[RAPTOR_RSS_NAMESPACES_SIZE]={
  { NULL,                     NULL,      },
  { NULL,                     NULL,      },
  { RSS0_91_NAMESPACE_URI,    "rss091",  },
  { RSS0_9_NAMESPACE_URI,     NULL,      },
  { RSS1_0_NAMESPACE_URI,     "rss",     },
  { ATOM0_3_NAMESPACE_URI,    NULL,      },
  { DC_NAMESPACE_URI,         "dc",      },
  { RSS2_0_ENC_NAMESPACE_URI, "enc",     },
  { RSS1_1_NAMESPACE_URI,     NULL,      },
  { CONTENT_NAMESPACE_URI,    "content", },
  { ATOM1_0_NAMESPACE_URI,    "atom",    },
  { RDF_NAMESPACE_URI,        "rdf",     },
  { ATOMTRIPLES_NAMESPACE_URI, "at",     },
};


const raptor_rss_info raptor_rss_types_info[RAPTOR_RSS_COMMON_SIZE+1]={
  { "channel",    RSS1_0_NS, 0 },
  { "image",      RSS1_0_NS, 0 },
  { "textinput",  RSS1_0_NS, 0 },
  { "item",       RSS1_0_NS, 0 },
  { "author",     ATOM1_0_NS, 0 },
  { "skipHours",  RSS0_91_NS, 0 },
  { "skipDays",   RSS0_91_NS, 0 },
  { "Enclosure",  RSS2_0_ENC_NS, 0 }, /* Enclosure class in RDF output */
  { "category",   ATOM1_0_NS, 0 },
  { "category",   RSS2_0_NS, 0 },
  { "source"  ,   RSS2_0_NS, 0 },
  { "feed",       ATOM1_0_NS, 0 },
  { "entry",      ATOM1_0_NS, 0 },
  { "<unknown>",  RSS_UNKNOWN_NS, 0 },
  { "<none>",  RSS_UNKNOWN_NS, 0 }
};


const raptor_rss_info raptor_rss_fields_info[RAPTOR_RSS_FIELDS_SIZE+2]={
  { "title",          RSS1_0_NS, 0 },
  { "link",           RSS1_0_NS, 0 },
  { "description",    RSS1_0_NS, 0 },
  { "url",            RSS1_0_NS, 0 },
  { "name",           RSS1_0_NS, 0 },
  { "language",       RSS0_91_NS, 0 },
  { "rating",         RSS0_91_NS, 0 },
  { "copyright",      RSS0_91_NS, 0 },
  { "pubDate",        RSS0_91_NS, 0 },
  { "lastBuildDate",  RSS0_91_NS, 0 },
  { "docs",           RSS0_91_NS, 0 },
  { "managingEditor", RSS0_91_NS, 0 },
  { "webMaster",      RSS0_91_NS, 0 },
  { "cloud",          RSS0_92_NS, 0 },
  { "ttl",            RSS2_0_NS, 0 },
  { "width",          RSS0_91_NS, 0 },
  { "height",         RSS0_91_NS, 0 },
  { "hour",           RSS0_91_NS, 0 },
  { "day",            RSS0_91_NS, 0 },
  { "generator",      RSS0_92_NS, 0 },
  { "source",         RSS0_92_NS, 0 },
  { "author",         RSS2_0_NS, 0 },
  { "guid",           RSS2_0_NS, 0 },
  { "enclosure",      RSS2_0_NS, RAPTOR_RSS_INFO_FLAG_BLOCK_VALUE, RAPTOR_RSS_ENCLOSURE },     /* enclosure in RSS */
  { "enclosure",      RSS2_0_ENC_NS, 0 }, /* In RDF output, not an RSS field */
  { "url",            RSS2_0_ENC_NS, 0 }, /* In RDF output, not an RSS field */
  { "length",         RSS2_0_ENC_NS, 0 }, /* In RDF output, not an RSS field */
  { "type",           RSS2_0_ENC_NS, 0 }, /* In RDF output, not an RSS field */
  { "length",         RSS2_0_NS, 0 },
  { "type",           RSS2_0_NS, 0 },
  { "category",       RSS0_92_NS, RAPTOR_RSS_INFO_FLAG_BLOCK_VALUE, RAPTOR_RSS_CATEGORY },
  { "comments",       RSS0_92_NS, 0 },
  { "items",          RSS1_0_NS, 0 },
  { "image",          RSS1_0_NS, 0 },
  { "textinput",      RSS1_0_NS, 0 },

  { "copyright",      ATOM0_3_NS, 0 },
  { "created",        ATOM0_3_NS, 0 },
  { "issued",         ATOM0_3_NS, 0 },
  { "modified",       ATOM0_3_NS, 0 },
  { "tagline",        ATOM0_3_NS, 0 },

  /* atom 1.0 required fields */
  { "id",             ATOM1_0_NS, RAPTOR_RSS_INFO_FLAG_URI_VALUE },
  { "title",          ATOM1_0_NS, 0 },
  { "updated",        ATOM1_0_NS, 0 },
  /* atom 1.0 optional fields */
  { "author",         ATOM1_0_NS, 0 },
  { "category",       ATOM1_0_NS, RAPTOR_RSS_INFO_FLAG_BLOCK_VALUE, RAPTOR_ATOM_CATEGORY },
  { "content",        ATOM1_0_NS, 0 },
  { "contributor",    ATOM1_0_NS, 0 },
  { "email",          ATOM1_0_NS, 0 },
  { "entry",          ATOM1_0_NS, 0 },
  { "feed",           ATOM1_0_NS, 0 },
  { "generator",      ATOM1_0_NS, 0 },
  { "icon",           ATOM1_0_NS, RAPTOR_RSS_INFO_FLAG_URI_VALUE },
  { "link",           ATOM1_0_NS, 0 },
  { "logo",           ATOM1_0_NS, RAPTOR_RSS_INFO_FLAG_URI_VALUE },
  { "name",           ATOM1_0_NS, 0 },
  { "published",      ATOM1_0_NS, 0 },
  { "rights",         ATOM1_0_NS, 0 },
  { "source",         ATOM1_0_NS, RAPTOR_RSS_INFO_FLAG_BLOCK_VALUE, RAPTOR_RSS_SOURCE },
  { "subtitle",       ATOM1_0_NS, 0 },
  { "summary",        ATOM1_0_NS, 0 },
  { "uri",            ATOM1_0_NS, 0 },

  { "title",          DC_NS, 0 },
  { "contributor",    DC_NS, 0 },
  { "creator",        DC_NS, 0 },
  { "publisher",      DC_NS, 0 },
  { "subject",        DC_NS, 0 },
  { "description",    DC_NS, 0 },
  { "date",           DC_NS, 0 },
  { "type",           DC_NS, 0 },
  { "format",         DC_NS, 0 },
  { "identifier",     DC_NS, 0 },
  { "language",       DC_NS, 0 },
  { "relation",       DC_NS, 0 },
  { "source",         DC_NS, 0 },
  { "coverage",       DC_NS, 0 },
  { "rights",         DC_NS, 0 },

  { "encoded",        CONTENT_NS, 0 },

  { "contentType",    ATOMTRIPLES_NS, 0 },

  { "<unknown>",      RSS_UNKNOWN_NS, 0 },
  { "<none>",         RSS_UNKNOWN_NS, 0 }
};


/* Crude and unofficial mappings from atom fields to RSS */
const raptor_field_pair raptor_atom_to_rss[]={
  /* atom clone of rss fields */
  { RAPTOR_RSS_FIELD_ATOM_SUMMARY,   RAPTOR_RSS_FIELD_DESCRIPTION },
  { RAPTOR_RSS_FIELD_ATOM_ID,        RAPTOR_RSS_FIELD_LINK },
  { RAPTOR_RSS_FIELD_ATOM_UPDATED,   RAPTOR_RSS_FIELD_DC_DATE },
  { RAPTOR_RSS_FIELD_ATOM_RIGHTS,    RAPTOR_RSS_FIELD_DC_RIGHTS },
  { RAPTOR_RSS_FIELD_ATOM_TITLE,     RAPTOR_RSS_FIELD_TITLE },
  { RAPTOR_RSS_FIELD_ATOM_SUMMARY,   RAPTOR_RSS_FIELD_CONTENT_ENCODED },

  /* atom 0.3 to atom 1.0 */
  { RAPTOR_RSS_FIELD_ATOM_COPYRIGHT, RAPTOR_RSS_FIELD_ATOM_RIGHTS },
  { RAPTOR_RSS_FIELD_ATOM_TAGLINE,   RAPTOR_RSS_FIELD_ATOM_SUBTITLE },
#if 0
  /* other old atom 0.3 fields */
  { RAPTOR_RSS_FIELD_ATOM_CREATED,  RAPTOR_RSS_FIELD_UNKNOWN },
  { RAPTOR_RSS_FIELD_ATOM_ISSUED,   RAPTOR_RSS_FIELD_UNKNOWN },
  { RAPTOR_RSS_FIELD_ATOM_MODIFIED, RAPTOR_RSS_FIELD_UNKNOWN },
#endif

  { RAPTOR_RSS_FIELD_UNKNOWN,       RAPTOR_RSS_FIELD_UNKNOWN }
};


const raptor_rss_block_info raptor_rss_blocks_info[RAPTOR_RSS_BLOCKS_SIZE+1] = {
  /*
  RSS 2 <enclosure> - optional element inside <item>
  attributes:
    url (required):    where the enclosure is located.  url
    length (required): how big enclosure it is in bytes. integer
    type (required):   what enclosure type is as a standard MIME type. string
  content: empty
  */
  { RAPTOR_RSS_ENCLOSURE, "url",    RSS_BLOCK_FIELD_TYPE_URL,    0 },
  { RAPTOR_RSS_ENCLOSURE, "length", RSS_BLOCK_FIELD_TYPE_STRING, 0 },
  { RAPTOR_RSS_ENCLOSURE, "type",   RSS_BLOCK_FIELD_TYPE_STRING, 1 },

  /*
  RSS 2 <category> - optional element inside <item>
  attributes:
    domain (optional): the domain. url
  content: category. string
  */
  { RAPTOR_RSS_CATEGORY, "domain",    RSS_BLOCK_FIELD_TYPE_URL,    0 },

  /*
  RSS 2 <source> - optional element inside <item>
  attributes:
    url (required): location of source. url
  content: source name. string
  */
  { RAPTOR_RSS_SOURCE, "url",    RSS_BLOCK_FIELD_TYPE_URL,    0 },

  /*
  Atom <category> - optional element inside <entry>
  attributes:
    term (required):   the category. string
    scheme (optional): categorization scheme. url
    label (optional):  human-readable label. string
  content: empty
  */
  { RAPTOR_ATOM_CATEGORY, "term",   RSS_BLOCK_FIELD_TYPE_STRING, 0 },
  { RAPTOR_ATOM_CATEGORY, "scheme", RSS_BLOCK_FIELD_TYPE_URL, 0 },
  { RAPTOR_ATOM_CATEGORY, "label",  RSS_BLOCK_FIELD_TYPE_STRING, 1 },
};


const unsigned char * const raptor_atom_namespace_uri=(const unsigned char *)"http://www.w3.org/2005/Atom";



int
raptor_rss_common_init(raptor_world* world) {
  int i;
  raptor_uri *namespace_uri;

  if(world->rss_common_initialised++)
    return 0;

  world->rss_namespaces_info_uris=(raptor_uri**)RAPTOR_CALLOC(raptor_uri* array, RAPTOR_RSS_NAMESPACES_SIZE, sizeof(raptor_uri*));
  if(!world->rss_namespaces_info_uris)
    return -1;
  for(i=0; i<RAPTOR_RSS_NAMESPACES_SIZE;i++) {
    const char *uri_string=raptor_rss_namespaces_info[i].uri_string;
    if(uri_string) {
      world->rss_namespaces_info_uris[i]=raptor_new_uri_v2(world, (const unsigned char*)uri_string);
      if(!world->rss_namespaces_info_uris[i])
        return -1;
    }
  }

  world->rss_types_info_uris=(raptor_uri**)RAPTOR_CALLOC(raptor_uri* array, RAPTOR_RSS_COMMON_SIZE, sizeof(raptor_uri*));
  if(!world->rss_types_info_uris)
    return -1;
  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    int n=raptor_rss_types_info[i].nspace;
    namespace_uri=world->rss_namespaces_info_uris[n];
    if(namespace_uri) {
      world->rss_types_info_uris[i]=raptor_new_uri_from_uri_local_name_v2(world, namespace_uri, (const unsigned char*)raptor_rss_types_info[i].name);
      if(!world->rss_types_info_uris[i])
        return -1;
    }
  }

  world->rss_fields_info_uris=(raptor_uri**)RAPTOR_CALLOC(raptor_uri* array, RAPTOR_RSS_FIELDS_SIZE, sizeof(raptor_uri*));
  if(!world->rss_fields_info_uris)
    return -1;
  for(i=0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
    namespace_uri=world->rss_namespaces_info_uris[raptor_rss_fields_info[i].nspace];
    if(namespace_uri) {
      world->rss_fields_info_uris[i]=raptor_new_uri_from_uri_local_name_v2(world, namespace_uri,
                                                                           (const unsigned char*)raptor_rss_fields_info[i].name);
      if(!world->rss_fields_info_uris[i])
        return -1;
    }
  }

  return 0;
}


void
raptor_rss_common_terminate(raptor_world* world) {
  int i;
  if(--world->rss_common_initialised)
    return;

  if(world->rss_types_info_uris) {
    for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
      if(world->rss_types_info_uris[i])
        raptor_free_uri_v2(world, world->rss_types_info_uris[i]);
    }
    RAPTOR_FREE(raptor_uri* array, world->rss_types_info_uris);
    world->rss_types_info_uris=NULL;
  }

  if(world->rss_fields_info_uris) {
    for(i=0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
      if(world->rss_fields_info_uris[i])
        raptor_free_uri_v2(world, world->rss_fields_info_uris[i]);
    }
    RAPTOR_FREE(raptor_uri* array, world->rss_fields_info_uris);
    world->rss_fields_info_uris=NULL;
  }

  if(world->rss_namespaces_info_uris) {
    for(i=0; i<RAPTOR_RSS_NAMESPACES_SIZE;i++) {
      if(world->rss_namespaces_info_uris[i])
        raptor_free_uri_v2(world, world->rss_namespaces_info_uris[i]);
    }
    RAPTOR_FREE(raptor_uri* array, world->rss_namespaces_info_uris);
    world->rss_namespaces_info_uris=NULL;
  }
}


void
raptor_rss_model_init(raptor_world* world, raptor_rss_model* rss_model)
{
  memset(rss_model->common, 0, sizeof(void*)*RAPTOR_RSS_COMMON_SIZE);

  rss_model->world=world;

  rss_model->last=rss_model->items=NULL;
  rss_model->items_count=0;

  RAPTOR_RSS_RDF_type_URI(rss_model)=raptor_new_uri_for_rdf_concept_v2(world, "type");
  RAPTOR_RSS_RDF_Seq_URI(rss_model)=raptor_new_uri_for_rdf_concept_v2(world, "Seq");
  RAPTOR_RSS_RSS_items_URI(rss_model)=raptor_new_uri_relative_to_base_v2(world, world->rss_namespaces_info_uris[RSS1_0_NS], (const unsigned char*)"items");
}
  

void
raptor_rss_model_clear(raptor_rss_model* rss_model)
{
  int i;
  raptor_rss_item* item;
  
  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    item=rss_model->common[i];
    while(item) {
      raptor_rss_item *next=item->next;
      raptor_free_rss_item(item);
      item=next;
    }
  }

  item=rss_model->items;
  while(item) {
    raptor_rss_item *next=item->next;

    raptor_free_rss_item(item);
    item=next;
  }
  rss_model->last=rss_model->items=NULL;

  for(i=0; i< RAPTOR_RSS_N_CONCEPTS; i++) {
    raptor_uri* concept_uri=rss_model->concepts[i];
    if(concept_uri) {
      raptor_free_uri_v2(rss_model->world, concept_uri);
      rss_model->concepts[i]=NULL;
    }
  }
}


raptor_rss_item*
raptor_new_rss_item(raptor_world* world)
{
  raptor_rss_item* item;

  item=(raptor_rss_item*)RAPTOR_CALLOC(raptor_rss_item, 1, 
                                       sizeof(raptor_rss_item));
  if(!item)
    return NULL;
  
  item->world=world;
  item->identifier.world=world;
  item->triples=raptor_new_sequence((raptor_sequence_free_handler*)raptor_free_statement_v2, (raptor_sequence_print_handler*)raptor_print_statement_v2);
  if(!item->triples) {
    RAPTOR_FREE(raptor_rss_item, item);
    return NULL;
  }
  return item;
}


int
raptor_rss_model_add_item(raptor_rss_model* rss_model)
{
  raptor_rss_item* item;

  item=raptor_new_rss_item(rss_model->world);
  if(!item)
    return 1;

  /* new list */
  if(!rss_model->items)
    rss_model->items=item;
  
  /* join last item to this one */
  if(rss_model->last)
    rss_model->last->next=item;
  
  /* this is now the last item */
  rss_model->last=item;
  rss_model->items_count++;

  RAPTOR_DEBUG2("Added item %d\n", rss_model->items_count);

  return 0;
}


raptor_rss_item*
raptor_rss_model_add_common(raptor_rss_model* rss_model,
                            raptor_rss_type type)
{
  raptor_rss_item* item;

  item=raptor_new_rss_item(rss_model->world);
  if(!item)
    return NULL;

  if(rss_model->common[type]==NULL) {
    RAPTOR_DEBUG3("Adding common type %d - %s\n", type,
                  raptor_rss_types_info[type].name);
    rss_model->common[type]=item; 
  } else {
    raptor_rss_item* next;
    RAPTOR_DEBUG3("Appending common type %d - %s\n", type, 
                  raptor_rss_types_info[type].name);
    for (next=rss_model->common[type]; next->next; next=next->next)
      ;
    next->next=item;
  }
  return item;
}


raptor_rss_item*
raptor_rss_model_get_common(raptor_rss_model* rss_model, raptor_rss_type type)
{
  raptor_rss_item* item;
  for(item=rss_model->common[type]; 
      item && item->next;
      item=item->next) ;
  return item;
}


void
raptor_free_rss_item(raptor_rss_item* item)
{
  int i;
  for(i=0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
    if(item->fields[i])
      raptor_rss_field_free(item->fields[i]);
  }
  if(item->blocks) 
    raptor_free_rss_block(item->blocks);
  if(item->uri)
    raptor_free_uri_v2(item->world, item->uri);
  raptor_free_identifier(&item->identifier);
  if(item->triples)
    raptor_free_sequence(item->triples);

  RAPTOR_FREE(raptor_rss_item, item);
}


void
raptor_rss_item_add_block(raptor_rss_item* item, 
                          raptor_rss_block *block)
{
  if(!item->blocks) {
    RAPTOR_DEBUG1("Adding first block\n");
    item->blocks = block;
  } else {
    raptor_rss_block *cur;

    RAPTOR_DEBUG1("Adding subsequent block\n");
    for(cur = item->blocks; cur->next; cur = cur->next)
      ;
    cur->next = block;
  }
}


void
raptor_rss_item_add_field(raptor_rss_item* item, int type, 
                          raptor_rss_field* field)
{
  if(!item->fields[type]) {
    RAPTOR_DEBUG3("Adding first type %d field %s\n", type, raptor_rss_fields_info[type].name);
    item->fields_count++;	
    item->fields[type]=field;
  } else { 
    raptor_rss_field* cur;

    RAPTOR_DEBUG1("Adding subsequent field\n");
    for(cur=item->fields[type]; cur->next; cur=cur->next) ;
    cur->next=field;
  }
}


raptor_rss_block*
raptor_new_rss_block(raptor_world* world, raptor_rss_type type)
{
  raptor_rss_block *block;
  block = (raptor_rss_block*)RAPTOR_CALLOC(raptor_rss_block, 1,
                                           sizeof(raptor_rss_block));
  /* init world field in identifier not created with raptor_new_identifier() */
  if(block) {
    block->identifier.world = world;
    block->rss_type = type;
    block->node_type = world->rss_types_info_uris[type];
  }
  
  return block;
}


void
raptor_free_rss_block(raptor_rss_block *block)
{
  int i;

  for(i = 0; i < RSS_BLOCK_MAX_URLS; i++) {
    if(block->urls[i])
      raptor_free_uri_v2(block->identifier.world, block->urls[i]);
  }

  for(i = 0; i < RSS_BLOCK_MAX_STRINGS; i++) {
    if(block->strings[i])
      RAPTOR_FREE(cstring, block->strings[i]);
  }

  if(block->next)
    raptor_free_rss_block(block->next);

  raptor_free_identifier(&(block->identifier));

  RAPTOR_FREE(raptor_rss_block, block);
}


raptor_rss_field*
raptor_rss_new_field(raptor_world* world)
{
  raptor_rss_field* field=(raptor_rss_field*)RAPTOR_CALLOC(raptor_rss_field, 1, sizeof(raptor_rss_field));
  if(field)
    field->world=world;
  return field;
}


void
raptor_rss_field_free(raptor_rss_field* field)
{
  if(field->value)
    RAPTOR_FREE(cstring, field->value);
  if(field->uri)
    raptor_free_uri_v2(field->world, field->uri);
  if(field->next)
    raptor_rss_field_free(field->next);
  RAPTOR_FREE(raptor_rss_field, field);
}


#define RAPTOR_ISO_DATE_FORMAT "%Y-%m-%dT%H:%M:%SZ"

int
raptor_rss_format_iso_date(char* buffer, size_t len, time_t unix_time) 
{
  struct tm* structured_time;

  if(len < RAPTOR_ISO_DATE_LEN)
    return 1;

  structured_time=gmtime(&unix_time);
  strftime(buffer, len+1, RAPTOR_ISO_DATE_FORMAT, structured_time);

  return 0;
}


int
raptor_rss_set_date_field(raptor_rss_field* field, time_t unix_time)
{
  size_t len=RAPTOR_ISO_DATE_LEN;
  
  if(field->value)
    RAPTOR_FREE(cstring, field->value);
  field->value=(unsigned char*)RAPTOR_MALLOC(cstring, len + 1);
  if(!field->value)
    return 1;
  
  if(raptor_rss_format_iso_date((char*)field->value, len, unix_time)) {
    RAPTOR_FREE(cstring, field->value);
    return 1;
  }

  return 0;
}


int
raptor_rss_date_uplift(raptor_rss_field* to_field, 
                       const unsigned char *date_string)
{
#ifdef RAPTOR_PARSEDATE_FUNCTION
  time_t unix_time;

  unix_time=RAPTOR_PARSEDATE_FUNCTION((const char*)date_string, NULL);
  if(unix_time < 0)
    return 1;

  return raptor_rss_set_date_field(to_field, unix_time);
#else
  return 1;
#endif
}
