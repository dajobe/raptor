/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_namespace.c - Raptor XML namespace classes
 *
 * $Id$
 *
 * Copyright (C) 2002 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology - http://www.ilrt.org/
 * University of Bristol - http://www.bristol.ac.uk/
 * 
 * This package is Free Software or Open Source available under the
 * following licenses (these are alternatives):
 *   1. GNU Lesser General Public License (LGPL)
 *   2. GNU General Public License (GPL)
 *   3. Mozilla Public License (MPL)
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * full license terms.
 * 
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef WIN32
#include <win32_config.h>
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

/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
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

static const char * const raptor_xml_uri="http://www.w3.org/XML/1998/namespace";
#ifndef RAPTOR_IN_REDLAND
static const char * const raptor_rdf_ms_uri=RAPTOR_RDF_MS_URI;
static const char * const raptor_rdf_schema_uri=RAPTOR_RDF_SCHEMA_URI;
#endif

void
raptor_namespaces_init(raptor_namespace_stack *nstack
#ifdef RAPTOR_IN_REDLAND
                       ,librdf_world *world
#endif
) {
  nstack->top=NULL;
#ifdef RAPTOR_IN_REDLAND
  nstack->world=world;
#endif
  /* defined at level -1 since always 'present' when inside the XML world */
  raptor_namespaces_start_namespace(nstack, "xml", raptor_xml_uri, -1);
}


int
raptor_namespaces_start_namespace(raptor_namespace_stack *nstack, 
                                  const char *prefix, 
                                  const char *ns_uri_string, int depth)
{
  raptor_namespace *ns;

#ifdef RAPTOR_IN_REDLAND
  ns=raptor_namespace_new(prefix, ns_uri_string, depth, nstack->world);
#else
  ns=raptor_namespace_new(prefix, ns_uri_string, depth);
#endif
  if(!ns)
    return 1;
  
  if(nstack->top)
    ns->next=nstack->top;
  nstack->top=ns;

  return 0;
}


void
raptor_namespaces_free(raptor_namespace_stack *nstack) {
  raptor_namespace *ns=nstack->top;
  while(ns) {
    raptor_namespace* next_ns=ns->next;

    raptor_namespace_free(ns);
    ns=next_ns;
  }
}


void 
raptor_namespaces_end_for_depth(raptor_namespace_stack *nstack, int depth)
{
  while(nstack->top &&
        nstack->top->depth == depth) {
    raptor_namespace* ns=nstack->top;
    raptor_namespace* next=ns->next;

    LIBRDF_DEBUG3(raptor_namespaces_end_for_depth,
                  "namespace prefix %s depth %d\n",
                  ns->prefix ? ns->prefix : "(default)", depth);

    raptor_namespace_free(ns);

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
                                 const char *prefix, int prefix_length)
{
  raptor_namespace* ns;
  
  for(ns=nstack->top; ns ; ns=ns->next)
    if(ns->prefix && prefix_length == ns->prefix_length && 
       !strncmp(prefix, ns->prefix, prefix_length))
      break;
  return ns;
}


raptor_namespace*
raptor_namespace_new(const char *prefix, 
                     const char *ns_uri_string, int depth
#ifdef RAPTOR_IN_REDLAND
                     ,librdf_world *world
#endif
)
{
  int prefix_length=0;
#ifdef RAPTOR_IN_REDLAND
#else
  int uri_length=0;
#endif
  int len;
  raptor_namespace *ns;
  char *p;

  LIBRDF_DEBUG4(raptor_namespaces_start_namespace,
                "namespace prefix %s uri %s depth %d\n",
                prefix ? prefix : "(default)", ns_uri_string, depth);

  /* Convert an empty namespace string "" to a NULL pointer */
  if(!*ns_uri_string)
    ns_uri_string=NULL;

  len=sizeof(raptor_namespace);
#ifdef RAPTOR_IN_REDLAND
#else
  if(ns_uri_string) {
    uri_length=strlen(ns_uri_string);
    len+=uri_length+1;
  }
#endif
  if(prefix) {
    prefix_length=strlen(prefix);
    len+=prefix_length+1;
  }

  /* Just one malloc for structure + namespace (maybe) + prefix (maybe)*/
  ns=(raptor_namespace*)LIBRDF_CALLOC(raptor_namespace, len, 1);
  if(!ns)
    return NULL;

  p=(char*)ns+sizeof(raptor_namespace);
#ifdef RAPTOR_IN_REDLAND
  ns->uri=librdf_new_uri(world, ns_uri_string);
  if(!ns->uri) {
    LIBRDF_FREE(raptor_namespace, ns);
    return NULL;
  }
#else
  if(ns_uri_string) {
    ns->uri=strcpy((char*)p, ns_uri_string);
    ns->uri_length=uri_length;
    p+= uri_length+1;
  }
#endif
  if(prefix) {
    ns->prefix=strcpy((char*)p, prefix);
    ns->prefix_length=prefix_length;

    if(!strcmp(ns->prefix, "xml"))
      ns->is_xml=1;
  }
  ns->depth=depth;

  /* set convienience flags when there is a defined namespace URI */
  if(ns_uri_string) {
#ifdef RAPTOR_IN_REDLAND
    if(librdf_uri_equals(ns->uri, librdf_concept_ms_namespace_uri))
      ns->is_rdf_ms=1;
    else if(librdf_uri_equals(ns->uri, librdf_concept_schema_namespace_uri))
      ns->is_rdf_schema=1;
#else
    if(!strcmp(ns_uri_string, raptor_rdf_ms_uri))
      ns->is_rdf_ms=1;
    else if(!strcmp(ns_uri_string, raptor_rdf_schema_uri))
      ns->is_rdf_schema=1;
#endif
  }

  return ns;
}


void 
raptor_namespace_free(raptor_namespace *ns)
{
#ifdef RAPTOR_IN_REDLAND
  if(ns->uri)
    librdf_free_uri(ns->uri);
#endif
  LIBRDF_FREE(raptor_namespace, ns);
}


raptor_uri*
raptor_namespace_get_uri(const raptor_namespace *ns) 
{
  return ns->uri;
}


const char*
raptor_namespace_get_prefix(const raptor_namespace *ns)
{
  return (const char*)ns->prefix;
}


raptor_uri*
raptor_namespace_local_name_to_uri(const raptor_namespace *ns,
                                   const char *local_name)
{
#ifdef RAPTOR_IN_REDLAND
  return librdf_new_uri_from_uri_local_name(ns->uri, local_name);
#else
  char *uri=(char*)LIBRDF_MALLOC(cstring, 
                                 ns->uri_length + strlen(local_name) + 1);
  if(!uri)
    return NULL;
  
  strcpy(uri, ns->uri);
  strcpy(uri + ns->uri_length, local_name);
  
  return uri;
#endif
}


#ifdef RAPTOR_DEBUG
void
raptor_namespace_print(FILE *stream, raptor_namespace* ns) 
{
  const char *uri_string=RAPTOR_URI_AS_STRING(ns->uri);
  if(ns->prefix)
    fprintf(stream, "%s:%s", ns->prefix, uri_string);
  else
    fprintf(stream, "(default):%s", uri_string);
}
#endif



#ifdef STANDALONE


/* one more prototype */
int main(int argc, char *argv[]);


#ifdef RAPTOR_IN_REDLAND
#include <librdf.h>
#endif

int
main(int argc, char *argv[]) 
{
  raptor_namespace_stack namespaces;

#ifdef RAPTOR_IN_REDLAND
  librdf_world *world=librdf_new_world();
  librdf_world_open(world);
#endif
  
  raptor_namespaces_init(&namespaces
#ifdef RAPTOR_IN_REDLAND
                         ,world
#endif
  );
  
  raptor_namespaces_start_namespace(&namespaces,
                                    "ex1",
                                    "http://example.org/ns1",
                                    0);

  raptor_namespaces_start_namespace(&namespaces,
                                    "ex2",
                                    "http://example.org/ns2",
                                    1);

  raptor_namespaces_end_for_depth(&namespaces, 1);

  raptor_namespaces_end_for_depth(&namespaces, 0);

  raptor_namespaces_free(&namespaces);

#ifdef RAPTOR_IN_REDLAND
  librdf_free_world(world);
#endif
  
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
