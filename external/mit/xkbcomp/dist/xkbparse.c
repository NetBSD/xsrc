/* A Bison parser, made by GNU Bison 3.6.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.6.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 91 "xkbparse.y"

#ifdef DEBUG
#define	YYDEBUG 1
#endif
#define	DEBUG_VAR parseDebug
#include "parseutils.h"
#include <X11/keysym.h>
#include <X11/extensions/XKBgeom.h>
#include <stdlib.h>

unsigned int parseDebug;


#line 85 "xkbparse.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
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


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    END_OF_FILE = 0,               /* END_OF_FILE  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    ERROR_TOK = 255,               /* ERROR_TOK  */
    XKB_KEYMAP = 1,                /* XKB_KEYMAP  */
    XKB_KEYCODES = 2,              /* XKB_KEYCODES  */
    XKB_TYPES = 3,                 /* XKB_TYPES  */
    XKB_SYMBOLS = 4,               /* XKB_SYMBOLS  */
    XKB_COMPATMAP = 5,             /* XKB_COMPATMAP  */
    XKB_GEOMETRY = 6,              /* XKB_GEOMETRY  */
    XKB_SEMANTICS = 7,             /* XKB_SEMANTICS  */
    XKB_LAYOUT = 8,                /* XKB_LAYOUT  */
    INCLUDE = 10,                  /* INCLUDE  */
    OVERRIDE = 11,                 /* OVERRIDE  */
    AUGMENT = 12,                  /* AUGMENT  */
    REPLACE = 13,                  /* REPLACE  */
    ALTERNATE = 14,                /* ALTERNATE  */
    VIRTUAL_MODS = 20,             /* VIRTUAL_MODS  */
    TYPE = 21,                     /* TYPE  */
    INTERPRET = 22,                /* INTERPRET  */
    ACTION_TOK = 23,               /* ACTION_TOK  */
    KEY = 24,                      /* KEY  */
    ALIAS = 25,                    /* ALIAS  */
    GROUP = 26,                    /* GROUP  */
    MODIFIER_MAP = 27,             /* MODIFIER_MAP  */
    INDICATOR = 28,                /* INDICATOR  */
    SHAPE = 29,                    /* SHAPE  */
    KEYS = 30,                     /* KEYS  */
    ROW = 31,                      /* ROW  */
    SECTION = 32,                  /* SECTION  */
    OVERLAY = 33,                  /* OVERLAY  */
    TEXT = 34,                     /* TEXT  */
    OUTLINE = 35,                  /* OUTLINE  */
    SOLID = 36,                    /* SOLID  */
    LOGO = 37,                     /* LOGO  */
    VIRTUAL = 38,                  /* VIRTUAL  */
    EQUALS = 40,                   /* EQUALS  */
    PLUS = 41,                     /* PLUS  */
    MINUS = 42,                    /* MINUS  */
    DIVIDE = 43,                   /* DIVIDE  */
    TIMES = 44,                    /* TIMES  */
    OBRACE = 45,                   /* OBRACE  */
    CBRACE = 46,                   /* CBRACE  */
    OPAREN = 47,                   /* OPAREN  */
    CPAREN = 48,                   /* CPAREN  */
    OBRACKET = 49,                 /* OBRACKET  */
    CBRACKET = 50,                 /* CBRACKET  */
    DOT = 51,                      /* DOT  */
    COMMA = 52,                    /* COMMA  */
    SEMI = 53,                     /* SEMI  */
    EXCLAM = 54,                   /* EXCLAM  */
    INVERT = 55,                   /* INVERT  */
    STRING = 60,                   /* STRING  */
    INTEGER = 61,                  /* INTEGER  */
    FLOAT = 62,                    /* FLOAT  */
    IDENT = 63,                    /* IDENT  */
    KEYNAME = 64,                  /* KEYNAME  */
    PARTIAL = 70,                  /* PARTIAL  */
    DEFAULT = 71,                  /* DEFAULT  */
    HIDDEN = 72,                   /* HIDDEN  */
    ALPHANUMERIC_KEYS = 73,        /* ALPHANUMERIC_KEYS  */
    MODIFIER_KEYS = 74,            /* MODIFIER_KEYS  */
    KEYPAD_KEYS = 75,              /* KEYPAD_KEYS  */
    FUNCTION_KEYS = 76,            /* FUNCTION_KEYS  */
    ALTERNATE_GROUP = 77           /* ALTERNATE_GROUP  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define END_OF_FILE 0
#define YYerror 256
#define YYUNDEF 257
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
#line 110 "xkbparse.y"

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

#line 291 "xkbparse.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);


/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* END_OF_FILE  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_ERROR_TOK = 3,                  /* ERROR_TOK  */
  YYSYMBOL_XKB_KEYMAP = 4,                 /* XKB_KEYMAP  */
  YYSYMBOL_XKB_KEYCODES = 5,               /* XKB_KEYCODES  */
  YYSYMBOL_XKB_TYPES = 6,                  /* XKB_TYPES  */
  YYSYMBOL_XKB_SYMBOLS = 7,                /* XKB_SYMBOLS  */
  YYSYMBOL_XKB_COMPATMAP = 8,              /* XKB_COMPATMAP  */
  YYSYMBOL_XKB_GEOMETRY = 9,               /* XKB_GEOMETRY  */
  YYSYMBOL_XKB_SEMANTICS = 10,             /* XKB_SEMANTICS  */
  YYSYMBOL_XKB_LAYOUT = 11,                /* XKB_LAYOUT  */
  YYSYMBOL_INCLUDE = 12,                   /* INCLUDE  */
  YYSYMBOL_OVERRIDE = 13,                  /* OVERRIDE  */
  YYSYMBOL_AUGMENT = 14,                   /* AUGMENT  */
  YYSYMBOL_REPLACE = 15,                   /* REPLACE  */
  YYSYMBOL_ALTERNATE = 16,                 /* ALTERNATE  */
  YYSYMBOL_VIRTUAL_MODS = 17,              /* VIRTUAL_MODS  */
  YYSYMBOL_TYPE = 18,                      /* TYPE  */
  YYSYMBOL_INTERPRET = 19,                 /* INTERPRET  */
  YYSYMBOL_ACTION_TOK = 20,                /* ACTION_TOK  */
  YYSYMBOL_KEY = 21,                       /* KEY  */
  YYSYMBOL_ALIAS = 22,                     /* ALIAS  */
  YYSYMBOL_GROUP = 23,                     /* GROUP  */
  YYSYMBOL_MODIFIER_MAP = 24,              /* MODIFIER_MAP  */
  YYSYMBOL_INDICATOR = 25,                 /* INDICATOR  */
  YYSYMBOL_SHAPE = 26,                     /* SHAPE  */
  YYSYMBOL_KEYS = 27,                      /* KEYS  */
  YYSYMBOL_ROW = 28,                       /* ROW  */
  YYSYMBOL_SECTION = 29,                   /* SECTION  */
  YYSYMBOL_OVERLAY = 30,                   /* OVERLAY  */
  YYSYMBOL_TEXT = 31,                      /* TEXT  */
  YYSYMBOL_OUTLINE = 32,                   /* OUTLINE  */
  YYSYMBOL_SOLID = 33,                     /* SOLID  */
  YYSYMBOL_LOGO = 34,                      /* LOGO  */
  YYSYMBOL_VIRTUAL = 35,                   /* VIRTUAL  */
  YYSYMBOL_EQUALS = 36,                    /* EQUALS  */
  YYSYMBOL_PLUS = 37,                      /* PLUS  */
  YYSYMBOL_MINUS = 38,                     /* MINUS  */
  YYSYMBOL_DIVIDE = 39,                    /* DIVIDE  */
  YYSYMBOL_TIMES = 40,                     /* TIMES  */
  YYSYMBOL_OBRACE = 41,                    /* OBRACE  */
  YYSYMBOL_CBRACE = 42,                    /* CBRACE  */
  YYSYMBOL_OPAREN = 43,                    /* OPAREN  */
  YYSYMBOL_CPAREN = 44,                    /* CPAREN  */
  YYSYMBOL_OBRACKET = 45,                  /* OBRACKET  */
  YYSYMBOL_CBRACKET = 46,                  /* CBRACKET  */
  YYSYMBOL_DOT = 47,                       /* DOT  */
  YYSYMBOL_COMMA = 48,                     /* COMMA  */
  YYSYMBOL_SEMI = 49,                      /* SEMI  */
  YYSYMBOL_EXCLAM = 50,                    /* EXCLAM  */
  YYSYMBOL_INVERT = 51,                    /* INVERT  */
  YYSYMBOL_STRING = 52,                    /* STRING  */
  YYSYMBOL_INTEGER = 53,                   /* INTEGER  */
  YYSYMBOL_FLOAT = 54,                     /* FLOAT  */
  YYSYMBOL_IDENT = 55,                     /* IDENT  */
  YYSYMBOL_KEYNAME = 56,                   /* KEYNAME  */
  YYSYMBOL_PARTIAL = 57,                   /* PARTIAL  */
  YYSYMBOL_DEFAULT = 58,                   /* DEFAULT  */
  YYSYMBOL_HIDDEN = 59,                    /* HIDDEN  */
  YYSYMBOL_ALPHANUMERIC_KEYS = 60,         /* ALPHANUMERIC_KEYS  */
  YYSYMBOL_MODIFIER_KEYS = 61,             /* MODIFIER_KEYS  */
  YYSYMBOL_KEYPAD_KEYS = 62,               /* KEYPAD_KEYS  */
  YYSYMBOL_FUNCTION_KEYS = 63,             /* FUNCTION_KEYS  */
  YYSYMBOL_ALTERNATE_GROUP = 64,           /* ALTERNATE_GROUP  */
  YYSYMBOL_YYACCEPT = 65,                  /* $accept  */
  YYSYMBOL_XkbFile = 66,                   /* XkbFile  */
  YYSYMBOL_XkbCompMapList = 67,            /* XkbCompMapList  */
  YYSYMBOL_XkbCompositeMap = 68,           /* XkbCompositeMap  */
  YYSYMBOL_XkbCompositeType = 69,          /* XkbCompositeType  */
  YYSYMBOL_XkbMapConfigList = 70,          /* XkbMapConfigList  */
  YYSYMBOL_XkbMapConfig = 71,              /* XkbMapConfig  */
  YYSYMBOL_XkbConfig = 72,                 /* XkbConfig  */
  YYSYMBOL_FileType = 73,                  /* FileType  */
  YYSYMBOL_OptFlags = 74,                  /* OptFlags  */
  YYSYMBOL_Flags = 75,                     /* Flags  */
  YYSYMBOL_Flag = 76,                      /* Flag  */
  YYSYMBOL_DeclList = 77,                  /* DeclList  */
  YYSYMBOL_Decl = 78,                      /* Decl  */
  YYSYMBOL_VarDecl = 79,                   /* VarDecl  */
  YYSYMBOL_KeyNameDecl = 80,               /* KeyNameDecl  */
  YYSYMBOL_KeyAliasDecl = 81,              /* KeyAliasDecl  */
  YYSYMBOL_VModDecl = 82,                  /* VModDecl  */
  YYSYMBOL_VModDefList = 83,               /* VModDefList  */
  YYSYMBOL_VModDef = 84,                   /* VModDef  */
  YYSYMBOL_InterpretDecl = 85,             /* InterpretDecl  */
  YYSYMBOL_InterpretMatch = 86,            /* InterpretMatch  */
  YYSYMBOL_VarDeclList = 87,               /* VarDeclList  */
  YYSYMBOL_KeyTypeDecl = 88,               /* KeyTypeDecl  */
  YYSYMBOL_SymbolsDecl = 89,               /* SymbolsDecl  */
  YYSYMBOL_SymbolsBody = 90,               /* SymbolsBody  */
  YYSYMBOL_SymbolsVarDecl = 91,            /* SymbolsVarDecl  */
  YYSYMBOL_ArrayInit = 92,                 /* ArrayInit  */
  YYSYMBOL_GroupCompatDecl = 93,           /* GroupCompatDecl  */
  YYSYMBOL_ModMapDecl = 94,                /* ModMapDecl  */
  YYSYMBOL_IndicatorMapDecl = 95,          /* IndicatorMapDecl  */
  YYSYMBOL_IndicatorNameDecl = 96,         /* IndicatorNameDecl  */
  YYSYMBOL_ShapeDecl = 97,                 /* ShapeDecl  */
  YYSYMBOL_SectionDecl = 98,               /* SectionDecl  */
  YYSYMBOL_SectionBody = 99,               /* SectionBody  */
  YYSYMBOL_SectionBodyItem = 100,          /* SectionBodyItem  */
  YYSYMBOL_RowBody = 101,                  /* RowBody  */
  YYSYMBOL_RowBodyItem = 102,              /* RowBodyItem  */
  YYSYMBOL_Keys = 103,                     /* Keys  */
  YYSYMBOL_Key = 104,                      /* Key  */
  YYSYMBOL_OverlayDecl = 105,              /* OverlayDecl  */
  YYSYMBOL_OverlayKeyList = 106,           /* OverlayKeyList  */
  YYSYMBOL_OverlayKey = 107,               /* OverlayKey  */
  YYSYMBOL_OutlineList = 108,              /* OutlineList  */
  YYSYMBOL_OutlineInList = 109,            /* OutlineInList  */
  YYSYMBOL_CoordList = 110,                /* CoordList  */
  YYSYMBOL_Coord = 111,                    /* Coord  */
  YYSYMBOL_DoodadDecl = 112,               /* DoodadDecl  */
  YYSYMBOL_DoodadType = 113,               /* DoodadType  */
  YYSYMBOL_FieldSpec = 114,                /* FieldSpec  */
  YYSYMBOL_Element = 115,                  /* Element  */
  YYSYMBOL_OptMergeMode = 116,             /* OptMergeMode  */
  YYSYMBOL_MergeMode = 117,                /* MergeMode  */
  YYSYMBOL_OptExprList = 118,              /* OptExprList  */
  YYSYMBOL_ExprList = 119,                 /* ExprList  */
  YYSYMBOL_Expr = 120,                     /* Expr  */
  YYSYMBOL_Term = 121,                     /* Term  */
  YYSYMBOL_ActionList = 122,               /* ActionList  */
  YYSYMBOL_Action = 123,                   /* Action  */
  YYSYMBOL_Lhs = 124,                      /* Lhs  */
  YYSYMBOL_Terminal = 125,                 /* Terminal  */
  YYSYMBOL_OptKeySymList = 126,            /* OptKeySymList  */
  YYSYMBOL_KeySymList = 127,               /* KeySymList  */
  YYSYMBOL_KeySym = 128,                   /* KeySym  */
  YYSYMBOL_KeySyms = 129,                  /* KeySyms  */
  YYSYMBOL_SignedNumber = 130,             /* SignedNumber  */
  YYSYMBOL_Number = 131,                   /* Number  */
  YYSYMBOL_Float = 132,                    /* Float  */
  YYSYMBOL_Integer = 133,                  /* Integer  */
  YYSYMBOL_KeyName = 134,                  /* KeyName  */
  YYSYMBOL_Ident = 135,                    /* Ident  */
  YYSYMBOL_String = 136,                   /* String  */
  YYSYMBOL_OptMapName = 137,               /* OptMapName  */
  YYSYMBOL_MapName = 138                   /* MapName  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

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


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
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

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

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
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
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
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
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

#define YYMAXUTOK   257


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
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
static const yytype_int16 yyrline[] =
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

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "END_OF_FILE", "error", "\"invalid token\"", "ERROR_TOK", "XKB_KEYMAP",
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

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   255,     1,     2,     3,     4,     5,     6,
       7,     8,    10,    11,    12,    13,    14,    20,    21,    22,
      23,    24,    25,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    60,    61,    62,    63,    64,    70,    71,    72,
      73,    74,    75,    76,    77
};
#endif

#define YYPACT_NINF (-179)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-183)

#define yytable_value_is_error(Yyn) \
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
static const yytype_int8 yyr2[] =
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


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

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

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


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
# ifndef YY_LOCATION_PRINT
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif


# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yykind < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yykind], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
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
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
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






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yykind);
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
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize;

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yynerrs = 0;
  yystate = 0;
  yyerrstatus = 0;

  yystacksize = YYINITDEPTH;
  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;


  YYDPRINTF ((stderr, "Starting parse\n"));

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
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
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
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

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

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= END_OF_FILE)
    {
      yychar = END_OF_FILE;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
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
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
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
#line 169 "xkbparse.y"
                        { (yyval.file)= rtrnValue= (yyvsp[0].file); }
#line 1744 "xkbparse.c"
    break;

  case 3:
#line 171 "xkbparse.y"
                        { (yyval.file)= rtrnValue= (yyvsp[0].file);  }
#line 1750 "xkbparse.c"
    break;

  case 4:
#line 173 "xkbparse.y"
                        { (yyval.file)= rtrnValue= (yyvsp[0].file); }
#line 1756 "xkbparse.c"
    break;

  case 5:
#line 177 "xkbparse.y"
                        { (yyval.file)= (XkbFile *)AppendStmt(&(yyvsp[-1].file)->common,&(yyvsp[0].file)->common); }
#line 1762 "xkbparse.c"
    break;

  case 6:
#line 179 "xkbparse.y"
                        { (yyval.file)= (yyvsp[0].file); }
#line 1768 "xkbparse.c"
    break;

  case 7:
#line 185 "xkbparse.y"
                        { (yyval.file)= CreateXKBFile((yyvsp[-5].uval),(yyvsp[-4].str),&(yyvsp[-2].file)->common,(yyvsp[-6].uval)); }
#line 1774 "xkbparse.c"
    break;

  case 8:
#line 188 "xkbparse.y"
                                        { (yyval.uval)= XkmKeymapFile; }
#line 1780 "xkbparse.c"
    break;

  case 9:
#line 189 "xkbparse.y"
                                        { (yyval.uval)= XkmSemanticsFile; }
#line 1786 "xkbparse.c"
    break;

  case 10:
#line 190 "xkbparse.y"
                                        { (yyval.uval)= XkmLayoutFile; }
#line 1792 "xkbparse.c"
    break;

  case 11:
#line 194 "xkbparse.y"
                        { (yyval.file)= (XkbFile *)AppendStmt(&(yyvsp[-1].file)->common,&(yyvsp[0].file)->common); }
#line 1798 "xkbparse.c"
    break;

  case 12:
#line 196 "xkbparse.y"
                        { (yyval.file)= (yyvsp[0].file); }
#line 1804 "xkbparse.c"
    break;

  case 13:
#line 202 "xkbparse.y"
                        { (yyval.file)= CreateXKBFile((yyvsp[-5].uval),(yyvsp[-4].str),(yyvsp[-2].any),(yyvsp[-6].uval)); }
#line 1810 "xkbparse.c"
    break;

  case 14:
#line 206 "xkbparse.y"
                        { (yyval.file)= CreateXKBFile((yyvsp[-2].uval),(yyvsp[-1].str),(yyvsp[0].any),(yyvsp[-3].uval)); }
#line 1816 "xkbparse.c"
    break;

  case 15:
#line 210 "xkbparse.y"
                                                { (yyval.uval)= XkmKeyNamesIndex; }
#line 1822 "xkbparse.c"
    break;

  case 16:
#line 211 "xkbparse.y"
                                                { (yyval.uval)= XkmTypesIndex; }
#line 1828 "xkbparse.c"
    break;

  case 17:
#line 212 "xkbparse.y"
                                                { (yyval.uval)= XkmCompatMapIndex; }
#line 1834 "xkbparse.c"
    break;

  case 18:
#line 213 "xkbparse.y"
                                                { (yyval.uval)= XkmSymbolsIndex; }
#line 1840 "xkbparse.c"
    break;

  case 19:
#line 214 "xkbparse.y"
                                                { (yyval.uval)= XkmGeometryIndex; }
#line 1846 "xkbparse.c"
    break;

  case 20:
#line 217 "xkbparse.y"
                                                { (yyval.uval)= (yyvsp[0].uval); }
#line 1852 "xkbparse.c"
    break;

  case 21:
#line 218 "xkbparse.y"
                                                { (yyval.uval)= 0; }
#line 1858 "xkbparse.c"
    break;

  case 22:
#line 221 "xkbparse.y"
                                                { (yyval.uval)= (((yyvsp[-1].uval))|((yyvsp[0].uval))); }
#line 1864 "xkbparse.c"
    break;

  case 23:
#line 222 "xkbparse.y"
                                                { (yyval.uval)= (yyvsp[0].uval); }
#line 1870 "xkbparse.c"
    break;

  case 24:
#line 225 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_Partial; }
#line 1876 "xkbparse.c"
    break;

  case 25:
#line 226 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_Default; }
#line 1882 "xkbparse.c"
    break;

  case 26:
#line 227 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_Hidden; }
#line 1888 "xkbparse.c"
    break;

  case 27:
#line 228 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_AlphanumericKeys; }
#line 1894 "xkbparse.c"
    break;

  case 28:
#line 229 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_ModifierKeys; }
#line 1900 "xkbparse.c"
    break;

  case 29:
#line 230 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_KeypadKeys; }
#line 1906 "xkbparse.c"
    break;

  case 30:
#line 231 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_FunctionKeys; }
#line 1912 "xkbparse.c"
    break;

  case 31:
#line 232 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_AlternateGroup; }
#line 1918 "xkbparse.c"
    break;

  case 32:
#line 236 "xkbparse.y"
                        { (yyval.any)= AppendStmt((yyvsp[-1].any),(yyvsp[0].any)); }
#line 1924 "xkbparse.c"
    break;

  case 33:
#line 237 "xkbparse.y"
                        { (yyval.any)= NULL; }
#line 1930 "xkbparse.c"
    break;

  case 34:
#line 241 "xkbparse.y"
                        {
			    (yyvsp[0].var)->merge= StmtSetMerge(&(yyvsp[0].var)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].var)->common;
			}
#line 1939 "xkbparse.c"
    break;

  case 35:
#line 246 "xkbparse.y"
                        {
			    (yyvsp[0].vmod)->merge= StmtSetMerge(&(yyvsp[0].vmod)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].vmod)->common;
			}
#line 1948 "xkbparse.c"
    break;

  case 36:
#line 251 "xkbparse.y"
                        {
			    (yyvsp[0].interp)->merge= StmtSetMerge(&(yyvsp[0].interp)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].interp)->common;
			}
#line 1957 "xkbparse.c"
    break;

  case 37:
#line 256 "xkbparse.y"
                        {
			    (yyvsp[0].keyName)->merge= StmtSetMerge(&(yyvsp[0].keyName)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].keyName)->common;
			}
#line 1966 "xkbparse.c"
    break;

  case 38:
#line 261 "xkbparse.y"
                        {
			    (yyvsp[0].keyAlias)->merge= StmtSetMerge(&(yyvsp[0].keyAlias)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].keyAlias)->common;
			}
#line 1975 "xkbparse.c"
    break;

  case 39:
#line 266 "xkbparse.y"
                        {
			    (yyvsp[0].keyType)->merge= StmtSetMerge(&(yyvsp[0].keyType)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].keyType)->common;
			}
#line 1984 "xkbparse.c"
    break;

  case 40:
#line 271 "xkbparse.y"
                        {
			    (yyvsp[0].syms)->merge= StmtSetMerge(&(yyvsp[0].syms)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].syms)->common;
			}
#line 1993 "xkbparse.c"
    break;

  case 41:
#line 276 "xkbparse.y"
                        {
			    (yyvsp[0].modMask)->merge= StmtSetMerge(&(yyvsp[0].modMask)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].modMask)->common;
			}
#line 2002 "xkbparse.c"
    break;

  case 42:
#line 281 "xkbparse.y"
                        {
			    (yyvsp[0].groupCompat)->merge= StmtSetMerge(&(yyvsp[0].groupCompat)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].groupCompat)->common;
			}
#line 2011 "xkbparse.c"
    break;

  case 43:
#line 286 "xkbparse.y"
                        {
			    (yyvsp[0].ledMap)->merge= StmtSetMerge(&(yyvsp[0].ledMap)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].ledMap)->common;
			}
#line 2020 "xkbparse.c"
    break;

  case 44:
#line 291 "xkbparse.y"
                        {
			    (yyvsp[0].ledName)->merge= StmtSetMerge(&(yyvsp[0].ledName)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].ledName)->common;
			}
#line 2029 "xkbparse.c"
    break;

  case 45:
#line 296 "xkbparse.y"
                        {
			    (yyvsp[0].shape)->merge= StmtSetMerge(&(yyvsp[0].shape)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].shape)->common;
			}
#line 2038 "xkbparse.c"
    break;

  case 46:
#line 301 "xkbparse.y"
                        {
			    (yyvsp[0].section)->merge= StmtSetMerge(&(yyvsp[0].section)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].section)->common;
			}
#line 2047 "xkbparse.c"
    break;

  case 47:
#line 306 "xkbparse.y"
                        {
			    (yyvsp[0].doodad)->merge= StmtSetMerge(&(yyvsp[0].doodad)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].doodad)->common;
			}
#line 2056 "xkbparse.c"
    break;

  case 48:
#line 311 "xkbparse.y"
                        {
			    if ((yyvsp[-1].uval)==MergeAltForm) {
				yyerror("cannot use 'alternate' to include other maps");
				(yyval.any)= &IncludeCreate(scanBuf,MergeDefault)->common;
			    }
			    else {
				(yyval.any)= &IncludeCreate(scanBuf,(yyvsp[-1].uval))->common;
			    }
                        }
#line 2070 "xkbparse.c"
    break;

  case 49:
#line 323 "xkbparse.y"
                        { (yyval.var)= VarCreate((yyvsp[-3].expr),(yyvsp[-1].expr)); }
#line 2076 "xkbparse.c"
    break;

  case 50:
#line 325 "xkbparse.y"
                        { (yyval.var)= BoolVarCreate((yyvsp[-1].sval),1); }
#line 2082 "xkbparse.c"
    break;

  case 51:
#line 327 "xkbparse.y"
                        { (yyval.var)= BoolVarCreate((yyvsp[-1].sval),0); }
#line 2088 "xkbparse.c"
    break;

  case 52:
#line 331 "xkbparse.y"
                        {
			    KeycodeDef *def;

			    def= KeycodeCreate((yyvsp[-3].str),(yyvsp[-1].expr));
			    if ((yyvsp[-3].str))
				free((yyvsp[-3].str));
			    (yyval.keyName)= def;
			}
#line 2101 "xkbparse.c"
    break;

  case 53:
#line 342 "xkbparse.y"
                        {
			    KeyAliasDef	*def;
			    def= KeyAliasCreate((yyvsp[-3].str),(yyvsp[-1].str));
			    if ((yyvsp[-3].str))	free((yyvsp[-3].str));	
			    if ((yyvsp[-1].str))	free((yyvsp[-1].str));	
			    (yyval.keyAlias)= def;
			}
#line 2113 "xkbparse.c"
    break;

  case 54:
#line 352 "xkbparse.y"
                        { (yyval.vmod)= (yyvsp[-1].vmod); }
#line 2119 "xkbparse.c"
    break;

  case 55:
#line 356 "xkbparse.y"
                        { (yyval.vmod)= (VModDef *)AppendStmt(&(yyvsp[-2].vmod)->common,&(yyvsp[0].vmod)->common); }
#line 2125 "xkbparse.c"
    break;

  case 56:
#line 358 "xkbparse.y"
                        { (yyval.vmod)= (yyvsp[0].vmod); }
#line 2131 "xkbparse.c"
    break;

  case 57:
#line 362 "xkbparse.y"
                        { (yyval.vmod)= VModCreate((yyvsp[0].sval),NULL); }
#line 2137 "xkbparse.c"
    break;

  case 58:
#line 364 "xkbparse.y"
                        { (yyval.vmod)= VModCreate((yyvsp[-2].sval),(yyvsp[0].expr)); }
#line 2143 "xkbparse.c"
    break;

  case 59:
#line 370 "xkbparse.y"
                        {
			    (yyvsp[-4].interp)->def= (yyvsp[-2].var);
			    (yyval.interp)= (yyvsp[-4].interp);
			}
#line 2152 "xkbparse.c"
    break;

  case 60:
#line 377 "xkbparse.y"
                        { (yyval.interp)= InterpCreate((yyvsp[-2].str), (yyvsp[0].expr)); }
#line 2158 "xkbparse.c"
    break;

  case 61:
#line 379 "xkbparse.y"
                        { (yyval.interp)= InterpCreate((yyvsp[0].str), NULL); }
#line 2164 "xkbparse.c"
    break;

  case 62:
#line 383 "xkbparse.y"
                        { (yyval.var)= (VarDef *)AppendStmt(&(yyvsp[-1].var)->common,&(yyvsp[0].var)->common); }
#line 2170 "xkbparse.c"
    break;

  case 63:
#line 385 "xkbparse.y"
                        { (yyval.var)= (yyvsp[0].var); }
#line 2176 "xkbparse.c"
    break;

  case 64:
#line 391 "xkbparse.y"
                        { (yyval.keyType)= KeyTypeCreate((yyvsp[-4].sval),(yyvsp[-2].var)); }
#line 2182 "xkbparse.c"
    break;

  case 65:
#line 397 "xkbparse.y"
                        { (yyval.syms)= SymbolsCreate((yyvsp[-4].str),(ExprDef *)(yyvsp[-2].var)); }
#line 2188 "xkbparse.c"
    break;

  case 66:
#line 401 "xkbparse.y"
                        { (yyval.var)= (VarDef *)AppendStmt(&(yyvsp[-2].var)->common,&(yyvsp[0].var)->common); }
#line 2194 "xkbparse.c"
    break;

  case 67:
#line 403 "xkbparse.y"
                        { (yyval.var)= (yyvsp[0].var); }
#line 2200 "xkbparse.c"
    break;

  case 68:
#line 404 "xkbparse.y"
                        { (yyval.var)= NULL; }
#line 2206 "xkbparse.c"
    break;

  case 69:
#line 408 "xkbparse.y"
                        { (yyval.var)= VarCreate((yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2212 "xkbparse.c"
    break;

  case 70:
#line 410 "xkbparse.y"
                        { (yyval.var)= VarCreate((yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2218 "xkbparse.c"
    break;

  case 71:
#line 412 "xkbparse.y"
                        { (yyval.var)= BoolVarCreate((yyvsp[0].sval),1); }
#line 2224 "xkbparse.c"
    break;

  case 72:
#line 414 "xkbparse.y"
                        { (yyval.var)= BoolVarCreate((yyvsp[0].sval),0); }
#line 2230 "xkbparse.c"
    break;

  case 73:
#line 416 "xkbparse.y"
                        { (yyval.var)= VarCreate(NULL,(yyvsp[0].expr)); }
#line 2236 "xkbparse.c"
    break;

  case 74:
#line 420 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[-1].expr); }
#line 2242 "xkbparse.c"
    break;

  case 75:
#line 422 "xkbparse.y"
                        { (yyval.expr)= ExprCreateUnary(ExprActionList,TypeAction,(yyvsp[-1].expr)); }
#line 2248 "xkbparse.c"
    break;

  case 76:
#line 426 "xkbparse.y"
                        { (yyval.groupCompat)= GroupCompatCreate((yyvsp[-3].ival),(yyvsp[-1].expr)); }
#line 2254 "xkbparse.c"
    break;

  case 77:
#line 430 "xkbparse.y"
                        { (yyval.modMask)= ModMapCreate((yyvsp[-4].sval),(yyvsp[-2].expr)); }
#line 2260 "xkbparse.c"
    break;

  case 78:
#line 434 "xkbparse.y"
                        { (yyval.ledMap)= IndicatorMapCreate((yyvsp[-4].sval),(yyvsp[-2].var)); }
#line 2266 "xkbparse.c"
    break;

  case 79:
#line 438 "xkbparse.y"
                        { (yyval.ledName)= IndicatorNameCreate((yyvsp[-3].ival),(yyvsp[-1].expr),False); }
#line 2272 "xkbparse.c"
    break;

  case 80:
#line 440 "xkbparse.y"
                        { (yyval.ledName)= IndicatorNameCreate((yyvsp[-3].ival),(yyvsp[-1].expr),True); }
#line 2278 "xkbparse.c"
    break;

  case 81:
#line 444 "xkbparse.y"
                        { (yyval.shape)= ShapeDeclCreate((yyvsp[-4].sval),(OutlineDef *)&(yyvsp[-2].outline)->common); }
#line 2284 "xkbparse.c"
    break;

  case 82:
#line 446 "xkbparse.y"
                        {
			    OutlineDef *outlines;
			    outlines= OutlineCreate(None,(yyvsp[-2].expr));
			    (yyval.shape)= ShapeDeclCreate((yyvsp[-4].sval),outlines);
			}
#line 2294 "xkbparse.c"
    break;

  case 83:
#line 454 "xkbparse.y"
                        { (yyval.section)= SectionDeclCreate((yyvsp[-4].sval),(yyvsp[-2].row)); }
#line 2300 "xkbparse.c"
    break;

  case 84:
#line 458 "xkbparse.y"
                        { (yyval.row)=(RowDef *)AppendStmt(&(yyvsp[-1].row)->common,&(yyvsp[0].row)->common);}
#line 2306 "xkbparse.c"
    break;

  case 85:
#line 460 "xkbparse.y"
                        { (yyval.row)= (yyvsp[0].row); }
#line 2312 "xkbparse.c"
    break;

  case 86:
#line 464 "xkbparse.y"
                        { (yyval.row)= RowDeclCreate((yyvsp[-2].key)); }
#line 2318 "xkbparse.c"
    break;

  case 87:
#line 466 "xkbparse.y"
                        { (yyval.row)= (RowDef *)(yyvsp[0].var); }
#line 2324 "xkbparse.c"
    break;

  case 88:
#line 468 "xkbparse.y"
                        { (yyval.row)= (RowDef *)(yyvsp[0].doodad); }
#line 2330 "xkbparse.c"
    break;

  case 89:
#line 470 "xkbparse.y"
                        { (yyval.row)= (RowDef *)(yyvsp[0].ledMap); }
#line 2336 "xkbparse.c"
    break;

  case 90:
#line 472 "xkbparse.y"
                        { (yyval.row)= (RowDef *)(yyvsp[0].overlay); }
#line 2342 "xkbparse.c"
    break;

  case 91:
#line 476 "xkbparse.y"
                        { (yyval.key)=(KeyDef *)AppendStmt(&(yyvsp[-1].key)->common,&(yyvsp[0].key)->common);}
#line 2348 "xkbparse.c"
    break;

  case 92:
#line 478 "xkbparse.y"
                        { (yyval.key)= (yyvsp[0].key); }
#line 2354 "xkbparse.c"
    break;

  case 93:
#line 482 "xkbparse.y"
                        { (yyval.key)= (yyvsp[-2].key); }
#line 2360 "xkbparse.c"
    break;

  case 94:
#line 484 "xkbparse.y"
                        { (yyval.key)= (KeyDef *)(yyvsp[0].var); }
#line 2366 "xkbparse.c"
    break;

  case 95:
#line 488 "xkbparse.y"
                        { (yyval.key)=(KeyDef *)AppendStmt(&(yyvsp[-2].key)->common,&(yyvsp[0].key)->common);}
#line 2372 "xkbparse.c"
    break;

  case 96:
#line 490 "xkbparse.y"
                        { (yyval.key)= (yyvsp[0].key); }
#line 2378 "xkbparse.c"
    break;

  case 97:
#line 494 "xkbparse.y"
                        { (yyval.key)= KeyDeclCreate((yyvsp[0].str),NULL); }
#line 2384 "xkbparse.c"
    break;

  case 98:
#line 496 "xkbparse.y"
                        { (yyval.key)= KeyDeclCreate(NULL,(yyvsp[-1].expr)); }
#line 2390 "xkbparse.c"
    break;

  case 99:
#line 500 "xkbparse.y"
                        { (yyval.overlay)= OverlayDeclCreate((yyvsp[-4].sval),(yyvsp[-2].olKey)); }
#line 2396 "xkbparse.c"
    break;

  case 100:
#line 504 "xkbparse.y"
                        {
			    (yyval.olKey)= (OverlayKeyDef *)
				AppendStmt(&(yyvsp[-2].olKey)->common,&(yyvsp[0].olKey)->common);
			}
#line 2405 "xkbparse.c"
    break;

  case 101:
#line 509 "xkbparse.y"
                        { (yyval.olKey)= (yyvsp[0].olKey); }
#line 2411 "xkbparse.c"
    break;

  case 102:
#line 513 "xkbparse.y"
                        { (yyval.olKey)= OverlayKeyCreate((yyvsp[-2].str),(yyvsp[0].str)); }
#line 2417 "xkbparse.c"
    break;

  case 103:
#line 517 "xkbparse.y"
                        { (yyval.outline)=(OutlineDef *)AppendStmt(&(yyvsp[-2].outline)->common,&(yyvsp[0].outline)->common);}
#line 2423 "xkbparse.c"
    break;

  case 104:
#line 519 "xkbparse.y"
                        { (yyval.outline)= (yyvsp[0].outline); }
#line 2429 "xkbparse.c"
    break;

  case 105:
#line 523 "xkbparse.y"
                        { (yyval.outline)= OutlineCreate(None,(yyvsp[-1].expr)); }
#line 2435 "xkbparse.c"
    break;

  case 106:
#line 525 "xkbparse.y"
                        { (yyval.outline)= OutlineCreate((yyvsp[-4].sval),(yyvsp[-1].expr)); }
#line 2441 "xkbparse.c"
    break;

  case 107:
#line 527 "xkbparse.y"
                        { (yyval.outline)= OutlineCreate((yyvsp[-2].sval),(yyvsp[0].expr)); }
#line 2447 "xkbparse.c"
    break;

  case 108:
#line 531 "xkbparse.y"
                        { (yyval.expr)= (ExprDef *)AppendStmt(&(yyvsp[-2].expr)->common,&(yyvsp[0].expr)->common); }
#line 2453 "xkbparse.c"
    break;

  case 109:
#line 533 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[0].expr); }
#line 2459 "xkbparse.c"
    break;

  case 110:
#line 537 "xkbparse.y"
                        {
			    ExprDef *expr;
			    expr= ExprCreate(ExprCoord,TypeUnknown);
			    expr->value.coord.x= (yyvsp[-3].ival);
			    expr->value.coord.y= (yyvsp[-1].ival);
			    (yyval.expr)= expr;
			}
#line 2471 "xkbparse.c"
    break;

  case 111:
#line 547 "xkbparse.y"
                        { (yyval.doodad)= DoodadCreate((yyvsp[-5].uval),(yyvsp[-4].sval),(yyvsp[-2].var)); }
#line 2477 "xkbparse.c"
    break;

  case 112:
#line 550 "xkbparse.y"
                                                { (yyval.uval)= XkbTextDoodad; }
#line 2483 "xkbparse.c"
    break;

  case 113:
#line 551 "xkbparse.y"
                                                { (yyval.uval)= XkbOutlineDoodad; }
#line 2489 "xkbparse.c"
    break;

  case 114:
#line 552 "xkbparse.y"
                                                { (yyval.uval)= XkbSolidDoodad; }
#line 2495 "xkbparse.c"
    break;

  case 115:
#line 553 "xkbparse.y"
                                                { (yyval.uval)= XkbLogoDoodad; }
#line 2501 "xkbparse.c"
    break;

  case 116:
#line 556 "xkbparse.y"
                                                { (yyval.sval)= (yyvsp[0].sval); }
#line 2507 "xkbparse.c"
    break;

  case 117:
#line 557 "xkbparse.y"
                                                { (yyval.sval)= (yyvsp[0].sval); }
#line 2513 "xkbparse.c"
    break;

  case 118:
#line 561 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"action",False); }
#line 2519 "xkbparse.c"
    break;

  case 119:
#line 563 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"interpret",False); }
#line 2525 "xkbparse.c"
    break;

  case 120:
#line 565 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"type",False); }
#line 2531 "xkbparse.c"
    break;

  case 121:
#line 567 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"key",False); }
#line 2537 "xkbparse.c"
    break;

  case 122:
#line 569 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"group",False); }
#line 2543 "xkbparse.c"
    break;

  case 123:
#line 571 "xkbparse.y"
                        {(yyval.sval)=XkbInternAtom(NULL,"modifier_map",False);}
#line 2549 "xkbparse.c"
    break;

  case 124:
#line 573 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"indicator",False); }
#line 2555 "xkbparse.c"
    break;

  case 125:
#line 575 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"shape",False); }
#line 2561 "xkbparse.c"
    break;

  case 126:
#line 577 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"row",False); }
#line 2567 "xkbparse.c"
    break;

  case 127:
#line 579 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"section",False); }
#line 2573 "xkbparse.c"
    break;

  case 128:
#line 581 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"text",False); }
#line 2579 "xkbparse.c"
    break;

  case 129:
#line 584 "xkbparse.y"
                                                { (yyval.uval)= (yyvsp[0].uval); }
#line 2585 "xkbparse.c"
    break;

  case 130:
#line 585 "xkbparse.y"
                                                { (yyval.uval)= MergeDefault; }
#line 2591 "xkbparse.c"
    break;

  case 131:
#line 588 "xkbparse.y"
                                                { (yyval.uval)= MergeDefault; }
#line 2597 "xkbparse.c"
    break;

  case 132:
#line 589 "xkbparse.y"
                                                { (yyval.uval)= MergeAugment; }
#line 2603 "xkbparse.c"
    break;

  case 133:
#line 590 "xkbparse.y"
                                                { (yyval.uval)= MergeOverride; }
#line 2609 "xkbparse.c"
    break;

  case 134:
#line 591 "xkbparse.y"
                                                { (yyval.uval)= MergeReplace; }
#line 2615 "xkbparse.c"
    break;

  case 135:
#line 592 "xkbparse.y"
                                                { (yyval.uval)= MergeAltForm; }
#line 2621 "xkbparse.c"
    break;

  case 136:
#line 595 "xkbparse.y"
                                                        { (yyval.expr)= (yyvsp[0].expr); }
#line 2627 "xkbparse.c"
    break;

  case 137:
#line 596 "xkbparse.y"
                                                { (yyval.expr)= NULL; }
#line 2633 "xkbparse.c"
    break;

  case 138:
#line 600 "xkbparse.y"
                        { (yyval.expr)= (ExprDef *)AppendStmt(&(yyvsp[-2].expr)->common,&(yyvsp[0].expr)->common); }
#line 2639 "xkbparse.c"
    break;

  case 139:
#line 602 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[0].expr); }
#line 2645 "xkbparse.c"
    break;

  case 140:
#line 606 "xkbparse.y"
                        { (yyval.expr)= ExprCreateBinary(OpDivide,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2651 "xkbparse.c"
    break;

  case 141:
#line 608 "xkbparse.y"
                        { (yyval.expr)= ExprCreateBinary(OpAdd,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2657 "xkbparse.c"
    break;

  case 142:
#line 610 "xkbparse.y"
                        { (yyval.expr)= ExprCreateBinary(OpSubtract,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2663 "xkbparse.c"
    break;

  case 143:
#line 612 "xkbparse.y"
                        { (yyval.expr)= ExprCreateBinary(OpMultiply,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2669 "xkbparse.c"
    break;

  case 144:
#line 614 "xkbparse.y"
                        { (yyval.expr)= ExprCreateBinary(OpAssign,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2675 "xkbparse.c"
    break;

  case 145:
#line 616 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[0].expr); }
#line 2681 "xkbparse.c"
    break;

  case 146:
#line 620 "xkbparse.y"
                        { (yyval.expr)= ExprCreateUnary(OpNegate,(yyvsp[0].expr)->type,(yyvsp[0].expr)); }
#line 2687 "xkbparse.c"
    break;

  case 147:
#line 622 "xkbparse.y"
                        { (yyval.expr)= ExprCreateUnary(OpUnaryPlus,(yyvsp[0].expr)->type,(yyvsp[0].expr)); }
#line 2693 "xkbparse.c"
    break;

  case 148:
#line 624 "xkbparse.y"
                        { (yyval.expr)= ExprCreateUnary(OpNot,TypeBoolean,(yyvsp[0].expr)); }
#line 2699 "xkbparse.c"
    break;

  case 149:
#line 626 "xkbparse.y"
                        { (yyval.expr)= ExprCreateUnary(OpInvert,(yyvsp[0].expr)->type,(yyvsp[0].expr)); }
#line 2705 "xkbparse.c"
    break;

  case 150:
#line 628 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[0].expr);  }
#line 2711 "xkbparse.c"
    break;

  case 151:
#line 630 "xkbparse.y"
                        { (yyval.expr)= ActionCreate((yyvsp[-3].sval),(yyvsp[-1].expr)); }
#line 2717 "xkbparse.c"
    break;

  case 152:
#line 632 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[0].expr);  }
#line 2723 "xkbparse.c"
    break;

  case 153:
#line 634 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[-1].expr);  }
#line 2729 "xkbparse.c"
    break;

  case 154:
#line 638 "xkbparse.y"
                        { (yyval.expr)= (ExprDef *)AppendStmt(&(yyvsp[-2].expr)->common,&(yyvsp[0].expr)->common); }
#line 2735 "xkbparse.c"
    break;

  case 155:
#line 640 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[0].expr); }
#line 2741 "xkbparse.c"
    break;

  case 156:
#line 644 "xkbparse.y"
                        { (yyval.expr)= ActionCreate((yyvsp[-3].sval),(yyvsp[-1].expr)); }
#line 2747 "xkbparse.c"
    break;

  case 157:
#line 648 "xkbparse.y"
                        {
			    ExprDef *expr;
                            expr= ExprCreate(ExprIdent,TypeUnknown);
                            expr->value.str= (yyvsp[0].sval);
                            (yyval.expr)= expr;
			}
#line 2758 "xkbparse.c"
    break;

  case 158:
#line 655 "xkbparse.y"
                        {
                            ExprDef *expr;
                            expr= ExprCreate(ExprFieldRef,TypeUnknown);
                            expr->value.field.element= (yyvsp[-2].sval);
                            expr->value.field.field= (yyvsp[0].sval);
                            (yyval.expr)= expr;
			}
#line 2770 "xkbparse.c"
    break;

  case 159:
#line 663 "xkbparse.y"
                        {
			    ExprDef *expr;
			    expr= ExprCreate(ExprArrayRef,TypeUnknown);
			    expr->value.array.element= None;
			    expr->value.array.field= (yyvsp[-3].sval);
			    expr->value.array.entry= (yyvsp[-1].expr);
			    (yyval.expr)= expr;
			}
#line 2783 "xkbparse.c"
    break;

  case 160:
#line 672 "xkbparse.y"
                        {
			    ExprDef *expr;
			    expr= ExprCreate(ExprArrayRef,TypeUnknown);
			    expr->value.array.element= (yyvsp[-5].sval);
			    expr->value.array.field= (yyvsp[-3].sval);
			    expr->value.array.entry= (yyvsp[-1].expr);
			    (yyval.expr)= expr;
			}
#line 2796 "xkbparse.c"
    break;

  case 161:
#line 683 "xkbparse.y"
                        {
			    ExprDef *expr;
                            expr= ExprCreate(ExprValue,TypeString);
                            expr->value.str= (yyvsp[0].sval);
                            (yyval.expr)= expr;
			}
#line 2807 "xkbparse.c"
    break;

  case 162:
#line 690 "xkbparse.y"
                        {
			    ExprDef *expr;
                            expr= ExprCreate(ExprValue,TypeInt);
                            expr->value.ival= (yyvsp[0].ival);
                            (yyval.expr)= expr;
			}
#line 2818 "xkbparse.c"
    break;

  case 163:
#line 697 "xkbparse.y"
                        {
			    ExprDef *expr;
			    expr= ExprCreate(ExprValue,TypeFloat);
			    expr->value.ival= (yyvsp[0].ival);
			    (yyval.expr)= expr;
			}
#line 2829 "xkbparse.c"
    break;

  case 164:
#line 704 "xkbparse.y"
                        {
			    ExprDef *expr;
			    expr= ExprCreate(ExprValue,TypeKeyName);
			    memset(expr->value.keyName,0,5);
			    strncpy(expr->value.keyName,(yyvsp[0].str),4);
			    free((yyvsp[0].str));
			    (yyval.expr)= expr;
			}
#line 2842 "xkbparse.c"
    break;

  case 165:
#line 714 "xkbparse.y"
                                                        { (yyval.expr)= (yyvsp[0].expr); }
#line 2848 "xkbparse.c"
    break;

  case 166:
#line 715 "xkbparse.y"
                                                        { (yyval.expr)= NULL; }
#line 2854 "xkbparse.c"
    break;

  case 167:
#line 719 "xkbparse.y"
                        { (yyval.expr)= AppendKeysymList((yyvsp[-2].expr),(yyvsp[0].str)); }
#line 2860 "xkbparse.c"
    break;

  case 168:
#line 721 "xkbparse.y"
                        { (yyval.expr)= AppendKeysymList((yyvsp[-2].expr),strdup("NoSymbol")); }
#line 2866 "xkbparse.c"
    break;

  case 169:
#line 723 "xkbparse.y"
                        { (yyval.expr)= CreateKeysymList((yyvsp[0].str)); }
#line 2872 "xkbparse.c"
    break;

  case 170:
#line 725 "xkbparse.y"
                        { (yyval.expr)= CreateKeysymList(strdup("NoSymbol")); }
#line 2878 "xkbparse.c"
    break;

  case 171:
#line 728 "xkbparse.y"
                                        { (yyval.str)= strdup(scanBuf); }
#line 2884 "xkbparse.c"
    break;

  case 172:
#line 729 "xkbparse.y"
                                        { (yyval.str)= strdup("section"); }
#line 2890 "xkbparse.c"
    break;

  case 173:
#line 731 "xkbparse.y"
                        {
			    if ((yyvsp[0].ival)<10)	{ (yyval.str)= malloc(2); (yyval.str)[0]= '0' + (yyvsp[0].ival); (yyval.str)[1]= '\0'; }
			    else	{ (yyval.str)= malloc(19); snprintf((yyval.str), 19, "0x%x", (yyvsp[0].ival)); }
			}
#line 2899 "xkbparse.c"
    break;

  case 174:
#line 738 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[-1].expr); }
#line 2905 "xkbparse.c"
    break;

  case 175:
#line 741 "xkbparse.y"
                                        { (yyval.ival)= -(yyvsp[0].ival); }
#line 2911 "xkbparse.c"
    break;

  case 176:
#line 742 "xkbparse.y"
                                            { (yyval.ival)= (yyvsp[0].ival); }
#line 2917 "xkbparse.c"
    break;

  case 177:
#line 745 "xkbparse.y"
                                        { (yyval.ival)= scanInt; }
#line 2923 "xkbparse.c"
    break;

  case 178:
#line 746 "xkbparse.y"
                                        { (yyval.ival)= scanInt*XkbGeomPtsPerMM; }
#line 2929 "xkbparse.c"
    break;

  case 179:
#line 749 "xkbparse.y"
                                        { (yyval.ival)= scanInt; }
#line 2935 "xkbparse.c"
    break;

  case 180:
#line 752 "xkbparse.y"
                                        { (yyval.ival)= scanInt; }
#line 2941 "xkbparse.c"
    break;

  case 181:
#line 755 "xkbparse.y"
                                        { (yyval.str)= strdup(scanBuf); }
#line 2947 "xkbparse.c"
    break;

  case 182:
#line 758 "xkbparse.y"
                                { (yyval.sval)= XkbInternAtom(NULL,scanBuf,False); }
#line 2953 "xkbparse.c"
    break;

  case 183:
#line 759 "xkbparse.y"
                                { (yyval.sval)= XkbInternAtom(NULL,"default",False); }
#line 2959 "xkbparse.c"
    break;

  case 184:
#line 762 "xkbparse.y"
                                { (yyval.sval)= XkbInternAtom(NULL,scanBuf,False); }
#line 2965 "xkbparse.c"
    break;

  case 185:
#line 765 "xkbparse.y"
                                { (yyval.str)= (yyvsp[0].str); }
#line 2971 "xkbparse.c"
    break;

  case 186:
#line 766 "xkbparse.y"
                                { (yyval.str)= NULL; }
#line 2977 "xkbparse.c"
    break;

  case 187:
#line 769 "xkbparse.y"
                                { (yyval.str)= strdup(scanBuf); }
#line 2983 "xkbparse.c"
    break;


#line 2987 "xkbparse.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

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
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= END_OF_FILE)
        {
          /* Return failure if at end of input.  */
          if (yychar == END_OF_FILE)
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

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
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
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

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


#if !defined yyoverflow
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
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 771 "xkbparse.y"

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

