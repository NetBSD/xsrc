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
#include <stdio.h>
#include <ctype.h>
#include "twm.h"
#include "util.h"
#include "parse.h"
#include "screen.h"
#include "icons.h"
#include "resize.h"
#include "add_window.h"
#include "events.h"
#include "clicktofocus.h"
#include "cursor.h"
#include "list.h"
#include "workmgr.h"
#ifdef VMS
#include <string.h>
#include <decw$include/Xos.h>
#include <decw$include/Xatom.h>
#include <X11Xmu/CharSet.h>
#include <decw$include/Xresource.h>
#else
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xmu/CharSet.h>
#include <X11/Xresource.h>
#endif
#ifdef macII
int strcmp(); /* missing from string.h in AUX 2.0 */
#endif
#ifdef BUGGY_HP700_SERVER
static void fakeRaiseLower ();
#endif

#ifdef GNOME
#  include "gnomewindefs.h" /* include GNOME hints definitions */
   extern Atom _XA_WIN_WORKSPACE;
   extern Atom _XA_WIN_STATE;
#endif /* GNOME */

extern void twmrc_error_prefix (void); /* in gram.c */
extern char *captivename;

/***********************************************************************
 *
 *  Procedure:
 *	CreateWorkSpaceManager - create the workspace manager window
 *		for this screen.
 *
 *  Returned Value:
 *	none
 *
 *  Inputs:
 *	none
 *
 ***********************************************************************
 */
#define WSPCWINDOW    0
#define OCCUPYWINDOW  1
#define OCCUPYBUTTON  2

static void Vanish			(virtualScreen *vs,
					 TwmWindow *tmp_win);
static void DisplayWin			(virtualScreen *vs,
					 TwmWindow *tmp_win);
static void CreateWorkSpaceManagerWindow (virtualScreen *vs);
static void CreateOccupyWindow		(void);
static unsigned int GetMaskFromResource	(TwmWindow *win, char *res);
static int GetPropertyFromMask		(unsigned int mask, char *prop,
					 long *gwkspc);
static void PaintWorkSpaceManagerBorder	(virtualScreen *vs);
static void PaintButton			(int which,
					 virtualScreen *vs, Window w,
					 char *label,
					 ColorPair cp, int state);
static void WMapRemoveFromList		(TwmWindow *win, WorkSpace *ws);
static int WMapWindowMayBeAdded         (TwmWindow *win);
static void WMapAddToList		(TwmWindow *win, WorkSpace *ws);
static void ResizeWorkSpaceManager	(virtualScreen *vs, TwmWindow *win);
static void ResizeOccupyWindow		(TwmWindow *win);
static WorkSpace *GetWorkspace		(char *wname);
static void WMapRedrawWindow		(Window window, int width, int height,
					 ColorPair cp, char *label);
static int CanChangeOccupation          (TwmWindow **twm_winp);
void safecopy                           (char *dest, char *src, int size);

Atom _XA_WM_OCCUPATION;
Atom _XA_WM_CURRENTWORKSPACE;
Atom _XA_WM_WORKSPACESLIST;
Atom _XA_WM_CTWMSLIST;
Atom _XA_WM_CTWM_VSCREENMAP;
Atom _OL_WIN_ATTR;

int       fullOccupation    = 0;
int       useBackgroundInfo = False;
XContext  MapWListContext = (XContext) 0;
static Cursor handCursor  = (Cursor) 0;
static Bool DontRedirect (Window window);

extern Bool donttoggleworkspacemanagerstate;
extern Bool MaybeAnimate;
extern FILE *tracefile;

void InitWorkSpaceManager (void)
{
    Scr->workSpaceMgr.count	    = 0;
    Scr->workSpaceMgr.workSpaceList = NULL;
    Scr->workSpaceMgr.initialstate  = BUTTONSSTATE;
    Scr->workSpaceMgr.geometry      = NULL;
    Scr->workSpaceMgr.buttonStyle   = STYLE_NORMAL;
    Scr->workSpaceMgr.windowcp.back = Scr->White;
    Scr->workSpaceMgr.windowcp.fore = Scr->Black;
    Scr->workSpaceMgr.windowcpgiven = False;

    Scr->workSpaceMgr.occupyWindow = calloc(1, sizeof (OccupyWindow));
    Scr->workSpaceMgr.occupyWindow->name      = "Occupy Window";
    Scr->workSpaceMgr.occupyWindow->icon_name = "Occupy Window Icon";
    Scr->workSpaceMgr.occupyWindow->geometry  = NULL;
    Scr->workSpaceMgr.occupyWindow->columns   = 0;
    Scr->workSpaceMgr.occupyWindow->twm_win   = (TwmWindow*) 0;
    Scr->workSpaceMgr.occupyWindow->vspace    = Scr->WMgrVertButtonIndent;
    Scr->workSpaceMgr.occupyWindow->hspace    = Scr->WMgrHorizButtonIndent;

    Scr->workSpaceMgr.curColors.back  = Scr->Black;
    Scr->workSpaceMgr.curColors.fore  = Scr->White;
    Scr->workSpaceMgr.defColors.back  = Scr->White;
    Scr->workSpaceMgr.defColors.fore  = Scr->Black;	
    Scr->workSpaceMgr.curImage        = None;
    Scr->workSpaceMgr.curPaint        = False;
    Scr->workSpaceMgr.defImage        = None;
    Scr->workSpaceMgr.vspace          = Scr->WMgrVertButtonIndent;
    Scr->workSpaceMgr.hspace          = Scr->WMgrHorizButtonIndent;
    Scr->workSpaceMgr.name	      = "WorkSpaceManager";
    Scr->workSpaceMgr.icon_name       = "WorkSpaceManager Icon";

    Scr->workSpaceMgr.windowFont.basename =
      "-adobe-courier-medium-r-normal--10-100-75-75-m-60-iso8859-1";
    /*"-adobe-courier-bold-r-normal--8-80-75-75-m-50-iso8859-1";*/

    XrmInitialize ();
    if (MapWListContext == (XContext) 0) MapWListContext = XUniqueContext ();
}

void ConfigureWorkSpaceManager (void) {
    virtualScreen *vs;

    for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
	/*
	 * Make sure this is all properly initialized to nothing.  Otherwise
	 * bad and undefined behavior can show up in certain cases (e.g.,
	 * with no Workspaces {} defined in .ctwmrc, the only defined
	 * workspace will be random memory bytes, which can causes crashes on
	 * e.g.  f.menu "TwmWindows".)
	 */
	WorkSpaceWindow *wsw = (WorkSpaceWindow*) calloc (1, sizeof (WorkSpaceWindow));
	wsw->state = Scr->workSpaceMgr.initialstate; /* BUTTONSSTATE */
	vs->wsw = wsw;
    }
}

void CreateWorkSpaceManager (void)
{
    char wrkSpcList [512];
    char vsmapbuf    [1024], *vsmap;
    virtualScreen    *vs;
    WorkSpace        *ws, *fws;
    int len, vsmaplen;
    long junk;

    if (! Scr->workSpaceManagerActive) return;

    Scr->workSpaceMgr.windowFont.basename =
      "-adobe-courier-medium-r-normal--10-100-75-75-m-60-iso8859-1";
    Scr->workSpaceMgr.buttonFont = Scr->IconManagerFont;
    Scr->workSpaceMgr.cp	 = Scr->IconManagerC;
    if (!Scr->BeNiceToColormap) GetShadeColors (&Scr->workSpaceMgr.cp);

    _XA_WM_OCCUPATION       = XInternAtom (dpy, "WM_OCCUPATION",        False);
    _XA_WM_CURRENTWORKSPACE = XInternAtom (dpy, "WM_CURRENTWORKSPACE",  False);
    _XA_WM_CTWM_VSCREENMAP  = XInternAtom (dpy, "WM_CTWM_VSCREENMAP", False);
#ifdef GNOME
    _XA_WM_WORKSPACESLIST   = XInternAtom (dpy, "_WIN_WORKSPACE_NAMES", False);
#else /* GNOME */
    _XA_WM_WORKSPACESLIST   = XInternAtom (dpy, "WM_WORKSPACESLIST", False);
#endif /* GNOME */
    _OL_WIN_ATTR            = XInternAtom (dpy, "_OL_WIN_ATTR",         False);

    NewFontCursor (&handCursor, "top_left_arrow");

    vsmaplen = sizeof(vsmapbuf);
    if(CtwmGetVScreenMap(dpy, Scr->Root, vsmapbuf, &vsmaplen) == True)
	vsmap = strtok(vsmapbuf, ",");
    else
	vsmap = NULL;

    /*
     * weird things can happen if the config file is changed or the atom
     * returned above is messed with.  Sometimes windows may disappear in
     * that case depending on what's changed.  (depending on where they were
     * on the actual screen.
     */
    ws = Scr->workSpaceMgr.workSpaceList;
    for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
      WorkSpaceWindow *wsw = vs->wsw;
      if(vsmap)
          fws = GetWorkspace(vsmap);
      else
          fws = NULL;
      if(fws) {
          wsw->currentwspc = fws;
	  vsmap = strtok(NULL, ",");
      } else {
          wsw->currentwspc = ws;
          ws = ws->next;
      }
      CreateWorkSpaceManagerWindow (vs);
    }
    CreateOccupyWindow ();

    for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
      WorkSpaceWindow *wsw = vs->wsw;
      WorkSpace *ws2 = wsw->currentwspc;
      MapSubwindow *msw = wsw->mswl [ws2->number];
      if (Scr->workSpaceMgr.curImage == None) {
	if (Scr->workSpaceMgr.curPaint) {
	  XSetWindowBackground (dpy, msw->w, Scr->workSpaceMgr.curColors.back);
	}
      } else {
	XSetWindowBackgroundPixmap (dpy, msw->w, Scr->workSpaceMgr.curImage->pixmap);
      }
      XSetWindowBorder (dpy, msw->w, Scr->workSpaceMgr.curBorderColor);
      XClearWindow (dpy, msw->w);

      if (useBackgroundInfo && ! Scr->DontPaintRootWindow) {
	if (ws2->image == None)
	    XSetWindowBackground       (dpy, vs->window, ws2->backcp.back);
	else
	    XSetWindowBackgroundPixmap (dpy, vs->window, ws2->image->pixmap);
	XClearWindow (dpy, vs->window);
      }
    }
    len = GetPropertyFromMask (0xFFFFFFFFu, wrkSpcList, &junk);
    XChangeProperty (dpy, Scr->Root, _XA_WM_WORKSPACESLIST, XA_STRING, 8, 
		     PropModeReplace, (unsigned char *) wrkSpcList, len);
}

void GotoWorkSpaceByName (virtualScreen *vs, char *wname)
{
    WorkSpace *ws;

    if (! Scr->workSpaceManagerActive) return;
    if (!vs) return;
    ws = GetWorkspace(wname);
    if (ws == NULL) return;
    GotoWorkSpace (vs, ws);
}

/* 6/19/1999 nhd for GNOME compliance */
void GotoWorkSpaceByNumber(virtualScreen *vs, int workspacenum)
{
    WorkSpace *ws;
    if(! Scr->workSpaceManagerActive) return;
    if (!vs) return;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
        if (ws->number == workspacenum) break;
    }
    if (ws == NULL) return;
    GotoWorkSpace (vs, ws);
}
 /* */
 
void GotoPrevWorkSpace (virtualScreen *vs)
{
    WorkSpace *ws1, *ws2;

    if (! Scr->workSpaceManagerActive) return;
    if (!vs) return;
    ws1 = Scr->workSpaceMgr.workSpaceList;
    if (ws1 == NULL) return;
    ws2 = ws1->next;

    while ((ws2 != vs->wsw->currentwspc) && (ws2 != NULL)) {
	ws1 = ws2;
	ws2 = ws2->next;
    }
    GotoWorkSpace (vs, ws1);
}

void GotoNextWorkSpace (virtualScreen *vs)
{
    WorkSpace *ws;
    if (! Scr->workSpaceManagerActive) return;
    if (!vs) return;

    ws = vs->wsw->currentwspc;
    ws = (ws->next != NULL) ? ws->next : Scr->workSpaceMgr.workSpaceList;
    GotoWorkSpace (vs, ws);
}

void GotoRightWorkSpace (virtualScreen *vs)
{
    WorkSpace *ws;
    int number, columns, count;

    if (!Scr->workSpaceManagerActive)
	return;
    if (!vs)
	return;

    ws      = vs->wsw->currentwspc;
    number  = ws->number;
    columns = Scr->workSpaceMgr.columns;
    count   = Scr->workSpaceMgr.count;
    number++;
    if ((number % columns) == 0)
	number -= columns;
    else if (number >= count)
	number = (number / columns) * columns;

    GotoWorkSpaceByNumber(vs, number);
}

void GotoLeftWorkSpace (virtualScreen *vs)
{
    WorkSpace *ws;
    int number, columns, count;

    if (!Scr->workSpaceManagerActive)
	return;
    if (!vs)
	return;

    ws      = vs->wsw->currentwspc;
    number  = ws->number;
    columns = Scr->workSpaceMgr.columns;
    count   = Scr->workSpaceMgr.count;
    number += (number % columns) ? -1 : (columns - 1);
    if (number >= count)
	number = count - 1;
    GotoWorkSpaceByNumber(vs, number);
}

void GotoUpWorkSpace (virtualScreen *vs)
{
    WorkSpace *ws;
    int number, lines, columns, count;

    if (!Scr->workSpaceManagerActive)
	return;
    if (!vs)
	return;

    ws      = vs->wsw->currentwspc;
    number  = ws->number;
    lines   = Scr->workSpaceMgr.lines;
    columns = Scr->workSpaceMgr.columns;
    count   = Scr->workSpaceMgr.count;
    number -=  columns;
    if (number < 0) {
	number += lines * columns;
	/* If the number of workspaces is not a multiple of nr of columns */
	if (number >= count)
	    number -= columns;
    }
    GotoWorkSpaceByNumber(vs, number);
}

void GotoDownWorkSpace (virtualScreen *vs)
{
    WorkSpace *ws;
    int number, columns, count;

    if (!Scr->workSpaceManagerActive)
	return;
    if (!vs)
	return;

    ws      = vs->wsw->currentwspc;
    number  = ws->number;
    columns = Scr->workSpaceMgr.columns;
    count   = Scr->workSpaceMgr.count;
    number +=  columns;
    if (number >= count) {
	number %= columns;
    }
    GotoWorkSpaceByNumber(vs, number);
}

void ShowBackground (virtualScreen *vs)
{
  static int state = 0;
  TwmWindow *twmWin;

  if (state) {
    for (twmWin = Scr->FirstWindow; twmWin != NULL; twmWin = twmWin->next) {
      if (twmWin->savevs == vs) {
	DisplayWin (vs, twmWin);
      }
      twmWin->savevs = NULL;
    }
    state = 0;
  } else {
    for (twmWin = Scr->FirstWindow; twmWin != NULL; twmWin = twmWin->next) {
      if (twmWin->vs == vs) {
	twmWin->savevs = twmWin->vs;
	Vanish (vs, twmWin);
      }
    }
    state = 1;
  }
}

void GotoWorkSpace (virtualScreen *vs, WorkSpace *ws)
{
    TwmWindow		 *twmWin;
    WorkSpace		 *oldws, *newws;
    WList		 *wl, *wl1;
    WinList		 winl;
    XSetWindowAttributes attr;
    XWindowAttributes    winattrs;
    unsigned long	 eventMask;
    IconMgr		 *iconmgr;
    Window		 oldw;
    Window		 neww;
    TwmWindow		 *focuswindow;
    TwmWindow		 *last_twmWin = NULL;
    virtualScreen	 *tmpvs;

    if (! Scr->workSpaceManagerActive) return;
    for (tmpvs = Scr->vScreenList; tmpvs != NULL; tmpvs = tmpvs->next) {
      if (ws == tmpvs->wsw->currentwspc) {
	XBell (dpy, 0);
	return;
      }
    }
    oldws = vs->wsw->currentwspc;
    newws = ws;
    if (oldws == newws) return;

    attr.backing_store = NotUseful;
    attr.save_under    = False;

    if (useBackgroundInfo && ! Scr->DontPaintRootWindow) {
	if (newws->image == None)
	    XSetWindowBackground       (dpy, vs->window, newws->backcp.back);
	else
	    XSetWindowBackgroundPixmap (dpy, vs->window, newws->image->pixmap);
	XClearWindow (dpy, vs->window);
    }

    /* If SaveWorkspaceFocus is on, save the focus of the last window. */
    if ( Scr->SaveWorkspaceFocus ) {
        oldws->save_focus = Scr->Focus;
    }

    focuswindow = (TwmWindow *)NULL;
    for (twmWin = Scr->FirstWindow; twmWin != NULL; twmWin = twmWin->next) {
	if (twmWin->vs == vs) {
	  if (!OCCUPY (twmWin, newws)) {
	    virtualScreen *tvs;
	    Vanish (vs, twmWin);
	    for (tvs = Scr->vScreenList; tvs != NULL; tvs = tvs->next) {
	      if (tvs == vs) continue;
	      if (OCCUPY (twmWin, tvs->wsw->currentwspc)) {
		DisplayWin (tvs, twmWin);
		break;
	      }
	    }
	  } else if (twmWin->hasfocusvisible) {
	      focuswindow = twmWin;
	      SetFocusVisualAttributes (focuswindow, False);
	  }
	}
    }
    /* Move to the end of the twmWin list */
    for (twmWin = Scr->FirstWindow; twmWin != NULL; twmWin = twmWin->next) {
	last_twmWin = twmWin;
    }
    /* Iconise in reverse order */
    for (twmWin = last_twmWin; twmWin != NULL; twmWin = twmWin->prev) {
	if (OCCUPY (twmWin, newws) && !twmWin->vs) DisplayWin (vs, twmWin);
    }
/*
   Reorganize icon manager window lists
*/
    for (twmWin = Scr->FirstWindow; twmWin != NULL; twmWin = twmWin->next) {
	wl = twmWin->iconmanagerlist;
	if (wl == NULL) continue;
	if (OCCUPY (wl->iconmgr->twm_win, newws)) continue;
	wl1 = wl;
	wl  = wl->nextv;
	while (wl != NULL) {
	    if (OCCUPY (wl->iconmgr->twm_win, newws)) break;
	    wl1 = wl;
	    wl  = wl->nextv;
	}
	if (wl != NULL) {
	    wl1->nextv = wl->nextv;
	    wl->nextv  = twmWin->iconmanagerlist;
	    twmWin->iconmanagerlist = wl;
	}
    }
    wl = (WList*)0;
    for (iconmgr = newws->iconmgr; iconmgr; iconmgr = iconmgr->next) {
	if (iconmgr->first) {
	    wl = iconmgr->first;
	    break;
	}
    }	
    CurrentIconManagerEntry (wl);
    if (focuswindow) {
	SetFocusVisualAttributes (focuswindow, True);
    }
    vs->wsw->currentwspc = newws;
    if (Scr->ReverseCurrentWorkspace && vs->wsw->state == MAPSTATE) {
        MapSubwindow *msw = vs->wsw->mswl [oldws->number];
	for (winl = msw->wl; winl != NULL; winl = winl->next) {
	    WMapRedrawName (vs, winl);
	}
	msw = vs->wsw->mswl [newws->number];
	for (winl = msw->wl; winl != NULL; winl = winl->next) {
	    WMapRedrawName (vs, winl);
	}
    } else
    if (vs->wsw->state == BUTTONSSTATE) {
        ButtonSubwindow *bsw = vs->wsw->bswl [oldws->number];
	PaintButton (WSPCWINDOW, vs, bsw->w, oldws->label, oldws->cp, off);
	bsw = vs->wsw->bswl [newws->number];
	PaintButton (WSPCWINDOW, vs, bsw->w, newws->label, newws->cp,  on);
    }
    oldws->iconmgr = Scr->iconmgr;
    Scr->iconmgr = newws->iconmgr;

    oldw = vs->wsw->mswl [oldws->number]->w;
    neww = vs->wsw->mswl [newws->number]->w;
    if (useBackgroundInfo) {
	if (oldws->image == None || Scr->NoImagesInWorkSpaceManager)
	    XSetWindowBackground       (dpy, oldw, oldws->backcp.back);
	else
	    XSetWindowBackgroundPixmap (dpy, oldw, oldws->image->pixmap);
    }
    else {
	if (Scr->workSpaceMgr.defImage == None || Scr->NoImagesInWorkSpaceManager)
	    XSetWindowBackground       (dpy, oldw, Scr->workSpaceMgr.defColors.back);
	else
	    XSetWindowBackgroundPixmap (dpy, oldw, Scr->workSpaceMgr.defImage->pixmap);
    }
    attr.border_pixel = Scr->workSpaceMgr.defBorderColor;
    XChangeWindowAttributes (dpy, oldw, CWBorderPixel, &attr);

    if (Scr->workSpaceMgr.curImage == None) {
	if (Scr->workSpaceMgr.curPaint) XSetWindowBackground (dpy, neww, Scr->workSpaceMgr.curColors.back);
    }
    else {
	XSetWindowBackgroundPixmap (dpy, neww, Scr->workSpaceMgr.curImage->pixmap);
    }
    attr.border_pixel =  Scr->workSpaceMgr.curBorderColor;
    XChangeWindowAttributes (dpy, neww, CWBorderPixel, &attr);

    XClearWindow (dpy, oldw);
    XClearWindow (dpy, neww);

    XGetWindowAttributes(dpy, Scr->Root, &winattrs);
    eventMask = winattrs.your_event_mask;
    XSelectInput(dpy, Scr->Root, eventMask & ~PropertyChangeMask);

    XChangeProperty (dpy, Scr->Root, _XA_WM_CURRENTWORKSPACE, XA_STRING, 8, 
		     PropModeReplace, (unsigned char *) newws->name, strlen (newws->name));
#ifdef GNOME
/* nhd 6/19/1999 for GNOME compliance
 * Publish which workspace the root window shows/contains.
 * Olaf Rhialto Seibert: However, don't do it when the root window is
 * captive, since it will be moved itself: for non-root windows this
 * property is used to indicate in which workspace it is contained.
 */
    
    if (!Scr->CaptiveRoot)
	XChangeProperty (dpy, Scr->Root, _XA_WIN_WORKSPACE, XA_CARDINAL, 32,
		    PropModeReplace, (unsigned char *) &(newws->number), 1);
#endif /* GNOME */
    XSelectInput(dpy, Scr->Root, eventMask);

    /*    XDestroyWindow (dpy, cachew);*/
    if (Scr->ChangeWorkspaceFunction.func != 0) {
	char *action;
	XEvent event;

	action = Scr->ChangeWorkspaceFunction.item ?
		Scr->ChangeWorkspaceFunction.item->action : NULL;
	ExecuteFunction (Scr->ChangeWorkspaceFunction.func, action,
			   (Window) 0, (TwmWindow*) 0, &event, C_ROOT, FALSE);
    }

    /* If SaveWorkspaceFocus is on, try to restore the focus to the last
       window which was focused when we left this workspace. */
    if (Scr->SaveWorkspaceFocus && newws->save_focus) {
	twmWin = newws->save_focus;
	if (OCCUPY(twmWin, newws)) {	/* check should not even be needed anymore */
	    WarpToWindow(twmWin, 0);
	} else {
	    newws->save_focus = NULL;
	}
    }

    /* keep track of the order of the workspaces across restarts */
    CtwmSetVScreenMap(dpy, Scr->Root, Scr->vScreenList);

    XSync (dpy, 0);
    if (Scr->ClickToFocus || Scr->SloppyFocus) set_last_window (newws);
    MaybeAnimate = True;
}

char *GetCurrentWorkSpaceName (virtualScreen *vs)
{
    if (! Scr->workSpaceManagerActive) return (NULL);
    if (!vs) vs = Scr->vScreenList;
    return vs->wsw->currentwspc->name;
}

void AddWorkSpace (char *name, char *background, char *foreground,
		   char *backback, char *backfore, char *backpix)
{
    WorkSpace *ws;
    int	      wsnum;
    Image     *image;

    wsnum = Scr->workSpaceMgr.count;
    if (wsnum == MAXWORKSPACE) return;

    fullOccupation |= (1 << wsnum);
    ws = (WorkSpace*) malloc (sizeof (WorkSpace));
    ws->FirstWindowRegion = NULL;
#if 0 /* def VMS */
    {
       char *ftemp;
       ftemp = (char *) malloc((strlen(name)+1)*sizeof(char));
       ws->name = strcpy (ftemp,name);
       ftemp = (char *) malloc((strlen(name)+1)*sizeof(char));
       ws->label = strcpy (ftemp,name);
    }
#else
    ws->name  = (char*) strdup (name);
    ws->label = (char*) strdup (name);
#endif
    ws->clientlist = NULL;
    ws->save_focus = NULL;

    if (background == NULL)
	ws->cp.back = Scr->IconManagerC.back;
    else
	GetColor (Scr->Monochrome, &(ws->cp.back), background);

    if (foreground == NULL)
	ws->cp.fore = Scr->IconManagerC.fore;
    else
	GetColor (Scr->Monochrome, &(ws->cp.fore), foreground);

#ifdef COLOR_BLIND_USER
    ws->cp.shadc = Scr->White;
    ws->cp.shadd = Scr->Black;
#else
    if (!Scr->BeNiceToColormap) GetShadeColors (&ws->cp);
#endif

    if (backback == NULL)
	GetColor (Scr->Monochrome, &(ws->backcp.back), "Black");
    else {
	GetColor (Scr->Monochrome, &(ws->backcp.back), backback);
	useBackgroundInfo = True;
    }

    if (backfore == NULL)
	GetColor (Scr->Monochrome, &(ws->backcp.fore), "White");
    else {
	GetColor (Scr->Monochrome, &(ws->backcp.fore), backfore);
	useBackgroundInfo = True;
    }
    if ((image = GetImage (backpix, ws->backcp)) != None) {
	ws->image = image;
	useBackgroundInfo = True;
    }
    else {
	ws->image = None;
    }
    ws->next   = NULL;
    ws->number = wsnum;
    Scr->workSpaceMgr.count++;

    if (Scr->workSpaceMgr.workSpaceList == NULL) {
	Scr->workSpaceMgr.workSpaceList = ws;
    }
    else {
	WorkSpace *wstmp = Scr->workSpaceMgr.workSpaceList;
	while (wstmp->next != NULL) { wstmp = wstmp->next; }
	wstmp->next = ws;
    }
    Scr->workSpaceManagerActive = 1;
}

static XrmOptionDescRec table [] = {
    {"-xrm",		NULL,		XrmoptionResArg, (XPointer) NULL},
};

void SetupOccupation (TwmWindow *twm_win,
		      int occupation_hint) /* <== [ Matthew McNeill Feb 1997 ] == */
{
    TwmWindow		*t;
    unsigned char	*prop;
    unsigned long	nitems, bytesafter;
    Atom		actual_type;
    int			actual_format;
    int			state;
    Window		icon;
    char		**cliargv = NULL;
    int			cliargc;
    Bool		status;
    char		*str_type;
    XrmValue		value;
    char		wrkSpcList [512];
    int			len;
    WorkSpace    	*ws;
    XWindowAttributes winattrs;
    unsigned long     eventMask;
    XrmDatabase       db = NULL;
    virtualScreen     *vs;
    long gwkspc = 0; /* for GNOME - which workspace we occupy */

    if (! Scr->workSpaceManagerActive) {
	twm_win->occupation = 1 << 0;   /* occupy workspace #0 */
	/*
	 * Choose some valid virtual screen.
	 * InitVirtualScreens() always seems to set this to non-NULL.
	 */
	twm_win->vs = Scr->vScreenList; /* only one virtual screen */
	/* more?... */

	return;
    }
    if (twm_win->wspmgr) return;

    /*twm_win->occupation = twm_win->iswinbox ? fullOccupation : 0;*/
    twm_win->occupation = 0;

    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	if (LookInList (ws->clientlist, twm_win->full_name, &twm_win->class)) {
            twm_win->occupation |= 1 << ws->number;
	}
    }

    if (LookInList (Scr->OccupyAll, twm_win->full_name, &twm_win->class)) {
        twm_win->occupation = fullOccupation;
    }

    if (XGetCommand (dpy, twm_win->w, &cliargv, &cliargc)) {
	XrmParseCommand (&db, table, 1, "ctwm", &cliargc, cliargv);
	status = XrmGetResource (db, "ctwm.workspace", "Ctwm.Workspace", &str_type, &value);
	if ((status == True) && (value.size != 0)) {
	    strncpy (wrkSpcList, value.addr, value.size);
	    twm_win->occupation = GetMaskFromResource (twm_win, wrkSpcList);
	}
	XrmDestroyDatabase (db);
	XFreeStringList (cliargv);
    }

    if (RestartPreviousState) {
	if (XGetWindowProperty (dpy, twm_win->w, _XA_WM_OCCUPATION, 0L, 2500, False,
				XA_STRING, &actual_type, &actual_format, &nitems,
				&bytesafter, &prop) == Success) {
	    if (nitems != 0) {
		twm_win->occupation = GetMaskFromProperty (prop, nitems);
		XFree ((char *) prop);
	    }
	}
    }

    if (twm_win->iconmgr) return; /* someone tried to modify occupation of icon managers */

    if (! Scr->TransientHasOccupation) {
	if (twm_win->transient) {
	    t = GetTwmWindow(twm_win->transientfor);
	    if (t != NULL) twm_win->occupation = t->occupation;
	}
	else
	if (twm_win->group != 0) {
	    t = GetTwmWindow(twm_win->group);
	    if (t != NULL) twm_win->occupation = t->occupation;
	}
    }

    /*============[ Matthew McNeill Feb 1997 ]========================*
     * added in functionality of specific occupation state. The value 
     * should be a valid occupation bit-field or 0 for the default action
     */

    if (occupation_hint != 0)
      twm_win->occupation = occupation_hint;

    /*================================================================*/

    if ((twm_win->occupation & fullOccupation) == 0) {
      vs = Scr->currentvs;
      if (vs && vs->wsw->currentwspc) 
	twm_win->occupation = 1 << vs->wsw->currentwspc->number;
      else {
	twm_win->occupation = 1;
      }
    }
    twm_win->vs = NULL;
    for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
      if (OCCUPY (twm_win, vs->wsw->currentwspc)) {
	twm_win->vs = vs;
	break;
      }
    }
	
    len = GetPropertyFromMask (twm_win->occupation, wrkSpcList, &gwkspc);

    if (!XGetWindowAttributes(dpy, twm_win->w, &winattrs)) return;
    eventMask = winattrs.your_event_mask;
    XSelectInput(dpy, twm_win->w, eventMask & ~PropertyChangeMask);

    XChangeProperty (dpy, twm_win->w, _XA_WM_OCCUPATION, XA_STRING, 8, 
		     PropModeReplace, (unsigned char *) wrkSpcList, len);
#ifdef GNOME
    XChangeProperty (dpy, twm_win->w, _XA_WIN_WORKSPACE, XA_CARDINAL, 32,
		     PropModeReplace, (unsigned char *)(&gwkspc), 1);
    if (XGetWindowProperty (dpy, twm_win->w, _XA_WIN_STATE, 0L, 32, False,
			    XA_CARDINAL, &actual_type, &actual_format, &nitems,
			    &bytesafter, &prop) 
	!= Success || nitems == 0) {
	gwkspc = 0;
    } else {
	gwkspc = (int)*prop;
	XFree ((char *)prop);
    }
    if (twm_win->occupation == fullOccupation)
      gwkspc |= WIN_STATE_STICKY;
    else
      gwkspc &= ~WIN_STATE_STICKY;
    XChangeProperty (dpy, twm_win->w, _XA_WIN_STATE, XA_CARDINAL, 32, 
		     PropModeReplace, (unsigned char *)&gwkspc, 1); 
#endif /* GNOME */
    XSelectInput (dpy, twm_win->w, eventMask);

/* kludge */
    state = NormalState;
    if (!(RestartPreviousState && GetWMState (twm_win->w, &state, &icon) &&
	 (state == NormalState || state == IconicState || state == InactiveState))) {
	if (twm_win->wmhints && (twm_win->wmhints->flags & StateHint))
	    state = twm_win->wmhints->initial_state;
    }
    if (visible (twm_win)) {
	if (state == InactiveState) SetMapStateProp (twm_win, NormalState);
    } else {
	if (state ==   NormalState) SetMapStateProp (twm_win, InactiveState);
    }
}

void safecopy(char *dest, char *src, int size)
{
    strncpy(dest, src, size - 1);
    dest[size - 1] = '\0';
}

Bool RedirectToCaptive (Window window)
{
    unsigned long	nitems, bytesafter;
    Atom		actual_type;
    int			actual_format;
    char		**cliargv = NULL;
    int			cliargc;
    Bool		status;
    char		*str_type;
    XrmValue		value;
    int			ret;
    Atom		_XA_WM_CTWM_ROOT;
    char		*atomname;
    Window		newroot;
    XWindowAttributes	wa;
    XrmDatabase         db = NULL;

    if (DontRedirect (window)) return (False);
    if (!XGetCommand (dpy, window, &cliargv, &cliargc)) return (False);
    XrmParseCommand (&db, table, 1, "ctwm", &cliargc, cliargv);
    if (db == NULL) {
        if (cliargv) XFreeStringList (cliargv);
	return False;
    }
    ret = False;
    status = XrmGetResource (db, "ctwm.redirect", "Ctwm.Redirect", &str_type, &value);
    if ((status == True) && (value.size != 0)) {
	char	         cctwm [64];
	Window		*prop;

	safecopy (cctwm, value.addr, sizeof(cctwm));
	atomname = (char*) malloc (strlen ("WM_CTWM_ROOT_") + strlen (cctwm) + 1);
	sprintf (atomname, "WM_CTWM_ROOT_%s", cctwm);
	_XA_WM_CTWM_ROOT = XInternAtom (dpy, atomname, False);
	
	if (XGetWindowProperty (dpy, Scr->Root, _XA_WM_CTWM_ROOT,
		0L, 1L, False, AnyPropertyType, &actual_type, &actual_format,
		&nitems, &bytesafter, (unsigned char **)&prop) == Success) {
	    if (actual_type == XA_WINDOW && actual_format == 32 &&
			nitems == 1 /*&& bytesafter == 0*/) {
		newroot = *prop;
		if (XGetWindowAttributes (dpy, newroot, &wa)) {
		    XReparentWindow (dpy, window, newroot, 0, 0);
		    XMapWindow (dpy, window);
		    ret = True;
		}
	    }
	    XFree ((char *)prop);
	}
    }
    status = XrmGetResource (db, "ctwm.rootWindow", "Ctwm.RootWindow", &str_type, &value);
    if ((status == True) && (value.size != 0)) {
	char rootw [32];
	unsigned int scanned;

	safecopy (rootw, value.addr, sizeof(rootw));
	if (sscanf (rootw, "%x", &scanned) == 1) {
	    newroot = scanned;
	    if (XGetWindowAttributes (dpy, newroot, &wa)) {
		XReparentWindow (dpy, window, newroot, 0, 0);
		XMapWindow (dpy, window);
		ret = True;
	    }
	}
    }
    XrmDestroyDatabase (db);
    XFreeStringList (cliargv);
    return (ret);
}

/*
 * The window whose occupation is being manipulated.
 */
static TwmWindow *occupyWin = (TwmWindow*) 0;

static int CanChangeOccupation(TwmWindow **twm_winp)
{
    TwmWindow *twm_win;

    if (!Scr->workSpaceManagerActive)
	return 0;
    if (occupyWin != NULL)
	return 0;
    twm_win = *twm_winp;
    if (twm_win->iconmgr)
	return 0;
    if (!Scr->TransientHasOccupation) {
	if (twm_win->transient)
	    return 0;
	if (twm_win->group != (Window) 0 && twm_win->group != twm_win->w) {
	    /*
	     * When trying to modify a group member window,
	     * operate on the group leader instead
	     * (and thereby on all group member windows as well).
	     * If we can't find the group leader, pretend it isn't set.
	     */
	    twm_win = GetTwmWindow(twm_win->group);
	    if (!twm_win)
		return 1;
	    *twm_winp = twm_win;
	}
    }
    return 1;
}

void Occupy (TwmWindow *twm_win)
{
    int		 x, y, junkX, junkY;
    unsigned int junkB, junkD;
    unsigned int width, height;
    int	       	 xoffset, yoffset;
    Window     	 junkW, w;
    unsigned int junkK;
    struct OccupyWindow    *occupyWindow;

    if (!CanChangeOccupation(&twm_win))
	return;

    occupyWindow = Scr->workSpaceMgr.occupyWindow;
    occupyWindow->tmpOccupation = twm_win->occupation;
    w = occupyWindow->w;
    XGetGeometry  (dpy, w, &junkW, &junkX, &junkY, &width, &height, &junkB, &junkD);
    XQueryPointer (dpy, Scr->Root, &junkW, &junkW, &junkX, &junkY, &x, &y, &junkK);
    x -= (width  / 2);
    y -= (height / 2);
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    xoffset = width  + 2 * Scr->BorderWidth;
    yoffset = height + 2 * Scr->BorderWidth + Scr->TitleHeight;
    if ((x + xoffset) > Scr->rootw) x = Scr->rootw - xoffset;
    if ((y + yoffset) > Scr->rooth) y = Scr->rooth - yoffset;

    occupyWindow->twm_win->occupation = twm_win->occupation;
    if (occupyWindow->twm_win->vs != Scr->currentvs) {
	XReparentWindow(dpy, occupyWindow->twm_win->frame, Scr->Root, x, y);
	occupyWindow->twm_win->vs = Scr->currentvs;
    } else
	XMoveWindow(dpy, occupyWindow->twm_win->frame, x, y);

    SetMapStateProp (occupyWindow->twm_win, NormalState);
    XMapWindow      (dpy, occupyWindow->w);
    XMapRaised      (dpy, occupyWindow->twm_win->frame);
    occupyWindow->twm_win->mapped = TRUE;
    occupyWin = twm_win;
}

void OccupyHandleButtonEvent (XEvent *event)
{
    WorkSpace	 *ws;
    OccupyWindow *occupyW;
    Window	 buttonW;

    if (! Scr->workSpaceManagerActive) return;
    if (occupyWin == (TwmWindow*) 0) return;

    buttonW = event->xbutton.window;
    if (buttonW == 0) return; /* icon */

    XGrabPointer (dpy, Scr->Root, True,
		  ButtonPressMask | ButtonReleaseMask,
		  GrabModeAsync, GrabModeAsync,
		  Scr->Root, None, CurrentTime);

    occupyW = Scr->workSpaceMgr.occupyWindow;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	if (occupyW->obuttonw [ws->number] == buttonW) break;
    }
    if (ws != NULL) {
	int mask = 1 << ws->number;
	if ((occupyW->tmpOccupation & mask) == 0) {
	    PaintButton (OCCUPYWINDOW, NULL, occupyW->obuttonw [ws->number],
			 ws->label, ws->cp, on);
	} else {
	    PaintButton (OCCUPYWINDOW, NULL, occupyW->obuttonw [ws->number],
			 ws->label, ws->cp, off);
	}
	occupyW->tmpOccupation ^= mask;
    }
    else
    if (buttonW == occupyW->OK) {
	if (occupyW->tmpOccupation == 0) return;
	ChangeOccupation (occupyWin, occupyW->tmpOccupation);
	XUnmapWindow (dpy, occupyW->twm_win->frame);
	occupyW->twm_win->mapped = FALSE;
	occupyW->twm_win->occupation = 0;
	occupyWin = (TwmWindow*) 0;
	XSync (dpy, 0);
    }
    else
    if (buttonW == occupyW->cancel) {
	XUnmapWindow (dpy, occupyW->twm_win->frame);
	occupyW->twm_win->mapped = FALSE;
	occupyW->twm_win->occupation = 0;
	occupyWin = (TwmWindow*) 0;
	XSync (dpy, 0);
    }
    else
    if (buttonW == occupyW->allworkspc) {
	for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	    PaintButton (OCCUPYWINDOW, NULL, occupyW->obuttonw [ws->number],
			 ws->label, ws->cp, on);
	}
	occupyW->tmpOccupation = fullOccupation;
    }
    if (ButtonPressed == -1) XUngrabPointer (dpy, CurrentTime);
}

void OccupyAll (TwmWindow *twm_win)
{
    IconMgr *save;

    if (!CanChangeOccupation(&twm_win))
	return;
    save = Scr->iconmgr;
    Scr->iconmgr = Scr->workSpaceMgr.workSpaceList->iconmgr;
    ChangeOccupation (twm_win, fullOccupation);
    Scr->iconmgr = save;
}

void AddToWorkSpace (char *wname, TwmWindow *twm_win)
{
    WorkSpace *ws;
    int newoccupation;

    if (!CanChangeOccupation(&twm_win))
	return;
    ws = GetWorkspace (wname);
    if (!ws)
	return;

    if (twm_win->occupation & (1 << ws->number))
	return;
    newoccupation = twm_win->occupation | (1 << ws->number);
    ChangeOccupation (twm_win, newoccupation);
}

void RemoveFromWorkSpace (char *wname, TwmWindow *twm_win)
{
    WorkSpace *ws;
    int newoccupation;

    if (!CanChangeOccupation(&twm_win))
	return;
    ws = GetWorkspace (wname);
    if (!ws)
	return;

    newoccupation = twm_win->occupation & ~(1 << ws->number);
    if (!newoccupation) return;
    ChangeOccupation (twm_win, newoccupation);
}

void ToggleOccupation (char *wname, TwmWindow *twm_win)
{
    WorkSpace *ws;
    int newoccupation;

    if (!CanChangeOccupation(&twm_win))
	return;
    ws = GetWorkspace (wname);
    if (!ws) return;

    newoccupation = twm_win->occupation ^ (1 << ws->number);
    if (!newoccupation) return;
    ChangeOccupation (twm_win, newoccupation);
}

void MoveToNextWorkSpace (virtualScreen *vs, TwmWindow *twm_win)
{
    WorkSpace *wlist1, *wlist2;
    int newoccupation;

    if (!CanChangeOccupation(&twm_win))
	return;

    wlist1 = vs->wsw->currentwspc;
    wlist2 = wlist1->next;
    wlist2 = wlist2 ? wlist2 : Scr->workSpaceMgr.workSpaceList;

    newoccupation = (twm_win->occupation ^ (1 << wlist1->number))
				         | (1 << wlist2->number);
    ChangeOccupation (twm_win, newoccupation);
}


void MoveToNextWorkSpaceAndFollow (virtualScreen *vs, TwmWindow *twm_win)
{
    if (!CanChangeOccupation(&twm_win))
	return;

    MoveToNextWorkSpace(vs, twm_win);
    GotoNextWorkSpace(vs);
#if 0
    RaiseWindow(twm_win);	/* XXX really do this? */
#endif
}


void MoveToPrevWorkSpace (virtualScreen *vs, TwmWindow *twm_win)
{
    WorkSpace *wlist1, *wlist2;
    int newoccupation;

    if (!CanChangeOccupation(&twm_win))
	return;

    wlist1 = Scr->workSpaceMgr.workSpaceList;
    wlist2 = vs->wsw->currentwspc;
    if (wlist1 == NULL) return;

    while (wlist1->next != wlist2 && wlist1->next != NULL) {
	wlist1 = wlist1->next;
    }

    newoccupation = (twm_win->occupation ^ (1 << wlist2->number))
					 | (1 << wlist1->number);
    ChangeOccupation (twm_win, newoccupation);
}

void MoveToPrevWorkSpaceAndFollow (virtualScreen *vs, TwmWindow *twm_win)
{
    if (!CanChangeOccupation(&twm_win))
	return;

    MoveToPrevWorkSpace(vs, twm_win);
    GotoPrevWorkSpace(vs);
#if 0
    RaiseWindow(twm_win);		/* XXX really do this? */
#endif
}

static WorkSpace *GetWorkspace (char *wname)
{
    WorkSpace *ws;

    if (!wname) return (NULL);
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	if (strcmp (ws->label, wname) == 0) break;
    }
    if (ws == NULL) {
	for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	    if (strcmp (ws->name, wname) == 0) break;
	}
    }
    return (ws);
}

void AllocateOthersIconManagers (void)
{
    IconMgr   *p = NULL, *ip, *oldp, *oldv;
    WorkSpace *ws;

    if (! Scr->workSpaceManagerActive) return;

    oldp = Scr->iconmgr;
    for (ws = Scr->workSpaceMgr.workSpaceList->next; ws != NULL; ws = ws->next) {
	ws->iconmgr  = (IconMgr *) malloc (sizeof (IconMgr));
	*ws->iconmgr = *oldp;
	oldv = ws->iconmgr;
	oldp->nextv = ws->iconmgr;
	oldv->nextv = NULL;

	for (ip = oldp->next; ip != NULL; ip = ip->next) {
	    p  = (IconMgr *) malloc (sizeof (IconMgr));
	    *p = *ip;
	    ip->nextv  = p;
	    p->next    = NULL;
	    p->prev    = oldv;
	    p->nextv   = NULL;
	    oldv->next = p;
	    oldv = p;
        }
	for (ip = ws->iconmgr; ip != NULL; ip = ip->next) {
	    ip->lasti = p;
        }
	oldp = ws->iconmgr;
    }
    Scr->workSpaceMgr.workSpaceList->iconmgr = Scr->iconmgr;
}

static void Vanish (virtualScreen *vs, TwmWindow *tmp_win)
{
    XWindowAttributes winattrs;
    unsigned long     eventMask;

    if (vs && tmp_win->vs && tmp_win->vs != vs)
	return;
    if (tmp_win->UnmapByMovingFarAway) {
	XMoveWindow (dpy, tmp_win->frame, Scr->rootw + 1, Scr->rooth + 1);
    } else if (tmp_win->mapped) {
	XGetWindowAttributes(dpy, tmp_win->w, &winattrs);
	eventMask = winattrs.your_event_mask;
	XSelectInput (dpy, tmp_win->w, eventMask & ~StructureNotifyMask);
	XUnmapWindow (dpy, tmp_win->w);
	XUnmapWindow (dpy, tmp_win->frame);
	XSelectInput (dpy, tmp_win->w, eventMask);

	if (!tmp_win->DontSetInactive)
	SetMapStateProp (tmp_win, InactiveState);
    } else if (tmp_win->icon_on && tmp_win->icon && tmp_win->icon->w) {
	XUnmapWindow (dpy, tmp_win->icon->w);
	IconDown (tmp_win);
    }

    /*
     * XXX - this may need to be tweaked to find the real window at 0x0.
     * Most people will setup virtualscreens left to right, but some
     * may not.  The purpose of this is in the event of a ctwm death/restart,
     * geometries of windows that were on unmapped workspaces will show
     * up where they belong.
     * XXX - XReparentWindow() messes up the stacking order of windows.
     * It should be avoided as much as possible. This already affects
     * switching away from and back to a workspace. Therefore do this only
     * if there are at least 2 virtual screens AND the new one (firstvs)
     * differs from where the window currently is. (Olaf Seibert).
     */

    if (Scr->vScreenList && Scr->vScreenList->next) {
	int x, y;
	unsigned int junk;
	Window junkW, w = tmp_win->frame;
	virtualScreen *firstvs = NULL;

	for (firstvs = Scr->vScreenList; firstvs; firstvs = firstvs->next)
	    if (firstvs->x == 0 && firstvs->y == 0)
		break;
	if (firstvs && firstvs != vs) {
	    XGetGeometry (dpy, w, &junkW, &x, &y, &junk, &junk, &junk, &junk);
	    XReparentWindow(dpy, w, firstvs->window, x, y);
	    tmp_win->vs = firstvs;
	}
    }

    tmp_win->old_parent_vs = tmp_win->vs;
    tmp_win->vs = NULL;
}

static void DisplayWin (virtualScreen *vs, TwmWindow *tmp_win)
{
    XWindowAttributes	winattrs;
    unsigned long	eventMask;

    /*
     * A window cannot be shown in multiple virtual screens, even if
     * it occupies both corresponding workspaces.
     */
    if (vs && tmp_win->vs)
	return;
    tmp_win->vs = vs;

    if (!tmp_win->mapped) {
	if (tmp_win->isicon) {
	    if (tmp_win->icon_on) {
		if (tmp_win->icon && tmp_win->icon->w) {
		    if (vs != tmp_win->old_parent_vs) {
			int x, y;
			unsigned int junk;
			Window junkW, w = tmp_win->icon->w;
			XGetGeometry (dpy, w, &junkW, &x, &y, &junk, &junk, &junk, &junk);
			XReparentWindow (dpy, w, vs->window, x, y);
		    }

		    IconUp (tmp_win);
		    XMapWindow (dpy, tmp_win->icon->w);
		    return;
		}
	    }
	}
	return;
    }
    if (tmp_win->UnmapByMovingFarAway) {
        if (vs)		/* XXX I don't believe the handling of UnmapByMovingFarAway is quite correct */
	    XReparentWindow (dpy, tmp_win->frame, vs->window,
		tmp_win->frame_x, tmp_win->frame_y);
	else
	    XMoveWindow (dpy, tmp_win->frame, tmp_win->frame_x, tmp_win->frame_y);
    } else {
	if (!tmp_win->squeezed) {
	    XGetWindowAttributes(dpy, tmp_win->w, &winattrs);
	    eventMask = winattrs.your_event_mask;
	    XSelectInput (dpy, tmp_win->w, eventMask & ~StructureNotifyMask);
	    XMapWindow   (dpy, tmp_win->w);
	    XSelectInput (dpy, tmp_win->w, eventMask);
	}
	if (vs != tmp_win->old_parent_vs) {
	    XReparentWindow (dpy, tmp_win->frame, vs->window, tmp_win->frame_x, tmp_win->frame_y);
	}
	XMapWindow (dpy, tmp_win->frame);
	SetMapStateProp (tmp_win, NormalState);
    }
}

void ChangeOccupation (TwmWindow *tmp_win, int newoccupation)
{
    TwmWindow *t;
    virtualScreen *vs;
    WorkSpace *ws;
    int	      oldoccupation;
    char      namelist [512];
    int	      len;
    int	      final_x, final_y;
    XWindowAttributes winattrs;
    unsigned long     eventMask;
    long      gwkspc = 0; /* for gnome - the workspace of this window */
    int changedoccupation;
#ifdef GNOME
    unsigned char *prop;
    unsigned long bytesafter, numitems;
    Atom actual_type;
    int actual_format; 	
#endif /* GNOME */

    if ((newoccupation == 0) || /* in case the property has been broken by another client */
	(newoccupation == tmp_win->occupation)) {
	len = GetPropertyFromMask (tmp_win->occupation, namelist, &gwkspc);
	XGetWindowAttributes(dpy, tmp_win->w, &winattrs);
	eventMask = winattrs.your_event_mask;
	XSelectInput(dpy, tmp_win->w, eventMask & ~PropertyChangeMask);

	XChangeProperty (dpy, tmp_win->w, _XA_WM_OCCUPATION, XA_STRING, 8, 
			 PropModeReplace, (unsigned char *) namelist, len);
#ifdef GNOME
 	XChangeProperty (dpy, tmp_win->w, _XA_WIN_WORKSPACE, XA_CARDINAL, 32,
			 PropModeReplace, (unsigned char *)(&gwkspc), 1);
 	if (XGetWindowProperty(dpy, tmp_win->w, _XA_WIN_STATE, 0L, 32, False,
			      XA_CARDINAL, &actual_type, &actual_format, &numitems,
			      &bytesafter, &prop) 
	   != Success || numitems == 0) {
	    gwkspc = 0;
	} else {
	    gwkspc = (int)*prop;
	    XFree((char *)prop);
	}
 	if (tmp_win->occupation == fullOccupation)
	  gwkspc |= WIN_STATE_STICKY;
 	else
	  gwkspc &= ~WIN_STATE_STICKY;
 	XChangeProperty (dpy, tmp_win->w, _XA_WIN_STATE, XA_CARDINAL, 32, 
			 PropModeReplace, (unsigned char *)&gwkspc, 1); 
#endif /* GNOME */
	XSelectInput (dpy, tmp_win->w, eventMask);
	return;
    }
    oldoccupation = tmp_win->occupation;
    tmp_win->occupation = newoccupation & ~oldoccupation;
    AddIconManager (tmp_win);
    tmp_win->occupation = newoccupation;
    RemoveIconManager (tmp_win);

    if (tmp_win->vs && !OCCUPY (tmp_win, tmp_win->vs->wsw->currentwspc)) {
	Vanish (tmp_win->vs, tmp_win);
    }
    /*
     * If a window occupies multiple workspaces, try to find another workspace
     * which is currently in another virtual screen, so that the window
     * can be shown there now.
     */
    if (!tmp_win->vs) {
      for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
	if (OCCUPY (tmp_win, vs->wsw->currentwspc)) {
	  DisplayWin (vs, tmp_win);
	  break;
	}
      }
    }
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	int mask = 1 << ws->number;
	if (oldoccupation & mask) {
	    if (!(newoccupation & mask)) {
		RemoveWindowFromRegion (tmp_win);
		if (PlaceWindowInRegion (tmp_win, &final_x, &final_y))
		    XMoveWindow (dpy, tmp_win->frame, final_x, final_y);
	    }
	    break;
	}
    }
    len = GetPropertyFromMask (newoccupation, namelist, &gwkspc);
    XGetWindowAttributes(dpy, tmp_win->w, &winattrs);
    eventMask = winattrs.your_event_mask;
    XSelectInput(dpy, tmp_win->w, eventMask & ~PropertyChangeMask);

    XChangeProperty (dpy, tmp_win->w, _XA_WM_OCCUPATION, XA_STRING, 8, 
		     PropModeReplace, (unsigned char *) namelist, len);

#ifdef GNOME
    /* Tell GNOME where this window lives */
    XChangeProperty (dpy, tmp_win->w, _XA_WIN_WORKSPACE, XA_CARDINAL, 32,
		     PropModeReplace, (unsigned char *)(&gwkspc), 1); 
    if (XGetWindowProperty (dpy, tmp_win->w, _XA_WIN_STATE, 0L, 32, False,
			    XA_CARDINAL, &actual_type, &actual_format, &numitems,
			    &bytesafter, &prop) 
	!= Success || numitems == 0) {
	gwkspc = 0;
    } else {
	gwkspc = (int)*prop;
	XFree ((char *)prop);
    }
    if (tmp_win->occupation == fullOccupation)
      gwkspc |= WIN_STATE_STICKY;
    else
      gwkspc &= ~WIN_STATE_STICKY;
    XChangeProperty (dpy, tmp_win->w, _XA_WIN_STATE, XA_CARDINAL, 32, 
		     PropModeReplace, (unsigned char *)&gwkspc, 1); 
#endif /* GNOME */
    XSelectInput(dpy, tmp_win->w, eventMask);

    if (!WMapWindowMayBeAdded(tmp_win)) {
	newoccupation = 0;
    }
    if (Scr->workSpaceMgr.noshowoccupyall) {
	/* We can safely change new/oldoccupation here, it's only used
	 * for WMapAddToList()/WMapRemoveFromList() from here on.
	 */
	/* if (newoccupation == fullOccupation)
	    newoccupation = 0; */
	if (oldoccupation == fullOccupation)
	    oldoccupation = 0;
    }
    changedoccupation = oldoccupation ^ newoccupation;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	int mask = 1 << ws->number;
	if (changedoccupation & mask) {
	    if (newoccupation & mask) {
		WMapAddToList (tmp_win, ws);
	    } else {
		WMapRemoveFromList (tmp_win, ws);
                if (Scr->SaveWorkspaceFocus && ws->save_focus == tmp_win) {
                    ws->save_focus = NULL;
                }
	    }
	}
    }

    if (! Scr->TransientHasOccupation) {
	for (t = Scr->FirstWindow; t != NULL; t = t->next) {
	    if (t != tmp_win &&
		((t->transient && t->transientfor == tmp_win->w) ||
		 t->group == tmp_win->w)) {
		ChangeOccupation (t, tmp_win->occupation);
	    }
	}
    }
}

void WmgrRedoOccupation (TwmWindow *win)
{
    WorkSpace *ws;
    int       newoccupation;

    if (LookInList (Scr->OccupyAll, win->full_name, &win->class)) {
	newoccupation = fullOccupation;
    }
    else {
	newoccupation = 0;
	for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	    if (LookInList (ws->clientlist, win->full_name, &win->class)) {
		newoccupation |= 1 << ws->number;
	    }
	}
    }
    if (newoccupation != 0) ChangeOccupation (win, newoccupation);
}

void WMgrRemoveFromCurrentWorkSpace (virtualScreen *vs, TwmWindow *win)
{
    WorkSpace *ws;
    int	      newoccupation;

    ws = vs->wsw->currentwspc;
    if (! OCCUPY (win, ws)) return;

    newoccupation = win->occupation & ~(1 << ws->number);
    if (newoccupation == 0) return;

    ChangeOccupation (win, newoccupation);
}

void WMgrAddToCurrentWorkSpaceAndWarp (virtualScreen *vs, char *winname)
{
    TwmWindow *tw;
    int       newoccupation;

    for (tw = Scr->FirstWindow; tw != NULL; tw = tw->next) {
	if (match (winname, tw->full_name)) break;
    }
    if (!tw) {
	for (tw = Scr->FirstWindow; tw != NULL; tw = tw->next) {
	    if (match (winname, tw->class.res_name)) break;
	}
	if (!tw) {
	    for (tw = Scr->FirstWindow; tw != NULL; tw = tw->next) {
		if (match (winname, tw->class.res_class)) break;
	    }
	}
    }
    if (!tw) {
	XBell (dpy, 0);
	return;
    }
    if ((! Scr->WarpUnmapped) && (! tw->mapped)) {
	XBell (dpy, 0);
	return;
    }
    if (! OCCUPY (tw, vs->wsw->currentwspc)) {
	newoccupation = tw->occupation | (1 << vs->wsw->currentwspc->number);
	ChangeOccupation (tw, newoccupation);
    }

    if (! tw->mapped) DeIconify (tw);
    WarpToWindow (tw, Scr->RaiseOnWarp);
}

static void CreateWorkSpaceManagerWindow (virtualScreen *vs)
{
    int		  mask;
    int		  lines, vspace, hspace, count, columns;
    unsigned int  width, height, bwidth, bheight;
    char	  *name, *icon_name, *geometry;
    int		  i, j;
    ColorPair	  cp;
    MyFont	  font;
    WorkSpace     *ws;
    int		  x, y, strWid, wid;
    unsigned long border;
    TwmWindow	  *tmp_win;
    XSetWindowAttributes	attr;
    XWindowAttributes		wattr;
    unsigned long		attrmask;
    XSizeHints	  sizehints;
    XWMHints	  wmhints;
    int		  gravity;
	XRectangle inc_rect;
	XRectangle logical_rect;

    name      = Scr->workSpaceMgr.name;
    icon_name = Scr->workSpaceMgr.icon_name;
    geometry  = Scr->workSpaceMgr.geometry;
    columns   = Scr->workSpaceMgr.columns;
    vspace    = Scr->workSpaceMgr.vspace;
    hspace    = Scr->workSpaceMgr.hspace;
    font      = Scr->workSpaceMgr.buttonFont;
    cp        = Scr->workSpaceMgr.cp;
    border    = Scr->workSpaceMgr.defBorderColor;

    count = 0;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) count++;
    Scr->workSpaceMgr.count = count;

    if (columns == 0) {
	lines   = 2;
	columns = ((count - 1) / lines) + 1;
    }
    else {
	lines   = ((count - 1) / columns) + 1;
    }
    Scr->workSpaceMgr.lines   = lines;
    Scr->workSpaceMgr.columns = columns;

    strWid = 0;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	XmbTextExtents(font.font_set, ws->label, strlen (ws->label),
		       &inc_rect, &logical_rect);
	wid = logical_rect.width;
	if (wid > strWid) strWid = wid;
    }
    if (geometry != NULL) {
	mask = XParseGeometry (geometry, &x, &y, &width, &height);
	bwidth  = (mask & WidthValue)  ? ((width - (columns * hspace)) / columns) : strWid + 10;
	bheight = (mask & HeightValue) ? ((height - (lines  * vspace)) / lines) : 22;
	width   = columns * (bwidth  + hspace);
	height  = lines   * (bheight + vspace);

	if (! (mask & YValue)) {
            y = 0;
	    mask |= YNegative;
	}
	if (mask & XValue) {
	    if (mask & XNegative) {
		x += vs->w - width;
		gravity = (mask & YNegative) ? SouthEastGravity : NorthEastGravity;
	    }
	    else {
		gravity = (mask & YNegative) ? SouthWestGravity : NorthWestGravity;
	    }
	}
	else {
	    x = (vs->w - width) / 2;
	    gravity = (mask & YValue) ? ((mask & YNegative) ?
			SouthGravity : NorthGravity) : SouthGravity;
	}
	if (mask & YNegative) y += vs->h - height;
    }
    else {
	bwidth  = strWid + 2 * Scr->WMgrButtonShadowDepth + 6;
	bheight = 22;
	width   = columns * (bwidth  + hspace);
	height  = lines   * (bheight + vspace);
	x       = (vs->w - width) / 2;
	y       = vs->h - height;
	gravity = NorthWestGravity;
    }

#define Dummy	1

    vs->wsw->width   = Dummy;
    vs->wsw->height  = Dummy;
    vs->wsw->bswl    = (ButtonSubwindow**)
      malloc (Scr->workSpaceMgr.count * sizeof (ButtonSubwindow*));
    vs->wsw->mswl    = (MapSubwindow**)
      malloc (Scr->workSpaceMgr.count * sizeof (MapSubwindow*));

    vs->wsw->w = XCreateSimpleWindow (dpy, Scr->Root, x, y, width, height, 0,
				      Scr->Black, cp.back);
    i = 0; j = 0;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
        Window mapsw, butsw;
        MapSubwindow *msw;
        ButtonSubwindow *bsw;

	vs->wsw->bswl [ws->number] = bsw = 
	    (ButtonSubwindow*) malloc (sizeof (ButtonSubwindow));
        vs->wsw->mswl [ws->number] = msw =
	    (MapSubwindow*)    malloc (sizeof (MapSubwindow));

        butsw = bsw->w =
	  XCreateSimpleWindow (dpy, vs->wsw->w,
			       Dummy /* x */, Dummy /* y */,
			       Dummy /* width */, Dummy /* height */,
			       0, Scr->Black, ws->cp.back);

	mapsw = msw->w =
	    XCreateSimpleWindow (dpy, vs->wsw->w,
				 Dummy /* x */, Dummy /* y */,
				 Dummy /* width */, Dummy /* height */,
				 1, border, ws->cp.back);

	if (vs->wsw->state == BUTTONSSTATE)
	    XMapWindow (dpy, butsw);
	else
	    XMapWindow (dpy, mapsw);

	vs->wsw->mswl [ws->number]->wl = NULL;
	if (useBackgroundInfo) {
	    if (ws->image == None || Scr->NoImagesInWorkSpaceManager)
		XSetWindowBackground       (dpy, mapsw, ws->backcp.back);
	    else
		XSetWindowBackgroundPixmap (dpy, mapsw, ws->image->pixmap);
	}
	else {
	    if (Scr->workSpaceMgr.defImage == None || Scr->NoImagesInWorkSpaceManager)
		XSetWindowBackground       (dpy, mapsw, Scr->workSpaceMgr.defColors.back);
	    else
		XSetWindowBackgroundPixmap (dpy, mapsw, Scr->workSpaceMgr.defImage->pixmap);
	}
	XClearWindow (dpy, butsw);
	i++;
	if (i == columns) {i = 0; j++;};
    }

    sizehints.flags       = USPosition | PBaseSize | PMinSize | PResizeInc | PWinGravity;
    sizehints.x           = x;
    sizehints.y           = y;
    sizehints.base_width  = columns * hspace;
    sizehints.base_height = lines   * vspace;
    sizehints.width_inc   = columns;
    sizehints.height_inc  = lines;
    sizehints.min_width   = columns  * (hspace + 2);
    sizehints.min_height  = lines    * (vspace + 2);
    sizehints.win_gravity = gravity;

    XSetStandardProperties (dpy, vs->wsw->w,
			name, icon_name, None, NULL, 0, NULL);
    XSetWMSizeHints (dpy, vs->wsw->w, &sizehints, XA_WM_NORMAL_HINTS);

    wmhints.flags	  = InputHint | StateHint;
    wmhints.input         = True;
    wmhints.initial_state = NormalState;
    XSetWMHints (dpy, vs->wsw->w, &wmhints);
    XSaveContext (dpy, vs->wsw->w, VirtScreenContext, (XPointer) vs);
    tmp_win = AddWindow (vs->wsw->w, 3, Scr->iconmgr);
    if (! tmp_win) {
	fprintf (stderr, "cannot create workspace manager window, exiting...\n");
	exit (1);
    }
    tmp_win->occupation = fullOccupation;
    tmp_win->vs = vs;
    tmp_win->attr.width = width;
    tmp_win->attr.height = height;
    ResizeWorkSpaceManager(vs, tmp_win);

    attrmask = 0;
    attr.cursor = Scr->ButtonCursor;
    attrmask |= CWCursor;
    attr.win_gravity = gravity;
    attrmask |= CWWinGravity;
    XChangeWindowAttributes (dpy, vs->wsw->w, attrmask, &attr);

    XGetWindowAttributes (dpy, vs->wsw->w, &wattr);
    attrmask = wattr.your_event_mask | KeyPressMask | KeyReleaseMask | ExposureMask;
    XSelectInput (dpy, vs->wsw->w, attrmask);

    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
        Window buttonw = vs->wsw->bswl [ws->number]->w;
        Window mapsubw = vs->wsw->mswl [ws->number]->w;
	XSelectInput (dpy, buttonw, ButtonPressMask | ButtonReleaseMask | ExposureMask);
	XSaveContext (dpy, buttonw, TwmContext,    (XPointer) tmp_win);
	XSaveContext (dpy, buttonw, ScreenContext, (XPointer) Scr);

	XSelectInput (dpy, mapsubw, ButtonPressMask | ButtonReleaseMask);
	XSaveContext (dpy, mapsubw, TwmContext,    (XPointer) tmp_win);
	XSaveContext (dpy, mapsubw, ScreenContext, (XPointer) Scr);
    }
    SetMapStateProp (tmp_win, WithdrawnState);
    vs->wsw->twm_win = tmp_win;

    ws = Scr->workSpaceMgr.workSpaceList;
    if (useBackgroundInfo && ! Scr->DontPaintRootWindow) {
	if (ws->image == None)
	    XSetWindowBackground (dpy, Scr->Root, ws->backcp.back);
	else
	    XSetWindowBackgroundPixmap (dpy, Scr->Root, ws->image->pixmap);
	XClearWindow (dpy, Scr->Root);
    }
    PaintWorkSpaceManager (vs);
}

void WMgrHandleExposeEvent (virtualScreen *vs, XEvent *event)
{
    WorkSpace *ws;
    Window buttonw;

    if (vs->wsw->state == BUTTONSSTATE) {
	for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	    buttonw = vs->wsw->bswl [ws->number]->w;
	    if (event->xexpose.window == buttonw) break;
	}
	if (ws == NULL) {
	  PaintWorkSpaceManagerBorder (vs);
	}
	else
	if (ws == vs->wsw->currentwspc)
	    PaintButton (WSPCWINDOW, vs, buttonw, ws->label, ws->cp, on);
	else
	    PaintButton (WSPCWINDOW, vs, buttonw, ws->label, ws->cp, off);
    }
    else {
	WinList	  wl;

        if (XFindContext (dpy, event->xexpose.window, MapWListContext,
		(XPointer *) &wl) == XCNOENT) return;
	if (wl && wl->twm_win && wl->twm_win->mapped) {
	    WMapRedrawName (vs, wl);
	}
    }
}

void PaintWorkSpaceManager (virtualScreen *vs)
{
    WorkSpace *ws;

    PaintWorkSpaceManagerBorder (vs);
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
        Window buttonw = vs->wsw->bswl [ws->number]->w;
	if (ws == vs->wsw->currentwspc)
	    PaintButton (WSPCWINDOW, vs, buttonw, ws->label, ws->cp, on);
	else
	    PaintButton (WSPCWINDOW, vs, buttonw, ws->label, ws->cp, off);
    }
}

static void PaintWorkSpaceManagerBorder (virtualScreen *vs)
{
    int width, height;

    width  = vs->wsw->width;
    height = vs->wsw->height;
    Draw3DBorder (vs->wsw->w, 0, 0, width, height, 2, Scr->workSpaceMgr.cp, off, True, False);
}

ColorPair occupyButtoncp;

char *ok_string		= "OK",
     *cancel_string	= "Cancel",
     *everywhere_string	= "All";

/*
 * Create the Occupy window. Do not do the layout of the parts, only
 * calculate the inial total size. For the layout, call ResizeOccupyWindow()
 * at the end.
 * There is only one Occupy window (per Screen), it is reparented to each
 * virtual screen as needed.
 */
static void CreateOccupyWindow (void) {
    int		  width, height, lines, columns;
    int		  bwidth, bheight, owidth, oheight, hspace, vspace;
    int		  min_bwidth, min_width;
    int		  i, j;
    Window	  w, OK, cancel, allworkspc;
    char	  *name, *icon_name;
    ColorPair	  cp;
    TwmWindow	  *tmp_win;
    WorkSpace     *ws;
    XSizeHints	  sizehints;
    XWMHints      wmhints;
    MyFont	  font;
    XSetWindowAttributes attr;
    XWindowAttributes	 wattr;
    unsigned long attrmask;
    OccupyWindow  *occwin;
    virtualScreen *vs;
    XRectangle inc_rect;
    XRectangle logical_rect;

    occwin = Scr->workSpaceMgr.occupyWindow;
    occwin->font     = Scr->IconManagerFont;
    occwin->cp       = Scr->IconManagerC;
#ifdef COLOR_BLIND_USER
    occwin->cp.shadc = Scr->White;
    occwin->cp.shadd = Scr->Black;
#else
    if (!Scr->BeNiceToColormap) GetShadeColors (&occwin->cp);
#endif
    vs        = Scr->vScreenList;
    name      = occwin->name;
    icon_name = occwin->icon_name;
    lines     = Scr->workSpaceMgr.lines;
    columns   = Scr->workSpaceMgr.columns;
    bwidth    = vs->wsw->bwidth;
    bheight   = vs->wsw->bheight;
    oheight   = bheight;
    vspace    = occwin->vspace;
    hspace    = occwin->hspace;
    cp        = occwin->cp;
    height    = ((bheight + vspace) * lines) + oheight + (2 * vspace);
    font      = occwin->font;
    XmbTextExtents(font.font_set, ok_string, strlen(ok_string),
		       &inc_rect, &logical_rect);
    min_bwidth = logical_rect.width;
    XmbTextExtents(font.font_set, cancel_string, strlen (cancel_string),
		   &inc_rect, &logical_rect);
    i = logical_rect.width;
    if (i > min_bwidth) min_bwidth = i;
    XmbTextExtents(font.font_set,everywhere_string, strlen (everywhere_string),
		   &inc_rect, &logical_rect);
    i = logical_rect.width;
    if (i > min_bwidth) min_bwidth = i;
    min_bwidth = (min_bwidth + hspace); /* normal width calculation */
    width = columns * (bwidth  + hspace); 
    min_width = 3 * (min_bwidth + hspace); /* width by text width */

    if (columns < 3) {
	owidth = min_bwidth + 2 * Scr->WMgrButtonShadowDepth + 2;
	if (width < min_width) width = min_width;
	bwidth = (width - columns * hspace) / columns;
    }
    else {
	owidth = min_bwidth + 2 * Scr->WMgrButtonShadowDepth + 2;
	width  = columns * (bwidth  + hspace);
    }
    occwin->lines   = lines;
    occwin->columns = columns;
    occwin->owidth  = owidth;

    w = occwin->w = XCreateSimpleWindow (dpy, Scr->Root, 0, 0, width, height,
					 1, Scr->Black, cp.back);
    occwin->obuttonw = (Window*) malloc (Scr->workSpaceMgr.count * sizeof (Window));
    i = 0; j = 0;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	Window bw =
	occwin->obuttonw [j * columns + i] =
	  XCreateSimpleWindow(dpy, w,
			      Dummy /* x */,
			      Dummy /* y */,
			      Dummy /* width */,
			      Dummy /* height */,
			      0, Scr->Black, ws->cp.back);
	XMapWindow (dpy, bw);
	i++;
	if (i == columns) {i = 0; j++;}
    }
    GetColor (Scr->Monochrome, &(occupyButtoncp.back), "gray50");
    occupyButtoncp.fore = Scr->White;
    if (!Scr->BeNiceToColormap) GetShadeColors (&occupyButtoncp);

    OK = XCreateSimpleWindow (dpy, w, Dummy, Dummy, Dummy, Dummy, 0,
			Scr->Black, occupyButtoncp.back);
    XMapWindow (dpy, OK);
    cancel = XCreateSimpleWindow (dpy, w, Dummy, Dummy, Dummy, Dummy, 0,
			Scr->Black, occupyButtoncp.back);
    XMapWindow (dpy, cancel);
    allworkspc = XCreateSimpleWindow (dpy, w, Dummy, Dummy, Dummy, Dummy, 0,
			Scr->Black, occupyButtoncp.back);
    XMapWindow (dpy, allworkspc);

    occwin->OK         = OK;
    occwin->cancel     = cancel;
    occwin->allworkspc = allworkspc;

    sizehints.flags       = PBaseSize | PMinSize | PResizeInc;
    sizehints.base_width  = columns;
    sizehints.base_height = lines;
    sizehints.width_inc   = columns;
    sizehints.height_inc  = lines;
    sizehints.min_width   = 2 * columns;
    sizehints.min_height  = 2 * lines;
    XSetStandardProperties (dpy, w, name, icon_name, None, NULL, 0, &sizehints);

    wmhints.flags	  = InputHint | StateHint;
    wmhints.input         = True;
    wmhints.initial_state = NormalState;
    XSetWMHints (dpy, w, &wmhints);
    tmp_win = AddWindow (w, FALSE, Scr->iconmgr);
    if (! tmp_win) {
	fprintf (stderr, "cannot create occupy window, exiting...\n");
	exit (1);
    }
    tmp_win->vs = None;
    tmp_win->occupation = 0;

    attrmask = 0;
    attr.cursor = Scr->ButtonCursor;
    attrmask |= CWCursor;
    XChangeWindowAttributes (dpy, w, attrmask, &attr);

    XGetWindowAttributes (dpy, w, &wattr);
    attrmask = wattr.your_event_mask | KeyPressMask | KeyReleaseMask | ExposureMask;
    XSelectInput (dpy, w, attrmask);

    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
        Window bw = occwin->obuttonw [ws->number];
	XSelectInput (dpy, bw, ButtonPressMask | ButtonReleaseMask | ExposureMask);
	XSaveContext (dpy, bw, TwmContext,    (XPointer) tmp_win);
	XSaveContext (dpy, bw, ScreenContext, (XPointer) Scr);
    }
    XSelectInput (dpy, occwin->OK, ButtonPressMask | ButtonReleaseMask | ExposureMask);
    XSaveContext (dpy, occwin->OK, TwmContext,    (XPointer) tmp_win);
    XSaveContext (dpy, occwin->OK, ScreenContext, (XPointer) Scr);
    XSelectInput (dpy, occwin->cancel, ButtonPressMask | ButtonReleaseMask | ExposureMask);
    XSaveContext (dpy, occwin->cancel, TwmContext,    (XPointer) tmp_win);
    XSaveContext (dpy, occwin->cancel, ScreenContext, (XPointer) Scr);
    XSelectInput (dpy, occwin->allworkspc, ButtonPressMask | ButtonReleaseMask | ExposureMask);
    XSaveContext (dpy, occwin->allworkspc, TwmContext,    (XPointer) tmp_win);
    XSaveContext (dpy, occwin->allworkspc, ScreenContext, (XPointer) Scr);

    SetMapStateProp (tmp_win, WithdrawnState);
    occwin->twm_win = tmp_win;
    Scr->workSpaceMgr.occupyWindow = occwin;

    tmp_win->attr.width = width;
    tmp_win->attr.height = height;
    ResizeOccupyWindow(tmp_win);	/* place all parts in the right place */
}

void PaintOccupyWindow (void)
{
    WorkSpace    *ws;
    OccupyWindow *occwin;
    int 	 width, height;

    occwin = Scr->workSpaceMgr.occupyWindow;
    width  = occwin->width;
    height = occwin->height;

    Draw3DBorder (occwin->w, 0, 0, width, height, 2, occwin->cp, off, True, False);

    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
        Window bw = occwin->obuttonw [ws->number];
	if (occwin->tmpOccupation & (1 << ws->number))
	    PaintButton (OCCUPYWINDOW, NULL, bw, ws->label, ws->cp, on);
	else
	    PaintButton (OCCUPYWINDOW, NULL, bw, ws->label, ws->cp, off);
    }
    PaintButton (OCCUPYBUTTON, NULL, occwin->OK,         ok_string,         occupyButtoncp, off);
    PaintButton (OCCUPYBUTTON, NULL, occwin->cancel,     cancel_string,     occupyButtoncp, off);
    PaintButton (OCCUPYBUTTON, NULL, occwin->allworkspc, everywhere_string, occupyButtoncp, off);
}

static void PaintButton (int which,
			 virtualScreen *vs, Window w,
			 char *label,
			 ColorPair cp, int state)
{
    OccupyWindow *occwin;
    int        bwidth, bheight;
    MyFont     font;
    int        strWid, strHei, hspace, vspace;
    XRectangle inc_rect;
    XRectangle logical_rect;

    occwin = Scr->workSpaceMgr.occupyWindow;
    if (which == WSPCWINDOW) {
	bwidth  = vs->wsw->bwidth;
	bheight = vs->wsw->bheight;
	font    = Scr->workSpaceMgr.buttonFont;
    }
    else
    if (which == OCCUPYWINDOW) {
	bwidth  = occwin->bwidth;
	bheight = occwin->bheight;
	font    = occwin->font;
    }
    else
    if (which == OCCUPYBUTTON) {
	bwidth  = occwin->owidth;
	bheight = occwin->bheight;
	font    = occwin->font;
    }
    else return;

    XmbTextExtents(font.font_set, label, strlen (label), &inc_rect, &logical_rect);
    strHei = logical_rect.height;
    vspace = ((bheight + strHei - font.descent) / 2);
    strWid = logical_rect.width;
    hspace = (bwidth - strWid) / 2;
    if (hspace < (Scr->WMgrButtonShadowDepth + 1)) hspace = Scr->WMgrButtonShadowDepth + 1;
    XClearWindow (dpy, w);

    if (Scr->Monochrome == COLOR) {
	Draw3DBorder (w, 0, 0, bwidth, bheight, Scr->WMgrButtonShadowDepth,
			cp, state, True, False);

	switch (Scr->workSpaceMgr.buttonStyle) {
	    case STYLE_NORMAL :
		break;

	    case STYLE_STYLE1 :
		Draw3DBorder (w,
			Scr->WMgrButtonShadowDepth - 1,
			Scr->WMgrButtonShadowDepth - 1,
			bwidth  - 2 * Scr->WMgrButtonShadowDepth + 2,
			bheight - 2 * Scr->WMgrButtonShadowDepth + 2,
			1, cp, (state == on) ? off : on, True, False);
		break;

	    case STYLE_STYLE2 :
		Draw3DBorder (w,
			Scr->WMgrButtonShadowDepth / 2,
			Scr->WMgrButtonShadowDepth / 2,
			bwidth  - Scr->WMgrButtonShadowDepth,
			bheight - Scr->WMgrButtonShadowDepth,
			1, cp, (state == on) ? off : on, True, False);
		break;

	    case STYLE_STYLE3 :
		Draw3DBorder (w,
			1,
			1,
			bwidth  - 2,
			bheight - 2,
			1, cp, (state == on) ? off : on, True, False);
		break;
	}
	FB (cp.fore, cp.back);
	XmbDrawString (dpy, w, font.font_set, Scr->NormalGC, hspace, vspace,
		       label, strlen (label));
    }
    else {
	Draw3DBorder (w, 0, 0, bwidth, bheight, Scr->WMgrButtonShadowDepth,
		cp, state, True, False);
	if (state == on) {
	    FB (cp.fore, cp.back);
	    XmbDrawImageString (dpy, w, font.font_set, Scr->NormalGC, hspace, vspace,
				label, strlen (label));
	}
	else {
	    FB (cp.back, cp.fore);
	    XmbDrawImageString (dpy, w, font.font_set, Scr->NormalGC, hspace, vspace,
				label, strlen (label));
	}
    }
}

static unsigned int GetMaskFromResource (TwmWindow *win, char *res)
{
    char      *name;
    char      wrkSpcName [64];
    WorkSpace *ws;
    int       mask, num, mode;

    mode = 0;
    if (*res == '+') {
	mode = 1;
	res++;
    }
    else
    if (*res == '-') {
	mode = 2;
	res++;
    }
    mask = 0;
    while (*res != '\0') {
	while (*res == ' ') res++;
	if (*res == '\0') break;
	name = wrkSpcName;
	while ((*res != '\0') && (*res != ' ')) {
	    if (*res == '\\') res++;
	    *name = *res;
	    name++; res++;
	}
	*name = '\0';
	if (strcmp (wrkSpcName, "all") == 0) {
	    mask = fullOccupation;
	    break;
	}
	if (strcmp (wrkSpcName, "current") == 0) {
	    virtualScreen *vs = Scr->currentvs;
	    if (vs) mask |= (1 << vs->wsw->currentwspc->number);
	    continue;
	}
	num  = 0;
	for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	    if (strcmp (wrkSpcName, ws->label) == 0) break;
	    num++;
	}
	if (ws != NULL) mask |= (1 << num);
	else {
	    twmrc_error_prefix ();
	    fprintf (stderr, "unknown workspace : %s\n", wrkSpcName);
	}
    }
    switch (mode) {
	case 0 :
	    return (mask);
	case 1 :
	    return (mask | win->occupation);
	case 2 :
	    return (win->occupation & ~mask);
    }
    return (0);			/* Never supposed to reach here, but just
				   in case... */
}

unsigned int GetMaskFromProperty (unsigned char *prop, unsigned long len)
{
    char         wrkSpcName [256];
    WorkSpace    *ws;
    unsigned int mask;
    int          num, l;

    mask = 0;
    l = 0;
    while (l < len) {
	strcpy (wrkSpcName, (char *)prop);
	l    += strlen ((char *)prop) + 1;
	prop += strlen ((char *)prop) + 1;
	if (strcmp (wrkSpcName, "all") == 0) {
	    mask = fullOccupation;
	    break;
	}
	num = 0;
	for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	    if (strcmp (wrkSpcName, ws->label) == 0) break;
	    num++;
	}
	if (ws == NULL) {
	    fprintf (stderr, "unknown workspace : %s\n", wrkSpcName);
	}
	else {
	    mask |= (1 << num);
	}
    }
    return (mask);
}

static int GetPropertyFromMask (unsigned int mask, char *prop, long *gwkspc)
{
    WorkSpace *ws;
    int       len;
    char      *p;

    if (mask == fullOccupation) {
	strcpy (prop, "all");
	return (3);
    }
    len = 0;
    p = prop;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	if (mask & (1 << ws->number)) {
	    strcpy (p, ws->label);
	    p   += strlen (ws->label) + 1;
	    len += strlen (ws->label) + 1;
	    *gwkspc = ws->number;
	}
    }
    return (len);
}

void AddToClientsList (char *workspace, char *client)
{
    WorkSpace *ws;

    if (strcmp (workspace, "all") == 0) {
	for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	    AddToList (&ws->clientlist, client, "");
	}
	return;
    }

    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	if (strcmp (ws->label, workspace) == 0) break;
    }
    if (ws == NULL) return;
    AddToList (&ws->clientlist, client, "");
}

void WMapToggleState (virtualScreen *vs)
{
    if (vs->wsw->state == BUTTONSSTATE) {
	WMapSetMapState (vs);
    } else {
	WMapSetButtonsState (vs);
    }
}

void WMapSetMapState (virtualScreen *vs)
{
    WorkSpace     *ws;

    if (vs->wsw->state == MAPSTATE) return;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	XUnmapWindow (dpy, vs->wsw->bswl [ws->number]->w);
	XMapWindow   (dpy, vs->wsw->mswl [ws->number]->w);
    }
    vs->wsw->state = MAPSTATE;
    MaybeAnimate = True;
}

void WMapSetButtonsState (virtualScreen *vs)
{
    WorkSpace     *ws;

    if (vs->wsw->state == BUTTONSSTATE) return;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	XUnmapWindow (dpy, vs->wsw->mswl [ws->number]->w);
	XMapWindow   (dpy, vs->wsw->bswl [ws->number]->w);
    }
    vs->wsw->state = BUTTONSSTATE;
}

/*
 * Verify if a window may be added to the workspace map.
 * This is not allowed for
 * - icon managers
 * - the occupy window
 * - workspace manager windows
 * - or, optionally, windows with full occupation.
 */
int WMapWindowMayBeAdded(TwmWindow *win)
{
    if (win->iconmgr)
	return 0;
    if (win == Scr->workSpaceMgr.occupyWindow->twm_win)
	return 0;
    if (win->wspmgr)
	return 0;
    if (Scr->workSpaceMgr.noshowoccupyall &&
	win->occupation == fullOccupation)
	return 0;
    return 1;
}

void WMapAddWindow (TwmWindow *win)
{
    WorkSpace     *ws;

    if (!WMapWindowMayBeAdded(win))
	return;

    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	if (OCCUPY (win, ws)) WMapAddToList (win, ws);
    }
}

void WMapDestroyWindow (TwmWindow *win)
{
    WorkSpace *ws;

    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	if (OCCUPY (win, ws)) WMapRemoveFromList (win, ws);
    }
    if (win == occupyWin) {
	OccupyWindow *occwin = Scr->workSpaceMgr.occupyWindow;
	XUnmapWindow (dpy, occwin->twm_win->frame);
	occwin->twm_win->mapped = FALSE;
	occwin->twm_win->occupation = 0;
	occupyWin = (TwmWindow*) 0;
    }
}

void WMapMapWindow (TwmWindow *win)
{
    virtualScreen *vs;
    WorkSpace *ws;
    WinList   wl;

    for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
      for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	for (wl = vs->wsw->mswl [ws->number]->wl; wl != NULL; wl = wl->next) {
	    if (wl->twm_win == win) {
		XMapWindow (dpy, wl->w);
		WMapRedrawName (vs, wl);
		break;
	    }
	}
      }
    }
}

void WMapSetupWindow (TwmWindow *win, int x, int y, int w, int h)
{
    virtualScreen *vs;
    WorkSpace     *ws;
    WinList	  wl;
    float	  wf, hf;

    if (win->iconmgr) return;
    if (!win->vs) return;

    if (win->wspmgr) {
	if (w == -1) return;
	ResizeWorkSpaceManager (win->vs, win);
	return;
    }
    if (win == Scr->workSpaceMgr.occupyWindow->twm_win) {
	if (w == -1) return;
	ResizeOccupyWindow (win);
	return;
    }
    for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
      WorkSpaceWindow *wsw = vs->wsw;
      wf = (float) (wsw->wwidth  - 2) / (float) vs->w;
      hf = (float) (wsw->wheight - 2) / (float) vs->h;
      for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	for (wl = wsw->mswl [ws->number]->wl; wl != NULL; wl = wl->next) {
	    if (win == wl->twm_win) {
		wl->x = (int) (x * wf);
		wl->y = (int) (y * hf);
		if (w == -1) {
		    XMoveWindow (dpy, wl->w, wl->x, wl->y);
		}
		else {
		    wl->width  = (unsigned int) ((w * wf) + 0.5);
		    wl->height = (unsigned int) ((h * hf) + 0.5);
		    if (!Scr->use3Dwmap) {
			wl->width  -= 2;
			wl->height -= 2;
		    }
		    if (wl->width  < 1) wl->width  = 1;
		    if (wl->height < 1) wl->height = 1;
		    XMoveResizeWindow (dpy, wl->w, wl->x, wl->y, wl->width, wl->height);
		}
		break;
	    }
	}
      }
    }
}

void WMapIconify (TwmWindow *win)
{
    WorkSpace *ws;
    WinList    wl;

    if (!win->vs) return;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	for (wl = win->vs->wsw->mswl [ws->number]->wl; wl != NULL; wl = wl->next) {
	    if (win == wl->twm_win) {
		XUnmapWindow (dpy, wl->w);
		break;
	    }
	}
    }
}

void WMapDeIconify (TwmWindow *win)
{
    WorkSpace *ws;
    WinList    wl;

    if (!win->vs) return;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	for (wl = win->vs->wsw->mswl [ws->number]->wl; wl != NULL; wl = wl->next) {
	    if (win == wl->twm_win) {
		if (Scr->NoRaiseDeicon)
		    XMapWindow (dpy, wl->w);
		else
		    XMapRaised (dpy, wl->w);
		WMapRedrawName (win->vs, wl);
		break;
	    }
	}
    }
}

void WMapRaiseLower (TwmWindow *win)
{
    WorkSpace *ws;

    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	if (OCCUPY (win, ws)) WMapRestack (ws);
    }
}

void WMapLower (TwmWindow *win)
{
    WorkSpace *ws;

    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	if (OCCUPY (win, ws)) WMapRestack (ws);
    }
}

void WMapRaise (TwmWindow *win)
{
    WorkSpace *ws;

    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	if (OCCUPY (win, ws)) WMapRestack (ws);
    }
}

void WMapRestack (WorkSpace *ws)
{
    virtualScreen *vs;
    TwmWindow	*win;
    WinList	wl;
    Window	root;
    Window	parent;
    Window	*children, *smallws;
    unsigned int number;
    int		i, j;

    number = 0;
    XQueryTree (dpy, Scr->Root, &root, &parent, &children, &number);
    smallws = (Window*) malloc (number * sizeof (Window));

    for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
      j = 0;
      for (i = number - 1; i >= 0; i--) {
	if (!(win = GetTwmWindow(children [i]))) {
	    continue;
	}
	if (win->frame != children [i]) continue; /* skip icons */
	if (! OCCUPY (win, ws)) continue;
	if (tracefile) {
	    fprintf (tracefile, "WMapRestack : w = %lx, win = %p\n", children [i], (void *)win);
	    fflush (tracefile);
	}
	for (wl = vs->wsw->mswl [ws->number]->wl; wl != NULL; wl = wl->next) {
	if (tracefile) {
	    fprintf (tracefile, "WMapRestack : wl = %p, twm_win = %p\n", (void *)wl, (void *)wl->twm_win);
	    fflush (tracefile);
	}
	    if (win == wl->twm_win) {
		smallws [j++] = wl->w;
		break;
	    }
	}
      }
      XRestackWindows (dpy, smallws, j);
    }
    XFree ((char *) children);
    free  (smallws);
    return;
}

void WMapUpdateIconName (TwmWindow *win)
{
    virtualScreen *vs;
    WorkSpace *ws;
    WinList   wl;

    for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
      for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	for (wl = vs->wsw->mswl [ws->number]->wl; wl != NULL; wl = wl->next) {
	    if (win == wl->twm_win) {
		WMapRedrawName (vs, wl);
		break;
	    }
	}
      }
    }
}

void WMgrHandleKeyReleaseEvent (virtualScreen *vs, XEvent *event)
{
    char	*keyname;
    KeySym	keysym;

    keysym  = XLookupKeysym ((XKeyEvent*) event, 0);
    if (! keysym) return;
    keyname = XKeysymToString (keysym);
    if (! keyname) return;
    if ((strcmp (keyname, "Control_R") == 0) || 
	(strcmp (keyname, "Control_L") == 0)) 
      {
	/* DontToggleWorkSpaceManagerState added 20040607 by dl*/
	if (!donttoggleworkspacemanagerstate)
	  {
	    WMapToggleState (vs);
	  }
	return;
      }
}

void WMgrHandleKeyPressEvent (virtualScreen *vs, XEvent *event)
{
    WorkSpace *ws;
    int	      len, i, lname;
    char      key [16];
    unsigned char k;
    char      name [128];
    char      *keyname;
    KeySym    keysym;

    keysym  = XLookupKeysym   ((XKeyEvent*) event, 0);
    if (! keysym) return;
    keyname = XKeysymToString (keysym);
    if (! keyname) return;
    if ((strcmp (keyname, "Control_R") == 0) || 
	(strcmp (keyname, "Control_L") == 0)) 
      {
	/* DontToggleWorkSpaceManagerState added 20040607 by dl*/
	if (!donttoggleworkspacemanagerstate)
	  {
	    WMapToggleState (vs);
	  }
	return;
    }
    if (vs->wsw->state == MAPSTATE) return;

    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	if (vs->wsw->bswl [ws->number]->w == event->xkey.subwindow) break;
    }
    if (ws == NULL) return;

    strcpy (name, ws->label);
    lname = strlen (name);
    len   = XLookupString (&(event->xkey), key, 16, NULL, NULL);
    for (i = 0; i < len; i++) {
        k = key [i];
	if (isprint (k)) {
	    name [lname++] = k;
	}
	else
	if ((k == 127) || (k == 8)) {
	    if (lname != 0) lname--;
	}
	else
	    break;
    }
    name [lname] = '\0';
    ws->label = realloc (ws->label, (strlen (name) + 1));
    strcpy (ws->label, name);
    if (ws == vs->wsw->currentwspc)
	PaintButton (WSPCWINDOW, vs, vs->wsw->bswl [ws->number]->w, ws->label, ws->cp,  on);
    else
	PaintButton (WSPCWINDOW, vs, vs->wsw->bswl [ws->number]->w, ws->label, ws->cp, off);
}

void WMgrHandleButtonEvent (virtualScreen *vs, XEvent *event)
{
    WorkSpaceWindow	*mw;
    WorkSpace		*ws, *oldws, *newws, *cws;
    WinList		wl;
    TwmWindow		*win;
    int			occupation;
    unsigned int	W0, H0, bw;
    int			cont;
    XEvent		ev;
    Window		w, sw, parent;
    int			X0, Y0, X1, Y1, XW, YW, XSW, YSW;
    Position		newX = 0, newY = 0, winX = 0, winY = 0;
    Window		junkW;
    unsigned int	junk;
    unsigned int	button;
    unsigned int	modifier;
    XSetWindowAttributes attrs;
    float		wf, hf;
    Boolean		alreadyvivible, realmovemode, startincurrent;
    Time		etime;

    parent   = event->xbutton.window;
    sw       = event->xbutton.subwindow;
    mw       = vs->wsw;
    button   = event->xbutton.button;
    modifier = event->xbutton.state;
    etime    = event->xbutton.time;

    if (vs->wsw->state == BUTTONSSTATE) {
	for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	    if (vs->wsw->bswl [ws->number]->w == parent) break;
	}
	if (ws == NULL) return;
	GotoWorkSpace (vs, ws);
	return;
    }

    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	if (vs->wsw->mswl [ws->number]->w == parent) break;
    }
    if (ws == NULL) return;
    if (sw == (Window) 0) {
	GotoWorkSpace (vs, ws);
	return;
    }
    oldws = ws;

    if (XFindContext (dpy, sw, MapWListContext, (XPointer *) &wl) == XCNOENT) return;
    win = wl->twm_win;
    if ((! Scr->TransientHasOccupation) && win->transient) return;

    XTranslateCoordinates (dpy, Scr->Root, sw, event->xbutton.x_root, event->xbutton.y_root,
				&XW, &YW, &junkW);
    realmovemode = ( Scr->ReallyMoveInWorkspaceManager && !(modifier & ShiftMask)) ||
		   (!Scr->ReallyMoveInWorkspaceManager &&  (modifier & ShiftMask));
    startincurrent = (oldws == vs->wsw->currentwspc);
    if (win->OpaqueMove) {
	int sw2, ss;

	sw2 = win->frame_width * win->frame_height;
	ss = vs->w * vs->h;
	if (sw2 > ((ss * Scr->OpaqueMoveThreshold) / 100))
	    Scr->OpaqueMove = FALSE;
	else
	    Scr->OpaqueMove = TRUE;
    } else {
	Scr->OpaqueMove = FALSE;
    }
    switch (button) {
	case 1 :
	    XUnmapWindow (dpy, sw);

	case 2 :
	    XGetGeometry (dpy, sw, &junkW, &X0, &Y0, &W0, &H0, &bw, &junk);
	    XTranslateCoordinates (dpy, vs->wsw->mswl [oldws->number]->w,
				mw->w, X0, Y0, &X1, &Y1, &junkW);

	    attrs.event_mask       = ExposureMask;
	    attrs.background_pixel = wl->cp.back;
	    attrs.border_pixel     = wl->cp.back;
	    w = XCreateWindow (dpy, mw->w, X1, Y1, W0, H0, bw,
				CopyFromParent,
				(unsigned int) CopyFromParent,
				(Visual *) CopyFromParent,
				CWEventMask | CWBackPixel | CWBorderPixel, &attrs);

	    XMapRaised (dpy, w);
	    WMapRedrawWindow (w, W0, H0, wl->cp, wl->twm_win->icon_name);
	    if (realmovemode && Scr->ShowWinWhenMovingInWmgr) {
		if (Scr->OpaqueMove) {
		    DisplayWin (vs, win);
		} else {
		    MoveOutline (Scr->Root,
			win->frame_x - win->frame_bw,
			win->frame_y - win->frame_bw,
			win->frame_width  + 2 * win->frame_bw,
			win->frame_height + 2 * win->frame_bw,
			win->frame_bw,
			win->title_height + win->frame_bw3D);
		}
	    }
	    break;

	case 3 :
	    occupation = win->occupation & ~(1 << oldws->number);
	    ChangeOccupation (win, occupation);
	    return;
	default :
	    return;
    }

    wf = (float) (mw->wwidth  - 1) / (float) vs->w;
    hf = (float) (mw->wheight - 1) / (float) vs->h;
    XGrabPointer (dpy, mw->w, False, ButtonPressMask | ButtonMotionMask | ButtonReleaseMask,
		  GrabModeAsync, GrabModeAsync, mw->w, Scr->MoveCursor, CurrentTime);

    alreadyvivible = False;
    cont = TRUE;
    while (cont) {
        MapSubwindow *msw;
	XMaskEvent (dpy, ButtonPressMask | ButtonMotionMask |
			 ButtonReleaseMask | ExposureMask, &ev);
	switch (ev.xany.type) {
	    case ButtonPress :
	    case ButtonRelease :
		if (ev.xbutton.button != button) break;
		cont = FALSE;
		newX = ev.xbutton.x;
		newY = ev.xbutton.y;

	    case MotionNotify :
		if (cont) {
		    newX = ev.xmotion.x;
		    newY = ev.xmotion.y;
		}
		if (realmovemode) {
		    for (cws = Scr->workSpaceMgr.workSpaceList;
			 cws != NULL;
			 cws = cws->next) {
		        msw = vs->wsw->mswl [cws->number];
			if ((newX >= msw->x) && (newX <  msw->x + mw->wwidth) &&
			    (newY >= msw->y) && (newY <  msw->y + mw->wheight)) {
			    break;
			}
		    }
		    if (!cws) break;
		    winX = newX - XW;
		    winY = newY - YW;
		    msw = vs->wsw->mswl [cws->number];
		    XTranslateCoordinates (dpy, mw->w, msw->w,
					winX, winY, &XSW, &YSW, &junkW);
		    winX = (int) (XSW / wf);
		    winY = (int) (YSW / hf);
		    if (Scr->DontMoveOff) {
			int width = win->frame_width;
			int height = win->frame_height;

			if ((winX < Scr->BorderLeft) && ((Scr->MoveOffResistance < 0) ||
                             (winX > Scr->BorderLeft - Scr->MoveOffResistance))) {
			    winX = Scr->BorderLeft;
			    newX = msw->x + XW + Scr->BorderLeft * mw->wwidth / vs->w;
			}
			if (((winX + width) > vs->w - Scr->BorderRight) &&
			    ((Scr->MoveOffResistance < 0) ||
			     ((winX + width) < vs->w - Scr->BorderRight + Scr->MoveOffResistance))) {
			    winX = vs->w - Scr->BorderRight - width;
			    newX = msw->x + mw->wwidth *
                                (1 - Scr->BorderRight / (double) vs->w) - wl->width + XW - 2;
			}
			if ((winY < Scr->BorderTop) && ((Scr->MoveOffResistance < 0) ||
                             (winY > Scr->BorderTop - Scr->MoveOffResistance))) {
			    winY = Scr->BorderTop;
			    newY = msw->y + YW + Scr->BorderTop * mw->height / vs->h;
			}
			if (((winY + height) > vs->h - Scr->BorderBottom) &&
			    ((Scr->MoveOffResistance < 0) ||
                             ((winY + height) < vs->h - Scr->BorderBottom + Scr->MoveOffResistance))) {
			    winY = vs->h - Scr->BorderBottom - height;
			    newY = msw->y + mw->wheight *
                                (1 - Scr->BorderBottom / (double) vs->h) - wl->height + YW - 2;
			}
		    }
		    WMapSetupWindow (win, winX, winY, -1, -1);
		    if (Scr->ShowWinWhenMovingInWmgr) goto movewin;
		    if (cws == vs->wsw->currentwspc) {
			if (alreadyvivible) goto movewin;
			if (Scr->OpaqueMove) {
			    XMoveWindow (dpy, win->frame, winX, winY);
			    DisplayWin (vs, win);
			} else {
			    MoveOutline (Scr->Root,
				winX - win->frame_bw, winY - win->frame_bw,
				win->frame_width  + 2 * win->frame_bw,
				win->frame_height + 2 * win->frame_bw,
				win->frame_bw,
				win->title_height + win->frame_bw3D);
			}
			alreadyvivible = True;
			goto move;
		    }
		    if (!alreadyvivible) goto move;
		    if (!OCCUPY (win, vs->wsw->currentwspc) ||
			(startincurrent && (button == 1))) {
			if (Scr->OpaqueMove) {
			    Vanish (vs, win);
			    XMoveWindow (dpy, win->frame, winX, winY);
			} else {
			    MoveOutline (Scr->Root, 0, 0, 0, 0, 0, 0);
			}
			alreadyvivible = False;
			goto move;
		    }
movewin:	    if (Scr->OpaqueMove) {
			XMoveWindow (dpy, win->frame, winX, winY);
		    } else {
			MoveOutline (Scr->Root,
				winX - win->frame_bw, winY - win->frame_bw,
				win->frame_width  + 2 * win->frame_bw,
				win->frame_height + 2 * win->frame_bw,
				win->frame_bw,
				win->title_height + win->frame_bw3D);
		    }
		}
move:		XMoveWindow (dpy, w, newX - XW, newY - YW);
		break;
	    case Expose :
		if (ev.xexpose.window == w) {
		    WMapRedrawWindow (w, W0, H0, wl->cp, wl->twm_win->icon_name);
		    break;
		}
		Event = ev;
		DispatchEvent ();
		break;
	}
    }
    if (realmovemode) {
	if (Scr->ShowWinWhenMovingInWmgr || alreadyvivible) {
	    if (Scr->OpaqueMove && !OCCUPY (win, vs->wsw->currentwspc)) {
		Vanish (vs, win);
	    }
	    if (!Scr->OpaqueMove) {
		MoveOutline (Scr->Root, 0, 0, 0, 0, 0, 0);
		WMapRedrawName (vs, wl);
	    }
	}
	SetupWindow (win, winX, winY, win->frame_width, win->frame_height, -1);
    }
    ev.xbutton.subwindow = (Window) 0;
    ev.xbutton.window = parent;
    XPutBackEvent   (dpy, &ev);
    XUngrabPointer  (dpy, CurrentTime);

    if ((ev.xbutton.time - etime) < 250) {
	KeyCode control_L_code, control_R_code;
	KeySym  control_L_sym,  control_R_sym;
	char keys [32];

	XMapWindow (dpy, sw);
	XDestroyWindow (dpy, w);
	GotoWorkSpace (vs, ws);
	if (!Scr->DontWarpCursorInWMap) WarpToWindow (win, Scr->RaiseOnWarp);
	control_L_sym  = XStringToKeysym  ("Control_L");
	control_R_sym  = XStringToKeysym  ("Control_R");
	control_L_code = XKeysymToKeycode (dpy, control_L_sym);
	control_R_code = XKeysymToKeycode (dpy, control_R_sym);
	XQueryKeymap (dpy, keys);
	if ((keys [control_L_code / 8] & ((char) 0x80 >> (control_L_code % 8))) ||
	     keys [control_R_code / 8] & ((char) 0x80 >> (control_R_code % 8))) {
	    WMapToggleState (vs);
	}
	return;
    }

    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
        MapSubwindow *msw = vs->wsw->mswl [ws->number];
	if ((newX >= msw->x) && (newX < msw->x + mw->wwidth) &&
	    (newY >= msw->y) && (newY < msw->y + mw->wheight)) {
	    break;
	}
    }
    newws = ws;
    switch (button) {
	case 1 :
	    if ((newws == NULL) || (newws == oldws) || OCCUPY (wl->twm_win, newws)) {
		XMapWindow (dpy, sw);
		break;
	    }
	    occupation = (win->occupation | (1 << newws->number)) & ~(1 << oldws->number);
	    ChangeOccupation (win, occupation);
	    if (newws == vs->wsw->currentwspc) {
		RaiseWindow (win);
		WMapRaise (win);
	    }
	    else WMapRestack (newws);
	    break;

	case 2 :
	    if ((newws == NULL) || (newws == oldws) ||
		OCCUPY (wl->twm_win, newws)) break;

	    occupation = win->occupation | (1 << newws->number);
	    ChangeOccupation (win, occupation);
	    if (newws == vs->wsw->currentwspc) {
		RaiseWindow (win);
		WMapRaise (win);
	    }
	    else WMapRestack (newws);
	    break;

	default :
	    return;
    }
    XDestroyWindow (dpy, w);
}

void InvertColorPair (ColorPair *cp)
{
    Pixel save;

    save = cp->fore;
    cp->fore = cp->back;
    cp->back = save;
    save = cp->shadc;
    cp->shadc = cp->shadd;
    cp->shadd = save;
}

void WMapRedrawName (virtualScreen *vs, WinList wl)
{
    int       w = wl->width;
    int       h = wl->height;
    ColorPair cp;
    char      *label;

    label  = wl->twm_win->icon_name;
     cp     = wl->cp;

    if (Scr->ReverseCurrentWorkspace && wl->wlist == vs->wsw->currentwspc) {
	InvertColorPair (&cp);
    }
    WMapRedrawWindow (wl->w, w, h, cp, label);
}

static void WMapRedrawWindow (Window window, int width, int height,
			      ColorPair cp, char *label)
{
    int		x, y, strhei, strwid;
    MyFont	font;
    XFontSetExtents *font_extents;
    XRectangle inc_rect;
    XRectangle logical_rect;
    XFontStruct **xfonts;
    char **font_names;
    register int i;
    int descent;
    int fnum;

    XClearWindow (dpy, window);
    font = Scr->workSpaceMgr.windowFont;

    font_extents = XExtentsOfFontSet(font.font_set);
    strhei = font_extents->max_logical_extent.height;

    if (strhei > height) return;

    XmbTextExtents(font.font_set, label, strlen (label),
		   &inc_rect, &logical_rect);
    strwid = logical_rect.width;
    x = (width  - strwid) / 2;
    if (x < 1) x = 1;

    fnum = XFontsOfFontSet(font.font_set, &xfonts, &font_names);
    for( i = 0, descent = 0; i < fnum; i++){
	/* xf = xfonts[i]; */
	descent = ((descent < (xfonts[i]->max_bounds.descent)) ? (xfonts[i]->max_bounds.descent): descent);
    }
    
    y = ((height + strhei) / 2) - descent;

    if (Scr->use3Dwmap) {
	Draw3DBorder (window, 0, 0, width, height, 1, cp, off, True, False);
	FB(cp.fore, cp.back);
    } else {
	FB (cp.back, cp.fore);
	XFillRectangle (dpy, window, Scr->NormalGC, 0, 0, width, height);
	FB (cp.fore, cp.back);
    }
    if (Scr->Monochrome != COLOR) {
	XmbDrawImageString (dpy, window, font.font_set, Scr->NormalGC, x, y, label, strlen (label));
    } else {
	XmbDrawString (dpy, window, font.font_set,Scr->NormalGC, x, y, label, strlen (label));
    }
}

static void WMapAddToList (TwmWindow *win, WorkSpace *ws)
{
    virtualScreen *vs;
    WinList wl;
    float   wf, hf;
    ColorPair cp;
    XSetWindowAttributes attr;
    unsigned long attrmask;
    unsigned int bw;

    cp.back = win->title.back;
    cp.fore = win->title.fore;
    if (Scr->workSpaceMgr.windowcpgiven) {
	cp.back = Scr->workSpaceMgr.windowcp.back;
	GetColorFromList (Scr->workSpaceMgr.windowBackgroundL,
			win->full_name, &win->class, &cp.back);
	cp.fore = Scr->workSpaceMgr.windowcp.fore;
	GetColorFromList (Scr->workSpaceMgr.windowForegroundL,
		      win->full_name, &win->class, &cp.fore);
    }
    if (Scr->use3Dwmap && !Scr->BeNiceToColormap) {
	GetShadeColors (&cp);
    }
    for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
      wf = (float) (vs->wsw->wwidth  - 2) / (float) vs->w;
      hf = (float) (vs->wsw->wheight - 2) / (float) vs->h;
      wl = (WinList) malloc (sizeof (struct winList));
      wl->wlist  = ws;
      wl->x      = (int) (win->frame_x * wf);
      wl->y      = (int) (win->frame_y * hf);
      wl->width  = (unsigned int) ((win->frame_width  * wf) + 0.5);
      wl->height = (unsigned int) ((win->frame_height * hf) + 0.5);
      bw = 0;
      if (!Scr->use3Dwmap) {
	bw = 1;
	wl->width  -= 2;
	wl->height -= 2;
      }
      if (wl->width  < 1) wl->width  = 1;
      if (wl->height < 1) wl->height = 1;
      wl->w = XCreateSimpleWindow (dpy, vs->wsw->mswl [ws->number]->w, wl->x, wl->y,
				   wl->width, wl->height, bw, Scr->Black, cp.back);
      attrmask = 0;
      if (Scr->BackingStore) {
	attr.backing_store = WhenMapped;
	attrmask |= CWBackingStore;
      }
      attr.cursor = handCursor;
      attrmask |= CWCursor;
      XChangeWindowAttributes (dpy, wl->w, attrmask, &attr);
      XSelectInput (dpy, wl->w, ExposureMask);
      XSaveContext (dpy, wl->w, TwmContext, (XPointer) vs->wsw->twm_win);
      XSaveContext (dpy, wl->w, ScreenContext, (XPointer) Scr);
      XSaveContext (dpy, wl->w, MapWListContext, (XPointer) wl);
      wl->twm_win = win;
      wl->cp      = cp;
      wl->next    = vs->wsw->mswl [ws->number]->wl;
      vs->wsw->mswl [ws->number]->wl = wl;
      if (win->mapped) XMapWindow (dpy, wl->w);
    }
}

static void WMapRemoveFromList (TwmWindow *win, WorkSpace *ws)
{
    virtualScreen *vs;
    WinList wl, *prev;

    for (vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
	prev = &vs->wsw->mswl [ws->number]->wl;
	wl = *prev;
	while (wl != NULL) {
	    if (win == wl->twm_win) {
		*prev = wl->next;
		XDeleteContext (dpy, wl->w, TwmContext);
		XDeleteContext (dpy, wl->w, ScreenContext);
		XDeleteContext (dpy, wl->w, MapWListContext);
		XDestroyWindow (dpy, wl->w);
		free (wl);
		break;
	    }
	    prev = &wl->next;
	    wl   = *prev;
	}
    }
}

static void ResizeWorkSpaceManager (virtualScreen *vs, TwmWindow *win)
{
    int           bwidth, bheight;
    int		  wwidth, wheight;
    int           hspace, vspace;
    int           lines, columns;
    int		  neww, newh;
    WorkSpace     *ws;
    TwmWindow	  *tmp_win;
    WinList	  wl;
    int           i, j;
    float	  wf, hf;

    neww = win->attr.width;
    newh = win->attr.height;
    if (neww == vs->wsw->width && newh == vs->wsw->height)
	return;

    hspace  = Scr->workSpaceMgr.hspace;
    vspace  = Scr->workSpaceMgr.vspace;
    lines   = Scr->workSpaceMgr.lines;
    columns = Scr->workSpaceMgr.columns;
    bwidth  = (neww - (columns * hspace)) / columns;
    bheight = (newh - (lines   * vspace)) / lines;
    wwidth  = neww / columns;
    wheight = newh / lines;
    wf = (float) (wwidth  - 2) / (float) vs->w;
    hf = (float) (wheight - 2) / (float) vs->h;

    i = 0;
    j = 0;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
        MapSubwindow *msw = vs->wsw->mswl [ws->number];
	XMoveResizeWindow (dpy, vs->wsw->bswl [ws->number]->w,
				i * (bwidth  + hspace) + (hspace / 2),
				j * (bheight + vspace) + (vspace / 2),
				bwidth, bheight);
	msw->x = i * wwidth;
	msw->y = j * wheight;
	XMoveResizeWindow (dpy, msw->w, msw->x, msw->y, wwidth - 2, wheight - 2);
	for (wl = msw->wl; wl != NULL; wl = wl->next) {
	    tmp_win    = wl->twm_win;
	    wl->x      = (int) (tmp_win->frame_x * wf);
	    wl->y      = (int) (tmp_win->frame_y * hf);
	    wl->width  = (unsigned int) ((tmp_win->frame_width  * wf) + 0.5);
	    wl->height = (unsigned int) ((tmp_win->frame_height * hf) + 0.5);
	    XMoveResizeWindow (dpy, wl->w, wl->x, wl->y, wl->width, wl->height);
	}
	i++;
	if (i == columns) {i = 0; j++;};
    }
    vs->wsw->bwidth    = bwidth;
    vs->wsw->bheight   = bheight;
    vs->wsw->width     = neww;
    vs->wsw->height    = newh;
    vs->wsw->wwidth	= wwidth;
    vs->wsw->wheight	= wheight;
    PaintWorkSpaceManager (vs);
}

static void ResizeOccupyWindow (TwmWindow *win)
{
    int        bwidth, bheight, owidth, oheight;
    int        hspace, vspace;
    int        lines, columns;
    int        neww, newh;
    WorkSpace  *ws;
    int        i, j, x, y;
    OccupyWindow *occwin = Scr->workSpaceMgr.occupyWindow;

    neww = win->attr.width;
    newh = win->attr.height;
    if (occwin->width == neww && occwin->height == newh)
	return;

    hspace  = occwin->hspace;
    vspace  = occwin->vspace;
    lines   = Scr->workSpaceMgr.lines;
    columns = Scr->workSpaceMgr.columns;
    bwidth  = (neww -  columns    * hspace) / columns;
    bheight = (newh - (lines + 2) * vspace) / (lines + 1);
    owidth  = occwin->owidth;
    oheight = bheight;

    i = 0;
    j = 0;
    for (ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	XMoveResizeWindow (dpy, occwin->obuttonw [j * columns + i],
			   i * (bwidth  + hspace) + (hspace / 2),
			   j * (bheight + vspace) + (vspace / 2),
			   bwidth, bheight);
	i++;
	if (i == columns) {i = 0; j++;}
    }
    hspace = (neww - 3 * owidth) / 4;
    x = hspace;
    y = ((bheight + vspace) * lines) + ((3 * vspace) / 2);
    XMoveResizeWindow (dpy, occwin->OK, x, y, owidth, oheight);
    x += owidth + hspace;
    XMoveResizeWindow (dpy, occwin->cancel, x, y, owidth, oheight);
    x += owidth + hspace;
    XMoveResizeWindow (dpy, occwin->allworkspc, x, y, owidth, oheight);

    occwin->width   = neww;
    occwin->height  = newh;
    occwin->bwidth  = bwidth;
    occwin->bheight = bheight;
    occwin->owidth  = owidth;
    PaintOccupyWindow ();
}

void WMapCreateCurrentBackGround (char *border,
				  char *background, char *foreground,
				  char *pixmap)
{
    Image *image;
    WorkSpaceMgr *ws = &Scr->workSpaceMgr;

    ws->curBorderColor = Scr->Black;
    ws->curColors.back = Scr->White;
    ws->curColors.fore = Scr->Black;
    ws->curImage       = None;

    if (border == NULL)
	return;
    GetColor (Scr->Monochrome, &ws->curBorderColor, border);
    if (background == NULL)
	return;
    ws->curPaint = True;
    GetColor (Scr->Monochrome, &ws->curColors.back, background);
    if (foreground == NULL)
	return;
    GetColor (Scr->Monochrome, &ws->curColors.fore, foreground);

    if (pixmap == NULL)
	return;
      if ((image = GetImage (pixmap, Scr->workSpaceMgr.curColors)) == None) {
	fprintf (stderr, "Can't find pixmap %s\n", pixmap);
	return;
    }
    ws->curImage = image;
}

void WMapCreateDefaultBackGround (char *border,
				  char *background, char *foreground,
				  char *pixmap)
{
    Image *image;
    WorkSpaceMgr *ws = &Scr->workSpaceMgr;

    ws->defBorderColor = Scr->Black;
    ws->defColors.back = Scr->White;
    ws->defColors.fore = Scr->Black;
    ws->defImage       = None;

    if (border == NULL)
	return;
    GetColor (Scr->Monochrome, &ws->defBorderColor, border);
    if (background == NULL)
	return;
    GetColor (Scr->Monochrome, &ws->defColors.back, background);
    if (foreground == NULL)
	return;
    GetColor (Scr->Monochrome, &ws->defColors.fore, foreground);
    if (pixmap == NULL)
	return;
    if ((image = GetImage (pixmap, ws->defColors)) == None)
	return;
    ws->defImage = image;
}

Bool AnimateRoot (void)
{
    virtualScreen *vs;
    ScreenInfo *scr;
    int	       scrnum;
    Image      *image;
    WorkSpace  *ws;
    Bool       maybeanimate;

    maybeanimate = False;
    for (scrnum = 0; scrnum < NumScreens; scrnum++) {
	if ((scr = ScreenList [scrnum]) == NULL) continue;
	if (! scr->workSpaceManagerActive) continue;

	for (vs = scr->vScreenList; vs != NULL; vs = vs->next) {
	  if (! vs->wsw->currentwspc) continue;
	  image = vs->wsw->currentwspc->image;
	  if ((image == None) || (image->next == None)) continue;
	  if (scr->DontPaintRootWindow) continue;

	  XSetWindowBackgroundPixmap (dpy, vs->window, image->pixmap);
	  XClearWindow (dpy, scr->Root);
	  vs->wsw->currentwspc->image = image->next;
	  maybeanimate = True;
	}
    }
    for (scrnum = 0; scrnum < NumScreens; scrnum++) {
	if ((scr = ScreenList [scrnum]) == NULL) continue;

	for (vs = scr->vScreenList; vs != NULL; vs = vs->next) {
	  if (vs->wsw->state == BUTTONSSTATE) continue;
	  for (ws = scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
	    image = ws->image;

	    if ((image == None) || (image->next == None)) continue;
	    if (ws == vs->wsw->currentwspc) continue;
	    XSetWindowBackgroundPixmap (dpy, vs->wsw->mswl [ws->number]->w, image->pixmap);
	    XClearWindow (dpy, vs->wsw->mswl [ws->number]->w);
	    ws->image = image->next;
	    maybeanimate = True;
	  }
	}
    }
    return (maybeanimate);
}

static char **GetCaptivesList (int scrnum)
{
    unsigned char	*prop, *p;
    unsigned long	bytesafter;
    unsigned long	len;
    Atom		actual_type;
    int			actual_format;
    char		**ret;
    int			count;
    int			i, l;
    Window		root;

    _XA_WM_CTWMSLIST = XInternAtom (dpy, "WM_CTWMSLIST", True);
    if (_XA_WM_CTWMSLIST == None) return ((char**)0);

    root = RootWindow (dpy, scrnum);
    if (XGetWindowProperty (dpy, root, _XA_WM_CTWMSLIST, 0L, 512,
			False, XA_STRING, &actual_type, &actual_format, &len,
			&bytesafter, &prop) != Success) return ((char**) 0);
    if (len == 0) return ((char**) 0);

    count = 0;
    p = prop;
    l = 0;
    while (l < len) {
	l += strlen ((char*)p) + 1;
	p += strlen ((char*)p) + 1;
	count++;
    }
    ret = (char**) malloc ((count + 1) * sizeof (char*));

    p = prop;
    l = 0;
    i = 0;
    while (l < len) {
	ret [i++] = (char*) strdup ((char*) p);
	l += strlen ((char*)p) + 1;
	p += strlen ((char*)p) + 1;
    }
    ret [i] = (char*) 0;
    XFree ((char *)prop);

    return (ret);
}

static void SetCaptivesList (int scrnum, char **clist)
{
    unsigned long	len;
    char		**cl;
    char		*s, *slist;
    Window		root = RootWindow (dpy, scrnum);

    _XA_WM_CTWMSLIST = XInternAtom (dpy, "WM_CTWMSLIST", False);
    cl  = clist; len = 0;
    while (*cl) { len += strlen (*cl++) + 1; }
    if (len == 0) {
	XDeleteProperty (dpy, root, _XA_WM_CTWMSLIST);
	return;
    }
    slist = (char*) malloc (len * sizeof (char));
    cl = clist; s  = slist;
    while (*cl) {
	strcpy (s, *cl);
	s += strlen (*cl);
	*s++ = '\0';
	cl++;
    }
    XChangeProperty (dpy, root, _XA_WM_CTWMSLIST, XA_STRING, 8, 
		     PropModeReplace, (unsigned char *) slist, len);
}

static void freeCaptiveList (char **clist)
{
    while (clist && *clist) { free (*clist++); }
}

void AddToCaptiveList (void)
{
    int		i, count;
    char	**clist, **cl, **newclist;
    int		busy [32];
    Atom	_XA_WM_CTWM_ROOT;
    char	*atomname;
    int		scrnum = Scr->screen;
    Window	croot  = Scr->Root;
    Window	root   = RootWindow (dpy, scrnum);

    for (i = 0; i < 32; i++) { busy [i] = 0; }
    clist = GetCaptivesList (scrnum);
    cl = clist;
    count = 0;
    while (cl && *cl) {
	count++;
	if (!captivename) {
	    if (!strncmp (*cl, "ctwm-", 5)) {
		int r, n;
		r = sscanf (*cl, "ctwm-%d", &n);
		cl++;
		if (r != 1) continue;
		if ((n < 0) || (n > 31)) continue;
		busy [n] = 1;
	    } else cl++;
	    continue;
	}
	if (!strcmp (*cl, captivename)) {
	    fprintf (stderr, "A captive ctwm with name %s is already running\n", captivename);
	    exit (1);
	}
	cl++;
    }
    if (!captivename) {
	for (i = 0; i < 32; i++) {
	    if (!busy [i]) break;
	}
	if (i == 32) { /* no one can tell we didn't try hard */
	    fprintf (stderr, "Cannot find a suitable name for captive ctwm\n");
	    exit (1);
	}
	captivename = (char*) malloc (8);
	sprintf (captivename, "ctwm-%d", i);
    }
    newclist = (char**) malloc ((count + 2) * sizeof (char*));
    for (i = 0; i < count; i++) {
	newclist [i] = (char*) strdup (clist [i]);
    }
    newclist [count] = (char*) strdup (captivename);
    newclist [count + 1] = (char*) 0;
    SetCaptivesList (scrnum, newclist);
    freeCaptiveList (clist);
    freeCaptiveList (newclist);
    free (clist); free (newclist);

    root = RootWindow (dpy, scrnum);
    atomname = (char*) malloc (strlen ("WM_CTWM_ROOT_") + strlen (captivename) +1);
    sprintf (atomname, "WM_CTWM_ROOT_%s", captivename);
    _XA_WM_CTWM_ROOT = XInternAtom (dpy, atomname, False);
    XChangeProperty (dpy, root, _XA_WM_CTWM_ROOT, XA_WINDOW, 32, 
		     PropModeReplace, (unsigned char *) &croot, 4);
}

void RemoveFromCaptiveList (void)
{
    int	 count;
    char **clist, **cl, **newclist;
    Atom _XA_WM_CTWM_ROOT;
    char *atomname;
    int scrnum = Scr->screen;
    Window root = RootWindow (dpy, scrnum);

    if (!captivename) return;
    clist = GetCaptivesList (scrnum);
    cl = clist; count = 0;
    while (*cl) {
      count++;
      cl++;
    }
    newclist = (char**) malloc (count * sizeof (char*));
    cl = clist; count = 0;
    while (*cl) {
	if (!strcmp (*cl, captivename)) { cl++; continue; }
	newclist [count++] = *cl;
	cl++;
    }
    newclist [count] = (char*) 0;
    SetCaptivesList (scrnum, newclist);
    freeCaptiveList (clist);
    free (clist); free (newclist);

    atomname = (char*) malloc (strlen ("WM_CTWM_ROOT_") + strlen (captivename) +1);
    sprintf (atomname, "WM_CTWM_ROOT_%s", captivename);
    _XA_WM_CTWM_ROOT = XInternAtom (dpy, atomname, True);
    if (_XA_WM_CTWM_ROOT == None) return;
    XDeleteProperty (dpy, root, _XA_WM_CTWM_ROOT);
}

void SetPropsIfCaptiveCtwm (TwmWindow *win)
{
    Window	window = win->w;
    Window	frame  = win->frame;
    Atom	_XA_WM_CTWM_ROOT;

    if (!CaptiveCtwmRootWindow (window)) return;
    _XA_WM_CTWM_ROOT = XInternAtom (dpy, "WM_CTWM_ROOT", True);
    if (_XA_WM_CTWM_ROOT == None) return;

    XChangeProperty (dpy, frame, _XA_WM_CTWM_ROOT, XA_WINDOW, 32, 
		     PropModeReplace, (unsigned char *) &window, 4);
}

Window CaptiveCtwmRootWindow (Window window)
{
    Window	       *prop;
    Window		w;
    unsigned long	bytesafter;
    unsigned long	len;
    Atom		actual_type;
    int			actual_format;
    Atom		_XA_WM_CTWM_ROOT;

    _XA_WM_CTWM_ROOT = XInternAtom (dpy, "WM_CTWM_ROOT", True);
    if (_XA_WM_CTWM_ROOT == None) return ((Window)0);

    if (XGetWindowProperty (dpy, window, _XA_WM_CTWM_ROOT, 0L, 1L,
			False, XA_WINDOW, &actual_type, &actual_format, &len,
			&bytesafter, (unsigned char **)&prop) != Success)
	return ((Window)0);
    if (len == 0) return ((Window)0);
    w = *prop;
    XFree ((char *)prop);
    return w;
}

CaptiveCTWM GetCaptiveCTWMUnderPointer (void)
{
    int		scrnum = Scr->screen;
    Window	root;
    Window	child, croot;
    CaptiveCTWM	cctwm;

    root = RootWindow (dpy, scrnum);
    while (1) {
	XQueryPointer (dpy, root, &JunkRoot, &child,
			&JunkX, &JunkY, &JunkX, &JunkY, &JunkMask);
	if (child && (croot = CaptiveCtwmRootWindow (child))) {
	    root = croot;
	    continue;
	}
	cctwm.root = root;
	XFetchName (dpy, root, &cctwm.name);
	if (!cctwm.name) cctwm.name = (char*) strdup ("Root");
	return (cctwm);
    }
}

void SetNoRedirect (Window window)
{
    Atom	_XA_WM_NOREDIRECT;

    _XA_WM_NOREDIRECT = XInternAtom (dpy, "WM_NOREDIRECT", False);
    if (_XA_WM_NOREDIRECT == None) return;

    XChangeProperty (dpy, window, _XA_WM_NOREDIRECT, XA_STRING, 8, 
		     PropModeReplace, (unsigned char *) "Yes", 4);
}

static Bool DontRedirect (Window window)
{
    unsigned char	*prop;
    unsigned long	bytesafter;
    unsigned long	len;
    Atom		actual_type;
    int			actual_format;
    Atom		_XA_WM_NOREDIRECT;

    _XA_WM_NOREDIRECT = XInternAtom (dpy, "WM_NOREDIRECT", True);
    if (_XA_WM_NOREDIRECT == None) return (False);

    if (XGetWindowProperty (dpy, window, _XA_WM_NOREDIRECT, 0L, 1L,
			False, XA_STRING, &actual_type, &actual_format, &len,
			&bytesafter, &prop) != Success) return (False);
    if (len == 0) return (False);
    XFree ((char *)prop);
    return (True);
}

Bool visible (TwmWindow *tmp_win)
{
  return (tmp_win->vs != NULL);
}

#ifdef BUGGY_HP700_SERVER
static void fakeRaiseLower (display, window)
Display *display;
Window   window;
{
    Window          root;
    Window          parent;
    Window          grandparent;
    Window         *children;
    unsigned int    number;
    XWindowChanges  changes;

    number = 0;
    XQueryTree (display, window, &root, &parent, &children, &number);
    XFree ((char *) children);
    XQueryTree (display, parent, &root, &grandparent, &children, &number);

    changes.stack_mode = (children [number-1] == window) ? Below : Above;
    XFree ((char *) children);
    XConfigureWindow (display, window, CWStackMode, &changes);
}
#endif



