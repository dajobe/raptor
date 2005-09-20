/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_sax2.c - Raptor SAX2 API
 *
 * $Id$
 *
 * Copyright (C) 2000-2005, David Beckett http://purl.org/net/dajobe/
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
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


/* Define this for far too much output */
#undef RAPTOR_DEBUG_CDATA


void
raptor_init_sax2(void)
{
#ifdef RAPTOR_XML_LIBXML
  xmlInitParser();
#endif
}


void
raptor_finish_sax2(void)
{
#ifdef RAPTOR_XML_LIBXML
  xmlCleanupParser();
#endif
}


raptor_sax2*
raptor_new_sax2(void* user_data,
                void *error_data, raptor_message_handler error_handler,
                void *fatal_error_data, raptor_message_handler fatal_error_handler,
                void *warning_data, raptor_message_handler warning_handler)
{
  raptor_sax2* sax2;
  sax2=(raptor_sax2*)RAPTOR_CALLOC(raptor_sax2, 1, sizeof(raptor_sax2));
  if(!sax2)
    return NULL;

#ifdef RAPTOR_XML_LIBXML
  sax2->magic=RAPTOR_LIBXML_MAGIC;
#endif
  
  sax2->user_data=user_data;
  sax2->error_data=error_data;
  sax2->error_handler=error_handler;
  sax2->fatal_error_data=fatal_error_data;
  sax2->fatal_error_handler=fatal_error_handler;
  sax2->warning_data=warning_data;
  sax2->warning_handler=warning_handler;
  
  return sax2;
}


void
raptor_free_sax2(raptor_sax2 *sax2) {
  raptor_xml_element *xml_element;

#ifdef RAPTOR_XML_EXPAT
  if(sax2->xp) {
    XML_ParserFree(sax2->xp);
    sax2->xp=NULL;
  }
#endif

#ifdef RAPTOR_XML_LIBXML
  if(sax2->xc) {
    raptor_libxml_free(sax2->xc);
    sax2->xc=NULL;
  }

#ifdef RAPTOR_LIBXML_MY_ENTITIES
  raptor_libxml_libxml_free_entities(rdf_parser);
#endif

#endif

  while( (xml_element=raptor_xml_element_pop(sax2)) )
    raptor_free_xml_element(xml_element);

  RAPTOR_FREE(raptor_sax2, sax2);
}



void
raptor_sax2_set_start_element_handler(raptor_sax2* sax2,
                                      raptor_sax2_start_element_handler handler)
{
  sax2->start_element_handler=handler;
}


void
raptor_sax2_set_end_element_handler(raptor_sax2* sax2,
                                    raptor_sax2_end_element_handler handler)
{
  sax2->end_element_handler=handler;
}


void
raptor_sax2_set_characters_handler(raptor_sax2* sax2,
                                   raptor_sax2_characters_handler handler)
{
  sax2->characters_handler=handler;
}


void
raptor_sax2_set_cdata_handler(raptor_sax2* sax2,
                              raptor_sax2_cdata_handler handler)
{
  sax2->cdata_handler=handler;
}


void
raptor_sax2_set_comment_handler(raptor_sax2* sax2,
                                raptor_sax2_comment_handler handler)
{
  sax2->comment_handler=handler;
}


void
raptor_sax2_set_unparsed_entity_decl_handler(raptor_sax2* sax2,
                                             raptor_sax2_unparsed_entity_decl_handler handler)
{
  sax2->unparsed_entity_decl_handler=handler;
}


void
raptor_sax2_set_external_entity_ref_handler(raptor_sax2* sax2,
                                            raptor_sax2_external_entity_ref_handler handler)
{
  sax2->external_entity_ref_handler=handler;
}


void
raptor_sax2_set_locator(raptor_sax2* sax2, raptor_locator* locator)
{
  sax2->locator=locator;
}


raptor_xml_element*
raptor_xml_element_pop(raptor_sax2 *sax2) 
{
  raptor_xml_element *element=sax2->current_element;

  if(!element)
    return NULL;

  sax2->current_element=element->parent;
  if(sax2->root_element == element) /* just deleted root */
    sax2->root_element=NULL;

  return element;
}


void
raptor_xml_element_push(raptor_sax2 *sax2, raptor_xml_element* element) 
{
  element->parent=sax2->current_element;
  sax2->current_element=element;
  if(!sax2->root_element)
    sax2->root_element=element;
}


const unsigned char*
raptor_sax2_inscope_xml_language(raptor_sax2 *sax2) {
  raptor_xml_element* xml_element;
  
  for(xml_element=sax2->current_element;
      xml_element; 
      xml_element=xml_element->parent)
    if(xml_element->xml_language)
      return xml_element->xml_language;
    
  return NULL;
}


raptor_uri*
raptor_sax2_inscope_base_uri(raptor_sax2 *sax2) {
  raptor_xml_element *xml_element;
  
  for(xml_element=sax2->current_element; 
      xml_element; 
      xml_element=xml_element->parent)
    if(xml_element->base_uri)
      return xml_element->base_uri;
    
  return NULL;
}


int
raptor_sax2_get_depth(raptor_sax2 *sax2) {
  return sax2->depth;
}

void
raptor_sax2_inc_depth(raptor_sax2 *sax2) {
  sax2->depth++;
}

void
raptor_sax2_dec_depth(raptor_sax2 *sax2) {
  sax2->depth--;
}


void
raptor_sax2_parse_start(raptor_sax2* sax2, raptor_uri *base_uri) {
  sax2->depth=0;
  sax2->root_element=NULL;
  sax2->current_element=NULL;

#ifdef RAPTOR_XML_EXPAT
  if(sax2->xp) {
    XML_ParserFree(sax2->xp);
    sax2->xp=NULL;
  }

  raptor_expat_init(sax2, base_uri);
#endif

#ifdef RAPTOR_XML_LIBXML
  raptor_libxml_init(sax2, base_uri);

#if LIBXML_VERSION < 20425
  sax2->first_read=1;
#endif

  if(sax2->xc) {
    raptor_libxml_free(sax2->xc);
    sax2->xc=NULL;
  }
#endif
}


int
raptor_sax2_parse_chunk(raptor_sax2* sax2, const unsigned char *buffer,
                        size_t len, int is_end) 
{
#ifdef RAPTOR_XML_EXPAT
  XML_Parser xp=sax2->xp;
#endif
#ifdef RAPTOR_XML_LIBXML
  /* parser context */
  xmlParserCtxtPtr xc=sax2->xc;
#endif
  int rc;
  
#ifdef RAPTOR_XML_LIBXML
  if(!xc) {
    if(!len) {
      /* no data given at all - emit a similar message to expat */
      raptor_sax2_update_document_locator(sax2, sax2->locator);
      sax2->error_handler(sax2->error_data, sax2->locator,
                          "XML Parsing failed - no element found");
      return 1;
    }

    xc = xmlCreatePushParserCtxt(&sax2->sax, sax2, /* user data */
                                 (char*)buffer, len, 
                                 NULL);
    if(!xc)
      goto handle_error;
    
    xc->userData = sax2; /* user data */
    xc->vctxt.userData = sax2; /* user data */
    xc->vctxt.error=raptor_libxml_validation_error;
    xc->vctxt.warning=raptor_libxml_validation_warning;
    xc->replaceEntities = 1;
    
    sax2->xc = xc;

    if(is_end)
      len=0;
    else
      return 0;
  }
#endif

  if(!len) {
#ifdef RAPTOR_XML_EXPAT
    rc=XML_Parse(xp, (char*)buffer, 0, 1);
    if(!rc) /* expat: 0 is failure */
      goto handle_error;
#endif
#ifdef RAPTOR_XML_LIBXML
    xmlParseChunk(xc, (char*)buffer, 0, 1);
#endif
    return 0;
  }


#ifdef RAPTOR_XML_EXPAT
  rc=XML_Parse(xp, (char*)buffer, len, is_end);
  if(!rc) /* expat: 0 is failure */
    goto handle_error;
  if(is_end)
    return 0;
#endif

#ifdef RAPTOR_XML_LIBXML

  /* This works around some libxml versions that fail to work
   * if the buffer size is larger than the entire file
   * and thus the entire parsing is done in one operation.
   *
   * The code below:
   *   2.4.19 (oldest tested) to 2.4.24 - required
   *   2.4.25                           - works with or without it
   *   2.4.26 or later                  - fails with this code
   */

#if LIBXML_VERSION < 20425
  if(sax2->first_read && is_end) {
    /* parse all but the last character */
    rc=xmlParseChunk(xc, (char*)buffer, len-1, 0);
    if(rc)
      goto handle_error;
    /* last character */
    rc=xmlParseChunk(xc, (char*)buffer + (len-1), 1, 0);
    if(rc)
      goto handle_error;
    /* end */
    xmlParseChunk(xc, (char*)buffer, 0, 1);
    return 0;
  }
#endif

#if LIBXML_VERSION < 20425
  sax2->first_read=0;
#endif
    
  rc=xmlParseChunk(xc, (char*)buffer, len, is_end);
  if(rc) /* libxml: non 0 is failure */
    goto handle_error;
  if(is_end)
    return 0;
#endif

  return 0;

  handle_error:

#ifdef RAPTOR_XML_EXPAT
#ifdef EXPAT_UTF8_BOM_CRASH
  if(sax2->tokens_count) {
#endif
    /* Work around a bug with the expat 1.95.1 shipped with RedHat 7.2
     * which dies here if the error is before <?xml?...
     * The expat 1.95.1 source release version works fine.
     */
    if(sax2->locator)
      raptor_sax2_update_document_locator(sax2, sax2->locator);
#ifdef EXPAT_UTF8_BOM_CRASH
  }
#endif
#endif /* EXPAT */

#if RAPTOR_XML_EXPAT
  if(1) {
    const char *error_prefix="XML Parsing failed - "; /* 21 chars */
    #define ERROR_PREFIX_LEN 21
    const char *error_message=XML_ErrorString(XML_GetErrorCode(xp));
    size_t error_length;
    char *error_buffer;

    error_length=strlen(error_message);
    error_buffer=(char*)RAPTOR_MALLOC(cstring, 
                                      ERROR_PREFIX_LEN + error_length+1);
    if(error_buffer) {
      strncpy(error_buffer, error_prefix, ERROR_PREFIX_LEN);
      strncpy(error_buffer+ERROR_PREFIX_LEN, error_message, error_length+1);

      sax2->error_handler(sax2->error_data, sax2->locator, error_buffer);
      RAPTOR_FREE(cstring, error_buffer);
    } else
      sax2->error_handler(sax2->error_data, sax2->locator, "XML Parsing failed");
  }
#endif
#ifdef RAPTOR_XML_LIBXML
  sax2->error_handler(sax2->error_data, sax2->locator, "XML Parsing failed");
#endif

  return 1;
}


void
raptor_sax2_update_document_locator(raptor_sax2* sax2, 
                                    raptor_locator* locator)
{
#ifdef RAPTOR_XML_EXPAT
  raptor_expat_update_document_locator(sax2, locator);
#endif
#ifdef RAPTOR_XML_LIBXML
  raptor_libxml_update_document_locator(sax2, locator);
#endif
}


/* start of an element */
void 
raptor_sax2_start_element(void* user_data, const unsigned char *name,
                          const unsigned char **atts)
{
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  
#ifdef RAPTOR_XML_EXPAT
#ifdef EXPAT_UTF8_BOM_CRASH
  sax2->tokens_count++;
#endif
#endif

#ifdef RAPTOR_XML_LIBXML
  if(atts) {
    int i;
    
    /* Do XML attribute value normalization */
    for (i = 0; atts[i]; i+=2) {
      unsigned char *value=(unsigned char*)atts[i+1];
      unsigned char *src = value;
      unsigned char *dst = xmlStrdup(value);

      if (!dst) {
        sax2->error_handler(sax2->error_data, sax2->locator, "Out of memory");
	return;
      }

      atts[i+1]=dst;

      while (*src == 0x20 || *src == 0x0d || *src == 0x0a || *src == 0x09) 
        src++;
      while (*src) {
	if (*src == 0x20 || *src == 0x0d || *src == 0x0a || *src == 0x09) {
          while (*src == 0x20 || *src == 0x0d || *src == 0x0a || *src == 0x09)
            src++;
          if (*src)
            *dst++ = 0x20;
	} else {
          *dst++ = *src++;
	}
      }
      *dst = '\0';
      xmlFree(value);
    }
  }
#endif

  if(sax2->start_element_handler)
    sax2->start_element_handler(sax2->user_data, name, atts);
}


/* end of an element */
void
raptor_sax2_end_element(void* user_data, const unsigned char *name)
{
  raptor_sax2* sax2=(raptor_sax2*)user_data;
#ifdef RAPTOR_XML_EXPAT
#ifdef EXPAT_UTF8_BOM_CRASH
  sax2->tokens_count++;
#endif
#endif

  if(sax2->end_element_handler)
    sax2->end_element_handler(sax2->user_data, name);
}


/* characters */
void
raptor_sax2_characters(void* user_data, const unsigned char *s, int len)
{
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  if(sax2->characters_handler)
    sax2->characters_handler(sax2->user_data, s, len);
}


/* like <![CDATA[...]> */
void
raptor_sax2_cdata(void* user_data, const unsigned char *s, int len)
{
  raptor_sax2* sax2=(raptor_sax2*)user_data;
#ifdef RAPTOR_XML_EXPAT
#ifdef EXPAT_UTF8_BOM_CRASH
  sax2->tokens_count++;
#endif
#endif

  if(sax2->cdata_handler)
    sax2->cdata_handler(sax2->user_data, s, len);
}


/* comment */
void
raptor_sax2_comment(void* user_data, const unsigned char *s)
{
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  if(sax2->comment_handler)
    sax2->comment_handler(sax2->user_data, s);
}


/* unparsed (NDATA) entity */
void
raptor_sax2_unparsed_entity_decl(void* user_data, 
                                 const unsigned char* entityName, 
                                 const unsigned char* base, 
                                 const unsigned char* systemId, 
                                 const unsigned char* publicId, 
                                 const unsigned char* notationName)
{
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  if(sax2->unparsed_entity_decl_handler)
    sax2->unparsed_entity_decl_handler(sax2->user_data,
                                       entityName, base, systemId, 
                                       publicId, notationName);
}


/* external entity reference */
int
raptor_sax2_external_entity_ref(void* user_data, 
                                const unsigned char* context, 
                                const unsigned char* base, 
                                const unsigned char* systemId, 
                                const unsigned char* publicId)
{
  raptor_sax2* sax2=(raptor_sax2*)user_data;
  if(sax2->external_entity_ref_handler)
    return sax2->external_entity_ref_handler(sax2->user_data,
                                             context, base, systemId, publicId);
  return 0;
}
