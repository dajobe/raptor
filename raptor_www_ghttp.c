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


#include <ghttp.h>


void
raptor_www_ghttp_init(raptor_www *www) 
{
  www->request = ghttp_request_new();
}


void
raptor_www_ghttp_free(raptor_www *www)
{
  /* Destroy the request.
   *
   * This closes any file descriptors that may be open and will free
   * any memory associated with the request. 
   */
  if(www->request) {
    ghttp_request_destroy(www->request);
    www->request=NULL;
  }
}


int
raptor_www_ghttp_fetch(raptor_www *www, const char *url) 
{
  
  while(1) { 
    /* Close the connection after you are done. */
    /* ghttp_set_header(www->request, http_hdr_Connection, "close"); */

    if(www->user_agent)
      ghttp_set_header(www->request, http_hdr_User_Agent, www->user_agent);

    /* Set the URI for the request object */
    if(ghttp_set_uri(www->request, (char*)url) <0) {
      raptor_www_error(www, "ghttp setup failed");
      return 1;
    }

    if(www->proxy)
      ghttp_set_proxy(www->request, www->proxy);
  

   /* Prepare the connection - hostname lookup and some other things */
    if(ghttp_prepare(www->request)) {
      raptor_www_error(www, "ghttp prepare failed");
      return 1;
    }

    /* Invoke the request */
    www->status=ghttp_process(www->request);
    
    www->status_code=ghttp_status_code(www->request);

    if(www->status != ghttp_done) {
      raptor_www_error(www, "ghttp fetch failed - %s",
                       ghttp_get_error(www->request));
      return 1;
    }

    if(www->status_code >= 200 && www->status_code <= 299)
      break;
    
    if(www->status_code == 301 || www->status_code == 302 ||
       www->status_code == 303 || www->status_code == 307) {
      url=ghttp_get_header(www->request, http_hdr_Location); 
      if(url) {
        ghttp_clean(www->request);
        continue;
      }
    }
    
    break;
  }
  


  www->type=(char*)ghttp_get_header(www->request, http_hdr_Content_Type);
  if(www->type) {
    if(www->content_type)
      www->content_type(www->userdata, www->type);
    www->free_type=0;
  }
  www->total_bytes=ghttp_get_body_len(www->request);
  /* Write out the body.
   *
   * Note that the body of the request may not be null terminated so
   * we have to be careful of the length.
   */
  if(www->write_bytes)
    www->write_bytes(www->userdata, 
                   ghttp_get_body(www->request), ghttp_get_body_len(www->request), 1);

  return 0;
}
