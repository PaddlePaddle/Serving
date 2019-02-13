/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* Written by Richard Stallman by simplifying the original so called
   ``semantic'' parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 1

/* Using locations.  */
#define YYLSP_NEEDED 0

/* If NAME_PREFIX is specified substitute the variables and functions
   names.  */
#define yyparse cfgidlparse
#define yylex   cfgidllex
#define yyerror cfgidlerror
#define yylval  cfgidllval
#define yychar  cfgidlchar
#define yydebug cfgidldebug
#define yynerrs cfgidlnerrs


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     KW_OVERWRITE = 258,
     KW_STRUCT = 259,
     KW_RAW = 260,
     KW_CHAR = 261,
     KW_UCHAR = 262,
     KW_INT8 = 263,
     KW_UINT8 = 264,
     KW_INT16 = 265,
     KW_UINT16 = 266,
     KW_INT32 = 267,
     KW_UINT32 = 268,
     KW_INT64 = 269,
     KW_UINT64 = 270,
     KW_FLOAT = 271,
     KW_DOUBLE = 272,
     KW_STRING = 273,
     INTEGER = 274,
     ID = 275,
     LITERAL_STRING = 276,
     DOUBLE = 277,
     VAR_CMD = 278
   };
#endif
#define KW_OVERWRITE 258
#define KW_STRUCT 259
#define KW_RAW 260
#define KW_CHAR 261
#define KW_UCHAR 262
#define KW_INT8 263
#define KW_UINT8 264
#define KW_INT16 265
#define KW_UINT16 266
#define KW_INT32 267
#define KW_UINT32 268
#define KW_INT64 269
#define KW_UINT64 270
#define KW_FLOAT 271
#define KW_DOUBLE 272
#define KW_STRING 273
#define INTEGER 274
#define ID 275
#define LITERAL_STRING 276
#define DOUBLE 277
#define VAR_CMD 278




/* Copy the first part of user declarations.  */
#line 5 "idl.gram"

#include <stdio.h>
#include <stdlib.h>
#include "idl.h"
/* to cancel the flex internal definition of idllex:-) */
#define  YY_DECL
#include "idl_gram.h"
#include "idl_lex.h"
  extern int cfgidllex(YYSTYPE* yylval,yyscan_t scanner,
		    meta_t *loc,idl_t *idl);
  static void cfgidlerror(yyscan_t scanner,meta_t *loc,
		       idl_t *idl,char *msg);


/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 40 "idl.gram"
typedef union YYSTYPE {
  meta_t *meta;
  var_t *var;
  group_t *grp;
  cf_list_t *cf_list;
  meta_list_t *meta_list;
  var_list_t *var_list;
  cf_t *cf;
} YYSTYPE;
/* Line 191 of yacc.c.  */
#line 155 "idl_gram.c"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



/* Copy the second part of user declarations.  */


/* Line 214 of yacc.c.  */
#line 167 "idl_gram.c"

#if ! defined (yyoverflow) || YYERROR_VERBOSE

# ifndef YYFREE
#  define YYFREE free
# endif
# ifndef YYMALLOC
#  define YYMALLOC malloc
# endif

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   define YYSTACK_ALLOC alloca
#  endif
# else
#  if defined (alloca) || defined (_ALLOCA_H)
#   define YYSTACK_ALLOC alloca
#  else
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
# endif
#endif /* ! defined (yyoverflow) || YYERROR_VERBOSE */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (defined (YYSTYPE_IS_TRIVIAL) && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
  };

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined (__GNUC__) && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif

#if defined (__STDC__) || defined (__cplusplus)
   typedef signed char yysigned_char;
#else
   typedef short yysigned_char;
#endif

/* YYFINAL -- State number of the termination state. */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   84

/* YYNTOKENS -- Number of terminals. */
#define YYNTOKENS  33
/* YYNNTS -- Number of nonterminals. */
#define YYNNTS  13
/* YYNRULES -- Number of rules. */
#define YYNRULES  41
/* YYNRULES -- Number of states. */
#define YYNSTATES  62

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   278

#define YYTRANSLATE(YYX) 						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const unsigned char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      29,    30,     2,     2,    28,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,    24,
       2,    27,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    25,     2,    26,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    31,     2,    32,     2,     2,     2,     2,
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
      15,    16,    17,    18,    19,    20,    21,    22,    23
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const unsigned char yyprhs[] =
{
       0,     0,     3,     6,     9,    10,    17,    19,    20,    25,
      29,    30,    32,    34,    36,    38,    40,    42,    44,    46,
      48,    50,    52,    54,    56,    58,    60,    63,    64,    68,
      70,    75,    79,    83,    85,    87,    89,    91,    93,   100,
     108,   111
};

/* YYRHS -- A `-1'-separated list of the rules' RHS. */
static const yysigned_char yyrhs[] =
{
      34,     0,    -1,    34,    35,    -1,    34,    44,    -1,    -1,
      38,    20,    37,    39,    24,    36,    -1,    23,    -1,    -1,
      37,    25,    19,    26,    -1,    37,    25,    26,    -1,    -1,
       5,    -1,     6,    -1,     7,    -1,     8,    -1,     9,    -1,
      10,    -1,    11,    -1,    12,    -1,    13,    -1,    14,    -1,
      15,    -1,    16,    -1,    17,    -1,    18,    -1,    20,    -1,
      27,    40,    -1,    -1,    40,    28,    41,    -1,    41,    -1,
      20,    29,    42,    30,    -1,    20,    29,    30,    -1,    42,
      28,    43,    -1,    43,    -1,    21,    -1,    20,    -1,    19,
      -1,    22,    -1,     4,    20,    31,    45,    32,    24,    -1,
      20,     3,    20,    31,    45,    32,    24,    -1,    45,    35,
      -1,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const unsigned short yyrline[] =
{
       0,    77,    77,    81,    85,    89,   142,   151,   158,   177,
     188,   196,   197,   198,   199,   200,   201,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   214,   220,   227,   234,
     245,   254,   266,   273,   284,   285,   286,   287,   291,   301,
     314,   322
};
#endif

#if YYDEBUG || YYERROR_VERBOSE
/* YYTNME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals. */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "KW_OVERWRITE", "KW_STRUCT", "KW_RAW",
  "KW_CHAR", "KW_UCHAR", "KW_INT8", "KW_UINT8", "KW_INT16", "KW_UINT16",
  "KW_INT32", "KW_UINT32", "KW_INT64", "KW_UINT64", "KW_FLOAT",
  "KW_DOUBLE", "KW_STRING", "INTEGER", "ID", "LITERAL_STRING", "DOUBLE",
  "VAR_CMD", "';'", "'['", "']'", "'='", "','", "'('", "')'", "'{'", "'}'",
  "$accept", "program", "variable", "optional_cmd", "optional_array",
  "type_name", "optional_constraint", "constraint_list",
  "constraint_function", "constraint_function_param_list",
  "cons_func_param", "group", "variable_list", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const unsigned short yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,    59,    91,    93,    61,    44,    40,
      41,   123,   125
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const unsigned char yyr1[] =
{
       0,    33,    34,    34,    34,    35,    36,    36,    37,    37,
      37,    38,    38,    38,    38,    38,    38,    38,    38,    38,
      38,    38,    38,    38,    38,    38,    39,    39,    40,    40,
      41,    41,    42,    42,    43,    43,    43,    43,    44,    44,
      45,    45
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const unsigned char yyr2[] =
{
       0,     2,     2,     2,     0,     6,     1,     0,     4,     3,
       0,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     0,     3,     1,
       4,     3,     3,     1,     1,     1,     1,     1,     6,     7,
       2,     0
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const unsigned char yydefact[] =
{
       4,     0,     1,     0,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     2,
       0,     3,     0,     0,    10,    41,     0,    27,     0,    41,
       0,     0,     0,    25,     0,    40,     0,     0,     9,     0,
      26,    29,     7,    38,     0,     8,     0,     0,     6,     5,
      39,    36,    35,    34,    37,    31,     0,    33,    28,     0,
      30,    32
};

/* YYDEFGOTO[NTERM-NUM]. */
static const yysigned_char yydefgoto[] =
{
      -1,     1,    35,    49,    27,    20,    32,    40,    41,    56,
      57,    21,    28
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -7
static const yysigned_char yypact[] =
{
      -7,    52,    -7,    -6,    -7,    -7,    -7,    -7,    -7,    -7,
      -7,    -7,    -7,    -7,    -7,    -7,    -7,    -7,    18,    -7,
       2,    -7,    15,     5,    -7,    -7,    16,    -1,    -5,    -7,
      -3,    25,    24,    -7,    26,    -7,    23,    27,    -7,    20,
      43,    -7,    28,    -7,    30,    -7,    54,    25,    -7,    -7,
      -7,    -7,    -7,    -7,    -7,    -7,    14,    -7,    -7,    -2,
      -7,    -7
};

/* YYPGOTO[NTERM-NUM].  */
static const yysigned_char yypgoto[] =
{
      -7,    -7,    76,    -7,    -7,    -7,    -7,    -7,    31,    -7,
      21,    -7,    50
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const unsigned char yytable[] =
{
       4,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    22,    33,    37,    51,    52,    53,
      54,    23,    24,    38,    30,    26,    31,    34,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    59,    33,    60,    39,    25,    29,    42,    46,
      43,    48,     2,    45,    50,    44,     3,     4,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    47,    18,    51,    52,    53,    54,    19,    58,    36,
      61,     0,     0,     0,    55
};

static const yysigned_char yycheck[] =
{
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    20,    20,    19,    19,    20,    21,
      22,     3,    20,    26,    25,    20,    27,    32,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    28,    20,    30,    20,    31,    31,    24,    29,
      24,    23,     0,    26,    24,    32,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    28,    20,    19,    20,    21,    22,     1,    47,    29,
      59,    -1,    -1,    -1,    30
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const unsigned char yystos[] =
{
       0,    34,     0,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    20,    35,
      38,    44,    20,     3,    20,    31,    20,    37,    45,    31,
      25,    27,    39,    20,    32,    35,    45,    19,    26,    20,
      40,    41,    24,    24,    32,    26,    29,    28,    23,    36,
      24,    19,    20,    21,    22,    30,    42,    43,    41,    28,
      30,    43
};

#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror (scanner, loc, idl, "syntax error: cannot back up");\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)		\
   ((Current).first_line   = (Rhs)[1].first_line,	\
    (Current).first_column = (Rhs)[1].first_column,	\
    (Current).last_line    = (Rhs)[N].last_line,	\
    (Current).last_column  = (Rhs)[N].last_column)
#endif

/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (&yylval, YYLEX_PARAM)
#else
# define YYLEX yylex (&yylval, scanner, loc, idl)
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)

# define YYDSYMPRINT(Args)			\
do {						\
  if (yydebug)					\
    yysymprint Args;				\
} while (0)

# define YYDSYMPRINTF(Title, Token, Value, Location)		\
do {								\
  if (yydebug)							\
    {								\
      YYFPRINTF (stderr, "%s ", Title);				\
      yysymprint (stderr, 					\
                  Token, Value);	\
      YYFPRINTF (stderr, "\n");					\
    }								\
} while (0)

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_stack_print (short *bottom, short *top)
#else
static void
yy_stack_print (bottom, top)
    short *bottom;
    short *top;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (/* Nothing. */; bottom <= top; ++bottom)
    YYFPRINTF (stderr, " %d", *bottom);
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yy_reduce_print (int yyrule)
#else
static void
yy_reduce_print (yyrule)
    int yyrule;
#endif
{
  int yyi;
  unsigned int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %u), ",
             yyrule - 1, yylno);
  /* Print the symbols being reduced, and their result.  */
  for (yyi = yyprhs[yyrule]; 0 <= yyrhs[yyi]; yyi++)
    YYFPRINTF (stderr, "%s ", yytname [yyrhs[yyi]]);
  YYFPRINTF (stderr, "-> %s\n", yytname [yyr1[yyrule]]);
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (Rule);		\
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YYDSYMPRINT(Args)
# define YYDSYMPRINTF(Title, Token, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if defined (YYMAXDEPTH) && YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

#endif /* !YYERROR_VERBOSE */



#if YYDEBUG
/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yysymprint (FILE *yyoutput, int yytype, YYSTYPE *yyvaluep)
#else
static void
yysymprint (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  if (yytype < YYNTOKENS)
    {
      YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
# ifdef YYPRINT
      YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
    }
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  switch (yytype)
    {
      default:
        break;
    }
  YYFPRINTF (yyoutput, ")");
}

#endif /* ! YYDEBUG */
/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

#if defined (__STDC__) || defined (__cplusplus)
static void
yydestruct (int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yytype, yyvaluep)
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  /* Pacify ``unused variable'' warnings.  */
  (void) yyvaluep;

  switch (yytype)
    {

      default:
        break;
    }
}


/* Prevent warnings from -Wmissing-prototypes.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM);
# else
int yyparse ();
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int yyparse (yyscan_t scanner, meta_t *loc, idl_t *idl);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */






/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
int yyparse (void *YYPARSE_PARAM)
# else
int yyparse (YYPARSE_PARAM)
  void *YYPARSE_PARAM;
# endif
#else /* ! YYPARSE_PARAM */
#if defined (__STDC__) || defined (__cplusplus)
int
yyparse (yyscan_t scanner, meta_t *loc, idl_t *idl)
#else
int
yyparse (scanner, loc, idl)
    yyscan_t scanner;
    meta_t *loc;
    idl_t *idl;
#endif
#endif
{
  /* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack.  */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;



#define YYPOPSTACK   (yyvsp--, yyssp--)

  YYSIZE_T yystacksize = YYINITDEPTH;

  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;


  /* When reducing, the number of symbols on the RHS of the reduced
     rule.  */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;


	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),

		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);

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

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YYDSYMPRINTF ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %s, ", yytname[yytoken]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;


  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
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
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 78 "idl.gram"
    {
  add_idl_var(idl,yyvsp[0].var);
;}
    break;

  case 3:
#line 82 "idl.gram"
    {
  add_idl_group(idl,yyvsp[0].grp);
;}
    break;

  case 5:
#line 90 "idl.gram"
    {
  meta_t *type=yyvsp[-5].meta,*id=yyvsp[-4].meta;
  meta_list_t *array_size_list=yyvsp[-3].meta_list;
  meta_list_t *cmdval = yyvsp[0].meta_list;
  cf_list_t *cf_list=yyvsp[-2].cf_list;
  cf_t *cf;

  var_t *var=alloc_var(idl);

  //allocate the necessary space 
  if(cf_list==NULL) {
  	cf_list=new cf_list_t;
  	idl->parse_list.push_back(cf_list);
  }
  if(array_size_list==NULL) {
  	array_size_list=new meta_list_t;
  	idl->parse_meta_list.push_back(array_size_list);
  }

  if(array_size_list->size()!=0){
    /* the arary size list does exist */
    cf=alloc_cf(idl);

    meta_t func;
    func.data="array";
    func.file=id->file;
    func.lineno=id->lineno;
    assemble_idl_cf(cf,&func,array_size_list);
    
    cf_list->push_back(cf);
  }
  if (cmdval != NULL) {
  	meta_t func ;
	func.data = "comment";
	func.file = id->file;
	func.lineno = id->lineno;
	cf_t *cf = alloc_cf(idl);
	assemble_idl_cf(cf, &func, cmdval);
	cf_list->push_back(cf);

	//delete cmdval;
  }
  assemble_idl_var(idl,var,id,type,cf_list);
  //delete cf_list;
  //delete array_size_list;



  yyval.var=var;
;}
    break;

  case 6:
#line 143 "idl.gram"
    {
	meta_list_t *lst = new meta_list_t;
	idl->parse_meta_list.push_back(lst);
	lst->push_back(yyvsp[0].meta);
	idl->cmdreserve = 0;
	yyval.meta_list = lst;
;}
    break;

  case 7:
#line 151 "idl.gram"
    {
	idl->cmdreserve = 0;
	yyval.meta_list = NULL;
;}
    break;

  case 8:
#line 159 "idl.gram"
    {
  meta_list_t *array_size_list=yyvsp[-3].meta_list;
  meta_t *array_tag=yyvsp[-1].meta;
  int array_size=atoi(array_tag->data.c_str());
  /* we have to validate here */
  if(array_size<=0){
    char tmpstr[2048];
    meta_t errinfo;
    snprintf(tmpstr,sizeof(tmpstr),"array size %d is invalid",array_size);      
    errinfo.data=tmpstr;
    errinfo.file=array_tag->file;
    errinfo.lineno=array_tag->lineno;
    add_idl_error(idl,errinfo);
    array_tag->data="0";
  }
  array_size_list->push_back(array_tag);
  yyval.meta_list=array_size_list;
;}
    break;

  case 9:
#line 178 "idl.gram"
    {
  meta_list_t *array_size_list=yyvsp[-2].meta_list;
  meta_t *array_size=alloc_meta(idl);
  array_size->data="0";
  array_size->file=loc->file;
  array_size->lineno=loc->lineno;
  array_size_list->push_back(array_size);
  yyval.meta_list=array_size_list;
;}
    break;

  case 10:
#line 188 "idl.gram"
    {
   meta_list_t *array_size_list=new meta_list_t;
   idl->parse_meta_list.push_back(array_size_list);
   yyval.meta_list=array_size_list;
 ;}
    break;

  case 11:
#line 196 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 12:
#line 197 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 13:
#line 198 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 14:
#line 199 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 15:
#line 200 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 16:
#line 201 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 17:
#line 202 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 18:
#line 203 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 19:
#line 204 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 20:
#line 205 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 21:
#line 206 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 22:
#line 207 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 23:
#line 208 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 24:
#line 209 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 25:
#line 210 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 26:
#line 215 "idl.gram"
    {
	yyval.cf_list=yyvsp[0].cf_list; 
	idl->cmdreserve = 1;
;}
    break;

  case 27:
#line 220 "idl.gram"
    {
	yyval.cf_list=NULL;
	idl->cmdreserve = 1;
;}
    break;

  case 28:
#line 228 "idl.gram"
    {
  cf_list_t *cf_list=yyvsp[-2].cf_list;
  cf_t *cf=yyvsp[0].cf;
  cf_list->push_back(cf);
  yyval.cf_list=cf_list;
;}
    break;

  case 29:
#line 235 "idl.gram"
    {
  cf_list_t *cf_list=new cf_list_t;
  idl->parse_list.push_back(cf_list);
  cf_t *cf=yyvsp[0].cf;
  cf_list->push_back(cf);
  yyval.cf_list=cf_list;
;}
    break;

  case 30:
#line 246 "idl.gram"
    {
  meta_t *func=yyvsp[-3].meta;
  meta_list_t *arg_list=yyvsp[-1].meta_list;
  cf_t *cf=alloc_cf(idl);
  assemble_idl_cf(cf,func,arg_list);
  //delete arg_list;
  yyval.cf=cf;
;}
    break;

  case 31:
#line 255 "idl.gram"
    {
  meta_t *func=yyvsp[-2].meta;
  meta_list_t *arg_list=new meta_list_t;
  cf_t *cf=alloc_cf(idl);
  assemble_idl_cf(cf,func,arg_list);
  delete arg_list;
  yyval.cf=cf;
;}
    break;

  case 32:
#line 267 "idl.gram"
    {
  meta_list_t *param_list=yyvsp[-2].meta_list;
  meta_t *param=yyvsp[0].meta;
  param_list->push_back(param);
  yyval.meta_list=param_list;
;}
    break;

  case 33:
#line 274 "idl.gram"
    {
  meta_list_t *param_list=new meta_list_t;
  idl->parse_meta_list.push_back(param_list);
  meta_t *param=yyvsp[0].meta;
  param_list->push_back(param);
  yyval.meta_list=param_list;
;}
    break;

  case 34:
#line 284 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 35:
#line 285 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 36:
#line 286 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 37:
#line 287 "idl.gram"
    {yyval.meta=yyvsp[0].meta;;}
    break;

  case 38:
#line 292 "idl.gram"
    {
  meta_t *id=yyvsp[-4].meta;
  var_list_t *var_list=yyvsp[-2].var_list;
  group_t *grp;
  grp=alloc_group(idl);
  assemble_idl_group(idl,grp,id,var_list);
  //delete var_list;
  yyval.grp=grp;
;}
    break;

  case 39:
#line 302 "idl.gram"
    {
  meta_t *new_id=yyvsp[-6].meta,*old_id=yyvsp[-4].meta;
  var_list_t *var_list=yyvsp[-2].var_list;
  group_t *grp;
  grp=alloc_group(idl);
  overwrite_idl_group(idl,grp,new_id,old_id,var_list);
  //delete var_list;
  yyval.grp=grp;
;}
    break;

  case 40:
#line 315 "idl.gram"
    {
  var_list_t *var_list=yyvsp[-1].var_list;
  var_t *var=yyvsp[0].var;
  var_list->push_back(var);
  yyval.var_list=var_list;
;}
    break;

  case 41:
#line 322 "idl.gram"
    {
  var_list_t *var_list=new var_list_t;
  idl->parse_var_list.push_back(var_list);
  yyval.var_list=var_list;
;}
    break;


    }

/* Line 1000 of yacc.c.  */
#line 1465 "idl_gram.c"

  yyvsp -= yylen;
  yyssp -= yylen;


  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;


  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (YYPACT_NINF < yyn && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  int yytype = YYTRANSLATE (yychar);
	  const char* yyprefix;
	  char *yymsg;
	  int yyx;

	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  int yyxbegin = yyn < 0 ? -yyn : 0;

	  /* Stay within bounds of both yycheck and yytname.  */
	  int yychecklim = YYLAST - yyn;
	  int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
	  int yycount = 0;

	  yyprefix = ", expecting ";
	  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	      {
		yysize += yystrlen (yyprefix) + yystrlen (yytname [yyx]);
		yycount += 1;
		if (yycount == 5)
		  {
		    yysize = 0;
		    break;
		  }
	      }
	  yysize += (sizeof ("syntax error, unexpected ")
		     + yystrlen (yytname[yytype]));
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "syntax error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[yytype]);

	      if (yycount < 5)
		{
		  yyprefix = ", expecting ";
		  for (yyx = yyxbegin; yyx < yyxend; ++yyx)
		    if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
		      {
			yyp = yystpcpy (yyp, yyprefix);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yyprefix = " or ";
		      }
		}
	      yyerror (scanner, loc, idl, yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror (scanner, loc, idl, "syntax error; also virtual memory exhausted");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror (scanner, loc, idl, "syntax error");
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* If at end of input, pop the error token,
	     then the rest of the stack, then return failure.  */
	  if (yychar == YYEOF)
	     for (;;)
	       {
		 YYPOPSTACK;
		 if (yyssp == yyss)
		   YYABORT;
		 YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
		 yydestruct (yystos[*yyssp], yyvsp);
	       }
        }
      else
	{
	  YYDSYMPRINTF ("Error: discarding", yytoken, &yylval, &yylloc);
	  yydestruct (yytoken, &yylval);
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

#ifdef __GNUC__
  /* Pacify GCC when the user code never invokes YYERROR and the label
     yyerrorlab therefore never appears in user code.  */
  if (0)
     goto yyerrorlab;
#endif

  yyvsp -= yylen;
  yyssp -= yylen;
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
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

      YYDSYMPRINTF ("Error: popping", yystos[*yyssp], yyvsp, yylsp);
      yydestruct (yystos[yystate], yyvsp);
      YYPOPSTACK;
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;


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

#ifndef yyoverflow
/*----------------------------------------------.
| yyoverflowlab -- parser overflow comes here.  |
`----------------------------------------------*/
yyoverflowlab:
  yyerror (scanner, loc, idl, "parser stack overflow");
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}


#line 329 "idl.gram"

void cfgidlerror(yyscan_t scanner,meta_t *loc,idl_t *idl,char *msg)
{
  char tmp[2048];
  meta_t errinfo;
  errinfo.file=loc->file;
  errinfo.lineno=loc->lineno;
  snprintf(tmp,sizeof(tmp),"%s before char:'%c'",
	  msg,
	  cfgidlget_text(scanner)[0]);
  errinfo.data=tmp;
  add_idl_error(idl,errinfo);
  return ;
}

