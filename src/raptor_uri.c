/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_uri.c - Raptor URI resolving implementation
 *
 * $Id$
 *
 * Copyright (C) 2002-2004 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology - http://www.ilrt.bris.ac.uk/
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

/* Prototypes for local functions */
static int raptor_uri_is_absolute (const unsigned char *uri);

static void raptor_uri_parse (const unsigned char *uri, unsigned char *buffer, size_t len, unsigned char **scheme, unsigned char **authority, unsigned char **path, unsigned char **query, unsigned char **fragment);


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


static int
raptor_uri_is_absolute (const unsigned char *uri)
{
  int result = 0;

  if (*uri && isalpha((int)*uri)) {
    ++uri;


   /* 
    * RFC 2396 3.1 Scheme Component
    * scheme        = alpha *( alpha | digit | "+" | "-" | "." )
    */
    while (*uri && (isalnum((int)*uri)
                    || (*uri == '+') || (*uri == '-') || (*uri == '.')))
      ++uri;

    result = (*uri == ':');
  }

  return result;
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



/*
 * raptor_uri_parse() and raptor_uri_resolve_uri_reference()
 * originally from code
 * Copyright (C) 2000 Jason Diamond - http://injektilo.org/
 * from http://injektilo.org/rdf/repat.html
 *
 * Much updated, documented and improved for removing fixed-size buffers.
 */


/**
 * raptor_uri_parse - Split a URI(-ref) into several string components
 * @uri: The URI string to split
 * @buffer: Destination buffer 
 * @len: Destination buffer length
 * @scheme: Pointer to buffer for scheme component
 * @authority: Pointer to buffer for URI authority (such as hostname)
 * @path: Pointer to buffer for path component
 * @query: Pointer to buffer for query string
 * @fragment: Pointer to buffer for URI-reference fragment
 * 
 * 
 **/
static void
raptor_uri_parse (const unsigned char *uri, unsigned char *buffer, size_t len,
                  unsigned char **scheme, unsigned char **authority, 
                  unsigned char **path, unsigned char **query, 
                  unsigned char **fragment)
{
  const unsigned char *s = NULL;
  unsigned char *d = NULL;

  *scheme = NULL;
  *authority = NULL;
  *path = NULL;
  *query = NULL;
  *fragment = NULL;

  s = uri;

  d = buffer;

  /* Extract the scheme if present */
  if (raptor_uri_is_absolute(uri)) {
    *scheme = d;
    
    while (*s != ':')
      *d++ = *s++;
    
    *d++ = '\0';
    
    /* Move to the char after the : */
    ++s;
  }


  /* Extract the authority (e.g. [user:pass@]hostname[:port] in http, etc.) */
  if (*s && *(s + 1) && *s == '/' && *(s + 1) == '/') {
    *authority = d;

    s += 2; /* skip // after scheme */
    
    while (*s && *s != '/' && *s != '?' && *s != '#')
      *d++ = *s++;
    
    *d++ = '\0';
  }


  /* Extract the path (anything before query string or fragment) */
  if (*s && *s != '?' && *s != '#') {
    *path = d;
    
    while (*s && *s != '?' && *s != '#')
      *d++ = *s++;

    *d++ = '\0';
  }


  /* Extract the query string if present */
  if (*s && *s == '?') {
    *query = d;
    
    ++s;
    
    while (*s && *s != '#')
      *d++ = *s++;
    
    *d++ = '\0';
  }

  
  /* Extract the URI-ref fragment if present */
  if (*s && *s == '#') {
    *fragment = d;
    
    ++s;
    
    while (*s)
      *d++ = *s++;
    
    *d = '\0';
  }

}


static unsigned char*
raptor_uri_construct(const unsigned char *scheme, 
                     const unsigned char *authority, 
                     const unsigned char *path, 
                     const unsigned char *query, 
                     const unsigned char *fragment)
{
  size_t len=0;
  unsigned char *p;
  
  if(scheme)
    len+= strlen((const char*)scheme)+1; /* : */
  if(authority)
    len+= 2 + strlen((const char*)authority); /* // */
  if(path)
    len+= strlen((const char*)path);
  if(fragment)
    len+= 1 + strlen((const char*)fragment); /* # */
  if(query)
    len+= 1 + strlen((const char*)query); /* ? */
  p=(unsigned char*)RAPTOR_MALLOC(cstring, len+1);
  if(!p)
    return NULL;
  *p='\0';
  
  if(scheme) {
    strcpy((char*)p, (const char*)scheme);
    strcat((char*)p,":");
  }
  if(authority) {
    strcat((char*)p, "//");
    strcat((char*)p, (const char*)authority);
  }
  if(path)
    strcat((char*)p, (const char*)path);
  if(fragment) {
    strcat((char*)p, "#");
    strcat((char*)p, (const char*)fragment);
  }
  if(query) {
    strcat((char*)p, "?");
    strcat((char*)p, (const char*)query);
  }

  return p;
}


/**
 * raptor_uri_resolve_uri_reference - Resolve a URI to a base URI
 * @base_uri: Base URI string
 * @reference_uri: Reference URI string
 * @buffer: Destination buffer URI
 * @length: Length of destination buffer
 * 
 **/
void
raptor_uri_resolve_uri_reference (const unsigned char *base_uri,
                                  const unsigned char *reference_uri,
                                  unsigned char *buffer, size_t length)
{
  unsigned char *base_buffer=NULL;
  unsigned char *reference_buffer=NULL;
  unsigned char *path_buffer=NULL;
  int reference_buffer_len;
  int base_uri_len;
  
  unsigned char *base_scheme;
  unsigned char *base_authority;
  unsigned char *base_path;
  unsigned char *base_query;
  unsigned char *base_fragment;

  unsigned char *reference_scheme;
  unsigned char *reference_authority;
  unsigned char *reference_path;
  unsigned char *reference_query;
  unsigned char *reference_fragment;

  unsigned char *result_scheme = NULL;
  unsigned char *result_authority = NULL;
  unsigned char *result_path = NULL;

  *buffer = '\0';

  reference_buffer_len=strlen((const char*)reference_uri)+1;
  reference_buffer=(unsigned char*)RAPTOR_MALLOC(cstring, reference_buffer_len);
  if(!reference_buffer)
    goto resolve_tidy;
  
  raptor_uri_parse (reference_uri, reference_buffer, reference_buffer_len,
                    &reference_scheme, &reference_authority,
                    &reference_path, &reference_query, &reference_fragment);

  /* is reference URI "" or "#frag"? */
  if (!reference_scheme && !reference_authority
      && !reference_path && !reference_query) {
    unsigned char *p;
    unsigned char c;

    /* Copy up to '\0' or '#' */
    for(p=buffer; (c= *base_uri) && c != '#'; p++, base_uri++)
      *p = c;
    *p='\0';
    
    if (reference_fragment) {
      *p++='#';
      strcpy((char*)p, (const char*)reference_fragment);
    }
    goto resolve_tidy;
  }
  
  /* reference has a scheme - is an absolute URI */
  if (reference_scheme) {
    strcpy ((char*)buffer, (const char*)reference_uri);
    goto resolve_tidy;
  }
  

  /* now the reference URI must be schemeless, i.e. relative */
  base_uri_len=strlen((const char*)base_uri);
  base_buffer=(unsigned char*)RAPTOR_MALLOC(cstring, base_uri_len+1);
  if(!base_buffer)
    goto resolve_tidy;

  raptor_uri_parse (base_uri, base_buffer, base_uri_len,
                    &base_scheme, &base_authority,
                    &base_path, &base_query, &base_fragment);
  
  result_scheme = base_scheme;
  
  /* an authority is given ( [user:pass@]hostname[:port] for http)
   * so the reference URI is like //authority
   */
  if (reference_authority) {
    result_authority = reference_authority;
    result_path = reference_path;
    goto resolve_end;
  }

  /* no - so now we have path (maybe with query, fragment) relative to base */
  result_authority = base_authority;
    

  if (reference_path && reference_path[0] == '/') {
    /* reference path is absolute */
    result_path = reference_path;
  } else {
    /* need to resolve relative path */
    unsigned char *p = NULL;

    /* Make result path be path buffer; initialise it to empty */
    int path_buffer_len=1;

    if(base_path)
      path_buffer_len +=strlen((const char*)base_path);
    else {
      base_path=(unsigned char*)"/"; /* static, but copied and not free()d  */
      path_buffer_len++;
    }

    if(reference_path)
      path_buffer_len +=strlen((const char*)reference_path);

    path_buffer=(unsigned char*)RAPTOR_MALLOC(cstring, path_buffer_len);
    if(!path_buffer)
      goto resolve_tidy;
    result_path = path_buffer;
    path_buffer[0] = '\0';
    
    p = (unsigned char*)strrchr((const char*)base_path, '/');
    if (p) {
      /* Found a /, copy everything before that to path_buffer */
      unsigned char *s = base_path;
      unsigned char *d = path_buffer;
      
      while (s <= p)
        *d++ = *s++;
      
      *d++ = '\0';
    }

    if (reference_path)
      strcat ((char*)path_buffer, (const char*)reference_path);


    /* NEW BLOCK - only ANSI C allows this */
    {
      /* remove all occurrences of "./" */
      
      unsigned char *p2 = path_buffer;
      unsigned char *s = path_buffer;
      
      while (*s) {
        if (*s == '/') {

          if (p2 == (s - 1) && *p2 == '.') {
            unsigned char *d = p2;
            
            ++s;
            
            while (*s)
              *d++ = *s++;
            
            *d = '\0';

            s = p2;
          } else {
            p2 = s + 1;
          }
        }
        
        ++s;
      }
      
      /* if the path ends with ".", remove it */
      
      if (p2 == (s - 1) && *p2 == '.')
        *p2 = '\0';

    } /* end BLOCK */

    

    /* NEW BLOCK - only ANSI C allows this */
    {
      /* remove all occurrences of "<segment>/../" */
      
      unsigned char *s = path_buffer;
      /* These form a small stack of path components */
      unsigned char *p3 = NULL; /* current component */
      unsigned char *p2 = NULL; /* previous component */
      unsigned char *p0 = NULL; /* before p2 */
      unsigned char last_char='\0';
      
      for (;*s; last_char=*s++) {

        /* find the path components */
        if (*s != '/') {
          /* Must be at the start or  follow a / */
          if(!last_char || last_char == '/') {
            if (!p3) {
              /* Empty stack; initialise */
              p3 = s;
            } else if (!p2) {
              /* Add 2nd component */
              p2 = s;
            }
          }
          continue;
        }


        /* Found a '/' */

        /* Wait till there are two components */
        if (!p3 || !p2)
          continue;
        
        /* Two components have been seen */
          
        /* If the previous one was '..' */
        if (p2 == (s - 2) && *p2 == '.' && *(p2 + 1) == '.') {
            
          /* If the current one isn't '..' */
          if (*p3 != '.' && *(p3 + 1) != '.') {
            unsigned char *d = p3;
            
            ++s;
            
            while (*s)
              *d++ = *s++;
            
            *d = '\0';
            
            if (p0 < p3) {
              s = p3 - 1;
              
              p3 = p0;
              p2 = NULL;
            } else {
              s = path_buffer;
              p3 = NULL;
              p2 = NULL;
              p0 = NULL;
            }
            
          } /* end if !.. */
          
        } else {
          /* shift the path components stack */
          p0 = p3;
          p3 = p2;
          p2 = NULL;
        }
          
        
      } /* end for */

      
      /* if the path ends with "<segment>/..", remove it */
      
      if (p2 == (s - 2) && *p2 == '.' && *(p2 + 1) == '.') {
        if (p3)
          *p3 = '\0';
      }

    } /* end BLOCK */

  } /* end resolve relative path */


  resolve_end:
  
  if (result_scheme) {
    strcpy ((char*)buffer, (const char*)result_scheme);
    strcat ((char*)buffer, ":");
  }
  
  if (result_authority) {
    strcat ((char*)buffer, "//");
    strcat ((char*)buffer, (const char*)result_authority);
  }
  
  if (result_path)
    strcat ((char*)buffer, (const char*)result_path);
  
  if (reference_query) {
    strcat ((char*)buffer, "?");
    strcat ((char*)buffer, (const char*)reference_query);
  }
  
  if (reference_fragment) {
    strcat ((char*)buffer, "#");
    strcat ((char*)buffer, (const char*)reference_fragment);
  }

  resolve_tidy:
  if(path_buffer)
    RAPTOR_FREE(cstring, path_buffer);
  if(base_buffer)
    RAPTOR_FREE(cstring, base_buffer);
  if(reference_buffer)
    RAPTOR_FREE(cstring, reference_buffer);
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
      *to++=c;
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
  unsigned char *buffer;
  char *filename;
  int uri_string_len=strlen((const char*)uri_string);
  size_t len=0;
  unsigned char *scheme, *authority, *path, *query, *fragment;
  unsigned char *from;
  char *to;
#ifdef WIN32
  unsigned char *p;
#endif

  buffer=(unsigned char*)RAPTOR_MALLOC(cstring, uri_string_len+1);
  if(!buffer)
    return NULL;
  
  raptor_uri_parse (uri_string, buffer, uri_string_len,
                    &scheme, &authority, &path, &query, &fragment);

  if(!scheme || raptor_strcasecmp((const char*)scheme, "file")) {
    RAPTOR_FREE(cstring, buffer);
    return NULL;
  }

  if(authority) {
    if(!*authority)
      authority=NULL;
    else if(!raptor_strcasecmp((const char*)authority, "localhost"))
      authority=NULL;
  }

  /* Cannot do much if there is no path */
  if(!path || (path && !*path)) {
    RAPTOR_FREE(cstring, buffer);
    return NULL;
  }

  /* See raptor_uri_filename_to_uri_string for details of the mapping */
#ifdef WIN32
  if(authority)
    len+=strlen((const char*)authority)+3;

  p=path+1;
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
  for(from=path; *from ; from++) {
    len++;
    if(*from == '%')
      from+= 2;
  }


  /* Something is wrong */
  if(!len) {
    RAPTOR_FREE(cstring, buffer);
    return NULL;
  }
    
  filename=(char*)RAPTOR_MALLOC(cstring, len+1);
  if(!filename) {
    RAPTOR_FREE(cstring, buffer);
    return NULL;
  }


  to=filename;

#ifdef WIN32
  if(authority) {
    *to++ = '\\';
    *to++ = '\\';
    from=authority;
    while( (*to++ = *from++) )
      ;
    to--;
    *to++ = '\\';
  }
  
  /* copy path after all /s */
  from=p;
#else
  from=path;
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
    if(fragment) {
      len=strlen((const char*)fragment);
      *fragment_p=(unsigned char*)RAPTOR_MALLOC(cstring, len+1);
      strcpy((char*)*fragment_p, (const char*)fragment);
    } else
      *fragment_p=NULL;
  }

  RAPTOR_FREE(cstring, buffer);

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
  unsigned char *buffer;
  size_t buffer_len=strlen((const char*)uri_string)+1;
  unsigned char *scheme;
  unsigned char *authority;
  unsigned char *path;
  unsigned char *query;
  unsigned char *fragment;
  unsigned char *new_uri_string;
  raptor_uri* new_uri;

  buffer=(unsigned char*)RAPTOR_MALLOC(cstring, buffer_len);
  if(!buffer)
    return NULL;
  
  raptor_uri_parse (uri_string, buffer, buffer_len,
                    &scheme, &authority, &path, &query, &fragment);

  if(!path)
    path=(unsigned char*)"/";
  
  new_uri_string=raptor_uri_construct(scheme, authority, path, NULL, NULL);
  RAPTOR_FREE(cstring, buffer);
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
  unsigned char *buffer;
  size_t buffer_len=strlen((const char*)uri_string)+1;
  unsigned char *scheme;
  unsigned char *authority;
  unsigned char *path;
  unsigned char *query;
  unsigned char *fragment;
  unsigned char *new_uri_string;
  raptor_uri* new_uri;

  buffer=(unsigned char*)RAPTOR_MALLOC(cstring, buffer_len);
  if(!buffer)
    return NULL;
  
  raptor_uri_parse (uri_string, buffer, buffer_len,
                    &scheme, &authority, &path, &query, &fragment);

  if(!path)
    path=(unsigned char*)"/";
  
  new_uri_string=raptor_uri_construct(scheme, authority, path, query, NULL);
  RAPTOR_FREE(cstring, buffer);
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


static int
assert_resolve_uri (const char *base_uri, const char *reference_uri, 
                    const char *absolute_uri)
{
  unsigned char buffer[256];

  raptor_uri_resolve_uri_reference ((const unsigned char*)base_uri,
                                    (const unsigned char*)reference_uri,
                                    buffer, sizeof (buffer));

  if (strcmp((const char*)buffer, absolute_uri))
    {
      fprintf(stderr, "FAIL relative %s gave %s != %s\n",
              reference_uri, buffer, absolute_uri);
      return 1;
    }
  return 0;
}


static int
assert_filename_to_uri (const char *filename, const char *reference_uri)
{
  unsigned char *uri;

  uri=raptor_uri_filename_to_uri_string(filename);

  if (!uri || strcmp((const char*)uri, (const char*)reference_uri))
    {
      fprintf(stderr, "FAIL raptor_uri_filename_to_uri_string filename %s gave URI %s != %s\n",
              filename, uri, reference_uri);
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
    {
      fprintf(stderr, "FAIL raptor_uri_uri_string_to_filename URI %s gave filename %s != NULL\n", uri, filename);
      if(filename)
        RAPTOR_FREE(cstring, filename);
      return 1;
    }
    }
  else if (filename && strcmp(filename, reference_filename))
    {
      fprintf(stderr, "FAIL raptor_uri_uri_string_to_filename URI %s gave filename %s != %s\n",
              uri, filename, reference_filename);
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
  
  fprintf(stderr, "raptor_uri_resolve_uri_reference: Testing with base URI %s\n", base_uri);
  

  failures += assert_resolve_uri (base_uri, "g:h", "g:h");
  failures += assert_resolve_uri (base_uri, "gpath", "http://example.org/bpath/cpath/gpath");
  failures += assert_resolve_uri (base_uri, "./gpath", "http://example.org/bpath/cpath/gpath");
  failures += assert_resolve_uri (base_uri, "gpath/", "http://example.org/bpath/cpath/gpath/");
  failures += assert_resolve_uri (base_uri, "/gpath", "http://example.org/gpath");
  failures += assert_resolve_uri (base_uri, "//gpath", "http://gpath");
  failures += assert_resolve_uri (base_uri, "?y", "http://example.org/bpath/cpath/?y");
  failures += assert_resolve_uri (base_uri, "gpath?y", "http://example.org/bpath/cpath/gpath?y");
  failures += assert_resolve_uri (base_uri, "#s", "http://example.org/bpath/cpath/d;p?querystr#s");
  failures += assert_resolve_uri (base_uri, "gpath#s", "http://example.org/bpath/cpath/gpath#s");
  failures += assert_resolve_uri (base_uri, "gpath?y#s", "http://example.org/bpath/cpath/gpath?y#s");
  failures += assert_resolve_uri (base_uri, ";x", "http://example.org/bpath/cpath/;x");
  failures += assert_resolve_uri (base_uri, "gpath;x", "http://example.org/bpath/cpath/gpath;x");
  failures += assert_resolve_uri (base_uri, "gpath;x?y#s", "http://example.org/bpath/cpath/gpath;x?y#s");
  failures += assert_resolve_uri (base_uri, ".", "http://example.org/bpath/cpath/");
  failures += assert_resolve_uri (base_uri, "./", "http://example.org/bpath/cpath/");
  failures += assert_resolve_uri (base_uri, "..", "http://example.org/bpath/");
  failures += assert_resolve_uri (base_uri, "../", "http://example.org/bpath/");
  failures += assert_resolve_uri (base_uri, "../gpath", "http://example.org/bpath/gpath");
  failures += assert_resolve_uri (base_uri, "../..", "http://example.org/");
  failures += assert_resolve_uri (base_uri, "../../", "http://example.org/");
  failures += assert_resolve_uri (base_uri, "../../gpath", "http://example.org/gpath");

  failures += assert_resolve_uri (base_uri, "", "http://example.org/bpath/cpath/d;p?querystr");

  failures += assert_resolve_uri (base_uri, "../../../gpath", "http://example.org/../gpath");
  failures += assert_resolve_uri (base_uri, "../../../../gpath", "http://example.org/../../gpath");

  failures += assert_resolve_uri (base_uri, "/./gpath", "http://example.org/./gpath");
  failures += assert_resolve_uri (base_uri, "/../gpath", "http://example.org/../gpath");
  failures += assert_resolve_uri (base_uri, "gpath.", "http://example.org/bpath/cpath/gpath.");
  failures += assert_resolve_uri (base_uri, ".gpath", "http://example.org/bpath/cpath/.gpath");
  failures += assert_resolve_uri (base_uri, "gpath..", "http://example.org/bpath/cpath/gpath..");
  failures += assert_resolve_uri (base_uri, "..gpath", "http://example.org/bpath/cpath/..gpath");

  failures += assert_resolve_uri (base_uri, "./../gpath", "http://example.org/bpath/gpath");
  failures += assert_resolve_uri (base_uri, "./gpath/.", "http://example.org/bpath/cpath/gpath/");
  failures += assert_resolve_uri (base_uri, "gpath/./hpath", "http://example.org/bpath/cpath/gpath/hpath");
  failures += assert_resolve_uri (base_uri, "gpath/../hpath", "http://example.org/bpath/cpath/hpath");
  failures += assert_resolve_uri (base_uri, "gpath;x=1/./y", "http://example.org/bpath/cpath/gpath;x=1/y");
  failures += assert_resolve_uri (base_uri, "gpath;x=1/../y", "http://example.org/bpath/cpath/y");

  failures += assert_resolve_uri (base_uri, "gpath?y/./x", "http://example.org/bpath/cpath/gpath?y/./x");
  failures += assert_resolve_uri (base_uri, "gpath?y/../x", "http://example.org/bpath/cpath/gpath?y/../x");
  failures += assert_resolve_uri (base_uri, "gpath#s/./x", "http://example.org/bpath/cpath/gpath#s/./x");
  failures += assert_resolve_uri (base_uri, "gpath#s/../x", "http://example.org/bpath/cpath/gpath#s/../x");

  failures += assert_resolve_uri (base_uri, "http:gauthority", "http:gauthority");

  failures += assert_resolve_uri (base_uri, "gpath/../../../hpath", "http://example.org/hpath");

  failures += assert_resolve_uri ("http://example.org/dir/file", "../../../absfile", "http://example.org/../../absfile");
  failures += assert_resolve_uri ("http://example.org/dir/file", "http://another.example.org/dir/file", "http://another.example.org/dir/file");


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
    fprintf(stderr, "WARNING: %s: Found no convenient directory - not testing relative files\n",
            argv[0]);
  else {
    sprintf((char*)uri_buffer, "file://%s/foo", dir);
    fprintf(stderr, "%s: Checking relative file name 'foo' in dir %s expecting URI %s\n",
            argv[0], dir, uri_buffer);
    failures += assert_filename_to_uri ("foo", (const char*)uri_buffer);
  }
#endif
 
#endif

  raptor_uri_init();
  
  uri1=raptor_new_uri((const unsigned char*)base_uri);

  str=raptor_uri_as_string(uri1);
  if(strcmp((const char*)str, base_uri)) {
    fprintf(stderr, "FAIL raptor_uri_as_string URI %s gave %s != %s\n",
            base_uri, str, base_uri);
    failures++;
  }
  
  uri2=raptor_new_uri_for_xmlbase(uri1);
  str=raptor_uri_as_string(uri2);
  if(strcmp((const char*)str, base_uri_xmlbase)) {
    fprintf(stderr, "FAIL raptor_new_uri_for_xmlbase URI %s gave %s != %s\n",
            base_uri, str, base_uri_xmlbase);
    failures++;
  }
  
  uri3=raptor_new_uri_for_retrieval(uri1);

  str=raptor_uri_as_string(uri3);
  if(strcmp((const char*)str, base_uri_retrievable)) {
    fprintf(stderr, "FAIL raptor_new_uri_for_retrievable URI %s gave %s != %s\n",
            base_uri, str, base_uri_retrievable);
    failures++;
  }
  
  raptor_free_uri(uri3);
  raptor_free_uri(uri2);
  raptor_free_uri(uri1);

  return failures ;
}

#endif
