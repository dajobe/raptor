/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdfdump.c - Raptor RDF Parser example code 
 *
 * $Id$
 *
 * Copyright (C) 2000-2004 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology - http://www.ilrt.bristol.ac.uk/
 * University of Bristol - http://www.bristol.ac.uk/
 * 
 * This package is Free Software or Open Source available under the
 * following licenses (these are alternatives):
 *   1. GNU Lesser General Public License (LGPL)
 *   2. GNU General Public License (GPL)
 *   3. Mozilla Public License (MPL)
 * 
 * See LICENSE.html or LICENSE.txt at the top of this package for the
 * full license terms.
 * 
 */


#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#ifdef WIN32
#include <win32_config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#else
#include <raptor_getopt.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <unistd.h>

/* Raptor includes */
#include <raptor.h>



#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif

static void print_statements(void *user_data, const raptor_statement *statement);
int main(int argc, char *argv[]);


static char *program=NULL;

/* replace newlines in literal string output with spaces */
static int replace_newlines=0;

/* extra noise? */
static int quiet=0;
/* just count, no printing */
static int count=0;

static int statement_count=0;

static enum { OUTPUT_FORMAT_SIMPLE, OUTPUT_FORMAT_NTRIPLES } output_format = OUTPUT_FORMAT_SIMPLE;


static
void print_statements(void *user_data, const raptor_statement *statement) 
{
  statement_count++;
  if(count)
    return;
  
  if(output_format == OUTPUT_FORMAT_SIMPLE)
    fprintf(stdout, "%s: Statement: ", program);

  /* replace newlines with spaces if object is a literal string */
  if(replace_newlines && 
     statement->object_type == RAPTOR_IDENTIFIER_TYPE_LITERAL) {
    char *s;
    for(s=(char*)statement->object; *s; s++)
      if(*s == '\n')
        *s=' ';
  }

  if(output_format == OUTPUT_FORMAT_SIMPLE)
    raptor_print_statement(statement, stdout);
  else
    raptor_print_statement_as_ntriples(statement, stdout);
  fputc('\n', stdout);
}


#ifdef HAVE_GETOPT_LONG
#define HELP_TEXT(short, long, description) "  -" short ", --" long "  " description
#define HELP_ARG(short, long) "--" #long
#else
#define HELP_TEXT(short, long, description) "  -" short "  " description
#define HELP_ARG(short, long) "-" #short
#endif


#define GETOPT_STRING "nsaf:hrqo:wecm:i:v"

#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"count", 0, 0, 'c'},
  {"ignore-errors", 0, 0, 'e'},
  {"feature", 0, 0, 'f'},
  {"help", 0, 0, 'h'},
  {"input", 1, 0, 'i'},
  {"mode", 1, 0, 'm'},
  {"ntriples", 0, 0, 'n'},
  {"output", 1, 0, 'o'},
  {"quiet", 0, 0, 'q'},
  {"replace-newlines", 0, 0, 'r'},
  {"scan", 0, 0, 's'},
  {"ignore-warnings", 0, 0, 'w'},
  {"version", 0, 0, 'v'},
  {NULL, 0, 0, 0}
};
#endif


static int error_count=0;
static int warning_count=0;

static int ignore_warnings=0;
static int ignore_errors=0;

static const char *title_format_string="Raptor RDF parser utility %s\n";


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


int
main(int argc, char *argv[]) 
{
  raptor_parser* rdf_parser=NULL;
  unsigned char *uri_string=NULL;
  int free_uri_string=0;
  unsigned char *base_uri_string=NULL;
  int rc;
  int scanning=0;
  const char *syntax_name="rdfxml";
  int strict_mode=0;
  int usage=0;
  int help=0;
  raptor_uri *base_uri;
  raptor_uri *uri;
  char *p;
  char *filename=NULL;
  raptor_feature feature;
  int feature_value= -1;

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

      case 'c':
        count=1;
        break;

      case 'f':
        if(optarg) {
          if(!strcmp(optarg, "help")) {
            int i;
            
            fprintf(stderr, "%s: Valid features for `" HELP_ARG(f, feature) "' are:\n", program);
            for(i=0; 1; i++) {
              const char *feature_name;
              const char *feature_label;
              if(raptor_features_enumerate(i, &feature_name, NULL, &feature_label))
                break;
              printf("  %-20s  %s\n", feature_name, feature_label);
            }
            fputs("\nFeature values are decimal integers 0 or larger, defaulting to 1 if omitted.\n", stderr);
            exit(0);
          } else {
            int i;
            size_t arg_len=strlen(optarg);
            
            for(i=0; 1; i++) {
              const char *feature_name;
              size_t len;
              
              if(raptor_features_enumerate(i, &feature_name, NULL, NULL))
                break;
              len=strlen(feature_name);
              if(!strncmp(optarg, feature_name, len)) {
                feature=i;
                if(len < arg_len && optarg[len] == '=')
                  feature_value=atoi(&optarg[len+1]);
                else if(len == arg_len)
                  feature_value=1;
                break;
              }
            }
            
            if(feature_value < 0 ) {
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

      case 'q':
        quiet=1;
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
          if(!strcmp(optarg, "simple"))
            output_format=OUTPUT_FORMAT_SIMPLE;
          else if (!strcmp(optarg, "ntriples"))
            output_format=OUTPUT_FORMAT_NTRIPLES;
          else {
            fprintf(stderr, "%s: invalid argument `%s' for `" HELP_ARG(o, output) "'\n",
                    program, optarg);
            fprintf(stderr, "Valid arguments are:\n  `simple'   for a simple format (default)\n  `ntriples' for N-Triples\n");
            usage=1;
          }
        }
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
        
      case 'e':
        ignore_errors=1;
        break;

      case 'v':
        fputs(raptor_version_string, stdout);
        fputc('\n', stdout);
        exit(0);
    }
    
  }

  if(optind != argc-1 && optind != argc-2 && !help && !usage) {
    usage=2; /* Title and usage */
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
    
    printf("Usage: %s [OPTIONS] <source URI> [base URI]\n", program);
    printf(title_format_string, raptor_version_string);
    puts(raptor_short_copyright_string);
    puts("Parse RDF content at the source URI into RDF triples.");
    puts("\nMain options:");
    puts(HELP_TEXT("h", "help            ", "Print this help, then exit"));
    puts(HELP_TEXT("i FORMAT", "input FORMAT ", "Set input format to one of:"));
    for(i=0; 1; i++) {
      const char *help_name;
      const char *help_label;
      if(raptor_syntaxes_enumerate(i, &help_name, &help_label, NULL, NULL))
        break;
      printf("    %-12s            %s", help_name, help_label);
      if(!i)
        puts(" (default)");
      else
        putchar('\n');
    }
    puts(HELP_TEXT("o FORMAT", "output FORMAT", "Set output format to one of:"));
    puts("    'simple'                A simple format (default)\n    'ntriples'              N-Triples");
    puts(HELP_TEXT("m MODE", "mode MODE  ", "Set parser mode - 'lax' (default) or 'strict'"));
    puts("\nAdditional options:");
    puts(HELP_TEXT("c", "count           ", "Count triples - no output"));
    puts(HELP_TEXT("e", "ignore-errors   ", "Ignore error messages"));
    puts(HELP_TEXT("f FEATURE(=VALUE)", "feature FEATURE(=VALUE)", "Set parser feature - use `help' for a list"));
    puts(HELP_TEXT("q", "quiet           ", "No extra information messages"));
    puts(HELP_TEXT("r", "replace-newlines", "Replace newlines with spaces in literals"));
    puts(HELP_TEXT("s", "scan            ", "Scan for <rdf:RDF> element in source"));
    puts(HELP_TEXT("w", "ignore-warnings ", "Ignore warning messages"));
    puts(HELP_TEXT("v", "version         ", "Print the Raptor version"));
    puts("\nReport bugs to <redland-dev@lists.librdf.org>.");
    puts("Raptor home page: http://www.redland.opensource.ac.uk/raptor/");
    exit(0);
  }


  if(optind == argc-1)
    uri_string=(unsigned char*)argv[optind];
  else {
    uri_string=(unsigned char*)argv[optind++];
    base_uri_string=(unsigned char*)argv[optind];
  }

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
  } else {
    base_uri=raptor_new_uri(base_uri_string);
    if(!base_uri) {
      fprintf(stderr, "%s: Failed to create URI for %s\n",
              program, base_uri_string);
      return(1);
    }
  }

  rdf_parser=raptor_new_parser(syntax_name);
  if(!rdf_parser) {
    fprintf(stderr, "%s: Failed to create raptor parser type %s\n", program,
            syntax_name);
    return(1);
  }
  
  raptor_set_error_handler(rdf_parser, rdf_parser, rdfdump_error_handler);
  raptor_set_warning_handler(rdf_parser, rdf_parser, rdfdump_warning_handler);
  
  raptor_set_parser_strict(rdf_parser, strict_mode);
  
  if(scanning)
    raptor_set_feature(rdf_parser, RAPTOR_FEATURE_SCANNING, 1);

  if(feature_value >= 0)
    raptor_set_feature(rdf_parser, feature, feature_value);

  if(!quiet) {
    if (filename) {
      if(base_uri_string)
        fprintf(stdout, "%s: Parsing file %s with base URI %s\n", program,
                filename, base_uri_string);
      else
        fprintf(stdout, "%s: Parsing file %s\n", program, filename);
    } else {
      if(base_uri_string)
        fprintf(stdout, "%s: Parsing URI %s with base URI %s\n", program,
                uri_string, base_uri_string);
      else
        fprintf(stdout, "%s: Parsing URI %s\n", program, uri_string);
    }
  }
  
  raptor_set_statement_handler(rdf_parser, NULL, print_statements);


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

  raptor_free_parser(rdf_parser);

  if(!quiet)
    fprintf(stdout, "%s: Parsing returned %d statements\n", program,
            statement_count);

  raptor_free_uri(base_uri);
  if(uri)
    raptor_free_uri(uri);
  if(free_uri_string)
    free(uri_string);

  raptor_finish();

  if(error_count && !ignore_errors)
    return 1;

  if(warning_count && !ignore_warnings)
    return 2;

  return(rc);
}
