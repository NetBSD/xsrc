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
 * $XConsortium: clrar.m,v 1.9 94/04/17 21:04:27 rws Exp $
 */
>>TITLE XClearArea CH06
>>#SYNOPSIS
void
XClearArea(display, w, x, y, width, height, exposures)
Display	*display = Dsp;
Window	w;
int 	x = 5;
int 	y = 6;
unsigned int 	width = 25;
unsigned int 	height = 20;
Bool	exposures = False;
>>ASSERTION Good A
A call to xname
paints a rectangular area 
in the window specified by the
.A w
argument
with the window's background pixel or pixmap.
>>STRATEGY
For window, pixmap
	Create drawable.
	Set window background pixel to 0.
	Call XClearArea.
	Verify area is set.  Outside is untouched.

	Set window background pixel to 1.
	Call XClearArea.
	Verify area is set.  Outside is untouched.

	Set window background pixmap
	Call XClearArea.
	Verify area is set.  Outside is untouched.
>>CODE
Pixmap	pm;
XVisualInfo	*vp;
struct	area 	area;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		w = makewin(display, vp);

		XSetWindowBackground(display, w, W_FG);
		XCALL;

		setarea(&area, x, y, width, height);

		if (checkarea(display, w, &area, W_FG, W_BG, 0) == False) {
			tet_result(TET_FAIL);
		} else {
			CHECK;
		}

		XSetWindowBackground(display, w, W_BG);
		XCALL;
		if (checkarea(display, w, &area, W_BG, W_BG, 0) == False) {
			tet_result(TET_FAIL);
		} else
			CHECK;

		pm = maketile(display, w);
		XSetWindowBackgroundPixmap(display, w, pm);
		XCALL;

		/*
		 * Check the area first and then the outside.
		 */
		if (checktile(display, w, &area, 0, 0, pm) == False) {
			report("Failed with background pixmap");
			tet_result(TET_FAIL);
		} else
			CHECK;
		if (checkarea(display, w, &area,  W_BG, W_BG, CHECK_OUT) == False) {
			report("Surrounding area was modified when area cleared with background pixmap");
			tet_result(TET_FAIL);
		} else
			CHECK;
	}

	CHECKPASS(4*nvinf());

>>ASSERTION  Good A
On a call to xname the subwindow-mode is
.S ClipByChildren .
>>STRATEGY
Create window
Create child window overlapping area to be cleared
Call XClearArea.
Verify that the child has not been drawn upon.
>>CODE
struct	area	carea;
Pixmap	pm;
XVisualInfo	*vp;
Window	child;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		w = makewin(display, vp);
		carea.x = x + width/2;
		carea.y = 0;
		carea.width = carea.height = 0;
		child = crechild(display, w, &carea);

		pm = maketile(display, w);
		XSetWindowBackgroundPixmap(display, w, pm);
		XCALL;

		carea.x = carea.y = carea.width = carea.height = 0;
		if (checkarea(display, child, &carea, W_BG, W_BG, CHECK_ALL) == False) {
			report("Child window was drawn upon by clear area");
			FAIL;
		} else {
			CHECK;
		}
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When
.A width
is zero, then it is replaced with the current width
of the window minus
.A x .
>>STRATEGY
Call XClearArea with width equal to zero.
Call checkarea to verify that area set has height of width - x.
>>CODE
struct	area	area;
XVisualInfo	*vp;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		w = makewin(display, vp);

		width = 0;
		XSetWindowBackground(display, w, W_FG);
		XCALL;

		area.x = x;
		area.y = y;
		getsize(display, w, &area.width, (unsigned int*)0);
		area.width -= x;
		area.height = height;
		if (checkarea(display, w, &area, W_FG, W_BG, 0) == True) {
			CHECK;
		} else {
			report("Fail on width 0");
			FAIL;
		}
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When
.A height
is zero, then it is
replaced with the current height of the window minus
.A y .
>>STRATEGY
Call XClearArea with height equal to zero.
Call checkarea to verify that area has height (window height - y).
>>CODE
struct	area	area;
XVisualInfo	*vp;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		w = makewin(display, vp);

		height = 0;
		XSetWindowBackground(display, w, W_FG);
		XCALL;
		area.x = x;
		area.y = y;
		getsize(display, w, (unsigned int*)0, &area.height);
		area.width = width;
		area.height -= y;
		if (checkarea(display, w, &area, W_FG, W_BG, 0) == True) {
			CHECK;
		} else {
			report("Fail on height 0");
			FAIL;
		}
	}
	CHECKPASS(nvinf());

>>ASSERTION def
When the window has a defined background tile, then
the rectangle clipped by any children
is tiled with a plane-mask of all ones and the
.S GXCopy
function.
>>ASSERTION Good A
When the window has a
.M background_pixmap
of
.S None ,
>># *** This is not neccesary -- it is the last thing set that matters ..steve
>># and an undefined
>># .M background_pixel
then the contents of the window are not changed.
>>STRATEGY
Create window.
Set background pixel to W_FG.
Set background pixmap to None.
Call XClearArea.
Check that the window is not changed.
>>CODE
struct	area	area;
XVisualInfo	*vp;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		w = makewin(display, vp);
		pattern(display, w);
		XSetWindowBackground(display, w, W_FG);
		XSetWindowBackgroundPixmap(display, w, None);
		XCALL;
		if (checkpattern(display, w, (struct area *)0) == False) {
			report("Window was changed when background was None");
			FAIL;
		} else {
			CHECK;
		}
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When the window has a
.M background_pixmap
of
.S ParentRelative
and the parent has a 
.M background_pixmap
of
.S None ,
then the contents of the window are not changed.
>>STRATEGY
Create parent window.
Create a child window covering the parent.
Set parent window background to none.
Set child window background pixmap to ParentRelative.
Clear area of child
Verify that there was no change to the child.
>>CODE
struct	area	area;
Window	child;
Window	win;
XVisualInfo	*vp;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		win = makewin(display, vp);
		area.x = area.y = area.width = area.height = 0;
		child = crechild(display, win, &area);
		pattern(display, child);

		/*
		 * Set parent to None, child to Parent Relative.
		 */
		XSetWindowBackgroundPixmap(display, win, None);
		XSetWindowBackgroundPixmap(display, child, ParentRelative);

		w = child;
		XCALL;
		if (checkpattern(display, child, (struct area *)0) == False) {
			report("Window was changed when background was ParentRelative");
			FAIL;
		} else {
			CHECK;
		}
	}
	CHECKPASS(nvinf());

>>ASSERTION Good A
When
.A exposures
is 
.S True ,
then one or more 
.S Expose 
events are generated for regions of the rectangle that are
either visible or are being retained in backing store.
>>STRATEGY
Set exposures to True.
Call XClearArea.
Check that an expose event is received.
>>CODE
XEvent	event;
XExposeEvent	ge;
XVisualInfo	*vp;
int 	n;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		w = makewin(display, vp);
		exposures = True;

		XSelectInput(display, w, ALLEVENTS);
		XCALL;
		XSelectInput(display, w, NoEventMask);

		if (getevent(display, &event) != 1 || event.type != Expose) {
			report("Expecting one Expose event");
			tet_result(TET_FAIL);
			return;
		}

		ge.type = Expose;
		ge.display = display;
		ge.window = w;
		ge.x = x;
		ge.y = y;
		ge.width = width;
		ge.height = height;
		ge.count = 0;

		n = checkevent((XEvent*)&ge, (XEvent*)&event);
		if (n == 0)
			CHECK;
		else {
			report("error in %d field%s of event", n, (n!=1)?"s":"");
			FAIL;
		}
	}
	CHECKPASS(nvinf());

>>ASSERTION Bad A
.ER Match inputonly
>>ASSERTION Bad A
.ER Value exposures True False
>>ASSERTION Bad A
.ER BadWindow 
>># HISTORY steve Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90
