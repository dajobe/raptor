/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_serialize_ntriples.c - N-Triples serializer
 *
 * Copyright (C) 2004-2008, David Beckett http://www.dajobe.org/
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

#ifdef WIN32
#include <win32_raptor_config.h>
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
#include "raptor.h"
#include "raptor_internal.h"


/*
 * Raptor N-Triples serializer object
 */
typedef struct {
  int dummy;
} raptor_ntriples_serializer_context;



/* create a new serializer */
static int
raptor_ntriples_serialize_init(raptor_serializer* serializer, const char *name)
{
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
  return raptor_string_python_write(string, len, delim, 0, iostr);
}


/**
 * raptor_term_ntriples_write:
 * @term: term to write
 * @iostr: raptor iostream
 * 
 * Write a #raptor_term formatted in N-Triples format to a #raptor_iostream
 * 
 * Return value: non-0 on failure
 **/
int
raptor_term_ntriples_write(const raptor_term *term, raptor_iostream* iostr)
{
  unsigned char *term_str;
  size_t len;

  switch(term->type) {
    case RAPTOR_TERM_TYPE_LITERAL:
      raptor_iostream_write_byte(iostr, '"');
      raptor_string_ntriples_write(term->value.literal.string,
                                   strlen((const char*)term->value.literal.string),
                                   '"',
                                   iostr);
      raptor_iostream_write_byte(iostr, '"');
      if(term->value.literal.language) {
        raptor_iostream_write_byte(iostr, '@');
        raptor_iostream_write_string(iostr, term->value.literal.language);
      }
      if(term->value.literal.datatype) {
        raptor_iostream_write_counted_string(iostr, "^^<", 3);
        raptor_iostream_write_string(iostr,
                                     raptor_uri_as_string(term->value.literal.datatype));
        raptor_iostream_write_byte(iostr, '>');
      }

      break;
      
    case RAPTOR_TERM_TYPE_BLANK:
      raptor_iostream_write_counted_string(iostr, "_:", 2);
      raptor_iostream_write_string(iostr, term->value.blank);
      break;
      
    case RAPTOR_TERM_TYPE_URI:
      raptor_iostream_write_byte(iostr, '<');
      term_str = raptor_uri_as_counted_string(term->value.uri, &len);
      raptor_string_ntriples_write(term_str, len, '>', iostr);
      raptor_iostream_write_byte(iostr, '>');
      break;
      
    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
      RAPTOR_FATAL2("Unknown raptor_term type %d", term->type);
      return 1;
  }

  return 0;
}


/**
 * raptor_statement_ntriples_write:
 * @statement: statement to write
 * @iostr: raptor iostream
 * 
 * Write a #raptor_statement formatted in N-Triples format to a #raptor_iostream
 * 
 * Return value: non-0 on failure
 **/
int
raptor_statement_ntriples_write(const raptor_statement *statement,
                                raptor_iostream* iostr)
{
  raptor_term_ntriples_write(statement->subject, iostr);
  raptor_iostream_write_byte(iostr, ' ');
  raptor_term_ntriples_write(statement->predicate, iostr);
  raptor_iostream_write_byte(iostr, ' ');
  raptor_term_ntriples_write(statement->object, iostr);
  raptor_iostream_write_counted_string(iostr, " .\n", 3);

  return 0;
}


/* serialize a statement */
static int
raptor_ntriples_serialize_statement(raptor_serializer* serializer, 
                                    raptor_statement *statement)
{
  raptor_statement_ntriples_write(statement, serializer->iostream);
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


static int
raptor_ntriples_serializer_register_factory(raptor_serializer_factory *factory)
{
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


int
raptor_init_serializer_ntriples(raptor_world* world) {
  return raptor_serializer_register_factory(world,
                                            "ntriples", "N-Triples", 
                                            "text/plain",
                                            NULL,
                                            (const unsigned char*)"http://www.w3.org/TR/rdf-testcases/#ntriples",
                                            &raptor_ntriples_serializer_register_factory);
}


