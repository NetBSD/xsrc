/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
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
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

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

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

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
#define	DEBUG_VAR parseDebug
unsigned int parseDebug;
#endif
#include "parseutils.h"
#include <X11/keysym.h>
#include <X11/extensions/XKBgeom.h>
#include <stdlib.h>


#line 84 "xkbparse.c"

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
#define YYEMPTY -2
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
#line 109 "xkbparse.y"

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

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
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
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
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

/* YYMAXUTOK -- Last valid token kind.  */
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
       0,   167,   167,   169,   171,   175,   177,   181,   187,   188,
     189,   192,   194,   198,   204,   209,   210,   211,   212,   213,
     216,   217,   220,   221,   224,   225,   226,   227,   228,   229,
     230,   231,   234,   236,   239,   244,   249,   254,   259,   264,
     269,   274,   279,   284,   289,   294,   299,   304,   309,   321,
     323,   325,   329,   340,   350,   354,   356,   360,   362,   366,
     375,   377,   381,   383,   387,   393,   399,   401,   403,   406,
     408,   410,   412,   414,   418,   420,   424,   428,   432,   436,
     438,   442,   444,   452,   456,   458,   462,   464,   466,   468,
     470,   474,   476,   480,   482,   486,   488,   492,   494,   498,
     502,   507,   511,   515,   517,   521,   523,   525,   529,   531,
     535,   545,   549,   550,   551,   552,   555,   556,   559,   561,
     563,   565,   567,   569,   571,   573,   575,   577,   579,   583,
     584,   587,   588,   589,   590,   591,   594,   595,   598,   600,
     604,   606,   608,   610,   612,   614,   618,   620,   622,   624,
     626,   628,   630,   632,   636,   638,   642,   646,   653,   661,
     670,   681,   688,   695,   702,   713,   714,   717,   719,   721,
     723,   727,   728,   729,   736,   740,   741,   744,   745,   748,
     751,   754,   757,   758,   761,   764,   765,   768
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
       0,     9,    10,    11,    31,    12,    13,    14,    32,    22,
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

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
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

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
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

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
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
#define YYNOMEM         goto yyexhaustedlab


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
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
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
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
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
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

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
    YYNOMEM;
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
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
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
  case 2: /* XkbFile: XkbCompMapList  */
#line 168 "xkbparse.y"
                        { (yyval.file)= rtrnValue= (yyvsp[0].file); }
#line 1733 "xkbparse.c"
    break;

  case 3: /* XkbFile: XkbMapConfigList  */
#line 170 "xkbparse.y"
                        { (yyval.file)= rtrnValue= (yyvsp[0].file);  }
#line 1739 "xkbparse.c"
    break;

  case 4: /* XkbFile: XkbConfig  */
#line 172 "xkbparse.y"
                        { (yyval.file)= rtrnValue= (yyvsp[0].file); }
#line 1745 "xkbparse.c"
    break;

  case 5: /* XkbCompMapList: XkbCompMapList XkbCompositeMap  */
#line 176 "xkbparse.y"
                        { (yyval.file)= (XkbFile *)AppendStmt(&(yyvsp[-1].file)->common,&(yyvsp[0].file)->common); }
#line 1751 "xkbparse.c"
    break;

  case 6: /* XkbCompMapList: XkbCompositeMap  */
#line 178 "xkbparse.y"
                        { (yyval.file)= (yyvsp[0].file); }
#line 1757 "xkbparse.c"
    break;

  case 7: /* XkbCompositeMap: OptFlags XkbCompositeType OptMapName OBRACE XkbMapConfigList CBRACE SEMI  */
#line 184 "xkbparse.y"
                        { (yyval.file)= CreateXKBFile((yyvsp[-5].uval),(yyvsp[-4].str),&(yyvsp[-2].file)->common,(yyvsp[-6].uval)); }
#line 1763 "xkbparse.c"
    break;

  case 8: /* XkbCompositeType: XKB_KEYMAP  */
#line 187 "xkbparse.y"
                                        { (yyval.uval)= XkmKeymapFile; }
#line 1769 "xkbparse.c"
    break;

  case 9: /* XkbCompositeType: XKB_SEMANTICS  */
#line 188 "xkbparse.y"
                                        { (yyval.uval)= XkmSemanticsFile; }
#line 1775 "xkbparse.c"
    break;

  case 10: /* XkbCompositeType: XKB_LAYOUT  */
#line 189 "xkbparse.y"
                                        { (yyval.uval)= XkmLayoutFile; }
#line 1781 "xkbparse.c"
    break;

  case 11: /* XkbMapConfigList: XkbMapConfigList XkbMapConfig  */
#line 193 "xkbparse.y"
                        { (yyval.file)= (XkbFile *)AppendStmt(&(yyvsp[-1].file)->common,&(yyvsp[0].file)->common); }
#line 1787 "xkbparse.c"
    break;

  case 12: /* XkbMapConfigList: XkbMapConfig  */
#line 195 "xkbparse.y"
                        { (yyval.file)= (yyvsp[0].file); }
#line 1793 "xkbparse.c"
    break;

  case 13: /* XkbMapConfig: OptFlags FileType OptMapName OBRACE DeclList CBRACE SEMI  */
#line 201 "xkbparse.y"
                        { (yyval.file)= CreateXKBFile((yyvsp[-5].uval),(yyvsp[-4].str),(yyvsp[-2].any),(yyvsp[-6].uval)); }
#line 1799 "xkbparse.c"
    break;

  case 14: /* XkbConfig: OptFlags FileType OptMapName DeclList  */
#line 205 "xkbparse.y"
                        { (yyval.file)= CreateXKBFile((yyvsp[-2].uval),(yyvsp[-1].str),(yyvsp[0].any),(yyvsp[-3].uval)); }
#line 1805 "xkbparse.c"
    break;

  case 15: /* FileType: XKB_KEYCODES  */
#line 209 "xkbparse.y"
                                                { (yyval.uval)= XkmKeyNamesIndex; }
#line 1811 "xkbparse.c"
    break;

  case 16: /* FileType: XKB_TYPES  */
#line 210 "xkbparse.y"
                                                { (yyval.uval)= XkmTypesIndex; }
#line 1817 "xkbparse.c"
    break;

  case 17: /* FileType: XKB_COMPATMAP  */
#line 211 "xkbparse.y"
                                                { (yyval.uval)= XkmCompatMapIndex; }
#line 1823 "xkbparse.c"
    break;

  case 18: /* FileType: XKB_SYMBOLS  */
#line 212 "xkbparse.y"
                                                { (yyval.uval)= XkmSymbolsIndex; }
#line 1829 "xkbparse.c"
    break;

  case 19: /* FileType: XKB_GEOMETRY  */
#line 213 "xkbparse.y"
                                                { (yyval.uval)= XkmGeometryIndex; }
#line 1835 "xkbparse.c"
    break;

  case 20: /* OptFlags: Flags  */
#line 216 "xkbparse.y"
                                                { (yyval.uval)= (yyvsp[0].uval); }
#line 1841 "xkbparse.c"
    break;

  case 21: /* OptFlags: %empty  */
#line 217 "xkbparse.y"
                                                { (yyval.uval)= 0; }
#line 1847 "xkbparse.c"
    break;

  case 22: /* Flags: Flags Flag  */
#line 220 "xkbparse.y"
                                                { (yyval.uval)= (((yyvsp[-1].uval))|((yyvsp[0].uval))); }
#line 1853 "xkbparse.c"
    break;

  case 23: /* Flags: Flag  */
#line 221 "xkbparse.y"
                                                { (yyval.uval)= (yyvsp[0].uval); }
#line 1859 "xkbparse.c"
    break;

  case 24: /* Flag: PARTIAL  */
#line 224 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_Partial; }
#line 1865 "xkbparse.c"
    break;

  case 25: /* Flag: DEFAULT  */
#line 225 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_Default; }
#line 1871 "xkbparse.c"
    break;

  case 26: /* Flag: HIDDEN  */
#line 226 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_Hidden; }
#line 1877 "xkbparse.c"
    break;

  case 27: /* Flag: ALPHANUMERIC_KEYS  */
#line 227 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_AlphanumericKeys; }
#line 1883 "xkbparse.c"
    break;

  case 28: /* Flag: MODIFIER_KEYS  */
#line 228 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_ModifierKeys; }
#line 1889 "xkbparse.c"
    break;

  case 29: /* Flag: KEYPAD_KEYS  */
#line 229 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_KeypadKeys; }
#line 1895 "xkbparse.c"
    break;

  case 30: /* Flag: FUNCTION_KEYS  */
#line 230 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_FunctionKeys; }
#line 1901 "xkbparse.c"
    break;

  case 31: /* Flag: ALTERNATE_GROUP  */
#line 231 "xkbparse.y"
                                                { (yyval.uval)= XkbLC_AlternateGroup; }
#line 1907 "xkbparse.c"
    break;

  case 32: /* DeclList: DeclList Decl  */
#line 235 "xkbparse.y"
                        { (yyval.any)= AppendStmt((yyvsp[-1].any),(yyvsp[0].any)); }
#line 1913 "xkbparse.c"
    break;

  case 33: /* DeclList: %empty  */
#line 236 "xkbparse.y"
                        { (yyval.any)= NULL; }
#line 1919 "xkbparse.c"
    break;

  case 34: /* Decl: OptMergeMode VarDecl  */
#line 240 "xkbparse.y"
                        {
			    (yyvsp[0].var)->merge= StmtSetMerge(&(yyvsp[0].var)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].var)->common;
			}
#line 1928 "xkbparse.c"
    break;

  case 35: /* Decl: OptMergeMode VModDecl  */
#line 245 "xkbparse.y"
                        {
			    (yyvsp[0].vmod)->merge= StmtSetMerge(&(yyvsp[0].vmod)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].vmod)->common;
			}
#line 1937 "xkbparse.c"
    break;

  case 36: /* Decl: OptMergeMode InterpretDecl  */
#line 250 "xkbparse.y"
                        {
			    (yyvsp[0].interp)->merge= StmtSetMerge(&(yyvsp[0].interp)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].interp)->common;
			}
#line 1946 "xkbparse.c"
    break;

  case 37: /* Decl: OptMergeMode KeyNameDecl  */
#line 255 "xkbparse.y"
                        {
			    (yyvsp[0].keyName)->merge= StmtSetMerge(&(yyvsp[0].keyName)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].keyName)->common;
			}
#line 1955 "xkbparse.c"
    break;

  case 38: /* Decl: OptMergeMode KeyAliasDecl  */
#line 260 "xkbparse.y"
                        {
			    (yyvsp[0].keyAlias)->merge= StmtSetMerge(&(yyvsp[0].keyAlias)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].keyAlias)->common;
			}
#line 1964 "xkbparse.c"
    break;

  case 39: /* Decl: OptMergeMode KeyTypeDecl  */
#line 265 "xkbparse.y"
                        {
			    (yyvsp[0].keyType)->merge= StmtSetMerge(&(yyvsp[0].keyType)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].keyType)->common;
			}
#line 1973 "xkbparse.c"
    break;

  case 40: /* Decl: OptMergeMode SymbolsDecl  */
#line 270 "xkbparse.y"
                        {
			    (yyvsp[0].syms)->merge= StmtSetMerge(&(yyvsp[0].syms)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].syms)->common;
			}
#line 1982 "xkbparse.c"
    break;

  case 41: /* Decl: OptMergeMode ModMapDecl  */
#line 275 "xkbparse.y"
                        {
			    (yyvsp[0].modMask)->merge= StmtSetMerge(&(yyvsp[0].modMask)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].modMask)->common;
			}
#line 1991 "xkbparse.c"
    break;

  case 42: /* Decl: OptMergeMode GroupCompatDecl  */
#line 280 "xkbparse.y"
                        {
			    (yyvsp[0].groupCompat)->merge= StmtSetMerge(&(yyvsp[0].groupCompat)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].groupCompat)->common;
			}
#line 2000 "xkbparse.c"
    break;

  case 43: /* Decl: OptMergeMode IndicatorMapDecl  */
#line 285 "xkbparse.y"
                        {
			    (yyvsp[0].ledMap)->merge= StmtSetMerge(&(yyvsp[0].ledMap)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].ledMap)->common;
			}
#line 2009 "xkbparse.c"
    break;

  case 44: /* Decl: OptMergeMode IndicatorNameDecl  */
#line 290 "xkbparse.y"
                        {
			    (yyvsp[0].ledName)->merge= StmtSetMerge(&(yyvsp[0].ledName)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].ledName)->common;
			}
#line 2018 "xkbparse.c"
    break;

  case 45: /* Decl: OptMergeMode ShapeDecl  */
#line 295 "xkbparse.y"
                        {
			    (yyvsp[0].shape)->merge= StmtSetMerge(&(yyvsp[0].shape)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].shape)->common;
			}
#line 2027 "xkbparse.c"
    break;

  case 46: /* Decl: OptMergeMode SectionDecl  */
#line 300 "xkbparse.y"
                        {
			    (yyvsp[0].section)->merge= StmtSetMerge(&(yyvsp[0].section)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].section)->common;
			}
#line 2036 "xkbparse.c"
    break;

  case 47: /* Decl: OptMergeMode DoodadDecl  */
#line 305 "xkbparse.y"
                        {
			    (yyvsp[0].doodad)->merge= StmtSetMerge(&(yyvsp[0].doodad)->common,(yyvsp[-1].uval));
			    (yyval.any)= &(yyvsp[0].doodad)->common;
			}
#line 2045 "xkbparse.c"
    break;

  case 48: /* Decl: MergeMode STRING  */
#line 310 "xkbparse.y"
                        {
			    if ((yyvsp[-1].uval)==MergeAltForm) {
				yyerror("cannot use 'alternate' to include other maps");
				(yyval.any)= &IncludeCreate(scanBuf,MergeDefault)->common;
			    }
			    else {
				(yyval.any)= &IncludeCreate(scanBuf,(yyvsp[-1].uval))->common;
			    }
                        }
#line 2059 "xkbparse.c"
    break;

  case 49: /* VarDecl: Lhs EQUALS Expr SEMI  */
#line 322 "xkbparse.y"
                        { (yyval.var)= VarCreate((yyvsp[-3].expr),(yyvsp[-1].expr)); }
#line 2065 "xkbparse.c"
    break;

  case 50: /* VarDecl: Ident SEMI  */
#line 324 "xkbparse.y"
                        { (yyval.var)= BoolVarCreate((yyvsp[-1].sval),1); }
#line 2071 "xkbparse.c"
    break;

  case 51: /* VarDecl: EXCLAM Ident SEMI  */
#line 326 "xkbparse.y"
                        { (yyval.var)= BoolVarCreate((yyvsp[-1].sval),0); }
#line 2077 "xkbparse.c"
    break;

  case 52: /* KeyNameDecl: KeyName EQUALS Expr SEMI  */
#line 330 "xkbparse.y"
                        {
			    KeycodeDef *def;

			    def= KeycodeCreate((yyvsp[-3].str),(yyvsp[-1].expr));
			    if ((yyvsp[-3].str))
				free((yyvsp[-3].str));
			    (yyval.keyName)= def;
			}
#line 2090 "xkbparse.c"
    break;

  case 53: /* KeyAliasDecl: ALIAS KeyName EQUALS KeyName SEMI  */
#line 341 "xkbparse.y"
                        {
			    KeyAliasDef	*def;
			    def= KeyAliasCreate((yyvsp[-3].str),(yyvsp[-1].str));
			    if ((yyvsp[-3].str))	free((yyvsp[-3].str));	
			    if ((yyvsp[-1].str))	free((yyvsp[-1].str));	
			    (yyval.keyAlias)= def;
			}
#line 2102 "xkbparse.c"
    break;

  case 54: /* VModDecl: VIRTUAL_MODS VModDefList SEMI  */
#line 351 "xkbparse.y"
                        { (yyval.vmod)= (yyvsp[-1].vmod); }
#line 2108 "xkbparse.c"
    break;

  case 55: /* VModDefList: VModDefList COMMA VModDef  */
#line 355 "xkbparse.y"
                        { (yyval.vmod)= (VModDef *)AppendStmt(&(yyvsp[-2].vmod)->common,&(yyvsp[0].vmod)->common); }
#line 2114 "xkbparse.c"
    break;

  case 56: /* VModDefList: VModDef  */
#line 357 "xkbparse.y"
                        { (yyval.vmod)= (yyvsp[0].vmod); }
#line 2120 "xkbparse.c"
    break;

  case 57: /* VModDef: Ident  */
#line 361 "xkbparse.y"
                        { (yyval.vmod)= VModCreate((yyvsp[0].sval),NULL); }
#line 2126 "xkbparse.c"
    break;

  case 58: /* VModDef: Ident EQUALS Expr  */
#line 363 "xkbparse.y"
                        { (yyval.vmod)= VModCreate((yyvsp[-2].sval),(yyvsp[0].expr)); }
#line 2132 "xkbparse.c"
    break;

  case 59: /* InterpretDecl: INTERPRET InterpretMatch OBRACE VarDeclList CBRACE SEMI  */
#line 369 "xkbparse.y"
                        {
			    (yyvsp[-4].interp)->def= (yyvsp[-2].var);
			    (yyval.interp)= (yyvsp[-4].interp);
			}
#line 2141 "xkbparse.c"
    break;

  case 60: /* InterpretMatch: KeySym PLUS Expr  */
#line 376 "xkbparse.y"
                        { (yyval.interp)= InterpCreate((yyvsp[-2].str), (yyvsp[0].expr)); }
#line 2147 "xkbparse.c"
    break;

  case 61: /* InterpretMatch: KeySym  */
#line 378 "xkbparse.y"
                        { (yyval.interp)= InterpCreate((yyvsp[0].str), NULL); }
#line 2153 "xkbparse.c"
    break;

  case 62: /* VarDeclList: VarDeclList VarDecl  */
#line 382 "xkbparse.y"
                        { (yyval.var)= (VarDef *)AppendStmt(&(yyvsp[-1].var)->common,&(yyvsp[0].var)->common); }
#line 2159 "xkbparse.c"
    break;

  case 63: /* VarDeclList: VarDecl  */
#line 384 "xkbparse.y"
                        { (yyval.var)= (yyvsp[0].var); }
#line 2165 "xkbparse.c"
    break;

  case 64: /* KeyTypeDecl: TYPE String OBRACE VarDeclList CBRACE SEMI  */
#line 390 "xkbparse.y"
                        { (yyval.keyType)= KeyTypeCreate((yyvsp[-4].sval),(yyvsp[-2].var)); }
#line 2171 "xkbparse.c"
    break;

  case 65: /* SymbolsDecl: KEY KeyName OBRACE SymbolsBody CBRACE SEMI  */
#line 396 "xkbparse.y"
                        { (yyval.syms)= SymbolsCreate((yyvsp[-4].str),(ExprDef *)(yyvsp[-2].var)); }
#line 2177 "xkbparse.c"
    break;

  case 66: /* SymbolsBody: SymbolsBody COMMA SymbolsVarDecl  */
#line 400 "xkbparse.y"
                        { (yyval.var)= (VarDef *)AppendStmt(&(yyvsp[-2].var)->common,&(yyvsp[0].var)->common); }
#line 2183 "xkbparse.c"
    break;

  case 67: /* SymbolsBody: SymbolsVarDecl  */
#line 402 "xkbparse.y"
                        { (yyval.var)= (yyvsp[0].var); }
#line 2189 "xkbparse.c"
    break;

  case 68: /* SymbolsBody: %empty  */
#line 403 "xkbparse.y"
                        { (yyval.var)= NULL; }
#line 2195 "xkbparse.c"
    break;

  case 69: /* SymbolsVarDecl: Lhs EQUALS Expr  */
#line 407 "xkbparse.y"
                        { (yyval.var)= VarCreate((yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2201 "xkbparse.c"
    break;

  case 70: /* SymbolsVarDecl: Lhs EQUALS ArrayInit  */
#line 409 "xkbparse.y"
                        { (yyval.var)= VarCreate((yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2207 "xkbparse.c"
    break;

  case 71: /* SymbolsVarDecl: Ident  */
#line 411 "xkbparse.y"
                        { (yyval.var)= BoolVarCreate((yyvsp[0].sval),1); }
#line 2213 "xkbparse.c"
    break;

  case 72: /* SymbolsVarDecl: EXCLAM Ident  */
#line 413 "xkbparse.y"
                        { (yyval.var)= BoolVarCreate((yyvsp[0].sval),0); }
#line 2219 "xkbparse.c"
    break;

  case 73: /* SymbolsVarDecl: ArrayInit  */
#line 415 "xkbparse.y"
                        { (yyval.var)= VarCreate(NULL,(yyvsp[0].expr)); }
#line 2225 "xkbparse.c"
    break;

  case 74: /* ArrayInit: OBRACKET OptKeySymList CBRACKET  */
#line 419 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[-1].expr); }
#line 2231 "xkbparse.c"
    break;

  case 75: /* ArrayInit: OBRACKET ActionList CBRACKET  */
#line 421 "xkbparse.y"
                        { (yyval.expr)= ExprCreateUnary(ExprActionList,TypeAction,(yyvsp[-1].expr)); }
#line 2237 "xkbparse.c"
    break;

  case 76: /* GroupCompatDecl: GROUP Integer EQUALS Expr SEMI  */
#line 425 "xkbparse.y"
                        { (yyval.groupCompat)= GroupCompatCreate((yyvsp[-3].ival),(yyvsp[-1].expr)); }
#line 2243 "xkbparse.c"
    break;

  case 77: /* ModMapDecl: MODIFIER_MAP Ident OBRACE ExprList CBRACE SEMI  */
#line 429 "xkbparse.y"
                        { (yyval.modMask)= ModMapCreate((yyvsp[-4].sval),(yyvsp[-2].expr)); }
#line 2249 "xkbparse.c"
    break;

  case 78: /* IndicatorMapDecl: INDICATOR String OBRACE VarDeclList CBRACE SEMI  */
#line 433 "xkbparse.y"
                        { (yyval.ledMap)= IndicatorMapCreate((yyvsp[-4].sval),(yyvsp[-2].var)); }
#line 2255 "xkbparse.c"
    break;

  case 79: /* IndicatorNameDecl: INDICATOR Integer EQUALS Expr SEMI  */
#line 437 "xkbparse.y"
                        { (yyval.ledName)= IndicatorNameCreate((yyvsp[-3].ival),(yyvsp[-1].expr),False); }
#line 2261 "xkbparse.c"
    break;

  case 80: /* IndicatorNameDecl: VIRTUAL INDICATOR Integer EQUALS Expr SEMI  */
#line 439 "xkbparse.y"
                        { (yyval.ledName)= IndicatorNameCreate((yyvsp[-3].ival),(yyvsp[-1].expr),True); }
#line 2267 "xkbparse.c"
    break;

  case 81: /* ShapeDecl: SHAPE String OBRACE OutlineList CBRACE SEMI  */
#line 443 "xkbparse.y"
                        { (yyval.shape)= ShapeDeclCreate((yyvsp[-4].sval),(OutlineDef *)&(yyvsp[-2].outline)->common); }
#line 2273 "xkbparse.c"
    break;

  case 82: /* ShapeDecl: SHAPE String OBRACE CoordList CBRACE SEMI  */
#line 445 "xkbparse.y"
                        {
			    OutlineDef *outlines;
			    outlines= OutlineCreate(None,(yyvsp[-2].expr));
			    (yyval.shape)= ShapeDeclCreate((yyvsp[-4].sval),outlines);
			}
#line 2283 "xkbparse.c"
    break;

  case 83: /* SectionDecl: SECTION String OBRACE SectionBody CBRACE SEMI  */
#line 453 "xkbparse.y"
                        { (yyval.section)= SectionDeclCreate((yyvsp[-4].sval),(yyvsp[-2].row)); }
#line 2289 "xkbparse.c"
    break;

  case 84: /* SectionBody: SectionBody SectionBodyItem  */
#line 457 "xkbparse.y"
                        { (yyval.row)=(RowDef *)AppendStmt(&(yyvsp[-1].row)->common,&(yyvsp[0].row)->common);}
#line 2295 "xkbparse.c"
    break;

  case 85: /* SectionBody: SectionBodyItem  */
#line 459 "xkbparse.y"
                        { (yyval.row)= (yyvsp[0].row); }
#line 2301 "xkbparse.c"
    break;

  case 86: /* SectionBodyItem: ROW OBRACE RowBody CBRACE SEMI  */
#line 463 "xkbparse.y"
                        { (yyval.row)= RowDeclCreate((yyvsp[-2].key)); }
#line 2307 "xkbparse.c"
    break;

  case 87: /* SectionBodyItem: VarDecl  */
#line 465 "xkbparse.y"
                        { (yyval.row)= (RowDef *)(yyvsp[0].var); }
#line 2313 "xkbparse.c"
    break;

  case 88: /* SectionBodyItem: DoodadDecl  */
#line 467 "xkbparse.y"
                        { (yyval.row)= (RowDef *)(yyvsp[0].doodad); }
#line 2319 "xkbparse.c"
    break;

  case 89: /* SectionBodyItem: IndicatorMapDecl  */
#line 469 "xkbparse.y"
                        { (yyval.row)= (RowDef *)(yyvsp[0].ledMap); }
#line 2325 "xkbparse.c"
    break;

  case 90: /* SectionBodyItem: OverlayDecl  */
#line 471 "xkbparse.y"
                        { (yyval.row)= (RowDef *)(yyvsp[0].overlay); }
#line 2331 "xkbparse.c"
    break;

  case 91: /* RowBody: RowBody RowBodyItem  */
#line 475 "xkbparse.y"
                        { (yyval.key)=(KeyDef *)AppendStmt(&(yyvsp[-1].key)->common,&(yyvsp[0].key)->common);}
#line 2337 "xkbparse.c"
    break;

  case 92: /* RowBody: RowBodyItem  */
#line 477 "xkbparse.y"
                        { (yyval.key)= (yyvsp[0].key); }
#line 2343 "xkbparse.c"
    break;

  case 93: /* RowBodyItem: KEYS OBRACE Keys CBRACE SEMI  */
#line 481 "xkbparse.y"
                        { (yyval.key)= (yyvsp[-2].key); }
#line 2349 "xkbparse.c"
    break;

  case 94: /* RowBodyItem: VarDecl  */
#line 483 "xkbparse.y"
                        { (yyval.key)= (KeyDef *)(yyvsp[0].var); }
#line 2355 "xkbparse.c"
    break;

  case 95: /* Keys: Keys COMMA Key  */
#line 487 "xkbparse.y"
                        { (yyval.key)=(KeyDef *)AppendStmt(&(yyvsp[-2].key)->common,&(yyvsp[0].key)->common);}
#line 2361 "xkbparse.c"
    break;

  case 96: /* Keys: Key  */
#line 489 "xkbparse.y"
                        { (yyval.key)= (yyvsp[0].key); }
#line 2367 "xkbparse.c"
    break;

  case 97: /* Key: KeyName  */
#line 493 "xkbparse.y"
                        { (yyval.key)= KeyDeclCreate((yyvsp[0].str),NULL); }
#line 2373 "xkbparse.c"
    break;

  case 98: /* Key: OBRACE ExprList CBRACE  */
#line 495 "xkbparse.y"
                        { (yyval.key)= KeyDeclCreate(NULL,(yyvsp[-1].expr)); }
#line 2379 "xkbparse.c"
    break;

  case 99: /* OverlayDecl: OVERLAY String OBRACE OverlayKeyList CBRACE SEMI  */
#line 499 "xkbparse.y"
                        { (yyval.overlay)= OverlayDeclCreate((yyvsp[-4].sval),(yyvsp[-2].olKey)); }
#line 2385 "xkbparse.c"
    break;

  case 100: /* OverlayKeyList: OverlayKeyList COMMA OverlayKey  */
#line 503 "xkbparse.y"
                        {
			    (yyval.olKey)= (OverlayKeyDef *)
				AppendStmt(&(yyvsp[-2].olKey)->common,&(yyvsp[0].olKey)->common);
			}
#line 2394 "xkbparse.c"
    break;

  case 101: /* OverlayKeyList: OverlayKey  */
#line 508 "xkbparse.y"
                        { (yyval.olKey)= (yyvsp[0].olKey); }
#line 2400 "xkbparse.c"
    break;

  case 102: /* OverlayKey: KeyName EQUALS KeyName  */
#line 512 "xkbparse.y"
                        { (yyval.olKey)= OverlayKeyCreate((yyvsp[-2].str),(yyvsp[0].str)); }
#line 2406 "xkbparse.c"
    break;

  case 103: /* OutlineList: OutlineList COMMA OutlineInList  */
#line 516 "xkbparse.y"
                        { (yyval.outline)=(OutlineDef *)AppendStmt(&(yyvsp[-2].outline)->common,&(yyvsp[0].outline)->common);}
#line 2412 "xkbparse.c"
    break;

  case 104: /* OutlineList: OutlineInList  */
#line 518 "xkbparse.y"
                        { (yyval.outline)= (yyvsp[0].outline); }
#line 2418 "xkbparse.c"
    break;

  case 105: /* OutlineInList: OBRACE CoordList CBRACE  */
#line 522 "xkbparse.y"
                        { (yyval.outline)= OutlineCreate(None,(yyvsp[-1].expr)); }
#line 2424 "xkbparse.c"
    break;

  case 106: /* OutlineInList: Ident EQUALS OBRACE CoordList CBRACE  */
#line 524 "xkbparse.y"
                        { (yyval.outline)= OutlineCreate((yyvsp[-4].sval),(yyvsp[-1].expr)); }
#line 2430 "xkbparse.c"
    break;

  case 107: /* OutlineInList: Ident EQUALS Expr  */
#line 526 "xkbparse.y"
                        { (yyval.outline)= OutlineCreate((yyvsp[-2].sval),(yyvsp[0].expr)); }
#line 2436 "xkbparse.c"
    break;

  case 108: /* CoordList: CoordList COMMA Coord  */
#line 530 "xkbparse.y"
                        { (yyval.expr)= (ExprDef *)AppendStmt(&(yyvsp[-2].expr)->common,&(yyvsp[0].expr)->common); }
#line 2442 "xkbparse.c"
    break;

  case 109: /* CoordList: Coord  */
#line 532 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[0].expr); }
#line 2448 "xkbparse.c"
    break;

  case 110: /* Coord: OBRACKET SignedNumber COMMA SignedNumber CBRACKET  */
#line 536 "xkbparse.y"
                        {
			    ExprDef *expr;
			    expr= ExprCreate(ExprCoord,TypeUnknown);
			    expr->value.coord.x= (yyvsp[-3].ival);
			    expr->value.coord.y= (yyvsp[-1].ival);
			    (yyval.expr)= expr;
			}
#line 2460 "xkbparse.c"
    break;

  case 111: /* DoodadDecl: DoodadType String OBRACE VarDeclList CBRACE SEMI  */
#line 546 "xkbparse.y"
                        { (yyval.doodad)= DoodadCreate((yyvsp[-5].uval),(yyvsp[-4].sval),(yyvsp[-2].var)); }
#line 2466 "xkbparse.c"
    break;

  case 112: /* DoodadType: TEXT  */
#line 549 "xkbparse.y"
                                                { (yyval.uval)= XkbTextDoodad; }
#line 2472 "xkbparse.c"
    break;

  case 113: /* DoodadType: OUTLINE  */
#line 550 "xkbparse.y"
                                                { (yyval.uval)= XkbOutlineDoodad; }
#line 2478 "xkbparse.c"
    break;

  case 114: /* DoodadType: SOLID  */
#line 551 "xkbparse.y"
                                                { (yyval.uval)= XkbSolidDoodad; }
#line 2484 "xkbparse.c"
    break;

  case 115: /* DoodadType: LOGO  */
#line 552 "xkbparse.y"
                                                { (yyval.uval)= XkbLogoDoodad; }
#line 2490 "xkbparse.c"
    break;

  case 116: /* FieldSpec: Ident  */
#line 555 "xkbparse.y"
                                                { (yyval.sval)= (yyvsp[0].sval); }
#line 2496 "xkbparse.c"
    break;

  case 117: /* FieldSpec: Element  */
#line 556 "xkbparse.y"
                                                { (yyval.sval)= (yyvsp[0].sval); }
#line 2502 "xkbparse.c"
    break;

  case 118: /* Element: ACTION_TOK  */
#line 560 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"action",False); }
#line 2508 "xkbparse.c"
    break;

  case 119: /* Element: INTERPRET  */
#line 562 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"interpret",False); }
#line 2514 "xkbparse.c"
    break;

  case 120: /* Element: TYPE  */
#line 564 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"type",False); }
#line 2520 "xkbparse.c"
    break;

  case 121: /* Element: KEY  */
#line 566 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"key",False); }
#line 2526 "xkbparse.c"
    break;

  case 122: /* Element: GROUP  */
#line 568 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"group",False); }
#line 2532 "xkbparse.c"
    break;

  case 123: /* Element: MODIFIER_MAP  */
#line 570 "xkbparse.y"
                        {(yyval.sval)=XkbInternAtom(NULL,"modifier_map",False);}
#line 2538 "xkbparse.c"
    break;

  case 124: /* Element: INDICATOR  */
#line 572 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"indicator",False); }
#line 2544 "xkbparse.c"
    break;

  case 125: /* Element: SHAPE  */
#line 574 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"shape",False); }
#line 2550 "xkbparse.c"
    break;

  case 126: /* Element: ROW  */
#line 576 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"row",False); }
#line 2556 "xkbparse.c"
    break;

  case 127: /* Element: SECTION  */
#line 578 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"section",False); }
#line 2562 "xkbparse.c"
    break;

  case 128: /* Element: TEXT  */
#line 580 "xkbparse.y"
                        { (yyval.sval)= XkbInternAtom(NULL,"text",False); }
#line 2568 "xkbparse.c"
    break;

  case 129: /* OptMergeMode: MergeMode  */
#line 583 "xkbparse.y"
                                                { (yyval.uval)= (yyvsp[0].uval); }
#line 2574 "xkbparse.c"
    break;

  case 130: /* OptMergeMode: %empty  */
#line 584 "xkbparse.y"
                                                { (yyval.uval)= MergeDefault; }
#line 2580 "xkbparse.c"
    break;

  case 131: /* MergeMode: INCLUDE  */
#line 587 "xkbparse.y"
                                                { (yyval.uval)= MergeDefault; }
#line 2586 "xkbparse.c"
    break;

  case 132: /* MergeMode: AUGMENT  */
#line 588 "xkbparse.y"
                                                { (yyval.uval)= MergeAugment; }
#line 2592 "xkbparse.c"
    break;

  case 133: /* MergeMode: OVERRIDE  */
#line 589 "xkbparse.y"
                                                { (yyval.uval)= MergeOverride; }
#line 2598 "xkbparse.c"
    break;

  case 134: /* MergeMode: REPLACE  */
#line 590 "xkbparse.y"
                                                { (yyval.uval)= MergeReplace; }
#line 2604 "xkbparse.c"
    break;

  case 135: /* MergeMode: ALTERNATE  */
#line 591 "xkbparse.y"
                                                { (yyval.uval)= MergeAltForm; }
#line 2610 "xkbparse.c"
    break;

  case 136: /* OptExprList: ExprList  */
#line 594 "xkbparse.y"
                                                        { (yyval.expr)= (yyvsp[0].expr); }
#line 2616 "xkbparse.c"
    break;

  case 137: /* OptExprList: %empty  */
#line 595 "xkbparse.y"
                                                { (yyval.expr)= NULL; }
#line 2622 "xkbparse.c"
    break;

  case 138: /* ExprList: ExprList COMMA Expr  */
#line 599 "xkbparse.y"
                        { (yyval.expr)= (ExprDef *)AppendStmt(&(yyvsp[-2].expr)->common,&(yyvsp[0].expr)->common); }
#line 2628 "xkbparse.c"
    break;

  case 139: /* ExprList: Expr  */
#line 601 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[0].expr); }
#line 2634 "xkbparse.c"
    break;

  case 140: /* Expr: Expr DIVIDE Expr  */
#line 605 "xkbparse.y"
                        { (yyval.expr)= ExprCreateBinary(OpDivide,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2640 "xkbparse.c"
    break;

  case 141: /* Expr: Expr PLUS Expr  */
#line 607 "xkbparse.y"
                        { (yyval.expr)= ExprCreateBinary(OpAdd,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2646 "xkbparse.c"
    break;

  case 142: /* Expr: Expr MINUS Expr  */
#line 609 "xkbparse.y"
                        { (yyval.expr)= ExprCreateBinary(OpSubtract,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2652 "xkbparse.c"
    break;

  case 143: /* Expr: Expr TIMES Expr  */
#line 611 "xkbparse.y"
                        { (yyval.expr)= ExprCreateBinary(OpMultiply,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2658 "xkbparse.c"
    break;

  case 144: /* Expr: Lhs EQUALS Expr  */
#line 613 "xkbparse.y"
                        { (yyval.expr)= ExprCreateBinary(OpAssign,(yyvsp[-2].expr),(yyvsp[0].expr)); }
#line 2664 "xkbparse.c"
    break;

  case 145: /* Expr: Term  */
#line 615 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[0].expr); }
#line 2670 "xkbparse.c"
    break;

  case 146: /* Term: MINUS Term  */
#line 619 "xkbparse.y"
                        { (yyval.expr)= ExprCreateUnary(OpNegate,(yyvsp[0].expr)->type,(yyvsp[0].expr)); }
#line 2676 "xkbparse.c"
    break;

  case 147: /* Term: PLUS Term  */
#line 621 "xkbparse.y"
                        { (yyval.expr)= ExprCreateUnary(OpUnaryPlus,(yyvsp[0].expr)->type,(yyvsp[0].expr)); }
#line 2682 "xkbparse.c"
    break;

  case 148: /* Term: EXCLAM Term  */
#line 623 "xkbparse.y"
                        { (yyval.expr)= ExprCreateUnary(OpNot,TypeBoolean,(yyvsp[0].expr)); }
#line 2688 "xkbparse.c"
    break;

  case 149: /* Term: INVERT Term  */
#line 625 "xkbparse.y"
                        { (yyval.expr)= ExprCreateUnary(OpInvert,(yyvsp[0].expr)->type,(yyvsp[0].expr)); }
#line 2694 "xkbparse.c"
    break;

  case 150: /* Term: Lhs  */
#line 627 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[0].expr);  }
#line 2700 "xkbparse.c"
    break;

  case 151: /* Term: FieldSpec OPAREN OptExprList CPAREN  */
#line 629 "xkbparse.y"
                        { (yyval.expr)= ActionCreate((yyvsp[-3].sval),(yyvsp[-1].expr)); }
#line 2706 "xkbparse.c"
    break;

  case 152: /* Term: Terminal  */
#line 631 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[0].expr);  }
#line 2712 "xkbparse.c"
    break;

  case 153: /* Term: OPAREN Expr CPAREN  */
#line 633 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[-1].expr);  }
#line 2718 "xkbparse.c"
    break;

  case 154: /* ActionList: ActionList COMMA Action  */
#line 637 "xkbparse.y"
                        { (yyval.expr)= (ExprDef *)AppendStmt(&(yyvsp[-2].expr)->common,&(yyvsp[0].expr)->common); }
#line 2724 "xkbparse.c"
    break;

  case 155: /* ActionList: Action  */
#line 639 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[0].expr); }
#line 2730 "xkbparse.c"
    break;

  case 156: /* Action: FieldSpec OPAREN OptExprList CPAREN  */
#line 643 "xkbparse.y"
                        { (yyval.expr)= ActionCreate((yyvsp[-3].sval),(yyvsp[-1].expr)); }
#line 2736 "xkbparse.c"
    break;

  case 157: /* Lhs: FieldSpec  */
#line 647 "xkbparse.y"
                        {
			    ExprDef *expr;
                            expr= ExprCreate(ExprIdent,TypeUnknown);
                            expr->value.str= (yyvsp[0].sval);
                            (yyval.expr)= expr;
			}
#line 2747 "xkbparse.c"
    break;

  case 158: /* Lhs: FieldSpec DOT FieldSpec  */
#line 654 "xkbparse.y"
                        {
                            ExprDef *expr;
                            expr= ExprCreate(ExprFieldRef,TypeUnknown);
                            expr->value.field.element= (yyvsp[-2].sval);
                            expr->value.field.field= (yyvsp[0].sval);
                            (yyval.expr)= expr;
			}
#line 2759 "xkbparse.c"
    break;

  case 159: /* Lhs: FieldSpec OBRACKET Expr CBRACKET  */
#line 662 "xkbparse.y"
                        {
			    ExprDef *expr;
			    expr= ExprCreate(ExprArrayRef,TypeUnknown);
			    expr->value.array.element= None;
			    expr->value.array.field= (yyvsp[-3].sval);
			    expr->value.array.entry= (yyvsp[-1].expr);
			    (yyval.expr)= expr;
			}
#line 2772 "xkbparse.c"
    break;

  case 160: /* Lhs: FieldSpec DOT FieldSpec OBRACKET Expr CBRACKET  */
#line 671 "xkbparse.y"
                        {
			    ExprDef *expr;
			    expr= ExprCreate(ExprArrayRef,TypeUnknown);
			    expr->value.array.element= (yyvsp[-5].sval);
			    expr->value.array.field= (yyvsp[-3].sval);
			    expr->value.array.entry= (yyvsp[-1].expr);
			    (yyval.expr)= expr;
			}
#line 2785 "xkbparse.c"
    break;

  case 161: /* Terminal: String  */
#line 682 "xkbparse.y"
                        {
			    ExprDef *expr;
                            expr= ExprCreate(ExprValue,TypeString);
                            expr->value.str= (yyvsp[0].sval);
                            (yyval.expr)= expr;
			}
#line 2796 "xkbparse.c"
    break;

  case 162: /* Terminal: Integer  */
#line 689 "xkbparse.y"
                        {
			    ExprDef *expr;
                            expr= ExprCreate(ExprValue,TypeInt);
                            expr->value.ival= (yyvsp[0].ival);
                            (yyval.expr)= expr;
			}
#line 2807 "xkbparse.c"
    break;

  case 163: /* Terminal: Float  */
#line 696 "xkbparse.y"
                        {
			    ExprDef *expr;
			    expr= ExprCreate(ExprValue,TypeFloat);
			    expr->value.ival= (yyvsp[0].ival);
			    (yyval.expr)= expr;
			}
#line 2818 "xkbparse.c"
    break;

  case 164: /* Terminal: KeyName  */
#line 703 "xkbparse.y"
                        {
			    ExprDef *expr;
			    expr= ExprCreate(ExprValue,TypeKeyName);
			    memset(expr->value.keyName,0,5);
			    strncpy(expr->value.keyName,(yyvsp[0].str),4);
			    free((yyvsp[0].str));
			    (yyval.expr)= expr;
			}
#line 2831 "xkbparse.c"
    break;

  case 165: /* OptKeySymList: KeySymList  */
#line 713 "xkbparse.y"
                                                        { (yyval.expr)= (yyvsp[0].expr); }
#line 2837 "xkbparse.c"
    break;

  case 166: /* OptKeySymList: %empty  */
#line 714 "xkbparse.y"
                                                        { (yyval.expr)= NULL; }
#line 2843 "xkbparse.c"
    break;

  case 167: /* KeySymList: KeySymList COMMA KeySym  */
#line 718 "xkbparse.y"
                        { (yyval.expr)= AppendKeysymList((yyvsp[-2].expr),(yyvsp[0].str)); }
#line 2849 "xkbparse.c"
    break;

  case 168: /* KeySymList: KeySymList COMMA KeySyms  */
#line 720 "xkbparse.y"
                        { (yyval.expr)= AppendKeysymList((yyvsp[-2].expr),strdup("NoSymbol")); }
#line 2855 "xkbparse.c"
    break;

  case 169: /* KeySymList: KeySym  */
#line 722 "xkbparse.y"
                        { (yyval.expr)= CreateKeysymList((yyvsp[0].str)); }
#line 2861 "xkbparse.c"
    break;

  case 170: /* KeySymList: KeySyms  */
#line 724 "xkbparse.y"
                        { (yyval.expr)= CreateKeysymList(strdup("NoSymbol")); }
#line 2867 "xkbparse.c"
    break;

  case 171: /* KeySym: IDENT  */
#line 727 "xkbparse.y"
                                        { (yyval.str)= strdup(scanBuf); }
#line 2873 "xkbparse.c"
    break;

  case 172: /* KeySym: SECTION  */
#line 728 "xkbparse.y"
                                        { (yyval.str)= strdup("section"); }
#line 2879 "xkbparse.c"
    break;

  case 173: /* KeySym: Integer  */
#line 730 "xkbparse.y"
                        {
			    if ((yyvsp[0].ival)<10)	{ (yyval.str)= malloc(2); (yyval.str)[0]= '0' + (yyvsp[0].ival); (yyval.str)[1]= '\0'; }
			    else	{ (yyval.str)= malloc(19); snprintf((yyval.str), 19, "0x%x", (yyvsp[0].ival)); }
			}
#line 2888 "xkbparse.c"
    break;

  case 174: /* KeySyms: OBRACE KeySymList CBRACE  */
#line 737 "xkbparse.y"
                        { (yyval.expr)= (yyvsp[-1].expr); }
#line 2894 "xkbparse.c"
    break;

  case 175: /* SignedNumber: MINUS Number  */
#line 740 "xkbparse.y"
                                        { (yyval.ival)= -(yyvsp[0].ival); }
#line 2900 "xkbparse.c"
    break;

  case 176: /* SignedNumber: Number  */
#line 741 "xkbparse.y"
                                            { (yyval.ival)= (yyvsp[0].ival); }
#line 2906 "xkbparse.c"
    break;

  case 177: /* Number: FLOAT  */
#line 744 "xkbparse.y"
                                        { (yyval.ival)= scanInt; }
#line 2912 "xkbparse.c"
    break;

  case 178: /* Number: INTEGER  */
#line 745 "xkbparse.y"
                                        { (yyval.ival)= scanInt*XkbGeomPtsPerMM; }
#line 2918 "xkbparse.c"
    break;

  case 179: /* Float: FLOAT  */
#line 748 "xkbparse.y"
                                        { (yyval.ival)= scanInt; }
#line 2924 "xkbparse.c"
    break;

  case 180: /* Integer: INTEGER  */
#line 751 "xkbparse.y"
                                        { (yyval.ival)= scanInt; }
#line 2930 "xkbparse.c"
    break;

  case 181: /* KeyName: KEYNAME  */
#line 754 "xkbparse.y"
                                        { (yyval.str)= strdup(scanBuf); }
#line 2936 "xkbparse.c"
    break;

  case 182: /* Ident: IDENT  */
#line 757 "xkbparse.y"
                                { (yyval.sval)= XkbInternAtom(NULL,scanBuf,False); }
#line 2942 "xkbparse.c"
    break;

  case 183: /* Ident: DEFAULT  */
#line 758 "xkbparse.y"
                                { (yyval.sval)= XkbInternAtom(NULL,"default",False); }
#line 2948 "xkbparse.c"
    break;

  case 184: /* String: STRING  */
#line 761 "xkbparse.y"
                                { (yyval.sval)= XkbInternAtom(NULL,scanBuf,False); }
#line 2954 "xkbparse.c"
    break;

  case 185: /* OptMapName: MapName  */
#line 764 "xkbparse.y"
                                { (yyval.str)= (yyvsp[0].str); }
#line 2960 "xkbparse.c"
    break;

  case 186: /* OptMapName: %empty  */
#line 765 "xkbparse.y"
                                { (yyval.str)= NULL; }
#line 2966 "xkbparse.c"
    break;

  case 187: /* MapName: STRING  */
#line 768 "xkbparse.y"
                                { (yyval.str)= strdup(scanBuf); }
#line 2972 "xkbparse.c"
    break;


#line 2976 "xkbparse.c"

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
  ++yynerrs;

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
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
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

#line 770 "xkbparse.y"

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

