/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * mkr_parser.y - Raptor mKR parser - over tokens from turtle grammar lexer
 *
 * Copyright (C) 2015, David Beckett http://www.dajobe.org/
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
 * mKR is defined in http://mkrmke.org/parser/
 *     question is sentence with at least one ?variable
 *
 * In this simplified implementation
 *     no pronouns
 *     no quantifiers
 *     no conditional
 *     no iteration
 *     variable has no ':', may have '?' '$' '*'
 *     mkr_sentence.c has most of the action functions
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

/* raptor_uri* raptor_new_uri_from_rdf_ordinal(raptor_world* world, int ordinal); */

/* Make lex/yacc interface as small as possible */
#undef yylex
#define yylex turtle_lexer_lex

/* Prototypes for local functions */
int mkr_syntax_error(raptor_parser *rdf_parser, const char *message, ...);
int mkr_parser_error(raptor_parser* rdf_parser, void* scanner, const char *msg);


%}


/* directives */

%require "3.0.0"

/* File prefix (bison -b) */
%file-prefix "mkr_parser"

/* Symbol prefix (bison -d : deprecated) */
%name-prefix "mkr_parser_"

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
  raptor_statement *statement;
  raptor_sequence *sequence;
  raptor_uri *uri;

  /* mKR */
  raptor_term *variable;
  MKR_nv *nv;
  MKR_pp *pp;
}


/* mKR tokens */
/* isVerb */
%token IS        "is"
/* hoVerb */
%token ISA       "isa"
%token ISS       "iss"
%token ISU       "isu"
%token ISASTAR   "isa*"
%token ISC       "isc"
%token ISG       "isg"
%token ISP       "isp"
%token ISCSTAR   "isc*"
/* hasVerb */
%token HAS       "has"
%token HASPART   "haspart"
%token ISAPART   "isapart"
/* doVerb */
%token DO        "do"
%token HDO       "hdo"
%token BANG      "!"
/* preposition */
%token AT        "at"
%token OF        "of"
%token WITH      "with"
%token OD        "od"
%token FROM      "from"
%token TO        "to"
%token IN        "in"
%token OUT       "out"
%token WHERE     "where"
/* hierarchy, relation */
%token mkrBEGIN  "begin"
%token mkrEND    "end"
%token HO        "ho"
%token REL       "rel"
/* other */
%token EQUALS    "="
%token ASSIGN    ":="
%token LET       "let"
%token DCOLON    "::"
%token DSTAR     "**"
%token atMKR     "@mkr"
%token atMKB     "@mkb"
%token atMKE     "@mke"

/* Turtle tokens */
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

/* mKR */
%token <string> VARIABLE "variable"

/* syntax error */
%token ERROR_TOKEN

%type <identifier> subject predicate object literal resource blankNode
%type <statement> predicateObject sentence
%type <sequence> objectList nvList ppList sentenceList predicateObjectList
%type <string> mkrbase mkrprefix

/* mKR */
%type <identifier> variable preposition isVerb hoVerb hasVerb doVerb nameOption sentenceName list set view graph ho rel
%type <nv> nv contextOption
%type <pp> pp

/* tidy up tokens after errors */

%destructor {
  if($$)
    RAPTOR_FREE(char*, $$);
} STRING_LITERAL BLANK_LITERAL INTEGER_LITERAL FLOATING_LITERAL DECIMAL_LITERAL IDENTIFIER LANGTAG VARIABLE mkrbase mkrprefix

%destructor {
  if($$)
    raptor_free_uri($$);
} URI_LITERAL QNAME_LITERAL

%destructor {
  if($$)
    raptor_free_term($$);
} subject predicate object literal resource blankNode variable preposition isVerb hoVerb hasVerb doVerb nameOption sentenceName list set view graph ho rel

%destructor {
  if($$)
    raptor_free_sequence($$);
} objectList nvList ppList sentenceList predicateObjectList

%destructor {
  if($$)
    raptor_free_statement($$);
} predicateObject sentence

%destructor {
  if($$)
    mkr_free_nv($$);
} nv contextOption

%destructor {
  if($$)
    mkr_free_pp($$);
} pp


%%

Document : sentenceList
;;

sentenceList: sentenceList sentence
| %empty
;

nv: predicate EQUALS object {$$ = mkr_new_nv(rdf_parser, MKR_NV, $1, $3);}
;

pp: preposition objectList  {$$ = mkr_new_pp(rdf_parser, MKR_PPOBJ, $1, $2);}
|   preposition nvList      {$$ = mkr_new_pp(rdf_parser, MKR_PPNV,  $1, $2);}
;

list: LEFT_ROUND            RIGHT_ROUND   {$$ = mkr_new_list(rdf_parser, NULL);}
|     LEFT_ROUND objectList RIGHT_ROUND   {$$ = mkr_new_list(rdf_parser, $2);}
;
set: LEFT_SQUARE            RIGHT_SQUARE  {$$ = mkr_new_set(rdf_parser, NULL);}
|    LEFT_SQUARE objectList RIGHT_SQUARE  {$$ = mkr_new_set(rdf_parser, $2);}
;

view:  LEFT_CURLY             sentenceList              RIGHT_CURLY {$$ = mkr_new_view(rdf_parser, $2);}
|      LEFT_CURLY LEFT_ROUND  sentenceList  RIGHT_ROUND RIGHT_CURLY {$$ = mkr_new_view(rdf_parser, $3);}
;
graph: LEFT_CURLY LEFT_SQUARE sentenceList RIGHT_SQUARE RIGHT_CURLY {$$ = mkr_new_graph(rdf_parser, $3);}
;

mkrbase: BASE     {$$ = (unsigned char*)"@base";}
|        atMKR    {$$ = (unsigned char*)"@mkr";}
;

mkrprefix: PREFIX {$$ = (unsigned char*)"@prefix";}
|          atMKB  {$$ = (unsigned char*)"@mkb";}
|          atMKE  {$$ = (unsigned char*)"@mke";}
;

/* view | graph | hierarchy | relation */
contextOption: IN variable subject {$$ = mkr_context(rdf_parser, MKR_BEGIN, $2, $3);}
| %empty                           {$$ = NULL;}
;
nameOption: variable DCOLON {$$ = $1;}
| %empty                    {$$ = NULL;}
;


subject: resource    { $$ = $1; }
| blankNode          { $$ = $1; }
| literal            { $$ = $1; }
| variable           { $$ = $1; }
| list               { $$ = $1; }
| set                { $$ = $1; }
| view               { $$ = $1; }
| graph              { $$ = $1; }
;

verb: hasVerb
| hoVerb
| isVerb
| doVerb
| variable
;

predicate: resource  { $$ = $1; }
| variable           { $$ = $1; }
| preposition        { $$ = $1; }
;

object: resource     { $$ = $1; }
| blankNode          { $$ = $1; }
| literal            { $$ = $1; }
| variable           { $$ = $1; }
| list               { $$ = $1; }
| set                { $$ = $1; }
| view               { $$ = $1; }
| graph              { $$ = $1; }
;

ho:  HO   {$$ = mkr_local_to_raptor_term(rdf_parser, (unsigned char*)"ho");}
rel: REL  {$$ = mkr_local_to_raptor_term(rdf_parser, (unsigned char*)"rel");}


sentence: contextOption nameOption view  SEMICOLON {$$ = mkr_sentence(rdf_parser, MKR_VIEW,  $1, $2, $3);}
|         contextOption nameOption graph SEMICOLON {$$ = mkr_sentence(rdf_parser, MKR_GRAPH, $1, $2, $3);}
| mkrbase URI_LITERAL                    SEMICOLON {$$ = mkr_base(rdf_parser, $<string>1, $2);}
| mkrprefix IDENTIFIER URI_LITERAL       SEMICOLON {$$ = mkr_prefix(rdf_parser, $<string>1, $2, $3);}

| mkrBEGIN variable subject              SEMICOLON {$$ = mkr_group(rdf_parser, MKR_BEGIN, $2, $3);}
| ho       objectList                    SEMICOLON {$$ = mkr_ho(rdf_parser,  $1, $2);}
| rel      objectList                    SEMICOLON {$$ = mkr_rel(rdf_parser, $1, $2);}
| mkrEND   variable subject              SEMICOLON {$$ = mkr_group(rdf_parser, MKR_END, $2, $3);}

| variable ASSIGN object                 SEMICOLON {$$ = mkr_assignment(rdf_parser, $1, $3);}
| doVerb  object                         SEMICOLON {$$ = mkr_command(rdf_parser, NULL, $1, $2, NULL);}
| doVerb  object ppList                  SEMICOLON {$$ = mkr_command(rdf_parser, NULL, $1, $2, $3);}

| subject doVerb  object                 SEMICOLON {$$ = mkr_action(rdf_parser, $1, $2, $3, NULL);}
| subject doVerb  object ppList          SEMICOLON {$$ = mkr_action(rdf_parser, $1, $2, $3, $4);}
| subject isVerb  object ppList          SEMICOLON {$$ = mkr_definition(rdf_parser, $1, $2, $3, $4);}

/* mKR9: Turtle style */
| subject predicateObjectList            SEMICOLON {$$ = mkr_poList(rdf_parser, $1, $2);}

/* mKR8: old mKR stye - not accepted
| subject hasVerb nvList                 SEMICOLON {$$ = mkr_attribute(rdf_parser, $1, $2, $3);}
| subject hoVerb  objectList             SEMICOLON {$$ = mkr_hierarchy(rdf_parser, $1, $2, $3);}
| subject isVerb  objectList             SEMICOLON {$$ = mkr_alias(rdf_parser, $1, $2, $3);}
*/

| error SEMICOLON {$$ = NULL;}
;

predicateObject: hasVerb nv      {$$ = mkr_attribute(rdf_parser, NULL, $1, $2);}
               | hoVerb  object  {$$ = mkr_hierarchy(rdf_parser, NULL, $1, $2);}
               | isVerb  object  {$$ = mkr_alias(rdf_parser,     NULL, $1, $2);}
;


predicateObjectList: predicateObjectList COMMA predicateObject
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("predicateObjectList 1\n");
  if($3) {
    printf(" predicateObject=\n");
    mkr_statement_print($3, stdout);
    printf("\n");
  } else  
    printf(" and empty object\n");
  if($1) {
    printf(" predicateObjectList=");
    raptor_sequence_print($1, stdout);
    printf("\n");
  } else
    printf(" and empty predicateObjectList\n");
#endif

  if(!$3)
    $$ = NULL;
  else {
    if(raptor_sequence_push($1, $3)) {
      raptor_free_sequence($1);
      YYERROR;
    }
    $$ = $1;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" predicateObjectList is now ");
    raptor_sequence_print($$, stdout);
    printf("\n\n");
#endif
  }
}
| predicateObject
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("predicateObjectList 2\n");
  if($1) {
    printf(" predicateObject=\n");
    mkr_statement_print($1, stdout);
    printf("\n");
  } else  
    printf(" and empty object\n");
#endif

  if(!$1)
    $$ = NULL;
  else {
#ifdef RAPTOR_DEBUG
    $$ = raptor_new_sequence((raptor_data_free_handler)raptor_free_statement,
                             (raptor_data_print_handler)mkr_statement_print);
#else
    $$ = raptor_new_sequence((raptor_data_free_handler)raptor_free_statement, NULL);
#endif
    if(!$$) {
      raptor_free_statement($1);
      YYERROR;
    }
    if(raptor_sequence_push($$, $1)) {
      raptor_free_sequence($$);
      $$ = NULL;
      YYERROR;
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" predicateObjectList is now ");
    raptor_sequence_print($$, stdout);
    printf("\n\n");
#endif
  }
}
;


objectList: objectList COMMA object
{
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
    if(raptor_sequence_push($1, $3)) {
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
#ifdef RAPTOR_DEBUG
    $$ = raptor_new_sequence((raptor_data_free_handler)raptor_free_term,
                             (raptor_data_print_handler)raptor_term_print_as_ntriples);
#else
    $$ = raptor_new_sequence((raptor_data_free_handler)raptor_free_term, NULL);
#endif
    if(!$$) {
      raptor_free_term($1);
      YYERROR;
    }
    if(raptor_sequence_push($$, $1)) {
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

nvList: nvList COMMA nv
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("nvList 1\n");
  if($3) {
    printf(" nv=\n");
    mkr_nv_print($3, stdout);
    printf("\n");
  } else  
    printf(" and empty nv\n");
  if($1) {
    printf(" nvList=");
    raptor_sequence_print($1, stdout);
    printf("\n");
  } else
    printf(" and empty nvList\n");
#endif

  if(!$3)
    $$ = NULL;
  else {
    if(raptor_sequence_push($1, $3)) {
      raptor_free_sequence($1);
      YYERROR;
    }
    $$ = $1;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" nvList is now ");
    raptor_sequence_print($$, stdout);
    printf("\n\n");
#endif
  }
}
| nv
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("nvList 2\n");
  if($1) {
    printf(" nv=\n");
    mkr_nv_print($1, stdout);
    printf("\n");
  } else  
    printf(" and empty nv\n");
#endif

  if(!$1)
    $$ = NULL;
  else {
#ifdef RAPTOR_DEBUG
    $$ = raptor_new_sequence((raptor_data_free_handler)mkr_free_nv,
                             (raptor_data_print_handler)mkr_nv_print);
#else
    $$ = raptor_new_sequence((raptor_data_free_handler)mkr_free_nv, NULL);
#endif
    if(!$$) {
      YYERROR;
    }
    if(raptor_sequence_push($$, $1)) {
      raptor_free_sequence($$);
      $$ = NULL;
      YYERROR;
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" nvList is now ");
    raptor_sequence_print($$, stdout);
    printf("\n\n");
#endif
  }
}
;

ppList: ppList pp
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("ppList 1\n");
  if($2) {
    printf(" pp=\n");
    mkr_pp_print($2, stdout);
    printf("\n");
  } else  
    printf(" and empty pp\n");
  if($1) {
    printf(" ppList=");
    raptor_sequence_print($1, stdout);
    printf("\n");
  } else
    printf(" and empty ppList\n");
#endif

  if(!$2)
    $$ = NULL;
  else {
    if(raptor_sequence_push($1, $2)) {
      raptor_free_sequence($1);
      YYERROR;
    }
    $$ = $1;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" ppList is now ");
    raptor_sequence_print($$, stdout);
    printf("\n\n");
#endif
  }
}
| pp
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("ppList 2\n");
  if($1) {
    printf(" pp=\n");
    mkr_pp_print($1, stdout);
    printf("\n");
  } else  
    printf(" and empty pp\n");
#endif

  if(!$1)
    $$ = NULL;
  else {
#ifdef RAPTOR_DEBUG
    $$ = raptor_new_sequence((raptor_data_free_handler)mkr_free_pp,
                             (raptor_data_print_handler)mkr_pp_print);
#else
    $$ = raptor_new_sequence((raptor_data_free_handler)mkr_free_pp, NULL);
#endif
    if(!$$) {
      YYERROR;
    }
    if(raptor_sequence_push($$, $1)) {
      raptor_free_sequence($$);
      $$ = NULL;
      YYERROR;
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" ppList is now ");
    raptor_sequence_print($$, stdout);
    printf("\n\n");
#endif
  }
}
;
variable: VARIABLE {$$ = mkr_new_variable(rdf_parser, MKR_VARIABLE, $1);}
;

isVerb: IS  {$$ = mkr_new_verb(rdf_parser, MKR_IS, (unsigned char*)"is");}
;

hoVerb: ISU      {$$ = mkr_new_verb(rdf_parser, MKR_ISU, (unsigned char*)"isu");}
|       ISS      {$$ = mkr_new_verb(rdf_parser, MKR_ISS, (unsigned char*)"iss");}
|       ISA      {$$ = mkr_new_verb(rdf_parser, MKR_ISA, (unsigned char*)"isa");}
|       ISASTAR  {$$ = mkr_new_verb(rdf_parser, MKR_ISASTAR, (unsigned char*)"isa*");}
|       ISP      {$$ = mkr_new_verb(rdf_parser, MKR_ISP, (unsigned char*)"isp");}
|       ISG      {$$ = mkr_new_verb(rdf_parser, MKR_ISG, (unsigned char*)"isg");}
|       ISC      {$$ = mkr_new_verb(rdf_parser, MKR_ISC, (unsigned char*)"isc");}
|       ISCSTAR  {$$ = mkr_new_verb(rdf_parser, MKR_ISCSTAR, (unsigned char*)"isc*");}
|       variable {$$ = $1;}
;

hasVerb: HAS      {$$ = mkr_new_verb(rdf_parser, MKR_HAS,     (unsigned char*)"has");}
|        HASPART  {$$ = mkr_new_verb(rdf_parser, MKR_HASPART, (unsigned char*)"haspart");}
|        ISAPART  {$$ = mkr_new_verb(rdf_parser, MKR_ISAPART, (unsigned char*)"isapart");}
;

doVerb: DO    {$$ = mkr_new_verb(rdf_parser, MKR_DO,   (unsigned char*)"do");}
|       HDO   {$$ = mkr_new_verb(rdf_parser, MKR_HDO,  (unsigned char*)"hdo");}
|       BANG  {$$ = mkr_new_verb(rdf_parser, MKR_BANG, (unsigned char*)"!");}
;

preposition: AT     {$$ = mkr_new_preposition(rdf_parser, MKR_AT,    (unsigned char*)"at");}
|            OF     {$$ = mkr_new_preposition(rdf_parser, MKR_OF,    (unsigned char*)"of");}
|            WITH   {$$ = mkr_new_preposition(rdf_parser, MKR_WITH,  (unsigned char*)"with");}
|            OD     {$$ = mkr_new_preposition(rdf_parser, MKR_OD,    (unsigned char*)"od");}
|            FROM   {$$ = mkr_new_preposition(rdf_parser, MKR_FROM,  (unsigned char*)"from");}
|            TO     {$$ = mkr_new_preposition(rdf_parser, MKR_TO,    (unsigned char*)"to");}
|            IN     {$$ = mkr_new_preposition(rdf_parser, MKR_IN,    (unsigned char*)"in");}
|            OUT    {$$ = mkr_new_preposition(rdf_parser, MKR_OUT,   (unsigned char*)"out");}
|            WHERE  {$$ = mkr_new_preposition(rdf_parser, MKR_WHERE, (unsigned char*)"where");}
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

/*
blankNodePropertyList: LEFT_SQUARE blankNode HAS predicateObjectList RIGHT_SQUARE
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
*/
    /* non-empty property list, handle it  */
/*
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf("resource\n predicateObjectList=");
    raptor_sequence_print($2, stdout);
    printf("\n");
#endif

    for(i = 0; i < raptor_sequence_size($2); i++) {
      raptor_statement* t2 = (raptor_statement*)raptor_sequence_get_at($2, i);
      t2->subject = raptor_term_copy($$);
      raptor_mkr_generate_statement(rdf_parser, t2);
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
*/


%%

/******************************************************************************/
/******************************************************************************/
/* Support functions */

/* concept isu genus; */
void
mkr_generate_type_statement(raptor_parser* parser, raptor_term* concept, raptor_term* genus)
{
  raptor_world* world = parser->world;
  raptor_statement* triple = NULL;
  raptor_term* isu = NULL;

  concept = raptor_term_copy(concept);
  isu = raptor_term_copy(mkr_local_to_raptor_term(parser, (unsigned char*)"isu"));
  genus = raptor_term_copy(genus);
  triple = raptor_new_statement_from_nodes(world, concept, isu, genus, NULL);
  raptor_mkr_generate_statement(parser, triple);
  raptor_free_statement(triple);
}

/******************************************************************************/
/* rdf:_n */
raptor_term*
mkr_new_index_term(raptor_world* world, int ordinal)
{
  raptor_uri* uri = NULL;
  raptor_term* term = NULL;


  uri = raptor_new_uri_from_rdf_ordinal(world, ordinal);
  term = raptor_new_term_from_uri(world, uri); /* RAPTOR_TERM_TYPE_URI */

  raptor_free_uri(uri);
#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_new_index_term: ");
  printf("ordinal = %u: term = ", ordinal);
  raptor_term_print_as_ntriples(term, stdout);
  print("\n");
#endif
  return term;
}

/******************************************************************************/
  /* code adapted from raptor_stringbuffer_append_decimal()
   * enough for 64 bit signed integer
   * INT64_MAX is  9223372036854775807 (19 digits) + 1 for sign 
   * Return value: NULL on failure
   */

raptor_term*
mkr_new_integer_term(raptor_world* world, int integer)
{
  raptor_term* term = NULL;
  raptor_uri* uri = NULL;


  unsigned char buf[20];
  unsigned char *p;
  int i = integer;
  size_t length = 1;

  if(integer < 0) {
    length++;
    i= -integer;
  }
  while(i /= 10)
    length++;

  p = buf+length-1;
  i = integer;
  if(i < 0)
    i= -i;
  do {
    *p-- = RAPTOR_GOOD_CAST(unsigned char, '0'+(i %10));
    i /= 10;
  } while(i);
  if(integer < 0)
    *p= '-';


  buf[length] = '\0';
  uri = raptor_uri_copy(world->xsd_integer_uri); 
  term = raptor_new_term_from_literal(world, buf, uri, NULL);

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_new_integer_term: ");
  printf("integer = %d: buf = (%s) term = ", integer, buf);
  raptor_term_print_as_ntriples(term, stdout);
  printf("\n");
#endif

  raptor_free_uri(uri);
  return term;
}

/******************************************************************************/
raptor_term*
mkr_new_list(raptor_parser *parser, raptor_sequence* objectList)
{
  raptor_world* world = parser->world;
  raptor_term* rdfSeq = rdf_local_to_raptor_term(parser, (unsigned char*)"Seq");
  raptor_term* list = NULL;
  raptor_term* subject = NULL;
  raptor_term* prop = NULL;
  raptor_term* object = NULL;
  raptor_statement* triple = NULL;
  int size = 0;
  int i;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;

  size = raptor_sequence_size(objectList);

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_new_list: size = %u\n", size);
#endif

  list = mkr_new_blank_term(parser);
  list->mkrtype = MKR_LIST;
  mkr_generate_type_statement(parser, list, rdfSeq);
  for(i = 0; i < size; i++) {
    subject = raptor_term_copy(list);
    prop = mkr_new_index_term(world, i+1);
    object = (raptor_term*)raptor_sequence_get_at(objectList, i);
    triple = raptor_new_statement_from_nodes(world, subject, prop, object, NULL);
    raptor_mkr_generate_statement(parser, triple);
    raptor_free_statement(triple);
  }
  if(size) {
    subject = raptor_term_copy(list);
    prop = mkr_local_to_raptor_term(parser, (unsigned char*)"size");
    object = mkr_new_integer_term(world, size);
    triple = raptor_new_statement_from_nodes(world, subject, prop, object, NULL);
    raptor_mkr_generate_statement(parser, triple);
    raptor_free_statement(triple);
  }

  raptor_free_term(rdfSeq);

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: exit mkr_new_list:\n");
#endif
  return list;
}

/******************************************************************************/
raptor_term*
mkr_new_set(raptor_parser *parser, raptor_sequence* objectList)
{
  raptor_world* world = parser->world;
  raptor_term* rdfBag = rdf_local_to_raptor_term(parser, (unsigned char*)"Bag");
  raptor_term* set = NULL;
  raptor_term* subject = NULL;
  raptor_term* prop = NULL;
  raptor_term* object = NULL;
  raptor_statement* triple = NULL;
  int size;
  int i;

  size = raptor_sequence_size(objectList);

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_new_set: size = %u\n", size);
#endif

  set = mkr_new_blank_term(parser);
  set->mkrtype = MKR_SET;
  mkr_generate_type_statement(parser, set,rdfBag);
  for(i = 0; i < size; i++) {
    subject = raptor_term_copy(set);
    prop = mkr_new_index_term(world, i+1);
    object = (raptor_term*)raptor_sequence_get_at(objectList, i);
    triple = raptor_new_statement_from_nodes(world, subject, prop, object, NULL);
    raptor_mkr_generate_statement(parser, triple);
    raptor_free_statement(triple);
  }
  if(size) {
    subject = raptor_term_copy(set);
    prop = mkr_local_to_raptor_term(parser, (unsigned char*)"size");
    object = mkr_new_integer_term(world, size);
    triple = raptor_new_statement_from_nodes(world, subject, prop, object, NULL);
    raptor_mkr_generate_statement(parser, triple);
    raptor_free_statement(triple);
  }

  raptor_free_term(rdfBag);

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: exit mkr_new_set:\n");
#endif
  return set;
}

/******************************************************************************/
raptor_term*
mkr_new_view(raptor_parser *parser, raptor_sequence* sentenceList)
{
  raptor_world* world = parser->world;
  raptor_term* mkrview = mkr_local_to_raptor_term(parser, (unsigned char*)"view");
  raptor_term* view = NULL;
  raptor_term* subject = NULL;
  raptor_term* prop = NULL;
  raptor_term* object = NULL;
  raptor_statement* triple = NULL;
  int size = raptor_sequence_size(sentenceList);

#if defined(RAPTOR_DEBUG)
  printf("mkr_new_view: size = %u\n", size);
#endif

  view = mkr_new_list(parser, sentenceList);
  view->mkrtype = MKR_VIEW;
  mkr_generate_type_statement(parser, view, mkrview);

  if(size) {
    subject = raptor_term_copy(view);
    prop = mkr_local_to_raptor_term(parser, (unsigned char*)"size");
    object = mkr_new_integer_term(world, size);
    triple = raptor_new_statement_from_nodes(world, subject, prop, object, NULL);
    raptor_mkr_generate_statement(parser, triple);
    raptor_free_statement(triple);
  }

  raptor_free_term(mkrview);

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: exit mkr_new_view:\n");
#endif
  return view;
}

/******************************************************************************/
raptor_term*
mkr_new_graph(raptor_parser *parser, raptor_sequence* sentenceList)
{
  raptor_world* world = parser->world;
  raptor_term* mkrgraph = mkr_local_to_raptor_term(parser, (unsigned char*)"graph");
  raptor_term* graph = NULL;
  raptor_term* subject = NULL;
  raptor_term* prop = NULL;
  raptor_term* object = NULL;
  raptor_statement* triple = NULL;
  int size = raptor_sequence_size(sentenceList);

#if defined(RAPTOR_DEBUG)
  fprintf(stdout, "mkr_new_graph: size = %u\n", size);
#endif

  graph = mkr_new_set(parser, sentenceList);
  graph->mkrtype = MKR_GRAPH;
  mkr_generate_type_statement(parser, graph, mkrgraph);

  if(size) {
    subject = raptor_term_copy(graph);
    prop = mkr_local_to_raptor_term(parser, (unsigned char*)"size");
    object = mkr_new_integer_term(world, size);
    triple = raptor_new_statement_from_nodes(world, subject, prop, object, NULL);
    raptor_mkr_generate_statement(parser, triple);
    raptor_free_statement(triple);
  }

  raptor_free_term(mkrgraph);

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: exit mkr_new_graph:\n");
#endif
  return graph;
}


/******************************************************************************/
/******************************************************************************/
raptor_term*
mkr_new_variable(raptor_parser *parser, mkr_type type, unsigned char* name)
{
  raptor_term *t = NULL;
  t = mkr_local_to_raptor_term(parser, name); /* RAPTOR_TERM_TYPE_URI */
  if(!t)
    printf("##### ERROR: mkr_new_variable: can't create new variable\n");
  t->mkrtype = type;

  return t;
}

raptor_term*
mkr_new_verb(raptor_parser *parser, mkr_type type, unsigned char* name)
{
  return mkr_new_variable(parser, type, name);
}

raptor_term*
mkr_new_pronoun(raptor_parser *parser, mkr_type type, unsigned char* name)
{
  return mkr_new_variable(parser, type, name);
}

raptor_term*
mkr_new_preposition(raptor_parser *parser, mkr_type type, unsigned char* name)
{
  return mkr_new_variable(parser, type, name);
}

raptor_term*
mkr_new_conjunction(raptor_parser *parser, mkr_type type, unsigned char* name)
{
  return mkr_new_variable(parser, type, name);
}

raptor_term*
mkr_new_quantifier(raptor_parser *parser, mkr_type type, unsigned char* name)
{
  return mkr_new_variable(parser, type, name);
}

raptor_term*
mkr_new_iterator(raptor_parser *parser, mkr_type type, unsigned char* name)
{
  return mkr_new_variable(parser, type, name);
}


/******************************************************************************/
/*****************************************************************************/
raptor_term*
mkr_local_to_raptor_term(raptor_parser* rdf_parser, unsigned char *local) 
{
  raptor_world* world = rdf_parser->world;
  raptor_uri *uri = NULL;
  raptor_term *term = NULL;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    printf("##### DEBUG: mkr_local_to_raptor_term: local = %s", (char*)local);
#endif
  uri = raptor_new_uri_for_mkr_concept(world, (const unsigned char*)local);
  term = raptor_new_term_from_uri(world, uri); /* RAPTOR_TERM_TYPE_URI */

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf(", term = ");
  raptor_term_print_as_ntriples(term, stdout);
  printf("\n");
#endif
  raptor_free_uri(uri);
  return term;
}

raptor_term*
rdf_local_to_raptor_term(raptor_parser* rdf_parser, unsigned char *local) 
{
  raptor_world* world = rdf_parser->world;
  raptor_uri *uri = NULL;
  raptor_term *term = NULL;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    printf("##### DEBUG: rdf_local_to_raptor_term: local = %s", (char*)local);
#endif
  uri = raptor_new_uri_for_rdf_concept(world, (const unsigned char*)local);
  term = raptor_new_term_from_uri(world, uri); /* RAPTOR_TERM_TYPE_URI */

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf(", term = ");
  raptor_term_print_as_ntriples(term, stdout);
  printf("\n");
#endif
  raptor_free_uri(uri);
  return term;
}


/*****************************************************************************/
raptor_term*
mkr_new_blank_term(raptor_parser* rdf_parser)
{
  raptor_world* world = rdf_parser->world;
  const unsigned char *id = NULL;
  raptor_term *blankNode = NULL;
  id = raptor_world_generate_bnodeid(world);
  if(!id) {
    printf("Cannot create bnodeid\n");
    goto blank_error;
  }

  blankNode = raptor_new_term_from_blank(world, id); /* RAPTOR_TERM_TYPE_BLANK */
  RAPTOR_FREE(char*, id);
  if(!blankNode) {
    printf("Cannot create blankNode\n");
    goto blank_error;
  }

  blank_error:
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_new_blank_term: ");
  raptor_term_print_as_ntriples(blankNode, stdout);
  printf("\n");
#endif
  return blankNode;
}

/*****************************************************************************/
int
raptorstring_to_csvstring(unsigned char *string, unsigned char *csvstr)
{
  int rc = 0;
  size_t len = strlen((char*)string);
  const char delim = '\x22';
  int quoting_needed = 0;
  size_t i;
  size_t j = 0;

  for(i = 0; i < len; i++) {
    char c = string[i];
    /* Quoting needed for delim (double quote), comma, linefeed or return */
    if(c == delim   || c == ',' || c == '\r' || c == '\n') {
      quoting_needed++;
      break;
    }
  }
  if(quoting_needed)
    csvstr[j++] = delim;
  for(i = 0; i < len; i++) {
    char c = string[i];
    if(quoting_needed && (c == delim))
      csvstr[j++] = delim;
    csvstr[j++] = c;
  }
  if(quoting_needed)
    csvstr[j++] = delim;
  csvstr[j++] = '\0';

  return rc;
}
/************************************************************************************/
/* nv, pp, value, rdflist Classes */


/*****************************************************************************/
/*****************************************************************************/
/* nv: predicate EQUALS object; */
MKR_nv*
mkr_new_nv(raptor_parser* rdf_parser, mkr_type type, raptor_term* predicate, raptor_term* object)
{
  raptor_world* world = rdf_parser->world;
  MKR_nv* nv = NULL;
  const char* separator = "=";

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_new_nv: ");
  raptor_term_print_as_ntriples(predicate, stdout);
  printf(" %s ", separator);
  raptor_term_print_as_ntriples(object, stdout);
  printf("\n");
#endif
  switch(type) {
    case MKR_NV:
      RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);
      raptor_world_open(world);
      nv = RAPTOR_CALLOC(MKR_nv*, 1, sizeof(*nv));
      if(!nv) {
        printf("##### ERROR: mkr_new_nv: cannot create MKR_nv\n");
        return NULL;
      }
      nv->nvType = type;
      nv->nvName = raptor_term_copy(predicate);
      nv->nvSeparator = (const unsigned char*)separator;
      nv->nvValue = raptor_term_copy(object);
      break;

  default:
    printf("##### ERROR: mkr_new_nv: not MKR_NV: %s\n", MKR_TYPE_NAME(type));
    break;
  }

  return nv;
}

/*****************************************************************************/
void
mkr_free_nv(MKR_nv* nv)
{
  mkr_type type = MKR_UNKNOWN;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_free_nv: ");
#endif
  if(!nv)
    return;
  type = nv->nvType;
return;
  switch(type) {
    case MKR_NV:
      if(nv->nvName) {
        raptor_free_term(nv->nvName);
        nv->nvName = NULL;
      }
      if(nv->nvValue) {
        raptor_free_term(nv->nvValue);
        nv->nvValue = NULL;
      }
      RAPTOR_FREE(MKR_nv, nv);
      nv = NULL;
      break;

    default:
      printf("##### ERROR: mkr_free_nv: not MKR_NV: %s\n", MKR_TYPE_NAME(type));
      break;
  }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: exit mkr_free_nv\n");
#endif
}

/*****************************************************************************/
void
mkr_nv_print(MKR_nv* nv, FILE* fh)
{
  mkr_type type = nv->nvType;

  switch(type) {
    case MKR_NV:
      printf("%s(", MKR_TYPE_NAME(type));
      raptor_term_print_as_ntriples(nv->nvName, stdout);
      printf(" %s ",nv->nvSeparator);
      raptor_term_print_as_ntriples(nv->nvValue, stdout);
      printf(")");
      break;

    default:
      printf("(not nv: %s)", MKR_TYPE_NAME(type));
      break;
  }
}

/*****************************************************************************/
/*****************************************************************************/
/* pp: preposition objectList */
/* pp: preposition nvList */
MKR_pp*
mkr_new_pp(raptor_parser* rdf_parser, mkr_type type, raptor_term* preposition, raptor_sequence* sequence)
{
  raptor_world* world = rdf_parser->world;
  MKR_pp* pp = NULL;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_new_pp: ");
  raptor_term_print_as_ntriples(preposition, stdout);
  printf(" ");
  raptor_sequence_print(sequence, stdout);
  printf("\n");
#endif
  switch(type) {
    case MKR_PP:
    case MKR_PPOBJ:
    case MKR_PPNV:
      RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);
      raptor_world_open(world);
      pp = RAPTOR_CALLOC(MKR_pp*, 1, sizeof(*pp));
      if(!pp) {
        printf("##### ERROR: mkr_new_pp: cannot create MKR_pp\n");
        goto pp_error;
      }
      pp->ppType = type;
      pp->ppPreposition = raptor_term_copy(preposition);
      pp->ppSequence = sequence;
      break;

    default:
    printf("##### ERROR: mkr_new_pp: not MKR_PP type: %s\n", MKR_TYPE_NAME(type));
    break;
  }

  pp_error:
  return pp;
}

/*****************************************************************************/
void
mkr_free_pp(MKR_pp* pp)
{
  mkr_type type = MKR_UNKNOWN;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_free_pp: ");
#endif
  if(!pp)
    return;
  type = pp->ppType;
return;
  switch(type) {
    case MKR_PP:
    case MKR_PPOBJ:
    case MKR_PPNV:
      if(pp->ppPreposition) {
        raptor_free_term(pp->ppPreposition);
        pp->ppPreposition = NULL;
      }
      if(pp->ppSequence) {
        raptor_free_sequence(pp->ppSequence);
        pp->ppSequence = NULL;
      }
      RAPTOR_FREE(MKR_pp, pp);
      pp = NULL;
      break;

    default:
      printf("##### ERROR: mkr_free_pp: not MKR_PP: %s\n", MKR_TYPE_NAME(type));
      break;
  }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: exit mkr_free_pp\n");
#endif
}

/*****************************************************************************/
void
mkr_pp_print(MKR_pp* pp, FILE* fh)
{
  mkr_type type = pp->ppType;

  switch(type) {
    case MKR_PP:
    case MKR_PPOBJ:
    case MKR_PPNV:
      printf("%s(", MKR_TYPE_NAME(type));
      raptor_term_print_as_ntriples(pp->ppPreposition, stdout);
      printf(" ");
      raptor_sequence_print(pp->ppSequence, stdout);
      printf(")");
      break;

    default:
      printf("(not pp: %s)", MKR_TYPE_NAME(type));
      break;
  }
}

/******************************************************************************/
/*****************************************************************************/
/* rdflist using first, rest, nil */

raptor_term*
mkr_new_rdflist(raptor_parser* rdf_parser, raptor_sequence* seq)
{
  int i;
  raptor_world* world = rdf_parser->world;
  raptor_term* rdflist = NULL;
  raptor_term* first_identifier = NULL;
  raptor_term* rest_identifier = NULL;
  raptor_term* blank = NULL;
  raptor_term* object = NULL;
  raptor_statement* triple = NULL;
  char const *errmsg = NULL;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: mkr_new_rdflist: ");
#endif

/* collection:  LEFT_SQUARE RIGHT_SQUARE */
  if(!seq) {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf("collection\n empty\n");
#endif

    rdflist = raptor_new_term_from_uri(world, RAPTOR_RDF_nil_URI(world));

    if(!rdflist)
      /*YYERROR;*/ printf("##### ERROR: mkr_rdflist: can't create nil term");

#if defined(RAPTOR_DEBUG)
    printf("return from mkr_new_rdflist\n");
#endif
    return rdflist;
  }

/* collection: LEFT_SQUARE seq RIGHT_SQUARE */
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("collection\n seq=");
  raptor_sequence_print(seq, stdout);
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
  printf("resource\n seq=");
  raptor_sequence_print(seq, stdout);
  printf("\n");
#endif

  object = raptor_new_term_from_uri(world, RAPTOR_RDF_nil_URI(world));
  if(!object)
    YYERR_MSG_GOTO(err_collection, "Cannot create rdf:nil term");

  for(i = raptor_sequence_size(seq)-1; i>=0; i--) { /* process objects in reverse order */
    raptor_term* temp;
    const unsigned char *blank_id;
    triple = raptor_new_statement_from_nodes(world, NULL, NULL, NULL, NULL);
    if(!triple)
      YYERR_MSG_GOTO(err_collection, "Cannot create new statement");
    triple->object = (raptor_term*)raptor_sequence_get_at(seq, i);

    blank_id = raptor_world_generate_bnodeid(rdf_parser->world);
    if(!blank_id)
      YYERR_MSG_GOTO(err_collection, "Cannot create bnodeid");

    blank = raptor_new_term_from_blank(rdf_parser->world,
                                       blank_id);
    RAPTOR_FREE(char*, blank_id);
    if(!blank)
      YYERR_MSG_GOTO(err_collection, "Cannot create bnode");
    
    triple->subject = blank;
    triple->predicate = first_identifier;
    /* object already set */
    raptor_mkr_generate_statement((raptor_parser*)rdf_parser, triple);
    
    temp = triple->object;
    
    triple->subject = blank;
    triple->predicate = rest_identifier;
    triple->object = object;
    raptor_mkr_generate_statement((raptor_parser*)rdf_parser, triple);

    triple->subject = NULL;
    triple->predicate = NULL;
    triple->object = temp;

    raptor_free_term(object);
    object = blank;
    blank = NULL;
  }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf(" after substitution seq=");
  raptor_sequence_print(seq, stdout);
  printf("\n\n");
#endif

  raptor_free_sequence(seq);

  raptor_free_term(first_identifier);
  raptor_free_term(rest_identifier);

  rdflist = object;

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

    raptor_free_sequence(seq);

    /* YYERROR_MSG(errmsg); */ printf("##### ERROR: mkr_rdflist: %s\n", errmsg);
  }

#if defined(RAPTOR_DEBUG)
  printf("return from mkr_new_rdflist\n");
#endif
  rdflist->mkrtype = MKR_RDFLIST;
  return rdflist;
}

void
mkr_free_rdflist(raptor_term* rdflist)
{
}

void
mkr_rdflist_print(raptor_term* rdflist, FILE* fh)
{
}

/*****************************************************************************/
/* rdfset using first, rest, nil */

raptor_term*
mkr_new_rdfset(raptor_parser* rdf_parser, raptor_sequence* seq)
{
  raptor_term* rdfset = NULL;

  rdfset = mkr_new_rdflist(rdf_parser, seq);
  rdfset->mkrtype = MKR_RDFSET;

  return rdfset;
}

void
mkr_free_rdfset(raptor_term* rdfset)
{
}

void
mkr_rdfset_print(raptor_term* rdfset, FILE* fh)
{
}

/*****************************************************************************/
/* sentence using subject, predicate, object */

raptor_term*
mkr_new_sentence(raptor_parser* rdf_parser, raptor_statement *triple) 
{
  raptor_world* world = rdf_parser->world;
  raptor_uri* uri = NULL;
  raptor_term* t1 = NULL;
  raptor_term* t2 = NULL;
  raptor_term* t3 = NULL;
  raptor_statement* s = NULL;
  raptor_term* sentence = NULL;

  sentence = mkr_new_blank_term(rdf_parser);
  t1 = raptor_term_copy(sentence);
  uri = raptor_new_uri_for_rdf_concept(world, (const unsigned char*)"subject");
  t2 = raptor_new_term_from_uri(world, uri);
  raptor_free_uri(uri);
  t3 = raptor_term_copy(triple->subject);
  s = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
  raptor_mkr_generate_statement(rdf_parser, s);
  /* raptor_free_statement(s); */

  t1 = raptor_term_copy(sentence);
  uri = raptor_new_uri_for_rdf_concept(world, (const unsigned char*)"predicate");
  t2 = raptor_new_term_from_uri(world, uri);
  raptor_free_uri(uri);
  t3 = raptor_term_copy(triple->predicate);
  s = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
  raptor_mkr_generate_statement(rdf_parser, s);
  /* raptor_free_statement(s); */

  t1 = raptor_term_copy(sentence);
  uri = raptor_new_uri_for_rdf_concept(world, (const unsigned char*)"object");
  t2 = raptor_new_term_from_uri(world, uri);
  raptor_free_uri(uri);
  t3 = raptor_term_copy(triple->object);
  s = raptor_new_statement_from_nodes(world, t1, t2, t3, NULL);
  raptor_mkr_generate_statement(rdf_parser, s);
  /* raptor_free_statement(s); */

  sentence->mkrtype = MKR_SENTENCE;
  return sentence;
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
int
mkr_parser_error(raptor_parser* rdf_parser, void* scanner, const char *msg)
{
  raptor_turtle_parser* mkr_parser;

  mkr_parser = (raptor_turtle_parser*)rdf_parser->context;
  
  if(mkr_parser->error_count++)
    return 0;

  rdf_parser->locator.line = mkr_parser->lineno;
#ifdef RAPTOR_TURTLE_USE_ERROR_COLUMNS
  rdf_parser->locator.column = turtle_lexer_get_column(yyscanner);
#endif

  raptor_log_error(rdf_parser->world, RAPTOR_LOG_LEVEL_ERROR,
                   &rdf_parser->locator, msg);

  return 0;
}


int
mkr_syntax_error(raptor_parser *rdf_parser, const char *message, ...)
{
  raptor_turtle_parser* mkr_parser = (raptor_turtle_parser*)rdf_parser->context;
  va_list arguments;

  if(!mkr_parser)
    return 1;

  if(mkr_parser->error_count++)
    return 0;

  rdf_parser->locator.line = mkr_parser->lineno;
#ifdef RAPTOR_TURTLE_USE_ERROR_COLUMNS
  rdf_parser->locator.column = turtle_lexer_get_column(yyscanner);
#endif

  va_start(arguments, message);
  
  raptor_parser_log_error_varargs(((raptor_parser*)rdf_parser),
                                  RAPTOR_LOG_LEVEL_ERROR, message, arguments);

  va_end(arguments);

  return 0;
}



#ifndef TURTLE_PUSH_PARSE
static int
mkr_parse(raptor_parser *rdf_parser, const char *string, size_t length)
{
  raptor_turtle_parser* mkr_parser = (raptor_turtle_parser*)rdf_parser->context;
  int rc;
  
  if(!string || !*string)
    return 0;
  
  if(turtle_lexer_lex_init(&mkr_parser->scanner))
    return 1;
  mkr_parser->scanner_set = 1;

#if defined(YYDEBUG) && YYDEBUG > 0
  turtle_lexer_set_debug(1 ,&mkr_parser->scanner);
  mkr_parser_debug = 1;
#endif

  turtle_lexer_set_extra(rdf_parser, mkr_parser->scanner);
  (void)turtle_lexer__scan_bytes((char *)string, (int)length, mkr_parser->scanner);

  rc = mkr_parser_parse(rdf_parser, mkr_parser->scanner);

  turtle_lexer_lex_destroy(mkr_parser->scanner);
  mkr_parser->scanner_set = 0;

  return rc;
}
#endif


#ifdef TURTLE_PUSH_PARSE
static int
mkr_push_parse(raptor_parser *rdf_parser, 
                  const char *string, size_t length)
{
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  raptor_world* world = rdf_parser->world;
#endif
  raptor_turtle_parser* mkr_parser;
  void *buffer;
  int status;
  yypstate *ps;

  mkr_parser = (raptor_turtle_parser*)rdf_parser->context;

  if(!string || !*string)
    return 0;
  
  if(turtle_lexer_lex_init(&mkr_parser->scanner))
    return 1;
  mkr_parser->scanner_set = 1;

#if defined(YYDEBUG) && YYDEBUG > 0
  turtle_lexer_set_debug(1 ,&mkr_parser->scanner);
  mkr_parser_debug = 1;
#endif

  turtle_lexer_set_extra(rdf_parser, mkr_parser->scanner);
  buffer = turtle_lexer__scan_bytes(string, length, mkr_parser->scanner);

  /* returns a parser instance or 0 on out of memory */
  ps = yypstate_new();
  if(!ps)
    return 1;

  do {
    YYSTYPE lval;
    int token;

    memset(&lval, 0, sizeof(YYSTYPE));
    
    token = turtle_lexer_lex(&lval, mkr_parser->scanner);

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    printf("token %s\n", turtle_token_print(world, token, &lval));
#endif

    status = yypush_parse(ps, token, &lval, rdf_parser, mkr_parser->scanner);

    /* turtle_token_free(world, token, &lval); */

    if(!token || token == EOF || token == ERROR_TOKEN)
      break;
  } while (status == YYPUSH_MORE);
  yypstate_delete(ps);

  turtle_lexer_lex_destroy(mkr_parser->scanner);
  mkr_parser->scanner_set = 0;

  return 0;
}
#endif


/**
 * raptor_mkr_parse_init - Initialise the Raptor mKR parser
 *
 * Return value: non 0 on failure
 **/

static int
raptor_mkr_parse_init(raptor_parser* rdf_parser, const char *name)
{
  raptor_turtle_parser* mkr_parser = (raptor_turtle_parser*)rdf_parser->context;

  if(raptor_namespaces_init(rdf_parser->world, &mkr_parser->namespaces, 0))
    return 1;

  mkr_parser->mkr = 1;
  mkr_parser->groupType = NULL;
  mkr_parser->groupName = NULL;
  mkr_parser->groupList = NULL;

  return 0;
}


/* PUBLIC FUNCTIONS */


/*
 * raptor_mkr_parse_terminate - Free the Raptor mKR parser
 * @rdf_parser: parser object
 * 
 **/
static void
raptor_mkr_parse_terminate(raptor_parser *rdf_parser) {
  raptor_turtle_parser *mkr_parser = (raptor_turtle_parser*)rdf_parser->context;

  raptor_namespaces_clear(&mkr_parser->namespaces);

  if(mkr_parser->scanner_set) {
    turtle_lexer_lex_destroy(mkr_parser->scanner);
    mkr_parser->scanner_set = 0;
  }

  if(mkr_parser->buffer)
    RAPTOR_FREE(cdata, mkr_parser->buffer);

  if(mkr_parser->graph_name) {
    raptor_free_term(mkr_parser->graph_name);
    mkr_parser->graph_name = NULL;
  }
}

/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

/****** NEW code from Turtle parser *****/


void
raptor_mkr_generate_statement(raptor_parser *parser, raptor_statement *t)
{
  raptor_mkr_clone_statement(parser, t);
  raptor_mkr_handle_statement(parser, &parser->statement);
  /* clear resources */
  raptor_statement_clear(&parser->statement);
}

void
raptor_mkr_handle_statement(raptor_parser *parser, raptor_statement *t)
{
  if(!t->subject || !t->predicate || !t->object)
    return;

  if(!parser->statement_handler)
    return;

  /* Generate the statement */
  (*parser->statement_handler)(parser->user_data, t);
}

void
raptor_mkr_defer_statement(raptor_parser *parser, raptor_statement *t)
{
  raptor_statement* st;
  raptor_turtle_parser* mkr_parser;

#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: raptor_mkr_defer_statement: ");
#endif

  raptor_mkr_clone_statement(parser, t);
  st = raptor_new_statement(parser->world);
  if(!st) {
    return;
  }
  /* copy static to dynamic statement, it's a move really */
  st->subject = parser->statement.subject, parser->statement.subject = NULL;
  st->predicate = parser->statement.predicate, parser->statement.predicate = NULL;
  st->object = parser->statement.object, parser->statement.object = NULL;
  st->graph = parser->statement.graph, parser->statement.graph = NULL;

  /* prep deferred list */
  mkr_parser = (raptor_turtle_parser*)parser->context;
  if(!mkr_parser->deferred) {
    mkr_parser->deferred = raptor_new_sequence((raptor_data_free_handler)raptor_free_statement, NULL);
    if(!mkr_parser->deferred) {
      goto free_seq;
    }
  }
  /* append to deferred list */
  if(raptor_sequence_push(mkr_parser->deferred, st)) {
  free_seq:
    raptor_free_statement(st);
  }
#if defined(RAPTOR_DEBUG)
  printf("##### DEBUG: exit raptor_mkr_defer_statement:\n");
#endif
}



/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/

/* code from old raptor_mkr_generate_statement() */

void
raptor_mkr_clone_statement(raptor_parser *parser, raptor_statement *t)
{
  raptor_turtle_parser *mkr_parser = (raptor_turtle_parser*)parser->context;
  raptor_statement *statement = &parser->statement;

  if(!t) {
    printf("##### ERROR: raptor_mkr_clone_statement: ignore NULL statement\n");
    return;
  }
  if(!t->subject) {
    printf("##### WARNING: raptor_mkr_clone_statement: ignore NULL subject\n");
    return;
  }
  if(!t->predicate) {
    printf("##### WARNING: raptor_mkr_clone_statement: ignore NULL predicate\n");
    return;
  }
  if(!t->object) {
    printf("##### WARNING: raptor_mkr_clone_statement: ignore NULL object\n");
    return;
  }

  if(!t->subject || !t->predicate || !t->object) {
    return;
  }

  if(!parser->statement_handler) {
    return;
  }

  if(mkr_parser->trig && mkr_parser->graph_name) {
    statement->graph = raptor_term_copy(mkr_parser->graph_name);
  }

  if(!parser->emitted_default_graph && !mkr_parser->graph_name) {
    /* for non-TRIG - start default graph at first triple */
    raptor_parser_start_graph(parser, NULL, 0);
    parser->emitted_default_graph++;
  }
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: raptor_mkr_clone_statement: ");
  mkr_statement_print(t, stdout);
  printf("\n");
#endif

  /* Two choices for subject for Turtle */
  switch(t->subject->type) {
    case RAPTOR_TERM_TYPE_BLANK:
      statement->subject = raptor_new_term_from_blank(parser->world,
                                                    t->subject->value.blank.string);
      statement->subject->mkrtype = t->subject->mkrtype;
      break;

    case RAPTOR_TERM_TYPE_URI:
      RAPTOR_ASSERT((t->subject->type != RAPTOR_TERM_TYPE_URI),
                  "subject type is not resource");
      statement->subject = raptor_new_term_from_uri(parser->world,
                                                  t->subject->value.uri);
      statement->subject->mkrtype = t->subject->mkrtype;
      break;

    case RAPTOR_TERM_TYPE_LITERAL:
    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
/* #if defined(RAPTOR_DEBUG) */
      printf("##### ERROR: raptor_mkr_clone_statement: unexpected default: subject->type = %s\n", RAPTOR_TERM_TYPE_NAME(t->subject->type));
/* #endif */
      break;
  }

  /* Predicates are URIs but check for bad ordinals */
  switch(t->predicate->type) {

    case RAPTOR_TERM_TYPE_URI:
      if(!strncmp((const char*)raptor_uri_as_string(t->predicate->value.uri),
                  "http://www.w3.org/1999/02/22-rdf-syntax-ns#_", 44)) {
        unsigned char* predicate_uri_string = raptor_uri_as_string(t->predicate->value.uri);
        int predicate_ordinal = raptor_check_ordinal(predicate_uri_string+44);
        if(predicate_ordinal <= 0)
          raptor_parser_error(parser, "Illegal ordinal value %d in property '%s'.", predicate_ordinal, predicate_uri_string);
      }
      statement->predicate = raptor_new_term_from_uri(parser->world,
                                                  t->predicate->value.uri);
      statement->predicate->mkrtype = t->predicate->mkrtype;
      break;

    case RAPTOR_TERM_TYPE_BLANK:
      if((t->predicate->mkrtype == MKR_DEFINITION) ||
         (t->predicate->mkrtype == MKR_ACTION) ||
         (t->predicate->mkrtype == MKR_COMMAND)
         ) {
        statement->predicate = raptor_new_term_from_blank(parser->world,
                                                    t->predicate->value.blank.string);
        statement->predicate->mkrtype = t->predicate->mkrtype;
      } else {
        printf("##### WARNING: unexpected blank predicate->mkrtype: %s\n", MKR_TYPE_NAME(t->predicate->mkrtype));
      }
      break;

    case RAPTOR_TERM_TYPE_LITERAL:
    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
/* #if defined(RAPTOR_DEBUG) */
      printf("##### ERROR: raptor_mkr_clone_statement: unexpected default: predicate->type = %s\n", RAPTOR_TERM_TYPE_NAME(t->predicate->type));
/* #endif */
      break;
  }
  

  /* Three choices for object for Turtle */
  switch(t->object->type) {
    case RAPTOR_TERM_TYPE_URI:
      statement->object = raptor_new_term_from_uri(parser->world,
                                                 t->object->value.uri);
      statement->object->mkrtype = t->object->mkrtype;
      break;

    case RAPTOR_TERM_TYPE_BLANK:
      statement->object = raptor_new_term_from_blank(parser->world,
                                                   t->object->value.blank.string);
      statement->object->mkrtype = t->object->mkrtype;
      break;

    case RAPTOR_TERM_TYPE_LITERAL:
      RAPTOR_ASSERT(t->object->type != RAPTOR_TERM_TYPE_LITERAL,
                  "object type is not literal");
      statement->object = raptor_new_term_from_literal(parser->world,
                                                     t->object->value.literal.string,
                                                     t->object->value.literal.datatype,
                                                     t->object->value.literal.language);
      statement->object->mkrtype = t->object->mkrtype;
      break;

    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
/* #if defined(RAPTOR_DEBUG) */
      printf("##### ERROR: raptor_mkr_clone_statement: unexpected default: object->type = %s\n", RAPTOR_TERM_TYPE_NAME(t->object->type));
/* #endif */
      break;
  }

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: exit raptor_mkr_clone_statement\n");
#endif
}


/*******************************************************************************/
/*******************************************************************************/
/*******************************************************************************/



static int
raptor_mkr_parse_chunk(raptor_parser* rdf_parser, 
                          const unsigned char *s, size_t len,
                          int is_end)
{
  char *ptr;
  raptor_turtle_parser *mkr_parser;
  int rc;

  mkr_parser = (raptor_turtle_parser*)rdf_parser->context;
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  RAPTOR_DEBUG2("adding %d bytes to line buffer\n", (int)len);
#endif

  if(len) {
    mkr_parser->buffer = RAPTOR_REALLOC(char*, mkr_parser->buffer,
                                           mkr_parser->buffer_length + len + 1);
    if(!mkr_parser->buffer) {
      raptor_parser_fatal_error(rdf_parser, "Out of memory");
      return 1;
    }

    /* move pointer to end of cdata buffer */
    ptr = mkr_parser->buffer+mkr_parser->buffer_length;

    /* adjust stored length */
    mkr_parser->buffer_length += len;

    /* now write new stuff at end of cdata buffer */
    memcpy(ptr, s, len);
    ptr += len;
    *ptr = '\0';

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
    RAPTOR_DEBUG3("buffer buffer now '%s' (%ld bytes)\n", 
                  mkr_parser->buffer, turtle_parser->buffer_length);
#endif
  }
  
  /* if not end, wait for rest of input */
  if(!is_end)
    return 0;

  /* Nothing to do */
  if(!mkr_parser->buffer_length)
    return 0;

#ifdef TURTLE_PUSH_PARSE
  rc = turtle_push_parse(rdf_parser, 
			 mkr_parser->buffer, mkr_parser->buffer_length);
#else
  rc = mkr_parse(rdf_parser, mkr_parser->buffer, mkr_parser->buffer_length);
#endif  

  if(rdf_parser->emitted_default_graph) {
    /* for non-TRIG - end default graph after last triple */
    raptor_parser_end_graph(rdf_parser, NULL, 0);
    rdf_parser->emitted_default_graph--;
  }
  return rc;
}


static int
raptor_mkr_parse_start(raptor_parser *rdf_parser) 
{
  raptor_locator *locator=&rdf_parser->locator;
  raptor_turtle_parser *mkr_parser = (raptor_turtle_parser*)rdf_parser->context;

  /* base URI required for mKR */
  if(!rdf_parser->base_uri)
    return 1;

  locator->line = 1;
  locator->column= -1; /* No column info */
  locator->byte= -1; /* No bytes info */

  if(mkr_parser->buffer_length) {
    RAPTOR_FREE(cdata, mkr_parser->buffer);
    mkr_parser->buffer = NULL;
    mkr_parser->buffer_length = 0;
  }
  
  mkr_parser->lineno = 1;

  return 0;
}


static int
raptor_mkr_parse_recognise_syntax(raptor_parser_factory* factory, 
                                     const unsigned char *buffer, size_t len,
                                     const unsigned char *identifier, 
                                     const unsigned char *suffix, 
                                     const char *mime_type)
{
  int score= 0;
  
  if(suffix) {
    if(!strcmp((const char*)suffix, "mkr"))
      score = 8;
    if(!strcmp((const char*)suffix, "ho"))
      score = 3;
    if(!strcmp((const char*)suffix, "rel"))
      score = 3;
  }
  
  if(mime_type) {
    if(strstr((const char*)mime_type, "mkr"))
      score += 6;
    if(strstr((const char*)mime_type, "ho"))
      score += 3;
    if(strstr((const char*)mime_type, "rel"))
      score += 3;
  }

  /* Do this as long as N3 is not supported since it shares the same syntax */
  if(buffer && len) {
#define  HAS_TURTLE_PREFIX (raptor_memstr((const char*)buffer, len, "@prefix ") != NULL)
/* The following could also be found with N-Triples but not with @prefix */
#define  HAS_MKR_RDF_URI (raptor_memstr((const char*)buffer, len, ": <http://mkrmke.net/ns/>") != NULL)

    if(HAS_TURTLE_PREFIX) {
      score = 6;
      if(HAS_MKR_RDF_URI)
        score += 2;
    }
  }
  
  return score;
}


raptor_uri*
raptor_mkr_get_graph(raptor_parser* rdf_parser)
{
  raptor_turtle_parser *mkr_parser;

  mkr_parser = (raptor_turtle_parser*)rdf_parser->context;
  if(mkr_parser->graph_name)
    return raptor_uri_copy(mkr_parser->graph_name->value.uri);

  return NULL;
}



#ifdef RAPTOR_PARSER_MKR
static const char* const mkr_names[4] = { "mkr", "ho", "rel", NULL };

static const char* const mkr_uri_strings[7] = {
  "http://mkrmke.net/ns/",
  "http://mkrmke.net/ns/unit/",
  "http://mkrmke.net/ns/existent/",
  "http://mkrmke.net/ns/space/",
  "http://mkrmke.net/ns/time/",
  "http://mkrmke.net/ns/view/",
  NULL
};
  
#define MKR_TYPES_COUNT 3
static const raptor_type_q mkr_types[MKR_TYPES_COUNT + 1] = {
  /* first one is the default */
  { "text/mkr", 8, 10},
  { "text/ho", 7, 10},
  { "text/rel", 8, 10},
  { NULL, 0}
};

static int
raptor_mkr_parser_register_factory(raptor_parser_factory *factory) 
{
  int rc = 0;

  factory->desc.names = mkr_names;

  factory->desc.mime_types = mkr_types;

  factory->desc.label = "mKR my Knowledge Representation Language";
  factory->desc.uri_strings = mkr_uri_strings;

  factory->desc.flags = RAPTOR_SYNTAX_NEED_BASE_URI;
  
  factory->context_length     = sizeof(raptor_turtle_parser);
  
  factory->init      = raptor_mkr_parse_init;
  factory->terminate = raptor_mkr_parse_terminate;
  factory->start     = raptor_mkr_parse_start;
  factory->chunk     = raptor_mkr_parse_chunk;
  factory->recognise_syntax = raptor_mkr_parse_recognise_syntax;
  factory->get_graph = raptor_mkr_get_graph;

  return rc;
}
#endif



#ifdef RAPTOR_PARSER_MKR
int
raptor_init_parser_mkr(raptor_world* world)
{
  return !raptor_world_register_parser_factory(world,
                                               &raptor_mkr_parser_register_factory);
}
#endif


#ifdef STANDALONE
#include <stdio.h>
#include <locale.h>

#define TURTLE_FILE_BUF_SIZE 2048

static void
mkr_parser_print_statement(void *user,
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
  raptor_turtle_parser mkr_parser; /* static */
  raptor_locator *locator = &rdf_parser.locator;
  FILE *fh;
  const char *filename;
  size_t nobj;
  
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 2
  mkr_parser_debug = 1;
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
  memset(&mkr_parser, 0, sizeof(mkr_parser));

  locator->line= locator->column = -1;
  locator->file= filename;

  mkr_parser.lineno= 1;

  rdf_parser.world = raptor_new_world();
  rdf_parser.context = &mkr_parser;
  rdf_parser.base_uri = raptor_new_uri(rdf_parser.world,
                                       (const unsigned char*)"http://example.org/fake-base-uri/");

  raptor_parser_set_statement_handler(&mkr_parser, stdout, mkr_parser_print_statement);
  raptor_mkr_parse_init(&rdf_parser, "mkr");
  
  mkr_parser.error_count = 0;

#ifdef TURTLE_PUSH_PARSE
  turtle_push_parse(&rdf_parser, string, strlen(string));
#else
  mkr_parse(&rdf_parser, string, strlen(string));
#endif

  raptor_mkr_parse_terminate(&rdf_parser);
  
  raptor_free_uri(rdf_parser.base_uri);

  raptor_free_world(rdf_parser.world);
  
  return (0);
}
#endif
