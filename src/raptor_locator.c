/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_locator.c - Raptor parsing locator functions
 *
 * $Id$
 *
 * Copyright (C) 2002 David Beckett - http://purl.org/net/dajobe/
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


/**
 * raptor_print_locator - Print a raptor locator to a stream
 * @stream: stream to print to
 * @locator: &raptor_locator to print
 * 
 **/
void
raptor_print_locator(FILE *stream, raptor_locator* locator) 
{
  if(!locator)
    return;

  if(locator->uri)
    fprintf(stream, "URI %s", raptor_uri_as_string(locator->uri));
  else if (locator->file)
    fprintf(stream, "file %s", locator->file);
  else
    return;
  if(locator->line >= 0) {
    fprintf(stream, ":%d", locator->line);
    if(locator->column >= 0)
      fprintf(stream, " column %d", locator->column);
  }
}



/**
 * raptor_format_locator - Format a raptor locator as a string
 * @buffer: buffer to store format
 * @length: size of buffer
 * @locator: &raptor_locator to format
 * 
 * If buffer is NULL or length is insufficient for the size of
 * the locator, returns the number of additional bytes required
 * in the buffer to write the locator.
 * 
 * Return value: 0 on success, >0 if additional bytes required in buffer, <0 on failure
 **/
int
raptor_format_locator(char *buffer, size_t length, raptor_locator* locator) 
{
  size_t bufsize=0;
  int count;
  
  if(!locator)
    return -1;

  if(locator->uri) {
    size_t uri_len;
    (void)raptor_uri_as_counted_string(locator->uri, &uri_len);
    bufsize= 4 + uri_len; /* "URI " */
  } else if (locator->file)
    bufsize= 5 + strlen(locator->file); /* "file " */
  else
    return -1;

  if(locator->line) {
    bufsize += snprintf(NULL, 0, ":%d", locator->line);
    if(locator->column >= 0)
      bufsize += snprintf(NULL, 0, " column %d", locator->column);
  }
  
  if(!buffer || !length || length < bufsize)
    return bufsize;
  

  if(locator->uri)
    count=sprintf(buffer, "URI %s", raptor_uri_as_string(locator->uri));
  else if (locator->file)
    count=sprintf(buffer, "file %s", locator->file);
  else
    return -1;

  buffer+= count;
  
  if(locator->line) {
    count=sprintf(buffer, ":%d", locator->line);
    if(locator->column >= 0)
      sprintf(buffer+count, " column %d", locator->column);
  }

  return 0;
}


void
raptor_update_document_locator (raptor_parser *rdf_parser) {
#ifdef RAPTOR_XML_EXPAT
  raptor_expat_update_document_locator (rdf_parser);
#endif
#ifdef RAPTOR_XML_LIBXML
  raptor_libxml_update_document_locator (rdf_parser);
#endif
}
