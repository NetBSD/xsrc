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
 * $XConsortium: strgn.m,v 1.7 94/04/17 21:10:35 rws Exp $
 */
>>TITLE XSetRegion CH10

XSetRegion(display, gc, r)
Display	*display = Dsp;
GC	gc;
Region	r = XCreateRegion();
>>SET tpcleanup strgncleanup
>>EXTERN
static void
strgncleanup()
{
	XDestroyRegion(r);
	tpcleanup();
}

>>ASSERTION Good A
A call to xname sets the
.M clip_mask
component of the
.A gc
argument to the region
.A r .
>>STRATEGY
Create a region using XCreateRegion.
Set the region to a rectangle using XUnionRectWithRegion.
Create a drawable using XCreateWindow.
Create a gc using XCreateGC.
Draw the clip mask rectangle using XFillRectangle.
Set the graphics function of the GC to GXxor using XSetFunction.
Set the foreground pixel of the gc to W_BG ^ W_FG using XSetForeground.
Set the clip mask of the gc to the region using xname.
Set every pixel in the drawable using XFillRectangle.
Verify that every pixel in the drawable is set to W_BG.
Destroy the region using XDestroyRegion.
>>CODE
XVisualInfo		*vi;
Window			win;
GC			gc;
unsigned int    	width;
unsigned int    	height;
static XRectangle	cliprect2 = { 23, 13, 53, 63 };
static XRectangle	cliprect = { 0,0, 1,1};

	resetvinf(VI_WIN);
	nextvinf(&vi);
	win = makewin(display, vi); /* Makes a window with bg W_BG. */

	gc = makegc(display, win);
	XSetFillStyle(display, gc, FillSolid);
	XSetFunction(display, gc, GXxor);
	XSetForeground(display, gc, W_BG^W_FG);

	XUnionRectWithRegion(&cliprect, r, r);
	XFillRectangle(display, win, gc, cliprect.x, cliprect.y, cliprect.width, cliprect.height);

	if( checkarea(display, win, (struct area *) NULL, W_BG, W_BG, CHECK_ALL|CHECK_DIFFER)){
		delete("XFillRectangle() did set any pixels to non-background values.");
		return;
	} else
		CHECK;

	XCALL;
	getsize(display, win, &width, &height);
	XFillRectangle(display, win, gc, 0,0, width, height);

	if( checkarea(display, win, (struct area *) NULL, W_BG, W_BG, CHECK_ALL) == 0){
		FAIL;
	} else
		CHECK;
	
	CHECKPASS(2);

>>ASSERTION Bad A
.ER BadGC
