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
 * $XConsortium: strclrs.m,v 1.13 94/04/17 21:04:15 rws Exp $
 */
>>TITLE XStoreColors CH05
void
XStoreColors(display, colormap, color, ncolors)
Display	*display = Dsp;
Colormap colormap = DefaultColormap(display, DefaultScreen(display));
XColor *color = &dummycol;
int ncolors = 1;
>>EXTERN
XColor dummycol = { 0L, 0,0,0, DoRed|DoGreen|DoBlue, 0 };
>>ASSERTION Good C
>># TODO 					dave - Jan 17
>># The X11R4 spec is vague about whether it's the closest available or 
>># or the actual values that are stored. 
>># O'Reilly Vol one page 190 has a footnote saying the actual values 
>># you tried to store may not be stored.
If any of the visual classes DirectColor, PseudoColor or Grayscale is supported:
A call to xname obtains the closest available RGB values for the
.A colormap
argument to those specified in the
.S red ,
.S green
and
.S blue
components of the
.S XColor
structures named by the
.A color
argument, and stores those values in the read-write colourmap 
entries specified by the
.M pixel
components of the
.S XColor
structures.
>>STRATEGY
For each visual class DirectColor, PseudoColor and GrayScale:
  Create a colourmap with alloc set to AllocAll;
  Store the color r = 0xffff g = 0xf1f0 b = 0x0ff8 in each cell of the colourmap with XStoreColors.
  Obtain the stored r,g b values with XQueryColors.  
  Store the obtained r,g b values in each cell with XStoreColors.
  Obtain the stored r, g, b values with XQueryColors.
  Verify that these values are identical to those originally obtained.
>>CODE
XVisualInfo 	*vp;
XColor		*colp, *cp, *testcol;
unsigned long 	vmask = (1<<DirectColor)|(1<<PseudoColor)|(1<<GrayScale);
unsigned long	i;

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		colormap = makecolmap(display, vp->visual, AllocAll);
		ncolors = maxsize(vp);

		color = (XColor *) malloc(ncolors * sizeof(XColor));
		if(color == (XColor *) 0) {
			delete("malloc() couldn't allocate %d cells for the XColor array.", ncolors);
			return;
		} else
			CHECK;

		for(i=0, colp = color; i < ncolors; i++, colp++) {
			colp->pixel = i;
			colp->red = 0xffff ^ (i*4);
			colp->green =  0xf1f0 ^ (i*8);
			colp->blue =  0x0ff8 ^ (i*6);
			colp->flags = (char) (DoRed|DoGreen|DoBlue);
		}

		XCALL;
		if(geterr() == Success)
			CHECK;

		/* Write colormap stored values back over the original array */
		XQueryColors(display, colormap, color, ncolors);

		XCALL;
		if(geterr() == Success)
			CHECK;

		testcol = (XColor *) malloc(ncolors * sizeof(XColor));
		if(testcol == (XColor *) 0) {
			delete("malloc() couldn't allocate %d cells for the XColor array.", ncolors);
			return;
		} else
			CHECK;

		for(i=0; i<ncolors; i++)
			testcol[i].pixel = i;

		XQueryColors(display, colormap, testcol, ncolors);

		for(i=0, colp = testcol, cp = color; i < ncolors; i++, colp++, cp++)
			if( (cp->red != colp->red) || (cp->green != colp->green) || (cp->blue != colp->blue) ) {
				report("cell %ld:  r %u g %u b %u instead of r %u g %u b %u",
						i,
						cp->red, cp->green, cp->blue,
						colp->red, colp->green, colp->blue);
				FAIL;
			}

		if( i == ncolors)
			CHECK;
		free((char *)color);
		free((char *)testcol);
	}
	CHECKPASS(5*nsupvis());

>>ASSERTION Good C
If any of the visual classes DirectColor, PseudoColor or Grayscale is supported:
A call to xname changes the red, green and blue values
in each of the 
.A ncolors 
read-write colourmap entries in accordance with the 
.M flags
components of the 
.S XColor
structures named by the
.A color
argument.
>>STRATEGY
For each supported visual class DirectColor, PseudoColor and GrayScale:
  Create a colormap with XCreateColormap.
  Obtain two distinct colormap values using XStoreColors and XQueryColors.
  For each possible combination DoRed, DoGreen and DoBlue in flags:
    Set all colormap cells to the first value with flags = DoRed|DoGreen|DoBlue
    Set all colormap cells to the second value with rbg components selected by the flag combination with XStoreColors.
    Verify that only the components of the colourcell specified by the flags were altered with XQueryColor.
>>CODE
XVisualInfo 	*vp;
XColor		*colp, *cp, *testcol, *refcol, *quercol;
unsigned long 	vmask = (1<<DirectColor)|(1<<PseudoColor)|(1<<GrayScale);
unsigned long	i,j;
char		flags;
XColor		*redp, *greenp, *bluep;

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		colormap = makecolmap(display, vp->visual, AllocAll);
		ncolors = maxsize(vp);

		refcol = (XColor *) malloc(ncolors * sizeof(XColor));
		testcol = (XColor *) malloc(ncolors * sizeof(XColor));
		quercol = (XColor *) malloc(ncolors * sizeof(XColor));

		if((quercol == (XColor *) 0) ||  (refcol == (XColor *) 0) ||  (testcol == (XColor *) 0)){
			delete("malloc() couldn't allocate %d cells for the XColor array.", ncolors);
			return;
		} else
			CHECK;

		for(i=0, cp = testcol, colp = refcol; i < ncolors; i++, cp++, colp++) {
			colp->pixel = i;
			colp->red = 0;
			colp->green = 0;
			colp->blue = 0;
			colp->flags = (DoRed|DoGreen|DoBlue);

			cp->pixel = i;
			cp->red = 0xffff;
			cp->green = 0xffff;
			cp->blue = 0xffff;
			cp->flags = (DoRed|DoGreen|DoBlue);

			quercol[(int)i].pixel = i;
		}

		color = refcol;
		XCALL;
		if(geterr() == Success)
			CHECK;

		/* Write colormap stored values back over the original array */
		XQueryColors(display, colormap, refcol, ncolors);


		color = testcol;
		XCALL;
		if(geterr() == Success)
			CHECK;
		/* Write colormap stored values back over the original array */
		XQueryColors(display, colormap, testcol, ncolors);


		for(i=0; i<8; i++) {

			flags = 0;

			if(i&DoRed) {
				flags |= DoRed;
				redp = testcol;
			} else
				redp = refcol;

			if(i&DoGreen) {
				flags |= DoGreen;
				greenp = testcol;
			} else
				greenp = refcol;

			if(i&DoBlue) {
				flags |= DoBlue;
				bluep = testcol;
			} else
				bluep = refcol;

			color = refcol;
			XCALL;
			if(geterr() == Success)
				CHECK;

			for(j=0, cp=testcol; j < ncolors; cp++, j++)
				cp->flags = flags;

			color = testcol;
			XCALL;
			if(geterr() == Success)
				CHECK;


			XQueryColors(display, colormap, quercol, ncolors);

			for(j=0, cp = quercol; j < ncolors; j++, cp++, redp++, greenp++, bluep++)
				if( (cp->red != redp->red) || (cp->green != greenp->green) || (cp->blue != bluep->blue) ) {
					report("cell %ld)  r %u g %u b %u instead of r %u g %u b %u",
							i,  cp->red, cp->green, cp->blue,  redp->red, greenp->green, bluep->blue);
					FAIL;
				}
			if (j == ncolors)
				CHECK;
		}

		free((char*)quercol);
		free((char*)testcol);
		free((char*)refcol);
	}
	CHECKPASS((3 + 3*i) * nsupvis());


>>ASSERTION Good D 1
When the 
.A colormap
is an installed colourmap for its screen, 
then any changes made by a call to xname are visible immediately.
>>ASSERTION Good C
If any of the visual classes DirectColor, PseudoColor or GrayScale is supported:
When one or more colourmap entries cannot be changed, then all the specified
colourmap entries that are allocated writable by any client are changed.
>>STRATEGY
If visual class is DirectColor, PseudoColor or GrayScale:
   Create a colormap with AllocNone
   Check >= 2 cells in colormap
   Allocate first cell as r/o, second as read write
   Update r/w cell to known value with XStoreColor.
   Discover what it was really set to, and remember.
   Update both cells, the r/w one to an opposite value, with xname
   Verify BadAccess occurred, will have Failed if not -- see CODE arg.
   Check that despite error the 2nd, r/w, cell was changed in value
   Repeat for next matching visual.
>>CODE BadAccess
XVisualInfo 	*vp;
XColor		*cp1;
XColor		*cp2;
XColor		colors[2];
unsigned long 	vmask = (1<<DirectColor)|(1<<PseudoColor)|(1<<GrayScale);
unsigned long	i;
unsigned long	junk;

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

/* If visual class is DirectColor, PseudoColor or GrayScale: */
	for(resetsupvis(vmask); nextsupvis(&vp); ) {

/*    Create a colormap with AllocNone */
		colormap = makecolmap(display, vp->visual, AllocNone);
		ncolors = maxsize(vp);
/*    Check >= 2 cells in colormap */
		if (ncolors < 2) {
			delete("Need at least two cells in colormap to test this assertion, only had %d available.", ncolors);
			return;
		} else
			CHECK;

		ncolors = 2;
		color = colors;

/*    Allocate first cell as r/o, second as read write */
		for(i=0, cp1 = color; i < 2; i++, cp1++) {
			cp1->pixel = i;
			cp1->green = cp1->blue = cp1->red = 0xffff ^ (i*32767);
			cp1->flags = (DoRed|DoGreen|DoBlue);
		}
		if (XAllocColor(display, colormap, &color[0]) == False) {
			delete("Could not allocate first cell as r/o.");
			return;
		} else
			CHECK;

		if (XAllocColorCells(display, colormap, True, &junk, 0,
					&color[1].pixel, 1) == False) {
			delete("Could not allocate second cell as r/w.");
			return;
		} else
			CHECK;

/*    Update r/w cell to known value with XStoreColor. */
		XStoreColor(display, colormap, &color[1]);
/*    Discover what it was really set to, and remember. */
		XQueryColor(display, colormap, &color[0]);

/*    Update both cells, the r/w one to an opposite value, with xname */
		color[1].red = color[1].green = color[1].blue ^= 0xffff;
		XCALL;
/*    Verify BadAccess occurred, will have Failed if not -- see CODE arg. */
		if (geterr() == BadAccess)
			CHECK;
		else
			report("Updating 2 cells, 1st r/o & 2nd r/w, did not give expected BadAccess error.");

/*    Check that despite error the 2nd, r/w, cell was changed in value */
		XQueryColor(display, colormap, cp2 = &color[1]);
		cp1 = cp2 - 1;
		/* now cp1 == remembered (different), cp2 == actual */

		if (cp1->red == cp2->red && cp1->green == cp2->green &&
					cp1->blue == cp2->blue) {
			report("After error the r/w cell was not updated: got r,g,b = %lu,%lu,%lu",
				cp2->red, cp2->green, cp2->blue);
			FAIL;
		} else
			CHECK;

/*    Repeat for next matching visual. */
	}

	CHECKPASS(5*nsupvis());

>>ASSERTION Bad C
If any of the visual classes DirectColor, PseudoColor or GrayScale is supported:
When the
.M pixel
component in more than one of the 
.S XColor
structures named by the
.A color
argument is not a valid entry in the colormap
argument, then a
.S BadValue
error occurs which will report any one of the
invalid pixel values.
>>STRATEGY
For each visual class DirectColor, PseudoColor and GrayScale:
  Create a colormap with alloc set to AllocAll.
  Store an array of 3 colorvalues with pixel values -1, 0, -2.
  Verify that a BadValue error occurs.
  Verify that the erroneous pixel was reported as either -1 or -2.
>>CODE BadValue
int		errval;
XVisualInfo 	*vp;
XColor		*cp1;
unsigned long 	vmask = (1<<DirectColor)|(1<<PseudoColor)|(1<<GrayScale);
XColor		colors[3];
unsigned long	i;

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {

		colormap = makecolmap(display, vp->visual, AllocAll);
		for(i=0, cp1 = colors; i < 3; i++, cp1++) {
			cp1->pixel = 0;
			cp1->red = 0xffff;
			cp1->green = 0;
			cp1->blue = 0;
			cp1->flags = (DoRed|DoGreen|DoBlue);
		}

		colors[0].pixel = -1;
		colors[1].pixel = 0;
		colors[2].pixel = -2;
		color = colors;
		ncolors = 3;
		XCALL;
		if(geterr() == BadValue) {
 			errval = getbadvalue();
                        if( (errval != -1) && (errval != -2) ) {
                                report("Erroneous value was reported as %d, instead of -1 or -2", errval);
                                FAIL;
                        } else {
                                CHECK;
				trace("Reported BadValue was %d", errval);
			}
		}
	}

	CHECKPASS(nsupvis());

		
>>ASSERTION Bad A
.ER BadAccess colormap-store
>>STRATEGY
For all visuals:
  Create a colormap with alloc set to AllocNone. (Unallocated for visual
	classes DirectColor, PseudoColor and GrayScale; R/O for visual
	classes TrueColor, StaticColor and StaticGray: all should
	provoke BadAccess)
  If visual class is not one of DirectColor, PseudoColor or GrayScale;
    Store distinct colors into all of the (already r/o) cells using xname.
    Verify that a BadAccess error is generated.
  Create a new client with XOpenDisplay.
  Allocate a readonly cell for the new client with XAllocColor.
  Store a value in the cell with xname.
  Verify that a BadAccess error occurred.
  Allocate a readonly cell with XAllocColor.
  Store a value in the cell with xname.
  Verify that a BadAccess error occurred.
>>CODE BadAccess
XVisualInfo	*vp;
XColor          *cp, *testcol;
Display		*disp2;
unsigned long	i;

	for(resetvinf(VI_WIN); nextvinf(&vp); ) {
		colormap = makecolmap(display, vp->visual, AllocNone);
		ncolors = maxsize(vp);
		testcol = (XColor *) malloc(ncolors * sizeof(XColor));

		if(testcol == (XColor *) 0) {
			delete("malloc() couldn't allocate %d cells for the XColor array.", ncolors);
			return;
		} else
			CHECK;

		for(i=0, cp = testcol; i < ncolors; i++, cp++) {
			cp->pixel = i;
			cp->green = cp->blue = cp->red = 0xffff ^ (i*3);
			cp->flags = (DoRed|DoGreen|DoBlue);
		}

		if (vp->class == TrueColor || vp->class == StaticColor ||
		    vp->class == StaticGray) { /* read-only map */

			trace("Testing a colormap with just r-o cells.");

			color = testcol;
			XCALL;
			if(geterr() == BadAccess)
				CHECK;
		} else {
			trace("Testing a colormap with unallocated cells.");
			XSync(display, False);
			CHECK;
		}

		trace("Open second client connection.");
		disp2 = opendisplay();

		if(XAllocColor(disp2, colormap, &testcol[0]) == False) {
			delete("XAllocColor() failed to allocate a r/o cell for a second client.");
			return;
		} else
			CHECK;

		trace("Trying to write into a r/o cell allocated by another client.");
		color = &testcol[0];
		ncolors = 1;
		XCALL;
		if(geterr() == BadAccess)
			CHECK;

		if(XAllocColor(display, colormap, &testcol[1]) == False) {
			delete("XAllocColor() failed to allocate a r/o cell");
			return;
		} else
			CHECK;

		trace("Trying to write into a r/o cell allocated by self.");
		color = &testcol[1];
		ncolors = 1;
		XCALL;
		if(geterr() == BadAccess)
			CHECK;
	}

	CHECKPASS(6*nvinf());

>>ASSERTION Bad A
.ER BadColor
>>ASSERTION Good D 3
When
.A rmask_return ,
.A gmask_return
and
.A bmask_return
have been returned by
a previous call to
.S XAllocColorPlanes
and  a read-write colourmap entry
for a pixel value is changed by a call to xname,  then the pixel value
is decomposed   into   three   components using   the   masks and  the
independent colourmap entries are updated.
>>ASSERTION Good C
If the visual class
.S DirectColor
is supported:
When the
.A colormap
argument was created with visual type
.S DirectColor
and with
.A alloc
set to
.S AllocAll ,
and a read-write colourmap entry for a pixel value is changed by a call to xname, then
the pixel value is decomposed into three components using the
.S red_mask ,
.S green_mask
and
.S blue_mask
in the visual and the independent colourmap entries are updated.
>>STRATEGY
If visual class DirectColor is supported:
   Create a colourmap using makecolmap and AlocAll
   Allocate an array of XColor's and one for each of reds, greens & blues
   Set up each of reds, greens & blues to have a distinguishable value,
     using xname, and make sure that's what's in the equivalent colourmaps.
   Read back some suitably "random" triples and check their components
   Verify that each component is the same as the corresponding entry in
     reds, greens & blues.
   Free the allocated storage: colors, reds, greens & blues.
>>EXTERN
#define lowbit(x)		((x) & (~(x) + 1))
#define bitcontig(x)	((((x) + lowbit(x)) & (x)) == 0)
#define bitsubset(a,b)	(((a) & (b)) == (a))

static int
maskshift(mask)
register unsigned long mask;
{
register int i;

	for (i = 0; mask; i++) {
		if (mask & 0x1)
			return i;
		mask >>= 1;
	}
	return i;
}

static
Bool set_one_col(refp, s, flags, mask)
	unsigned long *refp;
	char *s;
	char flags;
	unsigned long mask;
{
	unsigned long i;
	XColor *cp1;
	unsigned long *lrefp = refp;
	int fail = 0; /* used in the FAIL in XCall (spelt wrong intentionally) */
	/* expects to find color and ncolors in-scope */

	for(i=0, cp1 = color; i < ncolors; i++, cp1++, lrefp++) {
		unsigned long subfield_ix = i << maskshift(mask);

		if (!bitsubset(subfield_ix, mask)) {
			delete("Inconsistent maxsize() result: size of %d is too big to fit into %s mask 0x%lx (detected at index %lu, giving pixel 0x%lx)",
				ncolors, s, mask, i, subfield_ix);
			return False;
		}
		*lrefp = cp1->pixel = subfield_ix;
		cp1->flags = flags;
		switch (flags) {
		case DoRed:
			cp1->red = i;
			break;
		case DoGreen:
			cp1->green = i;
			break;
		case DoBlue:
			cp1->blue = i;
			break;
		default:
			delete("Flags 0x%x has more than one colour in it.", (unsigned int)flags);
			return False;
		}
	}
	/* The following may give a warning about the return; */
	XCALL;
	XQueryColors(display, colormap, color, ncolors);
	for(i=0,cp1=color,lrefp=refp; i < ncolors; i++, lrefp++, cp1++) {
		*lrefp = (flags==DoRed) ? cp1->red :
				((flags==DoGreen) ? cp1->green : cp1->blue);
	}
	return (geterr() == Success);
}
>>CODE
XVisualInfo 	*vp;
unsigned long 	vmask = (1<<DirectColor);
XColor		*colors;
unsigned long	i;
XColor		*refp;
unsigned long	*reds, *greens, *blues;
int		pathcnt = 0;
static XColor triples[] = {
		(unsigned long)~0L, 0,0,0, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 1,0,0, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 0,1,0, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 0,0,1, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 3,2,1, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 1,3,0, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 7,3,1, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 7,8,5, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 15,11,12, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 42,99,13, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 112,127,64, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 255,64,33, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 64,255,33, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 64,33,255, DoRed|DoGreen|DoBlue, 0,
		(unsigned long)~0L, 255,255,255, DoRed|DoGreen|DoBlue, 0
	};

/* If visual class DirectColor is supported: */
	if( (vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {

/*    Create a colourmap using makecolmap and AlocAll */
		colormap = makecolmap(display, vp->visual, AllocAll);
		ncolors = maxsize(vp);
/*    Allocate an array of XColor's and one for each of reds, greens & blues */
		colors = (XColor *) malloc(ncolors * sizeof(XColor));
		color = colors;
		reds = (unsigned long *) malloc(ncolors * sizeof(unsigned long));
		greens = (unsigned long *) malloc(ncolors * sizeof(unsigned long));
		blues = (unsigned long *) malloc(ncolors * sizeof(unsigned long));
		if ((colors == (XColor *) 0) || (reds == (unsigned long *) 0) ||
				(greens == (unsigned long *) 0) ||
				(blues == (unsigned long *) 0)) {
			delete("malloc() couldn't allocate %d cells for the XColor array.", ncolors);
			return;
		} else
			CHECK;

/*    Set up each of reds, greens & blues to have a distinguishable value, */
/*      using xname, and make sure that's what's in the equivalent colourmaps. */
		/*
		   kept in reds, greens or blues. This routine expects
		   color and ncolors to be set right. It uses XCall as well.
		*/
		if (!set_one_col(reds, "red", DoRed, vp->red_mask) ||
		    !set_one_col(greens, "green", DoGreen, vp->green_mask) ||
		    !set_one_col(blues, "blue", DoBlue, vp->blue_mask)
		   ) {
			delete("Failed to set up colourmap for test.");
			return;
		} else
			CHECK;

/*    Read back some suitably "random" triples and check their components */
		for (i=0, refp=triples; i < NELEM(triples); i++, refp++) {
			XColor testcol;
			unsigned long r,g,b;

			if (refp->red >= (unsigned)ncolors ||
					refp->green >= (unsigned)ncolors ||
					refp->blue >= (unsigned)ncolors)
				continue;

			r = refp->red << maskshift(vp->red_mask);
			if (!bitsubset(r, vp->red_mask))
				continue;
			g = refp->green << maskshift(vp->green_mask);
			if (!bitsubset(g, vp->green_mask))
				continue;
			b = refp->blue << maskshift(vp->blue_mask);
			if (!bitsubset(b, vp->blue_mask))
				continue;
			testcol.pixel = r | g | b;

			XQueryColor(display, colormap, &testcol);

/*    Verify that each component is the same as the corresponding entry in */
/*      reds, greens & blues. */
			if (testcol.flags != (DoRed | DoGreen | DoBlue) ||
			    testcol.red != reds[refp->red] ||
			    testcol.green != greens[refp->green] ||
			    testcol.blue != blues[refp->blue]) {
				report("Expected r,g,b = %lu, %lu, %lu but got %lu, %lu, %lu for pixel 0x%lx with r-,g-,b-masks 0x%lx, 0x%lx, 0x%lx",
					reds[refp->red], greens[refp->green], blues[refp->blue],
					testcol.red, testcol.green, testcol.blue,
					testcol.pixel, vp->red_mask,
					vp->green_mask, vp->blue_mask);
				FAIL;
			} else
				CHECK;

			pathcnt++;
		}

/*    Free the allocated storage: colors, reds, greens & blues. */
		free((char*)colors);
		free((char*)reds);
		free((char*)greens);
		free((char*)blues);
	}

	CHECKPASS(pathcnt+2*nsupvis());
			

>>#HISTORY	Cal 	Completed	Written in new style and format.
>>#HISTORY	Kieron	Completed		<Have a look>
>>#HISTORY	Cal	Action		Writing code.
