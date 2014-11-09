/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * turtle_lexer.l - Raptor Turtle lexer - making tokens for turtle grammar generator
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
 * To generate the C files from this source, rather than use the
 * shipped turtle_lexer.c/.h needs a patched version of flex 2.5.31 such
 * as the one available in Debian GNU/Linux.   Details below
 * near the %option descriptions.
 *
 */


/* recognise 8-bits */
%option 8bit
%option warn nodefault

/* all symbols prefixed by this */
%option prefix="turtle_lexer_"

/* This is not needed, flex is invoked -oturtle_lexer.c */
/* %option outfile="turtle_lexer.c" */

/* Emit a C header file for prototypes
 * Only available in flex 2.5.13 or newer.
 * It was renamed to header-file in flex 2.5.19
 */
%option header-file="turtle_lexer.h"

/* Do not emit #include <unistd.h>
 * Only available in flex 2.5.7 or newer.
 * Broken in flex 2.5.31 without patches.
 */
%option nounistd

/* Never interactive */
/*  No isatty() check */
%option never-interactive

/* Batch scanner */
%option batch

/* Never use yyunput */
%option nounput

/* Supply our own alloc/realloc/free functions */
%option noyyalloc noyyrealloc noyyfree

/* Re-entrant scanner */
%option reentrant

%option extra-type="raptor_parser*"

/* Makes yyget_lval() yyset_lval() and yylval appear */
%option bison-bridge
/* Makes yyget_lloc() yyset_lloc() and yylloc appear */
/* %option bison-locations */

  /* definitions */

%{

/* NOTE: These headers are NOT included here but are inserted by
 * fix-flex since otherwise it appears far too late in the generated C
 */

/*
#ifdef HAVE_CONFIG_H
#include <raptor_config.h>
#endif
*/

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
#ifdef HAVE_SETJMP_H
#include <setjmp.h>
#endif

#include "raptor2.h"
#include "raptor_internal.h"

#include <turtle_parser.h>
#include <turtle_common.h>


/* Prototypes */ 
static unsigned char *turtle_copy_token(unsigned char *text, size_t len);
static unsigned char *turtle_copy_string_token(raptor_parser* rdf_parser, unsigned char *text, size_t len, int delim);
void turtle_lexer_syntax_error(void* ctx, const char *message, ...) RAPTOR_PRINTF_FORMAT(2, 3);

#ifdef RAPTOR_DEBUG
const char * turtle_token_print(raptor_world* world, int token, YYSTYPE *lval);
#endif

#ifdef __cplusplus
#define INPUT_FN yyinput
#else
#define INPUT_FN input
#endif


#if FLEX_VERSION_DECIMAL < 20536
/* debian flex 2.5.35-10.1 added these column header prototypes in
 * re-entrant mode.  standard flex omits them
 */
void turtle_lexer_set_column(int column_no, yyscan_t yyscanner);
int turtle_lexer_get_column(yyscan_t yyscanner);
#endif

static void turtle_lexer_cleanup(yyscan_t yyscanner);

#ifdef HAVE_SETJMP
static jmp_buf turtle_lexer_fatal_error_longjmp_env;

/* fatal error handler declaration */
#define YY_FATAL_ERROR(msg) do {                                  \
    turtle_lexer_error(yyscanner, RAPTOR_LOG_LEVEL_FATAL, msg);   \
    longjmp(turtle_lexer_fatal_error_longjmp_env, 1);             \
    abort();                                                      \
} while(0)
#else
#define YY_FATAL_ERROR(msg) do {                                   \
    turtle_lexer_error(yyscanner, RAPTOR_LOG_LEVEL_FATAL, msg);    \
    abort();                                                       \
} while(0)
#endif
 
/* Remove the re-fill function since it should never be called */
#define YY_INPUT(buf,result,max_size) { return YY_NULL; }

static void turtle_lexer_error(yyscan_t yyscanner, raptor_log_level level, yyconst char *message, ...) RAPTOR_PRINTF_FORMAT(3, 4);

/* Fatal error handler that returns EOF instead of abort()/longjmp()
 * so that parser can clean up properly */
#define YY_FATAL_ERROR_EOF(msg) do { \
    turtle_lexer_error(yyscanner, RAPTOR_LOG_LEVEL_FATAL, "%s", msg);   \
    yyterminate(); \
} while(0)

/* Out-of-memory reporting macro */
#define TURTLE_LEXER_OOM() YY_FATAL_ERROR_EOF(turtle_lexer_oom_text)
static char turtle_lexer_oom_text[]="turtle_lexer: Out of memory";

/* Do not need input() to to read from stdin */
#define YY_NO_INPUT 1

%}

/* Tokens from Turtle 2013 spec - lex-ifyed to remove unicode ranges */
PN_CHARS_BASE [A-Za-z\x80-\xff]
PN_CHARS {PN_CHARS_BASE}|"_"|"-"|[0-9]
BS_ESCAPES [-_~\.!$&\'()*+,;=/?#@%]
HEX [0-9A-Fa-f]
PLX "%"{HEX}{HEX})|("\\"{BS_ESCAPES}

LANGTAG "@"[A-Za-z][-A-Z_a-z0-9]*

/* flex: only 1 level of definition expansion so have to expand PLX */
BN_LABEL  ({PN_CHARS_BASE}|"_"|[0-9])(({PN_CHARS}|".")*({PN_CHARS}))*
PN_PREFIX ({PN_CHARS_BASE})(({PN_CHARS}|".")*({PN_CHARS}))*
PN_LOCAL ({PN_CHARS_BASE}|"_"|[0-9]|":"|{PLX})(({PN_CHARS}|"."|":"|{PLX})*({PN_CHARS}|":"|{PLX}))*

QNAME {PN_PREFIX}?":"{PN_LOCAL}?

UCHAR "\\u"{HEX}{HEX}{HEX}{HEX}|"\\U"{HEX}{HEX}{HEX}{HEX}{HEX}{HEX}{HEX}{HEX}
IRI "<"([^\x00-\x20<>\"{}\|^`\\]|{UCHAR})*">"

INTEGER [-+]?[0-9]+
DECIMAL [-+]?[0-9]*"."[0-9]+
DOUBLE [-+]?([0-9]+"."[0-9]*{EXPONENT}|"."[0-9]+{EXPONENT}|[0-9]+{EXPONENT})
EXPONENT [eE][+-]?[0-9]+


%x PREF LONG_DLITERAL LONG_SLITERAL


%%
  /* rules */

%{
  raptor_parser *rdf_parser = yyextra;
  raptor_turtle_parser* turtle_parser = (raptor_turtle_parser*)rdf_parser->context;

#ifdef HAVE_SETJMP
  if(setjmp(turtle_lexer_fatal_error_longjmp_env))
    return 1;
#endif
%}
    

\r\n|\r|\n   { turtle_parser->lineno++; }
 
[\ \t\v]+   { /* empty */ }


"a" { return A; }

"."       { return DOT; } 
","       { return COMMA; } 
";"       { return SEMICOLON; }
"["       { return LEFT_SQUARE; }
"]"       { return RIGHT_SQUARE; }
"@prefix" { BEGIN(PREF); return PREFIX; }
[Pp][Rr][Ee][Ff][Ii][Xx] { BEGIN(PREF);
		return SPARQL_PREFIX; }
"@base"   { return BASE; }
[Bb][Aa][Ss][Ee] { return SPARQL_BASE; }
"^^"      { return HAT; }
"("       { return LEFT_ROUND; }
")"       { return RIGHT_ROUND; }
"{"       { return LEFT_CURLY; }
"}"       { return RIGHT_CURLY; }
"true"    { return TRUE_TOKEN; }
"false"   { return FALSE_TOKEN; }


\"([^\"\\\n\r]|\\[^\n\r])*\"   { yylval->string = turtle_copy_string_token(rdf_parser, (unsigned char*)yytext+1, yyleng-2, '"'); /* ' */
                                if(!yylval->string)
                                  yyterminate();

                                return STRING_LITERAL; }

\'([^\'\\\n\r]|\\[^\n\r])*\'   { yylval->string = turtle_copy_string_token(rdf_parser, (unsigned char*)yytext+1, yyleng-2, '"'); /* ' */
                                if(!yylval->string)
                                  yyterminate();

                                return STRING_LITERAL; }

\"\"\"				{ BEGIN(LONG_DLITERAL); 
                                  turtle_parser->sb = raptor_new_stringbuffer();
                                  if(!turtle_parser->sb)
                                    TURTLE_LEXER_OOM();
                          }

<LONG_DLITERAL>\"\"\"			{
		  size_t len;
     
		  BEGIN(INITIAL);
                  len = raptor_stringbuffer_length(turtle_parser->sb);
                  yylval->string = RAPTOR_MALLOC(unsigned char*, len + 1);
                  if(!yylval->string)
                    TURTLE_LEXER_OOM();
                  raptor_stringbuffer_copy_to_string(turtle_parser->sb, (unsigned char*)yylval->string, len);
                  yylval->string[len]='\0';

                  raptor_free_stringbuffer(turtle_parser->sb);
                  turtle_parser->sb = NULL;
                  return STRING_LITERAL; }

<LONG_DLITERAL>\"|(\\.|[^\"\\]|\n)*	{
		  char *p;

		  if(*yytext == EOF) {
                    BEGIN(INITIAL);
                    turtle_syntax_error(rdf_parser, "End of file in middle of literal");
                    raptor_free_stringbuffer(turtle_parser->sb);
                    turtle_parser->sb = NULL;
                    return EOF;
                  }

		  for(p = yytext; *p; p++) {
                    if(*p == '\n')
                      turtle_parser->lineno++;
                  }

                  if(raptor_stringbuffer_append_turtle_string(turtle_parser->sb, (unsigned char*)yytext, yyleng, '"', (raptor_simple_message_handler)turtle_lexer_syntax_error, rdf_parser, 0)) { /* " */
                    BEGIN(INITIAL);
                    raptor_free_stringbuffer(turtle_parser->sb);
                    turtle_parser->sb = NULL;
                    YY_FATAL_ERROR_EOF("raptor_stringbuffer_append_turtle_string failed");
                  }
                  
   }

<LONG_DLITERAL>\\	{
     		/* this should only happen if \ is at the end of the file so the Turtle doc is illegal anyway */
                    BEGIN(INITIAL);
                    raptor_free_stringbuffer(turtle_parser->sb);
                    turtle_parser->sb = NULL;
                    turtle_syntax_error(rdf_parser, "End of file in middle of \"\"\"literal\"\"\"");
                    yyterminate();
}

<LONG_DLITERAL><<EOF>>     {
                    BEGIN(INITIAL);
                    raptor_free_stringbuffer(turtle_parser->sb);
                    turtle_parser->sb = NULL;
                    turtle_syntax_error(rdf_parser, "End of file in middle of \"\"\"literal\"\"\"");
                    yyterminate();
}
    
\'\'\'				{ BEGIN(LONG_SLITERAL); 
                                  turtle_parser->sb = raptor_new_stringbuffer();
                                  if(!turtle_parser->sb)
                                    TURTLE_LEXER_OOM();
                          }

<LONG_SLITERAL>\'\'\'			{
		  size_t len;
     
		  BEGIN(INITIAL);
                  len = raptor_stringbuffer_length(turtle_parser->sb);
                  yylval->string = RAPTOR_MALLOC(unsigned char*, len + 1);
                  if(!yylval->string)
                    TURTLE_LEXER_OOM();
                  raptor_stringbuffer_copy_to_string(turtle_parser->sb, (unsigned char*)yylval->string, len);
                  yylval->string[len]='\0';

                  raptor_free_stringbuffer(turtle_parser->sb);
                  turtle_parser->sb = NULL;
                  return STRING_LITERAL; }

<LONG_SLITERAL>\'|(\\.|[^\'\\]|\n)*	{
		  char *p;

		  if(*yytext == EOF) {
                    BEGIN(INITIAL);
                    turtle_syntax_error(rdf_parser, "End of file in middle of \'\'\'literal\'\'\'");
                    raptor_free_stringbuffer(turtle_parser->sb);
                    turtle_parser->sb = NULL;
                    return EOF;
                  }

		  for(p = yytext; *p; p++) {
                    if(*p == '\n')
                      turtle_parser->lineno++;
                  }

                  if(raptor_stringbuffer_append_turtle_string(turtle_parser->sb, (unsigned char*)yytext, yyleng, '"', (raptor_simple_message_handler)turtle_lexer_syntax_error, rdf_parser, 0)) { /* " */
                    BEGIN(INITIAL);
                    raptor_free_stringbuffer(turtle_parser->sb);
                    turtle_parser->sb = NULL;
                    YY_FATAL_ERROR_EOF("raptor_stringbuffer_append_turtle_string failed");
                  }
                  
   }

<LONG_SLITERAL>\\	{
     		/* this should only happen if \ is at the end of the file so the Turtle doc is illegal anyway */
                    BEGIN(INITIAL);
                    raptor_free_stringbuffer(turtle_parser->sb);
                    turtle_parser->sb = NULL;
                    turtle_syntax_error(rdf_parser, "End of file in middle of '''literal'''");
                    yyterminate();
}

<LONG_SLITERAL><<EOF>>     {
                    BEGIN(INITIAL);
                    raptor_free_stringbuffer(turtle_parser->sb);
                    turtle_parser->sb = NULL;
                    turtle_syntax_error(rdf_parser, "End of file in middle of '''literal'''");
                    yyterminate();
}
    
"_:"{BN_LABEL}	{ yylval->string = turtle_copy_token((unsigned char*)yytext+2, yyleng-2);
          if(!yylval->string)
            YY_FATAL_ERROR_EOF("turtle_copy_token failed");
                          return BLANK_LITERAL; }

{QNAME}	{ yylval->uri = turtle_qname_to_uri(rdf_parser, (unsigned char*)yytext, yyleng);
          if(!yylval->uri) {
            turtle_lexer_error(yyscanner, RAPTOR_LOG_LEVEL_ERROR, "Failed to convert qname %s to URI", yytext);
            yyterminate();
          }

                          return QNAME_LITERAL; }

{DECIMAL}	{ yylval->string = turtle_copy_token((unsigned char*)yytext, yyleng);
                  if(!yylval->string)
                    YY_FATAL_ERROR_EOF("turtle_copy_token failed");
                        return DECIMAL_LITERAL;
}

{DOUBLE} { yylval->string = turtle_copy_token((unsigned char*)yytext, yyleng);
                  if(!yylval->string)
                    YY_FATAL_ERROR_EOF("turtle_copy_token failed");
                        return FLOATING_LITERAL;
}

{INTEGER}        { yylval->string = turtle_copy_token((unsigned char*)yytext, yyleng);
                  if(!yylval->string)
                    YY_FATAL_ERROR_EOF("turtle_copy_token failed");
                          return INTEGER_LITERAL; }

<PREF>[\ \t\v]+ { /* eat up leading whitespace */ }
<PREF>{PN_PREFIX}":"	{ yylval->string=turtle_copy_token((unsigned char*)yytext, yyleng);
                            if(!yylval->string)
                              YY_FATAL_ERROR_EOF("turtle_copy_token failed");
                          BEGIN(INITIAL);
                          return IDENTIFIER; }
<PREF>":"	{ BEGIN(INITIAL);
		  yylval->string = turtle_copy_token((unsigned char*)yytext, 0);
      if(!yylval->string)
        YY_FATAL_ERROR_EOF("turtle_copy_token failed");
                  return IDENTIFIER; }

<PREF>(.|\n)	{ BEGIN(INITIAL);
		  if(*yytext == EOF)
                    return EOF;

                  turtle_syntax_error(rdf_parser, "syntax error at '%c'", *yytext);
                  yyterminate();  }


{IRI}[\ \t\v\r\n]*("=")?[\ \t\v\r\n]*"{"   {
                  raptor_stringbuffer* sb;
                  unsigned char* uri_string;

                  /* make length just the IRI */
                  while(yytext[yyleng - 1] != '>')
                    yyleng--;

                  sb = raptor_new_stringbuffer();
                  if(!sb)
                    TURTLE_LEXER_OOM();

                  /* start at yytext + 1 to skip '<' and operate over
                   * length-2 bytes to skip '<' and '>'
                   */
                  if(raptor_stringbuffer_append_turtle_string(sb, (unsigned char*)yytext+1, yyleng-2, '>', (raptor_simple_message_handler)turtle_lexer_syntax_error, rdf_parser, 1)) {
                    raptor_free_stringbuffer(sb);
                    YY_FATAL_ERROR_EOF("raptor_stringbuffer_append_turtle_string failed");
                  }
                  uri_string = raptor_stringbuffer_as_string(sb);

                  if(!*uri_string)
                    yylval->uri = raptor_uri_copy(rdf_parser->base_uri);
                  else
                    yylval->uri = raptor_new_uri_relative_to_base(rdf_parser->world, rdf_parser->base_uri, uri_string);

                  raptor_free_stringbuffer(sb);

                  if(!yylval->uri)
                    TURTLE_LEXER_OOM();
                return GRAPH_NAME_LEFT_CURLY; }

{QNAME}[\ \t\v\r\n]*("=")?[\ \t\v\r\n]*"{"   {
               while(1) {
                  int c = yytext[yyleng - 1];
                  if(c == '{' || c == ' ' || c=='\t' || c == '\v' || c == '\n' ||
                     c == '=') {
                    yyleng--;
                  } else
                    break;
                }
                yytext[yyleng] = '\0';

                yylval->uri = turtle_qname_to_uri(rdf_parser, (unsigned char*)yytext, yyleng);
                if(!yylval->uri) {
                  turtle_lexer_error(yyscanner, RAPTOR_LOG_LEVEL_ERROR, "Failed to convert qname %s to URI", yytext);
                  yyterminate();
                }

                return GRAPH_NAME_LEFT_CURLY; }

{IRI}   { if(yyleng == 2) 
                  yylval->uri = raptor_uri_copy(rdf_parser->base_uri);
                else {
                  raptor_stringbuffer* sb;
                  unsigned char* uri_string;

                  yytext[yyleng-1]='\0';
                  sb = raptor_new_stringbuffer();
                  if(!sb)
                    TURTLE_LEXER_OOM();
                  if(raptor_stringbuffer_append_turtle_string(sb, (unsigned char*)yytext+1, yyleng-1, '>', (raptor_simple_message_handler)turtle_lexer_syntax_error, rdf_parser, 1)) {
                    raptor_free_stringbuffer(sb);
                    YY_FATAL_ERROR_EOF("raptor_stringbuffer_append_turtle_string failed");
                  }
                  uri_string = raptor_stringbuffer_as_string(sb);
                  yylval->uri = raptor_new_uri_relative_to_base(rdf_parser->world, rdf_parser->base_uri, uri_string);
                  if(!yylval->uri) {
                    raptor_free_stringbuffer(sb);
                    TURTLE_LEXER_OOM();
                  }
                  raptor_free_stringbuffer(sb);
                }
                return URI_LITERAL; }

{LANGTAG}	{ yylval->string = turtle_copy_token((unsigned char*)yytext+1, yyleng-1);
                  if(!yylval->string)
                    YY_FATAL_ERROR_EOF("turtle_copy_token failed");
                          return LANGTAG; }

\#[^\r\n]*(\r\n|\r|\n)	{ /* # comment */
		turtle_parser->lineno++;
                }

\#[^\r\n]*	{ /* # comment on the last line with no terminating newline */
                }

.         	{ if(*yytext == EOF)
                    return EOF;

                  turtle_syntax_error(rdf_parser, "syntax error at '%c'", *yytext);
                  yyterminate();
		}

%%
  /* user code */

int
yywrap (yyscan_t yyscanner) {
  return 1;
}


static unsigned char *
turtle_copy_token(unsigned char *text, size_t len)
{
  unsigned char *s;
  if(!len)
    len = strlen((const char*)text);
  s = RAPTOR_MALLOC(unsigned char*, len + 1);
  if(s) {
    memcpy(s, text, len);
    s[len] = '\0';
  }
  return s;
}


static unsigned char *
turtle_copy_string_token(raptor_parser* rdf_parser, 
                         unsigned char *string, size_t len, int delim)
{
  raptor_stringbuffer* sb = NULL;
  int rc;
  
  if(len) {
    sb = raptor_new_stringbuffer();
    if(!sb)
      return NULL;
    
    rc = raptor_stringbuffer_append_turtle_string(sb, string, len, delim,
                                                  (raptor_simple_message_handler)turtle_lexer_syntax_error,
                                                  rdf_parser, 0);
    if(rc) {
      raptor_free_stringbuffer(sb);
      return NULL;
    }

    len = raptor_stringbuffer_length(sb);
  }
  
  string = RAPTOR_MALLOC(unsigned char*, len + 1);
  if(string) {
    if(sb) 
      raptor_stringbuffer_copy_to_string(sb, string, len+1);
    string[len]='\0';
  }

  if(sb)
    raptor_free_stringbuffer(sb);
  
  return string;
}


void
turtle_lexer_syntax_error(void* ctx, const char *message, ...)
{
  raptor_parser* rdf_parser = (raptor_parser *)ctx;
  raptor_turtle_parser* turtle_parser = (raptor_turtle_parser*)rdf_parser->context;
  va_list arguments;
  
  rdf_parser->locator.line = turtle_parser->lineno;
#ifdef RAPTOR_TURTLE_USE_ERROR_COLUMNS
  rdf_parser->locator.column = turtle_lexer_get_column(yyscanner);
#endif

  va_start(arguments, message);
  raptor_parser_log_error_varargs(((raptor_parser*)rdf_parser),
                                  RAPTOR_LOG_LEVEL_ERROR, message, arguments);

  va_end(arguments);
}


/*
 * turtle_lexer_error:
 * @yyscanner: scanner object
 * @level: log level RAPTOR_LOG_LEVEL_FATAL otherwise error
 * @message: erro message
 *
 * INTERNAL - replacement for the generated error handler.
 */
static void turtle_lexer_error(yyscan_t yyscanner,
                               raptor_log_level level,
                               yyconst char *message, ...)
{
  raptor_parser *rdf_parser = NULL;
  va_list arguments;

  va_start(arguments, message);

  if(yyscanner)
    rdf_parser = (raptor_parser*)turtle_lexer_get_extra(yyscanner);

  /* This handles NULL rdf_parser properly */
  raptor_parser_log_error_varargs(rdf_parser, level, message, arguments);

  va_end(arguments);
}


/* Define LEXER_ALLOC_TRACKING to enable allocated memory tracking
 * - fixes lexer memory leak when ensure_buffer_stack fails
 */

#ifdef LEXER_ALLOC_TRACKING
typedef struct {
  /* Number of void* slots allocated */
  int lexer_allocs_size;
  /* Allocted void* slots follow in memory after this header */
} lexer_alloc_tracker_header;

/* Initial alloc tracker slot array size - 2 seems to be enough for almost all cases */
static const int initial_lexer_allocs_size = 2;
#endif

/*
 * turtle_lexer_cleanup:
 * @yyscanner:
 *
 * INTERNAL - Clean up unfreed lexer allocs if LEXER_ALLOC_TRACKING is enabled.
 */
static void turtle_lexer_cleanup(yyscan_t yyscanner)
{
#ifdef LEXER_ALLOC_TRACKING
  raptor_parser *rdf_parser;
  lexer_alloc_tracker_header *tracker;
  void **lexer_allocs;
  int i;

  if(!yyscanner)
    return;

  rdf_parser = (raptor_parser *)turtle_lexer_get_extra(yyscanner);
  if(!rdf_parser)
    return;

  tracker = (lexer_alloc_tracker_header *)rdf_parser->lexer_user_data;
  if(!tracker)
    return;
  lexer_allocs = (void**)&tracker[1];

  for(i = 0; i < tracker->lexer_allocs_size; ++i) {
    if(lexer_allocs[i])
      free(lexer_allocs[i]);
    lexer_allocs[i] = NULL;
  }
  free(rdf_parser->lexer_user_data);
  rdf_parser->lexer_user_data = NULL;
#endif
}


/*
 * turtle_lexer_alloc:
 * @size
 * @yyscanner
 *
 * INTERNAL - alloc replacement.
 * Tracks allocated cells if LEXER_ALLOC_TRACKING is enabled.
 */
void *turtle_lexer_alloc(yy_size_t size, yyscan_t yyscanner)
{
#ifdef LEXER_ALLOC_TRACKING
  raptor_parser *rdf_parser;
  lexer_alloc_tracker_header *tracker;
  void **lexer_allocs;
  int i;
  void *ptr;

  /* yyscanner not initialized -> probably initializing yyscanner itself
   * -> just malloc without tracking
   */
  if(!yyscanner)
    return malloc(size);

  rdf_parser = (raptor_parser *)turtle_lexer_get_extra(yyscanner);
  if(!rdf_parser)
    YY_FATAL_ERROR("lexer_alloc: yyscanner extra not initialized");

  /* try to allocate tracker if it does not exist */
  tracker = (lexer_alloc_tracker_header *)rdf_parser->lexer_user_data;
  if(!tracker) {
    /* allocate tracker header + array of void* slots */
    tracker = (lexer_alloc_tracker_header*)calloc(1, sizeof(lexer_alloc_tracker_header)+initial_lexer_allocs_size*sizeof(void*));
    if(!tracker)
      YY_FATAL_ERROR("lexer_alloc: cannot allocate tracker");
    tracker->lexer_allocs_size = initial_lexer_allocs_size;
    rdf_parser->lexer_user_data = (void *)tracker;
  }
  lexer_allocs = (void**)&tracker[1];

  /* allocate memory */
  ptr = malloc(size);
  
  /* find a free slot for ptr */
  for(i = 0; i < tracker->lexer_allocs_size; ++i) {
    if(!lexer_allocs[i]) {
      lexer_allocs[i] = ptr;
      break;
    }
  }

  /* no free slots -> grow tracker slot array */
  if(i>=tracker->lexer_allocs_size) {
    int j;
    void **dest;
    tracker = (lexer_alloc_tracker_header*)calloc(1, sizeof(lexer_alloc_tracker_header)+i*2*sizeof(void*));
    if(!tracker) {
      if(ptr)
        free(ptr);
      YY_FATAL_ERROR("lexer_alloc: cannot grow tracker");
    }
    tracker->lexer_allocs_size = i*2;
    
    /* copy data from old tracker */
    dest = (void**)&tracker[1];
    for(j = 0; j < i; ++j) {
      dest[j] = lexer_allocs[j];
    }
    
    /* set new item to first free slot */
    dest[j] = ptr;

    /* free old tracker and replace with new one */
    free(rdf_parser->lexer_user_data);
    rdf_parser->lexer_user_data = tracker;
  }

  return ptr;
#else
  return malloc(size);
#endif
}


/*
 * turtle_lexer_realloc:
 *
 * INTERNAL - realloc replacement
 * Tracks allocated cells if LEXER_ALLOC_TRACKING is enabled.
 */
void *turtle_lexer_realloc(void *ptr, yy_size_t size, yyscan_t yyscanner)
{
#ifdef LEXER_ALLOC_TRACKING
  raptor_parser *rdf_parser;
  lexer_alloc_tracker_header *tracker;
  void **lexer_allocs;
  int i;
  void *newptr;

  if(!yyscanner)
    YY_FATAL_ERROR("lexer_realloc: yyscanner not initialized");

  rdf_parser = (raptor_parser *)turtle_lexer_get_extra(yyscanner);
  if(!rdf_parser)
    YY_FATAL_ERROR("lexer_realloc: yyscanner extra not initialized");

  tracker = (lexer_alloc_tracker_header *)rdf_parser->lexer_user_data;
  if(!tracker)
    YY_FATAL_ERROR("lexer_realloc: no alloc tracker");
  lexer_allocs = (void**)&tracker[1];

  /* find the old slot for ptr */
  for(i = 0; i < tracker->lexer_allocs_size; ++i) {
    if(lexer_allocs[i] == ptr)
      break;
  }

  /* no old slot -> error */  
  if(i>=tracker->lexer_allocs_size)
    YY_FATAL_ERROR("lexer_realloc: cell not in tracker");

  /* realloc */
  newptr = realloc((char*)ptr, size);

  /* replace entry in tracker */
  lexer_allocs[i] = newptr;

  return newptr;
#else
  return realloc((char*)ptr, size);
#endif
}


/*
 * turtle_lexer_free:
 *
 * INTERNAL - free replacement.
 * Checks for NULL pointer to be freed unlike the default lexer free function.
 * Tracks allocated cells if LEXER_ALLOC_TRACKING is enabled.
 */
void turtle_lexer_free(void *ptr, yyscan_t yyscanner)
{
#ifdef LEXER_ALLOC_TRACKING
  raptor_parser *rdf_parser;
  lexer_alloc_tracker_header *tracker;
  void **lexer_allocs;
  int i;

  /* do not free NULL */
  if(!ptr)
    return;

  /* free ptr even if we would encounter an error */
  free(ptr);

  /* yyscanner is allocated with turtle_lexer_alloc() but it's never stored in the tracker
   * - we need yyscanner to access the tracker */
  if(!yyscanner || ptr == yyscanner)
    return;

  rdf_parser = (raptor_parser *)turtle_lexer_get_extra(yyscanner);
  if(!rdf_parser)
    return;

  tracker = (lexer_alloc_tracker_header *)rdf_parser->lexer_user_data;
  if(!tracker)
    return;
  lexer_allocs = (void**)&tracker[1];

  /* find the slot for ptr */
  for(i = 0; i < tracker->lexer_allocs_size; ++i) {
    if(lexer_allocs[i] == ptr)
      break;
  }

  /* no slot -> error */  
  if(i>=tracker->lexer_allocs_size)
    YY_FATAL_ERROR("lexer_free: cell not in tracker");

  /* remove entry from tracker */
  lexer_allocs[i] = NULL;
#else
  if(ptr)
    free(ptr);
#endif
}


#ifdef RAPTOR_DEBUG

const char *
turtle_token_print(raptor_world* world, int token, YYSTYPE *lval)
{
  static char buffer[2048];

  if(!token)
    return "<<EOF>>";
  
  switch(token) {
    case PREFIX:
      return "PREFIX";

    case BASE:
      return "BASE";

    case A:
      return "A";

    case DOT:
      return "DOT";

    case COMMA:
      return "COMMA";

    case SEMICOLON:
      return "SEMICOLON";

    case LEFT_SQUARE:
      return "LEFT_SQUARE";

    case RIGHT_SQUARE:
      return "RIGHT_SQUARE";

    case HAT:
      return "HAT";

    case STRING_LITERAL:
      sprintf(buffer, "STRING_LITERAL(%s)", lval->string);
      return buffer;

    case URI_LITERAL:
      sprintf(buffer, "URI_LITERAL(%s)", 
              (lval->uri ? (char*)raptor_uri_as_string(lval->uri) : ""));
      return buffer;

    case BLANK_LITERAL:
      sprintf(buffer, "BLANK_LITERAL(%s)", lval->string);
      return buffer;

    case QNAME_LITERAL:
      sprintf(buffer, "QNAME_LITERAL(%s)", 
              (lval->uri ? (char*)raptor_uri_as_string(lval->uri) : ""));
      return buffer;

    case INTEGER_LITERAL:
      sprintf(buffer, "INTEGER_LITERAL(%s)", lval->string);
      return buffer;

    case FLOATING_LITERAL:
      sprintf(buffer, "FLOATING_LITERAL(%s)", lval->string);
      return buffer;

    case IDENTIFIER:
      sprintf(buffer, "IDENTIFIER(%s)", 
              (lval->string ? (char*)lval->string : ""));
      return buffer;

    case LANGTAG:
      sprintf(buffer, "LANGTAG(%s)", 
              (lval->string ? (char*)lval->string : ""));
      return buffer;

    case DECIMAL_LITERAL:
      sprintf(buffer, "DECIMAL_LITERAL(%s)", lval->string);
      return buffer;

    case ERROR_TOKEN:
      return "ERROR";

    case LEFT_CURLY:
      return "{";
    
    case RIGHT_CURLY:
      return "}";

    case GRAPH_NAME_LEFT_CURLY:
      return "GRAPH_NAME {";

   default:
     RAPTOR_DEBUG2("UNKNOWN token %d - add a new case\n", token);
     return "(UNKNOWN)";
  }
}
#endif



void
turtle_token_free(raptor_world* world, int token, YYSTYPE *lval)
{
  if(!token)
    return;

  switch(token) {
    case STRING_LITERAL:
    case BLANK_LITERAL:
    case IDENTIFIER:
      if(lval->string)
        RAPTOR_FREE(char*, lval->string);
      break;

    case URI_LITERAL:
    case QNAME_LITERAL:
      if(lval->uri)
        raptor_free_uri(lval->uri);
      break;
    default:
      break;
  }
}


#ifdef STANDALONE

#define FILE_READ_BUF_SIZE 4096

int
main(int argc, char *argv[]) 
{
  char *turtle_string = NULL;
  raptor_parser rdf_parser;
  raptor_turtle_parser turtle_parser;
  yyscan_t scanner;
  int token = EOF;
  YYSTYPE lval;
  const unsigned char *uri_string;
  const char *filename = NULL;
  char *buf = NULL;
  size_t len;
  raptor_world* world;
  FILE *fh;
  
  world = raptor_new_world();
  
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
    fh = (FILE*)stdin;
  }

  turtle_string = RAPTOR_CALLOC(char*, FILE_READ_BUF_SIZE, 1);
  fread(turtle_string, FILE_READ_BUF_SIZE, 1, fh);
  fclose(fh);

  memset(&rdf_parser, 0, sizeof(rdf_parser));
  memset(&turtle_parser, 0, sizeof(turtle_parser));

  rdf_parser.world = world;

  /* discard namespace errors - caused by not interpreting @prefix 
   * and hence causing failed qname construction
   */
  raptor_namespaces_init(rdf_parser.world, &turtle_parser.namespaces, 0);

  yylex_init(&turtle_parser.scanner);
  scanner = turtle_parser.scanner;

  len =  strlen(RAPTOR_GOOD_CAST(const char*, turtle_string));
  buf =  RAPTOR_MALLOC(char*, len + 3);
  memcpy(buf, turtle_string, len);
  buf[len] =  ' ';
  buf[len + 1] =  buf[len + 2] = '\0'; /* YY_END_OF_BUFFER_CHAR; */
  (void)turtle_lexer__scan_buffer(buf, len + 3, scanner);

  turtle_lexer_set_extra(&rdf_parser, scanner);

  /* Initialise enough of the parser and locator to get error messages */
  rdf_parser.context = &turtle_parser;
  turtle_parser.lineno = 1;
  rdf_parser.locator.file = filename;
  rdf_parser.locator.column = -1;

  uri_string = raptor_uri_filename_to_uri_string(filename);
  rdf_parser.base_uri = raptor_new_uri(world, uri_string);
  RAPTOR_FREE(char*, uri_string);
  
  while(1) {
    memset(&lval, 0, sizeof(YYSTYPE));
    if(turtle_lexer_get_text(scanner) != NULL)
      printf("yyinput '%s'\n", turtle_lexer_get_text(scanner));
    token = yylex(&lval, scanner);
#ifdef RAPTOR_DEBUG
    printf("token %s\n", turtle_token_print(world, token, &lval));
#else
    printf("token %d\n", token);
#endif
    turtle_token_free(world, token, &lval);
    if(!token || token == EOF || token == ERROR_TOKEN)
      break;
  }

  if(buf)
    RAPTOR_FREE(char*, buf);
  
  yylex_destroy(scanner);

  raptor_namespaces_clear(&turtle_parser.namespaces);
  
  raptor_free_uri(rdf_parser.base_uri);

  RAPTOR_FREE(char*, turtle_string);

  raptor_free_world(world);


  if(token == ERROR_TOKEN)
    return 1;
 
  return 0;
}
#endif
