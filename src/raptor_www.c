/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_www.c - Raptor WWW retrieval core
 *
 * $Id$
 *
 * Copyright (C) 2003-2004 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology - http://www.ilrt.bris.ac.uk/
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
#include <stdarg.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"



static int raptor_www_file_fetch(raptor_www *www);


/* should raptor_www do initializing and cleanup of the WWW library */
static int raptor_www_do_www_init_finish=1;


#ifdef RAPTOR_WWW_LIBWWW
/* Non-zero only if Raptor initialised the W3C libwww library */
static int raptor_libwww_initialised=0;
#endif

void
raptor_www_init(void)
{
  static int initialized=0;
  if(initialized)
    return;

  if(raptor_www_do_www_init_finish) {
#ifdef RAPTOR_WWW_LIBCURL
    curl_global_init(CURL_GLOBAL_ALL);
#endif
#ifdef RAPTOR_WWW_LIBWWW
    if(!HTLib_isInitialized()) {
      raptor_libwww_initialised=1;
      HTLibInit(PACKAGE, VERSION);
    }
#endif
  }
  initialized=1;
}


/**
 * raptor_www_no_www_library_init_finish - Do not initialise or finish the lower level WWW library
 *
 * If this is called then the raptor_www library will neither
 * initialise or terminate the lower level WWW library.  Usually in
 * raptor_init either curl_global_init (for libcurl) or HTLibInit
 * (for w3c libwww) are called and in raptor_finish either
 * curl_global_cleanup or HTLibTerminate are called.
 *
 * This allows the application finer control over these libraries such
 * as setting other global options or potentially calling and terminating
 * raptor several times.  It does mean that applications which use
 * this call must do their own extra work in order to allocate and free
 * all resources to the system.
 * 
 * This function must be called before raptor_init.
 *
 **/
void
raptor_www_no_www_library_init_finish(void)
{
  raptor_www_do_www_init_finish=0;
}


void
raptor_www_finish(void)
{
  if(raptor_www_do_www_init_finish) {
#ifdef RAPTOR_WWW_LIBCURL
    curl_global_cleanup();
#endif

#ifdef RAPTOR_WWW_LIBWWW
    if(raptor_libwww_initialised)
      HTLibTerminate();
#endif
  }
}



raptor_www *
raptor_www_new_with_connection(void *connection)
{
  raptor_www *www=(raptor_www *)RAPTOR_CALLOC(www, 1, sizeof(raptor_www));
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
  www->curl_handle=(CURL*)connection;
  raptor_www_curl_init(www);
#endif
#ifdef RAPTOR_WWW_LIBXML
  raptor_www_libxml_init(www);
#endif
#ifdef RAPTOR_WWW_LIBWWW
  raptor_www_libwww_init(www);
#endif
#ifdef RAPTOR_WWW_LIBFETCH
  raptor_www_libfetch_init(www);
#endif

  return www;
}

raptor_www*
raptor_www_new(void)
{
  return raptor_www_new_with_connection(NULL);
}


void
raptor_www_free(raptor_www *www)
{
  /* free context */
  if(www->type) {
    if(www->free_type)
      RAPTOR_FREE(cstring, www->type);
    www->type=NULL;
  }
  
  if(www->user_agent) {
    RAPTOR_FREE(cstring, www->user_agent);
    www->user_agent=NULL;
  }

  if(www->proxy) {
    RAPTOR_FREE(cstring, www->proxy);
    www->proxy=NULL;
  }

  if(www->http_accept) {
    RAPTOR_FREE(cstring, www->http_accept);
    www->http_accept=NULL;
  }

#ifdef RAPTOR_WWW_LIBCURL
  raptor_www_curl_free(www);
#endif
#ifdef RAPTOR_WWW_LIBXML
  raptor_www_libxml_free(www);
#endif
#ifdef RAPTOR_WWW_LIBWWW
  raptor_www_libwww_free(www);
#endif
#ifdef RAPTOR_WWW_LIBFETCH
  raptor_www_libfetch_free(www);
#endif

  if(www->uri)
    raptor_free_uri(www->uri);

  RAPTOR_FREE(www, www);
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
  char *ua_copy=(char*)RAPTOR_MALLOC(cstring, strlen(user_agent)+1);
  if(!ua_copy)
    return;
  strcpy(ua_copy, user_agent);
  
  www->user_agent=ua_copy;
}


void
raptor_www_set_proxy(raptor_www *www, const char *proxy)
{
  char *proxy_copy=(char*)RAPTOR_MALLOC(cstring, strlen(proxy)+1);
  if(!proxy_copy)
    return;
  strcpy(proxy_copy, proxy);
  
  www->proxy=proxy_copy;
}


/**
 * raptor_www_set_http_accept - Set HTTP Accept header
 * @www: &raptor_www class
 * @value: Accept: header value or NULL to have an empty one.
 * 
 **/
void
raptor_www_set_http_accept(raptor_www *www, const char *value)
{
  char *value_copy;
  size_t len=8; /* strlen("Accept:")+1 */
  
  if(value)
    len+=1+strlen(value); /* " "+value */
  
  value_copy=(char*)RAPTOR_MALLOC(cstring, len);
  if(!value_copy)
    return;
  www->http_accept=value_copy;

  strcpy(value_copy, "Accept:");
  value_copy+=7;
  if(value) {
    *value_copy++=' ';
    strcpy(value_copy, value);
  }
}


/**
 * raptor_www_get_connection - Get internal WWW library connection object
 * @www: &raptor_www object 
 * 
 * Return value: connection object or NULL
 **/
void*
raptor_www_get_connection(raptor_www *www) 
{
#ifdef RAPTOR_WWW_NONE
  return NULL;
#endif

#ifdef RAPTOR_WWW_LIBCURL
  return www->curl_handle;
#endif

#ifdef RAPTOR_WWW_LIBXML
  return www->ctxt;
#endif

#ifdef RAPTOR_WWW_LIBWWW
  return NULL;
#endif

#ifdef RAPTOR_WWW_LIBFETCH
  return NULL;
#endif

  return NULL;
}


void
raptor_www_abort(raptor_www *www, const char *reason) {
  www->failed=1;
}


void
raptor_www_error_varargs(raptor_www *www, const char *message, va_list arguments)
{
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
}

  
void
raptor_www_error(raptor_www *www, const char *message, ...) 
{
  va_list arguments;

  va_start(arguments, message);
  raptor_www_error_varargs(www, message, arguments);
  va_end(arguments);
}

  
static int 
raptor_www_file_fetch(raptor_www *www) 
{
  char *filename;
  FILE *fh;
  unsigned char buffer[RAPTOR_WWW_BUFFER_SIZE];
  unsigned char *uri_string=raptor_uri_as_string(www->uri);
#if defined(HAVE_UNISTD_H) && defined(HAVE_SYS_STAT_H)
  struct stat buf;
#endif
  
  filename=raptor_uri_uri_string_to_filename(uri_string);
  if(!filename) {
    raptor_www_error(www, "Not a file: URI");
    return 1;
  }

#if defined(HAVE_UNISTD_H) && defined(HAVE_SYS_STAT_H)
  if(!stat(filename, &buf) && S_ISDIR(buf.st_mode)) {
    raptor_www_error(www, "Cannot read from a directory '%s'", filename);
    RAPTOR_FREE(cstring, filename);
    www->status_code=404;
    return 1;
  }
#endif

  fh=fopen(filename, "rb");
  if(!fh) {
    raptor_www_error(www, "file '%s' open failed - %s",
                     filename, strerror(errno));
    RAPTOR_FREE(cstring, filename);
    www->status_code=404;
    return 1;
  }

  while(!feof(fh)) {
    int len=fread(buffer, 1, RAPTOR_WWW_BUFFER_SIZE, fh);
    www->total_bytes += len;

    if(len > 0 && www->write_bytes)
      www->write_bytes(www, www->write_bytes_userdata, buffer, len, 1);

    if(feof(fh) || www->failed)
      break;
  }
  fclose(fh);

  RAPTOR_FREE(cstring, filename);
  
  if(!www->failed)
    www->status_code=200;
  
  return www->failed;
}


int
raptor_www_fetch(raptor_www *www, raptor_uri *uri) 
{
  www->uri=raptor_new_uri_for_retrieval(uri);
  
  www->locator.uri=uri;
  www->locator.line= -1;
  www->locator.column= -1;

#ifdef RAPTOR_WWW_NONE
  return raptor_www_file_fetch(www);
#else

  if(raptor_uri_is_file_uri(raptor_uri_as_string(www->uri)))
    return raptor_www_file_fetch(www);

#ifdef RAPTOR_WWW_LIBCURL
  return raptor_www_curl_fetch(www);
#endif

#ifdef RAPTOR_WWW_LIBXML
  return raptor_www_libxml_fetch(www);
#endif

#ifdef RAPTOR_WWW_LIBWWW
  return raptor_www_libwww_fetch(www);
#endif

#ifdef RAPTOR_WWW_LIBFETCH
  return raptor_www_libfetch_fetch(www);
#endif

#endif

  return 1; /* Default to failed */
}
