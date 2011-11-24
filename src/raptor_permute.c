/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * permute.c - Permutations of elements in a sequence
 *
 * Copyright (C) 2011, David Beckett http://www.dajobe.org/
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

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


#define REVERSE_DEBUG 0
#define PERMUTE_DEBUG 0


typedef struct 
{
  int* contents;
  size_t size;
} intseq;


static intseq*
new_intseq(size_t size) 
{
  intseq* iseq;
  int i;
  
  iseq = RAPTOR_CALLOC(intseq*, 1, sizeof(*iseq));
  iseq->size = size;
  iseq->contents = RAPTOR_CALLOC(int*, size, sizeof(int));

  for(i = 0; i < (int)size; i++) {
    iseq->contents[i] = i + 1;
  }

  return iseq;
}

static void
free_intseq(intseq* iseq)
{
  if(iseq->contents)
    RAPTOR_FREE(int*, iseq->contents);

  RAPTOR_FREE(intseq*, iseq);
}


static void
intseq_print(intseq *iseq, FILE* stream)
{
  size_t i;

  for(i = 0; i < iseq->size; i++) {
    if(i > 0)
      fputc(' ', stream);
    fprintf(stream, "%d", iseq->contents[i]);
  }
  
}


/**
 * intseq_reverse:
 * @iseq: int seq
 * @start_index: starting index
 * @length: number of elements to reverse
 *
 * Reverse a range of elements
 *
 * Return value: non-0 if arguments are out of range
 */
static int
intseq_reverse(intseq *iseq, size_t start_index, size_t length) 
{
  size_t end_index = start_index + length - 1;

#if REVERSE_DEBUG > 0
  fprintf(stderr, "intseq_reverse(%d, %d) end_index %d array size %d\n",
          (int)start_index, (int)length, (int)end_index, (int)iseq->size);

  fputs("  initial: ", stderr);
  intseq_print(iseq, stderr);
  fputc('\n', stderr);
#endif
  if(end_index >= iseq->size)
    return 1;
  
  if(length == 1)
    goto done;

  while( (start_index != end_index) && (start_index != end_index + 1) ) {
    int tmp;
    tmp = iseq->contents[start_index];
    iseq->contents[start_index] = iseq->contents[end_index];
    iseq->contents[end_index] = tmp;
    start_index++; end_index--;
  }

  done:
#if REVERSE_DEBUG > 0
  fputs("    final: ", stderr);
  intseq_print(iseq, stderr);
  fputc('\n', stderr);
#endif

  return 0;
}


static int
intseq_compare_at(intseq* iseq, size_t index1, size_t index2) 
{
  return iseq->contents[index1] - iseq->contents[index2];
}


/**
 * intseq_next_permutation:
 * @iseq: int seq
 *
 * Permute in lexicographic order
 *
 * Assumes the initial order of the items is lexicographically
 * increasing.  This function alters the order of the items until the
 * last permuatation is done at which point the contents is reset to
 * the intial order.
 *
 * Algorithm used is described in
 * <http://en.wikipedia.org/wiki/Permutation#Generation_in_lexicographic_order>
 *
 * Return value: non-0 at the last  permutation
 */

static int
intseq_next_permutation(intseq *iseq) 
{
  long k;
  long l;
  int temp;
  
  if(iseq->size < 2)
    return 1;
  
  /* 1. Find the largest index k such that a[k] < a[k + 1]. If no such
   * index exists, the permutation is the last permutation.
   */
  k = iseq->size - 2;
  while(k >= 0 && intseq_compare_at(iseq, k, k + 1) >= 0)
    k--;
  
  if(k == -1) {
    /* done - reset to starting order */
    intseq_reverse(iseq, 0, iseq->size);
    return 1;
  }
  
  /* 2. Find the largest index l such that a[k] < a[l]. Since k + 1
   * is such an index, l is well defined and satisfies k < l.
   */
  l = iseq->size - 1;
  while( intseq_compare_at(iseq, k, l) >= 0)
    l--;
  
  /* 3. Swap a[k] with a[l]. */
  temp = iseq->contents[k];
  iseq->contents[k] = iseq->contents[l];
  iseq->contents[l] = temp;

  /* 4. Reverse the sequence from a[k + 1] up to and including the
   * final element a[n].
   */
  intseq_reverse(iseq, k + 1, iseq->size - (k + 1));

  return 0;
}


#define MAX_SIZE 5

int main (int argc, char *argv[]) 
{
  const char *program = raptor_basename(argv[0]);
  int failures = 0;
  raptor_world *world;
  int size;
  int expected_counts[MAX_SIZE + 1] = { 1, 1, 2, 6, 24, 120 };
   
  world = raptor_new_world();
  if(!world || raptor_world_open(world))
    exit(1);

  for(size = 0; size <= MAX_SIZE; size++) {
    intseq* iseq;
    int count;

    iseq = new_intseq(size);

    fprintf(stderr, "%s: Permutation test %d.  Initial state: ", program, size);
    intseq_print(iseq, stderr);
    fputc('\n', stderr);

    for(count = 1; 1; count++) {
#if PERMUTE_DEBUG > 0
      fprintf(stderr, "Permutation %3d: ", count);
      intseq_print(iseq, stderr);
      fputc('\n', stderr);
#endif
      if(intseq_next_permutation(iseq))
        break;
    }

    fprintf(stderr, "%s: Returned %d results.  Final state: ", program, count);
    intseq_print(iseq, stderr);
    fputc('\n', stderr);

    free_intseq(iseq);

    if(count != expected_counts[size]) {
      fprintf(stderr, "%s: FAILED test %d - returned %d items expected %d\n",
              program, size, count, expected_counts[size]);
      failures++;
      break;
    }
  }


  /* This is mainly a test for crashes */
  for(size = 0; size <= MAX_SIZE; size++) {
    intseq* iseq;
    int st;

    iseq = new_intseq(size);

    for(st = 0; st < size + 1; st++) {
      intseq_reverse(iseq, 0, st) ;
      intseq_reverse(iseq, st, st) ;
      intseq_reverse(iseq, st, 0) ;
    }

    free_intseq(iseq);

  }
  
  raptor_free_world(world);
  
  return failures;
}
