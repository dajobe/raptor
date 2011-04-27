#include <stdio.h>
#include <raptor2.h>

/* rdfprint.c: print triples from parsing RDF/XML */

static void
print_triple(void* user_data, raptor_statement* triple) 
{
  raptor_statement_print_as_ntriples(triple, stdout);
  fputc('\n', stdout);
}

int
main(int argc, char *argv[])
{
  raptor_world *world = NULL;
  raptor_parser* rdf_parser = NULL;
  unsigned char *uri_string;
  raptor_uri *uri, *base_uri;

  world = raptor_new_world();

  rdf_parser = raptor_new_parser(world, "rdfxml");

  raptor_parser_set_statement_handler(rdf_parser, NULL, print_triple);

  uri_string = raptor_uri_filename_to_uri_string(argv[1]);
  uri = raptor_new_uri(world, uri_string);
  base_uri = raptor_uri_copy(uri);

  raptor_parser_parse_file(rdf_parser, uri, base_uri);

  raptor_free_parser(rdf_parser);

  raptor_free_uri(base_uri);
  raptor_free_uri(uri);
  raptor_free_memory(uri_string);

  raptor_free_world(world);

  return 0;
}
