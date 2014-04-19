/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_xml_writer.c - Raptor XML Writer for SAX2 events API
 *
 * Copyright (C) 2003-2010, David Beckett http://www.dajobe.org/
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

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"

#ifndef STANDALONE


#define XML_WRITER_AUTO_INDENT(xml_writer) RAPTOR_OPTIONS_GET_NUMERIC(xml_writer, RAPTOR_OPTION_WRITER_AUTO_INDENT)
#define XML_WRITER_AUTO_EMPTY(xml_writer) RAPTOR_OPTIONS_GET_NUMERIC(xml_writer, RAPTOR_OPTION_WRITER_AUTO_EMPTY)
#define XML_WRITER_INDENT(xml_writer) RAPTOR_OPTIONS_GET_NUMERIC(xml_writer, RAPTOR_OPTION_WRITER_INDENT_WIDTH)
#define XML_WRITER_XML_VERSION(xml_writer) RAPTOR_OPTIONS_GET_NUMERIC(xml_writer, RAPTOR_OPTION_WRITER_XML_VERSION)


#define XML_WRITER_FLUSH_CLOSE_BRACKET(xml_writer)              \
  if((XML_WRITER_AUTO_EMPTY(xml_writer)) && \
      xml_writer->current_element &&                            \
      !(xml_writer->current_element->content_cdata_seen ||      \
        xml_writer->current_element->content_element_seen)) {   \
    raptor_iostream_write_byte('>', xml_writer->iostr);         \
}


/* Define this for far too much output */
#undef RAPTOR_DEBUG_CDATA


struct raptor_xml_writer_s {
  raptor_world *world;
  
  int canonicalize;

  int depth;
  
  int my_nstack;
  raptor_namespace_stack *nstack;
  int nstack_depth;

  raptor_xml_element* current_element;

  /* outputting to this iostream */
  raptor_iostream *iostr;

  /* Has writing the XML declaration writing been checked? */
  int xml_declaration_checked;

  /* An extra newline is wanted */
  int pending_newline;

  /* Options (per-object) */
  raptor_object_options options;
};


/* 16 spaces */
#define SPACES_BUFFER_SIZE sizeof(spaces_buffer)
static const unsigned char spaces_buffer[] = {
  ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' '
};



/* helper functions */

/* Handle printing a pending newline OR newline with indenting */
static int
raptor_xml_writer_indent(raptor_xml_writer *xml_writer)
{
  int num_spaces;

  if(!XML_WRITER_AUTO_INDENT(xml_writer)) {
    if(xml_writer->pending_newline) {
      raptor_iostream_write_byte('\n', xml_writer->iostr);
      xml_writer->pending_newline = 0;

      if(xml_writer->current_element)
        xml_writer->current_element->content_cdata_seen = 1;
    }
    return 0;
  }
  
  num_spaces = xml_writer->depth * XML_WRITER_INDENT(xml_writer);

  /* Do not write an extra newline at the start of the document
   * (after the XML declaration or XMP processing instruction has
   * been writtten)
   */
  if(xml_writer->xml_declaration_checked == 1)
    xml_writer->xml_declaration_checked++;
  else {
    raptor_iostream_write_byte('\n', xml_writer->iostr);
    xml_writer->pending_newline = 0;
  }
  
  while(num_spaces > 0) {

    int count = (num_spaces > RAPTOR_GOOD_CAST(int, SPACES_BUFFER_SIZE)) ?
                 RAPTOR_GOOD_CAST(int, SPACES_BUFFER_SIZE) : num_spaces;

    raptor_iostream_counted_string_write(spaces_buffer, count,
                                         xml_writer->iostr);

    num_spaces -= count;
  }

  if(xml_writer->current_element)
    xml_writer->current_element->content_cdata_seen = 1;

  return 0;
}


struct nsd {
  const raptor_namespace *nspace;
  unsigned char *declaration;
  size_t length;
};


static int
raptor_xml_writer_nsd_compare(const void *a, const void *b)
{
  struct nsd* nsd_a = (struct nsd*)a;
  struct nsd* nsd_b = (struct nsd*)b;
  return strcmp((const char*)nsd_a->declaration, (const char*)nsd_b->declaration);
}


static int
raptor_xml_writer_start_element_common(raptor_xml_writer* xml_writer,
                                       raptor_xml_element* element,
                                       int auto_empty)
{
  raptor_iostream* iostr = xml_writer->iostr;
  raptor_namespace_stack *nstack = xml_writer->nstack;
  int depth = xml_writer->depth;
  int auto_indent = XML_WRITER_AUTO_INDENT(xml_writer);
  struct nsd *nspace_declarations = NULL;
  size_t nspace_declarations_count = 0;  
  unsigned int i;

  /* max is 1 per element and 1 for each attribute + size of declared */
  if(nstack) {
    int nspace_max_count = element->attribute_count+1;
    if(element->declared_nspaces)
      nspace_max_count += raptor_sequence_size(element->declared_nspaces);
    if(element->xml_language)
      nspace_max_count++;

    nspace_declarations = RAPTOR_CALLOC(struct nsd*, nspace_max_count,
                                        sizeof(struct nsd));
    if(!nspace_declarations)
      return 1;
  }

  if(element->name->nspace) {
    if(nstack && !raptor_namespaces_namespace_in_scope(nstack, element->name->nspace)) {
      nspace_declarations[0].declaration=
        raptor_namespace_format_as_xml(element->name->nspace,
                                       &nspace_declarations[0].length);
      if(!nspace_declarations[0].declaration)
        goto error;
      nspace_declarations[0].nspace = element->name->nspace;
      nspace_declarations_count++;
    }
  }

  if(nstack && element->attributes) {
    for(i = 0; i < element->attribute_count; i++) {
      /* qname */
      if(element->attributes[i]->nspace) {
        /* Check if we need a namespace declaration attribute */
        if(nstack && 
           !raptor_namespaces_namespace_in_scope(nstack, element->attributes[i]->nspace) && element->attributes[i]->nspace != element->name->nspace) {
          /* not in scope and not same as element (so already going to be declared)*/
          unsigned int j;
          int declare_me = 1;
          
          /* check it wasn't an earlier declaration too */
          for(j = 0; j < nspace_declarations_count; j++)
            if(nspace_declarations[j].nspace == element->attributes[j]->nspace) {
              declare_me = 0;
              break;
            }
            
          if(declare_me) {
            nspace_declarations[nspace_declarations_count].declaration=
              raptor_namespace_format_as_xml(element->attributes[i]->nspace,
                                             &nspace_declarations[nspace_declarations_count].length);
            if(!nspace_declarations[nspace_declarations_count].declaration)
              goto error;
            nspace_declarations[nspace_declarations_count].nspace = element->attributes[i]->nspace;
            nspace_declarations_count++;
          }
        }
      }

      /* Add the attribute + value */
      nspace_declarations[nspace_declarations_count].declaration=
        raptor_qname_format_as_xml(element->attributes[i],
                                   &nspace_declarations[nspace_declarations_count].length);
      if(!nspace_declarations[nspace_declarations_count].declaration)
        goto error;
      nspace_declarations[nspace_declarations_count].nspace = NULL;
      nspace_declarations_count++;

    }
  }

  if(nstack && element->declared_nspaces &&
     raptor_sequence_size(element->declared_nspaces) > 0) {
    for(i = 0; i< (unsigned int)raptor_sequence_size(element->declared_nspaces); i++) {
      raptor_namespace* nspace = (raptor_namespace*)raptor_sequence_get_at(element->declared_nspaces, i);
      unsigned int j;
      int declare_me = 1;
      
      /* check it wasn't an earlier declaration too */
      for(j = 0; j < nspace_declarations_count; j++)
        if(nspace_declarations[j].nspace == nspace) {
          declare_me = 0;
          break;
        }
      
      if(declare_me) {
        nspace_declarations[nspace_declarations_count].declaration=
          raptor_namespace_format_as_xml(nspace,
                                         &nspace_declarations[nspace_declarations_count].length);
        if(!nspace_declarations[nspace_declarations_count].declaration)
          goto error;
        nspace_declarations[nspace_declarations_count].nspace = nspace;
        nspace_declarations_count++;
      }

    }
  }

  if(nstack && element->xml_language) {
    size_t lang_len = strlen(RAPTOR_GOOD_CAST(char*, element->xml_language));
#define XML_LANG_PREFIX_LEN 10
    size_t buf_length = XML_LANG_PREFIX_LEN + lang_len + 1;
    unsigned char* buffer = RAPTOR_MALLOC(unsigned char*, buf_length + 1);
    const char quote = '\"';
    unsigned char* p;

    memcpy(buffer, "xml:lang=\"", XML_LANG_PREFIX_LEN);
    p = buffer + XML_LANG_PREFIX_LEN;
    p += raptor_xml_escape_string(xml_writer->world,
                                  element->xml_language, lang_len,
                                  p, buf_length, quote);
    *p++ = quote;
    *p = '\0';

    nspace_declarations[nspace_declarations_count].declaration = buffer;
    nspace_declarations[nspace_declarations_count].length = buf_length;
    nspace_declarations[nspace_declarations_count].nspace = NULL;
    nspace_declarations_count++;
  }
  

  raptor_iostream_write_byte('<', iostr);

  if(element->name->nspace && element->name->nspace->prefix_length > 0) {
    raptor_iostream_counted_string_write((const char*)element->name->nspace->prefix, 
                                         element->name->nspace->prefix_length,
                                         iostr);
    raptor_iostream_write_byte(':', iostr);
  }
  raptor_iostream_counted_string_write((const char*)element->name->local_name,
                                       element->name->local_name_length,
                                       iostr);

  /* declare namespaces and attributes */
  if(nspace_declarations_count) {
    int need_indent = 0;
    
    /* sort them into the canonical order */
    qsort((void*)nspace_declarations, 
          nspace_declarations_count, sizeof(struct nsd),
          raptor_xml_writer_nsd_compare);

    /* declare namespaces first */
    for(i = 0; i < nspace_declarations_count; i++) {
      if(!nspace_declarations[i].nspace)
        continue;

      if(auto_indent && need_indent) {
        /* indent attributes */
        raptor_xml_writer_newline(xml_writer);
        xml_writer->depth++;
        raptor_xml_writer_indent(xml_writer);
        xml_writer->depth--;
      }
      raptor_iostream_write_byte(' ', iostr);
      raptor_iostream_counted_string_write((const char*)nspace_declarations[i].declaration,
                                           nspace_declarations[i].length,
                                           iostr);
      RAPTOR_FREE(char*, nspace_declarations[i].declaration);
      nspace_declarations[i].declaration = NULL;
      need_indent = 1;
      
      if(raptor_namespace_stack_start_namespace(nstack,
                                                (raptor_namespace*)nspace_declarations[i].nspace,
                                                depth))
        goto error;
    }

    /* declare attributes */
    for(i = 0; i < nspace_declarations_count; i++) {
      if(nspace_declarations[i].nspace)
        continue;

      if(auto_indent && need_indent) {
        /* indent attributes */
        raptor_xml_writer_newline(xml_writer);
        xml_writer->depth++;
        raptor_xml_writer_indent(xml_writer);
        xml_writer->depth--;
      }
      raptor_iostream_write_byte(' ', iostr);
      raptor_iostream_counted_string_write((const char*)nspace_declarations[i].declaration,
                                           nspace_declarations[i].length,
                                           iostr);
      need_indent = 1;

      RAPTOR_FREE(char*, nspace_declarations[i].declaration);
      nspace_declarations[i].declaration = NULL;
    }
  }

  if(!auto_empty)
    raptor_iostream_write_byte('>', iostr);

  if(nstack)
    RAPTOR_FREE(stringarray, nspace_declarations);

  return 0;

  /* Clean up nspace_declarations on error */
  error:

  for(i = 0; i < nspace_declarations_count; i++) {
    if(nspace_declarations[i].declaration)
      RAPTOR_FREE(char*, nspace_declarations[i].declaration);
  }

  RAPTOR_FREE(stringarray, nspace_declarations);

  return 1;
}


static int
raptor_xml_writer_end_element_common(raptor_xml_writer* xml_writer,
                                     raptor_xml_element *element,
                                     int is_empty)
{
  raptor_iostream* iostr = xml_writer->iostr;

  if(is_empty)
    raptor_iostream_write_byte('/', iostr);
  else {
    
    raptor_iostream_write_byte('<', iostr);

    raptor_iostream_write_byte('/', iostr);

    if(element->name->nspace && element->name->nspace->prefix_length > 0) {
      raptor_iostream_counted_string_write((const char*)element->name->nspace->prefix, 
                                           element->name->nspace->prefix_length,
                                           iostr);
      raptor_iostream_write_byte(':', iostr);
    }
    raptor_iostream_counted_string_write((const char*)element->name->local_name,
                                         element->name->local_name_length,
                                         iostr);
  }
  
  raptor_iostream_write_byte('>', iostr);

  return 0;
  
}


/**
 * raptor_new_xml_writer:
 * @world: raptor_world object
 * @nstack: Namespace stack for the writer to start with (or NULL)
 * @iostr: I/O stream to write to
 * 
 * Constructor - Create a new XML Writer writing XML to a raptor_iostream
 * 
 * Return value: a new #raptor_xml_writer object or NULL on failure
 **/
raptor_xml_writer*
raptor_new_xml_writer(raptor_world* world,
                      raptor_namespace_stack *nstack,
                      raptor_iostream* iostr)
{
  raptor_xml_writer* xml_writer;

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  if(!iostr)
    return NULL;
  
  raptor_world_open(world);

  xml_writer = RAPTOR_CALLOC(raptor_xml_writer*, 1, sizeof(*xml_writer));
  if(!xml_writer)
    return NULL;

  xml_writer->world = world;

  xml_writer->nstack_depth = 0;

  xml_writer->nstack = nstack;
  if(!xml_writer->nstack) {
    xml_writer->nstack = raptor_new_namespaces(world, 1);
    xml_writer->my_nstack = 1;
  }

  xml_writer->iostr = iostr;

  raptor_object_options_init(&xml_writer->options,
                             RAPTOR_OPTION_AREA_XML_WRITER);
  
  return xml_writer;
}


/**
 * raptor_free_xml_writer:
 * @xml_writer: XML writer object
 *
 * Destructor - Free XML Writer
 * 
 **/
void
raptor_free_xml_writer(raptor_xml_writer* xml_writer)
{
  if(!xml_writer)
    return;

  if(xml_writer->nstack && xml_writer->my_nstack)
    raptor_free_namespaces(xml_writer->nstack);

  raptor_object_options_clear(&xml_writer->options);
  
  RAPTOR_FREE(raptor_xml_writer, xml_writer);
}


static void
raptor_xml_writer_write_xml_declaration(raptor_xml_writer* xml_writer)
{
  if(!xml_writer->xml_declaration_checked) {
    /* check that it should be written once only */
    xml_writer->xml_declaration_checked = 1;

    if(RAPTOR_OPTIONS_GET_NUMERIC(xml_writer,
                                  RAPTOR_OPTION_WRITER_XML_DECLARATION)) {
      raptor_iostream_string_write((const unsigned char*)"<?xml version=\"",
                                   xml_writer->iostr);
      raptor_iostream_counted_string_write((XML_WRITER_XML_VERSION(xml_writer) == 10) ?
                                           (const unsigned char*)"1.0" :
                                           (const unsigned char*)"1.1",
                                           3, xml_writer->iostr);
      raptor_iostream_string_write((const unsigned char*)"\" encoding=\"utf-8\"?>\n",
                                   xml_writer->iostr);
    }
  }

}


/**
 * raptor_xml_writer_empty_element:
 * @xml_writer: XML writer object
 * @element: XML element object
 *
 * Write an empty XML element to the XML writer.
 * 
 * Closes any previous empty element if XML writer option AUTO_EMPTY
 * is enabled.
 **/
void
raptor_xml_writer_empty_element(raptor_xml_writer* xml_writer,
                                raptor_xml_element *element)
{
  raptor_xml_writer_write_xml_declaration(xml_writer);

  XML_WRITER_FLUSH_CLOSE_BRACKET(xml_writer);
  
  if(xml_writer->pending_newline || XML_WRITER_AUTO_INDENT(xml_writer))
    raptor_xml_writer_indent(xml_writer);
  
  raptor_xml_writer_start_element_common(xml_writer, element, 1);

  raptor_xml_writer_end_element_common(xml_writer, element, 1);
  
  raptor_namespaces_end_for_depth(xml_writer->nstack, xml_writer->depth);
}


/**
 * raptor_xml_writer_start_element:
 * @xml_writer: XML writer object
 * @element: XML element object
 *
 * Write a start XML element to the XML writer.
 *
 * Closes any previous empty element if XML writer option AUTO_EMPTY
 * is enabled.
 *
 * Indents the start element if XML writer option AUTO_INDENT is enabled.
 **/
void
raptor_xml_writer_start_element(raptor_xml_writer* xml_writer,
                                raptor_xml_element *element)
{
  raptor_xml_writer_write_xml_declaration(xml_writer);

  XML_WRITER_FLUSH_CLOSE_BRACKET(xml_writer);
  
  if(xml_writer->pending_newline || XML_WRITER_AUTO_INDENT(xml_writer))
    raptor_xml_writer_indent(xml_writer);
  
  raptor_xml_writer_start_element_common(xml_writer, element,
                                         XML_WRITER_AUTO_EMPTY(xml_writer));

  xml_writer->depth++;

  /* SJS Note: This "if" clause is necessary because raptor_rdfxml.c
   * uses xml_writer for parseType="literal" and passes in elements
   * whose parent field is already set. The first time this function
   * is called, it sets element->parent to 0, causing the warn-07.rdf
   * test to fail. Subsequent calls to this function set
   * element->parent to its existing value. 
   */
  if(xml_writer->current_element)
    element->parent = xml_writer->current_element;
  
  xml_writer->current_element = element;
  if(element->parent)
    element->parent->content_element_seen = 1;
}


/**
 * raptor_xml_writer_end_element:
 * @xml_writer: XML writer object
 * @element: XML element object
 *
 * Write an end XML element to the XML writer.
 *
 * Indents the end element if XML writer option AUTO_INDENT is enabled.
 **/
void
raptor_xml_writer_end_element(raptor_xml_writer* xml_writer,
                              raptor_xml_element* element)
{
  int is_empty;

  xml_writer->depth--;
  
  if(xml_writer->pending_newline ||
     (XML_WRITER_AUTO_INDENT(xml_writer) && element->content_element_seen))
    raptor_xml_writer_indent(xml_writer);

  is_empty = XML_WRITER_AUTO_EMPTY(xml_writer) ?
    !(element->content_cdata_seen || element->content_element_seen) : 0;
  
  raptor_xml_writer_end_element_common(xml_writer, element, is_empty);
  
  raptor_namespaces_end_for_depth(xml_writer->nstack, xml_writer->depth);

  if(xml_writer->current_element)
    xml_writer->current_element = xml_writer->current_element->parent;
}


/**
 * raptor_xml_writer_newline:
 * @xml_writer: XML writer object
 *
 * Write a newline to the XML writer.
 *
 * Indents the next line if XML writer option AUTO_INDENT is enabled.
 **/
void
raptor_xml_writer_newline(raptor_xml_writer* xml_writer)
{
  xml_writer->pending_newline = 1;
}


/**
 * raptor_xml_writer_cdata:
 * @xml_writer: XML writer object
 * @s: string to XML escape and write
 *
 * Write CDATA XML-escaped to the XML writer.
 *
 * Closes any previous empty element if XML writer option AUTO_EMPTY
 * is enabled.
 *
 **/
void
raptor_xml_writer_cdata(raptor_xml_writer* xml_writer,
                        const unsigned char *s)
{
  raptor_xml_writer_write_xml_declaration(xml_writer);

  XML_WRITER_FLUSH_CLOSE_BRACKET(xml_writer);
  
  raptor_xml_escape_string_any_write(s, strlen((const char*)s),
                                      '\0',
                                      XML_WRITER_XML_VERSION(xml_writer),
                                      xml_writer->iostr);

  if(xml_writer->current_element)
    xml_writer->current_element->content_cdata_seen = 1;
}


/**
 * raptor_xml_writer_cdata_counted:
 * @xml_writer: XML writer object
 * @s: string to XML escape and write
 * @len: length of string
 *
 * Write counted CDATA XML-escaped to the XML writer.
 *
 * Closes any previous empty element if XML writer option AUTO_EMPTY
 * is enabled.
 *
 **/
void
raptor_xml_writer_cdata_counted(raptor_xml_writer* xml_writer,
                                const unsigned char *s, unsigned int len)
{
  raptor_xml_writer_write_xml_declaration(xml_writer);

  XML_WRITER_FLUSH_CLOSE_BRACKET(xml_writer);
  
  raptor_xml_escape_string_any_write(s, len,
                                      '\0',
                                      XML_WRITER_XML_VERSION(xml_writer),
                                      xml_writer->iostr);

  if(xml_writer->current_element)
    xml_writer->current_element->content_cdata_seen = 1;
}


/**
 * raptor_xml_writer_raw:
 * @xml_writer: XML writer object
 * @s: string to write
 *
 * Write a string raw to the XML writer.
 *
 * Closes any previous empty element if XML writer option AUTO_EMPTY
 * is enabled.
 *
 **/
void
raptor_xml_writer_raw(raptor_xml_writer* xml_writer,
                      const unsigned char *s)
{
  raptor_xml_writer_write_xml_declaration(xml_writer);

  XML_WRITER_FLUSH_CLOSE_BRACKET(xml_writer);
  
  raptor_iostream_string_write(s, xml_writer->iostr);

  if(xml_writer->current_element)
    xml_writer->current_element->content_cdata_seen = 1;
}


/**
 * raptor_xml_writer_raw_counted:
 * @xml_writer: XML writer object
 * @s: string to write
 * @len: length of string
 *
 * Write a counted string raw to the XML writer.
 *
 * Closes any previous empty element if XML writer option AUTO_EMPTY
 * is enabled.
 *
 **/
void
raptor_xml_writer_raw_counted(raptor_xml_writer* xml_writer,
                              const unsigned char *s, unsigned int len)
{
  raptor_xml_writer_write_xml_declaration(xml_writer);

  XML_WRITER_FLUSH_CLOSE_BRACKET(xml_writer);
  
  raptor_iostream_counted_string_write(s, len, xml_writer->iostr);

  if(xml_writer->current_element)
    xml_writer->current_element->content_cdata_seen = 1;
}


/**
 * raptor_xml_writer_comment:
 * @xml_writer: XML writer object
 * @s: comment string to write
 *
 * Write an XML comment to the XML writer.
 *
 * Closes any previous empty element if XML writer option AUTO_EMPTY
 * is enabled.
 *
 **/
void
raptor_xml_writer_comment(raptor_xml_writer* xml_writer,
                          const unsigned char *s)
{
  XML_WRITER_FLUSH_CLOSE_BRACKET(xml_writer);
  
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"<!-- ", 5);
  raptor_xml_writer_cdata(xml_writer, s);
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)" -->", 4);
}


/**
 * raptor_xml_writer_comment_counted:
 * @xml_writer: XML writer object
 * @s: comment string to write
 * @len: length of string
 *
 * Write a counted XML comment to the XML writer.
 *
 * Closes any previous empty element if XML writer option AUTO_EMPTY
 * is enabled.
 *
 **/
void
raptor_xml_writer_comment_counted(raptor_xml_writer* xml_writer,
                                  const unsigned char *s, unsigned int len)
{
  XML_WRITER_FLUSH_CLOSE_BRACKET(xml_writer);
  
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)"<!-- ", 5);
  raptor_xml_writer_cdata_counted(xml_writer, s, len);
  raptor_xml_writer_raw_counted(xml_writer, (const unsigned char*)" -->", 4);
}


/**
 * raptor_xml_writer_flush:
 * @xml_writer: XML writer object
 *
 * Finish the XML writer.
 *
 **/
void
raptor_xml_writer_flush(raptor_xml_writer* xml_writer)
{
  if(xml_writer->pending_newline) {
    raptor_iostream_write_byte('\n', xml_writer->iostr);
    xml_writer->pending_newline = 0;
  }
}


/**
 * raptor_xml_writer_set_option:
 * @xml_writer: #raptor_xml_writer xml_writer object
 * @option: option to set from enumerated #raptor_option values
 * @string: string option value (or NULL)
 * @integer: integer option value
 *
 * Set xml_writer option.
 * 
 * If @string is not NULL and the option type is numeric, the string
 * value is converted to an integer and used in preference to @integer.
 *
 * If @string is NULL and the option type is not numeric, an error is
 * returned.
 *
 * The @string values used are copied.
 *
 * The allowed options are available via
 * raptor_world_get_option_description().
 *
 * Return value: non 0 on failure or if the option is unknown
 **/
int
raptor_xml_writer_set_option(raptor_xml_writer *xml_writer, 
                             raptor_option option, char* string, int integer)
{
  return raptor_object_options_set_option(&xml_writer->options, option,
                                          string, integer);
}


/**
 * raptor_xml_writer_get_option:
 * @xml_writer: #raptor_xml_writer xml_writer object
 * @option: option to get value
 * @string_p: pointer to where to store string value
 * @integer_p: pointer to where to store integer value
 *
 * Get xml_writer option.
 * 
 * Any string value returned in *@string_p is shared and must
 * be copied by the caller.
 *
 * The allowed options are available via
 * raptor_world_get_option_description().
 *
 * Return value: option value or < 0 for an illegal option
 **/
int
raptor_xml_writer_get_option(raptor_xml_writer *xml_writer,
                             raptor_option option,
                             char** string_p, int* integer_p)
{
  return raptor_object_options_get_option(&xml_writer->options, option,
                                          string_p, integer_p);
}


/**
 * raptor_xml_writer_get_depth:
 * @xml_writer: #raptor_xml_writer xml writer object
 *
 * Get the current XML Writer element depth
 *
 * Return value: element stack depth
 */
int
raptor_xml_writer_get_depth(raptor_xml_writer *xml_writer)
{
  return xml_writer->depth;
}


#endif



#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);


const unsigned char *base_uri_string = (const unsigned char*)"http://example.org/base#";

#define OUT_BYTES_COUNT 135

int
main(int argc, char *argv[]) 
{
  raptor_world *world;
  const char *program = raptor_basename(argv[0]);
  raptor_iostream *iostr;
  raptor_namespace_stack *nstack;
  raptor_namespace* foo_ns;
  raptor_xml_writer* xml_writer;
  raptor_uri* base_uri;
  raptor_qname* el_name;
  raptor_xml_element *element;
  unsigned long offset;
  raptor_qname **attrs;
  raptor_uri* base_uri_copy = NULL;

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

  xml_writer = raptor_new_xml_writer(world, nstack, iostr);
  if(!xml_writer) {
    fprintf(stderr, "%s: Failed to create xml_writer to iostream\n", program);
    exit(1);
  }

  base_uri = raptor_new_uri(world, base_uri_string);

  foo_ns = raptor_new_namespace(nstack,
                              (const unsigned char*)"foo",
                              (const unsigned char*)"http://example.org/foo-ns#",
                              0);


  el_name = raptor_new_qname_from_namespace_local_name(world,
                                                       foo_ns,
                                                       (const unsigned char*)"bar", 
                                                       NULL);
  base_uri_copy = base_uri ? raptor_uri_copy(base_uri) : NULL;
  element = raptor_new_xml_element(el_name,
                                  NULL, /* language */
                                  base_uri_copy);

  raptor_xml_writer_start_element(xml_writer, element);
  raptor_xml_writer_cdata_counted(xml_writer, (const unsigned char*)"hello\n", 6);
  raptor_xml_writer_comment_counted(xml_writer, (const unsigned char*)"comment", 7);
  raptor_xml_writer_cdata(xml_writer, (const unsigned char*)"\n");
  raptor_xml_writer_end_element(xml_writer, element);

  raptor_free_xml_element(element);

  raptor_xml_writer_cdata(xml_writer, (const unsigned char*)"\n");

  el_name = raptor_new_qname(nstack, 
                             (const unsigned char*)"blah", 
                             NULL /* no attribute value - element */);
  base_uri_copy = base_uri ? raptor_uri_copy(base_uri) : NULL;
  element = raptor_new_xml_element(el_name,
                                   NULL, /* language */
                                   base_uri_copy);

  attrs = RAPTOR_CALLOC(raptor_qname**, 1, sizeof(raptor_qname*));
  attrs[0] = raptor_new_qname(nstack, 
                              (const unsigned char*)"a",
                              (const unsigned char*)"b" /* attribute value */);
  raptor_xml_element_set_attributes(element, attrs, 1);

  raptor_xml_writer_empty_element(xml_writer, element);

  raptor_xml_writer_cdata(xml_writer, (const unsigned char*)"\n");

  raptor_free_xml_writer(xml_writer);

  raptor_free_xml_element(element);

  raptor_free_namespace(foo_ns);

  raptor_free_namespaces(nstack);

  raptor_free_uri(base_uri);

  
  offset = raptor_iostream_tell(iostr);

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  fprintf(stderr, "%s: Freeing iostream\n", program);
#endif
  raptor_free_iostream(iostr);

  if(offset != OUT_BYTES_COUNT) {
    fprintf(stderr, "%s: I/O stream wrote %d bytes, expected %d\n", program,
            (int)offset, (int)OUT_BYTES_COUNT);
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
  if(string_len != offset) {
    fprintf(stderr, "%s: I/O stream created a string length %d, expected %d\n", program, (int)string_len, (int)offset);
    return 1;
  }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  fprintf(stderr, "%s: Made XML string of %d bytes\n", program, (int)string_len);
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
