/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdfdump.c - Raptor RDF Parser example code 
 *
 * $Id$
 *
 * Copyright (C) 2000-2001 David Beckett - http://purl.org/net/dajobe/
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

#include <stdio.h>
#include <string.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

/* Raptor includes */
#include "raptor.h"
#include "ntriples.h"


#ifdef NEED_OPTIND_DECLARATION
extern int optind;
extern char *optarg;
#endif

static void print_statements(void *user_data, const raptor_statement *statement);
int main(int argc, char *argv[]);


/* replace newlines in literal string output with spaces */
static int replace_newlines=0;

/* extra noise? */
static int quiet=0;

static int statement_count=0;

static enum { OUTPUT_FORMAT_SIMPLE, OUTPUT_FORMAT_NTRIPLES } output_format = OUTPUT_FORMAT_SIMPLE;


static
void print_statements(void *user_data, const raptor_statement *statement) 
{
  if(output_format == OUTPUT_FORMAT_SIMPLE)
    fputs("rdfdump: Statement: ", stdout);

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

  statement_count++;
}


#ifdef HAVE_GETOPT_LONG
#define HELP_TEXT(short, long, description) "  -" #short ", --" long "  " description "\n"
#else
#define HELP_TEXT(short, long, description) "  -" #short "  " description "\n"
#endif


#define GETOPT_STRING "nshrqo:w"

#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"ntriples", 0, 0, 'n'},
  {"scan", 0, 0, 's'},
  {"help", 0, 0, 'h'},
  {"replace-newlines", 0, 0, 'r'},
  {"quiet", 0, 0, 'q'},
  {"output", 1, 0, 'o'},
  {"ignore-warnings", 0, 0, 'w'},
  {NULL, 0, 0, 0}
};
#endif


static char *program=NULL;
static int error_count=0;
static int warning_count=0;

static int ignore_warnings=0;


static void
rdfdump_error_handler(void *data, raptor_locator *locator,
                      const char *message, va_list arguments) 
{
  fprintf(stderr, "%s: error - ", program);
  raptor_print_locator(stderr, locator);
  fputs(": ", stderr);
  vfprintf(stderr, message, arguments);
  fputc('\n', stderr);

  error_count++;
}


static void
rdfdump_warning_handler(void *data, raptor_locator *locator,
                        const char *message, va_list arguments) 
{
  if(!ignore_warnings) {
    fprintf(stderr, "%s: warning - ", program);
    raptor_print_locator(stderr, locator);
    fputs(": ", stderr);
    vfprintf(stderr, message, arguments);
    fputc('\n', stderr);
  }
  
  warning_count++;
}



int
main(int argc, char *argv[]) 
{
  raptor_parser* rdfxml_parser=NULL;
  raptor_ntriples_parser* rdfnt_parser=NULL;
  char *uri_string;
  char *base_uri_string;
  int rc;
  int scanning=0;
  int rdfxml=1;
  int usage=0;
#ifdef RAPTOR_IN_REDLAND
  librdf_world *world;
  librdf_uri *base_uri;
  librdf_uri *uri;
#else
  const char *base_uri;
  const char *uri;
#endif

  program=argv[0];

#ifdef RAPTOR_IN_REDLAND
  world=librdf_new_world();
  librdf_world_open(world);
#endif


  
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

      case 'o':
        if(optarg) {
          if(!strcmp(optarg, "simple"))
            output_format=OUTPUT_FORMAT_SIMPLE;
          else if (!strcmp(optarg, "ntriples"))
            output_format=OUTPUT_FORMAT_NTRIPLES;
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
    fprintf(stderr, HELP_TEXT(r, "replace-newlines", "Replace newlines with spaces in literals"));
    fprintf(stderr, HELP_TEXT(q, "quiet           ", "No extra information messages"));
    fprintf(stderr, HELP_TEXT(o, "output FORMAT   ", "Set output to 'simple'' or 'ntriples'"));
    fprintf(stderr, HELP_TEXT(w, "ignore-warnings ", "Ignore warning messages"));
    return(usage>1);
  }


  if(optind == argc-1)
    uri_string=base_uri_string=argv[optind];
  else {
    uri_string=argv[optind++];
    base_uri_string=argv[optind];
  }

  if(strncmp(uri_string, "file:", 5)) {
    fprintf(stderr, "%s: URI %s must be file:FILE at present\n", 
            program, uri_string);
    return(1);
  }
  

#ifdef RAPTOR_IN_REDLAND
  base_uri=librdf_new_uri(world, base_uri_string);
  if(!base_uri) {
    fprintf(stderr, "%s: Failed to create librdf_uri for %s\n",
            program, base_uri_string);
    return(1);
  }
  uri=librdf_new_uri(world, uri_string);
  if(!uri) {
    fprintf(stderr, "%s: Failed to create librdf_uri for %s\n",
            program, uri_string);
    return(1);
  }
#else
  uri=uri_string;
  base_uri=base_uri_string;
#endif

  if(rdfxml) {
#ifdef RAPTOR_IN_REDLAND
    rdfxml_parser=raptor_new(world);
#else
    rdfxml_parser=raptor_new();
#endif
    if(!rdfxml_parser) {
      fprintf(stderr, "%s: Failed to create raptor parser\n", program);
      return(1);
    }

    raptor_set_error_handler(rdfxml_parser, NULL, rdfdump_error_handler);
    raptor_set_warning_handler(rdfxml_parser, NULL, rdfdump_warning_handler);

    if(scanning)
      raptor_set_feature(rdfxml_parser, RAPTOR_FEATURE_SCANNING, 1);
  
  
  } else {
#ifdef RAPTOR_IN_REDLAND
    rdfnt_parser=raptor_ntriples_new(world);
#else
    rdfnt_parser=raptor_ntriples_new();
#endif
    if(!rdfnt_parser) {
      fprintf(stderr, "%s: Failed to create raptor parser\n", program);
      return(1);
    }

  }


  if(!quiet) {
    if(base_uri_string)
      fprintf(stdout, "%s: Parsing URI %s with base URI %s\n", program,
              uri_string, base_uri_string);
    else
      fprintf(stdout, "%s: Parsing URI %s\n", program, uri_string);
  }
  

  if(rdfxml)
    raptor_set_statement_handler(rdfxml_parser, NULL, print_statements);
  else
    raptor_ntriples_set_statement_handler(rdfnt_parser, NULL, print_statements);


  if(rdfxml) {
    /* PARSE the URI as RDF/XML */
    if(raptor_parse_file(rdfxml_parser, uri, base_uri)) {
      fprintf(stderr, "%s: Failed to parse RDF/XML into model\n", program);
      rc=1;
    } else
      rc=0;
    raptor_free(rdfxml_parser);
  } else {
    /* PARSE the URI as NTriples */
    if(raptor_ntriples_parse_file(rdfnt_parser, uri, base_uri)) {
      fprintf(stderr, "%s: Failed to parse RDF into model\n", program);
      rc=1;
    } else
      rc=0;
    raptor_ntriples_free(rdfnt_parser);
  }

  if(!quiet)
    fprintf(stdout, "%s: Parsing returned %d statements\n", program,
            statement_count);

#ifdef RAPTOR_IN_REDLAND
  librdf_free_uri(base_uri);

  librdf_free_world(world);
#endif

  if(error_count)
    return error_count;

  if(warning_count && !ignore_warnings)
    return 128+warning_count;

  return(rc);
}
