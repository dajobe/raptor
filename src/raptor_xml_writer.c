/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_xml_writer.c - Raptor XML Writer for SAX2 events API
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
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


#define RAPTOR_DEBUG_CDATA 0


struct raptor_xml_writer_s {
  int canonicalize;

  int depth;
  
  /* CDATA content of element and checks for mixed content */
  char *content_cdata;
  unsigned int content_cdata_length;

  /* namespaces stack when canonicalizing */
  raptor_namespace_stack content_cdata_namespaces;
  int content_cdata_namespaces_depth;

  raptor_uri_handler *uri_handler;
  void *uri_context;

  raptor_simple_message_handler error_handler;
  void *error_data;
};


raptor_xml_writer*
raptor_new_xml_writer(raptor_uri_handler *uri_handler,
                      void *uri_context,
                      raptor_simple_message_handler error_handler,
                      void *error_data,
                      int canonicalize)
{
  raptor_xml_writer* xml_writer;
  
  xml_writer=(raptor_xml_writer*)RAPTOR_CALLOC(raptor_xml_writer, sizeof(raptor_xml_writer), 1);
  if(!xml_writer)
    return NULL;
  
  xml_writer->content_cdata_namespaces_depth=0;

  xml_writer->uri_handler=uri_handler;
  xml_writer->uri_context=uri_context;

  xml_writer->error_handler=error_handler;
  xml_writer->error_data=error_data;

  raptor_namespaces_init(&xml_writer->content_cdata_namespaces,
                         uri_handler, uri_context,
                         error_handler, error_data);

  return xml_writer;
}


/**
 * raptor_free_xml_writer: Free XML writer content
 * @xml_writer: XML writer object
 * 
 **/
void
raptor_free_xml_writer(raptor_xml_writer* xml_writer)
{
  raptor_namespaces_free(&xml_writer->content_cdata_namespaces);
  if(xml_writer->content_cdata)
    RAPTOR_FREE(cstring, xml_writer->content_cdata);

  RAPTOR_FREE(raptor_xml_writer, xml_writer);
}


void
raptor_xml_writer_start_element(raptor_xml_writer* xml_writer,
                                raptor_sax2_element *element)
{
  int fmt_length;
  char *fmt_buffer=raptor_format_sax2_element(element, 
                                              &xml_writer->content_cdata_namespaces,
                                              &fmt_length, 0, 
                                              xml_writer->error_handler,
                                              xml_writer->error_data,
                                              xml_writer->depth);
  if(fmt_buffer && fmt_length) {
    /* Append cdata content content */
    if(xml_writer->content_cdata) {
      /* Append */
      char *new_cdata=(char*)RAPTOR_MALLOC(cstring, xml_writer->content_cdata_length + fmt_length + 1);
      if(new_cdata) {
        strncpy(new_cdata, xml_writer->content_cdata,
                xml_writer->content_cdata_length);
        strcpy(new_cdata+xml_writer->content_cdata_length, fmt_buffer);
        RAPTOR_FREE(cstring, xml_writer->content_cdata);
        xml_writer->content_cdata=new_cdata;
        xml_writer->content_cdata_length+=fmt_length;
      }
      RAPTOR_FREE(cstring, fmt_buffer);
      
#ifdef RAPTOR_DEBUG_CDATA
      RAPTOR_DEBUG3(raptor_xml_writer_start_element,
                    "content cdata appended, now: '%s' (%d bytes)\n", 
                    xml_writer->content_cdata,
                    xml_writer->content_cdata_length);
#endif
      
    } else {
      /* Copy - is empty */
      xml_writer->content_cdata=fmt_buffer;
      xml_writer->content_cdata_length=fmt_length;
      
#ifdef RAPTOR_DEBUG_CDATA
      RAPTOR_DEBUG3(raptor_xml_writer_start_element,
                    "content cdata copied, now: '%s' (%d bytes)\n", 
                    xml_writer->content_cdata,
                    xml_writer->content_cdata_length);
#endif
      
    }
  }

  xml_writer->depth++;
}


void
raptor_xml_writer_end_element(raptor_xml_writer* xml_writer,
                              raptor_sax2_element* element)
{
  int fmt_length;
  char *fmt_buffer;

  fmt_buffer=raptor_format_sax2_element(element, 
                                        &xml_writer->content_cdata_namespaces,
                                        &fmt_length, 1,
                                        xml_writer->error_handler,
                                        xml_writer->error_data,
                                        xml_writer->depth);

  if(fmt_buffer && fmt_length) {
    /* Append cdata content content */
    char *new_cdata=(char*)RAPTOR_MALLOC(cstring, xml_writer->content_cdata_length + fmt_length + 1);
    if(new_cdata) {
      strncpy(new_cdata, xml_writer->content_cdata,
              xml_writer->content_cdata_length);
      strcpy(new_cdata+xml_writer->content_cdata_length, fmt_buffer);
      RAPTOR_FREE(cstring, xml_writer->content_cdata);
      xml_writer->content_cdata=new_cdata;
      xml_writer->content_cdata_length += fmt_length;
    }
    RAPTOR_FREE(cstring, fmt_buffer);
  }

  xml_writer->depth--;

  raptor_namespaces_end_for_depth(&xml_writer->content_cdata_namespaces, 
                                  xml_writer->depth);

#ifdef RAPTOR_DEBUG_CDATA
  RAPTOR_DEBUG3(raptor_xml_writer_end_element,
                "content cdata now: '%s' (%d bytes)\n", 
                xml_writer->content_cdata, xml_writer->content_cdata_length);
#endif
}


void
raptor_xml_writer_cdata(raptor_xml_writer* xml_writer,
                        const unsigned char *s, int len)
{
  unsigned char *escaped_buffer=NULL;
  unsigned char *buffer;
  size_t escaped_buffer_len=raptor_xml_escape_string(s, len,
                                                     NULL, 0, '\0',
                                                     xml_writer->error_handler,
                                                     xml_writer->error_data);
  unsigned char *ptr;

  /* save a malloc/free when there is no escaping */
  if(escaped_buffer_len != len) {
    escaped_buffer=(unsigned char*)RAPTOR_MALLOC(cstring, escaped_buffer_len+1);
    if(!escaped_buffer)
      return;

    raptor_xml_escape_string(s, len,
                             escaped_buffer, escaped_buffer_len, '\0',
                             xml_writer->error_handler,
                             xml_writer->error_data);
    len=escaped_buffer_len;
  }

  buffer=(unsigned char*)RAPTOR_MALLOC(cstring, xml_writer->content_cdata_length + len + 1);
  if(!buffer)
    return;

  if(xml_writer->content_cdata_length) {
    strncpy(buffer, xml_writer->content_cdata, xml_writer->content_cdata_length);
    RAPTOR_FREE(cstring, xml_writer->content_cdata);
  }

  xml_writer->content_cdata=buffer;

  /* move pointer to end of cdata buffer */
  ptr=buffer+xml_writer->content_cdata_length;

  /* adjust stored length */
  xml_writer->content_cdata_length += len;

  /* now write new stuff at end of cdata buffer */
  if(escaped_buffer) {
    strncpy(ptr, escaped_buffer, len);
    RAPTOR_FREE(cstring, escaped_buffer);
  } else
    strncpy(ptr, (char*)s, len);
  ptr += len;
  *ptr = '\0';

}


unsigned char*
raptor_xml_writer_as_string(raptor_xml_writer* xml_writer,
                            int *length_p)
{
  if(length_p)
    *length_p=xml_writer->content_cdata_length;

  return xml_writer->content_cdata;
}
