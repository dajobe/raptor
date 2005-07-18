/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_guess_parse.c - Raptor guessing real parser implementation
 *
 * $Id$
 *
 * Copyright (C) 2005, David Beckett http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology http://www.ilrt.bristol.ac.uk/
 * University of Bristol, UK http://www.bristol.ac.uk/
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
 * guess parser object
 */
struct raptor_guess_parser_context_s {
  /* content type got from URI request */
  char* content_type;

  /* URI from start_parse */
  raptor_uri* uri;
  
  /* actual parser for dealing with the result */
  raptor_parser* parser;

  int started;

  const char *name;
};


typedef struct raptor_guess_parser_context_s raptor_guess_parser_context;


static int
raptor_guess_parse_init(raptor_parser* rdf_parser, const char *name)
{
  raptor_guess_parser_context *guess_parser=(raptor_guess_parser_context*)rdf_parser->context;
  guess_parser->content_type=NULL;
  
  return 0;
}


static void
raptor_guess_parse_terminate(raptor_parser *rdf_parser)
{
  raptor_guess_parser_context *guess_parser=(raptor_guess_parser_context*)rdf_parser->context;

  if(guess_parser->content_type)
    RAPTOR_FREE(cstring, guess_parser->content_type);

  if(guess_parser->parser)
    raptor_free_parser(guess_parser->parser);
}


static void
raptor_guess_parse_content_type_handler(raptor_parser* rdf_parser, 
                                        const char* content_type)
{
  raptor_guess_parser_context* guess_parser=(raptor_guess_parser_context*)rdf_parser->context;

  if(content_type) {
    size_t len=strlen(content_type);
    const unsigned char *identifier;
    
    guess_parser->content_type=RAPTOR_MALLOC(cstring, len+1);
    strncpy(guess_parser->content_type, content_type, len+1);

    RAPTOR_DEBUG2("Got content type '%s'\n", content_type);

    identifier=raptor_uri_as_string(rdf_parser->base_uri);
    guess_parser->name=raptor_guess_parser_name(NULL, content_type, NULL, 0,
                                                identifier);
    if(!guess_parser->name) {
      raptor_parser_error(rdf_parser, 
                          "Failed to guess parser from content type '%s'",
                          content_type);
    } else {
      RAPTOR_DEBUG2("Guessed parser name  '%s'\n", guess_parser->name);
      guess_parser->parser=raptor_new_parser(guess_parser->name);
      raptor_parser_copy_user_state(guess_parser->parser, rdf_parser);
    }
  }
}


static int
raptor_guess_parse_chunk(raptor_parser* rdf_parser, 
                        const unsigned char *buffer, size_t len,
                        int is_end)
{
  raptor_guess_parser_context* guess_parser=(raptor_guess_parser_context*)rdf_parser->context;
  raptor_parser* iparser=guess_parser->parser;
  
  if(!guess_parser->content_type || !iparser) {
    raptor_parse_abort(rdf_parser);
    return 1;
  }

  if(!guess_parser->started) {
    guess_parser->started=1;
    if(raptor_start_parse(iparser, rdf_parser->base_uri))
      return 1;
  }

  return iparser->factory->chunk(iparser, buffer, len, is_end);
}


static void
raptor_guess_parser_register_factory(raptor_parser_factory *factory) 
{
  factory->context_length     = sizeof(raptor_guess_parser_context);
  
  factory->init      = raptor_guess_parse_init;
  factory->terminate = raptor_guess_parse_terminate;
  factory->chunk     = raptor_guess_parse_chunk;
  factory->content_type_handler = raptor_guess_parse_content_type_handler;
}


void
raptor_init_parser_guess(void)
{
  raptor_parser_register_factory("guess",  
                                 "Guess the parser to use using Content-Type",
                                 NULL, NULL,
                                 NULL,
                                 &raptor_guess_parser_register_factory);
}
