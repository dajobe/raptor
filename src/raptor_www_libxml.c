/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_www_libghttp.c - Raptor WWW retrieval via libghttp
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
#include <config.h>
#endif

#ifdef WIN32
#include <win32_config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


static void
raptor_www_libxml_http_error (void *ctx, const char *msg, ...) 
{
  raptor_www* www=(raptor_www*)ctx;
  va_list args;
  
  www->failed=1;
  
  va_start(args, msg);
  fprintf(stderr, "libxml nanoHTTP error - ");
  vfprintf(stderr, msg, args);
  va_end(args);
  fprintf(stderr, "\n");
  exit(1);
}

void
raptor_www_libxml_init(raptor_www *www)
{
  initGenericErrorDefaultFunc(&www->old_handler);
  /* This will destroy the generic error default context:
   * there is no way to query and save it (xmlGenericErrorContext)
   */
  xmlSetGenericErrorFunc(www, raptor_www_libxml_http_error);
  www->ctxt=NULL;
}


void
raptor_www_libxml_free(raptor_www *www)
{
  xmlSetGenericErrorFunc(NULL, www->old_handler);
}


int
raptor_www_libxml_fetch(raptor_www *www) 
{
  if(www->proxy)
    xmlNanoHTTPScanProxy(www->proxy);

  www->ctxt=xmlNanoHTTPOpen(raptor_uri_as_string(www->uri), &www->type);
  if(!www->ctxt)
    return 1;
  
  if(www->type) {
    if(www->content_type) {
      www->content_type(www, www->content_type_userdata, www->type);
      if(www->failed) {
        xmlNanoHTTPClose(www->ctxt);
        return 1;
      }
    }
  }

  www->status_code=xmlNanoHTTPReturnCode(www->ctxt);
  
  while(1) {
    int len=xmlNanoHTTPRead(www->ctxt, www->buffer, RAPTOR_WWW_BUFFER_SIZE);
    if(len<0)
      break;
    
    www->total_bytes += len;

    if(www->write_bytes)
      www->write_bytes(www, www->write_bytes_userdata, www->buffer, len, 1);
    
    if(len < RAPTOR_WWW_BUFFER_SIZE || www->failed)
      break;
  }
  
  xmlNanoHTTPClose(www->ctxt);

  return www->failed;
}

