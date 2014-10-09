/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_turtle_writer.c - Raptor Turtle Writer
 *
 * Copyright (C) 2006, Dave Robillard
 * Copyright (C) 2003-2008, David Beckett http://www.dajobe.org/
 * Copyright (C) 2003-2005, University of Bristol, UK http://www.bristol.ac.uk/
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
#include <math.h>

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"

#ifndef STANDALONE


typedef enum {
  TURTLE_WRITER_AUTO_INDENT = 1
} raptor_turtle_writer_flags;


#define TURTLE_WRITER_AUTO_INDENT(turtle_writer) ((turtle_writer->flags & TURTLE_WRITER_AUTO_INDENT) != 0)

struct raptor_turtle_writer_s {
  raptor_world* world;

  int depth;
 
  raptor_uri* base_uri;

  int my_nstack;
  raptor_namespace_stack *nstack;
  int nstack_depth;

  /* outputting to this iostream */
  raptor_iostream *iostr;

  /* Turtle Writer flags - bits defined in enum raptor_turtle_writer_flags */
  int flags;

  /* indentation per level if formatting */
  int indent;
};


/* 16 spaces */
#define SPACES_BUFFER_SIZE sizeof(spaces_buffer)
static const unsigned char spaces_buffer[] = {
  ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' '
};


void
raptor_turtle_writer_increase_indent(raptor_turtle_writer *turtle_writer)
{
  turtle_writer->depth += turtle_writer->indent;
}


void
raptor_turtle_writer_decrease_indent(raptor_turtle_writer *turtle_writer)
{
  turtle_writer->depth -= turtle_writer->indent;
}

  
void
raptor_turtle_writer_newline(raptor_turtle_writer *turtle_writer)
{
  int num_spaces;
  
  raptor_iostream_write_byte('\n', turtle_writer->iostr);
 
  if(!TURTLE_WRITER_AUTO_INDENT(turtle_writer))
    return;

  num_spaces = turtle_writer->depth * turtle_writer->indent;

  while(num_spaces > 0) {
    int count;
    count = (num_spaces > RAPTOR_GOOD_CAST(int, SPACES_BUFFER_SIZE)) ? 
            RAPTOR_GOOD_CAST(int, SPACES_BUFFER_SIZE) : num_spaces;

    raptor_iostream_counted_string_write(spaces_buffer, count, turtle_writer->iostr);

    num_spaces -= count;
  }

  return;
}


/**
 * raptor_new_turtle_writer:
 * @world: raptor_world object
 * @base_uri: Base URI for the writer (or NULL)
 * @write_base_uri: non-0 to write '@base' directive to output
 * @nstack: Namespace stack for the writer to start with (or NULL)
 * @iostr: I/O stream to write to
 * 
 * Constructor - Create a new Turtle Writer writing Turtle to a raptor_iostream
 * 
 * Return value: a new #raptor_turtle_writer object or NULL on failure
 **/
raptor_turtle_writer*
raptor_new_turtle_writer(raptor_world* world,
                         raptor_uri* base_uri, int write_base_uri,
                         raptor_namespace_stack *nstack,
                         raptor_iostream* iostr)
{
  raptor_turtle_writer* turtle_writer;

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  if(!nstack || !iostr)
    return NULL;
  
  raptor_world_open(world);

  turtle_writer = RAPTOR_CALLOC(raptor_turtle_writer*, 1,
                                sizeof(*turtle_writer));

  if(!turtle_writer)
    return NULL;

  turtle_writer->world = world;

  turtle_writer->nstack_depth = 0;

  turtle_writer->nstack = nstack;
  if(!turtle_writer->nstack) {
    turtle_writer->nstack = raptor_new_namespaces(world, 1);
    turtle_writer->my_nstack = 1;
  }

  turtle_writer->iostr = iostr;

  turtle_writer->flags = 0;
  turtle_writer->indent = 2;

  turtle_writer->base_uri = NULL;
  /* Ensure any initial base URI is not written relative */
  if(base_uri && write_base_uri)
    raptor_turtle_writer_base(turtle_writer, base_uri);
  turtle_writer->base_uri = base_uri;

  return turtle_writer;
}


/**
 * raptor_free_turtle_writer:
 * @turtle_writer: Turtle writer object
 *
 * Destructor - Free Turtle Writer
 * 
 **/
void
raptor_free_turtle_writer(raptor_turtle_writer* turtle_writer)
{
  if(!turtle_writer)
    return;

  if(turtle_writer->nstack && turtle_writer->my_nstack)
    raptor_free_namespaces(turtle_writer->nstack);

  RAPTOR_FREE(raptor_turtle_writer, turtle_writer);
}


static int
raptor_turtle_writer_contains_newline(const unsigned char *s, size_t len)
{
  size_t i = 0;

  for( ; i < len; i++)
    if(s[i] == '\n')
      return 1;

  return 0;
}


/**
 * raptor_turtle_writer_raw:
 * @turtle_writer: Turtle writer object
 * @s: raw string to write
 *
 * Write a raw string to the Turtle writer verbatim.
 *
 **/
void
raptor_turtle_writer_raw(raptor_turtle_writer* turtle_writer,
                         const unsigned char *s)
{
  raptor_iostream_string_write(s, turtle_writer->iostr);
}


/**
 * raptor_turtle_writer_raw_counted:
 * @turtle_writer: Turtle writer object
 * @s: raw string to write
 * @len: length of string
 *
 * Write a counted string to the Turtle writer verbatim.
 *
 **/
void
raptor_turtle_writer_raw_counted(raptor_turtle_writer* turtle_writer,
                                 const unsigned char *s, unsigned int len)
{
  raptor_iostream_counted_string_write(s, len, turtle_writer->iostr);
}


/**
 * raptor_turtle_writer_namespace_prefix:
 * @turtle_writer: Turtle writer object
 * @ns: Namespace to write prefix declaration for
 *
 * Write a namespace prefix declaration (@prefix)
 *
 * Must only be used at the beginning of a document.
 */
void
raptor_turtle_writer_namespace_prefix(raptor_turtle_writer* turtle_writer,
                                      raptor_namespace* ns)
{
  raptor_iostream_string_write("@prefix ", turtle_writer->iostr);
  if(ns->prefix)
    raptor_iostream_string_write(raptor_namespace_get_prefix(ns),
                                 turtle_writer->iostr);
  raptor_iostream_counted_string_write(": ", 2, turtle_writer->iostr);
  raptor_turtle_writer_reference(turtle_writer, raptor_namespace_get_uri(ns));
  raptor_iostream_counted_string_write(" .\n", 3, turtle_writer->iostr);
}


/**
 * raptor_turtle_writer_base:
 * @turtle_writer: Turtle writer object
 * @base_uri: New base URI or NULL
 *
 * Write a base URI directive (@base) to set the in-scope base URI
 */
void
raptor_turtle_writer_base(raptor_turtle_writer* turtle_writer,
                          raptor_uri* base_uri)
{
  if(base_uri) {
    raptor_iostream_counted_string_write("@base ", 6, turtle_writer->iostr);
    raptor_turtle_writer_reference(turtle_writer, base_uri);
    raptor_iostream_counted_string_write(" .\n", 3, turtle_writer->iostr);
  }
}


/**
 * raptor_turtle_writer_reference:
 * @turtle_writer: Turtle writer object
 * @uri: URI to write
 *
 * Write a Turtle-encoded URI to the Turtle writer.
 *
 * Return value: non-0 on failure
 **/
int
raptor_turtle_writer_reference(raptor_turtle_writer* turtle_writer, 
                               raptor_uri* uri)
{
  return raptor_uri_escaped_write(uri, turtle_writer->base_uri, 
                                  RAPTOR_ESCAPED_WRITE_TURTLE_URI,
                                  turtle_writer->iostr);
}


/**
 * raptor_turtle_writer_qname:
 * @turtle_writer: Turtle writer object
 * @qname: qname to write
 *
 * Write a QName to the Turtle writer.
 *
 **/
void
raptor_turtle_writer_qname(raptor_turtle_writer* turtle_writer,
                           raptor_qname* qname)
{
  raptor_iostream* iostr = turtle_writer->iostr;
  
  if(qname->nspace && qname->nspace->prefix_length > 0)
    raptor_iostream_counted_string_write(qname->nspace->prefix,
                                         qname->nspace->prefix_length,
                                         iostr);
  raptor_iostream_write_byte(':', iostr);
  
  raptor_iostream_counted_string_write(qname->local_name,
                                       qname->local_name_length,
                                       iostr);
  return;
}


/**
 * raptor_turtle_writer_quoted_counted_string:
 * @turtle_writer: Turtle writer object
 * @s: string to write
 * @len: string length
 *
 * Write a Turtle escaped-string inside double quotes to the writer.
 *
 * Return value: non-0 on failure
 **/
int
raptor_turtle_writer_quoted_counted_string(raptor_turtle_writer* turtle_writer,
                                           const unsigned char *s, size_t len)
{
  const unsigned char *quotes = (const unsigned char *)"\"\"\"\"";
  const unsigned char *q = quotes + 2;
  size_t q_len = 1;
  int flags = RAPTOR_ESCAPED_WRITE_TURTLE_LITERAL;
  int rc = 0;

  if(!s)
    return 1;
  
  /* Turtle """longstring""" (2) or "string" (1) */
  if(raptor_turtle_writer_contains_newline(s, len)) {
    /* long string */
    flags = RAPTOR_ESCAPED_WRITE_TURTLE_LONG_LITERAL;
    q = quotes;
    q_len = 3;
  }

  raptor_iostream_counted_string_write(q, q_len, turtle_writer->iostr);
  rc = raptor_string_escaped_write(s, len, '"',
                                   flags, turtle_writer->iostr);
  raptor_iostream_counted_string_write(q, q_len, turtle_writer->iostr);

  return rc;
}


/*
 * raptor_turtle_writer_literal:
 * @turtle_writer: Turtle writer object
 * @nstack: Namespace stack for making a QName for datatype URI
 * @s: literal string to write (SHARED)
 * @lang: language tag (may be NULL)
 * @datatype: datatype URI (may be NULL)
 *
 * INTERNAL - Write a literal (possibly with lang and datatype) to the Turtle writer.
 *
 * Return value: non-0 on failure
 **/
int
raptor_turtle_writer_literal(raptor_turtle_writer* turtle_writer,
                             raptor_namespace_stack *nstack,
                             const unsigned char* s, const unsigned char* lang,
                             raptor_uri* datatype)
{
  /* DBL_MAX = 309 decimal digits */
  #define INT_MAX_LEN 309 

  /* DBL_EPSILON = 52 digits */
  #define FRAC_MAX_LEN 52

  char* endptr = (char *)s;
  int written = 0;

  /* typed literal special cases */
  if(datatype) {
    /* integer */
    if(raptor_uri_equals(datatype, turtle_writer->world->xsd_integer_uri)) {
      /* FIXME. Work around that gcc < 4.5 cannot disable warn_unused_result */
      long gcc_is_stupid = strtol((const char*)s, &endptr, 10);
      if(endptr != (char*)s && !*endptr) {
        raptor_iostream_string_write(s, turtle_writer->iostr);
        /* More gcc madness to 'use' the variable I didn't want */
        written = 1 + 0 * (int)gcc_is_stupid;
      } else {
        raptor_log_error(turtle_writer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                         "Illegal value for xsd:integer literal.");
      }

    /* double, decimal */
    } else if(raptor_uri_equals(datatype, turtle_writer->world->xsd_double_uri) ||
      raptor_uri_equals(datatype, turtle_writer->world->xsd_decimal_uri)) {
      /* FIXME. Work around that gcc < 4.5 cannot disable warn_unused_result */
      double gcc_is_doubly_stupid = strtod((const char*)s, &endptr);
      if(endptr != (char*)s && !*endptr) {
        raptor_iostream_string_write(s, turtle_writer->iostr);
        /* More gcc madness to 'use' the variable I didn't want */
        written = 1 +  0 * (int)gcc_is_doubly_stupid;
      } else {
        raptor_log_error(turtle_writer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                         "Illegal value for xsd:double or xsd:decimal literal.");
      }

    /* boolean */
    } else if(raptor_uri_equals(datatype, turtle_writer->world->xsd_boolean_uri)) {
      if(!strcmp((const char*)s, "0") || !strcmp((const char*)s, "false")) {
        raptor_iostream_string_write("false", turtle_writer->iostr);
        written = 1;
      } else if(!strcmp((const char*)s, "1") || !strcmp((const char*)s, "true")) {
        raptor_iostream_string_write("true", turtle_writer->iostr);
        written = 1;
      } else {
        raptor_log_error(turtle_writer->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                         "Illegal value for xsd:boolean literal.");
      }
    }
  }

  if(written)
    return 0;
    
  if(raptor_turtle_writer_quoted_counted_string(turtle_writer, s,
                                                strlen((const char*)s)))
    return 1;

  /* typed literal, not a special case */
  if(datatype) {
    raptor_qname* qname;

    raptor_iostream_string_write("^^", turtle_writer->iostr);
    qname = raptor_new_qname_from_namespace_uri(nstack, datatype, 10);
    if(qname) {
      raptor_turtle_writer_qname(turtle_writer, qname);
      raptor_free_qname(qname);
    } else
      raptor_turtle_writer_reference(turtle_writer, datatype);
  } else if(lang) {
    /* literal with language tag */
    raptor_iostream_write_byte('@', turtle_writer->iostr);
    raptor_iostream_string_write(lang, turtle_writer->iostr);
  }

  return 0;
}


/**
 * raptor_turtle_writer_comment:
 * @turtle_writer: Turtle writer object
 * @s: comment string to write
 *
 * Write a Turtle comment to the Turtle writer.
 *
 **/
void
raptor_turtle_writer_comment(raptor_turtle_writer* turtle_writer,
                             const unsigned char *string)
{
  unsigned char c;
  size_t len = strlen((const char*)string);

  raptor_iostream_counted_string_write((const unsigned char*)"# ", 2,
                                       turtle_writer->iostr);

  for(; (c=*string); string++, len--) {
    if(c == '\n') {
      raptor_turtle_writer_newline(turtle_writer);
      raptor_iostream_counted_string_write((const unsigned char*)"# ", 2,
                                           turtle_writer->iostr);
    } else if(c != '\r') { 
      /* skip carriage returns (windows... *sigh*) */
      raptor_iostream_write_byte(c, turtle_writer->iostr);
    }
  }
  
  raptor_turtle_writer_newline(turtle_writer);
}


/**
 * raptor_turtle_writer_set_option:
 * @turtle_writer: #raptor_turtle_writer turtle_writer object
 * @option: option to set from enumerated #raptor_option values
 * @value: integer option value (0 or larger)
 *
 * Set turtle_writer options with integer values.
 * 
 * The allowed options are available via 
 * raptor_world_get_option_description()
 *
 * Return value: non 0 on failure or if the option is unknown
 **/
int
raptor_turtle_writer_set_option(raptor_turtle_writer *turtle_writer, 
                                 raptor_option option, int value)
{
  if(value < 0 ||
     !raptor_option_is_valid_for_area(option, RAPTOR_OPTION_AREA_TURTLE_WRITER))
    return 1;
  
  switch(option) {
    case RAPTOR_OPTION_WRITER_AUTO_INDENT:
      if(value)
        turtle_writer->flags |= TURTLE_WRITER_AUTO_INDENT;
      else
        turtle_writer->flags &= ~TURTLE_WRITER_AUTO_INDENT;        
      break;

    case RAPTOR_OPTION_WRITER_INDENT_WIDTH:
      turtle_writer->indent = value;
      break;
    
    case RAPTOR_OPTION_WRITER_AUTO_EMPTY:
    case RAPTOR_OPTION_WRITER_XML_VERSION:
    case RAPTOR_OPTION_WRITER_XML_DECLARATION:
      break;
        
    /* parser options */
    case RAPTOR_OPTION_SCANNING:
    case RAPTOR_OPTION_ALLOW_NON_NS_ATTRIBUTES:
    case RAPTOR_OPTION_ALLOW_OTHER_PARSETYPES:
    case RAPTOR_OPTION_ALLOW_BAGID:
    case RAPTOR_OPTION_ALLOW_RDF_TYPE_RDF_LIST:
    case RAPTOR_OPTION_NORMALIZE_LANGUAGE:
    case RAPTOR_OPTION_NON_NFC_FATAL:
    case RAPTOR_OPTION_WARN_OTHER_PARSETYPES:
    case RAPTOR_OPTION_CHECK_RDF_ID:
    case RAPTOR_OPTION_HTML_TAG_SOUP:
    case RAPTOR_OPTION_MICROFORMATS:
    case RAPTOR_OPTION_HTML_LINK:
    case RAPTOR_OPTION_WWW_TIMEOUT:
    case RAPTOR_OPTION_STRICT:
      
    /* Shared */
    case RAPTOR_OPTION_NO_NET:
    case RAPTOR_OPTION_NO_FILE:
    case RAPTOR_OPTION_LOAD_EXTERNAL_ENTITIES:

    /* XML writer options */
    case RAPTOR_OPTION_RELATIVE_URIS:

    /* DOT serializer options */
    case RAPTOR_OPTION_RESOURCE_BORDER:
    case RAPTOR_OPTION_LITERAL_BORDER:
    case RAPTOR_OPTION_BNODE_BORDER:
    case RAPTOR_OPTION_RESOURCE_FILL:
    case RAPTOR_OPTION_LITERAL_FILL:
    case RAPTOR_OPTION_BNODE_FILL:

    /* JSON serializer options */
    case RAPTOR_OPTION_JSON_CALLBACK:
    case RAPTOR_OPTION_JSON_EXTRA_DATA:
    case RAPTOR_OPTION_RSS_TRIPLES:
    case RAPTOR_OPTION_ATOM_ENTRY_URI:
    case RAPTOR_OPTION_PREFIX_ELEMENTS:
    
    /* Turtle serializer option */
    case RAPTOR_OPTION_WRITE_BASE_URI:

    /* WWW option */
    case RAPTOR_OPTION_WWW_HTTP_CACHE_CONTROL:
    case RAPTOR_OPTION_WWW_HTTP_USER_AGENT:
    case RAPTOR_OPTION_WWW_CERT_FILENAME:
    case RAPTOR_OPTION_WWW_CERT_TYPE:
    case RAPTOR_OPTION_WWW_CERT_PASSPHRASE:
    case RAPTOR_OPTION_WWW_SSL_VERIFY_PEER:
    case RAPTOR_OPTION_WWW_SSL_VERIFY_HOST:
      
    default:
      return -1;
  }

  return 0;
}


/**
 * raptor_turtle_writer_set_option_string:
 * @turtle_writer: #raptor_turtle_writer turtle_writer object
 * @option: option to set from enumerated #raptor_option values
 * @value: option value
 *
 * Set turtle_writer options with string values.
 * 
 * The allowed options are available via
 * raptor_world_get_option_description().
 * If the option type is integer, the value is interpreted as an
 * integer.
 *
 * Return value: non 0 on failure or if the option is unknown
 **/
int
raptor_turtle_writer_set_option_string(raptor_turtle_writer *turtle_writer, 
                                        raptor_option option, 
                                        const unsigned char *value)
{
  if(!value ||
     !raptor_option_is_valid_for_area(option, RAPTOR_OPTION_AREA_TURTLE_WRITER))
    return 1;

  if(raptor_option_value_is_numeric(option))
    return raptor_turtle_writer_set_option(turtle_writer, option, 
                                            atoi((const char*)value));

  return 1;
}


/**
 * raptor_turtle_writer_get_option:
 * @turtle_writer: #raptor_turtle_writer serializer object
 * @option: option to get value
 *
 * Get various turtle_writer options.
 * 
 * The allowed options are available via raptor_options_enumerate().
 *
 * Note: no option value is negative
 *
 * Return value: option value or < 0 for an illegal option
 **/
int
raptor_turtle_writer_get_option(raptor_turtle_writer *turtle_writer, 
                                 raptor_option option)
{
  int result = -1;

  switch(option) {
    case RAPTOR_OPTION_WRITER_AUTO_INDENT:
      result = TURTLE_WRITER_AUTO_INDENT(turtle_writer);
      break;

    case RAPTOR_OPTION_WRITER_INDENT_WIDTH:
      result = turtle_writer->indent;
      break;
    
    /* writer options */
    case RAPTOR_OPTION_WRITER_AUTO_EMPTY:
    case RAPTOR_OPTION_WRITER_XML_VERSION:
    case RAPTOR_OPTION_WRITER_XML_DECLARATION:
      
    /* parser options */
    case RAPTOR_OPTION_SCANNING:
    case RAPTOR_OPTION_ALLOW_NON_NS_ATTRIBUTES:
    case RAPTOR_OPTION_ALLOW_OTHER_PARSETYPES:
    case RAPTOR_OPTION_ALLOW_BAGID:
    case RAPTOR_OPTION_ALLOW_RDF_TYPE_RDF_LIST:
    case RAPTOR_OPTION_NORMALIZE_LANGUAGE:
    case RAPTOR_OPTION_NON_NFC_FATAL:
    case RAPTOR_OPTION_WARN_OTHER_PARSETYPES:
    case RAPTOR_OPTION_CHECK_RDF_ID:
    case RAPTOR_OPTION_HTML_TAG_SOUP:
    case RAPTOR_OPTION_MICROFORMATS:
    case RAPTOR_OPTION_HTML_LINK:
    case RAPTOR_OPTION_WWW_TIMEOUT:
    case RAPTOR_OPTION_STRICT:

    /* Shared */
    case RAPTOR_OPTION_NO_NET:
    case RAPTOR_OPTION_NO_FILE:
    case RAPTOR_OPTION_LOAD_EXTERNAL_ENTITIES:

    /* XML writer options */
    case RAPTOR_OPTION_RELATIVE_URIS:

    /* DOT serializer options */
    case RAPTOR_OPTION_RESOURCE_BORDER:
    case RAPTOR_OPTION_LITERAL_BORDER:
    case RAPTOR_OPTION_BNODE_BORDER:
    case RAPTOR_OPTION_RESOURCE_FILL:
    case RAPTOR_OPTION_LITERAL_FILL:
    case RAPTOR_OPTION_BNODE_FILL:

    /* JSON serializer options */
    case RAPTOR_OPTION_JSON_CALLBACK:
    case RAPTOR_OPTION_JSON_EXTRA_DATA:
    case RAPTOR_OPTION_RSS_TRIPLES:
    case RAPTOR_OPTION_ATOM_ENTRY_URI:
    case RAPTOR_OPTION_PREFIX_ELEMENTS:
    
    /* Turtle serializer option */
    case RAPTOR_OPTION_WRITE_BASE_URI:

    /* WWW option */
    case RAPTOR_OPTION_WWW_HTTP_CACHE_CONTROL:
    case RAPTOR_OPTION_WWW_HTTP_USER_AGENT:
    case RAPTOR_OPTION_WWW_CERT_FILENAME:
    case RAPTOR_OPTION_WWW_CERT_TYPE:
    case RAPTOR_OPTION_WWW_CERT_PASSPHRASE:
    case RAPTOR_OPTION_WWW_SSL_VERIFY_PEER:
    case RAPTOR_OPTION_WWW_SSL_VERIFY_HOST:
      
    default:
      break;
  }
  
  return result;
}


/**
 * raptor_turtle_writer_get_option_string:
 * @turtle_writer: #raptor_turtle_writer serializer object
 * @option: option to get value
 *
 * Get turtle_writer options with string values.
 * 
 * The allowed options are available via raptor_options_enumerate().
 *
 * Return value: option value or NULL for an illegal option or no value
 **/
const unsigned char *
raptor_turtle_writer_get_option_string(raptor_turtle_writer *turtle_writer, 
                                        raptor_option option)
{
  return NULL;
}


/**
 * raptor_turtle_writer_bnodeid:
 * @turtle_writer: Turtle writer object
 * @bnodeid: blank node ID to write
 * @len: length of @bnodeid
 *
 * Write a blank node ID with leading _: to the Turtle writer.
 *
 **/
void
raptor_turtle_writer_bnodeid(raptor_turtle_writer* turtle_writer,
                             const unsigned char *bnodeid, size_t len)
{
  raptor_bnodeid_ntriples_write(bnodeid, len,
                                turtle_writer->iostr);
}


/**
 * raptor_turtle_writer_uri:
 * @turtle_writer: Turtle writer object
 * @uri: uri
 *
 * Write a #raptor_uri to a turtle writer in qname or URI form
 *
 * Return value: non-0 on failure
 */
int
raptor_turtle_writer_uri(raptor_turtle_writer* turtle_writer,
                         raptor_uri* uri)
{
  raptor_qname* qname;
  int rc = 0;

  if(!uri)
    return 1;

  qname = raptor_new_qname_from_namespace_uri(turtle_writer->nstack, uri, 10);

  /* XML Names allow leading '_' and '.' anywhere but Turtle does not */
  if(qname && !raptor_turtle_is_legal_turtle_qname(qname)) {
    raptor_free_qname(qname);
    qname = NULL;
  }

  if(qname) {
    raptor_turtle_writer_qname(turtle_writer, qname);
    raptor_free_qname(qname);
  } else {
    rc = raptor_turtle_writer_reference(turtle_writer, uri);
  }

  return rc;
}


/**
 * raptor_turtle_writer_term:
 * @turtle_writer: Turtle writer object
 * @term: term
 *
 * Write a #raptor_term to a turtle write
 *
 * Return value: non-0 on failure
 */
int
raptor_turtle_writer_term(raptor_turtle_writer* turtle_writer,
                          raptor_term* term)
{
  int rc = 0;

  if(!term)
    return 1;

  if(term->type == RAPTOR_TERM_TYPE_URI) {
    rc = raptor_turtle_writer_uri(turtle_writer, term->value.uri);
  } else if(term->type == RAPTOR_TERM_TYPE_LITERAL) {
    rc = raptor_turtle_writer_literal(turtle_writer,
                                      turtle_writer->nstack,
                                      term->value.literal.string,
                                      term->value.literal.language, 
                                      term->value.literal.datatype);
  } else if(term->type == RAPTOR_TERM_TYPE_BLANK) {
    rc = raptor_bnodeid_ntriples_write(term->value.blank.string,
                                       term->value.blank.string_len,
                                       turtle_writer->iostr);
  } else {
    rc = 2;
  }
  
  return rc;
}




#endif



#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);


const unsigned char *base_uri_string = (const unsigned char*)"http://example.org/base#";

const unsigned char* longstr = (const unsigned char*)"it's quoted\nand has newlines, \"s <> and\n\ttabbing";

#define OUT_BYTES_COUNT 149

int
main(int argc, char *argv[]) 
{
  raptor_world *world;
  const char *program = raptor_basename(argv[0]);
  raptor_iostream *iostr;
  raptor_namespace_stack *nstack;
  raptor_namespace* ex_ns;
  raptor_turtle_writer* turtle_writer;
  raptor_uri* base_uri;
  raptor_qname* el_name;
  unsigned long count;
  
  /* for raptor_new_iostream_to_string */
  void *string = NULL;
  size_t string_len = 0;

  world = raptor_new_world();
  if(!world || raptor_world_open(world))
    exit(1);
  
  iostr = raptor_new_iostream_to_string(world, &string, &string_len, NULL);
  if(!iostr) {
    fprintf(stderr, "%s: Failed to create iostream to string\n", program);
    exit(1);
  }

  nstack = raptor_new_namespaces(world, 1);

  base_uri = raptor_new_uri(world, base_uri_string);

  turtle_writer = raptor_new_turtle_writer(world, base_uri, 1, nstack, iostr);
  if(!turtle_writer) {
    fprintf(stderr, "%s: Failed to create turtle_writer to iostream\n", program);
    exit(1);
  }

  raptor_turtle_writer_set_option(turtle_writer, 
                                   RAPTOR_OPTION_WRITER_AUTO_INDENT, 1);

  ex_ns = raptor_new_namespace(nstack,
                              (const unsigned char*)"ex",
                              (const unsigned char*)"http://example.org/ns#",
                              0);


  raptor_turtle_writer_namespace_prefix(turtle_writer, ex_ns);

  raptor_turtle_writer_reference(turtle_writer, base_uri);
  
  raptor_turtle_writer_increase_indent(turtle_writer);
  raptor_turtle_writer_newline(turtle_writer);
  
  raptor_turtle_writer_raw(turtle_writer, (const unsigned char*)"ex:foo ");

  raptor_turtle_writer_quoted_counted_string(turtle_writer, longstr,
                                             strlen((const char*)longstr));
  raptor_turtle_writer_raw_counted(turtle_writer,
                                   (const unsigned char*)" ;", 2);
  raptor_turtle_writer_newline(turtle_writer);

  el_name = raptor_new_qname_from_namespace_local_name(world,
                                                       ex_ns,
                                                       (const unsigned char*)"bar", 
                                                       NULL);

  raptor_turtle_writer_qname(turtle_writer, el_name);
  raptor_free_qname(el_name);

  raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)" ", 1);

  raptor_turtle_writer_literal(turtle_writer, nstack,
                               (const unsigned char*)"10.0", NULL,
                               world->xsd_decimal_uri);

  raptor_turtle_writer_newline(turtle_writer);

  raptor_turtle_writer_decrease_indent(turtle_writer);

  raptor_turtle_writer_raw_counted(turtle_writer, (const unsigned char*)".", 1);
  raptor_turtle_writer_newline(turtle_writer);

  
  raptor_free_turtle_writer(turtle_writer);

  raptor_free_namespace(ex_ns);

  raptor_free_namespaces(nstack);

  raptor_free_uri(base_uri);

  
  count = raptor_iostream_tell(iostr);

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  fprintf(stderr, "%s: Freeing iostream\n", program);
#endif
  raptor_free_iostream(iostr);

  if(count != OUT_BYTES_COUNT) {
    fprintf(stderr, "%s: I/O stream wrote %d bytes, expected %d\n", program,
            (int)count, (int)OUT_BYTES_COUNT);
    fputs("[[", stderr);
    (void)fwrite(string, 1, string_len, stderr);
    fputs("]]\n", stderr);
    return 1;
  }
  
  if(!string) {
    fprintf(stderr, "%s: I/O stream failed to create a string\n", program);
    return 1;
  }
  string_len = strlen((const char*)string);
  if(string_len != count) {
    fprintf(stderr, "%s: I/O stream created a string length %d, expected %d\n", program, (int)string_len, (int)count);
    return 1;
  }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  fprintf(stderr, "%s: Made Turtle string of %d bytes\n", program, (int)string_len);
  fputs("[[", stderr);
  (void)fwrite(string, 1, string_len, stderr);
  fputs("]]\n", stderr);
#endif

  raptor_free_memory(string);

  raptor_free_world(world);

  /* keep gcc -Wall happy */
  return(0);
}

#endif
