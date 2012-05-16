/* This file is in the public domain */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include "strtok_r.h"

#ifdef NEED_RDFA_STRTOK_R

char *
rdfa_strtok_r(char *str, const char *delim, char **saveptr)
{
   char *p;

   if (str == NULL)
      str = *saveptr;

   if (str == NULL)
      return NULL;

   while (*str && strchr(delim, *str))
      str++;

   if (*str == '\0')
   {
      *saveptr = NULL;
      return NULL;
   }

   p = str;
   while (*p && !strchr(delim, *p))
      p++;

   if (*p == '\0')
      *saveptr = NULL;
   else
   {
      *p = '\0';
      p++;
      *saveptr = p;
   }

   return str;
}

#else /* ! NEED_RDFA_STRTOK_R */

typedef int blah; /* "ISO C forbids an empty translation unit" */

#endif /* NEED_RDFA_STRTOK_R */
