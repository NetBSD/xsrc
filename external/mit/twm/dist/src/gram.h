/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

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

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

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

#line 220 "gram.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_GRAM_H_INCLUDED  */
