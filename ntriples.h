/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * ntriples.h - Raptor N-Triples Parser interfaces and definition
 *
 * $Id$
 *
 * Copyright (C) 2001 David Beckett - http://purl.org/net/dajobe/
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



#ifndef RAPTOR_NTRIPLES_H
#define RAPTOR_NTRIPLES_H


#ifdef __cplusplus
extern "C" {
#endif

/* Public structure */
typedef raptor_parser raptor_ntriples_parser;

typedef enum { RAPTOR_NTRIPLES_TERM_TYPE_URI_REF, RAPTOR_NTRIPLES_TERM_TYPE_BLANK_NODE, RAPTOR_NTRIPLES_TERM_TYPE_LITERAL } raptor_ntriples_term_type;


/* Public functions */

/* Create */
raptor_parser* raptor_ntriples_new(void);

/* Destroy */
void raptor_ntriples_free(raptor_parser *parser);

/* Handlers */
void raptor_ntriples_set_error_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
void raptor_ntriples_set_fatal_error_handler(raptor_parser* parser, void *user_data, raptor_message_handler handler);
void raptor_ntriples_set_statement_handler(raptor_parser* parser, void *user_data, raptor_statement_handler handler);

/* Parsing functions */
int raptor_ntriples_parse_file(raptor_parser* parser, raptor_uri *uri, raptor_uri *base_uri);

/* Utility functions */
const char * raptor_ntriples_term_as_string (raptor_ntriples_term_type term);

#ifdef __cplusplus
}
#endif

#endif
