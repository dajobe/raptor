/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_rss.c - Raptor RSS tag soup parser
 *
 * $Id$
 *
 * Copyright (C) 2003-2004, David Beckett http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology http://www.ilrt.bristol.ac.uk/
 * University of Bristol, UK http://www.bristol.ac.uk/
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


#ifdef HAVE_LIBXML_XMLREADER_H
#include <libxml/xmlreader.h>



typedef enum {
  /* common */
  RAPTOR_RSS_CHANNEL,
  RAPTOR_RSS_IMAGE,
  RAPTOR_RSS_TEXTINPUT,

  /* list items */
  RAPTOR_RSS_ITEM,

  /* atom author */
  RAPTOR_ATOM_AUTHOR,

  /* also common, but IGNORED */
  RAPTOR_RSS_SKIPHOURS,
  RAPTOR_RSS_SKIPDAYS,
  RAPTOR_RSS_ENCLOSURE,

  /* unknown name found */
  RAPTOR_RSS_UNKNOWN,

  /* nothing found yet */
  RAPTOR_RSS_NONE,

  /* deliberately not counting NONE */
  RAPTOR_RSS_COMMON_SIZE=RAPTOR_RSS_NONE-RAPTOR_RSS_CHANNEL,
  RAPTOR_RSS_COMMON_IGNORED=RAPTOR_RSS_SKIPHOURS
} raptor_rss_type;


/* Namespaces used in RSS */
#define RSS1_0_NAMESPACE_URI  "http://purl.org/rss/1.0/"
#define RSS0_91_NAMESPACE_URI "http://purl.org/rss/1.0/modules/rss091#"
#define RSS2_0_ENC_NAMESPACE_URI "http://purl.oclc.org/net/rss_2.0/enc#"
#define ATOM0_3_NAMESPACE_URI "http://purl.org/atom/ns#"
#define DC_NAMESPACE_URI      "http://purl.org/dc/elements/1.1/"

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

  RAPTOR_RSS_NAMESPACES_SIZE = RSS2_0_ENC_NS+1
} rss_info_namespace;


typedef struct {
  const char *const uri_string;
  const char *prefix;
  raptor_uri* uri;
  raptor_namespace* nspace;
} raptor_rss_namespace_info;


static raptor_rss_namespace_info raptor_rss_namespaces_info[RAPTOR_RSS_NAMESPACES_SIZE]={
  { NULL,                     NULL,  },
  { NULL,                     NULL,  },
  { RSS0_91_NAMESPACE_URI,    "rss091", },
  { RSS0_9_NAMESPACE_URI,     NULL,   },
  { RSS1_0_NAMESPACE_URI,     NULL,   }, /* default namespace on writing */
  { ATOM0_3_NAMESPACE_URI,    "atom", },
  { DC_NAMESPACE_URI,         "dc",   },
  { RSS2_0_ENC_NAMESPACE_URI, "enc",  }
};


/* Typed nodes used in RSS */
typedef struct {
  const char* name;
  rss_info_namespace nspace;
  raptor_uri* uri;
  raptor_qname* qname;
} raptor_rss_info;

static raptor_rss_info raptor_rss_types_info[RAPTOR_RSS_COMMON_SIZE]={
  { "channel",    RSS1_0_NS },
  { "image",      RSS1_0_NS },
  { "textinput",  RSS1_0_NS },
  { "item",       RSS1_0_NS },
  { "author",     ATOM0_3_NS },
  { "skipHours",  RSS0_91_NS },
  { "skipDays",   RSS0_91_NS },
  { "Enclosure",  RSS2_0_ENC_NS }, /* Enclosure class in RDF output */
  { "<unknown>",  RSS_UNKNOWN_NS },
};


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

  RAPTOR_RSS_FIELD_ATOM_CONTENT,   /* atom 0.3 content */
  RAPTOR_RSS_FIELD_ATOM_COPYRIGHT, /* atom 0.3 content */
  RAPTOR_RSS_FIELD_ATOM_CREATED,   /* atom 0.3 created */
  RAPTOR_RSS_FIELD_ATOM_ID,        /* atom 0.3 id */
  RAPTOR_RSS_FIELD_ATOM_ISSUED,    /* atom 0.3 issued */
  RAPTOR_RSS_FIELD_ATOM_LINK,      /* atom 0.3 link */
  RAPTOR_RSS_FIELD_ATOM_MODIFIED,  /* atom 0.3 modified */
  RAPTOR_RSS_FIELD_ATOM_SUMMARY,   /* atom 0.3 summary */
  RAPTOR_RSS_FIELD_ATOM_TAGLINE,   /* atom 0.3 tagline */
  RAPTOR_RSS_FIELD_ATOM_TITLE,     /* atom 0.3 title */

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


  RAPTOR_RSS_FIELD_UNKNOWN,

  RAPTOR_RSS_FIELD_NONE,

  RAPTOR_RSS_FIELDS_SIZE=RAPTOR_RSS_FIELD_UNKNOWN
} raptor_rss_fields_type;


static raptor_rss_info raptor_rss_fields_info[RAPTOR_RSS_FIELDS_SIZE+2]={
  { "title",          RSS1_0_NS },
  { "link",           RSS1_0_NS },
  { "description",    RSS1_0_NS },
  { "url",            RSS1_0_NS },
  { "name",           RSS1_0_NS },
  { "language",       RSS0_91_NS },
  { "rating",         RSS0_91_NS },
  { "copyright",      RSS0_91_NS },
  { "pubDate",        RSS0_91_NS },
  { "lastBuildDate",  RSS0_91_NS },
  { "docs",           RSS0_91_NS },
  { "managingEditor", RSS0_91_NS },
  { "webMaster",      RSS0_91_NS },
  { "cloud",          RSS0_92_NS },
  { "ttl",            RSS2_0_NS },
  { "width",          RSS0_91_NS },
  { "height",         RSS0_91_NS },
  { "hour",           RSS0_91_NS },
  { "day",            RSS0_91_NS },
  { "generator",      RSS0_92_NS },
  { "source",         RSS0_92_NS },
  { "author",         RSS2_0_NS },
  { "guid",           RSS2_0_NS },
  { "enclosure",      RSS2_0_NS },     /* enclosure in RSS */
  { "enclosure",      RSS2_0_ENC_NS }, /* In RDF output, not an RSS field */
  { "url",            RSS2_0_ENC_NS }, /* In RDF output, not an RSS field */
  { "length",         RSS2_0_ENC_NS }, /* In RDF output, not an RSS field */
  { "type",           RSS2_0_ENC_NS }, /* In RDF output, not an RSS field */
  { "length",         RSS2_0_NS },
  { "type",           RSS2_0_NS },
  { "category",       RSS0_92_NS },
  { "comments",       RSS0_92_NS },
  { "items",          RSS1_0_NS },
  { "image",          RSS1_0_NS },
  { "textinput",      RSS1_0_NS },

  { "content",        ATOM0_3_NS },
  { "copyright",      ATOM0_3_NS },
  { "created",        ATOM0_3_NS },
  { "id",             ATOM0_3_NS },
  { "issued",         ATOM0_3_NS },
  { "link",           ATOM0_3_NS },
  { "modified",       ATOM0_3_NS },
  { "summary",        ATOM0_3_NS },
  { "tagline",        ATOM0_3_NS },
  { "title",          ATOM0_3_NS },

  { "title",          DC_NS },
  { "contributor",    DC_NS },
  { "creator",        DC_NS },
  { "publisher",      DC_NS },
  { "subject",        DC_NS },
  { "description",    DC_NS },
  { "date",           DC_NS },
  { "type",           DC_NS },
  { "format",         DC_NS },
  { "identifier",     DC_NS },
  { "language",       DC_NS },
  { "relation",       DC_NS },
  { "source",         DC_NS },
  { "coverage",       DC_NS },
  { "rights",         DC_NS },

  { "<unknown>",      RSS_UNKNOWN_NS },
  { "<none>",         RSS_UNKNOWN_NS }
};


/* Crude and unofficial mappings from atom fields to RSS */
typedef struct {
  raptor_rss_fields_type from;
  raptor_rss_fields_type to;
} raptor_field_pair;

static raptor_field_pair raptor_atom_to_rss[]={
  { RAPTOR_RSS_FIELD_ATOM_CONTENT,  RAPTOR_RSS_FIELD_DESCRIPTION },
  { RAPTOR_RSS_FIELD_ATOM_TITLE,    RAPTOR_RSS_FIELD_TITLE },
#if 0
  { RAPTOR_RSS_FIELD_ATOM_CREATED,  RAPTOR_RSS_FIELD_UNKNOWN },
  { RAPTOR_RSS_FIELD_ATOM_ID,       RAPTOR_RSS_FIELD_UNKNOWN },
  { RAPTOR_RSS_FIELD_ATOM_ISSUED,   RAPTOR_RSS_FIELD_UNKNOWN },
  { RAPTOR_RSS_FIELD_ATOM_LINK,     RAPTOR_RSS_FIELD_UNKNOWN },
  { RAPTOR_RSS_FIELD_ATOM_MODIFIED, RAPTOR_RSS_FIELD_UNKNOWN },
  { RAPTOR_RSS_FIELD_ATOM_SUMMARY,  RAPTOR_RSS_FIELD_UNKNOWN },
  { RAPTOR_RSS_FIELD_ATOM_TAGLINE,  RAPTOR_RSS_FIELD_UNKNOWN },
#endif
  { RAPTOR_RSS_FIELD_UNKNOWN,       RAPTOR_RSS_FIELD_UNKNOWN }
};
  

/* RSS enclosure support */
struct raptor_rss_enclosure_s
{
  raptor_identifier identifier;
  raptor_uri *node_type;
  raptor_uri *url; 
  char *length;
  char *type;
};
typedef struct raptor_rss_enclosure_s raptor_rss_enclosure;


/* RSS items (instances of typed nodes) containing fields */
struct raptor_rss_item_s
{
  raptor_uri *uri;
  raptor_identifier identifier;
  raptor_rss_info *node_type;
  char* fields[RAPTOR_RSS_FIELDS_SIZE];
  raptor_uri* uri_fields[RAPTOR_RSS_FIELDS_SIZE];
  raptor_rss_enclosure* enclosure;
  int fields_count;
  struct raptor_rss_item_s* next;
};
typedef struct raptor_rss_item_s raptor_rss_item;


#define RAPTOR_RSS_N_CONCEPTS 2

#define RAPTOR_RDF_type_URI(rss_parser) ((rss_parser)->concepts[0])
#define RAPTOR_RDF_Seq_URI(rss_parser)  ((rss_parser)->concepts[1])

/*
 * RSS parser object
 */
struct raptor_rss_parser_context_s {
  /* current line */
  char *line;
  /* current line length */
  int line_length;
  /* current char in line buffer */
  int offset;
  
  /* static statement for use in passing to user code */
  raptor_statement statement;

  /* libxml2 2.5.0+ xmlTextReader() - http://xmlsoft.org/xmlreader.html */
  xmlTextReaderPtr reader;
  xmlParserInputBufferPtr input;


  /* RAPTOR_RSS_CHANNEL, RAPTOR_RSS_IMAGE, RAPTOR_RSS_TEXTINPUT */
  raptor_rss_item common[RAPTOR_RSS_COMMON_SIZE];

  /* item count */
  int items_count;
  /* list of items RAPTOR_RSS_ITEM */
  raptor_rss_item* items;
  /* this points to the last one added, so we can append easy */
  raptor_rss_item* last;

  raptor_rss_type current_type;
  /* one place stack */
  raptor_rss_type prev_type;
  raptor_rss_fields_type current_field;

  raptor_uri* concepts[RAPTOR_RSS_N_CONCEPTS];
};


typedef struct raptor_rss_parser_context_s raptor_rss_parser_context;


static int raptor_rss_common_initialised=0;

static void
raptor_rss_common_init(void) {
  int i;
  if(raptor_rss_common_initialised++)
    return;
  
  for(i=0; i<RAPTOR_RSS_NAMESPACES_SIZE;i++) {
    const char *uri_string=raptor_rss_namespaces_info[i].uri_string;
    if(uri_string)
      raptor_rss_namespaces_info[i].uri=raptor_new_uri((const unsigned char*)uri_string);
  }

  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    int n=raptor_rss_types_info[i].nspace;
    raptor_uri *namespace_uri=raptor_rss_namespaces_info[n].uri;
    if(namespace_uri)
      raptor_rss_types_info[i].uri=raptor_new_uri_from_uri_local_name(namespace_uri, (const unsigned char*)raptor_rss_types_info[i].name);
  }

  for(i=0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
    raptor_uri *namespace_uri=raptor_rss_namespaces_info[raptor_rss_fields_info[i].nspace].uri;
    if(namespace_uri)
      raptor_rss_fields_info[i].uri=raptor_new_uri_from_uri_local_name(namespace_uri,
                                                                       (const unsigned char*)raptor_rss_fields_info[i].name);
  }

}


static void
raptor_rss_common_terminate(void) {
  int i;
  if(--raptor_rss_common_initialised)
    return;

  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    if(raptor_rss_types_info[i].uri)
      raptor_free_uri(raptor_rss_types_info[i].uri);
  }

  for(i=0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
    if(raptor_rss_fields_info[i].uri)
      raptor_free_uri(raptor_rss_fields_info[i].uri);
  }

  for(i=0; i<RAPTOR_RSS_NAMESPACES_SIZE;i++) {
    if(raptor_rss_namespaces_info[i].uri)
      raptor_free_uri(raptor_rss_namespaces_info[i].uri);
  }

}


static void
raptor_rss_context_init(raptor_rss_parser_context* rss_parser) {
  rss_parser->items_count=0;
  rss_parser->last=rss_parser->items=NULL;

  rss_parser->current_type=RAPTOR_RSS_NONE;
  rss_parser->prev_type=RAPTOR_RSS_NONE;
  rss_parser->current_field=RAPTOR_RSS_FIELD_NONE;
  
  if(rss_parser->reader) {
    xmlFreeTextReader(rss_parser->reader);
    rss_parser->reader=NULL;
  }

  if(rss_parser->input) {
    xmlFreeParserInputBuffer(rss_parser->input);
    rss_parser->input=NULL;
  }

  RAPTOR_RDF_type_URI(rss_parser)=raptor_new_uri_for_rdf_concept("type");
  RAPTOR_RDF_Seq_URI(rss_parser)=raptor_new_uri_for_rdf_concept("Seq");
}


static void
raptor_enclosure_free(raptor_rss_enclosure* enclosure) {
  if(enclosure->length)
    RAPTOR_FREE(cstring, enclosure->length);
  if(enclosure->type)
    RAPTOR_FREE(cstring, enclosure->type);
  if(enclosure->url)
    raptor_free_uri(enclosure->url);
  raptor_free_identifier(&(enclosure->identifier));
  RAPTOR_FREE(raptor_rss_enclosure, enclosure);
}

static void
raptor_clear_rss_item(raptor_rss_item* item) {
  int i;
  for(i=0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
    if(item->fields[i])
      RAPTOR_FREE(raptor_rss_item, item->fields[i]);
    if(item->uri_fields[i])
      raptor_free_uri(item->uri_fields[i]);
  }
  if(item->enclosure) 
    raptor_enclosure_free(item->enclosure);
  if(item->uri)
    raptor_free_uri(item->uri);
  raptor_free_identifier(&item->identifier);
}


static void
raptor_free_rss_item(raptor_rss_item* item) {
  raptor_clear_rss_item(item);
  RAPTOR_FREE(raptor_rss_item, item);
}

static void
raptor_clear_rss_items(raptor_rss_parser_context *rss_parser) {
  raptor_rss_item* item=rss_parser->items;
  while(item) {
    raptor_rss_item *next=item->next;

    raptor_free_rss_item(item);
    item=next;
  }
  rss_parser->last=rss_parser->items=NULL;
}


static void
raptor_rss_context_terminate(raptor_rss_parser_context* rss_parser) {
  int i;
  
  if(rss_parser->reader)
    xmlFreeTextReader(rss_parser->reader);

  if(rss_parser->input)
    xmlFreeParserInputBuffer(rss_parser->input);

  raptor_clear_rss_items(rss_parser);

  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++)
    raptor_clear_rss_item(&rss_parser->common[i]);

  for(i=0; i< RAPTOR_RSS_N_CONCEPTS; i++) {
    raptor_uri* concept_uri=rss_parser->concepts[i];
    if(concept_uri) {
      raptor_free_uri(concept_uri);
      rss_parser->concepts[i]=NULL;
    }
  }
}


static int
raptor_rss_parse_init(raptor_parser* rdf_parser, const char *name) {
  raptor_rss_common_init();
  xmlSubstituteEntitiesDefault(1);

  return 0;
}

static void
raptor_rss_parse_terminate(raptor_parser *rdf_parser) {
  raptor_rss_parser_context *rss_parser=(raptor_rss_parser_context*)rdf_parser->context;
  raptor_rss_context_terminate(rss_parser);
  raptor_rss_common_terminate();
}


static void
raptor_rss_error_handler(void *arg, 
                         const char *message,
                         xmlParserSeverities severity,
                         xmlTextReaderLocatorPtr xml_locator) 
{
  raptor_parser* rdf_parser=(raptor_parser*)arg;
  raptor_locator *locator=&rdf_parser->locator;

  locator->line= -1;
  locator->column= -1;
  if(arg)
    locator->line= xmlTextReaderLocatorLineNumber(xml_locator);

  raptor_parser_error(rdf_parser, message);
}


static int
raptor_rss_parse_start(raptor_parser *rdf_parser) 
{
  raptor_locator *locator=&rdf_parser->locator;
  raptor_rss_parser_context* rss_parser=(raptor_rss_parser_context*)rdf_parser->context;
  
  locator->line=1;
  locator->column=0;
  locator->byte=0;

  raptor_rss_context_init(rss_parser);
  return 0;
}


static raptor_rss_enclosure*
raptor_rss_new_enclosure(void)
{
  raptor_rss_enclosure* enclosure=(raptor_rss_enclosure*)RAPTOR_CALLOC(raptor_rss_enclosure, 1, sizeof(raptor_rss_enclosure));
  return enclosure;
}


static void
raptor_rss_item_add(raptor_rss_parser_context *rss_parser) {
  raptor_rss_item* item=(raptor_rss_item*)RAPTOR_CALLOC(raptor_rss_item, 1, sizeof(raptor_rss_item));
  item->next=NULL;
  
  /* new list */
  if(!rss_parser->items)
    rss_parser->items=item;
  
  /* join last item to this one */
  if(rss_parser->last)
    rss_parser->last->next=item;
  
  /* this is now the last item */
  rss_parser->last=item;
  rss_parser->items_count++;

  RAPTOR_DEBUG2("Added item %d\n", rss_parser->items_count);
}

#if LIBXML_VERSION < 20509

#define XML_READER_TYPE_ELEMENT 1
#define XML_READER_TYPE_TEXT 3
#define XML_READER_TYPE_CDATA 4
#define XML_READER_TYPE_ENTITY_REFERENCE 5
#define XML_READER_TYPE_ENTITY 6
#define XML_READER_TYPE_PROCESSING_INSTRUCTION 7
#define XML_READER_TYPE_COMMENT 8
#define XML_READER_TYPE_DOCUMENT 9
#define XML_READER_TYPE_DOCUMENT_TYPE 10
#define XML_READER_TYPE_DOCUMENT_FRAGMENT 11
#define XML_READER_TYPE_NOTATION 12
#define XML_READER_TYPE_WHITESPACE 13
#define XML_READER_TYPE_SIGNIFICANT_WHITESPACE 14
#define XML_READER_TYPE_END_ELEMENT 15
#define XML_READER_TYPE_END_ENTITY 16
#define XML_READER_TYPE_XML_DECLARATION 17

#endif



static void
raptor_rss_parser_processNode(raptor_parser *rdf_parser) {
  raptor_rss_parser_context* rss_parser=(raptor_rss_parser_context*)rdf_parser->context;
  xmlTextReaderPtr reader=rss_parser->reader;
  xmlChar *name;
  int free_name=0;
  xmlChar *value;
  int type;
  int is_empty;
  raptor_uri *uri=NULL;
  xmlChar *rel=NULL;
  raptor_rss_enclosure *enclosure=NULL;

#if LIBXML_VERSION > 20511
  name = (xmlChar *)xmlTextReaderConstLocalName(reader);
#else
  name = xmlTextReaderLocalName(reader);
  free_name = 1;
#endif
  if (name == NULL) {
    name = xmlStrdup(BAD_CAST "--");
    free_name = 1;
  }
  value = xmlTextReaderValue(reader);
  
  type=xmlTextReaderNodeType(reader);
  
  switch(type) {
    case XML_READER_TYPE_ELEMENT: /* start element */

      /* Must be checked before moving on to attributes */
      is_empty=xmlTextReaderIsEmptyElement(reader);

      if(rss_parser->current_type==RAPTOR_RSS_NONE) {
        if(!strcmp((const char*)name, "rss") || 
           !strcmp((const char*)name, "rdf") || !strcmp((const char*)name, "RDF")) {
          break;
        } else if(!strcmp((const char*)name, "feed")) {
          rss_parser->current_type=RAPTOR_RSS_CHANNEL;
          break;
        } if(!strcmp((const char*)name, "item") ||
             !strcmp((const char*)name, "entry")) {
          raptor_rss_item_add(rss_parser);
          rss_parser->current_type=RAPTOR_RSS_ITEM;
        } else {
          int i;
          rss_parser->current_type=RAPTOR_RSS_UNKNOWN;
          for(i=0; i<RAPTOR_RSS_COMMON_SIZE; i++)
            if(!strcmp((const char*)name, raptor_rss_types_info[i].name)) {
              rss_parser->current_type=(raptor_rss_type)i;
              break;
            }
        }
        
        if(rss_parser->current_type==RAPTOR_RSS_UNKNOWN) {
          RAPTOR_DEBUG2("Unknown start element named %s\n", name);
        } else {
          RAPTOR_DEBUG3("FOUND type %d - %s\n", rss_parser->current_type, raptor_rss_types_info[rss_parser->current_type].name);
        }
      } else { /* have current_type, this is an element inside */
        int i;
        raptor_rss_type old_type=rss_parser->current_type;
        
        /* check it is not a type here */
        if(!strcmp((const char*)name, "item") ||
           !strcmp((const char*)name, "entry")) {
          raptor_rss_item_add(rss_parser);
          rss_parser->current_type=RAPTOR_RSS_ITEM;
        } else {
          for(i=0; i<RAPTOR_RSS_COMMON_SIZE; i++)
            if(!strcmp((const char*)name, raptor_rss_types_info[i].name)) {
              rss_parser->current_type=(raptor_rss_type)i;
              break;
            }
        }
        
        if(rss_parser->current_type != old_type) {
          RAPTOR_DEBUG6("FOUND element %s for type %d - %s INSIDE current type %d - %s\n", name, rss_parser->current_type, raptor_rss_types_info[rss_parser->current_type].name, old_type, raptor_rss_types_info[old_type].name);
          rss_parser->prev_type=old_type;
          break;
        }
        
        rss_parser->current_field=RAPTOR_RSS_FIELD_UNKNOWN;
        for(i=0; i<RAPTOR_RSS_FIELDS_SIZE; i++)
          if(!strcmp((const char*)name, raptor_rss_fields_info[i].name)) {
#if LIBXML_VERSION > 20511
            const xmlChar *nspace_URI=xmlTextReaderConstNamespaceUri(reader);
#else
            xmlChar *nspace_URI=xmlTextReaderNamespaceUri(reader);
            int free_nspace_URI=1;
#endif
            if(nspace_URI &&
               !strcmp((const char*)nspace_URI, (const char*)raptor_rss_namespaces_info[RSS0_9_NS].uri_string)) {
              nspace_URI=(xmlChar*)raptor_rss_namespaces_info[RSS1_0_NS].uri_string;
#if LIBXML_VERSION > 20511
#else
              free_nspace_URI=0;
#endif
            }
            if(nspace_URI && raptor_rss_fields_info[i].nspace != RSS_NO_NS) {
              const unsigned char *field_nspace_URI=(const unsigned char*)raptor_rss_namespaces_info[raptor_rss_fields_info[i].nspace].uri_string;
            
              if(!strcmp((const char*)nspace_URI, (const char*)field_nspace_URI)) {
                rss_parser->current_field=(raptor_rss_fields_type)i;
#if LIBXML_VERSION > 20511
                /* nop */
#else
                if(free_nspace_URI)
                  xmlFree(nspace_URI);
#endif
                break;
              }
            } else {
              rss_parser->current_field=(raptor_rss_fields_type)i;
#if LIBXML_VERSION > 20511
              /* nop */
#else
              xmlFree(nspace_URI);
#endif
              break;
            }
          }
        
        if(rss_parser->current_field==RAPTOR_RSS_FIELD_UNKNOWN) {
          RAPTOR_DEBUG3("Unknown field element named %s inside type %s\n", name, raptor_rss_types_info[rss_parser->current_type].name);
        } else if (rss_parser->current_field == RAPTOR_RSS_FIELD_ENCLOSURE ){
          raptor_rss_item* update_item;

          RAPTOR_DEBUG1("FOUND new enclosure\n");
          if(rss_parser->current_type == RAPTOR_RSS_ITEM)
            update_item=rss_parser->last;
          else
            update_item=&rss_parser->common[rss_parser->current_type];
          enclosure=raptor_rss_new_enclosure();
          update_item->enclosure=enclosure;
        } else {
          RAPTOR_DEBUG4("FOUND field %d - %s inside type %s\n", rss_parser->current_field, raptor_rss_fields_info[rss_parser->current_field].name, raptor_rss_types_info[rss_parser->current_type].name);

          /* Rewrite item fields */
          for(i=0; raptor_atom_to_rss[i].from != RAPTOR_RSS_FIELD_UNKNOWN; i++) {
            if(raptor_atom_to_rss[i].from == rss_parser->current_field) {
              rss_parser->current_field=raptor_atom_to_rss[i].to;

              RAPTOR_DEBUG3("Rewrote into field %d - %s\n", rss_parser->current_field, raptor_rss_fields_info[rss_parser->current_field].name);
              break;
            }
          }

        }
      }

      /* Now check for attributes */
      while((xmlTextReaderMoveToNextAttribute(reader))) {
#if LIBXML_VERSION > 20511
        const xmlChar *attrName = xmlTextReaderConstLocalName(reader);
#else
        xmlChar *attrName = xmlTextReaderLocalName(reader);
#endif
        xmlChar *attrValue = xmlTextReaderValue(reader);
        RAPTOR_DEBUG3("  attribute %s=%s\n", attrName, attrValue);

        /* Pick a few attributes to care about */
        if(!strcmp((const char*)attrName, "isPermaLink")) {
          raptor_rss_item* update_item=rss_parser->last;
          if(!strcmp((const char*)name, "guid")) {
            /* <guid isPermaLink="..."> */
            if(update_item) {
              if(!strcmp((const char*)attrValue, "true")) {
                raptor_uri* guid_uri=raptor_new_uri((const unsigned char*)attrValue);
                RAPTOR_DEBUG2("    setting guid to URI '%s'\n", attrValue);
                update_item->uri_fields[RAPTOR_RSS_FIELD_GUID]=guid_uri;
              } else {
                RAPTOR_DEBUG2("    setting guid to string '%s'\n", attrValue);
                update_item->fields[RAPTOR_RSS_FIELD_GUID]=(char*)attrValue;
              }
            }
          }
        } else if(!strcmp((const char*)attrName, "url")) {
          if(!strcmp((const char*)name, "source")) {
            /* <source url="...">foo</source> */
            if(rss_parser->last) {
              /*
                rss_parser->last->source_url=attrValue; 
                attrValue=NULL;
               */
            }
          } else if (!strcmp((const char*)name, "enclosure") && enclosure) {
            RAPTOR_DEBUG2("  setting enclosure URL %s\n", attrValue);
            enclosure->url=raptor_new_uri((const unsigned char*)attrValue);
          }
        } else if(!strcmp((const char*)attrName, "domain")) {
          if(!strcmp((const char*)name, "category")) {
            /* <category domain="URL">foo</source> */
            if(rss_parser->last) {
              /*
                rss_parser->last->category_url=attrValue; 
                attrValue=NULL;
               */
            }
          }
        } else if(!strcmp((const char*)attrName, "rel")) {
          rel=attrValue;
          attrValue=NULL;
        } else if(!strcmp((const char*)attrName, "href")) {
          if(!strcmp((const char*)name, "link")) {
            RAPTOR_DEBUG2("  setting href as URI string for type %s\n", raptor_rss_types_info[rss_parser->current_type].name);
            if(uri)
              raptor_free_uri(uri);
            uri=raptor_new_uri((const unsigned char*)attrValue);
          }
        } else if (!strcmp((const char*)attrName, "length")) {
          if (!strcmp((const char*)name, "enclosure") && enclosure) {
            size_t len=strlen((const char*)attrValue);
            RAPTOR_DEBUG2("  setting enclosure length %s\n", attrValue);
            enclosure->length=(char*)RAPTOR_MALLOC(cstring, len+1);
            strncpy(enclosure->length, (char*)attrValue, len+1);
          }
        } else if (!strcmp((const char*)attrName, "type")) {
          if (!strcmp((const char*)name, "enclosure") && enclosure) {
            size_t len=strlen((const char*)attrValue);
            RAPTOR_DEBUG2("  setting enclosure type %s\n", attrValue);
            enclosure->type=(char*)RAPTOR_MALLOC(cstring, len+1);
            strncpy(enclosure->type, (char*)attrValue, len+1);
          }
        }

        if(attrValue)
          xmlFree(attrValue);
#if LIBXML_VERSION > 20511
        /* nop */
#else
        xmlFree(attrName);
#endif
      }
      
      if(!is_empty) {
        if(uri)
          raptor_free_uri(uri);
        if(rel)
          xmlFree(rel);
        break;
      }

      /* Empty element, so consider adding one of the attributes as
       * literal or URI content
       */
      if(rss_parser->current_type >= RAPTOR_RSS_COMMON_IGNORED) {
        /* skipHours, skipDays common but IGNORED */ 
        RAPTOR_DEBUG3("Ignoring empty element %s for type %s\n", name, raptor_rss_types_info[rss_parser->current_type].name);
      } else if(uri && rel && !strcmp((const char*)rel, "alternate")) {
        raptor_rss_item* update_item;

        if(rss_parser->current_type == RAPTOR_RSS_ITEM)
          update_item=rss_parser->last;
        else
          update_item=&rss_parser->common[rss_parser->current_type];
        
        RAPTOR_DEBUG3("Added URI to field %s of type %s\n", raptor_rss_fields_info[rss_parser->current_field].name, raptor_rss_types_info[rss_parser->current_type].name);
        if(!update_item->fields[rss_parser->current_field] &&
           !update_item->uri_fields[rss_parser->current_field])
          update_item->fields_count++;
        update_item->uri_fields[rss_parser->current_field]=uri;
        uri=NULL;
      }
      
      if(uri)
        raptor_free_uri(uri);
      if(rel)
        xmlFree(rel);

      /* FALLTHROUGH if is empty element */

    case XML_READER_TYPE_END_ELEMENT: /* end element */
      if(rss_parser->current_type != RAPTOR_RSS_NONE) {
        if(rss_parser->current_field != RAPTOR_RSS_FIELD_NONE) {
          RAPTOR_DEBUG3("Ending element %s field %s\n", name, raptor_rss_fields_info[rss_parser->current_field].name);
          rss_parser->current_field= RAPTOR_RSS_FIELD_NONE;
        } else {
          RAPTOR_DEBUG3("Ending element %s type %s\n", name, raptor_rss_types_info[rss_parser->current_type].name);
          if(rss_parser->prev_type != RAPTOR_RSS_NONE) {
            rss_parser->current_type=rss_parser->prev_type;
            rss_parser->prev_type=RAPTOR_RSS_NONE;
            RAPTOR_DEBUG3("Returning to type %d - %s\n", rss_parser->current_type, raptor_rss_types_info[rss_parser->current_type].name);
          } else
            rss_parser->current_type= RAPTOR_RSS_NONE;
        }
      }
      
      break;

    case XML_READER_TYPE_TEXT:
    case XML_READER_TYPE_SIGNIFICANT_WHITESPACE: /* FIXME */
    case XML_READER_TYPE_CDATA: /* FIXME */

      if((rss_parser->current_type==RAPTOR_RSS_NONE ||
          rss_parser->current_type==RAPTOR_RSS_UNKNOWN) ||
         (rss_parser->current_field==RAPTOR_RSS_FIELD_NONE ||
          rss_parser->current_field==RAPTOR_RSS_FIELD_UNKNOWN)) {
        char *p=(char*)value;
        while(*p) {
          if(!isspace(*p))
            break;
          p++;
        }
        if(*p)
          RAPTOR_DEBUG4("IGNORING non-whitespace text '%s' inside type %s, field %s\n", 
                        value, raptor_rss_types_info[rss_parser->current_type].name,
                        raptor_rss_fields_info[rss_parser->current_field].name);
        break;
      }

      if(rss_parser->current_type >= RAPTOR_RSS_COMMON_IGNORED) {
        /* skipHours, skipDays common but IGNORED */ 
        RAPTOR_DEBUG2("Ignoring fields for type %s\n", raptor_rss_types_info[rss_parser->current_type].name);
      } else {
        raptor_rss_item* update_item;
        
        if(rss_parser->current_type == RAPTOR_RSS_ITEM)
          update_item=rss_parser->last;
        else
          update_item=&rss_parser->common[rss_parser->current_type];
        
        RAPTOR_DEBUG4("Added text '%s' to field %s of type %s\n", value, raptor_rss_fields_info[rss_parser->current_field].name, raptor_rss_types_info[rss_parser->current_type].name);
        if(!update_item->fields[rss_parser->current_field] &&
           !update_item->uri_fields[rss_parser->current_field])
          update_item->fields_count++;
        update_item->fields[rss_parser->current_field]=(char*)value;
        value=NULL;
      }
      
      break;

    case XML_READER_TYPE_ENTITY_REFERENCE:
    case XML_READER_TYPE_ENTITY:
    case XML_READER_TYPE_PROCESSING_INSTRUCTION:
    case XML_READER_TYPE_COMMENT:
    case XML_READER_TYPE_DOCUMENT:
    case XML_READER_TYPE_DOCUMENT_TYPE:
    case XML_READER_TYPE_DOCUMENT_FRAGMENT:
    case XML_READER_TYPE_NOTATION:
    case XML_READER_TYPE_WHITESPACE: /* FIXME */
    case XML_READER_TYPE_END_ENTITY:
    case XML_READER_TYPE_XML_DECLARATION:
      break;
    
    default:
#if defined(RAPTOR_DEBUG)
      RAPTOR_DEBUG3("depth %d type %d", xmlTextReaderDepth(reader), type);
      fprintf(stderr," name %s %s", name,
              xmlTextReaderIsEmptyElement(reader) ? "Empty" : "");
      if (value == NULL)
        fprintf(stderr, "\n");
      else {
        fprintf(stderr, " '%s'\n", value);
      }
#endif
      RAPTOR_DEBUG2("Ignoring type %d\n", type);
  }
    
  if(value)
    xmlFree(value);

  if(free_name)
    xmlFree(name);

}

static void
raptor_rss_insert_enclosure_identifiers(raptor_parser* rdf_parser, 
                                        raptor_rss_enclosure *enclosure)
{
  raptor_identifier* identifier=&enclosure->identifier;

  identifier->id=raptor_generate_id(rdf_parser, 0, NULL);
  identifier->type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
  identifier->uri_source=RAPTOR_URI_SOURCE_GENERATED;
  enclosure->node_type=raptor_rss_types_info[RAPTOR_RSS_ENCLOSURE].uri;
}

static void
raptor_rss_insert_identifiers(raptor_parser* rdf_parser) 
{
  raptor_rss_parser_context* rss_parser=(raptor_rss_parser_context*)rdf_parser->context;
  int i;
  raptor_rss_item* item;
  
  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    raptor_identifier* identifier;

    item=&rss_parser->common[i];
    identifier=&item->identifier;
    
    if(!item->fields_count)
      continue;
    
    if(item->uri) {
      identifier->uri=raptor_uri_copy(item->uri);
      identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
      identifier->uri_source=RAPTOR_URI_SOURCE_URI;
    } else {
      int url_fields[2];
      int url_fields_count=1;
      int f;
      
      url_fields[0]=(i== RAPTOR_RSS_IMAGE) ? RAPTOR_RSS_FIELD_URL :
                                             RAPTOR_RSS_FIELD_LINK;
      if(i == RAPTOR_RSS_CHANNEL) {
        url_fields[1]=RAPTOR_RSS_FIELD_ATOM_ID;
        url_fields_count++;
      }

      for(f=0; f < url_fields_count; f++) {
        int field=url_fields[f];
        if(item->fields[field]) {
          identifier->uri=raptor_new_uri((const unsigned char*)item->fields[field]);
          identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          identifier->uri_source=RAPTOR_URI_SOURCE_URI;
          break;
        } else if(item->uri_fields[field]) {
          identifier->uri=raptor_uri_copy(item->uri_fields[field]);
          identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
          identifier->uri_source=RAPTOR_URI_SOURCE_URI;
          break;
        }
      }
      
      if(!identifier->uri) {
        /* need to make bnode */
        identifier->id=raptor_generate_id(rdf_parser, 0, NULL);
        identifier->type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
        identifier->uri_source=RAPTOR_URI_SOURCE_GENERATED;
      }
    }

    if(item->enclosure)
      raptor_rss_insert_enclosure_identifiers(rdf_parser, item->enclosure);

    item->node_type=&raptor_rss_types_info[i];
  }

  /* sequence of rss:item */
  for(item=rss_parser->items; item; item=item->next) {
    raptor_identifier* identifier=&item->identifier;
    
    if(item->uri) {
      identifier->uri=raptor_uri_copy(item->uri);
      identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
      identifier->uri_source=RAPTOR_URI_SOURCE_URI;
    } else {
      if(item->fields[RAPTOR_RSS_FIELD_LINK]) {
        identifier->uri=raptor_new_uri((const unsigned char*)item->fields[RAPTOR_RSS_FIELD_LINK]);
        identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
        identifier->uri_source=RAPTOR_URI_SOURCE_URI;
      } else if(item->uri_fields[RAPTOR_RSS_FIELD_LINK]) {
        identifier->uri=raptor_uri_copy(item->uri_fields[RAPTOR_RSS_FIELD_LINK]);
        identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
        identifier->uri_source=RAPTOR_URI_SOURCE_URI;
      } else if(item->fields[RAPTOR_RSS_FIELD_ATOM_LINK]) {
        identifier->uri=raptor_new_uri((const unsigned char*)item->fields[RAPTOR_RSS_FIELD_ATOM_LINK]);
        identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
        identifier->uri_source=RAPTOR_URI_SOURCE_URI;
      } else if(item->uri_fields[RAPTOR_RSS_FIELD_ATOM_LINK]) {
        identifier->uri=raptor_uri_copy(item->uri_fields[RAPTOR_RSS_FIELD_ATOM_LINK]);
        identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
        identifier->uri_source=RAPTOR_URI_SOURCE_URI;
      } else {
        /* need to make bnode */
        identifier->id=raptor_generate_id(rdf_parser, 0, NULL);
        identifier->type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
        identifier->uri_source=RAPTOR_URI_SOURCE_GENERATED;
      }
    }

    if(item->enclosure)
      raptor_rss_insert_enclosure_identifiers(rdf_parser, item->enclosure);

    item->node_type=&raptor_rss_types_info[RAPTOR_RSS_ITEM];
  }

}


static void
raptor_rss_emit_type_triple(raptor_parser* rdf_parser, 
                            raptor_identifier *resource,
                            raptor_uri *type_uri) 
{
  raptor_rss_parser_context* rss_parser=(raptor_rss_parser_context*)rdf_parser->context;

  rss_parser->statement.subject=resource->uri ? (void*)resource->uri : (void*)resource->id;
  rss_parser->statement.subject_type=resource->type;
  
  rss_parser->statement.predicate=RAPTOR_RDF_type_URI(rss_parser);
  rss_parser->statement.predicate_type=RAPTOR_IDENTIFIER_TYPE_PREDICATE;
  
  rss_parser->statement.object=(void*)type_uri;
  rss_parser->statement.object_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
  
  /* Generate the statement */
  (*rdf_parser->statement_handler)(rdf_parser->user_data, &rss_parser->statement);

}

static void
raptor_rss_emit_enclosure(raptor_parser* rdf_parser, 
                          raptor_rss_enclosure *enclosure)
{
  raptor_rss_parser_context* rss_parser=(raptor_rss_parser_context*)rdf_parser->context;
  raptor_identifier* identifier=&enclosure->identifier;

  rss_parser->statement.predicate=raptor_rss_fields_info[RAPTOR_RSS_RDF_ENCLOSURE].uri;
  rss_parser->statement.object=identifier->id;
  rss_parser->statement.object_type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
  (*rdf_parser->statement_handler)(rdf_parser->user_data, &rss_parser->statement);

  raptor_rss_emit_type_triple(rdf_parser, identifier, enclosure->node_type);

  if (enclosure->url) {
    rss_parser->statement.predicate=raptor_rss_fields_info[RAPTOR_RSS_RDF_ENCLOSURE_URL].uri;
    rss_parser->statement.object=enclosure->url;
    rss_parser->statement.object_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
    (*rdf_parser->statement_handler)(rdf_parser->user_data, &rss_parser->statement);
  }

  if (enclosure->type) {
    rss_parser->statement.predicate=raptor_rss_fields_info[RAPTOR_RSS_RDF_ENCLOSURE_TYPE].uri;
    rss_parser->statement.object=enclosure->type;
    rss_parser->statement.object_type=RAPTOR_IDENTIFIER_TYPE_LITERAL;
    (*rdf_parser->statement_handler)(rdf_parser->user_data, &rss_parser->statement);
  }

  if (enclosure->length) {
    rss_parser->statement.predicate=raptor_rss_fields_info[RAPTOR_RSS_RDF_ENCLOSURE_LENGTH].uri;
    rss_parser->statement.object=enclosure->length;
    rss_parser->statement.object_type=RAPTOR_IDENTIFIER_TYPE_LITERAL;
    (*rdf_parser->statement_handler)(rdf_parser->user_data, &rss_parser->statement);
  }

  rss_parser->statement.subject=enclosure->identifier.id;
}


static void
raptor_rss_emit_item(raptor_parser* rdf_parser, raptor_rss_item *item) 
{
  raptor_rss_parser_context* rss_parser=(raptor_rss_parser_context*)rdf_parser->context;
  int f;
  raptor_identifier* identifier=&item->identifier;
    
  if(!item->fields_count)
    return;

  raptor_rss_emit_type_triple(rdf_parser, identifier, item->node_type->uri);

  for(f=0; f< RAPTOR_RSS_FIELDS_SIZE; f++) {
    if(!item->fields[f] && !item->uri_fields[f])
      continue;
    
    rss_parser->statement.predicate=raptor_rss_fields_info[f].uri;
    if(!rss_parser->statement.predicate)
      continue;
    
    if(item->fields[f]) {
      rss_parser->statement.object=item->fields[f];
      rss_parser->statement.object_type=RAPTOR_IDENTIFIER_TYPE_LITERAL;
    } else {
      rss_parser->statement.object=item->uri_fields[f];
      rss_parser->statement.object_type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
    }
    
    /* Generate the statement */
    (*rdf_parser->statement_handler)(rdf_parser->user_data, &rss_parser->statement);
  }

  if(item->enclosure)
    raptor_rss_emit_enclosure(rdf_parser, item->enclosure);
}


static void
raptor_rss_emit_connection(raptor_parser* rdf_parser,
                           raptor_identifier *subject_identifier,
                           raptor_uri predicate_uri, int predicate_ordinal,
                           raptor_identifier *object_identifier) 
{
  raptor_rss_parser_context* rss_parser=(raptor_rss_parser_context*)rdf_parser->context;

  rss_parser->statement.subject=subject_identifier->uri ? (void*)subject_identifier->uri : (void*)subject_identifier->id;
  rss_parser->statement.subject_type=subject_identifier->type;

  if(predicate_uri) {
    rss_parser->statement.predicate=predicate_uri;
    rss_parser->statement.predicate_type=RAPTOR_IDENTIFIER_TYPE_PREDICATE;
  } else {
    rss_parser->statement.predicate=(void*)&predicate_ordinal;
    rss_parser->statement.predicate_type=RAPTOR_IDENTIFIER_TYPE_ORDINAL;
  }
  
  
  rss_parser->statement.object=object_identifier->uri ? (void*)object_identifier->uri : (void*)object_identifier->id;
  rss_parser->statement.object_type=object_identifier->type;
  
  /* Generate the statement */
  (*rdf_parser->statement_handler)(rdf_parser->user_data, &rss_parser->statement);

}



static void
raptor_rss_emit(raptor_parser* rdf_parser) 
{
  raptor_rss_parser_context* rss_parser=(raptor_rss_parser_context*)rdf_parser->context;
  int i;
  raptor_rss_item* item;

  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    if(!rss_parser->common[i].fields_count)
      continue;

    RAPTOR_DEBUG3("Emitting type %i - %s\n", i, raptor_rss_types_info[i].name);

    raptor_rss_emit_item(rdf_parser, &rss_parser->common[i]);

    /* Add connections to channel */
    if(i != RAPTOR_RSS_CHANNEL) {
      raptor_rss_emit_connection(rdf_parser,
                                 &rss_parser->common[RAPTOR_RSS_CHANNEL].identifier,
                                 raptor_rss_types_info[i].uri, 0,
                                 &rss_parser->common[i].identifier);
    }
  }

  if(rss_parser->items_count) {
    raptor_identifier *items;

    /* make a new genid for the <rdf:Seq> node */
    items=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_ANONYMOUS,
                                NULL, RAPTOR_URI_SOURCE_GENERATED,
                                (const unsigned char*)raptor_generate_id(rdf_parser, 0, NULL),
                                NULL, NULL, NULL);
  
    /* _:genid1 rdf:type rdf:Seq . */
    raptor_rss_emit_type_triple(rdf_parser, items, RAPTOR_RDF_Seq_URI(rss_parser));

    /* <channelURI> rss:items _:genid1 . */
    raptor_rss_emit_connection(rdf_parser,
                               &rss_parser->common[RAPTOR_RSS_CHANNEL].identifier,
                               raptor_rss_fields_info[RAPTOR_RSS_FIELD_ITEMS].uri, 0,
                               items);
    
    /* sequence of rss:item */
    for(i=1, item=rss_parser->items; item; item=item->next, i++) {
      
      raptor_rss_emit_item(rdf_parser, item);
      raptor_rss_emit_connection(rdf_parser,
                                 items,
                                 NULL, i,
                                 &item->identifier);
    }

    raptor_free_identifier(items);
  }
  
}


static int
raptor_rss_parse_chunk(raptor_parser* rdf_parser, 
                       const unsigned char *s, size_t len,
                       int is_end)
{
  raptor_rss_parser_context* rss_parser=(raptor_rss_parser_context*)rdf_parser->context;
  int ret;
  
  if(!rss_parser->reader) {
    unsigned char *uri=raptor_uri_as_string(rdf_parser->base_uri);

    rss_parser->input=xmlParserInputBufferCreateMem((const char*)s, len,
                                                    XML_CHAR_ENCODING_NONE);
    rss_parser->reader=xmlNewTextReader(rss_parser->input, (const char*)uri);
    
    xmlTextReaderSetErrorHandler(rss_parser->reader,
                                 raptor_rss_error_handler, 
                                 rdf_parser);
  } else if(s && len)
    xmlParserInputBufferPush(rss_parser->input, len, (const char*)s);

  if(!is_end)
    return 0;
  
  ret = xmlTextReaderRead(rss_parser->reader);
  while (ret == 1) {
    if(rdf_parser->failed)
      break;
    
    raptor_rss_parser_processNode(rdf_parser);
    ret = xmlTextReaderRead(rss_parser->reader);
  }

  xmlFreeTextReader(rss_parser->reader);
  rss_parser->reader=NULL;
  
  xmlFreeParserInputBuffer(rss_parser->input);
  rss_parser->input=NULL;

  if(rdf_parser->failed)
    return 1;

  /* turn strings into URIs, move things around if needed */
  raptor_rss_insert_identifiers(rdf_parser);
  
  /* generate the triples */
  raptor_rss_emit(rdf_parser);

  return (ret != 0);
}


static int
raptor_rss_parse_recognise_syntax(raptor_parser_factory* factory, 
                                  const unsigned char *buffer, size_t len,
                                  const unsigned char *identifier, 
                                  const unsigned char *suffix, 
                                  const char *mime_type)
{
  int score= 0;
  
  if(suffix) {
    if(!strcmp((const char*)suffix, "rss"))
      score=7;
    if(!strcmp((const char*)suffix, "atom"))
      score=5;
    if(!strcmp((const char*)suffix, "xml"))
      score=4;
  }
  
  if(identifier) {
    if(strstr((const char*)identifier, "rss2"))
      score+=5;
    else if(!suffix && strstr((const char*)identifier, "rss"))
      score+=4;
    else if(!suffix && strstr((const char*)identifier, "atom"))
      score+=4;
    else if(strstr((const char*)identifier, "rss.xml"))
      score+=4;
    else if(strstr((const char*)identifier, "atom.xml"))
      score+=4;
  }
  
  return score;
}


static void
raptor_rss_parser_register_factory(raptor_parser_factory *factory) 
{
  factory->context_length     = sizeof(raptor_rss_parser_context);
  
  factory->init      = raptor_rss_parse_init;
  factory->terminate = raptor_rss_parse_terminate;
  factory->start     = raptor_rss_parse_start;
  factory->chunk     = raptor_rss_parse_chunk;
  factory->recognise_syntax = raptor_rss_parse_recognise_syntax;
}


void
raptor_init_parser_rss (void) {
  raptor_parser_register_factory("rss-tag-soup",  "RSS Tag Soup",
                                 NULL, NULL,
                                 NULL,
                                 &raptor_rss_parser_register_factory);
}


/*
 * Raptor 'RSS 1.0' serializer object
 */
typedef struct {
  raptor_rss_parser_context parser;

  /* Triples with no assigned type node */
  raptor_sequence *triples;

  /* URIs of rdf:Seq items rdf:_<n> at offset n */
  raptor_sequence *items;

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
  raptor_rss_context_init(&rss_serializer->parser);

  rss_serializer->triples=raptor_new_sequence((raptor_sequence_free_handler*)raptor_free_statement, (raptor_sequence_print_handler*)raptor_print_statement);

  rss_serializer->items=raptor_new_sequence((raptor_sequence_free_handler*)raptor_free_rss_item, (raptor_sequence_print_handler*)NULL);

  return 0;
}
  

/* destroy a serializer */
static void
raptor_rss10_serialize_terminate(raptor_serializer* serializer)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  int i;
  
  raptor_rss_context_terminate(&rss_serializer->parser);
  raptor_rss_common_terminate();

  if(rss_serializer->triples)
    raptor_free_sequence(rss_serializer->triples);

  if(rss_serializer->items)
    raptor_free_sequence(rss_serializer->items);

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
      int field;

      for(field=0; field < RAPTOR_RSS_FIELDS_SIZE; field++) {
        if(!raptor_rss_fields_info[field].uri)
          continue;

        if((s->predicate_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE ||
            s->predicate_type == RAPTOR_IDENTIFIER_TYPE_PREDICATE) &&
           raptor_uri_equals((raptor_uri*)s->predicate,
                             raptor_rss_fields_info[field].uri)) {
          /* found field this triple to go in 'item' so move the
           * object value over 
           */
          if(s->object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE) {
            item->uri_fields[field]=(raptor_uri*)s->object;
            s->object=NULL;
          } else {
            item->fields[field]=(char*)s->object;
            s->object=NULL;
          }
          item->fields_count++;
          break;
        }
      }
      
      if(field < RAPTOR_RSS_FIELDS_SIZE) {
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
  raptor_rss_parser_context* rss_parser=&rss_serializer->parser;
  raptor_rss_item *item=NULL;
  int type;
  int handled=0;

  for(type=0; type< RAPTOR_RSS_COMMON_SIZE; type++) {
    raptor_uri *item_uri=(&rss_parser->common[type])->uri;
    if(item_uri && 
       raptor_uri_equals((raptor_uri*)s->subject, item_uri)) {
      item=&rss_parser->common[type];
      break;
    }
  }

  if(!item) {
    int i;
    
    for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
      item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
      if(item->uri &&
         raptor_uri_equals((raptor_uri*)s->subject, item->uri))
        break;
      
    }
    if(i < raptor_sequence_size(rss_serializer->items))
      type=RAPTOR_RSS_ITEM;
    else
      item=NULL;
  }
  

  if(item) {
    int field;

    for(field=0; field < RAPTOR_RSS_FIELDS_SIZE; field++) {
      if(!raptor_rss_fields_info[field].uri)
        continue;

      if((s->predicate_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE ||
          s->predicate_type == RAPTOR_IDENTIFIER_TYPE_PREDICATE) &&
         raptor_uri_equals((raptor_uri*)s->predicate,
                           raptor_rss_fields_info[field].uri)) {
        /* found field this triple to go in 'item' so move the
         * object value over 
         */
        if(s->object_type == RAPTOR_IDENTIFIER_TYPE_RESOURCE) {
          item->uri_fields[field]=(raptor_uri*)s->object;
          s->object=NULL;
        } else {
          item->fields[field]=(char*)s->object;
          s->object=NULL;
        }
        raptor_free_statement(s);
        RAPTOR_DEBUG3("Stored statement under typed node %i - %s\n",
                      type, raptor_rss_types_info[type].name);
        item->fields_count++;
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
  raptor_rss_parser_context* rss_parser=&rss_serializer->parser;
  int handled=0;
  
  if(raptor_uri_equals((raptor_uri*)statement->predicate, 
                       RAPTOR_RDF_type_URI(rss_parser))) {

    if(raptor_uri_equals((raptor_uri*)statement->object, 
                         RAPTOR_RDF_Seq_URI(rss_parser))) {

      /* triple (?resource rdf:type rdf:Seq) */
      RAPTOR_DEBUG2("Saw rdf:Seq with URI <%s<\n",
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
        } else
          item=&rss_parser->common[type];

        if(item) {
          raptor_identifier* identifier=&item->identifier;

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
  raptor_rss_parser_context* rss_parser=&rss_serializer->parser;
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

  rss_parser->items_count=raptor_sequence_size(rss_serializer->items);
}


static void
raptor_rss10_build_xml_names(raptor_serializer *serializer)
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_rss_parser_context* rss_parser=&rss_serializer->parser;
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
    if(uri) {
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
    raptor_rss_item* item=&rss_parser->common[i];
    if(!item->fields_count)
      continue;

    item->node_type=&raptor_rss_types_info[i];
  }

  for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
    raptor_rss_item* item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
    item->node_type=&raptor_rss_types_info[RAPTOR_RSS_ITEM];
  }
}


static void
raptor_rss10_emit_item(raptor_serializer* serializer,
                       raptor_rss_item *item) 
{
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_xml_writer* xml_writer=rss_serializer->xml_writer;
  raptor_uri *base_uri=serializer->base_uri;
  raptor_xml_element *element;
  raptor_qname **attrs;
  int f;
    
  if(!item->fields_count)
    return;

  if(base_uri)
    base_uri=raptor_uri_copy(base_uri);
  element=raptor_new_xml_element(raptor_qname_copy(item->node_type->qname), NULL, base_uri);
  attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname*));
  attrs[0]=raptor_new_qname_from_namespace_local_name(rss_serializer->rdf_nspace, (const unsigned char*)"about",  raptor_uri_as_string(item->uri));
  raptor_xml_element_set_attributes(element, attrs, 1);

  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"  ", 2);
  raptor_xml_writer_start_element(xml_writer, element);
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);

  for(f=0; f< RAPTOR_RSS_FIELDS_SIZE; f++) {
    raptor_xml_element* predicate;
    if(!item->fields[f] && !item->uri_fields[f])
      continue;

    if(f == RAPTOR_RSS_FIELD_ITEMS)
      /* Done after loop */
      continue;
    
    if(!raptor_rss_fields_info[f].uri)
      continue;
    
    if(base_uri)
      base_uri=raptor_uri_copy(base_uri);
    predicate=raptor_new_xml_element(raptor_qname_copy(raptor_rss_fields_info[f].qname), NULL, base_uri);

    if(item->fields[f]) {
      raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"    ", 4);
      raptor_xml_writer_start_element(xml_writer, predicate);
      raptor_xml_writer_cdata(xml_writer, (const unsigned char*)item->fields[f]);
      raptor_xml_writer_end_element(xml_writer, predicate);
    } else {
      attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname*));
      attrs[0]=raptor_new_qname_from_namespace_local_name(rss_serializer->rdf_nspace, (const unsigned char*)"resource",  raptor_uri_as_string(item->uri_fields[f]));
      raptor_xml_element_set_attributes(predicate, attrs, 1);

      raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"    ", 4);
      raptor_xml_writer_empty_element(xml_writer, predicate);
    }
    raptor_free_xml_element(predicate);

    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
  }


  if(item->fields[RAPTOR_RSS_FIELD_ITEMS] ||
     item->uri_fields[RAPTOR_RSS_FIELD_ITEMS]) {
    raptor_xml_element* rss_items_predicate;
    int i;
    raptor_qname *rdf_Seq_qname;
    raptor_xml_element *rdf_Seq_element;
    
    rdf_Seq_qname=raptor_new_qname_from_namespace_local_name(rss_serializer->rdf_nspace, (const unsigned char*)"Seq",  NULL);
    if(base_uri)
      base_uri=raptor_uri_copy(base_uri);
    rdf_Seq_element=raptor_new_xml_element(rdf_Seq_qname, NULL, base_uri);

    /* make the <rss:items><rdf:Seq><rdf:li /> .... </rdf:Seq></rss:items> */

    if(base_uri)
      base_uri=raptor_uri_copy(base_uri);
    rss_items_predicate=raptor_new_xml_element(raptor_qname_copy(raptor_rss_fields_info[RAPTOR_RSS_FIELD_ITEMS].qname), NULL, base_uri);

    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"    ", 4);
    raptor_xml_writer_start_element(xml_writer, rss_items_predicate);
    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);

    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"      ", 6);
    raptor_xml_writer_start_element(xml_writer, rdf_Seq_element);
    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
    
    for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
      raptor_rss_item* item_item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
      raptor_qname *rdf_li_qname;
      raptor_xml_element *rdf_li_element;
      
      rdf_li_qname=raptor_new_qname_from_namespace_local_name(rss_serializer->rdf_nspace, (const unsigned char*)"li",  NULL);
      if(base_uri)
        base_uri=raptor_uri_copy(base_uri);
      rdf_li_element=raptor_new_xml_element(rdf_li_qname, NULL, base_uri);
      attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname*));
      attrs[0]=raptor_new_qname_from_namespace_local_name(rss_serializer->rdf_nspace, (const unsigned char*)"resource",  raptor_uri_as_string(item_item->uri));
      raptor_xml_element_set_attributes(rdf_li_element, attrs, 1);
      
      raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"         ", 8);
      raptor_xml_writer_empty_element(xml_writer, rdf_li_element);
      raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
      
      raptor_free_xml_element(rdf_li_element);
    }
    
    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"      ", 6);
    raptor_xml_writer_end_element(xml_writer, rdf_Seq_element);
    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
    
    raptor_free_xml_element(rdf_Seq_element);

    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"    ", 4);
    raptor_xml_writer_end_element(xml_writer, rss_items_predicate);

    raptor_free_xml_element(rss_items_predicate);

    raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
  }


  /*
  if(item->enclosure)
    raptor_rss10_emit_enclosure(rss_serializer, item->enclosure);
  */


  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"  ", 2);
  raptor_xml_writer_end_element(xml_writer, element);
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);

  raptor_free_xml_element(element);

  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"\n", 1);
}


static int
raptor_rss10_serialize_end(raptor_serializer* serializer) {
  raptor_rss10_serializer_context *rss_serializer=(raptor_rss10_serializer_context*)serializer->context;
  raptor_rss_parser_context* rss_parser=&rss_serializer->parser;
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
  raptor_rss10_emit_item(serializer, &rss_parser->common[i]);


  if(rss_parser->items_count) {
    for(i=0; i < raptor_sequence_size(rss_serializer->items); i++) {
      raptor_rss_item* item=(raptor_rss_item*)raptor_sequence_get_at(rss_serializer->items, i);
      raptor_rss10_emit_item(serializer, item);
    }

  }

  for(i=RAPTOR_RSS_CHANNEL; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    if(!rss_parser->common[i].fields_count)
      continue;

    if(i != RAPTOR_RSS_CHANNEL) {
      RAPTOR_DEBUG3("Emitting type %i - %s\n", i, raptor_rss_types_info[i].name);
      raptor_rss10_emit_item(serializer, &rss_parser->common[i]);
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


/* end HAVE_LIBXML_XMLREADER_H */
#endif
