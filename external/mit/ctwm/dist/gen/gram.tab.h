/* A Bison parser, made by GNU Bison 3.4.1.  */

/* Bison interface for Yacc-like parsers in C

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

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

#line 180 "/home/fullermd/work/ctwm/bzr/4.0.x/ctwm-mktar.asYJVb/ctwm-4.0.3/build/gram.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_HOME_FULLERMD_WORK_CTWM_BZR_4_0_X_CTWM_MKTAR_ASYJVB_CTWM_4_0_3_BUILD_GRAM_TAB_H_INCLUDED  */
