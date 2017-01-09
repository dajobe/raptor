/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * strdup.c - strdup compatibility
 *
 * This file is in the public domain.
 * 
 */

#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#include <string.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* for RAPTOR_MALLOC() */
#include "raptor2.h"
#include "raptor_internal.h"


char*
raptor_strdup(const char* s)
{
  size_t len;
  char *buf;
  
  if(!s)
    return NULL;
  
  len = strlen(s) + 1;
  buf = RAPTOR_MALLOC(char*, len);
  if(buf)
    memcpy(buf, s, len);
  return buf;
}
