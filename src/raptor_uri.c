/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_uri.c - Raptor URI resolving implementation
 *
 * $Id$
 *
 * Originally from code
 * Copyright (C) 2000 Jason Diamond - http://injektilo.org/
 * from http://injektilo.org/rdf/repat.html
 *
 * Updated and integrated into Raptor:
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
#ifdef LIBRDF_INTERNAL
#include <rdf_config.h>
#else
#include <config.h>
#endif
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

extern int errno;

#ifdef LIBRDF_INTERNAL
/* if inside Redland */

#ifdef RAPTOR_DEBUG
#define LIBRDF_DEBUG 1
#endif

#include <librdf.h>

#else
/* else standalone */

#define LIBRDF_MALLOC(type, size) malloc(size)
#define LIBRDF_CALLOC(type, size, count) calloc(size, count)
#define LIBRDF_FREE(type, ptr)   free((void*)ptr)

#endif


/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#undef HAVE_STDLIB_H
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#undef HAVE_STDLIB_H
#endif


/* Raptor includes */
#include <raptor.h>



void raptor_uri_resolve_uri_reference (const char *base_uri, const char *reference_uri, char *buffer, size_t length);


/* Prototypes for local functions */
static int raptor_uri_is_absolute (const char *uri);

static void raptor_uri_parse (const char *uri, char *buffer, size_t len, char **scheme, char **authority, char **path, char **query, char **fragment);



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


  /* Extract the authority (e.g. hostname in http, ftp etc.) */
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
  char base_buffer[256];
  char reference_buffer[256];
  char path_buffer[256];

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

  raptor_uri_parse (reference_uri, reference_buffer, sizeof (reference_buffer),
                    &reference_scheme, &reference_authority,
                    &reference_path, &reference_query, &reference_fragment);

  if (!reference_scheme && !reference_authority
      && !reference_path && !reference_query) {
    strcpy (buffer, base_uri);

    if (reference_fragment) {
      strcat (buffer, "#");
      strcat (buffer, reference_fragment);
    }
  } else if (reference_scheme) {
    strcpy (buffer, reference_uri);
  } else { /* ZZ -- FIXME */
    raptor_uri_parse (base_uri, base_buffer, sizeof (base_buffer),
                      &base_scheme, &base_authority,
                      &base_path, &base_query, &base_fragment);
    
    result_scheme = base_scheme;
    
    if (reference_authority) {
      result_authority = reference_authority;
    } else { /* YY */
      result_authority = base_authority;
      
      if (reference_path && reference_path[0] == '/') {
        /* reference path is absolute */
        result_path = reference_path;

      } else { /* XX -  need to resolve relative path */
        char *p = NULL;

        /* Make result path be path buffer; initialise it to empty */

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


        /* NEW BLOCK - only ANSI C allows this - FIXME */
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

        

        /* NEW BLOCK - only ANSI C allows this - FIXME */
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

      } /* end if (XX) */

    } /* end if (YY) */

    
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

  } /* end if (ZZ) */

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

  return failures ;
}

#endif
