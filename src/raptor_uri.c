/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_uri.c - Raptor URI resolving implementation
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
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
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

/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


/* Prototypes for local functions */
static int raptor_uri_is_absolute (const char *uri);

static void raptor_uri_parse (const char *uri, char *buffer, size_t len, char **scheme, char **authority, char **path, char **query, char **fragment);


static raptor_uri_handler *raptor_current_uri_handler;
static void *raptor_current_uri_context;

void
raptor_uri_set_handler(raptor_uri_handler *handler, void *context) 
{
  raptor_current_uri_handler=handler;
  raptor_current_uri_context=context;
}


static raptor_uri*
raptor_default_new_uri(void *context, const char *uri_string) 
{
  int len=strlen(uri_string);
  char *p=(char*)LIBRDF_MALLOC(raptor_uri, len+1);
  if(!p)
    return NULL;
  strcpy((char*)p, uri_string);
  return (raptor_uri*)p;
}


raptor_uri*
raptor_new_uri(const char *uri_string) 
{
  return (*raptor_current_uri_handler->new_uri)(raptor_current_uri_context, uri_string);
}


static raptor_uri*
raptor_default_new_uri_from_uri_local_name(void *context,
                                           raptor_uri *uri,
                                           const char *local_name)
{
  int uri_length=strlen((char*)uri);
  char *p=(char*)LIBRDF_MALLOC(cstring, 
                               uri_length + strlen(local_name) + 1);
  if(!p)
    return NULL;
  
  strcpy(p, uri);
  strcpy(p + uri_length, local_name);
  
  return (raptor_uri*)p;
}


raptor_uri*
raptor_new_uri_from_uri_local_name(raptor_uri *uri, const char *local_name)
{
  return (*raptor_current_uri_handler->new_uri_from_uri_local_name)(raptor_current_uri_context, uri, local_name);
}


static raptor_uri*
raptor_default_new_uri_relative_to_base(void *context,
                                        raptor_uri *base_uri,
                                        const char *uri_string) 
{
  char *new_uri;
  int new_uri_len=strlen(base_uri)+strlen(uri_string)+1;

  new_uri=(char*)LIBRDF_MALLOC(cstring, new_uri_len+1);
  if(!new_uri)
    return NULL;
  
  /* If URI string is empty, just copy base URI */
  if(!*uri_string) {
    strcpy(new_uri, (char*)base_uri);
    return new_uri;
  }

  raptor_uri_resolve_uri_reference((char*)base_uri, uri_string,
                                   new_uri, new_uri_len);
  return new_uri;
}


raptor_uri*
raptor_new_uri_relative_to_base(raptor_uri *base_uri, const char *uri_string) 
{
  return (*raptor_current_uri_handler->new_uri_relative_to_base)(raptor_current_uri_context, base_uri, uri_string);
}


raptor_uri*
raptor_new_uri_from_id(raptor_uri *base_uri, const char *id) 
{
  raptor_uri *new_uri;
  char *local_name;
  int len;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  LIBRDF_DEBUG2(raptor_new_uri_from_id, "Using ID %s\n", id);
#endif

  /* "#id\0" */
  len=1+strlen(id)+1;
  local_name=(char*)LIBRDF_MALLOC(cstring, len);
  if(!local_name)
    return NULL;
  *local_name='#';
  strcpy(local_name+1, id);
  new_uri=raptor_new_uri_relative_to_base(base_uri, local_name);
  LIBRDF_FREE(cstring, local_name);
  return new_uri;
}


static raptor_uri*
raptor_default_new_uri_for_rdf_concept(void *context, const char *name) 
{
  raptor_uri *new_uri;
  char *base_uri=RAPTOR_RDF_MS_URI;
  int base_uri_len=strlen(base_uri);
  int new_uri_len;

  base_uri_len=strlen(base_uri);
  new_uri_len=base_uri_len+strlen(name)+1;
  new_uri=(char*)LIBRDF_MALLOC(cstring, new_uri_len);
  if(!new_uri)
    return NULL;
  strcpy((char*)new_uri, base_uri);
  strcpy((char*)new_uri+base_uri_len, name);
  return new_uri;
}


raptor_uri*
raptor_new_uri_for_rdf_concept(const char *name) 
{
  return (*raptor_current_uri_handler->new_uri_for_rdf_concept)(raptor_current_uri_context, name);
}


static void
raptor_default_free_uri(void *context, raptor_uri *uri) 
{
  LIBRDF_FREE(raptor_uri, uri);
}


void
raptor_free_uri(raptor_uri *uri)
{
  return (*raptor_current_uri_handler->free_uri)(raptor_current_uri_context, uri);
}


static int
raptor_default_uri_equals(void *context, raptor_uri* uri1, raptor_uri* uri2)
{
  return strcmp((char*)uri1, (char*)uri2)==0;
}


int
raptor_uri_equals(raptor_uri* uri1, raptor_uri* uri2)
{
  return (*raptor_current_uri_handler->uri_equals)(raptor_current_uri_context, uri1, uri2);
}


static int
raptor_uri_is_absolute (const char *uri)
{
  int result = 0;

  if (*uri && isalpha (*uri)) {
    ++uri;


   /* 
    * RFC 2396 3.1 Scheme Component
    * scheme        = alpha *( alpha | digit | "+" | "-" | "." )
    */
    while (*uri && (isalnum (*uri)
                    || (*uri == '+') || (*uri == '-') || (*uri == '.')))
      ++uri;

    result = (*uri == ':');
  }

  return result;
}


static raptor_uri*
raptor_default_uri_copy(void *context, raptor_uri *uri)
{
  char *new_uri=(char*)LIBRDF_MALLOC(cstring, strlen((char*)uri)+1);
  if(!new_uri)
    return NULL;
  strcpy(new_uri, (char*)uri);
  return new_uri;
}


raptor_uri*
raptor_uri_copy(raptor_uri *uri) 
{
  return (*raptor_current_uri_handler->uri_copy)(raptor_current_uri_context, uri);
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
raptor_uri_parse (const char *uri, char *buffer, size_t len,
                  char **scheme, char **authority, 
                  char **path, char **query, char **fragment)
{
  const char *s = NULL;
  char *d = NULL;

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


/**
 * raptor_uri_resolve_uri_reference - Resolve a URI to a base URI
 * @base_uri: Base URI string
 * @reference_uri: Reference URI string
 * @buffer: Destination buffer URI
 * @length: Length of destination buffer
 * 
 **/
void
raptor_uri_resolve_uri_reference (const char *base_uri,
                                  const char *reference_uri,
                                  char *buffer, size_t length)
{
  char *base_buffer=NULL;
  char *reference_buffer=NULL;
  char *path_buffer=NULL;
  int reference_buffer_len;
  int base_uri_len;
  
  char *base_scheme;
  char *base_authority;
  char *base_path;
  char *base_query;
  char *base_fragment;

  char *reference_scheme;
  char *reference_authority;
  char *reference_path;
  char *reference_query;
  char *reference_fragment;

  char *result_scheme = NULL;
  char *result_authority = NULL;
  char *result_path = NULL;

  *buffer = '\0';

  reference_buffer_len=strlen(reference_uri)+1;
  reference_buffer=(char*)LIBRDF_MALLOC(cstring, reference_buffer_len);
  if(!reference_buffer)
    goto resolve_tidy;
  
  raptor_uri_parse (reference_uri, reference_buffer, reference_buffer_len,
                    &reference_scheme, &reference_authority,
                    &reference_path, &reference_query, &reference_fragment);

  /* is reference URI "" or "#frag"? */
  if (!reference_scheme && !reference_authority
      && !reference_path && !reference_query) {
    strcpy (buffer, base_uri);

    if (reference_fragment) {
      strcat (buffer, "#");
      strcat (buffer, reference_fragment);
    }
    goto resolve_tidy;
  }
  
  /* reference has a scheme - is an absolute URI */
  if (reference_scheme) {
    strcpy (buffer, reference_uri);
    goto resolve_tidy;
  }
  

  /* now the reference URI must be schemeless, i.e. relative */
  base_uri_len=strlen(base_uri);
  base_buffer=(char*)LIBRDF_MALLOC(cstring, base_uri_len+1);
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
    goto resolve_end;
  }

  /* no - so now we have path (maybe with query, fragment) relative to base */
  result_authority = base_authority;
    

  if (reference_path && reference_path[0] == '/') {
    /* reference path is absolute */
    result_path = reference_path;
  } else {
    /* need to resolve relative path */
    char *p = NULL;

    /* Make result path be path buffer; initialise it to empty */
    int path_buffer_len=1;

    if(base_path)
      path_buffer_len +=strlen(base_path);
    else {
      base_path="/"; /* static, but copied and not free()d  */
      path_buffer_len++;
    }

    if(reference_path)
      path_buffer_len +=strlen(reference_path);

    path_buffer=(char*)LIBRDF_MALLOC(cstring, path_buffer_len);
    if(!path_buffer)
      goto resolve_tidy;
    result_path = path_buffer;
    path_buffer[0] = '\0';
    
    p = strrchr (base_path, '/');
    if (p) {
      /* Found a /, copy everything before that to path_buffer */
      char *s = base_path;
      char *d = path_buffer;
      
      while (s <= p)
        *d++ = *s++;
      
      *d++ = '\0';
    }

    if (reference_path)
      strcat (path_buffer, reference_path);


    /* NEW BLOCK - only ANSI C allows this */
    {
      /* remove all occurrences of "./" */
      
      char *p2 = path_buffer;
      char *s = path_buffer;
      
      while (*s) {
        if (*s == '/') {

          if (p2 == (s - 1) && *p2 == '.') {
            char *d = p2;
            
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
      
      char *s = path_buffer;
      /* These form a small stack of path components */
      char *p3 = NULL; /* current component */
      char *p2 = NULL; /* previous component */
      char *p0 = NULL; /* before p2 */
      
      for (;*s; ++s) {

        if (*s != '/') {
          if (!p3) {
            /* Empty stack; initialise */
            p3 = s;
          } else if (!p2) {
            /* Add 2nd component */
            p2 = s;
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
            char *d = p3;
            
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
    strcpy (buffer, result_scheme);
    strcat (buffer, ":");
  }
  
  if (result_authority) {
    strcat (buffer, "//");
    strcat (buffer, result_authority);
  }
  
  if (result_path)
    strcat (buffer, result_path);
  
  if (reference_query) {
    strcat (buffer, "?");
    strcat (buffer, reference_query);
  }
  
  if (reference_fragment) {
    strcat (buffer, "#");
    strcat (buffer, reference_fragment);
  }

  resolve_tidy:
  if(path_buffer)
    LIBRDF_FREE(cstring, path_buffer);
  if(base_buffer)
    LIBRDF_FREE(cstring, base_buffer);
  if(reference_buffer)
    LIBRDF_FREE(cstring, reference_buffer);
}



/**
 * raptor_uri_filename_to_uri_string - Converts a filename to a file: URI
 * @filename: The filename to convert
 * 
 * Handles the OS-specific escaping on turning filenames into URIs
 * and returns a new buffer that the caller must free().
 *
 * Return value: A newly allocated string with the URI or NULL on failure
 **/
char *
raptor_uri_filename_to_uri_string(const char *filename) 
{
  char *buffer;
#ifdef WIN32
  const char *from;
  char *to;
#else
  char path[PATH_MAX];
#endif
  /*     file: (filename) \0 */
  int len=5+strlen(filename)+1;
  
#ifdef WIN32
/*
 * On WIN32, filenames turn into
 *   "file://" + translated filename
 * where the translation is \\ turns into /
 * and if the filename does not start with '\', it is relative
 * in which case, a . is appended to the authority
 *
 * e.g
 *  FILENAME              URI
 *  c:\windows\system     file://c:/windows/system
 *  \\server\dir\file.doc file://server/dir/file.doc
 *  a:foo                 file://a:./foo
 *
 * There are also UNC names \\server\share\blah
 * that turn into file:///server/share/blah
 * using the above algorithm.
 */
  len+=2; /* // */
  if(*filename != '\\')
    len+=2; /* relative filename - add ./ */

#else
/* others - unix ... ? */

/*
 * "file://" + filename
 */
  len+=2;
  if(*filename != '/') {
    if(!getcwd(path, PATH_MAX))
      return NULL;
    strcat(path, "/");
    strcat(path, filename);
    filename=(const char*)path;
  }
#endif

  buffer=(char*)LIBRDF_MALLOC(cstring, len);
  if(!buffer)
    return NULL;

#ifdef WIN32
  strcpy(buffer, "file://");
  from=filename;
  to=buffer+7;
  if(*from == '\\' && from[1] == '\\')
    from+=2;
  while(*from) {
    char c=*from++;
    if (c == '\\')
      *to++='/';
    else if(c == ':') {
      *to++=c;
      if(*from != '\\') {
        *to++='.';
        *to++='/';
      }
    } else
      *to++=c;
  }
  *to='\0';
#else
  strcpy(buffer, "file://");
  strcpy(buffer+7, filename);
#endif
  
  return buffer;
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
raptor_uri_uri_string_to_filename(const char *uri_string) 
{
  char *buffer;
  char *filename;
  int uri_string_len=strlen(uri_string);
  int len=0;
  char *scheme, *authority, *path, *query, *fragment;
#ifdef WIN32
  char *p, *from, *to;
  int is_relative_path=0;
#endif

  buffer=(char*)LIBRDF_MALLOC(cstring, uri_string_len+1);
  if(!buffer)
    return NULL;
  
  raptor_uri_parse (uri_string, buffer, uri_string_len,
                    &scheme, &authority, &path, &query, &fragment);

  if(!scheme || strcasecmp(scheme, "file")) {
    LIBRDF_FREE(cstring, buffer);
    return NULL;
  }

  if(authority && !strcasecmp(authority, "localhost")) {
    authority=NULL;
  }

  /* See raptor_uri_filename_to_uri_string for details of the mapping */
#ifdef WIN32
  if(authority) {
    len+=strlen(authority);
    p=strchr(authority, ':');
    if(p) {
      /* Either 
       *   "a:" like in file://a:/... 
       * or
       *   "a:." like in file://a:./foo
       * giving device-relative path a:foo
       */
      if(p[1]=='.') {
        p[1]='\0';
        is_relative_path=1;
      }
    } else {
      /* Otherwise UNC like "server" in file://server//share */
      len+=2; /* \\ between "server" and "share" */
    }
  } /* end if authority */
  if(!is_relative_path)
    len++;/* for \ between authority and path */
  len--; /* for removing leading / off path */
#endif
  len+=strlen(path);


  filename=(char*)LIBRDF_MALLOC(cstring, len+1);
  if(!filename) {
    LIBRDF_FREE(cstring, buffer);
    return NULL;
  }


#ifdef WIN32
  *filename='\0';
  if(authority) {
    /* p was set above to ':' in authority */
    if(!p)
      strcpy(filename, "\\\\");
    strcat(filename, authority);
  }

  if(!is_relative_path)
    strcat(filename,"\\");

  /* find end of filename */
  to=filename;
  while(*to++)
    ;
  to--;
  
  /* copy path after leading \ */
  from=path+1;
  while(*from) {
    char c=*from++;
    if(c == '/')
      *to++ ='\\';
    else
      *to++ =c;
  }
  *to='\0';
#else
  strcpy(filename, path);
#endif


  LIBRDF_FREE(cstring, buffer);

  return filename;
}


/**
 * raptor_uri_is_file_uri - Check if a URI string is a a file: URI
 * @uri_string: The URI string to check
 * 
 * Return value: Non zero if URI string is a file: URI
 **/
int
raptor_uri_is_file_uri(const char* uri_string) {
  return strncasecmp(uri_string, "file:", 5)==0;
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

/* one more prototype */
int main(int argc, char *argv[]);


static int
assert_resolve_uri (const char *base_uri,
		    const char *reference_uri, const char *absolute_uri)
{
  char buffer[256];

  raptor_uri_resolve_uri_reference (base_uri, reference_uri,
                                    buffer, sizeof (buffer));

  if (strcmp (buffer, absolute_uri))
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
  char *uri;

  uri=raptor_uri_filename_to_uri_string(filename);

  if (!uri || strcmp (uri, reference_uri))
    {
      fprintf(stderr, "FAIL raptor_uri_filename_to_uri_string filename %s gave URI %s != %s\n",
              filename, uri, reference_uri);
      if(uri)
        LIBRDF_FREE(cstring, uri);
      return 1;
    }

  LIBRDF_FREE(cstring, uri);
  return 0;
}


static int
assert_uri_to_filename (const char *uri, const char *reference_filename)
{
  char *filename;

  filename=raptor_uri_uri_string_to_filename(uri);

  if (!filename || strcmp (filename, reference_filename))
    {
      fprintf(stderr, "FAIL raptor_uri_uri_string_to_filename URI %s gave filename %s != %s\n",
              uri, filename, reference_filename);
      if(filename)
        LIBRDF_FREE(cstring, filename);
      return 1;
    }

  LIBRDF_FREE(cstring, filename);
  return 0;
}


int
main(int argc, char *argv[]) 
{
  const char *base_uri = "http://a/b/c/d;p?q";
  int failures=0;
  
  fprintf(stderr, "raptor_uri_resolve_uri_reference: Testing with base URI %s\n", base_uri);
  

  failures += assert_resolve_uri (base_uri, "g:h", "g:h");
  failures += assert_resolve_uri (base_uri, "g", "http://a/b/c/g");
  failures += assert_resolve_uri (base_uri, "./g", "http://a/b/c/g");
  failures += assert_resolve_uri (base_uri, "g/", "http://a/b/c/g/");
  failures += assert_resolve_uri (base_uri, "/g", "http://a/g");
  failures += assert_resolve_uri (base_uri, "//g", "http://g");
  failures += assert_resolve_uri (base_uri, "?y", "http://a/b/c/?y");
  failures += assert_resolve_uri (base_uri, "g?y", "http://a/b/c/g?y");
  failures += assert_resolve_uri (base_uri, "#s", "http://a/b/c/d;p?q#s");
  failures += assert_resolve_uri (base_uri, "g#s", "http://a/b/c/g#s");
  failures += assert_resolve_uri (base_uri, "g?y#s", "http://a/b/c/g?y#s");
  failures += assert_resolve_uri (base_uri, ";x", "http://a/b/c/;x");
  failures += assert_resolve_uri (base_uri, "g;x", "http://a/b/c/g;x");
  failures += assert_resolve_uri (base_uri, "g;x?y#s", "http://a/b/c/g;x?y#s");
  failures += assert_resolve_uri (base_uri, ".", "http://a/b/c/");
  failures += assert_resolve_uri (base_uri, "./", "http://a/b/c/");
  failures += assert_resolve_uri (base_uri, "..", "http://a/b/");
  failures += assert_resolve_uri (base_uri, "../", "http://a/b/");
  failures += assert_resolve_uri (base_uri, "../g", "http://a/b/g");
  failures += assert_resolve_uri (base_uri, "../..", "http://a/");
  failures += assert_resolve_uri (base_uri, "../../", "http://a/");
  failures += assert_resolve_uri (base_uri, "../../g", "http://a/g");

  failures += assert_resolve_uri (base_uri, "", "http://a/b/c/d;p?q");

  failures += assert_resolve_uri (base_uri, "../../../g", "http://a/../g");
  failures += assert_resolve_uri (base_uri, "../../../../g", "http://a/../../g");

  failures += assert_resolve_uri (base_uri, "/./g", "http://a/./g");
  failures += assert_resolve_uri (base_uri, "/../g", "http://a/../g");
  failures += assert_resolve_uri (base_uri, "g.", "http://a/b/c/g.");
  failures += assert_resolve_uri (base_uri, ".g", "http://a/b/c/.g");
  failures += assert_resolve_uri (base_uri, "g..", "http://a/b/c/g..");
  failures += assert_resolve_uri (base_uri, "..g", "http://a/b/c/..g");

  failures += assert_resolve_uri (base_uri, "./../g", "http://a/b/g");
  failures += assert_resolve_uri (base_uri, "./g/.", "http://a/b/c/g/");
  failures += assert_resolve_uri (base_uri, "g/./h", "http://a/b/c/g/h");
  failures += assert_resolve_uri (base_uri, "g/../h", "http://a/b/c/h");
  failures += assert_resolve_uri (base_uri, "g;x=1/./y", "http://a/b/c/g;x=1/y");
  failures += assert_resolve_uri (base_uri, "g;x=1/../y", "http://a/b/c/y");

  failures += assert_resolve_uri (base_uri, "g?y/./x", "http://a/b/c/g?y/./x");
  failures += assert_resolve_uri (base_uri, "g?y/../x", "http://a/b/c/g?y/../x");
  failures += assert_resolve_uri (base_uri, "g#s/./x", "http://a/b/c/g#s/./x");
  failures += assert_resolve_uri (base_uri, "g#s/../x", "http://a/b/c/g#s/../x");

  failures += assert_resolve_uri (base_uri, "http:g", "http:g");

  failures += assert_resolve_uri (base_uri, "g/../../../h", "http://a/h");


#ifdef WIN32
  failures += assert_filename_to_uri ("c:\\windows\\system", "file://c:/windows/system");
  failures += assert_filename_to_uri ("\\\\server\\dir\\file.doc", "file://server/dir/file.doc");
  failures += assert_filename_to_uri ("a:foo", "file://a:./foo");


  failures += assert_uri_to_filename ("file://c:/windows/system", "c:\\windows\\system");
  failures += assert_uri_to_filename ("file://server/dir/file.doc", "\\\\server\\dir\\file.doc");
  failures += assert_uri_to_filename ("file://a:./foo", "a:foo");
#else

  failures += assert_filename_to_uri ("/path/to/file", "file:///path/to/file");
  failures += assert_uri_to_filename ("file:///path/to/file", "/path/to/file");

  /* Need to test this with a real dir (preferably not /)
   * so go with assuming /tmp exists on all unixen.  This is just a
   * test so pretty likely to work on all development systems that
   * are not WIN32
   */
  if(chdir("/tmp"))
    fprintf(stderr, "WARNING: %s: chdir(\"/tmp\"/) failed - not testing relative files\n",
            argv[0]);
  else
    failures += assert_filename_to_uri ("foo", "file:///tmp/foo");
 
#endif

  return failures ;
}

#endif
