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
 * $XConsortium: crtclrmp.m,v 1.13 94/04/17 21:03:52 rws Exp $
 */
>>TITLE XCreateColormap CH05
Colormap
XCreateColormap(display, win, visual, alloc)
Display	*display = Dsp;
Window	win = DRW(display);
Visual	*visual = &dummyvisual;
int 	alloc = AllocNone;
>>EXTERN
Visual dummyvisual;
>>ASSERTION Good A
A call to xname
creates a colourmap of the type
.A visual
on the screen for which the 
.A window 
argument was created
and returns a colourmap ID.
>>STRATEGY
For each visual type
  Create a colourmap for that visual.
  Verify that the colourpmap is usable by  Installing it.
>>CODE
XVisualInfo *vp;
Colormap	cmap;
Display		*d2;
Window		w2;
int i;


	alloc = AllocNone;
	for (resetvinf(VI_WIN); nextvinf(&vp); ) {

		win = makewin(display, vp);
		visual = vp->visual;
		cmap = XCALL;
		/*
		 * The only way that we can really check that we have a valid colourmap
		 * is to try and use it.  So try to install it.  Of course it
		 * could be that the install routine is broken but the point is that
		 * something is wrong.  Print out both routine names to alert the tester
		 * about possible sources of the problem.
		 */
		CATCH_ERROR(display);
		XInstallColormap(display, cmap);
		RESTORE_ERROR(display);
		if (GET_ERROR(display) != Success) {
			report("Result of a XCreateColormap could not be used with XInstallColormap");
			FAIL;
		} else {
			CATCH_ERROR(display);
			XUninstallColormap(display, cmap);
			RESTORE_ERROR(display);

			if (GET_ERROR(display) != Success) {
				report("Result of a XCreateColormap could not be used with XUninstallColormap");
				FAIL;
			} else 
				CHECK;
		}
	}
	CHECKPASS(nvinf());

>>ASSERTION Good D 2
If any of the visual classes
.S StaticGray ,
.S StaticColor
or
.S TrueColor
are supported:
When xname is called with a visual class that is one of
.S StaticGray ,
.S StaticColor
or
.S TrueColor
and alloc is
.S AllocNone ,
then the entries have defined values specific to the visual that are
implementation defined.
>>STRATEGY
For the visual classes TrueColor, StaticColor and StaticGray:
  Verify that a colormap of that class can be created with XCreateColormap.
>>CODE
Colormap	cmap;
XVisualInfo	*vp;
unsigned long vmask = ((1L<<TrueColor) | (1L<<StaticColor) | (1L<<StaticGray));

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		unsupported("TrueColor, StaticColor and StaticGray are not supported");
		return;
	}

	alloc = AllocNone;
	for ( resetsupvis(vmask); nextsupvis(&vp); ) {

		visual = vp->visual;
		cmap = XCALL;
		if (geterr() == Success)
			CHECK;
	}

	CHECKUNTESTED(nsupvis());

>>ASSERTION Good C
If any of the visual classes
.S PseudoColor ,
.S GrayScale
or
.S DirectColor
are supported:
When xname is called with a visual class that is one of
.S PseudoColor ,
.S GrayScale
or
.S DirectColor
and alloc is
.S AllocNone ,
then the colourmap has no allocated entries.
>>STRATEGY
For each visual class PseudoColor, GrayScale and DirectColor:
  Create a colourmap with alloc set to AllocNone.
  Verify that no cells were allocated by Allocating all the colourmap cells
   with XAllocColorCells.
>>CODE
Colormap	cmap;
XVisualInfo	*vi;
Status 		status;
unsigned long	*pix;
unsigned long	vmask;
int 	n;


#define RW_COLORMAP_MASK ((1L<<DirectColor) | (1L<<PseudoColor) | (1L<<GrayScale))

	if( (vmask = visualsupported(display, RW_COLORMAP_MASK)) == 0L) {
		unsupported("DirectColor, PseudoColor and GrayScale are not supported");
		return;
	}

	alloc = AllocNone;
	for(resetsupvis(vmask); nextsupvis(&vi); ) {

		visual = vi->visual;
		cmap = XCALL;
		n = maxsize(vi);
		pix = (unsigned long *)malloc(n * sizeof(unsigned long));

		if (pix == NULL) {
			delete("malloc failed for pix array");
			return;
		}

		trace("allocate %d colour cells", n);
		status = XAllocColorCells(display, cmap, False, (unsigned long *)0, 0, pix, n);
		free((void*)pix);
		if (status == False) {
			report("Unable to allocate all colour cells");
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(nsupvis());

>>ASSERTION Good A
If any of the visual classes
.S PseudoColor ,
.S GrayScale
or
.S DirectColor
are supported:
When xname is called with a visual class that is one of
.S PseudoColor ,
.S GrayScale
or
.S DirectColor
and the argument
.A alloc 
is
.S AllocAll ,
then the entire colourmap is allocated read-write.
>>STRATEGY
For each of the visual classes DirectColor, PseudoColor and GrayScale:
  Create a colormap with alloc set to AllocAll.
  Verify that the allocation of another r/w cell fails with XAllocColorCells.
>>CODE
XVisualInfo	*vi;
Colormap	cmap;
Status	status;
unsigned long	pix;
unsigned long 	vmask;

#define RW_COLORMAP_MASK ((1L<<DirectColor) | (1L<<PseudoColor) | (1L<<GrayScale))

	if( (vmask = visualsupported(display, RW_COLORMAP_MASK)) == 0L) {
		unsupported("DirectColor, PseudoColor and GrayScale are not supported");
		return;
	}

	alloc = AllocAll;
	for (resetsupvis(vmask); nextsupvis(&vi); ) {

		visual = vi->visual;
		cmap = XCALL;

		/*
		 * If the entire colourmap is allocated then it should not be
		 * possible to allocate any more entries.
		 */

		status = XAllocColorCells(display, cmap, False, (unsigned long*)0, 0, &pix, 1);

		if (status != (Status) 0) {
			report("There was an unallocated colour cell");
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(nsupvis());

>>ASSERTION Good C
If either of the visual classes
.S GrayScale 
or
.S PseudoColor
are supported: 
When the visual class is
.S GrayScale 
or
.S PseudoColor , 
and the argument 
.A alloc
is 
.S AllocAll ,
then a call to xname 
allocates N read-write colourmap entries with 
pixel values from zero to N \- 1,
where N is the number of colourmap entries value in the specified visual.
>>STRATEGY
For each of the visuals GrayScale and PseudoColor:
  Create a colormap with alloc set to AllocAll.
  For each of the 0..N-1 allocated cells:
    Write value r 31<<8, g 63<<8, b 127<<8 with XStoreColor.
>>CODE
Colormap	cmap;
XVisualInfo	*vi;
XColor		col;
unsigned long	pix;
unsigned long		vmask 	= (1L<<PseudoColor) | (1L<<GrayScale);
int 		n;

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		unsupported("PseudoColor and GrayScale are not supported");
		return;
	}

	col.red = 31<<8;
	col.green = 63<<8;
	col.blue = 127<<8;
	col.flags = DoRed|DoGreen|DoBlue;

	alloc = AllocAll;
	for(resetsupvis(vmask); nextsupvis(&vi); ) {

		visual = vi->visual;
		cmap = XCALL;
		if(geterr() != Success)
			continue;

		n = maxsize(vi);
		for(pix = 0L; pix < n; pix++) {
			col.pixel = pix;

			CATCH_ERROR(display);
			XStoreColor(display, cmap, &col);
			RESTORE_ERROR(display);
	
			if(GET_ERROR(display) != Success) {
				report("Pixel %lu could not be written in the colourmap.", pix);
				FAIL;
			}
		}

		if(pix == n)
			CHECK;
	}
	
	CHECKPASS(nsupvis());

>>ASSERTION Good C
If the visual class
.S DirectColor
is supported:
When the visual class is
.S DirectColor
and the argument
.A alloc
is
.S AllocAll,
then a call to xname 
allocates 
.S 2pow(nred+ngreen+nblue) 
read-write colourmap entries with 
pixel values obtained by ORing zero or more
of the planes obtained by ORing the 
.M red_mask ,
.M green_mask
and 
.M blue_mask 
values in the 
.A visual
argument,
where 
.S nreds ,
.S ngreens
and
.S nblues
are the number of bits in the respective masks.
>>STRATEGY
For the visual Directcolor:
  Create a colormap with alloc set to AllocAll.
  For each of the 0..N-1 allocated cells:
    Write value r 31<<8, g 63<<8, b 127<<8 with XStoreColor.
>>CODE
>>#
>># WARNING.
>># For now this test assumes that the OR of the
>># rgb masks will form a contiguous set of bits. This makes
>># generation of all the subsets very straightforward.
>>#
#define  NUM_COLS 4096
Colormap	cmap;
XVisualInfo	*vi;
XColor		colarr[NUM_COLS];
unsigned long	plane, count;
unsigned long		vmask = (1L<<DirectColor), b, planemask;
int 		n, i, j, bitsum;
int		bitpos[sizeof(long) * 8];


	if( (vmask = visualsupported(display, vmask)) == 0L) {
		unsupported("DirectColor is not supported");
		return;
	}

	for(i=0; i<NUM_COLS; i++) {
		colarr[i].red = 31<<8;
		colarr[i].green = 63<<8;
		colarr[i].blue = 127<<8;
		colarr[i].flags = DoRed|DoGreen|DoBlue;
	}		

	resetsupvis(vmask); 
	nextsupvis(&vi);

	alloc = AllocAll;
	visual = vi->visual;
	cmap = XCALL;
	if(geterr() == Success) {
		bitsum = (bitcount(vi->red_mask) + bitcount(vi->green_mask) + bitcount(vi->blue_mask));
		n = 1<< bitsum;
		for(count = 0L; count < n; count+=NUM_COLS) { 
			for (i=0; i<NUM_COLS; i++)
				colarr[i].pixel = count+i;
			CATCH_ERROR(display);
			XStoreColors(display, cmap, colarr,
				(count+NUM_COLS <= n) ? NUM_COLS : n-count);
			RESTORE_ERROR(display);
	
			if(GET_ERROR(display) != Success) {
				report("Pixel in range [%lu-%lu] could not be written in the colourmap.", count,count+NUM_COLS);
				FAIL;
			} 

		}
	}

	if (count >= n)
		CHECK;

	CHECKPASS(1);

>>ASSERTION Good C
If any of the visual classes
.S PseudoColor ,
.S GrayScale
or
.S DirectColor
are supported:
When xname is called with a visual class that is one of
.S PseudoColor ,
.S GrayScale
or
.S DirectColor
and the
.A alloc
argument is
.S AllocAll ,
then none of the entries can be freed with
.S XFreeColors().
>>STRATEGY
For the visual classes DirectColor, PseudoColor and GrayScale:
  Create a colourmap with XCreateColormap with alloc set to AllocAll.
  Deallocate a colorcell with XFreeColor.
  Allocate a colorcell with XAllocColorCell.
  Verify that the allocation failed.
>>CODE
XVisualInfo	*vi;
Colormap	cmap;
Status	status;
unsigned long	pix;
unsigned long 	vmask = ((1L<<DirectColor) | (1L<<PseudoColor) | (1L<<GrayScale));

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		unsupported("DirectColor, PseudoColor and GrayScale are not supported");
		return;
	}

	alloc = AllocAll;
	for (resetsupvis(vmask); nextsupvis(&vi); ) {

		visual = vi->visual;
		cmap = XCALL;

		status = XAllocColorCells(display, cmap, False, (unsigned long*)0, 0, &pix, 1);

		pix = 0;

		CATCH_ERROR(display);
		XFreeColors(display, cmap, &pix, 1, 0);
		RESTORE_ERROR(display);
		if( GET_ERROR(display) == Success) {
			delete("XFreeColors() did not give an error deallocating a cell.");
			FAIL;
		} else
			CHECK;

		status = XAllocColorCells(display, cmap, False, (unsigned long*)0, 0, &pix, 1);

		if (status != (Status) 0) {
			report("A cell was deallocated by XFreeColors().");
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(2*nsupvis());

>>ASSERTION Bad C
If any of the visual classes
.S StaticGray , 
.S StaticColor
or
.S TrueColor
are supported: 
When xname is called with a visual that is one of
.S StaticGray , 
.S StaticColor
or
.S TrueColor , 
and the argument
.A alloc
is other than
.S AllocNone ,
then a
.S BadMatch 
error occurs.
>>STRATEGY
For each visual class in TrueColor, StaticColor and StaticGray:
  Call XCreateColormap with Alloc set to AllocAll.
  Verify that a BadMatch error occurred.
>>CODE BadMatch
Colormap	cmap;
XVisualInfo	*vp;
unsigned long vmask = (1L<<TrueColor) | (1L<<StaticColor) | (1L<<StaticGray);

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		unsupported("TrueColor, StaticColor and StaticGray are not supported");
		return;
	}

	alloc = AllocAll;
	for ( resetsupvis(vmask); nextsupvis(&vp); ) {
		visual = vp->visual;
		cmap = XCALL;
		if (geterr() == BadMatch)
			CHECK;
	}

	CHECKPASS(nsupvis());

>>ASSERTION Bad D 3
>># Can't see how to obtain a visual that isn't
>># supported in order to provoke the error.
If there is a visual class that is not supported on the screen:
When xname is called with a visual that is not supported
on the screen for which the 
.A window 
argument was created,
then a
.S BadMatch 
error occurs.
>>ASSERTION Bad A
.ER Value alloc AllocNone AllocAll
>>ASSERTION Bad A
When xname is called with an invalid visual,
then a
.S BadValue
error occurs.
>>STRATEGY
Make a visual structure invalid using badvis.
Create a colourmap for this visual with XCreateColorMap.
Verify that a BadValue error occurred.
>>CODE BadValue
Visual	vi;

	alloc = AllocNone;
	badvis(&vi);
	visual = &vi;
	XCALL;
	if(geterr() == BadValue)
		PASS;
>>ASSERTION Bad A
.ER BadWindow
>>ASSERTION Bad B 1
.ER BadAlloc
>>#HISTORY	Steve	Completed	Written in old format.
>>#HISTORY	Cal 	Completed	Re-written in new style and format.
>>#HISTORY	Kieron	Completed		<Have a look>
>>#HISTORY	Cal	Action		Writing code.
