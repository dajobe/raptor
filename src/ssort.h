/*
**  ssort()  --  Fast, small, qsort()-compatible Shell sort
**
**  by Ray Gardner,  public domain   5/90
*/

#include <stddef.h>

/**
 * ssort_r:
 * @base: base data
 * @nel: number of elements at @base
 * @width: width of an element
 * @comp: comparison function taking args (a, b, @arg)
 * @arg: user data (thunk) for the comparison function @comp
 *
 * Fast, small shell sort compatible to qsort_r() taking an extra thunk / user data arg.
 *
 * Sorts data at @base of @nel elements of width @width using
 * comparison function @comp that takes args (a, b, @arg).
 *
 * Return value: non-0 on failure
 *
*/
static int
ssort_r(void* base, size_t nel, size_t width,
        raptor_data_compare_arg_handler comp,
        void* arg)
{
  size_t wnel, gap, k;

  /* bad args */
  if(!base || !width || !comp)
    return -1;

  /* nothing to do */
  if(nel < 2)
    return 0;

  wnel = width * nel;
  for(gap = 0; ++gap < nel;)
    gap *= 3;

  while((gap /= 3) != 0) {
    size_t wgap = width * gap;
    size_t i;

    for(i = wgap; i < wnel; i += width) {
      size_t j = i;
      do {
        char* a;
        char* b;

        j -= wgap;
        a = j + (char *)base;
        b = a + wgap;

        if ((*comp)(a, b, arg) <= 0)
          break;

        k = width;
        do {
          char tmp = *a;
          *a++ = *b;
          *b++ = tmp;
        } while (--k);

      } while(j >= wgap);
    }
  }

  return 0;
}
