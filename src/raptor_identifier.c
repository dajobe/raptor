/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_identifier.c - Raptor identifier classes
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
#include <ctype.h>
#include <stdarg.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


/**
 * raptor_new_identifier - Constructor - create a raptor_identifier
 * @type: raptor_identifier_type of identifier
 * @uri: &raptor_uri of identifier (if relevant) (SHARED)
 * @uri_source: raptor_uri_source of URI (if relevant)
 * @id: string for ID or genid (if relevant) (SHARED)
 * @literal: string for literal (SHARED)
 * @literal_datatype: &raptor_uri of identifier (SHARED)
 * @literal_language: literal language (SHARED)
 * 
 * Constructs a new identifier copying the URI, ID fields.
 * SHARED means raptor_new_identifier owns this argument after calling.
 * 
 * Return value: a new raptor_identifier object or NULL on failure
 **/
raptor_identifier*
raptor_new_identifier(raptor_identifier_type type,
                      raptor_uri *uri,
                      raptor_uri_source uri_source,
                      const unsigned char *id,
                      const unsigned char *literal,
                      raptor_uri *literal_datatype,
                      const unsigned char *literal_language)
{
  raptor_identifier *identifier;

  identifier=(raptor_identifier*)RAPTOR_CALLOC(raptor_identifier, 1,
                                               sizeof(raptor_identifier));
  if(!identifier)
    return NULL;

  identifier->type=type;
  identifier->is_malloced=1;

  /* SHARED */
  identifier->uri=uri;
  identifier->id=id;
  identifier->literal=literal;
  identifier->literal_datatype=literal_datatype;
  identifier->literal_language=literal_language;
  
  return identifier;
}


/**
 * raptor_copy_identifier - Copy raptor_identifiers
 * @dest: destination &raptor_identifier (previously created)
 * @src:  source &raptor_identifier
 * 
 * Return value: Non 0 on failure
 **/
int
raptor_copy_identifier(raptor_identifier *dest, raptor_identifier *src)
{
  raptor_free_identifier(dest);

  dest->type=src->type;
  dest->uri_source=src->uri_source;
  dest->ordinal=src->ordinal;

  if(src->uri) {
    dest->uri=raptor_uri_copy(src->uri);
    if(!dest->uri)
      return 0;
  }

  if(src->id) {
    int len=strlen((char*)src->id);
    
    dest->id=(unsigned char*)RAPTOR_MALLOC(cstring, len+1);
    if(!dest->id) {
      raptor_free_identifier(dest);
      return 0;
    }
    strncpy((char*)dest->id, (char*)src->id, len+1);
  }

  if(src->literal) {
    int len=strlen((char*)src->literal);
    
    dest->literal_language=(unsigned char*)RAPTOR_MALLOC(cstring, len+1);
    if(!dest->literal_language) {
      raptor_free_identifier(dest);
      return 0;
    }
    strncpy((char*)dest->literal_language, (char*)src->literal_language, len+1);
  }

  if(src->literal_datatype) {
    dest->literal_datatype=raptor_uri_copy(src->literal_datatype);
    if(!dest->literal_datatype) {
      raptor_free_identifier(dest);
      return 0;
    }
  }

  if(src->literal_language) {
    int len=strlen((char*)src->literal_language);
    
    dest->literal_language=(unsigned char*)RAPTOR_MALLOC(cstring, len+1);
    if(!dest->literal_language) {
      raptor_free_identifier(dest);
      return 0;
    }
    strncpy((char*)dest->literal_language, (char*)src->literal_language, len+1);
  }

  return 0;
}


/**
 * raptor_free_identifier - Destructor - destroy a raptor_identifier object
 * @identifier: &raptor_identifier object
 *
 **/
void
raptor_free_identifier(raptor_identifier *identifier) 
{
  if(identifier->uri) {
    raptor_free_uri(identifier->uri);
    identifier->uri=NULL;
  }

  if(identifier->id) {
    RAPTOR_FREE(cstring, (void*)identifier->id);
    identifier->id=NULL;
  }

  if(identifier->literal) {
    RAPTOR_FREE(cstring, identifier->literal);
    identifier->literal=NULL;
  }

  if(identifier->literal_datatype) {
    raptor_free_uri(identifier->literal_datatype);
    identifier->literal_datatype=NULL;
  }

  if(identifier->literal_language) {
    RAPTOR_FREE(cstring, identifier->literal_language);
    identifier->literal_language=NULL;
  }

  if(identifier->is_malloced)
    RAPTOR_FREE(identifier, (void*)identifier);
}


#ifdef RAPTOR_DEBUG
void
raptor_identifier_print(FILE *stream, raptor_identifier *identifier)
{
  if(!identifier) {
    fputs("-", stream);
    return;
  }
  
  if(identifier->type == RAPTOR_IDENTIFIER_TYPE_LITERAL || 
     identifier->type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL) {
    fputc('"', stream);
    fputs((const char*)identifier->literal, stream);
    fputc('"', stream);
    if(identifier->literal_language)
      fprintf(stream, "@%s", identifier->literal_language);
    if(identifier->type == RAPTOR_IDENTIFIER_TYPE_XML_LITERAL) {
      fputs("<http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral>", stream);
    } else if(identifier->literal_datatype) {
      fputc('<', stream);
      fputs(raptor_uri_as_string(identifier->literal_datatype), stream);
      fputc('>', stream);
    }
  } else if(identifier->type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
    fputs((const char*)identifier->id, stream);
  else if(identifier->type == RAPTOR_IDENTIFIER_TYPE_ORDINAL)
    fprintf(stream, "[rdf:_%d]", identifier->ordinal);
  else {
    fprintf(stream, "%s", raptor_uri_as_string(identifier->uri));
  }
}
#endif
