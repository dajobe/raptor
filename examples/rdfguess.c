#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <raptor2.h>


/* rdfguess.c: guess parser name from filename and its content */

#define READ_BUFFER_SIZE 256

static int
rdfguess_guess_name(raptor_world* world, const char* filename)
{
  char *buffer[READ_BUFFER_SIZE];
  raptor_iostream* iostr = NULL;
  const char* name;
  const unsigned char* identifier;
  const char* label;
  size_t read_len;
  size_t count;
  
  if(!strcmp(filename, "-")) {
    iostr = raptor_new_iostream_from_file_handle(world, stdin);
    identifier = NULL;
    label = "<stdin>";
  } else {
    if(access(filename, R_OK)) {
      fprintf(stderr, "rdfguess: file %s not found\n", filename);
      return 1;
    }
    
    iostr = raptor_new_iostream_from_filename(world, filename);
    identifier = (const unsigned char *)filename;
    label = filename;
  }
  
  if(!iostr) {
    fprintf(stderr, "rdfguess: Could not create iostream for %s\n", label);
    goto tidy;
  }

  read_len = READ_BUFFER_SIZE;
  count = raptor_iostream_read_bytes(buffer, 1, read_len, iostr);
  if(count < 1) {
    fprintf(stderr, "rdfguess: Failed to read any data from %s\n",
            label);
    goto tidy;
  }
  
  name = raptor_world_guess_parser_name(world,
                                        /* uri*/ NULL,
                                        /* mime_type */ NULL,
                                        (const unsigned char*)buffer,
                                        read_len, 
                                        identifier);
                                        
  if(name)
    fprintf(stdout, "rdfguess: %s guessed to be %s\n", label, name);
  else
    fprintf(stdout, "rdfguess: failed to guess parser for %s\n", label);
  
  tidy:
  raptor_free_iostream(iostr);
    
  return 0;
}
      

int
main(int argc, char *argv[])
{
  raptor_world *world = NULL;
  const char *filename;
  int rc = 0;
  
  world = raptor_new_world();

  if(argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
    fprintf(stderr, "USAGE rdfguess [FILENAMES...]\n");
    rc = 1;
    goto tidy;
  }

  if(argc == 1) {
    rc = rdfguess_guess_name(world, "-");
  } else {
    int i;

    for(i = 1; (filename = (const char*)argv[i]); i++) {
      rc = rdfguess_guess_name(world, filename);
      if(rc)
        break;
    }
  }

  tidy:
  raptor_free_world(world);

  return rc;
}
