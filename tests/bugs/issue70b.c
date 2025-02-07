/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * issue70.c - Raptor test for GitHub issue 70 second part
 * Heap read buffer overflow in raptor_ntriples_parse_term_internal()
 *
 * N-Triples test content: "_:/exaple/o"
 */

#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#include <string.h>

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


int
main(int argc, const char** argv)
{
  const char *program = raptor_basename(argv[0]);
  const unsigned char* ntriples_content = (const unsigned char*)"_:/exaple/o\n";
#define NTRIPLES_CONTENT_LEN 12
  const unsigned char* base_uri_string = (const unsigned char*)"http:o/www.w3.org/2001/sw/DataA#cess/df1.ttl";
  int failures = 0;
  raptor_world* world = NULL;
  raptor_uri* base_uri = NULL;
  raptor_parser* parser = NULL;
  int result;

  world = raptor_new_world();
  if(!world)
    goto cleanup;
  base_uri = raptor_new_uri(world, base_uri_string);
  if(!base_uri)
    goto cleanup;
  parser = raptor_new_parser(world, "ntriples");
  if(!parser)
    goto cleanup;

  (void)raptor_parser_parse_start(parser, base_uri);
  result = raptor_parser_parse_chunk(parser,
                                     ntriples_content,
                                     NTRIPLES_CONTENT_LEN, /* is_end */ 1);

  if(result) {
    fprintf(stderr, "%s: parsing '%s' N-Triples content failed with result %d\n", program, ntriples_content, result);
    fprintf(stderr, "%s: Base URI: '%s' (%lu)\n",
            program, base_uri_string, strlen((const char*)base_uri_string));
    failures++;
  }

  cleanup:
  raptor_free_parser(parser);
  raptor_free_uri(base_uri);
  raptor_free_world(world);

  return failures;
}
