/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: crechild.c,v 1.15 94/04/17 21:00:40 rws Exp $
 */

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

/*
 * Create, map and wait for a child window covering the
 * specified area of the parent window.
 * If either width or height are zero then the child extends to the left
 * or bottom edge of the parent window.
 * If ap is NULL then the child will exactly cover the parent window.
 */
Window
crechild(disp, w, ap)
Display	*disp;
Window	w;
struct	area	*ap;
{
Window	child;
XEvent	event;
XWindowAttributes	atts;

	child = creunmapchild(disp, w, ap);

	if (isdeleted())
		return None; /* avoid waiting for events that won't happen. */

	XSync(disp, True);
	XSelectInput(disp, child, ExposureMask);
	XMapWindow(disp, child);

	XGetWindowAttributes(disp, child, &atts);
	if (XPending(disp) && atts.map_state == IsViewable)
		XWindowEvent(disp, child, ExposureMask, &event);

	XSelectInput(disp, child, NoEventMask);

	return(child);
}

/*
 * Create, without mapping, a child window covering the
 * specified area of the parent window.
 * If either width or height are zero then the child extends to the left
 * or bottom edge of the parent window.
 * If ap is NULL then the child will exactly cover the parent window.
 */
Window
creunmapchild(disp, w, ap)
Display	*disp;
Window	w;
struct	area	*ap;
{
Window	child;
struct area ar;

	if(ap == (struct area *) 0) {	
		ar.width = ar.height = ar.x = ar.y = 0;
		ap = &ar;
	}

	/*
	 * If either width or height is 0, then use size of parent window.
	 * This ensures that the window covers the rest of the
	 * parent without sticking off the edge.
	 */
	if (ap->width == 0) {
		getsize(disp, w, &ap->width, (unsigned int*)0);
		ap->width -= ap->x;
	}
	if (ap->height == 0) {
		getsize(disp, w, (unsigned int*)0, &ap->height);
		ap->height -= ap->y;
	}

	child = XCreateSimpleWindow(disp, w, ap->x, ap->y, ap->width, ap->height, 0, W_FG, W_BG);

	return(child);
}
