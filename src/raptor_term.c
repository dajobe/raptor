/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_term.c - Raptor terms
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
 */

#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#ifdef WIN32
#include <win32_raptor_config.h>
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
 * raptor_new_term_from_uri:
 * @world: raptor world
 * @uri: uri
 *
 * Constructor - create a new URI statement term
 *
 * Takes a copy (reference) of the passed in @uri
 *
 * Return value: new term or NULL on failure
*/
raptor_term*
raptor_new_term_from_uri(raptor_world* world, raptor_uri* uri)
{
  raptor_term *t;

  if(!uri)
    return NULL;
  
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, NULL);

  raptor_world_open(world);

  t = (raptor_term*)RAPTOR_CALLOC(raptor_term, 1, sizeof(*t));
  if(!t)
    return NULL;

  t->usage = 1;
  t->world = world;
  t->type = RAPTOR_TERM_TYPE_URI;
  t->value.uri = raptor_uri_copy(uri);

  return t;
}


/**
 * raptor_new_term_from_counted_literal:
 * @world: raptor world
 * @literal: literal data (or NULL for empty literal)
 * @literal_len: length of literal
 * @datatype: literal datatype URI (or NULL)
 * @language: literal language (or NULL for no language)
 * @language_len: literal language length
 *
 * Constructor - create a new literal statement term
 *
 * Takes copies of the passed in @literal, @datatype, @language
 *
 * Only one of @language or @datatype may be given.  If both are
 * given, NULL is returned.  If @language is the empty string, it is
 * the equivalent to NULL.
 *
 * Return value: new term or NULL on failure
*/
raptor_term*
raptor_new_term_from_counted_literal(raptor_world* world,
                                     const unsigned char* literal,
                                     size_t literal_len,
                                     raptor_uri* datatype,
                                     const unsigned char* language,
                                     unsigned char language_len)
{
  raptor_term *t;
  unsigned char* new_literal = NULL;
  unsigned char* new_language = NULL;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, NULL);

  raptor_world_open(world);

  if(language && !*language)
    language = NULL;

  if(language && datatype)
    return NULL;
  

  new_literal = (unsigned char*)RAPTOR_MALLOC(cstring, literal_len + 1);
  if(!new_literal)
    return NULL;

  if(!literal || !*literal)
    literal_len = 0;

  if(literal_len) {
    memcpy(new_literal, literal, literal_len);
    new_literal[literal_len] = '\0';
  } else
    *new_literal = '\0';

  if(language) {
    new_language = (unsigned char*)RAPTOR_MALLOC(cstring, language_len + 1);
    if(!new_language) {
      RAPTOR_FREE(cstring, new_literal);
      return NULL;
    }
    memcpy(new_language, language, language_len);
    new_language[language_len] = '\0';
  } else
    language_len = 0;

  if(datatype)
    datatype = raptor_uri_copy(datatype);
  

  t = (raptor_term*)RAPTOR_CALLOC(raptor_term, 1, sizeof(*t));
  if(!t) {
    if(new_literal)
      RAPTOR_FREE(cstring, new_literal);
    if(new_language)
      RAPTOR_FREE(cstring, new_language);
    if(datatype)
      raptor_free_uri(datatype);
    return NULL;
  }
  t->usage = 1;
  t->world = world;
  t->type = RAPTOR_TERM_TYPE_LITERAL;
  t->value.literal.string = new_literal;
  t->value.literal.string_len = literal_len;
  t->value.literal.language = new_language;
  t->value.literal.language_len = language_len;
  t->value.literal.datatype = datatype;

  return t;
}


/**
 * raptor_new_term_from_literal:
 * @world: raptor world
 * @literal: literal data (or NULL for empty literal)
 * @datatype: literal datatype URI (or NULL)
 * @language: literal language (or NULL)
 *
 * Constructor - create a new literal statement term
 *
 * Takes copies of the passed in @literal, @datatype, @language
 *
 * Only one of @language or @datatype may be given.  If both are
 * given, NULL is returned.  If @language is the empty string, it is
 * the equivalent to NULL.
 *
 * Return value: new term or NULL on failure
*/
raptor_term*
raptor_new_term_from_literal(raptor_world* world,
                             const unsigned char* literal,
                             raptor_uri* datatype,
                             const unsigned char* language)
{
  size_t literal_len = 0;
  unsigned char language_len = 0;
  
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, NULL);

  raptor_world_open(world);

  if(literal)
    literal_len = strlen((const char*)literal);

  if(language)
    language_len = strlen((const char*)language);
  
  return raptor_new_term_from_counted_literal(world, literal, literal_len,
                                              datatype, language, language_len);
}


/**
 * raptor_new_term_from_counted_blank:
 * @world: raptor world
 * @blank: blank node identifier
 * @length: length of idnetifier
 *
 * Constructor - create a new blank node statement term from counted string ID
 *
 * Takes a copy of the passed in @blank
 *
 * Return value: new term or NULL on failure
*/
raptor_term*
raptor_new_term_from_counted_blank(raptor_world* world,
                                   const unsigned char* blank, size_t length)
{
  raptor_term *t;
  unsigned char* new_id;

  if(!blank)
    return NULL;
  
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, NULL);

  raptor_world_open(world);

  new_id = (unsigned char*)RAPTOR_MALLOC(cstring, length + 1);
  if(!new_id)
    return NULL;

  memcpy(new_id, blank, length);
  new_id[length] = '\0';

  t = (raptor_term*)RAPTOR_CALLOC(raptor_term, 1, sizeof(*t));
  if(!t) {
    RAPTOR_FREE(cstring, new_id);
    return NULL;
  }

  t->usage = 1;
  t->world = world;
  t->type = RAPTOR_TERM_TYPE_BLANK;
  t->value.blank.string = new_id;
  t->value.blank.string_len = length;

  return t;
}


/**
 * raptor_new_term_from_blank:
 * @world: raptor world
 * @blank: blank node identifier
 *
 * Constructor - create a new blank node statement term
 *
 * Takes a copy of the passed in @blank
 *
 * Return value: new term or NULL on failure
*/
raptor_term*
raptor_new_term_from_blank(raptor_world* world, const unsigned char* blank)
{
  size_t length;
  
  if(!blank)
    return NULL;
  
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, NULL);

  raptor_world_open(world);

  length = strlen((const char*)blank);

  return raptor_new_term_from_counted_blank(world, blank, length);
}


/**
 * raptor_term_copy:
 * @term: raptor term
 *
 * Copy constructor - get a copy of a statement term
 *
 * Return value: new term object or NULL on failure
 */
raptor_term*
raptor_term_copy(raptor_term* term)
{
  if(!term)
    return NULL;

  term->usage++;
  return term;
}


/**
 * raptor_free_term:
 * @term: #raptor_term object
 *
 * Destructor - destroy a raptor_term object.
 *
 **/
void
raptor_free_term(raptor_term *term)
{
  if(!term)
    return;
  
  if(--term->usage)
    return;
  
  switch(term->type) {
    case RAPTOR_TERM_TYPE_URI:
      if(term->value.uri) {
        raptor_free_uri(term->value.uri);
        term->value.uri = NULL;
      }
      break;

    case RAPTOR_TERM_TYPE_BLANK:
      if(term->value.blank.string) {
        RAPTOR_FREE(cstring, (void*)term->value.blank.string);
        term->value.blank.string = NULL;
      }
      break;
      
    case RAPTOR_TERM_TYPE_LITERAL:
      if(term->value.literal.string) {
        RAPTOR_FREE(cstring, (void*)term->value.literal.string);
        term->value.literal.string = NULL;
      }

      if(term->value.literal.datatype) {
        raptor_free_uri(term->value.literal.datatype);
        term->value.literal.datatype = NULL;
      }
      
      if(term->value.literal.language) {
        RAPTOR_FREE(cstring, (void*)term->value.literal.language);
        term->value.literal.language = NULL;
      }
      break;
      
    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
      break;
  }

  RAPTOR_FREE(term, (void*)term);
}


/**
 * raptor_term_as_counted_string:
 * @term: #raptor_term
 * @len_p: Pointer to location to store length of new string (if not NULL)
 *
 * Turns part of raptor term into a N-Triples format counted string.
 * 
 * Turns the given @term into an N-Triples escaped string using all the
 * escapes as defined in http://www.w3.org/TR/rdf-testcases/#ntriples
 *
 * This function uses raptor_term_ntriples_write() to write to an
 * #raptor_iostream which is the prefered way to write formatted
 * output.
 *
 * Return value: the new string or NULL on failure.  The length of
 * the new string is returned in *@len_p if len_p is not NULL.
 **/
unsigned char*
raptor_term_as_counted_string(raptor_term *term, size_t* len_p)
{
  raptor_iostream *iostr;
  void *string = NULL;
  int rc;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(term, raptor_term, NULL);
  
  iostr = raptor_new_iostream_to_string(term->world, 
                                        &string, len_p, NULL);
  if(!iostr)
    return NULL;
  rc = raptor_term_ntriples_write(term, iostr);
  raptor_free_iostream(iostr);
  
  if(rc) {
    if(string) {
      RAPTOR_FREE(cstring, string);
      string = NULL;
    }
  }

  return (unsigned char *)string;
}


/**
 * raptor_term_as_string:
 * @term: #raptor_term
 *
 * Turns part of raptor statement into a N-Triples format string.
 * 
 * Turns the given @term into an N-Triples escaped string using all the
 * escapes as defined in http://www.w3.org/TR/rdf-testcases/#ntriples
 *
 * Return value: the new string or NULL on failure.
 **/
unsigned char*
raptor_term_as_string(raptor_term *term)
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(term, raptor_term, NULL);
  
  return raptor_term_as_counted_string(term, NULL);
}


int
raptor_term_print_as_ntriples(const raptor_term *term, FILE* stream)
{
  int rc = 0;
  raptor_iostream* iostr;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(term, raptor_term, 1);
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(stream, FILE*, 1);
  
  iostr = raptor_new_iostream_to_file_handle(term->world, stream);
  if(!iostr)
    return 1;
  
  rc = raptor_term_ntriples_write(term, iostr);

  raptor_free_iostream(iostr);
  
  return rc;
}


/**
 * raptor_term_equals:
 * @t1: first term
 * @t2: second term
 *
 * Compare a pair of #raptor_term for equality
 *
 * Return value: non-0 if the terms are equal
 */
int
raptor_term_equals(raptor_term* t1, raptor_term* t2)
{
  int d = 0;

  if(!t1 || !t2)
    return 0;
  
  if(t1->type != t2->type)
    return 0;
  
  if(t1 == t2)
    return 1;
  
  switch(t1->type) {
    case RAPTOR_TERM_TYPE_URI:
      d = raptor_uri_equals(t1->value.uri, t2->value.uri);
      break;

    case RAPTOR_TERM_TYPE_BLANK:
      d = (t1->value.blank.string_len != t2->value.blank.string_len);
      if(d)
        break;

      d = !strcmp((const char*)t1->value.blank.string, 
                  (const char*)t2->value.blank.string);
      break;

    case RAPTOR_TERM_TYPE_LITERAL:
      d = (t1->value.literal.string_len != t2->value.literal.string_len);
      if(d)
        break;

      d = !strcmp((const char*)t1->value.literal.string,
                  (const char*)t2->value.literal.string);
      if(!d)
        break;
      
      if(t1->value.literal.language && t2->value.literal.language) {
        /* both have a language */
        d = !strcmp((const char*)t1->value.literal.language, 
                    (const char*)t2->value.literal.language);
        if(!d)
          break;
      } else if(t1->value.literal.language || t2->value.literal.language) {
        /* only one has a language - different */
        d = 0;
        break;
      }

      if(t1->value.literal.datatype && t2->value.literal.datatype) {
        /* both have a datatype */
        d = raptor_uri_equals(t1->value.literal.datatype,
                              t2->value.literal.datatype);
      } else if(t1->value.literal.datatype || t2->value.literal.datatype) {
        /* only one has a datatype - different */
        d = 0;
      }
      break;
      
    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
      break;
  }

  return d;
}


/**
 * raptor_term_compare:
 * @t1: first term
 * @t2: second term
 *
 * Compare a pair of #raptor_term
 *
 * If types are different, the #raptor_term_type order is used.
 *
 * Resource and datatype URIs are compared with raptor_uri_compare(),
 * blank nodes and literals with strcmp().  If one literal has no
 * language, it is earlier than one with a language.  If one literal
 * has no datatype, it is earlier than one with a datatype.
 * 
 * Return value: <0 if t1 is before t2, 0 if equal, >0 if t1 is after t2
 */
int
raptor_term_compare(const raptor_term *t1,  const raptor_term *t2)
{
  int d = 0;
  
  /* check for NULL terms */
  if(!t1 || !t2) {
    if(!t1 && !t2)
      return 0; /* both NULL */

    /* place NULLs before any other term */
    return t1 ? 1 : -1;
  }

  if(t1->type != t2->type)
    return (t1->type - t2->type);
  
  switch(t1->type) {
    case RAPTOR_TERM_TYPE_URI:
      d = raptor_uri_compare(t1->value.uri, t2->value.uri);
      break;

    case RAPTOR_TERM_TYPE_BLANK:
      d = strcmp((const char*)t1->value.blank.string,
                 (const char*)t2->value.blank.string);
      break;
      
    case RAPTOR_TERM_TYPE_LITERAL:
      d = strcmp((const char*)t1->value.literal.string,
                 (const char*)t2->value.literal.string);
      if(d)
        break;
      
      if(t1->value.literal.language && t2->value.literal.language) {
        /* both have a language */
        d = strcmp((const char*)t1->value.literal.language, 
                   (const char*)t2->value.literal.language);
      } else if(t1->value.literal.language || t2->value.literal.language)
        /* only one has a language; the language-less one is earlier */
        d = (!t1->value.literal.language ? -1 : 1);
      if(d)
        break;
      
      if(t1->value.literal.datatype && t2->value.literal.datatype) {
        /* both have a datatype */
        d = raptor_uri_compare(t1->value.literal.datatype,
                               t2->value.literal.datatype);
      } else if(t1->value.literal.datatype || t2->value.literal.datatype)
        /* only one has a datatype; the datatype-less one is earlier */
        d = (!t1->value.literal.datatype ? -1 : 1);
      break;
      
    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
      break;
  }

  return d;
}
