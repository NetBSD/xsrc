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
 * $XConsortium: strclr.m,v 1.18 94/04/17 21:04:14 rws Exp $
 */
>>TITLE XStoreColor CH05
void
XStoreColor(display, colormap, color)
Display	*display = Dsp;
Colormap colormap = DefaultColormap(display, DefaultScreen(display));
XColor *color = &dummycol;
>>EXTERN
XColor dummycol;
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
structure named by the
.A color
argument, and stores those values in the read-write colourmap entry 
specified by the
.M pixel
components of the
.S XColor
structure.
>>STRATEGY
For each visual class DirectColor, PseudoColor and GrayScale:
  Create a colormap with alloc set to AllocAll.
  Store the color r = 0xffff g = 0xf1f0 b = 0x0ff8 at cell 0.
  Obtain the rbg values of the stored color with XQueryColor.
  Store the returned rgb values in cell 0
  Obtain the stored values with XQueryColor.
  Verify the rgb values stored in both cases were identical.
>>CODE
XVisualInfo *vp;
XColor		testcol, storedcol, savedcol;
unsigned long vmask = (1<<DirectColor)|(1<<PseudoColor)|(1<<GrayScale);

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		unsupported("DirectColor, PseudoColor and GrayScale are not supported");
		return;
	}

	storedcol.pixel = 0L;
	testcol.pixel = 0L;
	testcol.red = 0xffff;
	testcol.green = 0xf1f0;
	testcol.blue = 0x0ff8;
	testcol.flags = DoRed|DoGreen|DoBlue;

	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		colormap = makecolmap(display, vp->visual, AllocAll);
		color = &testcol;
		XCALL;
		XQueryColor(display, colormap, &storedcol);
		savedcol = storedcol;
		color = &storedcol;
		XCALL;
		XQueryColor(display, colormap, &testcol);
		
		if( (savedcol.red != testcol.red) || (savedcol.green != testcol.green) || (savedcol.blue != testcol.blue) ) {
			report("XStoreColor() return RGB values r %u g %u b %u instead of r %u g %u b %u.", 
				savedcol.red, savedcol.green, savedcol.blue, testcol.red, testcol.green, testcol.blue);
			FAIL;
		} else
			CHECK;
	}
	CHECKPASS(nsupvis());

>>ASSERTION Good C
If any of the visual classes DirectColor, PseudoColor or GrayScale is supported:
A call to xname changes the red, green and blue values
in the read-write colourmap entry in accordance with the 
.M flags
component of the 
.S XColor
structure named by the
.A color
argument.
>>STRATEGY
For each supported visual class DirectColor, PseudoColor and GrayScale:
  Create a colormap with XCreateColormap.
  Store the color r = 0xffff g = 0xf1f0 b = 0x0ff8 at cell 0 with XStoreColor.
  Obtain the r,g and b values with XQueryColor.
  For each possible combination DoRed, DoGreen and DoBlue in flags:
    Store the bitwise complementary rgb values in the same cell with XStoreColor.
    Store the color r = 0xffff g = 0xf1f0 b = 0x0ff8 at cell 0 with XStoreColor.
    Verify that only the components of the colourcell specified by the flags value have been altered with XQueryColor.

>>CODE
XVisualInfo *vp;
int 		i;
XColor		namedcol, testcol, storedcol;
unsigned long 	vmask = (1<<DirectColor)|(1<<PseudoColor)|(1<<GrayScale);
unsigned short	redval, greenval, blueval;
char		flags;

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

	for(resetsupvis(vmask); nextsupvis(&vp); ) {

		testcol.pixel = 0L;
		testcol.red = 0xffff;
		testcol.green = 0xf1f0;
		testcol.blue = 0x0ff8;
		testcol.flags = DoRed|DoGreen|DoBlue;
	
		color = &testcol;
		colormap = makecolmap(display, vp->visual, AllocAll);
		XCALL;

		namedcol.pixel = 0L;
		XQueryColor(display, colormap, &namedcol);     /* named color has expected r,g,b values.*/

		testcol = namedcol;
		testcol.red ^= 0xffff;
		testcol.green ^= 0xffff;
		testcol.blue ^= 0xffff;
		testcol.flags = DoRed|DoGreen|DoBlue;
		color = &testcol;
		XCALL;
		XQueryColor(display, colormap, &testcol);      /* test color has the unexpected rgb vals */

		for(i=0; i<8; i++) {

			flags = 0;

			if(i&DoRed) {
				flags |= DoRed;
				redval = namedcol.red;
			} else
				redval = testcol.red;

			if(i&DoGreen) {
				flags |= DoGreen;
				greenval = namedcol.green;
			} else
				greenval = testcol.green;

			if(i&DoBlue) {
				flags |= DoBlue;
				blueval = namedcol.blue;
			} else
				blueval = testcol.blue;

			testcol.flags = DoRed|DoGreen|DoBlue;
			color = &testcol;
			XCALL;    /* Sore the inv colours */

			namedcol.flags = flags;
			color = &namedcol;
			XCALL;
			storedcol.pixel = 0L;
			storedcol.flags =  DoRed|DoGreen|DoBlue;
			XQueryColor(display, colormap, &storedcol);

			if( (redval != storedcol.red) || (greenval != storedcol.green) || (blueval != storedcol.blue) ) {
				report("XStoreColor() flags = %d RGB value r %u g %u b %u instead of r %u g %u b %u.", 
					(int) i,
					storedcol.red, storedcol.green, storedcol.blue, redval, greenval, blueval);
					FAIL;
				} else
					CHECK;
		}
	}

	CHECKPASS(i*nsupvis());

>>ASSERTION Good D 1
If any of the visual classes DirectColor, PseudoColor or GrayScale is supported:
When the 
.A colormap
is an installed colourmap for its screen, 
then any changes made by a call to xname are visible immediately.
>>ASSERTION Bad C
If any of the visual classes DirectColor, PseudoColor or GrayScale is supported:
When the 
.M pixel 
component in the
.S XColor
structure named by the
.A color
argument is not a valid entry in the
.A colormap
argument, 
then a
.S BadValue
error occurs.
>>STRATEGY
For each visual class DirectColor, PseudoColor and GrayScale:
  Create a colourmap with alloc set to AllocAll.
  Store the color r = 0xffff g = 0xf1f0 b = 0x0ff8 using XStoreColor with pixel = colormap_size.
  Verify that a BadValue error is generated.
>>CODE BadValue
XVisualInfo	*vp;
XColor		testcol;
unsigned long	vmask = (1<<DirectColor)|(1<<PseudoColor)|(1<<GrayScale);

	if( (vmask = visualsupported(display, vmask)) == 0L) {
		UNSUPPORTED;
		return;
	}

	testcol.red = 0xffff;
	testcol.green = 0xf1f0;
	testcol.blue = 0x0ff8;
	color = &testcol;
	for(resetsupvis(vmask); nextsupvis(&vp); ) {
		colormap = makecolmap(display, vp->visual, AllocAll);
		testcol.pixel = maxsize(vp) + 1;
		if (vp->class == DirectColor) {
			testcol.pixel = (vp->red_mask | vp->blue_mask | vp->green_mask);
			testcol.pixel |= testcol.pixel << 1;
		}
		trace("Testing with pixel set to > colormap_size (%ld).",testcol.pixel);
		
		XCALL;
		if(geterr() == BadValue)
			CHECK;

	}

	CHECKPASS(nsupvis());

>>ASSERTION Bad A
.ER BadAccess colormap-store
>>STRATEGY
>># For each visual class DirectColor, PseudoColor and GrayScale:
For all visuals:
  Create a colormap with alloc set to AllocNone. (Unallocated for visual
	classes DirectColor, PseudoColor and GrayScale; R/O for visual
	classes TrueColor, StaticColor and StaticGray: all should
	provoke BadAccess)
  For each colormap cell:
    Store the color r = 0xffff g = 0xf1f0 b = 0x0ff8 using xname.
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
XColor		testcol;
Display		*disp2;
unsigned long	i;

	for(resetvinf(VI_WIN); nextvinf(&vp); ) {
	
		testcol.red = 0xffff;
		testcol.green = 0xf1f0;
		testcol.blue = 0x0ff8;

		color = &testcol;

		colormap = makecolmap(display, vp->visual, AllocNone);
		trace("Testing a colormap with no allocated/just r-o cells.");
		for(i=0; i < maxsize(vp); i++) {
			testcol.pixel = i;
			XCALL;
			/* the XCALL will have done a check for geterr() ==
			   BadAccess (c.f. arg. after CODE keyword above)
			   and will FAIL if not.
			*/
		}
		if(i == maxsize(vp))
			CHECK;

		disp2 = opendisplay();

		if(XAllocColor(disp2, colormap, &testcol) == False) {
			delete("XAllocColor() failed to allocate a r/o cell for a second client.");
			return;
		} else
			CHECK;

		trace("Trying to write into a r/o cell allocated by another client.");
		color = &testcol;
		XCALL;
		if(geterr() == BadAccess)
			CHECK;

		testcol.red ^= 0xffff;
		testcol.green ^= 0xffff;
		testcol.blue ^= 0xffff;

		if(XAllocColor(display, colormap, &testcol) == False) {
			delete("XAllocColor() failed to allocate a r/o cell");
			return;
		} else
			CHECK;

		trace("Trying to write into a r/o cell allocated by self.");
		color = &testcol;
		XCALL;
		if(geterr() == BadAccess)
			CHECK;
	}

	CHECKPASS(5*nvinf());
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

static int ncolors = 0;

static
Bool set_one_col(refp, s, flags, mask)
	unsigned long *refp;
	char *s;
	char flags;
	unsigned long mask;
{
	unsigned long i;
	XColor *cp1;
	XColor *save_col = color; /* for restoring color, later */
	unsigned long *lrefp = refp;
	int fail = 0; /* used in the FAIL in XCall (spelt wrong intentionally) */
	/* expects to find color and ncolors in-scope */

	for(i=0, cp1 = color; i < ncolors; i++, cp1++) {
		unsigned long subfield_ix = i << maskshift(mask);

		if (!bitsubset(subfield_ix, mask)) {
			delete("Inconsistent maxsize() result: size of %d is too big to fit into %s mask 0x%lx (detected at index %lu, giving pixel 0x%lx)",
				ncolors, s, mask, i, subfield_ix);
			return False;
		}
		cp1->pixel = subfield_ix;
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
			color = save_col;
			return False;
		}
		color = cp1;
		/* The following may generate a warning about the return; */
		XCALL;
	}
	color = save_col;
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

	/* now it's safe to call set_one_col: color and ncolors are set. */


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
>>#HISTORY	Cal	Completed	Written in new style and format.
>>#HISTORY	Kieron	Completed		<Have a look>
>>#HISTORY	Cal	Action		Writing Code.
