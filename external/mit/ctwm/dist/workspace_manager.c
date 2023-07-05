/*
 * Copyright 1992 Claude Lecommandeur.
 */

#include "ctwm.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <X11/Xatom.h>

#include "ctwm_atoms.h"
#include "util.h"
#include "animate.h"
#include "screen.h"
#include "add_window.h"
#include "events.h"
#include "otp.h"
#include "cursor.h"
#include "image.h"
#include "drawing.h"
#include "list.h"
#include "occupation.h"
#include "vscreen.h"
#include "win_decorations.h"
#include "win_iconify.h"
#include "win_ops.h"
#include "win_utils.h"
#include "workspace_manager.h"
#include "workspace_utils.h"
#include "xparsegeometry.h"


#include "gram.tab.h"


// Temp; x-ref desc in workspace_utils
extern bool useBackgroundInfo;


static void CreateWorkSpaceManagerWindow(VirtualScreen *vs);
static void ResizeWorkSpaceManager(VirtualScreen *vs, TwmWindow *win);
static void PaintWorkSpaceManagerBorder(VirtualScreen *vs);

static void wmap_mapwin_backend(TwmWindow *win, bool handleraise);

static void WMapRedrawWindow(Window window, int width, int height,
                             ColorPair cp, const char *label);

static void InvertColorPair(ColorPair *cp);


static XContext MapWListContext = None;
static Cursor handCursor = None;



/*
 ****************************************************************
 *
 * First, functions related to general creation and drawing of the WSM
 * window and its backing structs
 *
 ****************************************************************
 */

/*
 * Allocate an X Context for WSM stuff.
 */
void
InitWorkSpaceManagerContext(void)
{
	if(MapWListContext == None) {
		MapWListContext = XUniqueContext();
	}
}


/**
 * Prep up structures for WSM windows in each VS.  Called (for each
 * Screen) in startup after InitVirtualScreens() has setup the VS stuff
 * (and after config file processing).  This also retrieves info for each
 * vs about which workspace is active, if available (from restarting
 * ourself, etc).
 *
 * XXX Passed param isn't quite complete, as we call some funcs that use
 * global Scr...
 */
void
ConfigureWorkSpaceManager(ScreenInfo *scr)
{
	WorkSpace *ws = Scr->workSpaceMgr.workSpaceList;
	char *vsmapbuf, *vsmap;

	// Get the workspace name that's up on this vscreen.  This is
	// where the startup process actually sets what workspace we're
	// [re]starting in.
	vsmapbuf = CtwmGetVScreenMap(dpy, Scr->Root);
	if(vsmapbuf != NULL) {
		// Property is a comma-separate list of the workspaces for
		// each vscreen, in magic order.  So we start by chopping off
		// the first and then re-chop in the loop below.
		vsmap = strtok(vsmapbuf, ",");
	}
	else {
		vsmap = NULL;
	}


	// Setup each vs
	for(VirtualScreen *vs = scr->vScreenList; vs != NULL; vs = vs->next) {
		WorkSpace *fws;

		/*
		 * Make sure this is all properly initialized to nothing.  Otherwise
		 * bad and undefined behavior can show up in certain cases (e.g.,
		 * with no Workspaces {} defined in .ctwmrc, the only defined
		 * workspace will be random memory bytes, which can causes crashes on
		 * e.g.  f.menu "TwmWindows".)
		 */
		WorkSpaceWindow *wsw = calloc(1, sizeof(WorkSpaceWindow));
		wsw->state = scr->workSpaceMgr.initialstate;

		// If we have a current ws for this vs, assign it in, and
		// loop onward to the ws for the next vs.  For any we don't
		// have a default for, the default ws is the first one we haven't
		// used yet.
		if((fws = GetWorkspace(vsmap)) != NULL) {
			wsw->currentwspc = fws;
			vsmap = strtok(NULL, ",");
		}
		else {
			wsw->currentwspc = ws;
			if(ws) {
				ws = ws->next;
			}
		}

		vs->wsw = wsw;
	}

	free(vsmapbuf);
}




/*
 * Create workspace manager windows for each vscreen.  Called (for each
 * screen) late in startup, after the preceeding funcs have run their
 * course.
 */
void
CreateWorkSpaceManager(void)
{
	if(! Scr->workSpaceManagerActive) {
		return;
	}

	/* Setup basic fonts/colors/cursors */
	Scr->workSpaceMgr.windowFont.basename =
	        "-adobe-courier-medium-r-normal--10-100-75-75-m-60-iso8859-1";
	Scr->workSpaceMgr.buttonFont = Scr->IconManagerFont;
	Scr->workSpaceMgr.cp         = Scr->IconManagerC;
	if(!Scr->BeNiceToColormap) {
		GetShadeColors(&Scr->workSpaceMgr.cp);
	}
	if(handCursor == None) {
		NewFontCursor(&handCursor, "top_left_arrow");
	}


	/*
	 * Create a WSM window for each vscreen.  We don't need one for each
	 * workspace (we just reuse the same one), but we do need one for
	 * each vscreen (since they have to be displayed simultaneously).
	 */
	for(VirtualScreen *vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
		CreateWorkSpaceManagerWindow(vs);
	}


	/*
	 * Init background in the WSM workspace subwindow and potentially the
	 * root window to the settings for the active workspace
	 *
	 * XXX CTAG_BGDRAW This process is also done in similar fashion
	 * during CreateWorkSpaceManagerWindow(), and the two parts are done
	 * split well apart during GotoWorkSpace().  The details of the
	 * process should be factored out into helper functions instead of
	 * being reimplemented in each place.  That will require a little
	 * shuffling of code, and careful thinking on the apparent
	 * differences (which seem like they may be cosmetic).  Todo.
	 */
	for(VirtualScreen *vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
		WorkSpaceWindow *wsw = vs->wsw;    // Our WSW
		WorkSpace *ws2 = wsw->currentwspc; // Active WS
		MapSubwindow *msw = wsw->mswl[ws2->number]; // Active WS's subwin

		/* Setup the background/border on the active workspace */
		if(Scr->workSpaceMgr.curImage == NULL) {
			if(Scr->workSpaceMgr.curPaint) {
				XSetWindowBackground(dpy, msw->w, Scr->workSpaceMgr.curColors.back);
			}
		}
		else {
			XSetWindowBackgroundPixmap(dpy, msw->w, Scr->workSpaceMgr.curImage->pixmap);
		}
		XSetWindowBorder(dpy, msw->w, Scr->workSpaceMgr.curBorderColor);
		XClearWindow(dpy, msw->w);

		/* Set the root window to the color/image of that WS if we should */
		if(useBackgroundInfo && ! Scr->DontPaintRootWindow) {
			if(ws2->image == NULL) {
				XSetWindowBackground(dpy, vs->window, ws2->backcp.back);
			}
			else {
				XSetWindowBackgroundPixmap(dpy, vs->window, ws2->image->pixmap);
			}
			XClearWindow(dpy, vs->window);
		}
	}


	/*
	 * Set the property we use to store the full list of workspaces.
	 *
	 * XXX This isn't really part of creating the WSM windows, so doesn't
	 * strictly belong here.  It does need to happen after the config
	 * file parsing setup the workspaces, so couldn't go into
	 * InitWorkSpaceManager().  It could probably move into
	 * ConfigureWorkSpaceManager() though, or could move into a separate
	 * hypotehtical ConfigureWorkSpaces() sort of thing...
	 */
	{
		char *wrkSpcList;
		int  len;

		len = GetPropertyFromMask(0xFFFFFFFFu, &wrkSpcList);
		XChangeProperty(dpy, Scr->Root, XA_WM_WORKSPACESLIST, XA_STRING, 8,
		                PropModeReplace, (unsigned char *) wrkSpcList, len);
		free(wrkSpcList);
	}
}


/*
 * Put together the actual window for the workspace manager.  Called as
 * part of CreateWorkSpaceManager() during startup, once per vscreen
 * (since there's a separate window for each).
 */
static void
CreateWorkSpaceManagerWindow(VirtualScreen *vs)
{
	unsigned int width, height;
	TwmWindow *tmp_win;
	int x, y, gravity;
	/* Shortcuts */
	const int vspace = Scr->workSpaceMgr.vspace;
	const int hspace = Scr->workSpaceMgr.hspace;
	const long count = Scr->workSpaceMgr.count;

	/* No workspaces?  Nothing to do. */
	if(count == 0) {
		return;
	}

	/*
	 * Work out grid.  wSM.columns will be filled if specified in
	 * WorkSpaceManageGeometry, or uninitialized (0) if not.
	 */
	{
		int lines, columns;
		columns = Scr->workSpaceMgr.columns;
		if(columns == 0) {
			lines = 2;
			columns = ((count - 1) / lines) + 1;
		}
		else {
			lines = ((count - 1) / columns) + 1;
		}
		Scr->workSpaceMgr.lines   = lines;
		Scr->workSpaceMgr.columns = columns;
	}


	/* Work out dimensions of stuff */
	{
		unsigned int bwidth, bheight;
		unsigned short strWid;
		WorkSpace *ws;
		const char *geometry = Scr->workSpaceMgr.geometry;
		const int lines      = Scr->workSpaceMgr.lines;
		const int columns    = Scr->workSpaceMgr.columns;

		/* Figure longest label */
		strWid = 0;
		for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
			XRectangle inc_rect;
			XRectangle logical_rect;
			unsigned short wid;
			const MyFont font = Scr->workSpaceMgr.buttonFont;

			XmbTextExtents(font.font_set, ws->label, strlen(ws->label),
			               &inc_rect, &logical_rect);
			wid = logical_rect.width;
			if(wid > strWid) {
				strWid = wid;
			}
		}

		/*
		 * If WorkSpaceManagerGeometry is given, work from that.  Else,
		 * create a workable minimum ourselves.
		 * */
		if(geometry != NULL) {
			int mask;

			/* Base button/subwindow sizes */
			bwidth = strWid + 10;
			bheight = 22;

			/* Adjust to WSMGeometry if specified */
			mask = RLayoutXParseGeometry(Scr->Layout, geometry, &x, &y, &width, &height);
			if(mask & WidthValue) {
				bwidth = (width - (columns * hspace)) / columns;
			}
			if(mask & HeightValue) {
				bheight = (height - (lines * vspace)) / lines;
			}

			/* Size of the whole thing is based off those */
			width  = columns * (bwidth  + hspace);
			height = lines   * (bheight + vspace);

			/*
			 * If no Y given, put it at the bottom of the screen.  If one
			 * is, just accept it.  If it's a negative, we have to figure
			 * out where that actually is on this vscreen.
			 */
			if(!(mask & YValue)) {
				y = 0;
				mask |= YNegative;
			}
			if(mask & YNegative) {
				y += vs->h - height;
			}

			/*
			 * If X is given, tweak as necessary for the vscreen
			 * location.  Otherwise, put it in in something like the
			 * middle.
			 */
			if(mask & XValue) {
				if(mask & XNegative) {
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
		}
		else {
			/* No geom specified, come up with one */
			bwidth  = strWid + 2 * Scr->WMgrButtonShadowDepth + 6;
			bheight = 22;
			width   = columns * (bwidth  + hspace);
			height  = lines   * (bheight + vspace);
			x       = (vs->w - width) / 2;
			y       = vs->h - height;
			gravity = NorthWestGravity;
		}
	}

	/* Set w/h to dummy values; ResizeWorkSpaceManager() writes real ones */
	vs->wsw->width  = 1;
	vs->wsw->height = 1;

	/* Allocate structs for map/button subwindows */
	vs->wsw->bswl = calloc(count, sizeof(ButtonSubwindow *));
	vs->wsw->mswl = calloc(count, sizeof(MapSubwindow *));

	/* Create main window */
	vs->wsw->w = XCreateSimpleWindow(dpy, Scr->Root, x, y, width, height, 0,
	                                 Scr->Black, Scr->workSpaceMgr.cp.back);


	/*
	 * Create the map and button subwindows for each workspace
	 */
	{
		WorkSpace *ws;

		for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
			MapSubwindow *msw;
			ButtonSubwindow *bsw;
			const int Dummy = 1;
			const unsigned long border = Scr->workSpaceMgr.defBorderColor;

			/* Alloc structs */
			vs->wsw->bswl[ws->number] = bsw
			                            = calloc(1, sizeof(ButtonSubwindow));
			vs->wsw->mswl[ws->number] = msw = calloc(1, sizeof(MapSubwindow));

			/*
			 * Create windows for button/map.  ResizeWorkSpaceManager()
			 * sets the real sizes and positions, so we dummy 'em.
			 */
			bsw->w = XCreateSimpleWindow(dpy, vs->wsw->w,
			                             Dummy, Dummy, Dummy, Dummy,
			                             0, Scr->Black, ws->cp.back);

			msw->w = XCreateSimpleWindow(dpy, vs->wsw->w,
			                             Dummy, Dummy, Dummy, Dummy,
			                             1, border, ws->cp.back);

			/* Map whichever is up by default */
			if(vs->wsw->state == WMS_buttons) {
				XMapWindow(dpy, bsw->w);
			}
			else {
				XMapWindow(dpy, msw->w);
			}

			/* Setup background on map-state window */
			/* XXX X-ref CTAG_BGDRAW in CreateWorkSpaceManager() */
			if(useBackgroundInfo) {
				if(ws->image == NULL || Scr->NoImagesInWorkSpaceManager) {
					XSetWindowBackground(dpy, msw->w, ws->backcp.back);
				}
				else {
					XSetWindowBackgroundPixmap(dpy, msw->w, ws->image->pixmap);
				}
			}
			else {
				if(Scr->workSpaceMgr.defImage == NULL || Scr->NoImagesInWorkSpaceManager) {
					XSetWindowBackground(dpy, msw->w, Scr->workSpaceMgr.defColors.back);
				}
				else {
					XSetWindowBackgroundPixmap(dpy, msw->w, Scr->workSpaceMgr.defImage->pixmap);
				}
			}

			/*
			 * Clear out button subwin; PaintWorkSpaceManager() fills it
			 * in.  Is this really necessary?
			 */
			XClearWindow(dpy, bsw->w);
		}
	}


	/* Set WM properties */
	{
		XSizeHints sizehints;
		XWMHints   wmhints;
		const int lines   = Scr->workSpaceMgr.lines;
		const int columns = Scr->workSpaceMgr.columns;
		const char *name      = Scr->workSpaceMgr.name;
		const char *icon_name = Scr->workSpaceMgr.icon_name;

		sizehints.flags       = USPosition | PBaseSize | PMinSize | PResizeInc
		                        | PWinGravity;
		sizehints.x           = x;
		sizehints.y           = y;
		sizehints.base_width  = columns * hspace;
		sizehints.base_height = lines   * vspace;
		sizehints.width_inc   = columns;
		sizehints.height_inc  = lines;
		sizehints.min_width   = columns  * (hspace + 2);
		sizehints.min_height  = lines    * (vspace + 2);
		sizehints.win_gravity = gravity;

		wmhints.flags         = InputHint | StateHint;
		wmhints.input         = True;
		wmhints.initial_state = NormalState;

		XmbSetWMProperties(dpy, vs->wsw->w, name, icon_name, NULL, 0,
		                   &sizehints, &wmhints, NULL);
	}


	/* Create our TwmWindow wrapping around it */
	tmp_win = AddWindow(vs->wsw->w, AWT_WORKSPACE_MANAGER,
	                    Scr->iconmgr, vs);
	if(! tmp_win) {
		fprintf(stderr, "cannot create workspace manager window, exiting...\n");
		exit(1);
	}
	tmp_win->occupation = fullOccupation;
	tmp_win->attr.width = width;
	tmp_win->attr.height = height;
	vs->wsw->twm_win = tmp_win;


	/* Do the figuring to size and internal-layout it */
	ResizeWorkSpaceManager(vs, tmp_win);


	/* Setup cursor/gravity and listen for events */
	{
		XWindowAttributes wattr;
		XSetWindowAttributes attr;
		unsigned long attrmask;

		attr.cursor = Scr->ButtonCursor;
		attr.win_gravity = gravity;
		attrmask = CWCursor | CWWinGravity;
		XChangeWindowAttributes(dpy, vs->wsw->w, attrmask, &attr);

		XGetWindowAttributes(dpy, vs->wsw->w, &wattr);
		attrmask = wattr.your_event_mask | KeyPressMask | KeyReleaseMask
		           | ExposureMask;
		XSelectInput(dpy, vs->wsw->w, attrmask);
	}


	/*
	 * Mark the buttons as listening to click and exposure events, and
	 * stash away some pointers in contexts.  We stash the overall WSM
	 * window in TwmContext, which means that when an event looks up the
	 * window, it finds the WSM rather than the subwindow, and then falls
	 * into the WMgrHandle*Event()'s, which then dig down into the event
	 * to find where it happened in there.
	 *
	 * The map window doesn't listen to expose events; it's just empty
	 * and background colored.  The individual subwindows in the map
	 * listen for exposes for drawing themselves.
	 *
	 * Dragging windows around to move or re-occupy in the map window
	 * does rely on motion events, but we don't listen for them here.
	 * That happens in WMgrHandleButtonEvent() after getting the initial
	 * click.  It changes the listen and runs through the action
	 * internally; those motions never run through our main event loop.
	 */
	for(WorkSpace *ws = Scr->workSpaceMgr.workSpaceList; ws != NULL;
	                ws = ws->next) {
		Window buttonw = vs->wsw->bswl[ws->number]->w;
		Window mapsubw = vs->wsw->mswl[ws->number]->w;

		XSelectInput(dpy, buttonw, ButtonPressMask | ButtonReleaseMask
		             | ExposureMask);
		XSaveContext(dpy, buttonw, TwmContext, (XPointer) tmp_win);
		XSaveContext(dpy, buttonw, ScreenContext, (XPointer) Scr);

		XSelectInput(dpy, mapsubw, ButtonPressMask | ButtonReleaseMask);
		XSaveContext(dpy, mapsubw, TwmContext, (XPointer) tmp_win);
		XSaveContext(dpy, mapsubw, ScreenContext, (XPointer) Scr);
	}


	/* Set WM_STATE prop */
	SetMapStateProp(tmp_win, WithdrawnState);


	/* Setup root window if necessary */
	/* XXX X-ref CTAG_BGDRAW in CreateWorkSpaceManager() */
	if(useBackgroundInfo && ! Scr->DontPaintRootWindow) {
		WorkSpace *ws = Scr->workSpaceMgr.workSpaceList;
		if(ws->image == NULL) {
			XSetWindowBackground(dpy, Scr->Root, ws->backcp.back);
		}
		else {
			XSetWindowBackgroundPixmap(dpy, Scr->Root, ws->image->pixmap);
		}
		XClearWindow(dpy, Scr->Root);
	}


	/*
	 * Don't have to PaintWorkSpaceManager(vs) here, because
	 * ResizeWorkSpaceManager() already called it for us.
	 */
}


/*
 * Size and layout a WSM.  Mostly an internal bit in the process of
 * setting it up.
 */
static void
ResizeWorkSpaceManager(VirtualScreen *vs, TwmWindow *win)
{
	WorkSpace *ws;
	int       i, j;
	/* Lots of shortcuts to ease reading */
	const int neww    = win->attr.width;
	const int newh    = win->attr.height;
	const int hspace  = Scr->workSpaceMgr.hspace;
	const int vspace  = Scr->workSpaceMgr.vspace;
	const int lines   = Scr->workSpaceMgr.lines;
	const int columns = Scr->workSpaceMgr.columns;
	const int bwidth  = (neww - (columns * hspace)) / columns;
	const int bheight = (newh - (lines   * vspace)) / lines;
	const int wwidth  = neww / columns;
	const int wheight = newh / lines;
	const float wf    = (float)(wwidth  - 2) / (float) vs->w;
	const float hf    = (float)(wheight - 2) / (float) vs->h;

	/* If nothing's changed since our last run, there's nothing to change */
	if(neww == vs->wsw->width && newh == vs->wsw->height) {
		return;
	}

	/* Set the new overall vals */
	vs->wsw->bwidth  = bwidth;
	vs->wsw->bheight = bheight;
	vs->wsw->width   = neww;
	vs->wsw->height  = newh;
	vs->wsw->wwidth  = wwidth;
	vs->wsw->wheight = wheight;

	/* Iterate over the WS's */
	i = 0;
	j = 0;
	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		MapSubwindow    *msw = vs->wsw->mswl[ws->number];
		ButtonSubwindow *bsw = vs->wsw->bswl[ws->number];

		/* Move button window to its place in the grid and size appropriately */
		XMoveResizeWindow(dpy, bsw->w,
		                  i * (bwidth  + hspace) + (hspace / 2),
		                  j * (bheight + vspace) + (vspace / 2),
		                  bwidth, bheight);

		/* Move the map window as well */
		msw->x = i * wwidth;
		msw->y = j * wheight;
		XMoveResizeWindow(dpy, msw->w, msw->x, msw->y, wwidth - 2, wheight - 2);

		/*
		 * Redo interior sizing and placement of all the windows in the
		 * WS in the map window
		 */
		for(WinList *wl = msw->wl; wl != NULL; wl = wl->next) {
			TwmWindow *tmp_win = wl->twm_win;
			wl->x      = (int)(tmp_win->frame_x * wf);
			wl->y      = (int)(tmp_win->frame_y * hf);
			wl->width  = (unsigned int)((tmp_win->frame_width  * wf) + 0.5);
			wl->height = (unsigned int)((tmp_win->frame_height * hf) + 0.5);
			XMoveResizeWindow(dpy, wl->w, wl->x, wl->y, wl->width, wl->height);
		}

		/* And around to the next WS */
		i++;
		if(i == columns) {
			i = 0;
			j++;
		};
	}


	/* Draw it */
	PaintWorkSpaceManager(vs);
}


/*
 * Draw up the button-state pieces of a WSM window.
 *
 * Note: this is currently stubbed out and does nothing.  Historically
 * it's been called during startup when the WSM window is put together,
 * and when the screen is unmasked.  However, the only apparent result is
 * that the border and buttons get drawn a little earlier; they already
 * get expose events that get picked up when we start the event loop.
 *
 * If we don't find any reason to reinstate it, we should remove this in
 * the future.
 */
void
PaintWorkSpaceManager(VirtualScreen *vs)
{
	WorkSpace *ws;

	/* x-ref header comment */
	return;

	PaintWorkSpaceManagerBorder(vs);

	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		Window buttonw = vs->wsw->bswl[ws->number]->w;
		ButtonState bs = (ws == vs->wsw->currentwspc) ? on : off;

		PaintWsButton(WSPCWINDOW, vs, buttonw, ws->label, ws->cp, bs);
	}
}


/*
 * Border around the WSM
 */
static void
PaintWorkSpaceManagerBorder(VirtualScreen *vs)
{
	int width, height;

	width  = vs->wsw->width;
	height = vs->wsw->height;
	Draw3DBorder(vs->wsw->w, 0, 0, width, height, 2, Scr->workSpaceMgr.cp, off,
	             true, false);
}


/*
 * Draw a workspace manager window on expose.  X-ref comment on
 * PaintWorkSpaceManager().
 */
void
WMgrHandleExposeEvent(VirtualScreen *vs, XEvent *event)
{
	if(vs->wsw->state == WMS_buttons) {
		Window buttonw;
		WorkSpace *ws;

		/* Find the button we're exposing */
		for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
			buttonw = vs->wsw->bswl[ws->number]->w;
			if(event->xexpose.window == buttonw) {
				break;
			}
		}

		/* If none, just paint the border.  Else paint the button. */
		if(ws == NULL) {
			PaintWorkSpaceManagerBorder(vs);
		}
		else {
			ButtonState bs = (ws == vs->wsw->currentwspc) ? on : off;
			PaintWsButton(WSPCWINDOW, vs, buttonw, ws->label, ws->cp, bs);
		}
	}
	else {
		WinList *wl;

		/*
		 * This is presumably exposing some individual window in the WS
		 * subwindow; find it from the stashed context on the window, and
		 * redraw it.
		 */
		if(XFindContext(dpy, event->xexpose.window, MapWListContext,
		                (XPointer *) &wl) == XCNOENT) {
			return;
		}
		if(wl && wl->twm_win && wl->twm_win->mapped) {
			WMapRedrawName(vs, wl);
		}
	}
}



/*
 * Moving the WSM between button and map state
 */
void
WMgrToggleState(VirtualScreen *vs)
{
	if(vs->wsw->state == WMS_buttons) {
		WMgrSetMapState(vs);
	}
	else {
		WMgrSetButtonsState(vs);
	}
}

void
WMgrSetMapState(VirtualScreen *vs)
{
	WorkSpace *ws;

	if(vs->wsw->state == WMS_map) {
		return;
	}
	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		XUnmapWindow(dpy, vs->wsw->bswl [ws->number]->w);
		XMapWindow(dpy, vs->wsw->mswl [ws->number]->w);
	}
	vs->wsw->state = WMS_map;
	MaybeAnimate = true;
}

void
WMgrSetButtonsState(VirtualScreen *vs)
{
	WorkSpace *ws;

	if(vs->wsw->state == WMS_buttons) {
		return;
	}
	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		XUnmapWindow(dpy, vs->wsw->mswl [ws->number]->w);
		XMapWindow(dpy, vs->wsw->bswl [ws->number]->w);
	}
	vs->wsw->state = WMS_buttons;
}




/*
 ****************************************************************
 *
 * Handlers for mouse/key actions in the WSM
 *
 ****************************************************************
 */

/*
 * Key press/release events in the WSM.  A major use (and only for
 * release) is the Ctrl-key switching between map and button state.  The
 * other use is on-the-fly renaming of workspaces by typing in the
 * button-state WSM.
 */
void
WMgrHandleKeyReleaseEvent(VirtualScreen *vs, XEvent *event)
{
	KeySym keysym;

	keysym = XLookupKeysym((XKeyEvent *) event, 0);
	if(! keysym) {
		return;
	}
	if(keysym == XK_Control_L || keysym == XK_Control_R) {
		/* DontToggleWorkSpaceManagerState added 20040607 by dl*/
		if(!Scr->DontToggleWorkspaceManagerState) {
			WMgrToggleState(vs);
		}
		return;
	}
}

void
WMgrHandleKeyPressEvent(VirtualScreen *vs, XEvent *event)
{
	WorkSpace *ws;

	/* Check if we're using Control to toggle the state */
	{
		KeySym keysym = XLookupKeysym((XKeyEvent *) event, 0);
		if(! keysym) {
			return;
		}
		if(keysym == XK_Control_L || keysym == XK_Control_R) {
			/* DontToggleWorkSpaceManagerState added 20040607 by dl*/
			if(!Scr->DontToggleWorkspaceManagerState) {
				WMgrToggleState(vs);
			}
			return;
		}
	}

	/* Otherwise, key presses do nothing in map state */
	if(vs->wsw->state == WMS_map) {
		return;
	}

	/*
	 * If we're typing in a button-state WSM, and the mouse is on one of
	 * the buttons, that means we're changing the name, so do that dance.
	 */
	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		if(vs->wsw->bswl[ws->number]->w == event->xkey.subwindow) {
			break;
		}
	}
	if(ws == NULL) {
		/* Not on a button, nothing to do */
		return;
	}


	/*
	 * Edit the label.
	 */
	{
		int    nkeys;
		char   keys[16];
		size_t nlen;
		char   *newname;

		/* Look up what keystrokes are queued.  Arbitrary buf size */
		nkeys = XLookupString(&(event->xkey), keys, 16, NULL, NULL);

		/* Label length can't grow to more than cur+nkeys */
		nlen = strlen(ws->label);
		newname = malloc(nlen + nkeys + 1);
		strcpy(newname, ws->label);

		/* Iterate over the passed keystrokes */
		for(int i = 0 ; i < nkeys ; i++) {
			unsigned char k = keys[i];

			if(isprint(k)) {
				/* Printable chars append to the string */
				newname[nlen++] = k;
			}
			else if((k == 127) || (k == 8)) {
				/*
				 * DEL or BS back up a char.
				 *
				 * XXX Would it be more generally correct to do this via
				 * keysyms, in the face of changed keyboard mappings or
				 * significantly differing locales?
				 */
				if(nlen != 0) {
					nlen--;
				}
			}
			else {
				/* Any other char stops the process dead */
				break;
			}
		}
		/* Now ends where it ends */
		newname[nlen] = '\0';

		/* Swap it in */
		free(ws->label);
		ws->label = newname;
	}


	/* Redraw the button with the new label */
	{
		ButtonState bs = (ws == vs->wsw->currentwspc) ? on : off;

		PaintWsButton(WSPCWINDOW, vs, vs->wsw->bswl[ws->number]->w, ws->label,
		              ws->cp, bs);
	}
}


/*
 * Mouse clicking in WSM.  Gets called on button press (not release).  In
 * the simple case, that's just switching workspaces.  In the more
 * complex, it's changing window occupation in various different ways, or
 * even moving windows.  When that's happening, it internally hijacks
 * button/motion/exposure events and implements them for the moving, with
 * magic escapes if it gets them for something else.  Ew.
 */
void
WMgrHandleButtonEvent(VirtualScreen *vs, XEvent *event)
{
	WorkSpace    *oldws, *newws;
	WinList      *wl;
	TwmWindow    *win;
	unsigned int W0, H0;
	XEvent       lastev;
	Window       w = 0;
	Position     newX = 0, newY = 0, winX = 0, winY = 0;
	bool         alreadyvivible, realmovemode;
	const WorkSpaceWindow *mw = vs->wsw;

	/* Shortcuts into the event */
	const Window parent = event->xbutton.window;    // Map/button for WS
	const Window sw     = event->xbutton.subwindow; // Map mini-win
	const Time etime    = event->xbutton.time;
	const unsigned int button   = event->xbutton.button;
	const unsigned int modifier = event->xbutton.state;


	/* If we're in button state, we're just clicking to change */
	if(vs->wsw->state == WMS_buttons) {
		WorkSpace *ws;
		for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
			if(vs->wsw->bswl[ws->number]->w == parent) {
				GotoWorkSpace(vs, ws);
				break;
			}
		}
		return;
	}

	/*
	 * If we get this far, we're in map state, where things are more
	 * complicated.  A simple click/release means change here too, but
	 * there's also the possibility of dragging a subwindow around to
	 * change its window's occupation.
	 */

	/* Find what workspace we're clicking in */
	for(oldws = Scr->workSpaceMgr.workSpaceList ; oldws != NULL ;
	                oldws = oldws->next) {
		if(vs->wsw->mswl[oldws->number]->w == parent) {
			break;
		}
	}
	if(oldws == NULL) {
		/* None?  We're done here. */
		return;
	}

	/*
	 * If clicked in the workspace but outside a window, we can only be
	 * switching workspaces.  So just do that, and we're done.
	 */
	if(sw == (Window) 0) {
		GotoWorkSpace(vs, oldws);
		return;
	}

	/* Use the context to find the winlist entry for this window */
	if(XFindContext(dpy, sw, MapWListContext, (XPointer *) &wl) == XCNOENT) {
		return;
	}
	win = wl->twm_win;

	/*
	 * Sometimes we skip transients, so do so.  XXX Should this
	 * GotoWorkSpace()?
	 */
	if((! Scr->TransientHasOccupation) && win->istransient) {
		return;
	}

	/*
	 * Are we trying to actually move the window, by moving its avatar in
	 * the WSM?  If ReallyMoveInWorkspaceManager is set, we're moving on
	 * click, but not in shift-click.  If it's not, it's the reverse.
	 *
	 * XXX This interacts really oddly and badly when you're also moving
	 * the window from WS to WS.
	 */
	realmovemode = false;
	if(Scr->ReallyMoveInWorkspaceManager) {
		if(!(modifier & ShiftMask)) {
			realmovemode = true;
		}
	}
	else if(modifier & ShiftMask) {
		realmovemode = true;
	}

	/*
	 * Frob screen-wide OpaqueMove as necessary for this window's
	 * details.
	 * XXX Really?
	 */
	if(win->OpaqueMove) {
		if(Scr->OpaqueMoveThreshold >= 200) {
			Scr->OpaqueMove = true;
		}
		else {
			const unsigned long winsz = win->frame_width * win->frame_height;
			const unsigned long scrsz = vs->w * vs->h;
			const float sf = Scr->OpaqueMoveThreshold / 100.0;
			if(winsz > (scrsz * sf)) {
				Scr->OpaqueMove = false;
			}
			else {
				Scr->OpaqueMove = true;
			}
		}
	}
	else {
		Scr->OpaqueMove = false;
	}

	/*
	 * Buttons inside the workspace manager, when clicking on the
	 * representation of a window:
	 * 1: drag window to a different workspace
	 * 2: drag a copy of the window to a different workspace
	 *    (show it in both workspaces)
	 * 3: remove the window from this workspace (if it isn't the last).
	 *
	 * XXX If you move between workspaces while also doing realmovemode,
	 * really messed up things happen.
	 */
	switch(button) {
		case 1 : {
			/*
			 * Moving from one to another; get rid of the old location,
			 * then fall through to the "duplicating" case below.
			 */
			XUnmapWindow(dpy, sw);
			/* FALLTHRU */
		}

		case 2 : {
			/*
			 * Duplicating window to another WS.  We create a copy of the
			 * window, and start moving that.  The 'moving' case above
			 * falls through to us after unmapping that old window,
			 * leaving just our copy, which is good enough.  This is
			 * really just setting up the visual stuff for the move; the
			 * actual occupation changes etc. come at the end of the
			 * motion, much lower down.
			 */
			int X0, Y0, X1, Y1;
			unsigned int bw;
			XSetWindowAttributes attrs;
			Window junkW;

			/* [XYWH]0 = size/location of the avatar in the map */
			XGetGeometry(dpy, sw, &junkW, &X0, &Y0, &W0, &H0, &bw, &JunkDepth);

			/*
			 * [XY]0 are the coordinates of the avatar subwindow inside
			 * the individual workspace window in the map.  Turn those
			 * into [XY]1 as the coordinates of it relative to the whole
			 * WSM window.
			 */
			XTranslateCoordinates(dpy, vs->wsw->mswl[oldws->number]->w,
			                      mw->w, X0, Y0, &X1, &Y1, &junkW);

			/*
			 * Create the copy window to drag around, as a child of the
			 * whole WSM (so we can move it between workspaces), and map
			 * it.
			 */
			attrs.event_mask       = ExposureMask;
			attrs.background_pixel = wl->cp.back;
			attrs.border_pixel     = wl->cp.back;
			w = XCreateWindow(dpy, mw->w, X1, Y1, W0, H0, bw,
			                  CopyFromParent, CopyFromParent, CopyFromParent,
			                  CWEventMask | CWBackPixel | CWBorderPixel,
			                  &attrs);
			XMapRaised(dpy, w);

			/* Do our dance on it to draw the name/color/etc */
			WMapRedrawWindow(w, W0, H0, wl->cp, wl->twm_win->icon_name);

			/*
			 * If we're moving the real window and
			 * AlwaysShowWindowWhenMovingFromWorkspaceManager is set in
			 * config, we need to be sure the real window is visible
			 * while we move it.
			 */
			if(realmovemode && Scr->ShowWinWhenMovingInWmgr) {
				if(Scr->OpaqueMove) {
					DisplayWin(vs, win);
				}
				else {
					MoveOutline(Scr->Root,
					            win->frame_x - win->frame_bw,
					            win->frame_y - win->frame_bw,
					            win->frame_width  + 2 * win->frame_bw,
					            win->frame_height + 2 * win->frame_bw,
					            win->frame_bw,
					            win->title_height + win->frame_bw3D);
				}
			}

			/* Move onward */
			break;
		}

		/*
		 * For the button 3 or anything else case, there's no dragging or
		 * anything, so they do their thing and just immediately return.
		 */
		case 3 : {
			int newocc = win->occupation & ~(1 << oldws->number);
			if(newocc != 0) {
				ChangeOccupation(win, newocc);
			}
			return;
		}

		default :
			return;
	}

	/*
	 * Keep dragging the representation of the window
	 *
	 * XXX Look back at this and see if we can move it to an inner
	 * function for readability...
	 */
	{
		const float wf = (float)(mw->wwidth  - 1) / (float) vs->w;
		const float hf = (float)(mw->wheight - 1) / (float) vs->h;
		int XW, YW;
		bool cont;
		Window junkW;

		/* Figure where in the subwindow the click was, and stash in XW/YW */
		XTranslateCoordinates(dpy, Scr->Root, sw, event->xbutton.x_root,
		                      event->xbutton.y_root,
		                      &XW, &YW, &junkW);

		/*
		 * Grab the pointer, lock it into the WSM, and get the events
		 * related to buttons and movement.  We don't need
		 * PointerMotionMask, since that would only happen after buttons
		 * are released, and we'll be done by then.
		 */
		XGrabPointer(dpy, mw->w, False,
		             ButtonPressMask | ButtonMotionMask | ButtonReleaseMask,
		             GrabModeAsync, GrabModeAsync, mw->w, Scr->MoveCursor,
		             CurrentTime);

		/* Start handling events until we're done */
		alreadyvivible = false;
		cont = true;
		while(cont) {
			XEvent ev;

			/* Grab the next event and handle */
			XMaskEvent(dpy, ButtonPressMask | ButtonMotionMask |
			           ButtonReleaseMask | ExposureMask, &ev);
			switch(ev.xany.type) {
				case Expose : {
					/* Something got exposed */
					if(ev.xexpose.window == w) {
						/*
						 * The win we're working with?  We know how to do
						 * that.
						 */
						WMapRedrawWindow(w, W0, H0, wl->cp,
						                 wl->twm_win->icon_name);
						break;
					}

					/* Else, delegate to our global dispatcher */
					Event = ev;
					DispatchEvent();
					break;
				}

				case ButtonPress :
				case ButtonRelease : {
					/*
					 * Events for buttons other than the one whose press
					 * started this activity are totally ignored.
					 */
					if(ev.xbutton.button != button) {
						break;
					}

					/*
					 * Otherwise, this is a press/release of the button
					 * that started things.  Though I'm not sure how it
					 * could be a press, since it was already pressed.
					 * Regardless, this is our exit condition.  Fall
					 * through into the Motion case to handle any
					 * remaining movement.
					 */
					lastev = ev;
					cont = false;
					newX = ev.xbutton.x;
					newY = ev.xbutton.y;
				}

				/* Everything remaining is motion handling */
				case MotionNotify : {
					/* If we fell through from above, no new movement */
					if(cont) {
						newX = ev.xmotion.x;
						newY = ev.xmotion.y;
					}

					/* Lots to do if we're moving the window for real */
					if(realmovemode) {
						int XSW, YSW;
						WorkSpace *cws;
						MapSubwindow *msw;

						/* Did the move start in the currently visible WS? */
						bool startincurrent = (oldws == vs->wsw->currentwspc);

						/* Find the workspace we wound up in */
						for(cws = Scr->workSpaceMgr.workSpaceList ;
						                cws != NULL ;
						                cws = cws->next) {
							msw = vs->wsw->mswl [cws->number];
							if((newX >= msw->x)
							                && (newX <  msw->x + mw->wwidth)
							                && (newY >= msw->y)
							                && (newY <  msw->y + mw->wheight)) {
								break;
							}
						}
						if(!cws) {
							/* None?  Ignore. */
							break;
						}

						/*
						 * Mouse is wherever it started inside the
						 * subwindow, so figure the X/Y of the top left
						 * of the subwindow from there.  (coords relative
						 * to the whole WSM because of grab)
						 */
						winX = newX - XW;
						winY = newY - YW;

						/* XXX redundant w/previous? */
						msw = vs->wsw->mswl[cws->number];

						/*
						 * Figure where those coords are relative to the
						 * per-workspace window.
						 */
						XTranslateCoordinates(dpy, mw->w, msw->w,
						                      winX, winY, &XSW, &YSW, &junkW);

						/*
						 * Now rework the win[XY] to be the coordinates
						 * the window would be in the whole screen, based
						 * on the [XY]SW inside the map, using our scale
						 * factors.
						 */
						winX = (int)(XSW / wf);
						winY = (int)(YSW / hf);

						/*
						 * Clip to the screen if DontMoveOff is set.
						 *
						 * XXX Can we use the Constrain*() functions for
						 * this, instead of implementing icky magic
						 * internally?
						 */
						if(Scr->DontMoveOff) {
							const int width = win->frame_width;
							const int height = win->frame_height;

							if((winX < Scr->BorderLeft) && ((Scr->MoveOffResistance < 0) ||
							                                (winX > Scr->BorderLeft - Scr->MoveOffResistance))) {
								winX = Scr->BorderLeft;
								newX = msw->x + XW + Scr->BorderLeft * mw->wwidth / vs->w;
							}
							if(((winX + width) > vs->w - Scr->BorderRight) &&
							                ((Scr->MoveOffResistance < 0) ||
							                 ((winX + width) < vs->w - Scr->BorderRight + Scr->MoveOffResistance))) {
								winX = vs->w - Scr->BorderRight - width;
								newX = msw->x + mw->wwidth *
								       (1 - Scr->BorderRight / (double) vs->w) - wl->width + XW - 2;
							}
							if((winY < Scr->BorderTop) && ((Scr->MoveOffResistance < 0) ||
							                               (winY > Scr->BorderTop - Scr->MoveOffResistance))) {
								winY = Scr->BorderTop;
								newY = msw->y + YW + Scr->BorderTop * mw->height / vs->h;
							}
							if(((winY + height) > vs->h - Scr->BorderBottom) &&
							                ((Scr->MoveOffResistance < 0) ||
							                 ((winY + height) < vs->h - Scr->BorderBottom + Scr->MoveOffResistance))) {
								winY = vs->h - Scr->BorderBottom - height;
								newY = msw->y + mw->wheight
								       * (1 - Scr->BorderBottom / (double) vs->h)
								       * - wl->height + YW - 2;
							}
						}


						/* Setup the avatar for the new position of win */
						WMapSetupWindow(win, winX, winY, -1, -1);

						/*
						 * If SWWMIW, we already made sure above the
						 * window is always visible on the screen.  So we
						 * don't need to do any of the following steps to
						 * maybe show it or maybe un-show it, since we
						 * know it's already always shown.
						 */
						if(Scr->ShowWinWhenMovingInWmgr) {
							goto movewin;
						}

						/*
						 * If we wound up in the same workspace as is
						 * being displayed on the screen, we need to make
						 * sure that window is showing up on the screen
						 * as a "about to be occupied here if you release
						 * the button" indication.
						 */
						if(cws == vs->wsw->currentwspc) {
							if(alreadyvivible) {
								/* Unless it's already there */
								goto movewin;
							}
							if(Scr->OpaqueMove) {
								XMoveWindow(dpy, win->frame, winX, winY);
								DisplayWin(vs, win);
							}
							else {
								MoveOutline(Scr->Root,
								            winX - win->frame_bw,
								            winY - win->frame_bw,
								            win->frame_width  + 2 * win->frame_bw,
								            win->frame_height + 2 * win->frame_bw,
								            win->frame_bw,
								            win->title_height + win->frame_bw3D);
							}
							alreadyvivible = true;

							/*
							 * We already moved it, skip past the other
							 * code trying to move it in other cases.
							 */
							goto move;
						}

						/*
						 * If it's for whatever reason not being shown on
						 * the screen, then we don't need to move it; hop
						 * straight to moving the avatar.
						 */
						if(!alreadyvivible) {
							goto move;
						}

						/*
						 * So it _is_ being shown.  If it's not supposed
						 * to be here, hide it away (don't need to worry
						 * about AlwaysShow, it would have skipped past
						 * us from ab0ve).  Also, if we started moving
						 * the window out of the visible workspace with
						 * button 1, it's gonna not be here if you
						 * release, so hide it away.
						 */
						if(!OCCUPY(win, vs->wsw->currentwspc) ||
						                (startincurrent && (button == 1))) {
							if(Scr->OpaqueMove) {
								Vanish(vs, win);
								XMoveWindow(dpy, win->frame, winX, winY);
							}
							else {
								MoveOutline(Scr->Root, 0, 0, 0, 0, 0, 0);
							}
							alreadyvivible = false;
							goto move;
						}

movewin:
						/*
						 * However we get here, the real window is
						 * visible and needs to be moved.  So move it.
						 */
						if(Scr->OpaqueMove) {
							XMoveWindow(dpy, win->frame, winX, winY);
						}
						else {
							MoveOutline(Scr->Root,
							            winX - win->frame_bw,
							            winY - win->frame_bw,
							            win->frame_width  + 2 * win->frame_bw,
							            win->frame_height + 2 * win->frame_bw,
							            win->frame_bw,
							            win->title_height + win->frame_bw3D);
						}
					}

move:
					/*
					 * Just move the subwindow in the map to the new
					 * location.
					 */
					XMoveWindow(dpy, w, newX - XW, newY - YW);

					/* And we're done.  Next event! */
					break;
				}
			}
		}
	}

	/*
	 * Finished with dragging (button released).
	 */
	if(realmovemode) {
		/* Moving the real window?  Move the real window. */
		if(Scr->ShowWinWhenMovingInWmgr || alreadyvivible) {
			if(Scr->OpaqueMove && !OCCUPY(win, vs->wsw->currentwspc)) {
				Vanish(vs, win);
			}
			if(!Scr->OpaqueMove) {
				MoveOutline(Scr->Root, 0, 0, 0, 0, 0, 0);
				WMapRedrawName(vs, wl);
			}
		}
		SetupWindow(win, winX, winY, win->frame_width, win->frame_height, -1);
	}

	/*
	 * Last event that caused us to escape is other code's
	 * responsibility, put it back in the queue.
	 */
	lastev.xbutton.subwindow = (Window) 0;
	lastev.xbutton.window = parent;
	XPutBackEvent(dpy, &lastev);

	/* End our grab */
	XUngrabPointer(dpy, CurrentTime);

	/*
	 * If you wanted to change workspaces, and clicked _outside_ a
	 * window, it would have just switched way up near the top of the
	 * function.  But if you clicked _in_ a window [in the WSM map], it
	 * would have to go through the whole fun to find out whether you
	 * wanted to move/reoccupy the window, or were just wanting to change
	 * WSen.
	 *
	 * So if it's been <250ms (completely arbitrary and non-configgable,
	 * probably should be rethought) since you started, assume you just
	 * wanted to switch workspaces.  Don't do any occupation change, And
	 * just switch.  Also do some magic related to the map/button
	 * toggling.
	 *
	 * XXX This still leaves any window _moves_ done.  It seems like it
	 * should probably revert those too?
	 */
	if((lastev.xbutton.time - etime) < 250) {
		KeyCode control_L_code, control_R_code;
		char keys [32];

		/* Re-show old miniwindow, destroy the temp, and warp to WS */
		XMapWindow(dpy, sw);
		XDestroyWindow(dpy, w);
		GotoWorkSpace(vs, oldws);
		if(!Scr->DontWarpCursorInWMap) {
			WarpToWindow(win, Scr->RaiseOnWarp);
		}

		/*
		 * The control keys toggle between map and button state.  If we
		 * did a short move, and ctrl is being held at the end, flip the
		 * state.  This has several possible causes and effects.
		 *
		 * One is that _during_ a move, we don't do look through KeyPress
		 * events, so we wouldn't see it happen yet.  But after we're
		 * done and return back into the event loop, that press will come
		 * in and cause the state to change.  It may be weird to the user
		 * to see that happen, not when they hit ctrl, but _later_, after
		 * they release the mouse button.  The "best" solution may be to
		 * find that press in the event queue and empty it out, but a
		 * cheap solution is just to pre-flip it and then let the event
		 * code flip it back.  Flickery maybe, but easy.  Now, _should_ we be
		 * doing that?  I'm a little doubtful...
		 *
		 * A second is that if the WSM is "naturally" in button mode, and
		 * you temporarily ctrl-flip it into map mode and then click in a
		 * window.  This code will cause it to automatically flip back
		 * after you release the mouse if you haven't released ctrl yet.
		 * This is apparently needed because, unless you have
		 * DontWarpCursorInWMap set, the previous few lines of code would
		 * have shifted the cursor to the window you clicked, which means
		 * you don't get a chance to release it in the WSM and flip the
		 * state back.  It seems a reasonable assumption that the user
		 * wanted a temporary change of state just for the purposes of
		 * the change.
		 *
		 * (if you had released the ctrl before the mouse, the release
		 * event would already be queued up up on the WSM, so would flip
		 * it back after we return)
		 *
		 * XXX Should we be checking DontToggleWorkSpaceManagerState
		 * here?
		 */
		control_L_code = XKeysymToKeycode(dpy, XStringToKeysym("Control_L"));
		control_R_code = XKeysymToKeycode(dpy, XStringToKeysym("Control_R"));
		XQueryKeymap(dpy, keys);
		if((keys [control_L_code / 8] & ((char) 0x80 >> (control_L_code % 8))) ||
		                keys [control_R_code / 8] & ((char) 0x80 >> (control_R_code % 8))) {
			WMgrToggleState(vs);
		}
		return;
	}


	/* Find the workspace we finally found up in */
	for(newws = Scr->workSpaceMgr.workSpaceList ; newws != NULL ;
	                newws = newws->next) {
		MapSubwindow *msw = vs->wsw->mswl[newws->number];
		if((newX >= msw->x) && (newX < msw->x + mw->wwidth) &&
		                (newY >= msw->y) && (newY < msw->y + mw->wheight)) {
			break;
		}
	}

	/* And finish off whatever we're supposed to be doing */
	switch(button) {
		case 1 : { /* moving to another workspace */
			int occupation;

			/* If nothing to change, re-map avatar and we're done */
			if((newws == NULL) || (newws == oldws) ||
			                OCCUPY(wl->twm_win, newws)) {
				XMapWindow(dpy, sw);
				break;
			}

			/* Out of the old, into the new */
			occupation = win->occupation;
			occupation |= (1 << newws->number);
			occupation &= ~(1 << oldws->number);
			ChangeOccupation(win, occupation);

			/*
			 * Raise it to the top if it's in our current ws, and
			 * properly slot it into the WSM map stack if not.
			 */
			if(newws == vs->wsw->currentwspc) {
				OtpRaise(win, WinWin);
				WMapRaise(win);
			}
			else {
				WMapRestack(newws);
			}

			break;
		}

		case 2 : { /* putting in extra workspace */
			int occupation;

			/* Nothing to do if it's going nowhere or places it already is */
			if((newws == NULL) || (newws == oldws) ||
			                OCCUPY(wl->twm_win, newws)) {
				break;
			}

			/* Move */
			occupation = win->occupation | (1 << newws->number);
			ChangeOccupation(win, occupation);

			/* Raise/stack */
			if(newws == vs->wsw->currentwspc) {
				OtpRaise(win, WinWin);
				WMapRaise(win);
			}
			else {
				WMapRestack(newws);
			}
			break;
		}

		/*
		 * Should actually never hit this state; only 1/2 would have
		 * gotten this far into the code...
		 */
		default :
			return;
	}

	/* Clean up our temporary moving-around-in the WSM window */
	XDestroyWindow(dpy, w);
}




/*
 ****************************************************************
 *
 * Functions for doing things with subwindows in the WSM in map state
 *
 ****************************************************************
 */

/*
 * Backend for mapping up windows in the WSM map.
 */
static void
wmap_mapwin_backend(TwmWindow *win, bool handleraise)
{
	VirtualScreen *vs;
	WorkSpace *ws;
	WinList *wl;

	for(vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
		for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
			for(wl = vs->wsw->mswl[ws->number]->wl; wl != NULL; wl = wl->next) {
				if(win == wl->twm_win) {
					/*
					 * When called via deiconify, we might have to do
					 * stuff related to auto-raising the window while we
					 * de-iconify.  When called via a map request, the
					 * window is always wherever it previously was in the
					 * stack.
					 */
					if(!handleraise || Scr->NoRaiseDeicon) {
						XMapWindow(dpy, wl->w);
					}
					else {
						XMapRaised(dpy, wl->w);
					}
					WMapRedrawName(vs, wl);
					break;
				}
			}
		}
	}
}

/*
 * Map up a window's subwindow in the map-mode WSM.  Happens as a result
 * of getting (or faking) a Map request event.  Notably, _not_ in the
 * process of de-iconifying a window; mostly as a result of _creating_
 * windows, or when a client maps itself without a request from us.
 *
 * x-ref comment on WMapDeIconify() for some subtle distinctions between
 * the two...
 */
void
WMapMapWindow(TwmWindow *win)
{
	/* We don't do raise handling */
	wmap_mapwin_backend(win, false);
}


/*
 * De-iconify a window in the WSM map.  The opposite of WMapIconify(),
 * and different from WMapMapWindow() in complicated ways.  This function
 * winds up getting called when a window is de-iconified via a ctwm
 * function.
 */
void
WMapDeIconify(TwmWindow *win)
{
	/* If it's not showing anywhere, nothing to do.  Is this possible? */
	if(!win->vs) {
		return;
	}

	/* Loop and map, handling raises if necessary */
	wmap_mapwin_backend(win, true);
}


/*
 * Hide away a window in the WSM map.  Happens when win is iconified;
 * different from destruction.
 */
void
WMapIconify(TwmWindow *win)
{
	VirtualScreen *vs;
	WorkSpace *ws;
	WinList *wl;

	if(!win->vs) {
		return;
	}

	for(vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
		for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
			for(wl = vs->wsw->mswl[ws->number]->wl; wl != NULL; wl = wl->next) {
				if(win == wl->twm_win) {
					XUnmapWindow(dpy, wl->w);
					break;
				}
			}
		}
	}
}


/*
 * Position a window in the WSM.  Happens as a result of moving things.
 */
void
WMapSetupWindow(TwmWindow *win, int x, int y, int w, int h)
{
	VirtualScreen *vs;

	/* If it's an icon manager, or not on a vscreen, nothing to do */
	if(win->isiconmgr || !win->vs) {
		return;
	}

	/* If it's a WSM, we may be resetting size/position, but that's it */
	if(win->iswspmgr) {
		if(w == -1) {
			return;
		}
		ResizeWorkSpaceManager(win->vs, win);
		return;
	}

	/* If it's an occupy window, ditto */
	if(win == Scr->workSpaceMgr.occupyWindow->twm_win) {
		if(w == -1) {
			return;
		}
		ResizeOccupyWindow(win);
		return;
	}


	/* For anything else, we're potentially showing something */
	for(vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
		WorkSpaceWindow *wsw = vs->wsw;
		WorkSpace *ws;

		/* Scale factors for windows in the map */
		float wf = (float)(wsw->wwidth  - 2) / (float) vs->w;
		float hf = (float)(wsw->wheight - 2) / (float) vs->h;

		/*
		 * Loop around windows in each WS to find all the places the
		 * requested window should show up.
		 */
		for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
			MapSubwindow *msw = wsw->mswl[ws->number];

			for(WinList *wl = msw->wl; wl != NULL; wl = wl->next) {
				if(win == wl->twm_win) {
					/* New positions */
					wl->x = (int)(x * wf);
					wl->y = (int)(y * hf);

					/* Rescale if necessary and move */
					if(w == -1) {
						XMoveWindow(dpy, wl->w, wl->x, wl->y);
					}
					else {
						wl->width  = (unsigned int)((w * wf) + 0.5);
						wl->height = (unsigned int)((h * hf) + 0.5);
						if(!Scr->use3Dwmap) {
							wl->width  -= 2;
							wl->height -= 2;
						}
						if(wl->width < 1) {
							wl->width = 1;
						}
						if(wl->height < 1) {
							wl->height = 1;
						}
						XMoveResizeWindow(dpy, wl->w, wl->x, wl->y,
						                  wl->width, wl->height);
					}
					break;
				}
			}
		}
	}
}


/*
 * Frontends for changing the stacking of windows in the WSM.
 *
 * Strictly speaker, we have no ability to raise or lower a window in the
 * map; we only draw the whole stack.  And these functions don't actually
 * change the stacking of a window, they're called as a _result_ of
 * changing the stacking, to notify the WSM to re-check and update
 * itself.  So while conceptually we maintain a separation for our
 * callers between various reasons this is being called, the
 * implementations are identical.
 */
void
WMapRaiseLower(TwmWindow *win)
{
	WorkSpace *ws;

	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		if(OCCUPY(win, ws)) {
			WMapRestack(ws);
		}
	}
}

void
WMapLower(TwmWindow *win)
{
	WMapRaiseLower(win);
}

void
WMapRaise(TwmWindow *win)
{
	WMapRaiseLower(win);
}


/*
 * Backend for redoing the stacking of a window in the WSM.
 *
 * XXX Since this tends to get called iteratively, there's probably
 * something better we can do than doing all this relatively expensive
 * stuff over and over...
 */
void
WMapRestack(WorkSpace *ws)
{
	WinList *wl;
	Window  root, parent; // Dummy
	Window  *children, *smallws;
	unsigned int nchildren;

	/* Get a whole list of the windows on the screen */
	nchildren = 0;
	XQueryTree(dpy, Scr->Root, &root, &parent, &children, &nchildren);

	/*
	 * We're presumably dealing with a [often very small] subset of them,
	 * but just allocate space for the whole list; easier than trying to
	 * shrink down, and really, how big can it possibly be?
	 */
	smallws = calloc(nchildren, sizeof(Window));

	/* Work it up per vscreen */
	for(VirtualScreen *vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
		int j = 0;
		const MapSubwindow *msw = vs->wsw->mswl[ws->number];

		/* Loop backward (from top to bottom of stack) */
		for(int i = nchildren - 1; i >= 0; i--) {
			TwmWindow *win = GetTwmWindow(children[i]);

			/*
			 * Only care about certain windows.  If it's not a window we
			 * know about, or not a frame (e.g., an icon), or doesn't
			 * occupy this workspace, skip it.
			 */
			if(win == NULL || win->frame != children[i] || !OCCUPY(win, ws)) {
				continue;
			}

			/* Debug */
			if(tracefile) {
				fprintf(tracefile, "WMapRestack : w = %lx, win = %p\n",
				        children[i], (void *)win);
				fflush(tracefile);
			}

			/* Find this window in the list for the map of this WS */
			for(wl = msw->wl; wl != NULL; wl = wl->next) {
				/* Debug */
				if(tracefile) {
					fprintf(tracefile, "WMapRestack : wl = %p, twm_win = %p\n",
					        (void *)wl, (void *)wl->twm_win);
					fflush(tracefile);
				}

				if(win == wl->twm_win) {
					/* There you are.  Add into our list to restack. */
					smallws[j++] = wl->w;
					break;
				}
			}
		}

		/*
		 * Restack the windows in the map.  Note that the order is
		 * reversed from earlier; XQueryTree() returns bottom->top,
		 * XRestackWindows() is passed top->bottom.
		 */
		XRestackWindows(dpy, smallws, j);
	}

	/* Cleanup */
	XFree(children);
	free(smallws);
	return;
}




/*
 ****************************************************************
 *
 * Bits related to the actual drawing of the windows in the map.
 *
 ****************************************************************
 */

/*
 * Update stuff in the WSM when win's icon name changes
 */
void
WMapUpdateIconName(TwmWindow *win)
{
	VirtualScreen *vs;
	WorkSpace *ws;
	WinList *wl;

	for(vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
		for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
			for(wl = vs->wsw->mswl[ws->number]->wl; wl != NULL; wl = wl->next) {
				if(win == wl->twm_win) {
					WMapRedrawName(vs, wl);
					break;
				}
			}
		}
	}
}


/*
 * Draw a window name into the window's representation in the map-state
 * WSM.
 */
void
WMapRedrawName(VirtualScreen *vs, WinList *wl)
{
	ColorPair cp = wl->cp;

	if(Scr->ReverseCurrentWorkspace && wl->wlist == vs->wsw->currentwspc) {
		InvertColorPair(&cp);
	}
	WMapRedrawWindow(wl->w, wl->width, wl->height, cp, wl->twm_win->icon_name);
}


/*
 * Draw up a window's representation in the map-state WSM, with the
 * window name.
 *
 * The drawing of the window name could probably be done a bit better.
 * The font size is based on a tiny fraction of the window's height, so
 * is probably usually too small to be useful, and often appears just as
 * some odd colored pixels at the top of the window.
 */
static void
WMapRedrawWindow(Window window, int width, int height,
                 ColorPair cp, const char *label)
{
	int x, y;
	const MyFont font = Scr->workSpaceMgr.windowFont;

	/* Blank out window background color */
	XClearWindow(dpy, window);

	/* Figure out where to position the name */
	{
		XRectangle inc_rect;
		XRectangle logical_rect;
		int strhei, strwid;
		int i, descent;
		XFontStruct **xfonts;
		char **font_names;
		int fnum;

		XmbTextExtents(font.font_set, label, strlen(label),
		               &inc_rect, &logical_rect);
		strwid = logical_rect.width;
		strhei = logical_rect.height;

		/*
		 * If it's too tall to fit, just give up now.
		 * XXX Probably should still do border stuff below...
		 */
		if(strhei > height) {
			return;
		}

		x = (width - strwid) / 2;
		if(x < 1) {
			x = 1;
		}

		fnum = XFontsOfFontSet(font.font_set, &xfonts, &font_names);
		for(i = 0, descent = 0; i < fnum; i++) {
			/* xf = xfonts[i]; */
			descent = ((descent < (xfonts[i]->max_bounds.descent)) ?
			           (xfonts[i]->max_bounds.descent) : descent);
		}

		y = ((height + strhei) / 2) - descent;
	}

	/* Draw up borders around the win */
	if(Scr->use3Dwmap) {
		Draw3DBorder(window, 0, 0, width, height, 1, cp, off, true, false);
		FB(cp.fore, cp.back);
	}
	else {
		FB(cp.back, cp.fore);
		XFillRectangle(dpy, window, Scr->NormalGC, 0, 0, width, height);
		FB(cp.fore, cp.back);
	}

	/* Write in the name */
	if(Scr->Monochrome != COLOR) {
		XmbDrawImageString(dpy, window, font.font_set, Scr->NormalGC, x, y,
		                   label, strlen(label));
	}
	else {
		XmbDrawString(dpy, window, font.font_set, Scr->NormalGC, x, y,
		              label, strlen(label));
	}
}




/*
 * Processes for adding/removing windows from the WSM
 */

/*
 * Add a window into any appropriate WSMs' maps.  Called during
 * AddWindow().
 */
void
WMapAddWindow(TwmWindow *win)
{
	WorkSpace *ws;

	if(!WMapWindowMayBeAdded(win)) {
		return;
	}

	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		if(OCCUPY(win, ws)) {
			WMapAddWindowToWorkspace(win, ws);
		}
	}
}


/*
 * Create WSM representation of a given window in a given WS.  Called
 * when windows get added to a workspace, either via WMapAddWindow()
 * during the AddWindow() process, or via an occupation change.
 *
 * (previously: WMapAddToList())
 */
void
WMapAddWindowToWorkspace(TwmWindow *win, WorkSpace *ws)
{
	ColorPair cp;

	/* Setup coloring for the window */
	cp.back = win->title.back;
	cp.fore = win->title.fore;
	if(Scr->workSpaceMgr.windowcpgiven) {
		cp.back = Scr->workSpaceMgr.windowcp.back;
		GetColorFromList(Scr->workSpaceMgr.windowBackgroundL,
		                 win->name, &win->class, &cp.back);
		cp.fore = Scr->workSpaceMgr.windowcp.fore;
		GetColorFromList(Scr->workSpaceMgr.windowForegroundL,
		                 win->name, &win->class, &cp.fore);
	}
	if(Scr->use3Dwmap && !Scr->BeNiceToColormap) {
		GetShadeColors(&cp);
	}

	/* We need a copy in each VS */
	for(VirtualScreen *vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
		unsigned int bw;
		WinList *wl;
		const float wf = (float)(vs->wsw->wwidth  - 2) / (float) vs->w;
		const float hf = (float)(vs->wsw->wheight - 2) / (float) vs->h;
		MapSubwindow *msw = vs->wsw->mswl[ws->number];

		/* Put together its winlist entry */
		wl = malloc(sizeof(struct winList));
		wl->wlist  = ws;
		wl->x      = (int)(win->frame_x * wf);
		wl->y      = (int)(win->frame_y * hf);
		wl->width  = (unsigned int)((win->frame_width  * wf) + 0.5);
		wl->height = (unsigned int)((win->frame_height * hf) + 0.5);
		wl->cp     = cp;
		wl->twm_win = win;

		/* Size the window bits */
		bw = 0;
		if(!Scr->use3Dwmap) {
			bw = 1;
			wl->width  -= 2;
			wl->height -= 2;
		}
		if(wl->width < 1) {
			wl->width  = 1;
		}
		if(wl->height < 1) {
			wl->height = 1;
		}

		/* Create its little window */
		wl->w = XCreateSimpleWindow(dpy, msw->w, wl->x, wl->y,
		                            wl->width, wl->height, bw,
		                            Scr->Black, cp.back);

		/* Setup cursor and attributes for it */
		{
			XSetWindowAttributes attr;
			unsigned long attrmask;

			attr.cursor = handCursor;
			attrmask = CWCursor;

			if(Scr->BackingStore) {
				attr.backing_store = WhenMapped;
				attrmask |= CWBackingStore;
			}

			XChangeWindowAttributes(dpy, wl->w, attrmask, &attr);
		}

		/* Setup events and stash context bits */
		XSelectInput(dpy, wl->w, ExposureMask);
		XSaveContext(dpy, wl->w, TwmContext, (XPointer) vs->wsw->twm_win);
		XSaveContext(dpy, wl->w, ScreenContext, (XPointer) Scr);
		XSaveContext(dpy, wl->w, MapWListContext, (XPointer) wl);

		/* Link it onto the front of the list */
		wl->next = msw->wl;
		msw->wl  = wl;

		/*
		 * And map it, if its window is mapped.  That'll kick an expose
		 * event, which will work its way down to WMapRedrawWindow(), and
		 * fill things in.
		 */
		if(win->mapped) {
			XMapWindow(dpy, wl->w);
		}
	} // And around to the next vscreen
}


/*
 * Remove a window from any WSM maps it's in.  Called during window
 * destruction process.
 *
 * (previously: WMapDestroyWindow())
 */
void
WMapRemoveWindow(TwmWindow *win)
{
	WorkSpace *ws;

	for(ws = Scr->workSpaceMgr.workSpaceList; ws != NULL; ws = ws->next) {
		if(OCCUPY(win, ws)) {
			WMapRemoveWindowFromWorkspace(win, ws);
		}
	}

	/*
	 * If it's a mapped occupy window, manually hide aways its bits in
	 * here.
	 *
	 * XXX Better belongs inline in caller or separate func?  This is the
	 * only thing exposing occupyWin out of occupation.c.
	 */
	if(win == occupyWin) {
		OccupyWindow *occwin = Scr->workSpaceMgr.occupyWindow;
		XUnmapWindow(dpy, occwin->twm_win->frame);
		occwin->twm_win->mapped = false;
		occwin->twm_win->occupation = 0;
		occupyWin = NULL;
	}
}


/*
 * Remove window's WSM representation.  Happens from WMapRemoveWindow()
 * as part of the window destruction process, and in the occupation
 * change process.
 *
 * (previously: WMapRemoveFromList())
 */
void
WMapRemoveWindowFromWorkspace(TwmWindow *win, WorkSpace *ws)
{
	VirtualScreen *vs;

	for(vs = Scr->vScreenList; vs != NULL; vs = vs->next) {
		WinList **prev = &vs->wsw->mswl[ws->number]->wl;

		/* Pull it out of the list and destroy it */
		for(WinList *wl = *prev ; wl != NULL ; wl = wl->next) {
			if(win != wl->twm_win) {
				/* Not it */
				prev = &wl->next;
				continue;
			}

			/* There you are.  Unlink and kill */
			*prev = wl->next;

			XDeleteContext(dpy, wl->w, TwmContext);
			XDeleteContext(dpy, wl->w, ScreenContext);
			XDeleteContext(dpy, wl->w, MapWListContext);
			XDestroyWindow(dpy, wl->w);
			free(wl);

			/* Around to the next vscreen */
			break;
		}
	}
}




/*
 ****************************************************************
 *
 * Utils-ish funcs
 *
 ****************************************************************
 */

/*
 * This is really more util.c fodder, but leaving it here for now because
 * it's only used once in WMapRedrawName().  If we start finding external
 * uses for it, it should be moved.
 */
static void
InvertColorPair(ColorPair *cp)
{
	Pixel save;

	save = cp->fore;
	cp->fore = cp->back;
	cp->back = save;
	save = cp->shadc;
	cp->shadc = cp->shadd;
	cp->shadd = save;
}


/*
 * Verify if a window may be added to the workspace map.
 * This is not allowed for
 * - icon managers
 * - the occupy window
 * - workspace manager windows
 * - or, optionally, windows with full occupation.
 */
bool
WMapWindowMayBeAdded(TwmWindow *win)
{
	if(win->isiconmgr) {
		return false;
	}
	if(win == Scr->workSpaceMgr.occupyWindow->twm_win) {
		return false;
	}
	if(win->iswspmgr) {
		return false;
	}
	if(Scr->workSpaceMgr.noshowoccupyall &&
	                win->occupation == fullOccupation) {
		return false;
	}
	return true;
}
