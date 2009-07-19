/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_namespace.c - Raptor XML namespace classes
 *
 * Copyright (C) 2002-2009, David Beckett http://www.dajobe.org/
 * Copyright (C) 2002-2005, University of Bristol, UK http://www.bristol.ac.uk/
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


/* Define these for far too much output */
#undef RAPTOR_DEBUG_VERBOSE


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

const unsigned char * const raptor_xml_namespace_uri=(const unsigned char *)"http://www.w3.org/XML/1998/namespace";
const unsigned char * const raptor_rdf_namespace_uri=(const unsigned char *)"http://www.w3.org/1999/02/22-rdf-syntax-ns#";
const unsigned int raptor_rdf_namespace_uri_len=43;
const unsigned char * const raptor_rdf_schema_namespace_uri=(const unsigned char *)"http://www.w3.org/2000/01/rdf-schema#";
const unsigned char * const raptor_xmlschema_datatypes_namespace_uri=(const unsigned char *)"http://www.w3.org/2001/XMLSchema#";
const unsigned char * const raptor_owl_namespace_uri=(const unsigned char *)"http://www.w3.org/2002/07/owl#";


#ifndef RAPTOR_DISABLE_V1
/**
 * raptor_namespaces_init:
 * @nstack: #raptor_namespace_stack to initialise
 * @uri_handler: URI handler function (ignored)
 * @uri_context: context for URI handler (ignored)
 * @error_handler: error handler function
 * @error_data: context for error handler
 * @defaults: namespaces to initialise.
 *
 * Initialise a namespaces stack some optional common namespaces.
 *
 * @defaults can be 0 for none, 1 for just XML, 2 for RDF, RDFS, OWL
 * and XSD (RDQL uses this) or 3+ undefined.
 *
 * @uri_handler and @uri_context parameters are ignored but are retained
 * in the API for backwards compatibility. Internally the same uri handler
 * as returned by raptor_uri_get_handler() will be used.
 *
 * raptor_init() MUST have been called before calling this function.
 * Use raptor_namespaces_init_v2() if using raptor_world APIs.
 *
 * Return value: non-0 on error
 */
int
raptor_namespaces_init(raptor_namespace_stack *nstack,
                       const raptor_uri_handler *uri_handler,
                       void *uri_context,
                       raptor_simple_message_handler error_handler,
                       void *error_data,
                       int defaults)
{
  return raptor_namespaces_init_v2(raptor_world_instance(),
                                   nstack,
                                   error_handler,
                                   error_data,
                                   defaults);
}
#endif


/* hash function to hash namespace prefix strings (usually short strings)
 *
 * Uses DJ Bernstein original hash function - good on short text keys.
 */
static unsigned int
raptor_hash_ns_string(const unsigned char *str, int length)
{
  unsigned int hash = 5381;
  int c;
  
  for(; length && (c = *str++); length--) {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return hash;
}


#define RAPTOR_NAMESPACES_HASHTABLE_SIZE 1024
/**
 * raptor_namespaces_init_v2:
 * @world: raptor_world object
 * @nstack: #raptor_namespace_stack to initialise
 * @error_handler: error handler function
 * @error_data: context for error handler
 * @defaults: namespaces to initialise.
 *
 * Initialise a namespaces stack some optional common namespaces.
 *
 * @defaults can be 0 for none, 1 for just XML, 2 for RDF, RDFS, OWL
 * and XSD (RDQL uses this) or 3+ undefined.
 *
 * Return value: non-0 on error
 */
int
raptor_namespaces_init_v2(raptor_world* world,
                          raptor_namespace_stack *nstack,
                          raptor_simple_message_handler error_handler,
                          void *error_data,
                          int defaults)
{
  int failures=0;

  nstack->world=world;

  nstack->error_handler=error_handler;
  nstack->error_data=error_data;

  nstack->size = 0;
  
  nstack->table_size = RAPTOR_NAMESPACES_HASHTABLE_SIZE;
  nstack->table = (raptor_namespace**)RAPTOR_CALLOC(raptor_namespaces,
                                                    RAPTOR_NAMESPACES_HASHTABLE_SIZE,
                                                    sizeof(raptor_namespace*));
  if(!nstack->table)
    return -1;

  nstack->def_namespace = NULL;

  nstack->rdf_ms_uri = raptor_new_uri_v2(nstack->world,
                                         (const unsigned char*)raptor_rdf_namespace_uri);
  failures += !nstack->rdf_ms_uri;
   
  nstack->rdf_schema_uri = raptor_new_uri_v2(nstack->world,
                                             (const unsigned char*)raptor_rdf_schema_namespace_uri);
  failures += !nstack->rdf_schema_uri;

  /* raptor_new_namespace_from_uri() that eventually gets called by
   * raptor_new_namespace() in raptor_namespaces_start_namespace_full()
   * needs rdf_ms_uri and rdf_schema_uri
   * - do not call if we had failures initializing those uris */
  if(defaults && !failures) {
    /* defined at level -1 since always 'present' when inside the XML world */
    failures += raptor_namespaces_start_namespace_full(nstack,
                                                       (const unsigned char*)"xml",
                                                       raptor_xml_namespace_uri, -1);
    if(defaults >= 2) {
      failures += raptor_namespaces_start_namespace_full(nstack,
                                                         (const unsigned char*)"rdf",
                                                         raptor_rdf_namespace_uri, 0);
      failures += raptor_namespaces_start_namespace_full(nstack,
                                                         (const unsigned char*)"rdfs",
                                                         raptor_rdf_schema_namespace_uri, 0);
      failures += raptor_namespaces_start_namespace_full(nstack,
                                                         (const unsigned char*)"xsd",
                                                         raptor_xmlschema_datatypes_namespace_uri, 0);
      failures += raptor_namespaces_start_namespace_full(nstack,
                                                         (const unsigned char*)"owl",
                                                         raptor_owl_namespace_uri, 0);
    }
  }
  return failures;
}


#ifndef RAPTOR_DISABLE_V1
/**
 * raptor_new_namespaces:
 * @uri_handler: URI handler function (ignored)
 * @uri_context: URI handler context data (ignored)
 * @error_handler: error handler function
 * @error_data: error handler data
 * @defaults: namespaces to initialise
 * 
 * Constructor - create a new #raptor_namespace_stack.
 *
 * See raptor_namespaces_init() for the values of @defaults.
 *
 * @uri_handler and @uri_context parameters are ignored but are retained
 * in the API for backwards compatibility. Internally the same uri handler
 * as returned by raptor_uri_get_handler() will be used.
 *
 * raptor_init() MUST have been called before calling this function.
 * Use raptor_new_namespaces_v2() if using raptor_world APIs.
 * 
 * Return value: a new namespace stack or NULL on failure
 **/
raptor_namespace_stack *
raptor_new_namespaces(const raptor_uri_handler *uri_handler,
                      void *uri_context,
                      raptor_simple_message_handler error_handler,
                      void *error_data,
                      int defaults) 
{
  return raptor_new_namespaces_v2(raptor_world_instance(),
                                  error_handler,
                                  error_data,
                                  defaults);
}
#endif


/**
 * raptor_new_namespaces_v2:
 * @world: raptor_world object
 * @error_handler: error handler function
 * @error_data: error handler data
 * @defaults: namespaces to initialise
 * 
 * Constructor - create a new #raptor_namespace_stack.
 *
 * See raptor_namespaces_init() for the values of @defaults.
 * 
 * Return value: a new namespace stack or NULL on failure
 **/
raptor_namespace_stack *
raptor_new_namespaces_v2(raptor_world* world,
                         raptor_simple_message_handler error_handler,
                         void *error_data,
                         int defaults) 
{
  raptor_namespace_stack *nstack=(raptor_namespace_stack *)RAPTOR_CALLOC(raptor_namespace_stack, 1, sizeof(raptor_namespace_stack));
  if(!nstack)
    return NULL;
                      
  if(raptor_namespaces_init_v2(world,
                               nstack, 
                               error_handler, error_data,
                               defaults)) {
    raptor_free_namespaces(nstack);
    nstack=NULL;
  }

  return nstack;
}
 

/**
 * raptor_namespaces_start_namespace:
 * @nstack: namespace stack
 * @nspace: namespace to start
 * 
 * Start a namespace on a stack of namespaces.
 **/
void
raptor_namespaces_start_namespace(raptor_namespace_stack *nstack, 
                                  raptor_namespace *nspace)
{
  unsigned int hash = raptor_hash_ns_string(nspace->prefix,
                                            nspace->prefix_length);
  const int bucket = hash % nstack->table_size;

  nstack->size++;
  
  if(nstack->table[bucket])
    nspace->next = nstack->table[bucket];
  nstack->table[bucket] = nspace;

  if(!nstack->def_namespace)
    nstack->def_namespace = nspace;

#ifndef STANDALONE
#ifdef RAPTOR_DEBUG_VERBOSE
    RAPTOR_DEBUG3("start namespace prefix %s depth %d\n", nspace->prefix ? (char*)nspace->prefix : "(default)", nspace->depth);
#endif
#endif

}


/**
 * raptor_namespaces_start_namespace_full - 
 * @nstack: namespace stack
 * @prefix: new namespace prefix (or NULL)
 * @ns_uri_string: new namespace URI (or NULL)
 * @depth: new namespace depth
 * 
 * Create a new namespace and start it on a stack of namespaces.
 * 
 * See raptor_new_namespace() for the meanings of @prefix,
 * @ns_uri_string and @depth for namespaces.
 *
 * Return value: non-0 on failure
 **/
int
raptor_namespaces_start_namespace_full(raptor_namespace_stack *nstack, 
                                       const unsigned char *prefix, 
                                       const unsigned char *ns_uri_string,
                                       int depth)
{
  raptor_namespace *ns;

  ns = raptor_new_namespace(nstack, prefix, ns_uri_string, depth);
  if(!ns)
    return 1;
  
  raptor_namespaces_start_namespace(nstack, ns);
  return 0;
}


/**
 * raptor_namespaces_clear:
 * @nstack: namespace stack
 * 
 * Empty a namespace stack of namespaces and any other resources.
 **/
void
raptor_namespaces_clear(raptor_namespace_stack *nstack)
{
  if(nstack->table) {
    int bucket;

    for(bucket = 0; bucket < nstack->table_size; bucket++) {
      raptor_namespace *ns = nstack->table[bucket];
      while(ns) {
        raptor_namespace* next_ns = ns->next;
        
        raptor_free_namespace(ns);
        nstack->size--;
        ns = next_ns;
      }
      nstack->table[bucket] = NULL;
    }

    RAPTOR_FREE(raptor_namespaces, nstack->table);
    nstack->table = NULL;
    nstack->table_size = 0;
  }

  if(nstack->world) {
    if(nstack->rdf_ms_uri) {
      raptor_free_uri_v2(nstack->world, nstack->rdf_ms_uri);
      nstack->rdf_ms_uri = NULL;
    }
    if(nstack->rdf_schema_uri) {
      raptor_free_uri_v2(nstack->world, nstack->rdf_schema_uri);
      nstack->rdf_schema_uri = NULL;
    }
  }

  nstack->size = 0;

  nstack->world = NULL;
}


/**
 * raptor_free_namespaces:
 * @nstack: namespace stack
 * 
 * Destructor - destroy a namespace stack
 **/
void
raptor_free_namespaces(raptor_namespace_stack *nstack)
{
  if(!nstack)
    return;
  
  raptor_namespaces_clear(nstack);

  RAPTOR_FREE(raptor_namespace_stack, nstack);
}


/**
 * raptor_namespaces_end_for_depth:
 * @nstack: namespace stack
 * @depth: depth
 * 
 * End all namespaces at the given depth in the namespace stack.
 **/
void 
raptor_namespaces_end_for_depth(raptor_namespace_stack *nstack, int depth)
{
  int bucket;
  for(bucket = 0; bucket < nstack->table_size; bucket++) {
    while(nstack->table[bucket] &&
          nstack->table[bucket]->depth == depth) {
      raptor_namespace* ns = nstack->table[bucket];
      raptor_namespace* next_ns = ns->next;

#ifndef STANDALONE
#ifdef RAPTOR_DEBUG_VERBOSE
      RAPTOR_DEBUG3("namespace prefix %s depth %d\n",
                    ns->prefix ? (char*)ns->prefix : "(default)", depth);
#endif
#endif
      raptor_free_namespace(ns);
      nstack->size--;

      nstack->table[bucket] = next_ns;
    }
  }
}


/**
 * raptor_namespaces_get_default_namespace:
 * @nstack: namespace stack
 * 
 * Get the current default namespace in-scope in a stack.
 * 
 * Return value: #raptor_namespace or NULL if no default namespace is in scope
 **/
raptor_namespace*
raptor_namespaces_get_default_namespace(raptor_namespace_stack *nstack)
{
  unsigned int hash = raptor_hash_ns_string((const unsigned char *)"", 0);
  const int bucket = hash % nstack->table_size;
  raptor_namespace* ns;

  for(ns = nstack->table[bucket]; ns && ns->prefix; ns = ns->next)
    ;
  return ns;
}


/**
 * raptor_namespaces_find_namespace:
 * @nstack: namespace stack
 * @prefix: namespace prefix to find
 * @prefix_length: length of prefix.
 * 
 * Find a namespace in a namespace stack by prefix.
 *
 * Note that this uses the @length so that the prefix may be a prefix (sic)
 * of a longer string.  If @prefix is NULL, the default namespace will
 * be returned if present, @prefix_length length is ignored in this case.
 * 
 * Return value: #raptor_namespace for the prefix or NULL on failure
 **/
raptor_namespace*
raptor_namespaces_find_namespace(raptor_namespace_stack *nstack, 
                                 const unsigned char *prefix, int prefix_length)
{
  raptor_namespace* ns;
  unsigned int hash = raptor_hash_ns_string(prefix, prefix_length);
  const int bucket = hash % (nstack->table_size);

  for(ns = nstack->table[bucket]; ns ; ns = ns->next) {
    if(!prefix && !ns->prefix)
      break;
    
    if(prefix_length == ns->prefix_length && 
       !strncmp((char*)prefix, (char*)ns->prefix, prefix_length))
      break;
  }

  return ns;
}


/**
 * raptor_namespaces_find_namespace_by_uri:
 * @nstack: namespace stack
 * @ns_uri: namespace URI to find
 * 
 * Find a namespace in a namespace stack by namespace URI.
 * 
 * Return value: #raptor_namespace for the URI or NULL on failure
 **/
raptor_namespace*
raptor_namespaces_find_namespace_by_uri(raptor_namespace_stack *nstack, 
                                        raptor_uri *ns_uri)
{
  int bucket;

  if(!ns_uri)
    return NULL;
  
  for (bucket = 0; bucket < nstack->table_size; bucket++) {
    raptor_namespace* ns;
    for(ns = nstack->table[bucket]; ns ; ns = ns->next)
      if(raptor_uri_equals_v2(nstack->world, ns->uri, ns_uri))
        return ns;
   }
  
  return NULL;
}


/**
 * raptor_namespaces_namespace_in_scope:
 * @nstack: namespace stack
 * @nspace: namespace
 * 
 * Test if a given namespace is in-scope in the namespace stack.
 * 
 * Return value: non-0 if the namespace is in scope.
 **/
int
raptor_namespaces_namespace_in_scope(raptor_namespace_stack *nstack, 
                                     const raptor_namespace *nspace)
{
  raptor_namespace* ns;
  int bucket;
  
  for(bucket = 0; bucket < nstack->table_size; bucket++) {
    for(ns = nstack->table[bucket]; ns ; ns = ns->next)
      if(raptor_uri_equals_v2(nstack->world, ns->uri, nspace->uri))
        return 1;
  }
  return 0;
}


/**
 * raptor_new_namespace_from_uri:
 * @nstack: namespace stack
 * @prefix: namespace prefix string
 * @ns_uri: namespace URI
 * @depth: depth of namespace in the stack
 * 
 * Constructor - create a new namespace from a prefix and URI object.
 * 
 * Return value: a new #raptor_namespace or NULL on failure
 **/
raptor_namespace*
raptor_new_namespace_from_uri(raptor_namespace_stack *nstack,
                              const unsigned char *prefix, 
                              raptor_uri* ns_uri, int depth)
{
  int prefix_length=0;
  int len;
  raptor_namespace *ns;
  unsigned char *p;

#ifndef STANDALONE
#if RAPTOR_DEBUG >1
  RAPTOR_DEBUG4("namespace prefix %s uri %s depth %d\n",
                prefix ? (char*)prefix : "(default)",
                ns_uri ? (char*)raptor_uri_as_string_v2(nstack->world, ns_uri) : "(none)",
                depth);
#endif
#endif

  if(prefix && !ns_uri) {
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
  if(ns_uri) {
    ns->uri = raptor_uri_copy_v2(nstack->world, ns_uri);
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
  if(ns->uri) {
    if(raptor_uri_equals_v2(nstack->world, ns->uri, nstack->rdf_ms_uri))
      ns->is_rdf_ms = 1;
    else if(raptor_uri_equals_v2(nstack->world, ns->uri, nstack->rdf_schema_uri))
      ns->is_rdf_schema=1;
  }

  ns->nstack=nstack;

  return ns;
}


/**
 * raptor_new_namespace:
 * @nstack: namespace stack
 * @prefix: namespace prefix string
 * @ns_uri_string: namespace URI string
 * @depth: depth of namespace in the stack
 * 
 * Constructor - create a new namespace from a prefix and URI string.
 * 
 * Return value: a new #raptor_namespace or NULL on failure
 **/
raptor_namespace*
raptor_new_namespace(raptor_namespace_stack *nstack,
                     const unsigned char *prefix, 
                     const unsigned char *ns_uri_string, int depth)
{
  raptor_uri* ns_uri=NULL;
  raptor_namespace* ns;

  /* Convert an empty namespace string "" to a NULL pointer */
  if(ns_uri_string && !*ns_uri_string)
    ns_uri_string=NULL;

  if(ns_uri_string) {
    ns_uri=raptor_new_uri_v2(nstack->world, ns_uri_string);
    if(!ns_uri)
      return NULL;
  }
  ns=raptor_new_namespace_from_uri(nstack, prefix, ns_uri, depth);
  if(ns_uri)
    raptor_free_uri_v2(nstack->world, ns_uri);

  return ns;
}


/**
 * raptor_namespace_copy:
 * @nstack: namespace stack
 * @ns: namespace
 * @new_depth: new depth
 * 
 * Copy a namespace to a new namespace stack with a new depth.
 * 
 * Return value: non-0 on failure
 **/
int
raptor_namespace_copy(raptor_namespace_stack *nstack,
                      raptor_namespace *ns,
                      int new_depth)
{
  raptor_namespace *new_ns;

  new_ns=raptor_new_namespace_from_uri(nstack, ns->prefix, ns->uri, new_depth);
  if(!new_ns)
    return 1;
  
  raptor_namespaces_start_namespace(nstack, new_ns);
  return 0;
}


/**
 * raptor_free_namespace:
 * @ns: namespace object
 * 
 * Destructor - destroy a namespace.
 **/
void 
raptor_free_namespace(raptor_namespace *ns)
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN(ns, raptor_namespace);

  if(ns->uri)
    raptor_free_uri_v2(ns->nstack->world, ns->uri);

  RAPTOR_FREE(raptor_namespace, ns);
}


/**
 * raptor_namespace_get_uri:
 * @ns: namespace object
 * 
 * Get the namespace URI.
 *
 * Return value: namespace URI or NULL
 **/
raptor_uri*
raptor_namespace_get_uri(const raptor_namespace *ns) 
{
  return ns->uri;
}


/**
 * raptor_namespace_get_prefix:
 * @ns: namespace object
 * 
 * Get the namespace prefix.
 * 
 * Return value: prefix string or NULL
 **/
const unsigned char*
raptor_namespace_get_prefix(const raptor_namespace *ns)
{
  return (const unsigned char*)ns->prefix;
}


/**
 * raptor_namespace_get_counted_prefix:
 * @ns: namespace object
 * @length_p: pointer to store length or NULL
 * 
 * Get the namespace prefix and length.
 * 
 * Return value: prefix string or NULL
 **/
const unsigned char*
raptor_namespace_get_counted_prefix(const raptor_namespace *ns, size_t *length_p)
{
  if(length_p)
    *length_p=ns->prefix_length;
  return (const unsigned char*)ns->prefix;
}


/**
 * raptor_namespaces_format:
 * @ns: namespace object
 * @length_p: pointer to length (or NULL)
 * 
 * Format a namespace in an XML style into a newly allocated string.
 *
 * Generates a string of the form xmlns:prefix="uri",
 * xmlns="uri", xmlns:prefix="" or xmlns="" depending on the
 * namespace's prefix or URI.  Double quotes are always used.
 *
 * If @length_p is not NULL, the length of the string is
 * stored in the address it points to.
 * 
 * See also raptor_new_namespace_parts_from_string()
 *
 * Return value: namespace formatted as newly allocated string or NULL on failure
 **/
unsigned char *
raptor_namespaces_format(const raptor_namespace *ns, size_t *length_p)
{
  size_t uri_length=0L;
  const unsigned char *uri_string=NULL;
  size_t xml_uri_length=0L;
  size_t length;
  unsigned char *buffer;
  const char quote='"';
  unsigned char *p;

  if(ns->uri) {
    uri_string=raptor_uri_as_counted_string_v2(ns->nstack->world, ns->uri, &uri_length);
    xml_uri_length=raptor_xml_escape_string(uri_string, uri_length,
                                            NULL, 0, quote, NULL, NULL);
  }

  length=8+xml_uri_length+ns->prefix_length; /* 8=length of [[xmlns=""] */

  if(ns->prefix)
    length++; /* for : */
  
  if(length_p)
    *length_p=length;

  buffer=(unsigned char*)RAPTOR_MALLOC(cstring, length+1);
  if(!buffer)
    return NULL;
  
  p=buffer;
  
  strncpy((char*)p, "xmlns", 5);
  p+= 5;
  
  if(ns->prefix) {
    *p++ = ':';
    strncpy((char*)p, (char*)ns->prefix, ns->prefix_length);
    p+= ns->prefix_length;
  }
  *p++ = '=';
  *p++ = quote;
  if(uri_length) {
    raptor_xml_escape_string(uri_string, uri_length,
                             p, xml_uri_length, quote,
                             NULL, NULL);
    p+= xml_uri_length;
  }
  *p++ = quote;
  *p++ = '\0';

  return buffer;
}


/**
 * raptor_iostream_write_namespace:
 * @iostr: raptor iosteram
 * @ns: namespace to write
 * 
 * Write a formatted namespace to an iostream
 * 
 * Return value: non-0 on failure
 **/
int
raptor_iostream_write_namespace(raptor_iostream* iostr, raptor_namespace *ns)
{
  size_t uri_length=0L;
  const unsigned char *uri_string=NULL;
  
  if(!ns || !iostr)
    return 1;
  
  if(ns->uri)
    uri_string=raptor_uri_as_counted_string_v2(ns->nstack->world, ns->uri, &uri_length);
  
  raptor_iostream_write_counted_string(iostr, "xmlns", 5);
  if(ns->prefix) {
    raptor_iostream_write_byte(iostr, ':');
    raptor_iostream_write_string(iostr, ns->prefix);
  }
  raptor_iostream_write_counted_string(iostr, "=\"", 2);
  if(uri_length)
    raptor_iostream_write_counted_string(iostr, uri_string, uri_length);
  raptor_iostream_write_byte(iostr, '"');

  return 0;
}


/**
 * raptor_new_namespace_parts_from_string:
 * @string: string to parse
 * @prefix: pointer to location to store namespace prefix
 * @uri_string: pointer to location to store namespace URI
 * 
 * Parse a string containin an XML style namespace declaration
 * into a namespace prefix and URI.
 * 
 * The string is of the form xmlns:prefix="uri",
 * xmlns="uri", xmlns:prefix="" or xmlns="".
 * The quotes can be single or double quotes.
 *
 * Two values are returned from this function into *@prefix and
 * *@uri_string neither of which may be NULL.
 *
 * See also raptor_namespaces_format()
 *
 * Return value: non-0 on failure.
 **/
int
raptor_new_namespace_parts_from_string(const unsigned char *string,
                                       unsigned char **prefix,
                                       unsigned char **uri_string)
{
  const unsigned char *t;
  unsigned char quote;
  
  if((!prefix || !uri_string))
    return 1;
  
  if(!string || (string && !*string))
    return 1;

  if(strncmp((const char*)string, "xmlns", 5))
    return 1;

  *prefix=NULL;
  *uri_string=NULL;

  /*
   * Four cases are expected and handled:
   * xmlns=""
   * xmlns="uri"
   * xmlns:foo=""
   * xmlns:foo="uri"
   *
   * (with " or ' quotes)
   */

  /* skip "xmlns" */
  string+= 5;
  
  if (*string == ':') {
    /* non-empty prefix */
    t= ++string;
    while(*string && *string != '=')
      string++;
    if(!*string || string == t)
      return 1;

    *prefix=(unsigned char*)RAPTOR_MALLOC(cstring, string-t+1);
    if(!*prefix)
      return 1;
    strncpy((char*)*prefix, (const char*)t, string-t);
    (*prefix)[string-t]='\0';
  }

  if(*string++ != '=')
    return 1;

  if(*string != '"' && *string != '\'')
    return 1;
  quote=*string++;

  t=string;
  while(*string && *string != quote)
    string++;

  if(*string != quote)
    return 1;

  if(!(string-t))
    /* xmlns...="" */
    *uri_string=NULL;
  else {
    *uri_string=(unsigned char*)RAPTOR_MALLOC(cstring, string-t+1);
    if(!*uri_string)
      return 1;
    strncpy((char*)*uri_string, (const char*)t, string-t);
    (*uri_string)[string-t]='\0';
  }
  
  return 0;
}


/**
 * raptor_namespaces_qname_from_uri:
 * @nstack: namespace stack
 * @uri: URI to use to make qname
 * @xml_version: XML Version
 * 
 * Make an appropriate XML Qname from the namespaces on a namespace stack
 * 
 * Makes a qname from the in-scope namespaces in a stack if the URI matches
 * the prefix and the rest is a legal XML name.
 *
 * Return value: #raptor_qname for the URI or NULL on failure
 **/
raptor_qname*
raptor_namespaces_qname_from_uri(raptor_namespace_stack *nstack, 
                                 raptor_uri *uri, int xml_version)
{
  unsigned char *uri_string;
  size_t uri_len;
  raptor_namespace* ns = NULL;
  unsigned char *ns_uri_string;
  size_t ns_uri_len;
  unsigned char *name = NULL;
  int bucket;

  if(!uri)
    return NULL;
  
  uri_string = raptor_uri_as_counted_string_v2(nstack->world, uri, &uri_len);

  for(bucket = 0; bucket < nstack->table_size; bucket++) {
    for(ns = nstack->table[bucket]; ns ; ns = ns->next) {
      if(!ns->uri)
        continue;
      
      ns_uri_string = raptor_uri_as_counted_string_v2(nstack->world, ns->uri,
                                                      &ns_uri_len);
      if(ns_uri_len >= uri_len)
        continue;
      if(strncmp((const char*)uri_string, (const char*)ns_uri_string,
                 ns_uri_len))
        continue;
      
      /* uri_string is a prefix of ns_uri_string */
      name = uri_string + ns_uri_len;
      if(!raptor_xml_name_check(name, uri_len-ns_uri_len, xml_version))
        name = NULL;
      
      /* If name is set, we've found a prefix with a legal XML name value */
      if(name)
        break;
    }
    if(name)
      break;
  }
  
  if(!ns)
    return NULL;

  return raptor_new_qname_from_namespace_local_name_v2(nstack->world, ns,
                                                       name,  NULL);
}


#ifdef RAPTOR_DEBUG
void
raptor_namespace_print(FILE *stream, raptor_namespace* ns) 
{
  const unsigned char *uri_string;

  uri_string = raptor_uri_as_string_v2(ns->nstack->world, ns->uri);
  if(ns->prefix)
    fprintf(stream, "%s:%s", ns->prefix, uri_string);
  else
    fprintf(stream, "(default):%s", uri_string);
}
#endif


raptor_namespace**
raptor_namespace_stack_to_array(raptor_namespace_stack *nstack,
                                size_t *size_p)
{
  raptor_namespace** ns_list;
  size_t size = 0;
  int bucket;
  
  ns_list = (raptor_namespace**)RAPTOR_CALLOC(namespace_stack, nstack->size,
                                              sizeof(raptor_namespace*));
  if(!ns_list)
    return NULL;
  
  for(bucket = 0; bucket < nstack->table_size; bucket++) {
    raptor_namespace* ns;

    for(ns = nstack->table[bucket]; ns; ns = ns->next) {
      int skip = 0;
      unsigned int i;
      if(ns->depth < 1)
        continue;
      
      for(i = 0; i < size; i++) {
        raptor_namespace* ns2 = ns_list[i];
        if((!ns->prefix && !ns2->prefix) ||
           (ns->prefix && ns2->prefix && 
            !strcmp((const char*)ns->prefix, (const char*)ns2->prefix))) {
          /* this prefix was seen (overridden) earlier so skip */
          skip = 1;
          break;
        }
      }
      if(!skip)
        ns_list[size++] = ns;
    }
  }
  
  if(size_p)
    *size_p = size;

  return ns_list;
}


#ifdef STANDALONE


/* one more prototype */
int main(int argc, char *argv[]);


int
main(int argc, char *argv[]) 
{
  raptor_world *world;
  const char *program=raptor_basename(argv[0]);
  raptor_namespace_stack namespaces;
  raptor_namespace* ns;

  world = raptor_new_world();
  if(!world || raptor_world_open(world))
    exit(1);

  raptor_namespaces_init_v2(world, &namespaces, NULL, NULL, 1);
  
  raptor_namespaces_start_namespace_full(&namespaces,
                                         (const unsigned char*)"ex1",
                                         (const unsigned char*)"http://example.org/ns1",
                                         0);

  raptor_namespaces_start_namespace_full(&namespaces,
                                         (const unsigned char*)"ex2",
                                         (const unsigned char*)"http://example.org/ns2",
                                         1);

  if(raptor_namespaces_find_namespace(&namespaces, NULL, 0)) {
      fprintf(stderr, "%s: Default namespace found when should not be found, returning error\n", 
              program);
    return(1);
  }

  raptor_namespaces_start_namespace_full(&namespaces,
                                         NULL,
                                         (const unsigned char*)"http://example.org/ns3",
                                         2);

  ns=raptor_namespaces_find_namespace(&namespaces, NULL, 0);
  if(!ns) {
    fprintf(stderr, "%s: Default namespace not found when should not be found, returning error\n", 
            program);
    return(1);
  }

  ns=raptor_namespaces_find_namespace(&namespaces, (const unsigned char*)"ex2", 3);
  if(!ns) {
    fprintf(stderr, "%s: namespace ex2 not found when should not be found, returning error\n", 
            program);
    return(1);
  }

  raptor_namespaces_end_for_depth(&namespaces, 2);

  raptor_namespaces_end_for_depth(&namespaces, 1);

  raptor_namespaces_end_for_depth(&namespaces, 0);

  raptor_namespaces_clear(&namespaces);

  raptor_free_world(world);

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
