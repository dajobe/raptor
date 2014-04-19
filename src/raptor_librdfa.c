/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_librdfa.c - Raptor RDFA Parser via librdfa implementation
 *
 * Copyright (C) 2008, David Beckett http://www.dajobe.org/
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

#include "rdfa.h"
#include "rdfa_utils.h"


#define RAPTOR_DEFAULT_RDFA_VERSION 0

/*
 * RDFA parser object
 */
struct raptor_librdfa_parser_context_s {
  /* librdfa object */
  rdfacontext* context;
  
  /* static statement for use in passing to user code */
  raptor_statement statement;

  /* 10 for 1.0  11 for 1.1 or otherwise is default (== 1.1) */
  int rdfa_version;
};


typedef struct raptor_librdfa_parser_context_s raptor_librdfa_parser_context;


static int
raptor_librdfa_parse_init(raptor_parser* rdf_parser, const char *name)
{
  raptor_librdfa_parser_context *librdfa_parser;
  int rdfa_version = RAPTOR_DEFAULT_RDFA_VERSION;

  librdfa_parser = (raptor_librdfa_parser_context*)rdf_parser->context;
  
  raptor_statement_init(&rdf_parser->statement, rdf_parser->world);

  if(!strcmp(name, "rdfa11"))
    rdfa_version = 11;
  else if(!strcmp(name, "rdfa10"))
    rdfa_version = 10;

  librdfa_parser->rdfa_version = rdfa_version;

  return 0;
}


static void
raptor_librdfa_parse_terminate(raptor_parser* rdf_parser)
{
  raptor_librdfa_parser_context *librdfa_parser;

  librdfa_parser = (raptor_librdfa_parser_context*)rdf_parser->context;

  if(librdfa_parser->context) {
    rdfa_parse_end(librdfa_parser->context);
    rdfa_free_context(librdfa_parser->context);
    librdfa_parser->context = NULL;
  }
}


static void
raptor_librdfa_generate_statement(rdftriple* triple, void* callback_data)
{
  raptor_parser* parser = (raptor_parser*)callback_data;
  raptor_statement *s = &parser->statement;
  raptor_term *subject_term = NULL;
  raptor_term *predicate_term = NULL;
  raptor_uri *predicate_uri = NULL;
  raptor_term *object_term = NULL;

  if(!parser->emitted_default_graph) {
    raptor_parser_start_graph(parser, NULL, 0);
    parser->emitted_default_graph++;
  }

  if(!parser->statement_handler)
    goto cleanup;

  if(!triple->subject || !triple->predicate || !triple->object) {
#ifdef RAPTOR_DEBUG
    RAPTOR_FATAL1("Triple has NULL parts\n");
#endif
    rdfa_free_triple(triple);
    return;
  }
  
  if(triple->predicate[0] == '_') {
    raptor_parser_warning(parser, 
                          "Ignoring RDFa triple with blank node predicate %s.",
                          triple->predicate);
    rdfa_free_triple(triple);
    return;
  }
  
  if(triple->object_type == RDF_TYPE_NAMESPACE_PREFIX) {
#ifdef RAPTOR_DEBUG
    RAPTOR_FATAL1("Triple has namespace object type\n");
#endif
    rdfa_free_triple(triple);
    return;
  }
  
  if((triple->subject[0] == '_') && (triple->subject[1] == ':')) {
    subject_term = raptor_new_term_from_blank(parser->world,
                                              (const unsigned char*)triple->subject + 2);
  } else {
    raptor_uri* subject_uri;
    
    subject_uri = raptor_new_uri(parser->world,
                                 (const unsigned char*)triple->subject);
    subject_term = raptor_new_term_from_uri(parser->world, subject_uri);
    raptor_free_uri(subject_uri);
    subject_uri = NULL;
  }
  s->subject = subject_term;
  

  predicate_uri = raptor_new_uri(parser->world,
                                 (const unsigned char*)triple->predicate);
  if(!predicate_uri)
    goto cleanup;

  predicate_term = raptor_new_term_from_uri(parser->world, predicate_uri);
  raptor_free_uri(predicate_uri);
  predicate_uri = NULL;
  s->predicate = predicate_term;
 

  if(triple->object_type == RDF_TYPE_IRI) {
    if((triple->object[0] == '_') && (triple->object[1] == ':')) {
      object_term = raptor_new_term_from_blank(parser->world,
                                               (const unsigned char*)triple->object + 2);
    } else {
      raptor_uri* object_uri;
      object_uri = raptor_new_uri(parser->world,
                                  (const unsigned char*)triple->object);
      if(!object_uri)
        goto cleanup;

      object_term = raptor_new_term_from_uri(parser->world, object_uri);
      raptor_free_uri(object_uri);
    }
  } else if(triple->object_type == RDF_TYPE_PLAIN_LITERAL) {
    object_term = raptor_new_term_from_literal(parser->world,
                                               (const unsigned char*)triple->object,
                                               NULL,
                                               (const unsigned char*)triple->language);
    
  } else if(triple->object_type == RDF_TYPE_XML_LITERAL) {
    raptor_uri* datatype_uri;
    datatype_uri = raptor_new_uri_from_counted_string(parser->world,
                                                      (const unsigned char*)raptor_xml_literal_datatype_uri_string,
                                                      raptor_xml_literal_datatype_uri_string_len);
    object_term = raptor_new_term_from_literal(parser->world,
                                               (const unsigned char*)triple->object,
                                               datatype_uri,
                                               NULL);
    raptor_free_uri(datatype_uri);
  } else if(triple->object_type == RDF_TYPE_TYPED_LITERAL) {
    raptor_uri *datatype_uri = NULL;
    const unsigned char* language = (const unsigned char*)triple->language;
    
    if(triple->datatype) {
      /* If datatype, no language allowed */
      language = NULL;
      datatype_uri = raptor_new_uri(parser->world,
                                    (const unsigned char*)triple->datatype);
      if(!datatype_uri)
        goto cleanup;
    }
    
    object_term = raptor_new_term_from_literal(parser->world,
                                               (const unsigned char*)triple->object,
                                               datatype_uri,
                                               language);
    raptor_free_uri(datatype_uri);
  } else {
    raptor_log_error_formatted(parser->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                               "Triple has unknown object term type %d", 
                               s->object->type);
    goto cleanup;
  }
  s->object = object_term;
  
  /* Generate statement */
  (*parser->statement_handler)(parser->user_data, s);

  cleanup:
  rdfa_free_triple(triple);
  
  if(subject_term)
    raptor_free_term(subject_term);
  if(predicate_term)
    raptor_free_term(predicate_term);
  if(object_term)
    raptor_free_term(object_term);
}


static void
raptor_librdfa_sax2_new_namespace_handler(void *user_data,
                                          raptor_namespace* nspace)
{
  raptor_parser* rdf_parser;
  rdf_parser = (raptor_parser*)user_data;
  raptor_parser_start_namespace(rdf_parser, nspace);
}



static int
raptor_librdfa_parse_start(raptor_parser* rdf_parser) 
{
  raptor_locator *locator = &rdf_parser->locator;
  raptor_librdfa_parser_context *librdfa_parser;
  int rc;
  char* base_uri_string = NULL;
  
  librdfa_parser = (raptor_librdfa_parser_context*)rdf_parser->context;

  locator->line = -1;
  locator->column = -1;
  locator->byte = 0;

  if(rdf_parser->base_uri)
    base_uri_string = (char*)raptor_uri_as_string(rdf_parser->base_uri);
  else
    /* base URI is required for rdfa - checked in rdfa_create_context() */
    return 1;

  if(librdfa_parser->context)
    rdfa_free_context(librdfa_parser->context);
  librdfa_parser->context = rdfa_create_context(base_uri_string);
  if(!librdfa_parser->context)
    return 1;

  librdfa_parser->context->namespace_handler = raptor_librdfa_sax2_new_namespace_handler;
  librdfa_parser->context->namespace_handler_user_data = rdf_parser;
  librdfa_parser->context->world = rdf_parser->world;
  librdfa_parser->context->locator = &rdf_parser->locator;
  
  librdfa_parser->context->callback_data = rdf_parser;
  /* returns triples */
  rdfa_set_default_graph_triple_handler(librdfa_parser->context, 
                                        raptor_librdfa_generate_statement);

  /* returns RDFa Processing Graph error triples - not used by raptor */
  rdfa_set_processor_graph_triple_handler(librdfa_parser->context, NULL);

  librdfa_parser->context->raptor_rdfa_version = librdfa_parser->rdfa_version;

  rc = rdfa_parse_start(librdfa_parser->context);
  if(rc != RDFA_PARSE_SUCCESS)
    return 1;
  
  return 0;
}


static int
raptor_librdfa_parse_chunk(raptor_parser* rdf_parser, 
                           const unsigned char *s, size_t len,
                           int is_end)
{
  raptor_librdfa_parser_context *librdfa_parser;
  int rval;

  librdfa_parser = (raptor_librdfa_parser_context*)rdf_parser->context;
  rval = rdfa_parse_chunk(librdfa_parser->context, (char*)s, len, is_end);

  if(is_end) {
    if(rdf_parser->emitted_default_graph) {
      raptor_parser_end_graph(rdf_parser, NULL, 0);
      rdf_parser->emitted_default_graph--;
    }
  }

  return rval != RDFA_PARSE_SUCCESS;
}

static int
raptor_librdfa_parse_recognise_syntax(raptor_parser_factory* factory, 
                                      const unsigned char *buffer, size_t len,
                                      const unsigned char *identifier, 
                                      const unsigned char *suffix, 
                                      const char *mime_type)
{
  int score = 0;
  
  if(identifier) {
    if(strstr((const char*)identifier, "RDFa"))
      score = 10;
  }
  
  if(buffer && len) {
#define  HAS_RDFA_1 (raptor_memstr((const char*)buffer, len, "-//W3C//DTD XHTML+RDFa 1.0//EN") != NULL)
#define  HAS_RDFA_2 (raptor_memstr((const char*)buffer, len, "http://www.w3.org/MarkUp/DTD/xhtml-rdfa-1.dtd") != NULL)

    if(HAS_RDFA_1 || HAS_RDFA_2)
      score = 10;
  }
  
  return score;
}


static const char* const rdfa_names[4] = { "rdfa", "rdfa11", "rdfa10", NULL };

static const char* const rdfa_uri_strings[3] = {
  "http://www.w3.org/ns/formats/RDFa",
  "http://www.w3.org/TR/rdfa/",
  NULL
};
  
#define RDFA_TYPES_COUNT 2
static const raptor_type_q html_types[RDFA_TYPES_COUNT + 1] = {
  { "text/html", 9, 6},
  { "application/xhtml+xml", 21, 8},
  { NULL, 0, 0}
};

static int
raptor_librdfa_parser_register_factory(raptor_parser_factory *factory) 
{
  int rc = 0;

  factory->desc.names = rdfa_names;

  factory->desc.mime_types = html_types;
  
  factory->desc.label = "RDF/A via librdfa";
  factory->desc.uri_strings = rdfa_uri_strings;
  
  factory->desc.flags = RAPTOR_SYNTAX_NEED_BASE_URI;
  
  factory->context_length     = sizeof(raptor_librdfa_parser_context);

  factory->init      = raptor_librdfa_parse_init;
  factory->terminate = raptor_librdfa_parse_terminate;
  factory->start     = raptor_librdfa_parse_start;
  factory->chunk     = raptor_librdfa_parse_chunk;
  factory->recognise_syntax = raptor_librdfa_parse_recognise_syntax;

  return rc;
}


int
raptor_init_parser_rdfa(raptor_world* world)
{
  return !raptor_world_register_parser_factory(world, 
                                               &raptor_librdfa_parser_register_factory);
}
