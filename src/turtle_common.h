/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * turtle_common.h - Turtle lexer/parser shared internals
 *
 * $Id$
 *
 * Copyright (C) 2003-2004 David Beckett - http://purl.org/net/dajobe/
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

#ifndef TURTLE_COMMON_H
#define TURTLE_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif


/* turtle_parser.y */
int turtle_syntax_error(raptor_parser *rdf_parser, const char *message, ...);
raptor_uri* turtle_qname_to_uri(raptor_parser *rdf_parser, unsigned char *name, size_t name_len);


/*
 * Turtle parser object
 */
struct raptor_turtle_parser_s {
  /* buffer */
  char *buffer;

  /* buffer length */
  int buffer_length;
  
  /* static statement for use in passing to user code */
  raptor_statement statement;

  raptor_namespace_stack namespaces;

  /* for lexer to store result in */
  YYSTYPE lval;

  /* STATIC lexer */
  yyscan_t scanner;

  int scanner_set;

  int lineno;
};


#ifdef __cplusplus
}
#endif

#endif
