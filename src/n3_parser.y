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
#include <n3_lexer.h>


/* Make verbose error messages for syntax errors */
#ifdef RAPTOR_DEBUG
#define YYERROR_VERBOSE 1
#endif

/* Slow down the grammar operation and watch it work */
#if RAPTOR_DEBUG > 2
#define YYDEBUG 1
#endif

/* Prototypes */ 
int n3_parser_error(const char *msg);


/* Make lex/yacc interface as small as possible */
inline int
n3_parser_lex(void) {
  return n3_lexer_lex();
}

static raptor_triple* raptor_new_triple(raptor_identifier *subject, raptor_identifier *predicate, raptor_identifier *object, const char *object_literal_language, raptor_uri *object_literal_datatype);
static void raptor_free_triple(raptor_triple *triple);
static void raptor_print_triple(raptor_triple *data, FILE *fh);



/* Prototypes for local functions */
static raptor_uri* n3_qname_to_uri(raptor_parser *rdf_parser, char *qname_string);

static void raptor_n3_generate_statement(raptor_parser *parser, raptor_triple *triple);

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
};


/* globals */
static raptor_parser *N3_Parser;

%}


/* Interface between lexer and parser */
%union {
  unsigned char *string;
  raptor_identifier *identifier;
  raptor_sequence *sequence;
}




/* word symbols */
%token PREFIX

/* others */

%token WS A AT HAT
%token DOT COMMA SEMICOLON
%token LEFT_SQUARE RIGHT_SQUARE

/* literals */
%token <string> STRING_LITERAL
%token <string> URI_LITERAL
%token <string> BLANK_LITERAL
/*%token <string> VARIABLE_LITERAL*/
%token <string> QNAME_LITERAL
%token <string> PREFIX
%token <string> IDENTIFIER

/* end of input */
%token END

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
  printf("\n propertyList=");
  raptor_sequence_print($2, stdout);
  printf("\n");
#endif
  for(i=0; i<raptor_sequence_size($2); i++) {
    raptor_triple* t2=(raptor_triple*)raptor_sequence_get_at($2, i);
    raptor_identifier *i2=RAPTOR_CALLOC(raptor_identifier, 1, sizeof(raptor_identifier));
    raptor_copy_identifier(i2, $1);
    t2->subject=i2;
  }
#if RAPTOR_DEBUG > 1  
  printf(" after substitution propertyList=");
  raptor_sequence_print($2, stdout);
  printf("\n\n");
#endif
  for(i=0; i<raptor_sequence_size($2); i++) {
    raptor_triple* t2=(raptor_triple*)raptor_sequence_get_at($2, i);
    raptor_n3_generate_statement(N3_Parser, t2);
  }

  raptor_free_sequence($2);
}
;


objectList: object COMMA objectList
{
  raptor_triple *triple;

#if RAPTOR_DEBUG > 1  
  printf("objectList 1\n");
#endif
  triple=raptor_new_triple(NULL, NULL, $1, NULL, NULL);
  $$=$3;
  raptor_sequence_shift($$, triple);
#if RAPTOR_DEBUG > 1  
  printf(" objectList is now ");
  raptor_sequence_print($$, stdout);
  printf("\n\n");
#endif
}
| object
{
  raptor_triple *triple;

#if RAPTOR_DEBUG > 1  
  printf("objectList 2\n");
#endif

  triple=raptor_new_triple(NULL, NULL, $1, NULL, NULL);
  $$=raptor_new_sequence((raptor_free_handler*)raptor_free_triple,
                         (raptor_print_handler*)raptor_print_triple);
  raptor_sequence_push($$, triple);
#if RAPTOR_DEBUG > 1  
  printf(" objectList is now ");
  raptor_sequence_print($$, stdout);
  printf("\n\n");
#endif
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
#if RAPTOR_DEBUG > 1  
  printf("propertyList 1\n verb=");
  raptor_identifier_print(stdout, $1);
  printf("\n objectList=");
  raptor_sequence_print($2, stdout);
  printf("\n propertyList=");
  raptor_sequence_print($4, stdout);
  printf("\n\n");
#endif

  $$=$4;
}
| verb objectList
{
  int i;
#if RAPTOR_DEBUG > 1  
  printf("propertyList 2\n verb=");
  raptor_identifier_print(stdout, $1);
  printf("\n objectList=");
  raptor_sequence_print($2, stdout);
  printf("\n");
#endif
  
  for(i=0; i<raptor_sequence_size($2); i++) {
    raptor_triple* t2=(raptor_triple*)raptor_sequence_get_at($2, i);
    raptor_identifier *i2=RAPTOR_CALLOC(raptor_identifier, 1, sizeof(raptor_identifier));
    raptor_copy_identifier(i2, $1);
    t2->predicate=i2;
  }

#if RAPTOR_DEBUG > 1  
  printf(" after substitution objectList=");
  raptor_sequence_print($2, stdout);
  printf("\n\n");
#endif
  
  $$=$2;
}
;

directive : PREFIX QNAME_LITERAL URI_LITERAL DOT
{
  char *prefix=$2;
  raptor_uri* uri;
  raptor_n3_parser* n3_parser=(raptor_n3_parser*)N3_Parser->context;

#if RAPTOR_DEBUG > 1  
  printf("directive @prefix %s %s\n",($2 ? (char*)$2 : "(default)"),$3);
#endif

  if(prefix) {
    int len=strlen(prefix);
    if(prefix[len-1] == ':')
      prefix[len-1]='\0';
  }

  uri=raptor_new_uri_relative_to_base(N3_Parser->base_uri, $3);
  raptor_namespaces_start_namespace_full(&n3_parser->namespaces,
                                         prefix, raptor_uri_as_string(uri), 0);
  raptor_free_uri(uri);
}
;

subject: resource
{
  $$=$1;
}
| BLANK_LITERAL
{
  printf("subject blank=\"%s\"\n", $1);
  $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_ANONYMOUS, NULL, RAPTOR_URI_SOURCE_BLANK_ID, $1, NULL, NULL, NULL);
}
;

predicate: URI_LITERAL
{
  raptor_uri *uri;

#if RAPTOR_DEBUG > 1  
  printf("predicate URI=\"%s\"\n", $1);
#endif

  uri=raptor_new_uri($1);
  $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_RESOURCE, uri, RAPTOR_URI_SOURCE_URI, NULL, NULL, NULL, NULL);
}
| QNAME_LITERAL
{
  raptor_uri *uri;

#if RAPTOR_DEBUG > 1  
  printf("predicate qname=\"%s\"\n", $1);
#endif

  uri=n3_qname_to_uri(N3_Parser, $1);
  if(uri)
    $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_RESOURCE, uri, RAPTOR_URI_SOURCE_ELEMENT, NULL, NULL, NULL, NULL);
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
  raptor_uri *uri;
  
#if RAPTOR_DEBUG > 1  
  printf("literal + language=\"%s\" datatype string=\"%s\" uri=\"%s\"\n", $1, $3, $5);
#endif

  uri=raptor_new_uri($5);
  $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_LITERAL, NULL, RAPTOR_URI_SOURCE_ELEMENT, NULL, $1, uri, $3);
  raptor_free_uri(uri);
}
| STRING_LITERAL AT IDENTIFIER HAT QNAME_LITERAL
{
  raptor_uri *uri;
  
#if RAPTOR_DEBUG > 1  
  printf("literal + language=\"%s\" datatype string=\"%s\" qname=\"%s\"\n", $1, $3, $5);
#endif

  uri=n3_qname_to_uri(N3_Parser, $5);
  if(uri) {
    $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_LITERAL, NULL, RAPTOR_URI_SOURCE_ELEMENT, NULL, $1, uri, $3);
    raptor_free_uri(uri);
  } else
    $$=NULL;
}
| STRING_LITERAL HAT URI_LITERAL
{
  raptor_uri *uri;
  
#if RAPTOR_DEBUG > 1  
  printf("literal + datatype string=\"%s\" uri=\"%s\"\n", $1, $3);
#endif

  uri=raptor_new_uri($3);
  $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_LITERAL, NULL, RAPTOR_URI_SOURCE_ELEMENT, NULL, $1, uri, NULL);
  raptor_free_uri(uri);
}
| STRING_LITERAL HAT QNAME_LITERAL
{
  raptor_uri *uri;
  
#if RAPTOR_DEBUG > 1  
  printf("literal + datatype string=\"%s\" qname=\"%s\"\n", $1, $3);
#endif

  uri=n3_qname_to_uri(N3_Parser, $3);
  if(uri) {
    $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_LITERAL, NULL, RAPTOR_URI_SOURCE_ELEMENT, NULL, $1, uri, NULL);
    raptor_free_uri(uri);
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
  raptor_uri *uri;

#if RAPTOR_DEBUG > 1  
  printf("resource URI=\"%s\"\n", $1);
#endif

  uri=raptor_new_uri($1);
  $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_RESOURCE, uri, RAPTOR_URI_SOURCE_URI, NULL, NULL, NULL, NULL);
}
| QNAME_LITERAL
{
  raptor_uri *uri;

#if RAPTOR_DEBUG > 1  
  printf("resource qname=\"%s\"\n", $1);
#endif

  uri=n3_qname_to_uri(N3_Parser, $1);
  if(uri)
    $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_RESOURCE, uri, RAPTOR_URI_SOURCE_ELEMENT, NULL, NULL, NULL, NULL);
  else
    $$=NULL;
}
| LEFT_SQUARE propertyList RIGHT_SQUARE
{
  int i;
  const unsigned char *id=raptor_generate_id(N3_Parser, 0, NULL);
  
  $$=raptor_new_identifier(RAPTOR_IDENTIFIER_TYPE_ANONYMOUS, NULL, RAPTOR_URI_SOURCE_GENERATED, id, NULL, NULL, NULL);

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
  }

#if RAPTOR_DEBUG > 1  
  printf(" after substitution objectList=");
  raptor_sequence_print($2, stdout);
  printf("\n\n");
#endif
  
}
;


%%


/* Support functions */

/* helper - everything passed in is now owned by triple */
static
raptor_triple* raptor_new_triple(raptor_identifier *subject,
                                 raptor_identifier *predicate,
                                 raptor_identifier *object,
                                 const char *object_literal_language,
                                 raptor_uri *object_literal_datatype) 
{
  raptor_triple* t;
  
  t=RAPTOR_MALLOC(raptor_triple, sizeof(raptor_triple));
  if(!t)
    return NULL;
  
  t->subject=subject;
  t->predicate=predicate;
  t->object=object;
  t->object_literal_language=object_literal_language;
  t->object_literal_datatype=object_literal_datatype;

  return t;
}

static
void raptor_free_triple(raptor_triple *t) {
  if(t->subject)
    raptor_free_identifier(t->subject);
  if(t->predicate)
    raptor_free_identifier(t->predicate);
  if(t->object)
    raptor_free_identifier(t->object);
  if(t->object_literal_datatype)
    raptor_free_uri(t->object_literal_datatype);
  if(t->object_literal_language)
    RAPTOR_FREE(cstring, t->object_literal_language);
}
 

static
void raptor_print_triple(raptor_triple *t, FILE *fh) 
{
  fputs("triple(", fh);
  raptor_identifier_print(fh, t->subject);
  fputs(", ", fh);
  raptor_identifier_print(fh, t->predicate);
  fputs(", ", fh);
  raptor_identifier_print(fh, t->object);
  fputc(')', fh);
}


static raptor_uri*
n3_qname_to_uri(raptor_parser *rdf_parser, char *qname_string) 
{
  raptor_n3_parser* n3_parser=(raptor_n3_parser*)rdf_parser->context;
  raptor_uri* uri;
  raptor_qname *name;

  if(*qname_string == ':')
    qname_string++;
  else {
    int len=strlen(qname_string);
    if(qname_string[len-1] == ':')
      qname_string[len-1]='\0';
  }
  name=raptor_new_qname(&n3_parser->namespaces, qname_string, NULL,
                        raptor_parser_simple_error, n3_parser);
  if(!name)
    return NULL;
  
  uri=raptor_uri_copy(name->uri);
  raptor_free_qname(name);
  return uri;
}



static int
n3_parse(raptor_parser *rdf_parser, const char *string) {
  void *buffer;

  /* FIXME LOCKING or re-entrant parser/lexer */

  N3_Parser=rdf_parser;

  buffer= n3_lexer__scan_string(string);
  n3_lexer__switch_to_buffer(buffer);
  n3_parser_parse();

  N3_Parser=NULL;

  /* FIXME UNLOCKING or re-entrant parser/lexer */
  
  return 0;
}


extern char *filename;
extern int lineno;
 
int
n3_parser_error(const char *msg)
{
  fprintf(stderr, "%s:%d: %s\n", filename, lineno, msg);
  return (0);
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
  if(n3_parser->buffer_length)
    RAPTOR_FREE(cdata, n3_parser->buffer);
}


static void
raptor_n3_generate_statement(raptor_parser *parser, raptor_triple *t)
{
  raptor_statement *statement=&parser->statement;
  int predicate_ordinal=0;

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

  n3_parse(rdf_parser, n3_parser->buffer);
  
  return 0;
}


static int
raptor_n3_parse_start(raptor_parser *rdf_parser) 
{
  raptor_locator *locator=&rdf_parser->locator;

  locator->line=1;
  locator->column=0;
  locator->byte=0;

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
  raptor_parser_register_factory("n3",  "N-Triples Plus",
                                 &raptor_n3_parser_register_factory);
}



#ifdef STANDALONE
#include <stdio.h>
#include <locale.h>

#define N3_FILE_BUF_SIZE 2048

int
main(int argc, char *argv[]) 
{
  char string[N3_FILE_BUF_SIZE];
  FILE *fh;

#if RAPTOR_DEBUG > 2
  n3_parser_debug=1;
#endif
  
  if(argc > 1) {
    filename=argv[1];
    fh = fopen(argv[1], "r");
  } else {
    filename="<stdin>";
    fh = stdin;
  }

  memset(string, 0, N3_FILE_BUF_SIZE);
  fread(string, N3_FILE_BUF_SIZE, 1, fh);
  
  if(argc>1)
    fclose(fh);

  raptor_init();
  
  n3_parse(NULL, string);

  raptor_finish();

  return (0);
}
#endif
