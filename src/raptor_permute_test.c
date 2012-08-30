/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_permute.c - Test of permutations of ints in a sequence
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

#include <stdio.h>
#include <string.h>

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


#define PERMUTE_DEBUG 0


typedef struct 
{
  raptor_sequence* seq;
  int* contents;
} intseq;


static int
print_handler(void *object, FILE *fh)
{
  int* pi = (int*)object;
  fprintf(fh, "%d", *pi);
  return 0;
}


static intseq*
new_intseq(int size) 
{
  intseq* iseq;
  int i;
  
  iseq = RAPTOR_CALLOC(intseq*, 1, sizeof(*iseq));
  iseq->contents = RAPTOR_CALLOC(int*, size, sizeof(int));
  /* will be a sequence of int* pointing into iseq->contents */
  iseq->seq = raptor_new_sequence(NULL, print_handler);

  for(i = 0; i < (int)size; i++) {
    iseq->contents[i] = i + 1;
    raptor_sequence_set_at(iseq->seq, i, &iseq->contents[i]);
  }

  return iseq;
}

static void
free_intseq(intseq* iseq)
{
  if(iseq->contents)
    RAPTOR_FREE(int*, iseq->contents);

  if(iseq->seq)
    raptor_free_sequence(iseq->seq);

  RAPTOR_FREE(intseq*, iseq);
}

static void
intseq_print(intseq *iseq, FILE* stream)
{
  raptor_sequence_print(iseq->seq, stream);
}

static int
intseq_reverse(intseq *iseq, int start_index, int length) 
{
  return raptor_sequence_reverse(iseq->seq, start_index, length);
}

static int
intseq_compare_at(const void* a, const void* b)
{
  int* ia = (int*)a;
  int* ib = (int*)b;
  return *ia - *ib;
}

static int
intseq_next_permutation(intseq *iseq)
{
  return raptor_sequence_next_permutation(iseq->seq, intseq_compare_at);
}

static int
intseq_get_at(intseq *iseq, int idx)
{       
  return *(int*)(raptor_sequence_get_at(iseq->seq, idx));
}
       
#define MAX_SIZE 5

int expected_results_size5[120][5] = {
  {1, 2, 3, 4, 5},
  {1, 2, 3, 5, 4},
  {1, 2, 4, 3, 5},
  {1, 2, 4, 5, 3},
  {1, 2, 5, 3, 4},
  {1, 2, 5, 4, 3},
  {1, 3, 2, 4, 5},
  {1, 3, 2, 5, 4},
  {1, 3, 4, 2, 5},
  {1, 3, 4, 5, 2},
  {1, 3, 5, 2, 4},
  {1, 3, 5, 4, 2},
  {1, 4, 2, 3, 5},
  {1, 4, 2, 5, 3},
  {1, 4, 3, 2, 5},
  {1, 4, 3, 5, 2},
  {1, 4, 5, 2, 3},
  {1, 4, 5, 3, 2},
  {1, 5, 2, 3, 4},
  {1, 5, 2, 4, 3},
  {1, 5, 3, 2, 4},
  {1, 5, 3, 4, 2},
  {1, 5, 4, 2, 3},
  {1, 5, 4, 3, 2},
  {2, 1, 3, 4, 5},
  {2, 1, 3, 5, 4},
  {2, 1, 4, 3, 5},
  {2, 1, 4, 5, 3},
  {2, 1, 5, 3, 4},
  {2, 1, 5, 4, 3},
  {2, 3, 1, 4, 5},
  {2, 3, 1, 5, 4},
  {2, 3, 4, 1, 5},
  {2, 3, 4, 5, 1},
  {2, 3, 5, 1, 4},
  {2, 3, 5, 4, 1},
  {2, 4, 1, 3, 5},
  {2, 4, 1, 5, 3},
  {2, 4, 3, 1, 5},
  {2, 4, 3, 5, 1},
  {2, 4, 5, 1, 3},
  {2, 4, 5, 3, 1},
  {2, 5, 1, 3, 4},
  {2, 5, 1, 4, 3},
  {2, 5, 3, 1, 4},
  {2, 5, 3, 4, 1},
  {2, 5, 4, 1, 3},
  {2, 5, 4, 3, 1},
  {3, 1, 2, 4, 5},
  {3, 1, 2, 5, 4},
  {3, 1, 4, 2, 5},
  {3, 1, 4, 5, 2},
  {3, 1, 5, 2, 4},
  {3, 1, 5, 4, 2},
  {3, 2, 1, 4, 5},
  {3, 2, 1, 5, 4},
  {3, 2, 4, 1, 5},
  {3, 2, 4, 5, 1},
  {3, 2, 5, 1, 4},
  {3, 2, 5, 4, 1},
  {3, 4, 1, 2, 5},
  {3, 4, 1, 5, 2},
  {3, 4, 2, 1, 5},
  {3, 4, 2, 5, 1},
  {3, 4, 5, 1, 2},
  {3, 4, 5, 2, 1},
  {3, 5, 1, 2, 4},
  {3, 5, 1, 4, 2},
  {3, 5, 2, 1, 4},
  {3, 5, 2, 4, 1},
  {3, 5, 4, 1, 2},
  {3, 5, 4, 2, 1},
  {4, 1, 2, 3, 5},
  {4, 1, 2, 5, 3},
  {4, 1, 3, 2, 5},
  {4, 1, 3, 5, 2},
  {4, 1, 5, 2, 3},
  {4, 1, 5, 3, 2},
  {4, 2, 1, 3, 5},
  {4, 2, 1, 5, 3},
  {4, 2, 3, 1, 5},
  {4, 2, 3, 5, 1},
  {4, 2, 5, 1, 3},
  {4, 2, 5, 3, 1},
  {4, 3, 1, 2, 5},
  {4, 3, 1, 5, 2},
  {4, 3, 2, 1, 5},
  {4, 3, 2, 5, 1},
  {4, 3, 5, 1, 2},
  {4, 3, 5, 2, 1},
  {4, 5, 1, 2, 3},
  {4, 5, 1, 3, 2},
  {4, 5, 2, 1, 3},
  {4, 5, 2, 3, 1},
  {4, 5, 3, 1, 2},
  {4, 5, 3, 2, 1},
  {5, 1, 2, 3, 4},
  {5, 1, 2, 4, 3},
  {5, 1, 3, 2, 4},
  {5, 1, 3, 4, 2},
  {5, 1, 4, 2, 3},
  {5, 1, 4, 3, 2},
  {5, 2, 1, 3, 4},
  {5, 2, 1, 4, 3},
  {5, 2, 3, 1, 4},
  {5, 2, 3, 4, 1},
  {5, 2, 4, 1, 3},
  {5, 2, 4, 3, 1},
  {5, 3, 1, 2, 4},
  {5, 3, 1, 4, 2},
  {5, 3, 2, 1, 4},
  {5, 3, 2, 4, 1},
  {5, 3, 4, 1, 2},
  {5, 3, 4, 2, 1},
  {5, 4, 1, 2, 3},
  {5, 4, 1, 3, 2},
  {5, 4, 2, 1, 3},
  {5, 4, 2, 3, 1},
  {5, 4, 3, 1, 2},
  {5, 4, 3, 2, 1}
};

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

#if PERMUTE_DEBUG > 0
    fprintf(stderr, "%s: Permutation test %d.  Initial state: ", program, size);
    intseq_print(iseq, stderr);
    fputc('\n', stderr);
#endif

    for(count = 1; 1; count++) {
#if PERMUTE_DEBUG > 1
      fprintf(stderr, "Permutation %3d: ", count);
      intseq_print(iseq, stderr);
      fputc('\n', stderr);
#endif
      if(size == 5) {
        int* expected_result = expected_results_size5[count - 1];
        int j;
        int ok = 1;
        
        for(j = 0; j < size; j++) {
          int actual = intseq_get_at(iseq, j);
          int expected = expected_result[j];
          if(actual != expected) {
            ok = 0;
            break;
          }
        }
        if(!ok) {
          fprintf(stderr, "%s: FAILED test %d result %d - returned ",
                  program, size, count);
          intseq_print(iseq, stderr);
          fputs(" expected [", stderr);
          for(j = 0; j < size; j++) {
            fprintf(stderr, "%d, ", expected_result[j]);
          }
          fputs("]\n", stderr);
          
          failures++;
        }
      }
      
      if(intseq_next_permutation(iseq))
        break;
    }

#if PERMUTE_DEBUG > 0
    fprintf(stderr, "%s: Returned %d results.  Final state: ", program, count);
    intseq_print(iseq, stderr);
    fputc('\n', stderr);
#endif

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
