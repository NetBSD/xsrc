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
#line 67 "gram.y"


#include <stdio.h>
#include <ctype.h>
#include "twm.h"
#include "menus.h"
#include "list.h"
#include "util.h"
#include "screen.h"
#include "parse.h"
#include "add_window.h"
#include "icons.h"
#include <X11/Xos.h>
#include <X11/Xmu/CharSet.h>

static char empty[1];
static char *Action = empty;
static char *Name = empty;
static MenuRoot *root, *pull = NULL;

static MenuRoot *GetRoot(const char *name, const char *fore, const char *back);
static void GotButton(int butt, int func);
static void GotKey(char *key, int func);
static void GotTitleButton(char *bitmapname, int func, Bool rightside);
static Bool CheckWarpScreenArg(char *s);
static Bool CheckWarpRingArg(char *s);
static Bool CheckColormapArg(char *s);
static void RemoveDQuote(char *str);

static char *ptr;
static name_list **list;
static int cont = 0;
static int color;
int mods = 0;
unsigned int mods_used = (ShiftMask | ControlMask | Mod1Mask);

static void yyerror(const char *s);


#line 111 "gram.c"

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

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
#ifndef YY_YY_GRAM_H_INCLUDED
# define YY_YY_GRAM_H_INCLUDED
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
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    LB = 258,                      /* LB  */
    RB = 259,                      /* RB  */
    LP = 260,                      /* LP  */
    RP = 261,                      /* RP  */
    MENUS = 262,                   /* MENUS  */
    MENU = 263,                    /* MENU  */
    BUTTON = 264,                  /* BUTTON  */
    DEFAULT_FUNCTION = 265,        /* DEFAULT_FUNCTION  */
    PLUS = 266,                    /* PLUS  */
    MINUS = 267,                   /* MINUS  */
    ALL = 268,                     /* ALL  */
    OR = 269,                      /* OR  */
    CURSORS = 270,                 /* CURSORS  */
    PIXMAPS = 271,                 /* PIXMAPS  */
    ICONS = 272,                   /* ICONS  */
    COLOR = 273,                   /* COLOR  */
    SAVECOLOR = 274,               /* SAVECOLOR  */
    MONOCHROME = 275,              /* MONOCHROME  */
    FUNCTION = 276,                /* FUNCTION  */
    ICONMGR_SHOW = 277,            /* ICONMGR_SHOW  */
    ICONMGR = 278,                 /* ICONMGR  */
    WINDOW_FUNCTION = 279,         /* WINDOW_FUNCTION  */
    ZOOM = 280,                    /* ZOOM  */
    ICONMGRS = 281,                /* ICONMGRS  */
    ICONMGR_GEOMETRY = 282,        /* ICONMGR_GEOMETRY  */
    ICONMGR_NOSHOW = 283,          /* ICONMGR_NOSHOW  */
    MAKE_TITLE = 284,              /* MAKE_TITLE  */
    GRAYSCALE = 285,               /* GRAYSCALE  */
    ICONIFY_BY_UNMAPPING = 286,    /* ICONIFY_BY_UNMAPPING  */
    DONT_ICONIFY_BY_UNMAPPING = 287, /* DONT_ICONIFY_BY_UNMAPPING  */
    NO_TITLE = 288,                /* NO_TITLE  */
    AUTO_RAISE = 289,              /* AUTO_RAISE  */
    NO_HILITE = 290,               /* NO_HILITE  */
    ICON_REGION = 291,             /* ICON_REGION  */
    META = 292,                    /* META  */
    SHIFT = 293,                   /* SHIFT  */
    LOCK = 294,                    /* LOCK  */
    CONTROL = 295,                 /* CONTROL  */
    WINDOW = 296,                  /* WINDOW  */
    TITLE = 297,                   /* TITLE  */
    ICON = 298,                    /* ICON  */
    ROOT = 299,                    /* ROOT  */
    FRAME = 300,                   /* FRAME  */
    COLON = 301,                   /* COLON  */
    EQUALS = 302,                  /* EQUALS  */
    SQUEEZE_TITLE = 303,           /* SQUEEZE_TITLE  */
    DONT_SQUEEZE_TITLE = 304,      /* DONT_SQUEEZE_TITLE  */
    START_ICONIFIED = 305,         /* START_ICONIFIED  */
    NO_TITLE_HILITE = 306,         /* NO_TITLE_HILITE  */
    TITLE_HILITE = 307,            /* TITLE_HILITE  */
    MOVE = 308,                    /* MOVE  */
    RESIZE = 309,                  /* RESIZE  */
    WAIT = 310,                    /* WAIT  */
    SELECT = 311,                  /* SELECT  */
    KILL = 312,                    /* KILL  */
    LEFT_TITLEBUTTON = 313,        /* LEFT_TITLEBUTTON  */
    RIGHT_TITLEBUTTON = 314,       /* RIGHT_TITLEBUTTON  */
    NUMBER = 315,                  /* NUMBER  */
    KEYWORD = 316,                 /* KEYWORD  */
    NKEYWORD = 317,                /* NKEYWORD  */
    CKEYWORD = 318,                /* CKEYWORD  */
    CLKEYWORD = 319,               /* CLKEYWORD  */
    FKEYWORD = 320,                /* FKEYWORD  */
    FSKEYWORD = 321,               /* FSKEYWORD  */
    SKEYWORD = 322,                /* SKEYWORD  */
    DKEYWORD = 323,                /* DKEYWORD  */
    JKEYWORD = 324,                /* JKEYWORD  */
    WINDOW_RING = 325,             /* WINDOW_RING  */
    WARP_CURSOR = 326,             /* WARP_CURSOR  */
    ERRORTOKEN = 327,              /* ERRORTOKEN  */
    NO_STACKMODE = 328,            /* NO_STACKMODE  */
    STRING = 329                   /* STRING  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define LB 258
#define RB 259
#define LP 260
#define RP 261
#define MENUS 262
#define MENU 263
#define BUTTON 264
#define DEFAULT_FUNCTION 265
#define PLUS 266
#define MINUS 267
#define ALL 268
#define OR 269
#define CURSORS 270
#define PIXMAPS 271
#define ICONS 272
#define COLOR 273
#define SAVECOLOR 274
#define MONOCHROME 275
#define FUNCTION 276
#define ICONMGR_SHOW 277
#define ICONMGR 278
#define WINDOW_FUNCTION 279
#define ZOOM 280
#define ICONMGRS 281
#define ICONMGR_GEOMETRY 282
#define ICONMGR_NOSHOW 283
#define MAKE_TITLE 284
#define GRAYSCALE 285
#define ICONIFY_BY_UNMAPPING 286
#define DONT_ICONIFY_BY_UNMAPPING 287
#define NO_TITLE 288
#define AUTO_RAISE 289
#define NO_HILITE 290
#define ICON_REGION 291
#define META 292
#define SHIFT 293
#define LOCK 294
#define CONTROL 295
#define WINDOW 296
#define TITLE 297
#define ICON 298
#define ROOT 299
#define FRAME 300
#define COLON 301
#define EQUALS 302
#define SQUEEZE_TITLE 303
#define DONT_SQUEEZE_TITLE 304
#define START_ICONIFIED 305
#define NO_TITLE_HILITE 306
#define TITLE_HILITE 307
#define MOVE 308
#define RESIZE 309
#define WAIT 310
#define SELECT 311
#define KILL 312
#define LEFT_TITLEBUTTON 313
#define RIGHT_TITLEBUTTON 314
#define NUMBER 315
#define KEYWORD 316
#define NKEYWORD 317
#define CKEYWORD 318
#define CLKEYWORD 319
#define FKEYWORD 320
#define FSKEYWORD 321
#define SKEYWORD 322
#define DKEYWORD 323
#define JKEYWORD 324
#define WINDOW_RING 325
#define WARP_CURSOR 326
#define ERRORTOKEN 327
#define NO_STACKMODE 328
#define STRING 329

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 108 "gram.y"

    int num;
    char *ptr;

#line 317 "gram.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_GRAM_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_LB = 3,                         /* LB  */
  YYSYMBOL_RB = 4,                         /* RB  */
  YYSYMBOL_LP = 5,                         /* LP  */
  YYSYMBOL_RP = 6,                         /* RP  */
  YYSYMBOL_MENUS = 7,                      /* MENUS  */
  YYSYMBOL_MENU = 8,                       /* MENU  */
  YYSYMBOL_BUTTON = 9,                     /* BUTTON  */
  YYSYMBOL_DEFAULT_FUNCTION = 10,          /* DEFAULT_FUNCTION  */
  YYSYMBOL_PLUS = 11,                      /* PLUS  */
  YYSYMBOL_MINUS = 12,                     /* MINUS  */
  YYSYMBOL_ALL = 13,                       /* ALL  */
  YYSYMBOL_OR = 14,                        /* OR  */
  YYSYMBOL_CURSORS = 15,                   /* CURSORS  */
  YYSYMBOL_PIXMAPS = 16,                   /* PIXMAPS  */
  YYSYMBOL_ICONS = 17,                     /* ICONS  */
  YYSYMBOL_COLOR = 18,                     /* COLOR  */
  YYSYMBOL_SAVECOLOR = 19,                 /* SAVECOLOR  */
  YYSYMBOL_MONOCHROME = 20,                /* MONOCHROME  */
  YYSYMBOL_FUNCTION = 21,                  /* FUNCTION  */
  YYSYMBOL_ICONMGR_SHOW = 22,              /* ICONMGR_SHOW  */
  YYSYMBOL_ICONMGR = 23,                   /* ICONMGR  */
  YYSYMBOL_WINDOW_FUNCTION = 24,           /* WINDOW_FUNCTION  */
  YYSYMBOL_ZOOM = 25,                      /* ZOOM  */
  YYSYMBOL_ICONMGRS = 26,                  /* ICONMGRS  */
  YYSYMBOL_ICONMGR_GEOMETRY = 27,          /* ICONMGR_GEOMETRY  */
  YYSYMBOL_ICONMGR_NOSHOW = 28,            /* ICONMGR_NOSHOW  */
  YYSYMBOL_MAKE_TITLE = 29,                /* MAKE_TITLE  */
  YYSYMBOL_GRAYSCALE = 30,                 /* GRAYSCALE  */
  YYSYMBOL_ICONIFY_BY_UNMAPPING = 31,      /* ICONIFY_BY_UNMAPPING  */
  YYSYMBOL_DONT_ICONIFY_BY_UNMAPPING = 32, /* DONT_ICONIFY_BY_UNMAPPING  */
  YYSYMBOL_NO_TITLE = 33,                  /* NO_TITLE  */
  YYSYMBOL_AUTO_RAISE = 34,                /* AUTO_RAISE  */
  YYSYMBOL_NO_HILITE = 35,                 /* NO_HILITE  */
  YYSYMBOL_ICON_REGION = 36,               /* ICON_REGION  */
  YYSYMBOL_META = 37,                      /* META  */
  YYSYMBOL_SHIFT = 38,                     /* SHIFT  */
  YYSYMBOL_LOCK = 39,                      /* LOCK  */
  YYSYMBOL_CONTROL = 40,                   /* CONTROL  */
  YYSYMBOL_WINDOW = 41,                    /* WINDOW  */
  YYSYMBOL_TITLE = 42,                     /* TITLE  */
  YYSYMBOL_ICON = 43,                      /* ICON  */
  YYSYMBOL_ROOT = 44,                      /* ROOT  */
  YYSYMBOL_FRAME = 45,                     /* FRAME  */
  YYSYMBOL_COLON = 46,                     /* COLON  */
  YYSYMBOL_EQUALS = 47,                    /* EQUALS  */
  YYSYMBOL_SQUEEZE_TITLE = 48,             /* SQUEEZE_TITLE  */
  YYSYMBOL_DONT_SQUEEZE_TITLE = 49,        /* DONT_SQUEEZE_TITLE  */
  YYSYMBOL_START_ICONIFIED = 50,           /* START_ICONIFIED  */
  YYSYMBOL_NO_TITLE_HILITE = 51,           /* NO_TITLE_HILITE  */
  YYSYMBOL_TITLE_HILITE = 52,              /* TITLE_HILITE  */
  YYSYMBOL_MOVE = 53,                      /* MOVE  */
  YYSYMBOL_RESIZE = 54,                    /* RESIZE  */
  YYSYMBOL_WAIT = 55,                      /* WAIT  */
  YYSYMBOL_SELECT = 56,                    /* SELECT  */
  YYSYMBOL_KILL = 57,                      /* KILL  */
  YYSYMBOL_LEFT_TITLEBUTTON = 58,          /* LEFT_TITLEBUTTON  */
  YYSYMBOL_RIGHT_TITLEBUTTON = 59,         /* RIGHT_TITLEBUTTON  */
  YYSYMBOL_NUMBER = 60,                    /* NUMBER  */
  YYSYMBOL_KEYWORD = 61,                   /* KEYWORD  */
  YYSYMBOL_NKEYWORD = 62,                  /* NKEYWORD  */
  YYSYMBOL_CKEYWORD = 63,                  /* CKEYWORD  */
  YYSYMBOL_CLKEYWORD = 64,                 /* CLKEYWORD  */
  YYSYMBOL_FKEYWORD = 65,                  /* FKEYWORD  */
  YYSYMBOL_FSKEYWORD = 66,                 /* FSKEYWORD  */
  YYSYMBOL_SKEYWORD = 67,                  /* SKEYWORD  */
  YYSYMBOL_DKEYWORD = 68,                  /* DKEYWORD  */
  YYSYMBOL_JKEYWORD = 69,                  /* JKEYWORD  */
  YYSYMBOL_WINDOW_RING = 70,               /* WINDOW_RING  */
  YYSYMBOL_WARP_CURSOR = 71,               /* WARP_CURSOR  */
  YYSYMBOL_ERRORTOKEN = 72,                /* ERRORTOKEN  */
  YYSYMBOL_NO_STACKMODE = 73,              /* NO_STACKMODE  */
  YYSYMBOL_STRING = 74,                    /* STRING  */
  YYSYMBOL_YYACCEPT = 75,                  /* $accept  */
  YYSYMBOL_twmrc = 76,                     /* twmrc  */
  YYSYMBOL_stmts = 77,                     /* stmts  */
  YYSYMBOL_stmt = 78,                      /* stmt  */
  YYSYMBOL_79_1 = 79,                      /* $@1  */
  YYSYMBOL_80_2 = 80,                      /* $@2  */
  YYSYMBOL_81_3 = 81,                      /* $@3  */
  YYSYMBOL_82_4 = 82,                      /* $@4  */
  YYSYMBOL_83_5 = 83,                      /* $@5  */
  YYSYMBOL_84_6 = 84,                      /* $@6  */
  YYSYMBOL_85_7 = 85,                      /* $@7  */
  YYSYMBOL_86_8 = 86,                      /* $@8  */
  YYSYMBOL_87_9 = 87,                      /* $@9  */
  YYSYMBOL_88_10 = 88,                     /* $@10  */
  YYSYMBOL_89_11 = 89,                     /* $@11  */
  YYSYMBOL_90_12 = 90,                     /* $@12  */
  YYSYMBOL_91_13 = 91,                     /* $@13  */
  YYSYMBOL_92_14 = 92,                     /* $@14  */
  YYSYMBOL_93_15 = 93,                     /* $@15  */
  YYSYMBOL_94_16 = 94,                     /* $@16  */
  YYSYMBOL_95_17 = 95,                     /* $@17  */
  YYSYMBOL_96_18 = 96,                     /* $@18  */
  YYSYMBOL_97_19 = 97,                     /* $@19  */
  YYSYMBOL_98_20 = 98,                     /* $@20  */
  YYSYMBOL_99_21 = 99,                     /* $@21  */
  YYSYMBOL_noarg = 100,                    /* noarg  */
  YYSYMBOL_sarg = 101,                     /* sarg  */
  YYSYMBOL_narg = 102,                     /* narg  */
  YYSYMBOL_full = 103,                     /* full  */
  YYSYMBOL_fullkey = 104,                  /* fullkey  */
  YYSYMBOL_keys = 105,                     /* keys  */
  YYSYMBOL_key = 106,                      /* key  */
  YYSYMBOL_contexts = 107,                 /* contexts  */
  YYSYMBOL_context = 108,                  /* context  */
  YYSYMBOL_contextkeys = 109,              /* contextkeys  */
  YYSYMBOL_contextkey = 110,               /* contextkey  */
  YYSYMBOL_pixmap_list = 111,              /* pixmap_list  */
  YYSYMBOL_pixmap_entries = 112,           /* pixmap_entries  */
  YYSYMBOL_pixmap_entry = 113,             /* pixmap_entry  */
  YYSYMBOL_cursor_list = 114,              /* cursor_list  */
  YYSYMBOL_cursor_entries = 115,           /* cursor_entries  */
  YYSYMBOL_cursor_entry = 116,             /* cursor_entry  */
  YYSYMBOL_color_list = 117,               /* color_list  */
  YYSYMBOL_color_entries = 118,            /* color_entries  */
  YYSYMBOL_color_entry = 119,              /* color_entry  */
  YYSYMBOL_120_22 = 120,                   /* $@22  */
  YYSYMBOL_save_color_list = 121,          /* save_color_list  */
  YYSYMBOL_s_color_entries = 122,          /* s_color_entries  */
  YYSYMBOL_s_color_entry = 123,            /* s_color_entry  */
  YYSYMBOL_win_color_list = 124,           /* win_color_list  */
  YYSYMBOL_win_color_entries = 125,        /* win_color_entries  */
  YYSYMBOL_win_color_entry = 126,          /* win_color_entry  */
  YYSYMBOL_squeeze = 127,                  /* squeeze  */
  YYSYMBOL_128_23 = 128,                   /* $@23  */
  YYSYMBOL_129_24 = 129,                   /* $@24  */
  YYSYMBOL_win_sqz_entries = 130,          /* win_sqz_entries  */
  YYSYMBOL_iconm_list = 131,               /* iconm_list  */
  YYSYMBOL_iconm_entries = 132,            /* iconm_entries  */
  YYSYMBOL_iconm_entry = 133,              /* iconm_entry  */
  YYSYMBOL_win_list = 134,                 /* win_list  */
  YYSYMBOL_win_entries = 135,              /* win_entries  */
  YYSYMBOL_win_entry = 136,                /* win_entry  */
  YYSYMBOL_icon_list = 137,                /* icon_list  */
  YYSYMBOL_icon_entries = 138,             /* icon_entries  */
  YYSYMBOL_icon_entry = 139,               /* icon_entry  */
  YYSYMBOL_function = 140,                 /* function  */
  YYSYMBOL_function_entries = 141,         /* function_entries  */
  YYSYMBOL_function_entry = 142,           /* function_entry  */
  YYSYMBOL_menu = 143,                     /* menu  */
  YYSYMBOL_menu_entries = 144,             /* menu_entries  */
  YYSYMBOL_menu_entry = 145,               /* menu_entry  */
  YYSYMBOL_action = 146,                   /* action  */
  YYSYMBOL_signed_number = 147,            /* signed_number  */
  YYSYMBOL_button = 148,                   /* button  */
  YYSYMBOL_string = 149,                   /* string  */
  YYSYMBOL_number = 150                    /* number  */
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
#define YYFINAL  3
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   339

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  75
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  76
/* YYNRULES -- Number of rules.  */
#define YYNRULES  193
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  285

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   329


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
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   138,   138,   141,   142,   145,   146,   147,   148,   149,
     150,   152,   158,   161,   167,   169,   170,   171,   171,   173,
     175,   178,   181,   185,   201,   202,   203,   203,   205,   205,
     207,   208,   208,   210,   210,   212,   212,   214,   216,   216,
     218,   220,   220,   222,   224,   224,   226,   228,   228,   230,
     230,   232,   232,   234,   234,   237,   237,   239,   239,   241,
     241,   243,   243,   245,   245,   247,   249,   249,   251,   267,
     275,   275,   277,   279,   279,   284,   293,   302,   313,   316,
     319,   320,   323,   324,   325,   326,   327,   336,   339,   340,
     343,   344,   345,   346,   347,   348,   349,   350,   351,   354,
     355,   358,   359,   360,   361,   362,   363,   364,   365,   366,
     367,   371,   374,   375,   378,   382,   385,   386,   389,   391,
     393,   395,   397,   399,   401,   403,   405,   407,   409,   411,
     413,   415,   417,   419,   421,   423,   425,   427,   429,   431,
     435,   439,   440,   443,   451,   451,   461,   471,   474,   475,
     478,   479,   482,   485,   486,   489,   494,   497,   497,   502,
     503,   503,   507,   508,   516,   519,   520,   523,   528,   536,
     539,   540,   543,   548,   551,   552,   555,   558,   561,   562,
     565,   571,   574,   575,   578,   583,   591,   592,   631,   632,
     633,   636,   648,   653
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
  "\"end of file\"", "error", "\"invalid token\"", "LB", "RB", "LP", "RP",
  "MENUS", "MENU", "BUTTON", "DEFAULT_FUNCTION", "PLUS", "MINUS", "ALL",
  "OR", "CURSORS", "PIXMAPS", "ICONS", "COLOR", "SAVECOLOR", "MONOCHROME",
  "FUNCTION", "ICONMGR_SHOW", "ICONMGR", "WINDOW_FUNCTION", "ZOOM",
  "ICONMGRS", "ICONMGR_GEOMETRY", "ICONMGR_NOSHOW", "MAKE_TITLE",
  "GRAYSCALE", "ICONIFY_BY_UNMAPPING", "DONT_ICONIFY_BY_UNMAPPING",
  "NO_TITLE", "AUTO_RAISE", "NO_HILITE", "ICON_REGION", "META", "SHIFT",
  "LOCK", "CONTROL", "WINDOW", "TITLE", "ICON", "ROOT", "FRAME", "COLON",
  "EQUALS", "SQUEEZE_TITLE", "DONT_SQUEEZE_TITLE", "START_ICONIFIED",
  "NO_TITLE_HILITE", "TITLE_HILITE", "MOVE", "RESIZE", "WAIT", "SELECT",
  "KILL", "LEFT_TITLEBUTTON", "RIGHT_TITLEBUTTON", "NUMBER", "KEYWORD",
  "NKEYWORD", "CKEYWORD", "CLKEYWORD", "FKEYWORD", "FSKEYWORD", "SKEYWORD",
  "DKEYWORD", "JKEYWORD", "WINDOW_RING", "WARP_CURSOR", "ERRORTOKEN",
  "NO_STACKMODE", "STRING", "$accept", "twmrc", "stmts", "stmt", "$@1",
  "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9", "$@10", "$@11",
  "$@12", "$@13", "$@14", "$@15", "$@16", "$@17", "$@18", "$@19", "$@20",
  "$@21", "noarg", "sarg", "narg", "full", "fullkey", "keys", "key",
  "contexts", "context", "contextkeys", "contextkey", "pixmap_list",
  "pixmap_entries", "pixmap_entry", "cursor_list", "cursor_entries",
  "cursor_entry", "color_list", "color_entries", "color_entry", "$@22",
  "save_color_list", "s_color_entries", "s_color_entry", "win_color_list",
  "win_color_entries", "win_color_entry", "squeeze", "$@23", "$@24",
  "win_sqz_entries", "iconm_list", "iconm_entries", "iconm_entry",
  "win_list", "win_entries", "win_entry", "icon_list", "icon_entries",
  "icon_entry", "function", "function_entries", "function_entry", "menu",
  "menu_entries", "menu_entry", "action", "signed_number", "button",
  "string", "number", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-159)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-161)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -159,    15,   265,  -159,  -159,   -56,   -30,   -25,    28,    30,
    -159,  -159,    32,  -159,   -56,  -159,   -25,   -30,  -159,   -56,
      40,  -159,  -159,    41,  -159,    42,  -159,    44,   -56,    46,
      48,  -159,    49,   -56,   -56,  -159,   -30,   -56,  -159,    58,
      60,  -159,  -159,  -159,  -159,  -159,  -159,   -28,    20,    61,
    -159,  -159,  -159,   -56,  -159,  -159,  -159,  -159,  -159,    65,
      66,  -159,  -159,    66,  -159,    76,  -159,  -159,    78,   -30,
      76,    76,    66,    76,    76,    76,    76,    76,    -4,    83,
      76,    76,    76,    43,    45,  -159,  -159,    76,    76,    76,
    -159,  -159,  -159,  -159,  -159,  -159,   -56,    85,  -159,   107,
       8,  -159,  -159,  -159,  -159,    -2,  -159,    86,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,    27,  -159,  -159,  -159,  -159,   -25,   -25,  -159,  -159,
    -159,    59,   153,    55,  -159,  -159,  -159,   -56,   -56,   -56,
     -56,   -56,   -56,   -56,   -56,   -56,   -56,   -56,  -159,  -159,
     -56,  -159,     0,     7,  -159,  -159,  -159,  -159,  -159,  -159,
       1,     2,   -30,     3,  -159,  -159,  -159,   -30,  -159,  -159,
    -159,  -159,  -159,  -159,   -56,     4,   -56,   -56,   -56,   -56,
     -56,   -56,   -56,   -56,   -56,   -56,   -56,  -159,  -159,  -159,
     -56,  -159,   -56,   -56,  -159,    17,  -159,  -159,  -159,  -159,
    -159,   -56,   -30,  -159,    33,  -159,    80,    13,    98,  -159,
    -159,    19,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,   103,  -159,  -159,  -159,   -35,
    -159,     5,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,   -25,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,   -25,  -159,  -159,  -159,   -56,  -159,   104,   -30,
    -159,   -30,   -30,   -30,  -159,  -159,  -159,    85,    62,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,   -56,     6,   106,  -159,
    -159,   -56,   -25,  -159,  -159
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       3,     0,     0,     1,     5,     0,     0,     0,     0,     0,
      59,    61,     0,    66,     0,    33,     0,    14,    31,     0,
      30,    47,    63,    19,    26,    46,    51,    40,     0,   156,
     159,    49,    37,     0,     0,    75,     0,     0,    73,    72,
      43,   192,     4,     6,     7,     8,     9,     0,     0,    55,
     193,   191,   186,     0,    68,   116,    16,   112,    15,     0,
       0,   148,    65,     0,    57,     0,    69,    13,     0,    12,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    77,    76,     0,     0,     0,
      80,    25,    23,    22,    80,    24,     0,     0,   187,     0,
       0,   174,    60,   141,    62,     0,    67,     0,   170,    34,
     165,    32,    11,    29,    48,    64,    18,    27,    45,    52,
      39,     0,   162,   161,    50,    36,     0,     0,    74,    71,
      42,     0,     0,     0,   182,    56,   115,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   117,   111,
       0,   113,     0,     0,   147,   151,   149,   150,   178,    58,
       0,     0,     0,     0,    20,    21,    87,    82,    83,    84,
      85,    88,    81,    99,     0,     0,   135,   127,   125,   121,
     123,   119,   129,   131,   133,   137,   139,   114,   173,   175,
       0,   140,     0,     0,   142,     0,   169,   171,   172,   164,
     166,     0,     0,   158,     0,    86,     0,     0,     0,   181,
     183,     0,   134,   126,   124,   120,   122,   118,   128,   130,
     132,   136,   138,   176,   146,   143,   177,   179,   180,     0,
      10,     0,    97,    98,    95,    96,    90,    91,    92,    93,
      94,     0,    89,   108,   109,   106,   107,   101,   102,   103,
     104,   105,     0,   100,   110,    53,     0,   184,     0,     0,
     167,     0,     0,     0,   188,    78,    79,     0,     0,   153,
     145,   168,   189,   190,   163,    54,     0,     0,     0,   152,
     154,     0,     0,   155,   185
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
      16,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,   -50,  -159,  -159,  -159,  -159,  -159,  -159,  -159,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,   130,
    -159,  -159,  -159,  -159,  -159,  -159,  -159,  -159,  -158,  -159,
    -159,   -13,  -159,  -159,    -5,   -16
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,    42,    73,    74,    70,    68,    65,    82,
      77,    89,    75,    71,    81,    76,   267,    97,   107,    59,
      60,    72,    63,    88,    87,    43,    44,    45,    91,    95,
     131,   172,   206,   242,   207,   253,    58,   100,   151,    56,
      99,   148,   104,   153,   194,   258,    62,   105,   156,   270,
     277,   280,    46,    79,    80,   163,   111,   161,   200,   109,
     160,   197,   102,   152,   189,   159,   195,   227,   135,   175,
     210,    54,   263,    47,    48,    51
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      49,    67,   154,    66,   188,   196,   199,   203,   209,    64,
     279,   191,   149,   106,    69,     3,   261,   262,    41,    90,
      85,   226,   115,    78,   256,    50,   243,   244,    83,    84,
      50,    55,    86,    57,    92,    61,   245,    52,    53,    41,
      52,    53,    93,   -28,   -17,   -44,    41,   -38,    98,  -157,
     246,  -160,   -35,   112,   247,   248,   249,   250,   251,   252,
     150,   -70,   155,   -41,   121,    50,    96,    94,   101,   103,
     192,   193,    41,   166,    41,    41,    41,    41,    41,   108,
      41,   110,    52,    53,    52,    53,   122,    41,   134,   158,
     126,   133,   127,   232,   233,   162,   167,   168,   169,   170,
     157,   174,   231,   234,   255,   171,  -144,   269,   276,   275,
     132,   136,   282,   164,   165,   137,   138,   235,     0,     0,
       0,   236,   237,   238,   239,   240,   241,     0,     0,     0,
     139,     0,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,   186,     0,     0,   187,   202,   190,     0,   140,
     141,   205,   142,     0,     0,   198,   201,     0,   204,     0,
     143,   144,   145,   146,   147,     0,     0,   166,     0,   208,
     211,   212,   213,   214,   215,   216,   217,   218,   219,   220,
     221,   222,   228,     0,     0,   223,   230,   224,   225,     0,
     167,   168,   169,   170,     0,     0,   229,     0,   257,   173,
     113,   114,   254,   116,   117,   118,   119,   120,     0,     0,
     123,   124,   125,   260,     0,   264,     0,   128,   129,   130,
       0,     0,     0,     0,   259,     0,     0,     0,   265,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   266,
       0,     0,     0,   271,     0,   272,   273,   274,     0,     0,
       0,   268,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    -2,     4,     0,     0,   284,
       0,   278,   281,     5,     6,     7,   283,     0,     0,     0,
       8,     9,    10,    11,    12,    13,    14,    15,     0,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,    26,
      27,    28,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    29,    30,    31,    32,     0,     0,     0,
       0,     0,     0,    33,    34,     0,    35,    36,     0,     0,
       0,     0,    37,     0,     0,    38,    39,     0,    40,    41
};

static const yytype_int16 yycheck[] =
{
       5,    17,     4,    16,     4,     4,     4,     4,     4,    14,
       4,     4,     4,    63,    19,     0,    11,    12,    74,    47,
      36,     4,    72,    28,     5,    60,    13,    14,    33,    34,
      60,     3,    37,     3,    47,     3,    23,    65,    66,    74,
      65,    66,    47,     3,     3,     3,    74,     3,    53,     3,
      37,     3,     3,    69,    41,    42,    43,    44,    45,    46,
      52,     3,    64,     3,    68,    60,     5,    47,     3,     3,
      63,    64,    74,    14,    74,    74,    74,    74,    74,     3,
      74,     3,    65,    66,    65,    66,     3,    74,     3,     3,
      47,    96,    47,    13,    14,    68,    37,    38,    39,    40,
     105,    46,    69,    23,     6,    46,     3,     3,    46,   267,
      94,     4,     6,   126,   127,     8,     9,    37,    -1,    -1,
      -1,    41,    42,    43,    44,    45,    46,    -1,    -1,    -1,
      23,    -1,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,    -1,    -1,   150,   162,   152,    -1,    42,
      43,   167,    45,    -1,    -1,   160,   161,    -1,   163,    -1,
      53,    54,    55,    56,    57,    -1,    -1,    14,    -1,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184,
     185,   186,   195,    -1,    -1,   190,   202,   192,   193,    -1,
      37,    38,    39,    40,    -1,    -1,   201,    -1,   211,    46,
      70,    71,   207,    73,    74,    75,    76,    77,    -1,    -1,
      80,    81,    82,   229,    -1,   231,    -1,    87,    88,    89,
      -1,    -1,    -1,    -1,   229,    -1,    -1,    -1,   241,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   252,
      -1,    -1,    -1,   259,    -1,   261,   262,   263,    -1,    -1,
      -1,   256,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     0,     1,    -1,    -1,   282,
      -1,   276,   277,     8,     9,    10,   281,    -1,    -1,    -1,
      15,    16,    17,    18,    19,    20,    21,    22,    -1,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    48,    49,    50,    51,    -1,    -1,    -1,
      -1,    -1,    -1,    58,    59,    -1,    61,    62,    -1,    -1,
      -1,    -1,    67,    -1,    -1,    70,    71,    -1,    73,    74
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    76,    77,     0,     1,     8,     9,    10,    15,    16,
      17,    18,    19,    20,    21,    22,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    48,
      49,    50,    51,    58,    59,    61,    62,    67,    70,    71,
      73,    74,    78,   100,   101,   102,   127,   148,   149,   149,
      60,   150,    65,    66,   146,     3,   114,     3,   111,    94,
      95,     3,   121,    97,   149,    83,   146,   150,    82,   149,
      81,    88,    96,    79,    80,    87,    90,    85,   149,   128,
     129,    89,    84,   149,   149,   150,   149,    99,    98,    86,
      47,   103,   146,   149,    47,   104,     5,    92,   149,   115,
     112,     3,   137,     3,   117,   122,   117,    93,     3,   134,
       3,   131,   150,   134,   134,   117,   134,   134,   134,   134,
     134,    68,     3,   134,   134,   134,    47,    47,   134,   134,
     134,   105,   105,   149,     3,   143,     4,     8,     9,    23,
      42,    43,    45,    53,    54,    55,    56,    57,   116,     4,
      52,   113,   138,   118,     4,    64,   123,   149,     3,   140,
     135,   132,    68,   130,   146,   146,    14,    37,    38,    39,
      40,    46,   106,    46,    46,   144,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,   149,   149,     4,   139,
     149,     4,    63,    64,   119,   141,     4,   136,   149,     4,
     133,   149,   150,     4,   149,   150,   107,   109,   149,     4,
     145,   149,   149,   149,   149,   149,   149,   149,   149,   149,
     149,   149,   149,   149,   149,   149,     4,   142,   146,   149,
     150,    69,    13,    14,    23,    37,    41,    42,    43,    44,
      45,    46,   108,    13,    14,    23,    37,    41,    42,    43,
      44,    45,    46,   110,   149,     6,     5,   146,   120,   149,
     150,    11,    12,   147,   150,   146,   146,    91,   149,     3,
     124,   150,   150,   150,   150,   143,    46,   125,   149,     4,
     126,   149,     6,   149,   146
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    75,    76,    77,    77,    78,    78,    78,    78,    78,
      78,    78,    78,    78,    78,    78,    78,    79,    78,    78,
      78,    78,    78,    78,    78,    78,    80,    78,    81,    78,
      78,    82,    78,    83,    78,    84,    78,    78,    85,    78,
      78,    86,    78,    78,    87,    78,    78,    88,    78,    89,
      78,    90,    78,    91,    78,    92,    78,    93,    78,    94,
      78,    95,    78,    96,    78,    78,    97,    78,    78,    78,
      98,    78,    78,    99,    78,   100,   101,   102,   103,   104,
     105,   105,   106,   106,   106,   106,   106,   106,   107,   107,
     108,   108,   108,   108,   108,   108,   108,   108,   108,   109,
     109,   110,   110,   110,   110,   110,   110,   110,   110,   110,
     110,   111,   112,   112,   113,   114,   115,   115,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     117,   118,   118,   119,   120,   119,   119,   121,   122,   122,
     123,   123,   124,   125,   125,   126,   127,   128,   127,   127,
     129,   127,   130,   130,   131,   132,   132,   133,   133,   134,
     135,   135,   136,   137,   138,   138,   139,   140,   141,   141,
     142,   143,   144,   144,   145,   145,   146,   146,   147,   147,
     147,   148,   149,   150
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     1,     0,     2,     1,     1,     1,     1,     1,
       6,     3,     2,     2,     1,     2,     2,     0,     3,     1,
       4,     4,     2,     2,     2,     2,     0,     3,     0,     3,
       1,     0,     3,     0,     3,     0,     3,     1,     0,     3,
       1,     0,     3,     1,     0,     3,     1,     0,     3,     0,
       3,     0,     3,     0,     9,     0,     4,     0,     4,     0,
       3,     0,     3,     0,     3,     2,     0,     3,     2,     2,
       0,     3,     1,     0,     3,     1,     2,     2,     6,     6,
       0,     2,     1,     1,     1,     1,     2,     1,     0,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     0,
       2,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     0,     2,     2,     3,     0,     2,     3,     2,
       3,     2,     3,     2,     3,     2,     3,     2,     3,     2,
       3,     2,     3,     2,     3,     2,     3,     2,     3,     2,
       3,     0,     2,     2,     0,     4,     2,     3,     0,     2,
       1,     1,     3,     0,     2,     2,     1,     0,     5,     1,
       0,     3,     0,     5,     3,     0,     2,     3,     4,     3,
       0,     2,     1,     3,     0,     2,     2,     3,     0,     2,
       1,     3,     0,     2,     2,     7,     1,     2,     1,     2,
       2,     2,     1,     1
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

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
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
  case 10: /* stmt: ICON_REGION string DKEYWORD DKEYWORD number number  */
#line 151 "gram.y"
                                        { AddIconRegion((yyvsp[-4].ptr), (yyvsp[-3].num), (yyvsp[-2].num), (yyvsp[-1].num), (yyvsp[0].num)); }
#line 1685 "gram.c"
    break;

  case 11: /* stmt: ICONMGR_GEOMETRY string number  */
#line 152 "gram.y"
                                                        { if (Scr->FirstTime)
                                                  {
                                                    Scr->iconmgr.geometry=(yyvsp[-1].ptr);
                                                    Scr->iconmgr.columns=(yyvsp[0].num);
                                                  }
                                                }
#line 1696 "gram.c"
    break;

  case 12: /* stmt: ICONMGR_GEOMETRY string  */
#line 158 "gram.y"
                                                { if (Scr->FirstTime)
                                                    Scr->iconmgr.geometry = (yyvsp[0].ptr);
                                                }
#line 1704 "gram.c"
    break;

  case 13: /* stmt: ZOOM number  */
#line 161 "gram.y"
                                        { if (Scr->FirstTime)
                                          {
                                                Scr->DoZoom = TRUE;
                                                Scr->ZoomCount = (short)(yyvsp[0].num);
                                          }
                                        }
#line 1715 "gram.c"
    break;

  case 14: /* stmt: ZOOM  */
#line 167 "gram.y"
                                        { if (Scr->FirstTime)
                                                Scr->DoZoom = TRUE; }
#line 1722 "gram.c"
    break;

  case 15: /* stmt: PIXMAPS pixmap_list  */
#line 169 "gram.y"
                                        {}
#line 1728 "gram.c"
    break;

  case 16: /* stmt: CURSORS cursor_list  */
#line 170 "gram.y"
                                        {}
#line 1734 "gram.c"
    break;

  case 17: /* $@1: %empty  */
#line 171 "gram.y"
                                        { list = &Scr->IconifyByUn; }
#line 1740 "gram.c"
    break;

  case 19: /* stmt: ICONIFY_BY_UNMAPPING  */
#line 173 "gram.y"
                                        { if (Scr->FirstTime)
                    Scr->IconifyByUnmapping = TRUE; }
#line 1747 "gram.c"
    break;

  case 20: /* stmt: LEFT_TITLEBUTTON string EQUALS action  */
#line 175 "gram.y"
                                                        {
                                          GotTitleButton ((yyvsp[-2].ptr), (yyvsp[0].num), False);
                                        }
#line 1755 "gram.c"
    break;

  case 21: /* stmt: RIGHT_TITLEBUTTON string EQUALS action  */
#line 178 "gram.y"
                                                         {
                                          GotTitleButton ((yyvsp[-2].ptr), (yyvsp[0].num), True);
                                        }
#line 1763 "gram.c"
    break;

  case 22: /* stmt: button string  */
#line 181 "gram.y"
                                        { root = GetRoot((yyvsp[0].ptr), NULLSTR, NULLSTR);
                                          Scr->Mouse[(yyvsp[-1].num)][C_ROOT][0].func = F_MENU;
                                          Scr->Mouse[(yyvsp[-1].num)][C_ROOT][0].menu = root;
                                        }
#line 1772 "gram.c"
    break;

  case 23: /* stmt: button action  */
#line 185 "gram.y"
                                        { Scr->Mouse[(yyvsp[-1].num)][C_ROOT][0].func = (yyvsp[0].num);
                                          if ((yyvsp[0].num) == F_MENU)
                                          {
                                            pull->prev = NULL;
                                            Scr->Mouse[(yyvsp[-1].num)][C_ROOT][0].menu = pull;
                                          }
                                          else
                                          {
                                            root = GetRoot(TWM_ROOT,NULLSTR,NULLSTR);
                                            Scr->Mouse[(yyvsp[-1].num)][C_ROOT][0].item =
                                                AddToMenu(root,"x",Action,
                                                          NULL,(yyvsp[0].num),NULLSTR,NULLSTR);
                                          }
                                          Action = empty;
                                          pull = NULL;
                                        }
#line 1793 "gram.c"
    break;

  case 24: /* stmt: string fullkey  */
#line 201 "gram.y"
                                        { GotKey((yyvsp[-1].ptr), (yyvsp[0].num)); }
#line 1799 "gram.c"
    break;

  case 25: /* stmt: button full  */
#line 202 "gram.y"
                                        { GotButton((yyvsp[-1].num), (yyvsp[0].num)); }
#line 1805 "gram.c"
    break;

  case 26: /* $@2: %empty  */
#line 203 "gram.y"
                                            { list = &Scr->DontIconify; }
#line 1811 "gram.c"
    break;

  case 28: /* $@3: %empty  */
#line 205 "gram.y"
                                        { list = &Scr->IconMgrNoShow; }
#line 1817 "gram.c"
    break;

  case 30: /* stmt: ICONMGR_NOSHOW  */
#line 207 "gram.y"
                                        { Scr->IconManagerDontShow = TRUE; }
#line 1823 "gram.c"
    break;

  case 31: /* $@4: %empty  */
#line 208 "gram.y"
                                        { list = &Scr->IconMgrs; }
#line 1829 "gram.c"
    break;

  case 33: /* $@5: %empty  */
#line 210 "gram.y"
                                        { list = &Scr->IconMgrShow; }
#line 1835 "gram.c"
    break;

  case 35: /* $@6: %empty  */
#line 212 "gram.y"
                                        { list = &Scr->NoTitleHighlight; }
#line 1841 "gram.c"
    break;

  case 37: /* stmt: NO_TITLE_HILITE  */
#line 214 "gram.y"
                                        { if (Scr->FirstTime)
                                                Scr->TitleHighlight = FALSE; }
#line 1848 "gram.c"
    break;

  case 38: /* $@7: %empty  */
#line 216 "gram.y"
                                        { list = &Scr->NoHighlight; }
#line 1854 "gram.c"
    break;

  case 40: /* stmt: NO_HILITE  */
#line 218 "gram.y"
                                        { if (Scr->FirstTime)
                                                Scr->Highlight = FALSE; }
#line 1861 "gram.c"
    break;

  case 41: /* $@8: %empty  */
#line 220 "gram.y"
                                        { list = &Scr->NoStackModeL; }
#line 1867 "gram.c"
    break;

  case 43: /* stmt: NO_STACKMODE  */
#line 222 "gram.y"
                                        { if (Scr->FirstTime)
                                                Scr->StackMode = FALSE; }
#line 1874 "gram.c"
    break;

  case 44: /* $@9: %empty  */
#line 224 "gram.y"
                                        { list = &Scr->NoTitle; }
#line 1880 "gram.c"
    break;

  case 46: /* stmt: NO_TITLE  */
#line 226 "gram.y"
                                        { if (Scr->FirstTime)
                                                Scr->NoTitlebar = TRUE; }
#line 1887 "gram.c"
    break;

  case 47: /* $@10: %empty  */
#line 228 "gram.y"
                                        { list = &Scr->MakeTitle; }
#line 1893 "gram.c"
    break;

  case 49: /* $@11: %empty  */
#line 230 "gram.y"
                                        { list = &Scr->StartIconified; }
#line 1899 "gram.c"
    break;

  case 51: /* $@12: %empty  */
#line 232 "gram.y"
                                        { list = &Scr->AutoRaise; }
#line 1905 "gram.c"
    break;

  case 53: /* $@13: %empty  */
#line 234 "gram.y"
                                                        {
                                        root = GetRoot((yyvsp[-5].ptr), (yyvsp[-3].ptr), (yyvsp[-1].ptr)); }
#line 1912 "gram.c"
    break;

  case 54: /* stmt: MENU string LP string COLON string RP $@13 menu  */
#line 236 "gram.y"
                                        { root->real_menu = TRUE;}
#line 1918 "gram.c"
    break;

  case 55: /* $@14: %empty  */
#line 237 "gram.y"
                                        { root = GetRoot((yyvsp[0].ptr), NULLSTR, NULLSTR); }
#line 1924 "gram.c"
    break;

  case 56: /* stmt: MENU string $@14 menu  */
#line 238 "gram.y"
                                        { root->real_menu = TRUE; }
#line 1930 "gram.c"
    break;

  case 57: /* $@15: %empty  */
#line 239 "gram.y"
                                        { root = GetRoot((yyvsp[0].ptr), NULLSTR, NULLSTR); }
#line 1936 "gram.c"
    break;

  case 59: /* $@16: %empty  */
#line 241 "gram.y"
                                        { list = &Scr->IconNames; }
#line 1942 "gram.c"
    break;

  case 61: /* $@17: %empty  */
#line 243 "gram.y"
                                        { color = COLOR; }
#line 1948 "gram.c"
    break;

  case 63: /* $@18: %empty  */
#line 245 "gram.y"
                                        { color = GRAYSCALE; }
#line 1954 "gram.c"
    break;

  case 66: /* $@19: %empty  */
#line 249 "gram.y"
                                        { color = MONOCHROME; }
#line 1960 "gram.c"
    break;

  case 68: /* stmt: DEFAULT_FUNCTION action  */
#line 251 "gram.y"
                                          { Scr->DefaultFunction.func = (yyvsp[0].num);
                                          if ((yyvsp[0].num) == F_MENU)
                                          {
                                            pull->prev = NULL;
                                            Scr->DefaultFunction.menu = pull;
                                          }
                                          else
                                          {
                                            root = GetRoot(TWM_ROOT,NULLSTR,NULLSTR);
                                            Scr->DefaultFunction.item =
                                                AddToMenu(root,"x",Action,
                                                          NULL,(yyvsp[0].num), NULLSTR, NULLSTR);
                                          }
                                          Action = empty;
                                          pull = NULL;
                                        }
#line 1981 "gram.c"
    break;

  case 69: /* stmt: WINDOW_FUNCTION action  */
#line 267 "gram.y"
                                         { Scr->WindowFunction.func = (yyvsp[0].num);
                                           root = GetRoot(TWM_ROOT,NULLSTR,NULLSTR);
                                           Scr->WindowFunction.item =
                                                AddToMenu(root,"x",Action,
                                                          NULL,(yyvsp[0].num), NULLSTR, NULLSTR);
                                           Action = empty;
                                           pull = NULL;
                                        }
#line 1994 "gram.c"
    break;

  case 70: /* $@20: %empty  */
#line 275 "gram.y"
                                        { list = &Scr->WarpCursorL; }
#line 2000 "gram.c"
    break;

  case 72: /* stmt: WARP_CURSOR  */
#line 277 "gram.y"
                                        { if (Scr->FirstTime)
                                            Scr->WarpCursor = TRUE; }
#line 2007 "gram.c"
    break;

  case 73: /* $@21: %empty  */
#line 279 "gram.y"
                                        { list = &Scr->WindowRingL; }
#line 2013 "gram.c"
    break;

  case 75: /* noarg: KEYWORD  */
#line 284 "gram.y"
                                        { if (!do_single_keyword ((yyvsp[0].num))) {
                                            parseWarning (
                                                 "unknown singleton keyword %d",
                                                 (yyvsp[0].num));
                                            ParseError = 1;
                                          }
                                        }
#line 2025 "gram.c"
    break;

  case 76: /* sarg: SKEYWORD string  */
#line 293 "gram.y"
                                        { if (!do_string_keyword ((yyvsp[-1].num), (yyvsp[0].ptr))) {
                                            parseWarning (
                                                 "unknown string keyword %d (value \"%s\")",
                                                 (yyvsp[-1].num), (yyvsp[0].ptr));
                                            ParseError = 1;
                                          }
                                        }
#line 2037 "gram.c"
    break;

  case 77: /* narg: NKEYWORD number  */
#line 302 "gram.y"
                                        { if (!do_number_keyword ((yyvsp[-1].num), (yyvsp[0].num))) {
                                            parseWarning (
                                                 "unknown numeric keyword %d (value %d)",
                                                 (yyvsp[-1].num), (yyvsp[0].num));
                                            ParseError = 1;
                                          }
                                        }
#line 2049 "gram.c"
    break;

  case 78: /* full: EQUALS keys COLON contexts COLON action  */
#line 313 "gram.y"
                                                           { (yyval.num) = (yyvsp[0].num); }
#line 2055 "gram.c"
    break;

  case 79: /* fullkey: EQUALS keys COLON contextkeys COLON action  */
#line 316 "gram.y"
                                                              { (yyval.num) = (yyvsp[0].num); }
#line 2061 "gram.c"
    break;

  case 82: /* key: META  */
#line 323 "gram.y"
                                        { mods |= Mod1Mask; }
#line 2067 "gram.c"
    break;

  case 83: /* key: SHIFT  */
#line 324 "gram.y"
                                        { mods |= ShiftMask; }
#line 2073 "gram.c"
    break;

  case 84: /* key: LOCK  */
#line 325 "gram.y"
                                        { mods |= LockMask; }
#line 2079 "gram.c"
    break;

  case 85: /* key: CONTROL  */
#line 326 "gram.y"
                                        { mods |= ControlMask; }
#line 2085 "gram.c"
    break;

  case 86: /* key: META number  */
#line 327 "gram.y"
                                        { if ((yyvsp[0].num) < 1 || (yyvsp[0].num) > 5) {
                                             parseWarning (
                                                  "bad modifier number (%d), must be 1-5",
                                                  (yyvsp[0].num));
                                             ParseError = 1;
                                          } else {
                                             mods |= (Mod1Mask << ((yyvsp[0].num) - 1));
                                          }
                                        }
#line 2099 "gram.c"
    break;

  case 87: /* key: OR  */
#line 336 "gram.y"
                                        { }
#line 2105 "gram.c"
    break;

  case 90: /* context: WINDOW  */
#line 343 "gram.y"
                                        { cont |= C_WINDOW_BIT; }
#line 2111 "gram.c"
    break;

  case 91: /* context: TITLE  */
#line 344 "gram.y"
                                        { cont |= C_TITLE_BIT; }
#line 2117 "gram.c"
    break;

  case 92: /* context: ICON  */
#line 345 "gram.y"
                                        { cont |= C_ICON_BIT; }
#line 2123 "gram.c"
    break;

  case 93: /* context: ROOT  */
#line 346 "gram.y"
                                        { cont |= C_ROOT_BIT; }
#line 2129 "gram.c"
    break;

  case 94: /* context: FRAME  */
#line 347 "gram.y"
                                        { cont |= C_FRAME_BIT; }
#line 2135 "gram.c"
    break;

  case 95: /* context: ICONMGR  */
#line 348 "gram.y"
                                        { cont |= C_ICONMGR_BIT; }
#line 2141 "gram.c"
    break;

  case 96: /* context: META  */
#line 349 "gram.y"
                                        { cont |= C_ICONMGR_BIT; }
#line 2147 "gram.c"
    break;

  case 97: /* context: ALL  */
#line 350 "gram.y"
                                        { cont |= C_ALL_BITS; }
#line 2153 "gram.c"
    break;

  case 98: /* context: OR  */
#line 351 "gram.y"
                                        {  }
#line 2159 "gram.c"
    break;

  case 101: /* contextkey: WINDOW  */
#line 358 "gram.y"
                                        { cont |= C_WINDOW_BIT; }
#line 2165 "gram.c"
    break;

  case 102: /* contextkey: TITLE  */
#line 359 "gram.y"
                                        { cont |= C_TITLE_BIT; }
#line 2171 "gram.c"
    break;

  case 103: /* contextkey: ICON  */
#line 360 "gram.y"
                                        { cont |= C_ICON_BIT; }
#line 2177 "gram.c"
    break;

  case 104: /* contextkey: ROOT  */
#line 361 "gram.y"
                                        { cont |= C_ROOT_BIT; }
#line 2183 "gram.c"
    break;

  case 105: /* contextkey: FRAME  */
#line 362 "gram.y"
                                        { cont |= C_FRAME_BIT; }
#line 2189 "gram.c"
    break;

  case 106: /* contextkey: ICONMGR  */
#line 363 "gram.y"
                                        { cont |= C_ICONMGR_BIT; }
#line 2195 "gram.c"
    break;

  case 107: /* contextkey: META  */
#line 364 "gram.y"
                                        { cont |= C_ICONMGR_BIT; }
#line 2201 "gram.c"
    break;

  case 108: /* contextkey: ALL  */
#line 365 "gram.y"
                                        { cont |= C_ALL_BITS; }
#line 2207 "gram.c"
    break;

  case 109: /* contextkey: OR  */
#line 366 "gram.y"
                                        { }
#line 2213 "gram.c"
    break;

  case 110: /* contextkey: string  */
#line 367 "gram.y"
                                        { Name = (yyvsp[0].ptr); cont |= C_NAME_BIT; }
#line 2219 "gram.c"
    break;

  case 114: /* pixmap_entry: TITLE_HILITE string  */
#line 378 "gram.y"
                                      { SetHighlightPixmap ((yyvsp[0].ptr)); }
#line 2225 "gram.c"
    break;

  case 118: /* cursor_entry: FRAME string string  */
#line 389 "gram.y"
                                      {
                        NewBitmapCursor(&Scr->FrameCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 2232 "gram.c"
    break;

  case 119: /* cursor_entry: FRAME string  */
#line 391 "gram.y"
                                {
                        NewFontCursor(&Scr->FrameCursor, (yyvsp[0].ptr)); }
#line 2239 "gram.c"
    break;

  case 120: /* cursor_entry: TITLE string string  */
#line 393 "gram.y"
                                      {
                        NewBitmapCursor(&Scr->TitleCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 2246 "gram.c"
    break;

  case 121: /* cursor_entry: TITLE string  */
#line 395 "gram.y"
                               {
                        NewFontCursor(&Scr->TitleCursor, (yyvsp[0].ptr)); }
#line 2253 "gram.c"
    break;

  case 122: /* cursor_entry: ICON string string  */
#line 397 "gram.y"
                                     {
                        NewBitmapCursor(&Scr->IconCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 2260 "gram.c"
    break;

  case 123: /* cursor_entry: ICON string  */
#line 399 "gram.y"
                              {
                        NewFontCursor(&Scr->IconCursor, (yyvsp[0].ptr)); }
#line 2267 "gram.c"
    break;

  case 124: /* cursor_entry: ICONMGR string string  */
#line 401 "gram.y"
                                        {
                        NewBitmapCursor(&Scr->IconMgrCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 2274 "gram.c"
    break;

  case 125: /* cursor_entry: ICONMGR string  */
#line 403 "gram.y"
                                 {
                        NewFontCursor(&Scr->IconMgrCursor, (yyvsp[0].ptr)); }
#line 2281 "gram.c"
    break;

  case 126: /* cursor_entry: BUTTON string string  */
#line 405 "gram.y"
                                       {
                        NewBitmapCursor(&Scr->ButtonCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 2288 "gram.c"
    break;

  case 127: /* cursor_entry: BUTTON string  */
#line 407 "gram.y"
                                {
                        NewFontCursor(&Scr->ButtonCursor, (yyvsp[0].ptr)); }
#line 2295 "gram.c"
    break;

  case 128: /* cursor_entry: MOVE string string  */
#line 409 "gram.y"
                                     {
                        NewBitmapCursor(&Scr->MoveCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 2302 "gram.c"
    break;

  case 129: /* cursor_entry: MOVE string  */
#line 411 "gram.y"
                              {
                        NewFontCursor(&Scr->MoveCursor, (yyvsp[0].ptr)); }
#line 2309 "gram.c"
    break;

  case 130: /* cursor_entry: RESIZE string string  */
#line 413 "gram.y"
                                       {
                        NewBitmapCursor(&Scr->ResizeCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 2316 "gram.c"
    break;

  case 131: /* cursor_entry: RESIZE string  */
#line 415 "gram.y"
                                {
                        NewFontCursor(&Scr->ResizeCursor, (yyvsp[0].ptr)); }
#line 2323 "gram.c"
    break;

  case 132: /* cursor_entry: WAIT string string  */
#line 417 "gram.y"
                                     {
                        NewBitmapCursor(&Scr->WaitCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 2330 "gram.c"
    break;

  case 133: /* cursor_entry: WAIT string  */
#line 419 "gram.y"
                              {
                        NewFontCursor(&Scr->WaitCursor, (yyvsp[0].ptr)); }
#line 2337 "gram.c"
    break;

  case 134: /* cursor_entry: MENU string string  */
#line 421 "gram.y"
                                     {
                        NewBitmapCursor(&Scr->MenuCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 2344 "gram.c"
    break;

  case 135: /* cursor_entry: MENU string  */
#line 423 "gram.y"
                              {
                        NewFontCursor(&Scr->MenuCursor, (yyvsp[0].ptr)); }
#line 2351 "gram.c"
    break;

  case 136: /* cursor_entry: SELECT string string  */
#line 425 "gram.y"
                                       {
                        NewBitmapCursor(&Scr->SelectCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 2358 "gram.c"
    break;

  case 137: /* cursor_entry: SELECT string  */
#line 427 "gram.y"
                                {
                        NewFontCursor(&Scr->SelectCursor, (yyvsp[0].ptr)); }
#line 2365 "gram.c"
    break;

  case 138: /* cursor_entry: KILL string string  */
#line 429 "gram.y"
                                     {
                        NewBitmapCursor(&Scr->DestroyCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 2372 "gram.c"
    break;

  case 139: /* cursor_entry: KILL string  */
#line 431 "gram.y"
                              {
                        NewFontCursor(&Scr->DestroyCursor, (yyvsp[0].ptr)); }
#line 2379 "gram.c"
    break;

  case 143: /* color_entry: CLKEYWORD string  */
#line 443 "gram.y"
                                        { if (!do_colorlist_keyword ((yyvsp[-1].num), color,
                                                                     (yyvsp[0].ptr))) {
                                            parseWarning (
                                                 "unhandled list color keyword %d (string \"%s\")",
                                                 (yyvsp[-1].num), (yyvsp[0].ptr));
                                            ParseError = 1;
                                          }
                                        }
#line 2392 "gram.c"
    break;

  case 144: /* $@22: %empty  */
#line 451 "gram.y"
                                        { list = do_colorlist_keyword((yyvsp[-1].num),color,
                                                                      (yyvsp[0].ptr));
                                          if (!list) {
                                            parseWarning (
                                                 "unhandled color list keyword %d (string \"%s\")",
                                                 (yyvsp[-1].num), (yyvsp[0].ptr));
                                            ParseError = 1;
                                          }
                                        }
#line 2406 "gram.c"
    break;

  case 145: /* color_entry: CLKEYWORD string $@22 win_color_list  */
#line 460 "gram.y"
                                        { /* No action */; }
#line 2412 "gram.c"
    break;

  case 146: /* color_entry: CKEYWORD string  */
#line 461 "gram.y"
                                        { if (!do_color_keyword ((yyvsp[-1].num), color,
                                                                 (yyvsp[0].ptr))) {
                                            parseWarning (
                                                 "unhandled color keyword %d (string \"%s\")",
                                                 (yyvsp[-1].num), (yyvsp[0].ptr));
                                            ParseError = 1;
                                          }
                                        }
#line 2425 "gram.c"
    break;

  case 150: /* s_color_entry: string  */
#line 478 "gram.y"
                                    { do_string_savecolor(color, (yyvsp[0].ptr)); }
#line 2431 "gram.c"
    break;

  case 151: /* s_color_entry: CLKEYWORD  */
#line 479 "gram.y"
                                    { do_var_savecolor((yyvsp[0].num)); }
#line 2437 "gram.c"
    break;

  case 155: /* win_color_entry: string string  */
#line 489 "gram.y"
                                        { if (Scr->FirstTime &&
                                              color == Scr->Monochrome)
                                            AddToList(list, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 2445 "gram.c"
    break;

  case 156: /* squeeze: SQUEEZE_TITLE  */
#line 494 "gram.y"
                                {
                                    if (HasShape) Scr->SqueezeTitle = TRUE;
                                }
#line 2453 "gram.c"
    break;

  case 157: /* $@23: %empty  */
#line 497 "gram.y"
                                { list = &Scr->SqueezeTitleL;
                                  if (HasShape && Scr->SqueezeTitle == -1)
                                    Scr->SqueezeTitle = TRUE;
                                }
#line 2462 "gram.c"
    break;

  case 159: /* squeeze: DONT_SQUEEZE_TITLE  */
#line 502 "gram.y"
                                     { Scr->SqueezeTitle = FALSE; }
#line 2468 "gram.c"
    break;

  case 160: /* $@24: %empty  */
#line 503 "gram.y"
                                     { list = &Scr->DontSqueezeTitleL; }
#line 2474 "gram.c"
    break;

  case 163: /* win_sqz_entries: win_sqz_entries string JKEYWORD signed_number number  */
#line 508 "gram.y"
                                                                        {
                                if (Scr->FirstTime) {
                                   do_squeeze_entry (list, (yyvsp[-3].ptr), (yyvsp[-2].num), (yyvsp[-1].num), (yyvsp[0].num));
                                }
                        }
#line 2484 "gram.c"
    break;

  case 167: /* iconm_entry: string string number  */
#line 523 "gram.y"
                                        { if (Scr->FirstTime)
                                            AddToList(list, (yyvsp[-2].ptr), (char *)
                                                AllocateIconManager((yyvsp[-2].ptr), NULLSTR,
                                                        (yyvsp[-1].ptr),(yyvsp[0].num)));
                                        }
#line 2494 "gram.c"
    break;

  case 168: /* iconm_entry: string string string number  */
#line 529 "gram.y"
                                        { if (Scr->FirstTime)
                                            AddToList(list, (yyvsp[-3].ptr), (char *)
                                                AllocateIconManager((yyvsp[-3].ptr),(yyvsp[-2].ptr),
                                                (yyvsp[-1].ptr), (yyvsp[0].num)));
                                        }
#line 2504 "gram.c"
    break;

  case 172: /* win_entry: string  */
#line 543 "gram.y"
                                        { if (Scr->FirstTime)
                                            AddToList(list, (yyvsp[0].ptr), NULL);
                                        }
#line 2512 "gram.c"
    break;

  case 176: /* icon_entry: string string  */
#line 555 "gram.y"
                                        { if (Scr->FirstTime) AddToList(list, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 2518 "gram.c"
    break;

  case 180: /* function_entry: action  */
#line 565 "gram.y"
                                        { AddToMenu(root, empty, Action, NULL, (yyvsp[0].num),
                                                NULLSTR, NULLSTR);
                                          Action = empty;
                                        }
#line 2527 "gram.c"
    break;

  case 184: /* menu_entry: string action  */
#line 578 "gram.y"
                                        { AddToMenu(root, (yyvsp[-1].ptr), Action, pull, (yyvsp[0].num),
                                                NULLSTR, NULLSTR);
                                          Action = empty;
                                          pull = NULL;
                                        }
#line 2537 "gram.c"
    break;

  case 185: /* menu_entry: string LP string COLON string RP action  */
#line 583 "gram.y"
                                                          {
                                          AddToMenu(root, (yyvsp[-6].ptr), Action, pull, (yyvsp[0].num),
                                                (yyvsp[-4].ptr), (yyvsp[-2].ptr));
                                          Action = empty;
                                          pull = NULL;
                                        }
#line 2548 "gram.c"
    break;

  case 186: /* action: FKEYWORD  */
#line 591 "gram.y"
                                { (yyval.num) = (yyvsp[0].num); }
#line 2554 "gram.c"
    break;

  case 187: /* action: FSKEYWORD string  */
#line 592 "gram.y"
                                   {
                                (yyval.num) = (yyvsp[-1].num);
                                Action = (yyvsp[0].ptr);
                                switch ((yyvsp[-1].num)) {
                                  case F_MENU:
                                    pull = GetRoot ((yyvsp[0].ptr), NULLSTR,NULLSTR);
                                    pull->prev = root;
                                    break;
                                  case F_WARPRING:
                                    if (!CheckWarpRingArg (Action)) {
                                        parseWarning (
                                             "ignoring invalid f.warptoring argument \"%s\"",
                                             Action);
                                        (yyval.num) = F_NOP;
                                    }
                                    break;
                                  case F_WARPTOSCREEN:
                                    if (!CheckWarpScreenArg (Action)) {
                                        parseWarning (
                                             "ignoring invalid f.warptoscreen argument \"%s\"",
                                             Action);
                                        (yyval.num) = F_NOP;
                                    }
                                    break;
                                  case F_COLORMAP:
                                    if (CheckColormapArg (Action)) {
                                        (yyval.num) = F_COLORMAP;
                                    } else {
                                        parseWarning (
                                             "ignoring invalid f.colormap argument \"%s\"",
                                             Action);
                                        (yyval.num) = F_NOP;
                                    }
                                    break;
                                } /* end switch */
                                   }
#line 2595 "gram.c"
    break;

  case 188: /* signed_number: number  */
#line 631 "gram.y"
                                        { (yyval.num) = (yyvsp[0].num); }
#line 2601 "gram.c"
    break;

  case 189: /* signed_number: PLUS number  */
#line 632 "gram.y"
                                        { (yyval.num) = (yyvsp[0].num); }
#line 2607 "gram.c"
    break;

  case 190: /* signed_number: MINUS number  */
#line 633 "gram.y"
                                        { (yyval.num) = -((yyvsp[0].num)); }
#line 2613 "gram.c"
    break;

  case 191: /* button: BUTTON number  */
#line 636 "gram.y"
                                        { (yyval.num) = (yyvsp[0].num);
                                          if ((yyvsp[0].num) == 0)
                                                yyerror("bad button 0");

                                          if ((yyvsp[0].num) > MAX_BUTTONS)
                                          {
                                                (yyval.num) = 0;
                                                yyerror("button number too large");
                                          }
                                        }
#line 2628 "gram.c"
    break;

  case 192: /* string: STRING  */
#line 648 "gram.y"
                                        { ptr = strdup((yyvsp[0].ptr));
                                          RemoveDQuote(ptr);
                                          (yyval.ptr) = ptr;
                                        }
#line 2637 "gram.c"
    break;

  case 193: /* number: NUMBER  */
#line 653 "gram.y"
                                        { (yyval.num) = (yyvsp[0].num); }
#line 2643 "gram.c"
    break;


#line 2647 "gram.c"

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

#line 656 "gram.y"


static void
yyerror(const char *s)
{
    parseWarning("error in input file:  %s", s ? s : "");
    ParseError = 1;
}

static void
RemoveDQuote(char *str)
{
    char *i, *o;
    int n;
    int count;

    for (i = str + 1, o = str; *i && *i != '\"'; o++) {
        if (*i == '\\') {
            switch (*++i) {
            case 'n':
                *o = '\n';
                i++;
                break;
            case 'b':
                *o = '\b';
                i++;
                break;
            case 'r':
                *o = '\r';
                i++;
                break;
            case 't':
                *o = '\t';
                i++;
                break;
            case 'f':
                *o = '\f';
                i++;
                break;
            case '0':
                if (*++i == 'x')
                    goto hex;
                else
                    --i;
                /* FALLTHRU */
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
                n = 0;
                count = 0;
                while (*i >= '0' && *i <= '7' && count < 3) {
                    n = (n << 3) + (*i++ - '0');
                    count++;
                }
                *o = (char) n;
                break;
              hex:
            case 'x':
                n = 0;
                count = 0;
                while (i++, count++ < 2) {
                    if (*i >= '0' && *i <= '9')
                        n = (n << 4) + (*i - '0');
                    else if (*i >= 'a' && *i <= 'f')
                        n = (n << 4) + (*i - 'a') + 10;
                    else if (*i >= 'A' && *i <= 'F')
                        n = (n << 4) + (*i - 'A') + 10;
                    else
                        break;
                }
                *o = (char) n;
                break;
            case '\n':
                i++;            /* punt */
                o--;            /* to account for o++ at end of loop */
                break;
            case '\"':
            case '\'':
            case '\\':
            default:
                *o = *i++;
                break;
            }
        }
        else
            *o = *i++;
    }
    *o = '\0';
}

static MenuRoot *
GetRoot(const char *name, const char *fore, const char *back)
{
    MenuRoot *tmp;

    tmp = FindMenuRoot(name);
    if (tmp == NULL)
        tmp = NewMenuRoot(name);

    if (fore) {
        int save;

        save = Scr->FirstTime;
        Scr->FirstTime = TRUE;
        GetColor(COLOR, &tmp->hi_fore, fore);
        GetColor(COLOR, &tmp->hi_back, back);
        Scr->FirstTime = (short) save;
    }

    return tmp;
}

static void
GotButton(int butt, int func)
{
    int i;

    for (i = 0; i < NUM_CONTEXTS; i++) {
        if ((cont & (1 << i)) == 0)
            continue;

        Scr->Mouse[butt][i][mods].func = func;
        if (func == F_MENU) {
            pull->prev = NULL;
            Scr->Mouse[butt][i][mods].menu = pull;
        }
        else {
            root = GetRoot(TWM_ROOT, NULLSTR, NULLSTR);
            Scr->Mouse[butt][i][mods].item = AddToMenu(root, "x", Action,
                                                       NULL, func, NULLSTR, NULLSTR);
        }
    }
    Action = empty;
    pull = NULL;
    cont = 0;
    mods_used |= (unsigned) mods;
    mods = 0;
}

static void
GotKey(char *key, int func)
{
    int i;

    for (i = 0; i < NUM_CONTEXTS; i++) {
        if ((cont & (1 << i)) == 0)
            continue;
        if (!AddFuncKey(key, i, mods, func, Name, Action))
            break;
    }

    Action = empty;
    pull = NULL;
    cont = 0;
    mods_used |= (unsigned) mods;
    mods = 0;
}

static void
GotTitleButton(char *bitmapname, int func, Bool rightside)
{
    if (!CreateTitleButton(bitmapname, func, Action, pull, rightside, True)) {
        parseWarning("unable to create %s titlebutton \"%s\"",
                     rightside ? "right" : "left", bitmapname);
    }
    Action = empty;
    pull = NULL;
}

static Bool
CheckWarpScreenArg(char *s)
{
    XmuCopyISOLatin1Lowered(s, s);

    if (strcmp(s, WARPSCREEN_NEXT) == 0 ||
        strcmp(s, WARPSCREEN_PREV) == 0 ||
        strcmp(s, WARPSCREEN_BACK) == 0)
        return True;

    for (; *s && isascii(*s) && isdigit(*s); s++);      /* SUPPRESS 530 */
    return (*s ? False : True);
}

static Bool
CheckWarpRingArg(char *s)
{
    XmuCopyISOLatin1Lowered(s, s);

    if (strcmp(s, WARPSCREEN_NEXT) == 0 ||
        strcmp(s, WARPSCREEN_PREV) == 0)
        return True;

    return False;
}

static Bool
CheckColormapArg(char *s)
{
    XmuCopyISOLatin1Lowered(s, s);

    if (strcmp(s, COLORMAP_NEXT) == 0 ||
        strcmp(s, COLORMAP_PREV) == 0 ||
        strcmp(s, COLORMAP_DEFAULT) == 0)
        return True;

    return False;
}
