/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_turtle_writer.c - Raptor Turtle Writer
 *
 * Copyright (C) 2006, Dave Robillard
 * Copyright (C) 2003-2007, David Beckett http://purl.org/net/dajobe/
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
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#include <math.h>

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"

#ifndef STANDALONE


typedef enum {
  TURTLE_WRITER_AUTO_INDENT = 1,
} raptor_turtle_writer_flags;


#define TURTLE_WRITER_AUTO_INDENT(turtle_writer) ((turtle_writer->flags & TURTLE_WRITER_AUTO_INDENT) != 0)

struct raptor_turtle_writer_s {
  int canonicalize;

  int depth;
 
  raptor_uri* base_uri;

  int my_nstack;
  raptor_namespace_stack *nstack;
  int nstack_depth;

  raptor_uri_handler *uri_handler;
  void *uri_context;

  raptor_simple_message_handler error_handler;
  void *error_data;

  /* outputting to this iostream */
  raptor_iostream *iostr;

  /* Turtle Writer flags - bits defined in enum raptor_turtle_writer_flags */
  int flags;

  /* indentation per level if formatting */
  int indent;
};

#define SPACES_BUFFER_SIZE 16
static unsigned char spaces_buffer[SPACES_BUFFER_SIZE];
static int spaces_inited = 0;


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
  
  if(!spaces_inited) {
    int i;
    for(i = 0; i < SPACES_BUFFER_SIZE; i++)
      spaces_buffer[i] = ' ';
    
    spaces_inited = 1;
  }

  raptor_iostream_write_byte(turtle_writer->iostr, '\n');
 
  if(!TURTLE_WRITER_AUTO_INDENT(turtle_writer))
    return;

  num_spaces = turtle_writer->depth * turtle_writer->indent;

  while(num_spaces > 0) {
    int count;
    count = (num_spaces > SPACES_BUFFER_SIZE) ? SPACES_BUFFER_SIZE : num_spaces;

    raptor_iostream_write_counted_string(turtle_writer->iostr, spaces_buffer, count);

    num_spaces -= count;
  }

  return;
}


/**
 * raptor_new_turtle_writer:
 * @base_uri: Base URI for the writer
 * @nstack: Namespace stack for the writer to start with (or NULL)
 * @uri_handler: URI handler function
 * @uri_context: URI handler context data
 * @iostr: I/O stream to write to
 * @error_handler: error handler function
 * @error_data: error handler data
 * 
 * Constructor - Create a new Turtle Writer writing Turtle to a raptor_iostream
 * 
 * Return value: a new #raptor_turtle_writer object or NULL on failure
 **/
raptor_turtle_writer*
raptor_new_turtle_writer(raptor_uri* base_uri,
                         raptor_namespace_stack *nstack,
                         raptor_uri_handler *uri_handler,
                         void *uri_context,
                         raptor_iostream* iostr,
                         raptor_simple_message_handler error_handler,
                         void *error_data)
{
  raptor_turtle_writer* turtle_writer=(raptor_turtle_writer*)RAPTOR_CALLOC(raptor_turtle_writer, 1, sizeof(raptor_turtle_writer)+1);

  if(!turtle_writer)
    return NULL;

  turtle_writer->nstack_depth=0;

  turtle_writer->base_uri = base_uri;

  turtle_writer->uri_handler=uri_handler;
  turtle_writer->uri_context=uri_context;

  turtle_writer->error_handler=error_handler;
  turtle_writer->error_data=error_data;

  turtle_writer->nstack=nstack;
  if(!turtle_writer->nstack) {
    turtle_writer->nstack=nstack=raptor_new_namespaces(uri_handler,
                                                       uri_context,
                                                       error_handler,
                                                       error_data,
                                                       1);
    turtle_writer->my_nstack=1;
  }

  turtle_writer->iostr=iostr;

  turtle_writer->flags = 0;
  turtle_writer->indent = 2;
  
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
  if(turtle_writer->nstack && turtle_writer->my_nstack)
    raptor_free_namespaces(turtle_writer->nstack);

  RAPTOR_FREE(raptor_turtle_writer, turtle_writer);
}


static int
raptor_turtle_writer_contains_newline(const unsigned char *s)
{
  size_t i=0;

  for( ; i < strlen((char*)s); i++)
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
  raptor_iostream_write_string(turtle_writer->iostr, s);
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
  raptor_iostream_write_counted_string(turtle_writer->iostr, s, len);
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
  raptor_iostream_write_string(turtle_writer->iostr, "@prefix ");
  if(ns->prefix)
    raptor_iostream_write_string(turtle_writer->iostr, 
                                 raptor_namespace_get_prefix(ns));
  raptor_iostream_write_string(turtle_writer->iostr, ": <");
  raptor_iostream_write_string(turtle_writer->iostr,
                               raptor_uri_as_string(raptor_namespace_get_uri(ns)));
  raptor_iostream_write_string(turtle_writer->iostr, "> .\n");
}


/**
 * raptor_turtle_writer_reference:
 * @turtle_writer: Turtle writer object
 * @uri: URI to write
 *
 * Write a URI to the Turtle writer.
 *
 **/
void
raptor_turtle_writer_reference(raptor_turtle_writer* turtle_writer, 
                               raptor_uri* uri)
{
  unsigned char* uri_str;
  size_t length;
  
  uri_str = raptor_uri_to_relative_counted_uri_string(turtle_writer->base_uri, uri, &length);

  raptor_iostream_write_byte(turtle_writer->iostr, '<');
  if (uri_str)
    raptor_iostream_write_string_ntriples(turtle_writer->iostr,
                                          uri_str, length, '>');
  raptor_iostream_write_byte(turtle_writer->iostr, '>');

  RAPTOR_FREE(cstring, uri_str);
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
  raptor_iostream* iostr=turtle_writer->iostr;
  
  if(qname->nspace && qname->nspace->prefix_length > 0)
    raptor_iostream_write_counted_string(iostr, qname->nspace->prefix,
                                         qname->nspace->prefix_length);
  raptor_iostream_write_byte(iostr, ':');
  
  raptor_iostream_write_counted_string(iostr, qname->local_name,
                                       qname->local_name_length);
  return;
}


/**
 * raptor_iostream_write_string_turtle:
 * @iostr: #raptor_iostream to write to
 * @string: UTF-8 string to write
 * @len: length of UTF-8 string
 *
 * Write an UTF-8 string using Turtle "longString" triple quoting to an iostream.
 **/
void
raptor_iostream_write_string_turtle(raptor_iostream *iostr,
                                    const unsigned char *string,
                                    size_t len)
{
  unsigned char c;
  
  for(; (c=*string); string++, len--) {
    if(c == '\"') {
      raptor_iostream_write_byte(iostr, '\\');
      raptor_iostream_write_byte(iostr, c);
    } else if(c == '\\') {
      raptor_iostream_write_byte(iostr, '\\');
      raptor_iostream_write_byte(iostr, '\\');
    } else {
      raptor_iostream_write_byte(iostr, c);
    }
  }
}


/**
 * raptor_turtle_writer_quoted:
 * @turtle_writer: Turtle writer object
 * @s: string to write (SHARED)
 *
 * Write a string raw to the Turtle writer.
 **/
void
raptor_turtle_writer_quoted(raptor_turtle_writer* turtle_writer,
                            unsigned char *s)
{
  raptor_stringbuffer* sb = raptor_new_stringbuffer();

  if(raptor_turtle_writer_contains_newline(s)) {
    raptor_iostream_write_string(turtle_writer->iostr, "\"\"\"");
    raptor_iostream_write_string_turtle(turtle_writer->iostr, s, 
                                        strlen((const char*)s));
    raptor_iostream_write_string(turtle_writer->iostr, "\"\"\"");
  } else {
    raptor_stringbuffer_append_turtle_string(sb, s, strlen((const char*)s),
                                             (int)'"',
        turtle_writer->error_handler, turtle_writer->error_data);

    raptor_iostream_write_byte(turtle_writer->iostr, '\"');

    /* FIXME: over-escapes things because ntriples is ASCII */
    raptor_iostream_write_string_ntriples(turtle_writer->iostr, s,
                                          strlen((const char*)s), '"');
    raptor_iostream_write_byte(turtle_writer->iostr, '\"');
  }

  raptor_free_stringbuffer(sb);
}


/* Convert num into turtle xsd:double format in buf.
 * Output form is xsd:double canonical form, excluding NaN, INF, -INF.
 * Returns nonzero on failure (num is unrepresentable) */
static int
raptor_turtle_writer_double(raptor_turtle_writer* turtle_writer,
                                    raptor_namespace_stack *nstack,
                                    char* buf, size_t buf_len,
                                    raptor_uri* datatype, double num)
{
  size_t e_index = 0;
  size_t trailing_zero_start = 0;
  size_t exponent_start;
  raptor_qname* qname;
  qname = raptor_namespaces_qname_from_uri(nstack, datatype, 10);
  
  if(num == 0.0f) {
    raptor_iostream_write_counted_string(turtle_writer->iostr, "0.0E0", 5);
    return 0;
  } else if(isnan(num) || isinf(num)) {
    raptor_iostream_write_byte(turtle_writer->iostr, '"');
    if(isnan(num)) {
      raptor_iostream_write_counted_string(turtle_writer->iostr, "NaN", 3);
    } else {
      if(isinf(num) < 0)
        raptor_iostream_write_counted_string(turtle_writer->iostr, "-INF", 4);
      else
        raptor_iostream_write_counted_string(turtle_writer->iostr, "INF", 3);
    }
    raptor_iostream_write_counted_string(turtle_writer->iostr, "\"^^", 3);
    
    if(qname) {
      raptor_turtle_writer_qname(turtle_writer, qname);
      raptor_free_qname(qname);
    } else
      raptor_turtle_writer_reference(turtle_writer, datatype);
    return 0;
  }

  snprintf(buf, buf_len, "%1.14E", num);

  /* now munge snprintf output into turtle form (yes, ick) */

  /* find the 'E' and start of mantissa trailing zeros */

  for( ; buf[e_index]; ++e_index) {
    if(e_index > 0 && buf[e_index] == '0' && buf[e_index-1] != '0')
      trailing_zero_start = e_index;
    else if(buf[e_index] == 'E')
      break;
  }

  if(buf[trailing_zero_start-1] == '.')
    ++trailing_zero_start;

  /* write an 'E' where the trailing zeros started */
  buf[trailing_zero_start] = 'E';
  if(buf[e_index+1] == '-') {
    buf[trailing_zero_start+1] = '-';
    ++trailing_zero_start;
  }

  exponent_start = e_index+2;
  while(buf[exponent_start] == '0')
    ++exponent_start;

  buf_len = strlen(buf);
  if(exponent_start == buf_len) {
    buf[trailing_zero_start+1] = '0';
    buf[trailing_zero_start+2] = '\0';
  } else {
    /* copy the exponent (minus leading zeros) after the new E */
    memmove(buf+trailing_zero_start+1, buf+exponent_start,
            buf_len-trailing_zero_start);
  }

  raptor_iostream_write_string(turtle_writer->iostr, buf);

  return 0;
}


/**
 * raptor_turtle_writer_literal:
 * @turtle_writer: Turtle writer object
 * @nstack: Namespace stack for making a QName for datatype URI
 * @s: literal string to write (SHARED)
 * @lang: language tag (may be NULL)
 * @datatype: datatype URI (may be NULL)
 *
 * Write a literal (possibly with lang and datatype) to the Turtle writer.
 **/
void
raptor_turtle_writer_literal(raptor_turtle_writer* turtle_writer,
                             raptor_namespace_stack *nstack,
                             unsigned char* s, unsigned char* lang,
                             raptor_uri* datatype)
{
  /* DBL_MAX = 309 decimal digits */
  #define INT_MAX_LEN 309 

  /* DBL_EPSILON = 52 digits */
  #define FRAC_MAX_LEN 52
  
  const size_t buflen = INT_MAX_LEN + FRAC_MAX_LEN + 3; /* sign, decimal, \0 */
  char buf[buflen];

  size_t len = 0;
  char* endptr = (char *)s;
  int written = 0;

  /* typed literal special cases */
  if(datatype) {
    const char* type_uri_str = (const char*)raptor_uri_as_string(datatype);

    /* integer */
    if(!strcmp(type_uri_str, "http://www.w3.org/2001/XMLSchema#integer")) {
      long inum = strtol((const char*)s, NULL, 10);
      if(inum != LONG_MIN && inum != LONG_MAX) {
        snprintf(buf, 20, "%ld", inum);
        raptor_iostream_write_string(turtle_writer->iostr, buf);
        written = 1;
      }
    
    /* double */
    } else if(!strcmp(type_uri_str, "http://www.w3.org/2001/XMLSchema#double")) {
      double dnum = strtod((const char*)s, &endptr);
      if(endptr != (char*)s) {
        written= !raptor_turtle_writer_double(turtle_writer, nstack,
                                              buf, 40,
                                              datatype, dnum);
        
        if(!written) {
          turtle_writer->error_handler(turtle_writer->error_data, "Illegal value for xsd:double literal.");
        }
      }
    
    /* decimal */
    } else if(!strcmp(type_uri_str, "http://www.w3.org/2001/XMLSchema#decimal")) {
      double dnum = strtod((const char*)s, &endptr);
      if(endptr != (char*)s) {
        const char* decimal = strchr((const char*)s, '.');
        const size_t max_digits = (decimal ? (endptr - decimal - 2) : 1);
        char* num_str = raptor_format_float(buf, &len, buflen, dnum, 1, max_digits, 0);
        raptor_iostream_write_counted_string(turtle_writer->iostr, num_str, len);
        written = 1;
      }
    
    /* boolean */
    } else if(!strcmp(type_uri_str, "http://www.w3.org/2001/XMLSchema#boolean")) {
      if(!strcmp((const char*)s, "0") || !strcmp((const char*)s, "false")) {
        raptor_iostream_write_string(turtle_writer->iostr, "false");
        written = 1;
      } else if(!strcmp((const char*)s, "1") || !strcmp((const char*)s, "true")) {
        raptor_iostream_write_string(turtle_writer->iostr, "true");
        written = 1;
      } else {
        turtle_writer->error_handler(turtle_writer->error_data, "Illegal value for xsd:boolean literal.");
      }
    }
  }

  if(written)
    return;
    
  raptor_turtle_writer_quoted(turtle_writer, s);

  /* typed literal, not a special case */
  if(datatype) {
    raptor_qname* qname;

    raptor_iostream_write_string(turtle_writer->iostr, "^^");
    qname = raptor_namespaces_qname_from_uri(nstack, datatype, 10);
    if(qname) {
      raptor_turtle_writer_qname(turtle_writer, qname);
      raptor_free_qname(qname);
    } else
      raptor_turtle_writer_reference(turtle_writer, datatype);
  } else if(lang) {
    /* literal with language tag */
    raptor_iostream_write_byte(turtle_writer->iostr, '@');
    raptor_iostream_write_string(turtle_writer->iostr, lang);
  }
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

  raptor_iostream_write_counted_string(turtle_writer->iostr,
                                       (const unsigned char*)"# ", 2);

  for(; (c=*string); string++, len--) {
    if(c == '\n') {
      raptor_turtle_writer_newline(turtle_writer);
      raptor_iostream_write_counted_string(turtle_writer->iostr,
                                           (const unsigned char*)"# ", 2);
    } else if(c != '\r') { 
      /* skip carriage returns (windows... *sigh*) */
      raptor_iostream_write_byte(turtle_writer->iostr, c);
    }
  }
  
  raptor_turtle_writer_newline(turtle_writer);
}


/**
 * raptor_turtle_writer_features_enumerate:
 * @feature: feature enumeration (0+)
 * @name: pointer to store feature short name (or NULL)
 * @uri: pointer to store feature URI (or NULL)
 * @label: pointer to feature label (or NULL)
 *
 * Get list of turtle_writer features.
 * 
 * If uri is not NULL, a pointer to a new raptor_uri is returned
 * that must be freed by the caller with raptor_free_uri().
 *
 * Return value: 0 on success, <0 on failure, >0 if feature is unknown
 **/
int
raptor_turtle_writer_features_enumerate(const raptor_feature feature,
                                        const char **name, 
                                        raptor_uri **uri, const char **label)
{
  return raptor_features_enumerate_common(feature, name, uri, label, 8);
}


/**
 * raptor_turtle_writer_set_feature:
 * @turtle_writer: #raptor_turtle_writer turtle_writer object
 * @feature: feature to set from enumerated #raptor_feature values
 * @value: integer feature value (0 or larger)
 *
 * Set turtle_writer features with integer values.
 * 
 * The allowed features are available via raptor_features_enumerate().
 *
 * Return value: non 0 on failure or if the feature is unknown
 **/
int
raptor_turtle_writer_set_feature(raptor_turtle_writer *turtle_writer, 
                                 raptor_feature feature, int value)
{
  if(value < 0)
    return -1;
  
  switch(feature) {
    case RAPTOR_FEATURE_WRITER_AUTO_INDENT:
      if(value)
        turtle_writer->flags |= TURTLE_WRITER_AUTO_INDENT;
      else
        turtle_writer->flags &= ~TURTLE_WRITER_AUTO_INDENT;        
      break;

    case RAPTOR_FEATURE_WRITER_INDENT_WIDTH:
      turtle_writer->indent = value;
      break;
    
    case RAPTOR_FEATURE_WRITER_AUTO_EMPTY:
	case RAPTOR_FEATURE_WRITER_XML_VERSION:
    case RAPTOR_FEATURE_WRITER_XML_DECLARATION:
      break;
        
    /* parser features */
    case RAPTOR_FEATURE_SCANNING:
    case RAPTOR_FEATURE_ASSUME_IS_RDF:
    case RAPTOR_FEATURE_ALLOW_NON_NS_ATTRIBUTES:
    case RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES:
    case RAPTOR_FEATURE_ALLOW_BAGID:
    case RAPTOR_FEATURE_ALLOW_RDF_TYPE_RDF_LIST:
    case RAPTOR_FEATURE_NORMALIZE_LANGUAGE:
    case RAPTOR_FEATURE_NON_NFC_FATAL:
    case RAPTOR_FEATURE_WARN_OTHER_PARSETYPES:
    case RAPTOR_FEATURE_CHECK_RDF_ID:
    case RAPTOR_FEATURE_HTML_TAG_SOUP:
    case RAPTOR_FEATURE_MICROFORMATS:
    case RAPTOR_FEATURE_HTML_LINK:

    /* Shared */
    case RAPTOR_FEATURE_NO_NET:

    /* XML writer features */
    case RAPTOR_FEATURE_RELATIVE_URIS:
    case RAPTOR_FEATURE_START_URI:

    /* DOT serializer features */
    case RAPTOR_FEATURE_RESOURCE_BORDER:
    case RAPTOR_FEATURE_LITERAL_BORDER:
    case RAPTOR_FEATURE_BNODE_BORDER:
    case RAPTOR_FEATURE_RESOURCE_FILL:
    case RAPTOR_FEATURE_LITERAL_FILL:
    case RAPTOR_FEATURE_BNODE_FILL:

    default:
      return -1;
      break;
  }

  return 0;
}


/**
 * raptor_turtle_writer_set_feature_string:
 * @turtle_writer: #raptor_turtle_writer turtle_writer object
 * @feature: feature to set from enumerated #raptor_feature values
 * @value: feature value
 *
 * Set turtle_writer features with string values.
 * 
 * The allowed features are available via raptor_turtle_writer_features_enumerate().
 * If the feature type is integer, the value is interpreted as an integer.
 *
 * Return value: non 0 on failure or if the feature is unknown
 **/
int
raptor_turtle_writer_set_feature_string(raptor_turtle_writer *turtle_writer, 
                                        raptor_feature feature, 
                                        const unsigned char *value)
{
  int value_is_string=(raptor_feature_value_type(feature) == 1);
  if(!value_is_string)
    return raptor_turtle_writer_set_feature(turtle_writer, feature, 
                                            atoi((const char*)value));
  else
    return -1;
}


/**
 * raptor_turtle_writer_get_feature:
 * @turtle_writer: #raptor_turtle_writer serializer object
 * @feature: feature to get value
 *
 * Get various turtle_writer features.
 * 
 * The allowed features are available via raptor_features_enumerate().
 *
 * Note: no feature value is negative
 *
 * Return value: feature value or < 0 for an illegal feature
 **/
int
raptor_turtle_writer_get_feature(raptor_turtle_writer *turtle_writer, 
                                 raptor_feature feature)
{
  int result= -1;

  switch(feature) {
    case RAPTOR_FEATURE_WRITER_AUTO_INDENT:
      result=TURTLE_WRITER_AUTO_INDENT(turtle_writer);
      break;

    case RAPTOR_FEATURE_WRITER_INDENT_WIDTH:
      result=turtle_writer->indent;
      break;
    
    /* writer features */
    case RAPTOR_FEATURE_WRITER_AUTO_EMPTY:
    case RAPTOR_FEATURE_WRITER_XML_VERSION:
    case RAPTOR_FEATURE_WRITER_XML_DECLARATION:
      
    /* parser features */
    case RAPTOR_FEATURE_SCANNING:
    case RAPTOR_FEATURE_ASSUME_IS_RDF:
    case RAPTOR_FEATURE_ALLOW_NON_NS_ATTRIBUTES:
    case RAPTOR_FEATURE_ALLOW_OTHER_PARSETYPES:
    case RAPTOR_FEATURE_ALLOW_BAGID:
    case RAPTOR_FEATURE_ALLOW_RDF_TYPE_RDF_LIST:
    case RAPTOR_FEATURE_NORMALIZE_LANGUAGE:
    case RAPTOR_FEATURE_NON_NFC_FATAL:
    case RAPTOR_FEATURE_WARN_OTHER_PARSETYPES:
    case RAPTOR_FEATURE_CHECK_RDF_ID:
    case RAPTOR_FEATURE_HTML_TAG_SOUP:
    case RAPTOR_FEATURE_MICROFORMATS:
    case RAPTOR_FEATURE_HTML_LINK:

    /* Shared */
    case RAPTOR_FEATURE_NO_NET:

    /* XML writer features */
    case RAPTOR_FEATURE_RELATIVE_URIS:
    case RAPTOR_FEATURE_START_URI:

    /* DOT serializer features */
    case RAPTOR_FEATURE_RESOURCE_BORDER:
    case RAPTOR_FEATURE_LITERAL_BORDER:
    case RAPTOR_FEATURE_BNODE_BORDER:
    case RAPTOR_FEATURE_RESOURCE_FILL:
    case RAPTOR_FEATURE_LITERAL_FILL:
    case RAPTOR_FEATURE_BNODE_FILL:

    default:
      break;
  }
  
  return result;
}


/**
 * raptor_turtle_writer_get_feature_string:
 * @turtle_writer: #raptor_turtle_writer serializer object
 * @feature: feature to get value
 *
 * Get turtle_writer features with string values.
 * 
 * The allowed features are available via raptor_features_enumerate().
 *
 * Return value: feature value or NULL for an illegal feature or no value
 **/
const unsigned char *
raptor_turtle_writer_get_feature_string(raptor_turtle_writer *turtle_writer, 
                                        raptor_feature feature)
{
  return NULL;
}

#endif



#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);


const unsigned char *base_uri_string=(const unsigned char*)"http://example.org/base#";

#define OUT_BYTES_COUNT 135

int
main(int argc, char *argv[]) 
{
#if 0
  const char *program=raptor_basename(argv[0]);
  raptor_uri_handler *uri_handler;
  void *uri_context;
  raptor_iostream *iostr;
  raptor_namespace_stack *nstack;
  raptor_namespace* foo_ns;
  raptor_turtle_writer* turtle_writer;
  raptor_uri* base_uri;
  raptor_qname* el_name;
  raptor_xml_element *element;
  size_t count;
  raptor_qname **attrs;
  raptor_uri* base_uri_copy=NULL;

  /* for raptor_new_iostream_to_string */
  void *string=NULL;
  size_t string_len=0;

  raptor_init();
  
  iostr=raptor_new_iostream_to_string(&string, &string_len, NULL);
  if(!iostr) {
    fprintf(stderr, "%s: Failed to create iostream to string\n", program);
    exit(1);
  }

  raptor_uri_get_handler(&uri_handler, &uri_context);

  nstack=raptor_new_namespaces(uri_handler, uri_context,
                               NULL, NULL, /* errors */
                               1);

  turtle_writer=raptor_new_turtle_writer(nstack,
                                   uri_handler, uri_context,
                                   iostr,
                                   NULL, NULL, /* errors */
                                   1);
  if(!turtle_writer) {
    fprintf(stderr, "%s: Failed to create turtle_writer to iostream\n", program);
    exit(1);
  }

  base_uri=raptor_new_uri(base_uri_string);

  foo_ns=raptor_new_namespace(nstack,
                              (const unsigned char*)"foo",
                              (const unsigned char*)"http://example.org/foo-ns#",
                              0);


  el_name=raptor_new_qname_from_namespace_local_name(foo_ns,
                                                     (const unsigned char*)"bar", 
                                                     NULL);
  base_uri_copy=base_uri ? raptor_uri_copy(base_uri) : NULL;
  element=raptor_new_xml_element(el_name,
                                  NULL, /* language */
                                  base_uri_copy);

  raptor_turtle_writer_start_element(turtle_writer, element);
  raptor_turtle_writer_cdata_counted(turtle_writer, (const unsigned char*)"hello\n", 6);
  raptor_turtle_writer_comment_counted(turtle_writer, (const unsigned char*)"comment", 7);
  raptor_turtle_writer_cdata(turtle_writer, (const unsigned char*)"\n");
  raptor_turtle_writer_end_element(turtle_writer, element);

  raptor_free_xml_element(element);

  raptor_turtle_writer_cdata(turtle_writer, (const unsigned char*)"\n");

  el_name=raptor_new_qname(nstack, 
                           (const unsigned char*)"blah", 
                           NULL, /* no attribute value - element */
                           NULL, NULL); /* errors */
  base_uri_copy=base_uri ? raptor_uri_copy(base_uri) : NULL;
  element=raptor_new_xml_element(el_name,
                                  NULL, /* language */
                                  base_uri_copy);

  attrs=(raptor_qname **)RAPTOR_CALLOC(qnamearray, 1, sizeof(raptor_qname*));
  attrs[0]=raptor_new_qname(nstack, 
                            (const unsigned char*)"a",
                            (const unsigned char*)"b", /* attribute value */
                            NULL, NULL); /* errors */
  raptor_xml_element_set_attributes(element, attrs, 1);

  raptor_turtle_writer_empty_element(turtle_writer, element);

  raptor_turtle_writer_cdata(turtle_writer, (const unsigned char*)"\n");

  raptor_free_turtle_writer(turtle_writer);

  raptor_free_xml_element(element);

  raptor_free_namespace(foo_ns);

  raptor_free_namespaces(nstack);

  raptor_free_uri(base_uri);

  
  count=raptor_iostream_get_bytes_written_count(iostr);

#ifdef RAPTOR_DEBUG
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
  string_len=strlen(string);
  if(string_len != count) {
    fprintf(stderr, "%s: I/O stream created a string length %d, expected %d\n", program, (int)string_len, (int)count);
    return 1;
  }

  fprintf(stderr, "%s: Made XML string of %d bytes\n", program, (int)string_len);
  fputs("[[", stderr);
  (void)fwrite(string, 1, string_len, stderr);
  fputs("]]\n", stderr);

  raptor_free_memory(string);
  

  raptor_finish();
#endif
  /* keep gcc -Wall happy */
  return(0);
}

#endif
