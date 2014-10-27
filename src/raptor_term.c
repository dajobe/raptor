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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


#ifndef STANDALONE

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

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  if(!uri)
    return NULL;
  
  raptor_world_open(world);

  t = RAPTOR_CALLOC(raptor_term*, 1, sizeof(*t));
  if(!t)
    return NULL;

  t->usage = 1;
  t->world = world;
  t->type = RAPTOR_TERM_TYPE_URI;
  t->value.uri = raptor_uri_copy(uri);

  return t;
}


/**
 * raptor_new_term_from_counted_uri_string:
 * @world: raptor world
 * @uri_string: UTF-8 encoded URI string.
 * @length: length of URI string
 *
 * Constructor - create a new URI statement term from a UTF-8 encoded Unicode string
 *
 * Note: The @uri_string need not be NULL terminated - a NULL will be
 * added to the copied string used.
 *
 * Return value: new term or NULL on failure
*/
raptor_term*
raptor_new_term_from_counted_uri_string(raptor_world* world, 
                                        const unsigned char *uri_string,
                                        size_t length)
{
  raptor_term *t;
  raptor_uri* uri;

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  uri = raptor_new_uri_from_counted_string(world, uri_string, length);
  if(!uri)
    return NULL;

  t = raptor_new_term_from_uri(world, uri);
  
  raptor_free_uri(uri);
  
  return t;
}


/**
 * raptor_new_term_from_uri_string:
 * @world: raptor world
 * @uri_string: UTF-8 encoded URI string.
 *
 * Constructor - create a new URI statement term from a UTF-8 encoded Unicode string
 *
 * Return value: new term or NULL on failure
*/
raptor_term*
raptor_new_term_from_uri_string(raptor_world* world, 
                                const unsigned char *uri_string)
{
  raptor_term *t;
  raptor_uri* uri;

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  uri = raptor_new_uri(world, uri_string);
  if(!uri)
    return NULL;

  t = raptor_new_term_from_uri(world, uri);
  
  raptor_free_uri(uri);
  
  return t;
}


/**
 * raptor_new_term_from_counted_literal:
 * @world: raptor world
 * @literal: UTF-8 encoded literal string (or NULL for empty literal)
 * @literal_len: length of literal
 * @datatype: literal datatype URI (or NULL)
 * @language: literal language (or NULL for no language)
 * @language_len: literal language length
 *
 * Constructor - create a new literal statement term from a counted UTF-8 encoded literal string
 *
 * Takes copies of the passed in @literal, @datatype, @language
 *
 * Only one of @language or @datatype may be given.  If both are
 * given, NULL is returned.  If @language is the empty string, it is
 * the equivalent to NULL.
 *
 * Note: The @literal need not be NULL terminated - a NULL will be
 * added to the copied string used.
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

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  raptor_world_open(world);

  if(language && !*language)
    language = NULL;

  if(language && datatype)
    return NULL;
  

  new_literal = RAPTOR_MALLOC(unsigned char*, literal_len + 1);
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
    unsigned char c;
    unsigned char* l;
    
    new_language = RAPTOR_MALLOC(unsigned char*, language_len + 1);
    if(!new_language) {
      RAPTOR_FREE(char*, new_literal);
      return NULL;
    }

    l = new_language;
    while((c = *language++)) {
      if(c == '_')
        c = '-';
      *l++ = c;
    }
    *l = '\0';
  } else
    language_len = 0;

  if(datatype)
    datatype = raptor_uri_copy(datatype);
  

  t = RAPTOR_CALLOC(raptor_term*, 1, sizeof(*t));
  if(!t) {
    if(new_literal)
      RAPTOR_FREE(char*, new_literal);
    if(new_language)
      RAPTOR_FREE(char*, new_language);
    if(datatype)
      raptor_free_uri(datatype);
    return NULL;
  }
  t->usage = 1;
  t->world = world;
  t->type = RAPTOR_TERM_TYPE_LITERAL;
  t->value.literal.string = new_literal;
  t->value.literal.string_len = RAPTOR_LANG_LEN_FROM_INT(literal_len);
  t->value.literal.language = new_language;
  t->value.literal.language_len = language_len;
  t->value.literal.datatype = datatype;

  return t;
}


/**
 * raptor_new_term_from_literal:
 * @world: raptor world
 * @literal: UTF-8 encoded literal string (or NULL for empty literal)
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
  size_t language_len = 0;
  
  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  raptor_world_open(world);

  if(literal)
    literal_len = strlen(RAPTOR_GOOD_CAST(const char*, literal));

  if(language)
    language_len = strlen(RAPTOR_GOOD_CAST(const char*, language));

  return raptor_new_term_from_counted_literal(world, literal, literal_len,
                                              datatype, language,
                                              RAPTOR_BAD_CAST(unsigned char, language_len));
}


/**
 * raptor_new_term_from_counted_blank:
 * @world: raptor world
 * @blank: UTF-8 encoded blank node identifier (or NULL)
 * @length: length of identifier (or 0)
 *
 * Constructor - create a new blank node statement term from a counted UTF-8 encoded blank node ID
 *
 * Takes a copy of the passed in @blank
 *
 * If @blank is NULL, creates a new internal identifier and uses it.
 * This will use the handler set with
 * raptor_world_set_generate_bnodeid_parameters()
 *
 * Note: The @blank need not be NULL terminated - a NULL will be
 * added to the copied string used.
 *
 * Return value: new term or NULL on failure
*/
raptor_term*
raptor_new_term_from_counted_blank(raptor_world* world,
                                   const unsigned char* blank, size_t length)
{
  raptor_term *t;
  unsigned char* new_id;

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  raptor_world_open(world);

  if (blank) {
    new_id = RAPTOR_MALLOC(unsigned char*, length + 1);
    if(!new_id)
      return NULL;
    memcpy(new_id, blank, length);
    new_id[length] = '\0';
  } else {
    new_id = raptor_world_generate_bnodeid(world);
    length = strlen((const char*)new_id);
  }

  t = RAPTOR_CALLOC(raptor_term*, 1, sizeof(*t));
  if(!t) {
    RAPTOR_FREE(char*, new_id);
    return NULL;
  }

  t->usage = 1;
  t->world = world;
  t->type = RAPTOR_TERM_TYPE_BLANK;
  t->value.blank.string = new_id;
  t->value.blank.string_len = RAPTOR_BAD_CAST(int, length);

  return t;
}


/**
 * raptor_new_term_from_blank:
 * @world: raptor world
 * @blank: UTF-8 encoded blank node identifier (or NULL)
 *
 * Constructor - create a new blank node statement term from a UTF-8 encoded blank node ID
 *
 * Takes a copy of the passed in @blank
 *
 * If @blank is NULL or an empty string, creates a new internal
 * identifier and uses it.  This will use the handler set with
 * raptor_world_set_generate_bnodeid_parameters()
 *
 * Return value: new term or NULL on failure
*/
raptor_term*
raptor_new_term_from_blank(raptor_world* world, const unsigned char* blank)
{
  size_t length = 0;
  
  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  raptor_world_open(world);

  if(blank) {
    if(*blank)
      length = strlen((const char*)blank);
    else
      blank = NULL;
  }

  return raptor_new_term_from_counted_blank(world, blank, length);
}


/**
 * raptor_new_term_from_counted_string:
 * @world: raptor world
 * @string: N-Triples format string (UTF-8)
 * @length: length of @string (or 0)
 *
 * Constructor - create a new term from a Turtle / N-Triples format string in UTF-8
 *
 * See also raptor_term_to_counted_string() and raptor_term_to_string()
 *
 * Return value: new term or NULL on failure
*/
raptor_term*
raptor_new_term_from_counted_string(raptor_world* world,
                                    unsigned char* string, size_t length)
{
  raptor_term* term = NULL;
  size_t bytes_read;
  raptor_locator locator;

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  if(!string)
    return NULL;

  if(!length)
    length = strlen(RAPTOR_GOOD_CAST(const char*, string));

  raptor_world_open(world);

  memset(&locator, '\0', sizeof(locator));
  locator.line = -1;

  bytes_read = raptor_ntriples_parse_term(world, &locator,
                                          string, &length, &term, 1);

  if(!bytes_read || length != 0) {
    if(term)
      raptor_free_term(term);
    term = NULL;
  }

  return term;
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
        RAPTOR_FREE(char*, term->value.blank.string);
        term->value.blank.string = NULL;
      }
      break;
      
    case RAPTOR_TERM_TYPE_LITERAL:
      if(term->value.literal.string) {
        RAPTOR_FREE(char*, term->value.literal.string);
        term->value.literal.string = NULL;
      }

      if(term->value.literal.datatype) {
        raptor_free_uri(term->value.literal.datatype);
        term->value.literal.datatype = NULL;
      }
      
      if(term->value.literal.language) {
        RAPTOR_FREE(char*, term->value.literal.language);
        term->value.literal.language = NULL;
      }
      break;
      
    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
      break;
  }

  RAPTOR_FREE(term, term);
}


/**
 * raptor_term_to_counted_string:
 * @term: #raptor_term
 * @len_p: Pointer to location to store length of new string (if not NULL)
 *
 * Turns a raptor term into a N-Triples format counted string.
 * 
 * Turns the given @term into an N-Triples escaped string using all the
 * escapes as defined in http://www.w3.org/TR/rdf-testcases/#ntriples
 *
 * This function uses raptor_term_ntriples_write() to write to an
 * #raptor_iostream which is the prefered way to write formatted
 * output.
 *
 * See also raptor_new_term_from_counted_string() to reverse this.
 *
 * See also raptor_term_to_turtle_string() to write as Turtle which
 * will include Turtle syntax such as 'true' for booleans and """quoting"""
 *
 * Return value: the new string or NULL on failure.  The length of
 * the new string is returned in *@len_p if len_p is not NULL.
 **/
unsigned char*
raptor_term_to_counted_string(raptor_term *term, size_t* len_p)
{
  raptor_iostream *iostr;
  void *string = NULL;
  int rc;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(term, raptor_term, NULL);
  
  iostr = raptor_new_iostream_to_string(term->world, 
                                        &string, len_p, NULL);
  if(!iostr)
    return NULL;

  rc = raptor_term_escaped_write(term, 0, iostr);
  raptor_free_iostream(iostr);
  
  if(rc) {
    if(string) {
      RAPTOR_FREE(char*, string);
      string = NULL;
    }
  }

  return (unsigned char *)string;
}


/**
 * raptor_term_to_string:
 * @term: #raptor_term
 *
 * Turns a raptor term into a N-Triples format string.
 * 
 * Turns the given @term into an N-Triples escaped string using all the
 * escapes as defined in http://www.w3.org/TR/rdf-testcases/#ntriples
 *
 * See also raptor_new_term_from_counted_string() to reverse this.
 *
 * See also raptor_term_to_turtle_string() to write as Turtle which
 * will include Turtle syntax such as 'true' for booleans and """quoting"""
 *
 * Return value: the new string or NULL on failure.
 **/
unsigned char*
raptor_term_to_string(raptor_term *term)
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(term, raptor_term, NULL);
  
  return raptor_term_to_counted_string(term, NULL);
}


/*
 * raptor_term_print_as_ntriples:
 * @term: #raptor_term
 * @stream: FILE stream
 *
 * INTERNAL - Print a term as N-Triples
 */
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
  
  rc = raptor_term_escaped_write(term, 0, iostr);

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
      if(t1->value.blank.string_len != t2->value.blank.string_len)
        /* different lengths */
        break;

      d = !strcmp((const char*)t1->value.blank.string, 
                  (const char*)t2->value.blank.string);
      break;

    case RAPTOR_TERM_TYPE_LITERAL:
      if(t1->value.literal.string_len != t2->value.literal.string_len)
        /* different lengths */
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
#endif



#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);

static const unsigned char *uri_string1 = (const unsigned char *)"http://http://www.dajobe.org/";
static unsigned int uri_string1_len = 29; /* strlen(uri_string1) */
static raptor_term_type uri_string1_type = RAPTOR_TERM_TYPE_URI;
static const unsigned char *uri_string2 = (const unsigned char *)"http://www.example.org/";
static unsigned int uri_string2_len = 23; /* strlen(uri_string2) */
static raptor_term_type uri_string2_type = RAPTOR_TERM_TYPE_URI;
static const unsigned char *literal_string1 = (const unsigned char *)"Dave Beckett";
static unsigned int literal_string1_len = 12; /* strlen(literal_string1) */
static raptor_term_type literal_string1_type = RAPTOR_TERM_TYPE_LITERAL;
static const unsigned char *bnodeid1 = (const unsigned char *)"abc123";
static unsigned int bnodeid1_len = 6; /* strlen(bnode_id1) */
static raptor_term_type bnodeid1_type = RAPTOR_TERM_TYPE_BLANK;
static const unsigned char* language1 = (const unsigned char*)"en";

int
main(int argc, char *argv[])
{
  raptor_world *world;
  const char *program = raptor_basename(argv[0]);
  int rc = 0;
  raptor_term* term1 = NULL; /* URI string 1 */
  raptor_term* term2 = NULL; /* literal string1 */
  raptor_term* term3 = NULL; /* blank node 1 */
  raptor_term* term4 = NULL; /* URI string 2 */
  raptor_term* term5 = NULL; /* URI string 1 again */
  raptor_uri* uri1;
  unsigned char* uri_str;
  size_t uri_len;
  
  
  world = raptor_new_world();
  if(!world || raptor_world_open(world))
    exit(1);


  /* check a term for NULL URI fails */
  term1 = raptor_new_term_from_uri(world, NULL);
  if(term1) {
    fprintf(stderr, "%s: raptor_new_uri(NULL) returned object rather than failing\n", program);
    rc = 1;
    goto tidy;
  }

  /* check a term for non-NULL URI succeeds */
  uri1 = raptor_new_uri(world, uri_string1);
  if(!uri1) {
    fprintf(stderr, "%s: raptor_new_uri(%s) failed\n", program, uri_string1);
    rc = 1;
    goto tidy;
  }
  term1 = raptor_new_term_from_uri(world, uri1);
  if(!term1) {
    fprintf(stderr, "%s: raptor_new_term_from_uri_string(URI %s) failed\n",
            program, uri_string1);
    rc = 1;
    goto tidy;
  }
  raptor_free_uri(uri1); uri1 = NULL;
  if(term1->type != uri_string1_type) {
    fprintf(stderr, "%s: raptor term 1 is of type %d expected %d\n",
            program, term1->type, uri_string1_type);
    rc = 1;
    goto tidy;
  }


  /* returns a pointer to shared string */
  uri_str = raptor_uri_as_counted_string(term1->value.uri, &uri_len);
  if(!uri_str) {
    fprintf(stderr, "%s: raptor_uri_as_counted_string term 1 failed\n",
            program);
    rc = 1;
    goto tidy;
  }

  if(uri_len != uri_string1_len) {
    fprintf(stderr, "%s: raptor term 1 URI is of length %d expected %d\n",
            program, (int)uri_len, (int)uri_string1_len);
    rc = 1;
    goto tidy;
  }

  
  /* check an empty literal is created from a NULL literal pointer succeeds */
  term2 = raptor_new_term_from_counted_literal(world, NULL, 0, NULL, NULL, 0);
  if(!term2) {
    fprintf(stderr, "%s: raptor_new_term_from_counted_literal() with all NULLs failed\n", program);
    rc = 1;
    goto tidy;
  }
  raptor_free_term(term2);


  /* check an empty literal from an empty language literal pointer succeeds */
  term2 = raptor_new_term_from_counted_literal(world, NULL, 0, NULL,
                                               (const unsigned char*)"", 0);
  if(!term2) {
    fprintf(stderr, "%s: raptor_new_term_from_counted_literal() with empty language failed\n", program);
    rc = 1;
    goto tidy;
  }
  raptor_free_term(term2);

  /* check a literal with language and datatype fails */
  uri1 = raptor_new_uri(world, uri_string1);
  if(!uri1) {
    fprintf(stderr, "%s: raptor_new_uri(%s) failed\n", program, uri_string1);
    rc = 1;
    goto tidy;
  }
  term2 = raptor_new_term_from_counted_literal(world, literal_string1,
                                               literal_string1_len, 
                                               uri1, language1, 0);
  raptor_free_uri(uri1); uri1 = NULL;
  if(term2) {
    fprintf(stderr, "%s: raptor_new_term_from_counted_literal() with language and datatype returned object rather than failing\n", program);
    rc = 1;
    goto tidy;
  }
  
  /* check a literal with no language and no datatype succeeds */
  term2 = raptor_new_term_from_counted_literal(world, literal_string1,
                                               literal_string1_len, NULL, NULL, 0);
  if(!term2) {
    fprintf(stderr, "%s: raptor_new_term_from_counted_literal(%s) failed\n",
            program, literal_string1);
    rc = 1;
    goto tidy;
  }
  if(term2->type != literal_string1_type) {
    fprintf(stderr, "%s: raptor term 2 is of type %d expected %d\n",
            program, term2->type, literal_string1_type);
    rc = 1;
    goto tidy;
  }
  

  /* check a blank node term with NULL id generates a new identifier */
  term3 = raptor_new_term_from_counted_blank(world, NULL, 0);
  if(!term3) {
    fprintf(stderr, "%s: raptor_new_term_from_counted_blank(NULL) failed\n",
            program);
    rc = 1;
    goto tidy;
  }
  if(term3->type != bnodeid1_type) {
    fprintf(stderr, "%s: raptor term 3 is of type %d expected %d\n",
            program, term3->type, bnodeid1_type);
    rc = 1;
    goto tidy;
  }
  raptor_free_term(term3);

  /* check a blank node term with an identifier succeeds */
  term3 = raptor_new_term_from_counted_blank(world, bnodeid1, bnodeid1_len);
  if(!term3) {
    fprintf(stderr, "%s: raptor_new_term_from_counted_blank(%s) failed\n",
            program, bnodeid1);
    rc = 1;
    goto tidy;
  }
  if(term3->type != bnodeid1_type) {
    fprintf(stderr, "%s: raptor term 3 is of type %d expected %d\n",
            program, term3->type, bnodeid1_type);
    rc = 1;
    goto tidy;
  }


  /* check a different URI term succeeds */
  term4 = raptor_new_term_from_counted_uri_string(world, uri_string2,
                                                  uri_string2_len);
  if(!term4) {
    fprintf(stderr,
            "%s: raptor_new_term_from_counted_uri_string(URI %s) failed\n",
            program, uri_string2);
    rc = 1;
    goto tidy;
  }
  if(term4->type != uri_string2_type) {
    fprintf(stderr, "%s: raptor term 4 is of type %d expected %d\n",
            program, term4->type, uri_string2_type);
    rc = 1;
    goto tidy;
  }
  /* returns a pointer to shared string */
  uri_str = raptor_uri_as_counted_string(term4->value.uri, &uri_len);
  if(!uri_str) {
    fprintf(stderr, "%s: raptor_uri_as_counted_string term 4 failed\n",
            program);
    rc = 1;
    goto tidy;
  }

  if(uri_len != uri_string2_len) {
    fprintf(stderr, "%s: raptor term 4 URI is of length %d expected %d\n",
            program, (int)uri_len, (int)uri_string2_len);
    rc = 1;
    goto tidy;
  }


  /* check the same URI term as term1 succeeds */
  term5 = raptor_new_term_from_uri_string(world, uri_string1);
  if(!term5) {
    fprintf(stderr, "%s: raptor_new_term_from_uri_string(URI %s) failed\n",
            program, uri_string1);
    rc = 1;
    goto tidy;
  }


  if(raptor_term_equals(term1, term2)) {
    fprintf(stderr, "%s: raptor_term_equals (URI %s, literal %s) returned equal, expected not-equal\n",
            program, uri_string1, literal_string1);
    rc = 1;
    goto tidy;
  }

  if(raptor_term_equals(term1, term3)) {
    fprintf(stderr, "%s: raptor_term_equals (URI %s, bnode %s) returned equal, expected not-equal\n",
            program, uri_string1, bnodeid1);
    rc = 1;
    goto tidy;
  }
  
  if(raptor_term_equals(term1, term4)) {
    fprintf(stderr, "%s: raptor_term_equals (URI %s, URI %s) returned equal, expected not-equal\n",
            program, uri_string1, uri_string2);
    rc = 1;
    goto tidy;
  }
  
  if(!raptor_term_equals(term1, term5)) {
    fprintf(stderr, "%s: raptor_term_equals (URI %s, URI %s) returned not-equal, expected equal\n",
            program, uri_string1, uri_string1);
    rc = 1;
    goto tidy;
  }

  if(term1->value.uri != term5->value.uri) {
    fprintf(stderr, "%s: term1 and term5 URI objects returned not-equal pointers, expected equal\n",
            program);
    /* This is not necessarily a failure if the raptor_uri module has had
     * the URI interning disabled with
     *   raptor_world_set_flag(world, RAPTOR_WORLD_FLAG_URI_INTERNING, 0) 
     * however this test suite does not do that, so it is a failure here.
     */
    rc = 1;
    goto tidy;
  }
  

  tidy:
  if(term1)
    raptor_free_term(term1);
  if(term2)
    raptor_free_term(term2);
  if(term3)
    raptor_free_term(term3);
  if(term4)
    raptor_free_term(term4);
  if(term5)
    raptor_free_term(term5);
  
  raptor_free_world(world);

  return rc;
}

#endif /* STANDALONE */
