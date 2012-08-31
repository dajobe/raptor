/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_serialize_dot.c - Serialize RDF graph to GraphViz DOT format
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
 * Raptor dot serializer object
 */
typedef struct {
  raptor_namespace_stack *nstack;
  raptor_sequence *namespaces;

  raptor_sequence *resources;
  raptor_sequence *literals;
  raptor_sequence *bnodes;
} raptor_dot_context;


/* add a namespace */
static int
raptor_dot_serializer_declare_namespace_from_namespace(raptor_serializer* serializer,
                                                       raptor_namespace *nspace)
{
  raptor_dot_context * context = (raptor_dot_context *)serializer->context;
  int i;

  for(i = 0 ; i < raptor_sequence_size(context->namespaces) ; i++ ) {
    raptor_namespace * ns;
    ns = (raptor_namespace *)raptor_sequence_get_at(context->namespaces, i);

    /* If prefix is already declared, ignore it */
    if((!ns->prefix && !nspace->prefix) ||
       (ns->prefix && nspace->prefix &&
        !strcmp((const char*)ns->prefix, (const char*)nspace->prefix)) ||
       (ns->uri && nspace->uri &&
        raptor_uri_equals(ns->uri, nspace->uri)) )
      return 1;
  }

  nspace = raptor_new_namespace_from_uri(context->nstack, nspace->prefix,
                                         nspace->uri, 0);

  if(!nspace)
    return 1;
  
  raptor_sequence_push(context->namespaces, nspace);

  return 0;
}


/* add a namespace */
static int
raptor_dot_serializer_declare_namespace(raptor_serializer* serializer,
                                        raptor_uri* uri,
                                        const unsigned char *prefix)
{
  raptor_dot_context * context = (raptor_dot_context *)serializer->context;
  raptor_namespace *ns;
  int rc;

  ns = raptor_new_namespace_from_uri(context->nstack, prefix, uri, 0);
  rc = raptor_dot_serializer_declare_namespace_from_namespace(serializer, ns);

  raptor_free_namespace(ns);

  return rc;
}


/* create a new serializer */
static int
raptor_dot_serializer_init(raptor_serializer *serializer, const char *name)
{
  raptor_dot_context * context = (raptor_dot_context *)serializer->context;

  /* Setup namespace handling */
  context->nstack = raptor_new_namespaces(serializer->world, 1);
  context->namespaces = raptor_new_sequence((raptor_data_free_handler)raptor_free_namespace, NULL);

  /* We keep a list of nodes to avoid duplication (which isn't
   * critical in graphviz, but why bloat the file?)
   */
  context->resources =
    raptor_new_sequence((raptor_data_free_handler)raptor_free_term, NULL);
  context->literals =
    raptor_new_sequence((raptor_data_free_handler)raptor_free_term, NULL);
  context->bnodes =
    raptor_new_sequence((raptor_data_free_handler)raptor_free_term, NULL);

  return 0;
}


/**
 * raptor_dot_iostream_write_string:
 * @iostr: #raptor_iostream to write to
 * @string: UTF-8 string to write
 * @len: length of UTF-8 string
 * or \0 for no escaping.
 *
 * Write an UTF-8 string, escaped for graphviz.
 *
 * Return value: non-0 on failure.
 **/
static int
raptor_dot_iostream_write_string(raptor_iostream *iostr,
                                 const unsigned char *string)
{
  unsigned char c;

  for( ; (c = *string) ; string++ ) {
    if( (c == '\\') || (c == '"') || (c == '|') ||
        (c == '{')  || (c == '}') ) {
      raptor_iostream_write_byte('\\', iostr);
      raptor_iostream_write_byte(c, iostr);
    } else if( c == '\n' ) {
      raptor_iostream_write_byte('\\', iostr);
      raptor_iostream_write_byte('n', iostr);
    } else
      raptor_iostream_write_byte(c, iostr);
  }

  return 0;
}


static void
raptor_dot_serializer_write_term_type(raptor_serializer * serializer,
                                      raptor_term_type type)
{
  switch(type) {
    case RAPTOR_TERM_TYPE_LITERAL:
      raptor_iostream_write_byte('L', serializer->iostream);
      break;

    case RAPTOR_TERM_TYPE_BLANK:
      raptor_iostream_write_byte('B', serializer->iostream);
      break;

    case RAPTOR_TERM_TYPE_URI:
      raptor_iostream_write_byte('R', serializer->iostream);
      break;

    case RAPTOR_TERM_TYPE_UNKNOWN:
      raptor_iostream_write_byte('?', serializer->iostream);
      break;
  }
}


static void
raptor_dot_serializer_write_uri(raptor_serializer* serializer,
                                raptor_uri* uri)
{
  raptor_dot_context* context = (raptor_dot_context*)serializer->context;
  unsigned char* full = raptor_uri_as_string(uri);
  int i;

  for(i = 0 ; i < raptor_sequence_size(context->namespaces) ; i++ ) {
    raptor_namespace* ns =
      (raptor_namespace*)raptor_sequence_get_at(context->namespaces, i);
    const unsigned char* ns_uri_string;
    size_t ns_uri_string_len;
    ns_uri_string = raptor_uri_as_counted_string(ns->uri, &ns_uri_string_len);

    if(!strncmp((char*)full, (char*)ns_uri_string, ns_uri_string_len) ) {
      const unsigned char* prefix = raptor_namespace_get_prefix(ns);
      
      if(prefix) {
        raptor_iostream_string_write(prefix, serializer->iostream);
        raptor_iostream_write_byte(':', serializer->iostream);
      }

      raptor_iostream_string_write(full + ns_uri_string_len,
                                   serializer->iostream);

      return;
    }
  }

  raptor_iostream_string_write(full, serializer->iostream);
}


static void
raptor_dot_serializer_write_term(raptor_serializer * serializer,
                                 raptor_term* term)
{
  switch(term->type) {
    case RAPTOR_TERM_TYPE_LITERAL:
      raptor_dot_iostream_write_string(serializer->iostream,
                                       term->value.literal.string);
      if(term->value.literal.language) {
        raptor_iostream_write_byte('|', serializer->iostream);
        raptor_iostream_string_write("Language: ", serializer->iostream);
        raptor_iostream_string_write(term->value.literal.language,
                                     serializer->iostream);
      }
      if(term->value.literal.datatype) {
        raptor_iostream_write_byte('|', serializer->iostream);
        raptor_iostream_string_write("Datatype: ", serializer->iostream);
        raptor_dot_serializer_write_uri(serializer, term->value.literal.datatype);
      }
      break;
      
    case RAPTOR_TERM_TYPE_BLANK:
      raptor_iostream_counted_string_write("_:", 2, serializer->iostream);
      raptor_iostream_string_write(term->value.blank.string, serializer->iostream);
      break;
  
    case RAPTOR_TERM_TYPE_URI:
      raptor_dot_serializer_write_uri(serializer, term->value.uri);
      break;
      
    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
      raptor_log_error_formatted(serializer->world, RAPTOR_LOG_LEVEL_ERROR,
                                 NULL, "Triple has unsupported term type %d",
                                 term->type);
  }
}


/* Check the list to see if the node is a duplicate. If not, add it
 * to the list.
 */
static void
raptor_dot_serializer_assert_node(raptor_serializer* serializer,
                                  raptor_term* assert_node)
{
  raptor_dot_context* context = (raptor_dot_context*)serializer->context;
  raptor_sequence* seq = NULL;
  int i;

  /* Which list are we searching? */
  switch(assert_node->type) {
    case RAPTOR_TERM_TYPE_URI:
      seq = context->resources;
      break;

    case RAPTOR_TERM_TYPE_BLANK:
      seq = context->bnodes;
      break;

    case RAPTOR_TERM_TYPE_LITERAL:
      seq = context->literals;
      break;

    case RAPTOR_TERM_TYPE_UNKNOWN:
      break;
  }

  for(i = 0 ; i < raptor_sequence_size(seq) ; i++ ) {
    raptor_term* node = (raptor_term*)raptor_sequence_get_at(seq, i);

    if(raptor_term_equals(node, assert_node))
      return;
  }

  raptor_sequence_push(seq, raptor_term_copy(assert_node));
}


/* start a serialize */
static int
raptor_dot_serializer_start(raptor_serializer* serializer)
{
  raptor_iostream_string_write((const unsigned char*)"digraph {\n\trankdir = LR;\n\tcharset=\"utf-8\";\n\n",
                               serializer->iostream);

  return 0;
}


static int
raptor_dot_serializer_write_colors(raptor_serializer* serializer,
                                   raptor_term_type type)
{
  char* value;
  
  switch(type) {
    case RAPTOR_TERM_TYPE_URI:
      value = RAPTOR_OPTIONS_GET_STRING(serializer,
                                        RAPTOR_OPTION_RESOURCE_BORDER);
      if(value) {
        raptor_iostream_string_write((const unsigned char*)", color=",
                                     serializer->iostream);
        raptor_iostream_string_write((const unsigned char*)value,
                                     serializer->iostream);
      }
      else
        raptor_iostream_string_write((const unsigned char*)", color = blue",
                                     serializer->iostream);

      value = RAPTOR_OPTIONS_GET_STRING(serializer,
                                        RAPTOR_OPTION_RESOURCE_FILL);
      if(value) {
        raptor_iostream_string_write((const unsigned char*)", style = filled, fillcolor=",
                                     serializer->iostream);
        raptor_iostream_string_write((const unsigned char*)value,
                                     serializer->iostream);
      }

      break;

    case RAPTOR_TERM_TYPE_BLANK:
      value = RAPTOR_OPTIONS_GET_STRING(serializer,
                                        RAPTOR_OPTION_BNODE_BORDER);
      if(value) {
        raptor_iostream_string_write((const unsigned char*)", color=",
                                     serializer->iostream);
        raptor_iostream_string_write((const unsigned char*)value,
                                     serializer->iostream);
      }
      else
        raptor_iostream_string_write((const unsigned char*)", color = green",
                                     serializer->iostream);

      value = RAPTOR_OPTIONS_GET_STRING(serializer,
                                        RAPTOR_OPTION_BNODE_FILL);
      if(value) {
        raptor_iostream_string_write((const unsigned char*)", style = filled, fillcolor=",
                                     serializer->iostream);
        raptor_iostream_string_write((const unsigned char*)value,
                                     serializer->iostream);
      }

      break;

    case RAPTOR_TERM_TYPE_LITERAL:
      value = RAPTOR_OPTIONS_GET_STRING(serializer,
                                        RAPTOR_OPTION_LITERAL_BORDER);
      if(value) {
        raptor_iostream_string_write((const unsigned char*)", color=",
                                     serializer->iostream);
        raptor_iostream_string_write((const unsigned char*)value,
                                     serializer->iostream);
      }

      value = RAPTOR_OPTIONS_GET_STRING(serializer,
                                        RAPTOR_OPTION_LITERAL_FILL);
      if(value) {
        raptor_iostream_string_write((const unsigned char*)", style = filled, fillcolor=",
                                     serializer->iostream);
        raptor_iostream_string_write((const unsigned char*)value,
                                     serializer->iostream);
      }

      break;

    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
      break;
  }

  return 0;
}


/* end a serialize */
static int
raptor_dot_serializer_end(raptor_serializer* serializer)
{
  raptor_dot_context* context = (raptor_dot_context*)serializer->context;
  raptor_term* node;
  int i;

  /* Print our nodes. */
  raptor_iostream_string_write((const unsigned char*)"\n\t// Resources\n",
                               serializer->iostream);
  for(i = 0 ; i < raptor_sequence_size(context->resources) ; i++ ) {
    node = (raptor_term*)raptor_sequence_get_at(context->resources, i);
    raptor_iostream_string_write((const unsigned char*)"\t\"R",
                                 serializer->iostream);
    raptor_dot_serializer_write_term(serializer, node);
    raptor_iostream_string_write((const unsigned char*)"\" [ label=\"",
                                 serializer->iostream);
    raptor_dot_serializer_write_term(serializer, node);
    raptor_iostream_string_write((const unsigned char*)"\", shape = ellipse",
                                 serializer->iostream);
    raptor_dot_serializer_write_colors(serializer, RAPTOR_TERM_TYPE_URI);
    raptor_iostream_string_write((const unsigned char*)" ];\n",
                                 serializer->iostream);
    
  }
  raptor_free_sequence(context->resources);

  raptor_iostream_string_write((const unsigned char*)"\n\t// Anonymous nodes\n",
                               serializer->iostream);
  for(i = 0 ; i < raptor_sequence_size(context->bnodes) ; i++ ) {
    node = (raptor_term *)raptor_sequence_get_at(context->bnodes, i);
    raptor_iostream_string_write((const unsigned char*)"\t\"B",
                                 serializer->iostream);
    raptor_dot_serializer_write_term(serializer, node);
    raptor_iostream_string_write((const unsigned char*)"\" [ label=\"",
                                 serializer->iostream);
    raptor_iostream_string_write((const unsigned char*)"\", shape = circle",
                                 serializer->iostream);
    raptor_dot_serializer_write_colors(serializer, RAPTOR_TERM_TYPE_BLANK);
    raptor_iostream_string_write((const unsigned char*)" ];\n",
                                 serializer->iostream);
  }
  raptor_free_sequence(context->bnodes);

  raptor_iostream_string_write((const unsigned char*)"\n\t// Literals\n",
                               serializer->iostream);
  for(i = 0 ; i < raptor_sequence_size(context->literals) ; i++ ) {
    node = (raptor_term*)raptor_sequence_get_at(context->literals, i);
    raptor_iostream_string_write((const unsigned char*)"\t\"L",
                                 serializer->iostream);
    raptor_dot_serializer_write_term(serializer, node);
    raptor_iostream_string_write((const unsigned char*)"\" [ label=\"",
                                 serializer->iostream);
    raptor_dot_serializer_write_term(serializer, node);
    raptor_iostream_string_write((const unsigned char*)"\", shape = record",
                                 serializer->iostream);
    raptor_dot_serializer_write_colors(serializer, RAPTOR_TERM_TYPE_LITERAL);
    raptor_iostream_string_write((const unsigned char*)" ];\n",
                                 serializer->iostream);
  }
  raptor_free_sequence(context->literals);

  raptor_iostream_string_write((const unsigned char*)"\n\tlabel=\"\\n\\nModel:\\n",
                               serializer->iostream);
  if(serializer->base_uri)
    raptor_iostream_string_write(raptor_uri_as_string(serializer->base_uri),
                                 serializer->iostream);
  else
    raptor_iostream_string_write("(Unknown)", serializer->iostream);

  if(raptor_sequence_size(context->namespaces)) {
    raptor_iostream_string_write((const unsigned char*)"\\n\\nNamespaces:\\n",
                                 serializer->iostream);

    for(i = 0 ; i < raptor_sequence_size(context->namespaces) ; i++ ) {
      raptor_namespace* ns;
      const unsigned char* prefix;

      ns = (raptor_namespace*)raptor_sequence_get_at(context->namespaces, i);

      prefix = raptor_namespace_get_prefix(ns);
      if(prefix) {
        raptor_iostream_string_write((const unsigned char*)ns->prefix,
                                     serializer->iostream);
        raptor_iostream_string_write((const unsigned char*)": ",
                                     serializer->iostream);
      }
      raptor_iostream_string_write(raptor_uri_as_string(ns->uri),
                                   serializer->iostream);
      raptor_iostream_string_write((const unsigned char*)"\\n",
                                   serializer->iostream);
    }

    raptor_free_sequence(context->namespaces);
  }

  raptor_iostream_string_write((const unsigned char*)"\";\n",
                               serializer->iostream);

  raptor_iostream_string_write((const unsigned char*) "}\n",
                               serializer->iostream);

  return 0;
}


/* destroy a serializer */
static void
raptor_dot_serializer_terminate(raptor_serializer* serializer)
{
  /* raptor_dot_context* context = (raptor_dot_context*)serializer->context; */

  /* Everything should have been freed in raptor_dot_serializer_end */
}

/* serialize a statement */
static int
raptor_dot_serializer_statement(raptor_serializer* serializer,
                                raptor_statement *statement)
{
  /* Cache the nodes for later. */
  raptor_dot_serializer_assert_node(serializer, statement->subject);
  raptor_dot_serializer_assert_node(serializer, statement->object);

  raptor_iostream_string_write((const unsigned char*)"\t\"",
                               serializer->iostream);
  raptor_dot_serializer_write_term_type(serializer, statement->subject->type);
  raptor_dot_serializer_write_term(serializer, statement->subject);
  raptor_iostream_string_write((const unsigned char*)"\" -> \"",
                               serializer->iostream);
  raptor_dot_serializer_write_term_type(serializer, statement->object->type);
  raptor_dot_serializer_write_term(serializer, statement->object);
  raptor_iostream_string_write((const unsigned char*)"\" [ label=\"",
                               serializer->iostream);
  raptor_dot_serializer_write_term(serializer, statement->predicate);
  raptor_iostream_string_write((const unsigned char*)"\" ];\n",
                               serializer->iostream);

  return 0;
}


static const char* const dot_names[2] = { "dot", NULL};

static const char* const dot_uri_strings[2] = {
  "http://www.graphviz.org/doc/info/lang.html",
  NULL
};

#define DOT_TYPES_COUNT 1
static const raptor_type_q dot_types[DOT_TYPES_COUNT + 1] = {
  { "text/x-graphviz", 15, 5},
  { NULL, 0, 0}
};

static int
raptor_dot_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->desc.names = dot_names;
  factory->desc.mime_types = dot_types;

  factory->desc.label = "GraphViz DOT format";
  factory->desc.uri_strings = dot_uri_strings;;

  factory->context_length = sizeof(raptor_dot_context);

  factory->init = raptor_dot_serializer_init;
  factory->declare_namespace = raptor_dot_serializer_declare_namespace;
  factory->declare_namespace_from_namespace =
    raptor_dot_serializer_declare_namespace_from_namespace;
  factory->serialize_start     = raptor_dot_serializer_start;
  factory->serialize_statement = raptor_dot_serializer_statement;
  factory->serialize_end       = raptor_dot_serializer_end;
  factory->finish_factory      = NULL;
  factory->terminate           = raptor_dot_serializer_terminate;

  return 0;
}


int
raptor_init_serializer_dot(raptor_world* world)
{
  return !raptor_serializer_register_factory(world,
                                             &raptor_dot_serializer_register_factory);
}
