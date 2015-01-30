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
    ANY = 258,
    ALL = 259,
    OPTIONAL = 260,
    NO = 261,
    SOME = 262,
    MANY = 263,
    THE = 264,
    IF = 265,
    THEN = 266,
    ELSE = 267,
    UNKNOWN = 268,
    FI = 269,
    NOT = 270,
    AND = 271,
    OR = 272,
    XOR = 273,
    IFF = 274,
    IMPLIES = 275,
    CAUSES = 276,
    BECAUSE = 277,
    EVERY = 278,
    WHILE = 279,
    UNTIL = 280,
    WHEN = 281,
    FORSOME = 282,
    FORALL = 283,
    ME = 284,
    I = 285,
    YOU = 286,
    IT = 287,
    HE = 288,
    SHE = 289,
    HERE = 290,
    NOW = 291,
    IS = 292,
    ISA = 293,
    ISS = 294,
    ISU = 295,
    ISASTAR = 296,
    ISC = 297,
    ISG = 298,
    ISP = 299,
    ISCSTAR = 300,
    HAS = 301,
    HASPART = 302,
    ISAPART = 303,
    DO = 304,
    HDO = 305,
    BANG = 306,
    AT = 307,
    OF = 308,
    WITH = 309,
    OD = 310,
    FROM = 311,
    TO = 312,
    IN = 313,
    OUT = 314,
    WHERE = 315,
    mkrBEGIN = 316,
    mkrEND = 317,
    HO = 318,
    REL = 319,
    EQUALS = 320,
    ASSIGN = 321,
    LET = 322,
    DCOLON = 323,
    DSTAR = 324,
    A = 325,
    HAT = 326,
    DOT = 327,
    COMMA = 328,
    SEMICOLON = 329,
    LEFT_SQUARE = 330,
    RIGHT_SQUARE = 331,
    LEFT_ROUND = 332,
    RIGHT_ROUND = 333,
    LEFT_CURLY = 334,
    RIGHT_CURLY = 335,
    TRUE_TOKEN = 336,
    FALSE_TOKEN = 337,
    PREFIX = 338,
    MKB = 339,
    MKE = 340,
    BASE = 341,
    SPARQL_PREFIX = 342,
    SPARQL_BASE = 343,
    STRING_LITERAL = 344,
    URI_LITERAL = 345,
    GRAPH_NAME_LEFT_CURLY = 346,
    BLANK_LITERAL = 347,
    QNAME_LITERAL = 348,
    IDENTIFIER = 349,
    LANGTAG = 350,
    INTEGER_LITERAL = 351,
    FLOATING_LITERAL = 352,
    DECIMAL_LITERAL = 353,
    VARIABLE = 354,
    ERROR_TOKEN = 355
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

#line 167 "mkr_parser.tab.h" /* yacc.c:1909  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int mkr_parser_parse (raptor_parser* rdf_parser, void* yyscanner);

#endif /* !YY_MKR_PARSER_MKR_PARSER_TAB_H_INCLUDED  */
