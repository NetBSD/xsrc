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
 * $XConsortium: cpyclrmpan.m,v 1.13 94/04/17 21:03:50 rws Exp $
 */
>>TITLE XCopyColormapAndFree CH05
Colormap
XCopyColormapAndFree(display, colormap)
Display *display = Dsp;
Colormap colormap = DefaultColormap(display, DefaultScreen(display));
>>ASSERTION Good A
A call to xname creates a colourmap of the same visual type and for the same screen as the
.A colormap
argument and returns the new colourmap ID.
>>STRATEGY
For each supported visual:
   Create a colourmap using XCreateColormap with alloc set to AllocNone.
   Allocate one r/o cell, get it's actual rgb values
   Allocate the rest of the cmap as r/w and store known values, read them back.
   Create a new colourmap with XCopyColormapAndFree.
   Check that it contains the values we know we had in the original.
   Verify that no error occurred.

>>CODE
Colormap	testcmap;
XVisualInfo	*vp;
unsigned long 	vmask = (1<<DirectColor)|(1<<PseudoColor)|(1<<GrayScale);
XColor		*cellmap, *cellptr;
XColor		cell;
unsigned long	l;
unsigned int	i;
int		pathcnt = 0, size;
XColor		color, color1, color2, ncol1, ncol2;
Display		*disp2;


	if( (vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

	color1.pixel = 0xffff;
	color1.red = 0x0ff0;
	color1.green = 0xf0f0;
	color1.blue = 0x0f0f;
	color1.flags = DoRed|DoGreen|DoBlue;

	color2.pixel = 1;
	color2.red = 0xffff;
	color2.green = 0xffff;
	color2.blue = 0xffff;
	color2.flags = DoRed|DoGreen|DoBlue;

	for(resetsupvis(vmask);  nextsupvis(&vp); ) {

		colormap = makecolmap(display, vp->visual, AllocNone);
		size = maxsize(vp);		

		cellmap = (XColor *) malloc(size * sizeof(XColor));

		if (cellmap == (XColor *)NULL) {
			delete("malloc failure");
			return;
		} else
			CHECK;

		*cellmap = color1;

		if( XAllocColor(display, colormap, cellmap) == False ) {
			delete("XAllocColor() failed to allocate a r/o colourcell");
			return;
		}
		XQueryColor(display, colormap, cellmap);

		for(i=1, cellptr = cellmap+1; i < size; i++, cellptr++) {
			if(XAllocColorCells(display, colormap, False, 0, 0L, &(cellptr->pixel), 1) == False ) {
				delete("XAllocColorCells() failed to allocate a r/w colourcell (%d)",i);
				return;
			}
			cellptr->red = 0xffff;
			cellptr->green = 0xf00f;
			cellptr->blue = 0x0ff0;
			cellptr->flags = DoRed|DoGreen|DoBlue;
			XStoreColor(display, colormap, cellptr);
			XQueryColor(display, colormap, cellptr);
		}
		if(i == size)
			CHECK;

		testcmap = XCALL;
			
		for(i=0, cellptr = cellmap; i< size; i++, cellptr++) {
			cell.pixel = cellptr->pixel;
			cell.flags = DoRed|DoGreen|DoBlue;
			XQueryColor(display, testcmap, &cell);
			if( (cell.red != cellptr->red) ||
			    (cell.green != cellptr->green) || (cell.blue != cellptr->blue) ){
				report("Copied colourmap cell pixel %ld had r %u g %u b %u instead of pixel %ld r %u g %u b %u",
					cell.pixel, cell.red, cell.green, cell.blue,
					cellptr->pixel, cellptr->red, cellptr->green, cellptr->blue);
				FAIL;
			}

			if(i==size)
				CHECK;
		}
	}
	
	CHECKPASS(2*nsupvis());

>>ASSERTION Good C
When the
.A colormap
argument
was created by the client with
.A alloc
set to
.S AllocAll ,
then all entries from the
argument
.A colormap
are moved to the new colourmap with the same colour values
and are freed
in 
.A colormap .
>>STRATEGY
For each supported visual class:
  Create a colormap with using XCreateColormap with alloc set to AllocAll.
  Fill with recognisable values.
  For each of the colour cells, record the rgb value with XQueryColors.
  Create a new colourmap with XCopyColormapAndFree.
  Verify that colourmap entries are identical with XQueryColors.
  Verify that all the previous cells in the original colormap are freed (i.e.
	that they can all be reallocated again).
>>CODE
Colormap	testcmap;
XVisualInfo	*vp;
unsigned long 	vmask = (1<<DirectColor)|(1<<PseudoColor)|(1<<GrayScale);
XColor		*cellmap, *cellptr;
XColor		cell;
unsigned long	l;
int		i;
int		pathcnt = 0;
int 	size;

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

	for (resetsupvis(vmask);  nextsupvis(&vp); ) {

		colormap = makecolmap(display, vp->visual, AllocAll);
		size = maxsize(vp);
		cellmap = (XColor *) malloc( (size * sizeof(XColor)));
		if(cellmap == (XColor *) 0) {
			delete("malloc() failed to allocate memory for XColor array.");
			return;
		}

/* fill colour map with some recognisable and distinguishable values */
		for(l=0, cellptr=cellmap; l< size; l++, cellptr++) {
			cellptr->pixel = l;
			cellptr->green = cellptr->blue = cellptr->red = l << 8;
			cellptr->flags = DoRed | DoGreen | DoBlue;
		}
		XStoreColors(display, colormap, cellmap, size);

/* server sets to "closest available values" so find what they were */
		XQueryColors(display, colormap, cellmap, size);

		testcmap = XCALL;

		pathcnt += size;

		for(l=0, cellptr=cellmap; l < size; l++, cellptr++) {
			cell.pixel = l;
			XQueryColor(display, testcmap, &cell);
			if((cell.pixel != cellptr->pixel) || (cell.red != cellptr->red) ||
			   (cell.green != cellptr->green) || (cell.blue != cellptr->blue)) { 
				report("Cell %lu had pixelvalue %lu (r %u g %u b %u) instead of pixelvalue %lu (r %u g %u b %u)",
						l,
						cell.pixel, cell.red, cell.green, cell.blue,
						cellptr->pixel, cellptr->red, cellptr->green, cellptr->blue);
				FAIL;
			} else 
				CHECK;
		}

		for(l=0; l < size; l++) {
			if (XAllocColorCells(display, colormap, False, NULL, 0, &cell.pixel, 1) == False) {
				report("Cell %u was not deallocated.", l);
				FAIL;
			} else
				CHECK;
		}

		free(cellmap);
	}

	CHECKPASS(pathcnt*2);

>>ASSERTION Good C
If any of the visual classes
.S PseudoColor ,
.S GrayScale ,
or
.S DirectColor
are supported:
When the
.A colormap
argument
was created by the client with
.S AllocNone ,
then all of the entries from the
.A colormap
argument
that have been
allocated by the client using
.S XAllocColor ,
.S XAllocNamedColor ,
.S XAllocColorCells ,
or
.S XAllocColorPlanes
and not freed since they were allocated 
are moved to the new colourmap with the same colour values 
and the same read-only or writable characteristics and are freed
in 
.A colormap .
>>STRATEGY
For each of the visual classes DirectColor, PseudoColor and GrayScale:
  Create a colormap with XCreateColormap and AllocNone.
  Create a  new client with XOpenDisplay.
  Allocate two ro cells for First client with XAllocColor.
  Record their rgb values.
  Allocate two rw cells for First client with XAllocColorCells.
  Set their rgb values and record their rgb values.
  Allocate the rest of the colormap for Second client  with XAllocColorCells.
  First client copies the colormap and free owned cells with with XCopyColormapAndFree.
  Verify that the Second clients cells in copy are not allocated with XAllocColorCells.
  Verify that the First clients four cells in copy are still there with correct rgb values.
  Verify that the four allocated cells are freed, can be reallocated, in old cmap.
  Verify r/o and r/w behaviour of these 4 in copy, with XStoreColors.
  
>>EXTERN
static Bool check_rgb(dpy, cmap, xcp)
	Display *dpy;
	Colormap cmap;
	XColor *xcp;
{
	XColor ncol;

	ncol.pixel = xcp->pixel;
	ncol.flags = DoRed | DoGreen | DoBlue;
	XQueryColor(dpy, cmap, &ncol);

	if ( ncol.red != xcp->red || ncol.green != xcp->green || ncol.blue != xcp->blue ) {
			report("mismatch, got rgb = 0x%x,0x%x,0x%x instead of 0x%x,0x%x,0x%x for pixel %lu",
				ncol.red, ncol.green, ncol.blue,
				xcp->red, xcp->green, xcp->blue, xcp->pixel);
			return False;
	} else
		return True;
}
>>CODE
Colormap	testcmap;
XVisualInfo	*vp;
unsigned long 	vmask = (1<<DirectColor)|(1<<PseudoColor)|(1<<GrayScale);
XColor		*cellmap, *cellptr;
XColor		cell;
unsigned long	l;
int	i;
int		pathcnt = 0, size;
XColor		color, ncol1, ncol2, cols[4];
Display		*disp2;
unsigned long	pix[2];
unsigned long	*copypix;

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

	cols[0].pixel = 0xffff;
	cols[0].red = 0x0ff0;
	cols[0].green = 0xf0f0;
	cols[0].blue = 0x0f0f;
	cols[0].flags = DoRed|DoGreen|DoBlue;

	cols[1].pixel = 1;
	cols[1].red = 0xffff;
	cols[1].green = 0xffff;
	cols[1].blue = 0xffff;
	cols[1].flags = DoRed|DoGreen|DoBlue;

	for(resetsupvis(vmask);  nextsupvis(&vp); ) {

		disp2 = opendisplay();
		colormap = makecolmap(display, vp->visual, AllocNone);
/* Allocate 2 r/o cells for a new client */
/* set them and find what they ended up as */
		if (XAllocColor(display, colormap, &cols[0]) == False) {
			delete("failed to allocate first r/o cell.");
			return;
		} else
			CHECK;
		trace("r/o Pixel value allocated was %lu", cols[0].pixel);
		XQueryColor(display, colormap, &cols[0]);

		if (XAllocColor(display, colormap, &cols[1]) == False) {
			delete("failed to allocate second r/o cell.");
			return;
		} else
			CHECK;
		trace("r/o Pixel value allocated was %lu", cols[1].pixel);
		XQueryColor(display, colormap, &cols[1]);
		
		size = maxsize(vp) - 4;
		if (size >= 0) {
/* Allocate 2 r/w cells for a new client */
			if(XAllocColorCells(display, colormap, False, 0L, 0, pix, 2) == False) {
				delete("XAllocColorCells() failed.");
				return;
			}
/* set the 2 r/w cells to the same values as the first two. */
			cols[2] = cols[0];
			cols[2].pixel = pix[0];
			cols[3] = cols[1];
			cols[3].pixel = pix[1];
			trace("r/w Pixel values allocated were %lu & %lu",
				cols[2].pixel, cols[3].pixel);
			XStoreColors(display, colormap, &cols[2], 2);
			XQueryColors(display, colormap, &cols[2], 2);
			CHECK;
		} else
			CHECK;

/* Allocate remaining cells to the other client */
		debug(1, "size=%d, maxsize=%d", size, maxsize(vp));
		copypix = (unsigned long *) malloc( maxsize(vp) * sizeof(unsigned long));

		for(i=0; i < size; i++) {
			if( XAllocColorCells(disp2, colormap, False, 0L, 0, &copypix[i], 1) == False) {
				delete("XAllocColorCells failed after %d cells", i);
				return;
			}
		}

		if( XAllocColorCells(disp2, colormap, False, 0L, 0, &copypix[i], 1) != False) {
			trace("Did not fail as expected");
		} else
			CHECK;

/* do copy and free with first client */
		testcmap = XCALL;

/* check remaining ones are free in copy */
		for ( i=0; XAllocColorCells(disp2, testcmap, False, 0L, 0, &copypix[i], 1) != False; i++) {
			; /* do nothing */
		}
		if ( (size >= 0 && i != size) || (size < 0 && i > 0) ) {
			report("allocated %d cells instead of %d, in copy",
					i, (size >= 0) ? size : 0);
			FAIL;
		} else
			CHECK;

/* now check the first four cells of copy are as we set them in original. */

		if (!check_rgb(disp2, testcmap, &cols[0]) |
			!check_rgb(disp2, testcmap, &cols[1]) |
			(size >= 0 && !check_rgb(disp2, testcmap, &cols[2])) |
			(size >= 0 && !check_rgb(disp2, testcmap, &cols[3])) ) {
				report("RGB values not the same!");
				FAIL;
		} else
			CHECK;
/* should check that can now allocate 4 more in old cmap */
		for(i=0; i < 4; i++) {
			if (size < 0 && i > 1) {
				CHECK;
			}
			else if( XAllocColorCells(disp2, colormap, False, 0L, 0, &copypix[i], 1) == False) {
				report("Could not allocate the supposedly freed cell (%d) in original cmap", i);
				FAIL;
			} else {
				trace("allocated cell %d, pixel %lu", i, copypix[i]);
				CHECK;
			}
		}

		if( XAllocColorCells(disp2, colormap, False, 0L, 0, &copypix[i], 1) != False) {
			report("Allocated one more cell in original cmap than expected");
			FAIL;
		} else
			CHECK;
/* chould check that the r/w or r/o characteristics are preserved in copy */
		startcall(disp2);
		XStoreColors(disp2, testcmap, cols, 2);
		endcall(disp2);
		if (geterr() != BadAccess) {
			report("Trying to update 2 r/o cells. Got %s, Expecting BadAccess", errorname(geterr()));
			FAIL;
		} else
			CHECK;

		if (size >= 0) {
			startcall(disp2);
			XStoreColors(disp2, testcmap, &cols[2], 2);
			endcall(disp2);
			if (geterr() != Success) {
				report("Trying to update 2 r/w cells. Got %s, Expecting Success", errorname(geterr()));
				FAIL;
			} else
				CHECK;
		} else
			CHECK;
	}
	
	CHECKPASS(13*nsupvis());

>>ASSERTION Bad B 1
.ER BadAlloc
>>ASSERTION Bad A
.ER BadColor
>>#HISTORY	Cal	Completed	Written in new format and style - 3/11/90.
>>#HISTORY	Kieron	Completed	<Have a look>
>>#HISTORY	Cal	Completed	Writing code.
>>#HISTORY	Kieron	Completed	re-writing code and strategy.
