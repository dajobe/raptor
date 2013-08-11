/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_serialize_ntriples.c - N-Triples and Nquads serializer
 *
 * Copyright (C) 2004-2010, David Beckett http://www.dajobe.org/
 * Copyright (C) 2004-2005, University of Bristol, UK http://www.bristol.ac.uk/
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
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


/*
 * Raptor N-Triples serializer object
 */
typedef struct {
  int is_nquads;
} raptor_ntriples_serializer_context;



/* create a new serializer */
static int
raptor_ntriples_serialize_init(raptor_serializer* serializer, const char *name)
{
  raptor_ntriples_serializer_context* ntriples_serializer;

  ntriples_serializer = (raptor_ntriples_serializer_context*)serializer->context;
  ntriples_serializer->is_nquads = !strcmp(name, "nquads");

  return 0;
}
  

/* destroy a serializer */
static void
raptor_ntriples_serialize_terminate(raptor_serializer* serializer)
{

}
  

/* add a namespace */
static int
raptor_ntriples_serialize_declare_namespace(raptor_serializer* serializer, 
                                            raptor_uri *uri,
                                            const unsigned char *prefix)
{
  /* NOP */
  return 0;
}


#if 0
/* start a serialize */
static int
raptor_ntriples_serialize_start(raptor_serializer* serializer)
{
  return 0;
}
#endif



/**
 * raptor_string_ntriples_write:
 * @string: UTF-8 string to write
 * @len: length of UTF-8 string
 * @delim: Terminating delimiter character for string (such as " or >)
 * or \0 for no escaping.
 * @iostr: #raptor_iostream to write to
 *
 * Write an UTF-8 string using N-Triples escapes to an iostream.
 * 
 * Return value: non-0 on failure such as bad UTF-8 encoding.
 **/
int
raptor_string_ntriples_write(const unsigned char *string,
                             size_t len,
                             const char delim,
                             raptor_iostream *iostr)
{
  return raptor_string_escaped_write(string, len, delim,
                                     RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL,
                                     iostr);
}


/**
 * raptor_bnodeid_ntriples_write:
 * @bnodeid: bnode ID to write
 * @len: length of bnode ID
 * @iostr: #raptor_iostream to write to
 *
 * Write a blank node ID in a form legal for N-Triples with _: prefix
 *
 * Return value: non-0 on failure
 **/
int
raptor_bnodeid_ntriples_write(const unsigned char *bnodeid,
                              size_t len,
                              raptor_iostream *iostr)
{
  unsigned int i;

  raptor_iostream_counted_string_write("_:", 2, iostr);

  for(i = 0; i < len; i++) {
    unsigned char c = *bnodeid++;
    if(!isalpha(c) && !isdigit(c)) {
      /* Replace characters not in legal N-Triples bnode set */
      c = 'z';
    }
    raptor_iostream_write_byte(c, iostr);
  }

  return 0;
}


/**
 * raptor_term_ntriples_write:
 * @term: term to write
 * @iostr: raptor iostream
 * 
 * Write a #raptor_term formatted in N-Triples format to a #raptor_iostream
 * 
 * @Deprecated: Use raptor_term_escaped_write() that allows
 * configuring format detail flags.
 *
 * Return value: non-0 on failure
 **/
int
raptor_term_ntriples_write(const raptor_term *term, raptor_iostream* iostr)
{
  return raptor_term_escaped_write(term,
                                   RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL,
                                   iostr);
}


/**
 * raptor_statement_ntriples_write:
 * @statement: statement to write
 * @iostr: raptor iostream
 * @write_graph_term: flag to write graph term if present
 * 
 * Write a #raptor_statement formatted in N-Triples or N-Quads format
 * to a #raptor_iostream
 * 
 * Return value: non-0 on failure
 **/
int
raptor_statement_ntriples_write(const raptor_statement *statement,
                                raptor_iostream* iostr,
                                int write_graph_term)
{
  unsigned int flags = RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(statement, raptor_statement, 1);

  if(raptor_term_escaped_write(statement->subject, flags, iostr))
    return 1;
  
  raptor_iostream_write_byte(' ', iostr);
  if(raptor_term_escaped_write(statement->predicate, flags, iostr))
    return 1;
  
  raptor_iostream_write_byte(' ', iostr);
  if(raptor_term_escaped_write(statement->object, flags, iostr))
    return 1;

  if(statement->graph && write_graph_term) {
    raptor_iostream_write_byte(' ', iostr);
    if(raptor_term_escaped_write(statement->graph, flags, iostr))
      return 1;
  }
  
  raptor_iostream_counted_string_write(" .\n", 3, iostr);

  return 0;
}


/* serialize a statement */
static int
raptor_ntriples_serialize_statement(raptor_serializer* serializer, 
                                    raptor_statement *statement)
{
  raptor_ntriples_serializer_context* ntriples_serializer;

  ntriples_serializer = (raptor_ntriples_serializer_context*)serializer->context;

  raptor_statement_ntriples_write(statement,
                                  serializer->iostream,
                                  ntriples_serializer->is_nquads);
  return 0;
}


#if 0
/* end a serialize */
static int
raptor_ntriples_serialize_end(raptor_serializer* serializer)
{
  return 0;
}
#endif
  
/* finish the serializer factory */
static void
raptor_ntriples_serialize_finish_factory(raptor_serializer_factory* factory)
{

}


#ifdef RAPTOR_SERIALIZER_NTRIPLES
static const char* const ntriples_names[2] = { "ntriples", NULL};

static const char* const ntriples_uri_strings[3] = {
  "http://www.w3.org/ns/formats/N-Triples",
  "http://www.w3.org/TR/rdf-testcases/#ntriples",
  NULL
};
  
#define NTRIPLES_TYPES_COUNT 2
static const raptor_type_q ntriples_types[NTRIPLES_TYPES_COUNT + 1] = {
  { "application/n-triples", 21, 10}, 
  { "text/plain", 10, 1}, 
  { NULL, 0, 0}
};

static int
raptor_ntriples_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->desc.names = ntriples_names;
  factory->desc.mime_types = ntriples_types;

  factory->desc.label =  "N-Triples";
  factory->desc.uri_strings = ntriples_uri_strings;

  factory->context_length     = sizeof(raptor_ntriples_serializer_context);
  
  factory->init                = raptor_ntriples_serialize_init;
  factory->terminate           = raptor_ntriples_serialize_terminate;
  factory->declare_namespace   = raptor_ntriples_serialize_declare_namespace;
  factory->serialize_start     = NULL;
  factory->serialize_statement = raptor_ntriples_serialize_statement;
  factory->serialize_end       = NULL;
  factory->finish_factory      = raptor_ntriples_serialize_finish_factory;

  return 0;
}
#endif


#ifdef RAPTOR_SERIALIZER_NQUADS
static const char* const nquads_names[2] = { "nquads", NULL};

static const char* const nquads_uri_strings[2] = {
  "http://sw.deri.org/2008/07/n-quads/#n-quads",
  NULL
};
  
#define NQUADS_TYPES_COUNT 1
static const raptor_type_q nquads_types[NQUADS_TYPES_COUNT + 1] = {
  { "text/x-nquads", 13, 10},
  { NULL, 0, 0}
};

static int
raptor_nquads_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->desc.names = nquads_names;
  factory->desc.mime_types = nquads_types;

  factory->desc.label = "N-Quads";
  factory->desc.uri_strings = nquads_uri_strings;

  factory->context_length     = sizeof(raptor_ntriples_serializer_context);
  
  factory->init                = raptor_ntriples_serialize_init;
  factory->terminate           = raptor_ntriples_serialize_terminate;
  factory->declare_namespace   = raptor_ntriples_serialize_declare_namespace;
  factory->serialize_start     = NULL;
  factory->serialize_statement = raptor_ntriples_serialize_statement;
  factory->serialize_end       = NULL;
  factory->finish_factory      = raptor_ntriples_serialize_finish_factory;

  return 0;
}
#endif

#ifdef RAPTOR_SERIALIZER_NTRIPLES
int
raptor_init_serializer_ntriples(raptor_world* world)
{
  return !raptor_serializer_register_factory(world,
                                             &raptor_ntriples_serializer_register_factory);
}
#endif

#ifdef RAPTOR_SERIALIZER_NQUADS
int
raptor_init_serializer_nquads(raptor_world* world)
{
  return !raptor_serializer_register_factory(world,
                                             &raptor_nquads_serializer_register_factory);
}
#endif
