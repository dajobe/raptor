/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * sort_r.c - Portable sort_r
 *
 * Copyright (C) 2014, David Beckett http://www.dajobe.org/
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


#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


#ifndef STANDALONE

#if defined(HAVE_QSORT_R) || defined(HAVE_QSORT_S)
/* Include inline code */
#include "sort_r.h"
#else
#include "ssort.h"
#endif


/**
 * raptor_sort_r:
 * @base: the array to be sorted
 * @nel: the number of elements in the array
 * @width: the size in bytes of each element of the array
 * @compar: comparison function
 * @user_data: a pointer to be passed to the comparison function
 *
 * Sort an array with an extra user data arg for the comparison funciton.
 *
 * Sorts data at @base of @nel elememnts of width @width using
 * comparison function @comp that takes args (void* data1, void*
 * data2, @user_data) and returns <0, 0, or >0 for object comparison.
 *
 */
void
raptor_sort_r(void *base, size_t nel, size_t width,
              raptor_data_compare_arg_handler compar, void *user_data)
{
#if defined(HAVE_QSORT_R) || defined(HAVE_QSORT_S)
  sort_r(base, nel, width, compar, user_data);
#else
  ssort_r(base, nel, width, compar, user_data);
#endif
}


#endif



#ifdef STANDALONE

/* one more prototype */
int main(int argc, char *argv[]);


/* Public Domain licensed example code by Isaac Turner from
 * https://github.com/noporpoise/sort_r
 */

/* Isaac Turner 18 Nov 2013 Public Domain */

/*
Comparison function to sort an array of int, inverting a given region.
`arg` should be of type int[2], with the elements representing the start and end
of the region to invert (inclusive).
*/
static int sort_r_cmp(const void *aa, const void *bb, void *arg)
{
  const int *a = (const int*)aa;
  const int *b = (const int*)bb;
  const int *interval = (const int*)arg;
  int cmp = *a - *b;
  int inv_start = interval[0], inv_end = interval[1];
  char norm = (*a < inv_start || *a > inv_end || *b < inv_start || *b > inv_end);
  return norm ? cmp : -cmp;
}

int
main(int argc, char *argv[])
{
  const char *program = raptor_basename(argv[0]);

  int i;
  /* sort 1..19, 30..20, 30..100 */
  int arr[18] = {1, 5, 28, 4, 3, 2, 10, 20, 18, 25, 21, 29, 34, 35, 14, 100, 27, 19};
  int tru[18] = {1, 2, 3, 4, 5, 10, 14, 18, 19, 29, 28, 27, 25, 21, 20, 34, 35, 100};

  /* Region to invert: 20-30 (inclusive) */
  int interval[2] = {20, 30};
  int failures = 0;

  raptor_sort_r(arr, 18, sizeof(int), sort_r_cmp, interval);

  /* Check PASS/FAIL */
  for(i = 0; i < 18; i++) {
    if(arr[i] != tru[i]) {
      printf("%s: sort_r() result %i: got %d expected %d", program, 
             i, arr[i], tru[i]);
      failures++;
    }
  }
  
  return failures;
}

#endif
