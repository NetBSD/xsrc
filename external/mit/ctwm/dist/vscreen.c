/*
 * Copyright 1992 Claude Lecommandeur.
 */

#include "ctwm.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xatom.h>

#include "ctwm_atoms.h"
#include "cursor.h"
#include "icons.h"
#include "list.h"
#include "otp.h"
#include "screen.h"
#include "vscreen.h"
#include "win_utils.h"


static void DisplayWinUnchecked(VirtualScreen *vs, TwmWindow *tmp_win);


static void init_def_vscreen(ScreenInfo *scr)
{
	VirtualScreen *vs = malloc(sizeof(VirtualScreen));

	vs->x      = 0;
	vs->y      = 0;
	vs->w      = scr->rootw;
	vs->h      = scr->rooth;
	vs->window = scr->Root;
	vs->next   = NULL;
	vs->wsw    = 0;
	scr->vScreenList = vs;
	scr->currentvs   = vs;
#ifdef VSCREEN
	scr->numVscreens = 1;
#endif
	return;
}


void InitVirtualScreens(ScreenInfo *scr)
{
#ifndef VSCREEN
	// Just do the faking if vscreens are all off anyway.
	init_def_vscreen(scr);
	return;
#else

	// Real implementation
	Cursor cursor;
	unsigned long valuemask, attrmask;
	XSetWindowAttributes attributes;
	name_list *nptr;
	VirtualScreen *vs00 = NULL;

	NewFontCursor(&cursor, "X_cursor");

	if(scr->VirtualScreens == NULL) {
		init_def_vscreen(scr);
		return;
	}
	scr->numVscreens = 0;

	attrmask  = ColormapChangeMask | EnterWindowMask | PropertyChangeMask |
	            SubstructureRedirectMask | KeyPressMask | ButtonPressMask |
	            ButtonReleaseMask;

	valuemask = CWBackPixel | CWOverrideRedirect | CWEventMask | CWCursor;
	attributes.override_redirect = True;
	attributes.event_mask        = attrmask;
	attributes.cursor            = cursor;
	attributes.background_pixel  = Scr->Black;

	scr->vScreenList = NULL;
	for(nptr = Scr->VirtualScreens; nptr != NULL; nptr = nptr->next) {
		VirtualScreen *vs;
		char *geometry = (char *) nptr->name;
		int x = 0, y = 0;
		unsigned int w = 0, h = 0;

		XParseGeometry(geometry, &x, &y, &w, &h);

		if((x < 0) || (y < 0) || (w > scr->rootw) || (h > scr->rooth)) {
			fprintf(stderr, "InitVirtualScreens : invalid geometry : %s\n", geometry);
			continue;
		}
		vs = malloc(sizeof(VirtualScreen));
		vs->x = x;
		vs->y = y;
		vs->w = w;
		vs->h = h;
		vs->window = XCreateWindow(dpy, Scr->Root, x, y, w, h,
		                           0, CopyFromParent, CopyFromParent,
		                           CopyFromParent, valuemask, &attributes);
		vs->wsw = 0;

		XSync(dpy, 0);
		XMapWindow(dpy, vs->window);
		XChangeProperty(dpy, vs->window, XA_WM_VIRTUALROOT, XA_STRING, 8,
		                PropModeReplace, (unsigned char *) "Yes", 4);

		vs->next = scr->vScreenList;
		scr->vScreenList = vs;
		Scr->numVscreens++;

		/*
		 * Remember which virtual screen is at (0,0).
		 */
		if(x == 0 && y == 0) {
			vs00 = vs;
		}
	}

	if(scr->vScreenList == NULL) {
		fprintf(stderr, "no valid VirtualScreens found, exiting...\n");
		exit(1);
	}
	/* Setup scr->{currentvs,Root{,x,y,w,h}} as if the
	 * _correct_ virtual screen is entered with the mouse.
	 * See HandleEnterNotify().
	 */
	if(vs00 == NULL) {
		vs00 = scr->vScreenList;
	}

	Scr->Root  = vs00->window;
#ifdef CAPTIVE
	Scr->rootx = Scr->crootx + vs00->x;
	Scr->rooty = Scr->crooty + vs00->y;
#else
	Scr->rootx = vs00->x;
	Scr->rooty = vs00->y;
#endif
	Scr->rootw = vs00->w;
	Scr->rooth = vs00->h;
	Scr->currentvs = vs00;
#endif  // VSCREEN
}

#ifdef VSCREEN
VirtualScreen *findIfVScreenOf(int x, int y)
{
	VirtualScreen *vs;
	for(vs = Scr->vScreenList; vs != NULL; vs = vs->next) {

		if((x >= vs->x) && ((x - vs->x) < vs->w) &&
		                (y >= vs->y) && ((y - vs->y) < vs->h)) {
			return vs;
		}
	}
	return NULL;
}
#endif

/*
 * Returns the order that virtual screens are displayed for the vscreen
 * list.  This is stored this way so everything ends up in the right place
 * on a ctwm restart.
 */
char *
CtwmGetVScreenMap(Display *display, Window rootw)
{
	unsigned char       *prop;
	unsigned long       bytesafter;
	unsigned long       len;
	Atom                actual_type;
	int                 actual_format;
	char                *ret;

	if(XA_WM_CTWM_VSCREENMAP == None) {
		return false;
	}
	if(XGetWindowProperty(display, rootw, XA_WM_CTWM_VSCREENMAP, 0L, 512,
	                      False, XA_STRING, &actual_type, &actual_format, &len,
	                      &bytesafter, &prop) != Success) {
		return NULL;
	}
	if(len == 0) {
		return NULL;
	}

	ret = malloc(len + 1);
	memcpy(ret, prop, len);
	ret[len] = '\0';
	XFree(prop);

	return ret;
}

bool
CtwmSetVScreenMap(Display *display, Window rootw,
                  struct VirtualScreen *firstvs)
{
	char                        buf[1024];
	int                         tally = 0;
	struct VirtualScreen        *vs;

	if(XA_WM_CTWM_VSCREENMAP == None) {
		return false;
	}

	memset(buf, 0, sizeof(buf));
	for(vs = firstvs; vs; vs = vs->next) {
		if(tally) {
			strcat(buf, ",");
		}
		if(vs->wsw && vs->wsw->currentwspc && vs->wsw->currentwspc->name) {
			strcat(buf, vs->wsw->currentwspc->name);
			tally++;
		}
	}

	if(! tally) {
		return false;
	}

	XChangeProperty(display, rootw, XA_WM_CTWM_VSCREENMAP, XA_STRING, 8,
	                PropModeReplace, (unsigned char *)buf, strlen(buf));
	return true;
}


/*
 * Display a window in a given virtual screen.
 */
void
DisplayWin(VirtualScreen *vs, TwmWindow *tmp_win)
{
	OtpCheckConsistency();
	DisplayWinUnchecked(vs, tmp_win);
	OtpCheckConsistency();
}

static void
DisplayWinUnchecked(VirtualScreen *vs, TwmWindow *tmp_win)
{
	/*
	 * A window cannot be shown in multiple virtual screens, even if
	 * it occupies both corresponding workspaces.
	 */
	if(vs && tmp_win->vs) {
		return;
	}

	/* This is where we're moving it */
	tmp_win->vs = vs;


	/* If it's unmapped, RFAI() moves the necessary bits here */
	if(!tmp_win->mapped) {
		ReparentFrameAndIcon(tmp_win);

		/* If it's got an icon that should be up, make it up here */
		if(tmp_win->isicon) {
			if(tmp_win->icon_on) {
				if(tmp_win->icon && tmp_win->icon->w) {

					IconUp(tmp_win);
					XMapWindow(dpy, tmp_win->icon->w);
				}
			}
		}

		/* All there is to do with unmapped wins */
		return;
	}


	/* If we make it this far, the window is mapped */

	if(tmp_win->UnmapByMovingFarAway) {
		/*
		 * XXX I don't believe the handling of UnmapByMovingFarAway is
		 * quite correct.
		 */
		if(vs) {
			XReparentWindow(dpy, tmp_win->frame, vs->window,
			                tmp_win->frame_x, tmp_win->frame_y);
		}
		else {
			XMoveWindow(dpy, tmp_win->frame, tmp_win->frame_x, tmp_win->frame_y);
		}
	}
	else {
		/* Map and move it here */
		if(!tmp_win->squeezed) {
			long eventMask;

			eventMask = mask_out_event(tmp_win->w, StructureNotifyMask);
			XMapWindow(dpy, tmp_win->w);
			restore_mask(tmp_win->w, eventMask);
		}

		ReparentFrameAndIcon(tmp_win);

		XMapWindow(dpy, tmp_win->frame);
		SetMapStateProp(tmp_win, NormalState);
	}
}


/*
 * Move a window's frame and icon to a new VS.  This mostly happens as a
 * backend bit of the DisplayWin() process, but it does get called
 * directly for the Occupy window.  XXX Should it?
 */
void
ReparentFrameAndIcon(TwmWindow *tmp_win)
{
	VirtualScreen *vs = tmp_win->vs; /* which virtual screen we want it in */

	/* parent_vs is the current real parent of the window */
	if(vs != tmp_win->parent_vs) {
		struct Icon *icon = tmp_win->icon;

		// This must always be something...
		assert(vs != NULL);

		tmp_win->parent_vs = vs;

		if(icon && icon->w) {
			ReparentWindowAndIcon(dpy, tmp_win, vs->window,
			                      tmp_win->frame_x, tmp_win->frame_y,
			                      icon->w_x, icon->w_y);
		}
		else {
			ReparentWindow(dpy, tmp_win,  WinWin, vs->window,
			               tmp_win->frame_x, tmp_win->frame_y);
		}
	}
}


/*
 * Get this window outta here.  Note that despite naming, this is
 * unrelated to f.vanish.
 */
void
Vanish(VirtualScreen *vs, TwmWindow *tmp_win)
{
	/* It's not here?  Nothing to do. */
	if(vs && tmp_win->vs && tmp_win->vs != vs) {
		return;
	}

	/* Unmap (or near-equivalent) all its bits */
	if(tmp_win->UnmapByMovingFarAway) {
		/* UnmapByMovingFarAway?  Move it off-screen */
		XMoveWindow(dpy, tmp_win->frame, Scr->rootw + 1, Scr->rooth + 1);
	}
	else if(tmp_win->mapped) {
		/* It's mapped; unmap it */
		long eventMask;

		eventMask = mask_out_event(tmp_win->w, StructureNotifyMask);
		XUnmapWindow(dpy, tmp_win->w);
		XUnmapWindow(dpy, tmp_win->frame);
		restore_mask(tmp_win->w, eventMask);

		if(!tmp_win->DontSetInactive) {
			SetMapStateProp(tmp_win, InactiveState);
		}
	}
	else if(tmp_win->icon_on && tmp_win->icon && tmp_win->icon->w) {
		/* It's not mapped, but the icon's up; hide it away */
		XUnmapWindow(dpy, tmp_win->icon->w);
		IconDown(tmp_win);
	}

#if 0
	/*
	 * The purpose of this is in the event of a ctwm death/restart,
	 * geometries of windows that were on unmapped workspaces will show
	 * up where they belong.
	 * XXX - I doubt its usefulness, since still-mapped windows won't
	 * enjoy this "protection", making it suboptimal at best.
	 * XXX - XReparentWindow() messes up the stacking order of windows.
	 * It should be avoided as much as possible. This already affects
	 * switching away from and back to a workspace. Therefore do this only
	 * if there are at least 2 virtual screens AND the new one (firstvs)
	 * differs from where the window currently is. (Olaf Seibert).
	 */

	if(Scr->numVscreens > 1) {
		int x, y;
		unsigned int junk;
		Window junkW, w = tmp_win->frame;
		VirtualScreen *firstvs = NULL;

		for(firstvs = Scr->vScreenList; firstvs; firstvs = firstvs->next)
			if(firstvs->x == 0 && firstvs->y == 0) {
				break;
			}
		if(firstvs && firstvs != vs) {
			tmp_win->vs = firstvs;
			ReparentFrameAndIcon(tmp_win);
		}
	}
#endif

	/* Currently displayed nowhere */
	tmp_win->vs = NULL;
}
