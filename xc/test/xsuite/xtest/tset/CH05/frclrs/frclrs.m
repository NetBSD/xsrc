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
 * $XConsortium: frclrs.m,v 1.15 94/04/17 21:03:55 rws Exp $
 */
>>TITLE XFreeColors CH05
void
XFreeColors(display, colormap, pixels, npixels, planes)
Display *display = Dsp;
Colormap colormap = DefaultColormap(display, DefaultScreen(display));
unsigned long *pixels = &dummypix;
int npixels = 1;
unsigned long planes = 0L;
>>EXTERN
unsigned long dummypix;

#define ENOUGH_TIME	100

static
Bool munch(size_in_out, pixels, npix)
	int	*size_in_out;
	unsigned long *pixels;
	int	npix;
{
	int i;
	int sz = 0;
	unsigned long junk;
	int max_sz = (*size_in_out) * ENOUGH_TIME;

	for(i=0; i < npix; i++, pixels++) {
		if(XAllocColorCells(display, colormap, False, 0L, 0, pixels, 1) == False) {
			delete("Could not allocate %d pixels with AllocColorCells (colormap size >= %d, done %d)", npix, *size_in_out, i);
			return False;
		}
	}

	sz = i;

	for(i=0; XAllocColorCells(display, colormap, False, 0L, 0, &junk, 1); i++) {
		if (i > max_sz) {
			delete("Still allocating after %d cells allocated in a colormap of size %d cells.",
				sz + i, *size_in_out);
			return False;
		}
	}

	trace("Rest of colormap allocated (%d + %d = %d cells. Notional size = %d cells).",
		sz, i, sz + i, *size_in_out);
	sz += i;
	*size_in_out = sz;
	return True;
}

>>ASSERTION Good C
If any of the visual classes DirectColor, PseudoColor or GrayScale is supported:
A call to xname function frees the colourmap entries
obtained by ORing the 
.A npixels
pixel values specified in the 
.A pixels
argument 
with zero or more of the planes specified in the 
.A planes
argument
that have been
allocated by the client using
.S XAllocColor ,
.S XAllocNamedColor ,
.S XAllocColorCells
or
.S XAllocColorPlanes.
>>STRATEGY
For each visual class DirectColor, PseudoColor and GrayScale:
  Create a colormap with XCreateColormap.
  Allocate 1 colormap cell with XAllocNamedColor.
  Allocate the remaining colourmap cells with XAllocColorCells.
  Free the cell allocated by XAllocNamedColor with XFreeColors.
  Allocate 1 colourmap cell with XAllocColorCells.
  Verify that the call did not return False.

For each visual class DirectColor, PseudoColor and GrayScale:
  Create a colormap with XCreateColormap.
  Allocate 1 colormap cell with XAllocColor.
  Allocate the remaining colourmap cells with XAllocColorCells.
  Free the cell allocated by XAllocColor with XFreeColors.
  Allocate 1 colourmap cell with XAllocColorCells.
  Verify that the call did not return False.

For each visual class DirectColor, PseudoColor and GrayScale:
  Create a colormap with XCreateColormap.
  Allocate the entire colourmap with XAllocColorCells.
  Free a cell allocated by XAllocColorCells with XFreeColors.
  Allocate 1 colourmap cell with XAllocColorCells.
  Verify that the call did not return False.

>>CODE
XVisualInfo 	*vp;
XColor		color1, exactcol;
char		*goodname;
int		size;
unsigned long	pixel;
unsigned long	*pixels;
unsigned long	vmask = (1<<DirectColor)|(1<<PseudoColor)|(1<<GrayScale);


	if((vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

	if((goodname = tet_getvar("XT_GOOD_COLORNAME")) == (char *) 0L) {
		delete("XT_GOOD_COLORNAME is not defined.");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp);) {

		trace("XAllocNamedColor().");
		colormap = makecolmap(display, vp->visual, AllocNone);

		if(XAllocNamedColor(display, colormap, goodname,  &color1, &exactcol) == False) {
			delete("XAllocNamedColor() failed.");
			return;
		}

		size = maxsize(vp);

		if (!munch(&size, &pixel, 1)) {
			return; /* delete() already called in munch */
		} else
			CHECK;

		pixels = &color1.pixel;
		npixels = 1;
		planes = 0L;

		XCALL;

		if(geterr() == Success)
			CHECK;

		if(XAllocColorCells(display, colormap, False, 0L, 0, &pixel, 1) == False) {
			report("XFreeColors() did not free a cell allocated by XAllocNamedColor().");
			FAIL;
		} else
			CHECK;

		freereg();
	}	



	for(resetsupvis(vmask); nextsupvis(&vp);) {

		trace("XAllocColor().");
		colormap = makecolmap(display, vp->visual, AllocNone);

		if(XAllocColor(display, colormap, &color1) == False) {
			delete("XAllocColor() failed.");
			return;
		}

		size = maxsize(vp);

		if (!munch(&size, &pixel, 1)) {
			return; /* delete() already called in munch */
		} else
			CHECK;

		pixels = &color1.pixel;
		npixels = 1;
		planes = 0L;

		XCALL;

		if(geterr() == Success)
			CHECK;

		if(XAllocColorCells(display, colormap, False, 0L, 0, &pixel, 1) == False) {
			report("XFreeColors() did not free a cell allocated by XAllocColor().");
			FAIL;
		} else
			CHECK;

		freereg();
	}	


	for(resetsupvis(vmask); nextsupvis(&vp);) {
		trace("XAllocColorCells().");
		colormap = makecolmap(display, vp->visual, AllocNone);

		size = maxsize(vp);

		if (!munch(&size, &pixel, 1)) {
			return; /* delete() already called in munch */
		} else
			CHECK;

		pixels = &pixel;
		npixels = 1;
		planes = 0L;

		XCALL;

		if(geterr() == Success)
			CHECK;

		if(XAllocColorCells(display, colormap, False, 0L, 0, &pixel, 1) == False) {
			report("XFreeColors() did not free a cell allocated by XAllocColorCells().");
			FAIL;
		} else
			CHECK;

		freereg();
	}	



	CHECKPASS(nsupvis() * 3 * 3);

>>ASSERTION Good C
If any of the visual classes DirectColor, PseudoColor or GrayScale is supported:
When a read-only colourmap entry has been allocated by another client,
then the colourmap entry is not freed on a call to xname.
>>STRATEGY
For each of the visual classes DirectColor, PseudoColor and GrayScale:
  Create a colormap with alloc set to AllocNone.
  Create a second client with XOpenDisplay().
  Allocate a r/o cell with XAllocColor for the first client.
  Allocate a r/o cell using the returned rgb values with XAllocColor for the second client.
  Allocate the rest of the colormap with XAllocColorCells.
  Free the cell for the first client with XFreeColors.
  Verify that the colormap is full with XAllocColorCell.
  Free the cell for the second client with XFreeColors.
  Verify that the cell was freed with XAllocColorCell.
>>CODE
Display		*disp2;
XVisualInfo 	*vp;
XColor		color1;
int		size;
unsigned long	pixel;
unsigned long	*pixels;
unsigned long	vmask = (1<<PseudoColor)|(1<<GrayScale)|(1<<DirectColor);


	if((vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp);) {

		colormap = makecolmap(display, vp->visual, AllocNone);

		if((disp2 = opendisplay()) == (Display *) 0) {
			delete("Could not open the display.");
			return;
		}

		if(XAllocColor(Dsp, colormap, &color1) == False) {
			delete("XAllocColor() failed for second client.");
			return;
		}

		if(XAllocColor(disp2, colormap, &color1) == False) {
			delete("XAllocColor() failed for first client.");
			return;
		}

		size = maxsize(vp);

		if (!munch(&size, &pixel, 1)) {
			return; /* delete() already called in munch */
		} else
			CHECK;

		display = Dsp;
		pixels = &color1.pixel;
		npixels = 1;
		planes = 0L;

		XCALL;
		if(geterr() == Success)
			CHECK;

		if(XAllocColorCells(Dsp, colormap, False, 0L, 0, &pixel, 1) != False) {
			report("Shared cell was freed while allocted to another client.");
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(3*nsupvis());

>>ASSERTION Good A
If the visual class DirectColor, PseudoColor or GrayScale is supported:
When a read-only colourmap entry has been allocated more than once by the client,
and xname has been called one less time than the colormap entry was allocated,
then a call to xname frees the colormap entry.
>>STRATEGY
For each of the visual classes DirectColor, PseudoColor and GrayScale:
  Create a colormap with alloc set to AllocNone.
  Allocate a cell maxsize(vp) times with XAllocColor.
  Allocate the rest of the colourmap with XAllocColorCells.
  Repeat maxsize(vp)-1 times:
    Deallocate the colourcell with XFreeColours.
    Verify that the cell is not deallocated with XAllocColorCells.
  Deallocate the colourcell with XFreeColours.
  Verify that the colourcell was deallocated with XAllocColors.
>>CODE
XVisualInfo 	*vp;
XColor		color1;
int		size, i, refs;
unsigned long	pixel;
unsigned long	*pixels;
unsigned long	vmask = (1<<DirectColor)|(1<<PseudoColor)|(1<<GrayScale);


	if((vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp);) {

		colormap = makecolmap(display, vp->visual, AllocNone);

		size = maxsize(vp);

		refs = size-1;
		color1.red = color1.green = color1.blue = 0;
		for(i=0; i < refs; i++)
			if(XAllocColor(display, colormap, &color1) == False) {
				delete("XAllocColor() failed (Iteration %d).", i);
				return;
			}

		if (!munch(&size, &pixel, 1)) {
			return; /* delete() already called in munch */
		} else
			CHECK;

		for(i=0; i < refs-1; i++) {
			pixels = &color1.pixel;
			npixels = 1;
			planes = 0L;
			XCALL;
		}

		if(XAllocColorCells(display, colormap, False, 0L, 0, &pixel, 1) != False) {
			delete("XAllocColorCells() did not fail with a full colourmap.");
			return;
		} else
			CHECK;

		pixels = &color1.pixel;
		npixels = 1;
		planes = 0L;
		XCALL;

		if(XAllocColorCells(display, colormap, False, 0L, 0, &pixel, 1) == False) {
			report("XFreeColors() did not free a cell allocated by XAllocColor().");
			FAIL;
		} else
			CHECK;

		freereg();
	}	

	CHECKPASS(3*nsupvis());
  
>>ASSERTION Bad A
If any of the visual classes DirectColor, PseudoColor or GrayScale is supported:
When one or more pixels cannot be freed, and one or more pixels can be freed,
then the pixels that are allocated by the client in the colourmap that 
can be freed are freed.
>>STRATEGY
For each visual class DirectColor, PseudoColor and GrayScale:
  Create a colormap with alloc set to AllocNone.
  Create a new client with XOpenDisplay.
  Allocate a r/o cell for the new client with XAllocColor.
  Allocate the rest of the colormap for the first client with XAllocColorCells.
  Free the entire colormap with the second client with XFreeColors.
  Verify that only one cell was freed with XAllocColorCells.
>>CODE BadAccess
Display		*disp2;
XVisualInfo 	*vp;
XColor		color;
int		size;
unsigned long	pixel;
unsigned long	*pixels, *cptr, *ptr;
unsigned long	vmask = (1<<DirectColor)|(1<<PseudoColor)|(1<<GrayScale);


	if((vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp);) {

		disp2 = opendisplay();
		colormap = makecolmap(disp2, vp->visual, AllocNone);

		size = maxsize(vp);

		if((cptr = (unsigned long *) malloc(size * sizeof(unsigned long))) == (unsigned long *)0) {
			delete("malloc(%u) failed.", size * sizeof(unsigned long));
			return;
		}

		ptr = cptr;
		color.red = color.green = color.blue = 0;
		if(XAllocColor(disp2, colormap, &color) == False) {
			delete("XAllocColor() failed for client 2");
			return;
		}

		*ptr++ = color.pixel;
		
		if (!munch(&size, ptr, size-1)) {
			return; /* delete() already called in munch */
		} else
			CHECK;

		pixels = cptr;
		npixels = size + 1;
		planes = 0L;

		startcall(disp2);
		XFreeColors(disp2, colormap, pixels, npixels, planes);
		endcall(disp2);
		/* should be BadAccess! Often not on R4 */
		if (geterr() != BadAccess) {
			report("Got %s, expecting BadAccess", errorname(geterr()));
			FAIL;
		} else
			CHECK;

		if(XAllocColorCells(display, colormap, False, 0L, 0, &pixel, 1) == False) {
			report("XFreeColors() did not free a cell allocated by XAllocColor().");
			FAIL;
		} else
			CHECK;

		if(XAllocColorCells(display, colormap, False, 0L, 0, &pixel, 1) != False) {
			report("Colormap was not completely allocated.");
			FAIL;
		}
		
		free(cptr);
		freereg();
	}

	CHECKPASS(3*nsupvis());

>>ASSERTION Good A
If the visual class
.S DirectColor
is supported:
When all related colormap entries are already freed, then a call to xname
with a particular pixel value allows that pixel value to be allocated
by a subsequent call to
.S XAllocColorPlanes .
>>#
>># No matter what the plane argument is when given to XFreeColors,
>># a BadAccess error is always generated.
>>#
>>STRATEGY
For the visual class DirectColor:
  Create a colourmap with XCreateColormap.
  Allocate the entire colormap with 2 pixels and red_mask-1, green_mask-1
    and blue_mask-1 red, green and blue planes with XAllocColorPlanes.
  Verify that further allocation does not succeed with XAllocColorPlanes.
  Free pixel2 and red|green|blue planes.
  Allocate 1 pixel with red_mask-1, green_mask-1 and blue_mask-1 planes.
  Verify that the call did not return False.
>>CODE
XVisualInfo	*vp;
int		nreds, ngreens, nblues;
int		reds, greens, blues;
unsigned long	pr[2], rr, gr, br;
unsigned long	tpr[2], trr, tgr, tbr;
unsigned long	vmask = (1<<DirectColor);

	if((vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}
	resetsupvis(vmask);
	nextsupvis(&vp);

	nreds = bitcount(vp->red_mask);
	ngreens = bitcount(vp->green_mask);
	nblues = bitcount(vp->blue_mask);

	reds = nreds - 1;
	greens = ngreens - 1;
	blues = nblues - 1;

	colormap = makecolmap(display, vp->visual, AllocNone);

	if((XAllocColorPlanes(display, colormap, False, pr, 2, reds, greens, blues, &rr, &gr, &br)) == False) {
		delete("XAllocColorPlanes failed.");
		return;
	} else
		CHECK;

	if((XAllocColorPlanes(display, colormap, False, tpr, 1, 0, 0, 0, &trr, &tgr, &tbr)) != False) {
		delete("Allocated colormap was not completely filled");
		return;
	} else
		CHECK;

	pixels = pr+1;
	npixels = 1;
	planes = rr | gr | br;
	XCALL;

	if((XAllocColorPlanes(display, colormap, False, pr, 1, reds, greens, blues, &rr, &gr, &br)) == False) {
		trace("Freed colormap cells could not be re-allocated.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Bad A
When a specified pixel is not a valid entry in the
.A colormap
argument, then a
.S BadValue
error occurs.
>>STRATEGY
For all supported visual types:
  Create a colormap with alloc set to AllocNone.
  Free a pixel with pixel value of 2power(longbits)-1 with XFreeColors.
  Verify that a BadValue error occurred.

  Create a colormap using XCreateColormap with alloc set to AllocNone.
  Allocate one readonly cell in the colormap with XAllocColor.
  Construct an array with the same pixel in both elements.
  Deallocate the colormap cells indicated by the array with XFreeColors.
  Verify that a BadValue error occurred.
>>CODE BadValue
XVisualInfo	*vp;
XColor		color;
unsigned long	pixel[2];
unsigned long	vmask = 0L;

	if((vmask = visualsupported(display, vmask)) == 0L) {
		delete("No visuals reported as supported");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {

		colormap = makecolmap(display, vp->visual, AllocNone);
		pixel[0] = ~0L;
		pixels = pixel;
		npixels = 1;
		planes = 0L;
		XCALL;
		if(geterr() == BadValue)
			CHECK;

		colormap = makecolmap(display, vp->visual, AllocNone);
		if(XAllocColor(display, colormap, &color) == False) {
			delete("XAllocColor Failed.");
			return;
		}

		pixel[0] = color.pixel;
		pixel[1] = ~0L;
		pixels = pixel;
		npixels = 2;
		planes = 0L;
		XCALL;
		if(geterr() == BadValue)
			CHECK;

	}	

	CHECKPASS(2*nsupvis());

>>ASSERTION Bad A
.ER BadAccess colormap-free
>>STRATEGY
  Create a colormap with alloc set to AllocNone.
  Free a pixel with pixel value = 0 with XFreeColors.
  Verify that a BadValue error occurred.
>>CODE BadAccess
XVisualInfo	*vp;
unsigned long	pixel[1];
unsigned long	vmask = 0L;

	if((vmask = visualsupported(display, vmask)) == 0L) {
		delete("No visuals reported as supported");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {

		colormap = makecolmap(display, vp->visual, AllocNone);
		pixel[0] = 0;
		pixels = pixel;
		npixels = 1;
		planes = 0L;
		XCALL;
		if(geterr() == BadAccess)
			CHECK;
	}	

	CHECKPASS(nsupvis());

>>ASSERTION Bad A
.ER BadColor
>>ASSERTION Bad A
When more than one
.M pixel
value is not a valid entry in the 
.M colormap
argument, then a 
.S BadValue
error occurs which will report any one of the invalid pixel values.
>>STRATEGY
For each supported visual class:
  Create a colormap using XCreateColormap with alloc set to AllocNone.
  Allocate one readonly cell in the colormap with XAllocColor.
  Construct an array with two invalid pixel values and one valid pixel value.
  Deallocate the colormap cells indicated by the array with XFreeColors.
  Verify that a BadValue error occurred.
  Verify that the bad value reported was one of the invalid pixel array elements.
>>CODE BadValue
XVisualInfo	*vp;
XColor		color;
int		errval;
unsigned long	pixel[3];
unsigned long	vmask = 0L;

	if((vmask = visualsupported(display, vmask)) == 0L) {
		delete("No visuals reported as supported");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {

		colormap = makecolmap(display, vp->visual, AllocNone);
		if(XAllocColor(display, colormap, &color) == False) {
			delete("XAllocColor Failed.");
			return;
		}
		pixel[0] = ~0L;
		pixel[1] = color.pixel;
		pixel[2] = ~0L;
		pixels = pixel;
		npixels = 3;
		planes = 0L;
		XCALL;
		if(geterr() == BadValue) {
			errval = getbadvalue();
			if( (errval != -1) && (errval != -2) ) {
				report("Erroneous value was reported as %d, instead of -1 or -2", errval);
				FAIL;
			} else
				CHECK;
		}

	}
	CHECKPASS(nsupvis());


>>#HISTORY	Cal	Completed	Written in new format and style.
>>#HISTORY	Kieron	Completed		<Have a look>
>>#HISTORY	Cal	Action		Writing code.
