/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_xml_writer.c - Raptor XML Writer for SAX2 events API
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
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


/* Define this for far too much output */
#undef RAPTOR_DEBUG_CDATA


struct raptor_xml_writer_s {
  int canonicalize;

  int depth;
  
  /* namespaces stack when canonicalizing */
  raptor_namespace_stack content_cdata_namespaces;
  int content_cdata_namespaces_depth;

  raptor_uri_handler *uri_handler;
  void *uri_context;

  raptor_simple_message_handler error_handler;
  void *error_data;

  raptor_sax2_element* current_element;

  raptor_stringbuffer *sb;
};


raptor_xml_writer*
raptor_new_xml_writer(raptor_uri_handler *uri_handler,
                      void *uri_context,
                      raptor_simple_message_handler error_handler,
                      void *error_data,
                      int canonicalize)
{
  raptor_xml_writer* xml_writer;
  
  xml_writer=(raptor_xml_writer*)RAPTOR_CALLOC(raptor_xml_writer, 1, sizeof(raptor_xml_writer)+1);
  if(!xml_writer)
    return NULL;

  /* Initialise to the empty string */
  xml_writer->content_cdata_namespaces_depth=0;

  xml_writer->uri_handler=uri_handler;
  xml_writer->uri_context=uri_context;

  xml_writer->error_handler=error_handler;
  xml_writer->error_data=error_data;

  raptor_namespaces_init(&xml_writer->content_cdata_namespaces,
                         uri_handler, uri_context,
                         error_handler, error_data,
                         0);

  xml_writer->sb=raptor_new_stringbuffer();
  
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
  raptor_namespaces_clear(&xml_writer->content_cdata_namespaces);
  if(xml_writer->sb)
    raptor_free_stringbuffer(xml_writer->sb);

  RAPTOR_FREE(raptor_xml_writer, xml_writer);
}


void
raptor_xml_writer_start_element(raptor_xml_writer* xml_writer,
                                raptor_sax2_element *element)
{
  size_t fmt_length;
  unsigned char *fmt_buffer=raptor_format_sax2_element(element, 
                                              &xml_writer->content_cdata_namespaces,
                                              &fmt_length, 0, 
                                              xml_writer->error_handler,
                                              xml_writer->error_data,
                                              xml_writer->depth);
  if(fmt_buffer && fmt_length) {
    raptor_stringbuffer_append_counted_string(xml_writer->sb, fmt_buffer, fmt_length, 0);
#ifdef RAPTOR_DEBUG_CDATA
    RAPTOR_DEBUG2("content cdata appended, now %d bytes\n", (int)raptor_stringbuffer_length(xml_writer->sb));
#endif
  }

  xml_writer->depth++;

  xml_writer->current_element=element;
  if(element && element->parent)
    element->parent->content_element_seen=1;
}


void
raptor_xml_writer_end_element(raptor_xml_writer* xml_writer,
                              raptor_sax2_element* element)
{
  size_t fmt_length;
  unsigned char *fmt_buffer;

  fmt_buffer=raptor_format_sax2_element(element, 
                                        &xml_writer->content_cdata_namespaces,
                                        &fmt_length, 1,
                                        xml_writer->error_handler,
                                        xml_writer->error_data,
                                        xml_writer->depth);

  if(fmt_buffer && fmt_length)
    raptor_stringbuffer_append_counted_string(xml_writer->sb, fmt_buffer, fmt_length, 0);
  xml_writer->depth--;

  raptor_namespaces_end_for_depth(&xml_writer->content_cdata_namespaces, 
                                  xml_writer->depth);

#ifdef RAPTOR_DEBUG_CDATA
  RAPTOR_DEBUG2("content cdata now %d bytes\n", (int)raptor_stringbuffer_length(xml_writer->sb));
#endif

  if(xml_writer->current_element)
    xml_writer->current_element = xml_writer->current_element->parent;
}


void
raptor_xml_writer_cdata(raptor_xml_writer* xml_writer,
                        const unsigned char *s, unsigned int len)
{
  unsigned char *buffer;
  int buffer_len=raptor_xml_escape_string(s, len,
                                          NULL, 0, '\0',
                                          xml_writer->error_handler,
                                          xml_writer->error_data);

  if(buffer_len < 0)
    return;
  
  buffer=(unsigned char*)RAPTOR_MALLOC(cstring, buffer_len+1);
  if(!buffer)
    return;

  if(buffer_len != (int)len)
    raptor_xml_escape_string(s, len,
                             buffer, buffer_len, '\0',
                             xml_writer->error_handler,
                             xml_writer->error_data);
  else {
    strncpy((char*)buffer, (const char*)s, buffer_len);
    buffer[buffer_len]='\0';
  }

  raptor_stringbuffer_append_counted_string(xml_writer->sb, buffer, buffer_len, 0);

  if(xml_writer->current_element)
    xml_writer->current_element->content_cdata_seen=1;
}


void
raptor_xml_writer_comment(raptor_xml_writer* xml_writer,
                        const unsigned char *s, unsigned int len)
{
  raptor_xml_writer_cdata(xml_writer, s, len);
}


unsigned char*
raptor_xml_writer_as_string(raptor_xml_writer* xml_writer,
                            unsigned int *length_p)
{
  if(length_p)
    *length_p=raptor_stringbuffer_length(xml_writer->sb);

  return raptor_stringbuffer_as_string(xml_writer->sb);
}
