/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_sequence.c - Raptor sequence support
 *
 * $Id$
 *
 * Copyright (C) 2003-2004, David Beckett http://purl.org/net/dajobe/
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


#include "raptor.h"
#include "raptor_internal.h"


#ifndef STANDALONE

/*
 * These sequences are:
 *   cheap to push/pop from the end of the sequence.
 *   expensive to shift/unshift from the start.
 */

struct raptor_sequence_s {
  int size;
  int capacity;
  void **sequence;
  raptor_sequence_free_handler *free_handler;
  raptor_sequence_print_handler *print_handler;
};


static int raptor_sequence_ensure(raptor_sequence *seq, int capacity);
static int raptor_sequence_grow(raptor_sequence *seq);

/* Constructor */
raptor_sequence*
raptor_new_sequence(raptor_sequence_free_handler *free_handler,
                    raptor_sequence_print_handler *print_handler) {
  raptor_sequence* seq=(raptor_sequence*)RAPTOR_MALLOC(raptor_sequence, sizeof(raptor_sequence));
  if(!seq)
    return NULL;
  seq->size=0;
  seq->capacity=0;
  seq->sequence=NULL;
  seq->free_handler=free_handler;
  seq->print_handler=print_handler;
  
  return seq;
}

/* Destructor*/

void
raptor_free_sequence(raptor_sequence* seq) {
  int i;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN(seq, raptor_sequence);

  if(seq->free_handler)
    for(i=0; i< seq->size; i++)
      if(seq->sequence[i])
        seq->free_handler(seq->sequence[i]);

  if(seq->sequence)
    free(seq->sequence);

  RAPTOR_FREE(raptor_sequence, seq);
}


static int
raptor_sequence_ensure(raptor_sequence *seq, int capacity) {
  void **new_sequence;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(seq, raptor_sequence, 1);

  if(seq->capacity > capacity)
    return 0;

  /* POLICY - minimum size */
  if(capacity < 8)
    capacity=8;

  new_sequence=(void**)calloc(capacity, sizeof(void*));
  if(!new_sequence)
    return 1;

  if(seq->size) {
    memcpy(new_sequence, seq->sequence, sizeof(void*)*seq->size);
    free(seq->sequence);
  }

  seq->sequence=new_sequence;
  seq->capacity=capacity;
  return 0;
}


static int
raptor_sequence_grow(raptor_sequence *seq) 
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(seq, raptor_sequence, 1);

  return raptor_sequence_ensure(seq, seq->capacity*2);
}



/* Methods */
int
raptor_sequence_size(raptor_sequence* seq) {
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(seq, raptor_sequence, -1);

  return seq->size;
}


/* Store methods */
int
raptor_sequence_set_at(raptor_sequence* seq, int idx, void *data) {
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(seq, raptor_sequence, 1);

  if(idx+1 > seq->capacity) {
    if(raptor_sequence_ensure(seq, idx+1))
      return 1;
  }
    
  if(seq->sequence[idx] && seq->free_handler)
    seq->free_handler(seq->sequence[idx]);
  
  seq->sequence[idx]=data;
  if(idx+1 > seq->size)
    seq->size=idx+1;
  return 0;
}


/* add to end of sequence */
int
raptor_sequence_push(raptor_sequence* seq, void *data) {
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(seq, raptor_sequence, 1);

  if(seq->size == seq->capacity) {
    if(raptor_sequence_grow(seq))
      return 1;
  }

  seq->sequence[seq->size]=data;
  seq->size++;
  return 0;
}


/* add to start of sequence */
int
raptor_sequence_shift(raptor_sequence* seq, void *data) {
  int i;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(seq, raptor_sequence, 1);

  if(seq->size == seq->capacity) {
    if(raptor_sequence_grow(seq))
      return 1;
  }

  for(i=seq->size; i>0; i--)
    seq->sequence[i]=seq->sequence[i-1];
  
  seq->sequence[0]=data;
  seq->size++;
  return 0;
}


/* Retrieval methods */
void*
raptor_sequence_get_at(raptor_sequence* seq, int idx) {
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(seq, raptor_sequence, NULL);

  if(idx > seq->size)
    return NULL;
  return seq->sequence[idx];
}

/* remove from end of sequence */
void*
raptor_sequence_pop(raptor_sequence* seq) {
  void *data;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(seq, raptor_sequence, NULL);

  if(!seq->size)
    return NULL;

  seq->size--;
  data=seq->sequence[seq->size];
  seq->sequence[seq->size]=NULL;

  return data;
}

/* remove from start of sequence */
void*
raptor_sequence_unshift(raptor_sequence* seq) {
  void *data;
  int i;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(seq, raptor_sequence, NULL);

  if(!seq->size)
    return NULL;
  
  data=seq->sequence[0];
  seq->size--;
  for(i=0; i<seq->size; i++)
    seq->sequence[i]=seq->sequence[i+1];
  seq->sequence[i]=NULL;
  
  return data;
}


int
raptor_compare_strings(const void *a, const void *b) 
{
  return strcmp(*(char**)a, *(char**)b);
}



/* sort sequence */
void
raptor_sequence_sort(raptor_sequence* seq, 
                     int(*compare)(const void *, const void *))
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN(seq, raptor_sequence);

  if(seq->size > 1)
    qsort(seq->sequence, seq->size, sizeof(void*), compare);
}



void
raptor_sequence_print_string(char *data, FILE *fh) 
{
  fputs(data, fh);
}


void
raptor_sequence_print_uri(char *data, FILE *fh) 
{
  raptor_uri* uri=(raptor_uri*)data;
  fputs((const char*)raptor_uri_as_string(uri), fh);
}


void
raptor_sequence_set_print_handler(raptor_sequence *seq,
                                  raptor_sequence_print_handler *print_handler) {
  if(!seq)
    return;
  seq->print_handler=print_handler;
}


/* print sequence */
void
raptor_sequence_print(raptor_sequence* seq, FILE* fh)
{
  int i;

  RAPTOR_ASSERT_OBJECT_POINTER_RETURN(seq, raptor_sequence);

  fputc('[', fh);
  for(i=0; i<seq->size; i++) {
    if(i)
      fputs(", ", fh);
    if(seq->sequence[i])
      seq->print_handler(seq->sequence[i], fh);
    else
      fputs("(empty)", fh);
  }
  fputc(']', fh);
}


/**
 * raptor_sequence_join: Join two sequences moving all items from one sequence to the end of another
 * @param dest: &raptor_sequence destination sequence
 * @param src: &raptor_sequence source sequence
 *
 * After this operation, sequence src will be empty (zero size) but
 * will have the same item capacity as before.
 *
 * Return value: non-0 on failure
 */
int
raptor_sequence_join(raptor_sequence* dest, raptor_sequence *src)
{
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(dest, raptor_sequence, 1);
  RAPTOR_ASSERT_OBJECT_POINTER_RETURN_VALUE(src, raptor_sequence, 1);

  if(raptor_sequence_ensure(dest, dest->size + src->size))
    return 1;
  memcpy(&dest->sequence[dest->size], src->sequence, sizeof(void*)*src->size);
  dest->size += src->size;

  src->size=0;

  return 0;
}


#endif



#ifdef STANDALONE
#include <stdio.h>

int main(int argc, char *argv[]);


#define assert_match_string(function, expr, string) do { char *result=expr; if(strcmp(result, string)) { fprintf(stderr, "%s:" #function " failed - returned %s, expected %s\n", program, result, string); exit(1); } } while(0)
#define assert_match_int(function, expr, value) do { int result=expr; if(result != value) { fprintf(stderr, "%s:" #function " failed - returned %d, expected %d\n", program, result, value); exit(1); } } while(0)

int
main(int argc, char *argv[]) 
{
  const char *program=raptor_basename(argv[0]);
  raptor_sequence* seq1=raptor_new_sequence(NULL, (raptor_sequence_print_handler*)raptor_sequence_print_string);
  raptor_sequence* seq2=raptor_new_sequence(NULL, (raptor_sequence_print_handler*)raptor_sequence_print_string);
  char *s;

  raptor_sequence_set_at(seq1, 0, (void*)"first");

  raptor_sequence_push(seq1, (void*)"third");

  raptor_sequence_shift(seq1, (void*)"second");

  s=(char*)raptor_sequence_get_at(seq1, 0);
  assert_match_string(raptor_sequence_get_at, s, "second");

  s=(char*)raptor_sequence_get_at(seq1, 1);
  assert_match_string(raptor_sequence_get_at, s, "first");
  
  s=(char*)raptor_sequence_get_at(seq1, 2);
  assert_match_string(raptor_sequence_get_at, s, "third");
  
  assert_match_int(raptor_sequence_size, raptor_sequence_size(seq1), 3);

  fprintf(stderr, "%s: sequence after additions: ", program);
  raptor_sequence_print(seq1, stderr);
  fputc('\n', stderr);

  /* now made alphabetical i.e. first, second, third */
  raptor_sequence_sort(seq1, raptor_compare_strings);

  fprintf(stderr, "%s: sequence after sort: ", program);
  raptor_sequence_print(seq1, stderr);
  fputc('\n', stderr);

  s=(char*)raptor_sequence_pop(seq1);
  assert_match_string(raptor_sequence_get_at, s, "third");

  assert_match_int(raptor_sequence_size, raptor_sequence_size(seq1), 2);

  fprintf(stderr, "%s: sequence after pop: ", program);
  raptor_sequence_print(seq1, stderr);
  fputc('\n', stderr);

  s=(char*)raptor_sequence_unshift(seq1);
  assert_match_string(raptor_sequence_get_at, s, "first");

  assert_match_int(raptor_sequence_size, raptor_sequence_size(seq1), 1);

  fprintf(stderr, "%s: sequence after unshift: ", program);
  raptor_sequence_print(seq1, stderr);
  fputc('\n', stderr);

  s=(char*)raptor_sequence_get_at(seq1, 0);
  assert_match_string(raptor_sequence_get_at, s, "second");
  
  raptor_sequence_push(seq2, (void*)"first.2");
  if(raptor_sequence_join(seq2, seq1)) {
    fprintf(stderr, "%s: raptor_sequence_join failed\n", program);
    exit(1);
  }

  assert_match_int(raptor_sequence_size, raptor_sequence_size(seq1), 0);
  assert_match_int(raptor_sequence_size, raptor_sequence_size(seq2), 2);

  raptor_free_sequence(seq1);
  raptor_free_sequence(seq2);

  return (0);
}
#endif
