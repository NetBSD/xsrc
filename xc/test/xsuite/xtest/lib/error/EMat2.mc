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
 * $XConsortium: EMat2.mc,v 1.5 94/04/17 21:14:57 rws Exp $
 */
>>ASSERTION Bad A
>>### Match gc-drawable-depth
When the graphics context and the drawable
do not have the same depth, then a
.S BadMatch
error occurs.
>>STRATEGY
If only one depth supported
  report UNSUPPORTED
Create pixmap of depth 1.
Create gc of different depth.
Call test function with this pixmap and gc.
Verify that a BadMatch error occurs.
>>CODE BadMatch
XVisualInfo	*vp;
Drawable	errpm;
int 	founddepth = 0;

	for (resetvinf(VI_PIX); nextvinf(&vp); ) {
		if (vp->depth != 1) {
			founddepth++;

			errpm = makewin(A_DISPLAY, vp);
			A_GC = makegc(A_DISPLAY, errpm);
#ifdef A_IMAGE
			A_IMAGE = makeimg(A_DISPLAY, vp, ZPixmap);
			dsetimg(A_IMAGE, W_FG);
#endif

			break;
		}
	}

	if (!founddepth) {
		report("Only one depth supported");
		tet_result(TET_UNSUPPORTED);
		return;
	}

	vp->depth = 1;
	A_DRAWABLE = makewin(A_DISPLAY, vp);
#ifdef A_DRAWABLE2
	A_DRAWABLE2 = makewin(A_DISPLAY, vp);
#endif

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;
