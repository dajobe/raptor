/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_general.c - Raptor general routines
 *
 * Copyright (C) 2000-2014, David Beckett http://www.dajobe.org/
 * Copyright (C) 2000-2005, University of Bristol, UK http://www.bristol.ac.uk/
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

#ifdef MAINTAINER_MODE
#include <git-version.h>
#endif

/* statics */

const char * const raptor_short_copyright_string = "Copyright 2000-2014 David Beckett. Copyright 2000-2005 University of Bristol";

const char * const raptor_copyright_string = "Copyright (C) 2000-2014 David Beckett - http://www.dajobe.org/\nCopyright (C) 2000-2005 University of Bristol - http://www.bristol.ac.uk/";

const char * const raptor_license_string = "LGPL 2.1 or newer, GPL 2 or newer, Apache 2.0 or newer.\nSee http://librdf.org/raptor/LICENSE.html for full terms.";

const char * const raptor_home_url_string = "http://librdf.org/raptor/";

/**
 * raptor_version_string:
 *
 * Library full version as a string.
 *
 * See also #raptor_version_decimal.
 */
const char * const raptor_version_string = RAPTOR_VERSION_STRING
#ifdef GIT_VERSION
" GIT " GIT_VERSION
#endif
;

/**
 * raptor_version_major:
 *
 * Library major version number as a decimal integer.
 */
const unsigned int raptor_version_major = RAPTOR_VERSION_MAJOR;

/**
 * raptor_version_minor:
 *
 * Library minor version number as a decimal integer.
 */
const unsigned int raptor_version_minor = RAPTOR_VERSION_MINOR;

/**
 * raptor_version_release:
 *
 * Library release version number as a decimal integer.
 */
const unsigned int raptor_version_release = RAPTOR_VERSION_RELEASE;

/**
 * raptor_version_decimal:
 *
 * Library full version as a decimal integer.
 *
 * See also #raptor_version_string.
 */
const unsigned int raptor_version_decimal = RAPTOR_VERSION_DECIMAL;


/**
 * raptor_new_world:
 * @version_decimal: raptor version as a decimal integer as defined by the macro #RAPTOR_VERSION and static int #raptor_version_decimal
 *
 * Allocate a new raptor_world object.
 *
 * Allocation of the world and initialization are decoupled to allow
 * changing settings on the world object before init.
 *
 * Settings and configuration of the world may be made after creating
 * the object and before the world is initialized using methods such
 * as raptor_world_set_flag(), raptor_world_set_log_handler(),
 * raptor_world_set_generate_bnodeid_handler().  Some configuration
 * may not be changed after initialization.
 *
 * The raptor_world is initialized with raptor_world_open().
 *
 * Return value: uninitialized raptor_world object or NULL on failure
 */
raptor_world *
raptor_new_world_internal(unsigned int version_decimal)
{
  raptor_world *world;
  
  if(version_decimal < RAPTOR_MIN_VERSION_DECIMAL) {
    fprintf(stderr,
            "raptor_new_world() called via header from version %u but minimum supported version is %u\n",
            version_decimal, RAPTOR_MIN_VERSION_DECIMAL);
    return NULL;
  }
  
  world = RAPTOR_CALLOC(raptor_world*, 1, sizeof(*world));
  if(world) {
    world->magic = RAPTOR2_WORLD_MAGIC;
    
    /* set default flags - can be updated by raptor_world_set_flag() */

    /* set: RAPTOR_LIBXML_FLAGS_GENERIC_ERROR_SAVE
     * set: RAPTOR_LIBXML_FLAGS_STRUCTURED_ERROR_SAVE
     */
    world->libxml_flags = RAPTOR_WORLD_FLAG_LIBXML_GENERIC_ERROR_SAVE |
                          RAPTOR_WORLD_FLAG_LIBXML_STRUCTURED_ERROR_SAVE ;
    /* set: URI Interning */
    world->uri_interning = 1;

    world->internal_ignore_errors = 0;
  }
  
  return world;
}


/**
 * raptor_world_open:
 * @world: raptor_world object
 *
 * Initialise the raptor library.
 *
 * Initializes a #raptor_world object created by raptor_new_world().
 * Allocation and initialization are decoupled to allow
 * changing settings on the world object before init.
 *
 * The initialized world object is used with subsequent raptor API calls.
 *
 * Return value: non-0 on failure
 */
int
raptor_world_open(raptor_world* world)
{
  int rc;

  if(!world)
    return -1;

  if(world->opened)
    return 0; /* not an error */

  world->opened = 1;

  rc = raptor_uri_init(world);
  if(rc)
    return rc;

  rc = raptor_concepts_init(world);
  if(rc)
    return rc;

  rc = raptor_parsers_init(world);
  if(rc)
    return rc;

  rc = raptor_serializers_init(world);
  if(rc)
    return rc;

  rc = raptor_sax2_init(world);
  if(rc)
    return rc;

  rc = raptor_www_init(world); 
  if(rc)
    return rc;

#ifdef RAPTOR_XML_LIBXML
  rc = raptor_libxml_init(world);
  if(rc)
    return rc;
#endif
  
  return 0;
}


/**
 * raptor_free_world:
 * @world: raptor_world object
 *
 * Terminate the raptor library.
 *
 * Destroys the raptor_world object and all related information.
 */
void
raptor_free_world(raptor_world* world)
{
  if(!world)
    return;

  if(world->default_generate_bnodeid_handler_prefix)
    RAPTOR_FREE(char*, world->default_generate_bnodeid_handler_prefix);

#ifdef RAPTOR_XML_LIBXML
  raptor_libxml_finish(world);
#endif

  raptor_www_finish(world);

  raptor_sax2_finish(world);

  raptor_serializers_finish(world);

  raptor_parsers_finish(world);

  raptor_concepts_finish(world);

  raptor_uri_finish(world);

  RAPTOR_FREE(raptor_world, world);
}


/**
 * raptor_world_set_generate_bnodeid_handler:
 * @world: #raptor_world world object
 * @user_data: user data pointer for callback
 * @handler: generate ID callback function
 *
 * Set the generate ID handler function.
 *
 * Sets the function to generate IDs for the library.  The handler is
 * called with the @user_data parameter.
 *
 * The final argument of the callback method is user_bnodeid, the value of
 * the rdf:nodeID attribute that the user provided if any (or NULL).
 * It can either be returned directly as the generated value when present or
 * modified.  The passed in value must be free()d if it is not used.
 *
 * If handler is NULL, the default method is used
 * 
 **/
void
raptor_world_set_generate_bnodeid_handler(raptor_world* world,
                                          void *user_data,
                                          raptor_generate_bnodeid_handler handler)
{
  world->generate_bnodeid_handler_user_data = user_data;
  world->generate_bnodeid_handler = handler;
}


static unsigned char*
raptor_world_default_generate_bnodeid_handler(void *user_data,
                                              unsigned char *user_bnodeid) 
{
  raptor_world *world = (raptor_world*)user_data;
  int id;
  unsigned char *buffer;
  const char* prefix;
  unsigned int prefix_length;
  size_t id_length;

  if(user_bnodeid)
    return user_bnodeid;

  id = ++world->default_generate_bnodeid_handler_base;

  id_length = raptor_format_integer(NULL, 0, id, /* base */ 10, -1, '\0');

  if(world->default_generate_bnodeid_handler_prefix) {
    prefix = world->default_generate_bnodeid_handler_prefix;
    prefix_length = world->default_generate_bnodeid_handler_prefix_length;
  } else {
    prefix = "genid";
    prefix_length = 5; /* strlen("genid") */
  }

  buffer = RAPTOR_MALLOC(unsigned char*, id_length + prefix_length + 1);
  if(!buffer)
    return NULL;

  memcpy(buffer, prefix, prefix_length);
  (void)raptor_format_integer(RAPTOR_GOOD_CAST(char*, &buffer[prefix_length]),
                              id_length + 1, id, /* base */ 10,-1, '\0');

  return buffer;
}


/**
 * raptor_world_generate_bnodeid:
 * @world: raptor_world object
 * 
 * Generate an new blank node ID
 *
 * Return value: newly allocated generated ID or NULL on failure
 **/
unsigned char*
raptor_world_generate_bnodeid(raptor_world *world)
{
  return raptor_world_internal_generate_id(world, NULL);
}


unsigned char*
raptor_world_internal_generate_id(raptor_world *world, 
                                  unsigned char *user_bnodeid)
{
  if(world->generate_bnodeid_handler)
    return world->generate_bnodeid_handler(world->generate_bnodeid_handler_user_data,
                                           user_bnodeid);
  else
    return raptor_world_default_generate_bnodeid_handler(world, user_bnodeid);
}


/**
 * raptor_world_set_generate_bnodeid_parameters:
 * @world: #raptor_world object
 * @prefix: prefix string
 * @base: integer base identifier
 *
 * Set default ID generation parameters.
 *
 * Sets the parameters for the default algorithm used to generate IDs.
 * The default algorithm uses both @prefix and @base to generate a new
 * identifier.   The exact identifier generated is not guaranteed to
 * be a strict concatenation of @prefix and @base but will use both
 * parts. The @prefix parameter is copied to generate an ID.
 *
 * For finer control of the generated identifiers, use
 * raptor_world_set_generate_bnodeid_handler().
 *
 * If @prefix is NULL, the default prefix is used (currently "genid")
 * If @base is less than 1, it is initialised to 1.
 * 
 **/
void
raptor_world_set_generate_bnodeid_parameters(raptor_world* world, 
                                             char *prefix, int base)
{
  char *prefix_copy = NULL;
  unsigned int length = 0;

  if(--base < 0)
    base = 0;

  if(prefix) {
    length = RAPTOR_BAD_CAST(unsigned int, strlen(prefix));
    
    prefix_copy = RAPTOR_MALLOC(char*, length + 1);
    if(!prefix_copy)
      return;

    memcpy(prefix_copy, prefix, length+1);
  }
  
  if(world->default_generate_bnodeid_handler_prefix)
    RAPTOR_FREE(char*, world->default_generate_bnodeid_handler_prefix);

  world->default_generate_bnodeid_handler_prefix = prefix_copy;
  world->default_generate_bnodeid_handler_prefix_length = length;
  world->default_generate_bnodeid_handler_base = base;
}


/**
 * raptor_world_set_libxslt_security_preferences:
 * @world: world
 * @security_preferences: security preferences (an #xsltSecurityPrefsPtr) or NULL
 * 
 * Set libxslt security preferences policy object
 *
 * The @security_preferences object will NOT become owned by
 * #raptor_world.
 *
 * If libxslt is compiled into the library, @security_preferences
 * should be an #xsltSecurityPrefsPtr and will be used to call
 * xsltSetCtxtSecurityPrefs() when an XSLT engine is initialised.
 * If @security_preferences is NULL, this will disable all raptor's
 * calls to xsltSetCtxtSecurityPrefs().
 *
 * If libxslt is not compiled in, the object set here is not used.
 *
 * Return value: 0 on success, non-0 on failure: <0 on errors and >0 if world is already opened
 */
int
raptor_world_set_libxslt_security_preferences(raptor_world *world, 
                                              void *security_preferences)
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, -1);

  if(world->opened)
    return 1;

  world->xslt_security_preferences = security_preferences;
  world->xslt_security_preferences_policy = 1;

  return 0;
}


/**
 * raptor_world_set_flag:
 * @world: world
 * @flag: flag
 * @value: value
 * 
 * Set library-wide configuration
 *
 * This function is used to control raptor-wide options across
 * classes.  These options must be set before raptor_world_open() is
 * called explicitly or implicitly (by creating a raptor object).
 * There is no enumeration function for these flags because they are
 * not user options and must be set before the library is
 * initialised.  For similar reasons, there is no get function.
 *
 * See the #raptor_world_flags documentation for full details of
 * what the flags mean.
 *
 * Return value: 0 on success, non-0 on failure: <0 on errors (-1 if flag is unknown, -2 if value is illegal) and >0 if world is already opened
 */
int
raptor_world_set_flag(raptor_world *world, raptor_world_flag flag, int value)
{
  int rc = 0;
  
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, -1);

  if(world->opened)
    return 1;

  switch(flag) {
    case RAPTOR_WORLD_FLAG_LIBXML_GENERIC_ERROR_SAVE:
    case RAPTOR_WORLD_FLAG_LIBXML_STRUCTURED_ERROR_SAVE:
      if(value)
        world->libxml_flags |= (int)flag;
      else
        world->libxml_flags &= ~(int)flag;
      break;

    case RAPTOR_WORLD_FLAG_URI_INTERNING:
      world->uri_interning = value;
      break;

    case RAPTOR_WORLD_FLAG_WWW_SKIP_INIT_FINISH:
      world->www_skip_www_init_finish = value;
      break;
  }

  return rc;
}


/**
 * raptor_world_set_log_handler:
 * @world: world object
 * @user_data: user data to pass to function
 * @handler: pointer to the function
 *
 * Set the message (error, warning, info) handling function.
 * 
 * The function will receive callbacks when messages are generated
 * 
 * Return value: non-0 on failure
 **/
int
raptor_world_set_log_handler(raptor_world *world, void *user_data,
                             raptor_log_handler handler)
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(world, raptor_world, -1);

  world->message_handler_user_data = user_data;
  world->message_handler = handler;

  return 0;
}


/**
 * raptor_basename:
 * @name: path
 * 
 * Get the basename of a path
 * 
 * Return value: filename part of a pathname
 **/
const char*
raptor_basename(const char *name)
{
  const char *p;
  if((p = strrchr(name, '/')))
    name = p+1;
  else if((p = strrchr(name, '\\')))
    name = p+1;

  return name;
}


const unsigned char * const raptor_xml_literal_datatype_uri_string = (const unsigned char *)"http://www.w3.org/1999/02/22-rdf-syntax-ns#XMLLiteral";
const unsigned int raptor_xml_literal_datatype_uri_string_len = 53;

/**
 * raptor_check_ordinal:
 * @name: ordinal string
 *
 * Check an RDF property ordinal, the n in rdf:_n
 *
 * Return value: ordinal integer or <0 if string is not a valid ordinal
 */
int
raptor_check_ordinal(const unsigned char *name)
{
  int ordinal= -1;
  unsigned char c;

  while((c=*name++)) {
    if(c < '0' || c > '9')
      return -1;
    if(ordinal <0)
      ordinal = 0;
    ordinal *= 10;
    ordinal += (c - '0');
  }
  return ordinal;
}


static const char* const raptor_domain_labels[RAPTOR_DOMAIN_LAST + 1] = {
  "none",
  "I/O Stream",
  "XML Namespace",
  "RDF Parser",
  "XML QName",
  "XML SAX2",
  "RDF Serializer",
  "RDF Term",
  "Turtle Writer",
  "URI",
  "World",
  "WWW",
  "XML Writer"
};


/**
 * raptor_domain_get_label:
 * @domain: domain
 *
 * Get label for a domain
 *
 * Return value: label string or NULL if domain is not valid
 */
const char*
raptor_domain_get_label(raptor_domain domain)
{
  return (domain <= RAPTOR_DOMAIN_LAST) ? raptor_domain_labels[domain] : NULL;
}



/* internal */
void
raptor_world_internal_set_ignore_errors(raptor_world* world, int flag)
{
  world->internal_ignore_errors = flag;
}


/**
 * raptor_free_memory:
 * @ptr: memory pointer
 *
 * Free memory allocated inside raptor.
 * 
 * Some systems require memory allocated in a library to
 * be deallocated in that library.  This function allows
 * memory allocated by raptor to be freed.
 *
 * Examples include the result of the '_to_' methods that returns
 * allocated memory such as raptor_uri_filename_to_uri_string,
 * raptor_uri_filename_to_uri_string
 * and raptor_uri_uri_string_to_filename_fragment
 *
 **/
void
raptor_free_memory(void *ptr)
{
  if(!ptr)
    return;

  RAPTOR_FREE(void, ptr);
}


/**
 * raptor_alloc_memory:
 * @size: size of memory to allocate
 *
 * Allocate memory inside raptor.
 * 
 * Some systems require memory allocated in a library to
 * be deallocated in that library.  This function allows
 * memory to be allocated inside the raptor shared library
 * that can be freed inside raptor either internally or via
 * raptor_free_memory.
 *
 * Examples include using this in the raptor_world_generate_bnodeid() handler
 * code to create new strings that will be used internally
 * as short identifiers and freed later on by the parsers.
 *
 * Return value: the address of the allocated memory or NULL on failure
 *
 **/
void*
raptor_alloc_memory(size_t size)
{
  return RAPTOR_MALLOC(void*, size);
}


/**
 * raptor_calloc_memory:
 * @nmemb: number of members
 * @size: size of item
 *
 * Allocate zeroed array of items inside raptor.
 * 
 * Some systems require memory allocated in a library to
 * be deallocated in that library.  This function allows
 * memory to be allocated inside the raptor shared library
 * that can be freed inside raptor either internally or via
 * raptor_free_memory.
 *
 * Examples include using this in the raptor_world_generate_bnodeid() handler
 * code to create new strings that will be used internally
 * as short identifiers and freed later on by the parsers.
 *
 * Return value: the address of the allocated memory or NULL on failure
 *
 **/
void*
raptor_calloc_memory(size_t nmemb, size_t size)
{
  return RAPTOR_CALLOC(void*, nmemb, size);
}


#if defined (RAPTOR_DEBUG) && defined(RAPTOR_MEMORY_SIGN)
void*
raptor_sign_malloc(size_t size)
{
  int *p;
  
  size += sizeof(int);
  
  p = (int*)malloc(size);
  *p++ = RAPTOR_SIGN_KEY;
  return p;
}

void*
raptor_sign_calloc(size_t nmemb, size_t size)
{
  int *p;
  
  /* turn into bytes */
  size = nmemb*size + sizeof(int);
  
  p = (int*)calloc(1, size);
  *p++ = RAPTOR_SIGN_KEY;
  return p;
}

void*
raptor_sign_realloc(void *ptr, size_t size)
{
  int *p;

  if(!ptr)
    return raptor_sign_malloc(size);
  
  p = (int*)ptr;
  p--;

  if(*p != RAPTOR_SIGN_KEY)
    RAPTOR_FATAL3("memory signature %08X != %08X", *p, RAPTOR_SIGN_KEY);

  size += sizeof(int);
  
  p = (int*)realloc(p, size);
  *p++= RAPTOR_SIGN_KEY;
  return p;
}

void
raptor_sign_free(void *ptr)
{
  int *p;

  if(!ptr)
    return;
  
  p = (int*)ptr;
  p--;

  if(*p != RAPTOR_SIGN_KEY)
    RAPTOR_FATAL3("memory signature %08X != %08X", *p, RAPTOR_SIGN_KEY);

  free(p);
}
#endif


int
raptor_check_world_internal(raptor_world* world, const char* name)
{
  static int __warned = 0;

  if(!world) {
    fprintf(stderr, "%s called with NULL world object\n", name);
    RAPTOR_ASSERT_DIE
  }
  
  /* In Raptor V1 ABI the first int of raptor_world is the 'opened' field */
  if(world->magic == RAPTOR1_WORLD_MAGIC_1 ||
     world->magic == RAPTOR1_WORLD_MAGIC_2) {
    if(!__warned++)
      fprintf(stderr, "%s called with Raptor V1 world object\n", name);
    return 1;
  }

  if(world->magic != RAPTOR2_WORLD_MAGIC) {
    if(!__warned++)
      fprintf(stderr, "%s called with invalid Raptor V2 world object\n", name);
    return 1;
  }

  return 0;
}
