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
 * $XConsortium: cpyar.m,v 1.8 94/04/17 21:04:29 rws Exp $
 */
>>TITLE XCopyArea CH06
void
xname(display, src, dest, gc, src_x, src_y, width, height, dest_x, dest_y)
Display	*display = Dsp;
Drawable	src;
Drawable	dest;
GC  	gc;
int 	src_x = 0;
int 	src_y = 0;
unsigned int 	width = 20;
unsigned int 	height = 40;
int 	dest_x = 5;
int 	dest_y = 6;
>>EXTERN
#include	"Xproto.h"

#define DEFAULTVINF	((XVisualInfo *)0)

#define	XOFF	50
#define TRIM	20

>>ASSERTION Good A
A call to xname combines the specified rectangle of
.A src
with the specified rectangle of
.A dest ,
according to the 
.M function 
in the argument
.A gc .
>>STRATEGY
Create window pair.
Tile background of first window
Copy to other window.
Verify copy on other window.
>>CODE
Window	w1, w2;
XVisualInfo	*vp;
struct	area	area;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		winpair(display, vp, &w1, &w2);

		pattern(display, w1);

		src = w1;
		dest = w2;

		gc = makegc(display, dest);

		XCALL;

		area.x = dest_x;
		area.y = dest_y;
		area.width = width;
		area.height = height;

		if (checkarea(display, dest, &area, 0, W_BG, CHECK_OUT) == False) {
			report("window modified outside the target area");
			FAIL;
		} else
			CHECK;

		if (checkpattern(display, dest, &area) == False) {
			report("area was not copied properly");
			FAIL;
		} else
			CHECK;

	}
	CHECKPASS(2*nvinf());

>>ASSERTION Good A
When regions of the source rectangle are obscured and have not been
retained in backing store 
or regions outside the boundaries of the source drawable are specified
and the destination is a pixmap,
then 
those regions are not copied.
>>STRATEGY
For all pixmaps
	make src and draw into it (background of 0)
	make dest
	copy region that extends off the edge of the source drawable
	verify that it is not copied.
>>CODE
Window	w1, w2;
XVisualInfo	*vp;
Pixmap	pm;
struct	area	area;

	for (resetvinf(VI_PIX); nextvinf(&vp); ) {
		winpair(display, vp, &w1, &w2);

		pattern(display, w1);

		src = w1;
		dest = w2;

		/*
		 * Use a width of the size of the window minus TRIM, therefore
		 * it will extend off the edge of the window by width-src_x pixels.
		 */
		getsize(display, w1, (unsigned int*)&width, (unsigned int*)0);
		src_x = width-XOFF;
		width -= TRIM;

		dest_x = 0;
		dest_y = 0;

		gc = makegc(display, dest);

		XCALL;

		area.x = dest_x;
		area.y = dest_y;
		area.width = width;
		area.height = height;

		if (checkarea(display, dest, &area, 0, W_BG, CHECK_OUT) == False) {
			report("window modified outside the target area");
			FAIL;
		} else
			CHECK;

		area.x = XOFF;
		area.width = width - XOFF - TRIM;

		if (checkarea(display, dest, &area, W_BG, 0, CHECK_IN) == False) {
			report("area corresponding to uncopied area was modified");
			FAIL;
		} else
			CHECK;

	}
	CHECKPASS(2*nvinf());

>>ASSERTION Good A
When regions of the source rectangle are obscured and have not been
retained in backing store 
or regions outside the boundaries of the source drawable are specified
and the destination is a window with a background other than
.S None ,
then all corresponding destination regions that are either
visible or are retained in backing store
are tiled with that background
with plane-mask of all ones and the
.S GXcopy 
function.
>>STRATEGY
For all visuals
	make src window and draw into it (background of 0)
	make dest window with background of 1 and draw into it
	copy region that extends off the edge of the source drawable
	verify that it is not copied.
	verify that the background is tiled in parts corresponding to uncopied area.
>>CODE
Window	w1, w2;
XVisualInfo	*vp;
Pixmap	pm;
struct	area	area;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		winpair(display, vp, &w1, &w2);

		pattern(display, w1);

		src = w1;
		dest = w2;

		/*
		 * Set the destination background pixmap.
		 */
		pm = maketile(display, w2);
		XSetWindowBackgroundPixmap(display, w2, pm);

		/*
		 * Use a width of the size of the window minus TRIM, therefore
		 * it will extend off the edge of the window by width-src_x pixels.
		 */
		getsize(display, w1, (unsigned int*)&width, (unsigned int*)0);
		src_x = width-XOFF;
		width -= TRIM;

		dest_x = 0;
		dest_y = 0;

		gc = makegc(display, dest);

		XCALL;

		area.x = dest_x;
		area.y = dest_y;
		area.width = width;
		area.height = height;

		if (checkarea(display, dest, &area, 0, W_BG, CHECK_OUT) == False) {
			report("window modified outside the target area");
			FAIL;
		} else
			CHECK;

		area.x = XOFF;
		area.width = width - XOFF - TRIM;

		if (checktile(display, dest, &area, 0, 0, pm) == False) {
			report("area was not copied properly");
			FAIL;
		} else
			CHECK;

	}
	CHECKPASS(2*nvinf());

>>ASSERTION Good A
When 
.M graphics-exposures 
is 
.S True , 
then
.S GraphicsExpose 
events for all corresponding destination regions are generated.
>>STRATEGY
For all visuals
	Set graphics-exposures to True.
	Copy area that extends beyond the boundries of the source window.
	Enable All Events.
	Check events received, and x,y,width,height in each.
>>CODE
Window	w1, w2;
XEvent	event;
XVisualInfo	*vp;
XGraphicsExposeEvent	ge;
int 	n;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {

		winpair(display, vp, &w1, &w2);

		src = w1;
		dest = w2;

		getsize(display, w1, (unsigned int*)&width, (unsigned int*)0);

		src_x = XOFF;
		dest_x = 0;

		gc = makegc(display, dest);
		XSetGraphicsExposures(display, gc, True);

		XSelectInput(display, src, ALLEVENTS);
		XCALL;
		XSelectInput(display, dest, NoEventMask);

		if (getevent(display, &event) != 1 || event.type != GraphicsExpose) {
			report("Expecting one GraphicsExpose event");
			FAIL;
			return;
		}

		ge.type = GraphicsExpose;
		ge.display = display;
		ge.drawable = dest;
		ge.x = dest_x+width-XOFF;
		ge.y = dest_y;
		ge.width = XOFF;
		ge.height = height;
		ge.count = 0;
		ge.major_code = X_CopyArea;
		ge.minor_code = 0;

		n = checkevent((XEvent*)&ge, (XEvent*)&event);
		if (n == 0)
			CHECK;
		else {
			report("error in %d field%s of event", n, (n!=1)?"s":"");
			tet_result(TET_FAIL);
		}
	}

	CHECKPASS(nvinf());

>>ASSERTION Good A
When 
.M graphics-exposures 
is 
.S True 
and no
.S GraphicsExpose 
events are generated, then a
.S NoExpose 
event is generated.
>>STRATEGY
For each visual
  Enable graphics-exposures.
  Enable all events.
  Copy area that is fully within the window.
  Verify that one event is received and that it is a NoExpose event.
>>CODE
Window	w1, w2;
XEvent	event;
XNoExposeEvent	ge;
XVisualInfo	*vp;
int 	n;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		winpair(display, vp, &w1, &w2);

		src = w1;
		dest = w2;

		gc = makegc(display, dest);

		XSelectInput(display, src, ALLEVENTS);
		XCALL;
		XSelectInput(display, dest, NoEventMask);

		if (getevent(display, &event) != 1 || event.type != NoExpose) {
			report("Expecting one NoExpose event");
			FAIL;
			return;
		}

		ge.type = NoExpose;
		ge.display = display;
		ge.drawable = dest;
		ge.major_code = X_CopyArea;
		ge.minor_code = 0;

		n = checkevent((XEvent*)&ge, (XEvent*)&event);
		if (n == 0)
			CHECK;
		else {
			report("error in %d field%s of event", n, (n!=1)?"s":"");
			tet_result(TET_FAIL);
		}
	}
	CHECKPASS(nvinf());

>>ASSERTION gc
On a call to xname the GC components
.M function ,
.M plane-mask ,
.M subwindow-mode ,
.M graphics-exposures ,
.M clip-x-origin ,
.M clip-y-origin ,
and
.M clip-mask
are used.
>>ASSERTION Bad A
.ER BadMatch inputonly
>>ASSERTION Bad A
.ER BadMatch gc-drawable-depth
>>ASSERTION Bad A
.ER BadMatch gc-drawable-screen
>>ASSERTION Bad A
.ER BadDrawable
>>ASSERTION Bad A
.ER BadGC
>># HISTORY steve Completed	Written in new format and style
>># HISTORY kieron Completed	Global and pixel checking to do - 19/11/90
>># HISTORY dave Completed	Final checking to do - 21/11/90
