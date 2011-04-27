#include <stdio.h>
#include <raptor2.h>

/* rdfcat.c: parse any RDF syntax and serialize to RDF/XML-Abbrev */

static raptor_serializer* rdf_serializer;

static void
serialize_triple(void* user_data, raptor_statement* triple) 
{
  raptor_serializer_serialize_statement(rdf_serializer, triple);
}

static void
declare_namespace(void* user_data, raptor_namespace *nspace)
{
  raptor_serializer_set_namespace_from_namespace(rdf_serializer, nspace);
}

int
main(int argc, char *argv[])
{
  raptor_world *world = NULL;
  raptor_parser* rdf_parser = NULL;
  unsigned char *uri_string;
  raptor_uri *uri, *base_uri;

  world = raptor_new_world();

  uri_string = raptor_uri_filename_to_uri_string(argv[1]);
  uri = raptor_new_uri(world, uri_string);
  base_uri = raptor_uri_copy(uri);

  /* Ask raptor to work out which parser to use */
  rdf_parser = raptor_new_parser(world, "guess");
  raptor_parser_set_statement_handler(rdf_parser, NULL, serialize_triple);
  raptor_parser_set_namespace_handler(rdf_parser, NULL, declare_namespace);

  rdf_serializer = raptor_new_serializer(world, "rdfxml-abbrev");

  raptor_serializer_start_to_file_handle(rdf_serializer, base_uri, stdout);
  raptor_parser_parse_file(rdf_parser, uri, base_uri);
  raptor_serializer_serialize_end(rdf_serializer);

  raptor_free_serializer(rdf_serializer);
  raptor_free_parser(rdf_parser);

  raptor_free_uri(base_uri);
  raptor_free_uri(uri);
  raptor_free_memory(uri_string);

  raptor_free_world(world);

  return 0;
}
