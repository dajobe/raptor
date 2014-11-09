/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * turtle_parser.y - Raptor Turtle / TRIG / N3 parsers - over tokens from turtle grammar lexer
 *
 * Copyright (C) 2003-2013, David Beckett http://www.dajobe.org/
 * Copyright (C) 2003-2005, University of Bristol, UK http://www.bristol.ac.uk/
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
 * Turtle is defined in http://www.dajobe.org/2004/01/turtle/
 *
 * Made from a subset of the terms in
 *   http://www.w3.org/DesignIssues/Notation3.html
 *
 * TRIG is defined in http://www.wiwiss.fu-berlin.de/suhl/bizer/TriG/Spec/
 */

%{
#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "raptor2.h"
#include "raptor_internal.h"

#include <turtle_parser.h>

#define YY_NO_UNISTD_H 1
#include <turtle_lexer.h>

#include <turtle_common.h>


/* Set RAPTOR_DEBUG to 3 for super verbose parsing - watching the shift/reduces */
#if 0
#undef RAPTOR_DEBUG
#define RAPTOR_DEBUG 3
#endif


/* Make verbose error messages for syntax errors */
#define YYERROR_VERBOSE 1

/* Fail with an debug error message if RAPTOR_DEBUG > 1 */
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
#define YYERROR_MSG(msg) do { fputs("** YYERROR ", RAPTOR_DEBUG_FH); fputs(msg, RAPTOR_DEBUG_FH); fputc('\n', RAPTOR_DEBUG_FH); YYERROR; } while(0)
#else
#define YYERROR_MSG(ignore) YYERROR
#endif
#define YYERR_MSG_GOTO(label,msg) do { errmsg = msg; goto label; } while(0)

/* Slow down the grammar operation and watch it work */
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 2
#undef YYDEBUG
#define YYDEBUG 1
#endif

#ifdef RAPTOR_DEBUG
const char * turtle_token_print(raptor_world* world, int token, YYSTYPE *lval);
#endif


/* the lexer does not seem to track this */
#undef RAPTOR_TURTLE_USE_ERROR_COLUMNS

/* set api.push-pull to "push" if this is defined */
#undef TURTLE_PUSH_PARSE

/* Prototypes */ 
int turtle_parser_error(raptor_parser* rdf_parser, void* scanner, const char *msg);

/* Make lex/yacc interface as small as possible */
#undef yylex
#define yylex turtle_lexer_lex

/* Prototypes for local functions */
static void raptor_turtle_generate_statement(raptor_parser *parser, raptor_statement *triple);

%}


/* directives */

%require "3.0.0"

/* File prefix (bison -b) */
%file-prefix "turtle_parser"

/* Symbol prefix (bison -d : deprecated) */
%name-prefix "turtle_parser_"

/* Write parser header file with macros (bison -d) */
%defines

/* Write output file with verbose descriptions of parser states */
%verbose

/* Generate code processing locations */
 /* %locations */

/* Pure parser - want a reentrant parser  */
%define api.pure full

/* Push or pull parser? */
%define api.push-pull pull

/* Pure parser argument: lexer - yylex() and parser - yyparse() */
%lex-param { yyscan_t yyscanner }
%parse-param { raptor_parser* rdf_parser } { void* yyscanner }

/* Interface between lexer and parser */
%union {
  unsigned char *string;
  raptor_term *identifier;
  raptor_sequence *sequence;
  raptor_uri *uri;
}


/* others */

%token A "a"
%token HAT "^"
%token DOT "."
%token COMMA ","
%token SEMICOLON ";"
%token LEFT_SQUARE "["
%token RIGHT_SQUARE "]"
%token LEFT_ROUND "("
%token RIGHT_ROUND ")"
%token LEFT_CURLY "{"
%token RIGHT_CURLY "}"
%token TRUE_TOKEN "true"
%token FALSE_TOKEN "false"
%token PREFIX "@prefix"
%token BASE "@base"
%token SPARQL_PREFIX "PREFIX"
%token SPARQL_BASE "BASE"

/* literals */
%token <string> STRING_LITERAL "string literal"
%token <uri> URI_LITERAL "URI literal"
%token <uri> GRAPH_NAME_LEFT_CURLY "Graph URI literal {"
%token <string> BLANK_LITERAL "blank node"
%token <uri> QNAME_LITERAL "QName"
%token <string> IDENTIFIER "identifier"
%token <string> LANGTAG "langtag"
%token <string> INTEGER_LITERAL "integer literal"
%token <string> FLOATING_LITERAL "floating point literal"
%token <string> DECIMAL_LITERAL "decimal literal"

/* syntax error */
%token ERROR_TOKEN

%type <identifier> subject predicate object verb literal resource blankNode collection blankNodePropertyList
%type <sequence> objectList itemList predicateObjectList predicateObjectListOpt

/* tidy up tokens after errors */

%destructor {
  if($$)
    RAPTOR_FREE(char*, $$);
} STRING_LITERAL BLANK_LITERAL INTEGER_LITERAL FLOATING_LITERAL DECIMAL_LITERAL IDENTIFIER LANGTAG

%destructor {
  if($$)
    raptor_free_uri($$);
} URI_LITERAL QNAME_LITERAL

%destructor {
  if($$)
    raptor_free_term($$);
} subject predicate object verb literal resource blankNode collection

%destructor {
  if($$)
    raptor_free_sequence($$);
} objectList itemList predicateObjectList predicateObjectListOpt

%%

Document : statementList
;;


graph: GRAPH_NAME_LEFT_CURLY
  {
    /* action in mid-rule so this is run BEFORE the triples in graphBody */
    raptor_turtle_parser* turtle_parser;

    turtle_parser = (raptor_turtle_parser*)rdf_parser->context;
    if(!turtle_parser->trig)
      turtle_parser_error(rdf_parser, yyscanner, "{ ... } is not allowed in Turtle");
    else {
      if(turtle_parser->graph_name)
        raptor_free_term(turtle_parser->graph_name);
      turtle_parser->graph_name = raptor_new_term_from_uri(rdf_parser->world, $1);
      raptor_free_uri($1);
      raptor_parser_start_graph(rdf_parser,
                                turtle_parser->graph_name->value.uri, 1);
    }
  }
  graphBody RIGHT_CURLY
{
  raptor_turtle_parser* turtle_parser;

  turtle_parser = (raptor_turtle_parser*)rdf_parser->context;

  if(turtle_parser->trig) {
    raptor_parser_end_graph(rdf_parser,
                            turtle_parser->graph_name->value.uri, 1);
    raptor_free_term(turtle_parser->graph_name);
    turtle_parser->graph_name = NULL;
    rdf_parser->emitted_default_graph = 0;
  }
}
|
LEFT_CURLY
  {
    /* action in mid-rule so this is run BEFORE the triples in graphBody */
    raptor_turtle_parser* turtle_parser;

    turtle_parser = (raptor_turtle_parser*)rdf_parser->context;
    if(!turtle_parser->trig)
      turtle_parser_error(rdf_parser, yyscanner, "{ ... } is not allowed in Turtle");
    else {
      raptor_parser_start_graph(rdf_parser, NULL, 1);
      rdf_parser->emitted_default_graph++;
    }
  }
  graphBody RIGHT_CURLY
{
  raptor_turtle_parser* turtle_parser;

  turtle_parser = (raptor_turtle_parser*)rdf_parser->context;
  if(turtle_parser->trig) {
    raptor_parser_end_graph(rdf_parser, NULL, 1);
    rdf_parser->emitted_default_graph = 0;
  }
}
;


graphBody: triplesList
| %empty
;

triplesList: dotTriplesList
| dotTriplesList DOT
;

dotTriplesList: triples
| dotTriplesList DOT triples
;

statementList: statementList statement
| %empty
;

statement: directive
| graph
| triples DOT
;

triples: subject predicateObjectList
{
  int i;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("triples 1\n subject=");
  if($1)
    raptor_term_print_as_ntriples($1, stdout);
  else
    fputs("NULL", stdout);
  if($2) {
    printf("\n predicateObjectList (reverse order to syntax)=");
    raptor_sequence_print($2, stdout);
    printf("\n");
  } else     
    printf("\n and empty predicateObjectList\n");
#endif

  if($1 && $2) {
    /* have subject and non-empty property list, handle it  */
    for(i = 0; i < raptor_sequence_size($2); i++) {
      raptor_statement* t2 = (raptor_statement*)raptor_sequence_get_at($2, i);
      t2->subject = raptor_term_copy($1);
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" after substitution predicateObjectList=");
    raptor_sequence_print($2, stdout);
    printf("\n\n");
#endif
    for(i = 0; i < raptor_sequence_size($2); i++) {
      raptor_statement* t2 = (raptor_statement*)raptor_sequence_get_at($2, i);
      raptor_turtle_generate_statement(rdf_parser, t2);
    }
  }

  if($2)
    raptor_free_sequence($2);

  if($1)
    raptor_free_term($1);
}
| blankNodePropertyList predicateObjectListOpt
{
  int i;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("triples 2\n blankNodePropertyList=");
  if($1)
    raptor_term_print_as_ntriples($1, stdout);
  else
    fputs("NULL", stdout);
  if($2) {
    printf("\n predicateObjectListOpt (reverse order to syntax)=");
    raptor_sequence_print($2, stdout);
    printf("\n");
  } else     
    printf("\n and empty predicateObjectListOpt\n");
#endif

  if($1 && $2) {
    /* have subject and non-empty predicate object list, handle it  */
    for(i = 0; i < raptor_sequence_size($2); i++) {
      raptor_statement* t2 = (raptor_statement*)raptor_sequence_get_at($2, i);
      t2->subject = raptor_term_copy($1);
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" after substitution predicateObjectListOpt=");
    raptor_sequence_print($2, stdout);
    printf("\n\n");
#endif
    for(i = 0; i < raptor_sequence_size($2); i++) {
      raptor_statement* t2 = (raptor_statement*)raptor_sequence_get_at($2, i);
      raptor_turtle_generate_statement(rdf_parser, t2);
    }
  }

  if($2)
    raptor_free_sequence($2);

  if($1)
    raptor_free_term($1);
}
| error DOT
;


objectList: objectList COMMA object
{
  raptor_statement *triple;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("objectList 1\n");
  if($3) {
    printf(" object=\n");
    raptor_term_print_as_ntriples($3, stdout);
    printf("\n");
  } else  
    printf(" and empty object\n");
  if($1) {
    printf(" objectList=");
    raptor_sequence_print($1, stdout);
    printf("\n");
  } else
    printf(" and empty objectList\n");
#endif

  if(!$3)
    $$ = NULL;
  else {
    triple = raptor_new_statement_from_nodes(rdf_parser->world, NULL, NULL, $3, NULL);
    if(!triple) {
      raptor_free_sequence($1);
      YYERROR;
    }
    if(raptor_sequence_push($1, triple)) {
      raptor_free_sequence($1);
      YYERROR;
    }
    $$ = $1;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" objectList is now ");
    raptor_sequence_print($$, stdout);
    printf("\n\n");
#endif
  }
}
| object
{
  raptor_statement *triple;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("objectList 2\n");
  if($1) {
    printf(" object=\n");
    raptor_term_print_as_ntriples($1, stdout);
    printf("\n");
  } else  
    printf(" and empty object\n");
#endif

  if(!$1)
    $$ = NULL;
  else {
    triple = raptor_new_statement_from_nodes(rdf_parser->world, NULL, NULL, $1, NULL);
    if(!triple)
      YYERROR;
#ifdef RAPTOR_DEBUG
    $$ = raptor_new_sequence((raptor_data_free_handler)raptor_free_statement,
                             (raptor_data_print_handler)raptor_statement_print);
#else
    $$ = raptor_new_sequence((raptor_data_free_handler)raptor_free_statement, NULL);
#endif
    if(!$$) {
      raptor_free_statement(triple);
      YYERROR;
    }
    if(raptor_sequence_push($$, triple)) {
      raptor_free_sequence($$);
      $$ = NULL;
      YYERROR;
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" objectList is now ");
    raptor_sequence_print($$, stdout);
    printf("\n\n");
#endif
  }
}
;

itemList: itemList object
{
  raptor_statement *triple;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("objectList 1\n");
  if($2) {
    printf(" object=\n");
    raptor_term_print_as_ntriples($2, stdout);
    printf("\n");
  } else  
    printf(" and empty object\n");
  if($1) {
    printf(" objectList=");
    raptor_sequence_print($1, stdout);
    printf("\n");
  } else
    printf(" and empty objectList\n");
#endif

  if(!$2)
    $$ = NULL;
  else {
    triple = raptor_new_statement_from_nodes(rdf_parser->world, NULL, NULL, $2, NULL);
    if(!triple) {
      raptor_free_sequence($1);
      YYERROR;
    }
    if(raptor_sequence_push($1, triple)) {
      raptor_free_sequence($1);
      YYERROR;
    }
    $$ = $1;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" objectList is now ");
    raptor_sequence_print($$, stdout);
    printf("\n\n");
#endif
  }
}
| object
{
  raptor_statement *triple;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("objectList 2\n");
  if($1) {
    printf(" object=\n");
    raptor_term_print_as_ntriples($1, stdout);
    printf("\n");
  } else  
    printf(" and empty object\n");
#endif

  if(!$1)
    $$ = NULL;
  else {
    triple = raptor_new_statement_from_nodes(rdf_parser->world, NULL, NULL, $1, NULL);
    if(!triple)
      YYERROR;
#ifdef RAPTOR_DEBUG
    $$ = raptor_new_sequence((raptor_data_free_handler)raptor_free_statement,
                             (raptor_data_print_handler)raptor_statement_print);
#else
    $$ = raptor_new_sequence((raptor_data_free_handler)raptor_free_statement, NULL);
#endif
    if(!$$) {
      raptor_free_statement(triple);
      YYERROR;
    }
    if(raptor_sequence_push($$, triple)) {
      raptor_free_sequence($$);
      $$ = NULL;
      YYERROR;
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" objectList is now ");
    raptor_sequence_print($$, stdout);
    printf("\n\n");
#endif
  }
}
;

verb: predicate
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("verb predicate=");
  raptor_term_print_as_ntriples($1, stdout);
  printf("\n");
#endif

  $$ = $1;
}
| A
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("verb predicate = rdf:type (a)\n");
#endif

  $$ = raptor_term_copy(RAPTOR_RDF_type_term(rdf_parser->world));
  if(!$$)
    YYERROR;
}
;


predicateObjectList: predicateObjectList SEMICOLON verb objectList
{
  int i;
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("predicateObjectList 1\n verb=");
  raptor_term_print_as_ntriples($3, stdout);
  printf("\n objectList=");
  raptor_sequence_print($4, stdout);
  printf("\n predicateObjectList=");
  raptor_sequence_print($1, stdout);
  printf("\n\n");
#endif
  
  if($4 == NULL) {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" empty objectList not processed\n");
#endif
  } else if($3 && $4) {
    /* non-empty property list, handle it  */
    for(i = 0; i < raptor_sequence_size($4); i++) {
      raptor_statement* t2 = (raptor_statement*)raptor_sequence_get_at($4, i);
      t2->predicate = raptor_term_copy($3);
    }
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" after substitution objectList=");
    raptor_sequence_print($4, stdout);
    printf("\n");
#endif
  }

  if($1 == NULL) {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" empty predicateObjectList not copied\n\n");
#endif
  } else if($3 && $4 && $1) {
    while(raptor_sequence_size($4)) {
      raptor_statement* t2 = (raptor_statement*)raptor_sequence_unshift($4);
      if(raptor_sequence_push($1, t2)) {
        raptor_free_sequence($1);
        raptor_free_term($3);
        raptor_free_sequence($4);
        YYERROR;
      }
    }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" after appending objectList (reverse order)=");
    raptor_sequence_print($1, stdout);
    printf("\n\n");
#endif

    raptor_free_sequence($4);
  }

  if($3)
    raptor_free_term($3);

  $$ = $1;
}
| verb objectList
{
  int i;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("predicateObjectList 2\n verb=");
  raptor_term_print_as_ntriples($1, stdout);
  if($2) {
    printf("\n objectList=");
    raptor_sequence_print($2, stdout);
    printf("\n");
  } else
    printf("\n and empty objectList\n");
#endif

  if($1 && $2) {
    for(i = 0; i < raptor_sequence_size($2); i++) {
      raptor_statement* t2 = (raptor_statement*)raptor_sequence_get_at($2, i);
      t2->predicate = raptor_term_copy($1);
    }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" after substitution objectList=");
    raptor_sequence_print($2, stdout);
    printf("\n\n");
#endif
  }

  if($1)
    raptor_free_term($1);

  $$ = $2;
}
| predicateObjectList SEMICOLON
{
  $$ = $1;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("predicateObjectList 5\n trailing semicolon returning existing list ");
  raptor_sequence_print($$, stdout);
  printf("\n\n");
#endif
}
;

directive : prefix | base
;

prefix: PREFIX IDENTIFIER URI_LITERAL DOT
{
  unsigned char *prefix = $2;
  raptor_turtle_parser* turtle_parser = (raptor_turtle_parser*)(rdf_parser->context);
  raptor_namespace *ns;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("directive PREFIX %s %s\n",($2 ? (char*)$2 : "(default)"), raptor_uri_as_string($3));
#endif

  if(prefix) {
    size_t len = strlen((const char*)prefix);
    if(prefix[len-1] == ':') {
      if(len == 1)
         /* declaring default namespace prefix PREFIX : ... */
        prefix = NULL;
      else
        prefix[len-1]='\0';
    }
  }

  ns = raptor_new_namespace_from_uri(&turtle_parser->namespaces, prefix, $3, 0);
  if(ns) {
    raptor_namespaces_start_namespace(&turtle_parser->namespaces, ns);
    raptor_parser_start_namespace(rdf_parser, ns);
  }

  if($2)
    RAPTOR_FREE(char*, $2);
  raptor_free_uri($3);

  if(!ns)
    YYERROR;
}
| SPARQL_PREFIX IDENTIFIER URI_LITERAL
{
  unsigned char *prefix = $2;
  raptor_turtle_parser* turtle_parser = (raptor_turtle_parser*)(rdf_parser->context);
  raptor_namespace *ns;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("directive @prefix %s %s.\n",($2 ? (char*)$2 : "(default)"), raptor_uri_as_string($3));
#endif

  if(prefix) {
    size_t len = strlen((const char*)prefix);
    if(prefix[len-1] == ':') {
      if(len == 1)
         /* declaring default namespace prefix @prefix : ... */
        prefix = NULL;
      else
        prefix[len-1]='\0';
    }
  }

  ns = raptor_new_namespace_from_uri(&turtle_parser->namespaces, prefix, $3, 0);
  if(ns) {
    raptor_namespaces_start_namespace(&turtle_parser->namespaces, ns);
    raptor_parser_start_namespace(rdf_parser, ns);
  }

  if($2)
    RAPTOR_FREE(char*, $2);
  raptor_free_uri($3);

  if(!ns)
    YYERROR;
}
;


base: BASE URI_LITERAL DOT
{
  raptor_uri *uri=$2;

  if(rdf_parser->base_uri)
    raptor_free_uri(rdf_parser->base_uri);
  rdf_parser->base_uri = uri;
}
| SPARQL_BASE URI_LITERAL
{
  raptor_uri *uri=$2;

  if(rdf_parser->base_uri)
    raptor_free_uri(rdf_parser->base_uri);
  rdf_parser->base_uri = uri;
}
;

subject: resource
{
  $$ = $1;
}
| blankNode
{
  $$ = $1;
}
| collection
{
  $$ = $1;
}
;


predicate: resource
{
  $$ = $1;
}
;


object: resource
{
  $$ = $1;
}
| blankNode
{
  $$ = $1;
}
| collection
{
  $$ = $1;
}
| blankNodePropertyList
{
  $$ = $1;
}
| literal
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("object literal=");
  raptor_term_print_as_ntriples($1, stdout);
  printf("\n");
#endif

  $$ = $1;
}
;


literal: STRING_LITERAL LANGTAG
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("literal + language string=\"%s\"\n", $1);
#endif

  $$ = raptor_new_term_from_literal(rdf_parser->world, $1, NULL, $2);
  RAPTOR_FREE(char*, $1);
  RAPTOR_FREE(char*, $2);
  if(!$$)
    YYERROR;
}
| STRING_LITERAL LANGTAG HAT URI_LITERAL
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("literal + language=\"%s\" datatype string=\"%s\" uri=\"%s\"\n", $1, $2, raptor_uri_as_string($4));
#endif

  if($4) {
    if($2) {
      raptor_parser_error(rdf_parser,
                          "Language not allowed with datatyped literal");
      RAPTOR_FREE(char*, $2);
      $2 = NULL;
    }
  
    $$ = raptor_new_term_from_literal(rdf_parser->world, $1, $4, NULL);
    RAPTOR_FREE(char*, $1);
    raptor_free_uri($4);
    if(!$$)
      YYERROR;
  } else
    $$ = NULL;
    
}
| STRING_LITERAL LANGTAG HAT QNAME_LITERAL
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("literal + language=\"%s\" datatype string=\"%s\" qname URI=<%s>\n", $1, $2, raptor_uri_as_string($4));
#endif

  if($4) {
    if($2) {
      raptor_parser_error(rdf_parser,
                          "Language not allowed with datatyped literal");
      RAPTOR_FREE(char*, $2);
      $2 = NULL;
    }
  
    $$ = raptor_new_term_from_literal(rdf_parser->world, $1, $4, NULL);
    RAPTOR_FREE(char*, $1);
    raptor_free_uri($4);
    if(!$$)
      YYERROR;
  } else
    $$ = NULL;

}
| STRING_LITERAL HAT URI_LITERAL
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("literal + datatype string=\"%s\" uri=\"%s\"\n", $1, raptor_uri_as_string($3));
#endif

  if($3) {
    $$ = raptor_new_term_from_literal(rdf_parser->world, $1, $3, NULL);
    RAPTOR_FREE(char*, $1);
    raptor_free_uri($3);
    if(!$$)
      YYERROR;
  } else
    $$ = NULL;
    
}
| STRING_LITERAL HAT QNAME_LITERAL
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("literal + datatype string=\"%s\" qname URI=<%s>\n", $1, raptor_uri_as_string($3));
#endif

  if($3) {
    $$ = raptor_new_term_from_literal(rdf_parser->world, $1, $3, NULL);
    RAPTOR_FREE(char*, $1);
    raptor_free_uri($3);
    if(!$$)
      YYERROR;
  } else
    $$ = NULL;
}
| STRING_LITERAL
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("literal string=\"%s\"\n", $1);
#endif

  $$ = raptor_new_term_from_literal(rdf_parser->world, $1, NULL, NULL);
  RAPTOR_FREE(char*, $1);
  if(!$$)
    YYERROR;
}
| INTEGER_LITERAL
{
  raptor_uri *uri;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("resource integer=%s\n", $1);
#endif
  uri = raptor_uri_copy(rdf_parser->world->xsd_integer_uri);
  $$ = raptor_new_term_from_literal(rdf_parser->world, $1, uri, NULL);
  RAPTOR_FREE(char*, $1);
  raptor_free_uri(uri);
  if(!$$)
    YYERROR;
}
| FLOATING_LITERAL
{
  raptor_uri *uri;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("resource double=%s\n", $1);
#endif
  uri = raptor_uri_copy(rdf_parser->world->xsd_double_uri);
  $$ = raptor_new_term_from_literal(rdf_parser->world, $1, uri, NULL);
  RAPTOR_FREE(char*, $1);
  raptor_free_uri(uri);
  if(!$$)
    YYERROR;
}
| DECIMAL_LITERAL
{
  raptor_uri *uri;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("resource decimal=%s\n", $1);
#endif
  uri = raptor_uri_copy(rdf_parser->world->xsd_decimal_uri);
  if(!uri) {
    RAPTOR_FREE(char*, $1);
    YYERROR;
  }
  $$ = raptor_new_term_from_literal(rdf_parser->world, $1, uri, NULL);
  RAPTOR_FREE(char*, $1);
  raptor_free_uri(uri);
  if(!$$)
    YYERROR;
}
| TRUE_TOKEN
{
  raptor_uri *uri;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  fputs("resource boolean true\n", stderr);
#endif
  uri = raptor_uri_copy(rdf_parser->world->xsd_boolean_uri);
  $$ = raptor_new_term_from_literal(rdf_parser->world,
                                    (const unsigned char*)"true", uri, NULL);
  raptor_free_uri(uri);
  if(!$$)
    YYERROR;
}
| FALSE_TOKEN
{
  raptor_uri *uri;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  fputs("resource boolean false\n", stderr);
#endif
  uri = raptor_uri_copy(rdf_parser->world->xsd_boolean_uri);
  $$ = raptor_new_term_from_literal(rdf_parser->world,
                                    (const unsigned char*)"false", uri, NULL);
  raptor_free_uri(uri);
  if(!$$)
    YYERROR;
}
;


resource: URI_LITERAL
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("resource URI=<%s>\n", raptor_uri_as_string($1));
#endif

  if($1) {
    $$ = raptor_new_term_from_uri(rdf_parser->world, $1);
    raptor_free_uri($1);
    if(!$$)
      YYERROR;
  } else
    $$ = NULL;
}
| QNAME_LITERAL
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("resource qname URI=<%s>\n", raptor_uri_as_string($1));
#endif

  if($1) {
    $$ = raptor_new_term_from_uri(rdf_parser->world, $1);
    raptor_free_uri($1);
    if(!$$)
      YYERROR;
  } else
    $$ = NULL;
}
;


predicateObjectListOpt: predicateObjectList
{
  $$ = $1;
}
| %empty
{
  $$ = NULL;
}
;


blankNode: BLANK_LITERAL
{
  const unsigned char *id;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("subject blank=\"%s\"\n", $1);
#endif
  id = raptor_world_internal_generate_id(rdf_parser->world, $1);
  if(!id)
    YYERROR;

  $$ = raptor_new_term_from_blank(rdf_parser->world, id);
  RAPTOR_FREE(char*, id);

  if(!$$)
    YYERROR;
}
;

blankNodePropertyList: LEFT_SQUARE predicateObjectListOpt RIGHT_SQUARE
{
  int i;
  const unsigned char *id;

  id = raptor_world_generate_bnodeid(rdf_parser->world);
  if(!id) {
    if($2)
      raptor_free_sequence($2);
    YYERROR;
  }

  $$ = raptor_new_term_from_blank(rdf_parser->world, id);
  RAPTOR_FREE(char*, id);
  if(!$$) {
    if($2)
      raptor_free_sequence($2);
    YYERROR;
  }

  if($2 == NULL) {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf("resource\n predicateObjectList=");
    raptor_term_print_as_ntriples($$, stdout);
    printf("\n");
#endif
  } else {
    /* non-empty property list, handle it  */
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf("resource\n predicateObjectList=");
    raptor_sequence_print($2, stdout);
    printf("\n");
#endif

    for(i = 0; i < raptor_sequence_size($2); i++) {
      raptor_statement* t2 = (raptor_statement*)raptor_sequence_get_at($2, i);
      t2->subject = raptor_term_copy($$);
      raptor_turtle_generate_statement(rdf_parser, t2);
    }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    printf(" after substitution objectList=");
    raptor_sequence_print($2, stdout);
    printf("\n\n");
#endif

    raptor_free_sequence($2);

  }
  
}
;


collection: LEFT_ROUND itemList RIGHT_ROUND
{
  int i;
  raptor_world* world = rdf_parser->world;
  raptor_term* first_identifier = NULL;
  raptor_term* rest_identifier = NULL;
  raptor_term* object = NULL;
  raptor_term* blank = NULL;
  char const *errmsg = NULL;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("collection\n objectList=");
  raptor_sequence_print($2, stdout);
  printf("\n");
#endif

  first_identifier = raptor_new_term_from_uri(world, RAPTOR_RDF_first_URI(world));
  if(!first_identifier)
    YYERR_MSG_GOTO(err_collection, "Cannot create rdf:first term");
  rest_identifier = raptor_new_term_from_uri(world, RAPTOR_RDF_rest_URI(world));
  if(!rest_identifier)
    YYERR_MSG_GOTO(err_collection, "Cannot create rdf:rest term");
  
  /* non-empty property list, handle it  */
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("resource\n predicateObjectList=");
  raptor_sequence_print($2, stdout);
  printf("\n");
#endif

  object = raptor_new_term_from_uri(world, RAPTOR_RDF_nil_URI(world));
  if(!object)
    YYERR_MSG_GOTO(err_collection, "Cannot create rdf:nil term");

  for(i = raptor_sequence_size($2)-1; i>=0; i--) {
    raptor_term* temp;
    raptor_statement* t2 = (raptor_statement*)raptor_sequence_get_at($2, i);
    const unsigned char *blank_id;

    blank_id = raptor_world_generate_bnodeid(rdf_parser->world);
    if(!blank_id)
      YYERR_MSG_GOTO(err_collection, "Cannot create bnodeid");

    blank = raptor_new_term_from_blank(rdf_parser->world,
                                       blank_id);
    RAPTOR_FREE(char*, blank_id);
    if(!blank)
      YYERR_MSG_GOTO(err_collection, "Cannot create bnode");
    
    t2->subject = blank;
    t2->predicate = first_identifier;
    /* t2->object already set to the value we want */
    raptor_turtle_generate_statement((raptor_parser*)rdf_parser, t2);
    
    temp = t2->object;
    
    t2->subject = blank;
    t2->predicate = rest_identifier;
    t2->object = object;
    raptor_turtle_generate_statement((raptor_parser*)rdf_parser, t2);

    t2->subject = NULL;
    t2->predicate = NULL;
    t2->object = temp;

    raptor_free_term(object);
    object = blank;
    blank = NULL;
  }
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf(" after substitution objectList=");
  raptor_sequence_print($2, stdout);
  printf("\n\n");
#endif

  raptor_free_sequence($2);

  raptor_free_term(first_identifier);
  raptor_free_term(rest_identifier);

  $$=object;

  err_collection:
  if(errmsg) {
    if(blank)
      raptor_free_term(blank);

    if(object)
      raptor_free_term(object);

    if(rest_identifier)
      raptor_free_term(rest_identifier);

    if(first_identifier)
      raptor_free_term(first_identifier);

    raptor_free_sequence($2);

    YYERROR_MSG(errmsg);
  }
}
|  LEFT_ROUND RIGHT_ROUND 
{
  raptor_world* world = rdf_parser->world;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("collection\n empty\n");
#endif

  $$ = raptor_new_term_from_uri(world, RAPTOR_RDF_nil_URI(world));
  if(!$$)
    YYERROR;
}
;


%%


/* Support functions */

int
turtle_parser_error(raptor_parser* rdf_parser, void* scanner, const char *msg)
{
  raptor_turtle_parser* turtle_parser;

  turtle_parser = (raptor_turtle_parser*)rdf_parser->context;
  
  if(turtle_parser->error_count++)
    return 0;

  rdf_parser->locator.line = turtle_parser->lineno;
#ifdef RAPTOR_TURTLE_USE_ERROR_COLUMNS
  rdf_parser->locator.column = turtle_lexer_get_column(yyscanner);
#endif

  raptor_log_error(rdf_parser->world, RAPTOR_LOG_LEVEL_ERROR,
                   &rdf_parser->locator, msg);

  return 0;
}


int
turtle_syntax_error(raptor_parser *rdf_parser, const char *message, ...)
{
  raptor_turtle_parser* turtle_parser = (raptor_turtle_parser*)rdf_parser->context;
  va_list arguments;

  if(!turtle_parser)
    return 1;

  if(turtle_parser->error_count++)
    return 0;

  rdf_parser->locator.line = turtle_parser->lineno;
#ifdef RAPTOR_TURTLE_USE_ERROR_COLUMNS
  rdf_parser->locator.column = turtle_lexer_get_column(yyscanner);
#endif

  va_start(arguments, message);
  
  raptor_parser_log_error_varargs(((raptor_parser*)rdf_parser),
                                  RAPTOR_LOG_LEVEL_ERROR, message, arguments);

  va_end(arguments);

  return 0;
}


raptor_uri*
turtle_qname_to_uri(raptor_parser *rdf_parser, unsigned char *name, size_t name_len) 
{
  raptor_turtle_parser* turtle_parser = (raptor_turtle_parser*)rdf_parser->context;

  if(!turtle_parser)
    return NULL;

  rdf_parser->locator.line = turtle_parser->lineno;
#ifdef RAPTOR_TURTLE_USE_ERROR_COLUMNS
  rdf_parser->locator.column = turtle_lexer_get_column(yyscanner);
#endif

  name_len = raptor_turtle_expand_qname_escapes(name, name_len,
                                                (raptor_simple_message_handler)turtle_parser_error, rdf_parser);
  if(!name_len)
    return NULL;
  
  return raptor_qname_string_to_uri(&turtle_parser->namespaces, name, name_len);
}



#ifndef TURTLE_PUSH_PARSE
static int
turtle_parse(raptor_parser *rdf_parser, const char *string, size_t length)
{
  raptor_turtle_parser* turtle_parser = (raptor_turtle_parser*)rdf_parser->context;
  int rc;
  
  if(!string || !*string)
    return 0;
  
  if(turtle_lexer_lex_init(&turtle_parser->scanner))
    return 1;
  turtle_parser->scanner_set = 1;

#if defined(YYDEBUG) && YYDEBUG > 0
  turtle_lexer_set_debug(1 ,&turtle_parser->scanner);
  turtle_parser_debug = 1;
#endif

  turtle_lexer_set_extra(rdf_parser, turtle_parser->scanner);
  (void)turtle_lexer__scan_bytes((char *)string, (int)length, turtle_parser->scanner);

  rc = turtle_parser_parse(rdf_parser, turtle_parser->scanner);

  turtle_lexer_lex_destroy(turtle_parser->scanner);
  turtle_parser->scanner_set = 0;

  return rc;
}
#endif


#ifdef TURTLE_PUSH_PARSE
static int
turtle_push_parse(raptor_parser *rdf_parser, 
                  const char *string, size_t length)
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  raptor_world* world = rdf_parser->world;
#endif
  raptor_turtle_parser* turtle_parser;
  void *buffer;
  int status;
  yypstate *ps;

  turtle_parser = (raptor_turtle_parser*)rdf_parser->context;

  if(!string || !*string)
    return 0;
  
  if(turtle_lexer_lex_init(&turtle_parser->scanner))
    return 1;
  turtle_parser->scanner_set = 1;

#if defined(YYDEBUG) && YYDEBUG > 0
  turtle_lexer_set_debug(1 ,&turtle_parser->scanner);
  turtle_parser_debug = 1;
#endif

  turtle_lexer_set_extra(rdf_parser, turtle_parser->scanner);
  buffer = turtle_lexer__scan_bytes(string, length, turtle_parser->scanner);

  /* returns a parser instance or 0 on out of memory */
  ps = yypstate_new();
  if(!ps)
    return 1;

  do {
    YYSTYPE lval;
    int token;

    memset(&lval, 0, sizeof(YYSTYPE));
    
    token = turtle_lexer_lex(&lval, turtle_parser->scanner);

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    printf("token %s\n", turtle_token_print(world, token, &lval));
#endif

    status = yypush_parse(ps, token, &lval, rdf_parser, turtle_parser->scanner);

    /* turtle_token_free(world, token, &lval); */

    if(!token || token == EOF || token == ERROR_TOKEN)
      break;
  } while (status == YYPUSH_MORE);
  yypstate_delete(ps);

  turtle_lexer_lex_destroy(turtle_parser->scanner);
  turtle_parser->scanner_set = 0;

  return 0;
}
#endif


/**
 * raptor_turtle_parse_init - Initialise the Raptor Turtle parser
 *
 * Return value: non 0 on failure
 **/

static int
raptor_turtle_parse_init(raptor_parser* rdf_parser, const char *name) {
  raptor_turtle_parser* turtle_parser = (raptor_turtle_parser*)rdf_parser->context;

  if(raptor_namespaces_init(rdf_parser->world, &turtle_parser->namespaces, 0))
    return 1;

  turtle_parser->trig = !strcmp(name, "trig");

  return 0;
}


/* PUBLIC FUNCTIONS */


/*
 * raptor_turtle_parse_terminate - Free the Raptor Turtle parser
 * @rdf_parser: parser object
 * 
 **/
static void
raptor_turtle_parse_terminate(raptor_parser *rdf_parser) {
  raptor_turtle_parser *turtle_parser = (raptor_turtle_parser*)rdf_parser->context;

  raptor_namespaces_clear(&turtle_parser->namespaces);

  if(turtle_parser->scanner_set) {
    turtle_lexer_lex_destroy(turtle_parser->scanner);
    turtle_parser->scanner_set = 0;
  }

  if(turtle_parser->buffer)
    RAPTOR_FREE(cdata, turtle_parser->buffer);

  if(turtle_parser->graph_name) {
    raptor_free_term(turtle_parser->graph_name);
    turtle_parser->graph_name = NULL;
  }
}


static void
raptor_turtle_generate_statement(raptor_parser *parser, raptor_statement *t)
{
  raptor_turtle_parser *turtle_parser = (raptor_turtle_parser*)parser->context;
  raptor_statement *statement = &parser->statement;

  if(!t->subject || !t->predicate || !t->object)
    return;

  if(!parser->statement_handler)
    return;

  if(turtle_parser->trig && turtle_parser->graph_name)
    statement->graph = raptor_term_copy(turtle_parser->graph_name);

  if(!parser->emitted_default_graph && !turtle_parser->graph_name) {
    /* for non-TRIG - start default graph at first triple */
    raptor_parser_start_graph(parser, NULL, 0);
    parser->emitted_default_graph++;
  }
  
  /* Two choices for subject for Turtle */
  if(t->subject->type == RAPTOR_TERM_TYPE_BLANK) {
    statement->subject = raptor_new_term_from_blank(parser->world,
                                                    t->subject->value.blank.string);
  } else {
    /* RAPTOR_TERM_TYPE_URI */
    RAPTOR_ASSERT(t->subject->type != RAPTOR_TERM_TYPE_URI,
                  "subject type is not resource");
    statement->subject = raptor_new_term_from_uri(parser->world,
                                                  t->subject->value.uri);
  }

  /* Predicates are URIs but check for bad ordinals */
  if(!strncmp((const char*)raptor_uri_as_string(t->predicate->value.uri),
              "http://www.w3.org/1999/02/22-rdf-syntax-ns#_", 44)) {
    unsigned char* predicate_uri_string = raptor_uri_as_string(t->predicate->value.uri);
    int predicate_ordinal = raptor_check_ordinal(predicate_uri_string+44);
    if(predicate_ordinal <= 0)
      raptor_parser_error(parser, "Illegal ordinal value %d in property '%s'.", predicate_ordinal, predicate_uri_string);
  }
  
  statement->predicate = raptor_new_term_from_uri(parser->world,
                                                  t->predicate->value.uri);
  

  /* Three choices for object for Turtle */
  if(t->object->type == RAPTOR_TERM_TYPE_URI) {
    statement->object = raptor_new_term_from_uri(parser->world,
                                                 t->object->value.uri);
  } else if(t->object->type == RAPTOR_TERM_TYPE_BLANK) {
    statement->object = raptor_new_term_from_blank(parser->world,
                                                   t->object->value.blank.string);
  } else {
    /* RAPTOR_TERM_TYPE_LITERAL */
    RAPTOR_ASSERT(t->object->type != RAPTOR_TERM_TYPE_LITERAL,
                  "object type is not literal");
    statement->object = raptor_new_term_from_literal(parser->world,
                                                     t->object->value.literal.string,
                                                     t->object->value.literal.datatype,
                                                     t->object->value.literal.language);
  }

  /* Generate the statement */
  (*parser->statement_handler)(parser->user_data, statement);

  raptor_free_term(statement->subject); statement->subject = NULL;
  raptor_free_term(statement->predicate); statement->predicate = NULL;
  raptor_free_term(statement->object); statement->object = NULL;
  if(statement->graph) {
    raptor_free_term(statement->graph); statement->graph = NULL;
  }
}



static int
raptor_turtle_parse_chunk(raptor_parser* rdf_parser, 
                          const unsigned char *s, size_t len,
                          int is_end)
{
  char *ptr;
  raptor_turtle_parser *turtle_parser;
  int rc;

  turtle_parser = (raptor_turtle_parser*)rdf_parser->context;
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2("adding %d bytes to line buffer\n", (int)len);
#endif

  if(len) {
    turtle_parser->buffer = RAPTOR_REALLOC(char*, turtle_parser->buffer,
                                           turtle_parser->buffer_length + len + 1);
    if(!turtle_parser->buffer) {
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      return 1;
    }

    /* move pointer to end of cdata buffer */
    ptr = turtle_parser->buffer+turtle_parser->buffer_length;

    /* adjust stored length */
    turtle_parser->buffer_length += len;

    /* now write new stuff at end of cdata buffer */
    memcpy(ptr, s, len);
    ptr += len;
    *ptr = '\0';

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    RAPTOR_DEBUG3("buffer buffer now '%s' (%ld bytes)\n", 
                  turtle_parser->buffer, turtle_parser->buffer_length);
#endif
  }
  
  /* if not end, wait for rest of input */
  if(!is_end)
    return 0;

  /* Nothing to do */
  if(!turtle_parser->buffer_length)
    return 0;

#ifdef TURTLE_PUSH_PARSE
  rc = turtle_push_parse(rdf_parser, 
			 turtle_parser->buffer, turtle_parser->buffer_length);
#else
  rc = turtle_parse(rdf_parser, turtle_parser->buffer, turtle_parser->buffer_length);
#endif  

  if(rdf_parser->emitted_default_graph) {
    /* for non-TRIG - end default graph after last triple */
    raptor_parser_end_graph(rdf_parser, NULL, 0);
    rdf_parser->emitted_default_graph--;
  }
  return rc;
}


static int
raptor_turtle_parse_start(raptor_parser *rdf_parser) 
{
  raptor_locator *locator=&rdf_parser->locator;
  raptor_turtle_parser *turtle_parser = (raptor_turtle_parser*)rdf_parser->context;

  /* base URI required for Turtle */
  if(!rdf_parser->base_uri)
    return 1;

  locator->line = 1;
  locator->column= -1; /* No column info */
  locator->byte= -1; /* No bytes info */

  if(turtle_parser->buffer_length) {
    RAPTOR_FREE(cdata, turtle_parser->buffer);
    turtle_parser->buffer = NULL;
    turtle_parser->buffer_length = 0;
  }
  
  turtle_parser->lineno = 1;

  return 0;
}


static int
raptor_turtle_parse_recognise_syntax(raptor_parser_factory* factory, 
                                     const unsigned char *buffer, size_t len,
                                     const unsigned char *identifier, 
                                     const unsigned char *suffix, 
                                     const char *mime_type)
{
  int score= 0;
  
  if(suffix) {
    if(!strcmp((const char*)suffix, "ttl"))
      score = 8;
    if(!strcmp((const char*)suffix, "n3"))
      score = 3;
  }
  
  if(mime_type) {
    if(strstr((const char*)mime_type, "turtle"))
      score += 6;
    if(strstr((const char*)mime_type, "n3"))
      score += 3;
  }

  /* Do this as long as N3 is not supported since it shares the same syntax */
  if(buffer && len) {
#define  HAS_TURTLE_PREFIX (raptor_memstr((const char*)buffer, len, "@prefix ") != NULL)
/* The following could also be found with N-Triples but not with @prefix */
#define  HAS_TURTLE_RDF_URI (raptor_memstr((const char*)buffer, len, ": <http://www.w3.org/1999/02/22-rdf-syntax-ns#>") != NULL)

    if(HAS_TURTLE_PREFIX) {
      score = 6;
      if(HAS_TURTLE_RDF_URI)
        score += 2;
    }
  }
  
  return score;
}


static raptor_uri*
raptor_turtle_get_graph(raptor_parser* rdf_parser)
{
  raptor_turtle_parser *turtle_parser;

  turtle_parser = (raptor_turtle_parser*)rdf_parser->context;
  if(turtle_parser->graph_name)
    return raptor_uri_copy(turtle_parser->graph_name->value.uri);

  return NULL;
}


#ifdef RAPTOR_PARSER_TRIG
static int
raptor_trig_parse_recognise_syntax(raptor_parser_factory* factory, 
                                   const unsigned char *buffer, size_t len,
                                   const unsigned char *identifier, 
                                   const unsigned char *suffix, 
                                   const char *mime_type)
{
  int score= 0;
  
  if(suffix) {
    if(!strcmp((const char*)suffix, "trig"))
      score = 9;
#ifndef RAPTOR_PARSER_TURTLE
    if(!strcmp((const char*)suffix, "ttl"))
      score = 8;
    if(!strcmp((const char*)suffix, "n3"))
      score = 3;
#endif
  }
  
  if(mime_type) {
    if(strstr((const char*)mime_type, "trig"))
      score = 6;
#ifndef RAPTOR_PARSER_TURTLE
    if(strstr((const char*)mime_type, "turtle"))
      score += 6;
    if(strstr((const char*)mime_type, "n3"))
      score += 3;
#endif
  }

#ifndef RAPTOR_PARSER_TURTLE
  /* Do this as long as N3 is not supported since it shares the same syntax */
  if(buffer && len) {
#define  HAS_TRIG_PREFIX (raptor_memstr((const char*)buffer, len, "@prefix ") != NULL)
/* The following could also be found with N-Triples but not with @prefix */
#define  HAS_TRIG_RDF_URI (raptor_memstr((const char*)buffer, len, ": <http://www.w3.org/1999/02/22-rdf-syntax-ns#>") != NULL)

    if(HAS_TRIG_PREFIX) {
      score = 6;
      if(HAS_TRIG_RDF_URI)
        score += 2;
    }
  }
#endif
  
  return score;
}
#endif


#ifdef RAPTOR_PARSER_TURTLE
static const char* const turtle_names[4] = { "turtle", "ntriples-plus", "n3", NULL };

static const char* const turtle_uri_strings[3] = {
  "http://www.w3.org/ns/formats/Turtle",
  "http://www.dajobe.org/2004/01/turtle/",
  NULL
};
  
#define TURTLE_TYPES_COUNT 6
static const raptor_type_q turtle_types[TURTLE_TYPES_COUNT + 1] = {
  /* first one is the default */
  { "text/turtle", 11, 10},
  { "application/x-turtle", 20, 10},
  { "application/turtle", 18, 10},
  { "text/n3", 7, 3},
  { "text/rdf+n3", 11, 3},
  { "application/rdf+n3", 18, 3},
  { NULL, 0}
};

static int
raptor_turtle_parser_register_factory(raptor_parser_factory *factory) 
{
  int rc = 0;

  factory->desc.names = turtle_names;

  factory->desc.mime_types = turtle_types;

  factory->desc.label = "Turtle Terse RDF Triple Language";
  factory->desc.uri_strings = turtle_uri_strings;

  factory->desc.flags = RAPTOR_SYNTAX_NEED_BASE_URI;
  
  factory->context_length     = sizeof(raptor_turtle_parser);
  
  factory->init      = raptor_turtle_parse_init;
  factory->terminate = raptor_turtle_parse_terminate;
  factory->start     = raptor_turtle_parse_start;
  factory->chunk     = raptor_turtle_parse_chunk;
  factory->recognise_syntax = raptor_turtle_parse_recognise_syntax;
  factory->get_graph = raptor_turtle_get_graph;

  return rc;
}
#endif


#ifdef RAPTOR_PARSER_TRIG
static const char* const trig_names[2] = { "trig", NULL };
  
static const char* const trig_uri_strings[2] = {
  "http://www.wiwiss.fu-berlin.de/suhl/bizer/TriG/Spec/",
  NULL
};
  
#define TRIG_TYPES_COUNT 1
static const raptor_type_q trig_types[TRIG_TYPES_COUNT + 1] = {
  /* first one is the default */
  { "application/x-trig", 18, 10},
  { NULL, 0, 0}
};

static int
raptor_trig_parser_register_factory(raptor_parser_factory *factory) 
{
  int rc = 0;

  factory->desc.names = trig_names;

  factory->desc.mime_types = trig_types;

  factory->desc.label = "TriG - Turtle with Named Graphs";
  factory->desc.uri_strings = trig_uri_strings;

  factory->desc.flags = RAPTOR_SYNTAX_NEED_BASE_URI;
  
  factory->context_length     = sizeof(raptor_turtle_parser);
  
  factory->init      = raptor_turtle_parse_init;
  factory->terminate = raptor_turtle_parse_terminate;
  factory->start     = raptor_turtle_parse_start;
  factory->chunk     = raptor_turtle_parse_chunk;
  factory->recognise_syntax = raptor_trig_parse_recognise_syntax;
  factory->get_graph = raptor_turtle_get_graph;

  return rc;
}
#endif


#ifdef RAPTOR_PARSER_TURTLE
int
raptor_init_parser_turtle(raptor_world* world)
{
  return !raptor_world_register_parser_factory(world,
                                               &raptor_turtle_parser_register_factory);
}
#endif

#ifdef RAPTOR_PARSER_TRIG
int
raptor_init_parser_trig(raptor_world* world)
{
  return !raptor_world_register_parser_factory(world,
                                               &raptor_trig_parser_register_factory);
}
#endif


#ifdef STANDALONE
#include <stdio.h>
#include <locale.h>

#define TURTLE_FILE_BUF_SIZE 2048

static void
turtle_parser_print_statement(void *user,
                              raptor_statement *statement) 
{
  FILE* stream = (FILE*)user;
  raptor_statement_print(statement, stream);
  putc('\n', stream);
}
  


int
main(int argc, char *argv[]) 
{
  char string[TURTLE_FILE_BUF_SIZE];
  raptor_parser rdf_parser; /* static */
  raptor_turtle_parser turtle_parser; /* static */
  raptor_locator *locator = &rdf_parser.locator;
  FILE *fh;
  const char *filename;
  size_t nobj;
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 2
  turtle_parser_debug = 1;
#endif

  if(argc > 1) {
    filename = argv[1];
    fh = fopen(filename, "r");
    if(!fh) {
      fprintf(stderr, "%s: Cannot open file %s - %s\n", argv[0], filename,
              strerror(errno));
      exit(1);
    }
  } else {
    filename="<stdin>";
    fh = stdin;
  }

  memset(string, 0, TURTLE_FILE_BUF_SIZE);
  nobj = fread(string, TURTLE_FILE_BUF_SIZE, 1, fh);
  if(nobj < TURTLE_FILE_BUF_SIZE) {
    if(ferror(fh)) {
      fprintf(stderr, "%s: file '%s' read failed - %s\n",
              argv[0], filename, strerror(errno));
      fclose(fh);
      return(1);
    }
  }
  
  if(argc > 1)
    fclose(fh);

  memset(&rdf_parser, 0, sizeof(rdf_parser));
  memset(&turtle_parser, 0, sizeof(turtle_parser));

  locator->line= locator->column = -1;
  locator->file= filename;

  turtle_parser.lineno= 1;

  rdf_parser.world = raptor_new_world();
  rdf_parser.context = &turtle_parser;
  rdf_parser.base_uri = raptor_new_uri(rdf_parser.world,
                                       (const unsigned char*)"http://example.org/fake-base-uri/");

  raptor_parser_set_statement_handler(&rdf_parser, stdout,
                                      turtle_parser_print_statement);
  raptor_turtle_parse_init(&rdf_parser, "turtle");
  
  turtle_parser.error_count = 0;

#ifdef TURTLE_PUSH_PARSE
  turtle_push_parse(&rdf_parser, string, strlen(string));
#else
  turtle_parse(&rdf_parser, string, strlen(string));
#endif

  raptor_turtle_parse_terminate(&rdf_parser);
  
  raptor_free_uri(rdf_parser.base_uri);

  raptor_free_world(rdf_parser.world);
  
  return (0);
}
#endif
