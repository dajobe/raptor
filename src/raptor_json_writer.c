/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_json_writer.c - Raptor JSON Writer
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
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif


/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"

#ifndef STANDALONE

struct raptor_json_writer_s {
  raptor_world* world;

  raptor_uri* base_uri;
  
  /* outputting to this iostream */
  raptor_iostream *iostr;

  /* current indent */
  int indent;

  /* indent step */
  int indent_step;
};



/**
 * raptor_new_json_writer:
 * @world: raptor_world object
 * @base_uri: Base URI for the writer
 * @iostr: I/O stream to write to
 * 
 * INTERNAL - Constructor - Create a new JSON writer writing to a raptor_iostream
 * 
 * Return value: a new #raptor_json_writer object or NULL on failure
 **/
raptor_json_writer*
raptor_new_json_writer(raptor_world* world,
                       raptor_uri* base_uri,
                       raptor_iostream* iostr)
{
  raptor_json_writer* json_writer;

  json_writer = RAPTOR_CALLOC(raptor_json_writer*, 1, sizeof(*json_writer));

  if(!json_writer)
    return NULL;

  json_writer->world = world;
  json_writer->iostr = iostr;
  json_writer->base_uri = base_uri;

  json_writer->indent_step = 2;
  
  return json_writer;
}


/**
 * raptor_free_json_writer:
 * @json_writer: JSON writer object
 *
 * INTERNAL - Destructor - Free JSON Writer
 * 
 **/
void
raptor_free_json_writer(raptor_json_writer* json_writer)
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN(json_writer, raptor_json_writer);

  RAPTOR_FREE(raptor_json_writer, json_writer);
}


static int
raptor_json_writer_quoted(raptor_json_writer* json_writer,
                          const char *value, size_t value_len)
{
  int rc = 0;
  
  if(!value) {
    raptor_iostream_counted_string_write("\"\"", 2, json_writer->iostr);
    return 0;
  }

  raptor_iostream_write_byte('\"', json_writer->iostr);
  rc = raptor_string_escaped_write((const unsigned char*)value, value_len,
                                   '"', RAPTOR_ESCAPED_WRITE_JSON_LITERAL,
                                   json_writer->iostr);
  raptor_iostream_write_byte('\"', json_writer->iostr);

  return rc;
}


static int
raptor_json_writer_spaces(raptor_json_writer* json_writer, int depth) 
{
  int i;
  for(i = 0; i < depth; i++)
    raptor_iostream_write_byte(' ', json_writer->iostr);
  return 0;
}


int
raptor_json_writer_newline(raptor_json_writer* json_writer)
{
  raptor_iostream_write_byte('\n', json_writer->iostr);
  if(json_writer->indent)
    raptor_json_writer_spaces(json_writer, json_writer->indent);
  return 0;
}


int
raptor_json_writer_key_value(raptor_json_writer* json_writer,
                             const char* key, size_t key_len,
                             const char* value, size_t value_len)
{
  if(!key_len && key)
    key_len = strlen(key);
  if(!value_len && value)
    value_len = strlen(value);
  
  raptor_json_writer_quoted(json_writer, key, key_len);
  raptor_iostream_counted_string_write(" : ", 3, json_writer->iostr);
  raptor_json_writer_quoted(json_writer, value, value_len);

  return 0;
}


int
raptor_json_writer_key_uri_value(raptor_json_writer* json_writer, 
                                 const char* key, size_t key_len,
                                 raptor_uri* uri)
{
  const char* value;
  size_t value_len;
  int rc = 0;
  
  value = (const char*)raptor_uri_to_relative_counted_uri_string(json_writer->base_uri, uri, &value_len);
  if(!value)
    return 1;

  if(key)
    rc = raptor_json_writer_key_value(json_writer, key, key_len, 
                                    value, value_len);
  else
    rc = raptor_json_writer_quoted(json_writer, value, value_len);
  
  RAPTOR_FREE(char*, value);

  return rc;
}


int
raptor_json_writer_start_block(raptor_json_writer* json_writer, char c)
{
  json_writer->indent += json_writer->indent_step;
  raptor_iostream_write_byte(c, json_writer->iostr);
  return 0;
}


int
raptor_json_writer_end_block(raptor_json_writer* json_writer, char c)
{
  raptor_iostream_write_byte(c, json_writer->iostr);
  json_writer->indent -= json_writer->indent_step;
  return 0;
}


int
raptor_json_writer_literal_object(raptor_json_writer* json_writer,
                                  unsigned char* s, size_t s_len,
                                  unsigned char* lang,
                                  raptor_uri* datatype)
{
  raptor_json_writer_start_block(json_writer, '{');
  raptor_json_writer_newline(json_writer);
    
  raptor_iostream_counted_string_write("\"value\" : ", 10, json_writer->iostr);

  raptor_json_writer_quoted(json_writer, (const char*)s, s_len);
  
  if(datatype || lang) {
    raptor_iostream_write_byte(',', json_writer->iostr);
    raptor_json_writer_newline(json_writer);

    if(datatype)
      raptor_json_writer_key_uri_value(json_writer, "datatype", 8, datatype);
    
    if(lang) {
      if(datatype) {
        raptor_iostream_write_byte(',', json_writer->iostr);
        raptor_json_writer_newline(json_writer);
      }

      raptor_json_writer_key_value(json_writer, "lang", 4,
                                   (const char*)lang, 0);
    }
  }

  raptor_iostream_write_byte(',', json_writer->iostr);
  raptor_json_writer_newline(json_writer);

  raptor_json_writer_key_value(json_writer, "type", 4, "literal", 7);

  raptor_json_writer_newline(json_writer);

  raptor_json_writer_end_block(json_writer, '}');
  raptor_json_writer_newline(json_writer);

  return 0;
}


int
raptor_json_writer_blank_object(raptor_json_writer* json_writer,
                                const unsigned char* blank,
                                size_t blank_len)
{
  raptor_json_writer_start_block(json_writer, '{');
  raptor_json_writer_newline(json_writer);

  raptor_iostream_counted_string_write("\"value\" : \"_:", 13,
                                       json_writer->iostr);
  raptor_iostream_counted_string_write((const char*)blank, blank_len,
                                       json_writer->iostr);
  raptor_iostream_counted_string_write("\",", 2, json_writer->iostr);
  raptor_json_writer_newline(json_writer);

  raptor_iostream_counted_string_write("\"type\" : \"bnode\"", 16,
                                       json_writer->iostr);
  raptor_json_writer_newline(json_writer);

  raptor_json_writer_end_block(json_writer, '}');
  return 0;
}


int
raptor_json_writer_uri_object(raptor_json_writer* json_writer,
                              raptor_uri* uri)
{
  raptor_json_writer_start_block(json_writer, '{');
  raptor_json_writer_newline(json_writer);

  raptor_json_writer_key_uri_value(json_writer, "value", 5, uri);
  raptor_iostream_write_byte(',', json_writer->iostr);
  raptor_json_writer_newline(json_writer);

  raptor_iostream_counted_string_write("\"type\" : \"uri\"", 14,
                                       json_writer->iostr);
  raptor_json_writer_newline(json_writer);

  raptor_json_writer_end_block(json_writer, '}');

  return 0;
}


int
raptor_json_writer_term(raptor_json_writer* json_writer,
                        raptor_term *term)
{
  int rc = 0;

  switch(term->type) {
    case RAPTOR_TERM_TYPE_URI:
      rc = raptor_json_writer_uri_object(json_writer, term->value.uri);
      break;

    case RAPTOR_TERM_TYPE_LITERAL:
      rc = raptor_json_writer_literal_object(json_writer,
                                             term->value.literal.string,
                                             term->value.literal.string_len,
                                             term->value.literal.language,
                                             term->value.literal.datatype);
      break;

    case RAPTOR_TERM_TYPE_BLANK:
      rc = raptor_json_writer_blank_object(json_writer,
                                           term->value.blank.string,
                                           term->value.blank.string_len);
      break;

    case RAPTOR_TERM_TYPE_UNKNOWN:
      default:
        raptor_log_error_formatted(json_writer->world, RAPTOR_LOG_LEVEL_ERROR,
                                   NULL,
                                   "Triple has unsupported term type %d",
                                   term->type);
        rc = 1;
        break;
  }

  return rc;
}

#endif
