/**
 * Copyright 2008 Digital Bazaar, Inc.
 *
 * This file is part of librdfa.
 * 
 * librdfa is Free Software, and can be licensed under any of the
 * following three licenses:
 * 
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any 
 *      newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 * 
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 * 
 * See LICENSE-* at the top of this software distribution for more
 * information regarding the details of each license.
 *
 * The iri module is used to process IRIs.
 */
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rdfa.h"

/**
 * Strips the iquery and ifragment part from an IRI. This leaves just the
 * scheme and the ihier-part, as defined in RFC 3987. This function will
 * copy the input string and return a new string that must be free()'d. 
 *
 * @param iri the IRI that should be stripped of anything after the iquery
 *            and fragment, if they exist.
 */
char* rdfa_iri_get_base(const char* iri)
{
   char* rval = NULL;
   const char* eindex = 0;

   /* search to see if there is iquery separator */
   eindex = strchr(iri, '?');
   if(eindex == NULL)
   {
      /* if there is no iquery separator, check to see if there is an
       * ifragment separator */
      eindex = strchr(iri, '#');
   }

   /* check to see if the output string needs to be different from the
    * input string */
   if(eindex == NULL)
   {
      /* there was no iquery or ifragment in the input string, so there is
       * no need to reformat the string */
      rval = strdup(iri);
   }
   else
   {
      /* the output string should be concatenated */
      unsigned int length = (unsigned int)(eindex - iri);
      rval = (char*)malloc(length + 1);
      rval = strncpy(rval, iri, length);
      rval[length] = '\0';
   }

   return rval;
}
