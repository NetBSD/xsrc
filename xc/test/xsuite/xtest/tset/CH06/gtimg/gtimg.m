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
 * $XConsortium: gtimg.m,v 1.16 94/04/17 21:05:37 rws Exp $
 */
>>TITLE XGetImage CH06
XImage *
XGetImage(display, d, x, y, width, height, plane_mask, format)
Display *display = Dsp;
Drawable d;
int x = 0;
int y = 0;
unsigned int width = 1;
unsigned int height = 1;
long plane_mask = AllPlanes;
int format = ZPixmap;
>>EXTERN
/*
 * pre_xcall() - set globals
 */
static void
pre_xcall(win, ap, p, f)
Drawable win;
struct area *ap;
long p;
int f;
{
	d = win;
	x = ap->x;
	y = ap->y;
	width = ap->width;
	height = ap->height;
	plane_mask = p;
	format = f;
}

/*
 * plane masks for plane mask-related tests.
 * exercises each plane individually and various combinations of
 * planes.
 */
static long planelist[] = {
	0,
	1<<0,
	1<<1,
	1<<2,
	1<<3,
	1<<4,
	1<<5,
	1<<6,
	1<<7,
	1<<8,
	1<<9,
	1<<10,
	1<<11,
	1<<12,
	1<<13,
	1<<14,
	1<<15,
	1<<16,
	1<<17,
	1<<18,
	1<<19,
	1<<20,
	1<<21,
	1<<22,
	1<<23,
	1<<24,
	1<<25,
	1<<26,
	1<<27,
	1<<28,
	1<<29,
	1<<30,
	1<<31,
	0x23, 0xf8, 0x765, 0x3987, 0x129078, 0x23567193
};

/*
 * This list contains a number of areas suitable for getting
 * from the drawable.
 * This list need not contain as many members as the planelist.
 * The list is treated as if it were circular.
 */
static struct area arealist[] = {
	{0, 0, W_STDWIDTH, W_STDHEIGHT},
	{W_STDWIDTH/2, W_STDHEIGHT/2, W_STDWIDTH/4, W_STDHEIGHT/4},
	{W_STDWIDTH/2, W_STDHEIGHT/2, W_STDWIDTH/2, W_STDHEIGHT/2},
	{0, 0, 1, 1},
	{1, 1, 1, 1},
	{1, 1, 2, 2},
	{W_STDWIDTH/2, W_STDHEIGHT/2, 3, 3},
	{W_STDWIDTH-1, W_STDHEIGHT-1, 1, 1},
	{W_STDWIDTH-5, W_STDHEIGHT-5, 4, 4}
};
>># MODIFIED	peterc	As per external review comments.
>>ASSERTION Good A
A call to xname returns a pointer to an
.S XImage 
structure containing
the contents of the specified rectangle
with upper left corner at
[
.A x ,
.A y
]
relative to the origin of the drawable
.A d
and with width
.A width
and height
.A height
in the format specified by the
.A format
argument.
>>STRATEGY
Create drawable.
Write known pattern to drawable.
Call XGetImage with XYPixmap format.
Verify XGetImage return value is not null.
Verify depth, width, height, and format values in gotten image structure.
Verify gotten image for known pattern.
Repeat for ZPixmap format.
Destroy images using XDestroyImage.
Repeat for each visual.
>>CODE
XVisualInfo *vp;
Window w;
XImage *im1, *im2;
static struct area area1 =
	{ 0, 0, W_STDWIDTH, W_STDHEIGHT };
static struct area area2 =
	{ W_STDWIDTH/2, W_STDHEIGHT/2, W_STDWIDTH/4, W_STDHEIGHT/4 };

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		im1 = im2 = (XImage *) 0;

/* Create drawable. */
		/* do XYPixmap testing */
		trace("XYPixmap");
		w = makewin(display, vp);

/* Write known pattern to drawable. */
		dset(display, w, W_FG);

/* Call XGetImage with XYPixmap format. */
		pre_xcall(w, &area1, AllPlanes, XYPixmap);
		im1 = XCALL;

/* Verify XGetImage return value is not null. */
		if (im1 == (XImage *) 0) {
			report("Null image returned.");
			FAIL;
			return;
		}
		else
			CHECK;

/* Verify depth, width, height, and format values in gotten image structure. */
		if (checkimgstruct(im1, vp->depth, area1.width, area1.height, XYPixmap) == False)
			FAIL;
		else
			CHECK;

/* Verify gotten image for known pattern. */
		if (checkimg(im1, (struct area *) 0, W_FG, W_FG, 0) == False) {
			report("Image was not properly gotten.");
			FAIL;
		}
		else
			CHECK;

/* Repeat for ZPixmap format. */
		/* do ZPixmap testing */
		trace("ZPixmap");
		w = makewin(display, vp);
		dset(display, w, W_BG);

		pre_xcall(w, &area2, AllPlanes, ZPixmap);
		im2 = XCALL;
		if (im2 == (XImage *) 0) {
			report("Null image returned.");
			FAIL;
			return;
		}
		else
			CHECK;

		if (checkimgstruct(im2, vp->depth, area2.width, area2.height, ZPixmap) == False)
			FAIL;
		else
			CHECK;

		if (checkimg(im2, (struct area *) 0, W_BG, W_BG, 0) == False) {
			report("Image was not properly gotten.");
			FAIL;
		}
		else
			CHECK;
	
/* Destroy images using XDestroyImage. */
		if (im1 != (XImage *) 0)
			(void) XDestroyImage(im1);
		if (im2 != (XImage *) 0)
			(void) XDestroyImage(im2);

/* Repeat for each visual. */
	}

	CHECKPASS(3*2*nvinf());
>>ASSERTION Good A
When the
.A format
is 
.S XYPixmap ,
then 
the image contains only the bit planes specified in
.A plane_mask .
>>STRATEGY
Create drawable.
Set only bits in drawable corresponding to planes specified by planemask.
Call XGetImage with XYPixmap format.
Verify XGetImage return value is not null.
Verify depth, width, height, and format values in gotten image structure.
Verify gotten image for known pattern.
Repeat with only bits set not in planes specified by planemask.
Destroy images using XDestroyImage.
Repeat for each planemask.
Repeat for each visual.
>>CODE
XVisualInfo *vp;
Window w;
XImage *im1;
XImage *im2;
long mask;
long *pp;
struct area *ap;
unsigned int numbits;
unsigned long pix;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
/* Create drawable. */
		w = makewin(display, vp);
		trace("depth of drawable: %d", vp->depth);
		for (ap = arealist, pp = planelist; pp < &planelist[NELEM(planelist)]; pp++) {
			im1 = im2 = (XImage *) 0;
			if (++ap >= (&arealist[NELEM(arealist)]))
				ap = arealist;
			mask = *pp & DEPTHMASK(vp->depth);
			numbits = bitcount(mask);
			trace("plane-mask 0x%x, numbits: %d, depth: %d",
				*pp, numbits, vp->depth);
			pre_xcall(w, ap, *pp, XYPixmap);

/* Set only bits in drawable corresponding to planes specified by planemask. */
			dset(display, w, mask);

/* Call XGetImage with XYPixmap format. */
			im1 = XCALL;

/* Verify XGetImage return value is not null. */
			if (im1 == (XImage *) 0 && numbits) {
				report("Null image returned.");
				FAIL;
				return;
			} else
				CHECK;

/* Verify depth, width, height, and format values in gotten image structure. */
			if (im1 && checkimgstruct(im1, numbits, ap->width, ap->height, XYPixmap) == False)
				FAIL;
			else
				CHECK;

/* Verify gotten image for known pattern. */
			pix = DEPTHMASK(numbits);
			if (im1 && checkimg(im1, (struct area *) 0, pix, pix, 0) == False) {
				report("Image was not properly gotten.");
				FAIL;
			}
			else
				CHECK;

/* Repeat with only bits set not in planes specified by planemask. */
			trace("Repeat with complement.");
			dset(display, w, ~mask);

			im2 = XCALL;

			if (im2 == (XImage *) 0 && numbits) {
				report("Null image returned.");
				FAIL;
				return;
			}
			else
				CHECK;

			if (im2 && checkimgstruct(im2, numbits, ap->width, ap->height, XYPixmap) == False)
				FAIL;
			else
				CHECK;

			if (im2 && checkimg(im2, (struct area *) 0, 0, 0, 0) == False) {
				report("Image was not properly gotten.");
				FAIL;
			}
			else
				CHECK;
/* Destroy images using XDestroyImage. */
			if (im1 != (XImage *) 0)
				(void) XDestroyImage(im1);
			if (im2 != (XImage *) 0)
				(void) XDestroyImage(im2);

/* Repeat for each planemask. */
		}


/* Repeat for each visual. */
	}

	CHECKPASS(nvinf() * 3 * 2 * NELEM(planelist));
>># NOTE peterc	This condition is already checked for in tests which request
>># NOTE peterc	a subset of planes of the display.
>># NOTE peterc	Therefore, this test is classified as "def".
>>ASSERTION def
When the
.A format
is 
.S XYPixmap
and the
.A plane_mask
only requests a subset of the planes of the
display, then the
.M depth
of the returned image will be the number of planes
requested.
>>ASSERTION Good A
When the
.A format
is 
.S ZPixmap , 
then a call to xname
returns as zero the bits in all planes not 
specified in
.A plane_mask .
>>STRATEGY
Create drawable.
Set only bits in drawable corresponding to planes specified by planemask.
Call XGetImage with ZPixmap format.
Verify XGetImage return value is not null.
Verify depth, width, height, and format values in gotten image structure.
Verify gotten image for zero-bits in
all planes not specified in plane_mask.
Repeat with only bits set not in planes specified by planemask.
Destroy images using XDestroyImage.
Repeat for each planemask.
Repeat for each visual.
>>CODE
XVisualInfo *vp;
Window w;
XImage *im1;
XImage *im2;
long mask;
long *pp;
struct area *ap;
unsigned long pix;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		im1 = im2 = (XImage *) 0;

/* Create drawable. */
		w = makewin(display, vp);
		trace("depth of drawable: %d", vp->depth);
		for (ap = arealist, pp = planelist; pp < &planelist[NELEM(planelist)]; pp++) {
			/* wrap area pointer at end of list */
			if (++ap >= (&arealist[NELEM(arealist)]))
				ap = arealist;
			mask = *pp;
			mask &= DEPTHMASK(vp->depth);
			trace("plane-mask 0x%x", *pp);
			pre_xcall(w, ap, *pp, ZPixmap);

/* Set only bits in drawable corresponding to planes specified by planemask. */
			dset(display, w, mask);

/* Call XGetImage with ZPixmap format. */
			im1 = XCALL;

/* Verify XGetImage return value is not null. */
			if (im1 == (XImage *) 0) {
				report("Null image returned.");
				FAIL;
				return;
			}
			else
				CHECK;

/* Verify depth, width, height, and format values in gotten image structure. */
			if (checkimgstruct(im1, vp->depth, ap->width, ap->height, ZPixmap) == False)
				FAIL;
			else
				CHECK;

/* Verify gotten image for zero-bits in */
/* all planes not specified in plane_mask. */
			pix = mask;
			if (checkimg(im1, (struct area *) 0, pix, pix, 0) == False) {
				report("Image was not properly gotten.");
				FAIL;
			}
			else
				CHECK;

/* Repeat with only bits set not in planes specified by planemask. */
			trace("Repeat with complement.");
			dset(display, w, ~mask);

			im2 = XCALL;

			if (im2 == (XImage *) 0) {
				report("Null image returned.");
				FAIL;
				return;
			}
			else
				CHECK;

			if (checkimgstruct(im2, vp->depth, ap->width, ap->height, ZPixmap) == False)
				FAIL;
			else
				CHECK;

			if (checkimg(im2, (struct area *) 0, 0, 0, 0) == False) {
				report("Image was not properly gotten.");
				FAIL;
			}
			else
				CHECK;
/* Destroy images using XDestroyImage. */
			if (im1 != (XImage *) 0)
				(void) XDestroyImage(im1);
			if (im2 != (XImage *) 0)
				(void) XDestroyImage(im2);

/* Repeat for each planemask. */
		}


/* Repeat for each visual. */
	}

	CHECKPASS(nvinf() * 3 * 2 * NELEM(planelist));
>># NOTE peterc	The returned depth is verified during all calls
>># NOTE peterc	to checkimgstruct().  checkimgstruct() is called
>># NOTE peterc	each time XGetIMage() is called to verify such
>># NOTE peterc	things as the image depth.
>># Therefore, next test is classified as "def".
>>ASSERTION def
When the
.A format
is 
.S ZPixmap , 
then the
.M depth
of the returned image
is as specified on
.A drawable
creation.
>># NOTE peterc	Truncation occurs in all tests using planelist.
>># NOTE peterc	No need to run them again here.
>># NOTE peterc	Therefore, next test is classified as "def".
>>ASSERTION def
The value for
.M plane_mask
is truncated to the 
.M depth 
of the drawable.
>># ADDED peterc	As per external review comments.
>>ASSERTION Good A
When the specified rectangle includes the window border,
then the contents of the window border are obtained in the
.S XImage
structure returned by a call to xname.
>>STRATEGY
Create drawable.
Set window border to W_FG.
Call XGetImage with ZPixmap format to get image of border pixel.
Verify XGetImage return value is not null.
Verify depth, width, height, and format values in gotten image structure.
Verify gotten image for known pattern.
Repeat with window border set to W_BG.
Destroy images using XDestroyImage.
Repeat for each type-window visual.
>>CODE
XVisualInfo *vp;
Window w;
XImage *im1, *im2;
static struct area area1 =
	{ -1, -1, 1, 1 };
static struct area area2 =
	{ W_STDWIDTH, W_STDHEIGHT, 1, 1 };

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {
		im1 = im2 = (XImage *) 0;

/* Create drawable. */
		/* do ZPixmap testing */
		trace("ZPixmap");
		w = makewin(display, vp);

/* Set window border to W_FG. */
		XSetWindowBorder(display, w, W_FG);

/* Call XGetImage with ZPixmap format to get image of border pixel. */
		pre_xcall(w, &area1, AllPlanes, ZPixmap);
		im1 = XCALL;

/* Verify XGetImage return value is not null. */
		if (im1 == (XImage *) 0) {
			report("Null image returned.");
			FAIL;
			return;
		}
		else
			CHECK;

/* Verify depth, width, height, and format values in gotten image structure. */
		if (checkimgstruct(im1, vp->depth, area1.width, area1.height, ZPixmap) == False)
			FAIL;
		else
			CHECK;

/* Verify gotten image for known pattern. */
		if (checkimg(im1, (struct area *) 0, W_FG, W_FG, 0) == False) {
			report("Image was not properly gotten.");
			FAIL;
		}
		else
			CHECK;

/* Repeat with window border set to W_BG. */
		trace("Repeat with window border to W_BG.");
		XSetWindowBorder(display, w, W_BG);

		pre_xcall(w, &area1, AllPlanes, ZPixmap);
		im2 = XCALL;

		if (im2 == (XImage *) 0) {
			report("Null image returned.");
			FAIL;
			return;
		}
		else
			CHECK;

		if (checkimgstruct(im2, vp->depth, area1.width, area1.height, ZPixmap) == False)
			FAIL;
		else
			CHECK;

		if (checkimg(im2, (struct area *) 0, W_BG, W_BG, 0) == False) {
			report("Image was not properly gotten.");
			FAIL;
		}
		else
			CHECK;

/* Destroy images using XDestroyImage. */
		if (im1 != (XImage *) 0)
			(void) XDestroyImage(im1);
		if (im2 != (XImage *) 0)
			(void) XDestroyImage(im2);

/* Repeat for each type-window visual. */
	}

	CHECKPASS(6*nvinf());
>># ADDED	peterc	Added as per external review comments.
>>ASSERTION Good D 1
When the drawable is a window and the window has backing-store and
has regions obscured by noninferior windows,
then backing store contents are returned for those regions in the
.S XImage
structure returned by a call to xname.
>>ASSERTION Good D 1
When the
.A drawable
is a window and
the window does not have backing-store
and regions of the window are obscured by noninferior windows,
then undefined values are returned for those regions.
>>ASSERTION Good B 1
When the
.A drawable
is a window and
visible regions of the window are obscured by inferior windows
of different depth to the specified window,
then undefined values are returned for those regions.
>>ASSERTION Good A
The pointer cursor image is not included in the returned contents.
>>STRATEGY
Create drawable.
Grab server.
Enable synchronization.
Save initial pointer location.
Warp pointer to drawable.
Get new pointer location.
Call XGetImage with pointer inside drawable.
Verify XGetImage returned success.
Verify depth, width, height, and format values in gotten image structure.
Get current pointer location.
Check to see if pointer moved.
Warp pointer outside of drawable.
Get new pointer location.
Call XGetImage with pointer outside drawable.
Verify XGetImage returned success.
Verify depth, width, height, and format values in gotten image structure.
Get current pointer location.
Check to see if pointer moved.
Warp pointer back to where it started.
Disable synchronization.
Ungrab server.
Verify that two images are identical.
Destroy gotten images.
>>CODE
XVisualInfo *vp;
Window w;
XImage *im1, *im2;	/* two images */
int root_x, root_y;	/* pointer location after XGetImage */
int oroot_x, oroot_y;	/* pointer location before XGetImage */
int sroot_x, sroot_y;	/* initial pointer location */
Window oldroot;		/* initial root window */
int itmp;		/* useless XQueryPointer return values */
unsigned int uitmp;	/* useless XQueryPointer return values */
Window wtmp;		/* useless XQueryPointer return values */
static struct area area =
	{ 0, 0, W_STDWIDTH, W_STDHEIGHT };

	im1 = im2 = (XImage *) 0;
	resetvinf(VI_WIN);
	nextvinf(&vp);	/* use first one we come to */

/* Create drawable. */
	w = makewin(display, vp);

/* Grab server. */
	XGrabServer(display);

/* Enable synchronization. */
	(void) XSynchronize(display, True);

/* Save initial pointer location. */
	(void) XQueryPointer(display, w, &oldroot, &wtmp, &sroot_x, &sroot_y, &itmp, &itmp, &uitmp);

/* Warp pointer to drawable. */
	XWarpPointer(display, None, w, 0, 0, 0, 0, 0, 0);

/* Get new pointer location. */
	if (XQueryPointer(display, w, &wtmp, &wtmp, &oroot_x, &oroot_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer on wrong root.");
		return;
	} else
		CHECK;

/* Call XGetImage with pointer inside drawable. */
	pre_xcall(w, &area, AllPlanes, ZPixmap);
	im1 = XCALL;

/* Verify XGetImage returned success. */
	if (geterr() != Success)
		FAIL;
	else
		CHECK;

/* Verify depth, width, height, and format values in gotten image structure. */
	if (checkimgstruct(im1, vp->depth, area.width, area.height, ZPixmap) == False)
		FAIL;
	else
		CHECK;

/* Get current pointer location. */
	if (XQueryPointer(display, w, &wtmp, &wtmp, &root_x, &root_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer moved.");
		return;
	}
	else
		CHECK;

/* Check to see if pointer moved. */
	if (oroot_x != root_x || oroot_y != root_y) {
		delete("Pointer moved.");
		return;
	}
	else
		CHECK;

/* Warp pointer outside of drawable. */
	XWarpPointer(display, None, w, W_STDWIDTH*2, W_STDHEIGHT*2, 0, 0, 0, 0);

/* Get new pointer location. */
	(void) XQueryPointer(display, w, &wtmp, &wtmp, &oroot_x, &oroot_y, &itmp, &itmp, &uitmp);

/* Call XGetImage with pointer outside drawable. */
	pre_xcall(w, &area, AllPlanes, ZPixmap);
	im2 = XCALL;

/* Verify XGetImage returned success. */
	if (geterr() != Success)
		FAIL;
	else
		CHECK;

/* Verify depth, width, height, and format values in gotten image structure. */
	if (checkimgstruct(im2, vp->depth, area.width, area.height, ZPixmap) == False)
		FAIL;
	else
		CHECK;

/* Get current pointer location. */
	if (XQueryPointer(display, w, &wtmp, &wtmp, &root_x, &root_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer moved.");
		return;
	}
	else
		CHECK;

/* Check to see if pointer moved. */
	if (oroot_x != root_x || oroot_y != root_y) {
		delete("Pointer moved.");
		return;
	}
	else
		CHECK;

/* Warp pointer back to where it started. */
	XWarpPointer(display, None, oldroot, 0, 0, 0, 0, sroot_x, sroot_y);

/* Disable synchronization. */
	(void) XSynchronize(display, False);

/* Ungrab server. */
	XUngrabServer(display);

/* Verify that two images are identical. */
	if (im1 == (XImage *) 0 || im2 == (XImage *) 0) {
		report("Null image returned.");
		FAIL;
	}
	else {
		int stop = 0;

		CHECK;
		for (root_x = 0; !stop && root_x < im1->width; root_x++)
			for (root_y = 0; !stop && root_y < im1->height; root_y++)
				if (XGetPixel(im1, root_x, root_y) !=
				    XGetPixel(im2, root_x, root_y))
					stop = 1;
		if (stop) {
			report("Images differ at (%d,%d)", root_x, root_y);
			FAIL;
		}
		else
			CHECK;
	}

/* Destroy gotten images. */
	if (im1 != (XImage *) 0)
		XDestroyImage(im1);
	if (im2 != (XImage *) 0)
		XDestroyImage(im2);
	CHECKPASS(11);
>>ASSERTION Bad A
When xname fails, then it returns
.S NULL .
>>STRATEGY
Create bad drawable.
Call XGetImage with bad drawable.
Verify XGetImage return value is null.
>>CODE BadDrawable
Window w;
XImage *image;
static struct area area =
	{ 0, 0, W_STDWIDTH, W_STDHEIGHT };

/* Create bad drawable. */
	w = badwin(display);

/* Call XGetImage with bad drawable. */
	pre_xcall(w, &area, AllPlanes, ZPixmap);
	image = XCALL;

/* Verify XGetImage return value is null. */
	if (image == (XImage *) 0) {
		CHECK;
		if (geterr() != BadDrawable)
			trace("Returned null, but returned incorrect error");
	}
	else {
		FAIL;
		/* Destroy gotten images. */
		XDestroyImage(image);
	}
	CHECKPASS(1);
>>ASSERTION Bad A
When the
.A drawable
is a pixmap and
the given rectangle is not wholly contained within the pixmap, 
then a
.S BadMatch 
error occurs.
>>STRATEGY
Create pixmap.
Call XGetImage with rectangle not wholly contained within the pixmap.
Verify XGetImage return value is null.
Verify that BadMatch error occurred.
>>CODE BadMatch
XVisualInfo *vp;
Pixmap p;
XImage *image;
static struct area area1 =
	{ -W_STDWIDTH, -W_STDHEIGHT, W_STDWIDTH*2, W_STDHEIGHT*2 };

	resetvinf(VI_PIX);
	nextvinf(&vp);	/* use first one we come to */

/* Create pixmap. */
	p = makepixm(display, vp);

/* Call XGetImage with rectangle not wholly contained within the pixmap. */
	pre_xcall(p, &area1, AllPlanes, ZPixmap);
	image = XCALL;

/* Verify XGetImage return value is null. */
	if (image != (XImage *) 0) {
		report("Null image not returned.");
		FAIL;
		/* Destroy gotten images. */
		XDestroyImage(image);
	}
	else
		CHECK;
/* Verify that BadMatch error occurred. */
	if (geterr() != BadMatch)
		FAIL;
	else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Bad A
When the
.A drawable
is a window and
the window is not viewable,
then a
.S BadMatch 
error occurs.
>>STRATEGY
Create window.
Call XUnmapWindow to make the window non-viewable.
Call XGetImage on window which is not viewable.
Verify XGetImage return value is null.
Verify that BadMatch error occurred.
>>CODE BadMatch
XVisualInfo *vp;
Window w;
XImage *image;
static struct area area1 =
	{ 0, 0, W_STDWIDTH, W_STDHEIGHT };

	resetvinf(VI_WIN);
	nextvinf(&vp);	/* use first one we come to */

/* Create window. */
	w = makewin(display, vp);

/* Call XUnmapWindow to make the window non-viewable. */
	XUnmapWindow(display, w);

/* Call XGetImage on window which is not viewable. */
	pre_xcall(w, &area1, AllPlanes, ZPixmap);
	image = XCALL;

/* Verify XGetImage return value is null. */
	if (image != (XImage *) 0) {
		report("Null image not returned.");
		FAIL;
		/* Destroy gotten images. */
		XDestroyImage(image);
	}
	else
		CHECK;

/* Verify that BadMatch error occurred. */
	if (geterr() != BadMatch)
		FAIL;
	else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Bad A
When the
.A drawable
is a window and
the window is viewable
and it is not the case that given there were no inferiors or overlapping windows
the specified rectangle of the window would be fully visible on the screen
and wholly contained within the outside edges of the window,
then a
.S BadMatch 
error occurs.
>>STRATEGY
Create window which is not fully visible on the screen.
Call XMapWindow to make sure the window is viewable.
Call XGetImage with rectangle extending beyond edge of screen.
Verify XGetImage return value is null.
Verify that BadMatch error occurred.
Create window which is fully visible on the screen.
Call XMapWindow to make sure the window is viewable.
Call XGetImage with rectangle extending beyond edge of window.
Verify XGetImage return value is null.
Verify that BadMatch error occurred.
>>CODE BadMatch
XVisualInfo *vp;
Window w;
Window w2;
int wx, wy;		/* coordinates of window */
static struct area area1 =
	{ 0, 0, W_STDWIDTH, W_STDHEIGHT };
static struct area area2 =
	{ 0, 0, W_STDWIDTH, W_STDHEIGHT };
XImage *image;
unsigned int tmpui;	/* uninteresting XGetGeometry return values */
Window	tmpw;		/* uninteresting XGetGeometry return values */

	resetvinf(VI_WIN);
	nextvinf(&vp);	/* use first one we come to */

/* Create window which is not fully visible on the screen. */
	w = makewinpos(display, vp, -W_STDWIDTH/2, -W_STDHEIGHT/2);

/* Call XMapWindow to make sure the window is viewable. */
	XMapWindow(display, w);

/* Call XGetImage with rectangle extending beyond edge of screen. */
	pre_xcall(w, &area1, AllPlanes, ZPixmap);
	image = XCALL;

/* Verify XGetImage return value is null. */
	if (image != (XImage *) 0) {
		report("Null image not returned.");
		FAIL;
		/* Destroy gotten images. */
		XDestroyImage(image);
	}
	else
		CHECK;

/* Verify that BadMatch error occurred. */
	if (geterr() != BadMatch)
		FAIL;
	else
		CHECK;

/* Create window which is fully visible on the screen. */
	w = makewinpos(display, vp, 0, 0);

/* Call XMapWindow to make sure the window is viewable. */
	XMapWindow(display, w);

/* Call XGetImage with rectangle extending beyond edge of window. */
	area2.width *= 2;
	area2.height *= 2;
	pre_xcall(w, &area2, AllPlanes, ZPixmap);
	image = XCALL;

/* Verify XGetImage return value is null. */
	if (image != (XImage *) 0) {
		report("Null image not returned.");
		FAIL;
		/* Destroy gotten images. */
		XDestroyImage(image);
	}
	else
		CHECK;

/* Verify that BadMatch error occurred. */
	if (geterr() != BadMatch)
		FAIL;
	else
		CHECK;

	CHECKPASS(4);
>>ASSERTION Bad A
.ER BadDrawable
>># MODIFIED	peterc	As per external review comments.
>>ASSERTION Bad A
.ER BadValue format XYPixmap ZPixmap
>>#.ER BadValue format XYBitmap XYPixmap ZPixmap
>># HISTORY kieron Completed    Check format and pass ac
>># HISTORY peterc Completed	Incorporated RTCB3.
>># HISTORY peterc Completed	Engineering.
