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
 * $XConsortium: events.c,v 1.5 94/04/17 21:00:45 rws Exp $
 */
/*LINTLIBRARY*/

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"tet_api.h"
#include	<unistd.h>

/*
 * Return in x_root_return and y_root_return the specified coordinates 
 * transformed from the coordinate space of the given window to that
 * of the specified window.  It is assumed that the two windows are on
 * the same screen.
 *
 * Can be used in conjunction with ROOTCOORDSET macro.
 */
void
rootcoordset(display, src_w, dest_w, src_x, src_y, dest_x_return, dest_y_return)
Display	*display;
Window	src_w;
Window	dest_w;
int	src_x;
int	src_y;
int	*dest_x_return;
int	*dest_y_return;
{
	Window	window;

	(void) XTranslateCoordinates(display, src_w, dest_w, src_x, src_y, dest_x_return, dest_y_return, &window);
}

/*
 * Set the serial member of the specified event to NextRequest.
 */
void
serialset(display, event)
Display	*display;
XEvent	*event;
{
	event->xany.serial = NextRequest(display);
}

/*
 * Compare serial field of two events.  Return True if they compare
 * equal, else return False.
 */
Bool
serialtest(good, ev)
XEvent	*good;
XEvent	*ev;
{
	if (ev->xany.serial != good->xany.serial) {
		report("Checking event type %s", eventname(good->type));
		report("found error in %s field, was %ld, expecting %ld","serial",ev->xany.serial, good->xany.serial);
		return(False);
	}
	return(True);
}
