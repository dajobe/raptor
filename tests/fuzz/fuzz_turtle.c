#include <raptor2.h>
#include <stdint.h>
#include <stddef.h>

static raptor_world *world = NULL;
static raptor_uri *base_uri = NULL;

static void
noop_statement_handler(void *user_data, raptor_statement *statement)
{
  (void)user_data;
  (void)statement;
}

static void
noop_log_handler(void *user_data, raptor_log_message *message)
{
  (void)user_data;
  (void)message;
}

int LLVMFuzzerInitialize(int *argc, char ***argv);
int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size);

int
LLVMFuzzerInitialize(int *argc, char ***argv)
{
  (void)argc;
  (void)argv;

  world = raptor_new_world();
  if(!world)
    return 1;
  raptor_world_open(world);
  raptor_world_set_log_handler(world, NULL, noop_log_handler);

  base_uri = raptor_new_uri(world,
                            (const unsigned char*)"http://example.invalid/base");
  if(!base_uri)
    return 1;

  return 0;
}

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
  raptor_parser *parser = NULL;

  parser = raptor_new_parser(world, "turtle");
  if(!parser)
    return 0;

  raptor_parser_set_statement_handler(parser, NULL, noop_statement_handler);
  raptor_parser_set_option(parser, RAPTOR_OPTION_NO_NET, NULL, 1);
  raptor_parser_set_option(parser, RAPTOR_OPTION_NO_FILE, NULL, 1);

  if(!raptor_parser_parse_start(parser, base_uri)) {
    size_t offset = 0;
    while(offset < size) {
      size_t remaining = size - offset;
      size_t chunk = (size_t)((unsigned int)data[offset] % 32U) + 1U;
      if(chunk > remaining)
        chunk = remaining;
      (void)raptor_parser_parse_chunk(parser, data + offset, chunk,
                                      (offset + chunk == size));
      offset += chunk;
    }
    if(size == 0)
      (void)raptor_parser_parse_chunk(parser, data, 0, 1);
  }

  raptor_free_parser(parser);
  return 0;
}

#if !defined(RAPTOR_USE_LIBFUZZER)
int
main(void)
{
  /* Built without a fuzzer runtime; nothing to do. */
  return 0;
}
#endif
