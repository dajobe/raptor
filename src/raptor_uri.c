/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_uri.c - Raptor URI resolving implementation
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
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


#ifdef WIN32_URI_TEST
#define WIN32
#endif


static raptor_uri_handler *raptor_uri_current_uri_handler;
static void *raptor_uri_current_uri_context;

void
raptor_uri_set_handler(raptor_uri_handler *handler, void *context) 
{
  raptor_uri_current_uri_handler=handler;
  raptor_uri_current_uri_context=context;
}

void
raptor_uri_get_handler(raptor_uri_handler **handler, void **context) 
{
  if(handler)
    *handler=raptor_uri_current_uri_handler;
  if(context)
    *context=raptor_uri_current_uri_context;
}


static raptor_uri*
raptor_default_new_uri(void *context, const unsigned char *uri_string) 
{
  unsigned char *p;
  size_t len;
  
  /* If uri_string is "file:path-to-file", turn it into a correct file:URI */
  if(raptor_uri_is_file_uri(uri_string)) {
    unsigned char *fragment=NULL;
    char *filename;
    raptor_uri* uri=NULL;

    filename=raptor_uri_uri_string_to_filename_fragment(uri_string, &fragment);
    if(filename && !access(filename, R_OK)) {
      uri=(raptor_uri*)raptor_uri_filename_to_uri_string(filename);
      /* If there was a fragment, reattach it to the new URI */
      if(fragment) {
        unsigned char *new_fragment;
        raptor_uri* new_uri;

        new_fragment=(unsigned char*)RAPTOR_MALLOC(cstring, strlen((const char*)fragment)+2);
        *new_fragment='#';
        strcpy((char*)new_fragment+1, (const char*)fragment);
        new_uri=raptor_new_uri_relative_to_base(uri, new_fragment);
        RAPTOR_FREE(cstring, new_fragment);
        raptor_free_uri(uri);
        uri=new_uri;
      }
    }
    if(filename)
      RAPTOR_FREE(cstring, filename);
    if(fragment)
      RAPTOR_FREE(cstring, fragment);
    if(uri)
      return uri;
  }

  len=strlen((const char*)uri_string);
  p=(unsigned char*)RAPTOR_MALLOC(raptor_uri, len+1);
  if(!p)
    return NULL;
  strcpy((char*)p, (const char*)uri_string);
  return (raptor_uri*)p;
}


raptor_uri*
raptor_new_uri(const unsigned char *uri_string) 
{
  return (*raptor_uri_current_uri_handler->new_uri)(raptor_uri_current_uri_context, uri_string);
}


static raptor_uri*
raptor_default_new_uri_from_uri_local_name(void *context,
                                           raptor_uri *uri,
                                           const unsigned char *local_name)
{
  int uri_length=strlen((char*)uri);
  unsigned char *p=(unsigned char*)RAPTOR_MALLOC(cstring, 
                                                 uri_length + strlen((const char*)local_name) + 1);
  if(!p)
    return NULL;
  
  strcpy((char*)p, (const char*)uri);
  strcpy((char*)p + uri_length, (const char*)local_name);
  
  return (raptor_uri*)p;
}


raptor_uri*
raptor_new_uri_from_uri_local_name(raptor_uri *uri, const unsigned char *local_name)
{
  return (*raptor_uri_current_uri_handler->new_uri_from_uri_local_name)(raptor_uri_current_uri_context, uri, local_name);
}


static raptor_uri*
raptor_default_new_uri_relative_to_base(void *context,
                                        raptor_uri *base_uri,
                                        const unsigned char *uri_string) 
{
  raptor_uri* new_uri;
  size_t new_uri_len=strlen((const char*)base_uri)+strlen((const char*)uri_string)+1;

  new_uri=(raptor_uri*)RAPTOR_MALLOC(cstring, new_uri_len+1);
  if(!new_uri)
    return NULL;
  
  /* If URI string is empty, just copy base URI */
  if(!*uri_string) {
    strcpy((char*)new_uri, (char*)base_uri);
    return new_uri;
  }

  raptor_uri_resolve_uri_reference((const unsigned char*)base_uri, uri_string,
                                   (unsigned char*)new_uri, new_uri_len);
  return new_uri;
}


raptor_uri*
raptor_new_uri_relative_to_base(raptor_uri *base_uri, 
                                const unsigned char *uri_string) 
{
  return (*raptor_uri_current_uri_handler->new_uri_relative_to_base)(raptor_uri_current_uri_context, base_uri, uri_string);
}


raptor_uri*
raptor_new_uri_from_id(raptor_uri *base_uri, const unsigned char *id) 
{
  raptor_uri *new_uri;
  unsigned char *local_name;
  int len;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2("Using ID %s\n", id);
#endif

  /* "#id\0" */
  len=1+strlen((char*)id)+1;
  local_name=(unsigned char*)RAPTOR_MALLOC(cstring, len);
  if(!local_name)
    return NULL;
  *local_name='#';
  strcpy((char*)local_name+1, (char*)id);
  new_uri=raptor_new_uri_relative_to_base(base_uri, local_name);
  RAPTOR_FREE(cstring, local_name);
  return new_uri;
}


static raptor_uri*
raptor_default_new_uri_for_rdf_concept(void *context, const char *name) 
{
  raptor_uri *new_uri;
  char *base_uri=RAPTOR_RDF_MS_URI;
  int base_uri_len=strlen(base_uri);
  int new_uri_len;

  new_uri_len=base_uri_len+strlen(name)+1;
  new_uri=(raptor_uri*)RAPTOR_MALLOC(cstring, new_uri_len);
  if(!new_uri)
    return NULL;
  strcpy((char*)new_uri, base_uri);
  strcpy((char*)new_uri+base_uri_len, name);
  return new_uri;
}


raptor_uri*
raptor_new_uri_for_rdf_concept(const char *name) 
{
  return (*raptor_uri_current_uri_handler->new_uri_for_rdf_concept)(raptor_uri_current_uri_context, name);
}


static void
raptor_default_free_uri(void *context, raptor_uri *uri) 
{
  RAPTOR_FREE(raptor_uri, uri);
}


void
raptor_free_uri(raptor_uri *uri)
{
  (*raptor_uri_current_uri_handler->free_uri)(raptor_uri_current_uri_context, uri);
}


static int
raptor_default_uri_equals(void *context, raptor_uri* uri1, raptor_uri* uri2)
{
  return strcmp((char*)uri1, (char*)uri2)==0;
}


int
raptor_uri_equals(raptor_uri* uri1, raptor_uri* uri2)
{
  return (*raptor_uri_current_uri_handler->uri_equals)(raptor_uri_current_uri_context, uri1, uri2);
}


static raptor_uri*
raptor_default_uri_copy(void *context, raptor_uri *uri)
{
  raptor_uri* new_uri=(raptor_uri*)RAPTOR_MALLOC(cstring, strlen((char*)uri)+1);
  if(!new_uri)
    return NULL;
  strcpy((char*)new_uri, (char*)uri);
  return new_uri;
}


raptor_uri*
raptor_uri_copy(raptor_uri *uri) 
{
  return (*raptor_uri_current_uri_handler->uri_copy)(raptor_uri_current_uri_context, uri);
}


static unsigned char*
raptor_default_uri_as_string(void *context, raptor_uri *uri)
{
  return (unsigned char*)uri;
}


unsigned char*
raptor_uri_as_string(raptor_uri *uri) 
{
  return (*raptor_uri_current_uri_handler->uri_as_string)(raptor_uri_current_uri_context, uri);
}


static unsigned char*
raptor_default_uri_as_counted_string(void *context, raptor_uri *uri,
                                     size_t* len_p)
{
  if(len_p)
    *len_p=strlen((char*)uri);
  return (unsigned char*)uri;
}


unsigned char*
raptor_uri_as_counted_string(raptor_uri *uri, size_t* len_p) 
{
  return (*raptor_uri_current_uri_handler->uri_as_counted_string)(raptor_uri_current_uri_context, uri, len_p);
}



/**
 * raptor_uri_filename_to_uri_string - Converts a filename to a file: URI
 * @filename: The filename to convert
 * 
 * Handles the OS-specific escaping on turning filenames into URIs
 * and returns a new buffer that the caller must free().  Turns
 * a space in the filname into %20 and '%' into %25.
 *
 * Return value: A newly allocated string with the URI or NULL on failure
 **/
unsigned char *
raptor_uri_filename_to_uri_string(const char *filename) 
{
  unsigned char *buffer;
  const char *from;
  char *to;
#ifndef WIN32
  char path[PATH_MAX];
#endif
  /*     "file://" ... \0 */
  size_t len=7+1;
  
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
    len+=3; /* relative filename - add / and ./ */
  else if(*filename == '\\')
    len-=2; /* two // from not needed in filename */
  else
    len++; /* / at start of path */

#else
/* others - unix: turn spaces into %20, '%' into %25 */

  if(*filename != '/') {
    if(!getcwd(path, PATH_MAX))
      return NULL;
    strcat(path, "/");
    strcat(path, filename);
    filename=(const char*)path;
  }
#endif

  /* add URI-escaped filename length */
  for(from=filename; *from ; from++) {
    len++;
    if(*from == ' ' || *from == '%')
      len+=2; /* strlen(%xx)-1 */
  }

  buffer=(unsigned char*)RAPTOR_MALLOC(cstring, len);
  if(!buffer)
    return NULL;

  strcpy((char*)buffer, "file://");
  from=filename;
  to=(char*)(buffer+7);
#ifdef WIN32
  if(*from == '\\' && from[1] == '\\')
    from+=2;
  else
    *to++ ='/';
#endif
  while(*from) {
    char c=*from++;
#ifdef WIN32
    if (c == '\\')
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
      *to++ ='%';
      *to++ ='2';
      *to++ =(c == ' ') ? '0' : '5';
    } else
      *to++ =c;
  }
  *to='\0';
  
#ifdef RAPTOR_DEBUG
  if(1) {
    size_t actual_len=strlen((const char*)buffer)+1;
    if(actual_len != len) {
      if(actual_len > len)
        RAPTOR_FATAL3("uri length %d is LONGER than malloced %d\n", 
                      (int)actual_len, (int)len);
      else
        RAPTOR_DEBUG4("uri '%s' length %d is shorter than malloced %d\n", 
                      buffer, (int)actual_len, (int)len);
    }
  }
#endif

  return buffer;
}


/**
 * raptor_uri_uri_string_to_filename_fragment - Convert a file: URI to a filename and fragment
 * @uri_string: The file: URI to convert
 * @fragment_p: Address of pointer to store any URI fragment or NULL
 * 
 * Handles the OS-specific file: URIs to filename mappings.  Returns
 * a new buffer containing the filename that the caller must free.
 *
 * If fragment_p is given, a new string containing the URI fragment
 * is returned, or NULL if none is present
 * 
 * Return value: A newly allocated string with the filename or NULL on failure
 **/
char *
raptor_uri_uri_string_to_filename_fragment(const unsigned char *uri_string,
                                           unsigned char **fragment_p) 
{
  char *filename;
  size_t len=0;
  raptor_uri_detail *ud=NULL;
  unsigned char *from;
  char *to;
#ifdef WIN32
  unsigned char *p;
#endif

  ud=raptor_new_uri_detail(uri_string);
  if(!ud)
    return NULL;
  

  if(!ud->scheme || raptor_strcasecmp((const char*)ud->scheme, "file")) {
    raptor_free_uri_detail(ud);
    return NULL;
  }

  if(ud->authority) {
    if(!*ud->authority)
      ud->authority=NULL;
    else if(!raptor_strcasecmp((const char*)ud->authority, "localhost"))
      ud->authority=NULL;
  }

  /* Cannot do much if there is no path */
  if(!ud->path || (ud->path && !*ud->path)) {
    raptor_free_uri_detail(ud);
    return NULL;
  }

  /* See raptor_uri_filename_to_uri_string for details of the mapping */
#ifdef WIN32
  if(ud->authority)
    len+=ud->authority_len+3;

  p=ud->path+1;
  if(*p && (p[1] == '|' || p[1] == ':')) {
    /* Either 
     *   "a:" like in file://a|/... or file://a:/... 
     * or
     *   "a:." like in file://a:./foo
     * giving device-relative path a:foo
     */
    if(p[2]=='.') {
      p[2]=*p;
      p[3]=':';
      p+= 2;
      len-= 2; /* remove 2 for ./ */
    } else
      p[1]=':';
  }
  len--; /* for removing leading / off path */
#endif


  /* add URI-escaped filename length */
  for(from=ud->path; *from ; from++) {
    len++;
    if(*from == '%')
      from+= 2;
  }


  /* Something is wrong */
  if(!len) {
    raptor_free_uri_detail(ud);
    return NULL;
  }
    
  filename=(char*)RAPTOR_MALLOC(cstring, len+1);
  if(!filename) {
    raptor_free_uri_detail(ud);
    return NULL;
  }


  to=filename;

#ifdef WIN32
  if(ud->authority) {
    *to++ = '\\';
    *to++ = '\\';
    from=ud->authority;
    while( (*to++ = *from++) )
      ;
    to--;
    *to++ = '\\';
  }
  
  /* copy path after all /s */
  from=p;
#else
  from=ud->path;
#endif

  while(*from) {
    char c=*from++;
#ifdef WIN32
    if(c == '/')
      *to++ ='\\';
    else
#endif
    if(c == '%') {
      if(*from && from[1]) {
        unsigned char hexbuf[3];
        unsigned char *endptr=NULL;
        hexbuf[0]=*from;
        hexbuf[1]=from[1];
        hexbuf[2]='\0';
        c=(char)strtol((const char*)hexbuf, (char**)&endptr, 16);
        if(endptr == &hexbuf[2])
          *to++ = c;
      }
      from+= 2;
    } else
      *to++ =c;
  }
  *to='\0';

#ifdef RAPTOR_DEBUG
  if(1) {
    size_t actual_len=strlen(filename);
    if(actual_len != len) {
      if(actual_len > len) {
        fprintf(stderr, "filename '%s'\n", filename);
        RAPTOR_FATAL3("Filename length %d is LONGER than malloced %d\n", 
                      (int)actual_len, (int)len);
      } else
        RAPTOR_DEBUG3("Filename length %d is shorter than malloced %d\n", 
                      (int)actual_len, (int)len);
    }
  }
#endif

  if(fragment_p) {
    if(ud->fragment) {
      len=ud->fragment_len;
      *fragment_p=(unsigned char*)RAPTOR_MALLOC(cstring, len+1);
      strncpy((char*)*fragment_p, (const char*)ud->fragment, len+1);
    } else
      *fragment_p=NULL;
  }

  raptor_free_uri_detail(ud);

  return filename;
}


/**
 * raptor_uri_uri_string_to_filename - Convert a file: URI to a filename
 * @uri_string: The file: URI to convert
 * 
 * Handles the OS-specific file: URIs to filename mappings.  Returns
 * a new buffer containing the filename that the caller must free.
 *
 * Return value: A newly allocated string with the filename or NULL on failure
 **/
char *
raptor_uri_uri_string_to_filename(const unsigned char *uri_string) 
{
  return raptor_uri_uri_string_to_filename_fragment(uri_string, NULL);
}


/**
 * raptor_uri_is_file_uri - Check if a URI string is a a file: URI
 * @uri_string: The URI string to check
 * 
 * Return value: Non zero if URI string is a file: URI
 **/
int
raptor_uri_is_file_uri(const unsigned char* uri_string) {
  return raptor_strncasecmp((const char*)uri_string, "file:", 5)==0;
}


/**
 * raptor_new_uri_for_xmlbase - Turn a URI into one suitable for XML base
 * @old_uri: URI to transform
 * 
 * Takes an existing URI and ensures it has a path (default /) and has
 * no fragment or query arguments - XML base does not use these.
 * 
 * Return value: new URI object or NULL on failure.
 **/
raptor_uri*
raptor_new_uri_for_xmlbase(raptor_uri* old_uri)
{
  unsigned char *uri_string=raptor_uri_as_string(old_uri);
  unsigned char *new_uri_string;
  raptor_uri* new_uri;
  raptor_uri_detail *ud;
  
  ud=raptor_new_uri_detail(uri_string);
  if(!ud)
    return NULL;

  if(!ud->path)
    ud->path=(unsigned char*)"/";
  
  ud->query=NULL; ud->query_len=0;
  ud->fragment=NULL; ud->fragment_len=0;
  new_uri_string=raptor_uri_detail_to_string(ud, NULL);
  raptor_free_uri_detail(ud);
  if(!new_uri_string)
    return NULL;
  
  new_uri=raptor_new_uri(new_uri_string);
  RAPTOR_FREE(cstring, new_uri_string);

  return new_uri;
}


/**
 * raptor_new_uri_for_retrieval - Turn a URI into one suitable for retrieval
 * @old_uri: URI to transform
 * 
 * Takes an existing URI and ensures it has a path (default /) and has
 * no fragment - URI retrieval does not use the fragment part.
 * 
 * Return value: new URI object or NULL on failure.
 **/
raptor_uri*
raptor_new_uri_for_retrieval(raptor_uri* old_uri)
{
  unsigned char *uri_string=raptor_uri_as_string(old_uri);
  unsigned char *new_uri_string;
  raptor_uri* new_uri;
  raptor_uri_detail *ud;
  

  ud=raptor_new_uri_detail(uri_string);
  if(!ud)
    return NULL;

  if(!ud->path)
    ud->path=(unsigned char*)"/";

  ud->fragment=NULL; ud->fragment_len=0;
  new_uri_string=raptor_uri_detail_to_string(ud, NULL);
  raptor_free_uri_detail(ud);
  if(!new_uri_string)
    return NULL;
  
  new_uri=raptor_new_uri(new_uri_string);
  RAPTOR_FREE(cstring, new_uri_string);

  return new_uri;
}


void
raptor_uri_init_default_handler(raptor_uri_handler *handler) 
{
  if(handler->initialised)
    return;
  
  handler->new_uri=raptor_default_new_uri;
  handler->new_uri_from_uri_local_name=raptor_default_new_uri_from_uri_local_name;
  handler->new_uri_relative_to_base=raptor_default_new_uri_relative_to_base;
  handler->new_uri_for_rdf_concept=raptor_default_new_uri_for_rdf_concept;
  handler->free_uri=raptor_default_free_uri;
  handler->uri_equals=raptor_default_uri_equals;
  handler->uri_copy=raptor_default_uri_copy;
  handler->uri_as_string=raptor_default_uri_as_string;
  handler->uri_as_counted_string=raptor_default_uri_as_counted_string;
  
  handler->initialised=1;
}

static raptor_uri_handler raptor_uri_default_handler = {
  raptor_default_new_uri,
  raptor_default_new_uri_from_uri_local_name,
  raptor_default_new_uri_relative_to_base,
  raptor_default_new_uri_for_rdf_concept,
  raptor_default_free_uri,
  raptor_default_uri_equals,
  raptor_default_uri_copy,
  raptor_default_uri_as_string,
  raptor_default_uri_as_counted_string,
  1
};


void
raptor_uri_init(void)
{
  raptor_uri_init_default_handler(&raptor_uri_default_handler);
  raptor_uri_set_handler(&raptor_uri_default_handler, NULL);
}




#ifdef STANDALONE

#include <stdio.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

/* one more prototype */
int main(int argc, char *argv[]);

static const char *program;


static int
assert_filename_to_uri (const char *filename, const char *reference_uri)
{
  unsigned char *uri;

  uri=raptor_uri_filename_to_uri_string(filename);

  if (!uri || strcmp((const char*)uri, (const char*)reference_uri)) {
    fprintf(stderr, 
            "%s: raptor_uri_filename_to_uri_string(%s) FAILED gaving URI %s != %s\n",
            program, filename, uri, reference_uri);
    if(uri)
      RAPTOR_FREE(cstring, uri);
    return 1;
  }

  RAPTOR_FREE(cstring, uri);
  return 0;
}


static int
assert_uri_to_filename (const char *uri, const char *reference_filename)
{
  char *filename;

  filename=raptor_uri_uri_string_to_filename((const unsigned char*)uri);

  if(filename && !reference_filename) {
    fprintf(stderr, 
            "%s: raptor_uri_uri_string_to_filename(%s) FAILED giving filename %s != NULL\n", 
            program, uri, filename);
    if(filename)
      RAPTOR_FREE(cstring, filename);
    return 1;
  } else if (filename && strcmp(filename, reference_filename)) {
    fprintf(stderr,
            "%s: raptor_uri_uri_string_to_filename(%s) FAILED gaving filename %s != %s\n",
            program, uri, filename, reference_filename);
    if(filename)
      RAPTOR_FREE(cstring, filename);
    return 1;
  }

  RAPTOR_FREE(cstring, filename);
  return 0;
}


int
main(int argc, char *argv[]) 
{
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

  int failures=0;

  if((program=strrchr(argv[0], '/')))
    program++;
  else if((program=strrchr(argv[0], '\\')))
    program++;
  else
    program=argv[0];
  
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


  failures += assert_uri_to_filename ("file:c:\\thing",     ":\\thing");
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

#if defined(HAVE_UNISTD_H) && defined(HAVE_SYS_STAT_H)
  /* Need to test this with a real dir (preferably not /)
   * This is just a test so pretty likely to work on all development systems
   * that are not WIN32
   */

  for(i=0; (dir=dirs[i]); i++) {
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
    fprintf(stderr,
            "%s: Checking relative file name 'foo' in dir %s expecting URI %s\n",
            program, dir, uri_buffer);
    failures += assert_filename_to_uri ("foo", (const char*)uri_buffer);
  }
#endif
 
#endif

  raptor_uri_init();
  
  uri1=raptor_new_uri((const unsigned char*)base_uri);

  str=raptor_uri_as_string(uri1);
  if(strcmp((const char*)str, base_uri)) {
    fprintf(stderr,
            "%s: raptor_uri_as_string(%s) FAILED gaving %s != %s\n",
            program, base_uri, str, base_uri);
    failures++;
  }
  
  uri2=raptor_new_uri_for_xmlbase(uri1);
  str=raptor_uri_as_string(uri2);
  if(strcmp((const char*)str, base_uri_xmlbase)) {
    fprintf(stderr, 
            "%s: raptor_new_uri_for_xmlbase(URI %s) FAILED giving %s != %s\n",
            program, base_uri, str, base_uri_xmlbase);
    failures++;
  }
  
  uri3=raptor_new_uri_for_retrieval(uri1);

  str=raptor_uri_as_string(uri3);
  if(strcmp((const char*)str, base_uri_retrievable)) {
    fprintf(stderr,
            "%s: raptor_new_uri_for_retrievable(%s) FAILED gaving %s != %s\n",
            program, base_uri, str, base_uri_retrievable);
    failures++;
  }
  
  raptor_free_uri(uri3);
  raptor_free_uri(uri2);
  raptor_free_uri(uri1);

  return failures ;
}

#endif
