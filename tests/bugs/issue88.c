/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * issue88.c - Raptor test for GitHub issue 88
 * Turtle and TriG tokens split across parse_chunk boundaries
 *
 * Parses documents as one chunk and then in fixed-size chunks, and
 * checks every chunking gives the same triples with no errors.
 */

#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Raptor includes */
#include "raptor2.h"
#include "raptor_internal.h"


/* Uses every kind of Turtle token, with long ones */
static const char turtle_content[] =
  "# Turtle with every kind of token\n"
  "@prefix ex: <http://example.org/vocabulary/terms#> .\n"
  "PREFIX xsd: <http://www.w3.org/2001/XMLSchema#>\n"
  "@base <http://example.org/base/> .\n"
  "ex:subjectWithALongLocalName ex:predicateWithALongLocalName\n"
  "    <relative/object> , <http://example.org/absolute/object> ;\n"
  "  a ex:Class ;\n"
  "  ex:string \"a string with \\\"escapes\\\" and \\u00e9\"@en-GB ;\n"
  "  ex:single 'single quoted' ;\n"
  "  ex:long \"\"\"a long\n"
  "string with \"quotes\" and \"\"two\"\" quotes\"\"\" ;\n"
  "  ex:longsingle '''another\n"
  "long string''' ;\n"
  "  ex:typed \"12345678\"^^xsd:integer ;\n"
  "  ex:numbers 1234567890 , -12.345 , 6.02e23 , +7 ;\n"
  "  ex:bools true , false ;\n"
  "  ex:blank _:blankNodeLabel , [ ex:inner \"inside\" ] ;\n"
  "  ex:list ( ex:one \"two\" 3 ( ) ) .\n"
  "_:blankNodeLabel ex:p ex:o . # trailing comment\n";

/* TriG graph names are matched as a token including the '{' */
static const char trig_content[] =
  "@prefix ex: <http://example.org/> .\n"
  "<http://example.org/graphs/one> {\n"
  "  ex:s ex:p \"in graph one\" .\n"
  "}\n"
  "ex:graphTwo = { ex:s ex:p \"in graph two\" . }\n"
  "{ ex:s ex:p \"in default graph\" . }\n";


typedef struct {
  raptor_stringbuffer *sb;
  int errors;
} parse_result;


static void
issue88_statement_handler(void *user_data, raptor_statement *statement)
{
  parse_result *result = (parse_result*)user_data;
  raptor_term *terms[4];
  int i;

  terms[0] = statement->subject;
  terms[1] = statement->predicate;
  terms[2] = statement->object;
  terms[3] = statement->graph;

  for(i = 0; i < 4; i++) {
    unsigned char *s;

    if(!terms[i])
      continue;
    s = raptor_term_to_string(terms[i]);
    if(s) {
      raptor_stringbuffer_append_string(result->sb, s, 1);
      raptor_free_memory(s);
    }
    raptor_stringbuffer_append_counted_string(result->sb,
                                              (const unsigned char*)" ", 1, 1);
  }
  raptor_stringbuffer_append_counted_string(result->sb,
                                            (const unsigned char*)"\n", 1, 1);
}


static void
issue88_log_handler(void *user_data, raptor_log_message *message)
{
  parse_result *result = (parse_result*)user_data;

  if(message->level >= RAPTOR_LOG_LEVEL_ERROR)
    result->errors++;
}


/* Parse content in chunks of chunk_size bytes; returns the triples as a
 * string or NULL on failure.  Errors are counted in *errors_p.  Uses a
 * new world each time so generated blank node IDs are the same. */
static unsigned char*
issue88_parse(const char *syntax,
              const unsigned char *content, size_t content_len,
              size_t chunk_size, int *errors_p)
{
  raptor_world *world = NULL;
  raptor_parser *parser = NULL;
  raptor_uri *base_uri = NULL;
  parse_result result;
  unsigned char *triples = NULL;
  size_t offset;

  result.sb = raptor_new_stringbuffer();
  result.errors = 0;
  if(!result.sb)
    return NULL;

  world = raptor_new_world();
  if(!world)
    goto cleanup;
  raptor_world_set_log_handler(world, &result, issue88_log_handler);
  if(raptor_world_open(world))
    goto cleanup;

  base_uri = raptor_new_uri(world,
                            (const unsigned char*)"http://example.org/doc");
  parser = raptor_new_parser(world, syntax);
  if(!base_uri || !parser)
    goto cleanup;

  raptor_parser_set_statement_handler(parser, &result,
                                      issue88_statement_handler);

  if(raptor_parser_parse_start(parser, base_uri))
    goto cleanup;

  for(offset = 0; offset < content_len; offset += chunk_size) {
    size_t len = content_len - offset;
    if(len > chunk_size)
      len = chunk_size;
    if(raptor_parser_parse_chunk(parser, content + offset, len, 0))
      result.errors++;
  }
  if(raptor_parser_parse_chunk(parser, NULL, 0, 1))
    result.errors++;

  triples = RAPTOR_MALLOC(unsigned char*,
                          raptor_stringbuffer_length(result.sb) + 1);
  if(triples)
    raptor_stringbuffer_copy_to_string(result.sb, triples,
                                       raptor_stringbuffer_length(result.sb) + 1);

  cleanup:
  raptor_free_parser(parser);
  raptor_free_uri(base_uri);
  raptor_free_stringbuffer(result.sb);
  raptor_free_world(world);

  *errors_p = result.errors;
  return triples;
}


/* Check content parses the same at each chunk size from min_chunk to
 * max_chunk (stepping by step) as it does as one chunk */
static int
issue88_check(const char *program, const char *syntax,
              const char *label, const unsigned char *content,
              size_t content_len, size_t min_chunk, size_t max_chunk,
              size_t step)
{
  unsigned char *expected;
  int errors = 0;
  int failures = 0;
  size_t chunk_size;

  expected = issue88_parse(syntax, content, content_len,
                           content_len, &errors);
  if(!expected || errors || !*expected) {
    fprintf(stderr, "%s: %s %s failed to parse as one chunk\n",
            program, syntax, label);
    RAPTOR_FREE(char*, expected);
    return 1;
  }

  for(chunk_size = min_chunk; chunk_size <= max_chunk; chunk_size += step) {
    unsigned char *triples;

    triples = issue88_parse(syntax, content, content_len, chunk_size,
                            &errors);
    if(!triples || errors || strcmp((const char*)triples,
                                    (const char*)expected)) {
      fprintf(stderr, "%s: %s %s gave different triples with %d-byte chunks\n",
              program, syntax, label, (int)chunk_size);
      failures++;
    }
    RAPTOR_FREE(char*, triples);
  }

  RAPTOR_FREE(char*, expected);
  return failures;
}


int
main(int argc, const char** argv)
{
  const char *program = raptor_basename(argv[0]);
  unsigned char *long_content = NULL;
  size_t long_len;
  size_t i;
  int failures = 0;
  static const char long_prefix[] = "<http://example.org/s> <http://example.org/p> \"\"\"";
  static const char long_suffix[] = "\"\"\" .\n<http://example.org/s> <http://example.org/q> \"after\" .\n";
#define LONG_LITERAL_LEN 20000

  (void)argc;

  /* Every chunk size from 1 byte to the whole document */
  failures += issue88_check(program, "turtle", "tokens",
                            (const unsigned char*)turtle_content,
                            strlen(turtle_content),
                            1, strlen(turtle_content), 1);
  failures += issue88_check(program, "trig", "graphs",
                            (const unsigned char*)trig_content,
                            strlen(trig_content),
                            1, strlen(trig_content), 1);

  /* A long literal bigger than the parser read buffer, split at many
   * places including buffer-sized boundaries */
  long_len = strlen(long_prefix) + LONG_LITERAL_LEN + strlen(long_suffix);
  long_content = RAPTOR_MALLOC(unsigned char*, long_len + 1);
  if(!long_content) {
    failures++;
    goto cleanup;
  }
  memcpy(long_content, long_prefix, strlen(long_prefix));
  for(i = 0; i < LONG_LITERAL_LEN; i++)
    long_content[strlen(long_prefix) + i] = (unsigned char)('a' + (i % 26));
  memcpy(long_content + strlen(long_prefix) + LONG_LITERAL_LEN, long_suffix,
         strlen(long_suffix) + 1);

  failures += issue88_check(program, "turtle", "long literal",
                            long_content, long_len, 1000, 9000, 1000);
  failures += issue88_check(program, "turtle", "long literal",
                            long_content, long_len, 4095, 4097, 1);
  failures += issue88_check(program, "turtle", "long literal",
                            long_content, long_len, 8191, 8193, 1);

  cleanup:
  RAPTOR_FREE(char*, long_content);

  return failures;
}
