/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_rss.h - Redland Parser Toolkit Internal RSS Model and API
 *
 * Copyright (C) 2004-2008, David Beckett http://www.dajobe.org/
 * Copyright (C) 2004-2005, University of Bristol, UK http://www.bristol.ac.uk/
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



#ifndef RAPTOR_RSS_H
#define RAPTOR_RSS_H

#ifdef __cplusplus
extern "C" {
#endif


typedef enum {
/* CONTAINERs */
  /* common */
  RAPTOR_RSS_CHANNEL,
  RAPTOR_RSS_IMAGE,
  RAPTOR_RSS_TEXTINPUT,

  /* list items */
  RAPTOR_RSS_ITEM,

  /* atom author */
  RAPTOR_ATOM_AUTHOR,
  /* atom link */
  RAPTOR_ATOM_LINK,

  /* itunes owner */
  RAPTOR_ITUNES_OWNER,

  /* containers but IGNORED */
  RAPTOR_RSS_SKIPHOURS,
  RAPTOR_RSS_SKIPDAYS,

/* metadata BLOCKs */
  RAPTOR_RSS_ENCLOSURE,
  RAPTOR_ATOM_CATEGORY,
  RAPTOR_RSS_SOURCE,

/* serializing containers */
  RAPTOR_ATOM_FEED,
  RAPTOR_ATOM_ENTRY,

  /* nothing found yet */
  RAPTOR_RSS_NONE,

  /* deliberately not counting NONE */
  RAPTOR_RSS_COMMON_SIZE    = RAPTOR_RSS_NONE - RAPTOR_RSS_CHANNEL,
  RAPTOR_RSS_COMMON_IGNORED = RAPTOR_RSS_SKIPHOURS
} raptor_rss_type;


/* Namespaces used in RSS */
#define RSS1_0_NAMESPACE_URI     "http://purl.org/rss/1.0/"
#define RSS0_91_NAMESPACE_URI    "http://purl.org/rss/1.0/modules/rss091#"
#define RSS2_0_ENC_NAMESPACE_URI "http://purl.oclc.org/net/rss_2.0/enc#"
#define ATOM0_3_NAMESPACE_URI    "http://purl.org/atom/ns#"
#define DC_NAMESPACE_URI         "http://purl.org/dc/elements/1.1/"
#define RSS1_1_NAMESPACE_URI     "http://purl.org/net/rss1.1#"
#define CONTENT_NAMESPACE_URI    "http://purl.org/rss/1.0/modules/content/"
#define ATOM1_0_NAMESPACE_URI    "http://www.w3.org/2005/Atom"
#define RDF_NAMESPACE_URI        "http://www.w3.org/1999/02/22-rdf-syntax-ns#"
#define ATOMTRIPLES_NAMESPACE_URI "http://purl.org/syndication/atomtriples/1"
#define ITUNES_NAMESPACE_URI      "http://www.itunes.com/dtds/podcast-1.0.dtd"

/* Old netscape namespace, turn into RSS 1.0 */
#define RSS0_9_NAMESPACE_URI  "http://my.netscape.com/rdf/simple/0.9/"

typedef enum {
  RSS_UNKNOWN_NS = 0,
  RSS_NO_NS      = 1,
  RSS0_91_NS     = 2,
  RSS0_9_NS      = 3,
  RSS0_92_NS     = RSS_NO_NS,
  RSS2_0_NS      = RSS_NO_NS,
  RSS1_0_NS      = 4,
  ATOM0_3_NS     = 5,
  DC_NS          = 6,
  RSS2_0_ENC_NS  = 7,
  RSS1_1_NS      = 8,
  CONTENT_NS     = 9,
  ATOM1_0_NS     = 10,
  RDF_NS         = 11,
  ATOMTRIPLES_NS = 12,
  ITUNES_NS      = 13,

  RAPTOR_RSS_NAMESPACES_SIZE = ITUNES_NS + 1
} rss_info_namespace;


typedef struct {
  const char *uri_string;
  const char *prefix;
} raptor_rss_namespace_info;


extern const raptor_rss_namespace_info raptor_rss_namespaces_info[RAPTOR_RSS_NAMESPACES_SIZE];
  
#define RAPTOR_RSS_INFO_FLAG_URI_VALUE 1
#define RAPTOR_RSS_INFO_FLAG_BLOCK_VALUE 2

/* Namespaced elements used in feeds */
typedef struct {
  const char* name;
  rss_info_namespace nspace;
  int flags;
  raptor_rss_type block_type;
} raptor_rss_field_info;

/* Fields of typed nodes used in RSS */
typedef enum {
  RAPTOR_RSS_FIELD_TITLE,
  RAPTOR_RSS_FIELD_LINK,
  RAPTOR_RSS_FIELD_DESCRIPTION,
  RAPTOR_RSS_FIELD_URL,           /* image */
  RAPTOR_RSS_FIELD_NAME,          /* textinput */
  RAPTOR_RSS_FIELD_LANGUAGE,      /* channel 0.91 */
  RAPTOR_RSS_FIELD_RATING,        /* channel 0.91 */
  RAPTOR_RSS_FIELD_COPYRIGHT,     /* channel 0.91 */
  RAPTOR_RSS_FIELD_PUBDATE,       /* channel 0.91, item 2.0 */
  RAPTOR_RSS_FIELD_LASTBUILDDATE, /* channel 0.91 */
  RAPTOR_RSS_FIELD_DOCS,          /* channel 0.91 */
  RAPTOR_RSS_FIELD_MANAGINGEDITOR,/* channel 0.91 */
  RAPTOR_RSS_FIELD_WEBMASTER,     /* channel 0.91 */
  RAPTOR_RSS_FIELD_CLOUD,         /* channel 0.92, 2.0 */
  RAPTOR_RSS_FIELD_TTL,           /* channel 2.0 */
  RAPTOR_RSS_FIELD_WIDTH,         /* image 0.91 */
  RAPTOR_RSS_FIELD_HEIGHT,        /* image 0.91 */
  RAPTOR_RSS_FIELD_HOUR,          /* skipHours 0.91 */
  RAPTOR_RSS_FIELD_DAY,           /* skipDays 0.91 */
  RAPTOR_RSS_FIELD_GENERATOR,     /* channel 0.92, 2.0 */
  RAPTOR_RSS_FIELD_SOURCE,        /* item 0.92, 2.0 */
  RAPTOR_RSS_FIELD_AUTHOR,        /* item 2.0 */
  RAPTOR_RSS_FIELD_GUID,          /* item 2.0 */
  RAPTOR_RSS_FIELD_ENCLOSURE,     /* item 0.92, 2.0 */
  RAPTOR_RSS_RDF_ENCLOSURE,        /* In RDF output, not an RSS field */
  RAPTOR_RSS_RDF_ENCLOSURE_CLASS,  /* In RDF output, not an RSS field */
  RAPTOR_RSS_RDF_ENCLOSURE_URL,    /* In RDF output, not an RSS field */
  RAPTOR_RSS_RDF_ENCLOSURE_LENGTH, /* In RDF output, not an RSS field */
  RAPTOR_RSS_RDF_ENCLOSURE_TYPE,   /* In RDF output, not an RSS field */
  RAPTOR_RSS_FIELD_LENGTH,        /* item 0.92, 2.0 */
  RAPTOR_RSS_FIELD_TYPE,          /* item 0.92, 2.0 */
  RAPTOR_RSS_FIELD_CATEGORY,      /* item 0.92, 2.0, channel 2.0 */
  RAPTOR_RSS_FIELD_COMMENTS,      /* comments v? */
  RAPTOR_RSS_FIELD_ITEMS,         /* rss 1.0 items */
  RAPTOR_RSS_FIELD_IMAGE,         /* rss 1.0 property from channel->image) */
  RAPTOR_RSS_FIELD_TEXTINPUT,     /* rss 1.0 property from channel->textinput */

  RAPTOR_RSS_FIELD_ATOM_COPYRIGHT, /* atom 0.3 copyright */
  RAPTOR_RSS_FIELD_ATOM_CREATED,   /* atom 0.3 created */
  RAPTOR_RSS_FIELD_ATOM_ISSUED,    /* atom 0.3 issued */
  RAPTOR_RSS_FIELD_ATOM_MODIFIED,  /* atom 0.3 modified */
  RAPTOR_RSS_FIELD_ATOM_TAGLINE,   /* atom 0.3 tagline */

  /* atom 1.0 required fields */
  RAPTOR_RSS_FIELD_ATOM_ID,          /* atom 1.0 id */
  RAPTOR_RSS_FIELD_ATOM_TITLE,       /* atom 1.0 title */
  RAPTOR_RSS_FIELD_ATOM_UPDATED,     /* atom 1.0 updated */
  /* atom 1.0 optional fields */
  RAPTOR_RSS_FIELD_ATOM_AUTHOR,      /* atom 1.0 author */
  RAPTOR_RSS_FIELD_ATOM_CATEGORY,    /* atom 1.0 category */
  RAPTOR_RSS_FIELD_ATOM_CONTENT,     /* atom 1.0 content */
  RAPTOR_RSS_FIELD_ATOM_CONTRIBUTOR, /* atom 1.0 contributor */
  RAPTOR_RSS_FIELD_ATOM_EMAIL,       /* atom 1.0 email */
  RAPTOR_RSS_FIELD_ATOM_ENTRY,       /* atom 1.0 entry */
  RAPTOR_RSS_FIELD_ATOM_FEED,        /* atom 1.0 feed */
  RAPTOR_RSS_FIELD_ATOM_GENERATOR,   /* atom 1.0 generator */
  RAPTOR_RSS_FIELD_ATOM_ICON,        /* atom 1.0 icon */
  RAPTOR_RSS_FIELD_ATOM_LINK,        /* atom 1.0 link */
  RAPTOR_RSS_FIELD_ATOM_LOGO,        /* atom 1.0 logo */
  RAPTOR_RSS_FIELD_ATOM_NAME,        /* atom 1.0 name */
  RAPTOR_RSS_FIELD_ATOM_PUBLISHED,   /* atom 1.0 published */
  RAPTOR_RSS_FIELD_ATOM_RIGHTS,      /* atom 1.0 rights */
  RAPTOR_RSS_FIELD_ATOM_SOURCE,      /* atom 1.0 source */
  RAPTOR_RSS_FIELD_ATOM_SUBTITLE,    /* atom 1.0 subtitle */
  RAPTOR_RSS_FIELD_ATOM_SUMMARY,     /* atom 1.0 summary */
  RAPTOR_RSS_FIELD_ATOM_URI,         /* atom 1.0 uri */

  RAPTOR_RSS_RDF_ATOM_AUTHOR_CLASS,      /* In RDF output, not atom field */
  RAPTOR_RSS_RDF_ATOM_CATEGORY_CLASS,    /* In RDF output, not atom field */
  RAPTOR_RSS_RDF_ATOM_LINK_CLASS,        /* In RDF output, not atom field */

  RAPTOR_RSS_FIELD_ATOM_LABEL,      /* atom 1.0 attribute label */
  RAPTOR_RSS_FIELD_ATOM_SCHEME,     /* atom 1.0 attribute scheme */
  RAPTOR_RSS_FIELD_ATOM_TERM,       /* atom 1.0 attribute term */
  RAPTOR_RSS_FIELD_ATOM_HREF,       /* atom 1.0 attribute term */
  RAPTOR_RSS_FIELD_ATOM_REL,        /* atom 1.0 attribute term */
  RAPTOR_RSS_FIELD_ATOM_TYPE,       /* atom 1.0 attribute term */
  RAPTOR_RSS_FIELD_ATOM_HREFLANG,   /* atom 1.0 attribute term */
  RAPTOR_RSS_FIELD_ATOM_LENGTH,     /* atom 1.0 attribute term */

  RAPTOR_RSS_FIELD_DC_TITLE,       /* DC title */
  RAPTOR_RSS_FIELD_DC_CONTRIBUTOR, /* DC contributor */
  RAPTOR_RSS_FIELD_DC_CREATOR,     /* DC creator */
  RAPTOR_RSS_FIELD_DC_PUBLISHER,   /* DC publisher */
  RAPTOR_RSS_FIELD_DC_SUBJECT,     /* DC subject */
  RAPTOR_RSS_FIELD_DC_DESCRIPTION, /* DC description */
  RAPTOR_RSS_FIELD_DC_DATE,        /* DC date */
  RAPTOR_RSS_FIELD_DC_TYPE,        /* DC type */
  RAPTOR_RSS_FIELD_DC_FORMAT,      /* DC format */
  RAPTOR_RSS_FIELD_DC_IDENTIFIER,  /* DC identifier */
  RAPTOR_RSS_FIELD_DC_LANGUAGE,    /* DC language */
  RAPTOR_RSS_FIELD_DC_RELATION,    /* DC relation */
  RAPTOR_RSS_FIELD_DC_SOURCE,      /* DC source */
  RAPTOR_RSS_FIELD_DC_COVERAGE,    /* DC coverage */
  RAPTOR_RSS_FIELD_DC_RIGHTS,      /* DC rights */


  RAPTOR_RSS_FIELD_CONTENT_ENCODED,  /* rss 1.0 module content:encoded */

  RAPTOR_RSS_FIELD_AT_CONTENT_TYPE,  /* at:contentType */

  RAPTOR_RSS_FIELD_ITUNES_AUTHOR,
  RAPTOR_RSS_FIELD_ITUNES_SUBTITLE,
  RAPTOR_RSS_FIELD_ITUNES_SUMARY,
  RAPTOR_RSS_FIELD_ITUNES_KEYWORDS,
  RAPTOR_RSS_FIELD_ITUNES_EXPLICIT,
  RAPTOR_RSS_FIELD_ITUNES_IMAGE,
  RAPTOR_RSS_FIELD_ITUNES_NAME,
  RAPTOR_RSS_FIELD_ITUNES_OWNER,
  RAPTOR_RSS_FIELD_ITUNES_BLOCK,
  RAPTOR_RSS_FIELD_ITUNES_CATEGORY,
  RAPTOR_RSS_FIELD_ITUNES_EMAIL,

  RAPTOR_RSS_FIELD_UNKNOWN,

  RAPTOR_RSS_FIELD_NONE,

  RAPTOR_RSS_FIELDS_SIZE=RAPTOR_RSS_FIELD_UNKNOWN
} raptor_rss_fields_type;

extern const raptor_rss_field_info raptor_rss_fields_info[RAPTOR_RSS_FIELDS_SIZE+2];

typedef struct raptor_rss_field_s raptor_rss_field;

typedef int (*raptor_rss_field_conversion)(raptor_rss_field* from_field, raptor_rss_field* to_field);

typedef struct {
  raptor_rss_fields_type from;
  raptor_rss_fields_type to;
  raptor_rss_field_conversion conversion;
} raptor_field_pair;

extern const raptor_field_pair raptor_atom_to_rss[];


#define RSS_BLOCK_FIELD_TYPE_URL 0
#define RSS_BLOCK_FIELD_TYPE_STRING 1

#define RSS_BLOCK_MAX_URLS 1
#define RSS_BLOCK_MAX_STRINGS 5

/* Feed metadata blocks support (was raptor_rss_enclosure) */
struct raptor_rss_block_s
{
  raptor_rss_type rss_type;
 
 /* enclosure: subject node URI/blank node */
  raptor_identifier identifier;

  /* enclosure: node RAPTOR_RSS_ENCLOSURE
     category: node RAPTOR_ATOM_CATEGORY 
     person: node RAPTOR_ATOM_AUTHOR or RAPTOR_ATOM_CONTRIBUTOR
     link: node RAPTOR_ATOM_LINK
   */
  raptor_uri *node_type;

  /* enclosure: 0: where enclosure is located - @url attr (required)
     atom category: 0: @scheme attr (optional) 
     rss category: 0: @domain attr (optional)
     rss source: 0: @url attr (required)
     person: 0: @atom:uri attr (optional)
     link: 0: @href attr (required)
   */
  raptor_uri *urls[RSS_BLOCK_MAX_URLS];

  /* enclosure: 0: content length @length attr (required)
                1: content type @type attr (required)
     atom category:  0: @term attr (required)
                     1: @label attr (optional)
     person:    0: @atom:name attr (required)
                1: @atom:email attr (optional)
     link:      0: @length attr (optional)
                1: @type attr (optional)
                2: @rel attr (optional)
                3: @hreflang attr (optional)
                4: @title attr (optional)
   */
  char *strings[RSS_BLOCK_MAX_STRINGS];

  /* next in list */
  struct raptor_rss_block_s* next;
};
typedef struct raptor_rss_block_s raptor_rss_block;

#define RAPTOR_RSS_ITEM_CONTAINER 1
#define RAPTOR_RSS_ITEM_BLOCK     2
typedef struct {
  const char* name;
  rss_info_namespace nspace;
  int flags;
  /* RDF class URI */
  raptor_rss_fields_type cls;
  /* RDF predicate URI to connect to the instance of this item */
  raptor_rss_fields_type predicate;
} raptor_rss_item_info;


extern const raptor_rss_item_info raptor_rss_items_info[RAPTOR_RSS_COMMON_SIZE+1];

#define RAPTOR_RSS_BLOCKS_SIZE 17
/* Metadata blocks info */
typedef struct {
  /* metadata block type it applies to */
  raptor_rss_type type;
  /* XML attribute (or NULL for field to use to store CDATA) */
  const char *attribute;
  /* How that attribute should be interpreted: url or string */
  int attribute_type;
  /* Index into urls/strings array to store it */
  int offset;
  /* RDF predicate this maps to */
  raptor_rss_fields_type field;
} raptor_rss_block_field_info;

extern const raptor_rss_block_field_info raptor_rss_block_fields_info[RAPTOR_RSS_BLOCKS_SIZE+1];


struct raptor_rss_field_s
{
  raptor_world* world;
  unsigned char* value;
  raptor_uri* uri;
  struct raptor_rss_field_s* next;
  /* this field was mapped from another vocab */
  unsigned int is_mapped:1;
  /* value is XML */
  unsigned int is_xml:1;
};

#define RAPTOR_RSS_FIELD_MAPPED

/* RSS items (instances of typed nodes) containing fields */
struct raptor_rss_item_s
{
  raptor_world* world;
  raptor_uri *uri;
  raptor_identifier identifier;
  const raptor_rss_item_info *node_type;
  int node_typei;
  raptor_rss_field* fields[RAPTOR_RSS_FIELDS_SIZE];
  raptor_rss_block* blocks;
  int fields_count;
  struct raptor_rss_item_s* next;
  /* Triples with this item as subject and do not fit in @fields */
  raptor_sequence* triples;
};
typedef struct raptor_rss_item_s raptor_rss_item;


/* raptor_rss_common.c */
#define RAPTOR_RSS_N_CONCEPTS 3

#define RAPTOR_RSS_RDF_type_URI(rss_model)  ((rss_model)->concepts[0])
#define RAPTOR_RSS_RDF_Seq_URI(rss_model)   ((rss_model)->concepts[1])
#define RAPTOR_RSS_RSS_items_URI(rss_model) ((rss_model)->concepts[2])


typedef struct {
  raptor_world* world;

  /* RAPTOR_RSS_CHANNEL, RAPTOR_RSS_IMAGE, RAPTOR_RSS_TEXTINPUT */
  raptor_rss_item* common[RAPTOR_RSS_COMMON_SIZE];

  /* list of items RAPTOR_RSS_ITEM */
  raptor_rss_item* items;

  /* this points to the last one added, so we can append easy */
  raptor_rss_item* last;

  /* item count */
  int items_count;

  raptor_uri* concepts[RAPTOR_RSS_N_CONCEPTS];

  raptor_namespace_stack *nstack;

} raptor_rss_model;


/* raptor_rss_common.c */
int raptor_rss_common_init(raptor_world* world);
void raptor_rss_common_terminate(raptor_world* world);

void raptor_rss_model_init(raptor_world* world, raptor_rss_model* rss_model);
void raptor_rss_model_clear(raptor_rss_model* rss_model);

raptor_rss_item* raptor_new_rss_item(raptor_world* world);
int raptor_rss_model_add_item(raptor_rss_model* rss_model);
raptor_rss_item* raptor_rss_model_add_common(raptor_rss_model* rss_model, raptor_rss_type type);
raptor_rss_item* raptor_rss_model_get_common(raptor_rss_model* rss_model, raptor_rss_type type);

void raptor_clear_rss_item(raptor_rss_item* item);
void raptor_free_rss_item(raptor_rss_item* item);
void raptor_rss_item_add_block(raptor_rss_item* item, raptor_rss_block *block);
void raptor_rss_item_add_field(raptor_rss_item* item, int type, raptor_rss_field* field);
int raptor_rss_item_equals_statement_subject(const raptor_rss_item *item, const raptor_statement *statement);
int raptor_rss_item_set_uri(raptor_rss_item *item, raptor_uri* uri);

raptor_rss_block *raptor_new_rss_block(raptor_world *world, raptor_rss_type rss_type, const unsigned char* id);
void raptor_free_rss_block(raptor_rss_block *block);

raptor_rss_field* raptor_rss_new_field(raptor_world* world);
void raptor_rss_field_free(raptor_rss_field* field);

#define RAPTOR_ISO_DATE_LEN 20
int raptor_rss_format_iso_date(char* buffer, size_t len, time_t unix_time);
int raptor_rss_set_date_field(raptor_rss_field* field, time_t unix_time);
int raptor_rss_date_uplift(raptor_rss_field* to_field, const unsigned char *date_string);

#ifdef __cplusplus
}
#endif

#endif
