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

 * Copyright 1990, 1991 UniSoft Group Limited.
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
 * $XConsortium: unnstllclr.m,v 1.5 94/04/17 21:07:06 rws Exp $
 */
>>TITLE XUninstallColormap CH07
void

Display	*display = Dsp;
Colormap	colormap;
>>ASSERTION Good B 3
A call to xname removes the specified colourmap from the required
list for its associated screen.
>>ASSERTION Good B 1
When the specified colourmap is uninstalled by a call to xname, then a
.S ColormapNotify
event is generated on each window that has that colourmap.
>>STRATEGY
Create colour map.
Install colour map.
Create window with the colour map.
Call xname to uninstall the colourmap.
If there is a ColormapNotify event
  Verify that the fields are correct.
else
  UNTESTED.
>>CODE
XVisualInfo	*vp;
Window	win;
XEvent	ev;
XColormapEvent	good;
XColormapEvent	*cmp;

	defsetevent(good, display, ColormapNotify);
	good.new = False;
	good.state = ColormapUninstalled;

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {

		win = makewin(display, vp);
		colormap = makecolmap(display, vp->visual, AllocNone);
		XSetWindowColormap(display, win, colormap);
		XSelectInput(display, win, ColormapChangeMask);
		XInstallColormap(display, colormap);

		XCALL;

		while (XCheckWindowEvent(display, win, ColormapChangeMask, &ev)) {
			cmp = (XColormapEvent*)&ev;
			if (cmp->colormap != colormap)
				continue;

			good.window = win;
			good.colormap = colormap;
			if (checkevent((XEvent*)&good, &ev))
				FAIL;
			else
				CHECK;

		}
	}

	if (fail == 0 && pass == nvinf())
		PASS;
	else if (fail == 0 && pass == 0)
		untested("The assertion could not be tested since no event was sent");
	else if (fail == 0)
		untested("The assertion could only be tested for some visual types");

>>ASSERTION Good B 1
When another colourmap is installed or uninstalled as a
side effect of a call to xname, then a
.S ColormapNotify
event is generated on each window that has that colourmap.
>>ASSERTION Good B 3
No other colourmaps are removed from the required list by a call to xname.
>>ASSERTION Bad A
.ER BadColor
