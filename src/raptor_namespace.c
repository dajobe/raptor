/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_namespace.c - Raptor XML namespace classes
 *
 * $Id$
 *
 * Copyright (C) 2002-2004, David Beckett http://purl.org/net/dajobe/
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


/*
 * Namespaces in XML
 * http://www.w3.org/TR/1999/REC-xml-names-19990114/#nsc-NSDeclared
 * (section 4) says:
 *
 * --------------------------------------------------------------------
 *   The prefix xml is by definition bound to the namespace name 
 *   http://www.w3.org/XML/1998/namespace
 * --------------------------------------------------------------------
 *
 * Errata NE05
 * http://www.w3.org/XML/xml-names-19990114-errata#NE05
 * changes that to read:
 *
 * --------------------------------------------------------------------
 * The prefix xml is by definition bound to the namespace name
 * http://www.w3.org/XML/1998/namespace. It may, but need not, be
 * declared, and must not be bound to any other namespace name. No
 * other prefix may be bound to this namespace name.
 *
 * The prefix xmlns is used only to declare namespace bindings and is
 * by definition bound to the namespace name
 * http://www.w3.org/2000/xmlns/. It must not be declared. No other
 * prefix may be bound to this namespace name.
 *
 * All other prefixes beginning with the three-letter sequence x, m, l,
 * in any case combination, are reserved. This means that
 *  * users should not use them except as defined by later specifications
 *  * processors must not treat them as fatal errors.
 * --------------------------------------------------------------------
 *
 * Thus should define it in the table of namespaces before we start.
 *
 * We *can* also define others, but let's not.
 *
 */

const char * const raptor_xml_namespace_uri=(const char *)"http://www.w3.org/XML/1998/namespace";
const char * const raptor_rdf_namespace_uri=(const char *)RAPTOR_RDF_MS_URI;
const unsigned int raptor_rdf_namespace_uri_len=43;
const char * const raptor_rdf_schema_namespace_uri=(const char *)RAPTOR_RDF_SCHEMA_URI;
const char * const raptor_xmlschema_datatypes_namespace_uri=(const char *)RAPTOR_XMLSCHEMA_DATATYPES_URI;
const char * const raptor_owl_namespace_uri=(const char *)RAPTOR_OWL_URI;


/**
 * raptor_namespaces_init: Initialise a raptor namespaces stack
 * @nstack: &raptor_namespace_stack to initialise
 * @uri_handler: URI handler function
 * @uri_context: context for URI handler
 * @error_handler: error handler function
 * @error_data: context for error handler
 * @defaults: namespaces to initialise.  0 for none, 1 for just XML,
 *   2 for RDF, RDFS, OWL and XSD (RDQL uses this), 3+ undefined
 */
void
raptor_namespaces_init(raptor_namespace_stack *nstack,
                       raptor_uri_handler *uri_handler,
                       void *uri_context,
                       raptor_simple_message_handler error_handler,
                       void *error_data,
                       int defaults)
{
  nstack->top=NULL;
  nstack->uri_handler=uri_handler;
  nstack->uri_context=uri_context;

  nstack->error_handler=error_handler;
  nstack->error_data=error_data;

  nstack->rdf_ms_uri    = uri_handler->new_uri(uri_context, raptor_rdf_namespace_uri);
  nstack->rdf_schema_uri= uri_handler->new_uri(uri_context, raptor_rdf_schema_namespace_uri);

  if(defaults) {
    /* defined at level -1 since always 'present' when inside the XML world */
    raptor_namespaces_start_namespace_full(nstack, (const unsigned char*)"xml",
                                           (unsigned char*)raptor_xml_namespace_uri, -1);
    if(defaults >= 2) {
      raptor_namespaces_start_namespace_full(nstack,
                                             (const unsigned char*)"rdf",
                                             (unsigned char*)raptor_rdf_namespace_uri, 0);
      raptor_namespaces_start_namespace_full(nstack,
                                             (const unsigned char*)"rdfs",
                                             (unsigned char*)raptor_rdf_schema_namespace_uri, 0);
      raptor_namespaces_start_namespace_full(nstack,
                                             (const unsigned char*)"xsd",
                                             (unsigned char*)raptor_xmlschema_datatypes_namespace_uri, 0);
      raptor_namespaces_start_namespace_full(nstack,
                                             (const unsigned char*)"owl",
                                             (unsigned char*)raptor_owl_namespace_uri, 0);
    }
  }
}


raptor_namespace_stack *
raptor_new_namespaces(raptor_uri_handler *uri_handler,
                      void *uri_context,
                      raptor_simple_message_handler error_handler,
                      void *error_data,
                      int defaults) 
{
  raptor_namespace_stack *nstack=(raptor_namespace_stack *)RAPTOR_MALLOC(raptor_namespace_stack, sizeof(raptor_namespace_stack));
  if(!nstack)
    return NULL;
                      
  raptor_namespaces_init(nstack, 
                         uri_handler, uri_context,
                         error_handler, error_data,
                         defaults);
  return nstack;
}
 

void
raptor_namespaces_start_namespace(raptor_namespace_stack *nstack, 
                                  raptor_namespace *nspace)
{
  if(nstack->top)
    nspace->next=nstack->top;
  nstack->top=nspace;
}


int
raptor_namespaces_start_namespace_full(raptor_namespace_stack *nstack, 
                                       const unsigned char *prefix, 
                                       const unsigned char *ns_uri_string,
                                       int depth)

{
  raptor_namespace *ns;

  ns=raptor_new_namespace(nstack, prefix, ns_uri_string, depth);
  if(!ns)
    return 1;
  
  raptor_namespaces_start_namespace(nstack, ns);
  return 0;
}


void
raptor_namespaces_clear(raptor_namespace_stack *nstack) {
  raptor_namespace *ns=nstack->top;
  while(ns) {
    raptor_namespace* next_ns=ns->next;

    raptor_free_namespace(ns);
    ns=next_ns;
  }
  nstack->top=NULL;

  if(nstack->uri_handler) {
    nstack->uri_handler->free_uri(nstack->uri_context, nstack->rdf_ms_uri);
    nstack->uri_handler->free_uri(nstack->uri_context, nstack->rdf_schema_uri);
  }

  nstack->uri_handler=(raptor_uri_handler*)NULL;
  nstack->uri_context=NULL;
}


void
raptor_free_namespaces(raptor_namespace_stack *nstack) {
  raptor_namespaces_clear(nstack);
  RAPTOR_FREE(raptor_namespace_stack, nstack);
}


void 
raptor_namespaces_end_for_depth(raptor_namespace_stack *nstack, int depth)
{
  while(nstack->top &&
        nstack->top->depth == depth) {
    raptor_namespace* ns=nstack->top;
    raptor_namespace* next=ns->next;

#ifndef STANDALONE
    RAPTOR_DEBUG3("namespace prefix %s depth %d\n", ns->prefix ? (char*)ns->prefix : "(default)", depth);
#endif
    raptor_free_namespace(ns);

    nstack->top=next;
  }

}


raptor_namespace*
raptor_namespaces_get_default_namespace(raptor_namespace_stack *nstack)
{
  raptor_namespace* ns;
  
  for(ns=nstack->top; ns && ns->prefix; ns=ns->next)
    ;
  return ns;
}


raptor_namespace*
raptor_namespaces_find_namespace(raptor_namespace_stack *nstack, 
                                 const unsigned char *prefix, int prefix_length)
{
  raptor_namespace* ns;
  
  for(ns=nstack->top; ns ; ns=ns->next)
    if(ns->prefix && prefix_length == ns->prefix_length && 
       !strncmp((char*)prefix, (char*)ns->prefix, prefix_length))
      break;
  return ns;
}


int
raptor_namespaces_namespace_in_scope(raptor_namespace_stack *nstack, 
                                     const raptor_namespace *nspace)
{
  raptor_namespace* ns;
  
  for(ns=nstack->top; ns ; ns=ns->next)
    if(nstack->uri_handler->uri_equals(nstack->uri_context, ns->uri, nspace->uri))
      return 1;
  return 0;
}


raptor_namespace*
raptor_new_namespace(raptor_namespace_stack *nstack,
                     const unsigned char *prefix, 
                     const unsigned char *ns_uri_string, int depth)
{
  int prefix_length=0;
  int len;
  raptor_namespace *ns;
  unsigned char *p;

  /* Convert an empty namespace string "" to a NULL pointer */
  if(ns_uri_string && !*ns_uri_string)
    ns_uri_string=NULL;

#ifndef STANDALONE
#if RAPTOR_DEBUG >1
  RAPTOR_DEBUG4("namespace prefix %s uri %s depth %d\n", prefix ? (char*)prefix : "(default)", ns_uri_string ? (char*)ns_uri_string : "(none)", depth);
#endif
#endif

  if(prefix && !ns_uri_string) {
    /* failed to find namespace - now what? */
    if(nstack->error_handler)
      nstack->error_handler((raptor_parser*)nstack->error_data, "The namespace URI for prefix \"%s\" is empty.", prefix);
    return NULL;
  }
  

  len=sizeof(raptor_namespace);
  if(prefix) {
    prefix_length=strlen((char*)prefix);
    len+=prefix_length+1;
  }

  /* Just one malloc for structure + namespace (maybe) + prefix (maybe)*/
  ns=(raptor_namespace*)RAPTOR_CALLOC(raptor_namespace, 1, len);
  if(!ns)
    return NULL;

  p=(unsigned char*)ns+sizeof(raptor_namespace);
  if(ns_uri_string) {
    ns->uri=(*nstack->uri_handler->new_uri)(nstack->uri_context, ns_uri_string);
    if(!ns->uri) {
      RAPTOR_FREE(raptor_namespace, ns);
      return NULL;
    }
  }
  if(prefix) {
    ns->prefix=(const unsigned char*)strcpy((char*)p, (char*)prefix);
    ns->prefix_length=prefix_length;

    if(!strcmp((char*)ns->prefix, "xml"))
      ns->is_xml=1;
  }
  ns->depth=depth;

  /* set convienience flags when there is a defined namespace URI */
  if(ns_uri_string) {
    if(nstack->uri_handler->uri_equals(nstack->uri_context, ns->uri, nstack->rdf_ms_uri))
      ns->is_rdf_ms=1;
    else if(nstack->uri_handler->uri_equals(nstack->uri_context, ns->uri, nstack->rdf_schema_uri))
      ns->is_rdf_schema=1;
  }

  ns->nstack=nstack;

  return ns;
}

/* copy to a new stack with a new depth */
int
raptor_namespace_copy(raptor_namespace_stack *nstack,
                      raptor_namespace *ns,
                      int new_depth)
     
{
  raptor_namespace *new_ns;

  unsigned char *uri_string=(*nstack->uri_handler->uri_as_string)(nstack->uri_context, ns->uri);

  new_ns=raptor_new_namespace(nstack, ns->prefix, uri_string, new_depth);
  if(!new_ns)
    return 1;
  
  raptor_namespaces_start_namespace(nstack, new_ns);
  return 0;
}


void 
raptor_free_namespace(raptor_namespace *ns)
{
  if(ns->uri)
    ns->nstack->uri_handler->free_uri(ns->nstack->uri_context, ns->uri);

  RAPTOR_FREE(raptor_namespace, ns);
}


raptor_uri*
raptor_namespace_get_uri(const raptor_namespace *ns) 
{
  return ns->uri;
}


const unsigned char*
raptor_namespace_get_prefix(const raptor_namespace *ns)
{
  return (const unsigned char*)ns->prefix;
}


unsigned char *
raptor_namespaces_format(const raptor_namespace *ns, size_t *length_p)
{
  size_t uri_length;
  const unsigned char *uri_string=raptor_uri_as_counted_string(ns->uri, &uri_length);
  size_t length=8+uri_length+ns->prefix_length; /* 8=length of [[xmlns=""] */
  unsigned char *buffer;

  if(ns->prefix)
    length++; /* for : */
  
  if(length_p)
    *length_p=length;

  buffer=(unsigned char*)RAPTOR_MALLOC(cstring, length+1);
  if(!buffer)
    return NULL;
  
  if(!uri_length) {
    if(ns->prefix)
      sprintf((char*)buffer, "xmlns:%s=\"\"", ns->prefix);
    else
      strcpy((char*)buffer, "xmlns=\"\"");
  } else {
    if(ns->prefix)
      sprintf((char*)buffer, "xmlns:%s=\"%s\"", ns->prefix, uri_string);
    else
      sprintf((char*)buffer, "xmlns=\"%s\"", uri_string);
  }

  return buffer;
}


#ifdef RAPTOR_DEBUG
void
raptor_namespace_print(FILE *stream, raptor_namespace* ns) 
{
  const unsigned char *uri_string=raptor_uri_as_string(ns->uri);
  if(ns->prefix)
    fprintf(stream, "%s:%s", ns->prefix, uri_string);
  else
    fprintf(stream, "(default):%s", uri_string);
}
#endif



#ifdef STANDALONE


/* one more prototype */
int main(int argc, char *argv[]);


int
main(int argc, char *argv[]) 
{
  raptor_namespace_stack namespaces;
  raptor_uri_handler *handler;
  void *context;

  raptor_uri_init();

  /* Use whatever the raptor_uri class has */
  raptor_uri_get_handler(&handler, &context);

  raptor_namespaces_init(&namespaces, handler, context, NULL, NULL, 1);
  
  raptor_namespaces_start_namespace_full(&namespaces,
                                         (const unsigned char*)"ex1",
                                         (const unsigned char*)"http://example.org/ns1",
                                         0);

  raptor_namespaces_start_namespace_full(&namespaces,
                                         (const unsigned char*)"ex2",
                                         (const unsigned char*)"http://example.org/ns2",
                                         1);

  raptor_namespaces_end_for_depth(&namespaces, 1);

  raptor_namespaces_end_for_depth(&namespaces, 0);

  raptor_namespaces_clear(&namespaces);

  /* keep gcc -Wall happy */
  return(0);
}

#endif

/*
 * Local Variables:
 * mode:c
 * c-basic-offset: 2
 * End:
 */
