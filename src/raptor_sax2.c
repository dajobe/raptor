/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_sax2.c - Raptor SAX2 API
 *
 * $Id$
 *
 * Copyright (C) 2000-2003 David Beckett - http://purl.org/net/dajobe/
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
#include <ctype.h>
#include <stdarg.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


/* Define this for far too much output */
#undef RAPTOR_DEBUG_CDATA





raptor_sax2_element*
raptor_sax2_element_pop(raptor_sax2 *sax2) 
{
  raptor_sax2_element *element=sax2->current_element;

  if(!element)
    return NULL;

  sax2->current_element=element->parent;
  if(sax2->root_element == element) /* just deleted root */
    sax2->root_element=NULL;

  return element;
}


void
raptor_sax2_element_push(raptor_sax2 *sax2, raptor_sax2_element* element) 
{
  element->parent=sax2->current_element;
  sax2->current_element=element;
  if(!sax2->root_element)
    sax2->root_element=element;
}


void
raptor_free_sax2_element(raptor_sax2_element *element)
{
  int i;

  for (i=0; i < element->attribute_count; i++)
    if(element->attributes[i])
      raptor_free_qname(element->attributes[i]);

  if(element->attributes)
    RAPTOR_FREE(raptor_qname_array, element->attributes);

  if(element->content_cdata_length)
    RAPTOR_FREE(raptor_qname_array, element->content_cdata);

  if(element->base_uri)
    raptor_free_uri(element->base_uri);

  if(element->xml_language)
    RAPTOR_FREE(cstring, element->xml_language);

  raptor_free_qname(element->name);
  RAPTOR_FREE(raptor_element, element);
}


#ifdef RAPTOR_DEBUG
void
raptor_print_sax2_element(raptor_sax2_element *element, FILE* stream)
{
  raptor_qname_print(stream, element->name);
  fputc('\n', stream);

  if(element->attribute_count) {
    int i;

    fputs(" attributes: ", stream);
    for (i = 0; i < element->attribute_count; i++) {
      if(i)
        fputc(' ', stream);
      raptor_qname_print(stream, element->attributes[i]);
      fprintf(stream, "='%s'", element->attributes[i]->value);
    }
    fputc('\n', stream);
  }
}
#endif


char *
raptor_format_sax2_element(raptor_sax2_element *element,
                           int *length_p, int is_end,
                           raptor_message_handler error_handler,
                           void *error_data)
{
  int length;
  char *buffer;
  char *ptr;
  int i;

  /* get length of element name (and namespace-prefix: if there is one) */
  length=element->name->local_name_length + 1; /* < */
  if(element->name->nspace &&
     element->name->nspace->prefix_length > 0)
    length += element->name->nspace->prefix_length + 1; /* : */

  if(is_end)
    length++; /* / */

  if (!is_end && element->attributes) {
    for(i=0; i < element->attribute_count; i++) {
      size_t escaped_attr_val_len;
      
      length++; /* ' ' between attributes and after element name */

      /* qname */
      length += element->attributes[i]->local_name_length;
      if(element->attributes[i]->nspace &&
         element->attributes[i]->nspace->prefix_length > 0)
         /* prefix: */
        length += element->attributes[i]->nspace->prefix_length + 1;

      /* XML escaped value */
      escaped_attr_val_len=raptor_xml_escape_string(element->attributes[i]->value, element->attributes[i]->value_length,
                                                    NULL, 0, '"',
                                                    error_handler, error_data);
      /* ="XML-escaped-value" */
      length += 3+escaped_attr_val_len;
    }
  }
  
  length++; /* > */
  
  if(length_p)
    *length_p=length;

  /* +1 here is for \0 at end */
  buffer=(char*)RAPTOR_MALLOC(cstring, length + 1);
  if(!buffer)
    return NULL;

  ptr=buffer;

  *ptr++ = '<';
  if(is_end)
    *ptr++ = '/';
  if(element->name->nspace && element->name->nspace->prefix_length > 0) {
    strncpy(ptr, (char*)element->name->nspace->prefix,
            element->name->nspace->prefix_length);
    ptr+= element->name->nspace->prefix_length;
    *ptr++=':';
  }
  strcpy(ptr, (char*)element->name->local_name);
  ptr += element->name->local_name_length;

  if(!is_end && element->attributes) {
    for(i=0; i < element->attribute_count; i++) {
      size_t escaped_attr_val_len;

      *ptr++ =' ';
      
      if(element->attributes[i]->nspace && 
         element->attributes[i]->nspace->prefix_length > 0) {
        strncpy(ptr, (char*)element->attributes[i]->nspace->prefix,
                element->attributes[i]->nspace->prefix_length);
        ptr+= element->attributes[i]->nspace->prefix_length;
        *ptr++=':';
      }
    
      strcpy(ptr, (char*)element->attributes[i]->local_name);
      ptr += element->attributes[i]->local_name_length;
      
      *ptr++ ='=';
      *ptr++ ='"';
      
      escaped_attr_val_len=raptor_xml_escape_string(element->attributes[i]->value, element->attributes[i]->value_length,
                                                    NULL, 0, '"',
                                                    error_handler, error_data);
      if(escaped_attr_val_len == element->attributes[i]->value_length) {
        /* save a malloc/free when there is no escaping */
        strcpy(ptr, element->attributes[i]->value);
        ptr += element->attributes[i]->value_length;
      } else {
        unsigned char *escaped_attr_val=(char*)RAPTOR_MALLOC(cstring,escaped_attr_val_len+1);
        raptor_xml_escape_string(element->attributes[i]->value, element->attributes[i]->value_length,
                                 escaped_attr_val, escaped_attr_val_len, '"',
                                 error_handler, error_data);
        
        strcpy(ptr, escaped_attr_val);
        RAPTOR_FREE(cstring,escaped_attr_val);
        ptr += escaped_attr_val_len;
      }
      *ptr++ ='"';
    }
  }
  
  *ptr++ = '>';
  *ptr='\0';

  return buffer;
}



