/*
 * Window de/iconification handling
 */

#include "ctwm.h"

#include <stdlib.h>
#include <sys/time.h>

#include <X11/extensions/shape.h>

#include "events.h"
#include "functions.h"
#include "iconmgr.h"
#include "icons.h"
#include "list.h"
#include "otp.h"
#include "screen.h"
#include "util.h"
#include "vscreen.h"
#include "win_iconify.h"
#include "win_ops.h"
#include "win_utils.h"
#include "workspace_manager.h"


/* Animations */
static void MosaicFade(TwmWindow *tmp_win, Window blanket);
static void ZoomInWindow(TwmWindow *tmp_win, Window blanket);
static void ZoomOutWindow(TwmWindow *tmp_win, Window blanket);
static void FadeWindow(TwmWindow *tmp_win, Window blanket);
static void SweepWindow(TwmWindow *tmp_win, Window blanket);

/* De/iconify utils */
static void Zoom(Window wf, Window wt);
static void ReMapOne(TwmWindow *t, TwmWindow *leader);
static void waitamoment(float timeout);



/*
 * The main routines for iconifying...
 */
void
Iconify(TwmWindow *tmp_win, int def_x, int def_y)
{
	TwmWindow *t;
	bool iconify;
	long eventMask;
	WList *wl;
	Window leader = (Window) - 1;
	Window blanket = (Window) - 1;

	iconify = (!tmp_win->iconify_by_unmapping);
	t = NULL;
	if(tmp_win->istransient) {
		leader = tmp_win->transientfor;
		t = GetTwmWindow(leader);
	}
	else if((leader = tmp_win->group) != 0 && leader != tmp_win->w) {
		t = GetTwmWindow(leader);
	}
	if(t && t->icon_on) {
		iconify = false;
	}
	if(iconify) {
		if(!tmp_win->icon || !tmp_win->icon->w) {
			CreateIconWindow(tmp_win, def_x, def_y);
		}
		else {
			IconUp(tmp_win);
		}
		if(visible(tmp_win)) {
			OtpRaise(tmp_win, IconWin);
			XMapWindow(dpy, tmp_win->icon->w);
		}
	}
	if(tmp_win->iconmanagerlist) {
		for(wl = tmp_win->iconmanagerlist; wl != NULL; wl = wl->nextv) {
			XMapWindow(dpy, wl->icon);
		}
	}

	/* Don't mask anything yet, just get the current for various uses */
	eventMask = mask_out_event(tmp_win->w, 0);

	/* iconify transients and window group first */
	UnmapTransients(tmp_win, iconify, eventMask);

	if(iconify) {
		Zoom(tmp_win->frame, tmp_win->icon->w);
	}

	/*
	 * Prevent the receipt of an UnmapNotify, since that would
	 * cause a transition to the Withdrawn state.
	 */
	tmp_win->mapped = false;

	if((Scr->IconifyStyle != ICONIFY_NORMAL) && !Scr->WindowMask) {
		XWindowAttributes winattrs;
		XSetWindowAttributes attr;

		XGetWindowAttributes(dpy, tmp_win->frame, &winattrs);
		blanket = XCreateWindow(dpy, Scr->Root, winattrs.x, winattrs.y,
		                        winattrs.width, winattrs.height, 0,
		                        CopyFromParent, CopyFromParent,
		                        CopyFromParent, None, &attr);
		XMapWindow(dpy, blanket);
	}

	mask_out_event_mask(tmp_win->w, StructureNotifyMask, eventMask);
	XUnmapWindow(dpy, tmp_win->w);
	XUnmapWindow(dpy, tmp_win->frame);
	restore_mask(tmp_win->w, eventMask);

	SetMapStateProp(tmp_win, IconicState);

	if((Scr->IconifyStyle != ICONIFY_NORMAL) && !Scr->WindowMask) {
		switch(Scr->IconifyStyle) {
			case ICONIFY_MOSAIC:
				MosaicFade(tmp_win, blanket);
				break;
			case ICONIFY_ZOOMIN:
				ZoomInWindow(tmp_win, blanket);
				break;
			case ICONIFY_ZOOMOUT:
				ZoomOutWindow(tmp_win, blanket);
				break;
			case ICONIFY_FADE:
				FadeWindow(tmp_win, blanket);
				break;
			case ICONIFY_SWEEP:
				SweepWindow(tmp_win, blanket);
				break;
			case ICONIFY_NORMAL:
				/* Placate insufficiently smart clang warning */
				break;
		}
		XDestroyWindow(dpy, blanket);
	}
	if(tmp_win == Scr->Focus) {
		SetFocus(NULL, EventTime);
		if(! Scr->ClickToFocus) {
			Scr->FocusRoot = true;
		}
	}
	tmp_win->isicon = true;
	tmp_win->icon_on = iconify;
	WMapIconify(tmp_win);
	if(! Scr->WindowMask && Scr->IconifyFunction.func != 0) {
		char *action;
		XEvent event;

		action = Scr->IconifyFunction.item ? Scr->IconifyFunction.item->action : NULL;
		ExecuteFunction(Scr->IconifyFunction.func, action,
		                (Window) 0, tmp_win, &event, C_ROOT, false);
	}
	XSync(dpy, 0);
}


/*
 * ... and its complement
 */
void
DeIconify(TwmWindow *tmp_win)
{
	TwmWindow *t = tmp_win;
	bool isicon = false;

	/* de-iconify the main window */
	if(Scr->WindowMask) {
		XRaiseWindow(dpy, Scr->WindowMask);
	}
	if(tmp_win->isicon) {
		isicon = true;
		if(tmp_win->icon_on && tmp_win->icon && tmp_win->icon->w) {
			Zoom(tmp_win->icon->w, tmp_win->frame);
		}
		else if(tmp_win->group != (Window) 0) {
			TwmWindow *tmpt = GetTwmWindow(tmp_win->group);
			if(tmpt) {
				t = tmpt;
				if(t->icon_on && t->icon && t->icon->w) {
					Zoom(t->icon->w, tmp_win->frame);
				}
			}
		}
	}

	ReMapOne(tmp_win, t);

	if(isicon &&
	                (Scr->WarpCursor ||
	                 LookInList(Scr->WarpCursorL, tmp_win->name, &tmp_win->class))) {
		WarpToWindow(tmp_win, false);
	}

	/* now de-iconify any window group transients */
	ReMapTransients(tmp_win);

	if(! Scr->WindowMask && Scr->DeIconifyFunction.func != 0) {
		char *action;
		XEvent event;

		action = Scr->DeIconifyFunction.item ?
		         Scr->DeIconifyFunction.item->action : NULL;
		ExecuteFunction(Scr->DeIconifyFunction.func, action,
		                (Window) 0, tmp_win, &event, C_ROOT, false);
	}
	XSync(dpy, 0);
}



/*
 * Animations for popping windows around.
 */
static void
MosaicFade(TwmWindow *tmp_win, Window blanket)
{
	int         srect;
	int         i, j, nrects;
	Pixmap      mask;
	GC          gc;
	XGCValues   gcv;
	XRectangle *rectangles;
	int  width = tmp_win->frame_width;
	int height = tmp_win->frame_height;

	srect = (width < height) ? (width / 20) : (height / 20);
	mask  = XCreatePixmap(dpy, blanket, width, height, 1);

	gcv.foreground = 1;
	gc = XCreateGC(dpy, mask, GCForeground, &gcv);
	XFillRectangle(dpy, mask, gc, 0, 0, width, height);
	gcv.function = GXclear;
	XChangeGC(dpy, gc, GCFunction, &gcv);

	nrects = ((width * height) / (srect * srect)) / 10;
	rectangles = calloc(nrects, sizeof(XRectangle));
	for(j = 0; j < nrects; j++) {
		rectangles [j].width  = srect;
		rectangles [j].height = srect;
	}
	for(i = 0; i < 10; i++) {
		for(j = 0; j < nrects; j++) {
			rectangles [j].x = ((lrand48() %  width) / srect) * srect;
			rectangles [j].y = ((lrand48() % height) / srect) * srect;
		}
		XFillRectangles(dpy, mask, gc, rectangles, nrects);
		XShapeCombineMask(dpy, blanket, ShapeBounding, 0, 0, mask, ShapeSet);
		XFlush(dpy);
		waitamoment(0.020);
	}
	XFreePixmap(dpy, mask);
	XFreeGC(dpy, gc);
	free(rectangles);
}


static void
ZoomInWindow(TwmWindow *tmp_win, Window blanket)
{
	Pixmap        mask;
	GC            gc, gcn;
	XGCValues     gcv;

	int i, nsteps = 20;
	int w = tmp_win->frame_width;
	int h = tmp_win->frame_height;
	int step = (MAX(w, h)) / (2.0 * nsteps);

	mask = XCreatePixmap(dpy, blanket, w, h, 1);
	gcv.foreground = 1;
	gc  = XCreateGC(dpy, mask, GCForeground, &gcv);
	gcv.function = GXclear;
	gcn = XCreateGC(dpy, mask, GCForeground | GCFunction, &gcv);

	for(i = 0; i < nsteps; i++) {
		XFillRectangle(dpy, mask, gcn, 0, 0, w, h);
		XFillArc(dpy, mask, gc, (w / 2) - ((nsteps - i) * step),
		         (h / 2) - ((nsteps - i) * step),
		         2 * (nsteps - i) * step,
		         2 * (nsteps - i) * step,
		         0, 360 * 64);
		XShapeCombineMask(dpy, blanket, ShapeBounding, 0, 0, mask, ShapeSet);
		XFlush(dpy);
		waitamoment(0.020);
	}
}


static void
ZoomOutWindow(TwmWindow *tmp_win, Window blanket)
{
	Pixmap        mask;
	GC            gc;
	XGCValues     gcv;

	int i, nsteps = 20;
	int w = tmp_win->frame_width;
	int h = tmp_win->frame_height;
	int step = (MAX(w, h)) / (2.0 * nsteps);

	mask  = XCreatePixmap(dpy, blanket, w, h, 1);
	gcv.foreground = 1;
	gc = XCreateGC(dpy, mask, GCForeground, &gcv);
	XFillRectangle(dpy, mask, gc, 0, 0, w, h);
	gcv.function = GXclear;
	XChangeGC(dpy, gc, GCFunction, &gcv);

	for(i = 0; i < nsteps; i++) {
		XFillArc(dpy, mask, gc, (w / 2) - (i * step),
		         (h / 2) - (i * step),
		         2 * i * step,
		         2 * i * step,
		         0, 360 * 64);
		XShapeCombineMask(dpy, blanket, ShapeBounding, 0, 0, mask, ShapeSet);
		XFlush(dpy);
		waitamoment(0.020);
	}
}


void
FadeWindow(TwmWindow *tmp_win, Window blanket)
{
	Pixmap        mask, stipple;
	GC            gc;
	XGCValues     gcv;
	static unsigned char stipple_bits[] = { 0x0F, 0x0F,
	                                        0xF0, 0xF0,
	                                        0x0F, 0x0F,
	                                        0xF0, 0xF0,
	                                        0x0F, 0x0F,
	                                        0xF0, 0xF0,
	                                        0x0F, 0x0F,
	                                        0xF0, 0xF0,
	                                      };
	int w = tmp_win->frame_width;
	int h = tmp_win->frame_height;

	stipple = XCreateBitmapFromData(dpy, blanket, (char *)stipple_bits, 8, 8);
	mask    = XCreatePixmap(dpy, blanket, w, h, 1);
	gcv.background = 0;
	gcv.foreground = 1;
	gcv.stipple    = stipple;
	gcv.fill_style = FillOpaqueStippled;
	gc = XCreateGC(dpy, mask, GCBackground | GCForeground | GCFillStyle | GCStipple,
	               &gcv);
	XFillRectangle(dpy, mask, gc, 0, 0, w, h);

	XShapeCombineMask(dpy, blanket, ShapeBounding, 0, 0, mask, ShapeSet);
	XFlush(dpy);
	waitamoment(0.10);
	XFreePixmap(dpy, stipple);
	XFlush(dpy);
}


static void
SweepWindow(TwmWindow *tmp_win, Window blanket)
{
	float step = 0.0;
	int i, nsteps = 20;
	int dir = 0, dist = tmp_win->frame_x, dist1;

	dist1 = tmp_win->frame_y;
	if(dist1 < dist) {
		dir = 1;
		dist = dist1;
	}
	dist1 = tmp_win->vs->w - (tmp_win->frame_x + tmp_win->frame_width);
	if(dist1 < dist) {
		dir = 2;
		dist = dist1;
	}
	dist1 = tmp_win->vs->h - (tmp_win->frame_y + tmp_win->frame_height);
	if(dist1 < dist) {
		dir = 3;
		dist = dist1;
		ALLOW_DEAD_STORE(dist);
	}

	switch(dir) {
		case 0:
			step = tmp_win->frame_x + tmp_win->frame_width;
			break;
		case 1:
			step = tmp_win->frame_y + tmp_win->frame_height;
			break;
		case 2:
			step = tmp_win->vs->w - tmp_win->frame_x;
			break;
		case 3:
			step = tmp_win->vs->h - tmp_win->frame_y;
			break;
	}
	step /= (float) nsteps;
	step /= (float) nsteps;
	for(i = 0; i < 20; i++) {
		int x = tmp_win->frame_x;
		int y = tmp_win->frame_y;
		switch(dir) {
			case 0:
				x -= i * i * step;
				break;
			case 1:
				y -= i * i * step;
				break;
			case 2:
				x += i * i * step;
				break;
			case 3:
				y += i * i * step;
				break;
		}
		XMoveWindow(dpy, blanket, x, y);
		XFlush(dpy);
		waitamoment(0.020);
	}
}



/*
 * Utils used by various bits above
 */

/***********************************************************************
 *
 *  Procedure:
 *      Zoom - zoom in or out of an icon
 *
 *  Inputs:
 *      wf      - window to zoom from
 *      wt      - window to zoom to
 *
 ***********************************************************************
 */
static void
Zoom(Window wf, Window wt)
{
	int fx, fy, tx, ty;                 /* from, to */
	unsigned int fw, fh, tw, th;        /* from, to */
	long dx, dy, dw, dh;
	long z;
	int j;

	if((Scr->IconifyStyle != ICONIFY_NORMAL) || !Scr->DoZoom
	                || Scr->ZoomCount < 1) {
		return;
	}

	if(wf == None || wt == None) {
		return;
	}

	XGetGeometry(dpy, wf, &JunkRoot, &fx, &fy, &fw, &fh, &JunkBW, &JunkDepth);
	XGetGeometry(dpy, wt, &JunkRoot, &tx, &ty, &tw, &th, &JunkBW, &JunkDepth);

	dx = (long) tx - (long) fx; /* going from -> to */
	dy = (long) ty - (long) fy; /* going from -> to */
	dw = (long) tw - (long) fw; /* going from -> to */
	dh = (long) th - (long) fh; /* going from -> to */
	z = (long)(Scr->ZoomCount + 1);

	for(j = 0; j < 2; j++) {
		long i;

		XDrawRectangle(dpy, Scr->Root, Scr->DrawGC, fx, fy, fw, fh);
		for(i = 1; i < z; i++) {
			int x = fx + (int)((dx * i) / z);
			int y = fy + (int)((dy * i) / z);
			unsigned width = (unsigned)(((long) fw) + (dw * i) / z);
			unsigned height = (unsigned)(((long) fh) + (dh * i) / z);

			XDrawRectangle(dpy, Scr->Root, Scr->DrawGC,
			               x, y, width, height);
		}
		XDrawRectangle(dpy, Scr->Root, Scr->DrawGC, tx, ty, tw, th);
	}
}


static void
ReMapOne(TwmWindow *t, TwmWindow *leader)
{
	if(t->icon_on) {
		Zoom(t->icon->w, t->frame);
	}
	else if(leader->icon) {
		Zoom(leader->icon->w, t->frame);
	}

	if(!t->squeezed) {
		XMapWindow(dpy, t->w);
	}
	t->mapped = true;
#ifdef CAPTIVE
	if(false && Scr->Root != Scr->CaptiveRoot) {        /* XXX dubious test */
		ReparentWindow(dpy, t, WinWin, Scr->Root, t->frame_x, t->frame_y);
	}
#endif
	if(!Scr->NoRaiseDeicon) {
		OtpRaise(t, WinWin);
	}
	XMapWindow(dpy, t->frame);
	SetMapStateProp(t, NormalState);

	if(t->icon && t->icon->w) {
		XUnmapWindow(dpy, t->icon->w);
		IconDown(t);
		if(Scr->ShrinkIconTitles) {
			t->icon->title_shrunk = true;
		}
	}
	if(t->iconmanagerlist) {
		WList *wl;

		for(wl = t->iconmanagerlist; wl != NULL; wl = wl->nextv) {
			XUnmapWindow(dpy, wl->icon);
		}
	}
	t->isicon = false;
	t->icon_on = false;
	WMapDeIconify(t);
}


/*
 * Mostly internal util of iconification, but squeezing code needs it
 * too.
 */
void
ReMapTransients(TwmWindow *tmp_win)
{
	TwmWindow *t;

	/* find t such that it is a transient or group member window */
	for(t = Scr->FirstWindow; t != NULL; t = t->next) {
		if(t != tmp_win &&
		                ((t->istransient && t->transientfor == tmp_win->w) ||
		                 (t->group == tmp_win->w && t->isicon))) {
			ReMapOne(t, tmp_win);
		}
	}
}


/*
 * Ditto previous note about squeezing.
 */
void
UnmapTransients(TwmWindow *tmp_win, bool iconify, long eventMask)
{
	TwmWindow *t;

	for(t = Scr->FirstWindow; t != NULL; t = t->next) {
		if(t != tmp_win &&
		                ((t->istransient && t->transientfor == tmp_win->w) ||
		                 t->group == tmp_win->w)) {
			if(iconify) {
				if(t->icon_on) {
					Zoom(t->icon->w, tmp_win->icon->w);
				}
				else if(tmp_win->icon) {
					Zoom(t->frame, tmp_win->icon->w);
				}
			}

			/*
			 * Prevent the receipt of an UnmapNotify, since that would
			 * cause a transition to the Withdrawn state.
			 */
			t->mapped = false;

			/*
			 * Note that here, we're setting masks relative to what we
			 * were passed, which is that of the window these are
			 * transient for, rather than relative to these windows'
			 * current masks.  I believe in practice it's the same thing,
			 * and it saves getting attributes on each for masking.
			 * Still, a little odd...
			 */
			mask_out_event_mask(t->w, StructureNotifyMask, eventMask);
			XUnmapWindow(dpy, t->w);
			XUnmapWindow(dpy, t->frame);
			restore_mask(t->w, eventMask);

			if(t->icon && t->icon->w) {
				XUnmapWindow(dpy, t->icon->w);
			}
			SetMapStateProp(t, IconicState);
			if(t == Scr->Focus) {
				SetFocus(NULL, EventTime);
				if(! Scr->ClickToFocus) {
					Scr->FocusRoot = true;
				}
			}
			if(t->iconmanagerlist) {
				XMapWindow(dpy, t->iconmanagerlist->icon);
			}
			t->isicon = true;
			t->icon_on = false;
			WMapIconify(t);
		}
	}
}


static void
waitamoment(float timeout)
{
	struct timeval timeoutstruct;
	int usec = timeout * 1000000;
	timeoutstruct.tv_usec = usec % (unsigned long) 1000000;
	timeoutstruct.tv_sec  = usec / (unsigned long) 1000000;
	select(0, NULL, NULL, NULL, &timeoutstruct);
}
