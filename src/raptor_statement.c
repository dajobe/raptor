/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_statement.c - Raptor terms and statements
 *
 * Copyright (C) 2008-2010, David Beckett http://www.dajobe.org/
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


/* prototypes for helper functions */


/**
 * raptor_new_statement:
 * @world: raptor world
 *
 * Constructor - create a new #raptor_statement.
 *
 */
raptor_statement*
raptor_new_statement(raptor_world *world)
{
  raptor_statement* statement;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, NULL);

  raptor_world_open(world);

  statement = (raptor_statement*)RAPTOR_CALLOC(raptor_statement,
                                               1, sizeof(*statement));
  if(!statement)
    return NULL;
  
  statement->world = world;
  /* dynamic - usage counted */
  statement->usage = 1;

  return statement;
}


raptor_statement*
raptor_new_statement_from_nodes(raptor_world* world, raptor_term *subject,
                                raptor_term *predicate, raptor_term *object,
                                raptor_term *graph)
{
  raptor_statement* t;
  
  t = raptor_new_statement(world);
  if(!t) {
    if(subject)
      raptor_free_term(subject);
    if(predicate)
      raptor_free_term(predicate);
    if(object)
      raptor_free_term(object);
    if(graph)
	  raptor_free_term(graph);
    return NULL;
  }
  
  t->subject = subject;
  t->predicate = predicate;
  t->object = object;
  t->graph = graph;

  return t;
}


/**
 * raptor_statement_init:
 * @statement: statement to initialize
 * @world: raptor world
 *
 * Initialize a static #raptor_statement.
 *
 */
void
raptor_statement_init(raptor_statement *statement, raptor_world *world)
{
  statement->world = world;

  /* static - not usage counted */
  statement->usage = -1;
}


/**
 * raptor_statement_copy:
 * @statement: statement to copy
 *
 * Copy a #raptor_statement.
 *
 * Return value: a new #raptor_statement or NULL on error
 */
raptor_statement*
raptor_statement_copy(raptor_statement *statement)
{
  /* static - not usage counted */
  if(statement->usage < 0) {
    raptor_statement* s2;
    /* s2 will be a dynamic, usage->counted statement */
    s2 = raptor_new_statement(statement->world);
    if(!s2)
      return NULL;

    s2->world = statement->world;
    if(statement->subject)
      s2->subject = raptor_term_copy(statement->subject);
    if(statement->predicate)
      s2->predicate = raptor_term_copy(statement->predicate);
    if(statement->object)
      s2->object = raptor_term_copy(statement->object);
    if(statement->graph)
      s2->graph = raptor_term_copy(statement->graph);

    return s2;
  }
  
  statement->usage++;

  return statement;
}


/**
 * raptor_free_statement:
 * @statement: statement
 *
 * Destructor
 *
 */
void
raptor_free_statement(raptor_statement *statement)
{
  /* dynamically or statically allocated? */
  int is_dynamic;

  if(!statement)
    return;

  is_dynamic = (statement->usage >= 0);

  /* dynamically allocated and still in use? */
  if(is_dynamic && --statement->usage)
    return;
  
  /* free contained terms for both statically and dynamically allodated statements */
  if(statement->subject) {
    raptor_free_term(statement->subject);
    statement->subject = NULL;
  }
  if(statement->predicate) {
    raptor_free_term(statement->predicate);
    statement->predicate = NULL;
  }
  if(statement->object) {
    raptor_free_term(statement->object);
    statement->object = NULL;
  }
  if(statement->graph) {
    raptor_free_term(statement->graph);
    statement->graph = NULL;
  }
  if(is_dynamic)
    RAPTOR_FREE(raptor_statement, statement);
}


/**
 * raptor_statement_print:
 * @statement: #raptor_statement object to print
 * @stream: FILE* stream
 *
 * Print a raptor_statement to a stream.
 *
 * Return value: non-0 on failure
 **/
int
raptor_statement_print(const raptor_statement * statement, FILE *stream) 
{
  int rc = 0;
  
  fputc('[', stream);

  if(statement->subject->type == RAPTOR_TERM_TYPE_BLANK) {
    fputs((const char*)statement->subject->value.blank, stream);
  } else {
#ifdef RAPTOR_DEBUG
    if(!statement->subject->value.uri)
      RAPTOR_FATAL1("Statement has NULL subject URI\n");
#endif
    fputs((const char*)raptor_uri_as_string(statement->subject->value.uri),
          stream);
  }

  fputs(", ", stream);

#ifdef RAPTOR_DEBUG
  if(!statement->predicate->value.uri)
    RAPTOR_FATAL1("Statement has NULL predicate URI\n");
#endif
  fputs((const char*)raptor_uri_as_string(statement->predicate->value.uri),
        stream);

  fputs(", ", stream);

  if(statement->object->type == RAPTOR_TERM_TYPE_LITERAL) {
    if(statement->object->value.literal.datatype) {
      raptor_uri* dt_uri = statement->object->value.literal.datatype;
      fputc('<', stream);
      fputs((const char*)raptor_uri_as_string(dt_uri), stream);
      fputc('>', stream);
    }
    fputc('"', stream);
    fputs((const char*)statement->object->value.literal.string, stream);
    fputc('"', stream);
  } else if(statement->object->type == RAPTOR_TERM_TYPE_BLANK)
    fputs((const char*)statement->object->value.blank, stream);
  else {
#ifdef RAPTOR_DEBUG
    if(!statement->object->value.uri)
      RAPTOR_FATAL1("Statement has NULL object URI\n");
#endif
    fputs((const char*)raptor_uri_as_string(statement->object->value.uri),
          stream);
  }

  if(statement->graph) {
    if(statement->graph->type == RAPTOR_TERM_TYPE_BLANK &&
       statement->graph->value.blank) {
      fputs(", ", stream);

      fputs((const char*)statement->graph->value.blank, stream);
    } else if(statement->graph->type == RAPTOR_TERM_TYPE_URI &&
              statement->graph->value.uri) {
      fputs(", ", stream);
      fputs((const char*)raptor_uri_as_string(statement->graph->value.uri),
            stream);
    }
  }
  
  fputc(']', stream);
  
  return rc;
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
  return raptor_term_as_counted_string(term, NULL);
}


int
raptor_term_print_as_ntriples(const raptor_term *term, FILE* stream)
{
  int rc = 0;
  raptor_iostream* iostr;

  if(!stream || !term)
    return 1;
  
  iostr = raptor_new_iostream_to_file_handle(term->world, stream);
  if(!iostr)
    return 1;
  
  rc = raptor_term_ntriples_write(term, iostr);

  raptor_free_iostream(iostr);
  
  return rc;
}


/**
 * raptor_statement_print_as_ntriples:
 * @statement: #raptor_statement to print
 * @stream: FILE* stream
 *
 * Print a raptor_statement in N-Triples form.
 *
 * Return value: non-0 on failure
 **/
int
raptor_statement_print_as_ntriples(const raptor_statement * statement,
                                   FILE *stream) 
{
  if(raptor_term_print_as_ntriples(statement->subject, stream))
    return 1;
  fputc(' ', stream);
  if(raptor_term_print_as_ntriples(statement->predicate, stream))
    return 1;
  fputc(' ', stream);
  if(raptor_term_print_as_ntriples(statement->object, stream))
    return 1;
  if(statement->graph) {
    fputc(' ', stream);
    if(raptor_term_print_as_ntriples(statement->graph, stream))
      return 1;
  }
  fputs(" .", stream);

  return 0;
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
      d = strcmp((const char*)t1->value.blank, (const char*)t2->value.blank);
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


/**
 * raptor_statement_compare:
 * @s1: first statement
 * @s2: second statement
 *
 * Compare a pair of #raptor_statement
 *
 * Uses raptor_term_compare() to check ordering between subjects,
 * predicates and objects of statements.
 * 
 * Return value: <0 if s1 is before s2, 0 if equal, >0 if s1 is after s2
 */
int
raptor_statement_compare(const raptor_statement *s1,
                         const raptor_statement *s2)
{
  int d = 0;

  d = raptor_term_compare(s1->subject, s2->subject);
  if(d)
    return d;

  /* predicates are URIs */
  d = raptor_term_compare(s1->predicate, s2->predicate);
  if(d)
    return d;

  /* objects are URIs or blank nodes or literals */
  d = raptor_term_compare(s1->object, s2->object);
  if(d)
    return d;

  /* graphs are URIs or blank nodes */
  d = raptor_term_compare(s1->graph, s2->graph);

  return d;
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
      if(term->value.blank) {
        RAPTOR_FREE(cstring, (void*)term->value.blank);
        term->value.blank = NULL;
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
 * Return value: new term or NULL on failure
*/
raptor_term*
raptor_new_term_from_literal(raptor_world* world,
                             const unsigned char* literal,
                             raptor_uri* datatype,
                             const unsigned char* language)
{
  raptor_term *t;
  unsigned char* new_literal = NULL;
  unsigned char* new_language = NULL;
  size_t literal_len = 0;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, NULL);

  raptor_world_open(world);

  if(literal)
    literal_len = strlen((const char*)literal);

  new_literal = (unsigned char*)RAPTOR_MALLOC(cstring, literal_len + 1);
  if(!new_literal)
    return NULL;

  if(literal_len)
    memcpy(new_literal, literal, literal_len + 1);
  else
    *new_literal = '\0';

  if(language && datatype) {
    raptor_log_error(world, RAPTOR_LOG_LEVEL_WARN, NULL,
                     "Ignoring language used with datatyped literal");
    RAPTOR_FREE(cstring, language);
    language = NULL;
  }
  

  if(language) {
    size_t language_len = strlen((const char*)language);

    new_language = (unsigned char*)RAPTOR_MALLOC(cstring, language_len + 1);
    if(!new_language) {
      RAPTOR_FREE(cstring, new_literal);
      return NULL;
    }
    memcpy(new_language, language, language_len + 1);
  }

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
  t->value.literal.language = new_language;
  t->value.literal.datatype = datatype;

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
  raptor_term *t;
  unsigned char* new_id;
  size_t len;

  if(!blank)
    return NULL;
  
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, NULL);

  raptor_world_open(world);

  len = strlen((char*)blank);
  new_id = (unsigned char*)RAPTOR_MALLOC(cstring, len + 1);
  if(!new_id)
    return NULL;
  memcpy(new_id, blank, len + 1);

  t = (raptor_term*)RAPTOR_CALLOC(raptor_term, 1, sizeof(*t));
  if(!t) {
    RAPTOR_FREE(cstring, new_id);
    return NULL;
  }
  t->usage = 1;
  t->world = world;
  t->type = RAPTOR_TERM_TYPE_BLANK;
  t->value.blank = new_id;

  return t;
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
  
  if(t1->type != t2->type)
    return 0;
  
  if(t1 == t2)
    return 1;
  
  switch(t1->type) {
    case RAPTOR_TERM_TYPE_URI:
      d = raptor_uri_equals(t1->value.uri, t2->value.uri);
      break;

    case RAPTOR_TERM_TYPE_BLANK:
      d = !strcmp((const char*)t1->value.blank, (const char*)t2->value.blank);
      break;

    case RAPTOR_TERM_TYPE_LITERAL:
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
