/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_rss.c - Raptor RSS tag soup parser
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
 */


#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#ifdef WIN32
#include <win32_config.h>
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

  /* also common, but IGNORED */
  RAPTOR_RSS_SKIPHOURS,
  RAPTOR_RSS_SKIPDAYS,

  /* list items */
  RAPTOR_RSS_ITEM,

  /* unknown name found */
  RAPTOR_RSS_UNKNOWN,

  /* nothing found yet */
  RAPTOR_RSS_NONE,

  /* deliberately not counting NONE */
  RAPTOR_RSS_COMMON_SIZE=RAPTOR_RSS_NONE-RAPTOR_RSS_CHANNEL,
  RAPTOR_RSS_COMMON_IGNORED=RAPTOR_RSS_SKIPHOURS
} raptor_rss_type;


#define RSS1_0_NAMESPACE_URI  "http://purl.org/rss/1.0/"
#define RSS0_91_NAMESPACE_URI "http://purl.org/rss/1.0/modules/rss091#"

typedef enum {
  RSS_UNKNOWN_NS = 0,
  RSS_NO_NS      = 1,
  RSS0_91_NS     = 2,
  RSS0_92_NS     = RSS_NO_NS,
  RSS2_0_NS      = RSS_NO_NS,
  RSS1_0_NS      = 4,

  RSS_NAMESPACES_SIZE = RSS1_0_NS+1
} rss_info_namespace;


static const char* const rss_namespace_uri_strings[RSS_NAMESPACES_SIZE]={
  NULL,
  NULL,
  RSS0_91_NAMESPACE_URI,
  NULL,
  RSS1_0_NAMESPACE_URI
};


typedef struct {
  const char* name;
  rss_info_namespace nspace;
  raptor_uri* uri;
} raptor_rss_info;

raptor_rss_info raptor_rss_types_info[RAPTOR_RSS_COMMON_SIZE]={
  { "channel",    RSS1_0_NS },
  { "image",      RSS1_0_NS },
  { "textinput",  RSS1_0_NS },
  { "skipHours",  RSS0_91_NS },
  { "skipDays",   RSS0_91_NS },
  { "item",       RSS1_0_NS },
  { "<unknown>",  RSS_UNKNOWN_NS },
};


/* Fields of channel, image, textinput, skipHours, skipDays, item */
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
  RAPTOR_RSS_FIELD_CATEGORY,      /* item 0.92, 2.0, channel 2.0 */
  RAPTOR_RSS_FIELD_COMMENTS,      /* comments v? */
  RAPTOR_RSS_FIELD_ITEMS,         /* rss 1.0 items */

  RAPTOR_RSS_FIELD_UNKNOWN,

  RAPTOR_RSS_FIELD_NONE,

  RAPTOR_RSS_FIELDS_SIZE=RAPTOR_RSS_FIELD_NONE
} raptor_rss_fields_type;


raptor_rss_info raptor_rss_fields_info[RAPTOR_RSS_FIELDS_SIZE]={
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
  { "enclosure",      RSS0_92_NS },
  { "category",       RSS0_92_NS },
  { "comments",       RSS0_92_NS },
  { "items",          RSS1_0_NS },
  { "<unknown>",      RSS_UNKNOWN_NS },
};


struct raptor_rss_item_s
{
  char *uri_string;
  raptor_identifier identifier;
  raptor_uri *node_type;
  char* fields[RAPTOR_RSS_FIELDS_SIZE];
  int fields_count;
  struct raptor_rss_item_s* next;
};
typedef struct raptor_rss_item_s raptor_rss_item;


#define RAPTOR_RSS_N_CONCEPTS 2

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

  raptor_uri* namespace_uris[RSS_NAMESPACES_SIZE];

  raptor_uri* concepts[RAPTOR_RSS_N_CONCEPTS];
};


typedef struct raptor_rss_parser_context_s raptor_rss_parser_context;


#define RAPTOR_RDF_type_URI(rss_parser) rss_parser->concepts[0]
#define RAPTOR_RDF_Seq_URI(rss_parser)  rss_parser->concepts[1]

static int
raptor_rss_parse_init(raptor_parser* rdf_parser, const char *name) {
  raptor_rss_parser_context *rss_parser=(raptor_rss_parser_context*)rdf_parser->context;
  int i;
  
  RAPTOR_RDF_type_URI(rss_parser)=raptor_new_uri_for_rdf_concept("type");
  RAPTOR_RDF_Seq_URI(rss_parser)=raptor_new_uri_for_rdf_concept("Seq");

  for(i=0; i<RSS_NAMESPACES_SIZE;i++) {
    const char *uri_string=rss_namespace_uri_strings[i];
    if(uri_string)
      rss_parser->namespace_uris[i]=raptor_new_uri(uri_string);
  }

  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    raptor_uri *namespace_uri=rss_parser->namespace_uris[raptor_rss_types_info[i].nspace];
    if(namespace_uri)
      raptor_rss_types_info[i].uri=raptor_new_uri_from_uri_local_name(namespace_uri,
                                                                     raptor_rss_types_info[i].name);
  }

  for(i=0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
    raptor_uri *namespace_uri=rss_parser->namespace_uris[raptor_rss_fields_info[i].nspace];
    if(namespace_uri)
      raptor_rss_fields_info[i].uri=raptor_new_uri_from_uri_local_name(namespace_uri,
                                                                     raptor_rss_fields_info[i].name);
  }

  xmlSubstituteEntitiesDefault(1);

  return 0;
}


static void
raptor_item_free(raptor_rss_item* item) {
  int i;
  for(i=0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
    if(item->fields[i])
      RAPTOR_FREE(raptor_rss_item, item->fields[i]);
  }
  /* don't free node_type - it is always a shared pointer to a URI */
  raptor_free_identifier(&item->identifier);
}


static void
raptor_rss_items_free(raptor_rss_parser_context *rss_parser) {
  raptor_rss_item* item=rss_parser->items;
  while(item) {
    raptor_rss_item *next=item->next;

    raptor_item_free(item);
    RAPTOR_FREE(raptor_rss_item, item);
    item=next;
  }
  rss_parser->last=rss_parser->items=NULL;
}


static void
raptor_rss_parse_terminate(raptor_parser *rdf_parser) {
  raptor_rss_parser_context *rss_parser=(raptor_rss_parser_context*)rdf_parser->context;
  int i;
  
  if(rss_parser->reader)
    xmlFreeTextReader(rss_parser->reader);

  if(rss_parser->input)
    xmlFreeParserInputBuffer(rss_parser->input);

  raptor_rss_items_free(rss_parser);

  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++)
    raptor_item_free(&rss_parser->common[i]);

  for(i=0; i<RSS_NAMESPACES_SIZE;i++) {
    if(rss_parser->namespace_uris[i])
      raptor_free_uri(rss_parser->namespace_uris[i]);
  }

  for(i=0; i< RAPTOR_RSS_N_CONCEPTS; i++) {
    raptor_uri* concept_uri=rss_parser->concepts[i];
    if(concept_uri) {
      raptor_free_uri(concept_uri);
      rss_parser->concepts[i]=NULL;
    }
  }

  for(i=0; i< RAPTOR_RSS_COMMON_SIZE; i++) {
    if(raptor_rss_types_info[i].uri)
      raptor_free_uri(raptor_rss_types_info[i].uri);
  }

  for(i=0; i< RAPTOR_RSS_FIELDS_SIZE; i++) {
    if(raptor_rss_fields_info[i].uri)
      raptor_free_uri(raptor_rss_fields_info[i].uri);
  }

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

  return 0;
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

  RAPTOR_DEBUG2(raptor_rss_item_add, "Added item %d\n", rss_parser->items_count);
}


static void
raptor_rss_parser_processNode(raptor_parser *rdf_parser) {
  raptor_rss_parser_context* rss_parser=(raptor_rss_parser_context*)rdf_parser->context;
  xmlTextReaderPtr reader=rss_parser->reader;
  xmlChar *name, *value;
  int type;
  
  name = xmlTextReaderName(reader);
  if (name == NULL)
    name = xmlStrdup(BAD_CAST "--");
  value = xmlTextReaderValue(reader);
  
  type=xmlTextReaderNodeType(reader);
  
  switch(type) {
    case 1: /* start element */
      if(rss_parser->current_type==RAPTOR_RSS_NONE) {
        if(!strcmp(name, "rss") || 
           !strcmp(name, "rdf") || !strcmp(name, "RDF")) {
          break;
        } if(!strcmp(name, "item")) {
          raptor_rss_item_add(rss_parser);
          rss_parser->current_type=RAPTOR_RSS_ITEM;
        } else {
          int i;
          rss_parser->current_type=RAPTOR_RSS_UNKNOWN;
          for(i=0; i<RAPTOR_RSS_COMMON_SIZE; i++)
            if(!strcmp(name, raptor_rss_types_info[i].name)) {
              rss_parser->current_type=i;
              break;
            }
        }
        
        if(rss_parser->current_type==RAPTOR_RSS_UNKNOWN) {
          RAPTOR_DEBUG2(raptor_rss_parser_processNode,
                        "Unknown start element named %s\n", name);
        } else {
          RAPTOR_DEBUG3(raptor_rss_parser_processNode,
                        "FOUND type %d - %s\n", rss_parser->current_type,
                        raptor_rss_types_info[rss_parser->current_type].name);
        }
      } else { /* have current_type, this is an element inside */
        int i;
        raptor_rss_type old_type=rss_parser->current_type;
        
        /* check it is not a type here */
        if(!strcmp(name, "item")) {
          raptor_rss_item_add(rss_parser);
          rss_parser->current_type=RAPTOR_RSS_ITEM;
        } else {
          for(i=0; i<RAPTOR_RSS_COMMON_SIZE; i++)
            if(!strcmp(name, raptor_rss_types_info[i].name)) {
              rss_parser->current_type=i;
              break;
            }
        }
        
        if(rss_parser->current_type != old_type) {
          RAPTOR_DEBUG6(raptor_rss_parser_processNode,
                        "FOUND element %s for type %d - %s INSIDE current type %d - %s\n", 
                        name,
                        rss_parser->current_type,
                        raptor_rss_types_info[rss_parser->current_type].name,
                        old_type, raptor_rss_types_info[old_type].name);
          rss_parser->prev_type=old_type;
          break;
        }
        
        rss_parser->current_field=RAPTOR_RSS_FIELD_UNKNOWN;
        for(i=0; i<RAPTOR_RSS_FIELDS_SIZE; i++)
          if(!strcmp(name, raptor_rss_fields_info[i].name)) {
            rss_parser->current_field=i;
            break;
          }
        
        if(rss_parser->current_field==RAPTOR_RSS_FIELD_UNKNOWN) {
          RAPTOR_DEBUG3(raptor_rss_parser_processNode,
                        "Unknown field element named %s inside type %s\n", name,
                        raptor_rss_types_info[rss_parser->current_type].name);
        } else {
          RAPTOR_DEBUG4(raptor_rss_parser_processNode,
                        "FOUND field %d - %s inside type %s\n",
                        rss_parser->current_field,
                        raptor_rss_fields_info[rss_parser->current_field].name,
                        raptor_rss_types_info[rss_parser->current_type].name);
        }
      }

      /* Now check for attributes */
      while((xmlTextReaderMoveToNextAttribute(reader))) {
        xmlChar *attrName = xmlTextReaderName(reader);
        xmlChar *attrValue = xmlTextReaderValue(reader);
        RAPTOR_DEBUG3(raptor_rss_parser_processNode, "  attribute %s=%s\n", 
                      attrName, attrValue);

        /* Pick a few attributes to care about */
        if(!strcmp(attrName, "isPermaLink")) {
          if(!strcmp(name, "guid")) {
            /* <guid isPermaLink="..."> */
            if(rss_parser->last) {
                /* rss_parser->last->guid_is_url=!strcmp(attrValue, "true"); */
            }
          }
        } else if(!strcmp(attrName, "url")) {
          if(!strcmp(name, "source")) {
            /* <source url="...">foo</source> */
            if(rss_parser->last) {
              /*
                rss_parser->last->source_url=attrValue; 
                attrValue=NULL;
               */
            }
          }
        } else if(!strcmp(attrName, "domain")) {
          if(!strcmp(name, "category")) {
            /* <category domain="URL">foo</source> */
            if(rss_parser->last) {
              /*
                rss_parser->last->category_url=attrValue; 
                attrValue=NULL;
               */
            }
          }
        }
        xmlFree(attrName);
        if(attrValue)
          xmlFree(attrValue);
      }

      if(!xmlTextReaderIsEmptyElement(reader))
        break;

      /* FALLTHROUGH if is empty element */

    case 15: /* end element */
      if(rss_parser->current_type != RAPTOR_RSS_NONE) {
        if(rss_parser->current_field != RAPTOR_RSS_FIELD_NONE) {
          RAPTOR_DEBUG3(raptor_rss_parser_processNode,
                        "Ending element %s field %s\n", 
                 name, raptor_rss_fields_info[rss_parser->current_field].name);
          rss_parser->current_field= RAPTOR_RSS_FIELD_NONE;
        } else {
          RAPTOR_DEBUG3(raptor_rss_parser_processNode,
                        "Ending element %s type %s\n", name,
                        raptor_rss_types_info[rss_parser->current_type].name);
          if(rss_parser->prev_type != RAPTOR_RSS_NONE) {
            rss_parser->current_type=rss_parser->prev_type;
            rss_parser->prev_type=RAPTOR_RSS_NONE;
            RAPTOR_DEBUG3(raptor_rss_parser_processNode,
                          "Returning to type %d - %s\n", 
                          rss_parser->current_type,
                          raptor_rss_types_info[rss_parser->current_type].name);
          } else
            rss_parser->current_type= RAPTOR_RSS_NONE;
        }
      }
      
      break;

    case 3: /* text */
      if((rss_parser->current_type==RAPTOR_RSS_NONE ||
          rss_parser->current_type==RAPTOR_RSS_UNKNOWN) ||
         (rss_parser->current_field==RAPTOR_RSS_FIELD_NONE ||
          rss_parser->current_field==RAPTOR_RSS_FIELD_UNKNOWN)) {
        char *p=value;
        while(*p) {
          if(!isspace(*p))
            break;
          p++;
        }
        if(*p)
          RAPTOR_DEBUG2(raptor_rss_parser_processNode,
                       "IGNORING non-whitespace text node '%s'\n", value);
        break;
      }

      if(rss_parser->current_type != RAPTOR_RSS_ITEM && 
         rss_parser->current_type >= RAPTOR_RSS_COMMON_IGNORED) {
        /* skipHours, skipDays common but IGNORED */ 
      } else {
        raptor_rss_item* update_item;
        
        if(rss_parser->current_type == RAPTOR_RSS_ITEM)
          update_item=rss_parser->last;
        else
          update_item=&rss_parser->common[rss_parser->current_type];
        
        RAPTOR_DEBUG4(raptor_rss_parser_processNode,
                      "Added text '%s' to field %s of type %s\n", value,
                      raptor_rss_fields_info[rss_parser->current_field].name,
                      raptor_rss_types_info[rss_parser->current_type].name);
        if(!update_item->fields[rss_parser->current_field])
          update_item->fields_count++;
        update_item->fields[rss_parser->current_field]=value;
        value=NULL;
      }
      
      break;

    case 4:  /* CData sections */
    case 5:  /* entity references */
    case 6:  /* entity declarations */
    case 7:  /* PIs */
    case 8:  /* comments */
    case 9:  /* document nodes */
    case 10: /* DTD/Doctype nodes */
    case 11: /* document fragment */
    case 12: /* notation nodes */
      break;
    
    default:
#if defined(RAPTOR_DEBUG)
      RAPTOR_DEBUG3(raptor_rss_parser_processNode, "depth %d type %d",
                    xmlTextReaderDepth(reader), type);
      fprintf(stderr," name %s %s", name,
              xmlTextReaderIsEmptyElement(reader) ? "Empty" : "");
      if (value == NULL)
        fprintf(stderr, "\n");
      else {
        fprintf(stderr, " '%s'\n", value);
      }
#endif
      RAPTOR_DEBUG2(raptor_rss_parser_processNode, "Ignoring type %d\n", type);
  }
    
  xmlFree(name);
  if(value)
    xmlFree(value);

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
    
    if(item->uri_string) {
      identifier->uri=raptor_new_uri(item->fields[RAPTOR_RSS_FIELD_LINK]);
      identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
      identifier->uri_source=RAPTOR_URI_SOURCE_URI;
    } else {
      int url_field=(i== RAPTOR_RSS_IMAGE) ? RAPTOR_RSS_FIELD_URL :
                                             RAPTOR_RSS_FIELD_LINK;
      
      if(item->fields[url_field]) {
        identifier->uri=raptor_new_uri(item->fields[url_field]);
        identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
        identifier->uri_source=RAPTOR_URI_SOURCE_URI;
      } else {
        /* need to make bnode */
        identifier->id=raptor_generate_id(rdf_parser, 0, NULL);
        identifier->type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
        identifier->uri_source=RAPTOR_URI_SOURCE_GENERATED;
      }
    }
    item->node_type=raptor_rss_types_info[i].uri;
  }

  /* sequence of rss:item */
  for(item=rss_parser->items; item; item=item->next) {
    raptor_identifier* identifier=&item->identifier;
    
    if(item->uri_string) {
      identifier->uri=raptor_new_uri(item->fields[RAPTOR_RSS_FIELD_LINK]);
      identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
      identifier->uri_source=RAPTOR_URI_SOURCE_URI;
    } else {
      if(item->fields[RAPTOR_RSS_FIELD_LINK]) {
        identifier->uri=raptor_new_uri(item->fields[RAPTOR_RSS_FIELD_LINK]);
        identifier->type=RAPTOR_IDENTIFIER_TYPE_RESOURCE;
        identifier->uri_source=RAPTOR_URI_SOURCE_URI;
      } else {
        /* need to make bnode */
        identifier->id=raptor_generate_id(rdf_parser, 0, NULL);
        identifier->type=RAPTOR_IDENTIFIER_TYPE_ANONYMOUS;
        identifier->uri_source=RAPTOR_URI_SOURCE_GENERATED;
      }
    }
    item->node_type=raptor_rss_types_info[RAPTOR_RSS_ITEM].uri;
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
raptor_rss_emit_item(raptor_parser* rdf_parser, raptor_rss_item *item) 
{
  raptor_rss_parser_context* rss_parser=(raptor_rss_parser_context*)rdf_parser->context;
  int f;
  raptor_identifier* identifier=&item->identifier;
    
  if(!item->fields_count)
    return;

  raptor_rss_emit_type_triple(rdf_parser, identifier, item->node_type);
  
  for(f=0; f< RAPTOR_RSS_COMMON_SIZE; f++) {
    char* value=item->fields[f];
    if(!value)
      continue;
    
    rss_parser->statement.predicate=raptor_rss_fields_info[f].uri;
    if(!rss_parser->statement.predicate)
      continue;
    
    rss_parser->statement.object=value;
    rss_parser->statement.object_type=RAPTOR_IDENTIFIER_TYPE_LITERAL;
    
    /* Generate the statement */
    (*rdf_parser->statement_handler)(rdf_parser->user_data, &rss_parser->statement);
  }
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

    RAPTOR_DEBUG3(raptor_rss_emit, "Emitting type %i - %s\n", i,
                  raptor_rss_types_info[i].name);

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
                                (char*)raptor_generate_id(rdf_parser, 0, NULL),
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
    char *uri=raptor_uri_as_string(rdf_parser->base_uri);

    rss_parser->input=xmlParserInputBufferCreateMem(s, len,
                                                    XML_CHAR_ENCODING_NONE);
    rss_parser->reader=xmlNewTextReader(rss_parser->input, uri);
    
    xmlTextReaderSetErrorHandler(rss_parser->reader,
                                 raptor_rss_error_handler, 
                                 rdf_parser);
  } else if(s && len)
    xmlParserInputBufferPush(rss_parser->input, len, s);

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

static void
raptor_rss_parser_register_factory(raptor_parser_factory *factory) 
{
  factory->context_length     = sizeof(raptor_rss_parser_context);
  
  factory->init      = raptor_rss_parse_init;
  factory->terminate = raptor_rss_parse_terminate;
  factory->start     = raptor_rss_parse_start;
  factory->chunk     = raptor_rss_parse_chunk;
}


void
raptor_init_parser_rss (void) {
  raptor_parser_register_factory("rss-tag-soup",  "RSS Tag Soup",
                                 &raptor_rss_parser_register_factory);
}


/* end HAVE_LIBXML_XMLREADER_H */
#endif
