/* A Bison parser, made by GNU Bison 3.0.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         mkr_parser_parse
#define yylex           mkr_parser_lex
#define yyerror         mkr_parser_error
#define yydebug         mkr_parser_debug
#define yynerrs         mkr_parser_nerrs


/* Copy the first part of user declarations.  */
#line 25 "./mkr_parser.y" /* yacc.c:339  */

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

/* Make lex/yacc interface as small as possible */
#undef yylex
#define yylex turtle_lexer_lex

/* Prototypes for local functions */
int mkr_syntax_error(raptor_parser *rdf_parser, const char *message, ...);
int mkr_parser_error(raptor_parser* rdf_parser, void* scanner, const char *msg);



#line 147 "mkr_parser.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "mkr_parser.tab.h".  */
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
#line 131 "./mkr_parser.y" /* yacc.c:355  */

  unsigned char *string;
  raptor_term *identifier;
  raptor_sequence *sequence;
  raptor_uri *uri;

  /* mKR */
  MKR_value *value;
  MKR_nv *nv;
  MKR_pp *pp;

#line 299 "mkr_parser.c" /* yacc.c:355  */
};
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif



int mkr_parser_parse (raptor_parser* rdf_parser, void* yyscanner);

#endif /* !YY_MKR_PARSER_MKR_PARSER_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 313 "mkr_parser.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   532

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  100
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  29
/* YYNRULES -- Number of rules.  */
#define YYNRULES  102
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  158

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   354

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   317,   317,   320,   321,   324,   327,   328,   331,   332,
     337,   338,   339,   342,   343,   345,   346,   350,   351,   352,
     353,   354,   355,   358,   359,   360,   363,   364,   365,   366,
     367,   368,   375,   376,   380,   381,   389,   390,   391,   392,
     393,   394,   395,   396,   397,   398,   399,   401,   402,   403,
     406,   414,   417,   418,   419,   423,   456,   496,   529,   640,
     673,   711,   744,   781,   784,   787,   788,   789,   790,   791,
     792,   793,   794,   795,   798,   799,   800,   803,   804,   805,
     808,   809,   810,   811,   812,   813,   814,   815,   816,   821,
     833,   856,   879,   895,   910,   921,   934,   947,   964,   977,
     993,  1007,  1025
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "\"no\"", "\"optional\"", "\"some\"",
  "\"many\"", "\"the\"", "\"if\"", "\"then\"", "\"else\"", "\"unknown\"",
  "\"fi\"", "\"not\"", "\"and\"", "\"or\"", "\"xor\"", "\"implies\"",
  "\"causes\"", "\"because\"", "\"any\"", "\"all\"", "\"every\"",
  "\"while\"", "\"until\"", "\"when\"", "\"forSome\"", "\"forAll\"",
  "\"me\"", "\"I\"", "\"you\"", "\"it\"", "\"he\"", "\"she\"", "\"here\"",
  "\"now\"", "\"is\"", "\"isa\"", "\"iss\"", "\"isu\"", "\"isa*\"",
  "\"isc\"", "\"isg\"", "\"isp\"", "\"isc*\"", "\"has\"", "\"haspart\"",
  "\"isapart\"", "\"do\"", "\"hdo\"", "\"!\"", "\"at\"", "\"of\"",
  "\"with\"", "\"od\"", "\"from\"", "\"to\"", "\"in\"", "\"out\"",
  "\"where\"", "\"ho\"", "\"rel\"", "\"begin\"", "\"end\"", "\"=\"",
  "\":=\"", "\"let\"", "\"::\"", "\"**\"", "\"a\"", "\"^\"", "\".\"",
  "\",\"", "\";\"", "\"[\"", "\"]\"", "\"(\"", "\")\"", "\"{\"", "\"}\"",
  "\"true\"", "\"false\"", "\"@prefix\"", "\"@mkb\"", "\"@mke\"",
  "\"@base\"", "\"PREFIX\"", "\"BASE\"", "\"string literal\"",
  "\"URI literal\"", "\"Graph URI literal {\"", "\"blank node\"",
  "\"QName\"", "\"identifier\"", "\"langtag\"", "\"integer literal\"",
  "\"floating point literal\"", "\"decimal literal\"", "\"variable\"",
  "ERROR_TOKEN", "$accept", "Document", "sentenceList", "nv", "pp",
  "value", "prefix", "viewOption", "nameOption", "subject", "predicate",
  "object", "collection", "set", "sentence", "predicateObject",
  "predicateObjectList", "objectList", "nvList", "ppList", "variable",
  "isVerb", "hoVerb", "hasVerb", "doVerb", "preposition", "literal",
  "resource", "blankNode", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354
};
# endif

#define YYPACT_NINF -123

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-123)))

#define YYTABLE_NINF -26

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -123,    27,   114,  -123,   -61,  -123,  -123,  -123,   -80,   434,
     434,   -80,   -80,   359,   384,  -123,  -123,  -123,  -123,  -123,
    -123,   -57,   -63,  -123,  -123,  -123,  -123,  -123,  -123,  -123,
     -46,   -80,   330,  -123,  -123,  -123,    -8,   434,  -123,  -123,
    -123,  -123,  -123,  -123,  -123,  -123,    13,  -123,  -123,  -123,
    -123,    29,   434,   434,  -123,   -53,  -123,   -56,   165,    -9,
     -66,    -4,   -20,   267,     5,  -123,  -123,  -123,  -123,  -123,
    -123,  -123,  -123,  -123,  -123,  -123,  -123,  -123,    32,  -123,
     434,   434,    80,   434,   409,   128,   434,  -123,  -123,     6,
    -123,    14,  -123,  -123,    17,  -123,  -123,  -123,   -30,    26,
      30,  -123,    -1,  -123,    65,  -123,  -123,  -123,  -123,  -123,
    -123,  -123,  -123,  -123,  -123,  -123,    24,  -123,  -123,  -123,
     178,  -123,    33,  -123,  -123,  -123,   229,   334,  -123,  -123,
    -123,  -123,  -123,  -123,  -123,  -123,  -123,   434,   280,   409,
    -123,   343,   216,  -123,  -123,  -123,  -123,    19,    36,    45,
      46,  -123,  -123,  -123,  -123,  -123,    80,  -123
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,     1,     0,    77,    78,    79,     0,     0,
       0,     0,     0,     0,     0,     4,    98,    99,    10,    11,
      12,     0,    94,   100,   102,   101,    95,    96,    97,    63,
       0,    16,     0,    19,    20,     3,    22,     0,    21,    17,
      18,    51,    13,    58,    28,    29,     0,    31,    30,    26,
      27,     0,     0,     0,    34,     0,    32,     0,     0,     0,
       0,    89,     0,     0,     0,    64,    67,    66,    65,    68,
      71,    70,    69,    72,    74,    75,    76,    56,     0,    73,
       0,     0,     0,     0,     0,     0,     0,    41,    42,     0,
      22,     0,    35,    33,     0,    38,    92,    93,     0,     0,
       0,    15,     0,    50,    54,    53,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    52,     0,    25,    24,    23,
       0,     4,     0,     8,    45,    62,     0,     0,    57,    40,
      43,    37,    90,    91,    39,    36,    55,     0,     0,     0,
      47,     0,     0,    44,    46,    61,    60,     6,     7,    31,
      26,    54,    49,     5,    48,     9,     0,    59
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -123,  -123,   -14,  -122,  -113,   -28,  -123,  -123,  -123,    22,
    -123,    12,     2,    15,    49,    11,  -123,     1,  -123,   -96,
      -2,    28,  -123,  -123,    94,   -79,    18,     0,    31
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,   115,   125,   122,    30,    31,    63,    32,
     116,    43,    44,    45,    35,    77,    78,    46,   148,   126,
      47,    80,    81,    82,    37,   127,    48,    49,    50
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      36,    58,    39,   118,    33,   146,    42,    60,   138,    52,
      53,    51,    41,   145,    55,    57,    86,    34,    29,    86,
      38,    93,    92,    96,   141,   145,    97,     3,   145,    64,
      79,    61,    59,    40,   157,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    62,   118,    85,
      90,    90,    39,    39,    33,    33,    36,    84,    39,   132,
      33,    36,   133,    39,    95,    33,    98,    34,    34,    99,
      38,    38,   101,    34,    89,    91,    38,   118,    34,   129,
     117,    38,   119,    40,    40,    86,    87,   130,   139,    40,
     131,    86,   104,   105,    40,   120,   123,    29,   128,   134,
      79,    86,    88,   135,   102,   103,   143,   142,   156,   -25,
     -23,   153,   100,   136,    -2,     4,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   149,    83,   150,   147,     0,
     137,   106,   107,   108,   109,   110,   111,   112,   113,   114,
      36,     0,    39,     0,    33,     0,     0,     0,     0,   151,
       0,   123,     0,     0,   117,     0,   119,    34,     0,     0,
      38,     0,     5,     6,     7,     8,     4,     0,     0,    23,
       0,     0,    25,    40,     9,    10,    11,    12,    29,   106,
     107,   108,   109,   110,   111,   112,   113,   114,    13,     0,
      14,     0,    15,     0,    16,    17,    18,    19,    20,    21,
       0,   124,    22,    23,     0,    24,    25,     0,     0,    26,
      27,    28,    29,     5,     6,     7,     8,     4,     0,     0,
       0,     0,     0,     0,     0,     9,    10,    11,    12,   106,
     107,   108,   109,   110,   111,   112,   113,   114,     0,    13,
       0,    14,     0,    15,    94,    16,    17,    18,    19,    20,
      21,   140,     0,    22,    23,     0,    24,    25,     0,     0,
      26,    27,    28,    29,     5,     6,     7,     8,     4,     0,
       0,     0,     0,     0,     0,     0,     9,    10,    11,    12,
     106,   107,   108,   109,   110,   111,   112,   113,   114,     0,
      13,     0,    14,     0,    15,   155,    16,    17,    18,    19,
      20,    21,   144,     0,    22,    23,     0,    24,    25,     0,
       0,    26,    27,    28,    29,     5,     6,     7,     8,     0,
       0,     0,     0,     0,     0,     0,     0,     9,    10,    11,
      12,   106,   107,   108,   109,   110,   111,   112,   113,   114,
       0,    13,     0,    14,     0,    15,     0,    16,    17,    18,
      19,    20,    21,   152,     0,    22,    23,     0,    24,    25,
       0,     0,    26,    27,    28,    29,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,     5,     6,
       7,     0,     0,     0,     0,   106,   107,   108,   109,   110,
     111,   112,   113,   114,   106,   107,   108,   109,   110,   111,
     112,   113,   114,     0,     0,     0,     0,     0,    13,     0,
      14,     0,     0,     0,    16,    17,   154,     0,     0,     0,
       0,     0,    22,    23,     0,    24,    25,     0,    29,    26,
      27,    28,    29,    13,    54,    14,     0,     0,     0,    16,
      17,     0,     0,     0,     0,     0,     0,    22,    23,     0,
      24,    25,     0,     0,    26,    27,    28,    29,    13,     0,
      14,    56,     0,     0,    16,    17,     0,     0,     0,     0,
       0,     0,    22,    23,     0,    24,    25,     0,     0,    26,
      27,    28,    29,    13,     0,    14,     0,   121,     0,    16,
      17,     0,     0,     0,     0,     0,     0,    22,    23,     0,
      24,    25,     0,     0,    26,    27,    28,    29,    13,     0,
      14,     0,     0,     0,    16,    17,     0,     0,     0,     0,
       0,     0,    22,    23,     0,    24,    25,     0,     0,    26,
      27,    28,    29
};

static const yytype_int16 yycheck[] =
{
       2,    15,     2,    82,     2,   127,     8,    70,   104,    11,
      12,    10,    73,   126,    13,    14,    72,     2,    98,    72,
       2,    77,    75,    89,   120,   138,    92,     0,   141,    31,
      32,    94,    89,     2,   156,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    93,   127,    37,
      52,    53,    52,    53,    52,    53,    58,    65,    58,    89,
      58,    63,    92,    63,    73,    63,    70,    52,    53,    89,
      52,    53,    67,    58,    52,    53,    58,   156,    63,    73,
      82,    63,    82,    52,    53,    72,    73,    73,    64,    58,
      73,    72,    80,    81,    63,    83,    84,    98,    86,    73,
     102,    72,    73,    73,    72,    73,    73,   121,    72,    64,
      64,   139,    63,   102,     0,     1,    51,    52,    53,    54,
      55,    56,    57,    58,    59,   127,    32,   127,   127,    -1,
     102,    51,    52,    53,    54,    55,    56,    57,    58,    59,
     142,    -1,   142,    -1,   142,    -1,    -1,    -1,    -1,   137,
      -1,   139,    -1,    -1,   156,    -1,   156,   142,    -1,    -1,
     142,    -1,    48,    49,    50,    51,     1,    -1,    -1,    89,
      -1,    -1,    92,   142,    60,    61,    62,    63,    98,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    74,    -1,
      76,    -1,    78,    -1,    80,    81,    82,    83,    84,    85,
      -1,    73,    88,    89,    -1,    91,    92,    -1,    -1,    95,
      96,    97,    98,    48,    49,    50,    51,     1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    -1,    74,
      -1,    76,    -1,    78,    79,    80,    81,    82,    83,    84,
      85,    73,    -1,    88,    89,    -1,    91,    92,    -1,    -1,
      95,    96,    97,    98,    48,    49,    50,    51,     1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,    63,
      51,    52,    53,    54,    55,    56,    57,    58,    59,    -1,
      74,    -1,    76,    -1,    78,    79,    80,    81,    82,    83,
      84,    85,    73,    -1,    88,    89,    -1,    91,    92,    -1,
      -1,    95,    96,    97,    98,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    60,    61,    62,
      63,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      -1,    74,    -1,    76,    -1,    78,    -1,    80,    81,    82,
      83,    84,    85,    73,    -1,    88,    89,    -1,    91,    92,
      -1,    -1,    95,    96,    97,    98,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    -1,    -1,    -1,    -1,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    -1,    -1,    -1,    -1,    -1,    74,    -1,
      76,    -1,    -1,    -1,    80,    81,    73,    -1,    -1,    -1,
      -1,    -1,    88,    89,    -1,    91,    92,    -1,    98,    95,
      96,    97,    98,    74,    75,    76,    -1,    -1,    -1,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    -1,
      91,    92,    -1,    -1,    95,    96,    97,    98,    74,    -1,
      76,    77,    -1,    -1,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    -1,    91,    92,    -1,    -1,    95,
      96,    97,    98,    74,    -1,    76,    -1,    78,    -1,    80,
      81,    -1,    -1,    -1,    -1,    -1,    -1,    88,    89,    -1,
      91,    92,    -1,    -1,    95,    96,    97,    98,    74,    -1,
      76,    -1,    -1,    -1,    80,    81,    -1,    -1,    -1,    -1,
      -1,    -1,    88,    89,    -1,    91,    92,    -1,    -1,    95,
      96,    97,    98
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,   101,   102,     0,     1,    48,    49,    50,    51,    60,
      61,    62,    63,    74,    76,    78,    80,    81,    82,    83,
      84,    85,    88,    89,    91,    92,    95,    96,    97,    98,
     106,   107,   109,   112,   113,   114,   120,   124,   126,   127,
     128,    73,   120,   111,   112,   113,   117,   120,   126,   127,
     128,   117,   120,   120,    75,   117,    77,   117,   102,    89,
      70,    94,    93,   108,   120,    36,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,   115,   116,   120,
     121,   122,   123,   124,    65,   111,    72,    73,    73,   109,
     120,   109,    75,    77,    79,    73,    89,    92,    70,    89,
     114,    67,    72,    73,   111,   111,    51,    52,    53,    54,
      55,    56,    57,    58,    59,   103,   110,   120,   125,   127,
     111,    78,   105,   111,    73,   104,   119,   125,   111,    73,
      73,    73,    89,    92,    73,    73,   115,   121,   119,    64,
      73,   119,   102,    73,    73,   104,   103,   117,   118,   120,
     127,   111,    73,   105,    73,    79,    72,   103
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   100,   101,   102,   102,   103,   104,   104,   105,   105,
     106,   106,   106,   107,   107,   108,   108,   109,   109,   109,
     109,   109,   109,   110,   110,   110,   111,   111,   111,   111,
     111,   111,   112,   112,   113,   113,   114,   114,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   114,   114,   114,
     114,   114,   115,   115,   115,   116,   116,   117,   117,   118,
     118,   119,   119,   120,   121,   122,   122,   122,   122,   122,
     122,   122,   122,   122,   123,   123,   123,   124,   124,   124,
     125,   125,   125,   125,   125,   125,   125,   125,   125,   126,
     126,   126,   126,   126,   126,   126,   126,   126,   126,   126,
     127,   127,   128
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     2,     0,     3,     2,     2,     1,     3,
       1,     1,     1,     2,     0,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     3,     2,     3,     4,     4,     3,     4,
       4,     3,     3,     4,     4,     3,     4,     4,     5,     5,
       3,     2,     2,     2,     2,     3,     1,     3,     1,     3,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     2,
       4,     4,     3,     3,     1,     1,     1,     1,     1,     1,
       1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (rdf_parser, yyscanner, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, rdf_parser, yyscanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, raptor_parser* rdf_parser, void* yyscanner)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (rdf_parser);
  YYUSE (yyscanner);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, raptor_parser* rdf_parser, void* yyscanner)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep, rdf_parser, yyscanner);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule, raptor_parser* rdf_parser, void* yyscanner)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              , rdf_parser, yyscanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule, rdf_parser, yyscanner); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
      default: yyformat = YY_("syntax error");
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, raptor_parser* rdf_parser, void* yyscanner)
{
  YYUSE (yyvaluep);
  YYUSE (rdf_parser);
  YYUSE (yyscanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  switch (yytype)
    {
          case 88: /* "string literal"  */
#line 274 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).string))
    RAPTOR_FREE(char*, ((*yyvaluep).string));
}
#line 1357 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 89: /* "URI literal"  */
#line 279 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).uri))
    raptor_free_uri(((*yyvaluep).uri));
}
#line 1366 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 91: /* "blank node"  */
#line 274 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).string))
    RAPTOR_FREE(char*, ((*yyvaluep).string));
}
#line 1375 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 92: /* "QName"  */
#line 279 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).uri))
    raptor_free_uri(((*yyvaluep).uri));
}
#line 1384 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 93: /* "identifier"  */
#line 274 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).string))
    RAPTOR_FREE(char*, ((*yyvaluep).string));
}
#line 1393 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 94: /* "langtag"  */
#line 274 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).string))
    RAPTOR_FREE(char*, ((*yyvaluep).string));
}
#line 1402 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 95: /* "integer literal"  */
#line 274 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).string))
    RAPTOR_FREE(char*, ((*yyvaluep).string));
}
#line 1411 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 96: /* "floating point literal"  */
#line 274 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).string))
    RAPTOR_FREE(char*, ((*yyvaluep).string));
}
#line 1420 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 97: /* "decimal literal"  */
#line 274 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).string))
    RAPTOR_FREE(char*, ((*yyvaluep).string));
}
#line 1429 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 98: /* "variable"  */
#line 274 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).string))
    RAPTOR_FREE(char*, ((*yyvaluep).string));
}
#line 1438 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 102: /* sentenceList  */
#line 289 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).sequence))
    raptor_free_sequence(((*yyvaluep).sequence));
}
#line 1447 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 103: /* nv  */
#line 304 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).nv))
    mkr_free_nv(((*yyvaluep).nv));
}
#line 1456 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 104: /* pp  */
#line 309 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).pp))
    mkr_free_pp(((*yyvaluep).pp));
}
#line 1465 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 105: /* value  */
#line 299 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).value))
    mkr_free_value(((*yyvaluep).value));
}
#line 1474 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 106: /* prefix  */
#line 274 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).string))
    RAPTOR_FREE(char*, ((*yyvaluep).string));
}
#line 1483 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 107: /* viewOption  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1492 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 108: /* nameOption  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1501 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 109: /* subject  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1510 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 110: /* predicate  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1519 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 111: /* object  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1528 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 112: /* collection  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1537 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 113: /* set  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1546 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 114: /* sentence  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1555 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 115: /* predicateObject  */
#line 304 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).nv))
    mkr_free_nv(((*yyvaluep).nv));
}
#line 1564 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 116: /* predicateObjectList  */
#line 289 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).sequence))
    raptor_free_sequence(((*yyvaluep).sequence));
}
#line 1573 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 117: /* objectList  */
#line 289 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).sequence))
    raptor_free_sequence(((*yyvaluep).sequence));
}
#line 1582 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 118: /* nvList  */
#line 289 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).sequence))
    raptor_free_sequence(((*yyvaluep).sequence));
}
#line 1591 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 119: /* ppList  */
#line 289 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).sequence))
    raptor_free_sequence(((*yyvaluep).sequence));
}
#line 1600 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 120: /* variable  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1609 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 121: /* isVerb  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1618 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 122: /* hoVerb  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1627 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 123: /* hasVerb  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1636 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 124: /* doVerb  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1645 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 125: /* preposition  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1654 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 126: /* literal  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1663 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 127: /* resource  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1672 "mkr_parser.c" /* yacc.c:1257  */
        break;

    case 128: /* blankNode  */
#line 284 "./mkr_parser.y" /* yacc.c:1257  */
      {
  if(((*yyvaluep).identifier))
    raptor_free_term(((*yyvaluep).identifier));
}
#line 1681 "mkr_parser.c" /* yacc.c:1257  */
        break;


      default:
        break;
    }
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (raptor_parser* rdf_parser, void* yyscanner)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex (&yylval, yyscanner);
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 5:
#line 324 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.nv) = mkr_new_nv(rdf_parser, MKR_NV, (yyvsp[-2].identifier), (yyvsp[0].value));}
#line 1947 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 6:
#line 327 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.pp) = mkr_new_pp(rdf_parser, MKR_PPOBJ, (yyvsp[-1].identifier), (yyvsp[0].sequence));}
#line 1953 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 7:
#line 328 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.pp) = mkr_new_pp(rdf_parser, MKR_PPNV,  (yyvsp[-1].identifier), (yyvsp[0].sequence));}
#line 1959 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 8:
#line 331 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.value) = mkr_new_value(rdf_parser, MKR_OBJECT,       (yyvsp[0].identifier));}
#line 1965 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 9:
#line 332 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.value) = mkr_new_value(rdf_parser, MKR_SENTENCELIST, (yyvsp[-1].sequence));}
#line 1971 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 10:
#line 337 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.string) = (unsigned char*)"@prefix";}
#line 1977 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 11:
#line 338 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.string) = (unsigned char*)"@mkb";}
#line 1983 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 12:
#line 339 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.string) = (unsigned char*)"@mke";}
#line 1989 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 13:
#line 342 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = (yyvsp[0].identifier);}
#line 1995 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 14:
#line 343 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = NULL;}
#line 2001 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 15:
#line 345 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = (yyvsp[-1].identifier);}
#line 2007 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 16:
#line 346 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = NULL;}
#line 2013 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 17:
#line 350 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2019 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 18:
#line 351 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2025 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 19:
#line 352 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2031 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 20:
#line 353 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2037 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 21:
#line 354 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2043 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 22:
#line 355 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2049 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 23:
#line 358 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2055 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 24:
#line 359 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2061 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 25:
#line 360 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2067 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 26:
#line 363 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2073 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 27:
#line 364 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2079 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 28:
#line 365 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2085 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 29:
#line 366 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2091 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 30:
#line 367 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2097 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 31:
#line 368 "./mkr_parser.y" /* yacc.c:1646  */
    { (yyval.identifier) = (yyvsp[0].identifier); }
#line 2103 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 32:
#line 375 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = raptor_new_term_from_uri(rdf_parser->world, RAPTOR_RDF_nil_URI(rdf_parser->world)); (yyval.identifier)->mkrtype = MKR_LIST;}
#line 2109 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 33:
#line 376 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_rdflist(rdf_parser, (yyvsp[-1].sequence)); (yyval.identifier)->mkrtype = MKR_RDFLIST;}
#line 2115 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 34:
#line 380 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = raptor_new_term_from_uri(rdf_parser->world, RAPTOR_RDF_nil_URI(rdf_parser->world)); (yyval.identifier)->mkrtype = MKR_SET;}
#line 2121 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 35:
#line 381 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_rdflist(rdf_parser, (yyvsp[-1].sequence)); (yyval.identifier)->mkrtype = MKR_OBJECTSET;}
#line 2127 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 36:
#line 389 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_sentence(rdf_parser, (yyvsp[-3].identifier), (yyvsp[-2].identifier), (yyvsp[-1].identifier));}
#line 2133 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 37:
#line 390 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_view(rdf_parser, (yyvsp[-2].sequence));}
#line 2139 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 38:
#line 391 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_base(rdf_parser, (yyvsp[-1].uri));}
#line 2145 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 39:
#line 392 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_prefix(rdf_parser, (yyvsp[-3].string), (yyvsp[-2].string), (yyvsp[-1].uri));}
#line 2151 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 40:
#line 393 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_group(rdf_parser, MKR_BEGIN, (yyvsp[-2].identifier), (yyvsp[-1].identifier));}
#line 2157 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 41:
#line 394 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_ho(rdf_parser, (yyvsp[-1].sequence));}
#line 2163 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 42:
#line 395 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_rel(rdf_parser, (yyvsp[-1].sequence));}
#line 2169 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 43:
#line 396 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_group(rdf_parser, MKR_END, (yyvsp[-2].identifier), (yyvsp[-1].identifier));}
#line 2175 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 44:
#line 397 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_assignment(rdf_parser, (yyvsp[-3].identifier), (yyvsp[-1].value));}
#line 2181 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 45:
#line 398 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_command(rdf_parser, NULL, (yyvsp[-2].identifier), (yyvsp[-1].identifier), NULL);}
#line 2187 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 46:
#line 399 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_command(rdf_parser, NULL, (yyvsp[-3].identifier), (yyvsp[-2].identifier), (yyvsp[-1].sequence));}
#line 2193 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 47:
#line 401 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_action(rdf_parser, (yyvsp[-3].identifier), (yyvsp[-2].identifier), (yyvsp[-1].identifier), NULL);}
#line 2199 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 48:
#line 402 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_action(rdf_parser, (yyvsp[-4].identifier), (yyvsp[-3].identifier), (yyvsp[-2].identifier), (yyvsp[-1].sequence));}
#line 2205 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 49:
#line 403 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_definition(rdf_parser, (yyvsp[-4].identifier), (yyvsp[-3].identifier), (yyvsp[-2].identifier), (yyvsp[-1].sequence));}
#line 2211 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 50:
#line 406 "./mkr_parser.y" /* yacc.c:1646  */
    {mkr_poList(rdf_parser, (yyvsp[-2].identifier), (yyvsp[-1].sequence));}
#line 2217 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 51:
#line 414 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = NULL;}
#line 2223 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 52:
#line 417 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.nv) = mkr_attribute(rdf_parser, NULL, (yyvsp[-1].identifier), (yyvsp[0].nv));}
#line 2229 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 53:
#line 418 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.nv) = mkr_hierarchy(rdf_parser, NULL, (yyvsp[-1].identifier), (yyvsp[0].identifier));}
#line 2235 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 54:
#line 419 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.nv) = mkr_alias(rdf_parser, NULL, (yyvsp[-1].identifier), (yyvsp[0].identifier));}
#line 2241 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 55:
#line 424 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("predicateObjectList 1\n");
  if((yyvsp[0].nv)) {
    printf(" predicateObject=\n");
    mkr_statement_print((yyvsp[0].nv), stdout);
    printf("\n");
  } else  
    printf(" and empty object\n");
  if((yyvsp[-2].sequence)) {
    printf(" predicateObjectList=");
    raptor_sequence_print((yyvsp[-2].sequence), stdout);
    printf("\n");
  } else
    printf(" and empty predicateObjectList\n");
#endif

  if(!(yyvsp[0].nv))
    (yyval.sequence) = NULL;
  else {
    if(raptor_sequence_push((yyvsp[-2].sequence), (yyvsp[0].nv))) {
      raptor_free_sequence((yyvsp[-2].sequence));
      YYERROR;
    }
    (yyval.sequence) = (yyvsp[-2].sequence);
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" predicateObjectList is now ");
    raptor_sequence_print((yyval.sequence), stdout);
    printf("\n\n");
#endif
  }
}
#line 2278 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 56:
#line 457 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("predicateObjectList 2\n");
  if((yyvsp[0].nv)) {
    printf(" predicateObject=\n");
    mkr_statement_print((yyvsp[0].nv), stdout);
    printf("\n");
  } else  
    printf(" and empty object\n");
#endif

  if(!(yyvsp[0].nv))
    (yyval.sequence) = NULL;
  else {
#ifdef RAPTOR_DEBUG
    (yyval.sequence) = raptor_new_sequence((raptor_data_free_handler)raptor_free_statement,
                             (raptor_data_print_handler)mkr_statement_print);
#else
    (yyval.sequence) = raptor_new_sequence((raptor_data_free_handler)raptor_free_statement, NULL);
#endif
    if(!(yyval.sequence)) {
      raptor_free_term((yyvsp[0].nv));
      YYERROR;
    }
    if(raptor_sequence_push((yyval.sequence), (yyvsp[0].nv))) {
      raptor_free_sequence((yyval.sequence));
      (yyval.sequence) = NULL;
      YYERROR;
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" predicateObjectList is now ");
    raptor_sequence_print((yyval.sequence), stdout);
    printf("\n\n");
#endif
  }
}
#line 2319 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 57:
#line 497 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("objectList 1\n");
  if((yyvsp[0].identifier)) {
    printf(" object=\n");
    raptor_term_print_as_ntriples((yyvsp[0].identifier), stdout);
    printf("\n");
  } else  
    printf(" and empty object\n");
  if((yyvsp[-2].sequence)) {
    printf(" objectList=");
    raptor_sequence_print((yyvsp[-2].sequence), stdout);
    printf("\n");
  } else
    printf(" and empty objectList\n");
#endif

  if(!(yyvsp[0].identifier))
    (yyval.sequence) = NULL;
  else {
    if(raptor_sequence_push((yyvsp[-2].sequence), (yyvsp[0].identifier))) {
      raptor_free_sequence((yyvsp[-2].sequence));
      YYERROR;
    }
    (yyval.sequence) = (yyvsp[-2].sequence);
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" objectList is now ");
    raptor_sequence_print((yyval.sequence), stdout);
    printf("\n\n");
#endif
  }
}
#line 2356 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 58:
#line 530 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("objectList 2\n");
  if((yyvsp[0].identifier)) {
    printf(" object=\n");
    raptor_term_print_as_ntriples((yyvsp[0].identifier), stdout);
    printf("\n");
  } else  
    printf(" and empty object\n");
#endif

  if(!(yyvsp[0].identifier))
    (yyval.sequence) = NULL;
  else {
#ifdef RAPTOR_DEBUG
    (yyval.sequence) = raptor_new_sequence((raptor_data_free_handler)raptor_free_term,
                             (raptor_data_print_handler)raptor_term_print_as_ntriples);
#else
    (yyval.sequence) = raptor_new_sequence((raptor_data_free_handler)raptor_free_term, NULL);
#endif
    if(!(yyval.sequence)) {
      raptor_free_term((yyvsp[0].identifier));
      YYERROR;
    }
    if(raptor_sequence_push((yyval.sequence), (yyvsp[0].identifier))) {
      raptor_free_sequence((yyval.sequence));
      (yyval.sequence) = NULL;
      YYERROR;
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" objectList is now ");
    raptor_sequence_print((yyval.sequence), stdout);
    printf("\n\n");
#endif
  }
}
#line 2397 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 59:
#line 641 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("nvList 1\n");
  if((yyvsp[0].nv)) {
    printf(" nv=\n");
    mkr_nv_print((yyvsp[0].nv), stdout);
    printf("\n");
  } else  
    printf(" and empty nv\n");
  if((yyvsp[-2].sequence)) {
    printf(" nvList=");
    raptor_sequence_print((yyvsp[-2].sequence), stdout);
    printf("\n");
  } else
    printf(" and empty nvList\n");
#endif

  if(!(yyvsp[0].nv))
    (yyval.sequence) = NULL;
  else {
    if(raptor_sequence_push((yyvsp[-2].sequence), (yyvsp[0].nv))) {
      raptor_free_sequence((yyvsp[-2].sequence));
      YYERROR;
    }
    (yyval.sequence) = (yyvsp[-2].sequence);
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" nvList is now ");
    raptor_sequence_print((yyval.sequence), stdout);
    printf("\n\n");
#endif
  }
}
#line 2434 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 60:
#line 674 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("nvList 2\n");
  if((yyvsp[0].nv)) {
    printf(" nv=\n");
    mkr_nv_print((yyvsp[0].nv), stdout);
    printf("\n");
  } else  
    printf(" and empty nv\n");
#endif

  if(!(yyvsp[0].nv))
    (yyval.sequence) = NULL;
  else {
#ifdef RAPTOR_DEBUG
    (yyval.sequence) = raptor_new_sequence((raptor_data_free_handler)mkr_free_nv,
                             (raptor_data_print_handler)mkr_nv_print);
#else
    (yyval.sequence) = raptor_new_sequence((raptor_data_free_handler)mkr_free_nv, NULL);
#endif
    if(!(yyval.sequence)) {
      YYERROR;
    }
    if(raptor_sequence_push((yyval.sequence), (yyvsp[0].nv))) {
      raptor_free_sequence((yyval.sequence));
      (yyval.sequence) = NULL;
      YYERROR;
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" nvList is now ");
    raptor_sequence_print((yyval.sequence), stdout);
    printf("\n\n");
#endif
  }
}
#line 2474 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 61:
#line 712 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("ppList 1\n");
  if((yyvsp[0].pp)) {
    printf(" pp=\n");
    mkr_pp_print((yyvsp[0].pp), stdout);
    printf("\n");
  } else  
    printf(" and empty pp\n");
  if((yyvsp[-1].sequence)) {
    printf(" ppList=");
    raptor_sequence_print((yyvsp[-1].sequence), stdout);
    printf("\n");
  } else
    printf(" and empty ppList\n");
#endif

  if(!(yyvsp[0].pp))
    (yyval.sequence) = NULL;
  else {
    if(raptor_sequence_push((yyvsp[-1].sequence), (yyvsp[0].pp))) {
      raptor_free_sequence((yyvsp[-1].sequence));
      YYERROR;
    }
    (yyval.sequence) = (yyvsp[-1].sequence);
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" ppList is now ");
    raptor_sequence_print((yyval.sequence), stdout);
    printf("\n\n");
#endif
  }
}
#line 2511 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 62:
#line 745 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("ppList 2\n");
  if((yyvsp[0].pp)) {
    printf(" pp=\n");
    mkr_pp_print((yyvsp[0].pp), stdout);
    printf("\n");
  } else  
    printf(" and empty pp\n");
#endif

  if(!(yyvsp[0].pp))
    (yyval.sequence) = NULL;
  else {
#ifdef RAPTOR_DEBUG
    (yyval.sequence) = raptor_new_sequence((raptor_data_free_handler)mkr_free_pp,
                             (raptor_data_print_handler)mkr_pp_print);
#else
    (yyval.sequence) = raptor_new_sequence((raptor_data_free_handler)mkr_free_pp, NULL);
#endif
    if(!(yyval.sequence)) {
      YYERROR;
    }
    if(raptor_sequence_push((yyval.sequence), (yyvsp[0].pp))) {
      raptor_free_sequence((yyval.sequence));
      (yyval.sequence) = NULL;
      YYERROR;
    }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
    printf(" ppList is now ");
    raptor_sequence_print((yyval.sequence), stdout);
    printf("\n\n");
#endif
  }
}
#line 2551 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 63:
#line 781 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_variable(rdf_parser, MKR_VARIABLE, (yyvsp[0].string));}
#line 2557 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 64:
#line 784 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_IS, (unsigned char*)"is");}
#line 2563 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 65:
#line 787 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_ISU, (unsigned char*)"isu");}
#line 2569 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 66:
#line 788 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_ISS, (unsigned char*)"iss");}
#line 2575 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 67:
#line 789 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_ISA, (unsigned char*)"isa");}
#line 2581 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 68:
#line 790 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_ISASTAR, (unsigned char*)"isa*");}
#line 2587 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 69:
#line 791 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_ISP, (unsigned char*)"isp");}
#line 2593 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 70:
#line 792 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_ISG, (unsigned char*)"isg");}
#line 2599 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 71:
#line 793 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_ISC, (unsigned char*)"isc");}
#line 2605 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 72:
#line 794 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_ISCSTAR, (unsigned char*)"isc*");}
#line 2611 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 73:
#line 795 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = (yyvsp[0].identifier);}
#line 2617 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 74:
#line 798 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_HAS,     (unsigned char*)"has");}
#line 2623 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 75:
#line 799 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_HASPART, (unsigned char*)"haspart");}
#line 2629 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 76:
#line 800 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_ISAPART, (unsigned char*)"isapart");}
#line 2635 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 77:
#line 803 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_DO,   (unsigned char*)"do");}
#line 2641 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 78:
#line 804 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_HDO,  (unsigned char*)"hdo");}
#line 2647 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 79:
#line 805 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_verb(rdf_parser, MKR_BANG, (unsigned char*)"!");}
#line 2653 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 80:
#line 808 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_preposition(rdf_parser, MKR_AT,    (unsigned char*)"at");}
#line 2659 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 81:
#line 809 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_preposition(rdf_parser, MKR_OF,    (unsigned char*)"of");}
#line 2665 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 82:
#line 810 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_preposition(rdf_parser, MKR_WITH,  (unsigned char*)"with");}
#line 2671 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 83:
#line 811 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_preposition(rdf_parser, MKR_OD,    (unsigned char*)"od");}
#line 2677 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 84:
#line 812 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_preposition(rdf_parser, MKR_FROM,  (unsigned char*)"from");}
#line 2683 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 85:
#line 813 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_preposition(rdf_parser, MKR_TO,    (unsigned char*)"to");}
#line 2689 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 86:
#line 814 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_preposition(rdf_parser, MKR_IN,    (unsigned char*)"in");}
#line 2695 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 87:
#line 815 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_preposition(rdf_parser, MKR_OUT,   (unsigned char*)"out");}
#line 2701 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 88:
#line 816 "./mkr_parser.y" /* yacc.c:1646  */
    {(yyval.identifier) = mkr_new_preposition(rdf_parser, MKR_WHERE, (unsigned char*)"where");}
#line 2707 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 89:
#line 822 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("literal + language string=\"%s\"\n", (yyvsp[-1].string));
#endif

  (yyval.identifier) = raptor_new_term_from_literal(rdf_parser->world, (yyvsp[-1].string), NULL, (yyvsp[0].string));
  RAPTOR_FREE(char*, (yyvsp[-1].string));
  RAPTOR_FREE(char*, (yyvsp[0].string));
  if(!(yyval.identifier))
    YYERROR;
}
#line 2723 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 90:
#line 834 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("literal + language=\"%s\" datatype string=\"%s\" uri=\"%s\"\n", (yyvsp[-3].string), (yyvsp[-2].string), raptor_uri_as_string((yyvsp[0].uri)));
#endif

  if((yyvsp[0].uri)) {
    if((yyvsp[-2].string)) {
      raptor_parser_error(rdf_parser,
                          "Language not allowed with datatyped literal");
      RAPTOR_FREE(char*, (yyvsp[-2].string));
      (yyvsp[-2].string) = NULL;
    }
  
    (yyval.identifier) = raptor_new_term_from_literal(rdf_parser->world, (yyvsp[-3].string), (yyvsp[0].uri), NULL);
    RAPTOR_FREE(char*, (yyvsp[-3].string));
    raptor_free_uri((yyvsp[0].uri));
    if(!(yyval.identifier))
      YYERROR;
  } else
    (yyval.identifier) = NULL;
    
}
#line 2750 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 91:
#line 857 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("literal + language=\"%s\" datatype string=\"%s\" qname URI=<%s>\n", (yyvsp[-3].string), (yyvsp[-2].string), raptor_uri_as_string((yyvsp[0].uri)));
#endif

  if((yyvsp[0].uri)) {
    if((yyvsp[-2].string)) {
      raptor_parser_error(rdf_parser,
                          "Language not allowed with datatyped literal");
      RAPTOR_FREE(char*, (yyvsp[-2].string));
      (yyvsp[-2].string) = NULL;
    }
  
    (yyval.identifier) = raptor_new_term_from_literal(rdf_parser->world, (yyvsp[-3].string), (yyvsp[0].uri), NULL);
    RAPTOR_FREE(char*, (yyvsp[-3].string));
    raptor_free_uri((yyvsp[0].uri));
    if(!(yyval.identifier))
      YYERROR;
  } else
    (yyval.identifier) = NULL;

}
#line 2777 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 92:
#line 880 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("literal + datatype string=\"%s\" uri=\"%s\"\n", (yyvsp[-2].string), raptor_uri_as_string((yyvsp[0].uri)));
#endif

  if((yyvsp[0].uri)) {
    (yyval.identifier) = raptor_new_term_from_literal(rdf_parser->world, (yyvsp[-2].string), (yyvsp[0].uri), NULL);
    RAPTOR_FREE(char*, (yyvsp[-2].string));
    raptor_free_uri((yyvsp[0].uri));
    if(!(yyval.identifier))
      YYERROR;
  } else
    (yyval.identifier) = NULL;
    
}
#line 2797 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 93:
#line 896 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("literal + datatype string=\"%s\" qname URI=<%s>\n", (yyvsp[-2].string), raptor_uri_as_string((yyvsp[0].uri)));
#endif

  if((yyvsp[0].uri)) {
    (yyval.identifier) = raptor_new_term_from_literal(rdf_parser->world, (yyvsp[-2].string), (yyvsp[0].uri), NULL);
    RAPTOR_FREE(char*, (yyvsp[-2].string));
    raptor_free_uri((yyvsp[0].uri));
    if(!(yyval.identifier))
      YYERROR;
  } else
    (yyval.identifier) = NULL;
}
#line 2816 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 94:
#line 911 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("literal string=\"%s\"\n", (yyvsp[0].string));
#endif

  (yyval.identifier) = raptor_new_term_from_literal(rdf_parser->world, (yyvsp[0].string), NULL, NULL);
  RAPTOR_FREE(char*, (yyvsp[0].string));
  if(!(yyval.identifier))
    YYERROR;
}
#line 2831 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 95:
#line 922 "./mkr_parser.y" /* yacc.c:1646  */
    {
  raptor_uri *uri;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("resource integer=%s\n", (yyvsp[0].string));
#endif
  uri = raptor_uri_copy(rdf_parser->world->xsd_integer_uri);
  (yyval.identifier) = raptor_new_term_from_literal(rdf_parser->world, (yyvsp[0].string), uri, NULL);
  RAPTOR_FREE(char*, (yyvsp[0].string));
  raptor_free_uri(uri);
  if(!(yyval.identifier))
    YYERROR;
}
#line 2848 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 96:
#line 935 "./mkr_parser.y" /* yacc.c:1646  */
    {
  raptor_uri *uri;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("resource double=%s\n", (yyvsp[0].string));
#endif
  uri = raptor_uri_copy(rdf_parser->world->xsd_double_uri);
  (yyval.identifier) = raptor_new_term_from_literal(rdf_parser->world, (yyvsp[0].string), uri, NULL);
  RAPTOR_FREE(char*, (yyvsp[0].string));
  raptor_free_uri(uri);
  if(!(yyval.identifier))
    YYERROR;
}
#line 2865 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 97:
#line 948 "./mkr_parser.y" /* yacc.c:1646  */
    {
  raptor_uri *uri;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("resource decimal=%s\n", (yyvsp[0].string));
#endif
  uri = raptor_uri_copy(rdf_parser->world->xsd_decimal_uri);
  if(!uri) {
    RAPTOR_FREE(char*, (yyvsp[0].string));
    YYERROR;
  }
  (yyval.identifier) = raptor_new_term_from_literal(rdf_parser->world, (yyvsp[0].string), uri, NULL);
  RAPTOR_FREE(char*, (yyvsp[0].string));
  raptor_free_uri(uri);
  if(!(yyval.identifier))
    YYERROR;
}
#line 2886 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 98:
#line 965 "./mkr_parser.y" /* yacc.c:1646  */
    {
  raptor_uri *uri;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  fputs("resource boolean true\n", stderr);
#endif
  uri = raptor_uri_copy(rdf_parser->world->xsd_boolean_uri);
  (yyval.identifier) = raptor_new_term_from_literal(rdf_parser->world,
                                    (const unsigned char*)"true", uri, NULL);
  raptor_free_uri(uri);
  if(!(yyval.identifier))
    YYERROR;
}
#line 2903 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 99:
#line 978 "./mkr_parser.y" /* yacc.c:1646  */
    {
  raptor_uri *uri;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  fputs("resource boolean false\n", stderr);
#endif
  uri = raptor_uri_copy(rdf_parser->world->xsd_boolean_uri);
  (yyval.identifier) = raptor_new_term_from_literal(rdf_parser->world,
                                    (const unsigned char*)"false", uri, NULL);
  raptor_free_uri(uri);
  if(!(yyval.identifier))
    YYERROR;
}
#line 2920 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 100:
#line 994 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("resource URI=<%s>\n", raptor_uri_as_string((yyvsp[0].uri)));
#endif

  if((yyvsp[0].uri)) {
    (yyval.identifier) = raptor_new_term_from_uri(rdf_parser->world, (yyvsp[0].uri));
    raptor_free_uri((yyvsp[0].uri));
    if(!(yyval.identifier))
      YYERROR;
  } else
    (yyval.identifier) = NULL;
}
#line 2938 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 101:
#line 1008 "./mkr_parser.y" /* yacc.c:1646  */
    {
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("resource qname URI=<%s>\n", raptor_uri_as_string((yyvsp[0].uri)));
#endif

  if((yyvsp[0].uri)) {
    (yyval.identifier) = raptor_new_term_from_uri(rdf_parser->world, (yyvsp[0].uri));
    raptor_free_uri((yyvsp[0].uri));
    if(!(yyval.identifier))
      YYERROR;
  } else
    (yyval.identifier) = NULL;
}
#line 2956 "mkr_parser.c" /* yacc.c:1646  */
    break;

  case 102:
#line 1026 "./mkr_parser.y" /* yacc.c:1646  */
    {
  const unsigned char *id;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1  
  printf("subject blank=\"%s\"\n", (yyvsp[0].string));
#endif
  id = raptor_world_internal_generate_id(rdf_parser->world, (yyvsp[0].string));
  if(!id)
    YYERROR;

  (yyval.identifier) = raptor_new_term_from_blank(rdf_parser->world, id);
  RAPTOR_FREE(char*, id);

  if(!(yyval.identifier))
    YYERROR;
}
#line 2976 "mkr_parser.c" /* yacc.c:1646  */
    break;


#line 2980 "mkr_parser.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (rdf_parser, yyscanner, YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
if(yytoken < 0) yytoken = YYUNDEFTOK;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
if(yytoken < 0) yytoken = YYUNDEFTOK;
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (rdf_parser, yyscanner, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval, rdf_parser, yyscanner);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, rdf_parser, yyscanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (rdf_parser, yyscanner, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, rdf_parser, yyscanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, rdf_parser, yyscanner);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1101 "./mkr_parser.y" /* yacc.c:1906  */


/******************************************************************************/
/******************************************************************************/
/* Support functions */

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
mkr_new_preposition(raptor_parser *parser, mkr_type type, unsigned char* name)
{
  return mkr_new_variable(parser, type, name);
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


/*****************************************************************************/
raptor_term*
mkr_new_blankTerm(raptor_parser* rdf_parser)
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
  printf("##### DEBUG: mkr_new_blankTerm: ");
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
/* nv: predicate EQUALS value; */
MKR_nv*
mkr_new_nv(raptor_parser* rdf_parser, mkr_type type, raptor_term* predicate, MKR_value* value)
{
  raptor_world* world = rdf_parser->world;
  MKR_nv* nv = NULL;
  const char* separator = "=";

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_new_nv: ");
  raptor_term_print_as_ntriples(predicate, stdout);
  printf(" %s ", separator);
  mkr_value_print(value, stdout);
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
      nv->nvValue = value;
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
        mkr_free_value(nv->nvValue);
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
  printf("exit\n");
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
      mkr_value_print(nv->nvValue, stdout);
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
mkr_new_pp(raptor_parser* rdf_parser, mkr_type type, raptor_term* preposition, raptor_sequence* value)
{
  raptor_world* world = rdf_parser->world;
  MKR_pp* pp = NULL;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_new_pp: ");
  raptor_term_print_as_ntriples(preposition, stdout);
  printf(" ");
  raptor_sequence_print(value, stdout);
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
      pp->ppSequence = value;
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
  printf("exit\n");
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

/*****************************************************************************/
/*****************************************************************************/
MKR_value*
mkr_new_value(raptor_parser* rdf_parser, mkr_type type, mkr_value* value)
{
  raptor_world* world = rdf_parser->world;
  MKR_value* val = NULL;
  raptor_term* rdflist = NULL;

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_new_value: ");
  printf("%s(", MKR_TYPE_NAME(type));
  switch(type) {
    case MKR_NIL:
      printf("( )");
      break;

    case MKR_OBJECT:
    case MKR_RDFLIST:
    case MKR_SENTENCE:
      raptor_term_print_as_ntriples((raptor_term*)value, stdout);
      break;

    case MKR_VALUESET:
    case MKR_OBJECTSET:
    case MKR_OBJECTLIST:
    case MKR_SENTENCELIST:
      raptor_sequence_print((raptor_sequence*)value, stdout);
      break;

    default:
    case MKR_RAPTOR_TERM:
    case MKR_RAPTOR_SEQUENCE:
    case MKR_VALUELIST:
    case MKR_NVLIST:
    case MKR_PPLIST:
      printf("##### ERROR: mkr_new_value: unexpected mkrtype: %s\n", MKR_TYPE_NAME(type));
      break;
  }
  printf(")\n");
#endif
  RAPTOR_CHECK_CONSTRUCTOR_WORLD(world);
  raptor_world_open(world);
  val = RAPTOR_CALLOC(MKR_value*, 1, sizeof(*val));
  if(!val) {
    printf("##### ERROR: mkr_new_value: can't create MKR_value\n");
    return NULL;
  }
  val->mkrtype = type;
  switch(type) {
    case MKR_NIL:
      /* if(!val)
        YYERROR; */
      val->mkrvalue = (void*)raptor_new_term_from_uri(world, RAPTOR_RDF_nil_URI(world));
      break;

    case MKR_OBJECT:
    case MKR_RDFLIST:
    case MKR_RDFSET:
    case MKR_SENTENCE:
      val->mkrvalue = (void*)raptor_term_copy((raptor_term*)value);
      break;

    case MKR_VALUELIST:
    case MKR_OBJECTLIST:
      val->mkrtype = MKR_RDFLIST;
      rdflist = mkr_new_rdflist(rdf_parser, (raptor_sequence*)value);
      val->mkrvalue = (void*)raptor_term_copy(rdflist);
      break;

    case MKR_VALUESET:
    case MKR_OBJECTSET:
    case MKR_SENTENCELIST:
      val->mkrvalue = (void*)value;
      break;

    default:
    case MKR_RAPTOR_TERM:
    case MKR_RAPTOR_SEQUENCE:
    case MKR_NVLIST:
    case MKR_PPLIST:
      printf("##### ERROR: mkr_new_value: unexpected mkrtype: %s\n", MKR_TYPE_NAME(type));
      val->mkrvalue = NULL;
      break;
  }

  return val;
}


/*****************************************************************************/
void
mkr_free_value(MKR_value* value)
{
  mkr_type type = MKR_UNKNOWN;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("##### DEBUG: mkr_free_value: ");
#endif
  if(!value)
    return;
  type = value->mkrtype;
  if(value->mkrvalue) {
    switch(type) {
      case MKR_NIL:
      case MKR_OBJECT:
      case MKR_RDFLIST:
      case MKR_RDFSET:
      case MKR_SENTENCE:
        raptor_free_term((raptor_term*)value->mkrvalue);
        value->mkrvalue = NULL;
        break;
  
      case MKR_SENTENCELIST:
        raptor_free_sequence((raptor_sequence*)value->mkrvalue);
        value->mkrvalue = NULL;
        break;
  
      default:
      case MKR_RAPTOR_TERM:
      case MKR_RAPTOR_SEQUENCE:
      case MKR_OBJECTSET:
      case MKR_OBJECTLIST:
      case MKR_VALUESET:
      case MKR_VALUELIST:
      case MKR_NVLIST:
      case MKR_PPLIST:
        printf("##### ERROR: mkr_free_value: unexpected mkrtype: %s\n", MKR_TYPE_NAME(type));
        break;
    }
  }
  /* RAPTOR_FREE(MKR_value, value);  causes core dump */
  value = NULL;
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("exit mkr_free_value\n");
#endif
}


/*****************************************************************************/
void
mkr_value_print(MKR_value* value, FILE* fh)
{
  mkr_type type = value->mkrtype;

  printf("%s(", MKR_TYPE_NAME(type));
  switch(type) {
    case MKR_NIL:
    case MKR_OBJECT:
    case MKR_RDFLIST:
    case MKR_RDFSET:
    case MKR_SENTENCE:
      raptor_term_print_as_ntriples((raptor_term*)value->mkrvalue, stdout);
      break;

    case MKR_VALUESET:
    case MKR_OBJECTSET:
    case MKR_SENTENCELIST:
      raptor_sequence_print((raptor_sequence*)value->mkrvalue, stdout);
      break;

    default:
    case MKR_RAPTOR_TERM:
    case MKR_RAPTOR_SEQUENCE:
    case MKR_OBJECTLIST:
    case MKR_VALUELIST:
    case MKR_NVLIST:
    case MKR_PPLIST:
      printf("(not value: %s)", MKR_TYPE_NAME(type));
      break;
  }
  printf(")");
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

/*****************************************************************************/
void
mkr_rdflist_print(raptor_term* rdflist, FILE* fh)
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

  sentence = mkr_new_blankTerm(rdf_parser);
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

  mkr_parser->trig = !strcmp(name, "trig");

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


void
raptor_mkr_generate_statement(raptor_parser *parser, raptor_statement *t)
{
  raptor_turtle_parser *mkr_parser = (raptor_turtle_parser*)parser->context;
  raptor_statement *statement = &parser->statement;

  if(!t) {
    printf("##### ERROR: raptor_mkr_generate_statement: ignore NULL statement\n");
    return;
  }

  if(!t->subject) {
    printf("##### WARNING: raptor_mkr_generate_statement: ignore NULL subject\n");
    return;
  }
  if(!t->predicate) {
    printf("##### WARNING: raptor_mkr_generate_statement: ignore NULL predicate\n");
    return;
  }
  if(!t->object) {
    printf("##### WARNING: raptor_mkr_generate_statement: ignore NULL object\n");
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
  printf("##### DEBUG: raptor_mkr_generate_statement: ");
  mkr_statement_print(t, stdout);
  printf("\n");
#endif

  /* Two choices for subject for Turtle */
  switch(t->subject->type) {
    case RAPTOR_TERM_TYPE_BLANK:
      statement->subject = raptor_new_term_from_blank(parser->world,
                                                    t->subject->value.blank.string);
      break;

    case RAPTOR_TERM_TYPE_URI:
      RAPTOR_ASSERT((t->subject->type != RAPTOR_TERM_TYPE_URI),
                  "subject type is not resource");
      statement->subject = raptor_new_term_from_uri(parser->world,
                                                  t->subject->value.uri);
      break;

    case RAPTOR_TERM_TYPE_LITERAL:
    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
/* #if defined(RAPTOR_DEBUG) */
      printf("##### ERROR: raptor_mkr_generate_statement: unexpected default: subject->type = %s\n", RAPTOR_TERM_TYPE_NAME(t->subject->type));
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
      break;

    case RAPTOR_TERM_TYPE_BLANK:
      if((t->predicate->mkrtype == MKR_DEFINITION) ||
         (t->predicate->mkrtype == MKR_ACTION) ||
         (t->predicate->mkrtype == MKR_COMMAND)
         ) {
        statement->predicate = raptor_new_term_from_blank(parser->world,
                                                    t->predicate->value.blank.string);
      } else {
        printf("##### WARNING: unexpected blank predicate->mkrtype: %s\n", MKR_TYPE_NAME(t->predicate->mkrtype));
      }
      break;

    case RAPTOR_TERM_TYPE_LITERAL:
    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
/* #if defined(RAPTOR_DEBUG) */
      printf("##### ERROR: raptor_mkr_generate_statement: unexpected default: predicate->type = %s\n", RAPTOR_TERM_TYPE_NAME(t->predicate->type));
/* #endif */
      break;
  }
  

  /* Three choices for object for Turtle */
  switch(t->object->type) {
    case RAPTOR_TERM_TYPE_URI:
      statement->object = raptor_new_term_from_uri(parser->world,
                                                 t->object->value.uri);
      break;

    case RAPTOR_TERM_TYPE_BLANK:
      statement->object = raptor_new_term_from_blank(parser->world,
                                                   t->object->value.blank.string);
      break;

    case RAPTOR_TERM_TYPE_LITERAL:
      RAPTOR_ASSERT(t->object->type != RAPTOR_TERM_TYPE_LITERAL,
                  "object type is not literal");
      statement->object = raptor_new_term_from_literal(parser->world,
                                                     t->object->value.literal.string,
                                                     t->object->value.literal.datatype,
                                                     t->object->value.literal.language);
      break;

    case RAPTOR_TERM_TYPE_UNKNOWN:
    default:
/* #if defined(RAPTOR_DEBUG) */
      printf("##### ERROR: raptor_mkr_generate_statement: unexpected default: object->type = %s\n", RAPTOR_TERM_TYPE_NAME(t->object->type));
/* #endif */
      break;
  }

  /* Generate the statement */
  (*parser->statement_handler)(parser->user_data, statement);

#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf(" free spo: ");
#endif
  raptor_free_term(statement->subject); statement->subject = NULL;
  raptor_free_term(statement->predicate); statement->predicate = NULL;
  raptor_free_term(statement->object); statement->object = NULL;
  if(statement->graph) {
    raptor_free_term(statement->graph); statement->graph = NULL;
  }
#if defined(RAPTOR_DEBUG) && RAPTOR_DEBUG > 1
  printf("exit raptor_mkr_generate_statement\n");
#endif
}



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
  mkr_statement_print(statement, stream);
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
