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
 * $XConsortium: qryclrs.m,v 1.11 94/04/17 21:04:03 rws Exp $
 */
>>TITLE XQueryColors CH05
void
XQueryColors(display, colormap, defs_in_out, ncolors)
Display	*display = Dsp;
Colormap colormap = DefaultColormap(display, DefaultScreen(display));
XColor	*defs_in_out = &dummycol;
int	ncolors;
>>EXTERN
XColor	dummycol;
>>ASSERTION Good A
A call to xname obtains the RGB values of the colourmap entries specified by the
.M pixel
components of the
.S XColor
structures named by the
.A defs_in_out
argument, and returns the RGB values in the
.M red ,
.M green ,
and
.M blue
components and sets the
.M flags
components to the bitwise OR of
.S DoRed ,
.S DoGreen ,
and
.S DoBlue .
>>STRATEGY
For each supported visual class :
  Create a colormap with XCreateColormap with alloc = AllocNone.
  Allocate a new r/o colourmap cell with XAllocColor.
  Obtain the rgb values and flags components with XQueryColors using the pixel returned by XAllocCOlor.
  Verify that the returned flags component was set to DORed|DoGreen|DoBlue.
  Verify that the rgb values are identical to those returned by XAllocColor.
>>CODE
XVisualInfo 	*vp;
XColor		*cellptr, *defptr, *refptr, *acptr;
int		size;
unsigned long	i;
unsigned long 	vmask = 0L;

	if((vmask = visualsupported(display, vmask)) != 0L)
		for(resetsupvis(vmask); nextsupvis(&vp); ) {
			size = vp->colormap_size;
			if (vp->class == DirectColor)
				size = maxsize(vp);
			colormap = makecolmap(display, vp->visual, AllocNone);

			defptr = (XColor *) malloc( size * sizeof(XColor));
			refptr = (XColor *) malloc( size * sizeof(XColor));
			
			if( (defptr == (XColor *) 0) || (refptr == (XColor *) 0)) {
				delete("malloc() failed to allocate space for the colour array.");
				return;
			}

			for(i=0, cellptr = defptr; i<size; i++, cellptr++) {  /* May or may not allocate the entire colourmap */
				cellptr->pixel = ~0L;
				cellptr->red = i<<8;
				cellptr->green = i<<8;
				cellptr->blue =  i<<8;
				cellptr->flags = 0;

				if(XAllocColor(display, colormap, cellptr) == False) {
					delete("XAllocColor() failed.");
					return;
				}

				refptr[i] = defptr[i];
			} 
			if(i == size)
				CHECK;

			ncolors = size;
			defs_in_out = defptr;
			XCALL;

			if(geterr() == Success)
				for(i=0, cellptr = defptr, acptr = refptr; i<size; i++, cellptr++, acptr++) {
					if(cellptr->flags != (DoRed|DoGreen|DoBlue)) {
						report("XQueryColors() did not set the flags of cell %u to (DoRed|DoGreen|DoBlue)", cellptr->pixel);
						FAIL;
					}

					if((cellptr->pixel != acptr->pixel) || (cellptr->red != acptr->red) ||
							(cellptr->green != acptr->green) || (cellptr->blue != acptr->blue)){
						report("XQueryColors() returned pixel %u  r %u g %u b %u instead of pixel %u r %u g %u b %u",
							cellptr->pixel, cellptr->red, cellptr->green, cellptr->blue,
							acptr->pixel, acptr->red, acptr->green, acptr->blue);
						FAIL;
					} 
				}
			else
				FAIL;	

			if(i == size)
				CHECK;

			free(defptr);
			free(refptr);
	}

	CHECKPASS(2*nsupvis());

>>ASSERTION Bad A
.ER BadColor
>>ASSERTION Good A
When the 
.M pixel 
component in one or more of the
.A ncolors
.S XColor
structures named by the
.A defs_in_out
argument is not a valid entry in the
.A colormap
argument, 
then a
.S BadValue
error occurs which will report any one of the
invalid pixel values.
>>STRATEGY
For each supported visual class:
  Create a colormap with XCreateColormap with alloc set to AllocNone.
  Allocate a r/o colormap cell with XAllocColor.
  Call XQueryColors with pixel array comprising pixel components 0L, -1L, -2L.
  Verify that a BadValue error occurs.
  Verify that the reported BadValue was either -1 or -2.
>>CODE BadValue
XVisualInfo 	*vp;
XColor		tcol;
XColor		qcol[3];
unsigned long	vmask;

	if((vmask = visualsupported(display, 0L)) == 0L) {
		delete("No visuals reported as valid.");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		colormap = makecolmap(display, vp->visual, AllocNone);
		XAllocColor(display, colormap, &tcol);
		defs_in_out = qcol;
		qcol[0].pixel = tcol.pixel;
		qcol[1].pixel = (unsigned long)-1L;
		qcol[2].pixel = (unsigned long)-2L;
		ncolors = 3;
		XCALL;
		if(geterr() == BadValue) {
			if((getbadvalue() != (unsigned long)-1) && (getbadvalue() != (unsigned long)-2)) {
				report("BadValue reported was neither -1 or -2");
				FAIL;
			} else
				CHECK;
		}

	}

	CHECKPASS(nsupvis());

>>#HISTORY	Cal	Completed	Written in new style and format.
>>#HISTORY	Kieron	Completed		<Have a look>
>>#HISTORY	Cal	Action		Writting code.
