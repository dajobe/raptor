/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_internal.h - Redland Parser Toolkit for RDF (Raptor) internals
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



#ifndef RAPTOR_INTERNAL_H
#define RAPTOR_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RAPTOR_INTERNAL

#ifdef RAPTOR_IN_REDLAND
#define LIBRDF_INTERNAL
#endif
  
#ifdef LIBRDF_INTERNAL
#ifdef LIBRDF_DEBUG
#define RAPTOR_DEBUG 1
#endif

#define IS_RDF_MS_CONCEPT(name, uri, local_name) librdf_uri_equals(uri, librdf_concept_uris[LIBRDF_CONCEPT_MS_##local_name])
#define RAPTOR_URI_AS_STRING(uri) (librdf_uri_as_string(uri))
#define RAPTOR_URI_AS_FILENAME(uri) (librdf_uri_as_filename(uri))
#undef RAPTOR_URI_TO_FILENAME
#define RAPTOR_FREE_URI(uri) librdf_free_uri(uri)
#else
/* else standalone */

#define LIBRDF_MALLOC(type, size) malloc(size)
#define LIBRDF_CALLOC(type, size, count) calloc(size, count)
#define LIBRDF_FREE(type, ptr)   free((void*)ptr)

#define IS_RDF_MS_CONCEPT(name, uri, local_name) !strcmp(name, #local_name)
#define RAPTOR_URI_AS_STRING(uri) ((const char*)uri)
#undef RAPTOR_URI_AS_FILENAME
#define RAPTOR_URI_TO_FILENAME(uri) (raptor_file_uri_to_filename(uri))
#define RAPTOR_FREE_URI(uri) LIBRDF_FREE(cstring, uri)

#ifdef RAPTOR_DEBUG
/* Debugging messages */
#define LIBRDF_DEBUG1(function, msg) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function); } while(0)
#define LIBRDF_DEBUG2(function, msg, arg1) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1);} while(0)
#define LIBRDF_DEBUG3(function, msg, arg1, arg2) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2);} while(0)
#define LIBRDF_DEBUG4(function, msg, arg1, arg2, arg3) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2, arg3);} while(0)
#define LIBRDF_DEBUG4(function, msg, arg1, arg2, arg3) do {fprintf(stderr, "%s:%d:%s: " msg, __FILE__, __LINE__, #function, arg1, arg2, arg3);} while(0)

#else
/* DEBUGGING TURNED OFF */

/* No debugging messages */
#define LIBRDF_DEBUG1(function, msg)
#define LIBRDF_DEBUG2(function, msg, arg1)
#define LIBRDF_DEBUG3(function, msg, arg1, arg2)
#define LIBRDF_DEBUG4(function, msg, arg1, arg2, arg3)

#endif


/* Fatal errors - always happen */
#define LIBRDF_FATAL1(function, msg) do {fprintf(stderr, "%s:%d:%s: fatal error: " msg, __FILE__, __LINE__ , #function); abort();} while(0)
#define LIBRDF_FATAL2(function, msg,arg) do {fprintf(stderr, "%s:%d:%s: fatal error: " msg, __FILE__, __LINE__ , #function, arg); abort();} while(0)

#endif

/* end of RAPTOR_INTERNAL */
#endif


#ifdef __cplusplus
}
#endif

#endif
