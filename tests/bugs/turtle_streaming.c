/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * turtle_streaming.c - Long Turtle and TriG tokens in small input chunks
 *
 * Checks token contents, graph names, EOF handling and cleanup without
 * requiring a large unfinished token to be rescanned for every byte.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "raptor2.h"

#define LONG_TOKEN_SIZE 65536
#define LARGE_TOKEN_SIZE (1024 * 1024)

typedef struct {
  const char *value;
  int uri_object;
  int named_graph;
  int statements;
  int errors;
  int error_line;
  int invalid;
} streaming_result;

static void
streaming_log_handler(void *user_data, raptor_log_message *message)
{
  streaming_result *result = (streaming_result*)user_data;
  if(message->level >= RAPTOR_LOG_LEVEL_ERROR) {
    result->errors++;
    if(message->locator)
      result->error_line = message->locator->line;
  }
}

static void
streaming_statement_handler(void *user_data, raptor_statement *statement)
{
  streaming_result *result = (streaming_result*)user_data;
  const unsigned char *value = NULL;

  result->statements++;
  if(result->named_graph) {
    if(!statement->graph || statement->graph->type != RAPTOR_TERM_TYPE_URI ||
       strcmp((const char*)raptor_uri_as_string(statement->graph->value.uri),
              "http://example.org/g"))
      result->invalid++;
  } else if(statement->graph)
    result->invalid++;

  if(!statement->object) {
    result->invalid++;
    return;
  }
  if(result->statements == 1 && result->uri_object) {
    if(statement->object->type == RAPTOR_TERM_TYPE_URI)
      value = raptor_uri_as_string(statement->object->value.uri);
  } else if(statement->object->type == RAPTOR_TERM_TYPE_LITERAL)
    value = statement->object->value.literal.string;

  if(!value || strcmp((const char*)value,
                     result->statements == 1 ? result->value : "after") ||
     result->statements > 2)
    result->invalid++;
}

/* finish: 0 abandons input, 1 sends a separate EOF, 2 ends the last chunk. */
static int
streaming_parse(const char *syntax, const char *document, size_t length,
                size_t chunk_size, int finish, const char *value,
                int uri_object, int expected_statements, int expected_error,
                int expected_line)
{
  raptor_world *world = raptor_new_world();
  raptor_parser *parser = NULL;
  raptor_uri *base_uri = NULL;
  streaming_result result;
  size_t offset;
  int rc = 0;
  int failed = 1;
  int statements, errors;

  memset(&result, 0, sizeof(result));
  result.value = value;
  result.uri_object = uri_object;
  result.named_graph = !strcmp(syntax, "trig");
  if(!world)
    return 1;
  raptor_world_set_log_handler(world, &result, streaming_log_handler);
  if(raptor_world_open(world))
    goto cleanup;
  parser = raptor_new_parser(world, syntax);
  base_uri = raptor_new_uri(world,
                            (const unsigned char*)"http://example.org/doc");
  if(!parser || !base_uri)
    goto cleanup;
  raptor_parser_set_statement_handler(parser, &result,
                                      streaming_statement_handler);
  if(raptor_parser_parse_start(parser, base_uri))
    goto cleanup;

  for(offset = 0; offset < length && !rc; offset += chunk_size) {
    size_t count = length - offset;
    if(count > chunk_size)
      count = chunk_size;
    rc = raptor_parser_parse_chunk(parser,
                                   (const unsigned char*)document + offset,
                                   count,
                                   finish == 2 && offset + count == length);
  }
  if(!rc && finish == 1)
    rc = raptor_parser_parse_chunk(parser, NULL, 0, 1);
  failed = result.invalid || result.statements != expected_statements ||
           ((rc != 0 || result.errors != 0) != expected_error) ||
           (expected_error && result.error_line != expected_line);

  statements = result.statements;
  errors = result.errors;
  raptor_free_parser(parser);
  parser = NULL;
  if(statements != result.statements || errors != result.errors)
    failed = 1;
  if(failed)
    fprintf(stderr,
            "%s: %lu-byte chunks, finish %d: statements %d, errors %d, "
            "line %d, invalid %d\n",
            syntax, (unsigned long)chunk_size, finish, result.statements,
            result.errors, result.error_line, result.invalid);

cleanup:
  raptor_free_parser(parser);
  raptor_free_uri(base_uri);
  raptor_free_world(world);
  return failed;
}

int
main(void)
{
  static const char *syntaxes[] = { "turtle", "trig" };
  static const char *openers[] = { "\"", "\"\"\"", "'''", "<" };
  static const char *closers[] = { "\"", "\"\"\"", "'''", ">" };
  static const size_t chunk_sizes[] = { 1, 25, 4096 };
  char *value = (char*)malloc(LARGE_TOKEN_SIZE + 32);
  char *document = (char*)malloc(LARGE_TOKEN_SIZE + 256);
  size_t syntax, kind, chunk, i, prefix_length, value_length, length;
  int failures = 0;
  int finish;

  if(!value || !document) {
    free(value);
    free(document);
    return 1;
  }

  for(syntax = 0; syntax < sizeof(syntaxes) / sizeof(syntaxes[0]); syntax++) {
    for(kind = 0; kind < sizeof(openers) / sizeof(openers[0]); kind++) {
      int uri_object = kind == 3;
      int expected_line = 1;
      size_t incomplete_length;
      const char *graph_start = syntax ? "<http://example.org/g> { " : "";
      const char *graph_end = syntax ? "}\n" : "";
      size_t start = uri_object ? strlen("http://example.org/") : 0;

      if(uri_object)
        memcpy(value, "http://example.org/", start);
      for(i = 0; i < LONG_TOKEN_SIZE; i++)
        value[start + i] = (kind == 1 || kind == 2) && i % 127 == 0 ?
                           '\n' : 'x';
      value_length = start + LONG_TOKEN_SIZE;
      value[value_length] = '\0';
      prefix_length = (size_t)snprintf(document, LONG_TOKEN_SIZE + 256,
                                       "%s<s> <p> %s", graph_start,
                                       openers[kind]);
      memcpy(document + prefix_length, value, value_length);
      length = prefix_length + value_length;
      length += (size_t)snprintf(document + length,
                                  LONG_TOKEN_SIZE + 256 - length,
                                  "%s .\n<s> <q> \"after\" .\n%s",
                                  closers[kind], graph_end);

      incomplete_length = prefix_length + value_length / 2;
      for(i = 0; i < incomplete_length; i++) {
        if(document[i] == '\n')
          expected_line++;
      }
      for(chunk = 0; chunk <= sizeof(chunk_sizes) / sizeof(chunk_sizes[0]);
          chunk++) {
        size_t chunk_size = chunk == sizeof(chunk_sizes) /
                                      sizeof(chunk_sizes[0]) ?
                            length : chunk_sizes[chunk];
        for(finish = 1; finish <= 2; finish++) {
          failures += streaming_parse(syntaxes[syntax], document, length,
                                       chunk_size, finish, value, uri_object,
                                       2, 0, 0);
          failures += streaming_parse(syntaxes[syntax], document,
                                       incomplete_length, chunk_size, finish,
                                       value, uri_object, 0, 1,
                                       expected_line);
        }
        failures += streaming_parse(syntaxes[syntax], document,
                                     incomplete_length, chunk_size, 0,
                                     value, uri_object, 0, 0, 0);
      }
    }
  }

  /* Also exceed Flex's initial buffer by two orders of magnitude. Fixed
   * read sizes would repeatedly rebuild the state for this token even
   * when the parser retries only after the input has doubled. */
  memset(value, 'x', LARGE_TOKEN_SIZE);
  value[LARGE_TOKEN_SIZE] = '\0';
  length = (size_t)snprintf(document, LARGE_TOKEN_SIZE + 256,
                             "<s> <p> \"\"\"%s\"\"\" .\n"
                             "<s> <q> \"after\" .\n", value);
  for(finish = 1; finish <= 2; finish++)
    failures += streaming_parse("turtle", document, length, 25, finish,
                                 value, 0, 2, 0, 0);

  free(value);
  free(document);
  return failures ? 1 : 0;
}
