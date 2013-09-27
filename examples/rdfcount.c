#include <stdio.h>
#include <raptor2.h>

/* rdfcount.c: parse any number of RDF/XML files and count the triples */

static void
count_triples(void* user_data, raptor_statement* triple)
{
  unsigned int* count_p = (unsigned int*)user_data;
  (*count_p)++;
}

int
main(int argc, char *argv[])
{
  raptor_world *world = NULL;
  const char* program = "rdfcount";
  raptor_parser* rdf_parser = NULL;
  unsigned int i;
  unsigned int count;
  unsigned int files_count = 0;
  unsigned int total_count = 0;

  world = raptor_new_world();

  /* just one parser is used and reused here */
  rdf_parser = raptor_new_parser(world, "rdfxml");

  raptor_parser_set_statement_handler(rdf_parser, &count, count_triples);

  for(i = 1; argv[i]; i++) {
    const char* filename = argv[i];
    unsigned char *uri_string;
    raptor_uri *uri, *base_uri;

    uri_string = raptor_uri_filename_to_uri_string(filename);
    uri = raptor_new_uri(world, uri_string);
    base_uri = raptor_uri_copy(uri);

    count = 0;
    if(!raptor_parser_parse_file(rdf_parser, uri, base_uri)) {
      fprintf(stderr, "%s: %s : %d triples\n", program, filename, count);
      total_count += count;
      files_count++;
    } else {
      fprintf(stderr, "%s: %s : failed to parse\n", program, filename);
    }

    raptor_free_uri(base_uri);
    raptor_free_uri(uri);
    raptor_free_memory(uri_string);
  }

  raptor_free_parser(rdf_parser);

  raptor_free_world(world);

  fprintf(stderr, "%s: Total count: %d files  %d triples\n",
          program, files_count, total_count);

  return 0;
}
