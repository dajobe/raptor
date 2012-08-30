/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_syntax_description.c - Raptor syntax description API
 *
 * Copyright (C) 2010, David Beckett http://www.dajobe.org/
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

#include <stdio.h>
#include <string.h>

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


static unsigned int
count_strings_array(const char* const * array) 
{
  unsigned int i;
  
  if(!array)
    return 0;
  
  for(i = 0; (array[i]); i++)
    ;

  return i;
}


static unsigned int
count_mime_types_array(const raptor_type_q* array)
{
  unsigned int i;
  
  if(!array)
    return 0;
  
  for(i = 0; (array[i].mime_type); i++)
    ;

  return i;
}


/**
 * raptor_syntax_description_validate:
 * @desc: description
 *
 * Validate a syntax description has the required fields (name, labels) and update counts
 * 
 * Returns: non-0 on failure
 **/
int
raptor_syntax_description_validate(raptor_syntax_description* desc)
{
  if(!desc || !desc->names || !desc->names[0] || !desc->label)
    return 1;

#ifdef RAPTOR_DEBUG
  /* Maintainer only check of static data */
  if(desc->mime_types) {
    unsigned int i;
    const raptor_type_q* type_q = NULL;

    for(i = 0; 
        (type_q = &desc->mime_types[i]) && type_q->mime_type;
        i++) {
      size_t len = strlen(type_q->mime_type);
      if(len != type_q->mime_type_len) {
        fprintf(stderr,
                "Format %s  mime type %s  actual len %d  static len %d\n",
                desc->names[0], type_q->mime_type,
                (int)len, (int)type_q->mime_type_len);
      }
    }
  }
#endif

  desc->names_count = count_strings_array(desc->names);
  if(!desc->names_count)
    return 1;
  
  desc->mime_types_count = count_mime_types_array(desc->mime_types);
  desc->uri_strings_count = count_strings_array(desc->uri_strings);

  return 0;
}
