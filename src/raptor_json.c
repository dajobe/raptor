/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_json.c - Raptor JSON Parser
 *
 * JSON
 * http://n2.talis.com/wiki/RDF_JSON_Specification
 *
 * Copyright (C) 2001-2010, David Beckett http://www.dajobe.org/
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

#include <yajl/yajl_parse.h>

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"

/* Set RAPTOR_DEBUG to > 1 to get lots of buffer related debugging */
/*
#undef RAPTOR_DEBUG
#define RAPTOR_DEBUG 2
*/

typedef enum {
  ROOT,
  MAP_ROOT,
  TRIPLES_KEY,
  TRIPLES_ARRAY,
  TRIPLES_TRIPLE,
  TRIPLES_TERM,
  RESOURCES_SUBJECT_KEY,
  RESOURCES_PREDICATE,
  RESOURCES_OBJECT_ARRAY,
  RESOURCES_OBJECT
} raptor_json_parse_state;

enum {
  ATTRIB_UNKNOWN,
  ATTRIB_VALUE,
  ATTRIB_LANG,
  ATTRIB_TYPE,
  ATTRIB_DATATYPE
};

enum {
  TERM_UNKNOWN,
  TERM_SUBJECT,
  TERM_PREDICATE,
  TERM_OBJECT
};


/*
 * JSON parser object
 */
struct raptor_json_parser_context_s {
  yajl_parser_config config;
  yajl_handle handle;

  /* Parser state */
  raptor_json_parse_state state;
  int term;
  int attrib;
  
  /* Temporary storage for resource parsing */
  raptor_term *resource_subject;
  raptor_term *resource_predicate;

  /* Temporary storage, while creating terms */
  raptor_term_type term_type;
  unsigned char* term_value;
  unsigned char* term_datatype;
  unsigned char* term_lang;

  /* Temporary storage, while creating statements */
  raptor_statement statement;
};

typedef struct raptor_json_parser_context_s raptor_json_parser_context;


static raptor_term*
raptor_json_new_term_from_counted_string(raptor_parser *rdf_parser, const unsigned char* str, unsigned int len)
{
  raptor_term *term = NULL;

  if (len > 2 && str[0] == '_' && str[1] == ':') {
    const unsigned char *node_id = &str[2];
    term = raptor_new_term_from_counted_blank(rdf_parser->world, node_id, len-2);

  } else {
    raptor_uri *uri = raptor_new_uri_from_counted_string(rdf_parser->world, str, len);
    if (!uri) {
      raptor_parser_error(rdf_parser, "Could not create uri.");
      return NULL;
    }
  
    term = raptor_new_term_from_uri(rdf_parser->world, uri);
    raptor_free_uri(uri);
  }

  return term;
}



static raptor_term*
raptor_json_generate_term(raptor_parser *rdf_parser)
{
  raptor_json_parser_context *context;
  raptor_term *term = NULL;

  context = (raptor_json_parser_context*)rdf_parser->context;

  switch(context->term_type) {
    case RAPTOR_TERM_TYPE_URI: {
      raptor_uri *uri = raptor_new_uri(rdf_parser->world, context->term_value);
      if(!uri) {
        raptor_parser_error(rdf_parser, "Could not create uri '%s', skipping", context->term_value);
        return NULL;
      }
      term = raptor_new_term_from_uri(rdf_parser->world, uri);
      raptor_free_uri(uri);
      break;
    }
    case RAPTOR_TERM_TYPE_LITERAL: {
      raptor_uri *datatype_uri = NULL;
      if (context->term_datatype) {
        datatype_uri = raptor_new_uri(rdf_parser->world, context->term_datatype);
      }
      term = raptor_new_term_from_literal(rdf_parser->world, context->term_value, datatype_uri, context->term_lang);
      if (datatype_uri) {
        raptor_free_uri(datatype_uri);
      }
      break;
    }
    case RAPTOR_TERM_TYPE_BLANK: {
      unsigned char *node_id = context->term_value;
      if (strlen((const char*)node_id) > 2 && node_id[0] == '_' && node_id[1] == ':') {
          node_id = &node_id[2];
      }
      term = raptor_new_term_from_blank(rdf_parser->world, node_id);
      break;
    }
    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
      fprintf(stderr,"unsupported term type: %d\n", context->term_type);
      break;
  }
  
  return term;
}


static int raptor_json_string(void * ctx, const unsigned char * stringVal,
                           unsigned int stringLen)
{
  raptor_parser* rdf_parser = (raptor_parser*)ctx;
  raptor_json_parser_context *context;
  context = (raptor_json_parser_context*)rdf_parser->context;

  if (context->state == TRIPLES_TERM ||
      context->state == RESOURCES_OBJECT) {
    // FIXME: do we really have to allocate new memory?
    // FIXME: use counted strings instead?
    unsigned char *str = malloc(stringLen+1);
    memcpy(str, stringVal, stringLen);
    str[stringLen] = '\0';

    switch (context->attrib) {
      case ATTRIB_VALUE:
        context->term_value = str;
      break;
      case ATTRIB_LANG:
        context->term_lang = str;
      break;
      case ATTRIB_TYPE:
        if (!strcmp((const char*)str,"uri")) {
          context->term_type = RAPTOR_TERM_TYPE_URI;
        } else if (!strcmp((const char*)str,"literal")) {
          context->term_type = RAPTOR_TERM_TYPE_LITERAL;
        } else if (!strcmp((const char*)str,"bnode")) {
          context->term_type = RAPTOR_TERM_TYPE_BLANK;
        } else {
          context->term_type = RAPTOR_TERM_TYPE_UNKNOWN;
          fprintf(stderr,"unknown term type: %s\n", str);
        }
        free(str);
      break;
      case ATTRIB_DATATYPE:
        context->term_datatype = str;
      break;
      default:
        fprintf(stderr,"unknown term attribute: %d\n", context->attrib);
        free(str);
      break;
    }
  } else {
    fprintf(stderr,"unexpected string\n");
    return 0;
  }
  return 1;
}

static int raptor_json_map_key(void * ctx, const unsigned char * stringVal,
                            unsigned int stringLen)
{
  raptor_parser* rdf_parser = (raptor_parser*)ctx;
  raptor_json_parser_context *context;
  context = (raptor_json_parser_context*)rdf_parser->context;

  if (context->state == MAP_ROOT) {
    if (!strncmp((const char*)stringVal,"triples",stringLen)) {
      context->state = TRIPLES_KEY;
      return 1;
    } else {
      context->resource_subject = raptor_json_new_term_from_counted_string(rdf_parser, stringVal, stringLen);
      context->state = RESOURCES_SUBJECT_KEY;
      return 1;
    }
  } else if (context->state == RESOURCES_PREDICATE) {
    context->resource_predicate = raptor_json_new_term_from_counted_string(rdf_parser, stringVal, stringLen);
    return 1;
  } else if (context->state == TRIPLES_TRIPLE) {
    if (!strncmp((const char*)stringVal,"subject",stringLen)) {
      context->term = TERM_SUBJECT;
      return 1;
    } else if (!strncmp((const char*)stringVal,"predicate",stringLen)) {
      context->term = TERM_PREDICATE;
      return 1;
    } else if (!strncmp((const char*)stringVal,"object",stringLen)) {
      context->term = TERM_OBJECT;
      return 1;
    } else {
      fprintf(stderr,"unexpected key name in triple definition.\n");
      return 0;
    }
  } else if (context->state == TRIPLES_TERM ||
             context->state == RESOURCES_OBJECT) {
    if (!strncmp((const char*)stringVal,"value",stringLen)) {
      context->attrib = ATTRIB_VALUE;
      return 1;
    } else if (!strncmp((const char*)stringVal,"type",stringLen)) {
      context->attrib = ATTRIB_TYPE;
      return 1;
    } else if (!strncmp((const char*)stringVal,"datatype",stringLen)) {
      context->attrib = ATTRIB_DATATYPE;
      return 1;
    } else if (!strncmp((const char*)stringVal,"lang",stringLen)) {
      context->attrib = ATTRIB_LANG;
      return 1;
    } else {
      context->attrib = ATTRIB_UNKNOWN;
      fprintf(stderr,"unexpected key name in triple definition.\n");
      return 0;
    }
    return 1;
  } else {
    fprintf(stderr,"unexpected map key\n");
    return 0;
  }
}

static int raptor_json_start_map(void * ctx)
{
  raptor_parser* rdf_parser = (raptor_parser*)ctx;
  raptor_json_parser_context *context;
  context = (raptor_json_parser_context*)rdf_parser->context;

  if (context->state == ROOT) {
    context->state = MAP_ROOT;
    return 1;
  } else if (context->state == RESOURCES_SUBJECT_KEY) {
    context->state = RESOURCES_PREDICATE;
    return 1;
  } else if (context->state == RESOURCES_OBJECT_ARRAY) {
    context->state = RESOURCES_OBJECT;
    context->attrib = ATTRIB_UNKNOWN;
    context->term_value = NULL;
    context->term_type = RAPTOR_TERM_TYPE_UNKNOWN;
    context->term_datatype = NULL;
    context->term_lang = NULL;
    return 1;
  } else if (context->state == TRIPLES_ARRAY) {
    raptor_statement_init(&context->statement, rdf_parser->world);
    context->term = TERM_UNKNOWN;
    context->state = TRIPLES_TRIPLE;
    return 1;
  } else if (context->state == TRIPLES_TRIPLE) {
    context->state = TRIPLES_TERM;
    context->attrib = ATTRIB_UNKNOWN;
    context->term_value = NULL;
    context->term_type = RAPTOR_TERM_TYPE_UNKNOWN;
    context->term_datatype = NULL;
    context->term_lang = NULL;
    return 1;
  } else {
    fprintf(stderr,"unexpected start of map\n");
    return 0;
  }
}


static int raptor_json_end_map(void * ctx)
{
  raptor_parser* rdf_parser = (raptor_parser*)ctx;
  raptor_json_parser_context *context;
  context = (raptor_json_parser_context*)rdf_parser->context;
  
  if (context->state == RESOURCES_OBJECT) {
    raptor_term *term = raptor_json_generate_term(rdf_parser);
    context->statement.subject = context->resource_subject;
    context->statement.predicate = context->resource_predicate;
    context->statement.object = term;

    /* Generate the statement */
    (*rdf_parser->statement_handler)(rdf_parser->user_data, &context->statement);
    
    context->state = RESOURCES_OBJECT_ARRAY;
    return 1;
  } else if (context->state == RESOURCES_PREDICATE) {
    context->state = MAP_ROOT;
    return 1;
  } else if (context->state == TRIPLES_TERM) {
    raptor_term *term = raptor_json_generate_term(rdf_parser);
    
    // Store it in the statement
    if (term) {
      switch(context->term) {
        case TERM_SUBJECT:
          context->statement.subject = term;
        break;
        case TERM_PREDICATE:
          context->statement.predicate = term;
        break;
        case TERM_OBJECT:
          context->statement.object = term;
        break;
      }
    }

    if (context->term_value)    free(context->term_value);
    if (context->term_lang)     free(context->term_lang);
    if (context->term_datatype) free(context->term_datatype);
    context->state = TRIPLES_TRIPLE;
    return 1;
  } else if (context->state == TRIPLES_TRIPLE) {
    /* Generate the statement */
    (*rdf_parser->statement_handler)(rdf_parser->user_data, &context->statement);
    // FIXME: free the statement
    context->state = TRIPLES_ARRAY;
    return 1;
  } else if (context->state == TRIPLES_KEY) {
    context->state = MAP_ROOT;
    return 1;
  } else if (context->state == MAP_ROOT) {
    context->state = ROOT;
    return 1;
  } else {
    fprintf(stderr,"unexpected end of map\n");
    return 0;
  }
}

static int raptor_json_start_array(void * ctx)
{
  raptor_parser* rdf_parser = (raptor_parser*)ctx;
  raptor_json_parser_context *context;
  context = (raptor_json_parser_context*)rdf_parser->context;

  if (context->state == RESOURCES_PREDICATE) {
    context->state = RESOURCES_OBJECT_ARRAY;
    return 1;
  } else if (context->state == TRIPLES_KEY) {
    context->state = TRIPLES_ARRAY;
    return 1;
  } else {
    fprintf(stderr,"unexpected start of array\n");
    return 0;
  }
}

static int raptor_json_end_array(void * ctx)
{
  raptor_parser* rdf_parser = (raptor_parser*)ctx;
  raptor_json_parser_context *context;
  context = (raptor_json_parser_context*)rdf_parser->context;

  if (context->state == RESOURCES_OBJECT_ARRAY) {
    context->state = RESOURCES_PREDICATE;
    return 1;
  } else if (context->state == TRIPLES_ARRAY) {
    context->state = MAP_ROOT;
    return 1;
  } else {
    fprintf(stderr,"unexpected end of array\n");
    return 0;
  }
}

static yajl_callbacks raptor_json_callbacks = {
  NULL,      /* null is not valid */
  NULL,      /* boolean is not valid */
  NULL,      /* integer is not valid */
  NULL,      /* double is not valid */
  NULL,      /* number is not valid */
  raptor_json_string,
  raptor_json_start_map,
  raptor_json_map_key,
  raptor_json_end_map,
  raptor_json_start_array,
  raptor_json_end_array
};

/**
 * raptor_json_parse_init:
 *
 * Initialise the Raptor JSON parser.
 *
 * Return value: non 0 on failure
 **/

static int
raptor_json_parse_init(raptor_parser* rdf_parser, const char *name)
{
  raptor_json_parser_context *context;
  context = (raptor_json_parser_context*)rdf_parser->context;

  raptor_statement_init(&context->statement, rdf_parser->world);

  /* Configure the parser */
  context->config.allowComments = 0;
  context->config.checkUTF8 = 0;

  // FIXME: use raptor memory allocation functions?
  context->handle = yajl_alloc(&raptor_json_callbacks,
                               &context->config,
                               NULL,
                               (void *)rdf_parser);

  if (!context->handle) {
    fprintf(stderr,"Failed to initialise YAJL parser.\n");
    return 1;
  }
  
  // FIXME: initialise to known state
  context->resource_subject = NULL;
  context->resource_predicate = NULL;

  return 0;
}


/*
 * raptor_json_parse_terminate - Free the Raptor JSON parser
 * @rdf_parser: parser object
 *
 **/
static void
raptor_json_parse_terminate(raptor_parser* rdf_parser)
{
  raptor_json_parser_context *context;
  context = (raptor_json_parser_context*)rdf_parser->context;

  if (context->handle) {
    yajl_free(context->handle);
  }

  // FIXME: clean up any remaining allocated memory
}



static int
raptor_json_parse_chunk(raptor_parser* rdf_parser,
                            const unsigned char *s, size_t len,
                            int is_end)
{
  raptor_json_parser_context *context = (raptor_json_parser_context*)rdf_parser->context;
  yajl_status status;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2("adding %d bytes to buffer\n", (unsigned int)len);
#endif

  if (len) {
    status = yajl_parse(context->handle, s, len);

    if (status != yajl_status_ok &&
        status != yajl_status_insufficient_data)
    {
      unsigned char * str = yajl_get_error(context->handle, 1, s, len);
      fprintf(stderr, "%s", (const char *) str);
      yajl_free_error(context->handle, str);
    }
  }

  if (is_end) {
    /* parse any remaining buffered data */
    status = yajl_parse_complete(context->handle);

    // FIXME: check for errors
  }

  return 0;
}


static int
raptor_json_parse_start(raptor_parser* rdf_parser)
{
  //raptor_json_parser_context *context = (raptor_json_parser_context*)rdf_parser->context;

  // FIXME: should some of the init code be here?

  return 0;
}


static int
raptor_json_parse_recognise_syntax(raptor_parser_factory* factory,
                                       const unsigned char *buffer, size_t len,
                                       const unsigned char *identifier,
                                       const unsigned char *suffix,
                                       const char *mime_type)
{
  int score = 0;

  if(suffix) {
    if(!strcmp((const char*)suffix, "json"))
      score = 8;
  }

  if(mime_type) {
    if(strstr((const char*)mime_type, "json"))
      score += 6;
  }

  // FIXME: check for a curly brace in the buffer?

  return score;
}


static const char* const json_names[2] = { "json", NULL };

#define JSON_TYPES_COUNT 2
static const raptor_type_q json_types[JSON_TYPES_COUNT + 1] = {
  { "application/json", 16, 1},
  { "text/json", 9, 1},
  { NULL, 0, 0}
};

static int
raptor_json_parser_register_factory(raptor_parser_factory *factory)
{
  int rc = 0;

  factory->desc.names = json_names;

  factory->desc.mime_types = json_types;
  factory->desc.mime_types_count = JSON_TYPES_COUNT;

  factory->desc.label = "JSON";
  factory->desc.uri_string = NULL; // "http://n2.talis.com/wiki/RDF_JSON_Specification";

  factory->desc.flags = 0;

  factory->context_length     = sizeof(raptor_json_parser_context);

  factory->init      = raptor_json_parse_init;
  factory->terminate = raptor_json_parse_terminate;
  factory->start     = raptor_json_parse_start;
  factory->chunk     = raptor_json_parse_chunk;
  factory->recognise_syntax = raptor_json_parse_recognise_syntax;

  return rc;
}


int
raptor_init_parser_json(raptor_world* world)
{
  return !raptor_world_register_parser_factory(world,
                                               &raptor_json_parser_register_factory);
}
