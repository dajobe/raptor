/* -*- Mode: c; c-basic-offset: 2 -*-
 *
 * n3_lexer.l - Raptor N3 lexer - making tokens for n3 grammar generator
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
 * This is N-Triples + 6 things
 *   N-Triples + @prefix + qnames + "a" + [] + , and ;
 * 
 * Missing: UTF-8 support (and N-Triples escapes)
 *
 */


/* recognise 8-bits */
%option 8bit
%option warn nodefault

/* all symbols prefixed by this */
%option prefix="n3_lexer_"

%option outfile="n3_lexer.c"
%option header-file="n3_lexer.h"

/* Do not emit #include <unistd.h> */
%option nounistd

/* Never interactive */
/*  No isatty() check */
%option never-interactive
/* Batch scanner */
%option batch

/* Never use yyunput */
%option nounput

/* make a yylineno please */
%option yylineno


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

#include <n3_parser.tab.h>


static unsigned char *copy_token(unsigned char *text, size_t len);
unsigned char *copy_string_token(unsigned char *text, size_t len, int delim);

char *filename=NULL;
int lineno=1;

#ifdef STANDALONE
YYSTYPE n3_parser_lval;
#endif

/* Missing rqql_lexer.c/h prototypes */
int n3_lexer_get_lineno(void);
FILE *n3_lexer_get_in(void);
FILE *n3_lexer_get_out(void);
int n3_lexer_get_leng(void);
char *n3_lexer_get_text(void);
void n3_lexer_set_lineno(int line_number);
void n3_lexer_set_in(FILE* in_str);
void n3_lexer_set_out(FILE* out_str);
int n3_lexer_get_debug(void);
void n3_lexer_set_debug(int bdebug);
int n3_lexer_lex_destroy(void);


%}


%%
  /* rules */

  int c;


\r\n|\r|\n   { lineno++; }
 
[\ \t\v]+   { /* empty */ }


"a" { return A; }

"."       { return DOT; } 
","       { return COMMA; } 
";"       { return SEMICOLON; }
"["       { return LEFT_SQUARE; }
"]"       { return RIGHT_SQUARE; }
"@prefix" { return PREFIX; }
"@"       { return AT; }
"^^"      { return HAT; }

'([^'\\\n\r]|\\[^\n\r])*'    { n3_parser_lval.string=copy_string_token(yytext+1, yyleng-2, '\"'); /* ' */
                               return STRING_LITERAL; }

\"([^"\\\n\r]|\\[^\n\r])*\"   { n3_parser_lval.string=copy_string_token(yytext+1, yyleng-2, '"'); /* ' */
                                return STRING_LITERAL; }

_:[A-Za-z0-9]+	{ n3_parser_lval.string=copy_token(yytext+2, yyleng-2);
                          return BLANK_LITERAL; }

[A-Za-z][A-Za-z0-9_]*:[A-Za-z][A-Za-z0-9_]*	{ n3_parser_lval.string=copy_token(yytext, yyleng);
                          return QNAME_LITERAL; }

:[A-Za-z][A-Za-z0-9_]*	{ n3_parser_lval.string=copy_token(yytext, yyleng);
                          return QNAME_LITERAL; }

[A-Za-z][A-Za-z0-9_]*:	{ n3_parser_lval.string=copy_token(yytext, yyleng);
                          return QNAME_LITERAL; }

:	{ n3_parser_lval.string=NULL;
                          return QNAME_LITERAL; }

\<[^>]*\>   { if(yyleng == 2) 
                n3_parser_lval.string=NULL;
              else
                n3_parser_lval.string=copy_token(yytext+1, yyleng-2);
              return URI_LITERAL; }

[A-Za-z][-A-Za-z0-9_]*	{ n3_parser_lval.string=copy_token(yytext, yyleng);
                          return IDENTIFIER; }

\#              { while((c=input()) != EOF && c != '\n'  && c != '\r' && c)
                    ;
                  lineno++;
                  if(c == EOF)
                    return EOF;
                }

.         	{ if (*yytext == EOF)
                    return EOF;

                  n3_syntax_error("Syntax error at '%c'", *yytext);
                  return ERROR;
		}

%%
  /* user code */

int
yywrap (void) {
  return 1;
}


unsigned char *copy_token(unsigned char *text, size_t len) {
  unsigned char *s;
  if(!len)
    len=strlen(text);
  s=(unsigned char *)malloc(len+1);

  strncpy(s, text, len);
  s[len] = '\0';
  return s;
}


unsigned char *copy_string_token(unsigned char *text, size_t len, int delim) {
  int i;
  unsigned char *s, *d;
  unsigned char *string;
  string=(unsigned char *)malloc(len+1);
  for(s=text, d=string, i=0; i<len; s++, i++) {
    unsigned char c=*s;

    if(c == '\\' ) {
      s++; i++;
      c=*s;
      if(c == 'n' || c == 'r' || c == 't' || c == '\\' || c == delim)
        *d++=c;
      else if (c == 'u' || c == 'U') {
        int ulen=(c == 'u') ? 4 : 8;
        unsigned long unichar=0;
        
        s++; i++;
        if(i+ulen > len) {
          n3_syntax_error("\\%c over end of line", c);
          return 0;
        }
        
        sscanf(s, ((ulen == 4) ? "%04lx" : "%08lx"), &unichar);
        s+= ulen-1;
        i+= ulen-1;
        
        if(unichar < 0 || unichar > 0x10ffff) {
          n3_syntax_error("Illegal Unicode character with code point #x%lX.", unichar);
          break;
        }
          
        d+=raptor_unicode_char_to_utf8(unichar, d);

      } else {
        /* don't handle \x where x isn't one of: \n \r \\ (delim) */
        n3_syntax_error("Illegal string escape \\%c in \"%s\"", c, text);
      }
    } else
      *d++=c;
  }
  *d='\0';
  return string;
}



#ifdef RAPTOR_DEBUG

const char *
n3_token_print(int token)
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
      sprintf(buffer, "STRING_LITERAL(%s)", n3_parser_lval.string);
      return buffer;

    case URI_LITERAL:
      sprintf(buffer, "URI_LITERAL(%s)", 
        (n3_parser_lval.string ? (char*)n3_parser_lval.string : ""));
      return buffer;

    case BLANK_LITERAL:
      sprintf(buffer, "BLANK_LITERAL(%s)", n3_parser_lval.string);
      return buffer;

    case QNAME_LITERAL:
      sprintf(buffer, "QNAME_LITERAL(%s)", 
              (n3_parser_lval.string ? (char*)n3_parser_lval.string : ""));
      return buffer;

    case IDENTIFIER:
      sprintf(buffer, "IDENTIFIER(%s)", 
              (n3_parser_lval.string ? (char*)n3_parser_lval.string : ""));
      return buffer;

    case ERROR:
      return "ERROR";

   default:
     fprintf(stderr, "n3_token_print: UNKNOWN token %d - add a new case\n", token);
     abort();
  }
}
#endif



#ifdef STANDALONE

int
n3_syntax_error(const char *message, ...)
{
  va_list arguments;

  va_start(arguments, message);

  vfprintf(stderr, message, arguments);

  va_end(arguments);

  fputc('\n', stderr);
  return 0;
}


int
main(int argc, char *argv[]) 
{
  int token=EOF;

  if(argc > 1) {
    filename=argv[1];
    yyin = fopen(argv[1], "r");
  } else {
    filename="<stdin>";
    yyin = stdin;
  }

  while(1) {
    if(yytext != NULL)
      printf("yyinput '%s'\n", yytext);
    token=yylex();
#ifdef RAPTOR_DEBUG
    printf("token %s\n", n3_token_print(token));
#else
    printf("token %d\n", token);
#endif
    if(!token || token == EOF || token == ERROR)
      break;
  }

  if(token==ERROR)
    return 1;
 
  return 0;
}
#endif
