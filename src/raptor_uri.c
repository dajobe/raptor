/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_uri.c - Raptor URI class
 *
 * Copyright (C) 2002-2010, David Beckett http://www.dajobe.org/
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

#if defined(STANDALONE) && defined(HAVE_UNISTD_H) && defined(HAVE_SYS_STAT_H)
/* for lstat() used in main() test which is in POSIX */
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#endif

#include <stdio.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif
#include <ctype.h>
#include <stdarg.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
/* for ptrdiff_t */
#ifdef HAVE_STDDEF_H
#include <stddef.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


/* Symbian OS uses similar path mappings as Windows but does not necessarily have the WIN32 flag defined */
#if defined(__SYMBIAN32__) && !defined(WIN32)
#define WIN32
#endif


/* raptor_uri structure */
struct raptor_uri_s {
  /* raptor_world object */
  raptor_world *world;
  /* the URI string */
  unsigned char *string;
  /* length of string */
  unsigned int length;
  /* usage count */
  int usage;
};

#ifndef STANDALONE

#define RAPTOR_URI_GETCWD_MAX 65536

static int
raptor_uri_hex_digit(unsigned char c)
{
  if(c >= '0' && c <= '9')
    return c - '0';
  if(c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if(c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return -1;
}

static int
raptor_uri_ascii_scheme_start(unsigned char c)
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static int
raptor_uri_ascii_scheme_char(unsigned char c)
{
  return raptor_uri_ascii_scheme_start(c) || (c >= '0' && c <= '9') ||
         c == '+' || c == '-' || c == '.';
}

/* Return decoded byte (0-255), -1 malformed %, -2 if not % (caller advances).
 *
 * Decoded NUL (%00) is returned literally as 0.  The "reject decoded NUL"
 * policy lives in the file-path callers (raptor_uri_file_path_decoded_length
 * and raptor_uri_copy_file_path_decoded) which treat c <= 0 as failure.
 * If a non-file-path caller is added in the future, replicate the c == 0
 * check there or move it into a wrapper; do not relax it inside the
 * file-path callers. */
static int
raptor_uri_percent_decode_byte(const unsigned char *s)
{
  int h1;
  int h2;

  if(*s != '%')
    return -2;

  if(!s[1] || !s[2])
    return -1;

  h1 = raptor_uri_hex_digit(s[1]);
  h2 = raptor_uri_hex_digit(s[2]);
  if(h1 < 0 || h2 < 0)
    return -1;

  return (h1 << 4) | h2;
}

static int
raptor_uri_filename_char_needs_escape(unsigned char c)
{
  if(c <= 0x20 || c >= 0x7f)
    return 1;

  switch(c) {
    case '%':
    case '#':
    case '?':
    case '<':
    case '>':
    case '"':
    case '{':
    case '}':
    case '|':
    case '\\':
    case '^':
    case '`':
    case '[':
    case ']':
      return 1;
    default:
      return 0;
  }
}

/* Return decoded path length, or 0 for malformed or NUL file path escapes. */
static size_t
raptor_uri_file_path_decoded_length(const unsigned char *path)
{
  size_t len = 0;
  const unsigned char *from = path;

  while(*from) {
    if(*from == '%') {
      /* Reject malformed % and decoded NUL (%00) in file paths */
      if(raptor_uri_percent_decode_byte(from) <= 0)
        return 0;
      from += 3;
      if(RAPTOR_SIZE_T_ADD_OVERFLOWS(len, 1))
        return 0;
      len++;
    } else {
      from++;
      if(RAPTOR_SIZE_T_ADD_OVERFLOWS(len, 1))
        return 0;
      len++;
    }
  }

  return len;
}

/* Copy decoded file path to @to; return bytes written or (size_t)-1 on error. */
static size_t
raptor_uri_copy_file_path_decoded(const unsigned char *path, char *to)
{
  char *p = to;
  const unsigned char *from = path;

  while(*from) {
    if(*from == '%') {
      int c = raptor_uri_percent_decode_byte(from);
      /* Reject malformed % and decoded NUL (%00) in file paths */
      if(c <= 0)
        return (size_t)-1;
      *p++ = (char)c;
      from += 3;
    } else {
      *p++ = (char)*from++;
    }
  }
  *p = '\0';

  return (size_t)(p - to);
}

/**
 * raptor_new_uri_from_counted_string:
 * @world: raptor_world object
 * @uri_string: URI string.
 * @length: length of URI string
 * 
 * Constructor - create a raptor URI from a UTF-8 encoded Unicode string.
 * 
 * Note: The @uri_string need not be NULL terminated - a NULL will be
 * added to the copied string used.  The counted string must not contain
 * embedded NUL bytes and @length must be non-zero and no larger than UINT_MAX.
 *
 * Return value: a new #raptor_uri object or NULL on failure.
 **/
raptor_uri*
raptor_new_uri_from_counted_string(raptor_world* world,
                                   const unsigned char *uri_string,
                                   size_t length)
{
  raptor_uri* new_uri;
  unsigned char *new_string;
  
  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  if(!uri_string || !*uri_string || length == 0 || length > UINT_MAX)
    return NULL;

  if(memchr(uri_string, '\0', length))
    return NULL;

  raptor_world_open(world);

  if(world->uris_tree) {
    raptor_uri key; /* on stack - not allocated */

    /* just to be safe */
    memset(&key, 0, sizeof(key));

    key.string = (unsigned char*)uri_string;
    key.length = (unsigned int)length;

    /* if existing URI found in tree, return it */
    new_uri = (raptor_uri*)raptor_avltree_search(world->uris_tree, &key);
    if(new_uri) {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
      RAPTOR_DEBUG3("Found existing URI %s with current usage %d\n",
                    uri_string, new_uri->usage);
#endif
      
      new_uri->usage++;
      
      goto unlock;
    }
  }
  

  /* otherwise create a new one */

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG1("Creating new URI '");
  fwrite(uri_string, sizeof(char), length, RAPTOR_DEBUG_FH);
  fputs("' in hash\n", RAPTOR_DEBUG_FH);
#endif

  new_uri = RAPTOR_CALLOC(raptor_uri*, 1, sizeof(*new_uri));
  if(!new_uri)
    goto unlock;

  new_uri->world = world;
  new_uri->length = (unsigned int)length;

  if(1) {
    size_t alloc_len;
    if(RAPTOR_SIZE_T_ADD_OVERFLOWS(length, 1)) {
      RAPTOR_FREE(raptor_uri, new_uri);
      new_uri = NULL;
      goto unlock;
    }
    alloc_len = length + 1;
    new_string = RAPTOR_MALLOC(unsigned char*, alloc_len);
  }
  if(!new_string) {
    RAPTOR_FREE(raptor_uri, new_uri);
    new_uri=NULL;
    goto unlock;
  }
  
  memcpy((char*)new_string, (const char*)uri_string, length);
  new_string[length] = '\0';
  new_uri->string = new_string;

  new_uri->usage = 1; /* for user */

  /* store in tree */
  if(world->uris_tree) {
    if(raptor_avltree_add(world->uris_tree, new_uri)) {
      RAPTOR_FREE(char*, new_string);
      RAPTOR_FREE(raptor_uri, new_uri);
      new_uri = NULL;
    }
  }

 unlock:

  return new_uri;
}


/**
 * raptor_new_uri:
 * @world: raptor_world object
 * @uri_string: URI string.
 * 
 * Constructor - create a raptor URI from a UTF-8 encoded Unicode string.
 * 
 * Return value: a new #raptor_uri object or NULL on failure.
 **/
raptor_uri*
raptor_new_uri(raptor_world* world, const unsigned char *uri_string) 
{
  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  if(!uri_string)
    return NULL;
  
  raptor_world_open(world);

  return raptor_new_uri_from_counted_string(world, uri_string,
                                            strlen((const char*)uri_string));
}


/**
 * raptor_new_uri_from_uri_local_name:
 * @world: raptor_world object
 * @uri: existing #raptor_uri
 * @local_name: non-NULL local name
 * 
 * Constructor - create a raptor URI from an existing URI and a local name.
 *
 * Creates a new URI from the concatenation of the @local_name to the
 * @uri.  This is NOT relative URI resolution, which is done by the
 * raptor_new_uri_relative_to_base() constructor.
 * 
 * Return value: a new #raptor_uri object or NULL on failure.
 **/
raptor_uri*
raptor_new_uri_from_uri_local_name(raptor_world* world, raptor_uri *uri,
                                   const unsigned char *local_name)
{
  size_t alloc_len;
  size_t content_len;
  unsigned char *new_string;
  raptor_uri* new_uri;
  size_t local_name_length;

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  if(!uri || !local_name)
    return NULL;
  
  raptor_world_open(world);

  local_name_length = strlen((const char*)local_name);
  
  if(RAPTOR_SIZE_T_ADD_OVERFLOWS((size_t)uri->length, local_name_length))
    return NULL;
  content_len = (size_t)uri->length + local_name_length;
  if(RAPTOR_SIZE_T_ADD_OVERFLOWS(content_len, 1))
    return NULL;
  alloc_len = content_len + 1;

  new_string = RAPTOR_MALLOC(unsigned char*, alloc_len);
  if(!new_string)
    return NULL;

  memcpy((char*)new_string, (const char*)uri->string, uri->length);
  memcpy((char*)(new_string + uri->length), (const char*)local_name,
         local_name_length + 1);

  new_uri = raptor_new_uri_from_counted_string(world, new_string, content_len);
  RAPTOR_FREE(char*, new_string);

  return new_uri;
}


/**
 * raptor_new_uri_relative_to_base_counted:
 * @world: raptor_world object
 * @base_uri: existing base URI
 * @uri_string: relative URI string
 * @uri_len: length of URI string (or 0)
 * 
 * Constructor - create a raptor URI from a base URI and a relative counted URI string.
 * 
 * Return value: a new #raptor_uri object or NULL on failure.
 **/
raptor_uri*
raptor_new_uri_relative_to_base_counted(raptor_world* world,
                                        raptor_uri *base_uri, 
                                        const unsigned char *uri_string,
                                        size_t uri_len)
{
  unsigned char *buffer;
  size_t buffer_length;
  raptor_uri* new_uri;
  size_t actual_length;
  
  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  if(!base_uri || !uri_string)
    return NULL;

  if(!uri_len)
    uri_len = strlen(RAPTOR_GOOD_CAST(const char*, uri_string));

  raptor_world_open(world);

  /* If URI string is empty, just copy base URI */
  if(!*uri_string)
    return raptor_uri_copy(base_uri);
  
  /* +1 for merge path slash; +1 for NUL terminator in buffer capacity */
  if(RAPTOR_SIZE_T_ADD_OVERFLOWS((size_t)base_uri->length, uri_len))
    return NULL;
  buffer_length = (size_t)base_uri->length + uri_len;
  if(RAPTOR_SIZE_T_ADD_OVERFLOWS(buffer_length, 2))
    return NULL;
  buffer_length += 2;

  buffer = RAPTOR_MALLOC(unsigned char*, buffer_length);
  if(!buffer)
    return NULL;
  
  actual_length = raptor_uri_resolve_uri_reference(base_uri->string, uri_string,
                                                   buffer, buffer_length);

  new_uri = raptor_new_uri_from_counted_string(world, buffer, actual_length);
  RAPTOR_FREE(char*, buffer);
  return new_uri;
}


/**
 * raptor_new_uri_relative_to_base:
 * @world: raptor_world object
 * @base_uri: existing base URI
 * @uri_string: relative URI string
 * 
 * Constructor - create a raptor URI from a base URI and a relative URI string.
 *
 * Use raptor_new_uri_relative_to_base_counted() if the URI string length is known
 *
 * Return value: a new #raptor_uri object or NULL on failure.
 **/
raptor_uri*
raptor_new_uri_relative_to_base(raptor_world* world,
                                raptor_uri *base_uri, 
                                const unsigned char *uri_string)
{
  return raptor_new_uri_relative_to_base_counted(world, base_uri, 
                                                 uri_string, 0);
}


/**
 * raptor_new_uri_from_id:
 * @world: raptor_world object
 * @base_uri: existing base URI
 * @id: RDF ID
 * 
 * Constructor - create a new URI from a base URI and RDF ID.
 *
 * This creates a URI equivalent to concatenating @base_uri with
 * ## and @id.
 * 
 * Return value: a new #raptor_uri object or NULL on failure.
 **/
raptor_uri*
raptor_new_uri_from_id(raptor_world *world, raptor_uri *base_uri,
                       const unsigned char *id) 
{
  raptor_uri *new_uri;
  unsigned char *local_name;
  size_t len;

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  if(!base_uri || !id)
    return NULL;

  raptor_world_open(world);

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2("Using ID %s\n", id);
#endif

  len = strlen((char*)id);
  /* "#id\0" */
  if(RAPTOR_SIZE_T_ADD_OVERFLOWS(len, 2))
    return NULL;
  local_name = RAPTOR_MALLOC(unsigned char*, len + 1 + 1);
  if(!local_name)
    return NULL;

  *local_name = '#';
  memcpy(local_name + 1, id, len + 1); /* len+1 to copy NUL */

  new_uri = raptor_new_uri_relative_to_base(world, base_uri, local_name);
  RAPTOR_FREE(char*, local_name);
  return new_uri;
}


/**
 * raptor_new_uri_for_rdf_concept:
 * @world: raptor_world object
 * @name: RDF namespace concept
 * 
 * Constructor - create a raptor URI for the RDF namespace concept name.
 *
 * Example: u=raptor_new_uri_for_rdf_concept("value") creates a new
 * URI for the rdf:value term.
 * 
 * Return value: a new #raptor_uri object or NULL on failure
 **/
raptor_uri*
raptor_new_uri_for_rdf_concept(raptor_world* world, const unsigned char *name) 
{
  raptor_uri *new_uri;
  unsigned char *new_uri_string;
  const unsigned char *base_uri_string = raptor_rdf_namespace_uri;
  size_t base_uri_string_len = raptor_rdf_namespace_uri_len;
  size_t new_uri_string_len;
  size_t name_len;
  
  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  if(!name)
    return NULL;
  
  raptor_world_open(world);

  name_len = strlen((const char*)name);
  if(RAPTOR_SIZE_T_ADD_OVERFLOWS(base_uri_string_len, name_len))
    return NULL;
  new_uri_string_len = base_uri_string_len + name_len;
  if(RAPTOR_SIZE_T_ADD_OVERFLOWS(new_uri_string_len, 1))
    return NULL;
  new_uri_string = RAPTOR_MALLOC(unsigned char*, new_uri_string_len + 1);
  if(!new_uri_string)
    return NULL;

  memcpy(new_uri_string, base_uri_string, base_uri_string_len);
  memcpy(new_uri_string + base_uri_string_len, name, name_len + 1); /* copy NUL */

  new_uri = raptor_new_uri_from_counted_string(world, new_uri_string,
                                               new_uri_string_len);
  RAPTOR_FREE(char*, new_uri_string);
  
  return new_uri;
}


/**
 * raptor_new_uri_from_uri_or_file_string:
 * @world: raptor_world object
 * @base_uri: existing base URI
 * @uri_or_file_string: URI string or filename
 *
 * Constructor - create a raptor URI from a string that is a relative or absolute URI or a filename
 *
 * If the @uri_or_file_string is a filename PATH that exists, the
 * result will be a URI file://PATH
 *
 * Absolute URIs are used directly and do not require @base_uri.  Relative
 * URI strings that are not existing filenames require @base_uri for
 * resolution.
 *
 * SECURITY: this function is intended for trusted CLI-style entry points
 * (e.g. `rapper file_or_url`) where filename-vs-URI disambiguation by
 * filesystem existence is desired.  For untrusted input the caller's
 * behaviour depends on filesystem state: a non-URI string that happens to
 * name an existing file is treated as a filename and converted to a
 * file:// URI.  Callers that process untrusted input MUST gate the result
 * with RAPTOR_OPTION_NO_FILE and/or a URI filter (see
 * raptor_parser_set_uri_filter, raptor_sax2_set_uri_filter); the URI
 * layer itself does not sandbox file access.
 *
 * Return value: a new #raptor_uri object or NULL on failure
 **/
raptor_uri*
raptor_new_uri_from_uri_or_file_string(raptor_world* world,
                                       raptor_uri* base_uri,
                                       const unsigned char* uri_or_file_string)
{
  raptor_uri* new_uri = NULL;
  const unsigned char* new_uri_string = NULL;
  const char* path = NULL;

  if(!uri_or_file_string)
    return NULL;

  if(raptor_uri_uri_string_is_absolute(uri_or_file_string) > 0) {
    new_uri = raptor_new_uri(world, uri_or_file_string);
    if(!new_uri)
      return NULL;
    new_uri_string = raptor_uri_as_string(new_uri);
    path = raptor_uri_uri_string_to_counted_filename_fragment(new_uri_string,
                                                              NULL, NULL, NULL);
  } else if(raptor_uri_filename_exists(uri_or_file_string) > 0) {
    /* uri_or_file_string is a file name, not a file: URI */
    path = RAPTOR_GOOD_CAST(const char*, uri_or_file_string);
  } else {
    new_uri = raptor_new_uri_relative_to_base(world, base_uri,
                                            uri_or_file_string);
    if(!new_uri)
      return NULL;
    new_uri_string = raptor_uri_as_string(new_uri);
    path = raptor_uri_uri_string_to_counted_filename_fragment(new_uri_string, 
                                                              NULL, NULL, NULL);
  }
  
  if(path) {
    if(new_uri) {
      raptor_free_uri(new_uri);
      new_uri = NULL;
    }
    
    /* new_uri_string is a string like "file://" + path */
    new_uri_string = raptor_uri_filename_to_uri_string(path);
    if(path != RAPTOR_GOOD_CAST(const char*, uri_or_file_string))
      RAPTOR_FREE(const char*, path);

    new_uri = raptor_new_uri(world, new_uri_string);
    RAPTOR_FREE(char*, new_uri_string);
  }

  return new_uri;
}


/**
 * raptor_free_uri:
 * @uri: URI to destroy
 *
 * Destructor - destroy a #raptor_uri object
 **/
void
raptor_free_uri(raptor_uri *uri)
{
  if(!uri)
    return;

  uri->usage--;
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG3("URI %s usage count now %d\n", uri->string, uri->usage);
#endif

  /* decrement usage, don't free if not 0 yet*/
  if(uri->usage > 0) {
    return;
  }

  /* this does not free the uri */
  if(uri->world->uris_tree)
    raptor_avltree_delete(uri->world->uris_tree, uri);

  if(uri->string)
    RAPTOR_FREE(char*, uri->string);
  RAPTOR_FREE(raptor_uri, uri);
}


/**
 * raptor_uri_equals:
 * @uri1: URI 1 (may be NULL)
 * @uri2: URI 2 (may be NULL)
 * 
 * Check if two URIs are equal.
 * 
 * A NULL URI is not equal to a non-NULL URI.
 *
 * Return value: non-0 if the URIs are equal
 **/
int
raptor_uri_equals(raptor_uri* uri1, raptor_uri* uri2)
{
  if(uri1 && uri2) {
    /* Both not-NULL - compare for equality */
    if(uri1 == uri2)
      return 1;
    else if (uri1->length != uri2->length)
      /* Different if lengths are different */
      return 0;
    else
      /* Same length compare: do not need strncmp() NUL checking */
      return memcmp((const char*)uri1->string, (const char*)uri2->string,
                    uri1->length) == 0;
  } else if(uri1 || uri2)
    /* Only one is NULL - not equal */
    return 0;
  else
    /* both NULL - equal */
    return 1;
}


/**
 * raptor_uri_compare:
 * @uri1: URI 1 (may be NULL)
 * @uri2: URI 2 (may be NULL)
 * 
 * Compare two URIs, ala strcmp.
 * 
 * A NULL URI is always less than (never equal to) a non-NULL URI.
 *
 * Return value: -1 if uri1 < uri2, 0 if equal, 1 if uri1 > uri2
 **/
int
raptor_uri_compare(raptor_uri* uri1, raptor_uri* uri2)
{
  if(uri1 == uri2)
    return 0;

  if(uri1 && uri2)  {
    /* compare common (shortest) prefix */
    unsigned int len = (uri1->length > uri2->length) ?
                       uri2->length : uri1->length;

    /* Same length compare: Do not need the strncmp() NUL checking */
    int result = memcmp((const char*)uri1->string, (const char*)uri2->string,
                        len);
    if(!result)
      /* if prefix is the same, the shorter is earlier */
      result = uri1->length - uri2->length;
    return result;
  }

  /* One arg is NULL - sort that first */
  return (!uri1) ? -1 : 1;
}


/**
 * raptor_uri_copy:
 * @uri: URI object
 * 
 * Constructor - get a copy of a URI.
 *
 * Return value: a new #raptor_uri object or NULL on failure
 **/
raptor_uri*
raptor_uri_copy(raptor_uri *uri) 
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(uri, raptor_uri, NULL);
  
  uri->usage++;
  return uri;
}


/**
 * raptor_uri_as_string:
 * @uri: #raptor_uri object
 * 
 * Get a string representation of a URI.
 *
 * Returns a shared pointer to a string representation of @uri.  This
 * string is shared and must not be freed, otherwise see use the
 * raptor_uri_to_string() or raptor_uri_to_counted_string() methods.
 * 
 * Return value: shared string representation of URI
 **/
unsigned char*
raptor_uri_as_string(raptor_uri *uri) 
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(uri, raptor_uri, NULL);

  return uri->string;
}


/**
 * raptor_uri_as_counted_string:
 * @uri: URI object
 * @len_p: address of length variable or NULL
 * 
 * Get a string representation of a URI with count.
 *
 * Returns a shared pointer to a string representation of @uri along
 * with the length of the string in @len_p, if not NULL.  This
 * string is shared and must not be freed, otherwise see use the
 * raptor_uri_to_string() or raptor_uri_to_counted_string() methods.
 * 
 * Return value: shared string representation of URI
 **/
unsigned char*
raptor_uri_as_counted_string(raptor_uri *uri, size_t* len_p) 
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(uri, raptor_uri, NULL);

  if(len_p)
    *len_p = uri->length;
  return uri->string;
}


/**
 * raptor_uri_counted_filename_to_uri_string:
 * @filename: The filename to convert
 * @filename_len: length of @filename or 0 to count it here
 *
 * Converts a counted filename to a file: URI.
 * 
 * Handles the OS-specific escaping on turning filenames into URIs
 * and returns a new buffer that the caller must free().  Unsafe URI
 * characters in the filename are encoded as \%XX escapes.
 *
 * Return value: A newly allocated string with the URI or NULL on failure
 **/
unsigned char*
raptor_uri_counted_filename_to_uri_string(const char *filename,
                                          size_t filename_len)
{
  unsigned char *buffer = NULL;
  const char *from;
  char *to;
#ifndef WIN32
  char *path = NULL;
#endif
  /* "file://" */
#define RAPTOR_LEN_FILE_CSS 7 
  static const char hex_digits[] = "0123456789ABCDEF";
  size_t len = RAPTOR_LEN_FILE_CSS;
  size_t fl;
  unsigned char c;

  if(!filename)
    return NULL;

  if(!filename_len)
    filename_len = strlen(filename);
  
#ifdef WIN32
/*
 * On WIN32, filenames turn into
 *   "file://" + translated filename
 * where the translation is \\ turns into / and unsafe URI characters
 * are encoded as \%XX escapes
 * and if the filename does not start with '\', it is relative
 * in which case, a . is appended to the authority
 *
 * e.g
 *  FILENAME              URI
 *  c:\windows\system     file:///c:/windows/system
 *  \\server\dir\file.doc file://server/dir/file.doc
 *  a:foo                 file:///a:./foo
 *  C:\Documents and Settings\myapp\foo.bat
 *                        file:///C:/Documents%20and%20Settings/myapp/foo.bat
 *
 * There are also UNC names \\server\share\blah
 * that turn into file:///server/share/blah
 * using the above algorithm.
 */
  if(filename_len > 1 && filename[1] == ':' &&
     (filename_len < 3 || filename[2] != '\\')) {
    if(RAPTOR_SIZE_T_ADD_OVERFLOWS(len, 3))
      goto path_done;
    len += 3; /* relative filename - add / and ./ */
  } else if(*filename == '\\') {
    if(len < 2)
      goto path_done;
    len -= 2; /* two // from not needed in filename */
  } else {
    if(RAPTOR_SIZE_T_ADD_OVERFLOWS(len, 1))
      goto path_done;
    len++; /* / at start of path */
  }

#else
/* others - unix: turn unsafe URI characters into %XX escapes */

  if(*filename != '/') {
    size_t path_max;
    size_t path_len;
    size_t new_filename_len;

#ifdef PATH_MAX
    path_max = PATH_MAX;
#else
    path_max = 1024; /* an initial guess at the length */
#endif
    path = (char*)malloc(path_max);
    while(path) {
      char *new_path;

      if(getcwd(path, path_max))
        break;

      /* getcwd failed */
      if(errno != ERANGE) {
        free(path);
        path = NULL;
        break;
      }

      if(path_max >= RAPTOR_URI_GETCWD_MAX) {
        free(path);
        path = NULL;
        break;
      }

      /* try again with a bigger buffer */
      path_max *= 2;
      if(path_max > RAPTOR_URI_GETCWD_MAX)
        path_max = RAPTOR_URI_GETCWD_MAX;
      new_path = (char*)realloc(path, path_max);
      if(!new_path) {
        free(path);
        path = NULL;
        break;
      }
      path = new_path;
    }
    if(!path)
      goto path_done;
    path_len = strlen(path);

    /* path + '/' + filename */
    if(RAPTOR_SIZE_T_ADD_OVERFLOWS(path_len, 1))
      goto path_done;
    if(RAPTOR_SIZE_T_ADD_OVERFLOWS(path_len + 1, filename_len))
      goto path_done;
    new_filename_len = path_len + 1 + filename_len;
    if(RAPTOR_SIZE_T_ADD_OVERFLOWS(new_filename_len, 1))
      goto path_done;
    if(path_max < new_filename_len + 1) {
      char *new_path = (char*)realloc(path, new_filename_len + 1);
      if(!new_path)
        goto path_done;
      path = new_path;
    }

    path[path_len] = '/';
    memcpy(path + path_len + 1, filename, filename_len);
    path[new_filename_len] = '\0';
    filename_len = new_filename_len;
    filename = (const char*)path;
  }
#endif

  /* add URI-escaped filename length */
  for(from = filename, fl = filename_len; fl ; from++, fl--) {
    if(RAPTOR_SIZE_T_ADD_OVERFLOWS(len, 1))
      goto path_done;
    len++;
    c = (unsigned char)*from;
#ifdef WIN32
    if(c == ':') {
      if(fl < 2 || from[1] != '\\') {
        if(RAPTOR_SIZE_T_ADD_OVERFLOWS(len, 2))
          goto path_done;
        len += 2;
      }
      continue;
    }
    if(c == '\\')
      c = '/';
#endif
    if(raptor_uri_filename_char_needs_escape(c)) {
      if(RAPTOR_SIZE_T_ADD_OVERFLOWS(len, 2))
        goto path_done;
      len += 2; /* strlen(%xx)-1 */
    }
  }

  if(RAPTOR_SIZE_T_ADD_OVERFLOWS(len, 1))
    goto path_done;
  buffer = RAPTOR_MALLOC(unsigned char*, len + 1);
  if(!buffer)
    goto path_done;

  memcpy(buffer, "file://", RAPTOR_LEN_FILE_CSS + 1); /* copy NUL */
  from = filename;
  to = (char*)(buffer + RAPTOR_LEN_FILE_CSS);
  fl = filename_len;
#ifdef WIN32
  if(fl > 1 && *from == '\\' && from[1] == '\\') {
    from += 2; fl -= 2;
  } else
    *to++ ='/';
#endif
  for(; fl; fl--) {
    c = (unsigned char)*from++;
#ifdef WIN32
    if(c == '\\')
      c = '/';
    else if(c == ':') {
      *to++ = c;
      if(fl < 2 || *from != '\\') {
        *to++ ='.';
        *to++ ='/';
      }
      continue;
    }
#endif
    if(raptor_uri_filename_char_needs_escape(c)) {
      *to++ = '%';
      *to++ = hex_digits[c >> 4];
      *to++ = hex_digits[c & 0x0f];
    } else
      *to++ = c;
  }
  *to = '\0';

  path_done:
#ifndef WIN32
  /* Normalize the resulting URI path after the "file://"  */
  if(buffer)
    raptor_uri_normalize_path(buffer + RAPTOR_LEN_FILE_CSS,
                              len - RAPTOR_LEN_FILE_CSS);

  if(path)
    free(path);
#endif
  
  return buffer;
}


/**
 * raptor_uri_filename_to_uri_string:
 * @filename: The filename to convert
 *
 * Converts a filename to a file: URI.
 *
 * Handles the OS-specific escaping on turning filenames into URIs
 * and returns a new buffer that the caller must free().  Unsafe URI
 * characters in the filename are encoded as \%XX escapes.
 *
 * Return value: A newly allocated string with the URI or NULL on failure
 **/
unsigned char *
raptor_uri_filename_to_uri_string(const char *filename)

{
  return raptor_uri_counted_filename_to_uri_string(filename, 0);
}


/**
 * raptor_uri_uri_string_to_counted_filename_fragment:
 * @uri_string: The file: URI to convert
 * @len_p: address of filename length variable or NULL
 * @fragment_p: Address of pointer to store any URI fragment or NULL
 * @fragment_len_p: address of length variable or NULL
 *
 * Convert a file: URI to a counted filename and counted fragment.
 * 
 * Handles the OS-specific file: URIs to filename mappings.  Returns
 * a new buffer containing the filename that the caller must free.
 * Malformed percent escapes and percent-decoded NUL bytes are rejected.
 * On POSIX systems, non-local authorities such as file://host/path are
 * rejected; empty and localhost authorities are accepted as local.
 * POSIX paths are normalized after percent decoding.
 *
 * If @len_p is present the length of the filename is returned
 *
 * If @fragment_p is given, a new string containing the URI fragment
 * is returned, or NULL if none is present.  If @fragment_len_p is present
 * the length is returned in it.
 * 
 * Return value: A newly allocated string with the filename or NULL on failure
 **/
char *
raptor_uri_uri_string_to_counted_filename_fragment(const unsigned char *uri_string,
                                                   size_t* len_p,
                                                   unsigned char **fragment_p,
                                                   size_t* fragment_len_p) 
{
  char *filename;
  size_t len = 0;
  size_t path_len = 0;
  size_t prefix_len = 0;
  raptor_uri_detail *ud = NULL;
  const unsigned char *path_for_decode;
#ifdef WIN32
  unsigned char *from;
  char *to;
  unsigned char *p;
#endif

  if(!uri_string || !*uri_string)
    return NULL;
  
  ud = raptor_new_uri_detail(uri_string);
  if(!ud)
    return NULL;
  

  if(!ud->scheme || raptor_strcasecmp((const char*)ud->scheme, "file")) {
    raptor_free_uri_detail(ud);
    return NULL;
  }

  if(ud->authority) {
    if(!*ud->authority)
      ud->authority = NULL;
    else if(!raptor_strcasecmp((const char*)ud->authority, "localhost"))
      ud->authority = NULL;
  }

#ifndef WIN32
  /* POSIX file: URIs are local paths only (file:///path).  Reject any
   * remaining authority (e.g. file://remote.example/path) — unlike
   * Windows UNC file: URIs, there is no valid local filename. */
  if(ud->authority) {
    raptor_free_uri_detail(ud);
    return NULL;
  }
#endif

  /* Cannot do much if there is no path */
  if(!ud->path || (ud->path && !*ud->path)) {
    raptor_free_uri_detail(ud);
    return NULL;
  }

  /* See raptor_uri_filename_to_uri_string for details of the mapping */
#ifdef WIN32
  /* Windows: authority is a UNC server name (file://server/share/...). */
  if(ud->authority) {
    if(RAPTOR_SIZE_T_ADD_OVERFLOWS(ud->authority_len, 3)) {
      raptor_free_uri_detail(ud);
      return NULL;
    }
    prefix_len = ud->authority_len + 3;
  }

  p = ud->path;
  /* remove leading slash from path if there is one */
  if(*p && p[0] == '/')
    p++;
  /* handle case where path starts with drive letter */
  if(*p && (p[1] == '|' || p[1] == ':')) {
    /* Either 
     *   "a:" like in file://a|/... or file://a:/... 
     * or
     *   "a:." like in file://a:./foo
     * giving device-relative path a:foo
     */
    if(p[2] == '.') {
      p[2] = *p;
      p[3] = ':';
      p += 2;
    } else
      p[1] = ':';
  }
#endif

#ifdef WIN32
  /* Use path after Windows drive/leading-slash adjustments above. */
  path_for_decode = p;
#else
  path_for_decode = ud->path;
#endif

  path_len = raptor_uri_file_path_decoded_length(path_for_decode);
  if(!path_len) {
    raptor_free_uri_detail(ud);
    return NULL;
  }

  if(RAPTOR_SIZE_T_ADD_OVERFLOWS(prefix_len, path_len)) {
    raptor_free_uri_detail(ud);
    return NULL;
  }
  len = prefix_len + path_len;
  if(RAPTOR_SIZE_T_ADD_OVERFLOWS(len, 1)) {
    raptor_free_uri_detail(ud);
    return NULL;
  }

  filename = RAPTOR_MALLOC(char*, len + 1);
  if(!filename) {
    raptor_free_uri_detail(ud);
    return NULL;
  }

#ifdef WIN32
  /* Map file: URI to a Windows path (UNC prefix, backslashes, % decode). */
  to = filename;

  if(ud->authority) {
    *to++ = '\\';
    *to++ = '\\';
    memcpy(to, ud->authority, ud->authority_len);
    to += ud->authority_len;
    *to++ = '\\';
  }
  
  /* copy path after all /s */
  from = p;
  while(*from) {
    char c = *from++;
    if(c == '/')
      *to++ = '\\';
    else if(c == '%') {
      const unsigned char *pct = from - 1;
      int decoded = raptor_uri_percent_decode_byte(pct);
      if(decoded <= 0) {
        RAPTOR_FREE(char*, filename);
        raptor_free_uri_detail(ud);
        return NULL;
      }
      from = pct + 3;
      *to++ = (char)decoded;
    } else
      *to++ = c;
  }
  *to = '\0';
  len = (size_t)(to - filename);
#else
  /* POSIX: decode path and normalize (collapse .. etc.) to a local filename. */
  len = raptor_uri_copy_file_path_decoded(ud->path, filename);
  if(len == (size_t)-1) {
    RAPTOR_FREE(char*, filename);
    raptor_free_uri_detail(ud);
    return NULL;
  }
  len = raptor_uri_normalize_path((unsigned char*)filename, len);
#endif

  if(len_p)
    *len_p = len;

  if(fragment_p) {
    size_t fragment_len = 0;

    if(ud->fragment) {
      fragment_len = ud->fragment_len;
      if(RAPTOR_SIZE_T_ADD_OVERFLOWS(fragment_len, 1)) {
        RAPTOR_FREE(char*, filename);
        raptor_free_uri_detail(ud);
        return NULL;
      }
      *fragment_p = RAPTOR_MALLOC(unsigned char*, fragment_len + 1);
      if(!*fragment_p) {
        RAPTOR_FREE(char*, filename);
        raptor_free_uri_detail(ud);
        return NULL;
      }
      memcpy(*fragment_p, ud->fragment, fragment_len + 1);
    } else
      *fragment_p = NULL;
    if(fragment_len_p)
      *fragment_len_p = fragment_len;
  }

  raptor_free_uri_detail(ud);

  return filename;
}


/**
 * raptor_uri_uri_string_to_filename_fragment:
 * @uri_string: The file: URI to convert
 * @fragment_p: Address of pointer to store any URI fragment or NULL
 *
 * Convert a file: URI to a filename and fragment.
 * 
 * Handles the OS-specific file: URIs to filename mappings.  Returns
 * a new buffer containing the filename that the caller must free.
 *
 * If @fragment_p is given, a new string containing the URI fragment
 * is returned, or NULL if none is present
 * 
 * See also raptor_uri_uri_string_to_counted_filename_fragment()
 *
 * Return value: A newly allocated string with the filename or NULL on failure
 **/
char *
raptor_uri_uri_string_to_filename_fragment(const unsigned char *uri_string,
                                           unsigned char **fragment_p) 
{
  return raptor_uri_uri_string_to_counted_filename_fragment(uri_string, NULL,
                                                            fragment_p, NULL);
}


/**
 * raptor_uri_uri_string_to_filename:
 * @uri_string: The file: URI to convert
 *
 * Convert a file: URI to a filename.
 * 
 * Handles the OS-specific file: URIs to filename mappings.  Returns
 * a new buffer containing the filename that the caller must free.
 *
 * See also raptor_uri_uri_string_to_counted_filename_fragment()
 *
 * Return value: A newly allocated string with the filename or NULL on failure
 **/
char *
raptor_uri_uri_string_to_filename(const unsigned char *uri_string) 
{
  return raptor_uri_uri_string_to_counted_filename_fragment(uri_string, NULL,
                                                            NULL, NULL);
}



/**
 * raptor_uri_uri_string_is_file_uri:
 * @uri_string: The URI string to check
 *
 * Check if a URI string is a file: URI (case-insensitive "file:" prefix).
 *
 * If @uri_string is NULL or empty, returns 0.  Older releases incorrectly
 * returned non-zero for those inputs.
 *
 * Return value: non-zero if @uri_string is a file: URI, else 0
 **/
int
raptor_uri_uri_string_is_file_uri(const unsigned char* uri_string)
{
  if(!uri_string || !*uri_string)
    return 0;

  return raptor_strncasecmp((const char*)uri_string, "file:", 5) == 0;
}


/**
 * raptor_new_uri_for_xmlbase:
 * @old_uri: URI to transform
 *
 * Constructor - create a URI suitable for use as an XML Base.
 * 
 * Takes an existing URI and ensures it has a path (default /) and has
 * no fragment or query arguments - XML base does not use these.
 * 
 * Return value: new #raptor_uri object or NULL on failure.
 **/
raptor_uri*
raptor_new_uri_for_xmlbase(raptor_uri* old_uri)
{
  unsigned char *uri_string;
  unsigned char *new_uri_string;
  raptor_uri* new_uri;
  raptor_uri_detail *ud;
  
  if(!old_uri)
    return NULL;

  uri_string = raptor_uri_as_string(old_uri);

  ud = raptor_new_uri_detail(uri_string);
  if(!ud)
    return NULL;

  if(!ud->path) {
    ud->path = (unsigned char*)"/";
    ud->path_len = 1;
  }
  
  ud->query = NULL; ud->query_len = 0;
  ud->fragment = NULL; ud->fragment_len = 0;
  new_uri_string = raptor_uri_detail_to_string(ud, NULL);
  raptor_free_uri_detail(ud);
  if(!new_uri_string)
    return NULL;
  
  new_uri = raptor_new_uri(old_uri->world, new_uri_string);
  RAPTOR_FREE(char*, new_uri_string);

  return new_uri;
}


/**
 * raptor_new_uri_for_retrieval:
 * @old_uri: URI to transform
 *
 * Constructor - create a URI suitable for retrieval.
 * 
 * Takes an existing URI and ensures it has a path (default /) and has
 * no fragment - URI retrieval does not use the fragment part.
 *
 * Return value: new #raptor_uri object or NULL on failure.
 **/
raptor_uri*
raptor_new_uri_for_retrieval(raptor_uri* old_uri)
{
  unsigned char *uri_string;
  unsigned char *new_uri_string;
  raptor_uri* new_uri;
  raptor_uri_detail *ud;
  
  if(!old_uri)
    return NULL;

  uri_string = raptor_uri_as_string(old_uri);

  ud = raptor_new_uri_detail(uri_string);
  if(!ud)
    return NULL;

  if(!ud->path) {
    ud->path = (unsigned char*)"/";
    ud->path_len = 1;
  }

  ud->fragment = NULL; ud->fragment_len = 0;
  new_uri_string = raptor_uri_detail_to_string(ud, NULL);
  raptor_free_uri_detail(ud);
  if(!new_uri_string)
    return NULL;
  
  new_uri = raptor_new_uri(old_uri->world, new_uri_string);
  RAPTOR_FREE(char*, new_uri_string);

  return new_uri;
}


int
raptor_uri_init(raptor_world* world)
{
  if(world->uri_interning && !world->uris_tree) {
    world->uris_tree = raptor_new_avltree((raptor_data_compare_handler)raptor_uri_compare,
                                          /* free */ NULL, 0);
    if(!world->uris_tree) {
#ifdef RAPTOR_DEBUG
      RAPTOR_FATAL1("Failed to create raptor URI avltree");
#else
      raptor_log_error(world, RAPTOR_LOG_LEVEL_ERROR, NULL,
                       "Failed to create raptor URI avltree");
#endif
    }
    
  }

  return 0;
}


void
raptor_uri_finish(raptor_world* world)
{
  if(world->uris_tree) {
    raptor_free_avltree(world->uris_tree);
    world->uris_tree = NULL;
  }
}


/*
 * raptor_uri_path_common_base_length:
 * @first_path: The first path (path only, not a full URI)
 * @first_path_len: Length of first_path
 * @second_path: The second path (path only, not a full URI)
 * @second_path_len: Length of second_path
 *
 * Find the common base length of two URI path components.
 * 
 * Return value: Length of the common base path
 **/

static size_t
raptor_uri_path_common_base_length(const unsigned char *first_path,
                                   size_t first_path_len,
                                   const unsigned char *second_path,
                                   size_t second_path_len)
{
  ptrdiff_t common_len = 0;
  const unsigned char *cur_ptr = first_path;
  const unsigned char *prev_ptr = first_path;
  
  /* Compare each path component of first_path and second_path until
   * there is a mismatch. Then return the length from the start of
   * the path to the last successful match. 
   */
  while((cur_ptr = (const unsigned char*)memchr(cur_ptr, '/', first_path_len))) {
    size_t seg_len;

    cur_ptr++;
    seg_len = (size_t)(cur_ptr - prev_ptr);
    if((size_t)common_len + seg_len > second_path_len)
      break;
    if(strncmp((const char*)first_path + common_len,
               (const char*)second_path + common_len, seg_len))
      break;

    first_path_len -= cur_ptr - prev_ptr;
    prev_ptr = cur_ptr;
    common_len = prev_ptr - first_path;
  }

  return prev_ptr - first_path;
}


/*
 * raptor_uri_path_make_relative_path:
 * @from_path: The base path (path only, not a full URI)
 * @from_path_len: Length of the base path
 * @to_path: The reference path (path only, not a full URI)
 * @to_path_len: Length of the reference path
 * @suffix: String to be appended to the final relative path
 * @suffix_len: Length of the suffix
 * @result_length_p: Location to store the length of the string or NULL
 *
 * Make a relative URI path.
 *
 * Return value: A newly allocated relative path string or NULL on failure.
 **/

static unsigned char *
raptor_uri_path_make_relative_path(const unsigned char *from_path, size_t from_path_len,
                                   const unsigned char *to_path, size_t to_path_len,
                                   const unsigned char *suffix, size_t suffix_len,
                                   size_t *result_length_p)
{
  size_t common_len, cur_len, final_len, to_dir_len;
  size_t up_dirs = 0;
  size_t up_len;
  const unsigned char *cur_ptr, *prev_ptr;
  unsigned char *final_path, *final_path_cur;

  common_len = raptor_uri_path_common_base_length(from_path, from_path_len,
                                                  to_path, to_path_len);
  
  if(result_length_p)
    *result_length_p=0;

  /* Count how many directories we have to go up */
  cur_ptr = from_path + common_len;
  prev_ptr = cur_ptr;
  cur_len = from_path_len - common_len;
  while((cur_ptr = (const unsigned char*)memchr(cur_ptr, '/', cur_len))) {
    cur_ptr++;
    up_dirs++;
    cur_len -= cur_ptr - prev_ptr;
    prev_ptr = cur_ptr;
  }
  
  /* Calculate how many characters of to_path subdirs (counted from the 
     common base) we have to add. */
  cur_ptr = to_path + common_len;
  prev_ptr = cur_ptr;
  cur_len = to_path_len - common_len;
  while((cur_ptr = (const unsigned char*)memchr(cur_ptr, '/', cur_len))) {
    cur_ptr++;
    cur_len -= cur_ptr - prev_ptr;
    prev_ptr = cur_ptr;
  }
  to_dir_len = prev_ptr - (to_path + common_len);
  
  /* Create the final relative path */
  if(up_dirs > ((size_t)-1) / 3)
    return NULL;
  up_len = up_dirs * 3; /* 3 for each "../" */
  if(RAPTOR_SIZE_T_ADD_OVERFLOWS(up_len, to_dir_len))
    return NULL;
  final_len = up_len + to_dir_len;
  if(RAPTOR_SIZE_T_ADD_OVERFLOWS(final_len, suffix_len))
    return NULL;
  final_len += suffix_len;
  if(RAPTOR_SIZE_T_ADD_OVERFLOWS(final_len, 1))
    return NULL;
  final_path = RAPTOR_MALLOC(unsigned char*, final_len + 1);
  if(!final_path)
    return NULL;
  *final_path=0;
  
  /* First, add the necessary "../" parts */
  final_path_cur = final_path;
  while(up_dirs--) {
    *final_path_cur++='.';
    *final_path_cur++='.';
    *final_path_cur++='/';
  }
  
  /* Then, add the path from the common base to the to_path */
  memcpy(final_path_cur, to_path + common_len, to_dir_len);
  final_path_cur += to_dir_len;
  
  /* Finally, add the suffix */
  if(suffix && suffix_len) {
    /* As a special case, if the suffix begins with a dot (".") and the final 
       output string so far is non-empty, skip the dot. */
    if(*suffix == '.' && final_path_cur != final_path) {
      /* Make sure that the dot really represents a directory and it's not
         just part of a file name like ".foo". In other words, the dot must
         either be the only character or the next character must be the 
         fragment or the query character. */
      if((suffix_len == 1) || 
          (suffix_len > 1 && (suffix[1] == '#' || suffix[1] == '?'))) {
        suffix++;
        suffix_len--;
        final_len--;
      }
    }
    if(suffix_len)
      memcpy(final_path_cur, suffix, suffix_len);
  }
  
  final_path[final_len] = 0;
  
  if(result_length_p)
    *result_length_p=final_len;
  
  return final_path;
}


/**
 * raptor_uri_to_relative_counted_uri_string:
 * @base_uri: The base absolute URI to resolve against (or NULL)
 * @reference_uri: The reference absolute URI to use
 * @length_p: Location to store the length of the relative URI string or NULL
 *
 * Get the counted relative URI string of a URI against a base URI.
 * 
 * Return value: A newly allocated relative URI string or NULL on failure
 **/

unsigned char*
raptor_uri_to_relative_counted_uri_string(raptor_uri *base_uri, 
                                          raptor_uri *reference_uri,
                                          size_t *length_p) {
  raptor_uri_detail *base_detail = NULL, *reference_detail;
  const unsigned char *base, *reference_str, *base_file, *reference_file;
  unsigned char *suffix, *cur_ptr;
  size_t base_len, reference_len, reference_file_len, suffix_len;
  unsigned char *result = NULL;
  int suffix_is_result = 0;
  
  if(!reference_uri)
    return NULL;
    
  if(length_p)
    *length_p=0;

  reference_str = raptor_uri_as_counted_string(reference_uri, &reference_len);
  reference_detail = raptor_new_uri_detail(reference_str);
  if(!reference_detail)
    goto err;
  
  if(!base_uri)
    goto buildresult;
  
  base = raptor_uri_as_counted_string(base_uri, &base_len);
  base_detail = raptor_new_uri_detail(base);
  if(!base_detail)
    goto err;
  
  /* Check if the whole URIs are equal */
  if(raptor_uri_equals(base_uri, reference_uri)) {
    reference_len = 0;
    goto buildresult;
  }
  
  /* Check if scheme and authority of the URIs are equal */
  if(base_detail->scheme_len == reference_detail->scheme_len &&
     base_detail->authority_len == reference_detail->authority_len &&
     !strncmp((const char*)base_detail->scheme, 
              (const char*)reference_detail->scheme,
              base_detail->scheme_len) &&
     (base_detail->authority_len == 0 ||
        !strncmp((const char*)base_detail->authority,
              (const char*)reference_detail->authority,
              base_detail->authority_len))) {
    
    if(!base_detail->path) {
      if(reference_detail->path) {
        /* if base has no path then the relative URI is relative
         * to scheme+authority so assemble that in the suffix
         * buffer (adding any query part or fragment needed)
         */
        reference_file = reference_detail->path;
        reference_file_len = reference_detail->path_len;
        suffix_is_result = 1;
        goto addqueryfragment;
      }
      goto buildresult;
    }
    
    /* Find the file name components */
    base_file = (const unsigned char*)strrchr((const char*)base_detail->path, '/');
    if(!base_file)
      goto buildresult;
    base_file++;

    if(!reference_detail->path)
      goto buildresult;
    reference_file = (const unsigned char*)strrchr((const char*)reference_detail->path, '/');
    if(!reference_file)
      goto buildresult;
    reference_file++;
    
    reference_file_len = reference_detail->path_len -
                         (reference_file - reference_detail->path);
    
    if(!strcmp((const char*)base_detail->path, (const char*)reference_detail->path)) {
      /* If the file names are equal, don't put them in the relative URI */
      reference_file = NULL;
      reference_file_len = 0;
    } else if(*base_file && !*reference_file) {
      /* If the base file is non-empty, but the reference file is
       * empty, use "."  as the file name.
       */
      reference_file = (const unsigned char*)".";
      reference_file_len = 1;
    }

  addqueryfragment:
    /* Calculate the length of the suffix (file name + query + fragment) */
    suffix_len = reference_file_len;
    if(RAPTOR_SIZE_T_ADD_OVERFLOWS(suffix_len, reference_detail->query_len))
      goto err;
    suffix_len += reference_detail->query_len;
    if(RAPTOR_SIZE_T_ADD_OVERFLOWS(suffix_len, reference_detail->fragment_len))
      goto err;
    suffix_len += reference_detail->fragment_len;
    
    if(reference_detail->query) {
      if(RAPTOR_SIZE_T_ADD_OVERFLOWS(suffix_len, 1))
        goto err;
      suffix_len++; /* add one char for the '?' */
    }
    if(reference_detail->fragment) {
      if(RAPTOR_SIZE_T_ADD_OVERFLOWS(suffix_len, 1))
        goto err;
      suffix_len++; /* add one char for the '#' */
    }
    
    /* Assemble the suffix */
    if(RAPTOR_SIZE_T_ADD_OVERFLOWS(suffix_len, 1))
      goto err;
    suffix = RAPTOR_MALLOC(unsigned char*, suffix_len + 1);
    if(!suffix)
      goto err;
    cur_ptr = suffix;
    if(reference_file) {
      memcpy(suffix, reference_file, reference_file_len);
      cur_ptr+= reference_file_len;
    }

    if(reference_detail->query) {
      *cur_ptr++='?';
      memcpy(cur_ptr, reference_detail->query, reference_detail->query_len);
      cur_ptr+= reference_detail->query_len;
    }

    if(reference_detail->fragment) {
      *cur_ptr++='#';
      memcpy(cur_ptr, reference_detail->fragment, reference_detail->fragment_len);
      cur_ptr+= reference_detail->fragment_len;
    }
    *cur_ptr=0;
    
    if(suffix_is_result) {
      /* If suffix is what we need, just use that as the result */
      result = suffix;
      if(length_p)
        *length_p=suffix_len;
    } else {
      /* Otherwise create the full relative path */
      result = raptor_uri_path_make_relative_path(base_detail->path,
                                                  base_detail->path_len,
                                                  reference_detail->path,
                                                  reference_detail->path_len,
                                                  suffix,
                                                  suffix_len,
                                                  length_p);
      RAPTOR_FREE(char*, suffix);
    }
  }

  
 buildresult:
  /* If result is NULL at this point, it means that we were unable to find a
     relative URI, so we'll return a full absolute URI instead. */
  if(!result) {
    if(RAPTOR_SIZE_T_ADD_OVERFLOWS(reference_len, 1))
      goto err;
    result = RAPTOR_MALLOC(unsigned char*, reference_len + 1);
    if(result) {
      if(reference_len)
        memcpy(result, reference_str, reference_len);
      result[reference_len] = 0;
      if(length_p)
        *length_p=reference_len;
    }
  }
  
  err:
  if(base_detail)
    raptor_free_uri_detail(base_detail);
  raptor_free_uri_detail(reference_detail);
  
  return result;
}


/**
 * raptor_uri_to_relative_uri_string:
 * @base_uri: The base absolute URI to resolve against
 * @reference_uri: The reference absolute URI to use
 *
 * Get the relative URI string of a URI against a base URI.
 * 
 * Return value: A newly allocated relative URI string or NULL on failure
 **/
unsigned char*
raptor_uri_to_relative_uri_string(raptor_uri *base_uri, 
                                  raptor_uri *reference_uri)
{
  return raptor_uri_to_relative_counted_uri_string(base_uri, reference_uri,
                                                   NULL);
}


/**
 * raptor_uri_print:
 * @uri: URI to print
 * @stream: The file handle to print to
 *
 * Print a URI to a file handle.
 *
 * Return value: non-0 on failure
 **/
int
raptor_uri_print(const raptor_uri* uri, FILE *stream)
{
  size_t nwritten = 0;
  size_t len = 10;
  unsigned char *string = (unsigned char*)"(NULL URI)";
  raptor_world* world = NULL;
  
  if(uri) {
    world = uri->world;
    string = raptor_uri_as_counted_string((raptor_uri*)uri, &len);
  }

  nwritten = fwrite(string, 1, len, stream);
  if(nwritten != len)
    raptor_log_error_formatted(world, RAPTOR_LOG_LEVEL_ERROR,
                               NULL, "fwrite failed - %s", strerror(errno));

  return (nwritten == len);
}


/**
 * raptor_uri_to_counted_string:
 * @uri: #raptor_uri object
 * @len_p: Pointer to length (or NULL)
 *
 * Get a new counted string for a URI.
 *
 * If @len_p is not NULL, the length of the string is stored in it.
 *
 * The memory allocated must be freed by the caller and
 * raptor_free_memory() should be used for best portability.
 *
 * Return value: new string or NULL on failure
 **/
unsigned char*
raptor_uri_to_counted_string(raptor_uri *uri, size_t *len_p)
{
  size_t len;
  unsigned char *string;
  unsigned char *new_string;

  if(!uri)
    return NULL;
  
  string = raptor_uri_as_counted_string(uri, &len);
  if(!string)
    return NULL;
  
  if(RAPTOR_SIZE_T_ADD_OVERFLOWS(len, 1))
    return NULL;
  new_string = RAPTOR_MALLOC(unsigned char*, len + 1); /* +1 for NULL termination */
  if(!new_string)
    return NULL;
  
  memcpy(new_string, string, len+1);

  if(len_p)
    *len_p=len;
  return new_string;
}


/**
 * raptor_uri_to_string:
 * @uri: #raptor_uri object
 *
 * Get a new string for a URI.
 *
 * The memory allocated must be freed by the caller and
 * raptor_free_memory() should be used for best portability.
 *
 * Return value: new string or NULL on failure
 **/
unsigned char*
raptor_uri_to_string(raptor_uri *uri)
{
  return raptor_uri_to_counted_string(uri, NULL);
}


/**
 * raptor_new_uri_from_rdf_ordinal:
 * @world: raptor_world object
 * @ordinal: integer rdf:_n
 * 
 * Internal - convert an integer rdf:_n ordinal to the resource URI
 * 
 * Return value: new URI object or NULL on failure
 **/
raptor_uri*
raptor_new_uri_from_rdf_ordinal(raptor_world* world, int ordinal)
{
  /* strlen(rdf namespace URI) + _ + decimal int number + \0 */
  unsigned char uri_string[43 + 1 + MAX_ASCII_INT_SIZE + 1];
  unsigned char *p = uri_string;

  memcpy(p, raptor_rdf_namespace_uri, raptor_rdf_namespace_uri_len);
  p += raptor_rdf_namespace_uri_len;
  *p++ = '_';
  (void)raptor_format_integer(RAPTOR_GOOD_CAST(char*, p),
                              MAX_ASCII_INT_SIZE + 1, ordinal, /* base */ 10,
                              -1, '\0');

  return raptor_new_uri(world, uri_string);
}


/**
 * raptor_uri_get_world:
 * @uri: #raptor_uri object
 *
 * Get the raptor_world object associated with a raptor_uri.
 *
 * Return value: raptor_world object
 **/
raptor_world*
raptor_uri_get_world(raptor_uri *uri)
{
  return uri->world;
}


/**
 * raptor_uri_filename_exists:
 * @path: file path
 *
 * Check if @path points to a file that exists
 *
 * Return value: > 0 if file exists, 0 if does not exist, < 0 on error
 **/
int
raptor_uri_filename_exists(const unsigned char* path)
{
  int exists = -1;
#ifdef HAVE_STAT
  struct stat stat_buffer;
#endif
  
  if(!path)
    return -1;

#ifdef HAVE_STAT
  if(!stat((const char*)path, &stat_buffer))
    exists = S_ISREG(stat_buffer.st_mode);
#else
  exists = (access(path, R_OK) < 0) ? -1 : 1;
#endif

  return exists;
}


/**
 * raptor_uri_file_exists:
 * @uri: URI string
 *
 * Check if a file: URI is a file that exists
 *
 * Return value: > 0 if file exists, 0 if does not exist, < 0 if not a file URI or error
 **/
int
raptor_uri_file_exists(raptor_uri* uri)
{
  const unsigned char* uri_string;
  char *filename;
  int exists;

  if(!uri)
    return -1;

  uri_string = raptor_uri_as_string(uri);
  if(!raptor_uri_uri_string_is_file_uri(uri_string))
    return -1;

  filename = raptor_uri_uri_string_to_filename(uri_string);
  if(!filename)
    return -1;

  exists = raptor_uri_filename_exists((const unsigned char*)filename);
  RAPTOR_FREE(char*, filename);

  return exists;
}



/**
 * raptor_uri_escaped_write:
 * @uri: uri to write
 * @base_uri: base uri to write relative to (or NULL)
 * @flags: bit flags - see #raptor_escaped_write_bitflags
 * @iostr: raptor iostream
 * 
 * Write a #raptor_uri formatted with escapes to a #raptor_iostream
 * 
 * Return value: non-0 on failure
 **/
int
raptor_uri_escaped_write(raptor_uri* uri,
                         raptor_uri* base_uri,
                         unsigned int flags,
                         raptor_iostream *iostr)
{
  unsigned char *uri_str;
  int uri_str_owned = 0;
  size_t len;

  if(!uri)
    return 1;
  
  raptor_iostream_write_byte('<', iostr);
  if(base_uri) {
    uri_str = raptor_uri_to_relative_counted_uri_string(base_uri, uri, &len);
    if(!uri_str)
      return 1;

    uri_str_owned = 1;
  } else {
    uri_str = raptor_uri_as_counted_string(uri, &len);
  }
  if(uri_str)
    raptor_string_escaped_write(uri_str, len, '>', flags, iostr);
  raptor_iostream_write_byte('>', iostr);

  if(uri_str_owned && uri_str)
    RAPTOR_FREE(char*, uri_str);

  return 0;
}


/**
 * raptor_uri_uri_string_is_absolute:
 * @uri_string: URI string to check
 * 
 * Check if a URI string is an absolute URI using the ASCII URI scheme
 * grammar.
 * 
 * Return value: >0 if absolute, 0 if not, < 0 on failure
 **/
int
raptor_uri_uri_string_is_absolute(const unsigned char* uri_string)
{
  const unsigned char* s = uri_string;

  if(!uri_string)
    return -1;
  
  /* 
   * scheme = alpha *( alpha | digit | "+" | "-" | "." )
   *    RFC 2396 section 3.1 Scheme Component
   */
  if(*s && raptor_uri_ascii_scheme_start(*s)) {
    s++;

    while(*s && raptor_uri_ascii_scheme_char(*s))
      s++;
  
    if(*s == ':')
      return 1;
  }


  return 0;
}


#endif /* !STANDALONE */


#ifdef STANDALONE

#include <stdio.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

/* one more prototype */
int main(int argc, char *argv[]);

static const char *program;


static int
assert_uri_is_valid(raptor_uri* uri)
{
  if(strlen((const char*)uri->string) != uri->length) {
    fprintf(stderr,
            "%s: URI with string '%s' is invalid. length is %d, recorded in object as %d\n",
            program, uri->string,
            (int)strlen((const char*)uri->string),
            (int)uri->length);
    return 0;
  }

  return 1;
}


static int
assert_filename_to_uri (const char *filename, const char *reference_uri)
{
  unsigned char *uri;

  uri = raptor_uri_filename_to_uri_string(filename);

  if(!uri || strcmp((const char*)uri, (const char*)reference_uri)) {
    fprintf(stderr, 
            "%s: raptor_uri_filename_to_uri_string(%s) FAILED gaving URI %s != %s\n",
            program, filename, uri, reference_uri);
    if(uri)
      RAPTOR_FREE(char*, uri);
    return 1;
  }

  RAPTOR_FREE(char*, uri);
  return 0;
}


static int
assert_uri_to_filename (const char *uri, const char *reference_filename)
{
  char *filename;

  filename = raptor_uri_uri_string_to_filename((const unsigned char*)uri);

  if(!filename && reference_filename) {
    fprintf(stderr,
            "%s: raptor_uri_uri_string_to_filename(%s) FAILED giving NULL expected '%s'\n",
            program, uri, reference_filename);
    return 1;
  }
  if(filename && !reference_filename) {
    fprintf(stderr, 
            "%s: raptor_uri_uri_string_to_filename(%s) FAILED giving filename %s != NULL\n", 
            program, uri, filename);
    if(filename)
      RAPTOR_FREE(char*, filename);
    return 1;
  } else if(filename && strcmp(filename, reference_filename)) {
    fprintf(stderr,
            "%s: raptor_uri_uri_string_to_filename(%s) FAILED gaving filename %s != %s\n",
            program, uri, filename, reference_filename);
    if(filename)
      RAPTOR_FREE(char*, filename);
    return 1;
  }

  RAPTOR_FREE(char*, filename);
  return 0;
}


static int
assert_uri_to_relative(raptor_world *world, const char *base, const char *uri, const char *relative)
{
  unsigned char *output;
  int result;
  raptor_uri* base_uri = NULL;
  raptor_uri* reference_uri = raptor_new_uri(world, (const unsigned char*)uri);
  size_t length = 0;

  if(!assert_uri_is_valid(reference_uri))
    return 1;

  if(base) {
    base_uri = raptor_new_uri(world, (const unsigned char*)base);
    if(base_uri && !assert_uri_is_valid(base_uri)) {
      raptor_free_uri(reference_uri);
      raptor_free_uri(base_uri);
      return 1;
    }
  }
  
  output = raptor_uri_to_relative_counted_uri_string(base_uri, reference_uri, 
                                                     &length);
  result = strcmp(relative, (const char*)output);
  if(result) {
    fprintf(stderr,
            "%s: raptor_uri_string_to_relative_uri_string FAILED: base='%s', uri='%s', expected='%s', got='%s'\n",
            program, base, uri, relative, output);
    RAPTOR_FREE(char*, output);
    return 1;
  }
  RAPTOR_FREE(char*, output);
  if(base_uri)
    raptor_free_uri(base_uri);
  raptor_free_uri(reference_uri);
  return 0;
}


int
main(int argc, char *argv[]) 
{
  raptor_world *world;
  const char *base_uri = "http://example.org/bpath/cpath/d;p?querystr#frag";
  const char *base_uri_xmlbase = "http://example.org/bpath/cpath/d;p";
  const char *base_uri_retrievable = "http://example.org/bpath/cpath/d;p?querystr";
#if !defined(WIN32) && !defined(WIN32_URI_TEST)
#if defined(HAVE_UNISTD_H) && defined(HAVE_SYS_STAT_H)
  const char* dirs[6] = { "/etc", "/bin", "/tmp", "/lib", "/var", NULL };
  #define URI_BUFFER_LEN 16
  unsigned char uri_buffer[URI_BUFFER_LEN]; /* strlen("file:///DIR/foo")+1 */  
  int i;
  const char *dir;
#endif
#endif
  unsigned char *str;
  raptor_uri *uri1, *uri2, *uri3;
  const unsigned char embedded_nul_uri[] = {
    'h', 't', 't', 'p', ':', 'a', '\0', 'b'
  };
  size_t uri_length;

  int failures = 0;

  world = raptor_new_world();
  if(!world || raptor_world_open(world))
    exit(1);

  if((program = strrchr(argv[0], '/')))
    program++;
  else if((program = strrchr(argv[0], '\\')))
    program++;
  else
    program = argv[0];

  uri1 = raptor_new_uri_from_counted_string(world, embedded_nul_uri,
                                            sizeof(embedded_nul_uri));
  if(uri1) {
    fprintf(stderr,
            "%s: raptor_new_uri_from_counted_string accepted embedded NUL\n",
            program);
    raptor_free_uri(uri1);
    failures++;
  }

  /* Security: counted-string constructor must reject length == 0 */
  uri1 = raptor_new_uri_from_counted_string(world,
                                            (const unsigned char*)"x", 0);
  if(uri1) {
    fprintf(stderr,
            "%s: raptor_new_uri_from_counted_string accepted length == 0\n",
            program);
    raptor_free_uri(uri1);
    failures++;
  }

  /* Security: counted-string constructor must reject length > UINT_MAX.
   * Only meaningful when size_t is wider than unsigned int. */
  if(sizeof(size_t) > sizeof(unsigned int)) {
    size_t huge = (size_t)UINT_MAX + 1;
    uri1 = raptor_new_uri_from_counted_string(world,
                                              (const unsigned char*)"x", huge);
    if(uri1) {
      fprintf(stderr,
              "%s: raptor_new_uri_from_counted_string accepted length > UINT_MAX\n",
              program);
      raptor_free_uri(uri1);
      failures++;
    }
  }


  /* Platform file: URI tests.  raptor_uri_win32_test is built with
   * -DWIN32_URI_TEST; Windows file URI vectors run only when WIN32 is
   * defined (native Windows build).  On other hosts that target skips. */
#if defined(WIN32) || defined(WIN32_URI_TEST)
#if defined(WIN32)
  /* Windows file: URI mapping (drive letters, UNC authority, backslashes). */
  failures += assert_filename_to_uri ("c:\\windows\\system", "file:///c:/windows/system");
  failures += assert_filename_to_uri ("\\\\server\\share\\file.doc", "file://server/share/file.doc");
  failures += assert_filename_to_uri ("a:foo", "file:///a:./foo");

  failures += assert_filename_to_uri ("C:\\Documents and Settings\\myapp\\foo.bat", "file:///C:/Documents%20and%20Settings/myapp/foo.bat");
  failures += assert_filename_to_uri ("C:\\My Documents\\%age.txt", "file:///C:/My%20Documents/%25age.txt");

  failures += assert_uri_to_filename ("file:///c|/windows/system", "c:\\windows\\system");
  failures += assert_uri_to_filename ("file:///c:/windows/system", "c:\\windows\\system");
  failures += assert_uri_to_filename ("file://server/share/file.doc", "\\\\server\\share\\file.doc");
  failures += assert_uri_to_filename ("file:///a:./foo", "a:foo");
  failures += assert_uri_to_filename ("file:///C:/Documents%20and%20Settings/myapp/foo.bat", "C:\\Documents and Settings\\myapp\\foo.bat");
  failures += assert_uri_to_filename ("file:///C:/My%20Documents/%25age.txt", "C:\\My Documents\\%age.txt");


  failures += assert_uri_to_filename ("file:c:\\thing",     "c:\\thing");
  failures += assert_uri_to_filename ("file:/c:\\thing",    "c:\\thing");
  failures += assert_uri_to_filename ("file://c:\\thing",   NULL);
  failures += assert_uri_to_filename ("file:///c:\\thing",  "c:\\thing");
  failures += assert_uri_to_filename ("file://localhost/",  NULL);
  failures += assert_uri_to_filename ("file://c:\\foo\\bar\\x.rdf",  NULL);

  /* Authority is a UNC server name; POSIX rejects non-local authority. */
  failures += assert_uri_to_filename("file://remote.example/etc/hosts",
                                     "\\\\remote.example\\etc\\hosts");

#else /* WIN32_URI_TEST on a non-Windows build */
  fprintf(stderr,
          "%s: Skipping Windows file URI tests (library not built with WIN32)\n",
          program);
  /* Silence -Wunused-function on this build configuration: the helper is
   * only invoked from the WIN32 branch above. */
  (void)assert_filename_to_uri;
#endif

#else /* POSIX - raptor_uri_test on non-Windows */
  /* POSIX file: URIs are local paths only (file:///path). */

  failures += assert_filename_to_uri ("/path/to/file", "file:///path/to/file");
  failures += assert_filename_to_uri ("/path/to/file with spaces", "file:///path/to/file%20with%20spaces");
  failures += assert_filename_to_uri ("/path/to/a#b?c", "file:///path/to/a%23b%3Fc");
  failures += assert_filename_to_uri ("/path/to/a\\b", "file:///path/to/a%5Cb");
  failures += assert_uri_to_filename ("file:///path/to/file", "/path/to/file");
  failures += assert_uri_to_filename ("file:///path/to/file%20with%20spaces", "/path/to/file with spaces");

  /* Tests for Issue#0000268 http://bugs.librdf.org/mantis/view.php?id = 268 */
  failures += assert_uri_to_filename ("file:///path/to/http%253A%252F%252Fwww.example.org%252Fa%252Fb%252Fc", "/path/to/http%3A%2F%2Fwww.example.org%2Fa%2Fb%2Fc");
  failures += assert_filename_to_uri ("/path/to/http%3A%2F%2Fwww.example.org%2Fa%2Fb%2Fc", "file:///path/to/http%253A%252F%252Fwww.example.org%252Fa%252Fb%252Fc");

  /* Security: normalize path traversal on file: URI decode */
  failures += assert_uri_to_filename("file:///tmp/foo/../../../etc/hosts",
                                     "/etc/hosts");
  failures += assert_uri_to_filename("file:///tmp/foo/%2e%2e/%2e%2e/etc/hosts",
                                     "/etc/hosts");
  failures += assert_uri_to_filename("file:///var/tmp/a/../../../etc/hosts",
                                     "/etc/hosts");

  /* Security: reject non-local file: authority on POSIX */
  failures += assert_uri_to_filename("file://remote.example/etc/hosts", NULL);

#if defined(HAVE_UNISTD_H) && defined(HAVE_SYS_STAT_H)
  /* Need to test this with a real dir (preferably not /)
   * This is just a test so pretty likely to work on all development systems
   * that are not WIN32
   */

  for(i = 0; (dir = dirs[i]); i++) {
    struct stat buf;
    if(!lstat(dir, &buf) && S_ISDIR(buf.st_mode) && !S_ISLNK(buf.st_mode)) {
      if(!chdir(dir))
        break;
    }
  }
  if(!dir)
    fprintf(stderr,
            "%s: WARNING: Found no convenient directory - not testing relative files\n",
            program);
  else {
    snprintf((char*)uri_buffer, URI_BUFFER_LEN, "file://%s/foo", dir);
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    fprintf(stderr,
            "%s: Checking relative file name 'foo' in dir %s expecting URI %s\n",
            program, dir, uri_buffer);
#endif
    failures += assert_filename_to_uri ("foo", (const char*)uri_buffer);
  }
#endif
 
#endif

  /* Security: malformed percent escape (both platforms) */
  failures += assert_uri_to_filename("file:///tmp/foo%", NULL);
  failures += assert_uri_to_filename("file:///tmp/%0", NULL);
  failures += assert_uri_to_filename("file:///tmp/%00", NULL);
  failures += assert_uri_to_filename("file:///tmp/%zz", NULL);

  /* Security: is_file_uri must not claim NULL/empty are file URIs */
  if(raptor_uri_uri_string_is_file_uri(NULL) ||
     raptor_uri_uri_string_is_file_uri((const unsigned char*)"")) {
    fprintf(stderr, "%s: raptor_uri_uri_string_is_file_uri NULL/empty should be 0\n",
            program);
    failures++;
  }

  uri1 = raptor_new_uri(world, (const unsigned char*)base_uri);

  str = raptor_uri_as_string(uri1);
  if(strcmp((const char*)str, base_uri)) {
    fprintf(stderr,
            "%s: raptor_uri_as_string(%s) FAILED gaving %s != %s\n",
            program, base_uri, str, base_uri);
    failures++;
  }
  
  uri2 = raptor_new_uri_for_xmlbase(uri1);
  str = raptor_uri_as_string(uri2);
  if(strcmp((const char*)str, base_uri_xmlbase)) {
    fprintf(stderr, 
            "%s: raptor_new_uri_for_xmlbase(URI %s) FAILED giving %s != %s\n",
            program, base_uri, str, base_uri_xmlbase);
    failures++;
  }
  
  uri3 = raptor_new_uri_for_retrieval(uri1);

  str = raptor_uri_as_string(uri3);
  if(strcmp((const char*)str, base_uri_retrievable)) {
    fprintf(stderr,
            "%s: raptor_new_uri_for_retrievable(%s) FAILED gaving %s != %s\n",
            program, base_uri, str, base_uri_retrievable);
    failures++;
  }
  
  raptor_free_uri(uri3);
  raptor_free_uri(uri2);
  raptor_free_uri(uri1);

  uri1 = raptor_new_uri(world, (const unsigned char*)"http://example.org/ns#");
  uri2 = raptor_new_uri_from_uri_local_name(world, uri1,
                                            (const unsigned char*)"term");
  if(!uri2) {
    fprintf(stderr, "%s: raptor_new_uri_from_uri_local_name returned NULL\n",
            program);
    failures++;
  } else {
    str = raptor_uri_as_counted_string(uri2, &uri_length);
    if(uri_length != 26 || strcmp((const char*)str, "http://example.org/ns#term")) {
      fprintf(stderr,
              "%s: raptor_new_uri_from_uri_local_name length/string mismatch\n",
              program);
      failures++;
    }
    raptor_free_uri(uri2);
  }
  uri2 = raptor_new_uri_from_uri_local_name(world, uri1, NULL);
  if(uri2) {
    fprintf(stderr,
            "%s: raptor_new_uri_from_uri_local_name accepted NULL local_name\n",
            program);
    raptor_free_uri(uri2);
    failures++;
  }
  raptor_free_uri(uri1);
  
  failures += assert_uri_to_relative(world, NULL, "http://example.com/foo/bar", "http://example.com/foo/bar");
  failures += assert_uri_to_relative(world, "", "http://example.com/foo/bar", "http://example.com/foo/bar");
  failures += assert_uri_to_relative(world, "foo:", "http://example.com/foo/bar", "http://example.com/foo/bar");
  failures += assert_uri_to_relative(world, "http://example.com/base/foo?foo#foo", "http://example.com/base/bar?bar#bar", "bar?bar#bar");
  failures += assert_uri_to_relative(world, "http://example.com/base/foo", "http://example.com/base/foo/", "foo/");
  failures += assert_uri_to_relative(world, "http://example.com/base/foo", "http://example.com/base/foo/.foo", "foo/.foo");
  failures += assert_uri_to_relative(world, "http://example.com/base/foo", "http://example.com/base/foo/.foo#bar", "foo/.foo#bar");
  failures += assert_uri_to_relative(world, "http://example.com/base/foo", "http://example.com/base/foo/bar", "foo/bar");
  failures += assert_uri_to_relative(world, "http://example.com/base/foo", "http://example.com/base/foo#bar", "#bar");
  failures += assert_uri_to_relative(world, "http://example.com/base/foo", "http://example.com/base/bar#foo", "bar#foo");
  failures += assert_uri_to_relative(world, "http://example.com/base/foo", "http://example.com/otherbase/foo", "../otherbase/foo");
  failures += assert_uri_to_relative(world, "http://example.com/base/foo", "http://example.com/otherbase/bar", "../otherbase/bar");
  failures += assert_uri_to_relative(world, "http://example.com/base/foo", "http://example.com/base/#foo", ".#foo");
  failures += assert_uri_to_relative(world, "http://example.com/base/foo", "http://example2.com/base/bar", "http://example2.com/base/bar");
  failures += assert_uri_to_relative(world, "http://example.com/base/one?path=/should/be/ignored", "http://example.com/base/two?path=/should/be/ignored", "two?path=/should/be/ignored");
  failures += assert_uri_to_relative(world, "http://example.org/base#", "http://www.foo.org", "http://www.foo.org");
  failures += assert_uri_to_relative(world, "http://example.org", "http://a.example.org/", "http://a.example.org/");
  failures += assert_uri_to_relative(world, "http://example.org", "http://a.example.org", "http://a.example.org");
  failures += assert_uri_to_relative(world, "http://abcdefgh.example.org/foo/bar/", "http://ijklmnop.example.org/", "http://ijklmnop.example.org/");
  failures += assert_uri_to_relative(world, "http://example.org", "http://example.org/a/b/c/d/efgh", "/a/b/c/d/efgh");

  if(1) {
    int ret;
    raptor_uri* u1;
    raptor_uri* u2;
    
    u1 = raptor_new_uri(world, (const unsigned char *)"http://example.org/abc");
    u2 = raptor_new_uri(world, (const unsigned char *)"http://example.org/def");

    ret = raptor_uri_compare(u1, u2);
    if(!(ret < 0)) {
      fprintf(stderr,
              "%s: raptor_uri_compare(%s, %s) FAILED gave %d expected <0\n",
              program, raptor_uri_as_string(u1), raptor_uri_as_string(u2),
              ret);
      failures++;
    }

    raptor_free_uri(u1);
    raptor_free_uri(u2);
  }

  raptor_free_world(world);

  return failures ;
}

#endif /* STANDALONE */
