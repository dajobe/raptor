#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <raptor.h>


/* rdfguess.c: guess parser name from filename and its content */

#define READ_BUFFER_SIZE 256

int
main(int argc, char *argv[])
{
  raptor_world *world = NULL;
  char *buffer[READ_BUFFER_SIZE];
  const char *filename;
  int rc = 1;
  int i;
  
  world = raptor_new_world();

  if(argc < 2) {
    fprintf(stderr, "USAGE rdfguess: FILENAMES...\n");
    goto tidy;
  }
  
  for(i = 1; (filename = (const char*)argv[i]); i++) {
    raptor_iostream* iostr = NULL;
    const char* name;
    size_t read_len;
    size_t count;

    if(access(filename, R_OK)) {
      fprintf(stderr, "rdfguess: %s not found\n", filename);
      goto tidy;
    }

    iostr = raptor_new_iostream_from_filename(world, filename);
    if(!iostr) {
      fprintf(stderr, "rdfguess: Could not create iostream for %s\n", filename);
      goto tidy;
    }

    read_len = READ_BUFFER_SIZE;
    count = raptor_iostream_read_bytes(buffer, 1, read_len, iostr);
    if(count < 1) {
      fprintf(stderr, "rdfguess: Failed to read any data from file %s\n",
              filename);
      raptor_free_iostream(iostr);
      goto tidy;
    }


    name = raptor_world_guess_parser_name(world,
                                          /* uri*/ NULL,
                                          /* mime_type */ NULL,
                                          (const unsigned char*)buffer,
                                          read_len, 
                                          /* identifier */ (const unsigned char *)filename);

    if(name)
      fprintf(stdout, "rdfguess: %s guessed to be %s\n", filename, name);
    else
      fprintf(stdout, "rdfguess: failed to guess parser for %s\n", filename);

    raptor_free_iostream(iostr);
  }
  
  rc = 0;

  tidy:
  raptor_free_world(world);

  return rc;
}
