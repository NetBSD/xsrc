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
 * $XConsortium: gettime.c,v 1.4 94/04/17 21:00:49 rws Exp $
 */

#include	"Xlib.h"
#include	"Xutil.h"
#include	"Xatom.h"
#include	"xtest.h"
#include	"xtestlib.h"
#include	"pixval.h"


/*
 * Get the current server time. Use a property attached to the root window
 * of the display called XT_TIMESTAMP and replaces it with 42 (32-bits).
 * The PropertyNotify event that's generated supplies the time stamp returned.
 * The event mask on the root window is restored to its initial state.
 * Returns CurrentTime on error (as well as deleting the test).
 */
Time
gettime(disp)
	Display	*disp;
{
	Window 	root;
	static Atom	prop = None;
	static int	data = 42;
	static char	*name = XT_TIMESTAMP;
	XWindowAttributes wattr;
	XEvent	ev;
	int	i;

	root = XDefaultRootWindow(disp);

	if (XGetWindowAttributes(disp, root, &wattr) == False) {
		delete("gettime: XGetWindowAttributes on root failed.");
		return CurrentTime;
	}

	if (prop==None && (prop=XInternAtom(disp, name, False)) == None) {
		delete("gettime: XInternAtom of '%s' failed.", name);
		return CurrentTime;
	}

	XSelectInput(disp, root, wattr.your_event_mask | PropertyChangeMask);

	XChangeProperty(disp, root, prop, XA_INTEGER, 32, PropModeReplace,
		(unsigned char *)&data, 1);

	for (i=0; i<10; i++, sleep(2)) {
		if (XCheckWindowEvent(disp, root, PropertyChangeMask, &ev))
			break;
	}

	if (i >= 10) {
		delete("gettime: Didn't receive expected PropertyNotify event");
		return CurrentTime;
	}

	XSelectInput(disp, root, wattr.your_event_mask);

	return ev.xproperty.time;
}

