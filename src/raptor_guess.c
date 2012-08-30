/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_guess.c - Raptor guessing real parser implementation
 *
 * Copyright (C) 2005-2008, David Beckett http://www.dajobe.org/
 * Copyright (C) 2005, University of Bristol, UK http://www.bristol.ac.uk/
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
 * guess parser object
 */
struct raptor_guess_parser_context_s {
  /* content type got from URI request */
  char* content_type;

  /* URI from start_parse */
  raptor_uri* uri;

  /* Non-0 when we need to guess */
  int do_guess;
  
  /* Actual parser to use */
  raptor_parser* parser;
};


typedef struct raptor_guess_parser_context_s raptor_guess_parser_context;


static int
raptor_guess_parse_init(raptor_parser* rdf_parser, const char *name)
{
  raptor_guess_parser_context *guess_parser = (raptor_guess_parser_context*)rdf_parser->context;
  guess_parser->content_type = NULL;

  guess_parser->do_guess = 1;

  return 0;
}


static void
raptor_guess_parse_terminate(raptor_parser *rdf_parser)
{
  raptor_guess_parser_context *guess_parser = (raptor_guess_parser_context*)rdf_parser->context;

  if(guess_parser->content_type)
    RAPTOR_FREE(char*, guess_parser->content_type);

  if(guess_parser->parser)
    raptor_free_parser(guess_parser->parser);
}


static void
raptor_guess_parse_content_type_handler(raptor_parser* rdf_parser, 
                                        const char* content_type)
{
  raptor_guess_parser_context* guess_parser = (raptor_guess_parser_context*)rdf_parser->context;

  if(content_type) {
    const char *p;
    size_t len;

    if((p = strchr(content_type,';')))
      len = p-content_type;
    else
      len = strlen(content_type);
    
    guess_parser->content_type = RAPTOR_MALLOC(char*, len + 1);
    memcpy(guess_parser->content_type, content_type, len);
    guess_parser->content_type[len]='\0';

    RAPTOR_DEBUG2("Got content type '%s'\n", guess_parser->content_type);
  }
}


static int
raptor_guess_parse_chunk(raptor_parser* rdf_parser, 
                        const unsigned char *buffer, size_t len,
                        int is_end)
{
  raptor_guess_parser_context* guess_parser = (raptor_guess_parser_context*)rdf_parser->context;

  if(guess_parser->do_guess) {
    const unsigned char *identifier = NULL;
    const char *name;
    
    guess_parser->do_guess = 0;

    if(rdf_parser->base_uri)
      identifier = raptor_uri_as_string(rdf_parser->base_uri);
    
    name = raptor_world_guess_parser_name(rdf_parser->world,
                                          NULL, guess_parser->content_type,
                                          buffer, len, identifier);
    if(!name) {
      raptor_parser_error(rdf_parser,
                          "Failed to guess parser from content type '%s'",
                          guess_parser->content_type ? 
                          guess_parser->content_type : "(none)");
      raptor_parser_parse_abort(rdf_parser);
      if(guess_parser->parser) {
        raptor_free_parser(guess_parser->parser);
        guess_parser->parser = NULL;
      }
      return 1;
    } else {
    
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
      RAPTOR_DEBUG2("Guessed parser name '%s'\n", name);
#endif

      /* If there is an existing guessed parser factory present and
       * it's different from the wanted parser, free it
       */
      if(guess_parser->parser) {
        raptor_parser_factory* factory = raptor_world_get_parser_factory(rdf_parser->world, name);

        if(guess_parser->parser->factory != factory) {
          raptor_free_parser(guess_parser->parser);
          guess_parser->parser = NULL;
        }
      }
      
      if(!guess_parser->parser) {
        guess_parser->parser = raptor_new_parser(rdf_parser->world, name);
        if(!guess_parser->parser)
          return 1;
      }

      /* copy any user data to the grddl parser */
      if(raptor_parser_copy_user_state(guess_parser->parser, rdf_parser))
        return 1;
      
      if(raptor_parser_parse_start(guess_parser->parser, rdf_parser->base_uri))
        return 1;
    }
  }
  

  /* now we can pass on calls to internal guess_parser */
  return raptor_parser_parse_chunk(guess_parser->parser, buffer, len, is_end);
}


static const char*
raptor_guess_accept_header(raptor_parser* rdf_parser)
{
  return raptor_parser_get_accept_header_all(rdf_parser->world);
}


static const char*
raptor_guess_guess_get_name(raptor_parser* rdf_parser)
{
  raptor_guess_parser_context *guess_parser;
  guess_parser = (raptor_guess_parser_context*)rdf_parser->context;

  if(guess_parser)
    return raptor_parser_get_name(guess_parser->parser);
  else
    return rdf_parser->factory->desc.names[0];
}


static const raptor_syntax_description*
raptor_guess_guess_get_description(raptor_parser* rdf_parser)
{
  raptor_guess_parser_context *guess_parser;
  guess_parser = (raptor_guess_parser_context*)rdf_parser->context;

  if(guess_parser && guess_parser->parser)
    return raptor_parser_get_description(guess_parser->parser);
  else
    return &rdf_parser->factory->desc;
}


static raptor_locator *
raptor_guess_guess_get_locator(raptor_parser *rdf_parser)
{
  raptor_guess_parser_context *guess_parser;
  guess_parser = (raptor_guess_parser_context*)rdf_parser->context;

  if(guess_parser && guess_parser->parser)
    return raptor_parser_get_locator(guess_parser->parser);
  else
    return &rdf_parser->locator;
}


static const char* const guess_names[2] = { "guess", NULL };

static int
raptor_guess_parser_register_factory(raptor_parser_factory *factory) 
{
  factory->desc.names = guess_names;

  factory->desc.mime_types = NULL;

  factory->desc.label = "Pick the parser to use using content type and URI";
  factory->desc.uri_strings = NULL;
  
  factory->desc.flags = RAPTOR_SYNTAX_NEED_BASE_URI;
  
  factory->context_length     = sizeof(raptor_guess_parser_context);
  
  factory->init      = raptor_guess_parse_init;
  factory->terminate = raptor_guess_parse_terminate;
  factory->chunk     = raptor_guess_parse_chunk;
  factory->content_type_handler = raptor_guess_parse_content_type_handler;
  factory->accept_header = raptor_guess_accept_header;
  factory->get_name = raptor_guess_guess_get_name;
  factory->get_description = raptor_guess_guess_get_description;
  factory->get_locator = raptor_guess_guess_get_locator;

  return 0;
}


int
raptor_init_parser_guess(raptor_world* world)
{
  return !raptor_world_register_parser_factory(world,
                                               &raptor_guess_parser_register_factory);
}
