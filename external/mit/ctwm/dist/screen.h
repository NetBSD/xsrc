/*
 * Copyright 1989 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising
 * or publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * M.I.T. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL M.I.T.
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/* 
 *  [ ctwm ]
 *
 *  Copyright 1992 Claude Lecommandeur.
 *            
 * Permission to use, copy, modify  and distribute this software  [ctwm] and
 * its documentation for any purpose is hereby granted without fee, provided
 * that the above  copyright notice appear  in all copies and that both that
 * copyright notice and this permission notice appear in supporting documen-
 * tation, and that the name of  Claude Lecommandeur not be used in adverti-
 * sing or  publicity  pertaining to  distribution of  the software  without
 * specific, written prior permission. Claude Lecommandeur make no represen-
 * tations  about the suitability  of this software  for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * Claude Lecommandeur DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL  IMPLIED WARRANTIES OF  MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL  Claude Lecommandeur  BE LIABLE FOR ANY SPECIAL,  INDIRECT OR
 * CONSEQUENTIAL  DAMAGES OR ANY  DAMAGES WHATSOEVER  RESULTING FROM LOSS OF
 * USE, DATA  OR PROFITS,  WHETHER IN AN ACTION  OF CONTRACT,  NEGLIGENCE OR
 * OTHER  TORTIOUS ACTION,  ARISING OUT OF OR IN  CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Claude Lecommandeur [ lecom@sic.epfl.ch ][ April 1992 ]
 */

/***********************************************************************
 *
 * $XConsortium: screen.h,v 1.62 91/05/01 17:33:09 keith Exp $
 *
 * twm per-screen data include file
 *
 * 11-3-88 Dave Payne, Apple Computer			File created
 *
 ***********************************************************************/

#ifndef _SCREEN_
#define _SCREEN_

#ifdef VMS
#include <decw$include/Xlib.h>
#include <decw$include/Xutil.h>
#include <decw$include/cursorfont.h>
#else
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#endif

#ifdef GNOME
#  include "gnome.h"
#endif /* GNOME */
#include "list.h"
#include "menus.h"
#include "iconmgr.h"
#include "vscreen.h"
#include "workmgr.h"

#define ICONIFY_NORMAL  0
#define ICONIFY_MOSAIC  1
#define ICONIFY_ZOOMIN  2
#define ICONIFY_ZOOMOUT 3
#define ICONIFY_SWEEP   4

struct _StdCmap {
    struct _StdCmap *next;		/* next link in chain */
    Atom atom;				/* property from which this came */
    int nmaps;				/* number of maps below */
    XStandardColormap *maps;		/* the actual maps */
};

#define SIZE_HINDENT 10
#define SIZE_VINDENT 2

struct _TitlebarPixmaps {
    Pixmap xlogo;
    Pixmap resize;
    Pixmap question;
    Pixmap menu;
    Pixmap delete;
};

struct ScreenInfo
{
    int screen;			/* the default screen */
    int d_depth;		/* copy of DefaultDepth(dpy, screen) */
    Visual *d_visual;		/* copy of DefaultVisual(dpy, screen) */
    int Monochrome;		/* is the display monochrome ? */
    int rootx;		        /* The x coordinate of the root window (virtual screen) relative to RealRoot */
    int rooty;		        /* The y coordinate of the root window (virtual screen) relative to RealRoot */
    int rootw;		        /* my copy of DisplayWidth(dpy, screen) */
    int rooth;	                /* my copy of DisplayHeight(dpy, screen) */

    int crootx;		        /* The x coordinate of the captive root window if any */
    int crooty;		        /* The y coordinate of the captive root window if any */
    int crootw;		        /* my copy of DisplayWidth(dpy, screen) */
    int crooth;	                /* my copy of DisplayHeight(dpy, screen) */

    int MaxWindowWidth;		/* largest window to allow */
    int MaxWindowHeight;	/* ditto */

    TwmWindow *FirstWindow;	/* the head of the twm window list */
    Colormaps RootColormaps;	/* the colormaps of the root window */

    Window Root;		/* the root window: the current virual screen */
    Window XineramaRoot;	/* the root window, may be CaptiveRoot or otherwise RealRoot */
    Window CaptiveRoot;		/* the captive root window, if any, or 0 */
    Window RealRoot;		/* the actual root window of the display */

/*
 *  +--RealRoot-----------------------------------------------------------+
 *  | the root of the display (most uses of this are probably incorrect!) |
 *  |                                                                     |
 *  |   +--CaptiveRoot--------------------------------------------------+ |
 *  |   | when captive window is used (most uses are likely incorrect!) | |
 *  |   |                                                               | |
 *  |   | +--XineramaRoot---------------------------------------------+ | |
 *  |   | | the root that encompasses all virual screens              | | |
 *  |   | |                                                           | | |
 *  |   | | +--Root-----------+ +--Root--------+ +--Root------------+ | | |
 *  |   | | | one or more     | | Most cases   | |                  | | | |
 *  |   | | | virtual screens | | use Root.    | |                  | | | |
 *  |   | | |                 | |              | |                  | | | |
 *  |   | | |                 | |              | |                  | | | |
 *  |   | | +-----------------+ +--------------+ +------------------+ | | |
 *  |   | +-----------------------------------------------------------+ | |
 *  |   +---------------------------------------------------------------+ |
 *  +---------------------------------------------------------------------+
 */

    Window SizeWindow;		/* the resize dimensions window */
    Window InfoWindow;		/* the information window */
    Window WindowMask;		/* the window masking the screen at startup */
    Window ShapeWindow;		/* an utilitary window for animated icons */

    Image   *WelcomeImage;
    GC       WelcomeGC;
    Colormap WelcomeCmap;
    Visual  *WelcomeVisual;

    name_list *ImageCache;	/* list of pixmaps */
    TitlebarPixmaps tbpm;	/* titlebar pixmaps */
    Image *UnknownImage;	/* the unknown icon pixmap */
    Pixmap siconifyPm;		/* the icon manager iconify pixmap */
    Pixmap pullPm;		/* pull right menu icon */
    unsigned int pullW, pullH;	/* size of pull right menu icon */
    char *HighlightPixmapName;	/* name of the hilite image if any */

    MenuRoot *MenuList;		/* head of the menu list */
    MenuRoot *LastMenu;		/* the last menu (mostly unused?) */
    MenuRoot *Windows;		/* the TwmWindows menu */
    MenuRoot *Icons;		/* the TwmIcons menu */
    MenuRoot *Workspaces;	/* the TwmWorkspaces menu */
    MenuRoot *AllWindows;	/* the TwmAllWindows menu */

  /*Added by dl 2004 */
    MenuRoot *AllIcons;         /* the TwmAllIcons menu */

  /******************************************************/
  /* Added by Dan Lilliehorn (dl@dl.nu) 2000-02-29)     */
    MenuRoot *Keys;             /* the TwmKeys menu     */
    MenuRoot *Visible;          /* thw TwmVisible menu  */

    TwmWindow *Ring;		/* one of the windows in window ring */
    TwmWindow *RingLeader;	/* current window in ring */

    MouseButton DefaultFunction;
    MouseButton WindowFunction;
    MouseButton ChangeWorkspaceFunction;
    MouseButton DeIconifyFunction;
    MouseButton IconifyFunction;

    struct {
      Colormaps *cmaps; 	/* current list of colormap windows */
      int maxCmaps;		/* maximum number of installed colormaps */
      unsigned long first_req;	/* seq # for first XInstallColormap() req in
				   pass thru loading a colortable list */
      int root_pushes;		/* current push level to install root
				   colormap windows */
      Colormaps *pushed_cmaps;	/* saved colormaps to install when pushes
				   drops to zero */
    } cmapInfo;

    struct {
	StdCmap *head, *tail;		/* list of maps */
	StdCmap *mru;			/* most recently used in list */
	int mruindex;			/* index of mru in entry */
    } StdCmapInfo;

    struct {
	int nleft, nright;		/* numbers of buttons in list */
	TitleButton *head;		/* start of list */
	int border;			/* button border */
	int pad;			/* button-padding */
	int width;			/* width of single button & border */
	int leftx;			/* start of left buttons */
	int titlex;			/* start of title */
	int rightoff;			/* offset back from right edge */
	int titlew;			/* width of title part */
    } TBInfo;
    ColorPair BorderTileC;	/* border tile colors */
    ColorPair TitleC;		/* titlebar colors */
    ColorPair MenuC;		/* menu colors */
    ColorPair MenuTitleC;	/* menu title colors */
    ColorPair IconC;		/* icon colors */
    ColorPair IconManagerC;	/* icon manager colors */
    ColorPair DefaultC;		/* default colors */
    ColorPair BorderColorC;	/* color of window borders */
    Pixel MenuShadowColor;	/* menu shadow color */
    Pixel IconBorderColor;	/* icon border color */
    Pixel IconManagerHighlight;	/* icon manager highlight */
    short ClearShadowContrast;  /* The contrast of the clear shadow */
    short DarkShadowContrast;   /* The contrast of the dark shadow */
    short IconJustification;	/* J_LEFT, J_CENTER or J_RIGHT */
    short IconRegionJustification;	/* J_LEFT, J_CENTER J_RIGHT or J_BORDER */
    short IconRegionAlignement;	/* J_TOP, J_CENTER, J_BOTTOM or J_BORDER */
    short TitleJustification;	/* J_LEFT, J_CENTER or J_RIGHT */
    short IconifyStyle;         /* ICONIFY_* */
    int   MaxIconTitleWidth;	/* */

    Cursor TitleCursor;		/* title bar cursor */
    Cursor FrameCursor;		/* frame cursor */
    Cursor IconCursor;		/* icon cursor */
    Cursor IconMgrCursor;	/* icon manager cursor */
    Cursor ButtonCursor;	/* title bar button cursor */
    Cursor MoveCursor;		/* move cursor */
    Cursor ResizeCursor;	/* resize cursor */
    Cursor WaitCursor;		/* wait a while cursor */
    Cursor MenuCursor;		/* menu cursor */
    Cursor SelectCursor;	/* dot cursor for f.move, etc. from menus */
    Cursor DestroyCursor;	/* skull and cross bones, f.destroy */
    Cursor AlterCursor;		/* cursor for alternate keymaps */

    WorkSpaceMgr workSpaceMgr;
    short	workSpaceManagerActive;

    virtualScreen *vScreenList;
    virtualScreen *currentvs;
    name_list     *VirtualScreens;

    name_list	*OccupyAll;	/* list of window names occupying all workspaces at startup */
    name_list	*UnmapByMovingFarAway;
    name_list	*DontSetInactive;
    name_list	*AutoSqueeze;
    name_list	*StartSqueezed;
    short 	use3Dmenus;
    short 	use3Dtitles;
    short 	use3Diconmanagers;
    short 	use3Dborders;
    short 	use3Dwmap;
    short	use3Diconborders;
    short	SunkFocusWindowTitle;
    short	WMgrVertButtonIndent;
    short	WMgrHorizButtonIndent;
    short	WMgrButtonShadowDepth;
    short	BeNiceToColormap;
    short	BorderCursors;
    short	BorderShadowDepth;
    short	TitleButtonShadowDepth;
    short	TitleShadowDepth;
    short	MenuShadowDepth;
    short	IconManagerShadowDepth;
    short	ReallyMoveInWorkspaceManager;
    short	ShowWinWhenMovingInWmgr;
    short	ReverseCurrentWorkspace;
    short	DontWarpCursorInWMap;
    short	XMoveGrid, YMoveGrid;
    short	FastServer;
    short	CenterFeedbackWindow;
    short	ShrinkIconTitles;
    short	AutoRaiseIcons;
    short       AutoFocusToTransients; /* kai */
    short       PackNewWindows;

    name_list *BorderColorL;
    name_list *IconBorderColorL;
    name_list *BorderTileForegroundL;
    name_list *BorderTileBackgroundL;
    name_list *TitleForegroundL;
    name_list *TitleBackgroundL;
    name_list *IconForegroundL;
    name_list *IconBackgroundL;
    name_list *IconManagerFL;
    name_list *IconManagerBL;
    name_list *IconMgrs;
    name_list *NoBorder;	/* list of window without borders          */
    name_list *NoIconTitle;	/* list of window names with no icon title */
    name_list *NoTitle;		/* list of window names with no title bar */
    name_list *MakeTitle;	/* list of window names with title bar */
    name_list *AutoRaise;	/* list of window names to auto-raise */
    name_list *AutoLower;	/* list of window names to auto-lower */
    name_list *IconNames;	/* list of window names and icon names */
    name_list *NoHighlight;	/* list of windows to not highlight */
    name_list *NoStackModeL;	/* windows to ignore stack mode requests */
    name_list *AlwaysOnTopL;	/* windows to keep on top */
    name_list *NoTitleHighlight;/* list of windows to not highlight the TB*/
    name_list *DontIconify;	/* don't iconify by unmapping */
    name_list *IconMgrNoShow;	/* don't show in the icon manager */
    name_list *IconMgrShow;	/* show in the icon manager */
    name_list *IconifyByUn;	/* windows to iconify by unmapping */
    name_list *StartIconified;	/* windows to start iconic */
    name_list *IconManagerHighlightL;	/* icon manager highlight colors */
    name_list *SqueezeTitleL;		/* windows of which to squeeze title */
    name_list *DontSqueezeTitleL;	/* windows of which not to squeeze */
    name_list *AlwaysSqueezeToGravityL;	/* windows which should squeeze toward gravity */
    name_list *WindowRingL;	/* windows in ring */
    name_list *WindowRingExcludeL;      /* windows excluded from ring */
    name_list *WarpCursorL;	/* windows to warp cursor to on deiconify */
    name_list *DontSave;
    name_list *WindowGeometries;
    name_list *IgnoreTransientL;

    name_list *OpaqueMoveList;
    name_list *NoOpaqueMoveList;
    name_list *OpaqueResizeList;
    name_list *NoOpaqueResizeList;
    name_list *IconMenuDontShow;

    GC NormalGC;		/* normal GC for everything */
    GC MenuGC;			/* gc for menus */
    GC DrawGC;			/* GC to draw lines for move and resize */
    GC BorderGC;		/* for drawing 3D borders */
    GC rootGC;                  /* used for allocating pixmaps in FindPixmap (util.c) */

    unsigned long Black;
    unsigned long White;
    unsigned long XORvalue;	/* number to use when drawing xor'ed */
    MyFont TitleBarFont;	/* title bar font structure */
    MyFont MenuFont;		/* menu font structure */
    MyFont IconFont;		/* icon font structure */
    MyFont SizeFont;		/* resize font structure */
    MyFont IconManagerFont;	/* window list font structure */
    MyFont DefaultFont;
    IconMgr *iconmgr;		/* default icon manager  */
    struct IconRegion *FirstRegion;	/* pointer to icon regions */
    struct IconRegion *LastRegion;	/* pointer to the last icon region */
    struct WindowRegion *FirstWindowRegion;	/* pointer to window regions */
    WindowBox *FirstWindowBox;	/* pointer to window boxes list */
    char *IconDirectory;	/* icon directory to search */
    char *PixmapDirectory;	/* Pixmap directory to search */
    int SizeStringOffset;	/* x offset in size window for drawing */
    int SizeStringWidth;	/* minimum width of size window */
    int BorderWidth;		/* border width of twm windows */
    int BorderLeft;
    int BorderRight;
    int BorderTop;
    int BorderBottom;
    int ThreeDBorderWidth;	/* 3D border width of twm windows */
    int IconBorderWidth;	/* border width of icon windows */
    int TitleHeight;		/* height of the title bar window */
    TwmWindow *Focus;		/* the twm window that has focus */
    int EntryHeight;		/* menu entry height */
    int FramePadding;		/* distance between decorations and border */
    int TitlePadding;		/* distance between items in titlebar */
    int ButtonIndent;		/* amount to shrink buttons on each side */
    int NumAutoRaises;		/* number of autoraise windows on screen */
    int NumAutoLowers;		/* number of autolower windows on screen */
    int TransientOnTop;		/* Percentage of the surface of it's leader */
    short AutoRaiseDefault;	/* AutoRaise all windows if true */
    short AutoLowerDefault;	/* AutoLower all windows if true */
    short NoDefaults;		/* do not add in default UI stuff */
    short UsePPosition;		/* what do with PPosition, see values below */
    short UseSunkTitlePixmap;
    short AutoRelativeResize;	/* start resize relative to position in quad */
    short FocusRoot;		/* is the input focus on the root ? */
    short WarpCursor;		/* warp cursor on de-iconify ? */
    short ForceIcon;		/* force the icon to the user specified */
    short NoGrabServer;		/* don't do server grabs */
    short NoRaiseMove;		/* don't raise window following move */
    short NoRaiseResize;	/* don't raise window following resize */
    short NoRaiseDeicon;	/* don't raise window on deiconify */
    short RaiseOnWarp;		/* do raise window on warp */
    short DontMoveOff;		/* don't allow windows to be moved off */
    int MoveOffResistance;	/* nb of pixel before moveOff gives up */
    int MovePackResistance;	/* nb of pixel before f.movepack gives up */
    short DoZoom;		/* zoom in and out of icons */
    short TitleFocus;		/* focus on window in title bar ? */
    short IconManagerFocus;	/* focus on iconified window ? */
    short NoIconTitlebar;	/* put title bars on icons */
    short NoTitlebar;		/* put title bars on windows */
    short DecorateTransients;	/* put title bars on transients */
    short IconifyByUnmapping;	/* simply unmap windows when iconifying */
    short ShowIconManager;	/* display the window list */
    short ShowWorkspaceManager;	/* display the workspace manager */
    short IconManagerDontShow;	/* show nothing in the icon manager */
    short AutoOccupy;		/* Do we automatically change occupation when name changes */
    short TransientHasOccupation;	/* Do transient-for windows have their own occupation */
    short DontPaintRootWindow;	/* don't paint anything on the root window */
    short BackingStore;		/* use backing store for menus */
    short SaveUnder;		/* use save under's for menus */
    short RandomPlacement;	/* randomly place windows that no give hints */
    short RandomDisplacementX;	/* randomly displace by this much horizontally */
    short RandomDisplacementY;	/* randomly displace by this much vertically */
    short OpaqueMove;		/* move the window rather than outline */
    short DoOpaqueMove;		/* move the window rather than outline */
    short OpaqueMoveThreshold;		/*  */
    short DoOpaqueResize;		/* resize the window rather than outline */
    short OpaqueResize;		/* resize the window rather than outline */
    short OpaqueResizeThreshold;	/*  */
    short Highlight;		/* should we highlight the window borders */
    short StackMode;		/* should we honor stack mode requests */
    short TitleHighlight;	/* should we highlight the titlebar */
    short MoveDelta;		/* number of pixels before f.move starts */
    short ZoomCount;		/* zoom outline count */
    short SortIconMgr;		/* sort entries in the icon manager */
    short Shadow;		/* show the menu shadow */
    short InterpolateMenuColors;/* make pretty menus */
    short StayUpMenus;		/* stay up menus */
    short WarpToDefaultMenuEntry; /* warp cursor to default menu entry, if any  */
    short ClickToFocus;		/* click to focus */
    short SloppyFocus;		/* "sloppy" focus */
    short SaveWorkspaceFocus;	/* Save and restore focus on workspace change. */
    short NoIconManagers;	/* Don't create any icon managers */
    short ClientBorderWidth;	/* respect client window border width */
    short SqueezeTitle;		/* make title as small as possible */
    short AlwaysSqueezeToGravity; /* squeeze toward gravity */
    short HaveFonts;		/* set if fonts have been loaded */
    short FirstTime;		/* first time we've read .twmrc */
    short CaseSensitive;	/* be case-sensitive when sorting names */
    short WarpUnmapped;		/* allow warping to unmapped windows */
    short WindowRingAll;	/* add all windows to the ring */
    short WarpRingAnyWhere;	/* warp to ring even if window is not visible */
    short ShortAllWindowsMenus;	/* Eliminates Icon and Workspace Managers */
    short OpenWindowTimeout;	/* Timeout when a window tries to open */
    short RaiseWhenAutoUnSqueeze;
    short RaiseOnClick;		/* Raise a window when clieked into */
    short RaiseOnClickButton;		/* Raise a window when clieked into */
    short IgnoreLockModifier;	/* Should we ignore the lock modifier */
    unsigned int IgnoreModifier;
    short IgnoreCaseInMenuSelection;	/* Should we ignore case in menu selection */
    short NoWarpToMenuTitle; /* warp cursor to clipped menu title */
    short NoImagesInWorkSpaceManager;   /* do not display mini images of the desktop background images on WSmap */

    FuncKey FuncKeyRoot;
    FuncButton FuncButtonRoot;

#ifdef GNOME
    GnomeData *gnomedata;
#endif /* GNOME */
};

extern int captive;
extern int MultiScreen;
extern int NumScreens;
extern ScreenInfo **ScreenList;
extern ScreenInfo *Scr;
extern int FirstScreen;

#define PPOS_OFF 0
#define PPOS_ON 1
#define PPOS_NON_ZERO 2
/* may eventually want an option for having the PPosition be the initial
   location for the drag lines */

#define RP_OFF 0
#define RP_ALL 1
#define RP_UNMAPPED 2

#define ONTOP_MAX 16
#define ONTOP_DEFAULT 8

#endif /* _SCREEN_ */
