/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_www_curl.c - Raptor WWW retrieval via libcurl
 *
 * Copyright (C) 2003-2008, David Beckett http://www.dajobe.org/
 * Copyright (C) 2003-2004, University of Bristol, UK http://www.bristol.ac.uk/
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

#ifdef RAPTOR_WWW_LIBCURL

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


static void
raptor_www_curl_update_status(raptor_www* www) 
{
  char* final_uri;
    
  if(www->failed)
    return;
  
  if(www->checked_status++)
    return;
  
  if(!www->final_uri) {
    /* If not already found in headers by
     * raptor_www_curl_header_callback() which overrides what libcurl
     * found in HTTP status line (3xx)
     */

    if(curl_easy_getinfo(www->curl_handle, CURLINFO_EFFECTIVE_URL, 
                         &final_uri) == CURLE_OK) {
      www->final_uri = raptor_new_uri(www->world, (const unsigned char*)final_uri);
      if(www->final_uri_handler)
        www->final_uri_handler(www, www->final_uri_userdata, www->final_uri);
    }
  }

}


static size_t
raptor_www_curl_write_callback(void *ptr, size_t size, size_t nmemb, void *userdata) 
{
  raptor_www* www = (raptor_www*)userdata;
  size_t bytes = size * nmemb;

  /* If WWW has been aborted, return nothing so that
   * libcurl will abort the transfer
   */
  if(www->failed)
    return 0;
  
  raptor_www_curl_update_status(www);

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 2
  RAPTOR_DEBUG2("Got %d bytes\n", bytes);
#endif

  if(www->write_bytes)
    www->write_bytes(www, www->write_bytes_userdata, ptr, size, nmemb);
  www->total_bytes += bytes;
  return bytes;
}


static size_t 
raptor_www_curl_header_callback(void* ptr,  size_t  size, size_t nmemb,
                                void *userdata) 
{
  raptor_www* www = (raptor_www*)userdata;
  size_t bytes = size * nmemb;
  int c;

  /* If WWW has been aborted, return nothing so that
   * libcurl will abort the transfer
   */
  if(www->failed)
    return 0;
  
#define CONTENT_TYPE_LEN 14
  if(!raptor_strncasecmp((char*)ptr, "Content-Type: ", CONTENT_TYPE_LEN)) {
    size_t len = bytes - CONTENT_TYPE_LEN - 2; /* for \r\n */
    char *type_buffer = RAPTOR_MALLOC(char*, len + 1);
    memcpy(type_buffer, (char*)ptr + 14, len);
    type_buffer[len]='\0';
    if(www->type)
      RAPTOR_FREE(char*, www->type);
    www->type = type_buffer;
    www->free_type = 1;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 2
    RAPTOR_DEBUG3("Got content type header '%s' (%d bytes)\n", type_buffer, len);
#endif
    if(www->content_type)
      www->content_type(www, www->content_type_userdata, www->type);
  }
  

#define CONTENT_LOCATION_LEN 18
  if(!raptor_strncasecmp((char*)ptr, "Content-Location: ", 
                         CONTENT_LOCATION_LEN)) {
    size_t uri_len = bytes - CONTENT_LOCATION_LEN - 2; /* for \r\n */
    unsigned char* uri_str = (unsigned char*)ptr + CONTENT_LOCATION_LEN;

    if(www->final_uri)
      raptor_free_uri(www->final_uri);

    /* Ensure it is NUL terminated */
    c = uri_str[uri_len];
    uri_str[uri_len] = '\0';
    www->final_uri = raptor_new_uri_relative_to_base_counted(www->world,
                                                             www->uri,
                                                             uri_str, uri_len);
    uri_str[uri_len] = c;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 2
    if(www->final_uri)
      RAPTOR_DEBUG2("Got content location header '%s'\n", 
                    raptor_uri_as_string(www->final_uri));
#endif
    if(www->final_uri_handler)
      www->final_uri_handler(www, www->final_uri_userdata, www->final_uri);
  }
  
  return bytes;
}


void
raptor_www_curl_init(raptor_www *www)
{
  if(!www->curl_handle) {
    www->curl_handle = curl_easy_init();
    www->curl_init_here = 1;
  }


#ifndef CURLOPT_WRITEDATA
#define CURLOPT_WRITEDATA CURLOPT_FILE
#endif

  /* send all data to this function  */
  curl_easy_setopt(www->curl_handle, CURLOPT_WRITEFUNCTION, 
                   raptor_www_curl_write_callback);
  /* ... using this data pointer */
  curl_easy_setopt(www->curl_handle, CURLOPT_WRITEDATA, www);


  /* send all headers to this function */
  curl_easy_setopt(www->curl_handle, CURLOPT_HEADERFUNCTION, 
                   raptor_www_curl_header_callback);
  /* ... using this data pointer */
  curl_easy_setopt(www->curl_handle, CURLOPT_WRITEHEADER, www);

  /* Make it follow Location: headers */
  curl_easy_setopt(www->curl_handle, CURLOPT_FOLLOWLOCATION, 1);

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 2
  curl_easy_setopt(www->curl_handle, CURLOPT_VERBOSE, (void*)1);
#endif

  curl_easy_setopt(www->curl_handle, CURLOPT_ERRORBUFFER, www->error_buffer);

  /* Connection timeout in seconds */
  curl_easy_setopt(www->curl_handle, CURLOPT_CONNECTTIMEOUT,
                   www->connection_timeout);
  curl_easy_setopt(www->curl_handle, CURLOPT_NOSIGNAL, 1);
}


void
raptor_www_curl_free(raptor_www *www)
{
    /* only tidy up if we did all the work */
  if(www->curl_init_here && www->curl_handle) {
    curl_easy_cleanup(www->curl_handle);
    www->curl_handle = NULL;
  }
}


int
raptor_www_curl_fetch(raptor_www *www) 
{
  struct curl_slist *slist = NULL;
    
  if(www->proxy)
    curl_easy_setopt(www->curl_handle, CURLOPT_PROXY, www->proxy);

  if(www->user_agent)
    curl_easy_setopt(www->curl_handle, CURLOPT_USERAGENT, www->user_agent);

  if(www->http_accept)
    slist = curl_slist_append(slist, (const char*)www->http_accept);

  /* ALWAYS disable curl default "Pragma: no-cache" */
  slist = curl_slist_append(slist, "Pragma:");
  if(www->cache_control)
    slist = curl_slist_append(slist, (const char*)www->cache_control);

  if(slist)
    curl_easy_setopt(www->curl_handle, CURLOPT_HTTPHEADER, slist);

  /* specify URL to get */
  curl_easy_setopt(www->curl_handle, CURLOPT_URL, 
                   raptor_uri_as_string(www->uri));

  if(curl_easy_perform(www->curl_handle)) {
    /* failed */
    www->failed = 1;
    raptor_www_error(www, "Resolving URI failed: %s", www->error_buffer);
  } else {
    long lstatus;

#ifndef CURLINFO_RESPONSE_CODE
#define CURLINFO_RESPONSE_CODE CURLINFO_HTTP_CODE
#endif

    /* Requires pointer to a long */
    if(curl_easy_getinfo(www->curl_handle, CURLINFO_RESPONSE_CODE, &lstatus) == CURLE_OK)
      /* CURL status code will always fit in an int */
      www->status_code = RAPTOR_GOOD_CAST(int, lstatus);

  }

  if(slist)
    curl_slist_free_all(slist);
  
  return www->failed;
}


int
raptor_www_curl_set_ssl_cert_options(raptor_www* www,
                                     const char* cert_filename,
                                     const char* cert_type,
                                     const char* cert_passphrase)
{
  /* client certificate file name */
  if(cert_filename)
    curl_easy_setopt(www->curl_handle, CURLOPT_SSLCERT, cert_filename);
  
  /* curl default is "PEM" */
  if(cert_type)
    curl_easy_setopt(www->curl_handle, CURLOPT_SSLCERTTYPE, cert_type);
  
  /* passphrase */
  /* Removed in 7.16.4 */
#if LIBCURL_VERSION_NUM < 0x071004
#define CURLOPT_KEYPASSWD CURLOPT_SSLKEYPASSWD
#endif
  if(cert_passphrase)
    curl_easy_setopt(www->curl_handle, CURLOPT_KEYPASSWD, cert_passphrase);

  return 0;
}


int
raptor_www_curl_set_ssl_verify_options(raptor_www* www, int verify_peer,
                                       int verify_host)
{
  if(verify_peer)
    verify_peer = 1;
  curl_easy_setopt(www->curl_handle, CURLOPT_SSL_VERIFYPEER, verify_peer);

  /* curl 7.28.1 removed the value 1 from being legal:
   * http://daniel.haxx.se/blog/2012/10/25/libcurl-claimed-to-be-dangerous/
   *
   * CURL GIT commit da82f59b697310229ccdf66104d5d65a44dfab98
   * Sat Oct 27 12:31:39 2012 +0200
   *
   * Legal values are:
   * 0 to disable host verifying
   * 2 (default) to enable host verifyinging
   */
  if(verify_host)
    verify_host = 2;
  curl_easy_setopt(www->curl_handle, CURLOPT_SSL_VERIFYHOST, verify_host);

  return 0;
}


#endif /* RAPTOR_WWW_LIBCURL */
