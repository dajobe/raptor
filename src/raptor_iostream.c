/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_iostream.c - Raptor I/O-stream class for abstracting I/O
 *
 * $Id$
 *
 * Copyright (C) 2004, David Beckett http://purl.org/net/dajobe/
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

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


#ifndef STANDALONE

struct raptor_iostream_s
{
  void *context;
  raptor_iostream_handler* handler;
  size_t bytes;
  int ended;
};



/* prototypes for local functions */


/**
 * raptor_new_iostream_from_handler - Create a new iostream over a user-defined handler
 * @context: pointer to context information to pass in to calls
 * @handler: pointer to handler methods
 *
 * Return value: new &raptor_iostream object or NULL on failure
 **/
raptor_iostream*
raptor_new_iostream_from_handler(void *context, raptor_iostream_handler *handler) {
  raptor_iostream* iostr=(raptor_iostream*)RAPTOR_CALLOC(raptor_iostream, 1, sizeof(raptor_iostream));
  if(!iostr)
    return NULL;

  iostr->handler=handler;
  iostr->context=(void*)context;

  if(iostr->handler->init && 
     iostr->handler->init(iostr->context)) {
    RAPTOR_FREE(raptor_iostream, iostr);
    return NULL;
  }
  return iostr;
}



/* Local handlers for writing to a throw-away sink */

static int
raptor_sink_iostream_write_byte(void *context, const int byte)
{
  return 0;
}

static int
raptor_sink_iostream_write_bytes(void *context,
                                 const void *ptr, size_t size, size_t nmemb)
{
  return 0;
}


static raptor_iostream_handler raptor_iostream_sink_handler={
  NULL, /* init */
  NULL, /* finish */
  raptor_sink_iostream_write_byte,
  raptor_sink_iostream_write_bytes,
  NULL, /* write_end */
};


/**
 * raptor_new_iostream_to_sink - Create a new iostream to a sink
 * 
 * Return value: new &raptor_iostream object or NULL on failure
 **/
raptor_iostream*
raptor_new_iostream_to_sink(void)
{
  return raptor_new_iostream_from_handler(NULL, &raptor_iostream_sink_handler);
}


/* Local handlers for writing to a filename */

static int
raptor_filename_iostream_write_byte(void *context, const int byte)
{
  FILE* handle=(FILE*)context;
  return (fputc(byte, handle) == byte);
}

static int
raptor_filename_iostream_write_bytes(void *context,
                                     const void *ptr, size_t size, size_t nmemb)
{
  FILE* handle=(FILE*)context;
  return (fwrite(ptr, size, nmemb, handle) == nmemb);
}

static void
raptor_filename_iostream_write_end(void *context)
{
  FILE* handle=(FILE*)context;
  fclose(handle);
}


static raptor_iostream_handler raptor_iostream_filename_handler={
  NULL, /* init */
  NULL, /* finish */
  raptor_filename_iostream_write_byte,
  raptor_filename_iostream_write_bytes,
  raptor_filename_iostream_write_end
};


/**
 * raptor_new_iostream_to_filename - Create a new iostream to a filename
 * 
 * Return value: new &raptor_iostream object or NULL on failure
 **/
raptor_iostream*
raptor_new_iostream_to_filename(const char *filename)
{
  FILE *handle;
  raptor_iostream* iostr;

  handle=fopen(filename, "wb");
  if(!handle)
    return NULL;
  
  iostr=(raptor_iostream*)RAPTOR_CALLOC(raptor_iostream, 1, sizeof(raptor_iostream));
  if(!iostr)
    return NULL;

  iostr->handler=&raptor_iostream_filename_handler;
  iostr->context=(void*)handle;

  if(iostr->handler->init && 
     iostr->handler->init(iostr->context)) {
    RAPTOR_FREE(raptor_iostream, iostr);
    return NULL;
  }
  return iostr;
}


static raptor_iostream_handler raptor_iostream_file_handler={
  NULL, /* init */
  NULL, /* finish */
  raptor_filename_iostream_write_byte,
  raptor_filename_iostream_write_bytes,
  NULL, /* write_end */
};


/**
 * raptor_new_iostream_to_file_handle - Create a new iostream to a FILE*
 * @handle: FILE* handle to write to
 * 
 * The handle must already be open for writing.
 * NOTE: This does not fclose the handle when it is finished.
 *
 * Return value: new &raptor_iostream object or NULL on failure
 **/
raptor_iostream*
raptor_new_iostream_to_file_handle(FILE *handle)
{
  raptor_iostream* iostr;

  if(!handle)
    return NULL;

  iostr=(raptor_iostream*)RAPTOR_CALLOC(raptor_iostream, 1, sizeof(raptor_iostream));
  if(!iostr)
    return NULL;

  iostr->handler=&raptor_iostream_file_handler;
  iostr->context=(void*)handle;

  if(iostr->handler->init && iostr->handler->init(iostr->context)) {
    RAPTOR_FREE(raptor_iostream, iostr);
    return NULL;
  }
  return iostr;
}



struct raptor_string_iostream_context {
  raptor_stringbuffer *sb;
  void *(*malloc_handler)(size_t size);
  void **string_p;
  size_t *length_p;
};


/* Local handlers for writing to a string */

static int
raptor_string_iostream_init(void *context) 
{
  return 0;
}

static void
raptor_string_iostream_finish(void *context) 
{
  struct raptor_string_iostream_context* con=(struct raptor_string_iostream_context*)context;
  size_t len=raptor_stringbuffer_length(con->sb);
  void *str=NULL;

  *con->string_p=NULL;
  if(con->length_p)
    *con->length_p=len;
  
  if(len) {
    str=(void*)con->malloc_handler(len+1);
    if(str) {
      raptor_stringbuffer_copy_to_string(con->sb, (unsigned char*)str, len+1);
      *con->string_p=str;
    }
  }

  if(!str && con->length_p)
    *con->length_p=0;
  
  raptor_free_stringbuffer(con->sb);
  RAPTOR_FREE(raptor_string_iostream_context, con);
  return;
}

static int
raptor_string_iostream_write_byte(void *context, const int byte)
{
  struct raptor_string_iostream_context* con=(struct raptor_string_iostream_context*)context;
  unsigned char buf=(unsigned char)byte;
  return raptor_stringbuffer_append_counted_string(con->sb, &buf, 1, 1);
}


static int
raptor_string_iostream_write_bytes(void *context,
                                   const void *ptr, size_t size, size_t nmemb)
{
  struct raptor_string_iostream_context* con=(struct raptor_string_iostream_context*)context;

  return raptor_stringbuffer_append_counted_string(con->sb, (const unsigned char*)ptr, size * nmemb, 1);
}

static raptor_iostream_handler raptor_iostream_string_handler={
  raptor_string_iostream_init,
  raptor_string_iostream_finish,
  raptor_string_iostream_write_byte,
  raptor_string_iostream_write_bytes,
  NULL /* write_end */
};


/**
 * raptor_new_iostream_to_string - Create a new iostream to a string
 * @string_p: pointer to location to hold string
 * @length_p: pointer to location to hold length of string (or NULL)
 * @malloc_handler: pointer to malloc to use to make string (or NULL)
 *
 * If malloc_handler is null, raptor will allocate it using it's
 * own memory allocator.  *string_p is set to NULL on failure (and
 * *length_p to 0 if length_p is not NULL).
 * 
 * Return value: new &raptor_iostream object or NULL on failure
 **/
raptor_iostream*
raptor_new_iostream_to_string(void **string_p, size_t *length_p,
                              void *(*malloc_handler)(size_t size))
{
  raptor_iostream* iostr;
  struct raptor_string_iostream_context* con;
  
  iostr=(raptor_iostream*)RAPTOR_CALLOC(raptor_iostream, 1, sizeof(raptor_iostream));
  if(!iostr)
    return NULL;

  con=(struct raptor_string_iostream_context*)RAPTOR_CALLOC(raptor_string_iostream_context, 1, sizeof(struct raptor_string_iostream_context));
  if(!con) {
    RAPTOR_FREE(raptor_iostream, iostr);
    return NULL;
  }

  con->sb=raptor_new_stringbuffer();
  if(!con->sb) {
    RAPTOR_FREE(raptor_iostream, iostr);
    RAPTOR_FREE(raptor_string_iostream_context, con);
    return NULL;
  }

  con->string_p=string_p;
  *string_p=NULL;

  con->length_p=length_p;  
  if(length_p)
    *length_p=0;

  if(malloc_handler)
    con->malloc_handler=malloc_handler;
  else
    con->malloc_handler=raptor_alloc_memory;
  
  iostr->handler=&raptor_iostream_string_handler;
  iostr->context=(void*)con;

  if(iostr->handler->init && iostr->handler->init(iostr->context)) {
    raptor_free_iostream(iostr);
    return NULL;
  }
  return iostr;
}


/**
 * raptor_free_iostream - Destroy a iostream
 * 
 **/
void
raptor_free_iostream(raptor_iostream *iostr)
{
  if(!iostr->ended)
    raptor_iostream_write_end(iostr);

  if(iostr->handler->finish)
    iostr->handler->finish(iostr->context);

  RAPTOR_FREE(raptor_iostream, iostr);
}



/**
 * raptor_iostream_write_byte - Write a byte to the iostream
 * @iostr: raptor iostream
 * @byte: byte to write
 *
 * Return value: non-0 on failure
 **/
int
raptor_iostream_write_byte(raptor_iostream *iostr, 
                           const int byte)
{
  iostr->bytes++;

  if(iostr->ended)
    return 1;
  if(!iostr->handler->write_byte)
    return 1;
  return iostr->handler->write_byte(iostr->context, byte);
}


/**
 * raptor_iostream_write_bytes - Write bytes to the iostream
 * @iostr: raptor iostream
 * @ptr: start of objects to write
 * @size: size of object
 * @nmemb: number of objects
 *
 * Return value: number of objects written or less than nmemb or 0 on failure
 **/
int
raptor_iostream_write_bytes(raptor_iostream *iostr,
                            const void *ptr, size_t size, size_t nmemb)
{
  iostr->bytes += (size*nmemb);
  
  if(iostr->ended)
    return 1;
  if(!iostr->handler->write_bytes)
    return 0;
  return iostr->handler->write_bytes(iostr->context, ptr, size, nmemb);
}


/**
 * raptor_iostream_write_string - Write a NUL-terminated string to the iostream
 * @iostr: raptor iostream
 * @str: string
 *
 * Return value: non-0 on failure
 **/
int
raptor_iostream_write_string(raptor_iostream *iostr, const void *string)
{
  size_t len=strlen((const char*)string);
  return (raptor_iostream_write_bytes(iostr, string, 1, len) != (int)len);
}


/**
 * raptor_iostream_write_counted_string - Write a counted string to the iostream
 * @iostr: raptor iostream
 * @string: string
 * @length: string length
 *
 * Return value: non-0 on failure
 **/
int
raptor_iostream_write_counted_string(raptor_iostream *iostr, const void *string, size_t length) 
{
  return (raptor_iostream_write_bytes(iostr, string, 1, length) != (int)length);
}


/**
 * raptor_iostream_write_end - End writing to the iostream
 *
 * @iostr: raptor iostream
 **/
void
raptor_iostream_write_end(raptor_iostream *iostr)
{
  if(iostr->ended)
    return;
  if(iostr->handler->write_end)
    iostr->handler->write_end(iostr->context);
  iostr->ended=1;
}


/**
 * raptor_iostream_get_bytes_written_count - Get the number of bytes written to the iostream
 * @iostr: raptor iostream
 *
 * Return value: number of bytes written or 0 if non written so far
 **/
size_t
raptor_iostream_get_bytes_written_count(raptor_iostream *iostr)
{
  return iostr->bytes;
}


/**
 * raptor_iostream_write_decimal - Write an integer in decimal to the iostream
 * @iostr: raptor iostream
 * @integer: integer to format as decimal
 * 
 * Return value: non-0 on failure
 **/
int
raptor_iostream_write_decimal(raptor_iostream* iostr, int integer)
{
  /* enough for 64 bit signed integer
   * INT64_MAX is  9223372036854775807 (19 digits) + 1 for sign 
   */
  unsigned char buf[20];
  unsigned char *p;
  int i=integer;
  size_t length=1;
  if(integer<0) {
    length++;
    i= -integer;
  }
  while(i/=10)
    length++;

  p=buf+length-1;
  i=integer;
  if(i<0)
    i= -i;
  do {
    *p-- ='0'+(i %10);
    i /= 10;
  } while(i);
  if(integer<0)
    *p= '-';
  
  return raptor_iostream_write_bytes(iostr, buf, 1, length);
}


/**
 * raptor_iostream_format_hexadecimal - Write an integer in hexadecimal to the iostream
 * @iostr: raptor iostream
 * @integer: unsigned integer to format as hexadecimal
 * @width: field width
 *
 * Always 0-fills the entire field and writes in uppercase A-F
 * 
 * Return value: non-0 on failure
 **/
int
raptor_iostream_format_hexadecimal(raptor_iostream* iostr, 
                                   unsigned int integer, int width)
{
  unsigned char *buf;
  unsigned char *p;
  int rc;

  if(width <1)
    return 1;
  
  buf=(unsigned char*)RAPTOR_MALLOC(cstring, width);
  if(!buf)
    return 1;
  
  p=buf+width-1;
  do {
    unsigned int digit=(integer & 15);
    *p-- =(digit < 10) ? '0'+digit : 'A'+(digit-10);
    integer>>=4;
  } while(integer);
  while(p >= buf)
    *p-- = '0';
  
  rc=raptor_iostream_write_bytes(iostr, buf, 1, width);
  RAPTOR_FREE(cstring, buf);
  return rc;
}

#endif



#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);


#define OUT_FILENAME "out.bin"
#define OUT_BYTES_COUNT 14


int
main(int argc, char *argv[]) 
{
  const char *program=raptor_basename(argv[0]);
#define TEST_ITEMS_COUNT 9
  int i;

  for(i=0; i<4; i++) {
    raptor_iostream *iostr;
    size_t count;

    /* for _to_file */
    FILE *handle;
    /* for _to_string */
    void *string;
    size_t string_len;

    switch(i) {
      case 0:
#ifdef RAPTOR_DEBUG
        fprintf(stderr, "%s: Creating iostream to filename '%s'\n", program, OUT_FILENAME);
#endif
        iostr=raptor_new_iostream_to_filename((const char*)OUT_FILENAME);
        if(!iostr) {
          fprintf(stderr, "%s: Failed to create iostream to filename '%s'\n",
                  program, OUT_FILENAME);
          exit(1);
        }
        break;

      case 1:
#ifdef RAPTOR_DEBUG
        fprintf(stderr, "%s: Creating iostream to file handle\n", program);
#endif
        handle=fopen((const char*)OUT_FILENAME, "wb");
        iostr=raptor_new_iostream_to_file_handle(handle);
        if(!iostr) {
          fprintf(stderr, "%s: Failed to create iostream to file handle\n", program);
          exit(1);
        }
        break;

      case 2:
#ifdef RAPTOR_DEBUG
        fprintf(stderr, "%s: Creating iostream to a string\n", program);
#endif
        iostr=raptor_new_iostream_to_string(&string, &string_len, NULL);
        if(!iostr) {
          fprintf(stderr, "%s: Failed to create iostream to string\n", program);
          exit(1);
        }
        break;

      case 3:
#ifdef RAPTOR_DEBUG
        fprintf(stderr, "%s: Creating iostream to sink\n", program);
#endif
        iostr=raptor_new_iostream_to_sink();
        if(!iostr) {
          fprintf(stderr, "%s: Failed to create iostream to sink\n", program);
          exit(1);
        }
        break;

      default:
        fprintf(stderr, "%s: Unknown test case %d init\n", program, i);
        exit(1);
    }
    

    raptor_iostream_write_bytes(iostr, (const void*)"Hello, world!", 1, 13);
    raptor_iostream_write_byte(iostr, '\n');

    count=raptor_iostream_get_bytes_written_count(iostr);
    if(count != OUT_BYTES_COUNT) {
      fprintf(stderr, "%s: I/O stream wrote %d bytes, expected %d\n", program,
              (int)count, (int)OUT_BYTES_COUNT);
      return 1;
    }
    
#ifdef RAPTOR_DEBUG
    fprintf(stderr, "%s: Freeing iostream\n", program);
#endif
    raptor_free_iostream(iostr);

    switch(i) {
      case 0:
        remove(OUT_FILENAME);
        break;

      case 1:
        fclose(handle);
        remove(OUT_FILENAME);
        break;

      case 2:
        if(!string) {
          fprintf(stderr, "%s: I/O stream failed to create a string\n", program);
          return 1;
        }
        if(string_len != count) {
          fprintf(stderr, "%s: I/O stream created a string length %d, expected %d\n", program, (int)string_len, (int)count);
          return 1;
        }
        raptor_free_memory(string);
        break;

      case 3:
        break;

      default:
        fprintf(stderr, "%s: Unknown test case %d tidy\n", program, i);
        exit(1);
    }
    
  }
  
  /* keep gcc -Wall happy */
  return(0);
}

#endif
