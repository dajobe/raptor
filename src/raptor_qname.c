/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_qname.c - Raptor XML qname class
 *
 * Copyright (C) 2002-2008, David Beckett http://www.dajobe.org/
 * Copyright (C) 2002-2004, University of Bristol, UK http://www.bristol.ac.uk/
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
 * raptor_new_qname:
 * @nstack: namespace stack to look up for namespaces
 * @name: element or attribute name
 * @value: attribute value (else is an element)
 *
 * Constructor - create a new XML qname.
 * 
 * Create a new qname from the local element/attribute name,
 * with optional (attribute) value.  The namespace stack is used
 * to look up the name and find the namespace and generate the
 * URI of the qname.
 * 
 * Return value: a new #raptor_qname object or NULL on failure
 **/
raptor_qname*
raptor_new_qname(raptor_namespace_stack *nstack, 
                 const unsigned char *name,
                 const unsigned char *value)
{
  raptor_qname* qname;
  const unsigned char *p;
  raptor_namespace* ns;
  unsigned char* new_name;
  unsigned int prefix_length;
  unsigned int local_name_length = 0;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2("name %s\n", name);
#endif  

  qname = RAPTOR_CALLOC(raptor_qname*, 1, sizeof(*qname));
  if(!qname)
    return NULL;
  qname->world = nstack->world;

  if(value) {
    size_t value_length = strlen((char*)value);
    unsigned char* new_value;

    new_value = RAPTOR_MALLOC(unsigned char*, value_length + 1);
    if(!new_value) {
      RAPTOR_FREE(raptor_qname, qname);
      return NULL;
    } 

    memcpy(new_value, value, value_length + 1); /* copy NUL */
    qname->value = new_value;
    qname->value_length = value_length;
  }


  /* Find : */
  for(p = name; *p && *p != ':'; p++)
    ;


  if(!*p) {
    local_name_length = (unsigned int)(p - name);

    /* No : in the name */
    new_name = RAPTOR_MALLOC(unsigned char*, local_name_length + 1);
    if(!new_name) {
      raptor_free_qname(qname);
      return NULL;
    }
    memcpy(new_name, name, local_name_length); /* no NUL to copy */
    new_name[local_name_length] = '\0';
    qname->local_name = new_name;
    qname->local_name_length = local_name_length;

    /* For elements only, pick up the default namespace if there is one */
    if(!value) {
      ns = raptor_namespaces_get_default_namespace(nstack);
      
      if(ns) {
        qname->nspace = ns;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
        RAPTOR_DEBUG2("Found default namespace %s\n", raptor_uri_as_string(ns->uri));
#endif
      } else {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
        RAPTOR_DEBUG1("No default namespace defined\n");
#endif
      }
    } /* if is_element */

  } else {
    /* There is a namespace prefix */

    prefix_length = (unsigned int)(p - name);
    p++; 

    /* p now is at start of local_name */
    local_name_length = (unsigned int)strlen((char*)p);
    new_name = RAPTOR_MALLOC(unsigned char*, local_name_length + 1);
    if(!new_name) {
      raptor_free_qname(qname);
      return NULL;
    }
    memcpy(new_name, p, local_name_length); /* No NUL to copy */
    new_name[local_name_length] = '\0';
    qname->local_name = new_name;
    qname->local_name_length = local_name_length;

    /* Find the namespace */
    ns = raptor_namespaces_find_namespace(nstack, name, prefix_length);

    if(!ns) {
      /* failed to find namespace - now what? */
      raptor_log_error_formatted(qname->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                                 "The namespace prefix in \"%s\" was not declared.", name);
    } else {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
      RAPTOR_DEBUG3("Found namespace prefix %s URI %s\n", ns->prefix, raptor_uri_as_string(ns->uri));
#endif
      qname->nspace = ns;
    }
  }



  /* If namespace has a URI and a local_name is defined, create the URI
   * for this element 
   */
  if(qname->nspace && local_name_length) {
    raptor_uri *uri = raptor_namespace_get_uri(qname->nspace);
    if(uri)
      uri = raptor_new_uri_from_uri_local_name(qname->world, uri, new_name);

    qname->uri = uri;
  }


  return qname;
}


/**
 * raptor_new_qname_from_namespace_local_name:
 * @world: raptor_world object
 * @ns: namespace of qname (or NULL)
 * @local_name: element or attribute name
 * @value: attribute value (else is an element)
 *
 * Constructor - create a new XML qname.
 * 
 * Create a new qname from the namespace and local element/attribute name,
 * with optional (attribute) value.
 * 
 * Return value: a new #raptor_qname object or NULL on failure
 **/
raptor_qname*
raptor_new_qname_from_namespace_local_name(raptor_world* world,
                                           raptor_namespace *ns, 
                                           const unsigned char *local_name,
                                           const unsigned char *value)
{
  raptor_qname* qname;
  unsigned char* new_name;
  unsigned int local_name_length;

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  if(!local_name)
    return NULL;

  local_name_length = (unsigned int)strlen((char*)local_name);

  raptor_world_open(world);

  qname = RAPTOR_CALLOC(raptor_qname*, 1, sizeof(*qname));
  if(!qname)
    return NULL;
  qname->world = world;

  if(value) {
    unsigned int value_length = (unsigned int)strlen((char*)value);
    unsigned char* new_value;

    new_value = RAPTOR_MALLOC(unsigned char*, value_length + 1);
    if(!new_value) {
      RAPTOR_FREE(raptor_qname, qname);
      return NULL;
    } 

    memcpy(new_value, value, value_length + 1); /* Copy NUL */
    qname->value = new_value;
    qname->value_length = value_length;
  }

  new_name = RAPTOR_MALLOC(unsigned char*, local_name_length + 1);
  if(!new_name) {
    raptor_free_qname(qname);
    return NULL;
  }

  memcpy(new_name, local_name, local_name_length); /* No NUL to copy */
  new_name[local_name_length] = '\0';
  qname->local_name = new_name;
  qname->local_name_length = local_name_length;

  qname->nspace = ns;

  if(qname->nspace) {
    qname->uri = raptor_namespace_get_uri(qname->nspace);
    if(qname->uri)
      qname->uri = raptor_new_uri_from_uri_local_name(qname->world, qname->uri, new_name);
  }
  
  return qname;
}


/**
 * raptor_qname_copy:
 * @qname: existing qname
 *
 * Copy constructor - copy an existing XML qname.
 *
 * Return value: a new #raptor_qname object or NULL on failure
 **/
raptor_qname*
raptor_qname_copy(raptor_qname *qname)
{
  raptor_qname* new_qname;
  unsigned char* new_name;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(qname, raptor_qname, NULL);
  
  new_qname = RAPTOR_CALLOC(raptor_qname*, 1, sizeof(*qname));
  if(!new_qname)
    return NULL;
  new_qname->world = qname->world;

  if(qname->value) {
    size_t value_length = qname->value_length;
    unsigned char* new_value;

    new_value = RAPTOR_MALLOC(unsigned char*, value_length + 1);
    if(!new_value) {
      RAPTOR_FREE(raptor_qname, new_qname);
      return NULL;
    } 

    memcpy(new_value, qname->value, value_length + 1); /* Copy NUL */
    new_qname->value = new_value;
    new_qname->value_length = value_length;
  }

  new_name = RAPTOR_MALLOC(unsigned char*, qname->local_name_length + 1);
  if(!new_name) {
    raptor_free_qname(new_qname);
    return NULL;
  }

  memcpy(new_name, qname->local_name, qname->local_name_length + 1); /* Copy NUL */
  new_qname->local_name = new_name;
  new_qname->local_name_length = qname->local_name_length;

  new_qname->nspace = qname->nspace;

  new_qname->uri = raptor_namespace_get_uri(new_qname->nspace);
  if(new_qname->uri)
    new_qname->uri = raptor_new_uri_from_uri_local_name(qname->world, new_qname->uri, new_name);
  
  return new_qname;
}


#ifdef RAPTOR_DEBUG
void
raptor_qname_print(FILE *stream, raptor_qname* name) 
{
  if(name->nspace) {
    const unsigned char *prefix = raptor_namespace_get_prefix(name->nspace);
    if(prefix)
      fprintf(stream, "%s:%s", prefix, name->local_name);
    else
      fprintf(stream, "(default):%s", name->local_name);
  } else
    fputs((char*)name->local_name, stream);
}
#endif


/**
 * raptor_free_qname:
 * @name: #raptor_qname object
 * 
 * Destructor - destroy a raptor_qname object.
 **/
void
raptor_free_qname(raptor_qname* name) 
{
  if(!name)
    return;

  if(name->local_name)
    RAPTOR_FREE(char*, name->local_name);

  if(name->uri && name->nspace)
    raptor_free_uri(name->uri);

  if(name->value)
    RAPTOR_FREE(char*, name->value);
  RAPTOR_FREE(raptor_qname, name);
}


/**
 * raptor_qname_equal:
 * @name1: first #raptor_qname
 * @name2: second #raptor_name
 * 
 * Compare two XML Qnames for equality.
 * 
 * Return value: non-0 if the qnames are equal.
 **/
int
raptor_qname_equal(raptor_qname *name1, raptor_qname *name2)
{
  if(name1->nspace != name2->nspace)
    return 0;
  if(name1->local_name_length != name2->local_name_length)
    return 0;
  if(strcmp((char*)name1->local_name, (char*)name2->local_name))
    return 0;
  return 1;
}



/**
 * raptor_qname_string_to_uri:
 * @nstack: #raptor_namespace_stack to decode the namespace
 * @name: QName string or NULL
 * @name_len: QName string length
 *
 * Get the URI for a qname.
 * 
 * Utility function to turn a string representing a QName in the
 * N3 style, into a new URI representing it.  A NULL name or name ":"
 * returns the default namespace URI.  A name "p:" returns
 * namespace name (URI) for the namespace with prefix "p".
 * 
 * Partially equivalent to 
 *   qname = raptor_new_qname(nstack, name, NULL);
 *   uri = raptor_uri_copy(qname->uri);
 *   raptor_free_qname(qname)
 * but without making the qname, and it also handles the NULL and
 * ":" name cases as well as error checking.
 *
 * Return value: new #raptor_uri object or NULL on failure
 **/
raptor_uri*
raptor_qname_string_to_uri(raptor_namespace_stack *nstack, 
                           const unsigned char *name, size_t name_len)
{
  raptor_uri *uri = NULL;
  const unsigned char *p;
  const unsigned char *original_name = name;
  const unsigned char *local_name = NULL;
  unsigned int local_name_length = 0;
  raptor_namespace* ns;

  /* Empty string is default namespace URI */
  if(!name) {
    ns = raptor_namespaces_get_default_namespace(nstack);
  } else {
    /* If starts with :, it is relative to default namespace, so skip it */
    if(*name == ':') {
      name++;
      name_len--;
      p = name + name_len;
    } else {
      for(p = name; *p && *p != ':'; p++)
        ;
    }
    
    /* If ends with :, it is the URI of a namespace */
    if(RAPTOR_GOOD_CAST(size_t, p-name) == (name_len - 1)) {
      ns = raptor_namespaces_find_namespace(nstack, name,
                                            RAPTOR_BAD_CAST(int, (name_len - 1)));
    } else {
      if(!*p) {
        local_name = name;
        local_name_length = (unsigned int)(p - name);
        
        /* pick up the default namespace if there is one */
        ns = raptor_namespaces_get_default_namespace(nstack);
      } else {
        /* There is a namespace prefix */
        unsigned int prefix_length = (unsigned int)(p - name);
        p++;

        local_name = p;
        local_name_length = (unsigned int)strlen((char*)p);
        
        /* Find the namespace */
        ns = raptor_namespaces_find_namespace(nstack, name, prefix_length);
      }
    }
  }
  
  if(!ns) {
    raptor_log_error_formatted(nstack->world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                               "The namespace prefix in \"%s\" was not declared.", 
                               original_name);
  }



  /* If namespace has a URI and a local_name is defined, return the URI
   * for this name
   */
  if(ns && (uri = raptor_namespace_get_uri(ns))) {
    if(local_name_length)
      uri = raptor_new_uri_from_uri_local_name(nstack->world, uri, local_name);
    else
      uri = raptor_uri_copy(uri);
  }

  return uri;
}


/**
 * raptor_qname_write:
 * @qname: QName to write
 * @iostr: raptor iosteram
 * 
 * Write a formatted qname to an iostream
 * 
 * Return value: non-0 on failure
 **/
int
raptor_qname_write(raptor_qname *qname, raptor_iostream* iostr)
{
  if(qname->nspace && qname->nspace->prefix_length > 0) {
    raptor_iostream_counted_string_write(qname->nspace->prefix,
                                         qname->nspace->prefix_length,
                                         iostr);
    raptor_iostream_write_byte(':', iostr);
  }
  
  raptor_iostream_counted_string_write(qname->local_name,
                                       qname->local_name_length,
                                       iostr);
  return 0;
}


/**
 * raptor_qname_to_counted_name:
 * @qname: QName to write
 * @length_p: pointer to variable to store length of name (or NULL)
 * 
 * Get the string form of a QName name
 * 
 * Return value: new string name or NULL on failure
 **/
unsigned char*
raptor_qname_to_counted_name(raptor_qname *qname, size_t* length_p)
{
  size_t len = qname->local_name_length;
  unsigned char* s;
  unsigned char *p;

  if(qname->nspace && qname->nspace->prefix_length > 0)
    len+= 1 + qname->nspace->prefix_length;

  if(length_p)
    *length_p=len;
  
  s = RAPTOR_MALLOC(unsigned char*, len + 1);
  if(!s)
    return NULL;

  p = s;
  if(qname->nspace && qname->nspace->prefix_length > 0) {
    memcpy(p, qname->nspace->prefix, qname->nspace->prefix_length); /* Do not copy NUL */
    p += qname->nspace->prefix_length;
    *p++ = ':';
  }
  
  memcpy(p, qname->local_name, qname->local_name_length + 1); /* Copy final NUL */

  return s;
}


/**
 * raptor_qname_get_namespace:
 * @name: #raptor_qname object
 * 
 * Get the #raptor_namespace of an XML QName.
 * 
 * Return value: the namespace
 **/
const raptor_namespace*
raptor_qname_get_namespace(raptor_qname* name)
{
  return name->nspace;
}


/**
 * raptor_qname_get_local_name:
 * @name: #raptor_qname object
 * 
 * Get the #raptor_local_name of an XML QName.
 * 
 * Return value: the local_name
 **/
const unsigned char*
raptor_qname_get_local_name(raptor_qname* name)
{
  return name->local_name;
}


/**
 * raptor_qname_get_value:
 * @name: #raptor_qname object
 * 
 * Get the #raptor_value of an XML QName.
 * 
 * Return value: the value
 **/
const unsigned char*
raptor_qname_get_value(raptor_qname* name)
{
  return name->value;
}

/**
 * raptor_qname_get_counted_value:
 * @name: #raptor_qname object
 * @length_p: pointer to variable to store length of name (or NULL)
 * 
 * Get the #raptor_value of an XML QName.
 * 
 * Return value: the value
 **/
const unsigned char*
raptor_qname_get_counted_value(raptor_qname* name, size_t* length_p)
{
  if(length_p)
    *length_p=name->value_length;
  return name->value;
}


/**
 * raptor_qname_format_as_xml:
 * @qname: qname object
 * @length_p: pointer to length (or NULL)
 *
 * Format a qname in an XML style into a newly allocated string.
 *
 * Generates a string of the form a:b="value" or a="value"
 * depending on the qname's prefix.  Double quotes are always used.
 *
 * If @length_p is not NULL, the length of the string is
 * stored in the address it points to.
 *
 * Return value: qname formatted as newly allocated string or NULL on failure
 **/
unsigned char*
raptor_qname_format_as_xml(const raptor_qname *qname, size_t *length_p)
{
  size_t length;
  unsigned char *buffer;
  const char quote='"';
  unsigned char *p;

  length = qname->local_name_length + 3 /* ="" */;
  if(qname->value_length)
    length += raptor_xml_escape_string(qname->world,
                                       qname->value, qname->value_length,
                                       NULL, 0, quote);

  if(qname->nspace && qname->nspace->prefix_length > 0)
    length += qname->nspace->prefix_length + 1; /* for : */

  if(length_p)
    *length_p = length;

  buffer = RAPTOR_MALLOC(unsigned char*, length + 1);
  if(!buffer)
    return NULL;

  p = buffer;

  if(qname->nspace && qname->nspace->prefix_length > 0) {
    memcpy(p, qname->nspace->prefix, qname->nspace->prefix_length);
    p += qname->nspace->prefix_length;
    *p++ = ':';
  }
  memcpy(p, qname->local_name, qname->local_name_length);
  p += qname->local_name_length;
  *p++ = '=';
  *p++ = quote;
  if(qname->value_length) {
    p += raptor_xml_escape_string(qname->world,
                                  qname->value, qname->value_length,
                                  p, length, quote);
  }
  *p++ = quote;
  /* *p used here since we never need to use value of p again [CLANG] */
  *p = '\0';

  return buffer;
}


