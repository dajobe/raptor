/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_sax2.c - Raptor SAX2 API
 *
 * $Id$
 *
 * Copyright (C) 2000-2004, David Beckett http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology http://www.ilrt.bristol.ac.uk/
 * University of Bristol, UK http://www.bristol.ac.uk/
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

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


/* Define this for far too much output */
#undef RAPTOR_DEBUG_CDATA


raptor_sax2*
raptor_new_sax2(void *user_data) {
  raptor_sax2* sax2;
  sax2=(raptor_sax2*)RAPTOR_CALLOC(raptor_sax2, 1, sizeof(raptor_sax2));
  if(!sax2)
    return NULL;
  
  sax2->user_data=user_data;
  return sax2;
}


void
raptor_free_sax2(raptor_sax2 *sax2) {
  raptor_xml_element *xml_element;

#ifdef RAPTOR_XML_EXPAT
  if(sax2->xp) {
    XML_ParserFree(sax2->xp);
    sax2->xp=NULL;
  }
#endif

#ifdef RAPTOR_XML_LIBXML
  if(sax2->xc) {
    raptor_libxml_free(sax2->xc);
    sax2->xc=NULL;
  }
#endif

  while( (xml_element=raptor_xml_element_pop(sax2)) )
    raptor_free_xml_element(xml_element);

  RAPTOR_FREE(raptor_sax2, sax2);
}



raptor_xml_element*
raptor_xml_element_pop(raptor_sax2 *sax2) 
{
  raptor_xml_element *element=sax2->current_element;

  if(!element)
    return NULL;

  sax2->current_element=element->parent;
  if(sax2->root_element == element) /* just deleted root */
    sax2->root_element=NULL;

  return element;
}


void
raptor_xml_element_push(raptor_sax2 *sax2, raptor_xml_element* element) 
{
  element->parent=sax2->current_element;
  sax2->current_element=element;
  if(!sax2->root_element)
    sax2->root_element=element;
}


const unsigned char*
raptor_sax2_inscope_xml_language(raptor_sax2 *sax2) {
  raptor_xml_element* xml_element;
  
  for(xml_element=sax2->current_element;
      xml_element; 
      xml_element=xml_element->parent)
    if(xml_element->xml_language)
      return xml_element->xml_language;
    
  return NULL;
}


raptor_uri*
raptor_sax2_inscope_base_uri(raptor_sax2 *sax2) {
  raptor_xml_element *xml_element;
  
  for(xml_element=sax2->current_element; 
      xml_element; 
      xml_element=xml_element->parent)
    if(xml_element->base_uri)
      return xml_element->base_uri;
    
  return NULL;
}


int
raptor_sax2_get_depth(raptor_sax2 *sax2) {
  return sax2->depth;
}

void
raptor_sax2_inc_depth(raptor_sax2 *sax2) {
  sax2->depth++;
}

void
raptor_sax2_dec_depth(raptor_sax2 *sax2) {
  sax2->depth--;
}


void
raptor_sax2_parse_start(raptor_sax2* sax2, raptor_uri *base_uri) {
#ifdef RAPTOR_XML_EXPAT
  XML_Parser xp;
#endif

  sax2->depth=0;
  sax2->root_element=NULL;
  sax2->current_element=NULL;

#ifdef RAPTOR_XML_EXPAT
  if(sax2->xp) {
    XML_ParserFree(sax2->xp);
    sax2->xp=NULL;
  }

  xp=sax2->xp=raptor_expat_init(sax2->user_data);
  XML_SetBase(xp, raptor_uri_as_string(base_uri));
#endif

#ifdef RAPTOR_XML_LIBXML
  raptor_libxml_init(&sax2->sax);

#if LIBXML_VERSION < 20425
  sax2->first_read=1;
#endif

  if(sax2->xc) {
    raptor_libxml_free(sax2->xc);
    sax2->xc=NULL;
  }
#endif
}


int
raptor_sax2_parse_chunk(raptor_sax2* sax2, const unsigned char *buffer,
                        size_t len, int is_end) 
{
  raptor_parser* rdf_parser=(raptor_parser*)sax2->user_data;
#ifdef RAPTOR_XML_EXPAT
  raptor_locator *locator=&rdf_parser->locator;
  XML_Parser xp=sax2->xp;
#endif
#ifdef RAPTOR_XML_LIBXML
  /* parser context */
  xmlParserCtxtPtr xc=sax2->xc;
#endif
  int rc;
  
#ifdef RAPTOR_XML_LIBXML
  if(!xc) {
    if(!len) {
      /* no data given at all - emit a similar message to expat */
      raptor_update_document_locator(rdf_parser);
      raptor_parser_error(rdf_parser, "XML Parsing failed - no element found");
      return 1;
    }

    xc = xmlCreatePushParserCtxt(&sax2->sax, sax2->user_data,
                                 (char*)buffer, len, 
                                 NULL);
    if(!xc)
      goto handle_error;
    
    xc->userData = sax2->user_data;
    xc->vctxt.userData = sax2->user_data;
    xc->vctxt.error=raptor_libxml_validation_error;
    xc->vctxt.warning=raptor_libxml_validation_warning;
    xc->replaceEntities = 1;
    
    sax2->xc = xc;

    if(is_end)
      len=0;
    else
      return 0;
  }
#endif

  if(!len) {
#ifdef RAPTOR_XML_EXPAT
    rc=XML_Parse(xp, (char*)buffer, 0, 1);
    if(!rc) /* expat: 0 is failure */
      goto handle_error;
#endif
#ifdef RAPTOR_XML_LIBXML
    xmlParseChunk(xc, (char*)buffer, 0, 1);
#endif
    return 0;
  }


#ifdef RAPTOR_XML_EXPAT
  rc=XML_Parse(xp, (char*)buffer, len, is_end);
  if(!rc) /* expat: 0 is failure */
    goto handle_error;
  if(is_end)
    return 0;
#endif

#ifdef RAPTOR_XML_LIBXML

  /* This works around some libxml versions that fail to work
   * if the buffer size is larger than the entire file
   * and thus the entire parsing is done in one operation.
   *
   * The code below:
   *   2.4.19 (oldest tested) to 2.4.24 - required
   *   2.4.25                           - works with or without it
   *   2.4.26 or later                  - fails with this code
   */

#if LIBXML_VERSION < 20425
  if(sax2->first_read && is_end) {
    /* parse all but the last character */
    rc=xmlParseChunk(xc, (char*)buffer, len-1, 0);
    if(rc)
      goto handle_error;
    /* last character */
    rc=xmlParseChunk(xc, (char*)buffer + (len-1), 1, 0);
    if(rc)
      goto handle_error;
    /* end */
    xmlParseChunk(xc, (char*)buffer, 0, 1);
    return 0;
  }
#endif

#if LIBXML_VERSION < 20425
  sax2->first_read=0;
#endif
    
  rc=xmlParseChunk(xc, (char*)buffer, len, is_end);
  if(rc) /* libxml: non 0 is failure */
    goto handle_error;
  if(is_end)
    return 0;
#endif

  return 0;

  handle_error:

#ifdef RAPTOR_XML_EXPAT
#ifdef EXPAT_UTF8_BOM_CRASH
  if(sax2->tokens_count) {
#endif
    /* Work around a bug with the expat 1.95.1 shipped with RedHat 7.2
     * which dies here if the error is before <?xml?...
     * The expat 1.95.1 source release version works fine.
     */
    locator->line=XML_GetCurrentLineNumber(xp);
    locator->column=XML_GetCurrentColumnNumber(xp);
    locator->byte=XML_GetCurrentByteIndex(xp);
#ifdef EXPAT_UTF8_BOM_CRASH
  }
#endif
#endif /* EXPAT */
      
  raptor_update_document_locator(rdf_parser);

#if RAPTOR_XML_EXPAT
  raptor_parser_error(rdf_parser, "XML Parsing failed - %s",
                      XML_ErrorString(XML_GetErrorCode(xp)));
#endif /* EXPAT */

#ifdef RAPTOR_XML_LIBXML
  raptor_parser_error(rdf_parser, "XML Parsing failed");
#endif

  return 1;
}


raptor_xml_element*
raptor_new_xml_element(raptor_qname *name,
                        const unsigned char *xml_language, 
                        raptor_uri *xml_base) {
  raptor_xml_element* xml_element;

  xml_element=(raptor_xml_element*)RAPTOR_CALLOC(raptor_xml_element, 1,
                                                   sizeof(raptor_xml_element));
  if(!xml_element)
    return NULL;

  /* Element name */
  xml_element->name=name;
  xml_element->xml_language=xml_language;
  xml_element->base_uri=xml_base;

  xml_element->declared_nspaces=NULL;

  return xml_element;
}


void
raptor_free_xml_element(raptor_xml_element *element)
{
  unsigned int i;

  for (i=0; i < element->attribute_count; i++)
    if(element->attributes[i])
      raptor_free_qname(element->attributes[i]);

  if(element->attributes)
    RAPTOR_FREE(raptor_qname_array, element->attributes);

  if(element->content_cdata_length)
    RAPTOR_FREE(raptor_qname_array, element->content_cdata);

  if(element->base_uri)
    raptor_free_uri(element->base_uri);

  if(element->xml_language)
    RAPTOR_FREE(cstring, element->xml_language);

  raptor_free_qname(element->name);

  if(element->declared_nspaces)
    raptor_free_sequence(element->declared_nspaces);

  RAPTOR_FREE(raptor_element, element);
}


raptor_qname*
raptor_xml_element_get_element(raptor_xml_element *xml_element) {
  return xml_element->name;
}


void
raptor_xml_element_set_attributes(raptor_xml_element* xml_element,
                                   raptor_qname **attributes, int count)
{
  xml_element->attributes=attributes;
  xml_element->attribute_count=count;
}


void
raptor_sax2_declare_namespace(raptor_xml_element* xml_element,
                              raptor_namespace *nspace) {
  if(!xml_element->declared_nspaces)
    xml_element->declared_nspaces=raptor_new_sequence(NULL, NULL);
  raptor_sequence_push(xml_element->declared_nspaces, nspace);
}


#ifdef RAPTOR_DEBUG
void
raptor_print_xml_element(raptor_xml_element *element, FILE* stream)
{
  raptor_qname_print(stream, element->name);
  fputc('\n', stream);

  if(element->attribute_count) {
    unsigned int i;
    int printed=0;

    fputs(" attributes: ", stream);
    for (i = 0; i < element->attribute_count; i++) {
      if(element->attributes[i]) {
        if(printed)
          fputc(' ', stream);
        raptor_qname_print(stream, element->attributes[i]);
        fprintf(stream, "='%s'", element->attributes[i]->value);
        printed=1;
      }
    }
    fputc('\n', stream);
  }
}
#endif


struct nsd
{
  const raptor_namespace *nspace;
  unsigned char *declaration;
  size_t length;
};


static int
raptor_nsd_compare(const void *a, const void *b) 
{
  struct nsd* nsd_a=(struct nsd*)a;
  struct nsd* nsd_b=(struct nsd*)b;
  return strcmp((const char*)nsd_a->declaration, (const char*)nsd_b->declaration);
}


int
raptor_iostream_write_xml_element(raptor_iostream* iostr,
                                  raptor_xml_element *element,
                                  raptor_namespace_stack *nstack,
                                  int is_empty,
                                  int is_end,
                                  raptor_simple_message_handler error_handler,
                                  void *error_data,
                                  int depth)
{
  struct nsd *nspace_declarations=NULL;
  size_t nspace_declarations_count=0;  
  unsigned int i;

  /* max is 1 per element and 1 for each attribute + size of declared */
  if(nstack) {
    int nspace_max_count=element->attribute_count+1;
    if(element->declared_nspaces)
      nspace_max_count += raptor_sequence_size(element->declared_nspaces);
    
    nspace_declarations=(struct nsd*)RAPTOR_CALLOC(nsdarray, nspace_max_count, sizeof(struct nsd));
  }

  if(element->name->nspace) {
    if(!is_end && nstack &&
       !raptor_namespaces_namespace_in_scope(nstack, element->name->nspace)) {
      nspace_declarations[0].declaration=
        raptor_namespaces_format(element->name->nspace,
                                 &nspace_declarations[0].length);
      nspace_declarations[0].nspace=element->name->nspace;
      nspace_declarations_count++;
    }
  }

  if (!is_end && element->attributes) {
    for(i=0; i < element->attribute_count; i++) {
      /* qname */
      if(element->attributes[i]->nspace) {
        if(nstack && 
           !raptor_namespaces_namespace_in_scope(nstack, element->attributes[i]->nspace) && element->attributes[i]->nspace != element->name->nspace) {
          /* not in scope and not same as element (so already going to be declared)*/
          unsigned int j;
          int declare_me=1;
          
          /* check it wasn't an earlier declaration too */
          for (j=0; j < nspace_declarations_count; j++)
            if(nspace_declarations[j].nspace == element->attributes[j]->nspace) {
              declare_me=0;
              break;
            }
            
          if(declare_me) {
            nspace_declarations[nspace_declarations_count].declaration=
              raptor_namespaces_format(element->attributes[i]->nspace,
                                       &nspace_declarations[nspace_declarations_count].length);
            nspace_declarations[nspace_declarations_count].nspace=element->attributes[i]->nspace;
            nspace_declarations_count++;
          }
        }

      }
    }
  }
  

  if(!is_end && nstack && element->declared_nspaces &&
     raptor_sequence_size(element->declared_nspaces) > 0) {
    for(i=0; i< (unsigned int)raptor_sequence_size(element->declared_nspaces); i++) {
      raptor_namespace* nspace=(raptor_namespace*)raptor_sequence_get_at(element->declared_nspaces, i);
      unsigned int j;
      int declare_me=1;
      
      /* check it wasn't an earlier declaration too */
      for (j=0; j < nspace_declarations_count; j++)
        if(nspace_declarations[j].nspace == nspace) {
          declare_me=0;
          break;
        }
      
      if(declare_me) {
        nspace_declarations[nspace_declarations_count].declaration=
          raptor_namespaces_format(nspace,
                                   &nspace_declarations[nspace_declarations_count].length);
        nspace_declarations[nspace_declarations_count].nspace=nspace;
        nspace_declarations_count++;
      }

    }
  }



  raptor_iostream_write_byte(iostr, '<');
  if(is_end)
    raptor_iostream_write_byte(iostr, '/');

  if(element->name->nspace && element->name->nspace->prefix_length > 0) {
    raptor_iostream_write_counted_string(iostr, 
                                         (const char*)element->name->nspace->prefix, 
                                         element->name->nspace->prefix_length);
    raptor_iostream_write_byte(iostr, ':');
  }
  raptor_iostream_write_counted_string(iostr, 
                                       (const char*)element->name->local_name,
                                       element->name->local_name_length);

  /* declare namespaces */
  if(nspace_declarations_count) {
    /* sort them into the canonical order */
    qsort((void*)nspace_declarations, 
          nspace_declarations_count, sizeof(struct nsd),
          raptor_nsd_compare);
    /* add them */
    for (i=0; i < nspace_declarations_count; i++) {
      raptor_iostream_write_byte(iostr, ' ');
      raptor_iostream_write_counted_string(iostr, 
                                           (const char*)nspace_declarations[i].declaration,
                                           nspace_declarations[i].length);
      RAPTOR_FREE(cstring, nspace_declarations[i].declaration);
      nspace_declarations[i].declaration=NULL;

      raptor_namespace_copy(nstack,
                            (raptor_namespace*)nspace_declarations[i].nspace,
                            depth);
    }
  }


  if(!is_end && element->attributes) {
    for(i=0; i < element->attribute_count; i++) {
      raptor_iostream_write_byte(iostr, ' ');
      
      if(element->attributes[i]->nspace && 
         element->attributes[i]->nspace->prefix_length > 0) {
        raptor_iostream_write_counted_string(iostr,
                                             (char*)element->attributes[i]->nspace->prefix,
                                             element->attributes[i]->nspace->prefix_length);
        raptor_iostream_write_byte(iostr, ':');
      }

      raptor_iostream_write_counted_string(iostr, 
                                           (const char*)element->attributes[i]->local_name,
                                           element->attributes[i]->local_name_length);
      
      raptor_iostream_write_counted_string(iostr, "=\"", 2);
      
      raptor_iostream_write_xml_escaped_string(iostr,
                                               element->attributes[i]->value, 
                                               element->attributes[i]->value_length,
                                               '"',
                                               error_handler, error_data);
      raptor_iostream_write_byte(iostr, '"');
    }
  }
  
  if(is_empty)
    raptor_iostream_write_byte(iostr, '/');

  raptor_iostream_write_byte(iostr, '>');

  if(nstack)
    RAPTOR_FREE(stringarray, nspace_declarations);

  return 0;
}
