#include <stdio.h>
#include <raptor2.h>
#include <stdlib.h>

/* rdfserialize.c: serialize 1 triple to RDF/XML-Abbrev */

int
main(int argc, char *argv[])
{
  raptor_world *world = NULL;
  raptor_serializer* rdf_serializer = NULL;
  unsigned char *uri_string;
  raptor_uri *base_uri;
  raptor_statement* triple;

  world = raptor_new_world();
  
  uri_string = raptor_uri_filename_to_uri_string(argv[1]);
  base_uri = raptor_new_uri(world, uri_string);

  rdf_serializer = raptor_new_serializer(world, "rdfxml-abbrev");
  raptor_serializer_start_to_file_handle(rdf_serializer, base_uri, stdout);
  
  /* Make a triple with URI subject, URI predicate, literal object */
  triple = raptor_new_statement(world);
  triple->subject = raptor_new_term_from_uri_string(world, (const unsigned char*)"http://example.org/subject");
  triple->predicate = raptor_new_term_from_uri_string(world, (const unsigned char*)"http://example.org/predicate");
  triple->object = raptor_new_term_from_literal(world,
                                                (const unsigned char*)"An example literal",
                                                NULL,
                                                (const unsigned char*)"en");

  /* Write the triple */
  raptor_serializer_serialize_statement(rdf_serializer, triple);

  /* Delete the triple */
  raptor_free_statement(triple);

  raptor_serializer_serialize_end(rdf_serializer);
  raptor_free_serializer(rdf_serializer);
  
  raptor_free_uri(base_uri);
  raptor_free_memory(uri_string);

  raptor_free_world(world);
  return 0;
}
