/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * strcasecmp.c - strcasecmp compatibility
 *
 * $Id$
 *
 * This file is in the public domain.
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
#include <ctype.h>

int raptor_strcasecmp(const char* s1, const char* s2);


int
raptor_strcasecmp(const char* s1, const char* s2)
{
  register int c1, c2;
  
  while(*s1 && *s2) {
    c1 = tolower(*s1);
    c2 = tolower(*s2);
    if (c1 != c2)
      return (c1 - c2);
    s1++;
    s2++;
  }
  return (int) (*s1 - *s2);
}
