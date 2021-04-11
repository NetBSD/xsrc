/*
 *       Copyright 1988 by Evans & Sutherland Computer Corporation,
 *                          Salt Lake City, Utah
 *  Portions Copyright 1989 by the Massachusetts Institute of Technology
 *                        Cambridge, Massachusetts
 *
 * Copyright 1992 Claude Lecommandeur.
 */

/**********************************************************************
 *
 * $XConsortium: gc.c,v 1.22 91/01/09 17:13:12 rws Exp $
 *
 * Open the fonts and create the GCs
 *
 * 31-Mar-88 Tom LaStrange        Initial Version.
 *
 * Do the necessary modification to be integrated in ctwm.
 * Can no longer be used for the standard twm.
 *
 * 22-April-92 Claude Lecommandeur.
 *
 *
 **********************************************************************/

#include "ctwm.h"

#include <stdio.h>

#include "gram.tab.h"
#include "screen.h"
#include "gc.h"

/***********************************************************************
 *
 *  Procedure:
 *      CreateGCs - open fonts and create all the needed GC's.  I only
 *                  want to do this once, hence the first_time flag.
 *
 ***********************************************************************
 */

void CreateGCs(void)
{
	static ScreenInfo *prevScr = NULL;
	XGCValues       gcv;
	unsigned long   gcm;
	static unsigned char greypattern [] = {0x0f, 0x05, 0x0f, 0x0a};
	Pixmap        greypixmap;
	static char dashlist [2] = {1, 1};

	if(!Scr->FirstTime || prevScr == Scr) {
		return;
	}

	prevScr = Scr;

	/* create GC's */

	gcm = 0;
	gcm |= GCFunction;
	gcv.function = GXxor;
	gcm |= GCLineWidth;
	gcv.line_width = 0;
	gcm |= GCForeground;
	gcv.foreground = Scr->XORvalue;
	gcm |= GCSubwindowMode;
	gcv.subwindow_mode = IncludeInferiors;

	Scr->DrawGC = XCreateGC(dpy, Scr->Root, gcm, &gcv);

	gcm = 0;
	gcm |= GCForeground;
	gcv.foreground = Scr->MenuC.fore;
	gcm |= GCBackground;
	gcv.background = Scr->MenuC.back;

	Scr->MenuGC = XCreateGC(dpy, Scr->Root, gcm, &gcv);

	gcm = 0;
	gcm |= GCPlaneMask;
	gcv.plane_mask = AllPlanes;
	/*
	 * Prevent GraphicsExpose and NoExpose events.  We'd only get NoExpose
	 * events anyway;  they cause BadWindow errors from XGetWindowAttributes
	 * call in FindScreenInfo (events.c) (since drawable is a pixmap).
	 */
	gcm |= GCGraphicsExposures;
	gcv.graphics_exposures = False;
	gcm |= GCLineWidth;
	gcv.line_width = 0;

	Scr->NormalGC = XCreateGC(dpy, Scr->Root, gcm, &gcv);

	greypixmap = XCreatePixmapFromBitmapData(dpy, Scr->Root,
	                (char *) greypattern, 4, 4, 1, 0, 1);

	if(Scr->Monochrome != COLOR) {
		gcm  = 0;
		gcm |= GCStipple;
		gcv.stipple    = greypixmap;
		gcm |= GCFillStyle;
		gcv.fill_style = FillOpaqueStippled;
		gcm |= GCForeground;
		gcv.foreground = Scr->Black;
		gcm |= GCBackground;
		gcv.background = Scr->White;
		Scr->BorderGC = XCreateGC(dpy, Scr->Root, gcm, &gcv);
		XSetDashes(dpy, Scr->BorderGC, 1, dashlist, 2);
	}
	else if(Scr->BeNiceToColormap) {
		gcm  = 0;
		gcm |= GCLineStyle;
		gcv.line_style = LineDoubleDash;
		Scr->BorderGC = XCreateGC(dpy, Scr->Root, gcm, &gcv);
		XSetDashes(dpy, Scr->BorderGC, 0, dashlist, 2);
	}
	else {
		Scr->BorderGC = XCreateGC(dpy, Scr->Root, 0, NULL);
	}
}
