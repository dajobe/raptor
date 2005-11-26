/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdfcat.c - Raptor RDF Parser example code to merge rdf records
 *
 * Created by Sid Steward (http://accesspdf.com) for Creative Commons
 * to simplify adding or changing records in RDF or Adobe XMP files.
 *
 * Copyright (C) 2000-2005, David Beckett http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology http://www.ilrt.bristol.ac.uk/
 * University of Bristol, UK http://www.bristol.ac.uk/
 * 
 * This package is Free Software and part of Redland http://librdf.org/
 * 
 * It is licensed under the following three licenses as alternatives:
 *   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
 *   2. GNU General Public License (GPL) V2 or any newer version
 *   3. Apache License, V2.0 or any newer version
 * 
 * You may not use this file except in compliance with at least one of
 * the above three licenses.
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * complete terms and further detail along with the license texts for
 * the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
 * 
 * 
 */


#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#ifdef WIN32
#include <win32_raptor_config.h>
#endif

#include <stdio.h>
#include <string.h>

/* Raptor includes */
#include <raptor.h>

/* for access() and R_OK */
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* many places for getopt */
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include <raptor_getopt.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif




#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif

static void print_statements(void *user_data, const raptor_statement *statement);
int main(int argc, char *argv[]);


static char *program=NULL;

/* replace newlines in literal string output with spaces */
static int replace_newlines=0;

/* XMP output? */
static int xmp=0;
/* omit duplicate triplets by subject+predicate; only the first survives */
static int drop_dup_preds=0;

static raptor_serializer* serializer=NULL;


static
void print_statements(void *user_data, const raptor_statement *statement) 
{
  /* replace newlines with spaces if object is a literal string */
  if(replace_newlines && 
     statement->object_type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
    char *s;
    for(s=(char*)statement->object; *s; s++)
      if(*s == '\n')
        *s=' ';
  }

  /* set all subjects to NULL URI for XMP output */
  if(xmp) {
    if(statement->subject_type!= RAPTOR_IDENTIFIER_TYPE_ANONYMOUS &&
       statement->subject)
      {
	*((char*)statement->subject)= 0;
      }
  }

  // ssteward debug
  fputs( statement->subject, stderr );
  fputs( ", ", stderr );
  //fputs( statement->predicate, stderr );
  //fputs( ", ", stderr );
  fputs( statement->object, stderr );
  fputs( "\n", stderr );

  raptor_serialize_statement(serializer, statement);
  return;
}


#ifdef HAVE_GETOPT_LONG
#define HELP_TEXT(short, long, description) "  -" short ", --" long "  " description
#define HELP_ARG(short, long) "--" #long
#define HELP_PAD "\n                          "
#else
#define HELP_TEXT(short, long, description) "  -" short "  " description
#define HELP_ARG(short, long) "-" #short
#define HELP_PAD "\n      "
#endif

/* this should match long_options, below */
#define GETOPT_STRING "ab:ef:hi:m:no:prsvwx"

#ifdef HAVE_GETOPT_LONG
/* this should match GETOPT_STRING, above */
static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"base", 1, 0, 'b'},
  {"ignore-errors", 0, 0, 'e'},
  {"feature", 1, 0, 'f'},
  {"help", 0, 0, 'h'},
  {"input", 1, 0, 'i'},
  {"mode", 1, 0, 'm'},
  {"ntriples", 0, 0, 'n'},
  {"output", 1, 0, 'o'},
  {"drop-dup-preds", 0, 0, 'p'}, /* works only with rdfxml-abbrev serializer */
  {"replace-newlines", 0, 0, 'r'},
  {"scan", 0, 0, 's'},
  {"version", 0, 0, 'v'},
  {"ignore-warnings", 0, 0, 'w'},
  {"xmp", 0, 0, 'x'}, /* works only with rdfxml-abbrev serializer */
  {NULL, 0, 0, 0}
};
#endif


static int error_count=0;
static int warning_count=0;

static int ignore_warnings=0;
static int ignore_errors=0;

static const char *title_format_string="Raptor RDF cat utility %s\n";


static void
rdfdump_error_handler(void *data, raptor_locator *locator,
                      const char *message)
{
  if(!ignore_errors) {
    fprintf(stderr, "%s: Error - ", program);
    raptor_print_locator(stderr, locator);
    fprintf(stderr, " - %s\n", message);
    
    raptor_parse_abort((raptor_parser*)data);
  }

  error_count++;
}


static void
rdfdump_warning_handler(void *data, raptor_locator *locator,
                        const char *message) 
{
  if(!ignore_warnings) {
    fprintf(stderr, "%s: Warning - ", program);
    raptor_print_locator(stderr, locator);
    fprintf(stderr, " - %s\n", message);
  }
  
  warning_count++;
}

struct namespace_decl
{
  unsigned char *prefix;
  unsigned char *uri_string;
};


static void
rdfdump_free_namespace_decl(void* data) {
  struct namespace_decl* nsd=(struct namespace_decl*)data;
  if(nsd->prefix)
    raptor_free_memory(nsd->prefix);
  if(nsd->uri_string)
    raptor_free_memory(nsd->uri_string);
  raptor_free_memory(nsd);
}


int
main(int argc, char *argv[]) 
{
  raptor_parser* rdf_parser=NULL;
  int rc=0;
  int scanning=0;
  const char *syntax_name="rdfxml"; /* default input syntax */
  const char *serializer_syntax_name="rdfxml-abbrev"; /* default output syntax */
  int strict_mode=0;
  int usage=0;
  int help=0;
  char *p=NULL;
  char *filename=NULL;
  raptor_feature parser_feature=(raptor_feature)-1;
  int parser_feature_value= -1;
  unsigned char* parser_feature_string_value=NULL;
  raptor_feature serializer_feature=(raptor_feature)-1;
  int serializer_feature_value= -1;
  unsigned char* serializer_feature_string_value=NULL;
  raptor_sequence *namespace_declarations=NULL;
  unsigned char *base_uri_string=NULL;
  raptor_uri *base_uri=NULL;

  program=argv[0];
  if((p=strrchr(program, '/')))
    program=p+1;
  else if((p=strrchr(program, '\\')))
    program=p+1;
  argv[0]=program;

  raptor_init();

  while (!usage && !help)
  {
    int c;
#ifdef HAVE_GETOPT_LONG
    int option_index = 0;

    c = getopt_long (argc, argv, GETOPT_STRING, long_options, &option_index);
#else
    c = getopt (argc, argv, GETOPT_STRING);
#endif
    if (c == -1)
      break;

    switch (c) {
      case 0:
      case '?': /* getopt() - unknown option */
        usage=1;
        break;
        
      case 'a':
        break;

      case 'b':
	if(optarg) {
	  base_uri_string = optarg;

	  base_uri=raptor_new_uri(base_uri_string);
	  if(!base_uri) {
	    fprintf(stderr, "%s: Failed to create URI for %s\n",
		    program, base_uri_string);

	    base_uri_string=NULL;

	    return(1);
	  }
	}
	break;

      case 'f':
        if(optarg) {
          if(!strcmp(optarg, "help")) {
            int i;
            
            printf("%s: Valid parser features are:\n", program);
            for(i=0; i <= RAPTOR_FEATURE_LAST; i++) {
              const char *feature_name;
              const char *feature_label;
              if(!raptor_features_enumerate((raptor_feature)i, &feature_name, NULL, &feature_label)) {
                const char *feature_type=(raptor_feature_value_type((raptor_feature)i) == 0) ? "" : " (string)";
                printf("  %-20s  %s%s\n", feature_name, feature_label, 
                       feature_type);
              }
            }
            printf("\n%s: Valid serializer features are:\n", program);
            for(i=0; i <= RAPTOR_FEATURE_LAST; i++) {
              const char *feature_name;
              const char *feature_label;
              if(!raptor_serializer_features_enumerate((raptor_feature)i, &feature_name, NULL, &feature_label)) {
                const char *feature_type=(raptor_feature_value_type((raptor_feature)i) == 0) ? "" : " (string)";
                printf("  %-20s  %s%s\n", feature_name, feature_label, 
                       feature_type);
              }
            }
            fputs("Features are set with `" HELP_ARG(f, feature) " FEATURE=VALUE or `-f FEATURE'\nand take a decimal integer VALUE except where noted.\nIf the VALUE is omitted, then it is given the value of 1.\n", stderr);
            fputs("\nA feature of the form xmlns:PREFIX=\"URI\" can be used to declare output\nnamespace prefixes and names for serializing using an XML-style syntax\nEither or both of PREFIX or URI can be omitted such as -f xmlns=\"URI\"\nThis form can be repeated for multiple declarations.\n", stderr);
            exit(0);
          } else if(!strncmp(optarg, "xmlns", 5)) {
            struct namespace_decl *nd;
            nd=(struct namespace_decl *)raptor_alloc_memory(sizeof(struct namespace_decl));
            if(raptor_new_namespace_parts_from_string((unsigned char*)optarg,
                                                      &nd->prefix,
                                                      &nd->uri_string)) {
              fprintf(stderr, "%s: Bad xmlns syntax in '%s'\n", program, 
                      optarg);
              rdfdump_free_namespace_decl(nd);
              exit(0);
            }

            if(!namespace_declarations)
              namespace_declarations=raptor_new_sequence(rdfdump_free_namespace_decl, NULL);

            raptor_sequence_push(namespace_declarations, nd);
          } else {
            int i;
            size_t arg_len=strlen(optarg);
            
            for(i=0; i <= RAPTOR_FEATURE_LAST; i++) {
              const char *feature_name;
              size_t len;
              
              if(raptor_features_enumerate((raptor_feature)i, &feature_name, NULL, NULL))
                continue;
              len=strlen(feature_name);
              if(!strncmp(optarg, feature_name, len)) {
                parser_feature=(raptor_feature)i;
                if(raptor_feature_value_type(parser_feature) == 0) {
                  if(len < arg_len && optarg[len] == '=')
                    parser_feature_value=atoi(&optarg[len+1]);
                  else if(len == arg_len)
                    parser_feature_value=1;
                } else {
                  if(len < arg_len && optarg[len] == '=')
                    parser_feature_string_value=(unsigned char*)&optarg[len+1];
                  else if(len == arg_len)
                    parser_feature_string_value=(unsigned char*)"";
                }
                break;
              }
            }
            
            for(i=0; i <= RAPTOR_FEATURE_LAST; i++) {
              const char *feature_name;
              size_t len;
              
              if(raptor_serializer_features_enumerate((raptor_feature)i, &feature_name, NULL, NULL))
                continue;
              len=strlen(feature_name);
              if(!strncmp(optarg, feature_name, len)) {
                serializer_feature=(raptor_feature)i;
                if(raptor_feature_value_type(serializer_feature) == 0) {
                  if(len < arg_len && optarg[len] == '=')
                    serializer_feature_value=atoi(&optarg[len+1]);
                  else if(len == arg_len)
                    serializer_feature_value=1;
                } else {
                  if(len < arg_len && optarg[len] == '=')
                    serializer_feature_string_value=(unsigned char*)&optarg[len+1];
                  else if(len == arg_len)
                    serializer_feature_string_value=(unsigned char*)"";
                }
                break;
              }
            }
            
            if(parser_feature_value < 0 && !parser_feature_string_value &&
               serializer_feature_value < 0 && !serializer_feature_string_value) {
              fprintf(stderr, "%s: invalid argument `%s' for `" HELP_ARG(f, feature) "'\nTry '%s " HELP_ARG(f, feature) " help' for a list of valid features\n",
                      program, optarg, program);
              usage=1;
            }
          }
        }
        break;

      case 'h':
        help=1;
        break;

      case 'n':
        syntax_name="ntriples";
        break;

      case 's':
        scanning=1;
        break;

      case 'r':
        replace_newlines=1;
        break;

      case 'm':
        if(optarg) {
          if(!strcmp(optarg, "strict"))
            strict_mode=1;
          else if (!strcmp(optarg, "lax"))
            strict_mode=0;
          else {
            fprintf(stderr, "%s: invalid argument `%s' for `" HELP_ARG(m, mode) "'\n",
                    program, optarg);
            fprintf(stderr, "Valid arguments are:\n  - `lax'\n  - `strict'\n");
            usage=1;
          }
        }
        break;

      case 'o':
        if(optarg) {
          if(raptor_serializer_syntax_name_check(optarg))
            serializer_syntax_name=optarg;
          else {
            int i;
            
            fprintf(stderr, "%s: invalid argument `%s' for `" HELP_ARG(o, output) "'\n",
                    program, optarg);
            fprintf(stderr, "Valid arguments are:\n");
            for(i=0; 1; i++) {
              const char *help_name;
              const char *help_label;
              if(raptor_serializers_enumerate(i, &help_name, &help_label, NULL, NULL))
                break;
              printf("  %-12s for %s\n", help_name, help_label);
            }
            usage=1;
            break;
            
          }
        }
        break;

      case 'p':
        drop_dup_preds=1;
        break;
        
      case 'i':
        if(optarg) {
          if(raptor_syntax_name_check(optarg))
            syntax_name=optarg;
          else {
            int i;
            
            fprintf(stderr, "%s: invalid argument `%s' for `" HELP_ARG(i, input) "'\n",
                    program, optarg);
            fprintf(stderr, "Valid arguments are:\n");
            for(i=0; 1; i++) {
              const char *help_name;
              const char *help_label;
              if(raptor_syntaxes_enumerate(i, &help_name, &help_label, NULL, NULL))
                break;
              printf("  %-12s for %s\n", help_name, help_label);
            }
            usage=1;
            break;
            
          }
        }
        break;

      case 'w':
        ignore_warnings=1;
        break;
        
      case 'x':
        xmp=1;
        break;
        
      case 'e':
        ignore_errors=1;
        break;

      case 'v':
        fputs(raptor_version_string, stdout);
        fputc('\n', stdout);
        exit(0);
    }
    
  }

  
  if(usage) {
    if(usage>1) {
      fprintf(stderr, title_format_string, raptor_version_string);
      fputs(raptor_short_copyright_string, stderr);
      fputc('\n', stderr);
    }
    fprintf(stderr, "Try `%s " HELP_ARG(h, help) "' for more information.\n",
                    program);
    exit(1);
  }

  if(help) {
    int i;
    
    printf("Usage: %s [OPTIONS] <filename> [<filename> ...]\n", program);
    printf(title_format_string, raptor_version_string);
    puts(raptor_short_copyright_string);
    puts("Adapted from rapper by Sid Steward for Creative Commons\n");
    puts("Parse RDF content, combining multiple input files into one output." );
    puts("\nMain options:");
    puts(HELP_TEXT("h", "help            ", "Print this help, then exit"));
    puts(HELP_TEXT("i FORMAT", "input FORMAT ", "Set the input format to one of:"));
    for(i=0; 1; i++) {
      const char *help_name;
      const char *help_label;
      if(raptor_syntaxes_enumerate(i, &help_name, &help_label, NULL, NULL))
        break;
      printf("    %-14s          %s\n", help_name, help_label);
    }
    printf("      The default is: %s\n", syntax_name );
    puts(HELP_TEXT("o FORMAT", "output FORMAT", "Set the output format to one of:"));
    for(i=0; 1; i++) {
      const char *help_name;
      const char *help_label;
      if(raptor_serializers_enumerate(i, &help_name, &help_label, NULL, NULL))
        break;
      printf("    %-14s          %s\n", help_name, help_label);
    }
    printf("      The default is: %s\n", serializer_syntax_name );
    puts(HELP_TEXT("m MODE", "mode MODE  ", "Set parser mode - 'lax' (default) or 'strict'"));
    puts("\nAdditional options:");
    puts(HELP_TEXT("e", "ignore-errors   ", "Ignore error messages"));
    puts(HELP_TEXT("f FEATURE(=VALUE)", "feature FEATURE(=VALUE)", HELP_PAD "Set parser or serializer features" HELP_PAD "Use `-f help' for a list of valid features"));
    puts(HELP_TEXT("r", "replace-newlines", "Replace newlines with spaces in literals"));
    puts(HELP_TEXT("s", "scan            ", "Scan for <rdf:RDF> element in source"));
    puts(HELP_TEXT("w", "ignore-warnings ", "Ignore warning messages"));
    puts(HELP_TEXT("v", "version         ", "Print the Raptor version"));
    puts(HELP_TEXT("b URI", "base URI    ", "Set the base URI"));
    puts("\nOptions for rdfxml-abbrev output only:");
    puts(HELP_TEXT("p", "drop-dup-preds  ", "When two triplets or more have matching" HELP_PAD "subject and predicate, keep only the" HELP_PAD "triplet parsed first."));
    puts(HELP_TEXT("x", "xmp             ", "Format output for Adobe XMP. This sets" HELP_PAD "all subjects to the empty string."));
    puts("\nReport bugs to http://bugs.librdf.org/");
    puts("Raptor home page: http://librdf.org/raptor/");

    raptor_finish();

    exit(0);
  }

  /* check feature compatibility */
  if( (xmp || drop_dup_preds) && strcmp(serializer_syntax_name, "rdfxml-abbrev") ) {
    fputs("Error: the --xmp (-x) and --drop_dup_preds (-p) options work only\n", stderr );
    fputs("with the rdfxml-abbrev output serializer.", stderr );
    return(1);
  }

  if(xmp) { /* output xmp declaration */
    fputs( "<?xpacket begin='﻿' id='W5M0MpCehiHzreSzNTczkc9d'?>\n<x:xmpmeta xmlns:x='adobe:ns:meta/'>", stdout );
  }

  /* create parser */
  rdf_parser=raptor_new_parser(syntax_name);
  if(!rdf_parser) {
    fprintf(stderr, "%s: Failed to create raptor parser type %s\n", program,
	    syntax_name);
    return(1);
  }

  /* configure parser */
  raptor_set_error_handler(rdf_parser, rdf_parser, rdfdump_error_handler);
  raptor_set_warning_handler(rdf_parser, rdf_parser, rdfdump_warning_handler);
  
  raptor_set_parser_strict(rdf_parser, strict_mode);
  
  if(scanning)
    raptor_set_feature(rdf_parser, RAPTOR_FEATURE_SCANNING, 1);

  if(parser_feature_value >= 0)
    raptor_set_feature(rdf_parser, 
                       parser_feature, parser_feature_value);
  if(parser_feature_string_value)
    raptor_parser_set_feature_string(rdf_parser, 
                                     parser_feature,
                                     parser_feature_string_value);

  /* wire serializer to parser via callback */
  raptor_set_statement_handler(rdf_parser, NULL, print_statements);

  /* create serializer */
  if(serializer_syntax_name) {    
    serializer=raptor_new_serializer(serializer_syntax_name);
    if(!serializer) {
      fprintf(stderr, 
              "%s: Failed to create raptor serializer type %s\n", program,
              serializer_syntax_name);
      return(1);
    }

    if(namespace_declarations) {
      int i;
      for(i=0; i< raptor_sequence_size(namespace_declarations); i++) {
        struct namespace_decl *nd=(struct namespace_decl *)raptor_sequence_get_at(namespace_declarations, i);
        raptor_uri *ns_uri=NULL;
        if(nd->uri_string)
          ns_uri=raptor_new_uri(nd->uri_string);
        
        raptor_serialize_set_namespace(serializer, ns_uri, nd->prefix);
        if(ns_uri)
          raptor_free_uri(ns_uri);
      }
      raptor_free_sequence(namespace_declarations);
      namespace_declarations=NULL;
    }
    
    if(serializer_feature_value >= 0)
      raptor_serializer_set_feature(serializer, 
                                    serializer_feature, serializer_feature_value);
    if(serializer_feature_string_value)
      raptor_serializer_set_feature_string(serializer, 
                                           serializer_feature,
                                           serializer_feature_string_value);

    if(drop_dup_preds) { 
      raptor_serializer_set_feature(serializer, RAPTOR_FEATURE_OMIT_DUP_PREDICATES, 1);
    }

    if(xmp) { /* output xmp declaration, not xml declaration */
      raptor_serializer_set_feature(serializer, RAPTOR_FEATURE_OMIT_XML_DECLARATION, 1);
    }

    raptor_serialize_start_to_file_handle(serializer, base_uri, stdout);
  }
  
  /* iterate over the input filenames */
  while( optind< argc ) {
    unsigned char *uri_string = (unsigned char*)argv[optind++];
    int free_uri_string=0;

    raptor_uri *uri=NULL;

    /* If uri_string is "path-to-file", turn it into a file: URI */
    if(!strcmp((const char*)uri_string, "-")) {
      if(!base_uri_string) {
	fprintf(stderr, "%s: A Base URI is required when reading from standard input.\n",
		program);
	return(1);
      }
      uri_string=NULL;
    } else if(!access((const char*)uri_string, R_OK)) {
      filename=(char*)uri_string;
      uri_string=raptor_uri_filename_to_uri_string(filename);
      if(!uri_string) {
	fprintf(stderr, "%s: Failed to create URI for file %s.\n",
		program, filename);
	return(1);
      }
      free_uri_string=1;
    }

    if(uri_string) {
      uri=raptor_new_uri(uri_string);
      if(!uri) {
	fprintf(stderr, "%s: Failed to create URI for %s\n",
		program, uri_string);
	return(1);
      }
    } else
      uri=NULL; /* stdin */


    if(!base_uri_string) {
      base_uri=raptor_uri_copy(uri);
    }

    /* PARSE the URI as RDF/XML */
    rc=0;
    if(!uri || filename) {
      if(raptor_parse_file(rdf_parser, uri, base_uri)) {
	fprintf(stderr, "%s: Failed to parse file %s %s content\n", program, 
		filename, syntax_name);
	rc=1;
      }
    } else {
      if(raptor_parse_uri(rdf_parser, uri, base_uri)) {
	fprintf(stderr, "%s: Failed to parse URI %s %s content\n", program, 
		uri_string, syntax_name);
	rc=1;
      }
    }

    /* free some resources */
    if(base_uri && !base_uri_string) { /* base_uri created inside loop */
      raptor_free_uri(base_uri);
      base_uri=NULL;
    }
    if(uri) {
      raptor_free_uri(uri);
      uri=NULL;
    }
    if(free_uri_string) {
      raptor_free_memory(uri_string);
      free_uri_string=0;
    }
  }

  /* wrap up and clean up */

  raptor_free_parser(rdf_parser);

  if(serializer) {
    raptor_serialize_end(serializer);
    raptor_free_serializer(serializer);
  }

  if(xmp) {
    puts( "</x:xmpmeta>\n<?xpacket end='r'?>" );
  }

  if(namespace_declarations)
    raptor_free_sequence(namespace_declarations);

  if(base_uri) {
    raptor_free_uri(base_uri);
    base_uri=NULL;
  }

  raptor_finish();

  if(error_count && !ignore_errors)
    return 1;

  if(warning_count && !ignore_warnings)
    return 2;

  return(rc);
}
