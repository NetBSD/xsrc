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
 * $XConsortium: EMat3.mc,v 1.6 94/04/17 21:14:57 rws Exp $
 */
>>ASSERTION Bad A
>>### Match gc-drawable-screen
When the graphics context and the drawable were not created for the same
root, then a
.S BadMatch
error occurs.
>>STRATEGY
If multiple screens are supported
  Create pixmap of depth 1.
  Create gc on alternate screen.
  Call test function with this pixmap and gc.
  Verify that a BadMatch error occurs.
else
  report UNSUPPORTED
>>CODE BadMatch
XVisualInfo	vi;
Pixmap	errpm;
int 	scr_num;

        if (config.alt_screen == -1) {
                unsupported("No alternate root supported");
                return;
        }

	scr_num = config.alt_screen;
	if (scr_num == DefaultScreen(A_DISPLAY)) {
		delete("The alternate root was the same as the one under test");
		return;
	}
	if (scr_num >= ScreenCount(A_DISPLAY)) {
		delete("The alternate root could not be accessed");
		return;
	}

	vi.visual = NULL;
	vi.screen = DefaultScreen(A_DISPLAY);	/* XXX */
	vi.depth = 1;
	A_DRAWABLE = makepixm(A_DISPLAY, &vi);
#ifdef A_DRAWABLE2
	A_DRAWABLE2 = makepixm(A_DISPLAY, &vi);
#endif
#ifdef A_IMAGE
	A_IMAGE = makeimg(A_DISPLAY, &vi, ZPixmap);
	dsetimg(A_IMAGE, W_FG);
#endif

	/*
	 * Create a 1x1 depth 1 pixmap on other screen
	 * and use it to create a gc
	 */
	errpm = XCreatePixmap(A_DISPLAY, RootWindow(A_DISPLAY, scr_num), 1, 1, 1);
	A_GC = makegc(A_DISPLAY, errpm);

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

	XFreePixmap(A_DISPLAY, errpm);
