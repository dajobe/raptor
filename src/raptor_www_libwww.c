/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_www_libwww.c - Raptor WWW retrieval via W3C libwww
 *
 * Copyright (C) 2003-2006, David Beckett http://purl.org/net/dajobe/
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

#ifdef WIN32
#include <win32_raptor_config.h>
#endif

#ifdef RAPTOR_WWW_LIBWWW

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

  if(0) { /* FIXME */
    raptor_www_error(www, "Failed");
    return 1;
  }
  return 0;
}

#endif /* RAPTOR_WWW_LIBWWW */
