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
#include <config.h>
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

/* for the memory allocation functions */
#if defined(HAVE_DMALLOC_H) && defined(RAPTOR_MEMORY_DEBUG_DMALLOC)
#include <dmalloc.h>
#endif

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
#else
#define HELP_TEXT(short, long, description) "  -" #short "  " description "\n"
#endif


#define GETOPT_STRING "nsahrqo:wcm:"

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
  {"count", 0, 0, 'c'},
  {"mode", 1, 0, 'm'},
  {NULL, 0, 0, 0}
};
#endif


static int error_count=0;
static int warning_count=0;

static int ignore_warnings=0;


static void
rdfdump_error_handler(void *data, raptor_locator *locator,
                      const char *message)
{
  fprintf(stderr, "%s: Error - ", program);
  raptor_print_locator(stderr, locator);
  fprintf(stderr, " - %s\n", message);

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
  char *base_uri_string=NULL;
  int rc;
  int scanning=0;
  int assume=0;
  int rdfxml=1;
  int strict_mode=0;
  int usage=0;
  const char *parser_name="rdfxml";
  raptor_uri *base_uri;
  raptor_uri *uri;
#ifdef RAPTOR_IN_REDLAND
  librdf_world *world;
#endif

  program=argv[0];

#ifdef RAPTOR_IN_REDLAND
  world=librdf_new_world();
  librdf_world_open(world);
#endif

  raptor_init();
  
  while (!usage)
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
#ifdef HAVE_GETOPT_LONG
        fprintf(stderr, "Unknown option %s\n", long_options[option_index].name);
#else
        fprintf(stderr, "Unknown option %s\n", argv[optind]);
#endif
        usage=2; /* usage and error */
        break;
        
      case 'a':
        assume=1;
        break;

      case 'c':
        count=1;
        break;

      case 'h':
        usage=1;
        break;

      case 'n':
        rdfxml=0;
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
            fprintf(stderr, "%s: invalid argument `%s' for `--mode'", program,
                    optarg);
            fprintf(stderr, "Valid arguments are:\n  - `lax'\n  - `strict'\n");
            fprintf(stderr, "Try `%s --help' for more information.\n",
                    program);
            exit(1);
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
            fprintf(stderr, "%s: invalid argument `%s' for `--output'",
                    program, optarg);
            fprintf(stderr, "Valid arguments are:\n  - `simple'\n  - `ntriples'\n");
            fprintf(stderr, "Try `%s --help' for more information.\n",
                    program);
            exit(1);
          }
        }
        break;

      case 'w':
        ignore_warnings=1;
    }
    
  }

  if(optind != argc-1 && optind != argc-2)
    usage=2; /* usage and error */
  

  if(usage) {
    fprintf(stderr, "Usage: %s [OPTIONS] <source file: URI> [base URI]\n", program);
    fprintf(stderr, "Parse the given file as RDF/XML or NTriples using Raptor\n");
    fprintf(stderr, HELP_TEXT(h, "help            ", "This message"));
    fprintf(stderr, HELP_TEXT(n, "ntriples        ", "Parse NTriples format rather than RDF/XML"));
    fprintf(stderr, HELP_TEXT(s, "scan            ", "Scan for <rdf:RDF> element in source"));
    fprintf(stderr, HELP_TEXT(a, "assume          ", "Assume document is rdf/xml (rdf:RDF optional)"));
    fprintf(stderr, HELP_TEXT(r, "replace-newlines", "Replace newlines with spaces in literals"));
    fprintf(stderr, HELP_TEXT(q, "quiet           ", "No extra information messages"));
    fprintf(stderr, HELP_TEXT(o, "output FORMAT   ", "Set output to 'simple'' or 'ntriples'"));
    fprintf(stderr, HELP_TEXT(w, "ignore-warnings ", "Ignore warning messages"));
    fprintf(stderr, HELP_TEXT(c, "count           ", "Count triples - no output"));
    fprintf(stderr, HELP_TEXT(m, "mode            ", "Set parser mode - lax (default) or strict"));
    return(usage>1);
  }


  if(optind == argc-1)
    uri_string=argv[optind];
  else {
    uri_string=argv[optind++];
    base_uri_string=argv[optind];
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

  parser_name=rdfxml ? "rdfxml" : "ntriples";

  rdf_parser=raptor_new_parser(parser_name);
  if(!rdf_parser) {
    fprintf(stderr, "%s: Failed to create raptor parser type %s\n", program,
            parser_name);
    return(1);
  }
  
  raptor_set_error_handler(rdf_parser, NULL, rdfdump_error_handler);
  raptor_set_warning_handler(rdf_parser, NULL, rdfdump_warning_handler);
  
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

#ifdef RAPTOR_IN_REDLAND
  librdf_free_world(world);
#endif

  raptor_finish();

  if(error_count)
    return 1;

  if(warning_count && !ignore_warnings)
    return 2;

  return(rc);
}
