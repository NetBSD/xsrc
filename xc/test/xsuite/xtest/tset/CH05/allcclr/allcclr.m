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
 * $XConsortium: allcclr.m,v 1.8 94/04/17 21:03:41 rws Exp $
 */
>>TITLE XAllocColor CH05
Status
XAllocColor(display, colormap, screen_in_out)
Display *display = Dsp;
Colormap colormap = DefaultColormap(display, DefaultScreen(display));
XColor *screen_in_out = &dummy;
>>EXTERN
XColor dummy;
>>ASSERTION Good A
A call to xname 
allocates a read-only entry in the
.A colormap
argument
corresponding to the closest RGB value that is supported by the hardware to the
value specified by the 
.M red ,
.M green ,
and
.M blue
components in the
.S XColor
structure named by the 
.A screen_in_out
argument, returns that value in the 
.M red ,
.M green ,
and
.M blue
components in the
.S XColor
structure named by the 
.A screen_in_out
argument 
and returns the colourmap entry in the
.M pixel
component in the 
.S XColor
structure named by the 
.A screen_in_out
argument.
>>STRATEGY
For each supported visual class:
  Create a colormap with XCreateColormap.
  Create a new client with XOpenDisplay.
  Allocate one read-only colorcell with XAllocColor with the second client.
  Reallocate a cell with the returned RGB values with the original client.
  Verify that the returned pixel is identical to the first and that the r, g and b value argee.
  Verify  the cell is read only with the failure of XStoreColor.
>>CODE
Display	*disp2;
XVisualInfo *vp;
Status status;
XColor screencol, testcol;
unsigned long vmask;

	if( (vmask = visualsupported(display, 0L)) == 0L) {
		delete("No visuals reported as valid.");
		return;
	}

	if( (disp2 = opendisplay()) == (Display *) 0) {
		delete("Could not open display");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		trace("Attempting AllocColor() for class %s", displayclassname(vp->class));
		colormap = makecolmap(display, vp -> visual, AllocNone);

		testcol.red = 0xf3f3;
		testcol.green = 0xe4e4;
		testcol.blue = 0xd5d5;
		
		trace("Source colour :  r %u g %u b %u", testcol.red, testcol.green, testcol.blue);
		screen_in_out = &testcol;
		display = disp2;
		status = XCALL;

		if( status == (Status) 0) {
			report("XAllocColor() failed to return non-zero.");
			FAIL;
			continue;
		}

		screencol = testcol;
		trace("Screen: pixel %lu, r %u g %u b %u", screencol.pixel , screencol.red, screencol.green, screencol.blue);
		trace("Test : pixel %lu, r %u g %u b %u", testcol.pixel, testcol.red, testcol.green, testcol.blue);

		screen_in_out = &testcol;
		display = Dsp; 
		status = XCALL;
	
		if(status == (Status) 0) {
			report("XAllocColor() failed to return non-zero.");
			FAIL;
			continue;	
		}

		if( screencol.pixel != testcol.pixel ) {
			report("XAllocColor() return pixel value %lu instead of %lu.", testcol.pixel, screencol.pixel);
			FAIL;
		} else
			CHECK;

		if( (screencol.red != testcol.red) || (screencol.green != testcol.green) || (screencol.blue != testcol.blue) ) {
			report("XAllocColor() return RGB values r %u g %u b %u instead of r %u g %u b %u.", 
				testcol.red, testcol.green, testcol.blue, screencol.red, screencol.green, screencol.blue);
			FAIL;
		} else
			CHECK;
	
		startcall(display);
		XStoreColor(display, colormap, &testcol);
		endcall(display);

		if (geterr() == Success) {
			report("Got success writing to a read-only cell");
			tet_result(TET_FAIL);
		} else
			CHECK;

	}

	CHECKPASS(3 * nsupvis());


>>ASSERTION Good A
A call to xname does not use or affect the 
.S flags
component of the
.S XColor 
structure named by the
.A screen_in_out
argument.
>>STRATEGY
For each supported visual class:
  Create a colormap with XCreateColormap.
  Allocate a colourcell with an XColor structure flags component of DoGreen,
   and red, green and blue components of 255<<8, 63<<8 and 31 <<8.
  Verify that the returned flags component is DoGreen.
  Allocate a colourcell with an XColor structure flags component of DoRed | DoBlue
   and red, green and blue components of 255<<8, 63<<8 and 31 <<8.
  Verify that the returned flags component is DoRed | DoBlue.
  Verify that the returned pixel and red, green and blue values are identical to
   those initially returned.
>>CODE
XVisualInfo *vp;
Status status;
XColor screencol, testcol;
unsigned long vmask;

	if( (vmask = visualsupported(display, 0L)) == 0L) {
		delete("No visuals reported as valid.");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		trace("Attempting AllocColor() for class %s", displayclassname(vp->class));
		colormap = makecolmap(display, vp -> visual, AllocNone);

		testcol.red = 255 << 8;
		testcol.green = 63 << 8;
		testcol.blue = 31 << 8;
		testcol.flags = DoGreen;		

		screen_in_out = &testcol;
		status = XCALL;

		if( status == (Status) 0) {
			report("XAllocColor() failed to return non-zero.");
			FAIL;
			continue;
		}

		if( DoGreen != testcol.flags ) {
			report("XAllocColor() altered flags value %lu to %lu.",
					 DoGreen, testcol.flags);
			FAIL;
		} else
			CHECK;

		screencol = testcol;
		trace("Screen: pixel %lu, r %u g %u b %u", screencol.pixel , screencol.red, screencol.green, screencol.blue);
		trace("Test : pixel %lu, r %u g %u b %u", testcol.pixel, testcol.red, testcol.green, testcol.blue);

		testcol.red = 255 << 8;
		testcol.green = 63 << 8;
		testcol.blue = 31 << 8;
		testcol.flags = DoRed | DoBlue;

		screen_in_out = &testcol;
		status = XCALL;
	
		if(status == (Status) 0) {
			report("XAllocColor() failed to return non-zero.");
			FAIL;
			continue;	
		}

		if( (DoRed | DoBlue) != testcol.flags ) {
			report("XAllocColor() altered flags value %lu to %lu.",
					 (DoRed | DoBlue), testcol.flags);
			FAIL;
		} else
			CHECK;

		if( screencol.pixel != testcol.pixel ) {
			report("XAllocColor() return pixel value %lu instead of %lu.", testcol.pixel, screencol.pixel);
			FAIL;
		} else
			CHECK;

		if( (screencol.red != testcol.red) || (screencol.green != testcol.green) || (screencol.blue != testcol.blue) ) {
			report("XAllocColor() return RGB values r %u g %u b %u instead of r %u g %u b %u.", 
				testcol.red, testcol.green, testcol.blue, screencol.red, screencol.green, screencol.blue);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(4 * nsupvis());



>>ASSERTION Good A
When a call to xname has allocated a read-only entry in a colourmap,
then a subsequent call to xname with identical
.M red ,
.M green
and
.M blue
components in the
.S XColor
structure named by the
.A screen_in_out
argument returns an identical pixel component in the
.S XColor
structure.
>>STRATEGY
For each supported visual class:
  Create a colormap with XCreateColormap.
  Allocate a colourcell with an XColor structure with
   red, green and blue components of 255<<8, 127<<8 and 63 <<8.
  Allocate a colourcell with an XColor structure having the
   the previously returned rgb values.
  Verify that the returned pixel is the same as that initially returned.
>>CODE
XVisualInfo *vp;
Status status;
XColor screencol, testcol;
unsigned long vmask;

	if( (vmask = visualsupported(display, 0L)) == 0L) {
		delete("No visuals reported as valid.");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		trace("Attempting AllocColor() for class %s", displayclassname(vp->class));
		colormap = makecolmap(display, vp -> visual, AllocNone);

		testcol.red = 255 << 8;
		testcol.green = 127 << 8;
		testcol.blue = 63 << 8;

		screen_in_out = &testcol;
		status = XCALL;

		if( status == (Status) 0) {
			report("XAllocColor() failed to return non-zero.");
			FAIL;
			continue;
		}

		screencol = testcol;
		trace("Screen: pixel %lu, r %u g %u b %u", screencol.pixel , screencol.red, screencol.green, screencol.blue);
		trace("Test : pixel %lu, r %u g %u b %u", testcol.pixel, testcol.red, testcol.green, testcol.blue);

		screen_in_out = &testcol;
		status = XCALL;
	
		if(status == (Status) 0) {
			report("XAllocColor() failed to return non-zero.");
			FAIL;
			continue;	
		}

		if( screencol.pixel != testcol.pixel ) {
			report("XAllocColor() return pixel value %lu instead of %lu.", testcol.pixel, screencol.pixel);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(nsupvis());

>>ASSERTION Bad A
.ER BadColor
>>ASSERTION Bad A
.ER Alloc
>>#HISTORY	Cal	Completed	Written in new format and style.	4/12/90
>>#HISTORY	Kieron	Completed		<Have a look>
>>#HISTORY	Cal	Action		Writting code.
