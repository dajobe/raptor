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



/* Local handlers for writing to a filename */

static void
raptor_filename_iostream_finish(void *context) 
{
  FILE* fh=(FILE*)context;
  fclose(fh);
  return;
}

static int
raptor_filename_iostream_write_bytes(void *context,
                                     const void *ptr, size_t size, size_t nmemb)
{
  FILE* fh=(FILE*)context;
  return (fwrite(ptr, size, nmemb, fh) == nmemb);
}

static int
raptor_filename_iostream_write_byte(void *context, const int byte)
{
  FILE* fh=(FILE*)context;
  return (fputc(byte, fh) == byte);
}


static raptor_iostream_handler raptor_iostream_filename_handler={
  NULL, /* init */
  raptor_filename_iostream_finish,
  raptor_filename_iostream_write_bytes,
  raptor_filename_iostream_write_byte
};


/**
 * raptor_new_iostream_to_filename - Create a new iostream to a filename
 * 
 * Return value: new &raptor_iostream object or NULL on failure
 **/
raptor_iostream*
raptor_new_iostream_to_filename(const char *filename)
{
  FILE *fh;
  raptor_iostream* iostr;

  fh=fopen(filename, "wb");
  if(!fh)
    return NULL;
  
  iostr=(raptor_iostream*)RAPTOR_CALLOC(raptor_iostream, 1, sizeof(raptor_iostream));
  if(!iostr)
    return NULL;

  iostr->handler=&raptor_iostream_filename_handler;
  iostr->context=(void*)fh;

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
  raptor_filename_iostream_write_bytes,
  raptor_filename_iostream_write_byte
};


/**
 * raptor_new_iostream_to_file_handle - Create a new iostream to a FILE*
 * 
 * Return value: new &raptor_iostream object or NULL on failure
 **/
raptor_iostream*
raptor_new_iostream_to_file_handle(FILE *fh)
{
  raptor_iostream* iostr;

  if(!fh)
    return NULL;

  iostr=(raptor_iostream*)RAPTOR_CALLOC(raptor_iostream, 1, sizeof(raptor_iostream));
  if(!iostr)
    return NULL;

  iostr->handler=&raptor_iostream_file_handler;
  iostr->context=(void*)fh;

  if(iostr->handler->init && iostr->handler->init(iostr->context)) {
    RAPTOR_FREE(raptor_iostream, iostr);
    return NULL;
  }
  return iostr;
}



struct raptor_string_iostream_context {
  raptor_stringbuffer *sb;
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
  void *str;
  
  if(con->length_p)
    *con->length_p=len;
  
  str=(void*)RAPTOR_MALLOC(memory, len+1);
  if(str) {
    strncpy(str, raptor_stringbuffer_as_string(con->sb), len+1);
    *con->string_p=str;
  } else {
    *con->string_p=NULL;
    if(con->length_p)
      *con->length_p=0;
  }
  
  raptor_free_stringbuffer(con->sb);
  return;
}

static int
raptor_string_iostream_write_bytes(void *context,
                                   const void *ptr, size_t size, size_t nmemb)
{
  struct raptor_string_iostream_context* con=(struct raptor_string_iostream_context*)context;

  return raptor_stringbuffer_append_counted_string(con->sb, ptr, size * nmemb, 1);
}

static int
raptor_string_iostream_write_byte(void *context, const int byte)
{
  struct raptor_string_iostream_context* con=(struct raptor_string_iostream_context*)context;
  char buf=(char)byte;
  return raptor_stringbuffer_append_counted_string(con->sb, &buf, 1, 1);
}


static raptor_iostream_handler raptor_iostream_string_handler={
  raptor_string_iostream_init,
  raptor_string_iostream_finish,
  raptor_string_iostream_write_bytes,
  raptor_string_iostream_write_byte
};


/**
 * raptor_new_iostream_to_string - Create a new iostream to a string
 * @string_p: pointer to location to hold string
 * @length_p: pointer to location to hold length of string (or NULL)
 * 
 * Return value: new &raptor_iostream object or NULL on failure
 **/
raptor_iostream*
raptor_new_iostream_to_string(void **string_p, size_t *length_p)
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
  con->length_p=length_p;  

  iostr->handler=&raptor_iostream_string_handler;
  iostr->context=(void*)con;

  if(iostr->handler->init && iostr->handler->init(iostr->context)) {
    iostr->handler->finish(iostr->context);
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
  if(iostr->handler->finish)
    iostr->handler->finish(iostr->context);

  RAPTOR_FREE(raptor_iostream, iostr);
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
  
  if(!iostr->handler->write_bytes)
    return 0;
  return iostr->handler->write_bytes(iostr->context, ptr, size, nmemb);
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

  if(!iostr->handler->write_byte)
    return 1;
  return iostr->handler->write_byte(iostr->context, byte);
}


/**
 * raptor_get_bytes_written_count - Get the number of bytes written to the iostream
 * @iostr: raptor iostream
 *
 * Return value: number of bytes written or 0 if non written so far
 **/
size_t
raptor_get_bytes_written_count(raptor_iostream *iostr)
{
  return iostr->bytes;
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
  raptor_iostream *iostr;
  int i;
  size_t count;

  /* for _to_file */
  FILE *fh;
  /* for _to_string */
  void *string;
  size_t string_len;

  for(i=0; i<3; i++) {

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
        fh=fopen((const char*)OUT_FILENAME, "wb");
        iostr=raptor_new_iostream_to_file_handle(fh);
        if(!iostr) {
          fprintf(stderr, "%s: Failed to create iostream to file handle\n", program);
          exit(1);
        }
        break;

      case 2:
#ifdef RAPTOR_DEBUG
        fprintf(stderr, "%s: Creating iostream to a string\n", program);
#endif
        iostr=raptor_new_iostream_to_string(&string, &string_len);
        if(!iostr) {
          fprintf(stderr, "%s: Failed to create iostream to string\n", program);
          exit(1);
        }
        break;

      default:
        fprintf(stderr, "%s: Unknown test case %d init\n", program, i);
        exit(1);
    }
    

    raptor_iostream_write_bytes(iostr, (const void*)"Hello, world!", 1, 13);
    raptor_iostream_write_byte(iostr, '\n');

    count=raptor_get_bytes_written_count(iostr);
    if(count != OUT_BYTES_COUNT) {
      fprintf(stderr, "%s: I/O stream wrote %d bytes, expected %d\n", program,
              count, OUT_BYTES_COUNT);
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
        fclose(fh);
        remove(OUT_FILENAME);
        break;

      case 2:
        if(!string) {
          fprintf(stderr, "%s: I/O stream failed to create a string\n", program);
          return 1;
        }
        if(string_len != count) {
          fprintf(stderr, "%s: I/O stream created a string length %d, expected %d\n", program, string_len, count);
          return 1;
        }
        raptor_free_memory(string);
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
