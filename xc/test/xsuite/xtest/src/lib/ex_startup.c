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
 * $XConsortium: ex_startup.c,v 1.10 94/04/17 21:00:45 rws Exp $
 */

#include "unistd.h"
#include "string.h"
#include "tet_api.h"
#include "xtest.h"
#include "Xlib.h"
#include "Xutil.h"
#include "xtestlib.h"
#include "pixval.h"

/*
 * Generic startup routine for functions which are called
 * via tet_exec.
 */

extern	int 	ntests;

Display	*Dsp;
Window	Win;

Window	ErrdefWindow;
Drawable ErrdefDrawable;
GC		ErrdefGC;
Colormap ErrdefColormap;

/* NOT DONE YET */
Pixmap	ErrdefPixmap;
Atom	ErrdefAtom;
Cursor	ErrdefCursor;
Font	ErrdefFont;

void
exec_startup()
{
char	*disp;
char	*cp;
extern	char	*TestName;
#if TEST_ANSI
extern	int 	unexp_err(Display *, XErrorEvent *ep);
extern	int 	io_err(Display *);
#else
extern	int 	unexp_err();
extern	int 	io_err();
#endif

	initconfig();

	/*
	 * Set the debug level first.
	 */
	if ((cp = tet_getvar("XT_DEBUG")) != NULL) {
	extern	int 	DebugLevel;

		setdblev(atov(cp));
	} else {
		setdblev(0);
	}

	/*
	 * Get the display to use and open it.
	 */
	disp = tet_getvar("XT_DISPLAY");

	if (disp == (char*)0) {
		report("exec_startup:XT_DISPLAY not set");
		UNRESOLVED;
		exit(0);
	}

	Dsp = XOpenDisplay(disp);

	if (Dsp == (Display *)0) {
		report("exec_startup:Could not open display");
		UNRESOLVED;
		exit(0);
	}

#ifdef notyet
	/* Get a window for general use */
	Win = makewin(Dsp);
#endif

	/*
	 * Set up the error handlers.
	 */
	(void) XSetErrorHandler(unexp_err);
	(void) XSetIOErrorHandler(io_err);

	/*
	 * Set up the default resources for error tests. At present
	 * just use the root window (could create a window specialy)
	 */
	ErrdefWindow = DefaultRootWindow(Dsp);
	ErrdefDrawable = DefaultRootWindow(Dsp);
	ErrdefGC = DefaultGC(Dsp, DefaultScreen(Dsp));
	ErrdefColormap = DefaultColormap(Dsp, DefaultScreen(Dsp));

	/*
	 * Sync and clear out queue.
	 */
	XSync(Dsp, True);

}

/*
 * Cleanup functions called at the end of the test purposes which
 * are called via tet_exec.
 */
void
exec_cleanup()
{
	if (Dsp)
		XCloseDisplay(Dsp);
}
