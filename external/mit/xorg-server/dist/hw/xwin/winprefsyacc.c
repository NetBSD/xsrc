/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

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
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "winprefsyacc.y" /* yacc.c:339  */

/*
 * Copyright (C) 1994-2000 The XFree86 Project, Inc. All Rights Reserved.
 * Copyright (C) Colin Harrison 2005-2008
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE XFREE86 PROJECT BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of the XFree86 Project
 * shall not be used in advertising or otherwise to promote the sale, use
 * or other dealings in this Software without prior written authorization
 * from the XFree86 Project.
 *
 * Authors:     Earle F. Philhower, III
 *              Colin Harrison
 */
/* $XFree86: $ */

#ifdef HAVE_XWIN_CONFIG_H
#include <xwin-config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#define _STDLIB_H 1 /* bison checks this to know if stdlib has been included */
#include <string.h>
#include "winprefs.h"

/* The following give better error messages in bison at the cost of a few KB */
#define YYERROR_VERBOSE 1

/* YYLTYPE_IS_TRIVIAL and YYENABLE_NLS defined to suppress warnings */
#define YYLTYPE_IS_TRIVIAL 1
#define YYENABLE_NLS 0

/* The global pref settings */
WINPREFS pref;

/* The working menu */  
static MENUPARSED menu;

/* Functions for parsing the tokens into out structure */
/* Defined at the end section of this file */

static void SetIconDirectory (char *path);
static void SetDefaultIcon (char *fname);
static void SetRootMenu (char *menu);
static void SetDefaultSysMenu (char *menu, int pos);
static void SetTrayIcon (char *fname);

static void OpenMenu(char *menuname);
static void AddMenuLine(const char *name, MENUCOMMANDTYPE cmd, const char *param);
static void CloseMenu(void);

static void OpenIcons(void);
static void AddIconLine(char *matchstr, char *iconfile);
static void CloseIcons(void);

static void OpenStyles(void);
static void AddStyleLine(char *matchstr, unsigned long style);
static void CloseStyles(void);

static void OpenSysMenu(void);
static void AddSysMenuLine(char *matchstr, char *menuname, int pos);
static void CloseSysMenu(void);

static int yyerror (const char *s);

extern char *yytext;
extern int yylineno;
extern int yylex(void);


#line 155 "winprefsyacc.c" /* yacc.c:339  */

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
   by #include "y.tab.h".  */
#ifndef YY_YY_WINPREFSYACC_H_INCLUDED
# define YY_YY_WINPREFSYACC_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    NEWLINE = 258,
    MENU = 259,
    LB = 260,
    RB = 261,
    ICONDIRECTORY = 262,
    DEFAULTICON = 263,
    ICONS = 264,
    STYLES = 265,
    TOPMOST = 266,
    MAXIMIZE = 267,
    MINIMIZE = 268,
    BOTTOM = 269,
    NOTITLE = 270,
    OUTLINE = 271,
    NOFRAME = 272,
    DEFAULTSYSMENU = 273,
    SYSMENU = 274,
    ROOTMENU = 275,
    SEPARATOR = 276,
    ATSTART = 277,
    ATEND = 278,
    EXEC = 279,
    ALWAYSONTOP = 280,
    DEBUGOUTPUT = 281,
    RELOAD = 282,
    TRAYICON = 283,
    FORCEEXIT = 284,
    SILENTEXIT = 285,
    STRING = 286
  };
#endif
/* Tokens.  */
#define NEWLINE 258
#define MENU 259
#define LB 260
#define RB 261
#define ICONDIRECTORY 262
#define DEFAULTICON 263
#define ICONS 264
#define STYLES 265
#define TOPMOST 266
#define MAXIMIZE 267
#define MINIMIZE 268
#define BOTTOM 269
#define NOTITLE 270
#define OUTLINE 271
#define NOFRAME 272
#define DEFAULTSYSMENU 273
#define SYSMENU 274
#define ROOTMENU 275
#define SEPARATOR 276
#define ATSTART 277
#define ATEND 278
#define EXEC 279
#define ALWAYSONTOP 280
#define DEBUGOUTPUT 281
#define RELOAD 282
#define TRAYICON 283
#define FORCEEXIT 284
#define SILENTEXIT 285
#define STRING 286

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 90 "winprefsyacc.y" /* yacc.c:355  */

  char *sVal;
  unsigned long uVal;
  int iVal;

#line 263 "winprefsyacc.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_WINPREFSYACC_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 280 "winprefsyacc.c" /* yacc.c:358  */

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
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
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
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   98

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  32
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  33
/* YYNRULES -- Number of rules.  */
#define YYNRULES  65
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  121

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   286

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
      25,    26,    27,    28,    29,    30,    31
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint8 yyrline[] =
{
       0,   133,   133,   134,   137,   138,   142,   143,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
     160,   163,   166,   169,   172,   175,   176,   177,   178,   179,
     182,   183,   186,   186,   189,   192,   193,   196,   196,   199,
     200,   201,   202,   205,   206,   207,   210,   211,   212,   213,
     216,   219,   220,   223,   223,   226,   227,   228,   231,   234,
     235,   238,   238,   241,   244,   247
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "NEWLINE", "MENU", "LB", "RB",
  "ICONDIRECTORY", "DEFAULTICON", "ICONS", "STYLES", "TOPMOST", "MAXIMIZE",
  "MINIMIZE", "BOTTOM", "NOTITLE", "OUTLINE", "NOFRAME", "DEFAULTSYSMENU",
  "SYSMENU", "ROOTMENU", "SEPARATOR", "ATSTART", "ATEND", "EXEC",
  "ALWAYSONTOP", "\"DEBUG\"", "RELOAD", "TRAYICON", "FORCEEXIT",
  "SILENTEXIT", "STRING", "$accept", "input", "line", "newline_or_nada",
  "command", "trayicon", "rootmenu", "defaultsysmenu", "defaulticon",
  "icondirectory", "menuline", "menulist", "menu", "$@1", "iconline",
  "iconlist", "icons", "$@2", "group1", "group2", "stylecombo",
  "styleline", "stylelist", "styles", "$@3", "atspot", "sysmenuline",
  "sysmenulist", "sysmenu", "$@4", "forceexit", "silentexit", "debug", YY_NULLPTR
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
     285,   286
};
# endif

#define YYPACT_NINF -47

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-47)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
     -47,     7,   -47,   -47,    -1,     0,     1,    18,    29,    15,
      42,    17,    19,    20,    46,    50,   -47,   -47,   -47,   -47,
     -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,   -47,
      49,    53,    54,   -47,   -47,     6,    55,    56,    57,    58,
     -47,   -47,   -47,   -47,   -47,    61,    61,   -47,   -47,    62,
     -47,   -47,   -47,   -47,    61,    61,    35,    38,   -47,    61,
     -19,   -47,    39,    35,    66,    27,    38,    67,    43,    72,
      -3,   -19,    70,    74,   -47,   -47,   -47,   -47,   -47,   -47,
     -47,   -47,   -47,     3,    -8,    75,   -47,   -47,    48,    43,
      76,    61,    52,    59,    77,    78,   -47,   -47,    61,   -47,
     -47,    61,     6,   -47,   -47,   -47,    81,    82,    61,    61,
     -47,   -47,    83,    61,    61,   -47,   -47,    61,   -47,   -47,
     -47
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     3,     5,    17,    14,
      15,     8,     9,    10,    11,    12,    13,    18,    19,    16,
       0,     0,     0,    37,    53,    55,     0,     0,     0,     0,
      63,    64,    32,    24,    23,     6,     6,    56,    57,     0,
      61,    21,    65,    20,     6,     6,     0,     0,    22,     6,
       0,     7,     0,    35,     0,     0,    51,     0,     0,     0,
       0,    30,     0,     0,    36,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,     0,    52,    54,     0,    59,
       0,     6,     0,     0,     0,     0,    31,    33,     6,    48,
      49,     6,    55,    60,    62,    25,     0,     0,     6,     6,
      34,    50,     0,     6,     6,    26,    29,     6,    28,    27,
      58
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
     -47,   -47,   -47,   -46,   -47,   -47,   -47,   -47,   -47,   -47,
     -47,    16,   -47,   -47,   -47,    25,   -47,   -47,     5,     8,
     -47,   -47,    26,   -47,   -47,    -9,   -47,     9,   -47,   -47,
     -47,   -47,   -47
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     1,    16,    56,    17,    18,    19,    20,    21,    22,
      71,    72,    23,    54,    63,    64,    24,    45,    83,    84,
      85,    66,    67,    25,    46,    49,    89,    90,    26,    59,
      27,    28,    29
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
      57,    92,    69,    76,    77,    78,    79,     2,    60,    61,
       3,     4,    70,    68,     5,     6,     7,     8,    80,    81,
      82,    93,    94,    33,    95,     9,    10,    11,    47,    48,
      30,    31,    32,    12,    34,    13,    14,    15,    76,    77,
      78,    79,    80,    81,    82,   105,    35,    36,    37,    40,
      38,    39,   110,    41,    42,   111,    43,    44,    50,    51,
      52,    53,   115,   116,    55,    58,    62,   118,   119,    65,
      73,   120,    75,    87,    88,    91,    97,    98,   101,   102,
     108,   109,   104,   106,   113,   114,   117,    96,    74,   100,
     107,    99,    86,   112,     0,     0,     0,     0,   103
};

static const yytype_int8 yycheck[] =
{
      46,     4,    21,    11,    12,    13,    14,     0,    54,    55,
       3,     4,    31,    59,     7,     8,     9,    10,    15,    16,
      17,    24,    25,     5,    27,    18,    19,    20,    22,    23,
      31,    31,    31,    26,     5,    28,    29,    30,    11,    12,
      13,    14,    15,    16,    17,    91,    31,     5,    31,     3,
      31,    31,    98,     3,     5,   101,     3,     3,     3,     3,
       3,     3,   108,   109,     3,     3,    31,   113,   114,    31,
      31,   117,     6,     6,    31,     3,     6,     3,     3,    31,
       3,     3,     6,    31,     3,     3,     3,    71,    63,    84,
      31,    83,    66,   102,    -1,    -1,    -1,    -1,    89
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    33,     0,     3,     4,     7,     8,     9,    10,    18,
      19,    20,    26,    28,    29,    30,    34,    36,    37,    38,
      39,    40,    41,    44,    48,    55,    60,    62,    63,    64,
      31,    31,    31,     5,     5,    31,     5,    31,    31,    31,
       3,     3,     5,     3,     3,    49,    56,    22,    23,    57,
       3,     3,     3,     3,    45,     3,    35,    35,     3,    61,
      35,    35,    31,    46,    47,    31,    53,    54,    35,    21,
      31,    42,    43,    31,    47,     6,    11,    12,    13,    14,
      15,    16,    17,    50,    51,    52,    54,     6,    31,    58,
      59,     3,     4,    24,    25,    27,    43,     6,     3,    51,
      50,     3,    31,    59,     6,    35,    31,    31,     3,     3,
      35,    35,    57,     3,     3,    35,    35,     3,    35,    35,
      35
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    32,    33,    33,    34,    34,    35,    35,    36,    36,
      36,    36,    36,    36,    36,    36,    36,    36,    36,    36,
      37,    38,    39,    40,    41,    42,    42,    42,    42,    42,
      43,    43,    45,    44,    46,    47,    47,    49,    48,    50,
      50,    50,    50,    51,    51,    51,    52,    52,    52,    52,
      53,    54,    54,    56,    55,    57,    57,    57,    58,    59,
      59,    61,    60,    62,    63,    64
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     0,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       3,     3,     4,     3,     3,     3,     4,     5,     5,     4,
       1,     2,     0,     7,     4,     1,     2,     0,     6,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       4,     1,     2,     0,     6,     0,     1,     1,     5,     1,
       2,     0,     7,     2,     2,     3
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
      yyerror (YY_("syntax error: cannot back up")); \
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
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
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
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
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
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
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
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
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
      yychar = yylex ();
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
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
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
        case 20:
#line 160 "winprefsyacc.y" /* yacc.c:1646  */
    { SetTrayIcon((yyvsp[-1].sVal)); free((yyvsp[-1].sVal)); }
#line 1438 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 21:
#line 163 "winprefsyacc.y" /* yacc.c:1646  */
    { SetRootMenu((yyvsp[-1].sVal)); free((yyvsp[-1].sVal)); }
#line 1444 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 22:
#line 166 "winprefsyacc.y" /* yacc.c:1646  */
    { SetDefaultSysMenu((yyvsp[-2].sVal), (yyvsp[-1].iVal)); free((yyvsp[-2].sVal)); }
#line 1450 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 23:
#line 169 "winprefsyacc.y" /* yacc.c:1646  */
    { SetDefaultIcon((yyvsp[-1].sVal)); free((yyvsp[-1].sVal)); }
#line 1456 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 24:
#line 172 "winprefsyacc.y" /* yacc.c:1646  */
    { SetIconDirectory((yyvsp[-1].sVal)); free((yyvsp[-1].sVal)); }
#line 1462 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 25:
#line 175 "winprefsyacc.y" /* yacc.c:1646  */
    { AddMenuLine("-", CMD_SEPARATOR, ""); }
#line 1468 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 26:
#line 176 "winprefsyacc.y" /* yacc.c:1646  */
    { AddMenuLine((yyvsp[-3].sVal), CMD_ALWAYSONTOP, ""); free((yyvsp[-3].sVal)); }
#line 1474 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 27:
#line 177 "winprefsyacc.y" /* yacc.c:1646  */
    { AddMenuLine((yyvsp[-4].sVal), CMD_EXEC, (yyvsp[-2].sVal)); free((yyvsp[-4].sVal)); free((yyvsp[-2].sVal)); }
#line 1480 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 28:
#line 178 "winprefsyacc.y" /* yacc.c:1646  */
    { AddMenuLine((yyvsp[-4].sVal), CMD_MENU, (yyvsp[-2].sVal)); free((yyvsp[-4].sVal)); free((yyvsp[-2].sVal)); }
#line 1486 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 29:
#line 179 "winprefsyacc.y" /* yacc.c:1646  */
    { AddMenuLine((yyvsp[-3].sVal), CMD_RELOAD, ""); free((yyvsp[-3].sVal)); }
#line 1492 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 32:
#line 186 "winprefsyacc.y" /* yacc.c:1646  */
    { OpenMenu((yyvsp[-1].sVal)); free((yyvsp[-1].sVal)); }
#line 1498 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 33:
#line 186 "winprefsyacc.y" /* yacc.c:1646  */
    {CloseMenu();}
#line 1504 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 34:
#line 189 "winprefsyacc.y" /* yacc.c:1646  */
    { AddIconLine((yyvsp[-3].sVal), (yyvsp[-2].sVal)); free((yyvsp[-3].sVal)); free((yyvsp[-2].sVal)); }
#line 1510 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 37:
#line 196 "winprefsyacc.y" /* yacc.c:1646  */
    {OpenIcons();}
#line 1516 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 38:
#line 196 "winprefsyacc.y" /* yacc.c:1646  */
    {CloseIcons();}
#line 1522 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 39:
#line 199 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.uVal)=STYLE_TOPMOST; }
#line 1528 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 40:
#line 200 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.uVal)=STYLE_MAXIMIZE; }
#line 1534 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 41:
#line 201 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.uVal)=STYLE_MINIMIZE; }
#line 1540 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 42:
#line 202 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.uVal)=STYLE_BOTTOM; }
#line 1546 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 43:
#line 205 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.uVal)=STYLE_NOTITLE; }
#line 1552 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 44:
#line 206 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.uVal)=STYLE_OUTLINE; }
#line 1558 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 45:
#line 207 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.uVal)=STYLE_NOFRAME; }
#line 1564 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 46:
#line 210 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.uVal)=(yyvsp[0].uVal); }
#line 1570 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 47:
#line 211 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.uVal)=(yyvsp[0].uVal); }
#line 1576 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 48:
#line 212 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.uVal)=(yyvsp[-1].uVal)|(yyvsp[0].uVal); }
#line 1582 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 49:
#line 213 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.uVal)=(yyvsp[-1].uVal)|(yyvsp[0].uVal); }
#line 1588 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 50:
#line 216 "winprefsyacc.y" /* yacc.c:1646  */
    { AddStyleLine((yyvsp[-3].sVal), (yyvsp[-2].uVal)); free((yyvsp[-3].sVal)); }
#line 1594 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 53:
#line 223 "winprefsyacc.y" /* yacc.c:1646  */
    {OpenStyles();}
#line 1600 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 54:
#line 223 "winprefsyacc.y" /* yacc.c:1646  */
    {CloseStyles();}
#line 1606 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 55:
#line 226 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.iVal)=AT_END; }
#line 1612 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 56:
#line 227 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.iVal)=AT_START; }
#line 1618 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 57:
#line 228 "winprefsyacc.y" /* yacc.c:1646  */
    { (yyval.iVal)=AT_END; }
#line 1624 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 58:
#line 231 "winprefsyacc.y" /* yacc.c:1646  */
    { AddSysMenuLine((yyvsp[-4].sVal), (yyvsp[-3].sVal), (yyvsp[-2].iVal)); free((yyvsp[-4].sVal)); free((yyvsp[-3].sVal)); }
#line 1630 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 61:
#line 238 "winprefsyacc.y" /* yacc.c:1646  */
    {OpenSysMenu();}
#line 1636 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 62:
#line 238 "winprefsyacc.y" /* yacc.c:1646  */
    {CloseSysMenu();}
#line 1642 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 63:
#line 241 "winprefsyacc.y" /* yacc.c:1646  */
    { pref.fForceExit = TRUE; }
#line 1648 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 64:
#line 244 "winprefsyacc.y" /* yacc.c:1646  */
    { pref.fSilentExit = TRUE; }
#line 1654 "winprefsyacc.c" /* yacc.c:1646  */
    break;

  case 65:
#line 247 "winprefsyacc.y" /* yacc.c:1646  */
    { ErrorF("LoadPreferences: %s\n", (yyvsp[-1].sVal)); free((yyvsp[-1].sVal)); }
#line 1660 "winprefsyacc.c" /* yacc.c:1646  */
    break;


#line 1664 "winprefsyacc.c" /* yacc.c:1646  */
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
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
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
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
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
                      yytoken, &yylval);
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
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

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
                  yystos[yystate], yyvsp);
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
  yyerror (YY_("memory exhausted"));
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
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
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
#line 251 "winprefsyacc.y" /* yacc.c:1906  */

/*
 * Errors in parsing abort and print log messages
 */
static int
yyerror (const char *s)
{
  ErrorF("LoadPreferences: %s line %d\n", s, yylineno);
  return 1;
}

/* Miscellaneous functions to store TOKENs into the structure */
static void
SetIconDirectory (char *path)
{
  strncpy (pref.iconDirectory, path, PATH_MAX);
  pref.iconDirectory[PATH_MAX] = 0;
}

static void
SetDefaultIcon (char *fname)
{
  strncpy (pref.defaultIconName, fname, NAME_MAX);
  pref.defaultIconName[NAME_MAX] = 0;
}

static void
SetTrayIcon (char *fname)
{
  strncpy (pref.trayIconName, fname, NAME_MAX);
  pref.trayIconName[NAME_MAX] = 0;
}

static void
SetRootMenu (char *menuname)
{
  strncpy (pref.rootMenuName, menuname, MENU_MAX);
  pref.rootMenuName[MENU_MAX] = 0;
}

static void
SetDefaultSysMenu (char *menuname, int pos)
{
  strncpy (pref.defaultSysMenuName, menuname, MENU_MAX);
  pref.defaultSysMenuName[MENU_MAX] = 0;
  pref.defaultSysMenuPos = pos;
}

static void
OpenMenu (char *menuname)
{
  if (menu.menuItem) free(menu.menuItem);
  menu.menuItem = NULL;
  strncpy(menu.menuName, menuname, MENU_MAX);
  menu.menuName[MENU_MAX] = 0;
  menu.menuItems = 0;
}

static void
AddMenuLine (const char *text, MENUCOMMANDTYPE cmd, const char *param)
{
  if (menu.menuItem==NULL)
    menu.menuItem = malloc(sizeof(MENUITEM));
  else
    menu.menuItem = realloc(menu.menuItem, sizeof(MENUITEM)*(menu.menuItems+1));

  strncpy (menu.menuItem[menu.menuItems].text, text, MENU_MAX);
  menu.menuItem[menu.menuItems].text[MENU_MAX] = 0;

  menu.menuItem[menu.menuItems].cmd = cmd;

  strncpy(menu.menuItem[menu.menuItems].param, param, PARAM_MAX);
  menu.menuItem[menu.menuItems].param[PARAM_MAX] = 0;

  menu.menuItem[menu.menuItems].commandID = 0;

  menu.menuItems++;
}

static void
CloseMenu (void)
{
  if (menu.menuItem==NULL || menu.menuItems==0)
    {
      ErrorF("LoadPreferences: Empty menu detected\n");
      return;
    }
  
  if (pref.menuItems)
    pref.menu = realloc (pref.menu, (pref.menuItems+1)*sizeof(MENUPARSED));
  else
    pref.menu = malloc (sizeof(MENUPARSED));
  
  memcpy (pref.menu+pref.menuItems, &menu, sizeof(MENUPARSED));
  pref.menuItems++;

  memset (&menu, 0, sizeof(MENUPARSED));
}

static void 
OpenIcons (void)
{
  if (pref.icon != NULL) {
    ErrorF("LoadPreferences: Redefining icon mappings\n");
    free(pref.icon);
    pref.icon = NULL;
  }
  pref.iconItems = 0;
}

static void
AddIconLine (char *matchstr, char *iconfile)
{
  if (pref.icon==NULL)
    pref.icon = malloc(sizeof(ICONITEM));
  else
    pref.icon = realloc(pref.icon, sizeof(ICONITEM)*(pref.iconItems+1));

  strncpy(pref.icon[pref.iconItems].match, matchstr, MENU_MAX);
  pref.icon[pref.iconItems].match[MENU_MAX] = 0;

  strncpy(pref.icon[pref.iconItems].iconFile, iconfile, PATH_MAX+NAME_MAX+1);
  pref.icon[pref.iconItems].iconFile[PATH_MAX+NAME_MAX+1] = 0;

  pref.icon[pref.iconItems].hicon = 0;

  pref.iconItems++;
}

static void 
CloseIcons (void)
{
}

static void
OpenStyles (void)
{
  if (pref.style != NULL) {
    ErrorF("LoadPreferences: Redefining window style\n");
    free(pref.style);
    pref.style = NULL;
  }
  pref.styleItems = 0;
}

static void
AddStyleLine (char *matchstr, unsigned long style)
{
  if (pref.style==NULL)
    pref.style = malloc(sizeof(STYLEITEM));
  else
    pref.style = realloc(pref.style, sizeof(STYLEITEM)*(pref.styleItems+1));

  strncpy(pref.style[pref.styleItems].match, matchstr, MENU_MAX);
  pref.style[pref.styleItems].match[MENU_MAX] = 0;

  pref.style[pref.styleItems].type = style;

  pref.styleItems++;
}

static void
CloseStyles (void)
{
}

static void
OpenSysMenu (void)
{
  if (pref.sysMenu != NULL) {
    ErrorF("LoadPreferences: Redefining system menu\n");
    free(pref.sysMenu);
    pref.sysMenu = NULL;
  }
  pref.sysMenuItems = 0;
}

static void
AddSysMenuLine (char *matchstr, char *menuname, int pos)
{
  if (pref.sysMenu==NULL)
    pref.sysMenu = malloc(sizeof(SYSMENUITEM));
  else
    pref.sysMenu = realloc(pref.sysMenu, sizeof(SYSMENUITEM)*(pref.sysMenuItems+1));

  strncpy (pref.sysMenu[pref.sysMenuItems].match, matchstr, MENU_MAX);
  pref.sysMenu[pref.sysMenuItems].match[MENU_MAX] = 0;

  strncpy (pref.sysMenu[pref.sysMenuItems].menuName, menuname, MENU_MAX);
  pref.sysMenu[pref.sysMenuItems].menuName[MENU_MAX] = 0;

  pref.sysMenu[pref.sysMenuItems].menuPos = pos;

  pref.sysMenuItems++;
}

static void
CloseSysMenu (void)
{
}

