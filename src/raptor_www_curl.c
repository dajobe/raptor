/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_www_curl.c - Raptor WWW retrieval via libcurl
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


static size_t
raptor_www_curl_write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) 
{
  raptor_www* www=(raptor_www*)userdata;
  int bytes=size*nmemb;

  /* If WWW has been aborted, return nothing so that
   * libcurl will abort the transfer
   */
  if(www->failed)
    return -1;
  
  if(www->write_bytes)
    www->write_bytes(www, www->write_bytes_userdata, ptr, size, nmemb);
  www->total_bytes += bytes;
  return bytes;
}


static size_t 
raptor_www_curl_header_callback(void  *ptr,  size_t  size, size_t nmemb, void *userdata) 
{
  raptor_www* www=(raptor_www*)userdata;
  int bytes=size*nmemb;

  /* If WWW has been aborted, return nothing so that
   * libcurl will abort the transfer
   */
  if(www->failed)
    return -1;
  
  if(!strncmp(ptr, "Content-Type: ", 14)) {
    int len=bytes-16;
    char *type_buffer=(char*)malloc(len+1);
    strncpy(type_buffer, ptr+14, len);
    type_buffer[len]='\0';
    www->type=type_buffer;
    if(www->content_type)
      www->content_type(www, www->content_type_userdata, www->type);
  }
  
  return bytes;
}


void
raptor_www_curl_init(raptor_www *www)
{
  if(!www->curl_handle)
    www->curl_handle=curl_easy_init();

  /* send all data to this function  */
  curl_easy_setopt(www->curl_handle, CURLOPT_WRITEFUNCTION, 
                   raptor_www_curl_write_callback);

  curl_easy_setopt(www->curl_handle, CURLOPT_HEADERFUNCTION, 
                   raptor_www_curl_header_callback);

#ifndef CURLOPT_WRITEDATA
#define CURLOPT_WRITEDATA CURLOPT_FILE
#endif
  /* pass a data pointer to the callback function */
  curl_easy_setopt(www->curl_handle, CURLOPT_WRITEDATA, www);
  curl_easy_setopt(www->curl_handle, CURLOPT_WRITEHEADER, www);

  /* Make it follow Location: headers */
  curl_easy_setopt(www->curl_handle, CURLOPT_FOLLOWLOCATION, 1);

#if 0
  curl_easy_setopt(www->curl_handle, CURLOPT_VERBOSE, (void*)1);
#endif

  curl_easy_setopt(www->curl_handle, CURLOPT_ERRORBUFFER, www->error_buffer);
}


void
raptor_www_curl_free(raptor_www *www)
{
  if(www->curl_handle)
    curl_easy_cleanup(www->curl_handle);
  curl_global_cleanup();
}


int
raptor_www_curl_fetch(raptor_www *www) 
{
  if(www->user_agent)
    curl_easy_setopt(www->curl_handle, CURLOPT_USERAGENT, www->user_agent);

  /* specify URL to get */
  curl_easy_setopt(www->curl_handle, CURLOPT_URL, 
                   raptor_uri_as_string(www->uri));

  www->status=curl_easy_perform(www->curl_handle);
  curl_easy_getinfo(www->curl_handle, CURLINFO_HTTP_CODE, &www->status_code);

  if(www->status) {
    raptor_www_error(www, www->error_buffer);
    return 1;
  }
  return 0;
}
