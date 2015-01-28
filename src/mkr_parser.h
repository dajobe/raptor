/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2013 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_MKR_PARSER_MKR_PARSER_TAB_H_INCLUDED
# define YY_MKR_PARSER_MKR_PARSER_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int mkr_parser_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    NO = 258,
    OPTIONAL = 259,
    SOME = 260,
    MANY = 261,
    THE = 262,
    IF = 263,
    THEN = 264,
    ELSE = 265,
    UNKNOWN = 266,
    FI = 267,
    NOT = 268,
    AND = 269,
    OR = 270,
    XOR = 271,
    IMPLIES = 272,
    CAUSES = 273,
    BECAUSE = 274,
    ANY = 275,
    ALL = 276,
    EVERY = 277,
    WHILE = 278,
    UNTIL = 279,
    WHEN = 280,
    FORSOME = 281,
    FORALL = 282,
    ME = 283,
    I = 284,
    YOU = 285,
    IT = 286,
    HE = 287,
    SHE = 288,
    HERE = 289,
    NOW = 290,
    IS = 291,
    ISA = 292,
    ISS = 293,
    ISU = 294,
    ISASTAR = 295,
    ISC = 296,
    ISG = 297,
    ISP = 298,
    ISCSTAR = 299,
    HAS = 300,
    HASPART = 301,
    ISAPART = 302,
    DO = 303,
    HDO = 304,
    BANG = 305,
    AT = 306,
    OF = 307,
    WITH = 308,
    OD = 309,
    FROM = 310,
    TO = 311,
    IN = 312,
    OUT = 313,
    WHERE = 314,
    HO = 315,
    REL = 316,
    mkrBEGIN = 317,
    mkrEND = 318,
    EQUALS = 319,
    ASSIGN = 320,
    LET = 321,
    DCOLON = 322,
    DSTAR = 323,
    A = 324,
    HAT = 325,
    DOT = 326,
    COMMA = 327,
    SEMICOLON = 328,
    LEFT_SQUARE = 329,
    RIGHT_SQUARE = 330,
    LEFT_ROUND = 331,
    RIGHT_ROUND = 332,
    LEFT_CURLY = 333,
    RIGHT_CURLY = 334,
    TRUE_TOKEN = 335,
    FALSE_TOKEN = 336,
    PREFIX = 337,
    MKB = 338,
    MKE = 339,
    BASE = 340,
    SPARQL_PREFIX = 341,
    SPARQL_BASE = 342,
    STRING_LITERAL = 343,
    URI_LITERAL = 344,
    GRAPH_NAME_LEFT_CURLY = 345,
    BLANK_LITERAL = 346,
    QNAME_LITERAL = 347,
    IDENTIFIER = 348,
    LANGTAG = 349,
    INTEGER_LITERAL = 350,
    FLOATING_LITERAL = 351,
    DECIMAL_LITERAL = 352,
    VARIABLE = 353,
    ERROR_TOKEN = 354
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE YYSTYPE;
union YYSTYPE
{
#line 131 "./mkr_parser.y" /* yacc.c:1909  */

  unsigned char *string;
  raptor_term *identifier;
  raptor_sequence *sequence;
  raptor_uri *uri;

  /* mKR */
  MKR_value *value;
  MKR_nv *nv;
  MKR_pp *pp;

#line 166 "mkr_parser.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int mkr_parser_parse (raptor_parser* rdf_parser, void* yyscanner);

#endif /* !YY_MKR_PARSER_MKR_PARSER_TAB_H_INCLUDED  */
