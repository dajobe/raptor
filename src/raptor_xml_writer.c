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



struct raptor_xml_writer_s {
  int canonicalize;
  
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
  RAPTOR_FREE(raptor_xml_writer, xml_writer);
}


void
raptor_xml_writer_start_element(raptor_xml_writer* xml_writer,
                                raptor_sax2_element *element)
{
  int length;
  char *str=raptor_format_sax2_element(element, &length, 0, 
                                       xml_writer->error_handler,
                                       xml_writer->error_data);
  /* append (str, length) */
  RAPTOR_FREE(cstring, str);
}


void
raptor_xml_writer_end_element(raptor_xml_writer* xml_writer,
                              raptor_sax2_element *element)
{
  int length;
  char * str=raptor_format_sax2_element(element, &length, 1, 
                                        xml_writer->error_handler,
                                        xml_writer->error_data);
  /* append (str, length) */
  RAPTOR_FREE(cstring, str);
}


void
raptor_xml_writer_cdata(raptor_xml_writer* xml_writer,
                        char *str, int length)
{
  /* append (str, length) */
}


char *
raptor_xml_writer_as_string(raptor_xml_writer* xml_writer)
{
  return NULL;
}





