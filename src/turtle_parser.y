/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * n3_parser.y - Raptor N3 parser - over tokens from n3 grammar lexer
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
 * 
 * Made from a subset of the terms in
 *   http://www.w3.org/DesignIssues/Notation3.html
 *
 */

%{
#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif

#ifdef WIN32
#include <win32_config.h>
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

#include <raptor.h>
#include <raptor_internal.h>

#include <n3_parser.tab.h>

#define YY_DECL int n3_lexer_lex (YYSTYPE *n3_parser_lval, yyscan_t yyscanner)
#include <n3_lexer.h>

#include <n3_common.h>


/* Make verbose error messages for syntax errors */
#ifdef RAPTOR_DEBUG
#define YYERROR_VERBOSE 1
#endif

/* Slow down the grammar operation and watch it work */
#if RAPTOR_DEBUG > 2
#define YYDEBUG 1
#endif

/* the lexer does not seem to track this */
#undef RAPTOR_N3_USE_ERROR_COLUMNS

/* Prototypes */ 
int n3_parser_error(raptor_parser* rdf_parser, const char *msg);

/* Missing n3_lexer.c/h prototypes */
int n3_lexer_get_column(yyscan_t yyscanner);
/* Not used here */
/* void n3_lexer_set_column(int  column_no , yyscan_t yyscanner);*/


/* What the lexer wants */
extern int n3_lexer_lex (YYSTYPE *n3_parser_lval, yyscan_t scanner);
#define YYLEX_PARAM ((raptor_n3_parser*)(((raptor_parser*)rdf_parser)->context))->scanner

/* Pure parser argument (a void*) */
#define YYPARSE_PARAM rdf_parser

/* Make the yyerror below use the rdf_parser */
#undef yyerror
#define yyerror(message) n3_parser_error(rdf_parser, message)

/* Make lex/yacc interface as small as possible */
#undef yylex
#define yylex n3_lexer_lex


static raptor_triple* raptor_new_triple(raptor_identifier *subject, raptor_identifier *predicate, raptor_identifier *object);
static void raptor_free_triple(raptor_triple *triple);

#ifdef RAPTOR_DEBUG
static void raptor_triple_print(raptor_triple *data, FILE *fh);
#endif


/* Prototypes for local functions */
static void raptor_n3_generate_statement(raptor_parser *parser, raptor_triple *triple);

%}


/* directives */



%pure-parser


/* Interface between lexer and parser */
%union {
  unsigned char *string;
  raptor_identifier *identifier;
  raptor_sequence *sequence;
  raptor_uri *uri;
}




/* word symbols */
%token PREFIX

/* others */

%token WS A AT HAT
%token DOT COMMA SEMICOLON
%token LEFT_SQUARE RIGHT_SQUARE

/* literals */
%token <string> STRING_LITERAL
%token <uri> URI_LITERAL
%token <string> BLANK_LITERAL
%token <uri> QNAME_LITERAL
%token <string> PREFIX
%token <string> IDENTIFIER

/* syntax error */
%token ERROR

%type <identifier> subject predicate object verb literal resource
%type <sequence> objectList propertyList

%%

Document : statementList
;

statementList: statement statementList 
| /* empty line */
;

statement: directive
| subject propertyList DOT
{
  int i;

#if RAPTOR_DEBUG > 1  
  printf("statement 2\n subject=");
  raptor_identifier_print(stdout, $1);
  if($2) {
    printf("\n propertyList=");
    raptor_sequence_print($2, stdout);
    printf("\n");
  } else     
    printf("\n and empty propertyList\n");
#endif

  if($2) {
    /* non-empty property list, handle it  */
    for(i=0; i<raptor_sequence_size($2); i++) {
      raptor_triple* t2=(raptor_triple*)raptor_sequence_get_at($2, i);
      raptor_identifier *i2=RAPTOR_CALLOC(raptor_identifier, 1, sizeof(raptor_identifier));
      raptor_copy_identifier(i2, $1);
      t2->subject=i2;
      t2->subject->is_malloced=1;
    }
#if RAPTOR_DEBUG > 1  
    printf(" after substitution propertyList=");
    raptor_sequence_print($2, stdout);
    printf("\n\n");
#endif
    for(i=0; i<raptor_sequence_size($2); i++) {
      raptor_triple* t2=(raptor_triple*)raptor_sequence_get_at($2, i);
      raptor_n3_generate_statement(rdf_parser, t2);
    }

    raptor_free_sequence($2);
  }

  if($1)
    raptor_free_identifier($1);
}
| error DOT
;


objectList: object COMMA objectList
{
  raptor_triple *triple;

#if RAPTOR_DEBUG > 1  
  printf("objectList 1\n");
  if($1) {
    printf(" object=\n");
    raptor_identifier_print(stdout, $1);
    printf("\n");
  } else  
    printf(" and empty object\n");
  if($3) {
    printf(" objectList=");
    raptor_sequence_print($3, stdout);
    printf("\n");
  } else
    printf(" and empty objectList\n");
#endif

  if(!$1)
    $$=NULL;
  else {
    triple=raptor_new_triple(NULL, NULL, $1);
    $$=$3;
    raptor_sequence_shift($$, triple);
#if RAPTOR_DEBUG > 1  
    printf(" objectList is now ");
    raptor_sequence_print($$, stdout);
    printf("\n\n");
#endif
  }
}
| object
{
  raptor_triple *triple;

#if RAPTOR_DEBUG > 1  
  printf("objectList 2\n");
  if($1) {
    printf(" object=\n");
    raptor_identifier_print(stdout, $1);
    printf("\n");
  } else  
    printf(" and empty object\n");
#endif

  if(!$1)
    $$=NULL;
  else {
    triple=raptor_new_triple(NULL, NULL, $1);
#ifdef RAPTOR_DEBUG
    $$=raptor_new_sequence((raptor_free_handler*)raptor_free_triple,
                           (raptor_print_handler*)raptor_triple_print);
#else
    $$=raptor_new_sequence((raptor_free_handler*)raptor_free_triple, NULL);
#endif
    raptor_sequence_push($$, triple);
#if RAPTOR_DEBUG > 1  
    printf(" objectList is now ");
    raptor_sequence_print($$, stdout);
    printf("\n\n");
#endif
  }
}
;

verb: predicate
{
#if RAPTOR_DEBUG > 1  
  printf("verb predicate=");
  raptor_identifier_print(stdout, $1);
  printf("\n");
#endif

  $$=$1;
}
| A
{
  raptor_uri *uri;

#if RAPTOR_DEBUG > 1  
  printf("verb predicate=rdf:type (a)\n");
#endif

  uri=raptor_new_uri_for_rdf_concept("type");
  $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_RESOURCE, uri, RAPTOR_URI_SOURCE_URI, NULL, NULL, NULL, NULL);
}
;


propertyList: verb objectList SEMICOLON propertyList
{
  int i;
  
#if RAPTOR_DEBUG > 1  
  printf("propertyList 1\n verb=");
  raptor_identifier_print(stdout, $1);
  printf("\n objectList=");
  raptor_sequence_print($2, stdout);
  printf("\n propertyList=");
  raptor_sequence_print($4, stdout);
  printf("\n\n");
#endif
  
  if($2 == NULL) {
#if RAPTOR_DEBUG > 1  
    printf(" empty objectList not processed\n");
#endif
  } else if($1 && $2) {
    /* non-empty property list, handle it  */
    for(i=0; i<raptor_sequence_size($2); i++) {
      raptor_triple* t2=(raptor_triple*)raptor_sequence_get_at($2, i);
      raptor_identifier *i2=RAPTOR_CALLOC(raptor_identifier, 1, sizeof(raptor_identifier));
      raptor_copy_identifier(i2, $1);
      t2->predicate=i2;
      t2->predicate->is_malloced=1;
    }
  
#if RAPTOR_DEBUG > 1  
    printf(" after substitution objectList=");
    raptor_sequence_print($2, stdout);
    printf("\n");
#endif
  }

  if($4 == NULL) {
#if RAPTOR_DEBUG > 1  
    printf(" empty propertyList not copied\n\n");
#endif
  } else if ($1 && $2 && $4) {
    while(raptor_sequence_size($4) > 0) {
      raptor_triple* t2=(raptor_triple*)raptor_sequence_unshift($4);
      raptor_sequence_push($2, t2);
    }

#if RAPTOR_DEBUG > 1  
    printf(" after appending objectList=");
    raptor_sequence_print($2, stdout);
    printf("\n\n");
#endif

    raptor_free_sequence($4);
  }

  if($1)
    raptor_free_identifier($1);

  $$=$2;
}
| verb objectList
{
  int i;
#if RAPTOR_DEBUG > 1  
  printf("propertyList 2\n verb=");
  raptor_identifier_print(stdout, $1);
  if($2) {
    printf("\n objectList=");
    raptor_sequence_print($2, stdout);
    printf("\n");
  } else
    printf("\n and empty objectList\n");
#endif

  if($1 && $2) {
    for(i=0; i<raptor_sequence_size($2); i++) {
      raptor_triple* t2=(raptor_triple*)raptor_sequence_get_at($2, i);
      raptor_identifier *i2=RAPTOR_CALLOC(raptor_identifier, 1, sizeof(raptor_identifier));
      raptor_copy_identifier(i2, $1);
      t2->predicate=i2;
      t2->predicate->is_malloced=1;
    }

#if RAPTOR_DEBUG > 1  
    printf(" after substitution objectList=");
    raptor_sequence_print($2, stdout);
    printf("\n\n");
#endif
  }

  if($1)
    raptor_free_identifier($1);

  $$=$2;
}
| /* empty */
{
#if RAPTOR_DEBUG > 1  
  printf("propertyList 3\n empty returning NULL\n\n");
#endif
  $$=NULL;
}
;

directive : PREFIX IDENTIFIER URI_LITERAL DOT
{
  char *prefix=$2;
  raptor_n3_parser* n3_parser=((raptor_parser*)rdf_parser)->context;

#if RAPTOR_DEBUG > 1  
  printf("directive @prefix %s %s\n",($2 ? (char*)$2 : "(default)"),raptor_uri_as_string($3));
#endif

  if(prefix) {
    int len=strlen(prefix);
    if(prefix[len-1] == ':') {
      if(len == 1)
         /* declaring default namespace prefix @prefix : ... */
        prefix=NULL;
      else
        prefix[len-1]='\0';
    }
  }

  raptor_namespaces_start_namespace_full(&n3_parser->namespaces,
                                         prefix, raptor_uri_as_string($3), 0);
  if($2)
    free($2);
  raptor_free_uri($3);
}
;

subject: resource
{
  $$=$1;
}
| BLANK_LITERAL
{
#if RAPTOR_DEBUG > 1  
  printf("subject blank=\"%s\"\n", $1);
#endif
  $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_ANONYMOUS, NULL, RAPTOR_URI_SOURCE_BLANK_ID, $1, NULL, NULL, NULL);
}
;

predicate: URI_LITERAL
{
#if RAPTOR_DEBUG > 1  
  printf("predicate URI=<%s>\n", raptor_uri_as_string($1));
#endif

  if($1)
    $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_RESOURCE, $1, RAPTOR_URI_SOURCE_URI, NULL, NULL, NULL, NULL);
  else
    $$=NULL;

}
| QNAME_LITERAL
{
#if RAPTOR_DEBUG > 1  
  printf("predicate qname URI=<%s>\n", raptor_uri_as_string($1));
#endif

  if($1)
    $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_RESOURCE, $1, RAPTOR_URI_SOURCE_ELEMENT, NULL, NULL, NULL, NULL);
  else
    $$=NULL;
}
;

object: resource
{
  $$=$1;
}
| literal
{
#if RAPTOR_DEBUG > 1  
  printf("object literal=");
  raptor_identifier_print(stdout, $1);
  printf("\n");
#endif

  $$=$1;
}
| BLANK_LITERAL
{
#if RAPTOR_DEBUG > 1  
  printf("object blank=\"%s\"\n", $1);
#endif

  $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_ANONYMOUS, NULL, RAPTOR_URI_SOURCE_BLANK_ID, $1, NULL, NULL, NULL);
}
;

literal: STRING_LITERAL AT IDENTIFIER
{
#if RAPTOR_DEBUG > 1  
  printf("literal + language string=\"%s\"\n", $1);
#endif

  $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_LITERAL, NULL, RAPTOR_URI_SOURCE_ELEMENT, NULL, $1, NULL, $3);
}
| STRING_LITERAL AT IDENTIFIER HAT URI_LITERAL
{
#if RAPTOR_DEBUG > 1  
  printf("literal + language=\"%s\" datatype string=\"%s\" uri=\"%s\"\n", $1, $3, raptor_uri_as_string($5));
#endif

  if($5)
    $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_LITERAL, NULL, RAPTOR_URI_SOURCE_ELEMENT, NULL, $1, $5, $3);
  else
    $$=NULL;
    
}
| STRING_LITERAL AT IDENTIFIER HAT QNAME_LITERAL
{
#if RAPTOR_DEBUG > 1  
  printf("literal + language=\"%s\" datatype string=\"%s\" qname URI=<%s>\n", $1, $3, raptor_uri_as_string($5));
#endif

  if($5)
    $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_LITERAL, NULL, RAPTOR_URI_SOURCE_ELEMENT, NULL, $1, $5, $3);
  else
    $$=NULL;

}
| STRING_LITERAL HAT URI_LITERAL
{
#if RAPTOR_DEBUG > 1  
  printf("literal + datatype string=\"%s\" uri=\"%s\"\n", $1, raptor_uri_as_string($3));
#endif

  if($3)
    $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_LITERAL, NULL, RAPTOR_URI_SOURCE_ELEMENT, NULL, $1, $3, NULL);
  else
    $$=NULL;
    
}
| STRING_LITERAL HAT QNAME_LITERAL
{
#if RAPTOR_DEBUG > 1  
  printf("literal + datatype string=\"%s\" qname URI=<%s>\n", $1, raptor_uri_as_string($3));
#endif

  if($3) {
    $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_LITERAL, NULL, RAPTOR_URI_SOURCE_ELEMENT, NULL, $1, $3, NULL);
  } else
    $$=NULL;
}
| STRING_LITERAL
{
#if RAPTOR_DEBUG > 1  
  printf("literal string=\"%s\"\n", $1);
#endif

  $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_LITERAL, NULL, RAPTOR_URI_SOURCE_ELEMENT, NULL, $1, NULL, NULL);
}
;

resource:
URI_LITERAL
{
#if RAPTOR_DEBUG > 1  
  printf("resource URI=<%s>\n", raptor_uri_as_string($1));
#endif

  if($1)
    $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_RESOURCE, $1, RAPTOR_URI_SOURCE_URI, NULL, NULL, NULL, NULL);
  else
    $$=NULL;
}
| QNAME_LITERAL
{
#if RAPTOR_DEBUG > 1  
  printf("resource qname URI=<%s>\n", raptor_uri_as_string($1));
#endif

  if($1)
    $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_RESOURCE, $1, RAPTOR_URI_SOURCE_ELEMENT, NULL, NULL, NULL, NULL);
  else
    $$=NULL;
}
| LEFT_SQUARE propertyList RIGHT_SQUARE
{
  int i;
  const unsigned char *id=raptor_generate_id(rdf_parser, 0, NULL);
  
  $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_ANONYMOUS, NULL, RAPTOR_URI_SOURCE_GENERATED, id, NULL, NULL, NULL);

  if($2 == NULL) {
#if RAPTOR_DEBUG > 1  
    printf("resource\n propertyList=");
    raptor_identifier_print(stdout, $$);
    printf("\n");
#endif
  } else {
    /* non-empty property list, handle it  */
#if RAPTOR_DEBUG > 1  
    printf("resource\n propertyList=");
    raptor_sequence_print($2, stdout);
    printf("\n");
#endif

    for(i=0; i<raptor_sequence_size($2); i++) {
      raptor_triple* t2=(raptor_triple*)raptor_sequence_get_at($2, i);
      raptor_identifier *i2=RAPTOR_CALLOC(raptor_identifier, 1, sizeof(raptor_identifier));
      raptor_copy_identifier(i2, $$);
      t2->subject=i2;
      t2->subject->is_malloced=1;
      raptor_n3_generate_statement(rdf_parser, t2);
    }

#if RAPTOR_DEBUG > 1  
    printf(" after substitution objectList=");
    raptor_sequence_print($2, stdout);
    printf("\n\n");
#endif

    raptor_free_sequence($2);

  }
  
}
;


%%


/* Support functions */

/* This is declared in n3_lexer.h but never used, so we always get
 * a warning unless this dummy code is here.  Used once below as a return.
 */
static int yy_init_globals (yyscan_t yyscanner ) { return 0; };


/* helper - everything passed in is now owned by triple */
static raptor_triple*
raptor_new_triple(raptor_identifier *subject,
                  raptor_identifier *predicate,
                  raptor_identifier *object) 
{
  raptor_triple* t;
  
  t=RAPTOR_MALLOC(raptor_triple, sizeof(raptor_triple));
  if(!t)
    return NULL;
  
  t->subject=subject;
  t->predicate=predicate;
  t->object=object;

  return t;
}

static void
raptor_free_triple(raptor_triple *t) {
  if(t->subject)
    raptor_free_identifier(t->subject);
  if(t->predicate)
    raptor_free_identifier(t->predicate);
  if(t->object)
    raptor_free_identifier(t->object);
  RAPTOR_FREE(raptor_triple, t);
}
 
#ifdef RAPTOR_DEBUG
static void
raptor_triple_print(raptor_triple *t, FILE *fh) 
{
  fputs("triple(", fh);
  raptor_identifier_print(fh, t->subject);
  fputs(", ", fh);
  raptor_identifier_print(fh, t->predicate);
  fputs(", ", fh);
  raptor_identifier_print(fh, t->object);
  fputc(')', fh);
}
#endif


extern char *filename;
 
int
n3_parser_error(raptor_parser* rdf_parser, const char *msg)
{
  raptor_n3_parser* n3_parser=(raptor_n3_parser*)rdf_parser->context;
  
  rdf_parser->locator.line=n3_parser->lineno;
#ifdef RAPTOR_N3_USE_ERROR_COLUMNS
  rdf_parser->locator.column=n3_lexer_get_column(yyscanner);
#endif

  raptor_parser_simple_error(rdf_parser, msg);
  return yy_init_globals(NULL); /* 0 but a way to use yy_init_globals */
}


int
n3_syntax_error(raptor_parser *rdf_parser, const char *message, ...)
{
  raptor_n3_parser* n3_parser=(raptor_n3_parser*)rdf_parser->context;
  va_list arguments;

  rdf_parser->locator.line=n3_parser->lineno;
#ifdef RAPTOR_N3_USE_ERROR_COLUMNS
  rdf_parser->locator.column=n3_lexer_get_column(yyscanner);
#endif

  va_start(arguments, message);
  
  raptor_parser_error_varargs(((raptor_parser*)rdf_parser), message, arguments);

  va_end(arguments);

  return (0);
}


raptor_uri*
n3_qname_to_uri(raptor_parser *rdf_parser, unsigned char *name, size_t name_len) 
{
  raptor_n3_parser* n3_parser=(raptor_n3_parser*)rdf_parser->context;

  rdf_parser->locator.line=n3_parser->lineno;
#ifdef RAPTOR_N3_USE_ERROR_COLUMNS
  rdf_parser->locator.column=n3_lexer_get_column(yyscanner);
#endif

  return raptor_qname_string_to_uri(&n3_parser->namespaces,
                                    name, name_len,
                                    raptor_parser_simple_error, rdf_parser);
}



static int
n3_parse(raptor_parser *rdf_parser, const char *string) {
  raptor_n3_parser* n3_parser=(raptor_n3_parser*)rdf_parser->context;
  void *buffer;
  
  if(!string || !*string)
    return 0;
  
  n3_lexer_lex_init(&n3_parser->scanner);
  n3_parser->scanner_set=1;

  n3_lexer_set_extra(((raptor_parser*)rdf_parser), n3_parser->scanner);
  buffer= n3_lexer__scan_string(string, n3_parser->scanner);

  n3_parser_parse(rdf_parser);

  n3_parser->scanner_set=0;

  return 0;
}


/**
 * raptor_n3_parse_init - Initialise the Raptor N3 parser
 *
 * Return value: non 0 on failure
 **/

static int
raptor_n3_parse_init(raptor_parser* rdf_parser, const char *name) {
  raptor_n3_parser* n3_parser=(raptor_n3_parser*)rdf_parser->context;
  raptor_uri_handler *uri_handler;
  void *uri_context;

  raptor_uri_get_handler(&uri_handler, &uri_context);

  raptor_namespaces_init(&n3_parser->namespaces,
                         uri_handler, uri_context,
                         raptor_parser_simple_error, rdf_parser, 
                         0);

  return 0;
}


/* PUBLIC FUNCTIONS */


/*
 * raptor_n3_parse_terminate - Free the Raptor N3 parser
 * @rdf_parser: parser object
 * 
 **/
static void
raptor_n3_parse_terminate(raptor_parser *rdf_parser) {
  raptor_n3_parser *n3_parser=(raptor_n3_parser*)rdf_parser->context;

  raptor_namespaces_clear(&n3_parser->namespaces);

  if(n3_parser->scanner_set) {
    n3_lexer_lex_destroy(&n3_parser->scanner);
    n3_parser->scanner_set=0;
  }

  if(n3_parser->buffer_length)
    RAPTOR_FREE(cdata, n3_parser->buffer);
}


static void
raptor_n3_generate_statement(raptor_parser *parser, raptor_triple *t)
{
  raptor_statement *statement=&parser->statement;
  int predicate_ordinal=0;

  if(!t->subject || !t->predicate || !t->object)
    return;

  /* Two choices for subject from N-Triples */
  statement->subject_type=t->subject->type;
  if(t->subject->type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
    statement->subject=t->subject->id;
  } else { /* URI */
    statement->subject=t->subject->uri;
  }

  /* Predicates are URIs, in Raptor some are turned into ordinals */
  if(!strncmp(raptor_uri_as_string(t->predicate->uri),
              "http://www.w3.org/1999/02/22-rdf-syntax-ns#_", 44)) {
    predicate_ordinal=raptor_check_ordinal(raptor_uri_as_string(t->predicate->uri)+44);
    if(predicate_ordinal > 0) {
      statement->predicate=(void*)&predicate_ordinal;
      statement->predicate_type=RAPTOR_IDENTIFIER_TYPE_ORDINAL;
    } else {
      predicate_ordinal=0;
    }
  }
  
  if(!predicate_ordinal) {
    statement->predicate_type=RAPTOR_IDENTIFIER_TYPE_PREDICATE;
    statement->predicate=t->predicate->uri;
  }
  

  /* Three choices for object from N-Triples */
  statement->object_type=t->object->type;
  if(t->object->type == RAPTOR_IDENTIFIER_TYPE_RESOURCE) {
    statement->object=t->object->uri;
  } else if(t->object->type == RAPTOR_IDENTIFIER_TYPE_ANONYMOUS) {
    statement->object=t->object->id;
  } else {  /* literal */
    statement->object=t->object->literal;
    statement->object_literal_language=t->object->literal_language;
    statement->object_literal_datatype=t->object->literal_datatype;
  }

  if(!parser->statement_handler)
    return;

  /* Generate the statement */
  (*parser->statement_handler)(parser->user_data, statement);
}



static int
raptor_n3_parse_chunk(raptor_parser* rdf_parser, 
                      const unsigned char *s, size_t len,
                      int is_end)
{
  char *buffer;
  char *ptr;
  raptor_n3_parser *n3_parser=(raptor_n3_parser*)rdf_parser->context;
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2(raptor_n3_parse_chunk, "adding %d bytes to line buffer\n", len);
#endif

  if(len) {
    buffer=(char*)RAPTOR_MALLOC(cstring, n3_parser->buffer_length + len + 1);
    if(!buffer) {
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      return 1;
    }

    if(n3_parser->buffer_length) {
      strncpy(buffer, n3_parser->buffer, n3_parser->buffer_length);
      RAPTOR_FREE(cstring, n3_parser->buffer);
    }

    n3_parser->buffer=buffer;

    /* move pointer to end of cdata buffer */
    ptr=buffer+n3_parser->buffer_length;

    /* adjust stored length */
    n3_parser->buffer_length += len;

    /* now write new stuff at end of cdata buffer */
    strncpy(ptr, (char*)s, len);
    ptr += len;
    *ptr = '\0';

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    RAPTOR_DEBUG3(raptor_n3_parse_chunk,
                  "buffer buffer now '%s' (%d bytes)\n", 
                  n3_parser->buffer, n3_parser->buffer_length);
#endif
  }
  
  /* if not end, wait for rest of input */
  if(!is_end)
    return 0;

  /* Nothing to do */
  if(!n3_parser->buffer_length)
    return 0;
  
  n3_parse(rdf_parser, n3_parser->buffer);
  
  return 0;
}


static int
raptor_n3_parse_start(raptor_parser *rdf_parser) 
{
  raptor_locator *locator=&rdf_parser->locator;
  raptor_n3_parser *n3_parser=(raptor_n3_parser*)rdf_parser->context;

  locator->line=1;
  locator->column= -1; /* No column info */
  locator->byte= -1; /* No bytes info */

  n3_parser->lineno=1;

  return 0;
}


static void
raptor_n3_parser_register_factory(raptor_parser_factory *factory) 
{
  factory->context_length     = sizeof(raptor_n3_parser);
  
  factory->init      = raptor_n3_parse_init;
  factory->terminate = raptor_n3_parse_terminate;
  factory->start     = raptor_n3_parse_start;
  factory->chunk     = raptor_n3_parse_chunk;
}


void
raptor_init_parser_n3 (void) {
  raptor_parser_register_factory("ntriples-plus",  "N-Triples Plus",
                                 &raptor_n3_parser_register_factory);
}



#ifdef STANDALONE
#include <stdio.h>
#include <locale.h>

#define N3_FILE_BUF_SIZE 2048

static
void n3_parser_print_statement(void *user, const raptor_statement *statement) 
{
  FILE* stream=(FILE*)user;
  raptor_print_statement(statement, stream);
  putc('\n', stream);
}
  


int
main(int argc, char *argv[]) 
{
  char string[N3_FILE_BUF_SIZE];
  raptor_parser rdf_parser; /* static */
  raptor_n3_parser n3_parser; /* static */
  raptor_locator *locator=&rdf_parser.locator;
  FILE *fh;

#if RAPTOR_DEBUG > 2
  n3_parser_debug=1;
#endif

  if(argc > 1) {
    filename=argv[1];
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

  memset(string, 0, N3_FILE_BUF_SIZE);
  fread(string, N3_FILE_BUF_SIZE, 1, fh);
  
  if(argc>1)
    fclose(fh);

  raptor_uri_init();

  memset(&rdf_parser, 0, sizeof(raptor_parser));
  memset(&n3_parser, 0, sizeof(raptor_n3_parser));

  locator->line= locator->column = -1;
  locator->file= filename;

  n3_parser.line= 1;

  rdf_parser.context=&n3_parser;
  rdf_parser.base_uri=raptor_new_uri("http://example.org/fake-base-uri/");

  raptor_set_statement_handler(&rdf_parser, stdout, n3_parser_print_statement);
  raptor_n3_parse_init(&rdf_parser, "ntriples-plus");
  
  n3_parse(&rdf_parser, string);

  raptor_free_uri(rdf_parser.base_uri);

  return (0);
}
#endif
