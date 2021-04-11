/*
 * Functions involving the info/identify window.
 */

#include "ctwm.h"

#include <stdio.h>
#include <stdlib.h>

#include <X11/Xatom.h>

#include "ctopts.h"
#include "drawing.h"
#include "functions.h"
#include "functions_internal.h"
#include "icons.h"
#include "otp.h"
#include "screen.h"
#include "version.h"
#include "vscreen.h"


/* We hold it in a big static buffer */
#define INFO_LINES 30
#define INFO_SIZE 200
static char Info[INFO_LINES][INFO_SIZE];

/* The builder */
static void Identify(TwmWindow *t);


/*
 * The functions that cause this to pop up.
 *
 * n.b.: these are referenced in the Developer Manual in doc/devman/; if
 * you make any changes here be sure to tweak that if necessary.
 */
DFHANDLER(identify)
{
	Identify(tmp_win);
}

DFHANDLER(version)
{
	Identify(NULL);
}


/*
 * Building and displaying.
 */

/*
 * Backend for f.identify and f.version: Fills in the Info array with the
 * appropriate bits for ctwm and the window specified (if any), and
 * sizes/pops up the InfoWindow.
 *
 * Notably, the bits of Info aren't written into the window during this
 * process; that happens later as a result of the expose event.
 */
static void
Identify(TwmWindow *t)
{
	int i, n, twidth, width, height;
	int x, y;
	unsigned int wwidth, wheight, bw, depth;
	Window junk;
	int px, py, dummy;
	unsigned udummy;
	unsigned char *prop;
	unsigned long nitems, bytesafter;
	Atom actual_type;
	int actual_format;
	XRectangle inc_rect;
	XRectangle logical_rect;
	char *ctopts;

	/*
	 * Include some checking we don't blow out _LINES.  We use snprintf()
	 * exclusively to avoid blowing out _SIZE.
	 *
	 * In an ideal world, we'd probably fix this to be more dynamically
	 * allocated, but this will do for now.
	 */
	n = 0;
#define CHKN do { \
                if(n > (INFO_LINES - 3)) { \
                        fprintf(stderr, "Overflowing Info[] on line %d\n", n); \
                        sprintf(Info[n++], "(overflow)"); \
                        goto info_dismiss; \
                } \
        } while(0)

	snprintf(Info[n++], INFO_SIZE, "Twm version:  %s", TwmVersion);
	CHKN;
	if(VCSRevision) {
		snprintf(Info[n++], INFO_SIZE, "VCS Revision:  %s", VCSRevision);
		CHKN;
	}

	ctopts = ctopts_string(", ");
	snprintf(Info[n++], INFO_SIZE, "Compile time options : %s", ctopts);
	free(ctopts);
	CHKN;

	Info[n++][0] = '\0';
	CHKN;

	if(t) {
		XGetGeometry(dpy, t->w, &JunkRoot, &JunkX, &JunkY,
		             &wwidth, &wheight, &bw, &depth);
		XTranslateCoordinates(dpy, t->w, Scr->Root, 0, 0,
		                      &x, &y, &junk);
		snprintf(Info[n++], INFO_SIZE, "Name               = \"%s\"",
		         t->name);
		CHKN;
		snprintf(Info[n++], INFO_SIZE, "Class.res_name     = \"%s\"",
		         t->class.res_name);
		CHKN;
		snprintf(Info[n++], INFO_SIZE, "Class.res_class    = \"%s\"",
		         t->class.res_class);
		CHKN;
		Info[n++][0] = '\0';
		CHKN;
		snprintf(Info[n++], INFO_SIZE,
		         "Geometry/root (UL) = %dx%d+%d+%d (Inner: %dx%d+%d+%d)",
		         wwidth + 2 * (bw + t->frame_bw3D),
		         wheight + 2 * (bw + t->frame_bw3D) + t->title_height,
		         x - (bw + t->frame_bw3D),
		         y - (bw + t->frame_bw3D + t->title_height),
		         wwidth, wheight, x, y);
		CHKN;
		snprintf(Info[n++], INFO_SIZE,
		         "Geometry/root (LR) = %dx%d-%d-%d (Inner: %dx%d-%d-%d)",
		         wwidth + 2 * (bw + t->frame_bw3D),
		         wheight + 2 * (bw + t->frame_bw3D) + t->title_height,
		         Scr->rootw - (x + wwidth + bw + t->frame_bw3D),
		         Scr->rooth - (y + wheight + bw + t->frame_bw3D),
		         wwidth, wheight,
		         Scr->rootw - (x + wwidth), Scr->rooth - (y + wheight));
		CHKN;
		snprintf(Info[n++], INFO_SIZE, "Border width       = %d", bw);
		CHKN;
		snprintf(Info[n++], INFO_SIZE, "3D border width    = %d", t->frame_bw3D);
		CHKN;
		snprintf(Info[n++], INFO_SIZE, "Depth              = %d", depth);
		CHKN;
		if(t->vs &&
		                t->vs->wsw &&
		                t->vs->wsw->currentwspc) {
			snprintf(Info[n++], INFO_SIZE, "Virtual Workspace  = %s",
			         t->vs->wsw->currentwspc->name);
			CHKN;
		}
		snprintf(Info[n++], INFO_SIZE, "OnTopPriority      = %d",
		         OtpEffectiveDisplayPriority(t));
		CHKN;

		if(t->icon != NULL) {
			int iwx, iwy;

			XGetGeometry(dpy, t->icon->w, &JunkRoot, &iwx, &iwy,
			             &wwidth, &wheight, &bw, &depth);
			Info[n++][0] = '\0';
			CHKN;
			snprintf(Info[n++], INFO_SIZE, "IconGeom/root     = %dx%d+%d+%d",
			         wwidth, wheight, iwx, iwy);
			CHKN;
			snprintf(Info[n++], INFO_SIZE, "IconGeom/intern   = %dx%d+%d+%d",
			         t->icon->w_width, t->icon->w_height,
			         t->icon->w_x, t->icon->w_y);
			CHKN;
			snprintf(Info[n++], INFO_SIZE, "IconBorder width  = %d", bw);
			CHKN;
			snprintf(Info[n++], INFO_SIZE, "IconDepth         = %d", depth);
			CHKN;
		}

		if(XGetWindowProperty(dpy, t->w, XA_WM_CLIENT_MACHINE, 0L, 64, False,
		                      XA_STRING, &actual_type, &actual_format, &nitems,
		                      &bytesafter, &prop) == Success) {
			if(nitems && prop) {
				snprintf(Info[n++], INFO_SIZE, "Client machine     = %s",
				         (char *)prop);
				XFree(prop);
				CHKN;
			}
		}
		Info[n++][0] = '\0';
		CHKN;
	}

#undef CHKN
info_dismiss:
	snprintf(Info[n++], INFO_SIZE, "Click to dismiss....");


	/*
	 * OK, it's all built now.
	 */

	/* figure out the width and height of the info window */
	height = n * (Scr->DefaultFont.height + 2);
	width = 1;
	for(i = 0; i < n; i++) {
		XmbTextExtents(Scr->DefaultFont.font_set, Info[i],
		               strlen(Info[i]), &inc_rect, &logical_rect);

		twidth = logical_rect.width;
		if(twidth > width) {
			width = twidth;
		}
	}

	/* Unmap if it's currently up, while we muck with it */
	if(Scr->InfoWindow.mapped) {
		XUnmapWindow(dpy, Scr->InfoWindow.win);
		/* Don't really need to bother since we're about to reset, but...  */
		Scr->InfoWindow.mapped = false;
	}

	/* Stash the new number of lines */
	Scr->InfoWindow.lines = n;

	width += 10;                /* some padding */
	height += 10;               /* some padding */
	if(XQueryPointer(dpy, Scr->Root, &JunkRoot, &JunkChild,
	                 &dummy, &dummy, &px, &py, &udummy)) {
		px -= (width / 2);
		py -= (height / 3);
		if(px + width + BW2 >= Scr->rootw) {
			px = Scr->rootw - width - BW2;
		}
		if(py + height + BW2 >= Scr->rooth) {
			py = Scr->rooth - height - BW2;
		}
		if(px < 0) {
			px = 0;
		}
		if(py < 0) {
			py = 0;
		}
	}
	else {
		px = py = 0;
	}
	XMoveResizeWindow(dpy, Scr->InfoWindow.win, px, py, width, height);
	XMapRaised(dpy, Scr->InfoWindow.win);
	Scr->InfoWindow.mapped = true;
	Scr->InfoWindow.width  = width;
	Scr->InfoWindow.height = height;
}


/*
 * And the routine to actually write the text into the InfoWindow.  This
 * gets called from events.c as a result of Expose events on the window.
 */
void
draw_info_window(void)
{
	int i;
	const int height = Scr->DefaultFont.height + 2;

	Draw3DBorder(Scr->InfoWindow.win, 0, 0,
	             Scr->InfoWindow.width, Scr->InfoWindow.height,
	             2, Scr->DefaultC, off, true, false);

	FB(Scr->DefaultC.fore, Scr->DefaultC.back);

	for(i = 0; i < Scr->InfoWindow.lines ; i++) {
		XmbDrawString(dpy, Scr->InfoWindow.win, Scr->DefaultFont.font_set,
		              Scr->NormalGC, 5,
		              (i * height) + Scr->DefaultFont.y + 5,
		              Info[i], strlen(Info[i]));
	}
}
