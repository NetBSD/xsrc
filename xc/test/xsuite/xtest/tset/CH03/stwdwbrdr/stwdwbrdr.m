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
 * $XConsortium: stwdwbrdr.m,v 1.8 94/04/17 21:03:19 rws Exp $
 */
>>TITLE XSetWindowBorder CH03
void

Display	*display = Dsp;
Window	w;
unsigned long	border_pixel;
>>EXTERN
static Window	parent;
static struct	area	ap;

static void
inittp()
{
	tpstartup();

	ap.x = 50;
	ap.y = 60;
	ap.width = 20;
	ap.height= 20;
}
>>SET tpstartup inittp
>>ASSERTION Good A
A call to xname sets the border of the window to the pixel value
specified by
.A border_pixel .
>>STRATEGY
Create a window, with a wide, tiled, border.
Set the border-pixel by calling xname.
Unmap then remap the window, to ensure the border change occurs.
Get the border-pixel.
Verify the border pixel was set.
Reset the border-pixel by calling xname.
>># The unmap/remap is to allow this test to pass, even if the borders
>># are not being repainted as soon as the pixel is changed.		stu
Unmap then remap the window, to ensure the border change occurs.
Get the border-pixel.
Verify the border pixel was set.
>>CODE
unsigned long	pixel;
Pixmap	pm;
XEvent	event;

	parent = defdraw(display, VI_WIN);

	w = creunmapchild(display, parent, &ap);

	XSetWindowBorderWidth(display, w, 5);
	XSetWindowBorderPixmap(display, w, maketile(display,w));

	XSelectInput(display, w, ExposureMask);	
	XMapWindow(display, w);
	XWindowEvent(display,w,ExposureMask, &event); /* Await exposure */

	pixel = getpixel(display, parent, ap.x, ap.y);
	border_pixel = (pixel == W_FG) ? W_BG:W_FG;
	trace("pixel at ap.x,ap.y: %d . Setting to %d", pixel, border_pixel);

	XCALL;
	XUnmapWindow(display, w);
	XMapWindow(display, w);
	XWindowEvent(display,w,ExposureMask, &event); /* Await exposure */

	if(getpixel(display, parent, ap.x, ap.y) != border_pixel) {
		report("%s did not set the border pixel to %d",
				TestName, border_pixel);
		FAIL;
	} else
		CHECK;

	border_pixel = pixel;
	trace("Now setting border to %d", border_pixel);

	XCALL;
	XUnmapWindow(display, w);
	XMapWindow(display, w);
	XWindowEvent(display,w,ExposureMask, &event); /* Await exposure */

	if(getpixel(display, parent, ap.x, ap.y) != border_pixel) {
		report("%s did not set the border pixel to %d",
				TestName, border_pixel);
		FAIL;
	} else
		CHECK;
	
	CHECKPASS(2);
>>ASSERTION Good A
When the border pixel value is changed, then the border is repainted.
>>STRATEGY
Create a window with a wide, tiled border.
Set the border-pixel by calling xname.
Unmap then remap the window, to ensure the border change occurs.
Get the border-pixel.
Verify the border pixel was set.
>>CODE
unsigned long	pixel;
Pixmap	pm;
XEvent	event;

/* Create windows */
	parent = defdraw(display, VI_WIN);
	w = creunmapchild(display, parent, &ap);
/* Set up child window border for test */
	XSetWindowBorderWidth(display, w, 5);
	XSetWindowBorderPixmap(display, w, maketile(display,w));
/* Ensure the window is mapped and visible */
	XSelectInput(display, w, ExposureMask);	
	XMapWindow(display, w);
	XWindowEvent(display,w,ExposureMask, &event); /* Await exposure */

	pixel = getpixel(display, parent, ap.x, ap.y);
	border_pixel = (pixel == W_FG) ? W_BG:W_FG;
	trace("pixel at ap.x,ap.y: %d . Setting to %d", pixel, border_pixel);

	XCALL;
	
	if(getpixel(display, parent, ap.x, ap.y) != border_pixel) {
		report("%s did not cause a repaint of the border", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
The border pixel value is truncated to the depth of the window.
>>STRATEGY
For each visual
  Set border-pixel to various values.
  Read one pixel back from the border.
  Verify that this pixel has been truncated to depth of window.
>>CODE
XVisualInfo	*vp;
static int 	pixlist[] = {
	0, 1, 3, 4, 17, 18, 200, 300, 303,
	0x1234, 0x12345, 0x123456, 0x1234567, 0x12345678};
long	pix;
long	borderpix;
int 	i;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		parent = makedrawable(display, vp);

		for (i = 0; i < NELEM(pixlist); i++) {
			pix = pixlist[i];

			border_pixel = pix;

			w = crechild(display, parent, &ap);
			XSetWindowBorderWidth(display, w, 3);

			XCALL;

			borderpix = getpixel(display, parent, ap.x, ap.y);
			if (borderpix == (pix & DEPTHMASK(vp->depth)))
				CHECK;
			else {
				report("Border pixel was not truncated (value 0x%x)", pix);
				report("  Was 0x%x, expecting 0x%x", borderpix,
					pix & DEPTHMASK(vp->depth));
				FAIL;
			}
		}
	}

	CHECKPASS(nvinf() * NELEM(pixlist));

>>ASSERTION Bad A
.ER BadMatch wininputonly
>>ASSERTION Bad A
.ER BadWindow 
