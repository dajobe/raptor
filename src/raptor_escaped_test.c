/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * raptor_escaped_test.c - Tests for counted strings written with escapes
 *
 * Copyright (C) 2026, Dave Beckett https://www.dajobe.org/
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
#include <string.h>

#include "raptor2.h"

typedef struct {
  const char* name;
  const unsigned char* input;
  size_t input_len;
  unsigned int flags;
  const unsigned char* expected;
  size_t expected_len;
  int expect_error;
} escaped_case;

static const escaped_case cases[] = {
  { "zero length", (const unsigned char*)"abc", 0,
    RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL, (const unsigned char*)"", 0, 0 },
  { "leading NUL", (const unsigned char*)"\0a", 2,
    RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL,
    (const unsigned char*)"\\u0000a", 7, 0 },
  { "embedded NUL", (const unsigned char*)"a\0b", 3,
    RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL,
    (const unsigned char*)"a\\u0000b", 8, 0 },
  { "trailing NUL", (const unsigned char*)"a\0", 2,
    RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL,
    (const unsigned char*)"a\\u0000", 7, 0 },
  { "multiple NULs", (const unsigned char*)"a\0b\0c", 5,
    RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL,
    (const unsigned char*)"a\\u0000b\\u0000c", 15, 0 },
  { "counted prefix", (const unsigned char*)"a\0b", 2,
    RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL,
    (const unsigned char*)"a\\u0000", 7, 0 },
  { "raw NUL", (const unsigned char*)"a\0b", 3, 0,
    (const unsigned char*)"a\0b", 3, 0 },
  { "JSON NUL and UTF-8", (const unsigned char*)"a\0\xc3\xa9", 4,
    RAPTOR_ESCAPED_WRITE_JSON_LITERAL,
    (const unsigned char*)"a\\u0000\xc3\xa9", 9, 0 },
  { "truncated UTF-8 after NUL", (const unsigned char*)"a\0\xc3\xa9", 3,
    RAPTOR_ESCAPED_WRITE_NTRIPLES_LITERAL, NULL, 0, 1 }
};

int main(int argc, char *argv[]);

int
main(int argc, char *argv[])
{
  raptor_world* world = raptor_new_world();
  size_t i;
  int failed = 0;

  (void)argc;
  if(!world || raptor_world_open(world))
    return 1;

  for(i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
    const escaped_case* test = &cases[i];
    void* output = NULL;
    size_t output_len = 0;
    raptor_iostream* iostr = raptor_new_iostream_to_string(world, &output,
                                                           &output_len, NULL);
    int rc;
    int case_failed = 0;

    if(!iostr) {
      fprintf(stderr, "%s: could not create output stream\n", argv[0]);
      failed = 1;
      break;
    }
    rc = raptor_string_escaped_write(test->input, test->input_len, '"',
                                     test->flags, iostr);
    raptor_free_iostream(iostr);
    if(!output)
      /* no output is always a failure */
      case_failed = 1;
    else if(rc && !test->expect_error)
      /* error expected but got success code */
      case_failed = 1;
    else if(!rc && test->expect_error)
      /* success expected but got error code */
      case_failed = 1;
    else if(!rc && (output_len != test->expected_len ||
                    memcmp(output, test->expected, test->expected_len)))
      /* success code but value is not as expected */
      case_failed = 1;

    if(case_failed) {
      fprintf(stderr, "%s: escaped string case '%s' failed\n",
              argv[0], test->name);
      failed = 1;
    }
    if(output)
      raptor_free_memory(output);
  }

  raptor_free_world(world);
  return failed;
}
