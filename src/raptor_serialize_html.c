/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_serialize_html.c - HTML Table serializer
 *
 * Copyright (C) 2010, David Beckett http://www.dajobe.org/
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
 * Raptor html serializer object
 */
typedef struct {
  int count;
} raptor_html_context;



/* create a new serializer */
static int
raptor_html_serialize_init(raptor_serializer* serializer, const char *name)
{
  return 0;
}


/* destroy a serializer */
static void
raptor_html_serialize_terminate(raptor_serializer* serializer)
{

}


/* start a serialize */
static int
raptor_html_serialize_start(raptor_serializer* serializer)
{
  raptor_html_context * context = (raptor_html_context *)serializer->context;
  raptor_iostream *iostr = serializer->iostream;

  context->count = 0;

  /* XML and HTML declarations */
  raptor_iostream_counted_string_write(
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n", 39, iostr);
  raptor_iostream_counted_string_write(
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\n"
    "        \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\n", 106, iostr);
  raptor_iostream_counted_string_write(
    "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n", 44, iostr);

  raptor_iostream_counted_string_write("<head>\n", 7, iostr);
  raptor_iostream_counted_string_write("  <title>Raptor Graph Serialisation</title>\n", 44, iostr);
  raptor_iostream_counted_string_write("</head>\n", 8, iostr);
  raptor_iostream_counted_string_write("<body>\n", 7, iostr);

  raptor_iostream_counted_string_write(
    "  <table id=\"triples\" border=\"1\">\n", 34, iostr);

  raptor_iostream_counted_string_write("    <tr>\n", 9, iostr);
  raptor_iostream_counted_string_write("      <th>Subject</th>\n", 23, iostr);
  raptor_iostream_counted_string_write("      <th>Predicate</th>\n", 25, iostr);
  raptor_iostream_counted_string_write("      <th>Object</th>\n", 22, iostr);
  raptor_iostream_counted_string_write("    </tr>\n", 10, iostr);

  return 0;
}


/* serialize a term */
static int
raptor_term_html_write(const raptor_term *term, raptor_iostream* iostr)
{
  unsigned char *str;
  size_t len;

  switch(term->type) {
    case RAPTOR_TERM_TYPE_LITERAL:
      raptor_iostream_counted_string_write("<span class=\"literal\">", 22,
                                           iostr);
      raptor_iostream_counted_string_write("<span class=\"value\"", 19, iostr);
      if(term->value.literal.language) {
        len = RAPTOR_LANG_LEN_TO_SIZE_T(term->value.literal.language_len);
        raptor_iostream_counted_string_write(" xml:lang=\"", 11, iostr);
        raptor_xml_escape_string_write(term->value.literal.language, len, '"',
                                       iostr);
        raptor_iostream_write_byte('"', iostr);
      }
      raptor_iostream_write_byte('>', iostr);
      len = term->value.literal.string_len;
      raptor_xml_escape_string_write(term->value.literal.string, len, 0, iostr);
      raptor_iostream_counted_string_write("</span>", 7, iostr);

      if(term->value.literal.datatype) {
        str = raptor_uri_as_counted_string(term->value.literal.datatype, &len);
        raptor_iostream_counted_string_write("^^&lt;<span class=\"datatype\">", 29, iostr);
        raptor_xml_escape_string_write(str, len, 0, iostr);
        raptor_iostream_counted_string_write("</span>&gt;", 11, iostr);
      }
      break;

    case RAPTOR_TERM_TYPE_BLANK:
      len = term->value.blank.string_len;
      raptor_iostream_counted_string_write("<span class=\"blank\">", 20, iostr);
      raptor_iostream_counted_string_write("_:", 2, iostr);
      raptor_xml_escape_string_write(term->value.blank.string, len, 0, iostr);
      break;

    case RAPTOR_TERM_TYPE_URI:
      str = raptor_uri_as_counted_string(term->value.uri, &len);
      raptor_iostream_counted_string_write("<span class=\"uri\">", 18, iostr);
      raptor_iostream_counted_string_write("<a href=\"", 9, iostr);
      raptor_xml_escape_string_write(str, len, '"', iostr);
      raptor_iostream_counted_string_write("\">", 2, iostr);
      raptor_xml_escape_string_write(str, len, 0, iostr);
      raptor_iostream_counted_string_write("</a>", 4, iostr);
      break;

    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
      raptor_log_error_formatted(term->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                                 "Triple has unsupported term type %d",
                                 term->type);
      return 1;
  }

  raptor_iostream_counted_string_write("</span>", 7, iostr);

  return 0;
}


/* serialize a statement */
static int
raptor_html_serialize_statement(raptor_serializer* serializer,
                                    raptor_statement *statement)
{
  raptor_html_context * context = (raptor_html_context *)serializer->context;
  raptor_iostream *iostr = serializer->iostream;

  raptor_iostream_counted_string_write("    <tr class=\"triple\">\n", 24,
                                       iostr);

  /* Subject */
  raptor_iostream_counted_string_write("      <td>", 10, iostr);
  raptor_term_html_write(statement->subject, iostr);
  raptor_iostream_counted_string_write("</td>\n", 6, iostr);

  /* Predicate */
  raptor_iostream_counted_string_write("      <td>", 10, iostr);
  raptor_term_html_write(statement->predicate, iostr);
  raptor_iostream_counted_string_write("</td>\n", 6, iostr);

  /* Object */
  raptor_iostream_counted_string_write("      <td>", 10, iostr);
  raptor_term_html_write(statement->object, iostr);
  raptor_iostream_counted_string_write("</td>\n", 6, iostr);

  raptor_iostream_counted_string_write("    </tr>\n", 10, iostr);

  context->count++;

  return 0;
}


/* end a serialize */
static int
raptor_html_serialize_end(raptor_serializer* serializer)
{
  raptor_html_context * context = (raptor_html_context *)serializer->context;
  raptor_iostream *iostr = serializer->iostream;

  raptor_iostream_counted_string_write("  </table>\n", 11, iostr);

  raptor_iostream_counted_string_write(
    "  <p>Total number of triples: <span class=\"count\">", 50, iostr);
  raptor_iostream_decimal_write(context->count, iostr);
  raptor_iostream_counted_string_write("</span>.</p>\n", 13, iostr);

  raptor_iostream_counted_string_write("</body>\n", 8, iostr);
  raptor_iostream_counted_string_write("</html>\n", 8, iostr);

  return 0;
}


/* finish the serializer factory */
static void
raptor_html_serialize_finish_factory(raptor_serializer_factory* factory)
{
  /* NOP */
}


static const char* const html_names[2] = { "html", NULL};

static const char* const html_uri_strings[2] = {
  "http://www.w3.org/1999/xhtml",
  NULL
};

#define HTML_TYPES_COUNT 2
static const raptor_type_q html_types[HTML_TYPES_COUNT + 1] = {
  { "application/xhtml+xml", 21, 10},
  { "text/html", 9, 10},
  { NULL, 0, 0}
};

static int
raptor_html_serializer_register_factory(raptor_serializer_factory *factory)
{
  factory->desc.names = html_names;
  factory->desc.mime_types = html_types;

  factory->desc.label = "HTML Table";
  factory->desc.uri_strings = html_uri_strings;

  factory->context_length      = sizeof(raptor_html_context);

  factory->init                = raptor_html_serialize_init;
  factory->terminate           = raptor_html_serialize_terminate;
  factory->declare_namespace   = NULL;
  factory->declare_namespace_from_namespace   = NULL;
  factory->serialize_start     = raptor_html_serialize_start;
  factory->serialize_statement = raptor_html_serialize_statement;
  factory->serialize_end       = raptor_html_serialize_end;
  factory->finish_factory      = raptor_html_serialize_finish_factory;

  return 0;
}


int
raptor_init_serializer_html(raptor_world* world)
{
  return !raptor_serializer_register_factory(world,
                                             &raptor_html_serializer_register_factory);
}
