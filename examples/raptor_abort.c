/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_abort.c - Raptor abort example code
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
#include <config.h>
#endif

#ifdef WIN32
#include <win32_config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#endif

/* Raptor includes */
#include <raptor.h>


static void handle_statements(void *user_data, const raptor_statement *statement);
int main(int argc, char *argv[]);


typedef struct 
{
  raptor_parser *parser;
  FILE *stream;
  int count;
  int max;
  int stopped;
} my_data;


static
void handle_statements(void *user_data, const raptor_statement *statement) 
{
  my_data* me=(my_data*)user_data;
  
  me->count++;
  if(me->count > me->max) {
    fprintf(me->stream, "Reached %d statements, stopping\n", me->max);
    raptor_parse_abort(me->parser);
    me->stopped=1;
    return;
  }

  fprintf(me->stream, "Saw statement %d\n", me->count);
}


int
main (int argc, char *argv[]) 
{
  raptor_parser* rdf_parser;
  raptor_uri* uri;
  my_data* me;
  const char *program;
  int rc;
  
  program=argv[0];

  if(argc != 2) {
    fprintf(stderr, "%s: USAGE [RDF-XML content URI]\n", program);
    exit(1);
  }

  raptor_init();

  me=(my_data*)malloc(sizeof(my_data));
  if(!me) {
    fprintf(stderr, "%s: Out of memory\n", program);
    exit(1);
  }

  me->stream=stderr;
  me->count=0;
  me->max=5;

  uri=raptor_new_uri(argv[1]);
  rdf_parser=raptor_new_parser("rdfxml");

  me->parser=rdf_parser;

  raptor_set_statement_handler(rdf_parser, me, handle_statements);

  me->stopped=0;
  rc=raptor_parse_uri(rdf_parser, uri, NULL);

  fprintf(stderr, "%s: Parser returned status %d, stopped? %s\n", program, rc,
          (me->stopped ? "yes" : "no"));

  free(me);
  
  raptor_free_parser(rdf_parser);

  raptor_free_uri(uri);

  raptor_finish();

#ifdef RAPTOR_WWW_LIBCURL
  curl_global_cleanup();
#endif

  return 0;
}
