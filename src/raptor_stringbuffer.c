/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_stringbuffer.c - Stringbuffer class for growing strings
 *
 * $Id$
 *
 * Copyright (C) 2003 David Beckett - http://purl.org/net/dajobe/
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
 * 
 */


#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#ifdef WIN32
#include <win32_config.h>
#endif


#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h> /* for abort() as used in errors */
#endif

/* Raptor includes */
#include "raptor.h"
#include "raptor_internal.h"


/*
 * The only methods needed here are:
 *  Create Set
 *  Destroy Set
 *  Check an ID present add it if not, return if added/not
 *
 */

struct raptor_stringbuffer_node_s
{
  struct raptor_stringbuffer_node_s* next;
  unsigned char *string;
  size_t length;
};
typedef struct raptor_stringbuffer_node_s raptor_stringbuffer_node;


struct raptor_stringbuffer_s
{
  /* Pointing to the first item in the list of nodes */
  raptor_stringbuffer_node* head;
  /* and the last */
  raptor_stringbuffer_node* tail;
  
  /* total length of the string */
  size_t length;

  /* frozen string if already calculated, or NULL if not present */
  unsigned char *string;
};


/* prototypes for local functions */
static int raptor_stringbuffer_append_string_common(raptor_stringbuffer* stringbuffer, const unsigned char *string, size_t length, int do_copy);


/* functions implementing the stringbuffer api */

/**
 * raptor_new_stringbuffer - Create a new stringbuffer
 * 
 * Return value: non 0 on failure
 **/
raptor_stringbuffer*
raptor_new_stringbuffer(void) 
{
  raptor_stringbuffer* sb=(raptor_stringbuffer*)RAPTOR_CALLOC(raptor_stringbuffer, 1, sizeof(raptor_stringbuffer));
  if(!sb)
    return NULL;

  return sb;
}


/**
 * raptor_free_stringbuffer - Destroy a stringbuffer
 * 
 **/
void
raptor_free_stringbuffer(raptor_stringbuffer *stringbuffer) 
{
  if(stringbuffer->head) {
    raptor_stringbuffer_node *node=stringbuffer->head;
  
    while(node) {
      raptor_stringbuffer_node *next=node->next;
      
      if(node->string)
        RAPTOR_FREE(cstring, node->string);
      RAPTOR_FREE(raptor_stringbuffer_node, node);
      node=next;
    }
  }

  RAPTOR_FREE(raptor_stringbuffer, stringbuffer);
}



/**
 * raptor_stringbuffer_append_string_common: - Add a string to the stringbuffer
 * @stringbuffer: raptor stringbuffer
 * @string: string
 * @length: length of string
 * @do_copy: non-0 to copy the string
 *
 * INTERNAL
 *
 * If do_copy is non-0, the passed-in string is copied into new memory
 * otherwise the stringbuffer becomes the owner of the string pointer
 * and will free it when the stringbuffer is destroyed.
 *
 * Return value: non-0 on failure
 **/
static int
raptor_stringbuffer_append_string_common(raptor_stringbuffer* stringbuffer, 
                                         const unsigned char *string, size_t length,
                                         int do_copy)
{
  raptor_stringbuffer_node *node;

  node=(raptor_stringbuffer_node*)RAPTOR_MALLOC(raptor_stringbuffer_node, sizeof(raptor_stringbuffer_node));
  if(!node)
    return 1;

  if(do_copy) {
    /* Note this copy does not include the \0 character - not needed  */
    node->string=(unsigned char*)RAPTOR_MALLOC(bytes, length);
    if(!node->string) {
      RAPTOR_FREE(raptor_stringbuffer_node, node);
      return 1;
    }
    strncpy((char*)node->string, (const char*)string, length);
  } else
    node->string=(unsigned char*)string;
  node->length=length;


  if(stringbuffer->tail) {
    stringbuffer->tail->next=node;
    stringbuffer->tail=node;
  } else
    stringbuffer->head=stringbuffer->tail=node;
  node->next=NULL;

  if(stringbuffer->string) {
    RAPTOR_FREE(cstring, stringbuffer->string);
    stringbuffer->string=NULL;
  }
  stringbuffer->length += length;

  return 0;
}




/**
 * raptor_stringbuffer_append_counted_string: - Add a string to the stringbuffer
 * @stringbuffer: raptor stringbuffer
 * @string: string
 * @length: length of string
 * @do_copy: non-0 to copy the string

 * If do_copy is non-0, the passed-in string is copied into new memory
 * otherwise the stringbuffer becomes the owner of the string pointer
 * and will free it when the stringbuffer is destroyed.
 *
 * Return value: non-0 on failure
 **/
int
raptor_stringbuffer_append_counted_string(raptor_stringbuffer* stringbuffer, 
                                          const unsigned char *string, size_t length,
                                          int do_copy)
{
  return raptor_stringbuffer_append_string_common(stringbuffer, string, length, do_copy);
}
  

/**
 * raptor_stringbuffer_append_string: - Add a string to the stringbuffer
 * @stringbuffer: raptor stringbuffer
 * @string: string
 * @do_copy: non-0 to copy the string
 * 
 * If do_copy is non-0, the passed-in string is copied into new memory
 * otherwise the stringbuffer becomes the owner of the string pointer
 * and will free it when the stringbuffer is destroyed.
 *
 * Return value: non-0 on failure
 **/
int
raptor_stringbuffer_append_string(raptor_stringbuffer* stringbuffer, 
                                  const unsigned char *string, int do_copy)
{
  return raptor_stringbuffer_append_string_common(stringbuffer, string, strlen((const char*)string), do_copy);
}


/**
 * raptor_stringbuffer_append_decimal: - Add an integer in decimal to the stringbuffer
 * @stringbuffer: raptor stringbuffer
 * @integer: integer to format as decimal and add
 * 
 * Return value: non-0 on failure
 **/
int
raptor_stringbuffer_append_decimal(raptor_stringbuffer* stringbuffer, 
                                   int integer)
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
  
  return raptor_stringbuffer_append_counted_string(stringbuffer, buf, length, 1);
}


/**
 * raptor_stringbuffer_length: - Return the stringbuffer length
 * @stringbuffer: raptor stringbuffer
 * 
 * Return value: size of stringbuffer
 **/
size_t
raptor_stringbuffer_length(raptor_stringbuffer* stringbuffer)
{
  return stringbuffer->length;
}



/**
 * raptor_stringbuffer_as_string: - Return the stringbuffer as a C string
 * @stringbuffer: raptor stringbuffer
 * 
 * Return value: NULL on failure or stringbuffer is mpety
 **/
unsigned char *
raptor_stringbuffer_as_string(raptor_stringbuffer* stringbuffer)
{
  raptor_stringbuffer_node *node;
  unsigned char *p;
  
  if(!stringbuffer->length)
    return NULL;
  if(stringbuffer->string)
    return stringbuffer->string;

  stringbuffer->string=(unsigned char*)RAPTOR_MALLOC(cstring, stringbuffer->length+1);
  if(!stringbuffer->string)
    return NULL;

  node=stringbuffer->head;
  p=stringbuffer->string;
  while(node) {
    strncpy((char*)p, (const char*)node->string, node->length);
    p+= node->length;
    node=node->next;
  }
  *p='\0';
  return stringbuffer->string;
}



#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);


int
main(int argc, char *argv[]) 
{
  const char *program=argv[0];
  const char *items[10] = { "the", "quick" ,"brown", "fox", "jumps", "over", "the", "lazy", "dog", NULL };
  const char *items_string = "thequickbrownfoxjumpsoverthelazydog";  
  const size_t items_len=35;
  const char *test_integer_string = "abcd";
#define TEST_INTEGERS_COUNT 7
  const int test_integers[TEST_INTEGERS_COUNT]={ 0, 1, -1, 11, 1234, 12345, -12345 };
  const char *test_integer_results[TEST_INTEGERS_COUNT]={ "abcd0", "abcd1", "abcd-1", "abcd11", "abcd1234", "abcd12345", "abcd-12345" };
  raptor_stringbuffer *sb;
  unsigned char *str;
  size_t len;
  int i=0;
  
#ifdef RAPTOR_DEBUG
  fprintf(stderr, "%s: Creating string buffer\n", program);
#endif

  sb=raptor_new_stringbuffer();
  if(!sb) {
    fprintf(stderr, "%s: Failed to create string buffer\n", program);
    exit(1);
  }

  for(i=0; items[i]; i++) {
    int rc;
    len=strlen(items[i]);

#ifdef RAPTOR_DEBUG
    fprintf(stderr, "%s: Adding string buffer item '%s'\n", program, items[i]);
#endif
  
    rc=raptor_stringbuffer_append_counted_string(sb, (unsigned char*)items[i], len, 1);
    if(rc) {
      fprintf(stderr, "%s: Adding string buffer item %d '%s' failed, returning error %d\n",
              program, i, items[i], rc);
      exit(1);
    }
  }

  len=raptor_stringbuffer_length(sb);
  if(len != items_len) {
    fprintf(stderr, "%s: string buffer len is %d, expected %d\n", program,
            (int)len, (int)items_len);
    exit(1);
  }

  str=raptor_stringbuffer_as_string(sb);
  if(strcmp((const char*)str, items_string)) {
    fprintf(stderr, "%s: string buffer contains '%s', expected '%s'\n",
            program, str, items_string);
    exit(1);
  }
  RAPTOR_FREE(cstring, str);

  for(i=0; i<TEST_INTEGERS_COUNT; i++) {
    raptor_stringbuffer *isb=raptor_new_stringbuffer();
    if(!isb) {
      fprintf(stderr, "%s: Failed to create string buffer\n", program);
      exit(1);
    }
    
    raptor_stringbuffer_append_string(isb, 
                                      (const unsigned char*)test_integer_string, 1);

#ifdef RAPTOR_DEBUG
    fprintf(stderr, "%s: Adding decimal integer %d to buffer\n", program, test_integers[i]);
#endif

    raptor_stringbuffer_append_decimal(isb, test_integers[i]);

    str=raptor_stringbuffer_as_string(isb);
    if(strcmp((const char*)str, test_integer_results[i])) {
      fprintf(stderr, "%s: string buffer contains '%s', expected '%s'\n",
              program, str, test_integer_results[i]);
      exit(1);
    }
    RAPTOR_FREE(cstring, str);
#ifdef RAPTOR_DEBUG
    fprintf(stderr, "%s: Freeing string buffer\n", program);
#endif
    raptor_free_stringbuffer(isb);
  }

#ifdef RAPTOR_DEBUG
  fprintf(stderr, "%s: Freeing string buffer\n", program);
#endif
  raptor_free_stringbuffer(sb);

  /* keep gcc -Wall happy */
  return(0);
}

#endif
