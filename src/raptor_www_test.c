/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_www_test.c - Raptor WWW retrieval test code
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
write_bytes_fh(void *userdata, const void *ptr, size_t size, size_t nmemb) 
{
  return fwrite(ptr, size, nmemb, (FILE*)userdata);
}


static void
write_content_type(void *userdata, const char *content_type) 
{
  fprintf((FILE*)userdata, "Content Type: %s\n", content_type);
}


int main (int argc, char *argv[]) 
{
  const char *url;
  raptor_www *www;
  const char *user_agent="raptor-www-test";
  
  if(argc>1)
    url=argv[1];
  else
    url="http://www.redland.opensource.ac.uk/";

  www=raptor_www_new();

  if(1) {
    raptor_www_set_userdata(www, stdout);
    www->content_type=write_content_type;
  } else {
    www->write_bytes=write_bytes_fh;
  }

  raptor_www_set_user_agent(www, user_agent);

  /* start retrieval (always a GET) */
  
  if(raptor_www_fetch(www, url)) {
    printf("Fetch failed\n");
  } else {
    printf("HTTP response status %d\n", www->status_code);
    
    printf("Returned %d bytes in body\n", www->total_bytes);
  }
  
  raptor_www_free(www);
  
  return 0;
}
