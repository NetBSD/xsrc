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
 * $XConsortium: cursor.c,v 1.3 94/04/17 21:00:41 rws Exp $
 */

#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"xtestlib.h"
#include	"pixval.h"

/* does this cursor match that of the window? */

curofwin(display, cursor, window)
Display	*display;
Cursor	cursor;
Window	window;
{
	return CompareCursorWithWindow(display, window, cursor);
}

/* has this window the default cursor? */

defcur(display, window)
Display	*display;
Window	window;
{
	Window root, wjunk, withdefault;
	int ijunk, rx, ry, res;
	unsigned int ujunk;

	if (!XQueryPointer(display, window, &root, &wjunk,
			&rx, &ry, &ijunk, &ijunk, &ujunk)) {
		delete("pointer not on same screen as window 0x%x.",
				(unsigned)window);
		return False;
	}
	withdefault = crechild(display, root, (struct area *)NULL);
	/* that window should have default Cursor, whatever that is */

	/*
	 * Note if a grab is active with a cursor specified then that's what
	 * we've got instead of our nice default cursor, so we fail.
	 * Also, if confine_to & grab then we may not get to our nice new
	 * window.
         * DO NOT USE THIS WHEN GRABS ARE ACTIVE.
	 */
	XWarpPointer(display, None, withdefault, 0, 0, 0, 0, 1, 1);
	res = CompareCurrentWithWindow(display, window);
	XWarpPointer(display, None, root, 0, 0, 0, 0, rx, ry);
	return res;
}

/* is the current cursor the same as that of window? */

spriteiswin(display, window)
Display	*display;
Window	window;
{
	return CompareCurrentWithWindow(display, window);
}
