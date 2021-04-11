/* A Bison parser, made by GNU Bison 3.4.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2019 Free Software Foundation,
   Inc.

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 23 "gram.y"

#include "ctwm.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "otp.h"
#include "iconmgr.h"
#include "icons.h"
#include "windowbox.h"
#include "functions_defs.h"
#include "list.h"
#include "util.h"
#include "occupation.h"
#include "screen.h"
#include "parse.h"
#include "parse_be.h"
#include "parse_yacc.h"
#include "cursor.h"
#include "win_decorations_init.h"
#include "win_regions.h"
#include "workspace_config.h"
#ifdef SOUNDS
#	include "sound.h"
#endif

static char *curWorkSpc = NULL;
static char *client = NULL;
static char *workspace = NULL;
static MenuItem *lastmenuitem = NULL;
static name_list **curplist = NULL;
static int color = 0;
extern char *yytext; // Have to manually pull this in

int yylex(void);

#line 108 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"

# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_HOME_FULLERMD_WORK_CTWM_BZR_4_0_X_CTWM_MKTAR_ASYJVB_CTWM_4_0_3_BUILD_GRAM_TAB_H_INCLUDED
# define YY_YY_HOME_FULLERMD_WORK_CTWM_BZR_4_0_X_CTWM_MKTAR_ASYJVB_CTWM_4_0_3_BUILD_GRAM_TAB_H_INCLUDED
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
    LB = 258,
    RB = 259,
    LP = 260,
    RP = 261,
    MENUS = 262,
    MENU = 263,
    BUTTON = 264,
    DEFAULT_FUNCTION = 265,
    PLUS = 266,
    MINUS = 267,
    ALL = 268,
    OR = 269,
    CURSORS = 270,
    PIXMAPS = 271,
    ICONS = 272,
    COLOR = 273,
    SAVECOLOR = 274,
    MONOCHROME = 275,
    FUNCTION = 276,
    ICONMGR_SHOW = 277,
    ICONMGR = 278,
    ALTER = 279,
    WINDOW_FUNCTION = 280,
    ZOOM = 281,
    ICONMGRS = 282,
    ICONMGR_GEOMETRY = 283,
    ICONMGR_NOSHOW = 284,
    MAKE_TITLE = 285,
    ICONIFY_BY_UNMAPPING = 286,
    DONT_ICONIFY_BY_UNMAPPING = 287,
    AUTO_POPUP = 288,
    NO_BORDER = 289,
    NO_ICON_TITLE = 290,
    NO_TITLE = 291,
    AUTO_RAISE = 292,
    NO_HILITE = 293,
    ICON_REGION = 294,
    WINDOW_REGION = 295,
    META = 296,
    SHIFT = 297,
    LOCK = 298,
    CONTROL = 299,
    WINDOW = 300,
    TITLE = 301,
    ICON = 302,
    ROOT = 303,
    FRAME = 304,
    COLON = 305,
    EQUALS = 306,
    SQUEEZE_TITLE = 307,
    DONT_SQUEEZE_TITLE = 308,
    WARP_ON_DEICONIFY = 309,
    START_ICONIFIED = 310,
    NO_TITLE_HILITE = 311,
    TITLE_HILITE = 312,
    MOVE = 313,
    RESIZE = 314,
    WAITC = 315,
    SELECT = 316,
    KILL = 317,
    LEFT_TITLEBUTTON = 318,
    RIGHT_TITLEBUTTON = 319,
    NUMBER = 320,
    KEYWORD = 321,
    NKEYWORD = 322,
    CKEYWORD = 323,
    CLKEYWORD = 324,
    FKEYWORD = 325,
    FSKEYWORD = 326,
    FNKEYWORD = 327,
    PRIORITY_SWITCHING = 328,
    PRIORITY_NOT_SWITCHING = 329,
    SKEYWORD = 330,
    SSKEYWORD = 331,
    WINDOW_RING = 332,
    WINDOW_RING_EXCLUDE = 333,
    WARP_CURSOR = 334,
    ERRORTOKEN = 335,
    GRAVITY = 336,
    SIJENUM = 337,
    NO_STACKMODE = 338,
    ALWAYS_ON_TOP = 339,
    WORKSPACE = 340,
    WORKSPACES = 341,
    WORKSPCMGR_GEOMETRY = 342,
    OCCUPYALL = 343,
    OCCUPYLIST = 344,
    MAPWINDOWCURRENTWORKSPACE = 345,
    MAPWINDOWDEFAULTWORKSPACE = 346,
    ON_TOP_PRIORITY = 347,
    UNMAPBYMOVINGFARAWAY = 348,
    OPAQUEMOVE = 349,
    NOOPAQUEMOVE = 350,
    OPAQUERESIZE = 351,
    NOOPAQUERESIZE = 352,
    DONTSETINACTIVE = 353,
    CHANGE_WORKSPACE_FUNCTION = 354,
    DEICONIFY_FUNCTION = 355,
    ICONIFY_FUNCTION = 356,
    AUTOSQUEEZE = 357,
    STARTSQUEEZED = 358,
    DONT_SAVE = 359,
    AUTO_LOWER = 360,
    ICONMENU_DONTSHOW = 361,
    WINDOW_BOX = 362,
    IGNOREMODIFIER = 363,
    WINDOW_GEOMETRIES = 364,
    ALWAYSSQUEEZETOGRAVITY = 365,
    VIRTUAL_SCREENS = 366,
    IGNORE_TRANSIENT = 367,
    EWMH_IGNORE = 368,
    MWM_IGNORE = 369,
    RPLAY_SOUNDS = 370,
    FORCE_FOCUS = 371,
    STRING = 372
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 62 "gram.y"

    int num;
    char *ptr;

#line 274 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_HOME_FULLERMD_WORK_CTWM_BZR_4_0_X_CTWM_MKTAR_ASYJVB_CTWM_4_0_3_BUILD_GRAM_TAB_H_INCLUDED  */



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
typedef unsigned short yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short yytype_int16;
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
#  define YYSIZE_T unsigned
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

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
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


#define YY_ASSERT(E) ((void) (0 && (E)))

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
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   710

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  118
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  165
/* YYNRULES -- Number of rules.  */
#define YYNRULES  382
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  546

#define YYUNDEFTOK  2
#define YYMAXUTOK   372

/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
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
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   106,   106,   106,   110,   111,   114,   115,   116,   117,
     118,   119,   122,   125,   128,   131,   131,   135,   135,   139,
     139,   143,   143,   148,   148,   153,   153,   158,   164,   167,
     173,   176,   176,   179,   179,   182,   188,   190,   191,   192,
     192,   194,   197,   197,   199,   200,   200,   202,   203,   203,
     205,   206,   206,   208,   210,   213,   216,   216,   218,   218,
     220,   224,   240,   241,   243,   243,   245,   245,   248,   250,
     247,   252,   252,   254,   254,   256,   256,   258,   258,   260,
     260,   262,   262,   264,   265,   265,   267,   267,   269,   269,
     271,   272,   272,   274,   274,   276,   276,   278,   280,   280,
     282,   284,   286,   289,   288,   292,   291,   294,   294,   296,
     296,   299,   299,   303,   302,   307,   306,   312,   312,   314,
     316,   316,   318,   319,   319,   321,   321,   323,   323,   325,
     327,   327,   329,   331,   331,   333,   333,   335,   335,   337,
     337,   339,   340,   340,   342,   342,   344,   345,   345,   348,
     348,   350,   350,   352,   352,   354,   354,   356,   356,   358,
     358,   360,   376,   384,   392,   400,   408,   408,   410,   412,
     412,   414,   415,   415,   419,   419,   421,   421,   423,   423,
     425,   425,   427,   427,   429,   430,   430,   434,   444,   452,
     462,   471,   479,   489,   501,   504,   507,   510,   511,   514,
     515,   516,   517,   518,   528,   538,   541,   556,   571,   572,
     575,   576,   577,   578,   579,   580,   581,   582,   583,   584,
     585,   588,   589,   592,   593,   594,   595,   596,   597,   598,
     599,   600,   601,   602,   603,   607,   610,   611,   614,   615,
     616,   624,   627,   628,   631,   635,   638,   639,   642,   644,
     646,   648,   650,   652,   654,   656,   658,   660,   662,   664,
     666,   668,   670,   672,   674,   676,   678,   680,   682,   684,
     688,   692,   693,   696,   705,   705,   716,   727,   730,   731,
     734,   735,   738,   741,   742,   745,   750,   753,   754,   757,
     760,   763,   764,   767,   771,   774,   775,   778,   782,   785,
     786,   789,   793,   796,   796,   801,   802,   802,   806,   807,
     815,   818,   819,   822,   827,   835,   838,   839,   842,   845,
     845,   851,   854,   855,   858,   861,   864,   867,   870,   875,
     878,   881,   884,   889,   892,   895,   898,   903,   906,   907,
     910,   915,   918,   919,   922,   922,   924,   924,   926,   926,
     930,   933,   934,   937,   945,   948,   949,   952,   960,   963,
     964,   967,   970,   981,   982,   985,   996,   999,  1000,  1003,
    1009,  1012,  1013,  1016,  1026,  1038,  1039,  1080,  1081,  1082,
    1085,  1097,  1103
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "LB", "RB", "LP", "RP", "MENUS", "MENU",
  "BUTTON", "DEFAULT_FUNCTION", "PLUS", "MINUS", "ALL", "OR", "CURSORS",
  "PIXMAPS", "ICONS", "COLOR", "SAVECOLOR", "MONOCHROME", "FUNCTION",
  "ICONMGR_SHOW", "ICONMGR", "ALTER", "WINDOW_FUNCTION", "ZOOM",
  "ICONMGRS", "ICONMGR_GEOMETRY", "ICONMGR_NOSHOW", "MAKE_TITLE",
  "ICONIFY_BY_UNMAPPING", "DONT_ICONIFY_BY_UNMAPPING", "AUTO_POPUP",
  "NO_BORDER", "NO_ICON_TITLE", "NO_TITLE", "AUTO_RAISE", "NO_HILITE",
  "ICON_REGION", "WINDOW_REGION", "META", "SHIFT", "LOCK", "CONTROL",
  "WINDOW", "TITLE", "ICON", "ROOT", "FRAME", "COLON", "EQUALS",
  "SQUEEZE_TITLE", "DONT_SQUEEZE_TITLE", "WARP_ON_DEICONIFY",
  "START_ICONIFIED", "NO_TITLE_HILITE", "TITLE_HILITE", "MOVE", "RESIZE",
  "WAITC", "SELECT", "KILL", "LEFT_TITLEBUTTON", "RIGHT_TITLEBUTTON",
  "NUMBER", "KEYWORD", "NKEYWORD", "CKEYWORD", "CLKEYWORD", "FKEYWORD",
  "FSKEYWORD", "FNKEYWORD", "PRIORITY_SWITCHING", "PRIORITY_NOT_SWITCHING",
  "SKEYWORD", "SSKEYWORD", "WINDOW_RING", "WINDOW_RING_EXCLUDE",
  "WARP_CURSOR", "ERRORTOKEN", "GRAVITY", "SIJENUM", "NO_STACKMODE",
  "ALWAYS_ON_TOP", "WORKSPACE", "WORKSPACES", "WORKSPCMGR_GEOMETRY",
  "OCCUPYALL", "OCCUPYLIST", "MAPWINDOWCURRENTWORKSPACE",
  "MAPWINDOWDEFAULTWORKSPACE", "ON_TOP_PRIORITY", "UNMAPBYMOVINGFARAWAY",
  "OPAQUEMOVE", "NOOPAQUEMOVE", "OPAQUERESIZE", "NOOPAQUERESIZE",
  "DONTSETINACTIVE", "CHANGE_WORKSPACE_FUNCTION", "DEICONIFY_FUNCTION",
  "ICONIFY_FUNCTION", "AUTOSQUEEZE", "STARTSQUEEZED", "DONT_SAVE",
  "AUTO_LOWER", "ICONMENU_DONTSHOW", "WINDOW_BOX", "IGNOREMODIFIER",
  "WINDOW_GEOMETRIES", "ALWAYSSQUEEZETOGRAVITY", "VIRTUAL_SCREENS",
  "IGNORE_TRANSIENT", "EWMH_IGNORE", "MWM_IGNORE", "RPLAY_SOUNDS",
  "FORCE_FOCUS", "STRING", "$accept", "twmrc", "$@1", "stmts", "stmt",
  "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11",
  "$@12", "$@13", "$@14", "$@15", "$@16", "$@17", "$@18", "$@19", "$@20",
  "$@21", "$@22", "$@23", "$@24", "$@25", "$@26", "$@27", "$@28", "$@29",
  "$@30", "$@31", "$@32", "$@33", "$@34", "$@35", "$@36", "$@37", "$@38",
  "$@39", "$@40", "$@41", "$@42", "$@43", "$@44", "$@45", "$@46", "$@47",
  "$@48", "$@49", "$@50", "$@51", "$@52", "$@53", "$@54", "$@55", "$@56",
  "$@57", "$@58", "$@59", "$@60", "$@61", "$@62", "$@63", "$@64", "$@65",
  "$@66", "$@67", "$@68", "noarg", "sarg", "narg", "keyaction", "full",
  "fullkey", "keys", "key", "vgrav", "hgrav", "contexts", "context",
  "contextkeys", "contextkey", "binding_list", "binding_entries",
  "binding_entry", "pixmap_list", "pixmap_entries", "pixmap_entry",
  "cursor_list", "cursor_entries", "cursor_entry", "color_list",
  "color_entries", "color_entry", "$@69", "save_color_list",
  "s_color_entries", "s_color_entry", "win_color_list",
  "win_color_entries", "win_color_entry", "wingeom_list",
  "wingeom_entries", "wingeom_entry", "geom_list", "geom_entries",
  "geom_entry", "ewmh_ignore_list", "ewmh_ignore_entries",
  "ewmh_ignore_entry", "mwm_ignore_list", "mwm_ignore_entries",
  "mwm_ignore_entry", "squeeze", "$@70", "$@71", "win_sqz_entries",
  "iconm_list", "iconm_entries", "iconm_entry", "workspc_list",
  "workspc_entries", "workspc_entry", "$@72", "workapp_list",
  "workapp_entries", "workapp_entry", "curwork", "defwork", "win_list",
  "win_entries", "win_entry", "occupy_list", "occupy_entries",
  "occupy_entry", "$@73", "$@74", "$@75", "occupy_workspc_list",
  "occupy_workspc_entries", "occupy_workspc_entry", "occupy_window_list",
  "occupy_window_entries", "occupy_window_entry", "icon_list",
  "icon_entries", "icon_entry", "rplay_sounds_list",
  "rplay_sounds_entries", "rplay_sounds_entry", "function",
  "function_entries", "function_entry", "menu", "menu_entries",
  "menu_entry", "action", "signed_number", "button", "string", "number", YY_NULLPTR
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
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372
};
# endif

#define YYPACT_NINF -242

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-242)))

#define YYTABLE_NINF -320

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -242,    34,  -242,  -242,   593,  -242,  -109,    -3,   -13,    62,
      67,  -242,  -242,  -242,  -242,  -109,  -242,   -13,    -3,  -242,
    -109,    68,  -242,    72,  -242,    76,  -242,    78,    80,    82,
      86,  -109,  -109,    87,    91,  -242,  -242,    93,  -109,  -109,
    -242,    -3,    89,    90,  -109,  -109,    95,  -242,   107,   119,
    -242,  -242,  -109,  -242,  -242,  -242,  -242,    44,  -242,   124,
     127,   135,   142,  -242,   -13,   -13,   -13,  -242,  -242,  -242,
     150,  -242,  -109,  -242,  -242,   155,  -242,  -242,  -242,  -242,
    -242,   156,  -242,  -242,  -242,  -242,  -242,  -242,     3,   110,
      79,  -242,  -242,  -242,  -109,  -242,  -242,  -242,  -242,  -242,
     159,   160,   161,   160,  -242,   162,  -242,  -242,   163,    -3,
     162,   162,   162,   162,   162,   162,   162,   162,   162,   162,
      88,    88,   164,   162,   162,   162,   162,   117,   121,  -242,
    -242,   162,  -242,   162,  -242,  -109,   162,   162,   162,   162,
     162,   167,    -3,   162,   170,   171,   172,    -3,    -3,    48,
     174,  -242,   162,   162,   162,   162,   162,   162,  -242,  -242,
    -242,   162,   162,   162,   162,   162,  -109,   175,   176,   162,
     177,   162,   178,   179,   180,   162,  -242,  -242,  -242,  -242,
    -242,  -242,  -109,   191,  -242,   401,    12,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,   192,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,   115,   115,  -242,  -242,  -242,  -242,  -242,   -13,   198,
     -13,   198,   162,  -242,   162,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -109,  -242,
    -109,  -242,  -242,  -242,   200,   162,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,   209,   372,   158,  -242,  -242,  -242,  -109,  -109,
    -109,  -109,  -109,  -109,  -109,  -109,  -109,  -109,  -109,  -242,
    -242,  -109,  -242,     8,    24,     7,  -242,  -242,     9,    11,
    -242,    -3,  -242,    14,  -242,  -242,  -242,  -242,  -242,  -242,
     162,    15,     6,    16,    17,   162,  -242,   162,    73,    18,
      19,    20,    23,    25,  -242,    -3,    -3,  -242,  -242,  -242,
    -242,  -242,  -242,  -109,    26,  -109,  -109,  -109,  -109,  -109,
    -109,  -109,  -109,  -109,  -109,  -109,  -242,  -242,  -242,  -109,
    -242,  -109,  -109,  -242,  -242,  -242,  -242,  -242,    41,  -242,
    -242,  -242,  -242,  -242,  -109,    -3,   162,  -242,   133,    43,
    -242,  -242,  -242,   203,  -242,  -109,  -109,  -242,  -242,  -242,
      27,  -242,    31,  -242,  -242,   205,  -242,  -242,  -109,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -109,  -242,  -242,   354,    54,   210,  -242,  -242,    81,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,   215,  -242,  -242,  -242,   -12,     1,  -242,    48,
    -242,  -242,    13,   216,  -242,  -242,   217,  -242,    32,  -242,
      33,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,   -13,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,   -13,  -242,  -242,
    -242,  -242,  -109,  -242,   218,    -3,  -242,   162,     2,    -3,
     -13,   -13,  -242,  -242,  -242,   217,   219,  -242,  -242,  -242,
     220,  -242,   221,  -242,  -242,   191,   190,  -242,  -242,  -242,
    -242,   162,     4,  -242,  -242,   443,  -242,    37,  -242,  -242,
    -242,    38,  -242,  -242,  -242,  -109,    39,  -242,   162,   238,
     -13,  -242,  -242,  -109,    40,  -242,  -242,  -242,   236,  -242,
    -242,  -109,  -242,   162,  -242,  -109,  -242,  -242,  -242,   -13,
    -242,  -242,  -109,  -242,  -109,  -242
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     4,     1,     0,     6,     0,     0,     0,     0,
       0,   153,   155,   157,   159,     0,    93,     0,    36,    91,
       0,    90,   135,    41,    64,   122,   120,   129,   132,   141,
     100,     0,     0,   302,   305,   142,   137,    97,     0,     0,
     187,     0,   109,   111,   189,   192,   171,   172,   168,   119,
     107,    66,     0,    71,    75,    31,    33,     0,    77,    44,
      47,    50,    53,    86,     0,     0,     0,    79,    81,   125,
     146,    73,     0,    68,   174,    83,   176,   133,   178,   180,
     182,   184,   381,     5,     7,     8,     9,    10,     0,     0,
     149,   382,   380,   375,     0,   161,   246,    38,   242,    37,
       0,     0,     0,     0,   151,     0,   162,    35,     0,    28,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    56,    58,   193,
     113,     0,   115,     0,   188,   191,     0,     0,     0,     0,
       0,     0,    30,     0,     0,     0,     0,     0,     0,     0,
     101,   377,     0,     0,     0,     0,     0,     0,   163,   164,
     165,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   197,    63,    61,    60,
     197,    62,     0,     0,   376,     0,     0,   359,   154,   271,
     156,   278,   158,   160,     0,   338,    94,   311,    92,    27,
      89,   136,    40,    65,   124,   121,   128,   131,   140,    99,
     206,     0,     0,   308,   307,   143,   138,    96,     0,     0,
       0,     0,     0,   110,     0,   112,   190,   170,   173,   167,
     118,   108,   316,    67,    29,    72,   342,    76,     0,    32,
       0,    34,   378,   379,   102,     0,    78,    43,    46,    49,
      52,    87,    80,    82,   126,   145,    74,    25,   197,   287,
     175,    85,   291,   177,   134,   295,   179,   299,   181,   363,
     183,   186,     0,     0,     0,   371,   150,   245,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   247,
     241,     0,   243,     0,     0,     0,   367,   152,     0,     0,
     207,     0,    23,     0,    54,   236,    57,    55,    59,   114,
       0,     0,     0,     0,     0,     0,   104,     0,    69,     0,
       0,     0,     0,     0,   205,     0,   199,   200,   201,   202,
     208,   198,   221,     0,     0,   265,   257,   255,   251,   253,
     249,   259,   261,   263,   267,   269,   244,   358,   360,     0,
     270,     0,     0,   272,   277,   281,   279,   280,     0,   337,
     339,   340,   310,   312,     0,     0,     0,   304,     0,     0,
     116,   315,   317,   318,   341,     0,     0,   343,   344,   329,
       0,   333,     0,   106,    26,     0,   286,   288,     0,   290,
     292,   293,   294,   296,   297,   298,   300,   301,   362,   364,
       0,   203,   204,     0,     0,     0,   370,   372,     0,   264,
     256,   254,   250,   252,   248,   258,   260,   262,   266,   268,
     361,   276,   273,   366,   368,   369,     0,    11,    24,     0,
     235,   237,     0,     0,   346,   348,     0,   330,     0,   334,
       0,    70,   289,   365,   219,   220,   216,   218,   217,   210,
     211,   212,   213,   214,     0,   215,   209,   232,   233,   229,
     231,   230,   223,   224,   225,   226,   227,     0,   228,   222,
     234,   147,     0,   373,     0,     0,   313,     0,    12,     0,
       0,   197,   238,   322,   320,     0,     0,   351,   345,   331,
       0,   335,     0,   195,   196,     0,     0,   283,   275,   314,
      16,     0,    13,   309,   240,     0,   239,     0,   347,   355,
     349,     0,   332,   336,   148,     0,     0,    18,     0,    14,
       0,   321,   323,   324,     0,   350,   352,   353,     0,   282,
     284,     0,    20,     0,   194,   325,   354,   356,   357,     0,
     285,    22,   326,   374,   327,   328
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -178,  -242,
     123,    35,  -242,  -242,  -242,  -242,    36,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,   143,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,    74,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -237,  -242,  -242,  -242,  -242,  -242,  -242,  -242,
    -242,  -242,  -242,  -242,  -242,  -242,  -242,  -241,  -242,  -242,
     -16,  -146,  -114,    -6,    -1
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,     2,     4,    83,   477,   501,   518,   533,   366,
     317,   145,   146,   112,   153,   154,   155,   156,   219,   221,
     113,   141,   167,   385,   143,   165,   144,   152,   161,   162,
     169,   157,   110,   108,   105,   126,   119,   245,   315,   140,
     131,   133,   222,   224,   139,   115,   114,   163,   116,   117,
     171,   111,   125,   118,   124,   164,   495,   183,   194,   100,
     101,   102,   103,   138,   136,   137,   168,   170,   172,   173,
     174,   175,    84,    85,    86,   482,   177,   181,   272,   331,
     211,   301,   403,   456,   404,   469,   306,   369,   431,    99,
     186,   292,    97,   185,   289,   190,   294,   353,   474,   192,
     295,   356,   498,   516,   530,   260,   319,   387,   263,   320,
     390,   266,   321,   393,   268,   322,   396,    87,   122,   123,
     303,   198,   299,   363,   233,   311,   372,   433,   484,   507,
     522,   239,   241,   196,   298,   360,   237,   312,   377,   436,
     485,   486,   488,   511,   526,   510,   524,   537,   188,   293,
     348,   270,   323,   399,   297,   358,   424,   276,   334,   407,
      95,   150,    88,    89,   151
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      90,   106,   273,   244,   -15,   -17,    92,   -19,    82,   104,
     374,   354,   347,   359,   109,   362,   290,   107,   367,   371,
     379,   381,   386,   389,   392,   120,   121,   395,   350,   398,
     406,   437,   127,   128,     3,   439,   489,   491,   134,   135,
     129,   521,   525,   529,   536,   423,   142,   430,   158,   159,
     160,   375,     7,    91,   176,   147,   148,    93,    94,   147,
     148,   149,    91,   480,   481,    96,   166,   457,   458,   291,
      98,   -88,   178,    93,    94,   -39,   355,   459,   460,  -123,
     318,  -127,   179,  -130,   182,  -139,   472,   324,   184,   -98,
    -303,   376,   351,   352,  -306,   461,   -95,   325,  -169,   462,
     463,   464,   465,   466,   467,    82,   130,   132,   199,    91,
    -166,    93,    94,    91,   326,   327,   328,   329,    82,    82,
      82,    82,  -117,    82,    82,    82,    82,   -42,    82,   226,
     -45,    82,    82,    82,    82,    82,    82,    82,   -48,   468,
      82,   234,    82,    82,    82,   -51,   242,   243,    82,    82,
      82,    93,    94,  -144,    82,    82,    82,    82,   -84,  -185,
     257,   180,   187,   189,   191,   195,   197,   213,   218,   210,
     232,    82,   220,   236,   238,   240,   274,  -103,   258,   259,
     262,   265,   267,   269,   200,   201,   202,   203,   204,   205,
     206,   207,   208,   209,   275,   296,   300,   214,   215,   216,
     217,   305,   304,  -105,   307,   223,  -319,   225,   333,   441,
     227,   228,   229,   230,   231,   429,   471,   235,  -274,   483,
     487,   497,   509,   324,   512,   513,   246,   247,   248,   249,
     250,   251,   313,   325,   314,   252,   253,   254,   255,   256,
     515,   -21,   539,   261,   212,   264,   193,   302,   508,   271,
     326,   327,   328,   329,   514,   432,     0,   308,     0,   330,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   335,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   479,     0,   346,     0,   349,     0,   357,
       0,     0,   361,   364,     0,     0,   309,   368,   310,     0,
     365,     0,     0,   505,     0,   373,   378,   380,   382,     0,
       0,     0,     0,   388,   391,   394,   397,   400,     0,   316,
       0,     0,     0,     0,   401,   402,     0,   405,   408,   409,
     410,   411,   412,   413,   414,   415,   416,   417,   418,   419,
       0,     0,   425,   420,     0,   421,   422,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   426,     0,
       0,     0,     0,     0,   427,     0,     0,   444,   445,   434,
     435,     0,     0,     0,   438,     0,   440,   446,   447,     0,
       0,     0,   442,     0,   370,     0,   324,     0,     0,   383,
       0,   384,   473,     0,   443,   448,   325,     0,   470,   449,
     450,   451,   452,   453,   454,   277,     0,     0,     0,   278,
     279,     0,     0,   326,   327,   328,   329,     0,     0,     0,
     475,   478,   332,     0,   280,   476,     0,     0,     0,     0,
       0,     0,   490,     0,   492,     0,     0,     0,   493,   455,
     428,     0,     0,     0,     0,     0,     0,   281,   282,     0,
     283,   494,     0,     0,     0,     0,     0,   324,     0,   284,
     285,   286,   287,   288,   504,   506,   496,   325,     0,     0,
       0,     0,   502,     0,   499,     0,     0,     0,   503,     0,
       0,     0,     0,     0,   326,   327,   328,   329,     0,     0,
       0,     0,     0,   520,     0,     0,   519,     0,     0,     0,
       0,   523,     0,     0,   534,   527,     0,     0,     0,   528,
     531,     0,     0,     0,     0,     0,     0,   535,   538,     0,
       0,     0,     0,   543,     0,   540,     0,     0,     0,   542,
       0,     0,     0,     0,     0,     0,   544,     0,   545,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   500,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   517,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   532,    -3,     5,     0,     0,     0,     0,     0,
       0,     6,     7,     8,     0,     0,     0,   541,     9,    10,
      11,    12,    13,    14,    15,    16,     0,     0,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    33,    34,    35,    36,    37,
       0,     0,     0,     0,     0,     0,    38,    39,     0,    40,
      41,     0,     0,     0,     0,     0,    42,    43,    44,    45,
      46,    47,    48,     0,     0,     0,    49,    50,     0,    51,
      52,    53,    54,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    77,    78,    79,    80,    81,
      82
};

static const yytype_int16 yycheck[] =
{
       6,    17,   180,   149,     3,     3,     7,     3,   117,    15,
       4,     4,     4,     4,    20,     4,     4,    18,     4,     4,
       4,     4,     4,     4,     4,    31,    32,     4,     4,     4,
       4,     4,    38,    39,     0,     4,     4,     4,    44,    45,
      41,     4,     4,     4,     4,     4,    52,     4,    64,    65,
      66,    45,     9,    65,    51,    11,    12,    70,    71,    11,
      12,    17,    65,    50,    51,     3,    72,    13,    14,    57,
       3,     3,    88,    70,    71,     3,    69,    23,    24,     3,
     258,     3,    88,     3,     5,     3,     5,    14,    94,     3,
       3,    85,    68,    69,     3,    41,     3,    24,     3,    45,
      46,    47,    48,    49,    50,   117,    17,    17,   109,    65,
       3,    70,    71,    65,    41,    42,    43,    44,   117,   117,
     117,   117,     3,   117,   117,   117,   117,     3,   117,   135,
       3,   117,   117,   117,   117,   117,   117,   117,     3,    85,
     117,   142,   117,   117,   117,     3,   147,   148,   117,   117,
     117,    70,    71,     3,   117,   117,   117,   117,     3,     3,
     166,    51,     3,     3,     3,     3,     3,     3,    51,    81,
       3,   117,    51,     3,     3,     3,   182,     3,     3,     3,
       3,     3,     3,     3,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,     3,     3,    81,   123,   124,   125,
     126,     3,   218,     3,   220,   131,     3,   133,    50,     4,
     136,   137,   138,   139,   140,    82,     6,   143,     3,     3,
       3,     3,     3,    14,     4,     4,   152,   153,   154,   155,
     156,   157,   238,    24,   240,   161,   162,   163,   164,   165,
      50,     3,     6,   169,   121,   171,   103,   212,   485,   175,
      41,    42,    43,    44,   495,   369,    -1,   221,    -1,    50,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   278,   279,   280,   281,   282,   283,   284,   285,
     286,   287,   288,   429,    -1,   291,    -1,   293,    -1,   295,
      -1,    -1,   298,   299,    -1,    -1,   222,   303,   224,    -1,
     301,    -1,    -1,   481,    -1,   311,   312,   313,   314,    -1,
      -1,    -1,    -1,   319,   320,   321,   322,   323,    -1,   245,
      -1,    -1,    -1,    -1,   325,   326,    -1,   333,   334,   335,
     336,   337,   338,   339,   340,   341,   342,   343,   344,   345,
      -1,    -1,   358,   349,    -1,   351,   352,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   364,    -1,
      -1,    -1,    -1,    -1,   365,    -1,    -1,    13,    14,   375,
     376,    -1,    -1,    -1,   380,    -1,   382,    23,    24,    -1,
      -1,    -1,   388,    -1,   310,    -1,    14,    -1,    -1,   315,
      -1,   317,   408,    -1,   400,    41,    24,    -1,   404,    45,
      46,    47,    48,    49,    50,     4,    -1,    -1,    -1,     8,
       9,    -1,    -1,    41,    42,    43,    44,    -1,    -1,    -1,
     426,   427,    50,    -1,    23,   426,    -1,    -1,    -1,    -1,
      -1,    -1,   438,    -1,   440,    -1,    -1,    -1,   454,    85,
     366,    -1,    -1,    -1,    -1,    -1,    -1,    46,    47,    -1,
      49,   467,    -1,    -1,    -1,    -1,    -1,    14,    -1,    58,
      59,    60,    61,    62,   480,   481,   472,    24,    -1,    -1,
      -1,    -1,   478,    -1,   475,    -1,    -1,    -1,   479,    -1,
      -1,    -1,    -1,    -1,    41,    42,    43,    44,    -1,    -1,
      -1,    -1,    -1,    50,    -1,    -1,   502,    -1,    -1,    -1,
      -1,   507,    -1,    -1,   520,   511,    -1,    -1,    -1,   515,
     516,    -1,    -1,    -1,    -1,    -1,    -1,   523,   524,    -1,
      -1,    -1,    -1,   539,    -1,   531,    -1,    -1,    -1,   535,
      -1,    -1,    -1,    -1,    -1,    -1,   542,    -1,   544,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   477,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   501,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   518,     0,     1,    -1,    -1,    -1,    -1,    -1,
      -1,     8,     9,    10,    -1,    -1,    -1,   533,    15,    16,
      17,    18,    19,    20,    21,    22,    -1,    -1,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    52,    53,    54,    55,    56,
      -1,    -1,    -1,    -1,    -1,    -1,    63,    64,    -1,    66,
      67,    -1,    -1,    -1,    -1,    -1,    73,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    83,    84,    -1,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   104,   105,   106,
     107,   108,   109,   110,   111,   112,   113,   114,   115,   116,
     117
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint16 yystos[] =
{
       0,   119,   120,     0,   121,     1,     8,     9,    10,    15,
      16,    17,    18,    19,    20,    21,    22,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    52,    53,    54,    55,    56,    63,    64,
      66,    67,    73,    74,    75,    76,    77,    78,    79,    83,
      84,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   122,   190,   191,   192,   235,   280,   281,
     281,    65,   282,    70,    71,   278,     3,   210,     3,   207,
     177,   178,   179,   180,   281,   152,   278,   282,   151,   281,
     150,   169,   131,   138,   164,   163,   166,   167,   171,   154,
     281,   281,   236,   237,   172,   170,   153,   281,   281,   282,
      17,   158,    17,   159,   281,   281,   182,   183,   181,   162,
     157,   139,   281,   142,   144,   129,   130,    11,    12,    17,
     279,   282,   145,   132,   133,   134,   135,   149,   278,   278,
     278,   146,   147,   165,   173,   143,   281,   140,   184,   148,
     185,   168,   186,   187,   188,   189,    51,   194,   278,   281,
      51,   195,     5,   175,   281,   211,   208,     3,   266,     3,
     213,     3,   217,   213,   176,     3,   251,     3,   239,   282,
     251,   251,   251,   251,   251,   251,   251,   251,   251,   251,
      81,   198,   198,     3,   251,   251,   251,   251,    51,   136,
      51,   137,   160,   251,   161,   251,   281,   251,   251,   251,
     251,   251,     3,   242,   282,   251,     3,   254,     3,   249,
       3,   250,   282,   282,   279,   155,   251,   251,   251,   251,
     251,   251,   251,   251,   251,   251,   251,   281,     3,     3,
     223,   251,     3,   226,   251,     3,   229,     3,   232,     3,
     269,   251,   196,   196,   281,     3,   275,     4,     8,     9,
      23,    46,    47,    49,    58,    59,    60,    61,    62,   212,
       4,    57,   209,   267,   214,   218,     3,   272,   252,   240,
      81,   199,   199,   238,   278,     3,   204,   278,   204,   251,
     251,   243,   255,   281,   281,   156,   251,   128,   196,   224,
     227,   230,   233,   270,    14,    24,    41,    42,    43,    44,
      50,   197,    50,    50,   276,   281,   281,   281,   281,   281,
     281,   281,   281,   281,   281,   281,   281,     4,   268,   281,
       4,    68,    69,   215,     4,    69,   219,   281,   273,     4,
     253,   281,     4,   241,   281,   282,   127,     4,   281,   205,
     251,     4,   244,   281,     4,    45,    85,   256,   281,     4,
     281,     4,   281,   251,   251,   141,     4,   225,   281,     4,
     228,   281,     4,   231,   281,     4,   234,   281,     4,   271,
     281,   282,   282,   200,   202,   281,     4,   277,   281,   281,
     281,   281,   281,   281,   281,   281,   281,   281,   281,   281,
     281,   281,   281,     4,   274,   278,   281,   282,   251,    82,
       4,   206,   280,   245,   281,   281,   257,     4,   281,     4,
     281,     4,   281,   281,    13,    14,    23,    24,    41,    45,
      46,    47,    48,    49,    50,    85,   201,    13,    14,    23,
      24,    41,    45,    46,    47,    48,    49,    50,    85,   203,
     281,     6,     5,   278,   216,   281,   282,   123,   281,   279,
      50,    51,   193,     3,   246,   258,   259,     3,   260,     4,
     281,     4,   281,   278,   278,   174,   281,     3,   220,   282,
     251,   124,   281,   282,   278,   196,   278,   247,   260,     3,
     263,   261,     4,     4,   275,    50,   221,   251,   125,   281,
      50,     4,   248,   281,   264,     4,   262,   281,   281,     4,
     222,   281,   251,   126,   278,   281,     4,   265,   281,     6,
     281,   251,   281,   278,   281,   281
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint16 yyr1[] =
{
       0,   118,   120,   119,   121,   121,   122,   122,   122,   122,
     122,   122,   122,   122,   122,   123,   122,   124,   122,   125,
     122,   126,   122,   127,   122,   128,   122,   122,   122,   122,
     122,   129,   122,   130,   122,   122,   122,   122,   122,   131,
     122,   122,   132,   122,   122,   133,   122,   122,   134,   122,
     122,   135,   122,   122,   122,   122,   136,   122,   137,   122,
     122,   122,   122,   122,   138,   122,   139,   122,   140,   141,
     122,   142,   122,   143,   122,   144,   122,   145,   122,   146,
     122,   147,   122,   122,   148,   122,   149,   122,   150,   122,
     122,   151,   122,   152,   122,   153,   122,   122,   154,   122,
     122,   122,   122,   155,   122,   156,   122,   157,   122,   158,
     122,   159,   122,   160,   122,   161,   122,   162,   122,   122,
     163,   122,   122,   164,   122,   165,   122,   166,   122,   122,
     167,   122,   122,   168,   122,   169,   122,   170,   122,   171,
     122,   122,   172,   122,   173,   122,   122,   174,   122,   175,
     122,   176,   122,   177,   122,   178,   122,   179,   122,   180,
     122,   122,   122,   122,   122,   122,   181,   122,   122,   182,
     122,   122,   183,   122,   184,   122,   185,   122,   186,   122,
     187,   122,   188,   122,   122,   189,   122,   190,   191,   191,
     191,   191,   191,   192,   193,   194,   195,   196,   196,   197,
     197,   197,   197,   197,   197,   197,   198,   199,   200,   200,
     201,   201,   201,   201,   201,   201,   201,   201,   201,   201,
     201,   202,   202,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   203,   203,   203,   204,   205,   205,   206,   206,
     206,   207,   208,   208,   209,   210,   211,   211,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     212,   212,   212,   212,   212,   212,   212,   212,   212,   212,
     213,   214,   214,   215,   216,   215,   215,   217,   218,   218,
     219,   219,   220,   221,   221,   222,   223,   224,   224,   225,
     226,   227,   227,   228,   229,   230,   230,   231,   232,   233,
     233,   234,   235,   236,   235,   235,   237,   235,   238,   238,
     239,   240,   240,   241,   241,   242,   243,   243,   244,   245,
     244,   246,   247,   247,   248,   248,   248,   248,   248,   249,
     249,   249,   249,   250,   250,   250,   250,   251,   252,   252,
     253,   254,   255,   255,   257,   256,   258,   256,   259,   256,
     260,   261,   261,   262,   263,   264,   264,   265,   266,   267,
     267,   268,   269,   270,   270,   271,   272,   273,   273,   274,
     275,   276,   276,   277,   277,   278,   278,   279,   279,   279,
     280,   281,   282
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     0,     2,     1,     1,     1,     1,
       1,     6,     7,     8,     9,     0,     8,     0,     9,     0,
      10,     0,    11,     0,     6,     0,     5,     3,     2,     3,
       2,     0,     3,     0,     3,     2,     1,     2,     2,     0,
       3,     1,     0,     3,     1,     0,     3,     1,     0,     3,
       1,     0,     3,     1,     4,     4,     0,     4,     0,     4,
       2,     2,     2,     2,     0,     3,     0,     3,     0,     0,
       6,     0,     3,     0,     3,     0,     3,     0,     3,     0,
       3,     0,     3,     1,     0,     3,     0,     3,     0,     3,
       1,     0,     3,     0,     3,     0,     3,     1,     0,     3,
       1,     2,     3,     0,     4,     0,     5,     0,     3,     0,
       3,     0,     3,     0,     4,     0,     5,     0,     3,     1,
       0,     3,     1,     0,     3,     0,     3,     0,     3,     1,
       0,     3,     1,     0,     3,     0,     3,     0,     3,     0,
       3,     1,     0,     3,     0,     3,     1,     0,     9,     0,
       4,     0,     4,     0,     3,     0,     3,     0,     3,     0,
       3,     2,     2,     2,     2,     2,     0,     3,     1,     0,
       3,     1,     0,     3,     0,     3,     0,     3,     0,     3,
       0,     3,     0,     3,     1,     0,     3,     1,     2,     1,
       3,     2,     1,     2,     4,     6,     6,     0,     2,     1,
       1,     1,     1,     2,     2,     1,     1,     1,     0,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     0,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     3,     0,     2,     2,     3,
       3,     3,     0,     2,     2,     3,     0,     2,     3,     2,
       3,     2,     3,     2,     3,     2,     3,     2,     3,     2,
       3,     2,     3,     2,     3,     2,     3,     2,     3,     2,
       3,     0,     2,     2,     0,     4,     2,     3,     0,     2,
       1,     1,     3,     0,     2,     2,     3,     0,     2,     2,
       3,     0,     2,     1,     3,     0,     2,     1,     3,     0,
       2,     1,     1,     0,     5,     1,     0,     3,     0,     5,
       3,     0,     2,     3,     4,     3,     0,     2,     1,     0,
       3,     3,     0,     2,     1,     2,     3,     4,     5,     3,
       4,     5,     6,     3,     4,     5,     6,     3,     0,     2,
       1,     3,     0,     2,     0,     3,     0,     4,     0,     4,
       3,     0,     2,     1,     3,     0,     2,     1,     3,     0,
       2,     2,     3,     0,     2,     2,     3,     0,     2,     1,
       3,     0,     2,     2,     7,     1,     2,     1,     2,     2,
       2,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
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


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
  YYFPRINTF (yyo, ")");
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
  unsigned long yylno = yyrline[yyrule];
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
                       &yyvsp[(yyi + 1) - (yynrhs)]
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
            else
              goto append;

          append:
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

  return (YYSIZE_T) (yystpcpy (yyres, yystr) - yyres);
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
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
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
    default: /* Avoid compiler warnings. */
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
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
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
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yynewstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  *yyssp = (yytype_int16) yystate;

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = (YYSIZE_T) (yyssp - yyss + 1);

# if defined yyoverflow
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
# else /* defined YYSTACK_RELOCATE */
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
# undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */

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
| yyreduce -- do a reduction.  |
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
  case 2:
#line 106 "gram.y"
    { InitGramVariables(); }
#line 1882 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 11:
#line 119 "gram.y"
    {
		      AddIconRegion((yyvsp[-4].ptr), (yyvsp[-3].num), (yyvsp[-2].num), (yyvsp[-1].num), (yyvsp[0].num), "undef", "undef", "undef");
		  }
#line 1890 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 12:
#line 122 "gram.y"
    {
		      AddIconRegion((yyvsp[-5].ptr), (yyvsp[-4].num), (yyvsp[-3].num), (yyvsp[-2].num), (yyvsp[-1].num), (yyvsp[0].ptr), "undef", "undef");
		  }
#line 1898 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 13:
#line 125 "gram.y"
    {
		      AddIconRegion((yyvsp[-6].ptr), (yyvsp[-5].num), (yyvsp[-4].num), (yyvsp[-3].num), (yyvsp[-2].num), (yyvsp[-1].ptr), (yyvsp[0].ptr), "undef");
		  }
#line 1906 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 14:
#line 128 "gram.y"
    {
		      AddIconRegion((yyvsp[-7].ptr), (yyvsp[-6].num), (yyvsp[-5].num), (yyvsp[-4].num), (yyvsp[-3].num), (yyvsp[-2].ptr), (yyvsp[-1].ptr), (yyvsp[0].ptr));
		  }
#line 1914 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 15:
#line 131 "gram.y"
    {
		      curplist = AddIconRegion((yyvsp[-4].ptr), (yyvsp[-3].num), (yyvsp[-2].num), (yyvsp[-1].num), (yyvsp[0].num), "undef", "undef", "undef");
		  }
#line 1922 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 17:
#line 135 "gram.y"
    {
		      curplist = AddIconRegion((yyvsp[-5].ptr), (yyvsp[-4].num), (yyvsp[-3].num), (yyvsp[-2].num), (yyvsp[-1].num), (yyvsp[0].ptr), "undef", "undef");
		  }
#line 1930 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 19:
#line 139 "gram.y"
    {
		      curplist = AddIconRegion((yyvsp[-6].ptr), (yyvsp[-5].num), (yyvsp[-4].num), (yyvsp[-3].num), (yyvsp[-2].num), (yyvsp[-1].ptr), (yyvsp[0].ptr), "undef");
		  }
#line 1938 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 21:
#line 143 "gram.y"
    {
		      curplist = AddIconRegion((yyvsp[-7].ptr), (yyvsp[-6].num), (yyvsp[-5].num), (yyvsp[-4].num), (yyvsp[-3].num), (yyvsp[-2].ptr), (yyvsp[-1].ptr), (yyvsp[0].ptr));
		  }
#line 1946 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 23:
#line 148 "gram.y"
    {
		      curplist = AddWindowRegion ((yyvsp[-2].ptr), (yyvsp[-1].num), (yyvsp[0].num));
		  }
#line 1954 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 25:
#line 153 "gram.y"
    {
		      curplist = addWindowBox ((yyvsp[-1].ptr), (yyvsp[0].ptr));
		  }
#line 1962 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 27:
#line 158 "gram.y"
    { if (Scr->FirstTime)
						  {
						    Scr->iconmgr->geometry= (char*)(yyvsp[-1].ptr);
						    Scr->iconmgr->columns=(yyvsp[0].num);
						  }
						}
#line 1973 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 28:
#line 164 "gram.y"
    { if (Scr->FirstTime)
						    Scr->iconmgr->geometry = (char*)(yyvsp[0].ptr);
						}
#line 1981 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 29:
#line 167 "gram.y"
    { if (Scr->FirstTime)
				{
				    Scr->workSpaceMgr.geometry= (char*)(yyvsp[-1].ptr);
				    Scr->workSpaceMgr.columns=(yyvsp[0].num);
				}
						}
#line 1992 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 30:
#line 173 "gram.y"
    { if (Scr->FirstTime)
				    Scr->workSpaceMgr.geometry = (char*)(yyvsp[0].ptr);
						}
#line 2000 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 31:
#line 176 "gram.y"
    {}
#line 2006 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 33:
#line 179 "gram.y"
    {}
#line 2012 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 35:
#line 182 "gram.y"
    { if (Scr->FirstTime)
					  {
						Scr->DoZoom = true;
						Scr->ZoomCount = (yyvsp[0].num);
					  }
					}
#line 2023 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 36:
#line 188 "gram.y"
    { if (Scr->FirstTime)
						Scr->DoZoom = true; }
#line 2030 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 37:
#line 190 "gram.y"
    {}
#line 2036 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 38:
#line 191 "gram.y"
    {}
#line 2042 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 39:
#line 192 "gram.y"
    { curplist = &Scr->IconifyByUn; }
#line 2048 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 41:
#line 194 "gram.y"
    { if (Scr->FirstTime)
		    Scr->IconifyByUnmapping = true; }
#line 2055 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 42:
#line 197 "gram.y"
    { curplist = &Scr->OpaqueMoveList; }
#line 2061 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 44:
#line 199 "gram.y"
    { if (Scr->FirstTime) Scr->DoOpaqueMove = true; }
#line 2067 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 45:
#line 200 "gram.y"
    { curplist = &Scr->NoOpaqueMoveList; }
#line 2073 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 47:
#line 202 "gram.y"
    { if (Scr->FirstTime) Scr->DoOpaqueMove = false; }
#line 2079 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 48:
#line 203 "gram.y"
    { curplist = &Scr->OpaqueMoveList; }
#line 2085 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 50:
#line 205 "gram.y"
    { if (Scr->FirstTime) Scr->DoOpaqueResize = true; }
#line 2091 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 51:
#line 206 "gram.y"
    { curplist = &Scr->NoOpaqueResizeList; }
#line 2097 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 53:
#line 208 "gram.y"
    { if (Scr->FirstTime) Scr->DoOpaqueResize = false; }
#line 2103 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 54:
#line 210 "gram.y"
    {
					  GotTitleButton ((yyvsp[-2].ptr), (yyvsp[0].num), false);
					}
#line 2111 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 55:
#line 213 "gram.y"
    {
					  GotTitleButton ((yyvsp[-2].ptr), (yyvsp[0].num), true);
					}
#line 2119 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 56:
#line 216 "gram.y"
    { CreateTitleButton((yyvsp[0].ptr), 0, NULL, NULL, false, true); }
#line 2125 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 58:
#line 218 "gram.y"
    { CreateTitleButton((yyvsp[0].ptr), 0, NULL, NULL, true, true); }
#line 2131 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 60:
#line 220 "gram.y"
    {
		    root = GetRoot((yyvsp[0].ptr), NULL, NULL);
		    AddFuncButton ((yyvsp[-1].num), C_ROOT, 0, F_MENU, root, NULL);
		}
#line 2140 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 61:
#line 224 "gram.y"
    {
			if ((yyvsp[0].num) == F_MENU) {
			    pull->prev = NULL;
			    AddFuncButton ((yyvsp[-1].num), C_ROOT, 0, (yyvsp[0].num), pull, NULL);
			}
			else {
			    MenuItem *item;

			    root = GetRoot(TWM_ROOT,NULL,NULL);
			    item = AddToMenu (root, "x", Action,
					NULL, (yyvsp[0].num), NULL, NULL);
			    AddFuncButton ((yyvsp[-1].num), C_ROOT, 0, (yyvsp[0].num), NULL, item);
			}
			Action = "";
			pull = NULL;
		}
#line 2161 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 62:
#line 240 "gram.y"
    { GotKey((yyvsp[-1].ptr), (yyvsp[0].num)); }
#line 2167 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 63:
#line 241 "gram.y"
    { GotButton((yyvsp[-1].num), (yyvsp[0].num)); }
#line 2173 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 64:
#line 243 "gram.y"
    { curplist = &Scr->DontIconify; }
#line 2179 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 66:
#line 245 "gram.y"
    {}
#line 2185 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 68:
#line 248 "gram.y"
    { mods = 0; }
#line 2191 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 69:
#line 250 "gram.y"
    { Scr->IgnoreModifier |= mods; mods = 0; }
#line 2197 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 71:
#line 252 "gram.y"
    { curplist = &Scr->OccupyAll; }
#line 2203 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 73:
#line 254 "gram.y"
    { curplist = &Scr->IconMenuDontShow; }
#line 2209 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 75:
#line 256 "gram.y"
    {}
#line 2215 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 77:
#line 258 "gram.y"
    { curplist = &Scr->UnmapByMovingFarAway; }
#line 2221 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 79:
#line 260 "gram.y"
    { curplist = &Scr->AutoSqueeze; }
#line 2227 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 81:
#line 262 "gram.y"
    { curplist = &Scr->StartSqueezed; }
#line 2233 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 83:
#line 264 "gram.y"
    { Scr->AlwaysSqueezeToGravity = true; }
#line 2239 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 84:
#line 265 "gram.y"
    { curplist = &Scr->AlwaysSqueezeToGravityL; }
#line 2245 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 86:
#line 267 "gram.y"
    { curplist = &Scr->DontSetInactive; }
#line 2251 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 88:
#line 269 "gram.y"
    { curplist = &Scr->IconMgrNoShow; }
#line 2257 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 90:
#line 271 "gram.y"
    { Scr->IconManagerDontShow = true; }
#line 2263 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 91:
#line 272 "gram.y"
    { curplist = &Scr->IconMgrs; }
#line 2269 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 93:
#line 274 "gram.y"
    { curplist = &Scr->IconMgrShow; }
#line 2275 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 95:
#line 276 "gram.y"
    { curplist = &Scr->NoTitleHighlight; }
#line 2281 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 97:
#line 278 "gram.y"
    { if (Scr->FirstTime)
						Scr->TitleHighlight = false; }
#line 2288 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 98:
#line 280 "gram.y"
    { curplist = &Scr->NoHighlight; }
#line 2294 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 100:
#line 282 "gram.y"
    { if (Scr->FirstTime)
						Scr->Highlight = false; }
#line 2301 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 101:
#line 285 "gram.y"
    { OtpScrSetZero(Scr, WinWin, (yyvsp[0].num)); }
#line 2307 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 102:
#line 287 "gram.y"
    { OtpScrSetZero(Scr, IconWin, (yyvsp[0].num)); }
#line 2313 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 103:
#line 289 "gram.y"
    { curplist = OtpScrPriorityL(Scr, WinWin, (yyvsp[0].num)); }
#line 2319 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 105:
#line 292 "gram.y"
    { curplist = OtpScrPriorityL(Scr, IconWin, (yyvsp[0].num)); }
#line 2325 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 107:
#line 294 "gram.y"
    { curplist = OtpScrPriorityL(Scr, WinWin, 8); }
#line 2331 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 109:
#line 296 "gram.y"
    { OtpScrSetSwitching(Scr, WinWin, false);
		                          curplist = OtpScrSwitchingL(Scr, WinWin); }
#line 2338 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 111:
#line 299 "gram.y"
    { OtpScrSetSwitching(Scr, WinWin, true);
		                          curplist = OtpScrSwitchingL(Scr, WinWin); }
#line 2345 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 113:
#line 303 "gram.y"
    { OtpScrSetSwitching(Scr, IconWin, false);
                                        curplist = OtpScrSwitchingL(Scr, IconWin); }
#line 2352 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 115:
#line 307 "gram.y"
    { OtpScrSetSwitching(Scr, IconWin, true);
		                          curplist = OtpScrSwitchingL(Scr, IconWin); }
#line 2359 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 117:
#line 312 "gram.y"
    { curplist = &Scr->NoStackModeL; }
#line 2365 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 119:
#line 314 "gram.y"
    { if (Scr->FirstTime)
						Scr->StackMode = false; }
#line 2372 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 120:
#line 316 "gram.y"
    { curplist = &Scr->NoBorder; }
#line 2378 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 122:
#line 318 "gram.y"
    { Scr->AutoPopup = true; }
#line 2384 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 123:
#line 319 "gram.y"
    { curplist = &Scr->AutoPopupL; }
#line 2390 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 125:
#line 321 "gram.y"
    { curplist = &Scr->DontSave; }
#line 2396 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 127:
#line 323 "gram.y"
    { curplist = &Scr->NoIconTitle; }
#line 2402 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 129:
#line 325 "gram.y"
    { if (Scr->FirstTime)
						Scr->NoIconTitlebar = true; }
#line 2409 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 130:
#line 327 "gram.y"
    { curplist = &Scr->NoTitle; }
#line 2415 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 132:
#line 329 "gram.y"
    { if (Scr->FirstTime)
						Scr->NoTitlebar = true; }
#line 2422 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 133:
#line 331 "gram.y"
    { curplist = &Scr->IgnoreTransientL; }
#line 2428 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 135:
#line 333 "gram.y"
    { curplist = &Scr->MakeTitle; }
#line 2434 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 137:
#line 335 "gram.y"
    { curplist = &Scr->StartIconified; }
#line 2440 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 139:
#line 337 "gram.y"
    { curplist = &Scr->AutoRaise; }
#line 2446 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 141:
#line 339 "gram.y"
    { Scr->AutoRaiseDefault = true; }
#line 2452 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 142:
#line 340 "gram.y"
    { curplist = &Scr->WarpOnDeIconify; }
#line 2458 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 144:
#line 342 "gram.y"
    { curplist = &Scr->AutoLower; }
#line 2464 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 146:
#line 344 "gram.y"
    { Scr->AutoLowerDefault = true; }
#line 2470 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 147:
#line 345 "gram.y"
    {
					root = GetRoot((yyvsp[-5].ptr), (yyvsp[-3].ptr), (yyvsp[-1].ptr)); }
#line 2477 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 148:
#line 347 "gram.y"
    { root->real_menu = true;}
#line 2483 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 149:
#line 348 "gram.y"
    { root = GetRoot((yyvsp[0].ptr), NULL, NULL); }
#line 2489 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 150:
#line 349 "gram.y"
    { root->real_menu = true; }
#line 2495 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 151:
#line 350 "gram.y"
    { root = GetRoot((yyvsp[0].ptr), NULL, NULL); }
#line 2501 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 153:
#line 352 "gram.y"
    { curplist = &Scr->IconNames; }
#line 2507 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 155:
#line 354 "gram.y"
    { color = COLOR; }
#line 2513 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 157:
#line 356 "gram.y"
    {}
#line 2519 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 159:
#line 358 "gram.y"
    { color = MONOCHROME; }
#line 2525 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 161:
#line 360 "gram.y"
    { Scr->DefaultFunction.func = (yyvsp[0].num);
					  if ((yyvsp[0].num) == F_MENU)
					  {
					    pull->prev = NULL;
					    Scr->DefaultFunction.menu = pull;
					  }
					  else
					  {
					    root = GetRoot(TWM_ROOT,NULL,NULL);
					    Scr->DefaultFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,(yyvsp[0].num), NULL, NULL);
					  }
					  Action = "";
					  pull = NULL;
					}
#line 2546 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 162:
#line 376 "gram.y"
    { Scr->WindowFunction.func = (yyvsp[0].num);
					   root = GetRoot(TWM_ROOT,NULL,NULL);
					   Scr->WindowFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,(yyvsp[0].num), NULL, NULL);
					   Action = "";
					   pull = NULL;
					}
#line 2559 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 163:
#line 384 "gram.y"
    { Scr->ChangeWorkspaceFunction.func = (yyvsp[0].num);
					   root = GetRoot(TWM_ROOT,NULL,NULL);
					   Scr->ChangeWorkspaceFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,(yyvsp[0].num), NULL, NULL);
					   Action = "";
					   pull = NULL;
					}
#line 2572 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 164:
#line 392 "gram.y"
    { Scr->DeIconifyFunction.func = (yyvsp[0].num);
					   root = GetRoot(TWM_ROOT,NULL,NULL);
					   Scr->DeIconifyFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,(yyvsp[0].num), NULL, NULL);
					   Action = "";
					   pull = NULL;
					}
#line 2585 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 165:
#line 400 "gram.y"
    { Scr->IconifyFunction.func = (yyvsp[0].num);
					   root = GetRoot(TWM_ROOT,NULL,NULL);
					   Scr->IconifyFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,(yyvsp[0].num), NULL, NULL);
					   Action = "";
					   pull = NULL;
					}
#line 2598 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 166:
#line 408 "gram.y"
    { curplist = &Scr->WarpCursorL; }
#line 2604 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 168:
#line 410 "gram.y"
    { if (Scr->FirstTime)
					    Scr->WarpCursor = true; }
#line 2611 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 169:
#line 412 "gram.y"
    { curplist = &Scr->WindowRingL; }
#line 2617 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 171:
#line 414 "gram.y"
    { Scr->WindowRingAll = true; }
#line 2623 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 172:
#line 415 "gram.y"
    { if (!Scr->WindowRingL)
					    Scr->WindowRingAll = true;
					  curplist = &Scr->WindowRingExcludeL; }
#line 2631 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 174:
#line 419 "gram.y"
    {  }
#line 2637 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 176:
#line 421 "gram.y"
    { }
#line 2643 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 178:
#line 423 "gram.y"
    { }
#line 2649 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 180:
#line 425 "gram.y"
    { }
#line 2655 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 182:
#line 427 "gram.y"
    { }
#line 2661 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 184:
#line 429 "gram.y"
    { Scr->ForceFocus = true; }
#line 2667 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 185:
#line 430 "gram.y"
    { curplist = &Scr->ForceFocusL; }
#line 2673 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 187:
#line 434 "gram.y"
    { if (!do_single_keyword ((yyvsp[0].num))) {
					    twmrc_error_prefix();
					    fprintf (stderr,
					"unknown singleton keyword %d\n",
						     (yyvsp[0].num));
					    ParseError = true;
					  }
					}
#line 2686 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 188:
#line 444 "gram.y"
    { if (!do_string_keyword ((yyvsp[-1].num), (yyvsp[0].ptr))) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (value \"%s\")\n",
						     (yyvsp[-1].num), (yyvsp[0].ptr));
					    ParseError = true;
					  }
					}
#line 2699 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 189:
#line 452 "gram.y"
    { if (!do_string_keyword ((yyvsp[0].num), DEFSTRING)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (no value)\n",
						     (yyvsp[0].num));
					    ParseError = true;
					  }
					}
#line 2712 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 190:
#line 463 "gram.y"
    { if (!do_string_string_keyword ((yyvsp[-2].num), (yyvsp[-1].ptr), (yyvsp[0].ptr))) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown strings keyword %d (value \"%s\" and \"%s\")\n",
						     (yyvsp[-2].num), (yyvsp[-1].ptr), (yyvsp[0].ptr));
					    ParseError = true;
					  }
					}
#line 2725 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 191:
#line 471 "gram.y"
    { if (!do_string_string_keyword ((yyvsp[-1].num), (yyvsp[0].ptr), NULL)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (value \"%s\")\n",
						     (yyvsp[-1].num), (yyvsp[0].ptr));
					    ParseError = true;
					  }
					}
#line 2738 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 192:
#line 479 "gram.y"
    { if (!do_string_string_keyword ((yyvsp[0].num), NULL, NULL)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (no value)\n",
						     (yyvsp[0].num));
					    ParseError = true;
					  }
					}
#line 2751 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 193:
#line 489 "gram.y"
    { if (!do_number_keyword ((yyvsp[-1].num), (yyvsp[0].num))) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown numeric keyword %d (value %d)\n",
						     (yyvsp[-1].num), (yyvsp[0].num));
					    ParseError = true;
					  }
					}
#line 2764 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 194:
#line 501 "gram.y"
    { (yyval.num) = (yyvsp[0].num); }
#line 2770 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 195:
#line 504 "gram.y"
    { (yyval.num) = (yyvsp[0].num); }
#line 2776 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 196:
#line 507 "gram.y"
    { (yyval.num) = (yyvsp[0].num); }
#line 2782 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 199:
#line 514 "gram.y"
    { mods |= Mod1Mask; }
#line 2788 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 200:
#line 515 "gram.y"
    { mods |= ShiftMask; }
#line 2794 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 201:
#line 516 "gram.y"
    { mods |= LockMask; }
#line 2800 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 202:
#line 517 "gram.y"
    { mods |= ControlMask; }
#line 2806 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 203:
#line 518 "gram.y"
    { if ((yyvsp[0].num) < 1 || (yyvsp[0].num) > 5) {
					     twmrc_error_prefix();
					     fprintf (stderr,
				"bad altkeymap number (%d), must be 1-5\n",
						      (yyvsp[0].num));
					     ParseError = true;
					  } else {
					     mods |= (Alt1Mask << ((yyvsp[0].num) - 1));
					  }
					}
#line 2821 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 204:
#line 528 "gram.y"
    { if ((yyvsp[0].num) < 1 || (yyvsp[0].num) > 5) {
					     twmrc_error_prefix();
					     fprintf (stderr,
				"bad modifier number (%d), must be 1-5\n",
						      (yyvsp[0].num));
					     ParseError = true;
					  } else {
					     mods |= (Mod1Mask << ((yyvsp[0].num) - 1));
					  }
					}
#line 2836 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 205:
#line 538 "gram.y"
    { }
#line 2842 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 206:
#line 541 "gram.y"
    {
			switch((yyvsp[0].num)) {
				case GRAV_NORTH:
				case GRAV_SOUTH:
					/* OK */
					(yyval.num) = (yyvsp[0].num);
					break;
				default:
					twmrc_error_prefix();
					fprintf(stderr, "Bad vertical gravity '%s'\n", yytext);
					ParseError = true;
					YYERROR;
			}
		}
#line 2861 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 207:
#line 556 "gram.y"
    {
			switch((yyvsp[0].num)) {
				case GRAV_EAST:
				case GRAV_WEST:
					/* OK */
					(yyval.num) = (yyvsp[0].num);
					break;
				default:
					twmrc_error_prefix();
					fprintf(stderr, "Bad horiz gravity '%s'\n", yytext);
					ParseError = true;
					YYERROR;
			}
		}
#line 2880 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 210:
#line 575 "gram.y"
    { cont |= C_WINDOW_BIT; }
#line 2886 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 211:
#line 576 "gram.y"
    { cont |= C_TITLE_BIT; }
#line 2892 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 212:
#line 577 "gram.y"
    { cont |= C_ICON_BIT; }
#line 2898 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 213:
#line 578 "gram.y"
    { cont |= C_ROOT_BIT; }
#line 2904 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 214:
#line 579 "gram.y"
    { cont |= C_FRAME_BIT; }
#line 2910 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 215:
#line 580 "gram.y"
    { cont |= C_WORKSPACE_BIT; }
#line 2916 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 216:
#line 581 "gram.y"
    { cont |= C_ICONMGR_BIT; }
#line 2922 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 217:
#line 582 "gram.y"
    { cont |= C_ICONMGR_BIT; }
#line 2928 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 218:
#line 583 "gram.y"
    { cont |= C_ALTER_BIT; }
#line 2934 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 219:
#line 584 "gram.y"
    { cont |= C_ALL_BITS; }
#line 2940 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 220:
#line 585 "gram.y"
    { }
#line 2946 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 223:
#line 592 "gram.y"
    { cont |= C_WINDOW_BIT; }
#line 2952 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 224:
#line 593 "gram.y"
    { cont |= C_TITLE_BIT; }
#line 2958 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 225:
#line 594 "gram.y"
    { cont |= C_ICON_BIT; }
#line 2964 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 226:
#line 595 "gram.y"
    { cont |= C_ROOT_BIT; }
#line 2970 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 227:
#line 596 "gram.y"
    { cont |= C_FRAME_BIT; }
#line 2976 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 228:
#line 597 "gram.y"
    { cont |= C_WORKSPACE_BIT; }
#line 2982 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 229:
#line 598 "gram.y"
    { cont |= C_ICONMGR_BIT; }
#line 2988 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 230:
#line 599 "gram.y"
    { cont |= C_ICONMGR_BIT; }
#line 2994 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 231:
#line 600 "gram.y"
    { cont |= C_ALTER_BIT; }
#line 3000 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 232:
#line 601 "gram.y"
    { cont |= C_ALL_BITS; }
#line 3006 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 233:
#line 602 "gram.y"
    { }
#line 3012 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 234:
#line 603 "gram.y"
    { Name = (char*)(yyvsp[0].ptr); cont |= C_NAME_BIT; }
#line 3018 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 235:
#line 607 "gram.y"
    {}
#line 3024 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 238:
#line 614 "gram.y"
    { SetCurrentTBAction((yyvsp[-1].num), mods, (yyvsp[0].num), Action, pull); mods = 0;}
#line 3030 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 239:
#line 615 "gram.y"
    { SetCurrentTBAction((yyvsp[-2].num), 0, (yyvsp[0].num), Action, pull);}
#line 3036 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 240:
#line 616 "gram.y"
    {
			/* Deprecated since 3.8, no longer supported */
			yyerror("Title buttons specifications without = are no "
			        "longer supported.");
		}
#line 3046 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 241:
#line 624 "gram.y"
    {}
#line 3052 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 244:
#line 631 "gram.y"
    { Scr->HighlightPixmapName = strdup((yyvsp[0].ptr)); }
#line 3058 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 245:
#line 635 "gram.y"
    {}
#line 3064 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 248:
#line 642 "gram.y"
    {
			NewBitmapCursor(&Scr->FrameCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3071 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 249:
#line 644 "gram.y"
    {
			NewFontCursor(&Scr->FrameCursor, (yyvsp[0].ptr)); }
#line 3078 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 250:
#line 646 "gram.y"
    {
			NewBitmapCursor(&Scr->TitleCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3085 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 251:
#line 648 "gram.y"
    {
			NewFontCursor(&Scr->TitleCursor, (yyvsp[0].ptr)); }
#line 3092 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 252:
#line 650 "gram.y"
    {
			NewBitmapCursor(&Scr->IconCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3099 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 253:
#line 652 "gram.y"
    {
			NewFontCursor(&Scr->IconCursor, (yyvsp[0].ptr)); }
#line 3106 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 254:
#line 654 "gram.y"
    {
			NewBitmapCursor(&Scr->IconMgrCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3113 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 255:
#line 656 "gram.y"
    {
			NewFontCursor(&Scr->IconMgrCursor, (yyvsp[0].ptr)); }
#line 3120 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 256:
#line 658 "gram.y"
    {
			NewBitmapCursor(&Scr->ButtonCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3127 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 257:
#line 660 "gram.y"
    {
			NewFontCursor(&Scr->ButtonCursor, (yyvsp[0].ptr)); }
#line 3134 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 258:
#line 662 "gram.y"
    {
			NewBitmapCursor(&Scr->MoveCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3141 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 259:
#line 664 "gram.y"
    {
			NewFontCursor(&Scr->MoveCursor, (yyvsp[0].ptr)); }
#line 3148 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 260:
#line 666 "gram.y"
    {
			NewBitmapCursor(&Scr->ResizeCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3155 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 261:
#line 668 "gram.y"
    {
			NewFontCursor(&Scr->ResizeCursor, (yyvsp[0].ptr)); }
#line 3162 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 262:
#line 670 "gram.y"
    {
			NewBitmapCursor(&Scr->WaitCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3169 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 263:
#line 672 "gram.y"
    {
			NewFontCursor(&Scr->WaitCursor, (yyvsp[0].ptr)); }
#line 3176 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 264:
#line 674 "gram.y"
    {
			NewBitmapCursor(&Scr->MenuCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3183 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 265:
#line 676 "gram.y"
    {
			NewFontCursor(&Scr->MenuCursor, (yyvsp[0].ptr)); }
#line 3190 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 266:
#line 678 "gram.y"
    {
			NewBitmapCursor(&Scr->SelectCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3197 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 267:
#line 680 "gram.y"
    {
			NewFontCursor(&Scr->SelectCursor, (yyvsp[0].ptr)); }
#line 3204 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 268:
#line 682 "gram.y"
    {
			NewBitmapCursor(&Scr->DestroyCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3211 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 269:
#line 684 "gram.y"
    {
			NewFontCursor(&Scr->DestroyCursor, (yyvsp[0].ptr)); }
#line 3218 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 270:
#line 688 "gram.y"
    {}
#line 3224 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 273:
#line 696 "gram.y"
    { if (!do_colorlist_keyword ((yyvsp[-1].num), color,
								     (yyvsp[0].ptr))) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled list color keyword %d (string \"%s\")\n",
						     (yyvsp[-1].num), (yyvsp[0].ptr));
					    ParseError = true;
					  }
					}
#line 3238 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 274:
#line 705 "gram.y"
    { curplist = do_colorlist_keyword((yyvsp[-1].num),color,
								      (yyvsp[0].ptr));
					  if (!curplist) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled color list keyword %d (string \"%s\")\n",
						     (yyvsp[-1].num), (yyvsp[0].ptr));
					    ParseError = true;
					  }
					}
#line 3253 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 276:
#line 716 "gram.y"
    { if (!do_color_keyword ((yyvsp[-1].num), color,
								 (yyvsp[0].ptr))) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled color keyword %d (string \"%s\")\n",
						     (yyvsp[-1].num), (yyvsp[0].ptr));
					    ParseError = true;
					  }
					}
#line 3267 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 277:
#line 727 "gram.y"
    {}
#line 3273 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 280:
#line 734 "gram.y"
    { do_string_savecolor(color, (yyvsp[0].ptr)); }
#line 3279 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 281:
#line 735 "gram.y"
    { do_var_savecolor((yyvsp[0].num)); }
#line 3285 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 282:
#line 738 "gram.y"
    {}
#line 3291 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 285:
#line 745 "gram.y"
    { if (Scr->FirstTime &&
					      color == Scr->Monochrome)
					    AddToList(curplist, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3299 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 286:
#line 750 "gram.y"
    {}
#line 3305 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 289:
#line 757 "gram.y"
    { AddToList (&Scr->WindowGeometries, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3311 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 290:
#line 760 "gram.y"
    {}
#line 3317 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 293:
#line 767 "gram.y"
    { AddToList (&Scr->VirtualScreens, (yyvsp[0].ptr), ""); }
#line 3323 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 294:
#line 771 "gram.y"
    { proc_ewmh_ignore(); }
#line 3329 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 297:
#line 778 "gram.y"
    { add_ewmh_ignore((yyvsp[0].ptr)); }
#line 3335 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 298:
#line 782 "gram.y"
    { proc_mwm_ignore(); }
#line 3341 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 301:
#line 789 "gram.y"
    { add_mwm_ignore((yyvsp[0].ptr)); }
#line 3347 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 302:
#line 793 "gram.y"
    {
				    if (HasShape) Scr->SqueezeTitle = true;
				}
#line 3355 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 303:
#line 796 "gram.y"
    { curplist = &Scr->SqueezeTitleL;
				  if (HasShape)
				    Scr->SqueezeTitle = true;
				}
#line 3364 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 305:
#line 801 "gram.y"
    { Scr->SqueezeTitle = false; }
#line 3370 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 306:
#line 802 "gram.y"
    { curplist = &Scr->DontSqueezeTitleL; }
#line 3376 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 309:
#line 807 "gram.y"
    {
				if (Scr->FirstTime) {
				   do_squeeze_entry (curplist, (yyvsp[-3].ptr), (yyvsp[-2].num), (yyvsp[-1].num), (yyvsp[0].num));
				}
			}
#line 3386 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 310:
#line 815 "gram.y"
    {}
#line 3392 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 313:
#line 822 "gram.y"
    { if (Scr->FirstTime)
					    AddToList(curplist, (yyvsp[-2].ptr),
						AllocateIconManager((yyvsp[-2].ptr), NULL,
							(yyvsp[-1].ptr),(yyvsp[0].num)));
					}
#line 3402 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 314:
#line 828 "gram.y"
    { if (Scr->FirstTime)
					    AddToList(curplist, (yyvsp[-3].ptr),
						AllocateIconManager((yyvsp[-3].ptr),(yyvsp[-2].ptr),
						(yyvsp[-1].ptr), (yyvsp[0].num)));
					}
#line 3412 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 315:
#line 835 "gram.y"
    {}
#line 3418 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 318:
#line 842 "gram.y"
    {
			AddWorkSpace ((yyvsp[0].ptr), NULL, NULL, NULL, NULL, NULL);
		}
#line 3426 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 319:
#line 845 "gram.y"
    {
			curWorkSpc = (char*)(yyvsp[0].ptr);
		}
#line 3434 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 321:
#line 851 "gram.y"
    {}
#line 3440 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 324:
#line 858 "gram.y"
    {
			AddWorkSpace (curWorkSpc, (yyvsp[0].ptr), NULL, NULL, NULL, NULL);
		}
#line 3448 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 325:
#line 861 "gram.y"
    {
			AddWorkSpace (curWorkSpc, (yyvsp[-1].ptr), (yyvsp[0].ptr), NULL, NULL, NULL);
		}
#line 3456 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 326:
#line 864 "gram.y"
    {
			AddWorkSpace (curWorkSpc, (yyvsp[-2].ptr), (yyvsp[-1].ptr), (yyvsp[0].ptr), NULL, NULL);
		}
#line 3464 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 327:
#line 867 "gram.y"
    {
			AddWorkSpace (curWorkSpc, (yyvsp[-3].ptr), (yyvsp[-2].ptr), (yyvsp[-1].ptr), (yyvsp[0].ptr), NULL);
		}
#line 3472 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 328:
#line 870 "gram.y"
    {
			AddWorkSpace (curWorkSpc, (yyvsp[-4].ptr), (yyvsp[-3].ptr), (yyvsp[-2].ptr), (yyvsp[-1].ptr), (yyvsp[0].ptr));
		}
#line 3480 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 329:
#line 875 "gram.y"
    {
		    WMapCreateCurrentBackGround ((yyvsp[-1].ptr), NULL, NULL, NULL);
		}
#line 3488 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 330:
#line 878 "gram.y"
    {
		    WMapCreateCurrentBackGround ((yyvsp[-2].ptr), (yyvsp[-1].ptr), NULL, NULL);
		}
#line 3496 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 331:
#line 881 "gram.y"
    {
		    WMapCreateCurrentBackGround ((yyvsp[-3].ptr), (yyvsp[-2].ptr), (yyvsp[-1].ptr), NULL);
		}
#line 3504 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 332:
#line 884 "gram.y"
    {
		    WMapCreateCurrentBackGround ((yyvsp[-4].ptr), (yyvsp[-3].ptr), (yyvsp[-2].ptr), (yyvsp[-1].ptr));
		}
#line 3512 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 333:
#line 889 "gram.y"
    {
		    WMapCreateDefaultBackGround ((yyvsp[-1].ptr), NULL, NULL, NULL);
		}
#line 3520 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 334:
#line 892 "gram.y"
    {
		    WMapCreateDefaultBackGround ((yyvsp[-2].ptr), (yyvsp[-1].ptr), NULL, NULL);
		}
#line 3528 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 335:
#line 895 "gram.y"
    {
		    WMapCreateDefaultBackGround ((yyvsp[-3].ptr), (yyvsp[-2].ptr), (yyvsp[-1].ptr), NULL);
		}
#line 3536 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 336:
#line 898 "gram.y"
    {
		    WMapCreateDefaultBackGround ((yyvsp[-4].ptr), (yyvsp[-3].ptr), (yyvsp[-2].ptr), (yyvsp[-1].ptr));
		}
#line 3544 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 337:
#line 903 "gram.y"
    {}
#line 3550 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 340:
#line 910 "gram.y"
    { if (Scr->FirstTime)
					    AddToList(curplist, (yyvsp[0].ptr), 0);
					}
#line 3558 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 341:
#line 915 "gram.y"
    {}
#line 3564 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 344:
#line 922 "gram.y"
    {client = (char*)(yyvsp[0].ptr);}
#line 3570 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 346:
#line 924 "gram.y"
    {client = (char*)(yyvsp[0].ptr);}
#line 3576 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 348:
#line 926 "gram.y"
    {workspace = (char*)(yyvsp[0].ptr);}
#line 3582 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 350:
#line 930 "gram.y"
    {}
#line 3588 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 353:
#line 937 "gram.y"
    {
				if(!AddToClientsList ((yyvsp[0].ptr), client)) {
					twmrc_error_prefix();
					fprintf(stderr, "unknown workspace '%s'\n", (yyvsp[0].ptr));
				}
			  }
#line 3599 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 354:
#line 945 "gram.y"
    {}
#line 3605 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 357:
#line 952 "gram.y"
    {
				if(!AddToClientsList (workspace, (yyvsp[0].ptr))) {
					twmrc_error_prefix();
					fprintf(stderr, "unknown workspace '%s'\n", workspace);
				}
			  }
#line 3616 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 358:
#line 960 "gram.y"
    {}
#line 3622 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 361:
#line 967 "gram.y"
    { if (Scr->FirstTime) AddToList(curplist, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3628 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 362:
#line 970 "gram.y"
    {
#ifndef SOUNDS
			twmrc_error_prefix();
			fprintf(stderr, "RplaySounds ignored; rplay support "
					"not configured.\n");
#else
			sound_set_from_config();
#endif
		}
#line 3642 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 365:
#line 985 "gram.y"
    {
#ifdef SOUNDS
			if(set_sound_event_name((yyvsp[-1].ptr), (yyvsp[0].ptr)) != 0) {
				twmrc_error_prefix();
				fprintf(stderr, "Failed adding sound for %s; "
						"maybe event name is invalid?\n", (yyvsp[-1].ptr));
			}
#endif
		}
#line 3656 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 366:
#line 996 "gram.y"
    {}
#line 3662 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 369:
#line 1003 "gram.y"
    { AddToMenu(root, "", Action, NULL, (yyvsp[0].num),
						    NULL, NULL);
					  Action = "";
					}
#line 3671 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 370:
#line 1009 "gram.y"
    {lastmenuitem = NULL;}
#line 3677 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 373:
#line 1016 "gram.y"
    {
			if ((yyvsp[0].num) == F_SEPARATOR) {
			    if (lastmenuitem) lastmenuitem->separated = true;
			}
			else {
			    lastmenuitem = AddToMenu(root, (yyvsp[-1].ptr), Action, pull, (yyvsp[0].num), NULL, NULL);
			    Action = "";
			    pull = NULL;
			}
		}
#line 3692 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 374:
#line 1026 "gram.y"
    {
			if ((yyvsp[0].num) == F_SEPARATOR) {
			    if (lastmenuitem) lastmenuitem->separated = true;
			}
			else {
			    lastmenuitem = AddToMenu(root, (yyvsp[-6].ptr), Action, pull, (yyvsp[0].num), (yyvsp[-4].ptr), (yyvsp[-2].ptr));
			    Action = "";
			    pull = NULL;
			}
		}
#line 3707 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 375:
#line 1038 "gram.y"
    { (yyval.num) = (yyvsp[0].num); }
#line 3713 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 376:
#line 1039 "gram.y"
    {
				(yyval.num) = (yyvsp[-1].num);
				Action = (char*)(yyvsp[0].ptr);
				switch ((yyvsp[-1].num)) {
				  case F_MENU:
				    pull = GetRoot ((yyvsp[0].ptr), NULL,NULL);
				    pull->prev = root;
				    break;
				  case F_WARPRING:
				    if (!CheckWarpRingArg (Action)) {
					twmrc_error_prefix();
					fprintf (stderr,
			"ignoring invalid f.warptoring argument \"%s\"\n",
						 Action);
					(yyval.num) = F_NOP;
				    }
				  case F_WARPTOSCREEN:
				    if (!CheckWarpScreenArg (Action)) {
					twmrc_error_prefix();
					fprintf (stderr,
			"ignoring invalid f.warptoscreen argument \"%s\"\n",
						 Action);
					(yyval.num) = F_NOP;
				    }
				    break;
				  case F_COLORMAP:
				    if (CheckColormapArg (Action)) {
					(yyval.num) = F_COLORMAP;
				    } else {
					twmrc_error_prefix();
					fprintf (stderr,
			"ignoring invalid f.colormap argument \"%s\"\n",
						 Action);
					(yyval.num) = F_NOP;
				    }
				    break;
				} /* end switch */
				   }
#line 3756 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 377:
#line 1080 "gram.y"
    { (yyval.num) = (yyvsp[0].num); }
#line 3762 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 378:
#line 1081 "gram.y"
    { (yyval.num) = (yyvsp[0].num); }
#line 3768 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 379:
#line 1082 "gram.y"
    { (yyval.num) = -((yyvsp[0].num)); }
#line 3774 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 380:
#line 1085 "gram.y"
    { (yyval.num) = (yyvsp[0].num);
					  if ((yyvsp[0].num) == 0)
						yyerror("bad button 0");

					  if ((yyvsp[0].num) > MAX_BUTTONS)
					  {
						(yyval.num) = 0;
						yyerror("button number too large");
					  }
					}
#line 3789 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 381:
#line 1097 "gram.y"
    { char *ptr = strdup((yyvsp[0].ptr));
					  RemoveDQuote(ptr);
					  (yyval.ptr) = ptr;
					}
#line 3798 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;

  case 382:
#line 1103 "gram.y"
    { (yyval.num) = (yyvsp[0].num); }
#line 3804 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"
    break;


#line 3808 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.c"

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
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

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
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;

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


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
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
#line 1106 "gram.y"

