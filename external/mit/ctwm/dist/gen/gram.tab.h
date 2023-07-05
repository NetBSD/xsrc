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

#ifndef YY_YY_HOME_FULLERMD_WORK_CTWM_BZR_DEV_CTWM_MKTAR_4Y0T7B_CTWM_4_1_0_BUILD_GRAM_TAB_H_INCLUDED
# define YY_YY_HOME_FULLERMD_WORK_CTWM_BZR_DEV_CTWM_MKTAR_4Y0T7B_CTWM_4_1_0_BUILD_GRAM_TAB_H_INCLUDED
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
    ALTER = 279,                   /* ALTER  */
    WINDOW_FUNCTION = 280,         /* WINDOW_FUNCTION  */
    ZOOM = 281,                    /* ZOOM  */
    ICONMGRS = 282,                /* ICONMGRS  */
    ICONMGR_GEOMETRY = 283,        /* ICONMGR_GEOMETRY  */
    ICONMGR_NOSHOW = 284,          /* ICONMGR_NOSHOW  */
    MAKE_TITLE = 285,              /* MAKE_TITLE  */
    ICONIFY_BY_UNMAPPING = 286,    /* ICONIFY_BY_UNMAPPING  */
    DONT_ICONIFY_BY_UNMAPPING = 287, /* DONT_ICONIFY_BY_UNMAPPING  */
    AUTO_POPUP = 288,              /* AUTO_POPUP  */
    NO_BORDER = 289,               /* NO_BORDER  */
    NO_ICON_TITLE = 290,           /* NO_ICON_TITLE  */
    NO_TITLE = 291,                /* NO_TITLE  */
    AUTO_RAISE = 292,              /* AUTO_RAISE  */
    NO_HILITE = 293,               /* NO_HILITE  */
    ICON_REGION = 294,             /* ICON_REGION  */
    WINDOW_REGION = 295,           /* WINDOW_REGION  */
    META = 296,                    /* META  */
    SHIFT = 297,                   /* SHIFT  */
    LOCK = 298,                    /* LOCK  */
    CONTROL = 299,                 /* CONTROL  */
    WINDOW = 300,                  /* WINDOW  */
    TITLE = 301,                   /* TITLE  */
    ICON = 302,                    /* ICON  */
    ROOT = 303,                    /* ROOT  */
    FRAME = 304,                   /* FRAME  */
    COLON = 305,                   /* COLON  */
    EQUALS = 306,                  /* EQUALS  */
    SQUEEZE_TITLE = 307,           /* SQUEEZE_TITLE  */
    DONT_SQUEEZE_TITLE = 308,      /* DONT_SQUEEZE_TITLE  */
    WARP_ON_DEICONIFY = 309,       /* WARP_ON_DEICONIFY  */
    START_ICONIFIED = 310,         /* START_ICONIFIED  */
    NO_TITLE_HILITE = 311,         /* NO_TITLE_HILITE  */
    TITLE_HILITE = 312,            /* TITLE_HILITE  */
    MOVE = 313,                    /* MOVE  */
    RESIZE = 314,                  /* RESIZE  */
    WAITC = 315,                   /* WAITC  */
    SELECT = 316,                  /* SELECT  */
    KILL = 317,                    /* KILL  */
    LEFT_TITLEBUTTON = 318,        /* LEFT_TITLEBUTTON  */
    RIGHT_TITLEBUTTON = 319,       /* RIGHT_TITLEBUTTON  */
    NUMBER = 320,                  /* NUMBER  */
    KEYWORD = 321,                 /* KEYWORD  */
    NKEYWORD = 322,                /* NKEYWORD  */
    CKEYWORD = 323,                /* CKEYWORD  */
    CLKEYWORD = 324,               /* CLKEYWORD  */
    FKEYWORD = 325,                /* FKEYWORD  */
    FSKEYWORD = 326,               /* FSKEYWORD  */
    FNKEYWORD = 327,               /* FNKEYWORD  */
    PRIORITY_SWITCHING = 328,      /* PRIORITY_SWITCHING  */
    PRIORITY_NOT_SWITCHING = 329,  /* PRIORITY_NOT_SWITCHING  */
    SKEYWORD = 330,                /* SKEYWORD  */
    SSKEYWORD = 331,               /* SSKEYWORD  */
    WINDOW_RING = 332,             /* WINDOW_RING  */
    WINDOW_RING_EXCLUDE = 333,     /* WINDOW_RING_EXCLUDE  */
    WARP_CURSOR = 334,             /* WARP_CURSOR  */
    ERRORTOKEN = 335,              /* ERRORTOKEN  */
    GRAVITY = 336,                 /* GRAVITY  */
    SIJENUM = 337,                 /* SIJENUM  */
    NO_STACKMODE = 338,            /* NO_STACKMODE  */
    ALWAYS_ON_TOP = 339,           /* ALWAYS_ON_TOP  */
    WORKSPACE = 340,               /* WORKSPACE  */
    WORKSPACES = 341,              /* WORKSPACES  */
    WORKSPCMGR_GEOMETRY = 342,     /* WORKSPCMGR_GEOMETRY  */
    OCCUPYALL = 343,               /* OCCUPYALL  */
    OCCUPYLIST = 344,              /* OCCUPYLIST  */
    MAPWINDOWCURRENTWORKSPACE = 345, /* MAPWINDOWCURRENTWORKSPACE  */
    MAPWINDOWDEFAULTWORKSPACE = 346, /* MAPWINDOWDEFAULTWORKSPACE  */
    ON_TOP_PRIORITY = 347,         /* ON_TOP_PRIORITY  */
    UNMAPBYMOVINGFARAWAY = 348,    /* UNMAPBYMOVINGFARAWAY  */
    OPAQUEMOVE = 349,              /* OPAQUEMOVE  */
    NOOPAQUEMOVE = 350,            /* NOOPAQUEMOVE  */
    OPAQUERESIZE = 351,            /* OPAQUERESIZE  */
    NOOPAQUERESIZE = 352,          /* NOOPAQUERESIZE  */
    DONTSETINACTIVE = 353,         /* DONTSETINACTIVE  */
    CHANGE_WORKSPACE_FUNCTION = 354, /* CHANGE_WORKSPACE_FUNCTION  */
    DEICONIFY_FUNCTION = 355,      /* DEICONIFY_FUNCTION  */
    ICONIFY_FUNCTION = 356,        /* ICONIFY_FUNCTION  */
    AUTOSQUEEZE = 357,             /* AUTOSQUEEZE  */
    STARTSQUEEZED = 358,           /* STARTSQUEEZED  */
    DONT_SAVE = 359,               /* DONT_SAVE  */
    AUTO_LOWER = 360,              /* AUTO_LOWER  */
    ICONMENU_DONTSHOW = 361,       /* ICONMENU_DONTSHOW  */
    WINDOW_BOX = 362,              /* WINDOW_BOX  */
    IGNOREMODIFIER = 363,          /* IGNOREMODIFIER  */
    WINDOW_GEOMETRIES = 364,       /* WINDOW_GEOMETRIES  */
    ALWAYSSQUEEZETOGRAVITY = 365,  /* ALWAYSSQUEEZETOGRAVITY  */
    VIRTUAL_SCREENS = 366,         /* VIRTUAL_SCREENS  */
    IGNORE_TRANSIENT = 367,        /* IGNORE_TRANSIENT  */
    EWMH_IGNORE = 368,             /* EWMH_IGNORE  */
    MWM_IGNORE = 369,              /* MWM_IGNORE  */
    MONITOR_LAYOUT = 370,          /* MONITOR_LAYOUT  */
    RPLAY_SOUNDS = 371,            /* RPLAY_SOUNDS  */
    FORCE_FOCUS = 372,             /* FORCE_FOCUS  */
    STRING = 373                   /* STRING  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 64 "gram.y"

    int num;
    char *ptr;

#line 187 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_HOME_FULLERMD_WORK_CTWM_BZR_DEV_CTWM_MKTAR_4Y0T7B_CTWM_4_1_0_BUILD_GRAM_TAB_H_INCLUDED  */
