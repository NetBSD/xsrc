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
#line 91 "xkbparse.y" /* yacc.c:339  */

#ifdef DEBUG
#define	YYDEBUG 1
#endif
#define	DEBUG_VAR parseDebug
#include "parseutils.h"
#include <X11/keysym.h>
#include <X11/extensions/XKBgeom.h>
#include <stdlib.h>

unsigned int parseDebug;


#line 80 "xkbparse.c" /* yacc.c:339  */

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
    END_OF_FILE = 0,
    ERROR_TOK = 255,
    XKB_KEYMAP = 1,
    XKB_KEYCODES = 2,
    XKB_TYPES = 3,
    XKB_SYMBOLS = 4,
    XKB_COMPATMAP = 5,
    XKB_GEOMETRY = 6,
    XKB_SEMANTICS = 7,
    XKB_LAYOUT = 8,
    INCLUDE = 10,
    OVERRIDE = 11,
    AUGMENT = 12,
    REPLACE = 13,
    ALTERNATE = 14,
    VIRTUAL_MODS = 20,
    TYPE = 21,
    INTERPRET = 22,
    ACTION_TOK = 23,
    KEY = 24,
    ALIAS = 25,
    GROUP = 26,
    MODIFIER_MAP = 27,
    INDICATOR = 28,
    SHAPE = 29,
    KEYS = 30,
    ROW = 31,
    SECTION = 32,
    OVERLAY = 33,
    TEXT = 34,
    OUTLINE = 35,
    SOLID = 36,
    LOGO = 37,
    VIRTUAL = 38,
    EQUALS = 40,
    PLUS = 41,
    MINUS = 42,
    DIVIDE = 43,
    TIMES = 44,
    OBRACE = 45,
    CBRACE = 46,
    OPAREN = 47,
    CPAREN = 48,
    OBRACKET = 49,
    CBRACKET = 50,
    DOT = 51,
    COMMA = 52,
    SEMI = 53,
    EXCLAM = 54,
    INVERT = 55,
    STRING = 60,
    INTEGER = 61,
    FLOAT = 62,
    IDENT = 63,
    KEYNAME = 64,
    PARTIAL = 70,
    DEFAULT = 71,
    HIDDEN = 72,
    ALPHANUMERIC_KEYS = 73,
    MODIFIER_KEYS = 74,
    KEYPAD_KEYS = 75,
    FUNCTION_KEYS = 76,
    ALTERNATE_GROUP = 77
  };
#endif
/* Tokens.  */
#define END_OF_FILE 0
#define ERROR_TOK 255
#define XKB_KEYMAP 1
#define XKB_KEYCODES 2
#define XKB_TYPES 3
#define XKB_SYMBOLS 4
#define XKB_COMPATMAP 5
#define XKB_GEOMETRY 6
#define XKB_SEMANTICS 7
#define XKB_LAYOUT 8
#define INCLUDE 10
#define OVERRIDE 11
#define AUGMENT 12
#define REPLACE 13
#define ALTERNATE 14
#define VIRTUAL_MODS 20
#define TYPE 21
#define INTERPRET 22
#define ACTION_TOK 23
#define KEY 24
#define ALIAS 25
#define GROUP 26
#define MODIFIER_MAP 27
#define INDICATOR 28
#define SHAPE 29
#define KEYS 30
#define ROW 31
#define SECTION 32
#define OVERLAY 33
#define TEXT 34
#define OUTLINE 35
#define SOLID 36
#define LOGO 37
#define VIRTUAL 38
#define EQUALS 40
#define PLUS 41
#define MINUS 42
#define DIVIDE 43
#define TIMES 44
#define OBRACE 45
#define CBRACE 46
#define OPAREN 47
#define CPAREN 48
#define OBRACKET 49
#define CBRACKET 50
#define DOT 51
#define COMMA 52
#define SEMI 53
#define EXCLAM 54
#define INVERT 55
#define STRING 60
#define INTEGER 61
#define FLOAT 62
#define IDENT 63
#define KEYNAME 64
#define PARTIAL 70
#define DEFAULT 71
#define HIDDEN 72
#define ALPHANUMERIC_KEYS 73
#define MODIFIER_KEYS 74
#define KEYPAD_KEYS 75
#define FUNCTION_KEYS 76
#define ALTERNATE_GROUP 77

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 110 "xkbparse.y" /* yacc.c:355  */

	int		 ival;
	unsigned	 uval;
	char		*str;
	Atom	 	sval;
	ParseCommon	*any;
	ExprDef		*expr;
	VarDef		*var;
	VModDef		*vmod;
	InterpDef	*interp;
	KeyTypeDef	*keyType;
	SymbolsDef	*syms;
	ModMapDef	*modMask;
	GroupCompatDef	*groupCompat;
	IndicatorMapDef	*ledMap;
	IndicatorNameDef *ledName;
	KeycodeDef	*keyName;
	KeyAliasDef	*keyAlias;
	ShapeDef	*shape;
	SectionDef	*section;
	RowDef		*row;
	KeyDef		*key;
	OverlayDef	*overlay;
	OverlayKeyDef	*olKey;
	OutlineDef	*outline;
	DoodadDef	*doodad;
	XkbFile		*file;

#line 276 "xkbparse.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);



/* Copy the second part of user declarations.  */

#line 293 "xkbparse.c" /* yacc.c:358  */

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
#define YYFINAL  18
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   747

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  65
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  74
/* YYNRULES -- Number of rules.  */
#define YYNRULES  187
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  340

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   257

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     4,     5,     6,     7,     8,     9,    10,    11,     2,
      12,    13,    14,    15,    16,     2,     2,     2,     2,     2,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,     2,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,     2,     2,     2,     2,
      52,    53,    54,    55,    56,     2,     2,     2,     2,     2,
      57,    58,    59,    60,    61,    62,    63,    64,     2,     2,
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
       2,     2,     2,     2,     2,     3,     1,     2
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   168,   168,   170,   172,   176,   178,   182,   188,   189,
     190,   193,   195,   199,   205,   210,   211,   212,   213,   214,
     217,   218,   221,   222,   225,   226,   227,   228,   229,   230,
     231,   232,   235,   237,   240,   245,   250,   255,   260,   265,
     270,   275,   280,   285,   290,   295,   300,   305,   310,   322,
     324,   326,   330,   341,   351,   355,   357,   361,   363,   367,
     376,   378,   382,   384,   388,   394,   400,   402,   404,   407,
     409,   411,   413,   415,   419,   421,   425,   429,   433,   437,
     439,   443,   445,   453,   457,   459,   463,   465,   467,   469,
     471,   475,   477,   481,   483,   487,   489,   493,   495,   499,
     503,   508,   512,   516,   518,   522,   524,   526,   530,   532,
     536,   546,   550,   551,   552,   553,   556,   557,   560,   562,
     564,   566,   568,   570,   572,   574,   576,   578,   580,   584,
     585,   588,   589,   590,   591,   592,   595,   596,   599,   601,
     605,   607,   609,   611,   613,   615,   619,   621,   623,   625,
     627,   629,   631,   633,   637,   639,   643,   647,   654,   662,
     671,   682,   689,   696,   703,   714,   715,   718,   720,   722,
     724,   728,   729,   730,   737,   741,   742,   745,   746,   749,
     752,   755,   758,   759,   762,   765,   766,   769
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "END_OF_FILE", "error", "$undefined", "ERROR_TOK", "XKB_KEYMAP",
  "XKB_KEYCODES", "XKB_TYPES", "XKB_SYMBOLS", "XKB_COMPATMAP",
  "XKB_GEOMETRY", "XKB_SEMANTICS", "XKB_LAYOUT", "INCLUDE", "OVERRIDE",
  "AUGMENT", "REPLACE", "ALTERNATE", "VIRTUAL_MODS", "TYPE", "INTERPRET",
  "ACTION_TOK", "KEY", "ALIAS", "GROUP", "MODIFIER_MAP", "INDICATOR",
  "SHAPE", "KEYS", "ROW", "SECTION", "OVERLAY", "TEXT", "OUTLINE", "SOLID",
  "LOGO", "VIRTUAL", "EQUALS", "PLUS", "MINUS", "DIVIDE", "TIMES",
  "OBRACE", "CBRACE", "OPAREN", "CPAREN", "OBRACKET", "CBRACKET", "DOT",
  "COMMA", "SEMI", "EXCLAM", "INVERT", "STRING", "INTEGER", "FLOAT",
  "IDENT", "KEYNAME", "PARTIAL", "DEFAULT", "HIDDEN", "ALPHANUMERIC_KEYS",
  "MODIFIER_KEYS", "KEYPAD_KEYS", "FUNCTION_KEYS", "ALTERNATE_GROUP",
  "$accept", "XkbFile", "XkbCompMapList", "XkbCompositeMap",
  "XkbCompositeType", "XkbMapConfigList", "XkbMapConfig", "XkbConfig",
  "FileType", "OptFlags", "Flags", "Flag", "DeclList", "Decl", "VarDecl",
  "KeyNameDecl", "KeyAliasDecl", "VModDecl", "VModDefList", "VModDef",
  "InterpretDecl", "InterpretMatch", "VarDeclList", "KeyTypeDecl",
  "SymbolsDecl", "SymbolsBody", "SymbolsVarDecl", "ArrayInit",
  "GroupCompatDecl", "ModMapDecl", "IndicatorMapDecl", "IndicatorNameDecl",
  "ShapeDecl", "SectionDecl", "SectionBody", "SectionBodyItem", "RowBody",
  "RowBodyItem", "Keys", "Key", "OverlayDecl", "OverlayKeyList",
  "OverlayKey", "OutlineList", "OutlineInList", "CoordList", "Coord",
  "DoodadDecl", "DoodadType", "FieldSpec", "Element", "OptMergeMode",
  "MergeMode", "OptExprList", "ExprList", "Expr", "Term", "ActionList",
  "Action", "Lhs", "Terminal", "OptKeySymList", "KeySymList", "KeySym",
  "KeySyms", "SignedNumber", "Number", "Float", "Integer", "KeyName",
  "Ident", "String", "OptMapName", "MapName", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   255,     1,     2,     3,     4,     5,     6,
       7,     8,    10,    11,    12,    13,    14,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    60,    61,    62,    63,    64,    70,    71,    72,
      73,    74,    75,    76,    77
};
# endif

#define YYPACT_NINF -179

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-179)))

#define YYTABLE_NINF -183

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     669,  -179,  -179,  -179,  -179,  -179,  -179,  -179,  -179,    19,
     138,  -179,   185,  -179,  -179,   736,   669,  -179,  -179,  -179,
      41,  -179,   297,  -179,  -179,  -179,  -179,  -179,  -179,  -179,
    -179,    -3,    -3,  -179,    -3,  -179,    13,  -179,    31,    31,
     669,  -179,    63,   544,    53,  -179,  -179,  -179,  -179,  -179,
    -179,   266,    59,    66,   104,     1,    98,    -9,  -179,    27,
      27,    52,     1,    79,    98,  -179,    98,   109,  -179,  -179,
    -179,   140,     1,  -179,  -179,  -179,  -179,  -179,  -179,  -179,
    -179,  -179,  -179,  -179,  -179,  -179,  -179,  -179,  -179,  -179,
      98,   130,  -179,   132,   135,   125,  -179,  -179,  -179,   144,
    -179,   145,  -179,   170,  -179,  -179,  -179,   176,   189,  -179,
     186,   192,   200,   196,   202,   210,   211,   212,    52,   213,
     220,   371,   681,   371,   371,  -179,     1,  -179,   371,   667,
     667,   371,   498,    27,   371,   371,   371,   667,    16,   453,
     205,  -179,   667,  -179,  -179,  -179,  -179,  -179,  -179,  -179,
    -179,  -179,   371,   371,   371,   371,   371,  -179,   358,   194,
    -179,   224,  -179,  -179,  -179,  -179,  -179,  -179,   226,   166,
     181,  -179,    51,  -179,   513,   541,    51,   556,     1,   -27,
    -179,  -179,   233,    22,   223,   219,    38,    51,   325,   597,
     229,   -24,    54,  -179,    56,  -179,   239,    98,   236,    98,
    -179,  -179,   412,  -179,  -179,  -179,   371,   612,  -179,  -179,
    -179,   291,  -179,  -179,   371,   371,   371,   371,   371,  -179,
     371,   371,  -179,  -179,   232,  -179,   244,   253,   -16,   268,
     276,   134,  -179,   277,   285,  -179,  -179,  -179,   287,   498,
     289,  -179,  -179,   288,   371,  -179,   303,    70,   154,  -179,
    -179,   290,  -179,   308,   -17,   311,   229,   330,   653,   284,
     317,  -179,   338,   320,  -179,   302,   322,   183,   183,  -179,
    -179,    51,   227,  -179,  -179,    80,   371,  -179,   681,  -179,
     -16,  -179,  -179,  -179,    51,  -179,    51,  -179,  -179,  -179,
     -24,  -179,  -179,  -179,  -179,   229,    51,   331,  -179,   470,
    -179,    27,  -179,  -179,  -179,  -179,  -179,  -179,   335,  -179,
    -179,  -179,   347,    99,   -25,   349,  -179,   100,  -179,   368,
    -179,  -179,  -179,   371,   101,  -179,  -179,  -179,   357,    27,
      27,   116,   361,   -25,  -179,  -179,  -179,  -179,  -179,  -179
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
      21,    24,    25,    26,    27,    28,    29,    30,    31,     0,
      21,     6,    21,    12,     4,     0,    20,    23,     1,     5,
       0,    11,     0,     8,    15,    16,    18,    17,    19,     9,
      10,   186,   186,    22,   186,   187,     0,   185,    33,     0,
      21,    33,   130,    21,   130,   131,   133,   132,   134,   135,
      32,     0,   129,     0,     0,     0,   120,   119,   118,   121,
       0,   122,   123,   124,   125,   126,   127,   128,   113,   114,
     115,     0,     0,   182,   181,   183,    34,    37,    38,    35,
      36,    39,    40,    42,    41,    43,    44,    45,    46,    47,
       0,   157,   117,     0,     0,   116,    48,     7,    13,     0,
      56,    57,   184,     0,   172,   180,   171,     0,    61,   173,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    50,     0,    54,     0,     0,
       0,     0,    68,     0,     0,     0,     0,     0,     0,     0,
       0,    51,     0,   120,   119,   121,   122,   123,   124,   125,
     127,   128,     0,     0,     0,     0,     0,   179,   157,     0,
     145,   150,   152,   163,   162,   164,   116,   161,   158,     0,
       0,    55,    58,    63,     0,     0,    60,   166,     0,     0,
      67,    73,     0,   116,     0,     0,     0,   139,     0,     0,
       0,     0,     0,   104,     0,   109,     0,   124,   126,     0,
      87,    89,     0,    85,    90,    88,     0,     0,   147,   150,
     146,     0,   148,   149,   137,     0,     0,     0,     0,   159,
       0,     0,    49,    52,     0,    62,     0,   172,     0,   171,
       0,     0,   155,     0,   165,   169,   170,    72,     0,     0,
       0,    53,    76,     0,     0,    79,     0,     0,     0,   178,
     177,     0,   176,     0,     0,     0,     0,     0,     0,     0,
       0,    84,     0,     0,   153,     0,   136,   141,   142,   140,
     143,   144,     0,    64,    59,     0,   137,    75,     0,    74,
       0,    65,    66,    70,    69,    77,   138,    78,   105,   175,
       0,    81,   103,    82,   108,     0,   107,     0,    94,     0,
      92,     0,    83,    80,   111,   151,   160,   174,     0,   154,
     167,   168,     0,     0,     0,     0,    91,     0,   101,     0,
     156,   110,   106,     0,     0,    96,    97,    86,     0,     0,
       0,     0,     0,     0,    99,   100,   102,    98,    93,    95
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -179,  -179,  -179,   397,  -179,   372,    -8,  -179,   389,    48,
    -179,   399,   375,  -179,   -19,  -179,  -179,  -179,  -179,   292,
    -179,  -179,  -119,  -179,  -179,  -179,   174,   177,  -179,  -179,
     369,  -179,  -179,  -179,  -179,   217,  -179,   129,  -179,   106,
    -179,  -179,   105,  -179,   193,  -178,   195,   398,  -179,   -23,
    -179,  -179,  -179,   172,  -132,   -81,    34,  -179,   175,   -29,
    -179,  -179,   222,   -55,   178,   162,   207,  -179,   -56,   -51,
     -45,   -30,   180,  -179
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     9,    10,    11,    31,    12,    13,    14,    32,    22,
      16,    17,    42,    50,   173,    77,    78,    79,    99,   100,
      80,   107,   174,    81,    82,   179,   180,   181,    83,    84,
     201,    86,    87,    88,   202,   203,   299,   300,   324,   325,
     204,   317,   318,   192,   193,   194,   195,   205,    90,   158,
      92,    51,    52,   265,   266,   187,   160,   231,   232,   161,
     162,   233,   234,   235,   236,   251,   252,   163,   164,   165,
     166,   167,    36,    37
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      94,   109,   108,   186,    21,   112,    95,   114,   110,   111,
     101,   175,   247,   104,   248,   238,   323,   113,   189,    18,
     104,   239,    93,   207,   190,   228,   103,   119,    91,   249,
     250,    74,    76,   115,   116,    21,   117,   105,    73,   106,
     159,    75,   169,   170,   105,    23,   106,   172,    15,    35,
     176,    29,    30,   185,    40,   188,    73,   190,    20,    75,
     120,   191,   140,   -14,   -71,    45,    46,    47,    48,    49,
     -71,    73,    41,   211,    75,    45,    46,    47,    48,    49,
     243,   101,   184,    74,    95,    95,   244,   183,   215,   216,
     217,   218,    95,   196,    95,    54,   253,    95,   255,   168,
      93,    93,   254,   182,   256,   105,    91,    91,    93,    91,
      93,    96,   288,    93,    91,    97,    91,   313,   256,    91,
     200,   109,   307,   209,   209,   262,   209,   209,   280,    95,
      95,   102,   105,   237,   267,   268,   269,   270,    -2,   271,
     272,   322,   328,   332,    95,    93,    93,   256,   329,   333,
     102,    91,    91,    98,   230,   225,   225,    95,   337,   284,
      93,  -112,    95,   286,   244,   118,    91,   115,   123,   259,
     225,   124,   109,    93,   125,   121,   296,   122,    93,    91,
     277,   128,   278,   200,    91,    -3,   208,   210,   225,   212,
     213,   331,   126,   127,   183,     1,     2,     3,     4,     5,
       6,     7,     8,   215,   216,   217,   218,   249,   250,   196,
     182,   129,    38,    95,    39,   222,    91,   130,   215,   216,
     217,   218,   217,   218,   109,   310,   131,   132,   133,    93,
     223,   215,   216,   217,   218,    91,   134,   135,   136,   298,
     219,   206,     1,     2,     3,     4,     5,     6,     7,     8,
     319,   137,   138,   139,    95,   230,   215,   216,   217,   218,
     220,   142,   141,   326,   215,   216,   217,   218,   242,   240,
      93,   221,   241,   306,   191,   257,    91,   258,   319,   336,
     298,   273,   326,    55,    56,    57,    58,    59,    60,    61,
      62,    63,    64,   274,    65,    66,  -127,    67,    68,    69,
      70,    71,    24,    25,    26,    27,    28,   143,   144,    58,
     145,  -182,   146,   147,   148,   149,    72,    65,   150,   276,
     151,    73,    74,   279,    75,   301,   152,   153,   215,   216,
     217,   218,   154,   280,   177,   264,   281,   285,   290,   155,
     156,   102,   105,   157,    73,    74,   305,    75,   143,   144,
      58,   145,   287,   146,   147,   148,   149,   291,    65,   150,
     293,   151,   215,   216,   217,   218,   302,   152,   153,   304,
     244,   295,   314,   154,   245,   215,   216,   217,   218,   320,
     155,   156,   102,   105,   157,    73,    74,   303,    75,   143,
     144,    58,   145,   321,   146,   147,   148,   149,   327,    65,
     150,   214,   151,   121,   330,   122,   334,    19,   152,   153,
     338,    34,    43,   282,   154,    33,    44,   283,   171,   261,
      85,   155,   156,   102,   105,   157,    73,    74,   316,    75,
     143,   144,    58,   145,   335,   146,   147,   197,   149,   339,
     198,   150,   199,    67,    68,    69,    70,   292,   308,    89,
     275,   294,   312,   309,   260,   289,     0,     0,   311,     0,
       0,     0,    72,     0,     0,     0,     0,    73,     0,     0,
      75,   143,   144,    58,   145,     0,   146,   147,   197,   149,
       0,   198,   150,   199,    67,    68,    69,    70,   143,   144,
      58,   145,     0,   146,   147,   148,   149,   297,    65,   150,
       0,   151,     0,    72,     0,     0,     0,     0,    73,     0,
       0,    75,   315,     0,     0,     0,   143,   144,    58,   145,
      72,   146,   147,   148,   149,    73,    65,   150,    75,   151,
       0,   143,   144,    58,   145,     0,   146,   147,   148,   149,
       0,    65,   150,   177,   151,     0,     0,     0,   178,     0,
       0,     0,     0,    73,     0,   224,    75,     0,     0,   143,
     144,    58,   145,    72,   146,   147,   148,   149,    73,    65,
     150,    75,   151,     0,   143,   144,    58,   145,     0,   146,
     147,   148,   149,   226,    65,   227,    53,   151,     0,     0,
       0,    72,     0,     0,     0,     0,    73,   228,     0,    75,
       0,     1,     2,     3,     4,     5,     6,     7,     8,   105,
       0,   229,     0,     0,    75,   143,   144,    58,   145,     0,
     146,   147,   148,   149,     0,    65,   150,     0,   151,     0,
     143,   144,    58,   145,     0,   146,   147,   148,   149,   246,
      65,   150,     0,   151,     0,     0,     0,    72,     0,     0,
       0,     0,    73,     0,   263,    75,     0,     0,     0,     0,
       0,     0,    72,     0,     0,     0,     0,    73,     0,     0,
      75,   143,   144,    58,   145,     0,   146,   147,   148,   149,
     297,    65,   150,     0,   151,   143,   144,    58,   145,     0,
     146,   147,   148,   149,     0,    65,   150,     0,   151,   143,
     144,    58,   145,    72,   146,   147,   148,   149,    73,    65,
     150,    75,   151,     0,     0,     0,     0,    72,     0,     0,
       0,     0,    73,     0,     0,    75,     1,     2,     3,     4,
       5,     6,     7,     8,     0,     0,    73,     0,     0,    75,
      23,    24,    25,    26,    27,    28,    29,    30
};

static const yytype_int16 yycheck[] =
{
      51,    57,    57,   135,    12,    61,    51,    63,    59,    60,
      55,   130,   190,    29,    38,    42,    41,    62,   137,     0,
      29,    48,    51,   142,    41,    41,    56,    72,    51,    53,
      54,    56,    51,    63,    64,    43,    66,    53,    55,    55,
     121,    58,   123,   124,    53,     4,    55,   128,     0,    52,
     131,    10,    11,   134,    41,   136,    55,    41,    10,    58,
      90,    45,   118,     0,    42,    12,    13,    14,    15,    16,
      48,    55,    41,   154,    58,    12,    13,    14,    15,    16,
      42,   126,   133,    56,   129,   130,    48,   132,    37,    38,
      39,    40,   137,   138,   139,    42,    42,   142,    42,   122,
     129,   130,    48,   132,    48,    53,   129,   130,   137,   132,
     139,    52,    42,   142,   137,    49,   139,   295,    48,   142,
     139,   177,    42,   152,   153,   206,   155,   156,    48,   174,
     175,    52,    53,   178,   215,   216,   217,   218,     0,   220,
     221,    42,    42,    42,   189,   174,   175,    48,    48,    48,
      52,   174,   175,    49,   177,   174,   175,   202,    42,   240,
     189,    52,   207,   244,    48,    25,   189,   197,    36,   199,
     189,    36,   228,   202,    49,    45,   257,    47,   207,   202,
      46,    36,    48,   202,   207,     0,   152,   153,   207,   155,
     156,   323,    48,    49,   239,    57,    58,    59,    60,    61,
      62,    63,    64,    37,    38,    39,    40,    53,    54,   254,
     239,    41,    32,   258,    34,    49,   239,    41,    37,    38,
      39,    40,    39,    40,   280,   280,    37,    41,    36,   258,
      49,    37,    38,    39,    40,   258,    36,    41,    36,   258,
      46,    36,    57,    58,    59,    60,    61,    62,    63,    64,
     301,    41,    41,    41,   299,   278,    37,    38,    39,    40,
      36,    41,    49,   314,    37,    38,    39,    40,    49,    36,
     299,    45,    49,    46,    45,    36,   299,    41,   329,   330,
     299,    49,   333,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    26,    49,    28,    29,    43,    31,    32,    33,
      34,    35,     5,     6,     7,     8,     9,    18,    19,    20,
      21,    43,    23,    24,    25,    26,    50,    28,    29,    43,
      31,    55,    56,    46,    58,    41,    37,    38,    37,    38,
      39,    40,    43,    48,    45,    44,    49,    49,    48,    50,
      51,    52,    53,    54,    55,    56,    44,    58,    18,    19,
      20,    21,    49,    23,    24,    25,    26,    49,    28,    29,
      49,    31,    37,    38,    39,    40,    49,    37,    38,    49,
      48,    41,    41,    43,    49,    37,    38,    39,    40,    44,
      50,    51,    52,    53,    54,    55,    56,    49,    58,    18,
      19,    20,    21,    46,    23,    24,    25,    26,    49,    28,
      29,    43,    31,    45,    36,    47,    49,    10,    37,    38,
      49,    22,    40,   239,    43,    16,    41,   240,   126,   202,
      51,    50,    51,    52,    53,    54,    55,    56,   299,    58,
      18,    19,    20,    21,   329,    23,    24,    25,    26,   333,
      28,    29,    30,    31,    32,    33,    34,   254,   276,    51,
     228,   256,   290,   278,    42,   248,    -1,    -1,   280,    -1,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    55,    -1,    -1,
      58,    18,    19,    20,    21,    -1,    23,    24,    25,    26,
      -1,    28,    29,    30,    31,    32,    33,    34,    18,    19,
      20,    21,    -1,    23,    24,    25,    26,    27,    28,    29,
      -1,    31,    -1,    50,    -1,    -1,    -1,    -1,    55,    -1,
      -1,    58,    42,    -1,    -1,    -1,    18,    19,    20,    21,
      50,    23,    24,    25,    26,    55,    28,    29,    58,    31,
      -1,    18,    19,    20,    21,    -1,    23,    24,    25,    26,
      -1,    28,    29,    45,    31,    -1,    -1,    -1,    50,    -1,
      -1,    -1,    -1,    55,    -1,    42,    58,    -1,    -1,    18,
      19,    20,    21,    50,    23,    24,    25,    26,    55,    28,
      29,    58,    31,    -1,    18,    19,    20,    21,    -1,    23,
      24,    25,    26,    42,    28,    29,    42,    31,    -1,    -1,
      -1,    50,    -1,    -1,    -1,    -1,    55,    41,    -1,    58,
      -1,    57,    58,    59,    60,    61,    62,    63,    64,    53,
      -1,    55,    -1,    -1,    58,    18,    19,    20,    21,    -1,
      23,    24,    25,    26,    -1,    28,    29,    -1,    31,    -1,
      18,    19,    20,    21,    -1,    23,    24,    25,    26,    42,
      28,    29,    -1,    31,    -1,    -1,    -1,    50,    -1,    -1,
      -1,    -1,    55,    -1,    42,    58,    -1,    -1,    -1,    -1,
      -1,    -1,    50,    -1,    -1,    -1,    -1,    55,    -1,    -1,
      58,    18,    19,    20,    21,    -1,    23,    24,    25,    26,
      27,    28,    29,    -1,    31,    18,    19,    20,    21,    -1,
      23,    24,    25,    26,    -1,    28,    29,    -1,    31,    18,
      19,    20,    21,    50,    23,    24,    25,    26,    55,    28,
      29,    58,    31,    -1,    -1,    -1,    -1,    50,    -1,    -1,
      -1,    -1,    55,    -1,    -1,    58,    57,    58,    59,    60,
      61,    62,    63,    64,    -1,    -1,    55,    -1,    -1,    58,
       4,     5,     6,     7,     8,     9,    10,    11
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    57,    58,    59,    60,    61,    62,    63,    64,    66,
      67,    68,    70,    71,    72,    74,    75,    76,     0,    68,
      74,    71,    74,     4,     5,     6,     7,     8,     9,    10,
      11,    69,    73,    76,    73,    52,   137,   138,   137,   137,
      41,    41,    77,    70,    77,    12,    13,    14,    15,    16,
      78,   116,   117,    42,    42,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    26,    28,    29,    31,    32,    33,
      34,    35,    50,    55,    56,    58,    79,    80,    81,    82,
      85,    88,    89,    93,    94,    95,    96,    97,    98,   112,
     113,   114,   115,   124,   134,   135,    52,    49,    49,    83,
      84,   135,    52,   136,    29,    53,    55,    86,   128,   133,
     134,   134,   133,   135,   133,   136,   136,   136,    25,   135,
     136,    45,    47,    36,    36,    49,    48,    49,    36,    41,
      41,    37,    41,    36,    36,    41,    36,    41,    41,    41,
     133,    49,    41,    18,    19,    21,    23,    24,    25,    26,
      29,    31,    37,    38,    43,    50,    51,    54,   114,   120,
     121,   124,   125,   132,   133,   134,   135,   136,   114,   120,
     120,    84,   120,    79,    87,    87,   120,    45,    50,    90,
      91,    92,   124,   135,   134,   120,   119,   120,   120,    87,
      41,    45,   108,   109,   110,   111,   135,    25,    28,    30,
      79,    95,    99,   100,   105,   112,    36,    87,   121,   124,
     121,   120,   121,   121,    43,    37,    38,    39,    40,    46,
      36,    45,    49,    49,    42,    79,    42,    29,    41,    55,
     114,   122,   123,   126,   127,   128,   129,   135,    42,    48,
      36,    49,    49,    42,    48,    49,    42,   110,    38,    53,
      54,   130,   131,    42,    48,    42,    48,    36,    41,   136,
      42,   100,   120,    42,    44,   118,   119,   120,   120,   120,
     120,   120,   120,    49,    49,   127,    43,    46,    48,    46,
      48,    49,    91,    92,   120,    49,   120,    49,    42,   131,
      48,    49,   109,    49,   111,    41,   120,    27,    79,   101,
     102,    41,    49,    49,    49,    44,    46,    42,   118,   123,
     128,   129,   130,   110,    41,    42,   102,   106,   107,   134,
      44,    46,    42,    41,   103,   104,   134,    49,    42,    48,
      36,   119,    42,    48,    49,   107,   134,    42,    49,   104
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    65,    66,    66,    66,    67,    67,    68,    69,    69,
      69,    70,    70,    71,    72,    73,    73,    73,    73,    73,
      74,    74,    75,    75,    76,    76,    76,    76,    76,    76,
      76,    76,    77,    77,    78,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    78,    78,    79,
      79,    79,    80,    81,    82,    83,    83,    84,    84,    85,
      86,    86,    87,    87,    88,    89,    90,    90,    90,    91,
      91,    91,    91,    91,    92,    92,    93,    94,    95,    96,
      96,    97,    97,    98,    99,    99,   100,   100,   100,   100,
     100,   101,   101,   102,   102,   103,   103,   104,   104,   105,
     106,   106,   107,   108,   108,   109,   109,   109,   110,   110,
     111,   112,   113,   113,   113,   113,   114,   114,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   116,
     116,   117,   117,   117,   117,   117,   118,   118,   119,   119,
     120,   120,   120,   120,   120,   120,   121,   121,   121,   121,
     121,   121,   121,   121,   122,   122,   123,   124,   124,   124,
     124,   125,   125,   125,   125,   126,   126,   127,   127,   127,
     127,   128,   128,   128,   129,   130,   130,   131,   131,   132,
     133,   134,   135,   135,   136,   137,   137,   138
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     2,     1,     7,     1,     1,
       1,     2,     1,     7,     4,     1,     1,     1,     1,     1,
       1,     0,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     2,     0,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     4,
       2,     3,     4,     5,     3,     3,     1,     1,     3,     6,
       3,     1,     2,     1,     6,     6,     3,     1,     0,     3,
       3,     1,     2,     1,     3,     3,     5,     6,     6,     5,
       6,     6,     6,     6,     2,     1,     5,     1,     1,     1,
       1,     2,     1,     5,     1,     3,     1,     1,     3,     6,
       3,     1,     3,     3,     1,     3,     5,     3,     3,     1,
       5,     6,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       0,     1,     1,     1,     1,     1,     1,     0,     3,     1,
       3,     3,     3,     3,     3,     1,     2,     2,     2,     2,
       1,     4,     1,     3,     3,     1,     4,     1,     3,     4,
       6,     1,     1,     1,     1,     1,     0,     3,     3,     1,
       1,     1,     1,     1,     3,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     1
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
        case 2:
#line 169 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.file)= rtrnValue= (yyvsp[0].file); }
#line 1703 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 3:
#line 171 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.file)= rtrnValue= (yyvsp[0].file);  }
#line 1709 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 4:
#line 173 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.file)= rtrnValue= (yyvsp[0].file); }
#line 1715 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 5:
#line 177 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.file)= (XkbFile *)AppendStmt(&(yyvsp[-1].file)->common,&(yyvsp[0].file)->common); }
#line 1721 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 6:
#line 179 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.file)= (yyvsp[0].file); }
#line 1727 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 7:
#line 185 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.file)= CreateXKBFile((yyvsp[-5].uval),(yyvsp[-4].str),&(yyvsp[-2].file)->common,(yyvsp[-6].uval)); }
#line 1733 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 8:
#line 188 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkmKeymapFile; }
#line 1739 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 9:
#line 189 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkmSemanticsFile; }
#line 1745 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 10:
#line 190 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkmLayoutFile; }
#line 1751 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 11:
#line 194 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.file)= (XkbFile *)AppendStmt(&(yyvsp[-1].file)->common,&(yyvsp[0].file)->common); }
#line 1757 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 12:
#line 196 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.file)= (yyvsp[0].file); }
#line 1763 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 13:
#line 202 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.file)= CreateXKBFile((yyvsp[-5].uval),(yyvsp[-4].str),(yyvsp[-2].any),(yyvsp[-6].uval)); }
#line 1769 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 14:
#line 206 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.file)= CreateXKBFile((yyvsp[-2].uval),(yyvsp[-1].str),(yyvsp[0].any),(yyvsp[-3].uval)); }
#line 1775 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 15:
#line 210 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkmKeyNamesIndex; }
#line 1781 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 16:
#line 211 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkmTypesIndex; }
#line 1787 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 17:
#line 212 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkmCompatMapIndex; }
#line 1793 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 18:
#line 213 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkmSymbolsIndex; }
#line 1799 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 19:
#line 214 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkmGeometryIndex; }
#line 1805 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 20:
#line 217 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= (yyvsp[0].uval); }
#line 1811 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 21:
#line 218 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= 0; }
#line 1817 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 22:
#line 221 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= (((yyvsp[-1].uval))|((yyvsp[0].uval))); }
#line 1823 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 23:
#line 222 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= (yyvsp[0].uval); }
#line 1829 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 24:
#line 225 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkbLC_Partial; }
#line 1835 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 25:
#line 226 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkbLC_Default; }
#line 1841 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 26:
#line 227 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkbLC_Hidden; }
#line 1847 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 27:
#line 228 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkbLC_AlphanumericKeys; }
#line 1853 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 28:
#line 229 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkbLC_ModifierKeys; }
#line 1859 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 29:
#line 230 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkbLC_KeypadKeys; }
#line 1865 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 30:
#line 231 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkbLC_FunctionKeys; }
#line 1871 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 31:
#line 232 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkbLC_AlternateGroup; }
#line 1877 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 32:
#line 236 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.any)= AppendStmt((yyvsp[-1].any),(yyvsp[0].any)); }
#line 1883 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 33:
#line 237 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.any)= NULL; }
#line 1889 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 34:
#line 241 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].var)->merge= StmtSetMerge(&(yyvsp[0].var)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].var)->common;
			}
#line 1898 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 35:
#line 246 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].vmod)->merge= StmtSetMerge(&(yyvsp[0].vmod)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].vmod)->common;
			}
#line 1907 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 36:
#line 251 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].interp)->merge= StmtSetMerge(&(yyvsp[0].interp)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].interp)->common;
			}
#line 1916 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 37:
#line 256 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].keyName)->merge= StmtSetMerge(&(yyvsp[0].keyName)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].keyName)->common;
			}
#line 1925 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 38:
#line 261 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].keyAlias)->merge= StmtSetMerge(&(yyvsp[0].keyAlias)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].keyAlias)->common;
			}
#line 1934 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 39:
#line 266 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].keyType)->merge= StmtSetMerge(&(yyvsp[0].keyType)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].keyType)->common;
			}
#line 1943 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 40:
#line 271 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].syms)->merge= StmtSetMerge(&(yyvsp[0].syms)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].syms)->common;
			}
#line 1952 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 41:
#line 276 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].modMask)->merge= StmtSetMerge(&(yyvsp[0].modMask)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].modMask)->common;
			}
#line 1961 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 42:
#line 281 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].groupCompat)->merge= StmtSetMerge(&(yyvsp[0].groupCompat)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].groupCompat)->common;
			}
#line 1970 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 43:
#line 286 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].ledMap)->merge= StmtSetMerge(&(yyvsp[0].ledMap)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].ledMap)->common;
			}
#line 1979 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 44:
#line 291 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].ledName)->merge= StmtSetMerge(&(yyvsp[0].ledName)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].ledName)->common;
			}
#line 1988 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 45:
#line 296 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].shape)->merge= StmtSetMerge(&(yyvsp[0].shape)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].shape)->common;
			}
#line 1997 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 46:
#line 301 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].section)->merge= StmtSetMerge(&(yyvsp[0].section)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].section)->common;
			}
#line 2006 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 47:
#line 306 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[0].doodad)->merge= StmtSetMerge(&(yyvsp[0].doodad)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].doodad)->common;
			}
#line 2015 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 48:
#line 311 "xkbparse.y" /* yacc.c:1646  */
    {
			    if ((yyvsp[-1].uval)==MergeAltForm) {
				yyerror("cannot use 'alternate' to include other maps");
				(yyval.any)= &IncludeCreate(scanBuf,MergeDefault)->common;
			    }
			    else {
				(yyval.any)= &IncludeCreate(scanBuf,(yyvsp[-1].uval))->common;
			    }
                        }
#line 2029 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 49:
#line 323 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.var)= VarCreate((yyvsp[-3].expr),(yyvsp[-1].expr)); }
#line 2035 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 50:
#line 325 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.var)= BoolVarCreate((yyvsp[-1].sval),1); }
#line 2041 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 51:
#line 327 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.var)= BoolVarCreate((yyvsp[-1].sval),0); }
#line 2047 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 52:
#line 331 "xkbparse.y" /* yacc.c:1646  */
    {
			    KeycodeDef *def;

			    def= KeycodeCreate((yyvsp[-3].str),(yyvsp[-1].expr));
			    if ((yyvsp[-3].str))
				free((yyvsp[-3].str));
			    (yyval.keyName)= def;
			}
#line 2060 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 53:
#line 342 "xkbparse.y" /* yacc.c:1646  */
    {
			    KeyAliasDef	*def;
			    def= KeyAliasCreate((yyvsp[-3].str),(yyvsp[-1].str));
			    if ((yyvsp[-3].str))	free((yyvsp[-3].str));	
			    if ((yyvsp[-1].str))	free((yyvsp[-1].str));	
			    (yyval.keyAlias)= def;
			}
#line 2072 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 54:
#line 352 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.vmod)= (yyvsp[-1].vmod); }
#line 2078 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 55:
#line 356 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.vmod)= (VModDef *)AppendStmt(&(yyvsp[-2].vmod)->common,&(yyvsp[0].vmod)->common); }
#line 2084 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 56:
#line 358 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.vmod)= (yyvsp[0].vmod); }
#line 2090 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 57:
#line 362 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.vmod)= VModCreate((yyvsp[0].sval),NULL); }
#line 2096 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 58:
#line 364 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.vmod)= VModCreate((yyvsp[-2].sval),(yyvsp[0].expr)); }
#line 2102 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 59:
#line 370 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyvsp[-4].interp)->def= (yyvsp[-2].var);
			    (yyval.interp)= (yyvsp[-4].interp);
			}
#line 2111 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 60:
#line 377 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.interp)= InterpCreate((yyvsp[-2].str), (yyvsp[0].expr)); }
#line 2117 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 61:
#line 379 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.interp)= InterpCreate((yyvsp[0].str), NULL); }
#line 2123 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 62:
#line 383 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.var)= (VarDef *)AppendStmt(&(yyvsp[-1].var)->common,&(yyvsp[0].var)->common); }
#line 2129 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 63:
#line 385 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.var)= (yyvsp[0].var); }
#line 2135 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 64:
#line 391 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.keyType)= KeyTypeCreate((yyvsp[-4].sval),(yyvsp[-2].var)); }
#line 2141 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 65:
#line 397 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.syms)= SymbolsCreate((yyvsp[-4].str),(ExprDef *)(yyvsp[-2].var)); }
#line 2147 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 66:
#line 401 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.var)= (VarDef *)AppendStmt(&(yyvsp[-2].var)->common,&(yyvsp[0].var)->common); }
#line 2153 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 67:
#line 403 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.var)= (yyvsp[0].var); }
#line 2159 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 68:
#line 404 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.var)= NULL; }
#line 2165 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 69:
#line 408 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.var)= VarCreate((yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2171 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 70:
#line 410 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.var)= VarCreate((yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2177 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 71:
#line 412 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.var)= BoolVarCreate((yyvsp[0].sval),1); }
#line 2183 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 72:
#line 414 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.var)= BoolVarCreate((yyvsp[0].sval),0); }
#line 2189 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 73:
#line 416 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.var)= VarCreate(NULL,(yyvsp[0].expr)); }
#line 2195 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 74:
#line 420 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (yyvsp[-1].expr); }
#line 2201 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 75:
#line 422 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= ExprCreateUnary(ExprActionList,TypeAction,(yyvsp[-1].expr)); }
#line 2207 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 76:
#line 426 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.groupCompat)= GroupCompatCreate((yyvsp[-3].ival),(yyvsp[-1].expr)); }
#line 2213 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 77:
#line 430 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.modMask)= ModMapCreate((yyvsp[-4].sval),(yyvsp[-2].expr)); }
#line 2219 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 78:
#line 434 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.ledMap)= IndicatorMapCreate((yyvsp[-4].sval),(yyvsp[-2].var)); }
#line 2225 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 79:
#line 438 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.ledName)= IndicatorNameCreate((yyvsp[-3].ival),(yyvsp[-1].expr),False); }
#line 2231 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 80:
#line 440 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.ledName)= IndicatorNameCreate((yyvsp[-3].ival),(yyvsp[-1].expr),True); }
#line 2237 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 81:
#line 444 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.shape)= ShapeDeclCreate((yyvsp[-4].sval),(OutlineDef *)&(yyvsp[-2].outline)->common); }
#line 2243 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 82:
#line 446 "xkbparse.y" /* yacc.c:1646  */
    {
			    OutlineDef *outlines;
			    outlines= OutlineCreate(None,(yyvsp[-2].expr));
			    (yyval.shape)= ShapeDeclCreate((yyvsp[-4].sval),outlines);
			}
#line 2253 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 83:
#line 454 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.section)= SectionDeclCreate((yyvsp[-4].sval),(yyvsp[-2].row)); }
#line 2259 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 84:
#line 458 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.row)=(RowDef *)AppendStmt(&(yyvsp[-1].row)->common,&(yyvsp[0].row)->common);}
#line 2265 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 85:
#line 460 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.row)= (yyvsp[0].row); }
#line 2271 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 86:
#line 464 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.row)= RowDeclCreate((yyvsp[-2].key)); }
#line 2277 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 87:
#line 466 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.row)= (RowDef *)(yyvsp[0].var); }
#line 2283 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 88:
#line 468 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.row)= (RowDef *)(yyvsp[0].doodad); }
#line 2289 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 89:
#line 470 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.row)= (RowDef *)(yyvsp[0].ledMap); }
#line 2295 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 90:
#line 472 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.row)= (RowDef *)(yyvsp[0].overlay); }
#line 2301 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 91:
#line 476 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.key)=(KeyDef *)AppendStmt(&(yyvsp[-1].key)->common,&(yyvsp[0].key)->common);}
#line 2307 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 92:
#line 478 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.key)= (yyvsp[0].key); }
#line 2313 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 93:
#line 482 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.key)= (yyvsp[-2].key); }
#line 2319 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 94:
#line 484 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.key)= (KeyDef *)(yyvsp[0].var); }
#line 2325 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 95:
#line 488 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.key)=(KeyDef *)AppendStmt(&(yyvsp[-2].key)->common,&(yyvsp[0].key)->common);}
#line 2331 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 96:
#line 490 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.key)= (yyvsp[0].key); }
#line 2337 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 97:
#line 494 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.key)= KeyDeclCreate((yyvsp[0].str),NULL); }
#line 2343 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 98:
#line 496 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.key)= KeyDeclCreate(NULL,(yyvsp[-1].expr)); }
#line 2349 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 99:
#line 500 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.overlay)= OverlayDeclCreate((yyvsp[-4].sval),(yyvsp[-2].olKey)); }
#line 2355 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 100:
#line 504 "xkbparse.y" /* yacc.c:1646  */
    {
			    (yyval.olKey)= (OverlayKeyDef *)
				AppendStmt(&(yyvsp[-2].olKey)->common,&(yyvsp[0].olKey)->common);
			}
#line 2364 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 101:
#line 509 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.olKey)= (yyvsp[0].olKey); }
#line 2370 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 102:
#line 513 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.olKey)= OverlayKeyCreate((yyvsp[-2].str),(yyvsp[0].str)); }
#line 2376 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 103:
#line 517 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.outline)=(OutlineDef *)AppendStmt(&(yyvsp[-2].outline)->common,&(yyvsp[0].outline)->common);}
#line 2382 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 104:
#line 519 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.outline)= (yyvsp[0].outline); }
#line 2388 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 105:
#line 523 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.outline)= OutlineCreate(None,(yyvsp[-1].expr)); }
#line 2394 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 106:
#line 525 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.outline)= OutlineCreate((yyvsp[-4].sval),(yyvsp[-1].expr)); }
#line 2400 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 107:
#line 527 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.outline)= OutlineCreate((yyvsp[-2].sval),(yyvsp[0].expr)); }
#line 2406 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 108:
#line 531 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (ExprDef *)AppendStmt(&(yyvsp[-2].expr)->common,&(yyvsp[0].expr)->common); }
#line 2412 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 109:
#line 533 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (yyvsp[0].expr); }
#line 2418 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 110:
#line 537 "xkbparse.y" /* yacc.c:1646  */
    {
			    ExprDef *expr;
			    expr= ExprCreate(ExprCoord,TypeUnknown);
			    expr->value.coord.x= (yyvsp[-3].ival);
			    expr->value.coord.y= (yyvsp[-1].ival);
			    (yyval.expr)= expr;
			}
#line 2430 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 111:
#line 547 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.doodad)= DoodadCreate((yyvsp[-5].uval),(yyvsp[-4].sval),(yyvsp[-2].var)); }
#line 2436 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 112:
#line 550 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkbTextDoodad; }
#line 2442 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 113:
#line 551 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkbOutlineDoodad; }
#line 2448 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 114:
#line 552 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkbSolidDoodad; }
#line 2454 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 115:
#line 553 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= XkbLogoDoodad; }
#line 2460 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 116:
#line 556 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= (yyvsp[0].sval); }
#line 2466 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 117:
#line 557 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= (yyvsp[0].sval); }
#line 2472 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 118:
#line 561 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= XkbInternAtom(NULL,"action",False); }
#line 2478 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 119:
#line 563 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= XkbInternAtom(NULL,"interpret",False); }
#line 2484 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 120:
#line 565 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= XkbInternAtom(NULL,"type",False); }
#line 2490 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 121:
#line 567 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= XkbInternAtom(NULL,"key",False); }
#line 2496 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 122:
#line 569 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= XkbInternAtom(NULL,"group",False); }
#line 2502 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 123:
#line 571 "xkbparse.y" /* yacc.c:1646  */
    {(yyval.sval)=XkbInternAtom(NULL,"modifier_map",False);}
#line 2508 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 124:
#line 573 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= XkbInternAtom(NULL,"indicator",False); }
#line 2514 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 125:
#line 575 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= XkbInternAtom(NULL,"shape",False); }
#line 2520 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 126:
#line 577 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= XkbInternAtom(NULL,"row",False); }
#line 2526 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 127:
#line 579 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= XkbInternAtom(NULL,"section",False); }
#line 2532 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 128:
#line 581 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= XkbInternAtom(NULL,"text",False); }
#line 2538 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 129:
#line 584 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= (yyvsp[0].uval); }
#line 2544 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 130:
#line 585 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= MergeDefault; }
#line 2550 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 131:
#line 588 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= MergeDefault; }
#line 2556 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 132:
#line 589 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= MergeAugment; }
#line 2562 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 133:
#line 590 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= MergeOverride; }
#line 2568 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 134:
#line 591 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= MergeReplace; }
#line 2574 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 135:
#line 592 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.uval)= MergeAltForm; }
#line 2580 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 136:
#line 595 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (yyvsp[0].expr); }
#line 2586 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 137:
#line 596 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= NULL; }
#line 2592 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 138:
#line 600 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (ExprDef *)AppendStmt(&(yyvsp[-2].expr)->common,&(yyvsp[0].expr)->common); }
#line 2598 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 139:
#line 602 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (yyvsp[0].expr); }
#line 2604 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 140:
#line 606 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= ExprCreateBinary(OpDivide,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2610 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 141:
#line 608 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= ExprCreateBinary(OpAdd,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2616 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 142:
#line 610 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= ExprCreateBinary(OpSubtract,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2622 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 143:
#line 612 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= ExprCreateBinary(OpMultiply,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2628 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 144:
#line 614 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= ExprCreateBinary(OpAssign,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2634 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 145:
#line 616 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (yyvsp[0].expr); }
#line 2640 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 146:
#line 620 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= ExprCreateUnary(OpNegate,(yyvsp[0].expr)->type,(yyvsp[0].expr)); }
#line 2646 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 147:
#line 622 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= ExprCreateUnary(OpUnaryPlus,(yyvsp[0].expr)->type,(yyvsp[0].expr)); }
#line 2652 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 148:
#line 624 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= ExprCreateUnary(OpNot,TypeBoolean,(yyvsp[0].expr)); }
#line 2658 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 149:
#line 626 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= ExprCreateUnary(OpInvert,(yyvsp[0].expr)->type,(yyvsp[0].expr)); }
#line 2664 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 150:
#line 628 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (yyvsp[0].expr);  }
#line 2670 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 151:
#line 630 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= ActionCreate((yyvsp[-3].sval),(yyvsp[-1].expr)); }
#line 2676 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 152:
#line 632 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (yyvsp[0].expr);  }
#line 2682 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 153:
#line 634 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (yyvsp[-1].expr);  }
#line 2688 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 154:
#line 638 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (ExprDef *)AppendStmt(&(yyvsp[-2].expr)->common,&(yyvsp[0].expr)->common); }
#line 2694 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 155:
#line 640 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (yyvsp[0].expr); }
#line 2700 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 156:
#line 644 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= ActionCreate((yyvsp[-3].sval),(yyvsp[-1].expr)); }
#line 2706 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 157:
#line 648 "xkbparse.y" /* yacc.c:1646  */
    {
			    ExprDef *expr;
                            expr= ExprCreate(ExprIdent,TypeUnknown);
                            expr->value.str= (yyvsp[0].sval);
                            (yyval.expr)= expr;
			}
#line 2717 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 158:
#line 655 "xkbparse.y" /* yacc.c:1646  */
    {
                            ExprDef *expr;
                            expr= ExprCreate(ExprFieldRef,TypeUnknown);
                            expr->value.field.element= (yyvsp[-2].sval);
                            expr->value.field.field= (yyvsp[0].sval);
                            (yyval.expr)= expr;
			}
#line 2729 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 159:
#line 663 "xkbparse.y" /* yacc.c:1646  */
    {
			    ExprDef *expr;
			    expr= ExprCreate(ExprArrayRef,TypeUnknown);
			    expr->value.array.element= None;
			    expr->value.array.field= (yyvsp[-3].sval);
			    expr->value.array.entry= (yyvsp[-1].expr);
			    (yyval.expr)= expr;
			}
#line 2742 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 160:
#line 672 "xkbparse.y" /* yacc.c:1646  */
    {
			    ExprDef *expr;
			    expr= ExprCreate(ExprArrayRef,TypeUnknown);
			    expr->value.array.element= (yyvsp[-5].sval);
			    expr->value.array.field= (yyvsp[-3].sval);
			    expr->value.array.entry= (yyvsp[-1].expr);
			    (yyval.expr)= expr;
			}
#line 2755 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 161:
#line 683 "xkbparse.y" /* yacc.c:1646  */
    {
			    ExprDef *expr;
                            expr= ExprCreate(ExprValue,TypeString);
                            expr->value.str= (yyvsp[0].sval);
                            (yyval.expr)= expr;
			}
#line 2766 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 162:
#line 690 "xkbparse.y" /* yacc.c:1646  */
    {
			    ExprDef *expr;
                            expr= ExprCreate(ExprValue,TypeInt);
                            expr->value.ival= (yyvsp[0].ival);
                            (yyval.expr)= expr;
			}
#line 2777 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 163:
#line 697 "xkbparse.y" /* yacc.c:1646  */
    {
			    ExprDef *expr;
			    expr= ExprCreate(ExprValue,TypeFloat);
			    expr->value.ival= (yyvsp[0].ival);
			    (yyval.expr)= expr;
			}
#line 2788 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 164:
#line 704 "xkbparse.y" /* yacc.c:1646  */
    {
			    ExprDef *expr;
			    expr= ExprCreate(ExprValue,TypeKeyName);
			    memset(expr->value.keyName,0,5);
			    strncpy(expr->value.keyName,(yyvsp[0].str),4);
			    free((yyvsp[0].str));
			    (yyval.expr)= expr;
			}
#line 2801 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 165:
#line 714 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (yyvsp[0].expr); }
#line 2807 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 166:
#line 715 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= NULL; }
#line 2813 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 167:
#line 719 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= AppendKeysymList((yyvsp[-2].expr),(yyvsp[0].str)); }
#line 2819 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 168:
#line 721 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= AppendKeysymList((yyvsp[-2].expr),strdup("NoSymbol")); }
#line 2825 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 169:
#line 723 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= CreateKeysymList((yyvsp[0].str)); }
#line 2831 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 170:
#line 725 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= CreateKeysymList(strdup("NoSymbol")); }
#line 2837 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 171:
#line 728 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.str)= strdup(scanBuf); }
#line 2843 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 172:
#line 729 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.str)= strdup("section"); }
#line 2849 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 173:
#line 731 "xkbparse.y" /* yacc.c:1646  */
    {
			    if ((yyvsp[0].ival)<10)	{ (yyval.str)= malloc(2); (yyval.str)[0]= '0' + (yyvsp[0].ival); (yyval.str)[1]= '\0'; }
			    else	{ (yyval.str)= malloc(19); snprintf((yyval.str), 19, "0x%x", (yyvsp[0].ival)); }
			}
#line 2858 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 174:
#line 738 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.expr)= (yyvsp[-1].expr); }
#line 2864 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 175:
#line 741 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.ival)= -(yyvsp[0].ival); }
#line 2870 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 176:
#line 742 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.ival)= (yyvsp[0].ival); }
#line 2876 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 177:
#line 745 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.ival)= scanInt; }
#line 2882 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 178:
#line 746 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.ival)= scanInt*XkbGeomPtsPerMM; }
#line 2888 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 179:
#line 749 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.ival)= scanInt; }
#line 2894 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 180:
#line 752 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.ival)= scanInt; }
#line 2900 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 181:
#line 755 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.str)= strdup(scanBuf); }
#line 2906 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 182:
#line 758 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= XkbInternAtom(NULL,scanBuf,False); }
#line 2912 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 183:
#line 759 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= XkbInternAtom(NULL,"default",False); }
#line 2918 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 184:
#line 762 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.sval)= XkbInternAtom(NULL,scanBuf,False); }
#line 2924 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 185:
#line 765 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.str)= (yyvsp[0].str); }
#line 2930 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 186:
#line 766 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.str)= NULL; }
#line 2936 "xkbparse.c" /* yacc.c:1646  */
    break;

  case 187:
#line 769 "xkbparse.y" /* yacc.c:1646  */
    { (yyval.str)= strdup(scanBuf); }
#line 2942 "xkbparse.c" /* yacc.c:1646  */
    break;


#line 2946 "xkbparse.c" /* yacc.c:1646  */
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
#line 771 "xkbparse.y" /* yacc.c:1906  */

void
yyerror(const char *s)
{
    if (warningLevel>0) {
	(void)fprintf(stderr,"%s: line %d of %s\n",s,lineNum,
					(scanFile?scanFile:"(unknown)"));
	if ((warningLevel>3))
	    (void)fprintf(stderr,"last scanned symbol is: %s\n",scanBuf);
    }
    return;
}


int
yywrap(void)
{
   return 1;
}

