/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * n3_common.h - N3 lexer/parser shared internals
 *
 * $Id$
 *
 * Copyright (C) 2003 David Beckett - http://purl.org/net/dajobe/
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

#ifndef N3_COMMON_H
#define N3_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif


/* n3_parser.y */
int n3_syntax_error(void *rdf_parser, const char *message, ...);
raptor_uri* n3_qname_to_uri(raptor_parser *rdf_parser, unsigned char *name, size_t name_len);


/*
 * N3 parser object
 */
struct raptor_n3_parser_s {
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
