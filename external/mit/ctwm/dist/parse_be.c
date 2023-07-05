/*
 * Parser backend routines.
 *
 * Roughly, these are the routines that the lex/yacc output calls to do
 * its work.
 *
 * This is very similar to the meaning of parse_yacc.c; the two may be
 * merged at some point.
 */

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <X11/Xatom.h>

#include "ctwm_atoms.h"
#include "screen.h"
#include "util.h"
#include "animate.h"
#include "functions_defs.h"
#include "image.h"
#include "list.h"
#include "occupation.h"
#include "parse.h"
#include "parse_be.h"
#include "parse_yacc.h"
#include "r_area.h"
#include "r_area_list.h"
#include "r_layout.h"
#ifdef SOUNDS
#  include "sound.h"
#endif

#include "gram.tab.h"


static int ParseRandomPlacement(const char *s);
static int ParseButtonStyle(const char *s);
static int ParseUsePPosition(const char *s);
static int ParseIconifyStyle(const char *s);



/**********************************************************************
 *
 *  Parsing table and routines
 *
 ***********************************************************************/

typedef struct _TwmKeyword {
	const char *name;
	int value;
	int subnum;
} TwmKeyword;

#define kw0_NoDefaults                  1
#define kw0_AutoRelativeResize          2
#define kw0_ForceIcons                  3
#define kw0_NoIconManagers              4
#define kw0_InterpolateMenuColors       6
//#define kw0_NoVersion                 7
#define kw0_SortIconManager             8
#define kw0_NoGrabServer                9
#define kw0_NoMenuShadows               10
#define kw0_NoRaiseOnMove               11
#define kw0_NoRaiseOnResize             12
#define kw0_NoRaiseOnDeiconify          13
#define kw0_DontMoveOff                 14
#define kw0_NoBackingStore              15
#define kw0_NoSaveUnders                16
#define kw0_RestartPreviousState        17
#define kw0_ClientBorderWidth           18
#define kw0_NoTitleFocus                19
#define kw0_DecorateTransients          21
#define kw0_ShowIconManager             22
#define kw0_NoCaseSensitive             23
#define kw0_NoRaiseOnWarp               24
#define kw0_WarpUnmapped                25
#define kw0_ShowWorkspaceManager        27
#define kw0_StartInMapState             28
#define kw0_NoShowOccupyAll             29
#define kw0_AutoOccupy                  30
#define kw0_TransientHasOccupation      31
#define kw0_DontPaintRootWindow         32
#define kw0_Use3DMenus                  33
#define kw0_Use3DTitles                 34
#define kw0_Use3DIconManagers           35
#define kw0_Use3DBorders                36
#define kw0_SunkFocusWindowTitle        37
#define kw0_BeNiceToColormap            38
#define kw0_WarpRingOnScreen            40
#define kw0_NoIconManagerFocus          41
#define kw0_StayUpMenus                 42
#define kw0_ClickToFocus                43
#define kw0_BorderResizeCursors         44
#define kw0_ReallyMoveInWorkspaceManager 45
#define kw0_ShowWinWhenMovingInWmgr     46
#define kw0_Use3DWMap                   47
#define kw0_ReverseCurrentWorkspace     48
#define kw0_DontWarpCursorInWMap        49
#define kw0_CenterFeedbackWindow        50
#define kw0_WarpToDefaultMenuEntry      51
#define kw0_ShrinkIconTitles            52
#define kw0_AutoRaiseIcons              53
//#define kw0_use3DIconBorders            54
#define kw0_UseSunkTitlePixmap          55
#define kw0_ShortAllWindowsMenus        56
#define kw0_RaiseWhenAutoUnSqueeze      57
#define kw0_RaiseOnClick                58
#define kw0_IgnoreLockModifier          59
#define kw0_AutoFocusToTransients       60 /* kai */
#define kw0_PackNewWindows              61
#define kw0_IgnoreCaseInMenuSelection   62
#define kw0_SloppyFocus                 63
#define kw0_NoImagesInWorkSpaceManager  64
#define kw0_NoWarpToMenuTitle           65
#define kw0_SaveWorkspaceFocus          66 /* blais */
#define kw0_RaiseOnWarp                 67
#define kw0_DontShowWelcomeWindow       68
#define kw0_AutoPriority                69
#define kw0_DontToggleWorkspacemanagerState 70
#define kw0_BackingStore                71
#define kw0_StartInButtonState          72
#define kw0_NoSortIconManager           73
#define kw0_NoRestartPreviousState      74
#define kw0_NoDecorateTransients        75
#define kw0_GrabServer                  76
#define kw0_DontNameDecorations         77
#define kw0_StrictWinNameEncoding       78

#define kws_UsePPosition                1
#define kws_IconFont                    2
#define kws_ResizeFont                  3
#define kws_MenuFont                    4
#define kws_TitleFont                   5
#define kws_IconManagerFont             6
#define kws_UnknownIcon                 7
#define kws_IconDirectory               8
#define kws_MaxWindowSize               9
#define kws_PixmapDirectory             10
/* RandomPlacement moved because it's now a string string keyword */
#define kws_IconJustification           12
#define kws_TitleJustification          13
#define kws_IconRegionJustification     14
#define kws_IconRegionAlignement        15
#define kws_SoundHost                   16
#define kws_WMgrButtonStyle             17
#define kws_WorkSpaceFont               18
#define kws_IconifyStyle                19
#define kws_IconSize                    20
#define kws_RplaySoundHost              21

#define kwss_RandomPlacement            1

#define kwn_ConstrainedMoveTime         1
#define kwn_MoveDelta                   2
#define kwn_XorValue                    3
#define kwn_FramePadding                4
#define kwn_TitlePadding                5
#define kwn_ButtonIndent                6
#define kwn_BorderWidth                 7
#define kwn_IconBorderWidth             8
#define kwn_TitleButtonBorderWidth      9
#define kwn_RaiseDelay                  10
#define kwn_TransientOnTop              11
#define kwn_OpaqueMoveThreshold         12
#define kwn_OpaqueResizeThreshold       13
#define kwn_WMgrVertButtonIndent        14
#define kwn_WMgrHorizButtonIndent       15
#define kwn_ClearShadowContrast         16
#define kwn_DarkShadowContrast          17
#define kwn_WMgrButtonShadowDepth       18
#define kwn_MaxIconTitleWidth           19
#define kwn_AnimationSpeed              20
#define kwn_ThreeDBorderWidth           21
#define kwn_MoveOffResistance           22
#define kwn_BorderShadowDepth           23
#define kwn_TitleShadowDepth            24
#define kwn_TitleButtonShadowDepth      25
#define kwn_MenuShadowDepth             26
#define kwn_IconManagerShadowDepth      27
#define kwn_MovePackResistance          28
#define kwn_XMoveGrid                   29
#define kwn_YMoveGrid                   30
#define kwn_OpenWindowTimeout           31
#define kwn_RaiseOnClickButton          32

#define kwn_BorderTop                   33
#define kwn_BorderBottom                34
#define kwn_BorderLeft                  35
#define kwn_BorderRight                 36

#define kwcl_BorderColor                1
#define kwcl_IconManagerHighlight       2
#define kwcl_BorderTileForeground       3
#define kwcl_BorderTileBackground       4
#define kwcl_TitleForeground            5
#define kwcl_TitleBackground            6
#define kwcl_IconForeground             7
#define kwcl_IconBackground             8
#define kwcl_IconBorderColor            9
#define kwcl_IconManagerForeground      10
#define kwcl_IconManagerBackground      11
#define kwcl_MapWindowBackground        12
#define kwcl_MapWindowForeground        13

#define kwc_DefaultForeground           1
#define kwc_DefaultBackground           2
#define kwc_MenuForeground              3
#define kwc_MenuBackground              4
#define kwc_MenuTitleForeground         5
#define kwc_MenuTitleBackground         6
#define kwc_MenuShadowColor             7


/*
 * The following is sorted alphabetically according to name (which must be
 * in lowercase and only contain the letters a-z).  It is fed to a binary
 * search to parse keywords.
 */
static const TwmKeyword keytable[] = {
	{ "a",                      ALTER, 0 },
	{ "all",                    ALL, 0 },
	{ "alter",                  ALTER, 0 },
	{ "alwaysontop",            ALWAYS_ON_TOP, 0 },
	{ "alwaysshowwindowwhenmovingfromworkspacemanager", KEYWORD, kw0_ShowWinWhenMovingInWmgr },
	{ "alwayssqueezetogravity", ALWAYSSQUEEZETOGRAVITY, 0 },
	{ "animationspeed",         NKEYWORD, kwn_AnimationSpeed },
	{ "autofocustotransients",  KEYWORD, kw0_AutoFocusToTransients }, /* kai */
	{ "autolower",              AUTO_LOWER, 0 },
	{ "autooccupy",             KEYWORD, kw0_AutoOccupy },
	{ "autopopup",              AUTO_POPUP, 0 },
	{ "autopriority",           KEYWORD, kw0_AutoPriority },
	{ "autoraise",              AUTO_RAISE, 0 },
	{ "autoraiseicons",         KEYWORD, kw0_AutoRaiseIcons },
	{ "autorelativeresize",     KEYWORD, kw0_AutoRelativeResize },
	{ "autosqueeze",            AUTOSQUEEZE, 0 },
	{ "backingstore",           KEYWORD, kw0_BackingStore },
	{ "benicetocolormap",       KEYWORD, kw0_BeNiceToColormap },
	{ "borderbottom",           NKEYWORD, kwn_BorderBottom },
	{ "bordercolor",            CLKEYWORD, kwcl_BorderColor },
	{ "borderleft",             NKEYWORD, kwn_BorderLeft },
	{ "borderresizecursors",    KEYWORD, kw0_BorderResizeCursors },
	{ "borderright",            NKEYWORD, kwn_BorderRight },
	{ "bordershadowdepth",      NKEYWORD, kwn_BorderShadowDepth },
	{ "bordertilebackground",   CLKEYWORD, kwcl_BorderTileBackground },
	{ "bordertileforeground",   CLKEYWORD, kwcl_BorderTileForeground },
	{ "bordertop",              NKEYWORD, kwn_BorderTop },
	{ "borderwidth",            NKEYWORD, kwn_BorderWidth },
	{ "button",                 BUTTON, 0 },
	{ "buttonindent",           NKEYWORD, kwn_ButtonIndent },
	{ "c",                      CONTROL, 0 },
	{ "center",                 SIJENUM, SIJ_CENTER },
	{ "centerfeedbackwindow",   KEYWORD, kw0_CenterFeedbackWindow },
	{ "changeworkspacefunction", CHANGE_WORKSPACE_FUNCTION, 0 },
	{ "clearshadowcontrast",    NKEYWORD, kwn_ClearShadowContrast },
	{ "clicktofocus",           KEYWORD, kw0_ClickToFocus },
	{ "clientborderwidth",      KEYWORD, kw0_ClientBorderWidth },
	{ "color",                  COLOR, 0 },
	{ "constrainedmovetime",    NKEYWORD, kwn_ConstrainedMoveTime },
	{ "control",                CONTROL, 0 },
	{ "cursors",                CURSORS, 0 },
	{ "darkshadowcontrast",     NKEYWORD, kwn_DarkShadowContrast },
	{ "decoratetransients",     KEYWORD, kw0_DecorateTransients },
	{ "defaultbackground",      CKEYWORD, kwc_DefaultBackground },
	{ "defaultforeground",      CKEYWORD, kwc_DefaultForeground },
	{ "defaultfunction",        DEFAULT_FUNCTION, 0 },
	{ "deiconifyfunction",      DEICONIFY_FUNCTION, 0 },
	{ "destroy",                KILL, 0 },
	{ "donticonifybyunmapping", DONT_ICONIFY_BY_UNMAPPING, 0 },
	{ "dontmoveoff",            KEYWORD, kw0_DontMoveOff },
	{ "dontnamedecorations",    KEYWORD, kw0_DontNameDecorations },
	{ "dontpaintrootwindow",    KEYWORD, kw0_DontPaintRootWindow },
	{ "dontsave",               DONT_SAVE, 0 },
	{ "dontsetinactive",        DONTSETINACTIVE, 0 },
	{ "dontshowwelcomewindow",  KEYWORD, kw0_DontShowWelcomeWindow },
	{ "dontsqueezetitle",       DONT_SQUEEZE_TITLE, 0 },
	{ "donttoggleworkspacemanagerstate", KEYWORD, kw0_DontToggleWorkspacemanagerState},
	{ "dontwarpcursorinwmap",   KEYWORD, kw0_DontWarpCursorInWMap },
	{ "east",                   GRAVITY, GRAV_EAST },
	{ "ewmhignore",             EWMH_IGNORE, 0 },
	{ "f",                      FRAME, 0 },
	{ "forcefocus",             FORCE_FOCUS, 0 },
	{ "forceicons",             KEYWORD, kw0_ForceIcons },
	{ "frame",                  FRAME, 0 },
	{ "framepadding",           NKEYWORD, kwn_FramePadding },
	{ "function",               FUNCTION, 0 },
	{ "grabserver",             KEYWORD, kw0_GrabServer },
	{ "i",                      ICON, 0 },
	{ "icon",                   ICON, 0 },
	{ "iconbackground",         CLKEYWORD, kwcl_IconBackground },
	{ "iconbordercolor",        CLKEYWORD, kwcl_IconBorderColor },
	{ "iconborderwidth",        NKEYWORD, kwn_IconBorderWidth },
	{ "icondirectory",          SKEYWORD, kws_IconDirectory },
	{ "iconfont",               SKEYWORD, kws_IconFont },
	{ "iconforeground",         CLKEYWORD, kwcl_IconForeground },
	{ "iconifybyunmapping",     ICONIFY_BY_UNMAPPING, 0 },
	{ "iconifyfunction",        ICONIFY_FUNCTION, 0 },
	{ "iconifystyle",           SKEYWORD, kws_IconifyStyle },
	{ "iconjustification",      SKEYWORD, kws_IconJustification },
	{ "iconmanagerbackground",  CLKEYWORD, kwcl_IconManagerBackground },
	{ "iconmanagerdontshow",    ICONMGR_NOSHOW, 0 },
	{ "iconmanagerfont",        SKEYWORD, kws_IconManagerFont },
	{ "iconmanagerforeground",  CLKEYWORD, kwcl_IconManagerForeground },
	{ "iconmanagergeometry",    ICONMGR_GEOMETRY, 0 },
	{ "iconmanagerhighlight",   CLKEYWORD, kwcl_IconManagerHighlight },
	{ "iconmanagers",           ICONMGRS, 0 },
	{ "iconmanagershadowdepth", NKEYWORD, kwn_IconManagerShadowDepth },
	{ "iconmanagershow",        ICONMGR_SHOW, 0 },
	{ "iconmenudontshow",       ICONMENU_DONTSHOW, 0 },
	{ "iconmgr",                ICONMGR, 0 },
	{ "iconregion",             ICON_REGION, 0 },
	{ "iconregionalignement",   SKEYWORD, kws_IconRegionAlignement },
	{ "iconregionjustification", SKEYWORD, kws_IconRegionJustification },
	{ "icons",                  ICONS, 0 },
	{ "iconsize",               SKEYWORD, kws_IconSize },
	{ "ignorecaseinmenuselection",      KEYWORD, kw0_IgnoreCaseInMenuSelection },
	{ "ignorelockmodifier",     KEYWORD, kw0_IgnoreLockModifier },
	{ "ignoremodifier",         IGNOREMODIFIER, 0 },
	{ "ignoretransient",        IGNORE_TRANSIENT, 0 },
	{ "interpolatemenucolors",  KEYWORD, kw0_InterpolateMenuColors },
	{ "l",                      LOCK, 0 },
	{ "left",                   SIJENUM, SIJ_LEFT },
	{ "lefttitlebutton",        LEFT_TITLEBUTTON, 0 },
	{ "lock",                   LOCK, 0 },
	{ "m",                      META, 0 },
	{ "maketitle",              MAKE_TITLE, 0 },
	{ "mapwindowbackground",    CLKEYWORD, kwcl_MapWindowBackground },
	{ "mapwindowcurrentworkspace", MAPWINDOWCURRENTWORKSPACE, 0},
	{ "mapwindowdefaultworkspace", MAPWINDOWDEFAULTWORKSPACE, 0},
	{ "mapwindowforeground",    CLKEYWORD, kwcl_MapWindowForeground },
	{ "maxicontitlewidth",      NKEYWORD, kwn_MaxIconTitleWidth },
	{ "maxwindowsize",          SKEYWORD, kws_MaxWindowSize },
	{ "menu",                   MENU, 0 },
	{ "menubackground",         CKEYWORD, kwc_MenuBackground },
	{ "menufont",               SKEYWORD, kws_MenuFont },
	{ "menuforeground",         CKEYWORD, kwc_MenuForeground },
	{ "menushadowcolor",        CKEYWORD, kwc_MenuShadowColor },
	{ "menushadowdepth",        NKEYWORD, kwn_MenuShadowDepth },
	{ "menutitlebackground",    CKEYWORD, kwc_MenuTitleBackground },
	{ "menutitleforeground",    CKEYWORD, kwc_MenuTitleForeground },
	{ "meta",                   META, 0 },
	{ "mod",                    META, 0 },  /* fake it */
	{ "monitorlayout",          MONITOR_LAYOUT, 0 },
	{ "monochrome",             MONOCHROME, 0 },
	{ "move",                   MOVE, 0 },
	{ "movedelta",              NKEYWORD, kwn_MoveDelta },
	{ "moveoffresistance",      NKEYWORD, kwn_MoveOffResistance },
	{ "movepackresistance",     NKEYWORD, kwn_MovePackResistance },
	{ "mwmignore",              MWM_IGNORE, 0 },
	{ "nobackingstore",         KEYWORD, kw0_NoBackingStore },
	{ "noborder",               NO_BORDER, 0 },
	{ "nocasesensitive",        KEYWORD, kw0_NoCaseSensitive },
	{ "nodecoratetransients",   KEYWORD, kw0_NoDecorateTransients },
	{ "nodefaults",             KEYWORD, kw0_NoDefaults },
	{ "nograbserver",           KEYWORD, kw0_NoGrabServer },
	{ "nohighlight",            NO_HILITE, 0 },
	{ "noiconmanagerfocus",     KEYWORD, kw0_NoIconManagerFocus },
	{ "noiconmanagers",         KEYWORD, kw0_NoIconManagers },
	{ "noicontitle",            NO_ICON_TITLE, 0  },
	{ "noimagesinworkspacemanager", KEYWORD, kw0_NoImagesInWorkSpaceManager },
	{ "nomenushadows",          KEYWORD, kw0_NoMenuShadows },
	{ "noopaquemove",           NOOPAQUEMOVE, 0 },
	{ "noopaqueresize",         NOOPAQUERESIZE, 0 },
	{ "noraiseondeiconify",     KEYWORD, kw0_NoRaiseOnDeiconify },
	{ "noraiseonmove",          KEYWORD, kw0_NoRaiseOnMove },
	{ "noraiseonresize",        KEYWORD, kw0_NoRaiseOnResize },
	{ "noraiseonwarp",          KEYWORD, kw0_NoRaiseOnWarp },
	{ "norestartpreviousstate", KEYWORD, kw0_NoRestartPreviousState },
	{ "north",                  GRAVITY, GRAV_NORTH },
	{ "nosaveunders",           KEYWORD, kw0_NoSaveUnders },
	{ "noshowoccupyall",        KEYWORD, kw0_NoShowOccupyAll },
	{ "nosorticonmanager",      KEYWORD, kw0_NoSortIconManager },
	{ "nostackmode",            NO_STACKMODE, 0 },
	{ "notitle",                NO_TITLE, 0 },
	{ "notitlefocus",           KEYWORD, kw0_NoTitleFocus },
	{ "notitlehighlight",       NO_TITLE_HILITE, 0 },
	{ "nowarptomenutitle",      KEYWORD, kw0_NoWarpToMenuTitle },
	{ "occupy",                 OCCUPYLIST, 0 },
	{ "occupyall",              OCCUPYALL, 0 },
	{ "ontoppriority",          ON_TOP_PRIORITY, 0 },
	{ "opaquemove",             OPAQUEMOVE, 0 },
	{ "opaquemovethreshold",    NKEYWORD, kwn_OpaqueMoveThreshold },
	{ "opaqueresize",           OPAQUERESIZE, 0 },
	{ "opaqueresizethreshold",  NKEYWORD, kwn_OpaqueResizeThreshold },
	{ "openwindowtimeout",      NKEYWORD, kwn_OpenWindowTimeout },
	{ "packnewwindows",         KEYWORD, kw0_PackNewWindows },
	{ "pixmapdirectory",        SKEYWORD, kws_PixmapDirectory },
	{ "pixmaps",                PIXMAPS, 0 },
	{ "prioritynotswitching",   PRIORITY_NOT_SWITCHING, 0 },
	{ "priorityswitching",      PRIORITY_SWITCHING, 0 },
	{ "r",                      ROOT, 0 },
	{ "raisedelay",             NKEYWORD, kwn_RaiseDelay },
	{ "raiseonclick",           KEYWORD, kw0_RaiseOnClick },
	{ "raiseonclickbutton",     NKEYWORD, kwn_RaiseOnClickButton },
	{ "raiseonwarp",            KEYWORD, kw0_RaiseOnWarp },
	{ "raisewhenautounsqueeze", KEYWORD, kw0_RaiseWhenAutoUnSqueeze },
	{ "randomplacement",        SSKEYWORD, kwss_RandomPlacement },
	{ "reallymoveinworkspacemanager",   KEYWORD, kw0_ReallyMoveInWorkspaceManager },
	{ "resize",                 RESIZE, 0 },
	{ "resizefont",             SKEYWORD, kws_ResizeFont },
	{ "restartpreviousstate",   KEYWORD, kw0_RestartPreviousState },
	{ "reversecurrentworkspace", KEYWORD, kw0_ReverseCurrentWorkspace },
	{ "right",                  SIJENUM, SIJ_RIGHT },
	{ "righttitlebutton",       RIGHT_TITLEBUTTON, 0 },
	{ "root",                   ROOT, 0 },
	{ "rplaysoundhost",         SKEYWORD, kws_RplaySoundHost },
	{ "rplaysounds",            RPLAY_SOUNDS, 0 },
	{ "s",                      SHIFT, 0 },
	{ "savecolor",              SAVECOLOR, 0},
	{ "saveworkspacefocus",     KEYWORD, kw0_SaveWorkspaceFocus },
	{ "schrinkicontitles",      KEYWORD, kw0_ShrinkIconTitles },
	{ "select",                 SELECT, 0 },
	{ "shift",                  SHIFT, 0 },
	{ "shortallwindowsmenus",   KEYWORD, kw0_ShortAllWindowsMenus },
	{ "showiconmanager",        KEYWORD, kw0_ShowIconManager },
	{ "showworkspacemanager",   KEYWORD, kw0_ShowWorkspaceManager },
	{ "shrinkicontitles",       KEYWORD, kw0_ShrinkIconTitles },
	{ "sloppyfocus",            KEYWORD, kw0_SloppyFocus },
	{ "sorticonmanager",        KEYWORD, kw0_SortIconManager },
	{ "soundhost",              SKEYWORD, kws_SoundHost },
	{ "south",                  GRAVITY, GRAV_SOUTH },
	{ "squeezetitle",           SQUEEZE_TITLE, 0 },
	{ "starticonified",         START_ICONIFIED, 0 },
	{ "startinbuttonstate",     KEYWORD, kw0_StartInButtonState },
	{ "startinmapstate",        KEYWORD, kw0_StartInMapState },
	{ "startsqueezed",          STARTSQUEEZED, 0 },
	{ "stayupmenus",            KEYWORD, kw0_StayUpMenus },
	{ "strictwinnameencoding",  KEYWORD, kw0_StrictWinNameEncoding  },
	{ "sunkfocuswindowtitle",   KEYWORD, kw0_SunkFocusWindowTitle },
	{ "t",                      TITLE, 0 },
	{ "threedborderwidth",      NKEYWORD, kwn_ThreeDBorderWidth },
	{ "title",                  TITLE, 0 },
	{ "titlebackground",        CLKEYWORD, kwcl_TitleBackground },
	{ "titlebuttonborderwidth", NKEYWORD, kwn_TitleButtonBorderWidth },
	{ "titlebuttonshadowdepth", NKEYWORD, kwn_TitleButtonShadowDepth },
	{ "titlefont",              SKEYWORD, kws_TitleFont },
	{ "titleforeground",        CLKEYWORD, kwcl_TitleForeground },
	{ "titlehighlight",         TITLE_HILITE, 0 },
	{ "titlejustification",     SKEYWORD, kws_TitleJustification },
	{ "titlepadding",           NKEYWORD, kwn_TitlePadding },
	{ "titleshadowdepth",       NKEYWORD, kwn_TitleShadowDepth },
	{ "transienthasoccupation", KEYWORD, kw0_TransientHasOccupation },
	{ "transientontop",         NKEYWORD, kwn_TransientOnTop },
	{ "unknownicon",            SKEYWORD, kws_UnknownIcon },
	{ "unmapbymovingfaraway",   UNMAPBYMOVINGFARAWAY, 0 },
	{ "usepposition",           SKEYWORD, kws_UsePPosition },
	{ "usesunktitlepixmap",     KEYWORD, kw0_UseSunkTitlePixmap },
	{ "usethreedborders",       KEYWORD, kw0_Use3DBorders },
	{ "usethreediconmanagers",  KEYWORD, kw0_Use3DIconManagers },
	{ "usethreedmenus",         KEYWORD, kw0_Use3DMenus },
	{ "usethreedtitles",        KEYWORD, kw0_Use3DTitles },
	{ "usethreedwmap",          KEYWORD, kw0_Use3DWMap },
#ifdef VSCREEN
	{ "virtualscreens",         VIRTUAL_SCREENS, 0 },
#endif
	{ "w",                      WINDOW, 0 },
	{ "wait",                   WAITC, 0 },
	{ "warpcursor",             WARP_CURSOR, 0 },
	{ "warpondeiconify",        WARP_ON_DEICONIFY, 0 },
	{ "warpringonscreen",       KEYWORD, kw0_WarpRingOnScreen },
	{ "warptodefaultmenuentry", KEYWORD, kw0_WarpToDefaultMenuEntry },
	{ "warpunmapped",           KEYWORD, kw0_WarpUnmapped },
	{ "west",                   GRAVITY, GRAV_WEST },
	{ "window",                 WINDOW, 0 },
#ifdef WINBOX
	{ "windowbox",              WINDOW_BOX, 0 },
#endif
	{ "windowfunction",         WINDOW_FUNCTION, 0 },
	{ "windowgeometries",       WINDOW_GEOMETRIES, 0 },
	{ "windowregion",           WINDOW_REGION, 0 },
	{ "windowring",             WINDOW_RING, 0 },
	{ "windowringexclude",      WINDOW_RING_EXCLUDE, 0},
	{ "wmgrbuttonshadowdepth",  NKEYWORD, kwn_WMgrButtonShadowDepth },
	{ "wmgrbuttonstyle",        SKEYWORD, kws_WMgrButtonStyle },
	{ "wmgrhorizbuttonindent",  NKEYWORD, kwn_WMgrHorizButtonIndent },
	{ "wmgrvertbuttonindent",   NKEYWORD, kwn_WMgrVertButtonIndent },
	{ "workspace",              WORKSPACE, 0 },
	{ "workspacefont",          SKEYWORD, kws_WorkSpaceFont },
	{ "workspacemanagergeometry", WORKSPCMGR_GEOMETRY, 0 },
	{ "workspaces",             WORKSPACES, 0},
	{ "xmovegrid",              NKEYWORD, kwn_XMoveGrid },
	{ "xorvalue",               NKEYWORD, kwn_XorValue },
	{ "xpmicondirectory",       SKEYWORD, kws_PixmapDirectory },
	{ "ymovegrid",              NKEYWORD, kwn_YMoveGrid },
	{ "zoom",                   ZOOM, 0 },
};

static const size_t numkeywords = (sizeof(keytable) / sizeof(keytable[0]));


/*
 * The lookup table for functions is generated.
 */
#include "functions_parse_table.h"



static int
kt_compare(const void *lhs, const void *rhs)
{
	const TwmKeyword *l = lhs;
	const TwmKeyword *r = rhs;
	return strcasecmp(l->name, r->name);
}

int
parse_keyword(const char *s, int *nump)
{
	const TwmKeyword srch = { .name = s };
	TwmKeyword *ret;
	const TwmKeyword *srchtab;
	size_t nstab;

	/* Guard; nothing can't be a valid keyword */
	if(s == NULL || strlen(s) < 1) {
		return ERRORTOKEN;
	}

	/*
	 * Functions are in their own table, so check for them there.
	 *
	 * This is safe as long as (strlen >= 1), which we already checked.
	 */
	if(s[0] == 'f' && s[1] == '.') {
		srchtab = funckeytable;
		nstab = numfunckeywords;
	}
	else {
		srchtab = keytable;
		nstab = numkeywords;
	}

	/* Find it */
	ret = bsearch(&srch, srchtab, nstab, sizeof(TwmKeyword), kt_compare);
	if(ret) {
		*nump = ret->subnum;
		return ret->value;
	}

	return ERRORTOKEN;
}


/*
 * Simple tester function
 */
void
chk_keytable_order(void)
{
	int i;

	for(i = 0 ; i < (numkeywords - 1) ; i++) {
		if(strcasecmp(keytable[i].name, keytable[i + 1].name) >= 0) {
			fprintf(stderr, "%s: INTERNAL ERROR: keytable sorting: "
			        "'%s' >= '%s'\n", ProgramName,
			        keytable[i].name, keytable[i + 1].name);
		}
	}

	for(i = 0 ; i < (numfunckeywords - 1) ; i++) {
		if(strcasecmp(funckeytable[i].name, funckeytable[i + 1].name) >= 0) {
			fprintf(stderr, "%s: INTERNAL ERROR: funckeytable sorting: "
			        "'%s' >= '%s'\n", ProgramName,
			        funckeytable[i].name, funckeytable[i + 1].name);
		}
	}
}



/*
 * action routines called by grammar
 */

bool
do_single_keyword(int keyword)
{
	switch(keyword) {
		case kw0_NoDefaults:
			Scr->NoDefaults = true;
			return true;

		case kw0_AutoRelativeResize:
			Scr->AutoRelativeResize = true;
			return true;

		case kw0_ForceIcons:
			if(Scr->FirstTime) {
				Scr->ForceIcon = true;
			}
			return true;

		case kw0_NoIconManagers:
			Scr->NoIconManagers = true;
			return true;

		case kw0_InterpolateMenuColors:
			if(Scr->FirstTime) {
				Scr->InterpolateMenuColors = true;
			}
			return true;

		case kw0_SortIconManager:
			if(Scr->FirstTime) {
				Scr->SortIconMgr = true;
			}
			return true;

		case kw0_NoSortIconManager:
			if(Scr->FirstTime) {
				Scr->SortIconMgr = false;
			}
			return true;

		case kw0_GrabServer:
			Scr->NoGrabServer = false;
			return true;

		case kw0_NoGrabServer:
			Scr->NoGrabServer = true;
			return true;

		case kw0_NoMenuShadows:
			if(Scr->FirstTime) {
				Scr->Shadow = false;
			}
			return true;

		case kw0_NoRaiseOnMove:
			if(Scr->FirstTime) {
				Scr->NoRaiseMove = true;
			}
			return true;

		case kw0_NoRaiseOnResize:
			if(Scr->FirstTime) {
				Scr->NoRaiseResize = true;
			}
			return true;

		case kw0_NoRaiseOnDeiconify:
			if(Scr->FirstTime) {
				Scr->NoRaiseDeicon = true;
			}
			return true;

		case kw0_DontMoveOff:
			Scr->DontMoveOff = true;
			return true;

		case kw0_NoBackingStore:
			Scr->BackingStore = false;
			return true;

		case kw0_BackingStore:
			Scr->BackingStore = true;
			return true;

		case kw0_NoSaveUnders:
			Scr->SaveUnder = false;
			return true;

		// XXX Shouldn't these be in Scr too?
		case kw0_RestartPreviousState:
			RestartPreviousState = true;
			return true;

		case kw0_NoRestartPreviousState:
			RestartPreviousState = false;
			return true;

		case kw0_ClientBorderWidth:
			if(Scr->FirstTime) {
				Scr->ClientBorderWidth = true;
			}
			return true;

		case kw0_NoTitleFocus:
			Scr->TitleFocus = false;
			return true;

		case kw0_DecorateTransients:
			Scr->DecorateTransients = true;
			return true;

		case kw0_NoDecorateTransients:
			Scr->DecorateTransients = false;
			return true;

		case kw0_ShowIconManager:
			Scr->ShowIconManager = true;
			return true;

		case kw0_ShowWorkspaceManager:
			Scr->ShowWorkspaceManager = true;
			return true;

		case kw0_StartInButtonState:
			Scr->workSpaceMgr.initialstate = WMS_buttons;
			return true;

		case kw0_StartInMapState:
			Scr->workSpaceMgr.initialstate = WMS_map;
			return true;

		case kw0_NoShowOccupyAll:
			Scr->workSpaceMgr.noshowoccupyall = true;
			return true;

		case kw0_AutoOccupy:
			Scr->AutoOccupy = true;
			return true;

		case kw0_AutoPriority:
			Scr->AutoPriority = true;
			return true;

		case kw0_TransientHasOccupation:
			Scr->TransientHasOccupation = true;
			return true;

		case kw0_DontPaintRootWindow:
			Scr->DontPaintRootWindow = true;
			return true;

		case kw0_UseSunkTitlePixmap:
			Scr->UseSunkTitlePixmap = true;
			return true;

		case kw0_Use3DBorders:
			Scr->use3Dborders = true;
			return true;

		case kw0_Use3DIconManagers:
			Scr->use3Diconmanagers = true;
			return true;

		case kw0_Use3DMenus:
			Scr->use3Dmenus = true;
			return true;

		case kw0_Use3DTitles:
			Scr->use3Dtitles = true;
			return true;

		case kw0_Use3DWMap:
			Scr->use3Dwmap = true;
			return true;

		case kw0_SunkFocusWindowTitle:
			Scr->SunkFocusWindowTitle = true;
			return true;

		case kw0_BeNiceToColormap:
			Scr->BeNiceToColormap = true;
			return true;

		case kw0_BorderResizeCursors:
			Scr->BorderCursors = true;
			return true;

		case kw0_NoCaseSensitive:
			Scr->CaseSensitive = false;
			return true;

		case kw0_NoRaiseOnWarp:
			Scr->RaiseOnWarp = false;
			return true;

		case kw0_RaiseOnWarp:
			Scr->RaiseOnWarp = true;
			return true;

		case kw0_WarpUnmapped:
			Scr->WarpUnmapped = true;
			return true;

		case kw0_WarpRingOnScreen:
			Scr->WarpRingAnyWhere = false;
			return true;

		case kw0_NoIconManagerFocus:
			Scr->IconManagerFocus = false;
			return true;

		case kw0_StayUpMenus:
			Scr->StayUpMenus = true;
			return true;

		case kw0_ClickToFocus:
			Scr->ClickToFocus = true;
			return true;

		case kw0_ReallyMoveInWorkspaceManager:
			Scr->ReallyMoveInWorkspaceManager = true;
			return true;

		case kw0_ShowWinWhenMovingInWmgr:
			Scr->ShowWinWhenMovingInWmgr = true;
			return true;

		case kw0_ReverseCurrentWorkspace:
			Scr->ReverseCurrentWorkspace = true;
			return true;

		case kw0_DontWarpCursorInWMap:
			Scr->DontWarpCursorInWMap = true;
			return true;

		case kw0_CenterFeedbackWindow:
			Scr->CenterFeedbackWindow = true;
			return true;

		case kw0_WarpToDefaultMenuEntry:
			Scr->WarpToDefaultMenuEntry = true;
			return true;

		case kw0_ShrinkIconTitles:
			Scr->ShrinkIconTitles = true;
			return true;

		case kw0_AutoRaiseIcons:
			Scr->AutoRaiseIcons = true;
			return true;

		/* kai */
		case kw0_AutoFocusToTransients:
			Scr->AutoFocusToTransients = true;
			return true;

		case kw0_ShortAllWindowsMenus:
			Scr->ShortAllWindowsMenus = true;
			return true;

		case kw0_RaiseWhenAutoUnSqueeze:
			Scr->RaiseWhenAutoUnSqueeze = true;
			return true;

		case kw0_RaiseOnClick:
			Scr->RaiseOnClick = true;
			return true;

		case kw0_IgnoreLockModifier:
			Scr->IgnoreModifier |= LockMask;
			return true;

		case kw0_PackNewWindows:
			Scr->PackNewWindows = true;
			return true;

		case kw0_IgnoreCaseInMenuSelection:
			Scr->IgnoreCaseInMenuSelection = true;
			return true;

		case kw0_SloppyFocus:
			Scr->SloppyFocus = true;
			return true;

		case kw0_SaveWorkspaceFocus:
			Scr->SaveWorkspaceFocus = true;
			return true;

		case kw0_NoImagesInWorkSpaceManager:
			Scr->NoImagesInWorkSpaceManager = true;
			return true;

		case kw0_NoWarpToMenuTitle:
			Scr->NoWarpToMenuTitle = true;
			return true;

		case kw0_DontShowWelcomeWindow:
			Scr->ShowWelcomeWindow = false;
			return true;

		case kw0_DontToggleWorkspacemanagerState:
			Scr->DontToggleWorkspaceManagerState = true;
			return true;

		case kw0_DontNameDecorations:
			Scr->NameDecorations = false;
			return true;

		case kw0_StrictWinNameEncoding:
			Scr->StrictWinNameEncoding = true;
			return true;

	}
	return false;
}


bool
do_string_string_keyword(int keyword, const char *s1, const char *s2)
{
	switch(keyword) {
		case kwss_RandomPlacement: {
			/* RandomPlacement {on,off,all,unmapped} [displacement geom] */
			int rp;
			int gmask, gx, gy;     // Geometry mask/x/y values
			unsigned int gjw, gjh; // width/height (ignored)
			int exmask = (XValue | YValue); // Bits we need in the mask

			rp = ParseRandomPlacement(s1);
			if(rp < 0) {
				twmrc_error_prefix();
				fprintf(stderr,
				        "ignoring invalid RandomPlacement argument 1 \"%s\"\n",
				        s1);
			}
			else {
				Scr->RandomPlacement = rp;
			}

			/* If no geom, we're done */
			if(s2 == NULL) {
				return true;
			}

			/*
			 * Figure what the geom means.  We actually don't care about
			 * the size (it probably won't even be provided), so the
			 * width/height are junk.  The X/Y offsets are what we need.
			 * But we do need them.
			 */
			gmask = XParseGeometry(s2, &gx, &gy, &gjw, &gjh);
#ifdef DEBUG
			fprintf(stderr, "DEBUG:: Mask = %x, Width = %d, Height = %d\n",
			        gmask, gjw, gjh);
			fprintf(stderr, "DEBUG:: X = %d, Y = %d\n", gx, gy);
#endif
			if((gmask & exmask) != exmask) {
				/* Didn't get X and Y */
				twmrc_error_prefix();
				fprintf(stderr,
				        "ignoring invalid RandomPlacement displacement \"%s\"\n", s2);
			}
			else {
				Scr->RandomDisplacementX = gx;
				Scr->RandomDisplacementY = gy;
			}

			/* Done */
			return true;
		}
	}
	return false;
}


bool
do_string_keyword(int keyword, char *s)
{
	switch(keyword) {
		case kws_UsePPosition: {
			int ppos = ParseUsePPosition(s);
			if(ppos < 0) {
				twmrc_error_prefix();
				fprintf(stderr,
				        "ignoring invalid UsePPosition argument \"%s\"\n", s);
			}
			else {
				Scr->UsePPosition = ppos;
			}
			return true;
		}

		case kws_IconFont:
			if(!Scr->HaveFonts) {
				Scr->IconFont.basename = s;
			}
			return true;

		case kws_ResizeFont:
			if(!Scr->HaveFonts) {
				Scr->SizeFont.basename = s;
			}
			return true;

		case kws_MenuFont:
			if(!Scr->HaveFonts) {
				Scr->MenuFont.basename = s;
			}
			return true;

		case kws_WorkSpaceFont:
			if(!Scr->HaveFonts) {
				Scr->workSpaceMgr.windowFont.basename = s;
			}
			return true;

		case kws_TitleFont:
			if(!Scr->HaveFonts) {
				Scr->TitleBarFont.basename = s;
			}
			return true;

		case kws_IconManagerFont:
			if(!Scr->HaveFonts) {
				Scr->IconManagerFont.basename = s;
			}
			return true;

		case kws_UnknownIcon:
			if(Scr->FirstTime) {
				Scr->UnknownImage = GetImage(s, Scr->IconC);
			}
			return true;

		case kws_IconDirectory:
			if(Scr->FirstTime) {
				Scr->IconDirectory = ExpandFilePath(s);
			}
			return true;

		case kws_PixmapDirectory:
			if(Scr->FirstTime) {
				Scr->PixmapDirectory = ExpandFilePath(s);
			}
			return true;

		case kws_MaxWindowSize: {
			int gmask;
			int exmask = (WidthValue | HeightValue);
			unsigned int gw, gh; // Stuff we care about
			int gjx, gjy;        // Stuff we don't

			gmask = XParseGeometry(s, &gjx, &gjy, &gw, &gh);
			if((gmask & exmask) != exmask) {
				twmrc_error_prefix();
				fprintf(stderr, "bad MaxWindowSize \"%s\"\n", s);
				return false;
			}
			if(gw == 0 || gh == 0) {
				twmrc_error_prefix();
				fprintf(stderr, "MaxWindowSize \"%s\" must be non-zero\n", s);
				return false;
			}
			Scr->MaxWindowWidth = gw;
			Scr->MaxWindowHeight = gh;
			return true;
		}

		case kws_IconJustification: {
			int just = ParseTitleJustification(s);

			if((just < 0) || (just == TJ_UNDEF)) {
				twmrc_error_prefix();
				fprintf(stderr,
				        "ignoring invalid IconJustification argument \"%s\"\n", s);
			}
			else {
				Scr->IconJustification = just;
			}
			return true;
		}
		case kws_IconRegionJustification: {
			int just = ParseIRJustification(s);

			if(just < 0 || (just == IRJ_UNDEF)) {
				twmrc_error_prefix();
				fprintf(stderr,
				        "ignoring invalid IconRegionJustification argument \"%s\"\n", s);
			}
			else {
				Scr->IconRegionJustification = just;
			}
			return true;
		}
		case kws_IconRegionAlignement: {
			int just = ParseAlignement(s);

			if(just < 0) {
				twmrc_error_prefix();
				fprintf(stderr,
				        "ignoring invalid IconRegionAlignement argument \"%s\"\n", s);
			}
			else {
				Scr->IconRegionAlignement = just;
			}
			return true;
		}

		case kws_TitleJustification: {
			int just = ParseTitleJustification(s);

			if((just < 0) || (just == TJ_UNDEF)) {
				twmrc_error_prefix();
				fprintf(stderr,
				        "ignoring invalid TitleJustification argument \"%s\"\n", s);
			}
			else {
				Scr->TitleJustification = just;
			}
			return true;
		}
		case kws_RplaySoundHost:
		case kws_SoundHost:
			if(Scr->FirstTime) {
				/* Warning to be enabled in the future before removal */
				if(0 && keyword == kws_SoundHost) {
					twmrc_error_prefix();
					fprintf(stderr, "SoundHost is deprecated, please "
					        "use RplaySoundHost instead.\n");
				}
#ifdef SOUNDS
				set_sound_host(s);
#else
				twmrc_error_prefix();
				fprintf(stderr, "Ignoring %sSoundHost; rplay not ronfigured.\n",
				        (keyword == kws_RplaySoundHost ? "Rplay" : ""));
#endif
			}
			return true;

		case kws_WMgrButtonStyle: {
			int style = ParseButtonStyle(s);

			if(style < 0) {
				twmrc_error_prefix();
				fprintf(stderr,
				        "ignoring invalid WMgrButtonStyle argument \"%s\"\n", s);
			}
			else {
				Scr->workSpaceMgr.buttonStyle = style;
			}
			return true;
		}

		case kws_IconifyStyle: {
			int style = ParseIconifyStyle(s);

			if(style < 0) {
				twmrc_error_prefix();
				fprintf(stderr, "ignoring invalid IconifyStyle argument \"%s\"\n", s);
			}
			else {
				Scr->IconifyStyle = style;
			}
			return true;
		}

#ifdef EWMH
		case kws_IconSize:
			if(sscanf(s, "%dx%d", &Scr->PreferredIconWidth,
			                &Scr->PreferredIconHeight) == 2) {
				/* ok */
			}
			else if(sscanf(s, "%d", &Scr->PreferredIconWidth) == 1) {
				Scr->PreferredIconHeight = Scr->PreferredIconWidth;
			}
			else {
				Scr->PreferredIconHeight = Scr->PreferredIconWidth = 48;
			}
			return true;
#endif
	}
	return false;
}


bool
do_number_keyword(int keyword, int num)
{
	switch(keyword) {
		case kwn_ConstrainedMoveTime:
			ConstrainedMoveTime = num;
			return true;

		case kwn_MoveDelta:
			Scr->MoveDelta = num;
			return true;

		case kwn_MoveOffResistance:
			Scr->MoveOffResistance = num;
			return true;

		case kwn_MovePackResistance:
			if(num < 0) {
				num = 20;
			}
			Scr->MovePackResistance = num;
			return true;

		case kwn_XMoveGrid:
			if(num < 1) {
				num = 1;
			}
			if(num > 100) {
				num = 100;
			}
			Scr->XMoveGrid = num;
			return true;

		case kwn_YMoveGrid:
			if(num < 1) {
				num = 1;
			}
			if(num > 100) {
				num = 100;
			}
			Scr->YMoveGrid = num;
			return true;

		case kwn_XorValue:
			if(Scr->FirstTime) {
				Scr->XORvalue = num;
			}
			return true;

		case kwn_FramePadding:
			if(Scr->FirstTime) {
				Scr->FramePadding = num;
			}
			return true;

		case kwn_TitlePadding:
			if(Scr->FirstTime) {
				Scr->TitlePadding = num;
			}
			return true;

		case kwn_ButtonIndent:
			if(Scr->FirstTime) {
				Scr->ButtonIndent = num;
			}
			return true;

		case kwn_ThreeDBorderWidth:
			if(Scr->FirstTime) {
				Scr->ThreeDBorderWidth = num;
			}
			return true;

		case kwn_BorderWidth:
			if(Scr->FirstTime) {
				Scr->BorderWidth = num;
			}
			return true;

		case kwn_IconBorderWidth:
			if(Scr->FirstTime) {
				Scr->IconBorderWidth = num;
			}
			return true;

		case kwn_TitleButtonBorderWidth:
			if(Scr->FirstTime) {
				Scr->TBInfo.border = num;
			}
			return true;

		case kwn_RaiseDelay:
			RaiseDelay = num;
			return true;

		case kwn_TransientOnTop:
			if(Scr->FirstTime) {
				Scr->TransientOnTop = num;
			}
			return true;

		case kwn_OpaqueMoveThreshold:
			if(Scr->FirstTime) {
				Scr->OpaqueMoveThreshold = num;
			}
			return true;

		case kwn_OpaqueResizeThreshold:
			if(Scr->FirstTime) {
				Scr->OpaqueResizeThreshold = num;
			}
			return true;

		case kwn_WMgrVertButtonIndent:
			if(Scr->FirstTime) {
				Scr->WMgrVertButtonIndent = num;
			}
			if(Scr->WMgrVertButtonIndent < 0) {
				Scr->WMgrVertButtonIndent = 0;
			}
			Scr->workSpaceMgr.vspace = Scr->WMgrVertButtonIndent;
			Scr->workSpaceMgr.occupyWindow->vspace = Scr->WMgrVertButtonIndent;
			return true;

		case kwn_WMgrHorizButtonIndent:
			if(Scr->FirstTime) {
				Scr->WMgrHorizButtonIndent = num;
			}
			if(Scr->WMgrHorizButtonIndent < 0) {
				Scr->WMgrHorizButtonIndent = 0;
			}
			Scr->workSpaceMgr.hspace = Scr->WMgrHorizButtonIndent;
			Scr->workSpaceMgr.occupyWindow->hspace = Scr->WMgrHorizButtonIndent;
			return true;

		case kwn_WMgrButtonShadowDepth:
			if(Scr->FirstTime) {
				Scr->WMgrButtonShadowDepth = num;
			}
			if(Scr->WMgrButtonShadowDepth < 1) {
				Scr->WMgrButtonShadowDepth = 1;
			}
			return true;

		case kwn_MaxIconTitleWidth:
			if(Scr->FirstTime) {
				Scr->MaxIconTitleWidth = num;
			}
			return true;

		case kwn_ClearShadowContrast:
			if(Scr->FirstTime) {
				Scr->ClearShadowContrast = num;
			}
			if(Scr->ClearShadowContrast < 0) {
				Scr->ClearShadowContrast = 0;
			}
			if(Scr->ClearShadowContrast > 100) {
				Scr->ClearShadowContrast = 100;
			}
			return true;

		case kwn_DarkShadowContrast:
			if(Scr->FirstTime) {
				Scr->DarkShadowContrast = num;
			}
			if(Scr->DarkShadowContrast < 0) {
				Scr->DarkShadowContrast = 0;
			}
			if(Scr->DarkShadowContrast > 100) {
				Scr->DarkShadowContrast = 100;
			}
			return true;

		case kwn_AnimationSpeed:
			if(num < 0) {
				num = 0;
			}
			SetAnimationSpeed(num);
			return true;

		case kwn_BorderShadowDepth:
			if(Scr->FirstTime) {
				Scr->BorderShadowDepth = num;
			}
			if(Scr->BorderShadowDepth < 0) {
				Scr->BorderShadowDepth = 2;
			}
			return true;

		case kwn_BorderLeft:
			if(Scr->FirstTime) {
				Scr->BorderLeft = num;
			}
			if(Scr->BorderLeft < 0) {
				Scr->BorderLeft = 0;
			}
			return true;

		case kwn_BorderRight:
			if(Scr->FirstTime) {
				Scr->BorderRight = num;
			}
			if(Scr->BorderRight < 0) {
				Scr->BorderRight = 0;
			}
			return true;

		case kwn_BorderTop:
			if(Scr->FirstTime) {
				Scr->BorderTop = num;
			}
			if(Scr->BorderTop < 0) {
				Scr->BorderTop = 0;
			}
			return true;

		case kwn_BorderBottom:
			if(Scr->FirstTime) {
				Scr->BorderBottom = num;
			}
			if(Scr->BorderBottom < 0) {
				Scr->BorderBottom = 0;
			}
			return true;

		case kwn_TitleButtonShadowDepth:
			if(Scr->FirstTime) {
				Scr->TitleButtonShadowDepth = num;
			}
			if(Scr->TitleButtonShadowDepth < 0) {
				Scr->TitleButtonShadowDepth = 2;
			}
			return true;

		case kwn_TitleShadowDepth:
			if(Scr->FirstTime) {
				Scr->TitleShadowDepth = num;
			}
			if(Scr->TitleShadowDepth < 0) {
				Scr->TitleShadowDepth = 2;
			}
			return true;

		case kwn_IconManagerShadowDepth:
			if(Scr->FirstTime) {
				Scr->IconManagerShadowDepth = num;
			}
			if(Scr->IconManagerShadowDepth < 0) {
				Scr->IconManagerShadowDepth = 2;
			}
			return true;

		case kwn_MenuShadowDepth:
			if(Scr->FirstTime) {
				Scr->MenuShadowDepth = num;
			}
			if(Scr->MenuShadowDepth < 0) {
				Scr->MenuShadowDepth = 2;
			}
			return true;

		case kwn_OpenWindowTimeout:
			if(Scr->FirstTime) {
				Scr->OpenWindowTimeout = num;
			}
			if(Scr->OpenWindowTimeout < 0) {
				Scr->OpenWindowTimeout = 0;
			}
			return true;

		case kwn_RaiseOnClickButton:
			if(Scr->FirstTime) {
				Scr->RaiseOnClickButton = num;
			}
			if(Scr->RaiseOnClickButton < 1) {
				Scr->RaiseOnClickButton = 1;
			}
			if(Scr->RaiseOnClickButton > MAX_BUTTONS) {
				Scr->RaiseOnClickButton = MAX_BUTTONS;
			}
			return true;


	}

	return false;
}

name_list **
do_colorlist_keyword(int keyword, int colormode, char *s)
{
	switch(keyword) {
		case kwcl_BorderColor:
			GetColor(colormode, &Scr->BorderColorC.back, s);
			return &Scr->BorderColorL;

		case kwcl_IconManagerHighlight:
			GetColor(colormode, &Scr->IconManagerHighlight, s);
			return &Scr->IconManagerHighlightL;

		case kwcl_BorderTileForeground:
			GetColor(colormode, &Scr->BorderTileC.fore, s);
			return &Scr->BorderTileForegroundL;

		case kwcl_BorderTileBackground:
			GetColor(colormode, &Scr->BorderTileC.back, s);
			return &Scr->BorderTileBackgroundL;

		case kwcl_TitleForeground:
			GetColor(colormode, &Scr->TitleC.fore, s);
			return &Scr->TitleForegroundL;

		case kwcl_TitleBackground:
			GetColor(colormode, &Scr->TitleC.back, s);
			return &Scr->TitleBackgroundL;

		case kwcl_IconForeground:
			GetColor(colormode, &Scr->IconC.fore, s);
			return &Scr->IconForegroundL;

		case kwcl_IconBackground:
			GetColor(colormode, &Scr->IconC.back, s);
			return &Scr->IconBackgroundL;

		case kwcl_IconBorderColor:
			GetColor(colormode, &Scr->IconBorderColor, s);
			return &Scr->IconBorderColorL;

		case kwcl_IconManagerForeground:
			GetColor(colormode, &Scr->IconManagerC.fore, s);
			return &Scr->IconManagerFL;

		case kwcl_IconManagerBackground:
			GetColor(colormode, &Scr->IconManagerC.back, s);
			return &Scr->IconManagerBL;

		case kwcl_MapWindowBackground:
			GetColor(colormode, &Scr->workSpaceMgr.windowcp.back, s);
			Scr->workSpaceMgr.windowcpgiven = true;
			return &Scr->workSpaceMgr.windowBackgroundL;

		case kwcl_MapWindowForeground:
			GetColor(colormode, &Scr->workSpaceMgr.windowcp.fore, s);
			Scr->workSpaceMgr.windowcpgiven = true;
			return &Scr->workSpaceMgr.windowForegroundL;
	}
	return NULL;
}

bool
do_color_keyword(int keyword, int colormode, char *s)
{
	switch(keyword) {
		case kwc_DefaultForeground:
			GetColor(colormode, &Scr->DefaultC.fore, s);
			return true;

		case kwc_DefaultBackground:
			GetColor(colormode, &Scr->DefaultC.back, s);
			return true;

		case kwc_MenuForeground:
			GetColor(colormode, &Scr->MenuC.fore, s);
			return true;

		case kwc_MenuBackground:
			GetColor(colormode, &Scr->MenuC.back, s);
			return true;

		case kwc_MenuTitleForeground:
			GetColor(colormode, &Scr->MenuTitleC.fore, s);
			return true;

		case kwc_MenuTitleBackground:
			GetColor(colormode, &Scr->MenuTitleC.back, s);
			return true;

		case kwc_MenuShadowColor:
			GetColor(colormode, &Scr->MenuShadowColor, s);
			return true;

	}

	return false;
}

/*
 * put_pixel_on_root() Save a pixel value in twm root window color property.
 */
static void
put_pixel_on_root(Pixel pixel)
{
	bool addone = true;
	Atom          retAtom;
	int           retFormat;
	unsigned long nPixels, retAfter;
	Pixel        *retProp;

	// Get current list
	if(XGetWindowProperty(dpy, Scr->Root, XA__MIT_PRIORITY_COLORS, 0, 8192,
	                      False, XA_CARDINAL, &retAtom,
	                      &retFormat, &nPixels, &retAfter,
	                      (unsigned char **)&retProp) != Success || !retProp) {
		return;
	}

	// See if we already have this one
	for(int i = 0; i < nPixels; i++) {
		if(pixel == retProp[i]) {
			addone = false;
		}
	}
	XFree(retProp);

	// If not, append it
	if(addone) {
		XChangeProperty(dpy, Scr->Root, XA__MIT_PRIORITY_COLORS,
		                XA_CARDINAL, 32, PropModeAppend,
		                (unsigned char *)&pixel, 1);
	}
}

/*
 * Stash for SaveColor{} values during config parsing.
 */
typedef struct _cnode {
	int i;
	int cmode;
	char *sname;
	struct _cnode *next;
} Cnode;
static Cnode *chead = NULL;

/**
 * Add a SaveColor{} entry to our stash.
 */
static void
add_cnode(int kwcl, int cmode, char *colname)
{
	Cnode *cnew;

	cnew = calloc(1, sizeof(Cnode));
	cnew->i     = kwcl;
	cnew->cmode = cmode;
	cnew->sname = colname;

	if(!chead) {
		chead = cnew;
	}
	else {
		cnew->next = chead;
		chead = cnew;
	}

	return;
}


/*
 * do_string_savecolor() save a color from a string in the twmrc file.
 */
void
do_string_savecolor(int colormode, char *s)
{
	add_cnode(0, colormode, s);
}

/*
 * do_var_savecolor() save a color from a var in the twmrc file.
 */
void
do_var_savecolor(int key)
{
	add_cnode(key, 0, NULL);
}

/*
 * assign_var_savecolor() traverse the var save color list placeing the pixels
 *                        in the root window property.
 */
void
assign_var_savecolor(void)
{
	Cnode *cp = chead;

	// Start with an empty property
	XChangeProperty(dpy, Scr->Root, XA__MIT_PRIORITY_COLORS,
	                XA_CARDINAL, 32, PropModeReplace, NULL, 0);

	// Loop over, stash 'em, and clean up
	while(cp != NULL) {
		Cnode *tmp_cp = cp;

		switch(cp->i) {
			case kwcl_BorderColor:
				put_pixel_on_root(Scr->BorderColorC.back);
				break;
			case kwcl_IconManagerHighlight:
				put_pixel_on_root(Scr->IconManagerHighlight);
				break;
			case kwcl_BorderTileForeground:
				put_pixel_on_root(Scr->BorderTileC.fore);
				break;
			case kwcl_BorderTileBackground:
				put_pixel_on_root(Scr->BorderTileC.back);
				break;
			case kwcl_TitleForeground:
				put_pixel_on_root(Scr->TitleC.fore);
				break;
			case kwcl_TitleBackground:
				put_pixel_on_root(Scr->TitleC.back);
				break;
			case kwcl_IconForeground:
				put_pixel_on_root(Scr->IconC.fore);
				break;
			case kwcl_IconBackground:
				put_pixel_on_root(Scr->IconC.back);
				break;
			case kwcl_IconBorderColor:
				put_pixel_on_root(Scr->IconBorderColor);
				break;
			case kwcl_IconManagerForeground:
				put_pixel_on_root(Scr->IconManagerC.fore);
				break;
			case kwcl_IconManagerBackground:
				put_pixel_on_root(Scr->IconManagerC.back);
				break;
			case kwcl_MapWindowForeground:
				put_pixel_on_root(Scr->workSpaceMgr.windowcp.fore);
				break;
			case kwcl_MapWindowBackground:
				put_pixel_on_root(Scr->workSpaceMgr.windowcp.back);
				break;
			case 0: {
				// This means it's a string, not one of our keywords
				Pixel p;
				GetColor(cp->cmode, &p, cp->sname);
				put_pixel_on_root(p);
			}
		}

		cp = cp->next;
		free(tmp_cp);
	}
	if(chead) {
		chead = NULL;
	}
}


/*
 * RandomPlacement [...] parse
 */
static int
ParseRandomPlacement(const char *s)
{
	/* No first arg -> 'all' */
	if(s == NULL) {
		return RP_ALL;
	}
	if(strlen(s) == 0) {
		return RP_ALL;
	}

#define CHK(str, ret) if(strcasecmp(s, str) == 0) { return RP_##ret; }
	CHK(DEFSTRING,  ALL);
	CHK("on",       ALL);
	CHK("all",      ALL);
	CHK("off",      OFF);
	CHK("unmapped", UNMAPPED);
#undef CHK

	return -1;
}


/*
 * Parse out IconRegionJustification string.
 *
 * X-ref comment on ParseAlignement about return value.
 */
int
ParseIRJustification(const char *s)
{
	if(strlen(s) == 0) {
		return -1;
	}

#define CHK(str, ret) if(strcasecmp(s, str) == 0) { return IRJ_##ret; }
	CHK(DEFSTRING, CENTER);
	CHK("undef",   UNDEF);
	CHK("left",    LEFT);
	CHK("center",  CENTER);
	CHK("right",   RIGHT);
	CHK("border",  BORDER);
#undef CHK

	return -1;
}


/*
 * Parse out string for title justification.  From TitleJustification,
 * IconJustification, iconjust arg to IconRegion.
 *
 * X-ref comment on ParseAlignement about return value.
 */
int
ParseTitleJustification(const char *s)
{
	if(strlen(s) == 0) {
		return -1;
	}

#define CHK(str, ret) if(strcasecmp(s, str) == 0) { return TJ_##ret; }
	/* XXX Different uses really have different defaults... */
	CHK(DEFSTRING, CENTER);
	CHK("undef",   UNDEF);
	CHK("left",    LEFT);
	CHK("center",  CENTER);
	CHK("right",   RIGHT);
#undef CHK

	return -1;
}


/*
 * Parse out the string specifier for IconRegion Alignement[sic].
 * Strictly speaking, this [almost always] returns an IRAlignement enum
 * value.  However, it's specified as int to allow the -1 return for
 * invalid values.  enum's start numbering from 0 (unless specific values
 * are given), so that's a safe out-of-bounds value.  And making an
 * IRA_INVALID value would just add unnecessary complication, since
 * during parsing is the only time it makes sense.
 */
int
ParseAlignement(const char *s)
{
	if(strlen(s) == 0) {
		return -1;
	}

#define CHK(str, ret) if(strcasecmp(s, str) == 0) { return IRA_##ret; }
	CHK(DEFSTRING, CENTER);
	CHK("center",  CENTER);
	CHK("top",     TOP);
	CHK("bottom",  BOTTOM);
	CHK("border",  BORDER);
	CHK("undef",   UNDEF);
#undef CHK

	return -1;
}

static int
ParseUsePPosition(const char *s)
{
	if(strlen(s) == 0) {
		return -1;
	}

#define CHK(str, ret) if(strcasecmp(s, str) == 0) { return PPOS_##ret; }
	CHK(DEFSTRING,  OFF);
	CHK("off",      OFF);
	CHK("on",       ON);
	CHK("non-zero", NON_ZERO);
	CHK("nonzero",  NON_ZERO);
#undef CHK

	return -1;
}

static int
ParseButtonStyle(const char *s)
{
	if(s == NULL || strlen(s) == 0) {
		return -1;
	}

#define CHK(str, ret) if(strcasecmp(s, str) == 0) { return STYLE_##ret; }
	CHK(DEFSTRING, NORMAL);
	CHK("normal",  NORMAL);
	CHK("style1",  STYLE1);
	CHK("style2",  STYLE2);
	CHK("style3",  STYLE3);
#undef CHK

	return -1;
}

static int
ParseIconifyStyle(const char *s)
{
	if(s == NULL || strlen(s) == 0) {
		return -1;
	}

#define CHK(str, ret) if(strcasecmp(s, str) == 0) { return ICONIFY_##ret; }
	CHK(DEFSTRING, NORMAL);
	CHK("normal",  NORMAL);
	CHK("mosaic",  MOSAIC);
	CHK("zoomin",  ZOOMIN);
	CHK("zoomout", ZOOMOUT);
	CHK("fade",    FADE);
	CHK("sweep",   SWEEP);
#undef CHK

	return -1;
}

void
do_squeeze_entry(name_list **slist, // squeeze or dont-squeeze list
                 const char *name,  // window name
                 SIJust justify,    // left, center, right
                 int num,           // signed num
                 int denom)         // 0 or indicates fraction denom
{
	int absnum = (num < 0 ? -num : num);

	if(denom < 0) {
		twmrc_error_prefix();
		fprintf(stderr, "negative SqueezeTitle denominator %d\n", denom);
		ParseError = true;
		return;
	}
	if(absnum > denom && denom != 0) {
		twmrc_error_prefix();
		fprintf(stderr, "SqueezeTitle fraction %d/%d outside window\n",
		        num, denom);
		ParseError = true;
		return;
	}
	/* Process the special cases from the manual here rather than
	 * each time we calculate the position of the title bar
	 * in ComputeTitleLocation().
	 * In fact, it's better to get rid of them entirely, but we
	 * probably should not do that for compatibility's sake.
	 * By using a non-zero denominator the position will be relative.
	 */
	if(denom == 0 && num == 0) {
		if(justify == SIJ_CENTER) {
			num = 1;
			denom = 2;
		}
		else if(justify == SIJ_RIGHT) {
			num = 2;
			denom = 2;
		}
		twmrc_error_prefix();
		fprintf(stderr, "deprecated SqueezeTitle faction 0/0, assuming %d/%d\n",
		        num, denom);
	}

	if(HasShape) {
		SqueezeInfo *sinfo;
		sinfo = malloc(sizeof(SqueezeInfo));

		if(!sinfo) {
			twmrc_error_prefix();
			fprintf(stderr, "unable to allocate %lu bytes for squeeze info\n",
			        (unsigned long) sizeof(SqueezeInfo));
			ParseError = true;
			return;
		}
		sinfo->justify = justify;
		sinfo->num = num;
		sinfo->denom = denom;
		AddToList(slist, name, sinfo);
	}
	return;
}


/*
 * Parsing for EWMHIgnore { } lists
 */
void
proc_ewmh_ignore(void)
{
#ifndef EWMH
	twmrc_error_prefix();
	fprintf(stderr, "EWMH not enabled, EWMHIgnore { } ignored.\n");
	ParseError = true;
	return;
#endif
	/* else nada */
	return;
}
void
add_ewmh_ignore(char *s)
{
#ifndef EWMH
	return;
#else

#define HANDLE(x) \
        if(strcasecmp(s, (x)) == 0) { \
                AddToList(&Scr->EWMHIgnore, (x), ""); \
                return; \
        }
	HANDLE("STATE_MAXIMIZED_VERT");
	HANDLE("STATE_MAXIMIZED_HORZ");
	HANDLE("STATE_FULLSCREEN");
	HANDLE("STATE_SHADED");
	HANDLE("STATE_ABOVE");
	HANDLE("STATE_BELOW");
#undef HANDLE

	twmrc_error_prefix();
	fprintf(stderr, "Unexpected EWMHIgnore value '%s'\n", s);
	ParseError = true;
	return;
#endif /* EWMH */
}


/*
 * Parsing for MWMIgnore { } lists
 */
void
proc_mwm_ignore(void)
{
	/* Nothing to do */
	return;
}
void
add_mwm_ignore(char *s)
{
#define HANDLE(x) \
        if(strcasecmp(s, (x)) == 0) { \
                AddToList(&Scr->MWMIgnore, (x), ""); \
                return; \
        }
	HANDLE("DECOR_BORDER");
	HANDLE("DECOR_TITLE");
#undef HANDLE

	twmrc_error_prefix();
	fprintf(stderr, "Unexpected MWMIgnore value '%s'\n", s);
	ParseError = true;
	return;
}


/*
 * Parsing for Layout { } lists, to override the monitor layout we
 * assumed or got from RANDR.
 */
static RAreaList *override_monitors;
static struct {
	char **names;
	int len;
	int cap;
} override_monitors_names;


/**
 * Allocate space for our monitor override list.
 */
void
init_layout_override(void)
{
	// 4 seems like a good guess.  If we're doing this, we're probably
	// making at least 2 monitors, and >4 is gonna be pretty rare, so...
	const int initsz = 4;

	override_monitors = RAreaListNew(initsz, NULL);
	if(override_monitors == NULL) {
		twmrc_error_prefix();
		fprintf(stderr, "Failed allocating RAreaList for monitors.\n");
		ParseError = true;
		return;
		// Maybe we should just abort(); if malloc failed allocating a
		// few dozen bytes this early, we're _screwed_.
	}

	override_monitors_names.names = calloc(initsz, sizeof(char *));
	override_monitors_names.len = 0;
	override_monitors_names.cap = initsz;

	return;
}

/**
 * Add an entry to our monitor list
 *
 * Expecting: [Name:]WxH[+X[+Y]]
 */
void
add_layout_override_entry(const char *s)
{
	const char *tmp;
	int xpgret;
	int x, y;
	unsigned int width, height;

	if(override_monitors == NULL) {
		// alloc failed, so just give up; we'll fail in the end anyway...
		return;
	}

	// Got a name?
	tmp = strchr(s, ':');
	if(tmp != NULL && tmp != s) {
		// Stash the name
		override_monitors_names.names[override_monitors_names.len]
		        = strndup(s, tmp - s);
		// len advances below

		// Advance to geom
		s = tmp + 1;
	}
	// Advance whether we got a name or not, to keep in sync.
	override_monitors_names.len++;


	// Either way, s points at the geom now
	xpgret = XParseGeometry(s, &x, &y, &width, &height);

	// Width and height are non-optional.  If x/y aren't given, we assume
	// +0+0.  If we're given -0's, well, we don't _support_ that, but
	// XPG() turns them into positives for us, so just accept it...
	const int has_hw = (WidthValue | HeightValue);
	if((xpgret & has_hw) != has_hw) {
		twmrc_error_prefix();
		fprintf(stderr, "Need both height and width in '%s'\n", s);
		ParseError = true;
		// Don't bother free()'ing stuff, we're going to exit after
		// parse completes
		return;
	}
	if(!(xpgret & XValue)) {
		x = 0;
	}
	if(!(xpgret & YValue)) {
		y = 0;
	}


	// And stash it
	RAreaListAdd(override_monitors, RAreaNewStatic(x, y, width, height));

	// Whether we had a name for this 'monitor' or not, we need to
	// possibly grow the names list, since it has to stay in lockstep
	// with the areas as we add 'em.
	{
		char ***names = &override_monitors_names.names;
		int len = override_monitors_names.len;

		if(len == override_monitors_names.cap) {
			char **tnames = realloc(*names, (len + 1) * sizeof(char *));
			if(tnames == NULL) {
				abort();
			}
			*names = tnames;
			override_monitors_names.cap++;
		}
	}

	return;
}

/**
 * Finalize the override layout and store it up globally.
 */
void
proc_layout_override(void)
{
	RLayout *new_layout;

	// Guard
	if(RAreaListLen(override_monitors) < 1) {
		// Make this non-fatal, so an empty spec not-quite-quietly does
		// nothing.
		twmrc_error_prefix();
		fprintf(stderr, "no monitors specified, ignoring MonitorLayout\n");

		// Since it's non-fatal, we _do_ need to cleanup more
		// carefully...
		RAreaListFree(override_monitors);
		for(int i = 0; i < override_monitors_names.len ; i++) {
			free(override_monitors_names.names[i]);
		}
		free(override_monitors_names.names);
		return;
	}

	new_layout = RLayoutNew(override_monitors);
	RLayoutSetMonitorsNames(new_layout, override_monitors_names.names);
	// Silently stop paying attention to o_m_n.  Don't free() anything,
	// since new_layout now owns it.  If we get another MonitorLayout{}
	// block, it'll start over again with init(), and allocate new space.

#ifdef DEBUG
	fprintf(stderr, "Overridden layout: ");
	RLayoutPrint(new_layout);
#endif

	RLayoutFree(Scr->Layout);
	Scr->Layout = new_layout;
	return;
}
