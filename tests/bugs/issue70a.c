/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * issue70a.c - Raptor test for GitHub issue 70 first part
 * Integer Underflow in raptor_uri_normalize_path()
 *
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
  const unsigned char* base_uri=      (const unsigned char*)"http:o/www.w3.org/2001/sw/DataA#cess/df1.ttl";
  const unsigned char* reference_uri= (const unsigned char*)".&/../?D/../../1999/02/22-rdf-syntax-ns#";
#define BUFFER_LEN 84
  unsigned char buffer[BUFFER_LEN + 1];
  size_t buffer_length = BUFFER_LEN + 1;
  int failures = 0;
#define EXPECTED_RESULT "http:?D/../../1999/02/22-rdf-syntax-ns#"
#define EXPECTED_RESULT_LEN 39UL
  int result;
  size_t result_len;

  buffer[0] = '\0';

  /* Crash used to happens here if RAPTOR_DEBUG > 3
   * raptor_rfc2396.c:398:raptor_uri_normalize_path: fatal error: Path length 0 does not match calculated -5.
   */
  result = raptor_uri_resolve_uri_reference(base_uri, reference_uri,
                                            buffer, buffer_length);
  result_len = strlen((const char*)buffer);

  if(strcmp((const char*)buffer, EXPECTED_RESULT) ||
     result_len != EXPECTED_RESULT_LEN) {
    fprintf(stderr, "%s: raptor_uri_resolve_uri_reference() failed with result %d\n", program, result);
    fprintf(stderr, "%s: Base URI: '%s' (%lu)\n",
            program, base_uri, strlen((const char*)base_uri));
    fprintf(stderr, "%s: Ref  URI: '%s' (%lu)\n", reference_uri,
            program, strlen((const char*)reference_uri));
    fprintf(stderr, "%s: Result buffer: '%s' (%lu)\n", program,
            buffer, strlen((const char*)buffer));
    fprintf(stderr, "%s: Expected: '%s' (%lu)\n", program,
            EXPECTED_RESULT, EXPECTED_RESULT_LEN);
    failures++;
  }

  return failures;
}
