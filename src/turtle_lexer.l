/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * turtle_lexer.l - Raptor Turtle lexer - making tokens for turtle grammar generator
 *
 * $Id$
 *
 * Copyright (C) 2003-2004 David Beckett - http://purl.org/net/dajobe/
 * Institute for Learning and Research Technology - http://www.ilrt.bris.ac.uk/
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
 * Turtle is defined in http://www.ilrt.bristol.ac.uk/discovery/2004/01/turtle/
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

%option reentrant


  /* definitions */

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

#include <turtle_parser.tab.h>

#include <turtle_common.h>


static unsigned char *copy_token(unsigned char *text, size_t len);
unsigned char *copy_string_token(raptor_parser* rdf_parser, unsigned char *text, size_t len, int delim);

char *filename=NULL;

#ifdef RAPTOR_DEBUG
const char * turtle_token_print(int token, YYSTYPE *lval);
#endif

int turtle_lexer_lex (YYSTYPE *turtle_parser_lval, yyscan_t yyscanner);
#define YY_DECL int turtle_lexer_lex (YYSTYPE *turtle_parser_lval, yyscan_t yyscanner)

#ifdef __cplusplus
#define INPUT_FN yyinput
#else
#define INPUT_FN input
#endif


/* Missing turtle_lexer.c/h prototypes */
int turtle_lexer_get_column(yyscan_t yyscanner);
void turtle_lexer_set_column(int  column_no , yyscan_t yyscanner);

%}

%x PREF


%%
  /* rules */

  int c;
  raptor_parser *rdf_parser=(raptor_parser*)yyextra;
  raptor_turtle_parser* turtle_parser=(raptor_turtle_parser*)rdf_parser->context;

\r\n|\r|\n   { turtle_parser->lineno++; }
 
[\ \t\v]+   { /* empty */ }


"a" { return A; }

"."       { return DOT; } 
","       { return COMMA; } 
";"       { return SEMICOLON; }
"["       { return LEFT_SQUARE; }
"]"       { return RIGHT_SQUARE; }
"@prefix" { BEGIN(PREF); return PREFIX; }
"@"       { return AT; }
"^^"      { return HAT; }
"("       { return LEFT_ROUND; }
")"       { return RIGHT_ROUND; }


'([^'\\\n\r]|\\[^\n\r])*'    { turtle_parser_lval->string=copy_string_token(rdf_parser, (unsigned char*)yytext+1, yyleng-2, '\"'); /* ' */
                               return STRING_LITERAL; }

\"([^"\\\n\r]|\\[^\n\r])*\"   { turtle_parser_lval->string=copy_string_token(rdf_parser, (unsigned char*)yytext+1, yyleng-2, '"'); /* ' */
                                return STRING_LITERAL; }

_:[A-Za-z_][A-Za-z0-9_]*	{ turtle_parser_lval->string=copy_token((unsigned char*)yytext+2, yyleng-2);
                          return BLANK_LITERAL; }

[A-Za-z][-A-Za-z0-9_]*:[A-Za-z_][-A-Za-z0-9_]*	{ turtle_parser_lval->uri=turtle_qname_to_uri(rdf_parser, (unsigned char*)yytext, yyleng);
                          return QNAME_LITERAL; }

:[A-Za-z_][-A-Za-z0-9_]*	{ turtle_parser_lval->uri=turtle_qname_to_uri(rdf_parser, (unsigned char*)yytext, yyleng);
                          return QNAME_LITERAL; }

[A-Za-z][-A-Za-z0-9_]*:	{ turtle_parser_lval->uri=turtle_qname_to_uri(rdf_parser, (unsigned char*)yytext, yyleng);
                          return QNAME_LITERAL; }

:	{ turtle_parser_lval->uri=turtle_qname_to_uri(rdf_parser, (unsigned char*)yytext, yyleng);
                          return QNAME_LITERAL; }

[0-9]+        { turtle_parser_lval->integer=atoi(yytext);
                          return INTEGER_LITERAL; }

<PREF>[\ \t\v]+ { /* eat up leading whitespace */ }
<PREF>(:|[A-Za-z][-A-Za-z0-9_]*:)	{ turtle_parser_lval->string=copy_token((unsigned char*)yytext, yyleng);
                          BEGIN(INITIAL);
                          return IDENTIFIER; }
<PREF>.		{ BEGIN(INITIAL);
		  if (*yytext == EOF)
                    return EOF;

                  turtle_syntax_error(rdf_parser, "syntax error at '%c' - @prefix name must end in :", *yytext);
                  return ERROR;  }


\<[^>]*\>   { if(yyleng == 2) 
                turtle_parser_lval->uri=raptor_uri_copy(rdf_parser->base_uri);
              else {
                yytext[yyleng-1]='\0';
                turtle_parser_lval->uri=raptor_new_uri_relative_to_base(rdf_parser->base_uri, (const unsigned char*)yytext+1);
              }
              return URI_LITERAL; }

[A-Za-z][-A-Za-z0-9_]*	{ turtle_parser_lval->string=copy_token((unsigned char*)yytext, yyleng);
                          return IDENTIFIER; }

\#              { while((c=INPUT_FN(yyscanner)) != EOF && c != '\n'  && c != '\r' && c)
                    ;
                  turtle_parser->lineno++;
                  if(c == EOF)
                    return EOF;
                }

.         	{ if (*yytext == EOF)
                    return EOF;

                  turtle_syntax_error(rdf_parser, "syntax error at '%c'", *yytext);
                  return ERROR;
		}

%%
  /* user code */

int
yywrap (yyscan_t yyscanner) {
  return 1;
}


unsigned char *
copy_token(unsigned char *text, size_t len) {
  unsigned char *s;
  if(!len)
    len=strlen((const char*)text);
  s=(unsigned char *)malloc(len+1);

  strncpy((char*)s, (const char*)text, len);
  s[len] = '\0';
  return s;
}


unsigned char *
copy_string_token(raptor_parser* rdf_parser, unsigned char *text, size_t len, int delim) {
  size_t i;
  unsigned char *s, *d;
  unsigned char *string=(unsigned char *)malloc(len+1);

  for(s=text, d=string, i=0; i<len; s++, i++) {
    unsigned char c=*s;

    if(c == '\\' ) {
      s++; i++;
      c=*s;
      if(c == 'n')
        *d++= '\n';
      else if(c == 'r')
        *d++= '\r';
      else if(c == 't')
        *d++= '\t';
      else if(c == '\\' || c == delim)
        *d++=c;
      else if (c == 'u' || c == 'U') {
        size_t ulen=(c == 'u') ? 4 : 8;
        unsigned long unichar=0;
        
        s++; i++;
        if(i+ulen > len) {
          turtle_syntax_error(rdf_parser, "\\%c over end of line", c);
          free(string);
          return NULL;
        }
        
        sscanf((const char*)s, ((ulen == 4) ? "%04lx" : "%08lx"), &unichar);
        s+= ulen-1;
        i+= ulen-1;
        
        if(unichar < 0 || unichar > 0x10ffff) {
          turtle_syntax_error(rdf_parser, "Illegal Unicode character with code point #x%lX.", unichar);
          free(string);
          return NULL;
        }
          
        d+=raptor_unicode_char_to_utf8(unichar, d);

      } else {
        /* don't handle \x where x isn't one of: \n \r \\ (delim) */
        turtle_syntax_error(rdf_parser, "Illegal string escape \\%c in \"%s\"", c, text);
      }
    } else
      *d++=c;
  }
  *d='\0';
  return string;
}



#ifdef RAPTOR_DEBUG

const char *
turtle_token_print(int token, YYSTYPE *lval)
{
  static char buffer[2048];

  if(!token)
    return "<<EOF>>";
  
  switch(token) {
    case PREFIX:
      return "PREFIX";

    case WS:
      return "WS";

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

    case AT:
      return "AT";

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

    case IDENTIFIER:
      sprintf(buffer, "IDENTIFIER(%s)", 
              (lval->string ? (char*)lval->string : ""));
      return buffer;

    case ERROR:
      return "ERROR";

   default:
     fprintf(stderr, "turtle_token_print: UNKNOWN token %d - add a new case\n", token);
     abort();
  }
}
#endif



#ifdef STANDALONE

static void
turtle_token_free(int token, YYSTYPE *lval)
{
  if(!token)
    return;

  switch(token) {
    case STRING_LITERAL:
    case BLANK_LITERAL:
    case IDENTIFIER:
      if(lval->string)
        free(lval->string);
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


int
main(int argc, char *argv[]) 
{
  raptor_parser rdf_parser;
  raptor_turtle_parser turtle_parser;
  yyscan_t scanner;
  int token=EOF;
  FILE *fh;
  YYSTYPE lval;
  const char *uri_string;
  
  raptor_init();
  
  if(argc > 1) {
    filename=argv[1];
    fh=fopen(filename, "r");
    if(!fh) {
      fprintf(stderr, "%s: Cannot open file %s - %s\n", argv[0], filename,
              strerror(errno));
      exit(1);
    }
  } else {
    filename="<stdin>";
    fh=stdin;
  }

  memset(&rdf_parser, 0, sizeof(raptor_parser));
  memset(&turtle_parser, 0, sizeof(raptor_turtle_parser));

  yylex_init(&turtle_parser.scanner);
  scanner=turtle_parser.scanner;
  turtle_lexer_set_in(fh, scanner);
  turtle_lexer_set_extra(&rdf_parser, scanner);

  /* Initialise enough of the parser and locator to get error messages */
  rdf_parser.context=&turtle_parser;
  turtle_parser.lineno=1;
  rdf_parser.locator.file=filename;
  rdf_parser.locator.column= -1;

  uri_string=raptor_uri_filename_to_uri_string(filename);
  rdf_parser.base_uri=raptor_new_uri(uri_string);
#if defined (RAPTOR_DEBUG) && defined(HAVE_DMALLOC_H)
#undef free
#endif
  free((void*)uri_string);
  
  while(1) {
    memset(&lval, 0, sizeof(YYSTYPE));
    if(turtle_lexer_get_text(scanner) != NULL)
      printf("yyinput '%s'\n", turtle_lexer_get_text(scanner));
    token=yylex(&lval, scanner);
#ifdef RAPTOR_DEBUG
    printf("token %s\n", turtle_token_print(token, &lval));
#else
    printf("token %d\n", token);
#endif
    turtle_token_free(token, &lval);
    if(!token || token == EOF || token == ERROR)
      break;
  }

  yylex_destroy(scanner);

  raptor_free_uri(rdf_parser.base_uri);

  raptor_finish();


  if(token==ERROR)
    return 1;
 
  return 0;
}
#endif
