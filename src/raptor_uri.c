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

/**
 * raptor_new_uri_from_counted_string:
 * @world: raptor_world object
 * @uri_string: URI string.
 * @length: length of URI string
 * 
 * Constructor - create a raptor URI from a UTF-8 encoded Unicode string.
 * 
 * Note: The @uri_string need not be NULL terminated - a NULL will be
 * added to the copied string used.
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

  if(!uri_string || !*uri_string)
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
  fputs(RAPTOR_DEBUG_FH, "' in hash\n")
#endif

  new_uri = RAPTOR_CALLOC(raptor_uri*, 1, sizeof(*new_uri));
  if(!new_uri)
    goto unlock;

  new_uri->world = world;
  new_uri->length = (unsigned int)length;

  new_string = RAPTOR_MALLOC(unsigned char*, length + 1);
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
 * @local_name: local name
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
  size_t len;
  unsigned char *new_string;
  raptor_uri* new_uri;
  size_t local_name_length;

  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);

  if(!uri)
    return NULL;
  
  raptor_world_open(world);

  local_name_length = strlen((const char*)local_name);
  
  len = uri->length + local_name_length;
  new_string = RAPTOR_MALLOC(unsigned char*, len + 1);
  if(!new_string)
    return NULL;

  memcpy((char*)new_string, (const char*)uri->string, uri->length);
  memcpy((char*)(new_string + uri->length), (const char*)local_name,
         local_name_length + 1);

  new_uri = raptor_new_uri_from_counted_string(world, new_string, len);
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
  
  /* +1 for adding any missing URI path '/' */
  buffer_length = base_uri->length + uri_len + 1;
  buffer = RAPTOR_MALLOC(unsigned char*, buffer_length + 1);
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
  new_uri_string_len = base_uri_string_len + name_len;
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
 * Return value: a new #raptor_uri object or NULL on failure
 **/
raptor_uri*
raptor_new_uri_from_uri_or_file_string(raptor_world* world,
                                       raptor_uri* base_uri,
                                       const unsigned char* uri_or_file_string)
{
  raptor_uri* new_uri = NULL;
  const unsigned char* new_uri_string;
  const char* path;

  if(raptor_uri_filename_exists(uri_or_file_string) > 0) {
    /* uri_or_file_string is a file name, not a file: URI */
    path = RAPTOR_GOOD_CAST(const char*, uri_or_file_string);
  } else {
    new_uri = raptor_new_uri_relative_to_base(world, base_uri,
                                            uri_or_file_string);
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
 * and returns a new buffer that the caller must free().  Turns a
 * space in the filename into \%20 and '%' into \%25.
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
  size_t len = RAPTOR_LEN_FILE_CSS;
  size_t fl;

  if(!filename)
    return NULL;

  if(!filename_len)
    filename_len = strlen(filename);
  
#ifdef WIN32
/*
 * On WIN32, filenames turn into
 *   "file://" + translated filename
 * where the translation is \\ turns into / and ' ' into %20, '%' into %25
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
  if(filename[1] == ':' && filename[2] != '\\')
    len += 3; /* relative filename - add / and ./ */
  else if(*filename == '\\')
    len -= 2; /* two // from not needed in filename */
  else
    len++; /* / at start of path */

#else
/* others - unix: turn spaces into %20, '%' into %25 */

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
    while(1) {
      /* malloc() failed or getcwd() succeeded */
      errno = 0;
      if(!path || getcwd(path, path_max))
        break;

      /* failed */
      if(errno != ERANGE)
        break;

      /* try again with a bigger buffer */
      path_max *= 2;
      path = (char*)realloc(path, path_max);
    }
    if(!path)
      goto path_done;
    path_len = strlen(path);

    /* path + '/' + filename */
    new_filename_len = path_len + 1 + filename_len;
    if(path_max < new_filename_len + 1) {
      path = (char*)realloc(path, new_filename_len + 1);
      if(!path)
        goto path_done;
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
    len++;
#ifdef WIN32
    if(*from == ':') {
      if(from[1] != '\\')
        len += 2;
    }
#endif
    if(*from == ' ' || *from == '%')
      len += 2; /* strlen(%xx)-1 */
  }

  buffer = RAPTOR_MALLOC(unsigned char*, len + 1);
  if(!buffer)
    goto path_done;

  memcpy(buffer, "file://", 7);
  from = filename;
  to = (char*)(buffer+7);
  fl = filename_len;
#ifdef WIN32
  if(*from == '\\' && from[1] == '\\') {
    from += 2; fl -= 2;
  } else
    *to++ ='/';
#endif
  for(; fl; fl--) {
    char c = *from++;
#ifdef WIN32
    if(c == '\\')
      *to++ ='/';
    else if(c == ':') {
      *to++ = c;
      if(*from != '\\') {
        *to++ ='.';
        *to++ ='/';
      }
    } else
#endif
    if(c == ' ' || c == '%') {
      *to++ = '%';
      *to++ = '2';
      *to++ = (c == ' ') ? '0' : '5';
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
 * and returns a new buffer that the caller must free().  Turns a
 * space in the filename into \%20 and '%' into \%25.
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
  raptor_uri_detail *ud = NULL;
  unsigned char *from;
  char *to;
#ifdef WIN32
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

  /* Cannot do much if there is no path */
  if(!ud->path || (ud->path && !*ud->path)) {
    raptor_free_uri_detail(ud);
    return NULL;
  }

  /* See raptor_uri_filename_to_uri_string for details of the mapping */
#ifdef WIN32
  if(ud->authority)
    len += ud->authority_len+3;

  p = ud->path;
  /* remove leading slash from path if there is one */
  if(*p && p[0] == '/') {
    p++;
    len--;
  }
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
      len -= 2; /* remove 2 for ./ */
    } else
      p[1] = ':';
  }
#endif


  /* add URI-escaped filename length */
  for(from = ud->path; *from ; from++) {
    len++;
    if(*from == '%')
      from += 2;
  }


  /* Something is wrong */
  if(!len) {
    raptor_free_uri_detail(ud);
    return NULL;
  }
    
  filename = RAPTOR_MALLOC(char*, len + 1);
  if(!filename) {
    raptor_free_uri_detail(ud);
    return NULL;
  }

  to = filename;

#ifdef WIN32
  if(ud->authority) {
    *to++ = '\\';
    *to++ = '\\';
    from = ud->authority;
    while( (*to++ = *from++) )
      ;
    to--;
    *to++ = '\\';
  }
  
  /* copy path after all /s */
  from = p;
#else
  from = ud->path;
#endif

  while(*from) {
    char c = *from++;
#ifdef WIN32
    if(c == '/')
      *to++ = '\\';
    else
#endif
    if(c == '%') {
      if(*from && from[1]) {
        char hexbuf[3];
        char *endptr = NULL;
        hexbuf[0] = (char)*from;
        hexbuf[1] = (char)from[1];
        hexbuf[2]='\0';
        c = (char)strtol((const char*)hexbuf, &endptr, 16);
        if(endptr == &hexbuf[2])
          *to++ = c;
      }
      from += 2;
    } else
      *to++ = c;
  }
  *to = '\0';

  if(len_p)
    *len_p = len;

  if(fragment_p) {
    size_t fragment_len = 0;

    if(ud->fragment) {
      fragment_len = ud->fragment_len;
      *fragment_p = RAPTOR_MALLOC(unsigned char*, fragment_len + 1);
      if(*fragment_p)
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
 * Check if a URI string is a file: URI.
 * 
 * Return value: Non zero if URI string is a file: URI
 **/
int
raptor_uri_uri_string_is_file_uri(const unsigned char* uri_string)
{
  if(!uri_string || !*uri_string)
    return 1;

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
    cur_ptr++;
    if(strncmp((const char*)first_path + common_len,
               (const char*)second_path + common_len, cur_ptr - prev_ptr))
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
  int up_dirs = 0;
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
  final_len = up_dirs*3 + to_dir_len + suffix_len; /* 3 for each "../" */
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
     !strncmp((const char*)base_detail->authority, 
              (const char*)reference_detail->authority,
              base_detail->authority_len)) {
    
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
    suffix_len = reference_file_len + reference_detail->query_len + 
                 reference_detail->fragment_len;
    
    if(reference_detail->query)
      suffix_len++; /* add one char for the '?' */
    if(reference_detail->fragment)
      suffix_len++; /* add one char for the '#' */
    
    /* Assemble the suffix */
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
  
  if(!uri)
    return -1;

  uri_string = raptor_uri_as_string(uri);
  if(!raptor_uri_uri_string_is_file_uri(uri_string))
    return -1;

  return raptor_uri_filename_exists(uri_string + 6);
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
 * @uri_string: uri to check write
 * 
 * Check if a uri string is an absolute URI
 * 
 * Return value: >0 if absolute, 0 if not, < 0 on failure
 **/
int
raptor_uri_uri_string_is_absolute(const unsigned char* uri_string)
{
  const unsigned char* s = uri_string;
  
  /* 
   * scheme = alpha *( alpha | digit | "+" | "-" | "." )
   *    RFC 2396 section 3.1 Scheme Component
   */
  if(*s && isalpha((int)*s)) {
    s++;

    while(*s && (isalnum((int)*s) ||
                 (*s == '+') || (*s == '-') || (*s == '.')))
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
#ifndef WIN32
#if defined(HAVE_UNISTD_H) && defined(HAVE_SYS_STAT_H)
  const char* dirs[6] = { "/etc", "/bin", "/tmp", "/lib", "/var", NULL };
  unsigned char uri_buffer[16]; /* strlen("file:///DIR/foo")+1 */  
  int i;
  const char *dir;
#endif
#endif
  unsigned char *str;
  raptor_uri *uri1, *uri2, *uri3;

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
  
#ifdef WIN32
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

#else

  failures += assert_filename_to_uri ("/path/to/file", "file:///path/to/file");
  failures += assert_filename_to_uri ("/path/to/file with spaces", "file:///path/to/file%20with%20spaces");
  failures += assert_uri_to_filename ("file:///path/to/file", "/path/to/file");
  failures += assert_uri_to_filename ("file:///path/to/file%20with%20spaces", "/path/to/file with spaces");

  /* Tests for Issue#0000268 http://bugs.librdf.org/mantis/view.php?id = 268 */
  failures += assert_uri_to_filename ("file:///path/to/http%253A%252F%252Fwww.example.org%252Fa%252Fb%252Fc", "/path/to/http%3A%2F%2Fwww.example.org%2Fa%2Fb%2Fc");
  failures += assert_filename_to_uri ("/path/to/http%3A%2F%2Fwww.example.org%2Fa%2Fb%2Fc", "file:///path/to/http%253A%252F%252Fwww.example.org%252Fa%252Fb%252Fc");

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
    sprintf((char*)uri_buffer, "file://%s/foo", dir);
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    fprintf(stderr,
            "%s: Checking relative file name 'foo' in dir %s expecting URI %s\n",
            program, dir, uri_buffer);
#endif
    failures += assert_filename_to_uri ("foo", (const char*)uri_buffer);
  }
#endif
 
#endif

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
