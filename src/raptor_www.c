/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_www.c - Raptor WWW retrieval core
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
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"



#if defined(RAPTOR_WWW_LIBXML) || defined(RAPTOR_WWW_LIBGHTTP)
static int raptor_www_file_fetch(raptor_www *www, const char *url);
#endif


raptor_www *
raptor_www_new(void)
{
  raptor_www *www=calloc(sizeof(raptor_www), 1);
  if(!www)
    return NULL;
  
  www->type=NULL;
  www->free_type=1; /* default is to free content type */
  www->total_bytes=0;
  www->failed=0;
  www->status_code=0;
  www->write_bytes=NULL;
  www->content_type=NULL;

#ifdef RAPTOR_WWW_LIBCURL
  raptor_www_curl_init(www);
#endif
#ifdef RAPTOR_WWW_LIBXML
  raptor_www_libxml_init(www);
#endif
#ifdef RAPTOR_WWW_LIBGHTTP
  raptor_www_ghttp_init(www);
#endif

  return www;
}


void
raptor_www_free(raptor_www *www)
{
  /* free context */
  if(www->type) {
    if(www->free_type)
      free(www->type);
    www->type=NULL;
  }
  
  if(www->user_agent) {
    free(www->user_agent);
    www->user_agent=NULL;
  }

  if(www->proxy) {
    free(www->proxy);
    www->proxy=NULL;
  }

#ifdef RAPTOR_WWW_LIBCURL
  raptor_www_curl_free(www);
#endif
#ifdef RAPTOR_WWW_LIBXML
  raptor_www_libxml_free(www);
#endif
#ifdef RAPTOR_WWW_LIBGHTTP
  raptor_www_ghttp_free(www);
#endif

  if(www->uri)
    raptor_free_uri(www->uri);
}



void
raptor_www_set_error_handler(raptor_www *www, 
                             raptor_message_handler error_handler, 
                             void *error_data)
{
  www->error_handler=error_handler;
  www->error_data=error_data;
}


void
raptor_www_set_write_bytes_handler(raptor_www *www, 
                                   raptor_www_write_bytes_handler handler, 
                                   void *user_data)
{
  www->write_bytes=handler;
  www->write_bytes_userdata=user_data;
}


void
raptor_www_set_content_type_handler(raptor_www *www, 
                                    raptor_www_content_type_handler handler, 
                                    void *user_data)
{
  www->content_type=handler;
  www->content_type_userdata=user_data;
}


void
raptor_www_set_user_agent(raptor_www *www, const char *user_agent)
{
  char *ua_copy=malloc(strlen(user_agent)+1);
  if(!ua_copy)
    return;
  strcpy(ua_copy, user_agent);
  
  www->user_agent=ua_copy;
}


void
raptor_www_set_proxy(raptor_www *www, const char *proxy)
{
  char *proxy_copy=malloc(strlen(proxy)+1);
  if(!proxy_copy)
    return;
  strcpy(proxy_copy, proxy);
  
  www->proxy=proxy_copy;
}

void
raptor_www_error(raptor_www *www, const char *message, ...) 
{
  va_list arguments;

  va_start(arguments, message);
  if(www->error_handler) {
    char *buffer=raptor_vsnprintf(message, arguments);
    if(!buffer) {
      fprintf(stderr, "raptor_www_error: Out of memory\n");
      return;
    }
    www->error_handler(www->error_data, &www->locator, buffer);
    RAPTOR_FREE(cstring, buffer);
  } else {
    raptor_print_locator(stderr, &www->locator);
    fprintf(stderr, " raptor www error - ");
    vfprintf(stderr, message, arguments);
    fputc('\n', stderr);
  }

  va_end(arguments);
}

  
#if defined(RAPTOR_WWW_LIBXML) || defined(RAPTOR_WWW_LIBGHTTP) || defined(RAPTOR_WWW_NONE)

static int 
raptor_www_file_fetch(raptor_www *www, const char *url) 
{
  char *filename;
  FILE *fh;
/* FIXME */
#define BUFFER_SIZE 256
  unsigned char buffer[BUFFER_SIZE];
  int status=0;
 
  filename=raptor_uri_uri_string_to_filename(url);
  if(!filename) {
    raptor_www_error(www, "Not a file: URI");
    return 1;
  }

  fh=fopen(filename, "rb");
  if(!fh) {
    raptor_www_error(www, "file '%s' open failed - %s",
                     filename, strerror(errno));
    free(filename);
    www->status_code=404;
    return 1;
  }

  while(!feof(fh)) {
    int len=fread(buffer, 1, BUFFER_SIZE, fh);
    www->total_bytes += len;

    if(www->write_bytes)
      www->write_bytes(www, www->userdata, buffer, len, 1);

    if(len < BUFFER_SIZE)
      break;

  }
  fclose(fh);

  free(filename);
  
  if(!status)
    www->status_code=200;
  
  return status;
}

#endif


int
raptor_www_fetch(raptor_www *www, raptor_uri *uri) 
{
  www->uri=raptor_uri_copy(uri);
  
  www->locator.uri=uri;
  www->locator.line= -1;
  www->locator.column= -1;

#ifdef RAPTOR_WWW_NONE
  return raptor_www_file_fetch(www);
#endif

#ifdef RAPTOR_WWW_LIBCURL
  return raptor_www_curl_fetch(www);
#endif

#ifdef RAPTOR_WWW_LIBXML
  if(raptor_uri_is_file_uri(url))
    return raptor_www_file_fetch(www);

  return raptor_www_libxml_fetch(www);
#endif

#ifdef RAPTOR_WWW_LIBGHTTP
  if(raptor_uri_is_file_uri(url))
    return raptor_www_file_fetch(www);

  return raptor_www_ghttp_fetch(www);
#endif

}
