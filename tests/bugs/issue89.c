/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * issue89.c - Turtle and TriG cleanup after errors or interrupted input
 *
 * Run with LeakSanitizer to check that freeing a parser releases pending
 * grammar values, deferred statements and partial long literals.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raptor2.h"

typedef struct {
  int errors;
  int statements;
} parse_result;

static void
issue89_log_handler(void *user_data, raptor_log_message *message)
{
  parse_result *result = (parse_result*)user_data;
  if(message->level >= RAPTOR_LOG_LEVEL_ERROR)
    result->errors++;
}

static void
issue89_statement_handler(void *user_data, raptor_statement *statement)
{
  parse_result *result = (parse_result*)user_data;
  (void)statement;
  result->statements++;
}

/* finish: 0 abandons input, 1 sends a separate EOF, 2 marks the last chunk.
 * expect_error: -1 accepts either outcome when sweeping a mixed corpus. */
static int
issue89_parse(const char *syntax, const unsigned char *content, size_t len,
              size_t chunk_size, int finish, int expect_error)
{
  raptor_world *world = raptor_new_world();
  raptor_parser *parser = NULL;
  raptor_uri *base_uri = NULL;
  parse_result result = { 0, 0 };
  size_t offset;
  int rc = 0;
  int failed = 1;
  int errors, statements;

  if(!world)
    return 1;
  raptor_world_set_log_handler(world, &result, issue89_log_handler);
  if(raptor_world_open(world))
    goto cleanup;
  parser = raptor_new_parser(world, syntax);
  base_uri = raptor_new_uri(world,
                            (const unsigned char*)"http://example.org/doc");
  if(!parser || !base_uri)
    goto cleanup;
  raptor_parser_set_statement_handler(parser, &result,
                                      issue89_statement_handler);
  if(raptor_parser_parse_start(parser, base_uri))
    goto cleanup;

  /* Stop at the first error, without feeding a final EOF chunk. */
  for(offset = 0; offset < len && !rc; offset += chunk_size) {
    size_t count = len - offset;
    if(count > chunk_size)
      count = chunk_size;
    rc = raptor_parser_parse_chunk(parser, content + offset, count,
                                   finish == 2 && offset + count == len);
  }
  if(!rc && finish == 1)
    rc = raptor_parser_parse_chunk(parser, NULL, 0, 1);
  failed = expect_error >= 0 && (!!(rc || result.errors) != expect_error);

  /* Destruction must not parse more input or call application handlers. */
  errors = result.errors;
  statements = result.statements;
  raptor_free_parser(parser);
  parser = NULL;
  if(errors != result.errors || statements != result.statements)
    failed = 1;

cleanup:
  raptor_free_parser(parser);
  raptor_free_uri(base_uri);
  raptor_free_world(world);
  return failed;
}

int
main(int argc, char **argv)
{
  static const char *bad[] = {
    "<s> <p> 123e .",
    "<s> <p> <o> ; <q> [ <r> <t> ] ; <u> ? .",
    "<s> <p> ( <o> [ <q> <r> ] ? ) .",
    "<s> <p> \"value\"^^missing:type .",
    "<s> <p> \"\"\"unterminated",
    "<s> <p> [ <q> <r> ]",
    "<s> <p> <o> ; <q> <r> ; <t> ."
  };
  static const char valid[] =
    "<s> <p> ( <o> [ <q> \"value\" ] ) ; <r> \"\"\"long\ntext\"\"\" .";
  static const char graph[] =
    "<graph> { <s> <p> ( <o> [ <q> \"value\" ] ) . }";
  const char *syntaxes[] = { "turtle", "trig" };
  size_t i, j, chunk, len;
  int failures = 0;

  /* Optional corpus input for sanitizer sweeps. */
  if(argc == 3) {
    FILE *fh = fopen(argv[2], "rb");
    unsigned char *content;
    long size;
    if(!fh)
      return 1;
    if(fseek(fh, 0, SEEK_END) || (size = ftell(fh)) < 0 ||
       fseek(fh, 0, SEEK_SET)) {
      fclose(fh);
      return 1;
    }
    content = (unsigned char*)malloc((size_t)size + 1);
    if(!content) {
      fclose(fh);
      return 1;
    }
    len = fread(content, 1, (size_t)size, fh);
    fclose(fh);
    failures = len != (size_t)size;
    failures += issue89_parse(argv[1], content, len, len ? len : 1, 1, -1);
    failures += issue89_parse(argv[1], content, len, 1, 1, -1);
    failures += issue89_parse(argv[1], content, len, len ? len : 1, 2, -1);
    failures += issue89_parse(argv[1], content, len, 1, 2, -1);
    free(content);
    return failures;
  }

  for(i = 0; i < sizeof(syntaxes) / sizeof(syntaxes[0]); i++) {
    for(j = 0; j < sizeof(bad) / sizeof(bad[0]); j++) {
      len = strlen(bad[j]);
      for(chunk = 1; chunk <= len; chunk++) {
        if(issue89_parse(syntaxes[i], (const unsigned char*)bad[j],
                         len, chunk, 1, 1)) {
          fprintf(stderr, "%s: bad input %d with %d-byte chunks failed\n",
                  syntaxes[i], (int)j, (int)chunk);
          failures++;
        }
        failures += issue89_parse(syntaxes[i], (const unsigned char*)bad[j],
                                    len, chunk, 2, 1);
      }
    }
    /* Free at every incomplete prefix of a valid document. */
    for(len = 0; len <= strlen(valid); len++)
      failures += issue89_parse(syntaxes[i], (const unsigned char*)valid,
                                 len, 1, 0, 0);
    failures += issue89_parse(syntaxes[i], (const unsigned char*)valid,
                               strlen(valid), 1, 1, 0);
  }
  for(len = 0; len <= strlen(graph); len++)
    failures += issue89_parse("trig", (const unsigned char*)graph,
                               len, 1, 0, 0);
  failures += issue89_parse("trig", (const unsigned char*)graph,
                             strlen(graph), 1, 2, 0);
  return failures ? 1 : 0;
}
