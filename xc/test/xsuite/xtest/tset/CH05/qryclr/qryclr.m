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
 * $XConsortium: qryclr.m,v 1.9 94/04/17 21:04:02 rws Exp $
 */
>>TITLE XQueryColor CH05
void
XQueryColor(display, colormap, def_in_out)
Display *display = Dsp;
Colormap colormap = DefaultColormap(display, DefaultScreen(display));
XColor *def_in_out = &dummycol;
>>EXTERN
XColor dummycol;
>>ASSERTION Good A
A call to xname obtains the RGB
values of the colourmap entry specified by the
.M pixel
component of the
.S XColor 
structure named by the
.A def_in_out
argument , 
and returns the RGB values in the 
.M red ,
.M green
and
.M blue
components and sets the
.M flags
component to the bitwise OR of
.S DoRed , 
.S DoGreen 
and
.S DoBlue .
>>STRATEGY
For each supported visual class:
  Create a colormap with XCreateColormap with alloc set to AllocNone.
  Allocate a new r/o colourmap cell with XAllocColor.
  Obtain the rgb values and flags components with XQueryColor using the pixel returned by XAllocColor.
  Verify that the returned flags component was set to DORed|DoGreen|DoBlue.
  Verify that the rgb values are identical to those returned by XAllocColor.

>>CODE
XVisualInfo 	*vp;
XColor		qcol, qcolr;
unsigned long 	vmask = 0L;

	if((vmask = visualsupported(display, 0L)) == 0L) {
		delete("No visuals reported as valid.");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		colormap = makecolmap(display, vp->visual, AllocNone);
		qcol.pixel = ~0L;
		qcol.red = 0xfefe;
		qcol.green = 0xefef;
		qcol.blue = 0xfeef;
		qcol.flags = 0;

		if (XAllocColor(display, colormap, &qcol) == False) {
			delete("XAllocColor() failed.");
			return;
		} else
			CHECK;

		qcolr = qcol;
		def_in_out = &qcol;
		XCALL;

		if(geterr() == Success)
			CHECK;

		if(qcol.flags != (DoRed|DoGreen|DoBlue)) {
			report("XQueryColor() did not set the flags to (DoRed|DoGreen|DoBlue)");
			FAIL;
		} else
			CHECK;

		if((qcol.pixel != qcolr.pixel) || (qcol.red != qcolr.red) || (qcol.green != qcolr.green) || (qcol.blue != qcolr.blue)){
			report("XQueryColor() returned  r %u g %u b %u instead of r %u g %u b %u", qcol.red, qcol.green, qcol.blue, qcolr.red, qcolr.green, qcolr.blue);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(4*nsupvis());

>>ASSERTION Bad A
.ER BadColor
>>ASSERTION Bad A
When the 
.M pixel 
component in the
.S XColor
structure named by the
.A def_in_out
argument is not a valid entry in the
.A colormap
argument, 
then a
.S BadValue
error occurs.
>>STRATEGY
For each supported visual class:
  Create a colormap with XCreateColormap with alloc set to AllocNone.
  Call XQueryColor with pixel component = -1.
  Verify that a BadValue error occurs.
>>CODE BadValue  
XVisualInfo 	*vp;
XColor		qcol;
unsigned long	vmask;

	if((vmask = visualsupported(display, 0L)) == 0L) {
		delete("No visuals reported as valid.");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		colormap = makecolmap(display, vp->visual, AllocNone);
		def_in_out = &qcol;
		qcol.pixel = ~0L;
		XCALL;
		if(geterr() == BadValue)
			CHECK;
	}

	CHECKPASS(nsupvis());

  
>>#HISTORY	Cal	Completed	Written in new style and format.
>>#HISTORY	Kieron	Completed		<Have a look>
>>#HISTORY	Cal	Action		Writting code.
