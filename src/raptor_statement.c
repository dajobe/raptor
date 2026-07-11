/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_statement.c - Raptor statements
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>
/* for ptrdiff_t */
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif
#include <stdarg.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


/* prototypes for helper functions */


/**
 * raptor_new_statement:
 * @world: raptor world
 *
 * Constructor - create a new #raptor_statement.
 *
 * Return value: new raptor statement or NULL on failure
 */
raptor_statement*
raptor_new_statement(raptor_world *world)
{
  raptor_statement* statement;

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  raptor_world_open(world);

  statement = RAPTOR_CALLOC(raptor_statement*, 1, sizeof(*statement));
  if(!statement)
    return NULL;
  
  statement->world = world;
  /* dynamic - usage counted */
  statement->usage = 1;

  return statement;
}


/**
 * raptor_new_statement_from_nodes:
 * @world: raptor world
 * @subject: subject term (or NULL)
 * @predicate: predicate term (or NULL)
 * @object: object term (or NULL)
 * @graph: graph name term (or NULL)
 *
 * Constructor - create a new #raptor_statement from a set of terms
 *
 * The @subject, @predicate, @object and @graph become owned by the statement.
 *
 * Return value: new raptor statement or NULL on failure
 */
raptor_statement*
raptor_new_statement_from_nodes(raptor_world* world, raptor_term *subject,
                                raptor_term *predicate, raptor_term *object,
                                raptor_term *graph)
{
  raptor_statement* t;
  
  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

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
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN(world, raptor_world);
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN(statement, raptor_statement);

  /* ensure all fields are set to NULL to start with */
  memset(statement, 0, sizeof(*statement));

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
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(statement, raptor_statement, NULL);

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
 * raptor_statement_clear:
 * @statement: #raptor_statement object
 *
 * Empty a raptor_statement of terms.
 * 
 **/
void
raptor_statement_clear(raptor_statement *statement)
{
  if(!statement)
    return;

  /* raptor_free_term() does a NULL check */

  raptor_free_term(statement->subject);
  statement->subject = NULL;

  raptor_free_term(statement->predicate);
  statement->predicate = NULL;

  raptor_free_term(statement->object);
  statement->object = NULL;

  raptor_free_term(statement->graph);
  statement->graph = NULL;
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

  raptor_statement_clear(statement);
  
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
  
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(statement, raptor_statement, 1);

  fputc('[', stream);

  if(!statement->subject) {
    fputs("NULL", stream);
  } else {
    if(statement->subject->type == RAPTOR_TERM_TYPE_BLANK)
      fputs((const char*)statement->subject->value.blank.string, stream);
    else
      raptor_uri_print(statement->subject->value.uri, stream);
  }
  
  fputs(", ", stream);

  if(statement->predicate)
    raptor_uri_print(statement->predicate->value.uri, stream);
  else
    fputs("NULL", stream);
  
  fputs(", ", stream);

  if(!statement->object) {
    fputs("NULL", stream);
  } else {
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
      fputs((const char*)statement->object->value.blank.string, stream);
    else {
      raptor_uri_print(statement->object->value.uri, stream);
    }
  }

  if(statement->graph) {
    if(statement->graph->type == RAPTOR_TERM_TYPE_BLANK &&
       statement->graph->value.blank.string) {
      fputs(", ", stream);

      fputs((const char*)statement->graph->value.blank.string, stream);
    } else if(statement->graph->type == RAPTOR_TERM_TYPE_URI &&
              statement->graph->value.uri) {
      fputs(", ", stream);
      raptor_uri_print(statement->graph->value.uri, stream);
    }
  }
  
  fputc(']', stream);
  
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
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(statement, raptor_statement, 1);

  if(raptor_term_print_as_ntriples(statement->subject, stream))
    return 1;

  fputc(' ', stream);
  if(raptor_term_print_as_ntriples(statement->predicate, stream))
    return 1;

  fputc(' ', stream);
  if(raptor_term_print_as_ntriples(statement->object, stream))
    return 1;

  fputs(" .", stream);

  return 0;
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

  if(!s1 || !s2) {
    /* If one or both are NULL, return a stable comparison order */
    ptrdiff_t pd = (s2 - s1);
    
    /* copy the sign of the (unknown size) signed integer 'd' into an
     * int result
     */
    return (pd > 0) - (pd < 0);
  }

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
 * raptor_statement_equals:
 * @s1: first statement
 * @s2: second statement
 *
 * Compare a pair of #raptor_statement for equality
 *
 * Return value: non-0 if statements are equal
 */
int
raptor_statement_equals(const raptor_statement* s1, const raptor_statement* s2)
{
  if(!s1 || !s2)
    return 0;
  
  if(!raptor_term_equals(s1->subject, s2->subject))
    return 0;
  
  if(!raptor_term_equals(s1->predicate, s2->predicate))
    return 0;
  
  if(!raptor_term_equals(s1->object, s2->object))
    return 0;

  return 1;
}
