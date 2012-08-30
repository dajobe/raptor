/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_rss_common.c - Raptor Feeds (RSS and Atom) common code
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif


/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"
#include "raptor_rss.h"


static int raptor_rss_field_conversion_date_uplift(raptor_rss_field* from_field, raptor_rss_field* to_field);


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
  { ITUNES_NAMESPACE_URI,     "itunes",  },
};


const raptor_rss_item_info raptor_rss_items_info[RAPTOR_RSS_COMMON_SIZE+1]={
  { "channel",    RSS1_0_NS, RAPTOR_RSS_ITEM_CONTAINER, RAPTOR_RSS_FIELD_NONE, RAPTOR_RSS_FIELD_NONE },
  { "image",      RSS1_0_NS, RAPTOR_RSS_ITEM_CONTAINER, RAPTOR_RSS_FIELD_NONE, RAPTOR_RSS_FIELD_NONE },
  { "textinput",  RSS1_0_NS, RAPTOR_RSS_ITEM_CONTAINER, RAPTOR_RSS_FIELD_NONE, RAPTOR_RSS_FIELD_NONE },
  { "item",       RSS1_0_NS, RAPTOR_RSS_ITEM_CONTAINER, RAPTOR_RSS_FIELD_NONE, RAPTOR_RSS_FIELD_NONE },
  { "author",     ATOM1_0_NS, RAPTOR_RSS_ITEM_CONTAINER, RAPTOR_RSS_RDF_ATOM_AUTHOR_CLASS, RAPTOR_RSS_FIELD_ATOM_AUTHOR },
  { "Link",       ATOM1_0_NS, RAPTOR_RSS_ITEM_BLOCK, RAPTOR_RSS_RDF_ATOM_LINK_CLASS, RAPTOR_RSS_FIELD_ATOM_LINK },
  { "owner"  ,   ITUNES_NS, RAPTOR_RSS_ITEM_CONTAINER, RAPTOR_RSS_FIELD_ITUNES_OWNER, RAPTOR_RSS_FIELD_ITUNES_OWNER },
  { "skipHours",  RSS0_91_NS, RAPTOR_RSS_ITEM_CONTAINER, RAPTOR_RSS_FIELD_NONE, RAPTOR_RSS_FIELD_NONE },
  { "skipDays",   RSS0_91_NS, RAPTOR_RSS_ITEM_CONTAINER, RAPTOR_RSS_FIELD_NONE, RAPTOR_RSS_FIELD_NONE },
  { "Enclosure",  RSS2_0_ENC_NS, RAPTOR_RSS_ITEM_BLOCK, RAPTOR_RSS_RDF_ENCLOSURE_CLASS, RAPTOR_RSS_RDF_ENCLOSURE },
  { "category",   ATOM1_0_NS, RAPTOR_RSS_ITEM_BLOCK, RAPTOR_RSS_RDF_ATOM_CATEGORY_CLASS, RAPTOR_RSS_FIELD_ATOM_CATEGORY },
  { "source"  ,   RSS2_0_NS, RAPTOR_RSS_ITEM_BLOCK, RAPTOR_RSS_FIELD_SOURCE, RAPTOR_RSS_FIELD_NONE },
  { "feed",       ATOM1_0_NS, RAPTOR_RSS_ITEM_CONTAINER, RAPTOR_RSS_FIELD_NONE, RAPTOR_RSS_FIELD_NONE },
  { "entry",      ATOM1_0_NS, RAPTOR_RSS_ITEM_CONTAINER, RAPTOR_RSS_FIELD_NONE, RAPTOR_RSS_FIELD_NONE },
  { "<none>",  RSS_UNKNOWN_NS, RAPTOR_RSS_ITEM_CONTAINER, RAPTOR_RSS_FIELD_NONE, RAPTOR_RSS_FIELD_NONE }
};


const raptor_rss_field_info raptor_rss_fields_info[RAPTOR_RSS_FIELDS_SIZE+2]={
  { "title",          RSS1_0_NS, 0 },
  { "link",           RSS1_0_NS, 0 }, /* Actually a URI but RSS 1.0 spec wants this as an (XML & RDF) literal */
  { "description",    RSS1_0_NS, 0 },
  { "url",            RSS1_0_NS, 0 },
  { "name",           RSS1_0_NS, 0 },
  { "language",       RSS0_91_NS, 0 },
  { "rating",         RSS0_91_NS, 0 },
  { "copyright",      RSS0_91_NS, 0 },
  { "pubDate",        RSS0_91_NS, 0 },
  { "lastBuildDate",  RSS0_91_NS, 0 },
  { "docs",           RSS0_91_NS, RAPTOR_RSS_INFO_FLAG_URI_VALUE },
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
  { "enclosure",      RSS2_0_ENC_NS, 0 }, /* RDF output predicate, not an RSS field */
  { "Enclosure",      RSS2_0_ENC_NS, 0 }, /* RDF output class, not an RSS field */
  { "url",            RSS2_0_ENC_NS, 0 }, /* In RDF output, not an RSS field */
  { "length",         RSS2_0_ENC_NS, 0 }, /* In RDF output, not an RSS field */
  { "type",           RSS2_0_ENC_NS, 0 }, /* In RDF output, not an RSS field */
  { "length",         RSS2_0_NS, 0 },
  { "type",           RSS2_0_NS, 0 },
  { "category",       RSS0_92_NS, 0 },
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
  { "author",         ATOM1_0_NS, 0, RAPTOR_ATOM_AUTHOR },
  { "category",       ATOM1_0_NS, RAPTOR_RSS_INFO_FLAG_BLOCK_VALUE, RAPTOR_ATOM_CATEGORY },
  { "content",        ATOM1_0_NS, 0 },
  { "contributor",    ATOM1_0_NS, 0 },
  { "email",          ATOM1_0_NS, 0 },
  { "entry",          ATOM1_0_NS, 0 },
  { "feed",           ATOM1_0_NS, 0 },
  { "generator",      ATOM1_0_NS, 0 },
  { "icon",           ATOM1_0_NS, RAPTOR_RSS_INFO_FLAG_URI_VALUE },
  { "link",           ATOM1_0_NS, RAPTOR_RSS_INFO_FLAG_BLOCK_VALUE, RAPTOR_ATOM_LINK },
  { "logo",           ATOM1_0_NS, RAPTOR_RSS_INFO_FLAG_URI_VALUE },
  { "name",           ATOM1_0_NS, 0 },
  { "published",      ATOM1_0_NS, 0 },
  { "rights",         ATOM1_0_NS, 0 },
  { "source",         ATOM1_0_NS, RAPTOR_RSS_INFO_FLAG_BLOCK_VALUE, RAPTOR_RSS_SOURCE },
  { "subtitle",       ATOM1_0_NS, 0 },
  { "summary",        ATOM1_0_NS, 0 },
  { "uri",            ATOM1_0_NS, 0 },

  { "Author",         ATOM1_0_NS, 0 },
  { "Category",       ATOM1_0_NS, 0 },
  { "Link",           ATOM1_0_NS, 0 },

  { "label",          ATOM1_0_NS, 0 },
  { "scheme",         ATOM1_0_NS, RAPTOR_RSS_INFO_FLAG_URI_VALUE },
  { "term",           ATOM1_0_NS, 0 },
  { "href",           ATOM1_0_NS, RAPTOR_RSS_INFO_FLAG_URI_VALUE },
  { "rel",            ATOM1_0_NS, 0 },
  { "type",           ATOM1_0_NS, 0 },
  { "hreflang",       ATOM1_0_NS, 0 },
  { "length",         ATOM1_0_NS, 0 },

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

  { "author",         ITUNES_NS, 0 },
  { "subtitle",       ITUNES_NS, 0 },
  { "summary",        ITUNES_NS, 0 },
  { "keywords",       ITUNES_NS, 0 },
  { "explicit",       ITUNES_NS, 0 },
  { "image",          ITUNES_NS, 0 },
  { "name",           ITUNES_NS, 0 },
  { "owner",          ITUNES_NS, 0 },
  { "block",          ITUNES_NS, 0 },
  { "category",       ITUNES_NS, 0 },
  { "email",          ITUNES_NS, 0 },


  { "<unknown>",      RSS_UNKNOWN_NS, 0 },
  { "<none>",         RSS_UNKNOWN_NS, 0 }
};


/* FIeld mappings from atom fields to RSS/DC */
const raptor_field_pair raptor_atom_to_rss[]={
  /* rss clone of atom fields */
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
  /* other old atom 0.3 fields - IGNORED */
  { RAPTOR_RSS_FIELD_ATOM_CREATED,  RAPTOR_RSS_FIELD_UNKNOWN },
  { RAPTOR_RSS_FIELD_ATOM_ISSUED,   RAPTOR_RSS_FIELD_UNKNOWN },
  { RAPTOR_RSS_FIELD_ATOM_MODIFIED, RAPTOR_RSS_FIELD_UNKNOWN },
#endif

#ifdef RAPTOR_PARSEDATE_FUNCTION
  /* convert to ISO date */
  { RAPTOR_RSS_FIELD_PUBDATE,        RAPTOR_RSS_FIELD_DC_DATE,
    &raptor_rss_field_conversion_date_uplift },
#endif

  /* rss content encoded */
  { RAPTOR_RSS_FIELD_DESCRIPTION,    RAPTOR_RSS_FIELD_CONTENT_ENCODED },

  { RAPTOR_RSS_FIELD_UNKNOWN,        RAPTOR_RSS_FIELD_UNKNOWN }
};


const raptor_rss_block_field_info raptor_rss_block_fields_info[RAPTOR_RSS_BLOCKS_SIZE+1] = {
  /*
  RSS 2 <enclosure> - optional element inside <item>
  attributes:
    url (required):    where the enclosure is located.  url
    length (required): how big enclosure it is in bytes. integer
    type (required):   what enclosure type is as a standard MIME type. string
  content: empty
  */
  { RAPTOR_RSS_ENCLOSURE, "url",    RSS_BLOCK_FIELD_TYPE_URL,    0, RAPTOR_RSS_RDF_ENCLOSURE_URL },
  { RAPTOR_RSS_ENCLOSURE, "length", RSS_BLOCK_FIELD_TYPE_STRING, 0, RAPTOR_RSS_RDF_ENCLOSURE_LENGTH },
  { RAPTOR_RSS_ENCLOSURE, "type",   RSS_BLOCK_FIELD_TYPE_STRING, 1, RAPTOR_RSS_RDF_ENCLOSURE_TYPE },

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
  { RAPTOR_ATOM_CATEGORY, "term",   RSS_BLOCK_FIELD_TYPE_STRING, 0, RAPTOR_RSS_FIELD_ATOM_TERM },
  { RAPTOR_ATOM_CATEGORY, "scheme", RSS_BLOCK_FIELD_TYPE_URL, 0, RAPTOR_RSS_FIELD_ATOM_SCHEME },
  { RAPTOR_ATOM_CATEGORY, "label",  RSS_BLOCK_FIELD_TYPE_STRING, 1, RAPTOR_RSS_FIELD_ATOM_LABEL },

  /*
  Atom <link> - optional element inside <entry>
  attributes:
    href (required):  . url
    rel (optional): . string
    type (optional): . string
    hreflang (optional): . string
    title (optional): . string
    length (optional): . string
  content: empty
  */
  { RAPTOR_ATOM_LINK, "href", RSS_BLOCK_FIELD_TYPE_URL, RAPTOR_RSS_LINK_HREF_URL_OFFSET, RAPTOR_RSS_FIELD_ATOM_HREF },
  { RAPTOR_ATOM_LINK, "rel",  RSS_BLOCK_FIELD_TYPE_STRING, RAPTOR_RSS_LINK_REL_STRING_OFFSET, RAPTOR_RSS_FIELD_ATOM_REL },
  { RAPTOR_ATOM_LINK, "type", RSS_BLOCK_FIELD_TYPE_STRING, 1, RAPTOR_RSS_FIELD_ATOM_TYPE },
  { RAPTOR_ATOM_LINK, "hreflang", RSS_BLOCK_FIELD_TYPE_STRING, 2, RAPTOR_RSS_FIELD_ATOM_HREFLANG },
  { RAPTOR_ATOM_LINK, "title", RSS_BLOCK_FIELD_TYPE_STRING, 3, RAPTOR_RSS_FIELD_ATOM_TITLE },
  { RAPTOR_ATOM_LINK, "length", RSS_BLOCK_FIELD_TYPE_STRING, 4, RAPTOR_RSS_FIELD_ATOM_LENGTH },
  { RAPTOR_ATOM_LINK, NULL, RSS_BLOCK_FIELD_TYPE_URL, 0, RAPTOR_RSS_FIELD_ATOM_HREF },

  /* sentinel */
  { RAPTOR_RSS_NONE, NULL, 0, 0 }
};


const unsigned char * const raptor_atom_namespace_uri = (const unsigned char *)"http://www.w3.org/2005/Atom";



int
raptor_rss_common_init(raptor_world* world) {
  int i;
  raptor_uri *namespace_uri;

  if(world->rss_common_initialised++)
    return 0;

  world->rss_namespaces_info_uris = RAPTOR_CALLOC(raptor_uri**, 
                                                  RAPTOR_RSS_NAMESPACES_SIZE,
                                                  sizeof(raptor_uri*));
  if(!world->rss_namespaces_info_uris)
    return -1;
  for(i = 0; i < RAPTOR_RSS_NAMESPACES_SIZE;i++) {
    const char *uri_string = raptor_rss_namespaces_info[i].uri_string;
    if(uri_string) {
      world->rss_namespaces_info_uris[i] = raptor_new_uri(world, (const unsigned char*)uri_string);
      if(!world->rss_namespaces_info_uris[i])
        return -1;
    }
  }

  world->rss_types_info_uris = RAPTOR_CALLOC(raptor_uri**,
                                             RAPTOR_RSS_COMMON_SIZE,
                                             sizeof(raptor_uri*));
  if(!world->rss_types_info_uris)
    return -1;
  for(i = 0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    int n = raptor_rss_items_info[i].nspace;
    namespace_uri = world->rss_namespaces_info_uris[n];
    if(namespace_uri) {
      world->rss_types_info_uris[i] = raptor_new_uri_from_uri_local_name(world, namespace_uri, (const unsigned char*)raptor_rss_items_info[i].name);
      if(!world->rss_types_info_uris[i])
        return -1;
    }
  }

  world->rss_fields_info_uris = RAPTOR_CALLOC(raptor_uri**,
                                              RAPTOR_RSS_FIELDS_SIZE,
                                              sizeof(raptor_uri*));
  if(!world->rss_fields_info_uris)
    return -1;
  for(i = 0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
    namespace_uri = world->rss_namespaces_info_uris[raptor_rss_fields_info[i].nspace];
    if(namespace_uri) {
      world->rss_fields_info_uris[i] = raptor_new_uri_from_uri_local_name(world, namespace_uri,
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
    for(i = 0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
      if(world->rss_types_info_uris[i])
        raptor_free_uri(world->rss_types_info_uris[i]);
    }
    RAPTOR_FREE(raptor_uri* array, world->rss_types_info_uris);
    world->rss_types_info_uris = NULL;
  }

  if(world->rss_fields_info_uris) {
    for(i = 0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
      if(world->rss_fields_info_uris[i])
        raptor_free_uri(world->rss_fields_info_uris[i]);
    }
    RAPTOR_FREE(raptor_uri* array, world->rss_fields_info_uris);
    world->rss_fields_info_uris = NULL;
  }

  if(world->rss_namespaces_info_uris) {
    for(i = 0; i < RAPTOR_RSS_NAMESPACES_SIZE;i++) {
      if(world->rss_namespaces_info_uris[i])
        raptor_free_uri(world->rss_namespaces_info_uris[i]);
    }
    RAPTOR_FREE(raptor_uri* array, world->rss_namespaces_info_uris);
    world->rss_namespaces_info_uris = NULL;
  }
}


void
raptor_rss_model_init(raptor_world* world, raptor_rss_model* rss_model)
{
  memset(rss_model->common, 0,
         sizeof(raptor_rss_item*) * RAPTOR_RSS_COMMON_SIZE);

  rss_model->world = world;

  rss_model->last = rss_model->items = NULL;
  rss_model->items_count = 0;

  RAPTOR_RSS_RSS_items_URI(rss_model) = raptor_new_uri_relative_to_base(world, world->rss_namespaces_info_uris[RSS1_0_NS], (const unsigned char*)"items");
}
  

void
raptor_rss_model_clear(raptor_rss_model* rss_model)
{
  int i;
  raptor_rss_item* item;
  
  for(i = 0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    item = rss_model->common[i];
    while(item) {
      raptor_rss_item *next = item->next;
      raptor_free_rss_item(item);
      item = next;
    }
  }

  item = rss_model->items;
  while(item) {
    raptor_rss_item *next = item->next;

    raptor_free_rss_item(item);
    item = next;
  }
  rss_model->last = rss_model->items = NULL;

  for(i = 0; i< RAPTOR_RSS_N_CONCEPTS; i++) {
    raptor_uri* concept_uri = rss_model->concepts[i];
    if(concept_uri) {
      raptor_free_uri(concept_uri);
      rss_model->concepts[i] = NULL;
    }
  }
}


raptor_rss_item*
raptor_new_rss_item(raptor_world* world)
{
  raptor_rss_item* item;

  item = RAPTOR_CALLOC(raptor_rss_item*, 1, sizeof(*item));
  if(!item)
    return NULL;
  
  item->world = world;
  item->triples = raptor_new_sequence((raptor_data_free_handler)raptor_free_statement, (raptor_data_print_handler)raptor_statement_print);
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

  item = raptor_new_rss_item(rss_model->world);
  if(!item)
    return 1;

  /* new list */
  if(!rss_model->items)
    rss_model->items = item;
  
  /* join last item to this one */
  if(rss_model->last)
    rss_model->last->next = item;
  
  /* this is now the last item */
  rss_model->last = item;
  rss_model->items_count++;

  RAPTOR_DEBUG2("Added item %d\n", rss_model->items_count);

  return 0;
}


raptor_rss_item*
raptor_rss_model_add_common(raptor_rss_model* rss_model,
                            raptor_rss_type type)
{
  raptor_rss_item* item;

  item = raptor_new_rss_item(rss_model->world);
  if(!item)
    return NULL;

  if(rss_model->common[type] == NULL) {
    RAPTOR_DEBUG3("Adding common type %d - %s\n", type,
                  raptor_rss_items_info[type].name);
    rss_model->common[type] = item; 
  } else {
    raptor_rss_item* next;
    RAPTOR_DEBUG3("Appending common type %d - %s\n", type, 
                  raptor_rss_items_info[type].name);
    for(next = rss_model->common[type]; next->next; next = next->next)
      ;
    next->next = item;
  }
  return item;
}


raptor_rss_item*
raptor_rss_model_get_common(raptor_rss_model* rss_model, raptor_rss_type type)
{
  raptor_rss_item* item;
  for(item = rss_model->common[type]; 
      item && item->next;
      item = item->next) ;
  return item;
}


void
raptor_free_rss_item(raptor_rss_item* item)
{
  int i;
  for(i = 0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
    if(item->fields[i])
      raptor_rss_field_free(item->fields[i]);
  }
  if(item->blocks) 
    raptor_free_rss_block(item->blocks);
  if(item->uri)
    raptor_free_uri(item->uri);
  if(item->term)
    raptor_free_term(item->term);
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
    item->fields[type] = field;
  } else { 
    raptor_rss_field* cur;

    RAPTOR_DEBUG1("Adding subsequent field\n");
    for(cur = item->fields[type]; cur->next; cur = cur->next) ;
    cur->next = field;
  }
}


int
raptor_rss_item_equals_statement_subject(const raptor_rss_item *item,
                                         const raptor_statement *statement)
{
  return raptor_term_equals(statement->subject, item->term);
}


int
raptor_rss_item_set_uri(raptor_rss_item *item, raptor_uri* uri)
{
  RAPTOR_DEBUG3("Set node %p to URI <%s>\n", item,
                raptor_uri_as_string(uri));
  
  item->uri = raptor_uri_copy(uri);
  if(!item->uri)
    return 1;
  
  item->term = raptor_new_term_from_uri(item->world, item->uri);
  return 0;
}


/*
 * raptor_new_rss_block:
 * @world: world
 * @type: RSS block type
 * @block_term: Block subject term (shared)
 *
 * INTERNAL - Create a new RSS Block such as <author> etc
 *
 * Return value: new RSS block or NULL on failure
 */
raptor_rss_block*
raptor_new_rss_block(raptor_world* world, raptor_rss_type type,
                     raptor_term* block_term)
{
  raptor_rss_block *block;
  block = RAPTOR_CALLOC(raptor_rss_block*, 1, sizeof(*block));

  if(block) {
    block->rss_type = type;
    block->node_type = world->rss_types_info_uris[type];
    block->identifier = raptor_term_copy(block_term);
  }
  
  return block;
}


void
raptor_free_rss_block(raptor_rss_block *block)
{
  int i;

  for(i = 0; i < RSS_BLOCK_MAX_URLS; i++) {
    if(block->urls[i])
      raptor_free_uri(block->urls[i]);
  }

  for(i = 0; i < RSS_BLOCK_MAX_STRINGS; i++) {
    if(block->strings[i])
      RAPTOR_FREE(char*, block->strings[i]);
  }

  if(block->next)
    raptor_free_rss_block(block->next);

  if(block->identifier)
    raptor_free_term(block->identifier);

  RAPTOR_FREE(raptor_rss_block, block);
}


raptor_rss_field*
raptor_rss_new_field(raptor_world* world)
{
  raptor_rss_field* field = RAPTOR_CALLOC(raptor_rss_field*, 1, sizeof(*field));
  if(field)
    field->world = world;
  return field;
}


void
raptor_rss_field_free(raptor_rss_field* field)
{
  if(field->value)
    RAPTOR_FREE(char*, field->value);
  if(field->uri)
    raptor_free_uri(field->uri);
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

  structured_time = gmtime(&unix_time);
  strftime(buffer, len+1, RAPTOR_ISO_DATE_FORMAT, structured_time);

  return 0;
}


int
raptor_rss_set_date_field(raptor_rss_field* field, time_t unix_time)
{
  size_t len = RAPTOR_ISO_DATE_LEN;
  
  if(field->value)
    RAPTOR_FREE(char*, field->value);
  field->value = RAPTOR_MALLOC(unsigned char*, len + 1);
  if(!field->value)
    return 1;
  
  if(raptor_rss_format_iso_date((char*)field->value, len, unix_time)) {
    RAPTOR_FREE(char*, field->value);
    return 1;
  }

  return 0;
}


static int
raptor_rss_field_conversion_date_uplift(raptor_rss_field* from_field,
                                        raptor_rss_field* to_field)
{
#ifdef RAPTOR_PARSEDATE_FUNCTION
  time_t unix_time;
  char *date_string = (char*)from_field->value;
  
  if(!date_string)
    return 1;
  
  unix_time = RAPTOR_PARSEDATE_FUNCTION(date_string, NULL);
  if(unix_time < 0)
    return 1;

  return raptor_rss_set_date_field(to_field, unix_time);
#else
  return 1;
#endif
}
