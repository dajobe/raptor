/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_xml_writer.c - Raptor XML Writer for SAX2 events API
 *
 * $Id$
 *
 * Copyright (C) 2003-2005, David Beckett http://purl.org/net/dajobe/
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

#ifndef STANDALONE


/* Define this for far too much output */
#undef RAPTOR_DEBUG_CDATA


struct raptor_xml_writer_s {
  int canonicalize;

  int depth;
  
  int my_nstack;
  raptor_namespace_stack *nstack;
  int nstack_depth;

  raptor_uri_handler *uri_handler;
  void *uri_context;

  raptor_simple_message_handler error_handler;
  void *error_data;

  raptor_xml_element* current_element;

  /* outputting to this iostream */
  raptor_iostream *iostr;
};


raptor_xml_writer*
raptor_new_xml_writer(raptor_namespace_stack *nstack,
                      raptor_uri_handler *uri_handler,
                      void *uri_context,
                      raptor_iostream* iostr,
                      raptor_simple_message_handler error_handler,
                      void *error_data,
                      int canonicalize) {
  raptor_xml_writer* xml_writer;
  
  xml_writer=(raptor_xml_writer*)RAPTOR_CALLOC(raptor_xml_writer, 1, sizeof(raptor_xml_writer)+1);
  if(!xml_writer)
    return NULL;

  xml_writer->nstack_depth=0;

  xml_writer->uri_handler=uri_handler;
  xml_writer->uri_context=uri_context;

  xml_writer->error_handler=error_handler;
  xml_writer->error_data=error_data;

  xml_writer->nstack=nstack;
  if(!xml_writer->nstack) {
    xml_writer->nstack=nstack=raptor_new_namespaces(uri_handler, uri_context,
                                                    error_handler, error_data,
                                                    1);
    xml_writer->my_nstack=1;
  }

  xml_writer->iostr=iostr;
  
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
  if(xml_writer->nstack && xml_writer->my_nstack)
    raptor_free_namespaces(xml_writer->nstack);

  RAPTOR_FREE(raptor_xml_writer, xml_writer);
}


void
raptor_xml_writer_empty_element(raptor_xml_writer* xml_writer,
                                raptor_xml_element *element)
{
  raptor_iostream_write_xml_element(xml_writer->iostr,
                                     element, 
                                     xml_writer->nstack,
                                     1,
                                     0,
                                     xml_writer->error_handler,
                                     xml_writer->error_data,
                                     xml_writer->depth);
  raptor_namespaces_end_for_depth(xml_writer->nstack, xml_writer->depth);
}


void
raptor_xml_writer_start_element(raptor_xml_writer* xml_writer,
                                raptor_xml_element *element)
{
  raptor_iostream_write_xml_element(xml_writer->iostr,
                                     element, 
                                     xml_writer->nstack,
                                     0,
                                     0,
                                     xml_writer->error_handler,
                                     xml_writer->error_data,
                                     xml_writer->depth);

  xml_writer->depth++;
  
  xml_writer->current_element=element;
  if(element && element->parent)
    element->parent->content_element_seen=1;
}


void
raptor_xml_writer_end_element(raptor_xml_writer* xml_writer,
                              raptor_xml_element* element)
{
  raptor_iostream_write_xml_element(xml_writer->iostr, 
                                     element, 
                                     xml_writer->nstack,
                                     0,
                                     1,
                                     xml_writer->error_handler,
                                     xml_writer->error_data,
                                     xml_writer->depth);
  
  xml_writer->depth--;

  raptor_namespaces_end_for_depth(xml_writer->nstack, xml_writer->depth);

  if(xml_writer->current_element)
    xml_writer->current_element = xml_writer->current_element->parent;
}


void
raptor_xml_writer_cdata(raptor_xml_writer* xml_writer,
                        const unsigned char *s)
{
  raptor_iostream_write_xml_escaped_string(xml_writer->iostr,
                                           s, strlen((const char*)s),
                                           '\0',
                                           xml_writer->error_handler,
                                           xml_writer->error_data);

  if(xml_writer->current_element)
    xml_writer->current_element->content_cdata_seen=1;
}


void
raptor_xml_writer_cdata_counted(raptor_xml_writer* xml_writer,
                                const unsigned char *s, unsigned int len)
{
  raptor_iostream_write_xml_escaped_string(xml_writer->iostr,
                                           s, len,
                                           '\0',
                                           xml_writer->error_handler,
                                           xml_writer->error_data);

  if(xml_writer->current_element)
    xml_writer->current_element->content_cdata_seen=1;
}


void
raptor_xml_writer_raw(raptor_xml_writer* xml_writer,
                      const unsigned char *s)
{
  raptor_iostream_write_string(xml_writer->iostr, s);

  if(xml_writer->current_element)
    xml_writer->current_element->content_cdata_seen=1;
}


void
raptor_xml_writer_raw_counted(raptor_xml_writer* xml_writer,
                              const unsigned char *s, unsigned int len)
{
  raptor_iostream_write_counted_string(xml_writer->iostr, s, len);

  if(xml_writer->current_element)
    xml_writer->current_element->content_cdata_seen=1;
}


void
raptor_xml_writer_comment(raptor_xml_writer* xml_writer,
                          const unsigned char *s)
{
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"<!-- ", 5);
  raptor_xml_writer_cdata(xml_writer, s);
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)" -->", 4);
}


void
raptor_xml_writer_comment_counted(raptor_xml_writer* xml_writer,
                                  const unsigned char *s, unsigned int len)
{
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"<!-- ", 5);
  raptor_xml_writer_cdata_counted(xml_writer, s, len);
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)" -->", 4);
}


#endif



#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);


const unsigned char *base_uri_string=(const unsigned char*)"http://example.org/base#";

#define OUT_BYTES_COUNT 96

int
main(int argc, char *argv[]) 
{
  const char *program=raptor_basename(argv[0]);
  raptor_uri_handler *uri_handler;
  void *uri_context;
  raptor_iostream *iostr;
  raptor_namespace_stack *nstack;
  raptor_namespace* foo_ns;
  raptor_xml_writer* xml_writer;
  raptor_uri* base_uri;
  raptor_qname* el_name;
  raptor_xml_element *element;
  size_t count;
  raptor_qname **attrs;

  /* for raptor_new_iostream_to_string */
  char *string=NULL;
  size_t string_len=0;

  raptor_init();
  
  iostr=raptor_new_iostream_to_string((void**)&string, &string_len, NULL);
  if(!iostr) {
    fprintf(stderr, "%s: Failed to create iostream to string\n", program);
    exit(1);
  }

  raptor_uri_get_handler(&uri_handler, &uri_context);

  nstack=raptor_new_namespaces(uri_handler, uri_context,
                               NULL, NULL, /* errors */
                               1);

  xml_writer=raptor_new_xml_writer(nstack,
                                   uri_handler, uri_context,
                                   iostr,
                                   NULL, NULL, /* errors */
                                   1);
  if(!xml_writer) {
    fprintf(stderr, "%s: Failed to create xml_writer to iostream\n", program);
    exit(1);
  }

  base_uri=raptor_new_uri(base_uri_string);

  foo_ns=raptor_new_namespace(nstack,
                              (const unsigned char*)"foo",
                              (const unsigned char*)"http://example.org/foo-ns#",
                              0);


  el_name=raptor_new_qname_from_namespace_local_name(foo_ns,
                                                     (const unsigned char*)"bar", 
                                                     NULL);
  element=raptor_new_xml_element(el_name,
                                  NULL, /* language */
                                  raptor_uri_copy(base_uri));

  raptor_xml_writer_start_element(xml_writer, element);
  raptor_xml_writer_cdata_counted(xml_writer, (const unsigned char*)"hello\n", 6);
  raptor_xml_writer_comment_counted(xml_writer, (const unsigned char*)"comment", 7);
  raptor_xml_writer_cdata(xml_writer, (const unsigned char*)"\n");
  raptor_xml_writer_end_element(xml_writer, element);

  raptor_free_xml_element(element);

  raptor_xml_writer_cdata(xml_writer, (const unsigned char*)"\n");

  el_name=raptor_new_qname(nstack, 
                           (const unsigned char*)"blah", 
                           NULL, /* no attribute value - element */
                           NULL, NULL); /* errors */
  element=raptor_new_xml_element(el_name,
                                  NULL, /* language */
                                  raptor_uri_copy(base_uri));

  attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname*));
  attrs[0]=raptor_new_qname(nstack, 
                            (const unsigned char*)"a",
                            (const unsigned char*)"b", /* attribute value */
                            NULL, NULL); /* errors */
  raptor_xml_element_set_attributes(element, attrs, 1);

  raptor_xml_writer_empty_element(xml_writer, element);

  raptor_xml_writer_cdata(xml_writer, (const unsigned char*)"\n");

  raptor_free_xml_writer(xml_writer);

  raptor_free_xml_element(element);

  raptor_free_namespace(foo_ns);

  raptor_free_namespaces(nstack);

  raptor_free_uri(base_uri);

  
  count=raptor_iostream_get_bytes_written_count(iostr);

#ifdef RAPTOR_DEBUG
  fprintf(stderr, "%s: Freeing iostream\n", program);
#endif
  raptor_free_iostream(iostr);

  if(count != OUT_BYTES_COUNT) {
    fprintf(stderr, "%s: I/O stream wrote %d bytes, expected %d\n", program,
            (int)count, (int)OUT_BYTES_COUNT);
    fputs("[[", stderr);
    fwrite(string, 1, string_len, stderr);
    fputs("]]\n", stderr);
    return 1;
  }
  
  if(!string) {
    fprintf(stderr, "%s: I/O stream failed to create a string\n", program);
    return 1;
  }
  string_len=strlen(string);
  if(string_len != count) {
    fprintf(stderr, "%s: I/O stream created a string length %d, expected %d\n", program, (int)string_len, (int)count);
    return 1;
  }

  fprintf(stderr, "%s: Made XML string of %d bytes\n", program, (int)string_len);
  fputs("[[", stderr);
  fwrite(string, 1, string_len, stderr);
  fputs("]]\n", stderr);

  raptor_free_memory(string);
  

  raptor_finish();
  
  /* keep gcc -Wall happy */
  return(0);
}

#endif
