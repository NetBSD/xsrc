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
 * $XConsortium: allcclrpln.m,v 1.8 94/04/17 21:03:47 rws Exp $
 */
>>TITLE XAllocColorPlanes CH05
Status
XAllocColorPlanes(display, colormap, contig, pixels_return, ncolors, nreds, ngreens, nblues, rmask_return, gmask_return, bmask_return)
Display *display = Dsp;
Colormap colormap = DefaultColormap(display, DefaultScreen(display));
Bool contig;
unsigned long *pixels_return = &dummy;
int ncolors = 1;
int nreds;
int ngreens;
int nblues;
unsigned long *rmask_return = &dummy;
unsigned long *gmask_return = &dummy;
unsigned long *bmask_return = &dummy;
>>EXTERN
static unsigned long dummy;

static int contiguous(mask)
unsigned long mask;
{
	if(!mask)
		return(0L);

	while((mask&1) == 0L)
		mask>>=1;
	mask++;
	return( bitcount(mask) == 1);
}

static int maskshift(mask)
unsigned long mask;
{
	int	i = 0;

	if(!mask)
		return(0);

	while((mask&1) == 0L) {
		mask>>=1;
		i++;
	}
	return(i);
}

>>ASSERTION Good A
A call to xname allocates 
.A ncolors*2pow(nreds+ngreens+nblues)
read/write colourmap entries whose indices in the colourmap are 
obtained by ORing each pixel value returned in the 
.A pixels_return
argument 
with zero or more of the planes obtained by ORing the masks
returned in the 
.A rmask_return ,
.A gmask_return
and
.A bmask_return
arguments.
>>STRATEGY
For visual class DirectColor:
  Create a colormap with alloc set to AllocNone.
  Allocate one colour and all planes with XAllocColorPlanes.
  Verify that the function did not return zero.
  Verify the correct number of cells have been allocated by initialising
    them with XStoreColors, and reading them with XQueryColors.
>>CODE
XVisualInfo	*vp;
unsigned long	vmask = (1<<DirectColor);
unsigned long	*pixm, *rr, *gr, *br;
unsigned long	rm, gm, bm, pm, rgbmask, pixel, r, g, b;
XColor 		color, qcolor;
Status		status;
int		cells;
int		rshift, gshift, bshift;

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		tet_result(TET_UNSUPPORTED);
		return;
	}

	resetsupvis(vmask);
	nextsupvis(&vp);
		
	colormap = makecolmap(display, vp->visual, AllocNone);
	ncolors = 1;
	nreds = bitcount(vp->red_mask);
	ngreens = bitcount(vp->green_mask);
	nblues = bitcount(vp->blue_mask);
	cells = ncolors * (1 << (nreds+ngreens+nblues));
	contig = False;
	pixels_return = &pm;
	rmask_return = &rm;
	gmask_return = &gm;
	bmask_return = &bm;

	status = XCALL;
	if(status == (Status) 0) {
		report("%s failed to return non-zero", TestName);
		FAIL;
	} else {
		trace("%d cells allocated in the colormap", cells);
		CHECK;
	}

	rgbmask = rm | gm | bm ;
	rshift = maskshift(rm);
	gshift = maskshift(gm);
	bshift = maskshift(bm);
	trace("rgbmask is %x", rgbmask);
	trace("red shift is %d", rshift);
	trace("green shift is %d", gshift);
	trace("blue shift is %d", bshift);

	/* 
	 * The following code works on the assumption that for each subfield, 
	 * all planes in the subfield mask are allocated (so are contiguous).
	 *
	 * (It does not assume the subfields are adjacent)
	 */

	for(r = 0; r<(1<<nreds); r++) {
		pixel = (r<<rshift) | pm;
		debug(1, "pixel value %x", pixel);
		color.pixel = pixel;
		color.flags =DoRed;
		color.red = 0xffff;

		startcall(display);
		XStoreColor(display, colormap, &color);
		endcall(display);

		if(geterr() != Success) {
			report("XStoreColor() failed with pixel value %lu", pixel);
			FAIL;
		} else {
			qcolor.pixel = pixel;
			startcall(display);
			XQueryColor(display, colormap, &qcolor);
			endcall(display);
		if(geterr() != Success) {
			report("XQueryColor() failed with pixel value %lu", pixel);
			FAIL;
		} else
			CHECK;
		}
	}

	for(g = 0; g<(1<<ngreens); g++) {
		pixel = (g<<gshift) | pm;
		debug(1, "pixel value %x", pixel);
		color.pixel = pixel;
		color.flags =DoGreen;
		color.green = 0xffff;

		startcall(display);
		XStoreColor(display, colormap, &color);
		endcall(display);

		if(geterr() != Success) {
			report("XStoreColor() failed with pixel value %lu", pixel);
			FAIL;
		} else {
			qcolor.pixel = pixel;
			startcall(display);
			XQueryColor(display, colormap, &qcolor);
			endcall(display);
			if(geterr() != Success) {
				report("XQueryColor() failed with pixel value %lu", pixel);
				FAIL;
			} else
				CHECK;
		}
	}

	for(b = 0; b<(1<<nblues); b++) {
		pixel = (b<<bshift) | pm;
		debug(1, "pixel value %x", pixel);
		color.pixel = pixel;
		color.flags =DoBlue;
		color.blue = 0xffff;

		startcall(display);
		XStoreColor(display, colormap, &color);
		endcall(display);

		if(geterr() != Success) {
			report("XStoreColor() failed with pixel value %lu", pixel);
			FAIL;
		} else {
			qcolor.pixel = pixel;
			startcall(display);
			XQueryColor(display, colormap, &qcolor);
			endcall(display);
			if(geterr() != Success) {
				report("XQueryColor() failed with pixel value %lu", pixel);
				FAIL;
			} else
				CHECK;
		}
	}

	CHECKPASS(1 + (1<<nreds) + (1<<ngreens) + (1<<nblues));
>>ASSERTION Good A
A call to xname sets
.A nreds
bits to one in the mask named by
.A rmask_return ,
.A ngreens 
bits to one in the mask named by
.A gmask_return ,
and
.A nblues
bits to one in the mask named by
.A bmask_return
and returns
.A ncolors
pixels values in the
.A pixels_return
argument
such that no mask has a bit set in common with
any other mask or with any of the pixels.
>>STRATEGY
For the visual class DirectColor:
  Create a colormap with alloc set to AllocNone.
  Allocate one colour and all planes with XAllocColorPlanes.
  Verify the bitcount of the OR of the colourplane masks equals the 
    sum of the individual bitcounts.
  Verify that the AND of the pixel and the OR of the colourplane masks is zero.
  Create a colormap with alloc set to AllocNone.
  Allocate 3 colours and 3 planes with XAllocColorPlanes.
  Verify the bitcount of the OR of the colourplane masks equals the 
    sum of the individual bitcounts.
  Verify that the AND of each pixel and the OR of the colourplane masks is zero.
>>CODE
XVisualInfo	*vp;
unsigned long	vmask = (1<<DirectColor);
unsigned long	*pixm, *rr, *gr, *br;
unsigned long	rm, gm, bm, pm, rgbmask;
unsigned long	pm_array[3], i;
Status		status;
int		cells, rnreds, rngreens, rnblues;

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		tet_result(TET_UNSUPPORTED);
		return;
	}

	resetsupvis(vmask);
	nextsupvis(&vp);
		
	colormap = makecolmap(display, vp->visual, AllocNone);
	ncolors = 1;
	nreds = bitcount(vp->red_mask);
	ngreens = bitcount(vp->green_mask);
	nblues = bitcount(vp->blue_mask);
	cells = ncolors * (1 << (nreds+ngreens+nblues));
	contig = False;
	pixels_return = &pm;
	rmask_return = &rm;
	gmask_return = &gm;
	bmask_return = &bm;

	status = XCALL;
	if(status == (Status) 0) {
		report("%s failed to return non-zero", TestName);
		FAIL;
	} else {
		trace("%d cells allocated in the colormap", cells);
		CHECK;
	}

	rnreds = bitcount(rm);
	rngreens = bitcount(gm);
	rnblues = bitcount(bm);

	if( (rnreds != nreds) || (rngreens != ngreens) || (rnblues != nblues) ) {
		report("Request for %d reds %d greens %d blues returned %d reds %d greens %d blues.",
			  nreds, ngreens, nblues, rnreds, rngreens, rnblues);
		FAIL;
	} else
		CHECK;

	rgbmask = rm | gm | bm ;
	
	if( bitcount(rgbmask) != rnreds + rnblues + rngreens) {
		report("The returned colourplane  masks had at least one shared bit.");
		FAIL;
	} else
		CHECK;

	if( (rgbmask & pm) != 0L) {
		report("A pixel shared at least one bit with a colourplane.");
		FAIL;
	} else
		CHECK;

	/*
	 * Repeat requesting a number of colours and fewer planes.
	 */
	colormap = makecolmap(display, vp->visual, AllocNone);

	ncolors = 3;
	/* Make sure that this visual is big enough to support so many colours */
	if (ncolors > bitcount(vp->blue_mask))
		ncolors = bitcount(vp->blue_mask);
	if (ncolors > bitcount(vp->green_mask))
		ncolors = bitcount(vp->green_mask);
	if (ncolors > bitcount(vp->red_mask))
		ncolors = bitcount(vp->red_mask);
	debug(1, "ncolors = %d", ncolors);

	nreds = 1;
	ngreens = 1;
	nblues = 1;
	cells = ncolors * (1 << (nreds+ngreens+nblues));
	contig = False;
	pixels_return = &pm_array[0];
	rmask_return = &rm;
	gmask_return = &gm;
	bmask_return = &bm;

	status = XCALL;
	if(status == (Status) 0) {
		report("%s failed to return non-zero", TestName);
		FAIL;
	} else {
		trace("%d cells allocated in the colormap", cells);
		CHECK;
	}

	rnreds = bitcount(rm);
	rngreens = bitcount(gm);
	rnblues = bitcount(bm);

	if( (rnreds != nreds) || (rngreens != ngreens) || (rnblues != nblues) ) {
		report("Request for %d reds %d greens %d blues returned %d reds %d greens %d blues.",
			  nreds, ngreens, nblues, rnreds, rngreens, rnblues);
		FAIL;
	} else
		CHECK;

	rgbmask = rm | gm | bm ;
	
	if( bitcount(rgbmask) != rnreds + rnblues + rngreens) {
		report("The returned colourplane  masks had at least one shared bit.");
		FAIL;
	} else
		CHECK;

	for (i = 0; i < ncolors; i++) {
		pm = pm_array[i];
		trace("pixel value of 0x%lx", pm);
		if( (rgbmask & pm) != 0L) {
			report("pixel value %ld shared at least one bit with a colourplane.", pm);
			FAIL;
		} else
			CHECK;
		if ((pm & ~DEPTHMASK(vp->depth)) != 0L) {
			report("pixel value 0x%lx contained bits outside the depth", pm);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(7+2*ncolors);

>>ASSERTION Good A
If the visual class
.S DirectColor
is supported:
When the visual type of the 
.A colormap
argument is 
.S DirectColor ,
then
.A rmask_return
lies within the red pixel subfield
and
the 
.A gmask_return
lies within the green pixel subfield
and
the 
.A bmask_return
lies within the blue pixel subfield.
>>STRATEGY
For the visual class DirectColor:
  Create a colormap with alloc set to AllocNone.
  Allocate one colour and all planes with XAllocColorPlanes.
  Verify that the returned masks and the colour subfield masks are identical.
>>CODE
XVisualInfo	*vp;
unsigned long	vmask = (1<<DirectColor);
unsigned long	*pixm, *rr, *gr, *br;
unsigned long	rm, gm, bm, pm, rgbmask, pixel, i;
Status		status;
int		cells;

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		tet_result(TET_UNSUPPORTED);
		return;
	}

	resetsupvis(vmask);
	nextsupvis(&vp);
		
	colormap = makecolmap(display, vp->visual, AllocNone);
	ncolors = 1;
	nreds = bitcount(vp->red_mask);
	ngreens = bitcount(vp->green_mask);
	nblues = bitcount(vp->blue_mask);
	cells = ncolors * (1 << (nreds+ngreens+nblues));
	contig = False;
	pixels_return = &pm;
	rmask_return = &rm;
	gmask_return = &gm;
	bmask_return = &bm;

	status = XCALL;
	if(status == (Status) 0) {
		report("%s failed to return non-zero", TestName);
		FAIL;
	} else {
		trace("%d cells allocated in the colormap", cells);
		CHECK;
	}

	/*
	 * We can check the mask returned matches the pixel subfield exactly
	 * since we requested all planes to be allocated.
	 */
	if( (vp->red_mask != rm) ) {
		report("Returned red mask not contained in red pixel subfield");
		FAIL;
	} else
		CHECK;
	if( (vp->green_mask != gm) ) {
		report("Returned green mask not contained in green pixel subfield");
		FAIL;
	} else
		CHECK;
	if( (vp->blue_mask != bm) ) {
		report("Returned blue mask not contained in blue pixel subfield");
		FAIL;
	} else
		CHECK;

	CHECKPASS(4);

>>ASSERTION Good A
When the
.A contig 
argument is 
.S True ,  
then
.A rmask_return ,
.A gmask_return
and
.A bmask_return
each contains a contiguous set of bits set to one.
>>STRATEGY
For the visual class DirectColor:
  Create a colormap with alloc set to AllocNone.
  Allocate one colour and some planes with XAllocColorPlanes and contig = True.
  Verify that the returned masks were contiguous.
>>CODE
XVisualInfo	*vp;
unsigned long	vmask = (1<<DirectColor);
unsigned long	*pixm, *rr, *gr, *br;
unsigned long	rm, gm, bm, pm, rgbmask, pixel, i;
XColor 		color;
Status		status;
int		cells;

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		tet_result(TET_UNSUPPORTED);
		return;
	}

	resetsupvis(vmask);
	nextsupvis(&vp);
		
	colormap = makecolmap(display, vp->visual, AllocNone);
	ncolors = 1;
	nreds = bitcount(vp->red_mask);
	ngreens = bitcount(vp->green_mask);
	nblues = bitcount(vp->blue_mask);
	cells = ncolors * (1 << (nreds+ngreens+nblues));
	contig = True;
	pixels_return = &pm;
	rmask_return = &rm;
	gmask_return = &gm;
	bmask_return = &bm;

	/* 
	 * Just ask for a small number of planes if more was possible.
	 * Originally this test requested all planes - but of course this
	 * gave little scope for contig to make any difference.
	 */
	if(nreds > 2)
		nreds = 2;
	if(ngreens > 2)
		ngreens = 2;
	if(nblues > 2)
		nblues = 2;
	trace("test with %d reds %d greens %d blues", nreds, ngreens, nblues);

	status = XCALL;
	if(status == (Status) 0) {
		report("%s failed to return non-zero", TestName);
		FAIL;
	} else {
		trace("%d cells allocated in the colormap", cells);
		CHECK;
	}

	if((contiguous(rm) && contiguous(gm) && contiguous(bm)) == 0) {
		report("Returned colourplane masks were not contiguous");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Bad A
.ER BadColor
>>ASSERTION Bad A
When the 
.A ncolors
argument is zero, then a 
.S BadValue 
error occurs.
>>STRATEGY
For all supported visual classes:
  Create a colormap with alloc set to AllocNone.
  Call XAllocColorPlanes with ncolors = 0.
>>CODE BadValue
XVisualInfo *vp;
Visual *visual;
unsigned long vmask = (1<<DirectColor);

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		tet_result(TET_UNSUPPORTED);
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		colormap = makecolmap(display, vp->visual, AllocNone);
		ncolors = 0;
		XCALL;
		if(geterr() == BadValue)
			CHECK;
	}

	CHECKPASS(nsupvis());

>>ASSERTION Bad A
.ER BadValue contig True False
>>#HISTORY	Cal	Completed 	Written in new style and format 4/12/90
>>#HISTORY	Cal	Action		Writing code.
