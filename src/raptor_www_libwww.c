/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_www_libwww.c - Raptor WWW retrieval via W3C libwww
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
#include <stdarg.h>

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


/* libwww includes */
#undef PACKAGE
#undef VERSION
#include "WWWLib.h"
#include "WWWInit.h"


void
raptor_www_libwww_init(raptor_www *www)
{
}


void
raptor_www_libwww_free(raptor_www *www)
{
}


int
raptor_www_libwww_fetch(raptor_www *www) 
{
  HTRequest* request;
  HTChunk* chunk;
  BOOL status = NO;
  char *uri_string=raptor_uri_as_string(www->uri);
  
  /* FIXME */
  RAPTOR_FATAL1(raptor_www_libwww_fetch, "Not working - pleas use another www library for now\n");

  if(www->user_agent)
    HTLib_setAppName(www->user_agent);

  HTEventInit();

  request = HTRequest_new();
  if(!request)
    return 1;
  
  /* Start the GET request */
  chunk = HTLoadToChunk (uri_string, request);
  if (status == NO) {
    HTRequest_delete(request);
    return 1;
  }

  HTChunk_delete(chunk);
  HTRequest_delete(request);

  if(0) { /**/
    raptor_www_error(www, "Failed");
    return 1;
  }
  return 0;
}
