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
#line 23 "gram.y"

#include "ctwm.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "otp.h"
#include "iconmgr.h"
#include "icons.h"
#ifdef WINBOX
#include "windowbox.h"
#endif
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

#line 111 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"

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

#include "gram.tab.h"
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
  YYSYMBOL_ALTER = 24,                     /* ALTER  */
  YYSYMBOL_WINDOW_FUNCTION = 25,           /* WINDOW_FUNCTION  */
  YYSYMBOL_ZOOM = 26,                      /* ZOOM  */
  YYSYMBOL_ICONMGRS = 27,                  /* ICONMGRS  */
  YYSYMBOL_ICONMGR_GEOMETRY = 28,          /* ICONMGR_GEOMETRY  */
  YYSYMBOL_ICONMGR_NOSHOW = 29,            /* ICONMGR_NOSHOW  */
  YYSYMBOL_MAKE_TITLE = 30,                /* MAKE_TITLE  */
  YYSYMBOL_ICONIFY_BY_UNMAPPING = 31,      /* ICONIFY_BY_UNMAPPING  */
  YYSYMBOL_DONT_ICONIFY_BY_UNMAPPING = 32, /* DONT_ICONIFY_BY_UNMAPPING  */
  YYSYMBOL_AUTO_POPUP = 33,                /* AUTO_POPUP  */
  YYSYMBOL_NO_BORDER = 34,                 /* NO_BORDER  */
  YYSYMBOL_NO_ICON_TITLE = 35,             /* NO_ICON_TITLE  */
  YYSYMBOL_NO_TITLE = 36,                  /* NO_TITLE  */
  YYSYMBOL_AUTO_RAISE = 37,                /* AUTO_RAISE  */
  YYSYMBOL_NO_HILITE = 38,                 /* NO_HILITE  */
  YYSYMBOL_ICON_REGION = 39,               /* ICON_REGION  */
  YYSYMBOL_WINDOW_REGION = 40,             /* WINDOW_REGION  */
  YYSYMBOL_META = 41,                      /* META  */
  YYSYMBOL_SHIFT = 42,                     /* SHIFT  */
  YYSYMBOL_LOCK = 43,                      /* LOCK  */
  YYSYMBOL_CONTROL = 44,                   /* CONTROL  */
  YYSYMBOL_WINDOW = 45,                    /* WINDOW  */
  YYSYMBOL_TITLE = 46,                     /* TITLE  */
  YYSYMBOL_ICON = 47,                      /* ICON  */
  YYSYMBOL_ROOT = 48,                      /* ROOT  */
  YYSYMBOL_FRAME = 49,                     /* FRAME  */
  YYSYMBOL_COLON = 50,                     /* COLON  */
  YYSYMBOL_EQUALS = 51,                    /* EQUALS  */
  YYSYMBOL_SQUEEZE_TITLE = 52,             /* SQUEEZE_TITLE  */
  YYSYMBOL_DONT_SQUEEZE_TITLE = 53,        /* DONT_SQUEEZE_TITLE  */
  YYSYMBOL_WARP_ON_DEICONIFY = 54,         /* WARP_ON_DEICONIFY  */
  YYSYMBOL_START_ICONIFIED = 55,           /* START_ICONIFIED  */
  YYSYMBOL_NO_TITLE_HILITE = 56,           /* NO_TITLE_HILITE  */
  YYSYMBOL_TITLE_HILITE = 57,              /* TITLE_HILITE  */
  YYSYMBOL_MOVE = 58,                      /* MOVE  */
  YYSYMBOL_RESIZE = 59,                    /* RESIZE  */
  YYSYMBOL_WAITC = 60,                     /* WAITC  */
  YYSYMBOL_SELECT = 61,                    /* SELECT  */
  YYSYMBOL_KILL = 62,                      /* KILL  */
  YYSYMBOL_LEFT_TITLEBUTTON = 63,          /* LEFT_TITLEBUTTON  */
  YYSYMBOL_RIGHT_TITLEBUTTON = 64,         /* RIGHT_TITLEBUTTON  */
  YYSYMBOL_NUMBER = 65,                    /* NUMBER  */
  YYSYMBOL_KEYWORD = 66,                   /* KEYWORD  */
  YYSYMBOL_NKEYWORD = 67,                  /* NKEYWORD  */
  YYSYMBOL_CKEYWORD = 68,                  /* CKEYWORD  */
  YYSYMBOL_CLKEYWORD = 69,                 /* CLKEYWORD  */
  YYSYMBOL_FKEYWORD = 70,                  /* FKEYWORD  */
  YYSYMBOL_FSKEYWORD = 71,                 /* FSKEYWORD  */
  YYSYMBOL_FNKEYWORD = 72,                 /* FNKEYWORD  */
  YYSYMBOL_PRIORITY_SWITCHING = 73,        /* PRIORITY_SWITCHING  */
  YYSYMBOL_PRIORITY_NOT_SWITCHING = 74,    /* PRIORITY_NOT_SWITCHING  */
  YYSYMBOL_SKEYWORD = 75,                  /* SKEYWORD  */
  YYSYMBOL_SSKEYWORD = 76,                 /* SSKEYWORD  */
  YYSYMBOL_WINDOW_RING = 77,               /* WINDOW_RING  */
  YYSYMBOL_WINDOW_RING_EXCLUDE = 78,       /* WINDOW_RING_EXCLUDE  */
  YYSYMBOL_WARP_CURSOR = 79,               /* WARP_CURSOR  */
  YYSYMBOL_ERRORTOKEN = 80,                /* ERRORTOKEN  */
  YYSYMBOL_GRAVITY = 81,                   /* GRAVITY  */
  YYSYMBOL_SIJENUM = 82,                   /* SIJENUM  */
  YYSYMBOL_NO_STACKMODE = 83,              /* NO_STACKMODE  */
  YYSYMBOL_ALWAYS_ON_TOP = 84,             /* ALWAYS_ON_TOP  */
  YYSYMBOL_WORKSPACE = 85,                 /* WORKSPACE  */
  YYSYMBOL_WORKSPACES = 86,                /* WORKSPACES  */
  YYSYMBOL_WORKSPCMGR_GEOMETRY = 87,       /* WORKSPCMGR_GEOMETRY  */
  YYSYMBOL_OCCUPYALL = 88,                 /* OCCUPYALL  */
  YYSYMBOL_OCCUPYLIST = 89,                /* OCCUPYLIST  */
  YYSYMBOL_MAPWINDOWCURRENTWORKSPACE = 90, /* MAPWINDOWCURRENTWORKSPACE  */
  YYSYMBOL_MAPWINDOWDEFAULTWORKSPACE = 91, /* MAPWINDOWDEFAULTWORKSPACE  */
  YYSYMBOL_ON_TOP_PRIORITY = 92,           /* ON_TOP_PRIORITY  */
  YYSYMBOL_UNMAPBYMOVINGFARAWAY = 93,      /* UNMAPBYMOVINGFARAWAY  */
  YYSYMBOL_OPAQUEMOVE = 94,                /* OPAQUEMOVE  */
  YYSYMBOL_NOOPAQUEMOVE = 95,              /* NOOPAQUEMOVE  */
  YYSYMBOL_OPAQUERESIZE = 96,              /* OPAQUERESIZE  */
  YYSYMBOL_NOOPAQUERESIZE = 97,            /* NOOPAQUERESIZE  */
  YYSYMBOL_DONTSETINACTIVE = 98,           /* DONTSETINACTIVE  */
  YYSYMBOL_CHANGE_WORKSPACE_FUNCTION = 99, /* CHANGE_WORKSPACE_FUNCTION  */
  YYSYMBOL_DEICONIFY_FUNCTION = 100,       /* DEICONIFY_FUNCTION  */
  YYSYMBOL_ICONIFY_FUNCTION = 101,         /* ICONIFY_FUNCTION  */
  YYSYMBOL_AUTOSQUEEZE = 102,              /* AUTOSQUEEZE  */
  YYSYMBOL_STARTSQUEEZED = 103,            /* STARTSQUEEZED  */
  YYSYMBOL_DONT_SAVE = 104,                /* DONT_SAVE  */
  YYSYMBOL_AUTO_LOWER = 105,               /* AUTO_LOWER  */
  YYSYMBOL_ICONMENU_DONTSHOW = 106,        /* ICONMENU_DONTSHOW  */
  YYSYMBOL_WINDOW_BOX = 107,               /* WINDOW_BOX  */
  YYSYMBOL_IGNOREMODIFIER = 108,           /* IGNOREMODIFIER  */
  YYSYMBOL_WINDOW_GEOMETRIES = 109,        /* WINDOW_GEOMETRIES  */
  YYSYMBOL_ALWAYSSQUEEZETOGRAVITY = 110,   /* ALWAYSSQUEEZETOGRAVITY  */
  YYSYMBOL_VIRTUAL_SCREENS = 111,          /* VIRTUAL_SCREENS  */
  YYSYMBOL_IGNORE_TRANSIENT = 112,         /* IGNORE_TRANSIENT  */
  YYSYMBOL_EWMH_IGNORE = 113,              /* EWMH_IGNORE  */
  YYSYMBOL_MWM_IGNORE = 114,               /* MWM_IGNORE  */
  YYSYMBOL_MONITOR_LAYOUT = 115,           /* MONITOR_LAYOUT  */
  YYSYMBOL_RPLAY_SOUNDS = 116,             /* RPLAY_SOUNDS  */
  YYSYMBOL_FORCE_FOCUS = 117,              /* FORCE_FOCUS  */
  YYSYMBOL_STRING = 118,                   /* STRING  */
  YYSYMBOL_YYACCEPT = 119,                 /* $accept  */
  YYSYMBOL_twmrc = 120,                    /* twmrc  */
  YYSYMBOL_121_1 = 121,                    /* $@1  */
  YYSYMBOL_stmts = 122,                    /* stmts  */
  YYSYMBOL_stmt = 123,                     /* stmt  */
  YYSYMBOL_124_2 = 124,                    /* $@2  */
  YYSYMBOL_125_3 = 125,                    /* $@3  */
  YYSYMBOL_126_4 = 126,                    /* $@4  */
  YYSYMBOL_127_5 = 127,                    /* $@5  */
  YYSYMBOL_128_6 = 128,                    /* $@6  */
  YYSYMBOL_129_7 = 129,                    /* $@7  */
  YYSYMBOL_130_8 = 130,                    /* $@8  */
  YYSYMBOL_131_9 = 131,                    /* $@9  */
  YYSYMBOL_132_10 = 132,                   /* $@10  */
  YYSYMBOL_133_11 = 133,                   /* $@11  */
  YYSYMBOL_134_12 = 134,                   /* $@12  */
  YYSYMBOL_135_13 = 135,                   /* $@13  */
  YYSYMBOL_136_14 = 136,                   /* $@14  */
  YYSYMBOL_137_15 = 137,                   /* $@15  */
  YYSYMBOL_138_16 = 138,                   /* $@16  */
  YYSYMBOL_139_17 = 139,                   /* $@17  */
  YYSYMBOL_140_18 = 140,                   /* $@18  */
  YYSYMBOL_141_19 = 141,                   /* $@19  */
  YYSYMBOL_142_20 = 142,                   /* $@20  */
  YYSYMBOL_143_21 = 143,                   /* $@21  */
  YYSYMBOL_144_22 = 144,                   /* $@22  */
  YYSYMBOL_145_23 = 145,                   /* $@23  */
  YYSYMBOL_146_24 = 146,                   /* $@24  */
  YYSYMBOL_147_25 = 147,                   /* $@25  */
  YYSYMBOL_148_26 = 148,                   /* $@26  */
  YYSYMBOL_149_27 = 149,                   /* $@27  */
  YYSYMBOL_150_28 = 150,                   /* $@28  */
  YYSYMBOL_151_29 = 151,                   /* $@29  */
  YYSYMBOL_152_30 = 152,                   /* $@30  */
  YYSYMBOL_153_31 = 153,                   /* $@31  */
  YYSYMBOL_154_32 = 154,                   /* $@32  */
  YYSYMBOL_155_33 = 155,                   /* $@33  */
  YYSYMBOL_156_34 = 156,                   /* $@34  */
  YYSYMBOL_157_35 = 157,                   /* $@35  */
  YYSYMBOL_158_36 = 158,                   /* $@36  */
  YYSYMBOL_159_37 = 159,                   /* $@37  */
  YYSYMBOL_160_38 = 160,                   /* $@38  */
  YYSYMBOL_161_39 = 161,                   /* $@39  */
  YYSYMBOL_162_40 = 162,                   /* $@40  */
  YYSYMBOL_163_41 = 163,                   /* $@41  */
  YYSYMBOL_164_42 = 164,                   /* $@42  */
  YYSYMBOL_165_43 = 165,                   /* $@43  */
  YYSYMBOL_166_44 = 166,                   /* $@44  */
  YYSYMBOL_167_45 = 167,                   /* $@45  */
  YYSYMBOL_168_46 = 168,                   /* $@46  */
  YYSYMBOL_169_47 = 169,                   /* $@47  */
  YYSYMBOL_170_48 = 170,                   /* $@48  */
  YYSYMBOL_171_49 = 171,                   /* $@49  */
  YYSYMBOL_172_50 = 172,                   /* $@50  */
  YYSYMBOL_173_51 = 173,                   /* $@51  */
  YYSYMBOL_174_52 = 174,                   /* $@52  */
  YYSYMBOL_175_53 = 175,                   /* $@53  */
  YYSYMBOL_176_54 = 176,                   /* $@54  */
  YYSYMBOL_177_55 = 177,                   /* $@55  */
  YYSYMBOL_178_56 = 178,                   /* $@56  */
  YYSYMBOL_179_57 = 179,                   /* $@57  */
  YYSYMBOL_180_58 = 180,                   /* $@58  */
  YYSYMBOL_181_59 = 181,                   /* $@59  */
  YYSYMBOL_182_60 = 182,                   /* $@60  */
  YYSYMBOL_183_61 = 183,                   /* $@61  */
  YYSYMBOL_184_62 = 184,                   /* $@62  */
  YYSYMBOL_185_63 = 185,                   /* $@63  */
  YYSYMBOL_186_64 = 186,                   /* $@64  */
  YYSYMBOL_187_65 = 187,                   /* $@65  */
  YYSYMBOL_188_66 = 188,                   /* $@66  */
  YYSYMBOL_189_67 = 189,                   /* $@67  */
  YYSYMBOL_190_68 = 190,                   /* $@68  */
  YYSYMBOL_191_69 = 191,                   /* $@69  */
  YYSYMBOL_noarg = 192,                    /* noarg  */
  YYSYMBOL_sarg = 193,                     /* sarg  */
  YYSYMBOL_narg = 194,                     /* narg  */
  YYSYMBOL_keyaction = 195,                /* keyaction  */
  YYSYMBOL_full = 196,                     /* full  */
  YYSYMBOL_fullkey = 197,                  /* fullkey  */
  YYSYMBOL_keys = 198,                     /* keys  */
  YYSYMBOL_key = 199,                      /* key  */
  YYSYMBOL_vgrav = 200,                    /* vgrav  */
  YYSYMBOL_hgrav = 201,                    /* hgrav  */
  YYSYMBOL_contexts = 202,                 /* contexts  */
  YYSYMBOL_context = 203,                  /* context  */
  YYSYMBOL_contextkeys = 204,              /* contextkeys  */
  YYSYMBOL_contextkey = 205,               /* contextkey  */
  YYSYMBOL_binding_list = 206,             /* binding_list  */
  YYSYMBOL_binding_entries = 207,          /* binding_entries  */
  YYSYMBOL_binding_entry = 208,            /* binding_entry  */
  YYSYMBOL_pixmap_list = 209,              /* pixmap_list  */
  YYSYMBOL_pixmap_entries = 210,           /* pixmap_entries  */
  YYSYMBOL_pixmap_entry = 211,             /* pixmap_entry  */
  YYSYMBOL_cursor_list = 212,              /* cursor_list  */
  YYSYMBOL_cursor_entries = 213,           /* cursor_entries  */
  YYSYMBOL_cursor_entry = 214,             /* cursor_entry  */
  YYSYMBOL_color_list = 215,               /* color_list  */
  YYSYMBOL_color_entries = 216,            /* color_entries  */
  YYSYMBOL_color_entry = 217,              /* color_entry  */
  YYSYMBOL_218_70 = 218,                   /* $@70  */
  YYSYMBOL_save_color_list = 219,          /* save_color_list  */
  YYSYMBOL_s_color_entries = 220,          /* s_color_entries  */
  YYSYMBOL_s_color_entry = 221,            /* s_color_entry  */
  YYSYMBOL_win_color_list = 222,           /* win_color_list  */
  YYSYMBOL_win_color_entries = 223,        /* win_color_entries  */
  YYSYMBOL_win_color_entry = 224,          /* win_color_entry  */
  YYSYMBOL_wingeom_list = 225,             /* wingeom_list  */
  YYSYMBOL_wingeom_entries = 226,          /* wingeom_entries  */
  YYSYMBOL_wingeom_entry = 227,            /* wingeom_entry  */
  YYSYMBOL_vscreen_geom_list = 228,        /* vscreen_geom_list  */
  YYSYMBOL_vscreen_geom_entries = 229,     /* vscreen_geom_entries  */
  YYSYMBOL_vscreen_geom_entry = 230,       /* vscreen_geom_entry  */
  YYSYMBOL_ewmh_ignore_list = 231,         /* ewmh_ignore_list  */
  YYSYMBOL_ewmh_ignore_entries = 232,      /* ewmh_ignore_entries  */
  YYSYMBOL_ewmh_ignore_entry = 233,        /* ewmh_ignore_entry  */
  YYSYMBOL_mwm_ignore_list = 234,          /* mwm_ignore_list  */
  YYSYMBOL_mwm_ignore_entries = 235,       /* mwm_ignore_entries  */
  YYSYMBOL_mwm_ignore_entry = 236,         /* mwm_ignore_entry  */
  YYSYMBOL_layout_geom_list = 237,         /* layout_geom_list  */
  YYSYMBOL_layout_geom_entries = 238,      /* layout_geom_entries  */
  YYSYMBOL_layout_geom_entry = 239,        /* layout_geom_entry  */
  YYSYMBOL_squeeze = 240,                  /* squeeze  */
  YYSYMBOL_241_71 = 241,                   /* $@71  */
  YYSYMBOL_242_72 = 242,                   /* $@72  */
  YYSYMBOL_win_sqz_entries = 243,          /* win_sqz_entries  */
  YYSYMBOL_iconm_list = 244,               /* iconm_list  */
  YYSYMBOL_iconm_entries = 245,            /* iconm_entries  */
  YYSYMBOL_iconm_entry = 246,              /* iconm_entry  */
  YYSYMBOL_workspc_list = 247,             /* workspc_list  */
  YYSYMBOL_workspc_entries = 248,          /* workspc_entries  */
  YYSYMBOL_workspc_entry = 249,            /* workspc_entry  */
  YYSYMBOL_250_73 = 250,                   /* $@73  */
  YYSYMBOL_workapp_list = 251,             /* workapp_list  */
  YYSYMBOL_workapp_entries = 252,          /* workapp_entries  */
  YYSYMBOL_workapp_entry = 253,            /* workapp_entry  */
  YYSYMBOL_curwork = 254,                  /* curwork  */
  YYSYMBOL_defwork = 255,                  /* defwork  */
  YYSYMBOL_win_list = 256,                 /* win_list  */
  YYSYMBOL_win_entries = 257,              /* win_entries  */
  YYSYMBOL_win_entry = 258,                /* win_entry  */
  YYSYMBOL_occupy_list = 259,              /* occupy_list  */
  YYSYMBOL_occupy_entries = 260,           /* occupy_entries  */
  YYSYMBOL_occupy_entry = 261,             /* occupy_entry  */
  YYSYMBOL_262_74 = 262,                   /* $@74  */
  YYSYMBOL_263_75 = 263,                   /* $@75  */
  YYSYMBOL_264_76 = 264,                   /* $@76  */
  YYSYMBOL_occupy_workspc_list = 265,      /* occupy_workspc_list  */
  YYSYMBOL_occupy_workspc_entries = 266,   /* occupy_workspc_entries  */
  YYSYMBOL_occupy_workspc_entry = 267,     /* occupy_workspc_entry  */
  YYSYMBOL_occupy_window_list = 268,       /* occupy_window_list  */
  YYSYMBOL_occupy_window_entries = 269,    /* occupy_window_entries  */
  YYSYMBOL_occupy_window_entry = 270,      /* occupy_window_entry  */
  YYSYMBOL_icon_list = 271,                /* icon_list  */
  YYSYMBOL_icon_entries = 272,             /* icon_entries  */
  YYSYMBOL_icon_entry = 273,               /* icon_entry  */
  YYSYMBOL_rplay_sounds_list = 274,        /* rplay_sounds_list  */
  YYSYMBOL_rplay_sounds_entries = 275,     /* rplay_sounds_entries  */
  YYSYMBOL_rplay_sounds_entry = 276,       /* rplay_sounds_entry  */
  YYSYMBOL_function = 277,                 /* function  */
  YYSYMBOL_function_entries = 278,         /* function_entries  */
  YYSYMBOL_function_entry = 279,           /* function_entry  */
  YYSYMBOL_menu = 280,                     /* menu  */
  YYSYMBOL_menu_entries = 281,             /* menu_entries  */
  YYSYMBOL_menu_entry = 282,               /* menu_entry  */
  YYSYMBOL_action = 283,                   /* action  */
  YYSYMBOL_signed_number = 284,            /* signed_number  */
  YYSYMBOL_button = 285,                   /* button  */
  YYSYMBOL_string = 286,                   /* string  */
  YYSYMBOL_number = 287                    /* number  */
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
#define YYLAST   977

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  119
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  169
/* YYNRULES -- Number of rules.  */
#define YYNRULES  388
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  554

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   373


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
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   109,   109,   109,   113,   114,   117,   118,   119,   120,
     121,   122,   125,   128,   131,   134,   134,   138,   138,   142,
     142,   146,   146,   151,   151,   156,   156,   163,   169,   172,
     178,   181,   181,   184,   184,   187,   193,   195,   196,   197,
     197,   199,   202,   202,   204,   205,   205,   207,   208,   208,
     210,   211,   211,   213,   215,   218,   221,   221,   223,   223,
     225,   229,   245,   246,   248,   248,   250,   250,   253,   255,
     252,   257,   257,   259,   259,   261,   261,   263,   263,   265,
     265,   267,   267,   269,   270,   270,   272,   272,   274,   274,
     276,   277,   277,   279,   279,   281,   281,   283,   285,   285,
     287,   289,   291,   294,   293,   297,   296,   299,   299,   301,
     301,   304,   304,   308,   307,   312,   311,   317,   317,   319,
     321,   321,   323,   324,   324,   326,   326,   335,   335,   337,
     339,   339,   341,   343,   343,   345,   345,   347,   347,   349,
     349,   351,   352,   352,   354,   354,   356,   357,   357,   360,
     360,   362,   362,   364,   364,   366,   366,   368,   368,   370,
     370,   372,   388,   396,   404,   412,   420,   420,   422,   424,
     424,   426,   427,   427,   431,   431,   433,   433,   435,   435,
     437,   437,   439,   439,   441,   441,   443,   444,   444,   448,
     458,   466,   476,   485,   493,   503,   515,   518,   521,   524,
     525,   528,   529,   530,   531,   532,   542,   552,   555,   570,
     585,   586,   589,   590,   591,   592,   593,   594,   595,   596,
     597,   598,   599,   602,   603,   606,   607,   608,   609,   610,
     611,   612,   613,   614,   615,   616,   617,   621,   624,   625,
     628,   629,   630,   638,   641,   642,   645,   649,   652,   653,
     656,   658,   660,   662,   664,   666,   668,   670,   672,   674,
     676,   678,   680,   682,   684,   686,   688,   690,   692,   694,
     696,   698,   702,   706,   707,   710,   719,   719,   730,   741,
     744,   745,   748,   749,   752,   755,   756,   759,   764,   767,
     768,   771,   774,   777,   778,   781,   789,   792,   793,   796,
     800,   803,   804,   807,   811,   814,   815,   818,   822,   825,
     825,   830,   831,   831,   835,   836,   844,   847,   848,   851,
     856,   864,   867,   868,   871,   874,   874,   880,   883,   884,
     887,   890,   893,   896,   899,   904,   907,   910,   913,   918,
     921,   924,   927,   932,   935,   936,   939,   944,   947,   948,
     951,   951,   953,   953,   955,   955,   959,   962,   963,   966,
     974,   977,   978,   981,   989,   992,   993,   996,   999,  1010,
    1011,  1014,  1025,  1028,  1029,  1032,  1038,  1041,  1042,  1045,
    1055,  1067,  1068,  1110,  1111,  1112,  1115,  1127,  1133
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
  "FUNCTION", "ICONMGR_SHOW", "ICONMGR", "ALTER", "WINDOW_FUNCTION",
  "ZOOM", "ICONMGRS", "ICONMGR_GEOMETRY", "ICONMGR_NOSHOW", "MAKE_TITLE",
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
  "IGNORE_TRANSIENT", "EWMH_IGNORE", "MWM_IGNORE", "MONITOR_LAYOUT",
  "RPLAY_SOUNDS", "FORCE_FOCUS", "STRING", "$accept", "twmrc", "$@1",
  "stmts", "stmt", "$@2", "$@3", "$@4", "$@5", "$@6", "$@7", "$@8", "$@9",
  "$@10", "$@11", "$@12", "$@13", "$@14", "$@15", "$@16", "$@17", "$@18",
  "$@19", "$@20", "$@21", "$@22", "$@23", "$@24", "$@25", "$@26", "$@27",
  "$@28", "$@29", "$@30", "$@31", "$@32", "$@33", "$@34", "$@35", "$@36",
  "$@37", "$@38", "$@39", "$@40", "$@41", "$@42", "$@43", "$@44", "$@45",
  "$@46", "$@47", "$@48", "$@49", "$@50", "$@51", "$@52", "$@53", "$@54",
  "$@55", "$@56", "$@57", "$@58", "$@59", "$@60", "$@61", "$@62", "$@63",
  "$@64", "$@65", "$@66", "$@67", "$@68", "$@69", "noarg", "sarg", "narg",
  "keyaction", "full", "fullkey", "keys", "key", "vgrav", "hgrav",
  "contexts", "context", "contextkeys", "contextkey", "binding_list",
  "binding_entries", "binding_entry", "pixmap_list", "pixmap_entries",
  "pixmap_entry", "cursor_list", "cursor_entries", "cursor_entry",
  "color_list", "color_entries", "color_entry", "$@70", "save_color_list",
  "s_color_entries", "s_color_entry", "win_color_list",
  "win_color_entries", "win_color_entry", "wingeom_list",
  "wingeom_entries", "wingeom_entry", "vscreen_geom_list",
  "vscreen_geom_entries", "vscreen_geom_entry", "ewmh_ignore_list",
  "ewmh_ignore_entries", "ewmh_ignore_entry", "mwm_ignore_list",
  "mwm_ignore_entries", "mwm_ignore_entry", "layout_geom_list",
  "layout_geom_entries", "layout_geom_entry", "squeeze", "$@71", "$@72",
  "win_sqz_entries", "iconm_list", "iconm_entries", "iconm_entry",
  "workspc_list", "workspc_entries", "workspc_entry", "$@73",
  "workapp_list", "workapp_entries", "workapp_entry", "curwork", "defwork",
  "win_list", "win_entries", "win_entry", "occupy_list", "occupy_entries",
  "occupy_entry", "$@74", "$@75", "$@76", "occupy_workspc_list",
  "occupy_workspc_entries", "occupy_workspc_entry", "occupy_window_list",
  "occupy_window_entries", "occupy_window_entry", "icon_list",
  "icon_entries", "icon_entry", "rplay_sounds_list",
  "rplay_sounds_entries", "rplay_sounds_entry", "function",
  "function_entries", "function_entry", "menu", "menu_entries",
  "menu_entry", "action", "signed_number", "button", "string", "number", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-244)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-326)

#define yytable_value_is_error(Yyn) \
  0

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
    -244,    62,  -244,  -244,   611,  -244,   -53,    21,   -10,    67,
      68,  -244,  -244,  -244,  -244,   -53,  -244,   -10,    21,  -244,
     -53,    69,  -244,    73,  -244,    74,  -244,    79,    81,    85,
      87,   -53,   -53,    89,    90,  -244,  -244,    94,   -53,   -53,
    -244,    21,    82,    91,   -53,   -53,   104,  -244,   108,   111,
    -244,  -244,   -53,  -244,  -244,  -244,  -244,    41,  -244,   112,
     114,   115,   118,  -244,   -10,   -10,   -10,  -244,  -244,  -244,
     125,  -244,   -53,  -244,  -244,   128,  -244,  -244,  -244,  -244,
    -244,  -244,   136,  -244,  -244,  -244,  -244,  -244,  -244,     4,
      95,   149,  -244,  -244,  -244,   -53,  -244,  -244,  -244,  -244,
    -244,   163,   165,   166,   165,  -244,   167,  -244,  -244,   171,
      21,   167,   167,   167,   167,   167,   167,   167,   167,   167,
     167,    78,    78,   172,   167,   167,   167,   167,   121,   131,
    -244,  -244,   167,  -244,   167,  -244,   -53,   167,   167,   167,
     167,   167,   173,    21,   167,   187,   188,   189,    21,    21,
      45,   205,  -244,   167,   167,   167,   167,   167,   167,  -244,
    -244,  -244,   167,   167,   167,   167,   167,   -53,   218,   219,
     167,   220,   167,   222,   223,   224,   225,   167,  -244,  -244,
    -244,  -244,  -244,  -244,   -53,   226,  -244,   156,    30,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,   228,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,   151,   151,  -244,  -244,  -244,  -244,  -244,
     -10,   230,   -10,   230,   167,  -244,   167,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
     -53,  -244,   -53,  -244,  -244,  -244,   232,   167,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,   157,   169,   159,  -244,
    -244,  -244,   -53,   -53,   -53,   -53,   -53,   -53,   -53,   -53,
     -53,   -53,   -53,  -244,  -244,   -53,  -244,     7,    12,    -2,
    -244,  -244,     8,     9,  -244,    21,  -244,    11,  -244,  -244,
    -244,  -244,  -244,  -244,   167,    14,     6,    15,    16,   167,
    -244,   167,   153,    17,    18,    19,    20,    23,    25,  -244,
      21,    21,  -244,  -244,  -244,  -244,  -244,  -244,   -53,    26,
     -53,   -53,   -53,   -53,   -53,   -53,   -53,   -53,   -53,   -53,
     -53,  -244,  -244,  -244,   -53,  -244,   -53,   -53,  -244,  -244,
    -244,  -244,  -244,    24,  -244,  -244,  -244,  -244,  -244,   -53,
      21,   167,  -244,   155,    50,  -244,  -244,  -244,   235,  -244,
     -53,   -53,  -244,  -244,  -244,    27,  -244,    31,  -244,  -244,
     236,  -244,  -244,   -53,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,   -53,  -244,
    -244,   139,    55,   233,  -244,  -244,    42,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
     238,  -244,  -244,  -244,   -20,     1,  -244,    45,  -244,  -244,
      13,   239,  -244,  -244,   240,  -244,    32,  -244,    33,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,   -10,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,   -10,  -244,  -244,  -244,  -244,
     -53,  -244,   241,    21,  -244,   167,     2,    21,   -10,   -10,
    -244,  -244,  -244,   240,   242,  -244,  -244,  -244,   247,  -244,
     248,  -244,  -244,   226,   196,  -244,  -244,  -244,  -244,   167,
       5,  -244,  -244,   206,  -244,    37,  -244,  -244,  -244,    38,
    -244,  -244,  -244,   -53,    39,  -244,   167,   250,   -10,  -244,
    -244,   -53,    40,  -244,  -244,  -244,   249,  -244,  -244,   -53,
    -244,   167,  -244,   -53,  -244,  -244,  -244,   -10,  -244,  -244,
     -53,  -244,   -53,  -244
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_int16 yydefact[] =
{
       2,     0,     4,     1,     0,     6,     0,     0,     0,     0,
       0,   153,   155,   157,   159,     0,    93,     0,    36,    91,
       0,    90,   135,    41,    64,   122,   120,   129,   132,   141,
     100,     0,     0,   308,   311,   142,   137,    97,     0,     0,
     189,     0,   109,   111,   191,   194,   171,   172,   168,   119,
     107,    66,     0,    71,    75,    31,    33,     0,    77,    44,
      47,    50,    53,    86,     0,     0,     0,    79,    81,   125,
     146,    73,     0,    68,   174,    83,   176,   133,   178,   180,
     182,   184,   186,   387,     5,     7,     8,     9,    10,     0,
       0,   149,   388,   386,   381,     0,   161,   248,    38,   244,
      37,     0,     0,     0,     0,   151,     0,   162,    35,     0,
      28,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    56,    58,
     195,   113,     0,   115,     0,   190,   193,     0,     0,     0,
       0,     0,     0,    30,     0,     0,     0,     0,     0,     0,
       0,   101,   383,     0,     0,     0,     0,     0,     0,   163,
     164,   165,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   199,    63,
      61,    60,   199,    62,     0,     0,   382,     0,     0,   365,
     154,   273,   156,   280,   158,   160,     0,   344,    94,   317,
      92,    27,    89,   136,    40,    65,   124,   121,   128,   131,
     140,    99,   208,     0,     0,   314,   313,   143,   138,    96,
       0,     0,     0,     0,     0,   110,     0,   112,   192,   170,
     173,   167,   118,   108,   322,    67,    29,    72,   348,    76,
       0,    32,     0,    34,   384,   385,   102,     0,    78,    43,
      46,    49,    52,    87,    80,    82,   126,   145,    74,    25,
     199,   289,   175,    85,   293,   177,   134,   297,   179,   301,
     181,   305,   183,   369,   185,   188,     0,     0,     0,   377,
     150,   247,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   249,   243,     0,   245,     0,     0,     0,
     373,   152,     0,     0,   209,     0,    23,     0,    54,   238,
      57,    55,    59,   114,     0,     0,     0,     0,     0,     0,
     104,     0,    69,     0,     0,     0,     0,     0,     0,   207,
       0,   201,   202,   203,   204,   210,   200,   223,     0,     0,
     267,   259,   257,   253,   255,   251,   261,   263,   265,   269,
     271,   246,   364,   366,     0,   272,     0,     0,   274,   279,
     283,   281,   282,     0,   343,   345,   346,   316,   318,     0,
       0,     0,   310,     0,     0,   116,   321,   323,   324,   347,
       0,     0,   349,   350,   335,     0,   339,     0,   106,    26,
       0,   288,   290,     0,   292,   294,   295,   296,   298,   299,
     300,   302,   303,   304,   306,   307,   368,   370,     0,   205,
     206,     0,     0,     0,   376,   378,     0,   266,   258,   256,
     252,   254,   250,   260,   262,   264,   268,   270,   367,   278,
     275,   372,   374,   375,     0,    11,    24,     0,   237,   239,
       0,     0,   352,   354,     0,   336,     0,   340,     0,    70,
     291,   371,   221,   222,   218,   220,   219,   212,   213,   214,
     215,   216,     0,   217,   211,   234,   235,   231,   233,   232,
     225,   226,   227,   228,   229,     0,   230,   224,   236,   147,
       0,   379,     0,     0,   319,     0,    12,     0,     0,   199,
     240,   328,   326,     0,     0,   357,   351,   337,     0,   341,
       0,   197,   198,     0,     0,   285,   277,   320,    16,     0,
      13,   315,   242,     0,   241,     0,   353,   361,   355,     0,
     338,   342,   148,     0,     0,    18,     0,    14,     0,   327,
     329,   330,     0,   356,   358,   359,     0,   284,   286,     0,
      20,     0,   196,   331,   360,   362,   363,     0,   287,    22,
     332,   380,   333,   334
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -175,
    -244,   132,    43,  -244,  -244,  -244,  -244,    35,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,   158,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,   436,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -234,  -244,  -244,  -244,
    -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,  -244,
    -244,  -243,  -244,  -244,   -16,  -147,  -113,    -6,    -1
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     1,     2,     4,    84,   485,   509,   526,   541,   371,
     321,   146,   147,   113,   154,   155,   156,   157,   221,   223,
     114,   142,   168,   390,   144,   166,   145,   153,   162,   163,
     170,   158,   111,   109,   106,   127,   120,   247,   319,   141,
     132,   134,   224,   226,   140,   116,   115,   164,   117,   118,
     172,   112,   126,   119,   125,   165,   503,   185,   196,   101,
     102,   103,   104,   139,   137,   138,   169,   171,   173,   174,
     175,   176,   177,    85,    86,    87,   490,   179,   183,   276,
     336,   213,   305,   411,   464,   412,   477,   310,   374,   439,
     100,   188,   296,    98,   187,   293,   192,   298,   358,   482,
     194,   299,   361,   506,   524,   538,   262,   323,   392,   265,
     324,   395,   268,   325,   398,   270,   326,   401,   272,   327,
     404,    88,   123,   124,   307,   200,   303,   368,   235,   315,
     377,   441,   492,   515,   530,   241,   243,   198,   302,   365,
     239,   316,   382,   444,   493,   494,   496,   519,   534,   518,
     532,   545,   190,   297,   353,   274,   328,   407,   301,   363,
     432,   280,   339,   415,    96,   151,    89,    90,   152
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      91,   107,   359,   246,   -15,   -17,    93,   277,   -19,   105,
     379,   352,   364,   367,   110,   372,   355,   108,   376,   384,
     386,   391,   394,   397,   400,   121,   122,   403,   431,   406,
     414,   445,   128,   129,   294,   447,   497,   499,   135,   136,
     130,   529,   533,   537,   544,    92,   143,   480,   159,   160,
     161,   380,   148,   149,   438,   178,   148,   149,   150,     7,
      94,    95,     3,   488,   489,    83,   167,   360,   465,   466,
      97,    99,   -88,   180,    94,    95,   -39,  -123,   467,   468,
     356,   357,  -127,   181,  -130,   322,    92,   295,  -139,   186,
     -98,   381,  -309,  -312,    94,    95,   469,   -95,    83,   131,
     470,   471,   472,   473,   474,   475,    92,  -169,   133,   201,
      92,  -166,    94,    95,  -117,   -42,    83,   -45,   -48,    83,
      83,   -51,    83,    83,    83,    83,    83,    83,  -144,    83,
     228,   -84,    83,    83,    83,    83,    83,    83,    83,  -187,
     476,    83,   236,    83,    83,    83,   182,   244,   245,    83,
      83,    83,   452,   453,   184,    83,    83,    83,    83,   212,
     281,   259,   454,   455,   282,   283,   189,   329,   191,   193,
     197,   329,   220,    83,   199,   215,   234,   330,   278,   284,
     456,   330,   222,   329,   457,   458,   459,   460,   461,   462,
     238,   240,   242,   330,   331,   332,   333,   334,   331,   332,
     333,   334,   285,   286,   308,   287,   311,   335,  -103,   338,
     331,   332,   333,   334,   288,   289,   290,   291,   292,   337,
     329,   260,   261,   264,   463,   267,   269,   271,   273,   279,
     330,   300,   304,   309,   317,  -105,   318,   437,  -325,   479,
     449,  -276,   491,   495,   505,   517,   523,   331,   332,   333,
     334,   520,   521,   -21,   214,   547,   528,   306,   312,   516,
     522,   440,   195,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   340,   341,   342,   343,
     344,   345,   346,   347,   348,   349,   350,     0,     0,   351,
     487,   354,     0,   362,     0,     0,   366,   369,     0,     0,
       0,   373,     0,     0,   370,     0,     0,     0,     0,   378,
     383,   385,   387,     0,   513,     0,     0,   393,   396,   399,
     402,   405,   408,     0,     0,     0,     0,     0,     0,   409,
     410,     0,   413,   416,   417,   418,   419,   420,   421,   422,
     423,   424,   425,   426,   427,     0,     0,   433,   428,     0,
     429,   430,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   434,     0,     0,     0,     0,     0,   435,
       0,     0,     0,     0,   442,   443,     0,     0,     0,   446,
       0,   448,     0,     0,     0,     0,     0,   450,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     481,     0,   451,     0,     0,     0,   478,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   483,   486,
       0,     0,     0,   484,     0,     0,     0,     0,     0,     0,
     498,     0,   500,     0,     0,     0,   501,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   502,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   512,   514,   504,     0,     0,     0,     0,     0,
     510,     0,   507,     0,     0,     0,   511,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   527,     0,     0,     0,     0,   531,
       0,     0,   542,   535,     0,     0,     0,   536,   539,     0,
       0,     0,     0,     0,     0,   543,   546,     0,     0,     0,
       0,   551,     0,   548,     0,     0,     0,   550,     0,     0,
       0,     0,     0,     0,   552,     0,   553,   202,   203,   204,
     205,   206,   207,   208,   209,   210,   211,     0,     0,     0,
     216,   217,   218,   219,     0,     0,     0,     0,   225,     0,
     227,     0,     0,   229,   230,   231,   232,   233,     0,     0,
     237,     0,     0,     0,     0,     0,     0,     0,     0,   248,
     249,   250,   251,   252,   253,     0,     0,     0,   254,   255,
     256,   257,   258,     0,     0,     0,   263,     0,   266,     0,
       0,    -3,     5,   275,     0,     0,     0,     0,     0,     6,
       7,     8,     0,     0,     0,     0,     9,    10,    11,    12,
      13,    14,    15,    16,     0,     0,    17,    18,    19,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      31,    32,     0,     0,     0,     0,     0,     0,     0,     0,
     313,     0,   314,    33,    34,    35,    36,    37,     0,     0,
       0,     0,     0,     0,    38,    39,     0,    40,    41,     0,
       0,     0,     0,   320,    42,    43,    44,    45,    46,    47,
      48,     0,     0,     0,    49,    50,     0,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    65,    66,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     375,     0,     0,     0,     0,   388,     0,   389,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   436,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   508,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   525,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,   540,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   549
};

static const yytype_int16 yycheck[] =
{
       6,    17,     4,   150,     3,     3,     7,   182,     3,    15,
       4,     4,     4,     4,    20,     4,     4,    18,     4,     4,
       4,     4,     4,     4,     4,    31,    32,     4,     4,     4,
       4,     4,    38,    39,     4,     4,     4,     4,    44,    45,
      41,     4,     4,     4,     4,    65,    52,     5,    64,    65,
      66,    45,    11,    12,     4,    51,    11,    12,    17,     9,
      70,    71,     0,    50,    51,   118,    72,    69,    13,    14,
       3,     3,     3,    89,    70,    71,     3,     3,    23,    24,
      68,    69,     3,    89,     3,   260,    65,    57,     3,    95,
       3,    85,     3,     3,    70,    71,    41,     3,   118,    17,
      45,    46,    47,    48,    49,    50,    65,     3,    17,   110,
      65,     3,    70,    71,     3,     3,   118,     3,     3,   118,
     118,     3,   118,   118,   118,   118,   118,   118,     3,   118,
     136,     3,   118,   118,   118,   118,   118,   118,   118,     3,
      85,   118,   143,   118,   118,   118,    51,   148,   149,   118,
     118,   118,    13,    14,     5,   118,   118,   118,   118,    81,
       4,   167,    23,    24,     8,     9,     3,    14,     3,     3,
       3,    14,    51,   118,     3,     3,     3,    24,   184,    23,
      41,    24,    51,    14,    45,    46,    47,    48,    49,    50,
       3,     3,     3,    24,    41,    42,    43,    44,    41,    42,
      43,    44,    46,    47,   220,    49,   222,    50,     3,    50,
      41,    42,    43,    44,    58,    59,    60,    61,    62,    50,
      14,     3,     3,     3,    85,     3,     3,     3,     3,     3,
      24,     3,    81,     3,   240,     3,   242,    82,     3,     6,
       4,     3,     3,     3,     3,     3,    50,    41,    42,    43,
      44,     4,     4,     3,   122,     6,    50,   214,   223,   493,
     503,   374,   104,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   282,   283,   284,   285,
     286,   287,   288,   289,   290,   291,   292,    -1,    -1,   295,
     437,   297,    -1,   299,    -1,    -1,   302,   303,    -1,    -1,
      -1,   307,    -1,    -1,   305,    -1,    -1,    -1,    -1,   315,
     316,   317,   318,    -1,   489,    -1,    -1,   323,   324,   325,
     326,   327,   328,    -1,    -1,    -1,    -1,    -1,    -1,   330,
     331,    -1,   338,   339,   340,   341,   342,   343,   344,   345,
     346,   347,   348,   349,   350,    -1,    -1,   363,   354,    -1,
     356,   357,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,   369,    -1,    -1,    -1,    -1,    -1,   370,
      -1,    -1,    -1,    -1,   380,   381,    -1,    -1,    -1,   385,
      -1,   387,    -1,    -1,    -1,    -1,    -1,   393,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     416,    -1,   408,    -1,    -1,    -1,   412,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   434,   435,
      -1,    -1,    -1,   434,    -1,    -1,    -1,    -1,    -1,    -1,
     446,    -1,   448,    -1,    -1,    -1,   462,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   475,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   488,   489,   480,    -1,    -1,    -1,    -1,    -1,
     486,    -1,   483,    -1,    -1,    -1,   487,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   510,    -1,    -1,    -1,    -1,   515,
      -1,    -1,   528,   519,    -1,    -1,    -1,   523,   524,    -1,
      -1,    -1,    -1,    -1,    -1,   531,   532,    -1,    -1,    -1,
      -1,   547,    -1,   539,    -1,    -1,    -1,   543,    -1,    -1,
      -1,    -1,    -1,    -1,   550,    -1,   552,   111,   112,   113,
     114,   115,   116,   117,   118,   119,   120,    -1,    -1,    -1,
     124,   125,   126,   127,    -1,    -1,    -1,    -1,   132,    -1,
     134,    -1,    -1,   137,   138,   139,   140,   141,    -1,    -1,
     144,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   153,
     154,   155,   156,   157,   158,    -1,    -1,    -1,   162,   163,
     164,   165,   166,    -1,    -1,    -1,   170,    -1,   172,    -1,
      -1,     0,     1,   177,    -1,    -1,    -1,    -1,    -1,     8,
       9,    10,    -1,    -1,    -1,    -1,    15,    16,    17,    18,
      19,    20,    21,    22,    -1,    -1,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     224,    -1,   226,    52,    53,    54,    55,    56,    -1,    -1,
      -1,    -1,    -1,    -1,    63,    64,    -1,    66,    67,    -1,
      -1,    -1,    -1,   247,    73,    74,    75,    76,    77,    78,
      79,    -1,    -1,    -1,    83,    84,    -1,    86,    87,    88,
      89,    90,    91,    92,    93,    94,    95,    96,    97,    98,
      99,   100,   101,   102,   103,   104,   105,   106,   107,   108,
     109,   110,   111,   112,   113,   114,   115,   116,   117,   118,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
     314,    -1,    -1,    -1,    -1,   319,    -1,   321,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   371,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   485,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   509,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,   526,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,   541
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int16 yystos[] =
{
       0,   120,   121,     0,   122,     1,     8,     9,    10,    15,
      16,    17,    18,    19,    20,    21,    22,    25,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    52,    53,    54,    55,    56,    63,    64,
      66,    67,    73,    74,    75,    76,    77,    78,    79,    83,
      84,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   123,   192,   193,   194,   240,   285,
     286,   286,    65,   287,    70,    71,   283,     3,   212,     3,
     209,   178,   179,   180,   181,   286,   153,   283,   287,   152,
     286,   151,   170,   132,   139,   165,   164,   167,   168,   172,
     155,   286,   286,   241,   242,   173,   171,   154,   286,   286,
     287,    17,   159,    17,   160,   286,   286,   183,   184,   182,
     163,   158,   140,   286,   143,   145,   130,   131,    11,    12,
      17,   284,   287,   146,   133,   134,   135,   136,   150,   283,
     283,   283,   147,   148,   166,   174,   144,   286,   141,   185,
     149,   186,   169,   187,   188,   189,   190,   191,    51,   196,
     283,   286,    51,   197,     5,   176,   286,   213,   210,     3,
     271,     3,   215,     3,   219,   215,   177,     3,   256,     3,
     244,   287,   256,   256,   256,   256,   256,   256,   256,   256,
     256,   256,    81,   200,   200,     3,   256,   256,   256,   256,
      51,   137,    51,   138,   161,   256,   162,   256,   286,   256,
     256,   256,   256,   256,     3,   247,   287,   256,     3,   259,
       3,   254,     3,   255,   287,   287,   284,   156,   256,   256,
     256,   256,   256,   256,   256,   256,   256,   256,   256,   286,
       3,     3,   225,   256,     3,   228,   256,     3,   231,     3,
     234,     3,   237,     3,   274,   256,   198,   198,   286,     3,
     280,     4,     8,     9,    23,    46,    47,    49,    58,    59,
      60,    61,    62,   214,     4,    57,   211,   272,   216,   220,
       3,   277,   257,   245,    81,   201,   201,   243,   283,     3,
     206,   283,   206,   256,   256,   248,   260,   286,   286,   157,
     256,   129,   198,   226,   229,   232,   235,   238,   275,    14,
      24,    41,    42,    43,    44,    50,   199,    50,    50,   281,
     286,   286,   286,   286,   286,   286,   286,   286,   286,   286,
     286,   286,     4,   273,   286,     4,    68,    69,   217,     4,
      69,   221,   286,   278,     4,   258,   286,     4,   246,   286,
     287,   128,     4,   286,   207,   256,     4,   249,   286,     4,
      45,    85,   261,   286,     4,   286,     4,   286,   256,   256,
     142,     4,   227,   286,     4,   230,   286,     4,   233,   286,
       4,   236,   286,     4,   239,   286,     4,   276,   286,   287,
     287,   202,   204,   286,     4,   282,   286,   286,   286,   286,
     286,   286,   286,   286,   286,   286,   286,   286,   286,   286,
     286,     4,   279,   283,   286,   287,   256,    82,     4,   208,
     285,   250,   286,   286,   262,     4,   286,     4,   286,     4,
     286,   286,    13,    14,    23,    24,    41,    45,    46,    47,
      48,    49,    50,    85,   203,    13,    14,    23,    24,    41,
      45,    46,    47,    48,    49,    50,    85,   205,   286,     6,
       5,   283,   218,   286,   287,   124,   286,   284,    50,    51,
     195,     3,   251,   263,   264,     3,   265,     4,   286,     4,
     286,   283,   283,   175,   286,     3,   222,   287,   256,   125,
     286,   287,   283,   198,   283,   252,   265,     3,   268,   266,
       4,     4,   280,    50,   223,   256,   126,   286,    50,     4,
     253,   286,   269,     4,   267,   286,   286,     4,   224,   286,
     256,   127,   283,   286,     4,   270,   286,     6,   286,   256,
     286,   283,   286,   286
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int16 yyr1[] =
{
       0,   119,   121,   120,   122,   122,   123,   123,   123,   123,
     123,   123,   123,   123,   123,   124,   123,   125,   123,   126,
     123,   127,   123,   128,   123,   129,   123,   123,   123,   123,
     123,   130,   123,   131,   123,   123,   123,   123,   123,   132,
     123,   123,   133,   123,   123,   134,   123,   123,   135,   123,
     123,   136,   123,   123,   123,   123,   137,   123,   138,   123,
     123,   123,   123,   123,   139,   123,   140,   123,   141,   142,
     123,   143,   123,   144,   123,   145,   123,   146,   123,   147,
     123,   148,   123,   123,   149,   123,   150,   123,   151,   123,
     123,   152,   123,   153,   123,   154,   123,   123,   155,   123,
     123,   123,   123,   156,   123,   157,   123,   158,   123,   159,
     123,   160,   123,   161,   123,   162,   123,   163,   123,   123,
     164,   123,   123,   165,   123,   166,   123,   167,   123,   123,
     168,   123,   123,   169,   123,   170,   123,   171,   123,   172,
     123,   123,   173,   123,   174,   123,   123,   175,   123,   176,
     123,   177,   123,   178,   123,   179,   123,   180,   123,   181,
     123,   123,   123,   123,   123,   123,   182,   123,   123,   183,
     123,   123,   184,   123,   185,   123,   186,   123,   187,   123,
     188,   123,   189,   123,   190,   123,   123,   191,   123,   192,
     193,   193,   193,   193,   193,   194,   195,   196,   197,   198,
     198,   199,   199,   199,   199,   199,   199,   199,   200,   201,
     202,   202,   203,   203,   203,   203,   203,   203,   203,   203,
     203,   203,   203,   204,   204,   205,   205,   205,   205,   205,
     205,   205,   205,   205,   205,   205,   205,   206,   207,   207,
     208,   208,   208,   209,   210,   210,   211,   212,   213,   213,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   214,   214,   214,   214,   214,   214,   214,   214,
     214,   214,   215,   216,   216,   217,   218,   217,   217,   219,
     220,   220,   221,   221,   222,   223,   223,   224,   225,   226,
     226,   227,   228,   229,   229,   230,   231,   232,   232,   233,
     234,   235,   235,   236,   237,   238,   238,   239,   240,   241,
     240,   240,   242,   240,   243,   243,   244,   245,   245,   246,
     246,   247,   248,   248,   249,   250,   249,   251,   252,   252,
     253,   253,   253,   253,   253,   254,   254,   254,   254,   255,
     255,   255,   255,   256,   257,   257,   258,   259,   260,   260,
     262,   261,   263,   261,   264,   261,   265,   266,   266,   267,
     268,   269,   269,   270,   271,   272,   272,   273,   274,   275,
     275,   276,   277,   278,   278,   279,   280,   281,   281,   282,
     282,   283,   283,   284,   284,   284,   285,   286,   287
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
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
       0,     3,     0,     3,     0,     3,     1,     0,     3,     1,
       2,     1,     3,     2,     1,     2,     4,     6,     6,     0,
       2,     1,     1,     1,     1,     2,     2,     1,     1,     1,
       0,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     0,     2,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     0,     2,
       2,     3,     3,     3,     0,     2,     2,     3,     0,     2,
       3,     2,     3,     2,     3,     2,     3,     2,     3,     2,
       3,     2,     3,     2,     3,     2,     3,     2,     3,     2,
       3,     2,     3,     0,     2,     2,     0,     4,     2,     3,
       0,     2,     1,     1,     3,     0,     2,     2,     3,     0,
       2,     2,     3,     0,     2,     1,     3,     0,     2,     1,
       3,     0,     2,     1,     3,     0,     2,     1,     1,     0,
       5,     1,     0,     3,     0,     5,     3,     0,     2,     3,
       4,     3,     0,     2,     1,     0,     3,     3,     0,     2,
       1,     2,     3,     4,     5,     3,     4,     5,     6,     3,
       4,     5,     6,     3,     0,     2,     1,     3,     0,     2,
       0,     3,     0,     4,     0,     4,     3,     0,     2,     1,
       3,     0,     2,     1,     3,     0,     2,     2,     3,     0,
       2,     2,     3,     0,     2,     1,     3,     0,     2,     2,
       7,     1,     2,     1,     2,     2,     2,     1,     1
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
  case 2: /* $@1: %empty  */
#line 109 "gram.y"
                  { InitGramVariables(); }
#line 1943 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 11: /* stmt: ICON_REGION string vgrav hgrav number number  */
#line 122 "gram.y"
                                                               {
		      AddIconRegion((yyvsp[-4].ptr), (yyvsp[-3].num), (yyvsp[-2].num), (yyvsp[-1].num), (yyvsp[0].num), "undef", "undef", "undef");
		  }
#line 1951 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 12: /* stmt: ICON_REGION string vgrav hgrav number number string  */
#line 125 "gram.y"
                                                                      {
		      AddIconRegion((yyvsp[-5].ptr), (yyvsp[-4].num), (yyvsp[-3].num), (yyvsp[-2].num), (yyvsp[-1].num), (yyvsp[0].ptr), "undef", "undef");
		  }
#line 1959 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 13: /* stmt: ICON_REGION string vgrav hgrav number number string string  */
#line 128 "gram.y"
                                                                             {
		      AddIconRegion((yyvsp[-6].ptr), (yyvsp[-5].num), (yyvsp[-4].num), (yyvsp[-3].num), (yyvsp[-2].num), (yyvsp[-1].ptr), (yyvsp[0].ptr), "undef");
		  }
#line 1967 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 14: /* stmt: ICON_REGION string vgrav hgrav number number string string string  */
#line 131 "gram.y"
                                                                                    {
		      AddIconRegion((yyvsp[-7].ptr), (yyvsp[-6].num), (yyvsp[-5].num), (yyvsp[-4].num), (yyvsp[-3].num), (yyvsp[-2].ptr), (yyvsp[-1].ptr), (yyvsp[0].ptr));
		  }
#line 1975 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 15: /* $@2: %empty  */
#line 134 "gram.y"
                                                               {
		      curplist = AddIconRegion((yyvsp[-4].ptr), (yyvsp[-3].num), (yyvsp[-2].num), (yyvsp[-1].num), (yyvsp[0].num), "undef", "undef", "undef");
		  }
#line 1983 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 17: /* $@3: %empty  */
#line 138 "gram.y"
                                                                      {
		      curplist = AddIconRegion((yyvsp[-5].ptr), (yyvsp[-4].num), (yyvsp[-3].num), (yyvsp[-2].num), (yyvsp[-1].num), (yyvsp[0].ptr), "undef", "undef");
		  }
#line 1991 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 19: /* $@4: %empty  */
#line 142 "gram.y"
                                                                             {
		      curplist = AddIconRegion((yyvsp[-6].ptr), (yyvsp[-5].num), (yyvsp[-4].num), (yyvsp[-3].num), (yyvsp[-2].num), (yyvsp[-1].ptr), (yyvsp[0].ptr), "undef");
		  }
#line 1999 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 21: /* $@5: %empty  */
#line 146 "gram.y"
                                                                                    {
		      curplist = AddIconRegion((yyvsp[-7].ptr), (yyvsp[-6].num), (yyvsp[-5].num), (yyvsp[-4].num), (yyvsp[-3].num), (yyvsp[-2].ptr), (yyvsp[-1].ptr), (yyvsp[0].ptr));
		  }
#line 2007 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 23: /* $@6: %empty  */
#line 151 "gram.y"
                                                   {
		      curplist = AddWindowRegion ((yyvsp[-2].ptr), (yyvsp[-1].num), (yyvsp[0].num));
		  }
#line 2015 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 25: /* $@7: %empty  */
#line 156 "gram.y"
                                           {
#ifdef WINBOX
		      curplist = addWindowBox ((yyvsp[-1].ptr), (yyvsp[0].ptr));
#endif
		  }
#line 2025 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 27: /* stmt: ICONMGR_GEOMETRY string number  */
#line 163 "gram.y"
                                                        { if (Scr->FirstTime)
						  {
						    Scr->iconmgr->geometry= (char*)(yyvsp[-1].ptr);
						    Scr->iconmgr->columns=(yyvsp[0].num);
						  }
						}
#line 2036 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 28: /* stmt: ICONMGR_GEOMETRY string  */
#line 169 "gram.y"
                                                { if (Scr->FirstTime)
						    Scr->iconmgr->geometry = (char*)(yyvsp[0].ptr);
						}
#line 2044 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 29: /* stmt: WORKSPCMGR_GEOMETRY string number  */
#line 172 "gram.y"
                                                        { if (Scr->FirstTime)
				{
				    Scr->workSpaceMgr.geometry= (char*)(yyvsp[-1].ptr);
				    Scr->workSpaceMgr.columns=(yyvsp[0].num);
				}
						}
#line 2055 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 30: /* stmt: WORKSPCMGR_GEOMETRY string  */
#line 178 "gram.y"
                                                { if (Scr->FirstTime)
				    Scr->workSpaceMgr.geometry = (char*)(yyvsp[0].ptr);
						}
#line 2063 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 31: /* $@8: %empty  */
#line 181 "gram.y"
                                            {}
#line 2069 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 33: /* $@9: %empty  */
#line 184 "gram.y"
                                            {}
#line 2075 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 35: /* stmt: ZOOM number  */
#line 187 "gram.y"
                                        { if (Scr->FirstTime)
					  {
						Scr->DoZoom = true;
						Scr->ZoomCount = (yyvsp[0].num);
					  }
					}
#line 2086 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 36: /* stmt: ZOOM  */
#line 193 "gram.y"
                                        { if (Scr->FirstTime)
						Scr->DoZoom = true; }
#line 2093 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 37: /* stmt: PIXMAPS pixmap_list  */
#line 195 "gram.y"
                                        {}
#line 2099 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 38: /* stmt: CURSORS cursor_list  */
#line 196 "gram.y"
                                        {}
#line 2105 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 39: /* $@10: %empty  */
#line 197 "gram.y"
                                        { curplist = &Scr->IconifyByUn; }
#line 2111 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 41: /* stmt: ICONIFY_BY_UNMAPPING  */
#line 199 "gram.y"
                                        { if (Scr->FirstTime)
		    Scr->IconifyByUnmapping = true; }
#line 2118 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 42: /* $@11: %empty  */
#line 202 "gram.y"
                                { curplist = &Scr->OpaqueMoveList; }
#line 2124 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 44: /* stmt: OPAQUEMOVE  */
#line 204 "gram.y"
                                { if (Scr->FirstTime) Scr->DoOpaqueMove = true; }
#line 2130 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 45: /* $@12: %empty  */
#line 205 "gram.y"
                                { curplist = &Scr->NoOpaqueMoveList; }
#line 2136 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 47: /* stmt: NOOPAQUEMOVE  */
#line 207 "gram.y"
                                { if (Scr->FirstTime) Scr->DoOpaqueMove = false; }
#line 2142 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 48: /* $@13: %empty  */
#line 208 "gram.y"
                                { curplist = &Scr->OpaqueMoveList; }
#line 2148 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 50: /* stmt: OPAQUERESIZE  */
#line 210 "gram.y"
                                { if (Scr->FirstTime) Scr->DoOpaqueResize = true; }
#line 2154 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 51: /* $@14: %empty  */
#line 211 "gram.y"
                                        { curplist = &Scr->NoOpaqueResizeList; }
#line 2160 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 53: /* stmt: NOOPAQUERESIZE  */
#line 213 "gram.y"
                                        { if (Scr->FirstTime) Scr->DoOpaqueResize = false; }
#line 2166 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 54: /* stmt: LEFT_TITLEBUTTON string EQUALS action  */
#line 215 "gram.y"
                                                        {
					  GotTitleButton ((yyvsp[-2].ptr), (yyvsp[0].num), false);
					}
#line 2174 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 55: /* stmt: RIGHT_TITLEBUTTON string EQUALS action  */
#line 218 "gram.y"
                                                         {
					  GotTitleButton ((yyvsp[-2].ptr), (yyvsp[0].num), true);
					}
#line 2182 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 56: /* $@15: %empty  */
#line 221 "gram.y"
                                          { CreateTitleButton((yyvsp[0].ptr), 0, NULL, NULL, false, true); }
#line 2188 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 58: /* $@16: %empty  */
#line 223 "gram.y"
                                           { CreateTitleButton((yyvsp[0].ptr), 0, NULL, NULL, true, true); }
#line 2194 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 60: /* stmt: button string  */
#line 225 "gram.y"
                                        {
		    root = GetRoot((yyvsp[0].ptr), NULL, NULL);
		    AddFuncButton ((yyvsp[-1].num), C_ROOT, 0, F_MENU, root, NULL);
		}
#line 2203 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 61: /* stmt: button action  */
#line 229 "gram.y"
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
#line 2224 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 62: /* stmt: string fullkey  */
#line 245 "gram.y"
                                        { GotKey((yyvsp[-1].ptr), (yyvsp[0].num)); }
#line 2230 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 63: /* stmt: button full  */
#line 246 "gram.y"
                                        { GotButton((yyvsp[-1].num), (yyvsp[0].num)); }
#line 2236 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 64: /* $@17: %empty  */
#line 248 "gram.y"
                                            { curplist = &Scr->DontIconify; }
#line 2242 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 66: /* $@18: %empty  */
#line 250 "gram.y"
                             {}
#line 2248 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 68: /* $@19: %empty  */
#line 253 "gram.y"
                        { mods = 0; }
#line 2254 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 69: /* $@20: %empty  */
#line 255 "gram.y"
                        { Scr->IgnoreModifier |= mods; mods = 0; }
#line 2260 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 71: /* $@21: %empty  */
#line 257 "gram.y"
                                        { curplist = &Scr->OccupyAll; }
#line 2266 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 73: /* $@22: %empty  */
#line 259 "gram.y"
                                        { curplist = &Scr->IconMenuDontShow; }
#line 2272 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 75: /* $@23: %empty  */
#line 261 "gram.y"
                             {}
#line 2278 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 77: /* $@24: %empty  */
#line 263 "gram.y"
                                        { curplist = &Scr->UnmapByMovingFarAway; }
#line 2284 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 79: /* $@25: %empty  */
#line 265 "gram.y"
                                        { curplist = &Scr->AutoSqueeze; }
#line 2290 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 81: /* $@26: %empty  */
#line 267 "gram.y"
                                        { curplist = &Scr->StartSqueezed; }
#line 2296 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 83: /* stmt: ALWAYSSQUEEZETOGRAVITY  */
#line 269 "gram.y"
                                                { Scr->AlwaysSqueezeToGravity = true; }
#line 2302 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 84: /* $@27: %empty  */
#line 270 "gram.y"
                                                { curplist = &Scr->AlwaysSqueezeToGravityL; }
#line 2308 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 86: /* $@28: %empty  */
#line 272 "gram.y"
                                        { curplist = &Scr->DontSetInactive; }
#line 2314 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 88: /* $@29: %empty  */
#line 274 "gram.y"
                                        { curplist = &Scr->IconMgrNoShow; }
#line 2320 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 90: /* stmt: ICONMGR_NOSHOW  */
#line 276 "gram.y"
                                        { Scr->IconManagerDontShow = true; }
#line 2326 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 91: /* $@30: %empty  */
#line 277 "gram.y"
                                        { curplist = &Scr->IconMgrs; }
#line 2332 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 93: /* $@31: %empty  */
#line 279 "gram.y"
                                        { curplist = &Scr->IconMgrShow; }
#line 2338 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 95: /* $@32: %empty  */
#line 281 "gram.y"
                                        { curplist = &Scr->NoTitleHighlight; }
#line 2344 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 97: /* stmt: NO_TITLE_HILITE  */
#line 283 "gram.y"
                                        { if (Scr->FirstTime)
						Scr->TitleHighlight = false; }
#line 2351 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 98: /* $@33: %empty  */
#line 285 "gram.y"
                                        { curplist = &Scr->NoHighlight; }
#line 2357 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 100: /* stmt: NO_HILITE  */
#line 287 "gram.y"
                                        { if (Scr->FirstTime)
						Scr->Highlight = false; }
#line 2364 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 101: /* stmt: ON_TOP_PRIORITY signed_number  */
#line 290 "gram.y"
                                        { OtpScrSetZero(Scr, WinWin, (yyvsp[0].num)); }
#line 2370 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 102: /* stmt: ON_TOP_PRIORITY ICONS signed_number  */
#line 292 "gram.y"
                                        { OtpScrSetZero(Scr, IconWin, (yyvsp[0].num)); }
#line 2376 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 103: /* $@34: %empty  */
#line 294 "gram.y"
                                        { curplist = OtpScrPriorityL(Scr, WinWin, (yyvsp[0].num)); }
#line 2382 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 105: /* $@35: %empty  */
#line 297 "gram.y"
                                        { curplist = OtpScrPriorityL(Scr, IconWin, (yyvsp[0].num)); }
#line 2388 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 107: /* $@36: %empty  */
#line 299 "gram.y"
                                        { curplist = OtpScrPriorityL(Scr, WinWin, 8); }
#line 2394 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 109: /* $@37: %empty  */
#line 301 "gram.y"
                                        { OtpScrSetSwitching(Scr, WinWin, false);
		                          curplist = OtpScrSwitchingL(Scr, WinWin); }
#line 2401 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 111: /* $@38: %empty  */
#line 304 "gram.y"
                                         { OtpScrSetSwitching(Scr, WinWin, true);
		                          curplist = OtpScrSwitchingL(Scr, WinWin); }
#line 2408 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 113: /* $@39: %empty  */
#line 308 "gram.y"
                                        { OtpScrSetSwitching(Scr, IconWin, false);
                                        curplist = OtpScrSwitchingL(Scr, IconWin); }
#line 2415 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 115: /* $@40: %empty  */
#line 312 "gram.y"
                                        { OtpScrSetSwitching(Scr, IconWin, true);
		                          curplist = OtpScrSwitchingL(Scr, IconWin); }
#line 2422 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 117: /* $@41: %empty  */
#line 317 "gram.y"
                                        { curplist = &Scr->NoStackModeL; }
#line 2428 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 119: /* stmt: NO_STACKMODE  */
#line 319 "gram.y"
                                        { if (Scr->FirstTime)
						Scr->StackMode = false; }
#line 2435 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 120: /* $@42: %empty  */
#line 321 "gram.y"
                                        { curplist = &Scr->NoBorder; }
#line 2441 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 122: /* stmt: AUTO_POPUP  */
#line 323 "gram.y"
                                        { Scr->AutoPopup = true; }
#line 2447 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 123: /* $@43: %empty  */
#line 324 "gram.y"
                                        { curplist = &Scr->AutoPopupL; }
#line 2453 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 125: /* $@44: %empty  */
#line 326 "gram.y"
                                        {
#ifndef SESSION
			twmrc_error_prefix();
			fprintf(stderr, "DontSave ignored; session support "
					"disabled.\n");
#endif
				curplist = &Scr->DontSave;
			}
#line 2466 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 127: /* $@45: %empty  */
#line 335 "gram.y"
                                        { curplist = &Scr->NoIconTitle; }
#line 2472 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 129: /* stmt: NO_ICON_TITLE  */
#line 337 "gram.y"
                                        { if (Scr->FirstTime)
						Scr->NoIconTitlebar = true; }
#line 2479 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 130: /* $@46: %empty  */
#line 339 "gram.y"
                                        { curplist = &Scr->NoTitle; }
#line 2485 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 132: /* stmt: NO_TITLE  */
#line 341 "gram.y"
                                        { if (Scr->FirstTime)
						Scr->NoTitlebar = true; }
#line 2492 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 133: /* $@47: %empty  */
#line 343 "gram.y"
                                        { curplist = &Scr->IgnoreTransientL; }
#line 2498 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 135: /* $@48: %empty  */
#line 345 "gram.y"
                                        { curplist = &Scr->MakeTitle; }
#line 2504 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 137: /* $@49: %empty  */
#line 347 "gram.y"
                                        { curplist = &Scr->StartIconified; }
#line 2510 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 139: /* $@50: %empty  */
#line 349 "gram.y"
                                        { curplist = &Scr->AutoRaise; }
#line 2516 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 141: /* stmt: AUTO_RAISE  */
#line 351 "gram.y"
                                        { Scr->AutoRaiseDefault = true; }
#line 2522 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 142: /* $@51: %empty  */
#line 352 "gram.y"
                                        { curplist = &Scr->WarpOnDeIconify; }
#line 2528 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 144: /* $@52: %empty  */
#line 354 "gram.y"
                                        { curplist = &Scr->AutoLower; }
#line 2534 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 146: /* stmt: AUTO_LOWER  */
#line 356 "gram.y"
                                        { Scr->AutoLowerDefault = true; }
#line 2540 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 147: /* $@53: %empty  */
#line 357 "gram.y"
                                                        {
					root = GetRoot((yyvsp[-5].ptr), (yyvsp[-3].ptr), (yyvsp[-1].ptr)); }
#line 2547 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 148: /* stmt: MENU string LP string COLON string RP $@53 menu  */
#line 359 "gram.y"
                                        { root->real_menu = true;}
#line 2553 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 149: /* $@54: %empty  */
#line 360 "gram.y"
                                        { root = GetRoot((yyvsp[0].ptr), NULL, NULL); }
#line 2559 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 150: /* stmt: MENU string $@54 menu  */
#line 361 "gram.y"
                                        { root->real_menu = true; }
#line 2565 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 151: /* $@55: %empty  */
#line 362 "gram.y"
                                        { root = GetRoot((yyvsp[0].ptr), NULL, NULL); }
#line 2571 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 153: /* $@56: %empty  */
#line 364 "gram.y"
                                        { curplist = &Scr->IconNames; }
#line 2577 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 155: /* $@57: %empty  */
#line 366 "gram.y"
                                        { color = COLOR; }
#line 2583 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 157: /* $@58: %empty  */
#line 368 "gram.y"
                                        {}
#line 2589 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 159: /* $@59: %empty  */
#line 370 "gram.y"
                                        { color = MONOCHROME; }
#line 2595 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 161: /* stmt: DEFAULT_FUNCTION action  */
#line 372 "gram.y"
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
#line 2616 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 162: /* stmt: WINDOW_FUNCTION action  */
#line 388 "gram.y"
                                         { Scr->WindowFunction.func = (yyvsp[0].num);
					   root = GetRoot(TWM_ROOT,NULL,NULL);
					   Scr->WindowFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,(yyvsp[0].num), NULL, NULL);
					   Action = "";
					   pull = NULL;
					}
#line 2629 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 163: /* stmt: CHANGE_WORKSPACE_FUNCTION action  */
#line 396 "gram.y"
                                                   { Scr->ChangeWorkspaceFunction.func = (yyvsp[0].num);
					   root = GetRoot(TWM_ROOT,NULL,NULL);
					   Scr->ChangeWorkspaceFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,(yyvsp[0].num), NULL, NULL);
					   Action = "";
					   pull = NULL;
					}
#line 2642 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 164: /* stmt: DEICONIFY_FUNCTION action  */
#line 404 "gram.y"
                                            { Scr->DeIconifyFunction.func = (yyvsp[0].num);
					   root = GetRoot(TWM_ROOT,NULL,NULL);
					   Scr->DeIconifyFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,(yyvsp[0].num), NULL, NULL);
					   Action = "";
					   pull = NULL;
					}
#line 2655 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 165: /* stmt: ICONIFY_FUNCTION action  */
#line 412 "gram.y"
                                          { Scr->IconifyFunction.func = (yyvsp[0].num);
					   root = GetRoot(TWM_ROOT,NULL,NULL);
					   Scr->IconifyFunction.item =
						AddToMenu(root,"x",Action,
							  NULL,(yyvsp[0].num), NULL, NULL);
					   Action = "";
					   pull = NULL;
					}
#line 2668 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 166: /* $@60: %empty  */
#line 420 "gram.y"
                                        { curplist = &Scr->WarpCursorL; }
#line 2674 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 168: /* stmt: WARP_CURSOR  */
#line 422 "gram.y"
                                        { if (Scr->FirstTime)
					    Scr->WarpCursor = true; }
#line 2681 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 169: /* $@61: %empty  */
#line 424 "gram.y"
                                        { curplist = &Scr->WindowRingL; }
#line 2687 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 171: /* stmt: WINDOW_RING  */
#line 426 "gram.y"
                                        { Scr->WindowRingAll = true; }
#line 2693 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 172: /* $@62: %empty  */
#line 427 "gram.y"
                                        { if (!Scr->WindowRingL)
					    Scr->WindowRingAll = true;
					  curplist = &Scr->WindowRingExcludeL; }
#line 2701 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 174: /* $@63: %empty  */
#line 431 "gram.y"
                                        {  }
#line 2707 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 176: /* $@64: %empty  */
#line 433 "gram.y"
                                        { }
#line 2713 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 178: /* $@65: %empty  */
#line 435 "gram.y"
                                        { }
#line 2719 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 180: /* $@66: %empty  */
#line 437 "gram.y"
                                        { }
#line 2725 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 182: /* $@67: %empty  */
#line 439 "gram.y"
                                 { init_layout_override(); }
#line 2731 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 184: /* $@68: %empty  */
#line 441 "gram.y"
                               { }
#line 2737 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 186: /* stmt: FORCE_FOCUS  */
#line 443 "gram.y"
                              { Scr->ForceFocus = true; }
#line 2743 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 187: /* $@69: %empty  */
#line 444 "gram.y"
                              { curplist = &Scr->ForceFocusL; }
#line 2749 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 189: /* noarg: KEYWORD  */
#line 448 "gram.y"
                                        { if (!do_single_keyword ((yyvsp[0].num))) {
					    twmrc_error_prefix();
					    fprintf (stderr,
					"unknown singleton keyword %d\n",
						     (yyvsp[0].num));
					    ParseError = true;
					  }
					}
#line 2762 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 190: /* sarg: SKEYWORD string  */
#line 458 "gram.y"
                                        { if (!do_string_keyword ((yyvsp[-1].num), (yyvsp[0].ptr))) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (value \"%s\")\n",
						     (yyvsp[-1].num), (yyvsp[0].ptr));
					    ParseError = true;
					  }
					}
#line 2775 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 191: /* sarg: SKEYWORD  */
#line 466 "gram.y"
                                        { if (!do_string_keyword ((yyvsp[0].num), DEFSTRING)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (no value)\n",
						     (yyvsp[0].num));
					    ParseError = true;
					  }
					}
#line 2788 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 192: /* sarg: SSKEYWORD string string  */
#line 477 "gram.y"
                                        { if (!do_string_string_keyword ((yyvsp[-2].num), (yyvsp[-1].ptr), (yyvsp[0].ptr))) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown strings keyword %d (value \"%s\" and \"%s\")\n",
						     (yyvsp[-2].num), (yyvsp[-1].ptr), (yyvsp[0].ptr));
					    ParseError = true;
					  }
					}
#line 2801 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 193: /* sarg: SSKEYWORD string  */
#line 485 "gram.y"
                                        { if (!do_string_string_keyword ((yyvsp[-1].num), (yyvsp[0].ptr), NULL)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (value \"%s\")\n",
						     (yyvsp[-1].num), (yyvsp[0].ptr));
					    ParseError = true;
					  }
					}
#line 2814 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 194: /* sarg: SSKEYWORD  */
#line 493 "gram.y"
                                        { if (!do_string_string_keyword ((yyvsp[0].num), NULL, NULL)) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown string keyword %d (no value)\n",
						     (yyvsp[0].num));
					    ParseError = true;
					  }
					}
#line 2827 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 195: /* narg: NKEYWORD number  */
#line 503 "gram.y"
                                        { if (!do_number_keyword ((yyvsp[-1].num), (yyvsp[0].num))) {
					    twmrc_error_prefix();
					    fprintf (stderr,
				"unknown numeric keyword %d (value %d)\n",
						     (yyvsp[-1].num), (yyvsp[0].num));
					    ParseError = true;
					  }
					}
#line 2840 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 196: /* keyaction: EQUALS keys COLON action  */
#line 515 "gram.y"
                                            { (yyval.num) = (yyvsp[0].num); }
#line 2846 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 197: /* full: EQUALS keys COLON contexts COLON action  */
#line 518 "gram.y"
                                                           { (yyval.num) = (yyvsp[0].num); }
#line 2852 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 198: /* fullkey: EQUALS keys COLON contextkeys COLON action  */
#line 521 "gram.y"
                                                              { (yyval.num) = (yyvsp[0].num); }
#line 2858 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 201: /* key: META  */
#line 528 "gram.y"
                                        { mods |= Mod1Mask; }
#line 2864 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 202: /* key: SHIFT  */
#line 529 "gram.y"
                                        { mods |= ShiftMask; }
#line 2870 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 203: /* key: LOCK  */
#line 530 "gram.y"
                                        { mods |= LockMask; }
#line 2876 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 204: /* key: CONTROL  */
#line 531 "gram.y"
                                        { mods |= ControlMask; }
#line 2882 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 205: /* key: ALTER number  */
#line 532 "gram.y"
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
#line 2897 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 206: /* key: META number  */
#line 542 "gram.y"
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
#line 2912 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 207: /* key: OR  */
#line 552 "gram.y"
                                        { }
#line 2918 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 208: /* vgrav: GRAVITY  */
#line 555 "gram.y"
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
#line 2937 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 209: /* hgrav: GRAVITY  */
#line 570 "gram.y"
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
#line 2956 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 212: /* context: WINDOW  */
#line 589 "gram.y"
                                        { cont |= C_WINDOW_BIT; }
#line 2962 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 213: /* context: TITLE  */
#line 590 "gram.y"
                                        { cont |= C_TITLE_BIT; }
#line 2968 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 214: /* context: ICON  */
#line 591 "gram.y"
                                        { cont |= C_ICON_BIT; }
#line 2974 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 215: /* context: ROOT  */
#line 592 "gram.y"
                                        { cont |= C_ROOT_BIT; }
#line 2980 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 216: /* context: FRAME  */
#line 593 "gram.y"
                                        { cont |= C_FRAME_BIT; }
#line 2986 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 217: /* context: WORKSPACE  */
#line 594 "gram.y"
                                        { cont |= C_WORKSPACE_BIT; }
#line 2992 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 218: /* context: ICONMGR  */
#line 595 "gram.y"
                                        { cont |= C_ICONMGR_BIT; }
#line 2998 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 219: /* context: META  */
#line 596 "gram.y"
                                        { cont |= C_ICONMGR_BIT; }
#line 3004 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 220: /* context: ALTER  */
#line 597 "gram.y"
                                        { cont |= C_ALTER_BIT; }
#line 3010 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 221: /* context: ALL  */
#line 598 "gram.y"
                                        { cont |= C_ALL_BITS; }
#line 3016 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 222: /* context: OR  */
#line 599 "gram.y"
                                        { }
#line 3022 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 225: /* contextkey: WINDOW  */
#line 606 "gram.y"
                                        { cont |= C_WINDOW_BIT; }
#line 3028 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 226: /* contextkey: TITLE  */
#line 607 "gram.y"
                                        { cont |= C_TITLE_BIT; }
#line 3034 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 227: /* contextkey: ICON  */
#line 608 "gram.y"
                                        { cont |= C_ICON_BIT; }
#line 3040 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 228: /* contextkey: ROOT  */
#line 609 "gram.y"
                                        { cont |= C_ROOT_BIT; }
#line 3046 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 229: /* contextkey: FRAME  */
#line 610 "gram.y"
                                        { cont |= C_FRAME_BIT; }
#line 3052 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 230: /* contextkey: WORKSPACE  */
#line 611 "gram.y"
                                        { cont |= C_WORKSPACE_BIT; }
#line 3058 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 231: /* contextkey: ICONMGR  */
#line 612 "gram.y"
                                        { cont |= C_ICONMGR_BIT; }
#line 3064 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 232: /* contextkey: META  */
#line 613 "gram.y"
                                        { cont |= C_ICONMGR_BIT; }
#line 3070 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 233: /* contextkey: ALTER  */
#line 614 "gram.y"
                                        { cont |= C_ALTER_BIT; }
#line 3076 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 234: /* contextkey: ALL  */
#line 615 "gram.y"
                                        { cont |= C_ALL_BITS; }
#line 3082 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 235: /* contextkey: OR  */
#line 616 "gram.y"
                                        { }
#line 3088 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 236: /* contextkey: string  */
#line 617 "gram.y"
                                        { Name = (char*)(yyvsp[0].ptr); cont |= C_NAME_BIT; }
#line 3094 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 237: /* binding_list: LB binding_entries RB  */
#line 621 "gram.y"
                                        {}
#line 3100 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 240: /* binding_entry: button keyaction  */
#line 628 "gram.y"
                                   { SetCurrentTBAction((yyvsp[-1].num), mods, (yyvsp[0].num), Action, pull); mods = 0;}
#line 3106 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 241: /* binding_entry: button EQUALS action  */
#line 629 "gram.y"
                                       { SetCurrentTBAction((yyvsp[-2].num), 0, (yyvsp[0].num), Action, pull);}
#line 3112 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 242: /* binding_entry: button COLON action  */
#line 630 "gram.y"
                                      {
			/* Deprecated since 3.8, no longer supported */
			yyerror("Title buttons specifications without = are no "
			        "longer supported.");
		}
#line 3122 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 243: /* pixmap_list: LB pixmap_entries RB  */
#line 638 "gram.y"
                                       {}
#line 3128 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 246: /* pixmap_entry: TITLE_HILITE string  */
#line 645 "gram.y"
                                      { Scr->HighlightPixmapName = strdup((yyvsp[0].ptr)); }
#line 3134 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 247: /* cursor_list: LB cursor_entries RB  */
#line 649 "gram.y"
                                       {}
#line 3140 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 250: /* cursor_entry: FRAME string string  */
#line 656 "gram.y"
                                      {
			NewBitmapCursor(&Scr->FrameCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3147 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 251: /* cursor_entry: FRAME string  */
#line 658 "gram.y"
                                {
			NewFontCursor(&Scr->FrameCursor, (yyvsp[0].ptr)); }
#line 3154 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 252: /* cursor_entry: TITLE string string  */
#line 660 "gram.y"
                                      {
			NewBitmapCursor(&Scr->TitleCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3161 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 253: /* cursor_entry: TITLE string  */
#line 662 "gram.y"
                               {
			NewFontCursor(&Scr->TitleCursor, (yyvsp[0].ptr)); }
#line 3168 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 254: /* cursor_entry: ICON string string  */
#line 664 "gram.y"
                                     {
			NewBitmapCursor(&Scr->IconCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3175 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 255: /* cursor_entry: ICON string  */
#line 666 "gram.y"
                              {
			NewFontCursor(&Scr->IconCursor, (yyvsp[0].ptr)); }
#line 3182 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 256: /* cursor_entry: ICONMGR string string  */
#line 668 "gram.y"
                                        {
			NewBitmapCursor(&Scr->IconMgrCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3189 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 257: /* cursor_entry: ICONMGR string  */
#line 670 "gram.y"
                                 {
			NewFontCursor(&Scr->IconMgrCursor, (yyvsp[0].ptr)); }
#line 3196 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 258: /* cursor_entry: BUTTON string string  */
#line 672 "gram.y"
                                       {
			NewBitmapCursor(&Scr->ButtonCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3203 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 259: /* cursor_entry: BUTTON string  */
#line 674 "gram.y"
                                {
			NewFontCursor(&Scr->ButtonCursor, (yyvsp[0].ptr)); }
#line 3210 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 260: /* cursor_entry: MOVE string string  */
#line 676 "gram.y"
                                     {
			NewBitmapCursor(&Scr->MoveCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3217 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 261: /* cursor_entry: MOVE string  */
#line 678 "gram.y"
                              {
			NewFontCursor(&Scr->MoveCursor, (yyvsp[0].ptr)); }
#line 3224 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 262: /* cursor_entry: RESIZE string string  */
#line 680 "gram.y"
                                       {
			NewBitmapCursor(&Scr->ResizeCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3231 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 263: /* cursor_entry: RESIZE string  */
#line 682 "gram.y"
                                {
			NewFontCursor(&Scr->ResizeCursor, (yyvsp[0].ptr)); }
#line 3238 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 264: /* cursor_entry: WAITC string string  */
#line 684 "gram.y"
                                      {
			NewBitmapCursor(&Scr->WaitCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3245 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 265: /* cursor_entry: WAITC string  */
#line 686 "gram.y"
                               {
			NewFontCursor(&Scr->WaitCursor, (yyvsp[0].ptr)); }
#line 3252 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 266: /* cursor_entry: MENU string string  */
#line 688 "gram.y"
                                     {
			NewBitmapCursor(&Scr->MenuCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3259 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 267: /* cursor_entry: MENU string  */
#line 690 "gram.y"
                              {
			NewFontCursor(&Scr->MenuCursor, (yyvsp[0].ptr)); }
#line 3266 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 268: /* cursor_entry: SELECT string string  */
#line 692 "gram.y"
                                       {
			NewBitmapCursor(&Scr->SelectCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3273 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 269: /* cursor_entry: SELECT string  */
#line 694 "gram.y"
                                {
			NewFontCursor(&Scr->SelectCursor, (yyvsp[0].ptr)); }
#line 3280 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 270: /* cursor_entry: KILL string string  */
#line 696 "gram.y"
                                     {
			NewBitmapCursor(&Scr->DestroyCursor, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3287 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 271: /* cursor_entry: KILL string  */
#line 698 "gram.y"
                              {
			NewFontCursor(&Scr->DestroyCursor, (yyvsp[0].ptr)); }
#line 3294 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 272: /* color_list: LB color_entries RB  */
#line 702 "gram.y"
                                      {}
#line 3300 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 275: /* color_entry: CLKEYWORD string  */
#line 710 "gram.y"
                                        { if (!do_colorlist_keyword ((yyvsp[-1].num), color,
								     (yyvsp[0].ptr))) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled list color keyword %d (string \"%s\")\n",
						     (yyvsp[-1].num), (yyvsp[0].ptr));
					    ParseError = true;
					  }
					}
#line 3314 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 276: /* $@70: %empty  */
#line 719 "gram.y"
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
#line 3329 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 278: /* color_entry: CKEYWORD string  */
#line 730 "gram.y"
                                        { if (!do_color_keyword ((yyvsp[-1].num), color,
								 (yyvsp[0].ptr))) {
					    twmrc_error_prefix();
					    fprintf (stderr,
			"unhandled color keyword %d (string \"%s\")\n",
						     (yyvsp[-1].num), (yyvsp[0].ptr));
					    ParseError = true;
					  }
					}
#line 3343 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 279: /* save_color_list: LB s_color_entries RB  */
#line 741 "gram.y"
                                        {}
#line 3349 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 282: /* s_color_entry: string  */
#line 748 "gram.y"
                                        { do_string_savecolor(color, (yyvsp[0].ptr)); }
#line 3355 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 283: /* s_color_entry: CLKEYWORD  */
#line 749 "gram.y"
                                        { do_var_savecolor((yyvsp[0].num)); }
#line 3361 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 284: /* win_color_list: LB win_color_entries RB  */
#line 752 "gram.y"
                                          {}
#line 3367 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 287: /* win_color_entry: string string  */
#line 759 "gram.y"
                                        { if (Scr->FirstTime &&
					      color == Scr->Monochrome)
					    AddToList(curplist, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3375 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 288: /* wingeom_list: LB wingeom_entries RB  */
#line 764 "gram.y"
                                        {}
#line 3381 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 291: /* wingeom_entry: string string  */
#line 771 "gram.y"
                                { AddToList (&Scr->WindowGeometries, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3387 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 292: /* vscreen_geom_list: LB vscreen_geom_entries RB  */
#line 774 "gram.y"
                                                     {}
#line 3393 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 295: /* vscreen_geom_entry: string  */
#line 781 "gram.y"
                                 {
#ifdef VSCREEN
				   AddToList (&Scr->VirtualScreens, (yyvsp[0].ptr), "");
#endif
				   }
#line 3403 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 296: /* ewmh_ignore_list: LB ewmh_ignore_entries RB  */
#line 789 "gram.y"
                                                    { proc_ewmh_ignore(); }
#line 3409 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 299: /* ewmh_ignore_entry: string  */
#line 796 "gram.y"
                                 { add_ewmh_ignore((yyvsp[0].ptr)); }
#line 3415 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 300: /* mwm_ignore_list: LB mwm_ignore_entries RB  */
#line 800 "gram.y"
                                           { proc_mwm_ignore(); }
#line 3421 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 303: /* mwm_ignore_entry: string  */
#line 807 "gram.y"
                                 { add_mwm_ignore((yyvsp[0].ptr)); }
#line 3427 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 304: /* layout_geom_list: LB layout_geom_entries RB  */
#line 811 "gram.y"
                                                    { proc_layout_override(); }
#line 3433 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 307: /* layout_geom_entry: string  */
#line 818 "gram.y"
                                 { add_layout_override_entry((yyvsp[0].ptr)); }
#line 3439 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 308: /* squeeze: SQUEEZE_TITLE  */
#line 822 "gram.y"
                                {
				    if (HasShape) Scr->SqueezeTitle = true;
				}
#line 3447 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 309: /* $@71: %empty  */
#line 825 "gram.y"
                                { curplist = &Scr->SqueezeTitleL;
				  if (HasShape)
				    Scr->SqueezeTitle = true;
				}
#line 3456 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 311: /* squeeze: DONT_SQUEEZE_TITLE  */
#line 830 "gram.y"
                                     { Scr->SqueezeTitle = false; }
#line 3462 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 312: /* $@72: %empty  */
#line 831 "gram.y"
                                     { curplist = &Scr->DontSqueezeTitleL; }
#line 3468 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 315: /* win_sqz_entries: win_sqz_entries string SIJENUM signed_number number  */
#line 836 "gram.y"
                                                                        {
				if (Scr->FirstTime) {
				   do_squeeze_entry (curplist, (yyvsp[-3].ptr), (yyvsp[-2].num), (yyvsp[-1].num), (yyvsp[0].num));
				}
			}
#line 3478 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 316: /* iconm_list: LB iconm_entries RB  */
#line 844 "gram.y"
                                      {}
#line 3484 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 319: /* iconm_entry: string string number  */
#line 851 "gram.y"
                                        { if (Scr->FirstTime)
					    AddToList(curplist, (yyvsp[-2].ptr),
						AllocateIconManager((yyvsp[-2].ptr), NULL,
							(yyvsp[-1].ptr),(yyvsp[0].num)));
					}
#line 3494 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 320: /* iconm_entry: string string string number  */
#line 857 "gram.y"
                                        { if (Scr->FirstTime)
					    AddToList(curplist, (yyvsp[-3].ptr),
						AllocateIconManager((yyvsp[-3].ptr),(yyvsp[-2].ptr),
						(yyvsp[-1].ptr), (yyvsp[0].num)));
					}
#line 3504 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 321: /* workspc_list: LB workspc_entries RB  */
#line 864 "gram.y"
                                        {}
#line 3510 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 324: /* workspc_entry: string  */
#line 871 "gram.y"
                                {
			AddWorkSpace ((yyvsp[0].ptr), NULL, NULL, NULL, NULL, NULL);
		}
#line 3518 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 325: /* $@73: %empty  */
#line 874 "gram.y"
                                {
			curWorkSpc = (char*)(yyvsp[0].ptr);
		}
#line 3526 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 327: /* workapp_list: LB workapp_entries RB  */
#line 880 "gram.y"
                                        {}
#line 3532 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 330: /* workapp_entry: string  */
#line 887 "gram.y"
                                        {
			AddWorkSpace (curWorkSpc, (yyvsp[0].ptr), NULL, NULL, NULL, NULL);
		}
#line 3540 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 331: /* workapp_entry: string string  */
#line 890 "gram.y"
                                        {
			AddWorkSpace (curWorkSpc, (yyvsp[-1].ptr), (yyvsp[0].ptr), NULL, NULL, NULL);
		}
#line 3548 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 332: /* workapp_entry: string string string  */
#line 893 "gram.y"
                                        {
			AddWorkSpace (curWorkSpc, (yyvsp[-2].ptr), (yyvsp[-1].ptr), (yyvsp[0].ptr), NULL, NULL);
		}
#line 3556 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 333: /* workapp_entry: string string string string  */
#line 896 "gram.y"
                                                {
			AddWorkSpace (curWorkSpc, (yyvsp[-3].ptr), (yyvsp[-2].ptr), (yyvsp[-1].ptr), (yyvsp[0].ptr), NULL);
		}
#line 3564 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 334: /* workapp_entry: string string string string string  */
#line 899 "gram.y"
                                                        {
			AddWorkSpace (curWorkSpc, (yyvsp[-4].ptr), (yyvsp[-3].ptr), (yyvsp[-2].ptr), (yyvsp[-1].ptr), (yyvsp[0].ptr));
		}
#line 3572 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 335: /* curwork: LB string RB  */
#line 904 "gram.y"
                               {
		    WMapCreateCurrentBackGround ((yyvsp[-1].ptr), NULL, NULL, NULL);
		}
#line 3580 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 336: /* curwork: LB string string RB  */
#line 907 "gram.y"
                                      {
		    WMapCreateCurrentBackGround ((yyvsp[-2].ptr), (yyvsp[-1].ptr), NULL, NULL);
		}
#line 3588 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 337: /* curwork: LB string string string RB  */
#line 910 "gram.y"
                                             {
		    WMapCreateCurrentBackGround ((yyvsp[-3].ptr), (yyvsp[-2].ptr), (yyvsp[-1].ptr), NULL);
		}
#line 3596 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 338: /* curwork: LB string string string string RB  */
#line 913 "gram.y"
                                                    {
		    WMapCreateCurrentBackGround ((yyvsp[-4].ptr), (yyvsp[-3].ptr), (yyvsp[-2].ptr), (yyvsp[-1].ptr));
		}
#line 3604 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 339: /* defwork: LB string RB  */
#line 918 "gram.y"
                               {
		    WMapCreateDefaultBackGround ((yyvsp[-1].ptr), NULL, NULL, NULL);
		}
#line 3612 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 340: /* defwork: LB string string RB  */
#line 921 "gram.y"
                                      {
		    WMapCreateDefaultBackGround ((yyvsp[-2].ptr), (yyvsp[-1].ptr), NULL, NULL);
		}
#line 3620 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 341: /* defwork: LB string string string RB  */
#line 924 "gram.y"
                                             {
		    WMapCreateDefaultBackGround ((yyvsp[-3].ptr), (yyvsp[-2].ptr), (yyvsp[-1].ptr), NULL);
		}
#line 3628 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 342: /* defwork: LB string string string string RB  */
#line 927 "gram.y"
                                                    {
		    WMapCreateDefaultBackGround ((yyvsp[-4].ptr), (yyvsp[-3].ptr), (yyvsp[-2].ptr), (yyvsp[-1].ptr));
		}
#line 3636 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 343: /* win_list: LB win_entries RB  */
#line 932 "gram.y"
                                    {}
#line 3642 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 346: /* win_entry: string  */
#line 939 "gram.y"
                                        { if (Scr->FirstTime)
					    AddToList(curplist, (yyvsp[0].ptr), 0);
					}
#line 3650 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 347: /* occupy_list: LB occupy_entries RB  */
#line 944 "gram.y"
                                       {}
#line 3656 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 350: /* $@74: %empty  */
#line 951 "gram.y"
                         {client = (char*)(yyvsp[0].ptr);}
#line 3662 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 352: /* $@75: %empty  */
#line 953 "gram.y"
                                   {client = (char*)(yyvsp[0].ptr);}
#line 3668 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 354: /* $@76: %empty  */
#line 955 "gram.y"
                                   {workspace = (char*)(yyvsp[0].ptr);}
#line 3674 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 356: /* occupy_workspc_list: LB occupy_workspc_entries RB  */
#line 959 "gram.y"
                                                       {}
#line 3680 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 359: /* occupy_workspc_entry: string  */
#line 966 "gram.y"
                                 {
				if(!AddToClientsList ((yyvsp[0].ptr), client)) {
					twmrc_error_prefix();
					fprintf(stderr, "unknown workspace '%s'\n", (yyvsp[0].ptr));
				}
			  }
#line 3691 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 360: /* occupy_window_list: LB occupy_window_entries RB  */
#line 974 "gram.y"
                                                      {}
#line 3697 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 363: /* occupy_window_entry: string  */
#line 981 "gram.y"
                                 {
				if(!AddToClientsList (workspace, (yyvsp[0].ptr))) {
					twmrc_error_prefix();
					fprintf(stderr, "unknown workspace '%s'\n", workspace);
				}
			  }
#line 3708 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 364: /* icon_list: LB icon_entries RB  */
#line 989 "gram.y"
                                     {}
#line 3714 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 367: /* icon_entry: string string  */
#line 996 "gram.y"
                                        { if (Scr->FirstTime) AddToList(curplist, (yyvsp[-1].ptr), (yyvsp[0].ptr)); }
#line 3720 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 368: /* rplay_sounds_list: LB rplay_sounds_entries RB  */
#line 999 "gram.y"
                                                     {
#ifndef SOUNDS
			twmrc_error_prefix();
			fprintf(stderr, "RplaySounds ignored; rplay support "
					"not configured.\n");
#else
			sound_set_from_config();
#endif
		}
#line 3734 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 371: /* rplay_sounds_entry: string string  */
#line 1014 "gram.y"
                                        {
#ifdef SOUNDS
			if(set_sound_event_name((yyvsp[-1].ptr), (yyvsp[0].ptr)) != 0) {
				twmrc_error_prefix();
				fprintf(stderr, "Failed adding sound for %s; "
						"maybe event name is invalid?\n", (yyvsp[-1].ptr));
			}
#endif
		}
#line 3748 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 372: /* function: LB function_entries RB  */
#line 1025 "gram.y"
                                         {}
#line 3754 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 375: /* function_entry: action  */
#line 1032 "gram.y"
                                        { AddToMenu(root, "", Action, NULL, (yyvsp[0].num),
						    NULL, NULL);
					  Action = "";
					}
#line 3763 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 376: /* menu: LB menu_entries RB  */
#line 1038 "gram.y"
                                     {lastmenuitem = NULL;}
#line 3769 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 379: /* menu_entry: string action  */
#line 1045 "gram.y"
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
#line 3784 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 380: /* menu_entry: string LP string COLON string RP action  */
#line 1055 "gram.y"
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
#line 3799 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 381: /* action: FKEYWORD  */
#line 1067 "gram.y"
                                { (yyval.num) = (yyvsp[0].num); }
#line 3805 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 382: /* action: FSKEYWORD string  */
#line 1068 "gram.y"
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
				    break;
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
#line 3849 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 383: /* signed_number: number  */
#line 1110 "gram.y"
                                        { (yyval.num) = (yyvsp[0].num); }
#line 3855 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 384: /* signed_number: PLUS number  */
#line 1111 "gram.y"
                                        { (yyval.num) = (yyvsp[0].num); }
#line 3861 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 385: /* signed_number: MINUS number  */
#line 1112 "gram.y"
                                        { (yyval.num) = -((yyvsp[0].num)); }
#line 3867 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 386: /* button: BUTTON number  */
#line 1115 "gram.y"
                                        { (yyval.num) = (yyvsp[0].num);
					  if ((yyvsp[0].num) == 0)
						yyerror("bad button 0");

					  if ((yyvsp[0].num) > MAX_BUTTONS)
					  {
						(yyval.num) = 0;
						yyerror("button number too large");
					  }
					}
#line 3882 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 387: /* string: STRING  */
#line 1127 "gram.y"
                                        { char *ptr = strdup((yyvsp[0].ptr));
					  RemoveDQuote(ptr);
					  (yyval.ptr) = ptr;
					}
#line 3891 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;

  case 388: /* number: NUMBER  */
#line 1133 "gram.y"
                                        { (yyval.num) = (yyvsp[0].num); }
#line 3897 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"
    break;


#line 3901 "/home/fullermd/work/ctwm/bzr/dev/ctwm-mktar.4Y0T7b/ctwm-4.1.0/build/gram.tab.c"

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

#line 1136 "gram.y"

