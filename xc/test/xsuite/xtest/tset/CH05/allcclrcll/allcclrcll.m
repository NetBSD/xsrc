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
 * $XConsortium: allcclrcll.m,v 1.12 94/04/17 21:03:46 rws Exp $
 */
>>TITLE XAllocColorCells CH05
Status
XAllocColorCells(display, colormap, contig, plane_masks_return, nplanes, pixels_return, npixels)
Display *display = Dsp;
Colormap colormap;
Bool contig;
unsigned long *plane_masks_return;
unsigned int nplanes;
unsigned long *pixels_return;
unsigned int npixels;
>>EXTERN
#define lowbit(x)	((x) & (~(x) + 1))
#define bitcontig(x)	((((x) + lowbit(x)) & (x)) == 0)
#define bitsubset(a,b)	(((a) & (b)) == (a))

>>ASSERTION Good C
If any of the visual classes
.S DirectColor ,
.S PseudoColor ,
or
.S GrayScale
is supported:
A call to xname allocates 
.A npixels*2pow(nplanes)
read/write colourmap entries whose indices in the colourmap are 
obtained by ORing each pixel value returned in the 
.A pixels_return
argument 
with zero or more of the planes returned in the 
.A plane_mask
argument.
>>STRATEGY
For each visual class DirectColor, PseudoColor and GrayScale:
  Create a colormap.
  Allocate all colors and no planemasks with XAllocColorCells.
  Verify the correct number of cells have been allocated by initialising 
    them with XStoreColors, and reading them with XQueryColors.
  Allocate planes and masks such that lg(colors) + planes = lg(colormap_size)
  Verify the correct number of cells have been allocated by initialising 
    them with XStoreColors, and reading them with XQueryColors.
>>CODE
XVisualInfo *vp;
XColor color, qcolor;
Visual *visual;
Status status;
Colormap cmap;
unsigned long vmask, pmask, mask, planemask;
int i, j, k, cells;
int pathcnt;

#define RW_COLORMAP_MASK ((1L<<DirectColor) | (1L<<PseudoColor) | (1L<<GrayScale))

	if( (vmask = visualsupported(display, RW_COLORMAP_MASK)) == 0L) {
		unsupported("DirectColor, PseudoColor and GrayScale are not supported");
		return;
	}

	contig = False;
	color.red = 1<<8;
	color.green = 2<<8;
	color.blue = 3<<8;
	color.flags = DoRed | DoGreen | DoBlue;

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		trace("Attempting XAllocColorCells() for class %s", displayclassname(vp->class));

		/* Test that we can allocate the entire colormap, nplanes = 0 */

		colormap = makecolmap(display, vp -> visual, AllocNone);
		nplanes = 0;
		npixels = maxsize(vp);
		plane_masks_return = (unsigned long *) 0;
		pixels_return = (unsigned long *) malloc( npixels * sizeof(unsigned long));
		trace("Testing XAllocColorCells with %u colors and %u planes", npixels, nplanes);
		if(pixels_return == (unsigned long *) 0) {
			delete("Malloc failed to allocate memory for pixels return value.");
			return;
		}

		for(j=0; j<npixels; j++)
			pixels_return[j] = npixels+1;

		status = XCALL;
		if(status == (Status) 0) {
			report("%s failed to return non-zero", TestName);
			report("allocating all %d cells with visual class %s.",
				 npixels, displayclassname(vp->class));
			FAIL;
			continue;
		} else
			CHECK;

		for(pathcnt = 0, j=0; j < npixels; j++) {
			color.pixel = pixels_return[j];
			startcall(display);
			XStoreColor(display, colormap, &color);
			endcall(display);
			if(geterr() != Success) {
				report("XStoreColor() failed (%s) to store pixel value %lu in colormap of class %s", errorname(geterr()), color.pixel, displayclassname(vp->class));
				FAIL;
			} else {
				qcolor.pixel = pixels_return[j];
				startcall(display);
				XQueryColor(display, colormap, &qcolor);
				endcall(display);
				if(geterr() != Success) {
					report("XQueryColor() failed (%s) to return pixel value %lu in colormap of class %s", errorname(geterr()), qcolor.pixel, displayclassname(vp->class));
					FAIL;
				} else
					pathcnt++;
			}
		}

		if(pathcnt == npixels)
			CHECK;

		free( (char *) pixels_return);

		/* Test that we can allocate planes and cells of size floor(log2(colormapsize)). */

		colormap = makecolmap(display, vp -> visual, AllocNone);
		cells = 1<<lg(maxsize(vp));
		npixels = (unsigned int) 1<<(lg(maxsize(vp))/2);
		nplanes = (unsigned int) lg(maxsize(vp)) - lg(npixels);
		plane_masks_return = (unsigned long *) malloc(nplanes * sizeof(unsigned long));
		pixels_return = (unsigned long *) malloc(npixels * sizeof(unsigned long));

		if(plane_masks_return == (unsigned long*) 0 || pixels_return == (unsigned long*) 0) {
			delete("Malloc() failed to allocate memory for return values.");
			return;
		} else
			CHECK;

		for(j=0; j<npixels; j++)
			pixels_return[j] = cells+1;

		for(j=0; j<nplanes; j++)
			plane_masks_return[j] = cells+1;

		trace("Testing XAllocColorCells() with nplanes = %u, npixels = %u, cells = %d, pixels * 2 pow planes = %u", nplanes, npixels, cells, npixels * (1<<nplanes));

		status = XCALL;		
		if(status == (Status) 0) {
			report("%s failed to return non-zero", TestName);
			report("allocating %lu pixels and %lu planes with visual class %s.",
				 npixels, nplanes, displayclassname(vp->class));
			FAIL;
			continue;
		} else
			CHECK;

		for(pathcnt = 0, j=0; j < npixels; j++)
			for(i=0; i < (1<<nplanes); i++) {
				planemask = 0L;
				for(k=0, mask=1; k < nplanes; k++, mask <<= 1) /*                                        */
					if((unsigned long) i & mask)           /* Select the i th subset of plane masks. */
						mask |= plane_masks_return[k]; /*                                        */
		
				color.pixel = pixels_return[j] | planemask;
				startcall(display);
				XStoreColor(display, colormap, &color);
				endcall(display);

				if(geterr() != Success) {
					report("XStoreColor() failed (%s) to store pixel value %lu in colormap of class %s", errorname(geterr()), color.pixel, displayclassname(vp->class));
					FAIL;
				} else {
					qcolor.pixel = pixels_return[j] | planemask;
					startcall(display);
					XQueryColor(display, colormap, &qcolor);
					endcall(display);
					if(geterr() != Success) {
						report("XQueryColor() failed (%s) to return pixel value %lu in colormap of class %s", errorname(geterr()), qcolor.pixel, displayclassname(vp->class));
						FAIL;
					} else
						pathcnt++;
				}
		}

		if(pathcnt == npixels*(1<<nplanes))
			CHECK;

		free( (char *) pixels_return);
		free( (char *) plane_masks_return);
	}

	CHECKPASS(5 * nsupvis());

>>ASSERTION Good C
If any of the visual classes
.S DirectColor ,
.S PseudoColor ,
or
.S GrayScale
is supported:
A call to xname returns
.A nplanes
plane masks in the 
.A plane_mask_return
argument 
and 
.A npixels
pixel values in the 
.A pixels_return
argument 
such that no plane mask has a bit set in common with
any other plane mask or with any of the pixels.
>>STRATEGY
For each supported visual with r/w colour cells:
  Create a colourmap with alloc set to AllocNone.
  Allocate planes and masks such that lg(colors) + planes = lg(colormap_size)
  Form the bitwise OR of all the pixels.
  For each planemask:
    Verify that the bitwise AND of every other planemask is 0.
    Verify that the bitwise AND of the planemask and the OR of the colours is 0.
>>CODE
XVisualInfo *vp;
Visual *visual;
Status status;
Colormap cmap;
unsigned long vmask, planemask, pixelmask;
int i, j, cells, pathcnt = 0;


	if( (vmask = visualsupported(display, RW_COLORMAP_MASK)) == 0L) {
		unsupported("DirectColor, PseudoColor and GrayScale are not supported");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		trace("Attempting XAllocColorCells() for class %s", displayclassname(vp->class));

		contig = False;
		colormap = makecolmap(display, vp -> visual, AllocNone);
		cells = 1<<lg(maxsize(vp));
		npixels = (unsigned int) 1<<(lg(maxsize(vp))/2);
		nplanes = (unsigned int) lg(maxsize(vp)) - lg(npixels);
		plane_masks_return = (unsigned long *) malloc(nplanes * sizeof(unsigned long));
		pixels_return = (unsigned long *) malloc(npixels * sizeof(unsigned long));
		if(plane_masks_return == (unsigned long*) 0 ||
		   pixels_return == (unsigned long*) 0) {
			delete("Malloc() failed to allocate memory for return values.");
			return;
		} else
			CHECK;

		pathcnt += nplanes*(nplanes+1);

		for(j=0; j<npixels; j++)
			pixels_return[j] = cells+1;

		for(j=0; j<nplanes; j++)
			plane_masks_return[j] = cells+1;

		trace("Testing XAllocColorCells() with nplanes = %u, npixels = %u, cells = %d, pixels * 2 pow planes = %u",
			nplanes, npixels, cells, npixels * (1<<nplanes));
		
		status = XCALL;

		if(status == (Status) 0) {
			report("%s failed to return non-zero", TestName);
			report("for visual class %s.", 	
						displayclassname(vp->class));
			FAIL;
		} else {
			pixelmask = 0L;
			for(j=0; j < npixels; j++)
				pixelmask |= pixels_return[j];
	
			for(j=0; j < nplanes; j++) {
				planemask = plane_masks_return[j];
				for(i=0; i < nplanes; i++) 
					if(((planemask & plane_masks_return[i]) != 0) && (i != j)) {
						report("Plane mask %d(%lu) has a bit in common with mask %d(%ln).", j, planemask, i, plane_masks_return[i]);
						FAIL;
					} else
						CHECK;

				if( (pixelmask & planemask) != 0L) {
					report("Plane mask %d(%lu) has a bit in common with a pixel value.", j, planemask);
					FAIL;
				} else
					CHECK;
			}
		}

		free( (char *) plane_masks_return);
		free( (char *) pixels_return);
	}

	CHECKPASS(pathcnt+nsupvis());

>>ASSERTION Good A	
If either of the visual classes
.S PseudoColor
or
.S GrayScale
are supported:
When the visual type of the 
.A colormap
argument is 
.S PseudoColor
or
.S GrayScale ,
then a call to xname sets exactly one bit in each plane mask in
.A plane_mask_return .
>>STRATEGY
For the visuals PseudoColor and GrayScale:
  Create a colourmap with alloc set to AllocNone.
  Allocate 1 color and all planemasks with XAllocColorCells.
  Verify that the number of bits set in each planemask is exactly 1.
>>CODE
XVisualInfo *vp;
Visual *visual;
Status status;
Colormap cmap;
unsigned long vmask;
int j, cells, pathcnt = 0;


	if( (vmask = visualsupported(display, (1<<PseudoColor) | (1<<GrayScale)) ) == 0L) {
		unsupported("PseudoColor and GrayScale are not supported");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		trace("Attempting XAllocColorCells() for class %s", displayclassname(vp->class));

		contig = False;
		colormap = makecolmap(display, vp -> visual, AllocNone);
		cells = 1<<lg(maxsize(vp));
		npixels = 1;
		nplanes = (unsigned int) lg(maxsize(vp));
		plane_masks_return = (unsigned long *) malloc(nplanes * sizeof(unsigned long));
		pixels_return = (unsigned long *) malloc(npixels * sizeof(unsigned long));
		if(plane_masks_return == (unsigned long*) 0 || pixels_return == (unsigned long*) 0) {
			delete("Malloc() failed to allocate memory for return values.");
			return;
		} else
			CHECK;

		pathcnt += nplanes;

		for(j=0; j<npixels; j++)
			pixels_return[j] = cells+1;

		for(j=0; j<nplanes; j++)
			plane_masks_return[j] = cells+1;

		trace("Testing XAllocColorCells() with nplanes = %u, npixels = %u, cells = %d, pixels * 2 pow planes = %u", nplanes, npixels, cells, npixels * (1<<nplanes));
		
		status = XCALL;

		if(status == (Status) 0) {
			report("%s failed to return non-zero", TestName);
			report("for visual class %s.", 	
						displayclassname(vp->class));
			FAIL;
		} else {
			int bits;
			for(j=0; j < nplanes; j++)
				if( (bits = bitcount(plane_masks_return[j])) != 1 ) {
					report("Plane_mask %d had %d bits set instead of 1", j, bits);
					FAIL;
				}
				else
					CHECK;
		}
	}

	CHECKPASS(pathcnt + nsupvis());

>>ASSERTION Good A
If the visual class
.S DirectColor
is supported:
When the visual type of the 
.A colormap
argument is 
.S DirectColor , 
then a call to xname sets exactly three bits in each plane mask in
.A plane_mask_return .
>>STRATEGY
For the visual class DirectColor:
  Create a colormap.
  Allocate 1 color and all planemasks with XAllocColorCells.
  Verify that for each planemask exactly three bits are set.
>>CODE
XVisualInfo *vp;
Visual *visual;
Status status;
Colormap cmap;
unsigned long vmask;
int j, cells;


	if( (vmask = visualsupported(display, 1<<DirectColor)) == 0L) {
		unsupported("DirectColor is not supported");
		return;
	}

	resetsupvis(vmask);
	nextsupvis(&vp);
	trace("Attempting XAllocColorCells() for class %s", displayclassname(vp->class));
	contig = False;
	colormap = XCreateColormap(display, DRW(display), vp -> visual, AllocNone);
	cells = 1<<lg(maxsize(vp));
	npixels = 1;
	nplanes = (unsigned int) lg(maxsize(vp));
	plane_masks_return = (unsigned long *) malloc(nplanes * sizeof(unsigned long));
	pixels_return = (unsigned long *) malloc(npixels * sizeof(unsigned long));
	if(plane_masks_return == (unsigned long*) 0 || pixels_return == (unsigned long*) 0) {
		delete("Malloc() failed to allocate memory for return values.");
		return;
	} else
		CHECK;

	for(j=0; j<npixels; j++)
		pixels_return[j] = cells+1;

	for(j=0; j<nplanes; j++)
		plane_masks_return[j] = cells+1;

	trace("Testing XAllocColorCells() with nplanes = %u, npixels = %u, cells = %d, pixels * 2 pow planes = %u", nplanes, npixels, cells, npixels * (1<<nplanes));
		
	status = XCALL;

	if(status == (Status) 0) {
		report("%s failed to return non-zero", TestName);
		report("for visual class %s.", 	
					displayclassname(vp->class));
		FAIL;
	} else {
		int bits;
		for(j=0; j < nplanes; j++)
			if( (bits = bitcount(plane_masks_return[j])) != 3 ) {
				report("Plane_mask %d had %d bits set instead of 3", j, bits);
				FAIL;
			}
			else
				CHECK;
	}

	CHECKPASS(nplanes+nsupvis());


>>ASSERTION Good A
If either of the visual classes
.S PseudoColor
or
.S GrayScale
are supported:
When the
.A contig
argument is
.S True ,
and the visual type of the 
.A colormap
argument is 
.S GrayScale
or
.S PseudoColor ,
then the mask formed by ORing the 
plane masks in
.A plane_mask_return
contains one set of contiguous bits.
>>STRATEGY
For the visual types PseudoColor and GreyScale:
  Create a colourmap.
  Allocate 1 color and some planemasks using XAllocColorCells and contig = True.
  Form the bitwise OR of all the planemasks.
  Verify that the returned planemasks were contiguous.
>>CODE
XVisualInfo *vp;
Visual *visual;
Status status;
Colormap cmap;
unsigned long vmask;
unsigned long planeor;
int j, cells;


	if( (vmask = visualsupported(display, (1<<PseudoColor) | (1<<GrayScale)) ) == 0L) {
		unsupported("PseudoColor and GrayScale are not supported");
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		trace("Attempting XAllocColorCells() for class %s", displayclassname(vp->class));

		contig = True;
		colormap = makecolmap(display, vp -> visual, AllocNone);
		cells = 1<<lg(maxsize(vp));
		npixels = 1;
		nplanes = (unsigned int) lg(maxsize(vp));
		/* 
		 * Just ask for a small number of planes if more was possible.
		 * Originally this test requested all planes - but of course this
		 * gave little scope for contig to make any difference.
		 */
		if(nplanes > 2) 
			nplanes = 2;
		plane_masks_return = (unsigned long *) malloc(nplanes * sizeof(unsigned long));
		pixels_return = (unsigned long *) malloc(npixels * sizeof(unsigned long));
		if(plane_masks_return == (unsigned long*) 0 || pixels_return == (unsigned long*) 0) {
			delete("Malloc() failed to allocate memory for return values.");
			return;
		} else
			CHECK;

		for(j=0; j<npixels; j++)
			pixels_return[j] = cells+1;

		for(j=0; j<nplanes; j++)
			plane_masks_return[j] = cells+1;

		trace("Testing XAllocColorCells() with contig = True, nplanes = %u, npixels = %u, cells = %d, pixels * 2 pow planes = %u", nplanes, npixels, cells, npixels * (1<<nplanes));
		
		status = XCALL;

		if(status == (Status) 0) {
			report("%s failed to return non-zero", TestName);
			report("for visual class %s.", 	
						displayclassname(vp->class));
			FAIL;
		} else {
			planeor = 0L;
			for(j=0; j < nplanes; j++)
				planeor |= plane_masks_return[j];

			if(bitcontig(planeor) == 0) {
				report("The allocated planemask bits were not contiguous.");
				FAIL;
			} else
				CHECK;
		}
	}

	CHECKPASS(2*nsupvis());
  
>>ASSERTION Good A
If the visual class
.S DirectColor
is supported:
When the
.A contig
argument is
.S True ,
and the visual type of the 
.A colormap
argument is 
.S DirectColor ,
then the mask formed by ORing the 
plane masks in
.A plane_mask_return
contains three sets of contiguous bits set to one, one lying in each of 
the red, green and blue pixel subfields.
>>STRATEGY
For the visual class DirectColor:
  Create a colormap.
  Allocate 1 color and some planemasks using XAllocColorCells and contig = True.
  Form the bitwise OR of all the planemasks per bit set.
  Verify that the returned planemasks in the red, green and blue subfields 
    were contiguous and distinct.
>>CODE
XVisualInfo *vp;
Visual *visual;
Status status;
Colormap cmap;
unsigned long vmask;
unsigned long planeor;
int j, cells;


	if( (vmask = visualsupported(display, 1<<DirectColor)) == 0L) {
		unsupported("DirectColor is not supported");
		return;
	}

	resetsupvis(vmask);
	nextsupvis(&vp);
	trace("Attempting XAllocColorCells() for class %s", displayclassname(vp->class));
	contig = True;
	colormap = makecolmap(display, vp -> visual, AllocNone);
	cells = 1<<lg(maxsize(vp));
	npixels = 1;
	nplanes = (unsigned int) lg(maxsize(vp));
	trace("nplanes %d", nplanes);
	/* 
	 * Just ask for a small number of planes if more was possible.
	 * Originally this test requested all planes - but of course this
	 * gave little scope for contig to make any difference.
	 */
	if(nplanes > 2) 
		nplanes = 2;
	trace("nplanes %d", nplanes);
	plane_masks_return = (unsigned long *) malloc(nplanes * sizeof(unsigned long));
	pixels_return = (unsigned long *) malloc(npixels * sizeof(unsigned long));
	if(plane_masks_return == (unsigned long*) 0 || pixels_return == (unsigned long*) 0) {
		delete("Malloc() failed to allocate memory for return values.");
		return;
	} else
		CHECK;


	for(j=0; j<npixels; j++)
		pixels_return[j] = cells+1;

	for(j=0; j<nplanes; j++)
		plane_masks_return[j] = cells+1;

	for(j=0; j < nplanes; j++)
	{
		trace("plane_mask_return is initially %x", plane_masks_return[j]);
	}

	trace("Testing XAllocColorCells() with nplanes = %u, npixels = %u, cells = %d, pixels * 2 pow planes = %u", nplanes, npixels, cells, npixels * (1<<nplanes));
		
	status = XCALL;

	if(status == (Status) 0) {
		report("XAllocColorCells() failed to return non-zero for visual class %s.", displayclassname(vp->class));
		FAIL;
	} else {
		unsigned long planeor1 = 0L, planeor2 = 0L, planeor3 = 0L, pln;

		for(j=0; j < nplanes; j++)
		{
			trace("plane_mask_return is %x", plane_masks_return[j]);
			pln = plane_masks_return[j];
			if (bitcount(pln) != 3) {
				report("Planemask %d did not have 3 bits set (0x%lx)", j, pln);
				FAIL;
			} else
				CHECK;
			planeor1 |= lowbit(pln);
			pln &= ~lowbit(pln);
			planeor2 |= lowbit(pln);
			pln &= ~lowbit(pln);
			planeor3 |= lowbit(pln);
			pln &= ~lowbit(pln);
		}

		trace("red_mask is %x", vp->red_mask);
		trace("green_mask is %x", vp->green_mask);
		trace("blue_mask is %x", vp->blue_mask);
		if(!((bitsubset(planeor1, vp->red_mask) &&
		      bitsubset(planeor2, vp->green_mask) &&
		      bitsubset(planeor3, vp->blue_mask)) ||
		     (bitsubset(planeor1, vp->red_mask) &&
		      bitsubset(planeor3, vp->green_mask) &&
		      bitsubset(planeor2, vp->blue_mask)) ||
		     (bitsubset(planeor2, vp->red_mask) &&
		      bitsubset(planeor1, vp->green_mask) &&
		      bitsubset(planeor3, vp->blue_mask)) ||
		     (bitsubset(planeor2, vp->red_mask) &&
		      bitsubset(planeor3, vp->green_mask) &&
		      bitsubset(planeor1, vp->blue_mask)) ||
		     (bitsubset(planeor3, vp->red_mask) &&
		      bitsubset(planeor1, vp->green_mask) &&
		      bitsubset(planeor2, vp->blue_mask)) ||
		     (bitsubset(planeor3, vp->red_mask) &&
		      bitsubset(planeor2, vp->green_mask) &&
		      bitsubset(planeor1, vp->blue_mask)))) {
			report("Planemasks (0x%lx, 0x%lx, 0x%lx) are not subsets of RGB masks (0x%lx, 0x%lx, 0x%lx)", planeor1, planeor2, planeor3, vp->red_mask, vp->green_mask, vp->blue_mask);
			FAIL;
		} else
			CHECK;
		if (bitcount(planeor1) != nplanes ||
		    bitcount(planeor2) != nplanes ||
		    bitcount(planeor3) != nplanes) {
			report("Planemasks not all distinct (0x%lx, 0x%lx, 0x%lx)", planeor1, planeor2, planeor3);
			FAIL;
		} else
			CHECK;
		if (!bitcontig(planeor1) ||
		    !bitcontig(planeor2) ||
		    !bitcontig(planeor3)) {
			report("Planemasks not all contiguous (0x%lx, 0x%lx, 0x%lx)", planeor1, planeor2, planeor3);
			FAIL;
		} else
			CHECK;
		
	}

	CHECKPASS(4+nplanes);

>>ASSERTION Bad A
.ER BadColor 
>>ASSERTION Bad A
When the
.A npixels
argument is zero, then a
.S BadValue
error occurs.
>>STRATEGY
Verify that with npixels = 0, nplanes = 0, with the DefaultColormap, 
  XAllocColorCells generates a BadValue error.
>>CODE BadValue

	colormap = DefaultColormap(display, DefaultScreen(display));
	npixels = 0;
	nplanes = 0;
	plane_masks_return = (unsigned long *) 0;
	pixels_return = (unsigned long *) 0;
	contig = False;
	XCALL;
	if(geterr() != BadValue)
		FAIL;
	else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Bad A
.ER BadValue contig True False
>>#HISTORY	Cal	Completed	Written in new format and style 4/12/90.
>>#HISTORY	Kieron	Completed		<Have a look>
>>#HISTORY	Cal	Completed		Writting code.
>>#HISTORY	Kieron	Completed		Bug fixes.
