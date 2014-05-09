/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_locator.c - Raptor parsing locator functions
 *
 * Copyright (C) 2002-2006, David Beckett http://www.dajobe.org/
 * Copyright (C) 2002-2006, University of Bristol, UK http://www.bristol.ac.uk/
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


/**
 * raptor_locator_print:
 * @locator: #raptor_locator to print
 * @stream: stream to print to
 *
 * Print a raptor locator to a stream.
 * 
 * Return value: non-0 on failure
 **/
int
raptor_locator_print(raptor_locator* locator, FILE *stream)
{
  if(!locator)
    return 1;

  if(locator->uri)
    fprintf(stream, "URI %s", raptor_uri_as_string(locator->uri));
  else if(locator->file)
    fprintf(stream, "file %s", locator->file);
  else
    return 0;
  if(locator->line >= 0) {
    fprintf(stream, ":%d", locator->line);
    if(locator->column >= 0)
      fprintf(stream, " column %d", locator->column);
  }

  return 0;
}


/**
 * raptor_locator_format:
 * @buffer: buffer to store format
 * @length: size of buffer (excluding NUL)
 * @locator: #raptor_locator to format
 *
 * Format a raptor locator as a string.
 * 
 * If buffer is NULL or @length is insufficient for the size of
 * the locator, returns the number of additional bytes required
 * in the buffer to write the locator.  Writes a terminating '\0'.
 * 
 * Return value: 0 on success, >0 if additional bytes required in buffer, <0 on failure
 **/
int
raptor_locator_format(char *buffer, size_t length, raptor_locator* locator) 
{
  size_t bufsize = 0;
  const char* label_str;
  size_t label_len = 0;
  const char* value_str = NULL;
  size_t value_len;
  
  if(!locator)
    return -1;

  #define URI_STR "URI "
  #define URI_STR_LEN 4  /* strlen(URI_STR) */
  #define FILE_STR "file "
  #define FILE_STR_LEN 5  /* strlen(FILE_STR) */
  #define COLUMN_STR " column "
  #define COLUMN_STR_LEN 8  /* strlen(COLUMN_STR) */

  if(locator->uri) {
    label_str = URI_STR;
    label_len = URI_STR_LEN;
    value_str = (const char*)raptor_uri_as_counted_string(locator->uri,
                                                          &value_len);
  } else if(locator->file) {
    label_str = FILE_STR;
    label_len = FILE_STR_LEN;
    value_str = locator->file;
    value_len = strlen(value_str);
  } else
    return -1;

  bufsize = label_len + value_len;

  if(locator->line > 0) {
    bufsize += 1 + raptor_format_integer(NULL, 0, locator->line, /* base */ 10,
                                         -1, '\0');
    if(locator->column >= 0)
      bufsize += COLUMN_STR_LEN +
        raptor_format_integer(NULL, 0, locator->column, /* base */ 10,
                              -1, '\0');
  }
  
  if(!buffer || !length || length < (bufsize + 1)) /* +1 for NUL */
    return RAPTOR_BAD_CAST(int, bufsize);
  

  memcpy(buffer, label_str, label_len);
  buffer += label_len;
  memcpy(buffer, value_str, value_len);
  buffer += value_len;
  
  if(locator->line > 0) {
    *buffer ++= ':';
    buffer += raptor_format_integer(buffer, length,
                                    locator->line, /* base */ 10,
                                    -1, '\0');
    if(locator->column >= 0) {
      memcpy(buffer, COLUMN_STR, COLUMN_STR_LEN);
      buffer += COLUMN_STR_LEN;
      buffer += raptor_format_integer(buffer, length,
                                      locator->column, /* base */ 10,
                                      -1, '\0');
    }
  }
  *buffer = '\0';

  return 0;
}


/**
 * raptor_locator_line:
 * @locator: locator
 *
 * Get line number from locator.
 *
 * Return value: integer line number, or -1 if there is no line number available
 **/
int
raptor_locator_line(raptor_locator *locator)
{
  if(!locator)
    return -1;
  return locator->line;
}


/**
 * raptor_locator_column:
 * @locator: locator
 *
 * Get column number from locator.
 *
 * Return value: integer column number, or -1 if there is no column number available
 **/
int
raptor_locator_column(raptor_locator *locator)
{
  if(!locator)
    return -1;
  return locator->column;
}


/**
 * raptor_locator_byte:
 * @locator: locator
 *
 * Get the locator byte offset from locator.
 *
 * Return value: integer byte number, or -1 if there is no byte offset available
 **/
int
raptor_locator_byte(raptor_locator *locator)
{
  if(!locator)
    return -1;
  return locator->byte;
}


/**
 * raptor_locator_file:
 * @locator: locator
 *
 * Get file name from locator.
 *
 * Return value: string file name, or NULL if there is no filename available
 **/
const char *
raptor_locator_file(raptor_locator *locator)
{
  if(!locator)
    return NULL;
  return locator->file;
}


/**
 * raptor_locator_uri:
 * @locator: locator
 *
 * Get URI from locator.
 *
 * Returns a pointer to a shared string version of the URI in
 * the locator.  This must be copied if it is needed.
 *
 * Return value: string URI, or NULL if there is no URI available
 **/
const char *
raptor_locator_uri(raptor_locator *locator)
{
  if(!locator)
    return NULL;

  return (const char*)raptor_uri_as_string(locator->uri);
}
