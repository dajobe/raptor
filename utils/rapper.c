/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdfdump.c - Raptor RDF Parser example code 
 *
 * $Id$
 *
 * Copyright (C) 2000-2003 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology - http://www.ilrt.org/
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

#ifdef RAPTOR_IN_REDLAND
#include <librdf.h>
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
#define HELP_TEXT(short, long, description) "  -" #short ", --" long "  " description "\n"
#define HELP_ARG(short, long) "--" #long
#else
#define HELP_TEXT(short, long, description) "  -" #short "  " description "\n"
#define HELP_ARG(short, long) "-" #short
#endif


#define GETOPT_STRING "nsahrqo:wecm:i:v"

#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"ntriples", 0, 0, 'n'},
  {"scan", 0, 0, 's'},
  {"assume", 0, 0, 'a'},
  {"help", 0, 0, 'h'},
  {"replace-newlines", 0, 0, 'r'},
  {"quiet", 0, 0, 'q'},
  {"output", 1, 0, 'o'},
  {"ignore-warnings", 0, 0, 'w'},
  {"ignore-errors", 0, 0, 'e'},
  {"count", 0, 0, 'c'},
  {"mode", 1, 0, 'm'},
  {"input", 1, 0, 'i'},
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


#ifdef RAPTOR_DEBUG
void raptor_stats_print(raptor_parser *rdf_parser, FILE *stream);
#endif

int
main(int argc, char *argv[]) 
{
  raptor_parser* rdf_parser=NULL;
  char *uri_string=NULL;
  int free_uri_string=0;
  char *base_uri_string=NULL;
  int rc;
  int scanning=0;
  int assume=0;
  int rdfxml=1;
  int rss_tag_soup=0;
  int strict_mode=0;
  int usage=0;
  int help=0;
  const char *parser_name="rdfxml";
  raptor_uri *base_uri;
  raptor_uri *uri;
  char *p;
#ifdef RAPTOR_IN_REDLAND
  librdf_world *world;
#endif

  program=argv[0];
  if((p=strrchr(program, '/')))
    program=p+1;
  else if((p=strrchr(program, '\\')))
    program=p+1;
  argv[0]=program;

#ifdef RAPTOR_IN_REDLAND
  world=librdf_new_world();
  librdf_world_open(world);
#else
  raptor_init();
#endif
  
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
        assume=1;
        break;

      case 'c':
        count=1;
        break;

      case 'h':
        help=1;
        break;

      case 'n':
        rdfxml=0; rss_tag_soup=0;
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
            fprintf(stderr, "Valid arguments are:\n  - `simple'\n  - `ntriples'\n");
            usage=1;
          }
        }
        break;

      case 'i':
        if(optarg) {
          if(!strcmp(optarg, "rdfxml")) {
            rdfxml=1; rss_tag_soup=0;
          } else if (!strcmp(optarg, "ntriples")) {
            rdfxml=0; rss_tag_soup=0;
          } else if (!strcmp(optarg, "rss-tag-soup")) {
            rdfxml=0; rss_tag_soup=1;
          } else {
            fprintf(stderr, "%s: invalid argument `%s' for `" HELP_ARG(i, input) "'\n",
                    program, optarg);
            fprintf(stderr, "Valid arguments are:\n  `rdfxml'        for RDF/XML content \n  `ntriples'      for N-Triples\n  `rss-tag-soup'  for RSS Tag Soup\n");
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
    if(usage>1)
      fprintf(stderr, title_format_string, raptor_version_string);
    fprintf(stderr, "Try `%s " HELP_ARG(h, help) "' for more information.\n",
                    program);
    exit(1);
  }

  if(help) {
    printf("Usage: %s [OPTIONS] <source URI> [base URI]\n", program);
    printf(title_format_string, raptor_version_string);
    printf("Parse RDF content at the source URI into RDF triples.\n");
    printf("\nMain options:\n");
    printf(HELP_TEXT(h, "help            ", "Print this help, then exit"));
    printf(HELP_TEXT(i, "input FORMAT    ", "Set input format to one of:"));
    printf("    'rdfxml'                RDF/XML (default)\n    'ntriples'              N-Triples\n    'rss-tag-soup'          RSS tag soup\n");
    printf(HELP_TEXT(o, "output FORMAT   ", "Set output format to one of:"));
    printf("    'simple'                A simple format (default)\n    'ntriples'              N-Triples\n");
    printf(HELP_TEXT(m, "mode            ", "Set parser mode - 'lax' (default) or 'strict'"));
    printf("\nAdditional options:\n");
    printf(HELP_TEXT(a, "assume          ", "Assume document is rdf/xml (rdf:RDF optional)"));
    printf(HELP_TEXT(c, "count           ", "Count triples - no output"));
    printf(HELP_TEXT(e, "ignore-errors   ", "Ignore error messages"));
    printf(HELP_TEXT(q, "quiet           ", "No extra information messages"));
    printf(HELP_TEXT(r, "replace-newlines", "Replace newlines with spaces in literals"));
    printf(HELP_TEXT(s, "scan            ", "Scan for <rdf:RDF> element in source"));
    printf(HELP_TEXT(w, "ignore-warnings ", "Ignore warning messages"));
    printf(HELP_TEXT(v, "version         ", "Print the Raptor version"));
    printf("\nReport bugs to <redland-dev@lists.librdf.org>.\n");
    printf("Raptor home page: http://www.redland.opensource.ac.uk/raptor/\n");
    exit(0);
  }


  if(optind == argc-1)
    uri_string=argv[optind];
  else {
    uri_string=argv[optind++];
    base_uri_string=argv[optind];
  }

  /* If uri_string is "path-to-file", turn it into a file: URI */
  if(!access(uri_string, R_OK)) {
    uri_string=raptor_uri_filename_to_uri_string(uri_string);
    free_uri_string=1;
  }
  
  uri=raptor_new_uri(uri_string);
  if(!uri) {
    fprintf(stderr, "%s: Failed to create URI for %s\n",
            program, uri_string);
    return(1);
  }

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

  parser_name=rdfxml ? "rdfxml" : (rss_tag_soup ? "rss-tag-soup" : "ntriples");

  rdf_parser=raptor_new_parser(parser_name);
  if(!rdf_parser) {
    fprintf(stderr, "%s: Failed to create raptor parser type %s\n", program,
            parser_name);
    return(1);
  }
  
  raptor_set_error_handler(rdf_parser, rdf_parser, rdfdump_error_handler);
  raptor_set_warning_handler(rdf_parser, rdf_parser, rdfdump_warning_handler);
  
  if(scanning)
    raptor_set_feature(rdf_parser, RAPTOR_FEATURE_SCANNING, 1);
  if(assume)
    raptor_set_feature(rdf_parser, RAPTOR_FEATURE_ASSUME_IS_RDF, 1);
  
  
  if(!quiet) {
    if(base_uri_string)
      fprintf(stdout, "%s: Parsing URI %s with base URI %s\n", program,
              uri_string, base_uri_string);
    else
      fprintf(stdout, "%s: Parsing URI %s\n", program, uri_string);
  }
  
  raptor_set_statement_handler(rdf_parser, NULL, print_statements);


  /* PARSE the URI as RDF/XML */
  if(raptor_parse_uri(rdf_parser, uri, base_uri)) {
    fprintf(stderr, "%s: Failed to parse %s content into model\n", program, 
            parser_name);
    rc=1;
  } else
    rc=0;

#ifdef RAPTOR_DEBUG
  raptor_stats_print(rdf_parser, stderr);
#endif
  raptor_free_parser(rdf_parser);

  if(!quiet)
    fprintf(stdout, "%s: Parsing returned %d statements\n", program,
            statement_count);

  raptor_free_uri(base_uri);
  raptor_free_uri(uri);
  if(free_uri_string)
    free(uri_string);

#ifdef RAPTOR_IN_REDLAND
  librdf_free_world(world);
#else
  raptor_finish();
#endif

  if(error_count && !ignore_errors)
    return 1;

  if(warning_count && !ignore_warnings)
    return 2;

  return(rc);
}
