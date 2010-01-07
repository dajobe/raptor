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
static void raptor_print_statement_part_as_ntriples(raptor_world* world, FILE* stream, const void *term, raptor_identifier_type type, raptor_uri* literal_datatype, const unsigned char *literal_language);


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
raptor_statement_copy(const raptor_statement *statement)
{
  raptor_statement *s;

  s = (raptor_statement*)RAPTOR_CALLOC(raptor_statement, 1,
                                       sizeof(raptor_statement));
  if(!s)
    return NULL;

  s->world = statement->world;

  s->subject.type = statement->subject.type;
  if(statement->subject.type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
    unsigned char *new_blank = (unsigned char*)RAPTOR_MALLOC(cstring, strlen((char*)statement->subject.value)+1);
    if(!new_blank)
      goto oom;
    strcpy((char*)new_blank, (const char*)statement->subject.value);
    s->subject.value = new_blank;
  } else
    s->subject.value = raptor_uri_copy_v2(statement->world,
                                          (raptor_uri*)statement->subject.value);

  s->predicate.type = RAPTOR_IDENTIFIER_TYPE_RESOURCE;
  s->predicate.value = raptor_uri_copy_v2(statement->world,
                                          (raptor_uri*)statement->predicate.value);
  

  s->object.type = statement->object.type;
  if(statement->object.type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
    unsigned char *string;
    char *language = NULL;
    raptor_uri *uri = NULL;
    
    string = (unsigned char*)RAPTOR_MALLOC(cstring,
                                           strlen((char*)statement->object.value)+1);
    if(!string)
      goto oom;
    strcpy((char*)string, (const char*)statement->object.value);
    s->object.value = string;

    if(statement->object.literal_language) {
      language = (char*)RAPTOR_MALLOC(cstring, strlen((const char*)statement->object.literal_language)+1);
      if(!language)
        goto oom;
      strcpy(language, (const char*)statement->object.literal_language);
      s->object.literal_language = (const unsigned char*)language;
    }

    if(statement->object.literal_datatype) {
      uri = raptor_uri_copy_v2(statement->world,
                               (raptor_uri*)statement->object.literal_datatype);
      s->object.literal_datatype = uri;
    }
  } else if(statement->object.type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
    char *blank = (char*)statement->object.value;
    unsigned char *new_blank = (unsigned char*)RAPTOR_MALLOC(cstring, strlen(blank)+1);
    if(!new_blank)
      goto oom;
    strcpy((char*)new_blank, (const char*)blank);
    s->object.value = new_blank;
  } else {
    raptor_uri *uri = raptor_uri_copy_v2(statement->world,
                                         (raptor_uri*)statement->object.value);
    s->object.value = uri;
  }

  return s;

  oom:
  raptor_free_statement(s);
  return NULL;
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
  if(statement->subject.value) {
    if(statement->subject.type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
      raptor_free_uri_v2(statement->world, (raptor_uri*)statement->subject.value);
    else
      RAPTOR_FREE(cstring, (void*)statement->subject.value);
  }

  if(statement->predicate.value) {
    if(statement->predicate.type == RAPTOR_IDENTIFIER_TYPE_RESOURCE)
      raptor_free_uri_v2(statement->world, (raptor_uri*)statement->predicate.value);
    else
      RAPTOR_FREE(cstring, (void*)statement->predicate.value);
  }

  if(statement->object.type == RAPTOR_IDENTIFIER_TYPE_RESOURCE) {
    if(statement->object.value)
      raptor_free_uri_v2(statement->world, (raptor_uri*)statement->object.value);
  } else {
    if(statement->object.value)
      RAPTOR_FREE(cstring, (void*)statement->object.value);

    if(statement->object.literal_language)
      RAPTOR_FREE(cstring, (void*)statement->object.literal_language);
    if(statement->object.literal_datatype)
      raptor_free_uri_v2(statement->world,
                         (raptor_uri*)statement->object.literal_datatype);
  }

  RAPTOR_FREE(raptor_statement, statement);
}


/**
 * raptor_print_statement:
 * @statement: #raptor_statement object to print
 * @stream: #FILE* stream
 *
 * Print a raptor_statement to a stream.
 **/
void
raptor_print_statement(const raptor_statement * statement, FILE *stream) 
{
  fputc('[', stream);

  if(statement->subject.type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
    fputs((const char*)statement->subject.value, stream);
  } else {
#ifdef RAPTOR_DEBUG
    if(!statement->subject.value)
      RAPTOR_FATAL1("Statement has NULL subject URI\n");
#endif
    fputs((const char*)raptor_uri_as_string_v2(statement->world,
                                               (raptor_uri*)statement->subject.value), stream);
  }

  fputs(", ", stream);

#ifdef RAPTOR_DEBUG
  if(!statement->predicate.value)
    RAPTOR_FATAL1("Statement has NULL predicate URI\n");
#endif
  fputs((const char*)raptor_uri_as_string_v2(statement->world,
                                               (raptor_uri*)statement->predicate.value), stream);

  fputs(", ", stream);

  if(statement->object.type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
    if(statement->object.literal_datatype) {
      fputc('<', stream);
      fputs((const char*)raptor_uri_as_string_v2(statement->world,
                                                 (raptor_uri*)statement->object.literal_datatype), stream);
      fputc('>', stream);
    }
    fputc('"', stream);
    fputs((const char*)statement->object.value, stream);
    fputc('"', stream);
  } else if(statement->object.type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
    fputs((const char*)statement->object.value, stream);
  else {
#ifdef RAPTOR_DEBUG
    if(!statement->object.value)
      RAPTOR_FATAL1("Statement has NULL object URI\n");
#endif
    fputs((const char*)raptor_uri_as_string_v2(statement->world,
                                               (raptor_uri*)statement->object.value), stream);
  }

  fputc(']', stream);
}


/**
 * raptor_statement_part_as_counted_string:
 * @world: raptor_world object
 * @term: #raptor_statement part (subject, predicate, object)
 * @type: #raptor_statement part type
 * @literal_datatype: #raptor_statement part datatype
 * @literal_language: #raptor_statement part language
 * @len_p: Pointer to location to store length of new string (if not NULL)
 *
 * Turns part of raptor statement into a N-Triples format counted string.
 * 
 * Turns the given @term into an N-Triples escaped string using all the
 * escapes as defined in http://www.w3.org/TR/rdf-testcases/#ntriples
 *
 * The part (subject, predicate, object) of the raptor_statement is
 * typically passed in as @term, the part type (subject_type,
 * predicate_type, object_type) is passed in as @type.  When the part
 * is a literal, the @literal_datatype and @literal_language fields
 * are set, otherwise NULL (usually object_datatype,
 * object_literal_language).
 *
 * Return value: the new string or NULL on failure.  The length of
 * the new string is returned in *@len_p if len_p is not NULL.
 **/
unsigned char*
raptor_statement_part_as_counted_string(raptor_world* world,
                                        const void *term, 
                                        raptor_identifier_type type,
                                        raptor_uri* literal_datatype,
                                        const unsigned char *literal_language,
                                        size_t* len_p)
{
  size_t len = 0, term_len, uri_len;
  size_t language_len = 0;
  unsigned char *s, *buffer = NULL;
  unsigned char *uri_string = NULL;
  
  switch(type) {
    case RAPTOR_IDENTIFIER_TYPE_LITERAL:
      term_len = strlen((const char*)term);
      len = 2+term_len;
      if(literal_language && type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
        language_len = strlen((const char*)literal_language);
        len+= language_len+1;
      }
      if(literal_datatype) {
        uri_string = raptor_uri_as_counted_string_v2(world,
                                                     (raptor_uri*)literal_datatype, &uri_len);
        len += 4+uri_len;
      }
  
      buffer = (unsigned char*)RAPTOR_MALLOC(cstring, len+1);
      if(!buffer)
        return NULL;

      s = buffer;
      *s++ ='"';
      /* raptor_print_ntriples_string(stream, (const char*)term, '"'); */
      strcpy((char*)s, (const char*)term);
      s+= term_len;
      *s++ ='"';
      if(literal_language && type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
        *s++ ='@';
        strcpy((char*)s, (const char*)literal_language);
        s+= language_len;
      }

      if(literal_datatype) {
        *s++ ='^';
        *s++ ='^';
        *s++ ='<';
        strcpy((char*)s, (const char*)uri_string);
        s+= uri_len;
        *s++ ='>';
      }
      *s++ ='\0';
      
      break;
      
    case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
      len = 2+strlen((const char*)term);
      buffer = (unsigned char*)RAPTOR_MALLOC(cstring, len+1);
      if(!buffer)
        return NULL;
      s = buffer;
      *s++ ='_';
      *s++ =':';
      strcpy((char*)s, (const char*)term);
      break;
      
    case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
      uri_string = raptor_uri_as_counted_string_v2(world,
                                                   (raptor_uri*)term, &uri_len);
      len = 2+uri_len;
      buffer = (unsigned char*)RAPTOR_MALLOC(cstring, len+1);
      if(!buffer)
        return NULL;

      s = buffer;
      *s++ ='<';
      /* raptor_print_ntriples_string(stream, raptor_uri_as_string((raptor_uri*)term), '\0'); */
      strcpy((char*)s, (const char*)uri_string);
      s+= uri_len;
      *s++ ='>';
      *s++ ='\0';
      break;
      
    case RAPTOR_IDENTIFIER_TYPE_UNKNOWN:
    default:
      RAPTOR_FATAL2("Unknown type %d", type);
  }

  if(len_p)
    *len_p=len;
  
 return buffer;
}


/**
 * raptor_statement_part_as_string:
 * @world: raptor_world object
 * @term: #raptor_statement part (subject, predicate, object)
 * @type: #raptor_statement part type
 * @literal_datatype: #raptor_statement part datatype
 * @literal_language: #raptor_statement part language
 *
 * Turns part of raptor statement into a N-Triples format string.
 * 
 * Turns the given @term into an N-Triples escaped string using all the
 * escapes as defined in http://www.w3.org/TR/rdf-testcases/#ntriples
 *
 * The part (subject, predicate, object) of the raptor_statement is
 * typically passed in as @term, the part type (subject_type,
 * predicate_type, object_type) is passed in as @type.  When the part
 * is a literal, the @literal_datatype and @literal_language fields
 * are set, otherwise NULL (usually object_datatype,
 * object_literal_language).
 *
 * Return value: the new string or NULL on failure.
 **/
unsigned char*
raptor_statement_part_as_string(raptor_world* world,
                                const void *term, 
                                raptor_identifier_type type,
                                raptor_uri* literal_datatype,
                                const unsigned char *literal_language)
{
  return raptor_statement_part_as_counted_string(world, term, type,
                                                 literal_datatype,
                                                 literal_language,
                                                 NULL);
}


static void
raptor_print_statement_part_as_ntriples(raptor_world* world,
                                        FILE* stream,
                                        const void *term, 
                                        raptor_identifier_type type,
                                        raptor_uri* literal_datatype,
                                        const unsigned char *literal_language) 
{
  switch(type) {
    case RAPTOR_IDENTIFIER_TYPE_LITERAL:
      fputc('"', stream);
      raptor_print_ntriples_string(stream, (const unsigned char*)term, '"');
      fputc('"', stream);
      if(literal_language && type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
        fputc('@', stream);
        fputs((const char*)literal_language, stream);
      }
      if(literal_datatype) {
        fputs("^^<", stream);
        fputs((const char*)raptor_uri_as_string_v2(world,
                                                   (raptor_uri*)literal_datatype), stream);
        fputc('>', stream);
      }

      break;
      
    case RAPTOR_IDENTIFIER_TYPE_ANONYMOUS:
      fputs("_:", stream);
      fputs((const char*)term, stream);
      break;
      
    case RAPTOR_IDENTIFIER_TYPE_RESOURCE:
      fputc('<', stream);
      raptor_print_ntriples_string(stream, raptor_uri_as_string_v2(world, (raptor_uri*)term), '\0');
      fputc('>', stream);
      break;
      
    case RAPTOR_IDENTIFIER_TYPE_UNKNOWN:
    default:
      RAPTOR_FATAL2("Unknown type %d", type);
  }
}


/**
 * raptor_print_statement_as_ntriples:
 * @statement: #raptor_statement to print
 * @stream: #FILE* stream
 *
 * Print a raptor_statement in N-Triples form.
 * 
 **/
void
raptor_print_statement_as_ntriples(const raptor_statement * statement,
                                   FILE *stream) 
{
  raptor_print_statement_part_as_ntriples(statement->world,
                                          stream,
                                          statement->subject.value,
                                          statement->subject.type,
                                          NULL, NULL);
  fputc(' ', stream);
  raptor_print_statement_part_as_ntriples(statement->world,
                                          stream,
                                          statement->predicate.value,
                                          statement->predicate.type,
                                          NULL, NULL);
  fputc(' ', stream);
  raptor_print_statement_part_as_ntriples(statement->world,
                                          stream,
                                          statement->object.value,
                                          statement->object.type,
                                          statement->object.literal_datatype,
                                          statement->object.literal_language);
  fputs(" .", stream);
}

 
/**
 * raptor_statement_compare:
 * @s1: first statement
 * @s2: second statement
 *
 * Compare a pair of #raptor_statement
 *
 * If types are different, the #raptor_identifier_type order is used.
 * Resource and datatype URIs are compared with raptor_uri_compare(),
 * blank nodes and literals with strcmp().  If one literal has no
 * language, it is earlier than one with a language.  If one literal
 * has no datatype, it is earlier than one with a datatype.
 * 
 * Return value: <0 if s1 is before s2, 0 if equal, >0 if s1 is after s2
 */
int
raptor_statement_compare(const raptor_statement *s1,
                         const raptor_statement *s2)
{
  int d = 0;

  if(s1->subject.value && s2->subject.value) {
    d = s1->subject.type != s2->subject.type;
    if(d)
      return d;

    /* subjects are URIs or blank nodes */
    if(s1->subject.type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
      d = strcmp((char*)s1->subject.value, (char*)s2->subject.value);
    else
      d = raptor_uri_compare_v2(s1->world,
                                (raptor_uri*)s1->subject.value,
                                (raptor_uri*)s2->subject.value);
  } else if(s1->subject.value || s2->subject.value)
    d = (!s1->subject.value ? -1 : 1);
  if(d)
    return d;
  

  /* predicates are URIs */
  if(s1->predicate.value && s2->predicate.value) {
    d = raptor_uri_compare_v2(s1->world,
                              (raptor_uri*)s1->predicate.value,
                              (raptor_uri*)s2->predicate.value);
  } else if(s1->predicate.value || s2->predicate.value)
    d = (!s1->predicate.value ? -1 : 1);
  if(d)
    return d;


  /* objects are URIs or blank nodes or literals */
  if(s1->object.value && s2->object.value) {
    if(s1->object.type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
      d = strcmp((char*)s1->object.value, (char*)s2->object.value);
      if(d)
        return d;

      if(s1->object.literal_language && s2->object.literal_language) {
        /* both have a language */
        d = strcmp((char*)s1->object.literal_language,
                   (char*)s2->object.literal_language);
      } else if(s1->object.literal_language || s2->object.literal_language)
        /* only one has a language; the language-less one is earlier */
        d = (!s1->object.literal_language ? -1 : 1);
      if(d)
        return d;

      if(s1->object.literal_datatype && s2->object.literal_datatype) {
        /* both have a datatype */
        d = raptor_uri_compare_v2(s1->world,
                                  (raptor_uri*)s1->object.literal_datatype,
                                  (raptor_uri*)s2->object.literal_datatype);
      } else if(s1->object.literal_datatype || s2->object.literal_datatype)
        /* only one has a datatype; the datatype-less one is earlier */
        d = (!s1->object.literal_datatype ? -1 : 1);
      if(d)
        return d;

    } else if(s1->object.type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS)
      d = strcmp((char*)s1->object.value, (char*)s2->object.value);
    else
      d = raptor_uri_compare_v2(s1->world,
                                (raptor_uri*)s1->object.value,
                                (raptor_uri*)s2->object.value);
  } else if(s1->object.value || s2->object.value)
    d = (!s1->object.value ? -1 : 1);

  return d;
}
