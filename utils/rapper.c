/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * rdfdump.c - Rapier RDF Parser example code 
 *
 * $Id$
 *
 * Copyright (C) 2000 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology, University of Bristol.
 *
 *    This package is Free Software available under either of two licenses
 *    (see FAQS.html to see why):
 * 
 * 1. The GNU Lesser General Public License (LGPL)
 * 
 *    See http://www.gnu.org/copyleft/lesser.html or COPYING.LIB for the
 *    full license text.
 *      _________________________________________________________________
 * 
 *      Copyright (C) 2000 David Beckett, Institute for Learning and
 *      Research Technology, University of Bristol. All Rights Reserved.
 * 
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public License
 *      as published by the Free Software Foundation; either version 2 of
 *      the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful, but
 *      WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library; if not, write to the Free Software
 *      Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 *      USA
 *      _________________________________________________________________
 * 
 *    NOTE - under Term 3 of the LGPL, you may choose to license the entire
 *    library under the GPL. See COPYING for the full license text.
 * 
 * 2. The Mozilla Public License
 * 
 *    See http://www.mozilla.org/MPL/MPL-1.1.html or MPL.html for the full
 *    license text.
 * 
 *    Under MPL section 13. I declare that all of the Covered Code is
 *    Multiple Licensed:
 *      _________________________________________________________________
 * 
 *      The contents of this file are subject to the Mozilla Public License
 *      version 1.1 (the "License"); you may not use this file except in
 *      compliance with the License. You may obtain a copy of the License
 *      at http://www.mozilla.org/MPL/
 * 
 *      Software distributed under the License is distributed on an "AS IS"
 *      basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 *      the License for the specific language governing rights and
 *      limitations under the License.
 * 
 *      The Initial Developer of the Original Code is David Beckett.
 *      Portions created by David Beckett are Copyright (C) 2000 David
 *      Beckett, Institute for Learning and Research Technology, University
 *      of Bristol. All Rights Reserved.
 * 
 *      Alternatively, the contents of this file may be used under the
 *      terms of the GNU Lesser General Public License, in which case the
 *      provisions of the LGPL License are applicable instead of those
 *      above. If you wish to allow use of your version of this file only
 *      under the terms of the LGPL License and not to allow others to use
 *      your version of this file under the MPL, indicate your decision by
 *      deleting the provisions above and replace them with the notice and
 *      other provisions required by the LGPL License. If you do not delete
 *      the provisions above, a recipient may use your version of this file
 *      under either the MPL or the LGPL License.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <getopt.h>

#ifdef LIBRDF_INTERNAL
#include <librdf.h>
#endif

#include <rapier.h>


static void print_statements(void *user_data, const rapier_statement *statement);
int main(int argc, char *argv[]);



static
void print_statements(void *user_data, const rapier_statement *statement) 
{
  fputs("rdfdump: Statement: ", stdout);
  rapier_print_statement(statement, stdout);
  fputc('\n', stdout);
}


#ifdef HAVE_GETOPT_LONG
#define HELP_TEXT(short, long, description) "  -" #short ", --" #long "  " description "\n"
#else
#define HELP_TEXT(short, long, description) "  -" #short "  " description "\n"
#endif


#define GETOPT_STRING "sh"

#ifdef HAVE_GETOPT_LONG
static struct option long_options[] =
{
  /* name, has_arg, flag, val */
  {"scan", 0, 0, 's'},
  {"help", 0, 0, 'h'},
  {NULL, 0, 0, 0}
};
#endif



int
main(int argc, char *argv[]) 
{
  rapier_parser* parser;
  char *uri;
  char *program=argv[0];
  int rc;
  int scanning=0;
  int usage=0;
#ifdef LIBRDF_INTERNAL
  librdf_uri *base_uri;
#else
  const char *base_uri;
#endif

#ifdef LIBRDF_INTERNAL
  librdf_init_world(NULL, NULL);
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

      case 's':
        scanning=1;
        break;
    }
    
  }
  
  if(optind != argc-1)
    usage=2; /* usage and error */
  

  if(usage) {
    fprintf(stderr, "Usage: %s [OPTIONS] <RDF source file: URI>\n", program);
    fprintf(stderr, "Parse the given file as RDF using Rapier\n");
    fprintf(stderr, HELP_TEXT(h, help, "This message"));
    fprintf(stderr, HELP_TEXT(s, scan, "Scan for <rdf:RDF> element in source"));
    return(usage>1);
  }

  uri=argv[optind];

#ifdef LIBRDF_INTERNAL
  base_uri=librdf_new_uri(uri);
  if(!base_uri) {
    fprintf(stderr, "%s: Failed to create librdf_uri for %s\n",
            program, uri);
    return(1);
  }
#else
  base_uri=uri;
#endif
  
  parser=rapier_new(base_uri);
  if(!parser) {
    fprintf(stderr, "%s: Failed to create rapier parser\n", program);
    return(1);
  }


  if(scanning)
    rapier_set_feature(parser, RAPIER_FEATURE_SCANNING, 1);
  

  /* PARSE the URI as RDF/XML*/
  fprintf(stdout, "%s: Parsing URI %s\n", program, uri);

  rapier_set_statement_handler(parser, NULL, print_statements);

  if(rapier_parse_file(parser, uri, uri)) {
    fprintf(stderr, "%s: Failed to parse RDF into model\n", program);
    rc=1;
  } else
    rc=0;
  rapier_free(parser);

#ifdef LIBRDF_INTERNAL
  librdf_free_uri(base_uri);

  librdf_destroy_world();
#endif

  return(rc);
}
