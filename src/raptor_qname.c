/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_qname.c - Raptor XML qname class
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
 * http://www.w3.org/TR/1999/REC-xml-names-19990114/#defaulting
 * says:
 *
 * --------------------------------------------------------------------
 *  5.2 Namespace Defaulting
 *
 *  A default namespace is considered to apply to the element where it
 *  is declared (if that element has no namespace prefix), and to all
 *  elements with no prefix within the content of that element. 
 *
 *  If the URI reference in a default namespace declaration is empty,
 *  then unprefixed elements in the scope of the declaration are not
 *  considered to be in any namespace.
 *
 *  Note that default namespaces do not apply directly to attributes.
 *
 * [...]
 *
 *  5.3 Uniqueness of Attributes
 *
 *  In XML documents conforming to this specification, no tag may
 *  contain two attributes which:
 *
 *    1. have identical names, or 
 *
 *    2. have qualified names with the same local part and with
 *    prefixes which have been bound to namespace names that are
 *    identical.
 * --------------------------------------------------------------------
 */

/**
 * raptor_new_qname - Create a new XML qname
 * @nstack: namespace stack to look up for namespaces
 * @name: element or attribute name
 * @value: attribute value (else is an element)
 * @error_handler: function to call on an error
 * @error_data: user data for error function
 * 
 * Create a new qname from the local element/attribute name,
 * with optional (attribute) value.  The namespace stack is used
 * to look up the name and find the namespace and generate the
 * URI of the qname.
 * 
 * Return value: 
 **/
raptor_qname*
raptor_new_qname(raptor_namespace_stack *nstack, const char *name,
                 const char *value,
                 raptor_internal_message_handler error_handler,
                 void *error_data)
{
  raptor_qname* qname;
  const char *p;
  raptor_namespace* ns;
  char* new_name;
  int prefix_length;
  int local_name_length=0;

#if RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2(raptor_new_qname, "name %s\n", name);
#endif  

  qname=(raptor_qname*)RAPTOR_CALLOC(raptor_qname, sizeof(raptor_qname), 1);
  if(!qname)
    return NULL;


  if(value) {
    int value_length=strlen(value);
    char* new_value=(char*)RAPTOR_MALLOC(cstring, value_length+1);

    if(!new_value) {
      RAPTOR_FREE(raptor_qname, qname);
      return NULL;
    } 
    strcpy(new_value, value);
    qname->value=new_value;
    qname->value_length=value_length;
  }


  /* Find : */
  for(p=name; *p && *p != ':'; p++)
    ;


  if(!*p) {
    local_name_length=p-name;

    /* No : in the name */
    new_name=(char*)RAPTOR_MALLOC(cstring, local_name_length+1);
    if(!new_name) {
      raptor_free_qname(qname);
      return NULL;
    }
    strcpy(new_name, name);
    qname->local_name=new_name;
    qname->local_name_length=local_name_length;

    /* For elements only, pick up the default namespace if there is one */
    if(!value) {
      ns=raptor_namespaces_get_default_namespace(nstack);
      
      if(ns) {
        qname->nspace=ns;
#if RAPTOR_DEBUG > 1
        RAPTOR_DEBUG2(raptor_new_qname,
                      "Found default namespace %s\n", ns->uri);
#endif
      } else {
#if RAPTOR_DEBUG > 1
        RAPTOR_DEBUG1(raptor_new_qname,
                      "No default namespace defined\n");
#endif
      }
    } /* if is_element */

  } else {
    /* There is a namespace prefix */

    prefix_length=p-name;
    p++; 

    /* p now is at start of local_name */
    local_name_length=strlen(p);
    new_name=(char*)RAPTOR_MALLOC(cstring, local_name_length+1);
    if(!new_name) {
      raptor_free_qname(qname);
      return NULL;
    }
    strcpy(new_name, p);
    qname->local_name=new_name;
    qname->local_name_length=local_name_length;

    /* Find the namespace */
    ns=raptor_namespaces_find_namespace(nstack, name, prefix_length);

    if(!ns) {
      /* failed to find namespace - now what? */
      if(error_handler)
        error_handler(error_data, "The namespace prefix in \"%s\" was not declared.", name);
    } else {
#if RAPTOR_DEBUG > 1
      RAPTOR_DEBUG3(raptor_new_qname,
                    "Found namespace prefix %s URI %s\n", ns->prefix, ns->uri);
#endif
      qname->nspace=ns;
    }
  }



  /* If namespace has a URI and a local_name is defined, create the URI
   * for this element 
   */
  if(qname->nspace && raptor_namespace_get_uri(qname->nspace) &&
     local_name_length) {
    raptor_uri* uri=raptor_namespace_local_name_to_uri(qname->nspace,
                                                       new_name);
    if(!uri) {
      raptor_free_qname(qname);
      return NULL;
    }
    qname->uri=uri;
  }


  return qname;
}


#ifdef RAPTOR_DEBUG
void
raptor_qname_print(FILE *stream, raptor_qname* name) 
{
  if(name->nspace) {
    const char *prefix=raptor_namespace_get_prefix(name->nspace);
    if(prefix)
      fprintf(stream, "%s:%s", prefix, name->local_name);
    else
      fprintf(stream, "(default):%s", name->local_name);
  } else
    fputs(name->local_name, stream);
}
#endif

void
raptor_free_qname(raptor_qname* name) 
{
  if(name->local_name)
    RAPTOR_FREE(cstring, (void*)name->local_name);

  if(name->uri)
    raptor_free_uri(name->uri);

  if(name->value)
    RAPTOR_FREE(cstring, (void*)name->value);
  RAPTOR_FREE(raptor_qname, name);
}


int
raptor_qname_equal(raptor_qname *name1, raptor_qname *name2)
{
  if(name1->nspace != name2->nspace)
    return 0;
  if(name1->local_name_length != name2->local_name_length)
    return 0;
  if(strcmp(name1->local_name, name2->local_name))
    return 0;
  return 1;
}



/*
 * Local Variables:
 * mode:c
 * c-basic-offset: 2
 * End:
 */
