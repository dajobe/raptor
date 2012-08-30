/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_memstr.c - search for a string in a block of memory
 *
 * Copyright (C) 2008, David Beckett http://www.dajobe.org/
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

#include <string.h>

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


/*
 * raptor_memstr:
 * @haystack: memory block to search in
 * @haystack_len: size of memory block
 * @needle: string to search with
 *
 * INTERNAL: Search for a string in a block of memory
 *
 * The block of memory in @haystack may not be NUL terminated but
 * the searching for @needle will end if a NUL is found in @haystack.
 *
 * Return value: pointer to match string or NULL on failure or failed to find
 */
const char*
raptor_memstr(const char *haystack, size_t haystack_len, const char *needle)
{
  size_t needle_len;
  const char *p;
  
  if(!haystack || !needle)
    return NULL;
  
  if(!*needle)
    return haystack;
  
  needle_len = strlen(needle);

  /* loop invariant: haystack_len is always length of remaining buffer at *p */
  for(p = haystack;
      (haystack_len >= needle_len) && *p;
      p++, haystack_len--) {

    /* check match */
    if(!memcmp(p, needle, needle_len))
      return p;
  }
  
  return NULL;
}

    
