/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_serialize_json.c - JSON serializers
 *
 * Copyright (C) 2008-2009, David Beckett http://www.dajobe.org/
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


/*
 * Raptor JSON serializer object
 */
typedef struct {
  /* non-0 if json-r otherwise json-t */
  int is_resource;

  int need_subject_comma;

  /* JSON writer object */
  raptor_json_writer* json_writer;

  /* Ordered sequence of triples if is_resource */
  raptor_avltree* avltree;

  /* Last statement generated if is_resource (shared pointer) */
  raptor_statement* last_statement;

  int need_object_comma;

} raptor_json_context;


static int raptor_json_serialize_init(raptor_serializer* serializer,
                                      const char *name);
static void raptor_json_serialize_terminate(raptor_serializer* serializer);
static int raptor_json_serialize_start(raptor_serializer* serializer);
static int raptor_json_serialize_statement(raptor_serializer* serializer, 
                                           raptor_statement *statement);
static int raptor_json_serialize_end(raptor_serializer* serializer);
static void raptor_json_serialize_finish_factory(raptor_serializer_factory* factory);


/*
 * raptor serializer JSON implementation
 */


/* create a new serializer */
static int
raptor_json_serialize_init(raptor_serializer* serializer, const char *name)
{
  raptor_json_context* context = (raptor_json_context*)serializer->context;

  context->is_resource=!strcmp(name,"json");

  /* Default for JSON serializer is absolute URIs */
  /* RAPTOR_OPTIONS_SET_NUMERIC(serializer, RAPTOR_OPTION_RELATIVE_URIS, 0); */
  
  return 0;
}


/* destroy a serializer */
static void
raptor_json_serialize_terminate(raptor_serializer* serializer)
{
  raptor_json_context* context = (raptor_json_context*)serializer->context;

  if(context->json_writer) {
    raptor_free_json_writer(context->json_writer);
    context->json_writer = NULL;
  }

  if(context->avltree) {
    raptor_free_avltree(context->avltree);
    context->avltree = NULL;
  }
}


static int
raptor_json_serialize_start(raptor_serializer* serializer)
{
  raptor_json_context* context = (raptor_json_context*)serializer->context;
  raptor_uri* base_uri;
  char* value;
  
  base_uri = RAPTOR_OPTIONS_GET_NUMERIC(serializer, RAPTOR_OPTION_RELATIVE_URIS)
             ? serializer->base_uri : NULL;
  
  context->json_writer = raptor_new_json_writer(serializer->world,
                                                base_uri,
                                                serializer->iostream);
  if(!context->json_writer)
    return 1;

  if(context->is_resource) {
    context->avltree = raptor_new_avltree((raptor_data_compare_handler)raptor_statement_compare,
                                          (raptor_data_free_handler)raptor_free_statement,
                                          0);
    if(!context->avltree) {
      raptor_free_json_writer(context->json_writer);
      context->json_writer = NULL;
      return 1;
    }
  }

  /* start callback */
  value = RAPTOR_OPTIONS_GET_STRING(serializer, RAPTOR_OPTION_JSON_CALLBACK);
  if(value) {
    raptor_iostream_string_write(value, serializer->iostream);
    raptor_iostream_write_byte('(', serializer->iostream);
  }

  if(!context->is_resource) {
    /* start outer object */
    raptor_json_writer_start_block(context->json_writer, '{');
    raptor_json_writer_newline(context->json_writer);

    /* start triples array */
    raptor_iostream_counted_string_write((const unsigned char*)"\"triples\" : ", 12,
                                         serializer->iostream);
    raptor_json_writer_start_block(context->json_writer, '[');
    raptor_json_writer_newline(context->json_writer);
  }
  
  return 0;
}


static int
raptor_json_serialize_statement(raptor_serializer* serializer, 
                                raptor_statement *statement)
{
  raptor_json_context* context = (raptor_json_context*)serializer->context;

  if(context->is_resource) {
    raptor_statement* s = raptor_statement_copy(statement);
    if(!s)
      return 1;
    return raptor_avltree_add(context->avltree, s);
  }

  if(context->need_subject_comma) {
    raptor_iostream_write_byte(',', serializer->iostream);
    raptor_json_writer_newline(context->json_writer);
  }

  /* start triple */
  raptor_json_writer_start_block(context->json_writer, '{');
  raptor_json_writer_newline(context->json_writer);

  /* subject */
  raptor_iostream_string_write((const unsigned char*)"\"subject\" : ",
                               serializer->iostream);
  raptor_json_writer_term(context->json_writer, statement->subject);
  raptor_iostream_write_byte(',', serializer->iostream);
  raptor_json_writer_newline(context->json_writer);
  
  /* predicate */
  raptor_iostream_string_write((const unsigned char*)"\"predicate\" : ",
                               serializer->iostream);
  raptor_json_writer_term(context->json_writer, statement->predicate);
  raptor_iostream_write_byte(',', serializer->iostream);
  raptor_json_writer_newline(context->json_writer);

  /* object */
  raptor_iostream_string_write((const unsigned char*)"\"object\" : ",
                               serializer->iostream);
  raptor_json_writer_term(context->json_writer, statement->object);
  raptor_json_writer_newline(context->json_writer);

  /* end triple */
  raptor_json_writer_end_block(context->json_writer, '}');

  context->need_subject_comma = 1;
  return 0;
}


/* return 0 to abort visit */
static int
raptor_json_serialize_avltree_visit(int depth, void* data, void *user_data)
{
  raptor_serializer* serializer = (raptor_serializer*)user_data;
  raptor_json_context* context = (raptor_json_context*)serializer->context;

  raptor_statement* statement = (raptor_statement*)data;
  raptor_statement* s1 = statement;
  raptor_statement* s2 = context->last_statement;
  int new_subject = 0;
  int new_predicate = 0;
  unsigned int flags = RAPTOR_ESCAPED_WRITE_JSON_LITERAL;

  if(s2) {
    new_subject = !raptor_term_equals(s1->subject, s2->subject);

    if(new_subject) {
      /* end last predicate */
      raptor_json_writer_newline(context->json_writer);

      raptor_json_writer_end_block(context->json_writer, ']');
      raptor_json_writer_newline(context->json_writer);

      /* end last statement */
      raptor_json_writer_end_block(context->json_writer, '}');
      raptor_json_writer_newline(context->json_writer);

      context->need_subject_comma = 1;
      context->need_object_comma = 0;
    }
  } else
    new_subject = 1;
  
  if(new_subject)  {
    if(context->need_subject_comma) {
      raptor_iostream_write_byte(',', serializer->iostream);
      raptor_json_writer_newline(context->json_writer);
    }

    /* start triple */

    /* subject */
    switch(s1->subject->type) {
      case RAPTOR_TERM_TYPE_URI:
        raptor_json_writer_key_uri_value(context->json_writer, 
                                         NULL, 0,
                                         s1->subject->value.uri);
        break;
        
      case RAPTOR_TERM_TYPE_BLANK:
        raptor_iostream_counted_string_write("\"_:", 3, serializer->iostream);
        raptor_string_escaped_write(s1->subject->value.blank.string, 0,
                                    '"', flags,
                                    serializer->iostream);
        raptor_iostream_write_byte('"', serializer->iostream);
        break;
        
      case RAPTOR_TERM_TYPE_LITERAL:
      case RAPTOR_TERM_TYPE_UNKNOWN:
      default:
        raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR,
                                   NULL,
                                   "Triple has unsupported subject term type %d", 
                                   s1->subject->type);
        break;
    }

    raptor_iostream_counted_string_write(" : ", 3, serializer->iostream);
    raptor_json_writer_start_block(context->json_writer, '{');
  
    raptor_json_writer_newline(context->json_writer);
  }
  

  /* predicate */
  if(context->last_statement) {
    if(new_subject)
      new_predicate = 1;
    else {
      new_predicate = !raptor_uri_equals(s1->predicate->value.uri,
                                         s2->predicate->value.uri);
      if(new_predicate) {
        raptor_json_writer_newline(context->json_writer);
        raptor_json_writer_end_block(context->json_writer, ']');
        raptor_iostream_write_byte(',', serializer->iostream);
        raptor_json_writer_newline(context->json_writer);
      }
    }
  } else
    new_predicate = 1;

  if(new_predicate) {
    /* start predicate */

    raptor_json_writer_key_uri_value(context->json_writer, 
                                   NULL, 0,
                                   s1->predicate->value.uri);
    raptor_iostream_counted_string_write(" : ", 3, serializer->iostream);
    raptor_json_writer_start_block(context->json_writer, '[');
    raptor_iostream_write_byte(' ', serializer->iostream);

    context->need_object_comma = 0;
  }

  if(context->need_object_comma) {
    raptor_iostream_write_byte(',', serializer->iostream);
    raptor_json_writer_newline(context->json_writer);
  }
  
  /* object */
  raptor_json_writer_term(context->json_writer, s1->object);
  if(s1->object->type != RAPTOR_TERM_TYPE_LITERAL)
    raptor_json_writer_newline(context->json_writer);

  /* end triple */

  context->need_object_comma = 1;
  context->last_statement = statement;

  return 1;
}


static int
raptor_json_serialize_end(raptor_serializer* serializer)
{
  raptor_json_context* context = (raptor_json_context*)serializer->context;
  char* value;
  
  raptor_json_writer_newline(context->json_writer);

  if(context->is_resource) {
    /* start outer object */
    raptor_json_writer_start_block(context->json_writer, '{');
    raptor_json_writer_newline(context->json_writer);
    
    raptor_avltree_visit(context->avltree,
                         raptor_json_serialize_avltree_visit,
                         serializer);

    /* end last triples block */
    if(context->last_statement) {
      raptor_json_writer_newline(context->json_writer);
      raptor_json_writer_end_block(context->json_writer, ']');
      raptor_json_writer_newline(context->json_writer);
      
      raptor_json_writer_end_block(context->json_writer, '}');
      raptor_json_writer_newline(context->json_writer);
    }
  } else {
    /* end triples array */
    raptor_json_writer_end_block(context->json_writer, ']');
    raptor_json_writer_newline(context->json_writer);
  }


  value = RAPTOR_OPTIONS_GET_STRING(serializer, RAPTOR_OPTION_JSON_EXTRA_DATA);
  if(value) {
    raptor_iostream_write_byte(',', serializer->iostream);
    raptor_json_writer_newline(context->json_writer);
    raptor_iostream_string_write(value, serializer->iostream);
    raptor_json_writer_newline(context->json_writer);
  }


  /* end outer object */
  raptor_json_writer_end_block(context->json_writer, '}');
  raptor_json_writer_newline(context->json_writer);

  /* end callback */
  if(RAPTOR_OPTIONS_GET_STRING(serializer, RAPTOR_OPTION_JSON_CALLBACK))
    raptor_iostream_counted_string_write((const unsigned char*)");", 2,
                                         serializer->iostream);

  return 0;
}


static void
raptor_json_serialize_finish_factory(raptor_serializer_factory* factory)
{
  /* NOP */
}



static const char* const json_triples_names[3] = { "json-triples", NULL};

#define JSON_TRIPLES_TYPES_COUNT 2
static const raptor_type_q json_triples_types[JSON_TRIPLES_TYPES_COUNT + 1] = {
  { "application/json", 16, 0},
  { "text/json", 9, 1},
  { NULL, 0, 0}
};

static int
raptor_json_triples_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->desc.names = json_triples_names;
  factory->desc.mime_types = json_triples_types;

  factory->desc.label = "RDF/JSON Triples";
  factory->desc.uri_strings = NULL;

  factory->context_length     = sizeof(raptor_json_context);
  
  factory->init                = raptor_json_serialize_init;
  factory->terminate           = raptor_json_serialize_terminate;
  factory->declare_namespace   = NULL;
  factory->declare_namespace_from_namespace   = NULL;
  factory->serialize_start     = raptor_json_serialize_start;
  factory->serialize_statement = raptor_json_serialize_statement;
  factory->serialize_end       = raptor_json_serialize_end;
  factory->finish_factory      = raptor_json_serialize_finish_factory;

  return 0;
}


static const char* const json_resource_names[2] = { "json", NULL};

static const char* const json_resource_uri_strings[2] = {
  "http://docs.api.talis.com/platform-api/output-types/rdf-json",
  NULL
};

#define JSON_RESOURCE_TYPES_COUNT 2
static const raptor_type_q json_resource_types[JSON_RESOURCE_TYPES_COUNT + 1] = {
  { "application/json", 16, 10},
  { "text/json", 9, 1},
  { NULL, 0, 0}
};

static int
raptor_json_resource_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->desc.names = json_resource_names;
  factory->desc.mime_types = json_resource_types;

  factory->desc.label = "RDF/JSON Resource-Centric";
  factory->desc.uri_strings = json_resource_uri_strings;
  
  factory->context_length     = sizeof(raptor_json_context);
  
  factory->init                = raptor_json_serialize_init;
  factory->terminate           = raptor_json_serialize_terminate;
  factory->declare_namespace   = NULL;
  factory->declare_namespace_from_namespace   = NULL;
  factory->serialize_start     = raptor_json_serialize_start;
  factory->serialize_statement = raptor_json_serialize_statement;
  factory->serialize_end       = raptor_json_serialize_end;
  factory->finish_factory      = raptor_json_serialize_finish_factory;

  return 0;
}


int
raptor_init_serializer_json(raptor_world* world)
{
  int rc;
  
  rc = !raptor_serializer_register_factory(world,
                                           &raptor_json_triples_serializer_register_factory);
  if(rc)
    return rc;
  
  rc = !raptor_serializer_register_factory(world,
                                           &raptor_json_resource_serializer_register_factory);

  return rc;
}
