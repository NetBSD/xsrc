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
 * $XConsortium: gtsbimg.m,v 1.17 94/04/17 21:05:39 rws Exp $
 */
>>TITLE XGetSubImage CH06
XImage *
XGetSubImage(display, d, x, y, width, height, plane_mask, format, dest_image, dest_x, dest_y)
Display *display = Dsp;
Drawable d;
int x = 0;
int y = 0;
unsigned int width = 1;
unsigned int height = 1;
long plane_mask = AllPlanes;
int format = ZPixmap;
>># The following was being initialised incorrectly. Removed for present ..sr
XImage *dest_image;
int dest_x = 0;
int dest_y = 0;
>>EXTERN

#ifndef A_IMAGE /* Idealy would be automatic */
#define A_IMAGE dest_image
#endif

static struct area darea;	/* the area written to in dest_image */

static XVisualInfo	*mivp;

/*
 * pre_xcall() - set globals
 */
static void
pre_xcall(win, ap, p, f, dp)
Drawable win;
struct area *ap;
long p;
int f;
struct area *dp;
{
	/* set function call arguments */
	d = win;
	x = ap->x;
	y = ap->y;
	width = ap->width;
	height = ap->height;
	plane_mask = p;
	format = f;
	dest_x = dp->x;
	dest_y = dp->y;
	/* remember area written to */
	darea.x = dest_x;
	darea.y = dest_y;
	darea.width = width;
	darea.height = height;
}

/*
 * plane masks for plane mask-related tests.  Exercises various
 * combinations of planes.
 */
static long planelist[] = {
	0,
	0x23,
	0x129078,
	0x23567193
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
static struct area destlist[] = {
	{0, 0, 0, 0},
	{W_STDWIDTH/2, W_STDHEIGHT/2, 0, 0},
	{W_STDWIDTH/2, W_STDHEIGHT/2, 0, 0},
	{0, 0, 0, 0},
	{1, 1, 0, 0},
	{1, 1, 0, 0},
	{W_STDWIDTH/2, W_STDHEIGHT/2, 0, 0},
	{W_STDWIDTH-1, W_STDHEIGHT-1, 0, 0},
	{W_STDWIDTH-5, W_STDHEIGHT-5, 0, 0}
};
>>#MODIFIED	peterc	As per external review comments.
>>ASSERTION Good A
A call to xname returns a pointer to an
.S XImage
structure containing
the contents of the rectangle
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
and in the format specified by the
.A format
argument.
>>STRATEGY
Create image in XYPixmap format with all pixels set to W_BG.
Create drawable with all pixels set to W_FG.
Call XGetSubImage with XYPixmap format and
width and height equal to size of image.
Verify XGetSubImage return value is not null.
Verify depth, width, height, and format
values in gotten image structure.
Verify gotten image for known pattern.
Repeat call to XGetSubImage getting a subset of the entire drawable.
Repeat for ZPixmap format.
Repeat for each visual.
>>CODE
XImage *image;
XVisualInfo *vp;
Window w;
static struct area area1 =
	{ 0, 0, W_STDWIDTH, W_STDHEIGHT };
static struct area area2 =
	{ W_STDWIDTH/2, W_STDHEIGHT/2, W_STDWIDTH/4, W_STDHEIGHT/4 };
static struct area area3 =
	{ 0, 0, W_STDWIDTH/4, W_STDHEIGHT/4 };

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {

/* Create image in XYPixmap format with all pixels set to W_BG. */
		trace("XYPixmap");
		dest_image = makeimg(display, vp, XYPixmap);

/* Create drawable with all pixels set to W_FG. */
		w = makewin(display, vp);
		dset(display, w, W_FG);

/* Call XGetSubImage with XYPixmap format and */
/* width and height equal to size of image. */
		pre_xcall(w, &area1, AllPlanes, XYPixmap, &area3);
		image = XCALL;

/* Verify XGetSubImage return value is not null. */
		if (image == (XImage *) 0) {
			report("Null image returned.");
			FAIL;
			return;
		}
		else
			CHECK;

/* Verify depth, width, height, and format */
/* values in gotten image structure. */
		if (checkimgstruct(image, vp->depth, W_STDWIDTH, W_STDHEIGHT, XYPixmap) == False)
			FAIL;
		else
			CHECK;

/* Verify gotten image for known pattern. */
		if (checkimg(image, &darea, W_FG, W_BG, 0) == False) {
			report("Returned image did not match expected image pattern");
			FAIL;
		}
		else
			CHECK;

/* Repeat call to XGetSubImage getting a subset of the entire drawable. */
		dest_image = makeimg(display, vp, XYPixmap);

		/* be paranoid and set drawable again */
		dset(display, w, W_FG);

		/* Call XGetSubImage with XYPixmap format. */
		pre_xcall(w, &area2, AllPlanes, XYPixmap, &area3);
		image = XCALL;

		/* Verify XGetSubImage return value is not null. */
		if (image == (XImage *) 0) {
			report("Null image returned.");
			FAIL;
			return;
		}
		else
			CHECK;

		/* Verify depth, width, height, and format */
		/* values in gotten image structure. */
		if (checkimgstruct(image, vp->depth, W_STDWIDTH, W_STDHEIGHT, XYPixmap) == False)
			FAIL;
		else
			CHECK;

		/* Verify gotten image for known pattern. */
		if (checkimg(image, &darea, W_FG, W_BG, 0) == False) {
			report("Returned image did not match expected image pattern");
			FAIL;
		}
		else
			CHECK;

/* Repeat for ZPixmap format. */
		trace("ZPixmap");
		dest_image = makeimg(display, vp, ZPixmap);

		w = makewin(display, vp);
		dset(display, w, W_FG);

		pre_xcall(w, &area1, AllPlanes, ZPixmap, &area3);
		image = XCALL;

		if (image == (XImage *) 0) {
			report("Null image returned.");
			FAIL;
			return;
		}
		else
			CHECK;
		if (checkimgstruct(image, vp->depth, W_STDWIDTH, W_STDHEIGHT, ZPixmap) == False)
			FAIL;
		else
			CHECK;
		if (checkimg(image, &darea, W_FG, W_BG, 0) == False) {
			report("Returned image did not match expected image pattern");
			FAIL;
		}
		else
			CHECK;

		dest_image = makeimg(display, vp, ZPixmap);
		dset(display, w, W_FG);

		pre_xcall(w, &area2, AllPlanes, ZPixmap, &area3);
		image = XCALL;

		if (image == (XImage *) 0) {
			report("Null image returned.");
			FAIL;
			return;
		}
		else
			CHECK;
		if (checkimgstruct(image, vp->depth, W_STDWIDTH, W_STDHEIGHT, ZPixmap) == False)
			FAIL;
		else
			CHECK;
		if (checkimg(image, &darea, W_FG, W_BG, 0) == False) {
			report("Returned image did not match expected image pattern");
			FAIL;
		}
		else
			CHECK;

/* Repeat for each visual. */
	}

	CHECKPASS(3*2*2*nvinf());
>>ASSERTION Good A
A call to xname returns a pointer to the same
.S XImage 
structure specified by
.A dest_image .
>>STRATEGY
Create image.
Create drawable.
Call XGetSubImage to get subimage from drawable.
Verify XGetSubImage returned pointer to same XImage structure.
>>CODE
XImage *image;
Window w;
XVisualInfo *vp;
static struct area area =
	{ 0, 0, W_STDWIDTH, W_STDHEIGHT };

	resetvinf(VI_WIN);
	if (nvinf() == 0) {
		unsupported("At least one window required, is XT_DEBUG_PIXMAP_ONLY == Yes?");
		return;
	}
	nextvinf(&vp);	/* use first visual */
	/* this assumes that there is at least one visual supported...*/
/* Create image. */
	trace("ZPixmap");
	dest_image = makeimg(display, vp, ZPixmap);

/* Create drawable. */
	w = makewin(display, vp);
	dset(display, w, W_FG);

/* Call XGetSubImage to get subimage from drawable. */
	pre_xcall(w, &area, AllPlanes, ZPixmap, &area);
	image = XCALL;

/* Verify XGetSubImage returned pointer to same XImage structure. */
	if (image != dest_image) {
		report("Returned pointer not the same.");
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(1);
>>ASSERTION Good A
When the
.A format
is 
.S XYPixmap ,
then 
the image contains only the bit planes specified in
.A plane_mask .
>>STRATEGY
Create image in XYPixmap format.
Create drawable.
Clear all pixels in image.
Set only bits in drawable corresponding to planes specified by planemask.
Call XGetSubImage with XYPixmap format.
Verify XGetSubImage return value is not null.
Verify depth, width, height, and format
values in gotten image structure.
Verify gotten image for known pattern.
Repeat with only bits set not in planes specified by planemask.
Repeat for each planemask.
Repeat for each visual.
>>CODE
XVisualInfo *vp;
Window w;
XImage *image;
long mask;
long *pp;
struct area *ap;
struct area *dp;
unsigned int  numbits;
unsigned long pix;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {

/* Create image in XYPixmap format. */
		trace("XYPixmap");
		dest_image = makeimg(display, vp, XYPixmap);

/* Create drawable. */
		w = makewin(display, vp);
		trace("depth of drawable: %d", vp->depth);
		ap = arealist;
		dp = destlist;
		pp = planelist;
		for (; pp < &planelist[NELEM(planelist)]; pp++) {
			/* wrap area pointer at end of list */
			if (++ap >= (&arealist[NELEM(arealist)]))
				ap = arealist;
			if (++dp >= (&destlist[NELEM(destlist)]))
				dp = destlist;
			mask = *pp & DEPTHMASK(vp->depth);
			numbits = bitcount(mask);
			trace("plane-mask 0x%x, depth: %d", *pp, vp->depth);
			pre_xcall(w, ap, *pp, XYPixmap, dp);

/* Clear all pixels in image. */
			dsetimg(dest_image, 0);

/* Set only bits in drawable corresponding to planes specified by planemask. */
			dset(display, w, mask);

/* Call XGetSubImage with XYPixmap format. */
			trace("numbits: %d, depth: %d",
				numbits, vp->depth);
			if (numbits != vp->depth)
				trace("Some Xlib implementations are known to exit during the next call to XGetSubImage.");
			image = XCALL;

/* Verify XGetSubImage return value is not null. */
			if (image == (XImage *) 0 && numbits) {
				report("Null image returned.");
				FAIL;
				return;
			}
			else
				CHECK;

/* Verify depth, width, height, and format */
/* values in gotten image structure. */
			if (image && checkimgstruct(image, vp->depth, W_STDWIDTH, W_STDHEIGHT, XYPixmap) == False)
				FAIL;
			else
				CHECK;

/* Verify gotten image for known pattern. */
			pix = getpix(mask, plane_mask);
			if (image && checkimg(image, &darea, pix, 0, 0) == False) {
				report("Returned image did not match expected image pattern");
				FAIL;
			}
			else
				CHECK;

/* Repeat with only bits set not in planes specified by planemask. */
			trace("Repeat with complement.");
			dsetimg(dest_image, ~mask);
			dset(display, w, ~mask);

			image = XCALL;

			if (image == (XImage *) 0 && numbits) {
				report("Null image returned.");
				FAIL;
				return;
			}
			else
				CHECK;

			if (image && checkimgstruct(image, vp->depth, W_STDWIDTH, W_STDHEIGHT, XYPixmap) == False)
				FAIL;
			else
				CHECK;

			if (image && checkimg(image, &darea, 0, ~mask & DEPTHMASK(vp->depth), 0) == False) {
				report("Returned image did not match expected image pattern");
				FAIL;
			}
			else
				CHECK;

/* Repeat for each planemask. */
		}


/* Repeat for each visual. */
	}

	CHECKPASS(nvinf() * 6 * NELEM(planelist));

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
Create image in ZPixmap format.
Create drawable.
Clear all pixels in image.
Set only bits in drawable corresponding to planes specified by planemask.
Call XGetSubImage with ZPixmap format.
Verify XGetSubImage return value is not null.
Verify depth, width, height, and format
values in gotten image structure.
Verify gotten image for zero-bits in
all planes not specified in plane_mask.
Repeat with only bits set not in planes specified by planemask.
Repeat for each planemask.
Repeat for each visual.
>>CODE
XVisualInfo *vp;
Window w;
XImage *image;
long mask;
long *pp;
struct area *ap;
struct area *dp;
unsigned long pix;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {

/* Create image in ZPixmap format. */
		trace("ZPixmap");
		dest_image = makeimg(display, vp, ZPixmap);

/* Create drawable. */
		w = makewin(display, vp);
		trace("depth of drawable: %d", vp->depth);
		ap = arealist;
		dp = destlist;
		pp = planelist;
		for (; pp < &planelist[NELEM(planelist)]; pp++) {
			/* wrap area pointer at end of list */
			if (++ap >= (&arealist[NELEM(arealist)]))
				ap = arealist;
			if (++dp >= (&destlist[NELEM(destlist)]))
				dp = destlist;
			mask = *pp & DEPTHMASK(vp->depth);
			trace("plane-mask 0x%x", *pp);
			pre_xcall(w, ap, *pp, ZPixmap, dp);

/* Clear all pixels in image. */
			dsetimg(dest_image, 0);

/* Set only bits in drawable corresponding to planes specified by planemask. */
			dset(display, w, mask);

/* Call XGetSubImage with ZPixmap format. */
			image = XCALL;

/* Verify XGetSubImage return value is not null. */
			if (image == (XImage *) 0) {
				report("Null image returned.");
				FAIL;
				return;
			}
			else
				CHECK;

/* Verify depth, width, height, and format */
/* values in gotten image structure. */
			if (checkimgstruct(image, vp->depth, W_STDWIDTH, W_STDHEIGHT, ZPixmap) == False)
				FAIL;
			else
				CHECK;

/* Verify gotten image for zero-bits in */
/* all planes not specified in plane_mask. */
			pix = mask;
			if (checkimg(image, &darea, pix, 0, 0) == False) {
				report("Returned image did not match expected image pattern");
				FAIL;
			}
			else
				CHECK;

/* Repeat with only bits set not in planes specified by planemask. */
			trace("Repeat with complement.");
			dsetimg(dest_image, ~mask);
			dset(display, w, ~mask);

			image = XCALL;

			if (image == (XImage *) 0) {
				report("Null image returned.");
				FAIL;
				return;
			}
			else
				CHECK;

			if (checkimgstruct(image, vp->depth, W_STDWIDTH, W_STDHEIGHT, ZPixmap) == False)
				FAIL;
			else
				CHECK;

			if (checkimg(image, &darea, 0, ~mask & DEPTHMASK(vp->depth), 0) == False) {
				report("Returned image did not match expected image pattern");
				FAIL;
			}
			else
				CHECK;

/* Repeat for each planemask. */
		}

/* Repeat for each visual. */
	}

	CHECKPASS(nvinf() * 6 * NELEM(planelist));

>># NOTE peterc	I believe this assertion to be a mistake.
>>#		This applies only to XGetImage().
>># RESOLVED dave 11/4/91 - Bob Scheifler has confirmed this.
>>#
>>#>>ASSERTION Good B 1
>>#When the
>>#.A format
>>#is 
>>#.S ZPixmap , 
>>#then the
>>#.M depth
>>#of the returned image
>>#is as specified on
>>#.A drawable
>>#creation.

>># NOTE peterc	Truncation occurs in all tests using planelist.
>># NOTE peterc	No need to run them again here.
>># NOTE peterc	Therefore, next test is classified as "def".
>>ASSERTION def
The value for
.M plane_mask
is truncated to the 
.M depth 
of the drawable.
>>ASSERTION Good A
When the specified subimage does not fit at the specified location
(
.A dest_x ,
.A dest_y
)
on the destination image, then the right and bottom edges are clipped.
>>STRATEGY
Create image.
Create drawable.
Call XGetSubImage to get subimage from drawable.
Verify XGetSubImage returned non-null.
Verify depth, width, height, and format
values in gotten image structure.
>>CODE
XImage *image;
Window w;
XVisualInfo *vp;
static struct area area1 =
	{ 0, 0, W_STDWIDTH, W_STDHEIGHT };
static struct area dest =
	{ W_STDWIDTH/2, W_STDHEIGHT/2, W_STDWIDTH/2, W_STDHEIGHT/2 };

	resetvinf(VI_WIN_PIX);
	if (nvinf() == 0) {
		unsupported("At least one drawable required, is XT_DEBUG_PIXMAP_ONLY == Yes && XT_DEBUG_WINDOW_ONLY == Yes?");
		return;
	}
	nextvinf(&vp);	/* use first visual */
	/* this assumes that there is at least one visual supported...*/
/* Create image. */
	dest_image = makeimg(display, vp, ZPixmap);

/* Create drawable. */
	w = makewin(display, vp);
	dset(display, w, W_FG);

/* Call XGetSubImage to get subimage from drawable. */
	pre_xcall(w, &area1, AllPlanes, ZPixmap, &dest);
	image = XCALL;

/* Verify XGetSubImage returned non-null. */
	if (image == (XImage *) 0) {
		report("Null image returned.");
		FAIL;
		return;
	}
	else
		CHECK;

/* Verify depth, width, height, and format */
/* values in gotten image structure. */
	if (checkimgstruct(image, vp->depth, W_STDWIDTH, W_STDHEIGHT, ZPixmap) == False)
		FAIL;
	else
		CHECK;

	if (checkimg(image, &dest, W_FG, W_BG, 0) == False) {
		report("Returned image did not match expected image pattern");
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(3);
>>#ADDED	peterc	As per external review comments.
>>ASSERTION Good A
When the specified rectangle includes the window border,
then the contents of the window border are obtained in the
.S XImage
structure.
>>STRATEGY
Create image in ZPixmap format.
Create drawable.
Set window border to W_FG.
Call XGetSubImage with ZPixmap format to get image of border pixel.
Verify XGetSubImage return value is not null.
Verify depth, width, height, and format
values in gotten image structure.
Verify gotten image for known pattern.
Repeat with window border set to W_BG.
Repeat for each type-window visual.
>>CODE
XVisualInfo *vp;
Window w;
XImage *image;
XImage *zimage;
static struct area area1 =
	{ -1, -1, 1, 1 };
static struct area area2 =
	{ W_STDWIDTH, W_STDHEIGHT, 1, 1 };
static struct area aread =
	{ 1, 1, 0, 0 };

	for (resetvinf(VI_WIN); nextvinf(&vp); ) {

/* Create image in ZPixmap format. */
		zimage = makeimg(display, vp, ZPixmap);

/* Create drawable. */
		/* do ZPixmap testing */
		trace("ZPixmap");
		dest_image = zimage;
		w = makewin(display, vp);

/* Set window border to W_FG. */
		XSetWindowBorder(display, w, W_FG);

/* Call XGetSubImage with ZPixmap format to get image of border pixel. */
		pre_xcall(w, &area1, AllPlanes, ZPixmap, &aread);
		image = XCALL;

/* Verify XGetSubImage return value is not null. */
		if (image == (XImage *) 0) {
			report("Null image returned.");
			FAIL;
			return;
		}
		else
			CHECK;

/* Verify depth, width, height, and format */
/* values in gotten image structure. */
		if (checkimgstruct(image, vp->depth, W_STDWIDTH, W_STDHEIGHT, ZPixmap) == False)
			FAIL;
		else
			CHECK;

/* Verify gotten image for known pattern. */
		if (checkimg(image, &darea, W_FG, W_BG, 0) == False) {
			report("Returned image did not match expected image pattern");
			FAIL;
		}
		else
			CHECK;

/* Repeat with window border set to W_BG. */
		trace("Repeat with window border set to W_BG.");
		XSetWindowBorder(display, w, W_BG);
		dsetimg(dest_image, W_FG);

		pre_xcall(w, &area1, AllPlanes, ZPixmap, &aread);
		image = XCALL;

		if (image == (XImage *) 0) {
			report("Null image returned.");
			FAIL;
			return;
		}
		else
			CHECK;

		if (checkimgstruct(image, vp->depth, W_STDWIDTH, W_STDHEIGHT, ZPixmap) == False)
			FAIL;
		else
			CHECK;

		if (checkimg(image, &darea, W_BG, W_FG, 0) == False) {
			report("Returned image did not match expected image pattern");
			FAIL;
		}
		else
			CHECK;
	}

	CHECKPASS(6*nvinf());
>>#ADDED	peterc	As per external review comments.
>>ASSERTION Good D 1
When the drawable is a window and the window has backing-store
and has regions obscured by noninferior windows,
then backing-store contents are returned for those regions in the
.S XImage
structure returned by a call to xname.
>>ASSERTION Good D 1
When the
.A drawable
is a window and
the window does not have backing-store
and regions of the window are obscured by noninferior windows,
then undefined values are returned for those regions.
>>ASSERTION Good D 1
When the
.A drawable
is a window and
visible regions of the window are obscured by inferior windows
of different depth to the specified window,
then undefined values are returned for those regions.
>>ASSERTION Good A
The pointer cursor image is not included in the returned contents.
>>STRATEGY
Create 2 images.
Create drawable.
Grab server.
Enable synchronization.
Save initial pointer location.
Warp pointer to drawable.
Get new pointer location.
Call XGetSubImage with pointer inside drawable.
Verify depth, width, height, and format
values in gotten image structure.
Get current pointer location.
Check to see if pointer moved.
Warp pointer outside of drawable.
Get new pointer location.
Call XGetSubImage with pointer outside drawable.
Verify depth, width, height, and format
values in gotten image structure.
Get current pointer location.
Check to see if pointer moved.
Warp pointer back to where it started.
Disable synchronization.
Ungrab server.
Verify that two images are identical.
>>CODE
XVisualInfo *vp;
Window w;
XImage *im1, *im2;	/* two images */
int root_x, root_y;	/* pointer location after XGetSubImage */
int oroot_x, oroot_y;	/* pointer location before XGetSubImage */
int sroot_x, sroot_y;	/* initial pointer location */
Window oldroot;		/* initial root window */
int itmp;		/* useless XQueryPointer return values */
unsigned int uitmp;	/* useless XQueryPointer return values */
Window wtmp;		/* useless XQueryPointer return values */
static struct area area =
	{ 0, 0, W_STDWIDTH, W_STDHEIGHT };

	im1 = im2 = (XImage *) 0;
	resetvinf(VI_WIN);
	nextvinf(&vp);	/* use first visual */

/* Create 2 images. */
	im1 = makeimg(display, vp, ZPixmap);
	if (isdeleted())
		return;
	else
		CHECK;
	im2 = makeimg(display, vp, ZPixmap);
	if (isdeleted())
		return;
	else
		CHECK;
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

/* Call XGetSubImage with pointer inside drawable. */
	pre_xcall(w, &area, AllPlanes, ZPixmap, &area);
	dest_image = im1;
	im1 = XCALL;

/* Verify depth, width, height, and format */
/* values in gotten image structure. */
	if (checkimgstruct(im1, vp->depth, area.width, area.height, ZPixmap) == False)
		FAIL;
	else
		CHECK;

/* Get current pointer location. */
	if (XQueryPointer(display, w, &wtmp, &wtmp, &root_x, &root_y, &itmp, &itmp, &uitmp) == False) {
		delete("Pointer on wrong root.");
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

/* Call XGetSubImage with pointer outside drawable. */
	pre_xcall(w, &area, AllPlanes, ZPixmap, &area);
	dest_image = im2;
	im2 = XCALL;

/* Verify depth, width, height, and format */
/* values in gotten image structure. */
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

	CHECKPASS(11);
>>ASSERTION Bad A
When xname fails, then it returns
.S NULL .
>>STRATEGY
Create image.
Create bad drawable.
Call XGetSubImage with bad drawable.
Verify XGetSubImage return value is null.
>>CODE BadDrawable
Window w;
XVisualInfo *vp;
XImage *im;
static struct area area =
	{ 0, 0, W_STDWIDTH, W_STDHEIGHT };

	resetvinf(VI_WIN_PIX);
	if (nvinf() == 0) {
		unsupported("At least one drawable required, is XT_DEBUG_PIXMAP_ONLY == Yes? && XT_DEBUG_WINDOW_ONLY == Yes");
		return;
	}
	nextvinf(&vp);	/* use first visual */
	/* this assumes that there is at least one visual supported...*/
/* Create image. */
	dest_image = makeimg(display, vp, ZPixmap);

/* Create bad drawable. */
	w = badwin(display);

/* Call XGetSubImage with bad drawable. */
	pre_xcall(w, &area, AllPlanes, ZPixmap, &area);
	im = XCALL;

/* Verify XGetSubImage return value is null. */
	if (im == (XImage *) 0)
		CHECK;
	else
		FAIL;

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
Create image.
Create pixmap.
Call XGetSubImage.
Verify XGetSubImage return value is null.
Verify that BadMatch error occurred.
>>CODE BadMatch
Pixmap p;
XVisualInfo *vp;
XImage *im;
static struct area area =
	{ W_STDWIDTH/2, W_STDHEIGHT/2, W_STDWIDTH, W_STDHEIGHT };

	resetvinf(VI_PIX);
	if (nvinf() == 0) {
		unsupported("At least one pixmap required, is XT_DEBUG_WINDOW_ONLY == Yes?");
		return;
	}
	nextvinf(&vp);	/* use first visual */
	/* this assumes that there is at least one visual supported...*/
/* Create image. */
	dest_image = makeimg(display, vp, ZPixmap);

/* Create pixmap. */
	p = makepixm(display, vp);

/* Call XGetSubImage. */
	pre_xcall(p, &area, AllPlanes, ZPixmap, &area);
	im = XCALL;

/* Verify XGetSubImage return value is null. */
	if (im == (XImage *) 0)
		CHECK;
	else
		FAIL;

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
Create image.
Call XUnmapWindow to make the window non-viewable.
Call XSubGetImage on window which is not viewable.
Verify XSubGetImage return value is null.
Verify that BadMatch error occurred.
>>CODE BadMatch
XVisualInfo *vp;
Window w;
XImage *image;
static struct area area =
	{ 0, 0, W_STDWIDTH, W_STDHEIGHT };

	resetvinf(VI_WIN);
	nextvinf(&vp);	/* use first visual */

/* Create window. */
	w = makewin(display, vp);

/* Create image. */
	dest_image = makeimg(display, vp, ZPixmap);

/* Call XUnmapWindow to make the window non-viewable. */
	XUnmapWindow(display, w);

/* Call XSubGetImage on window which is not viewable. */
	pre_xcall(w, &area, AllPlanes, ZPixmap, &area);
	image = XCALL;

/* Verify XSubGetImage return value is null. */
	if (image != (XImage *) 0) {
		report("Null image not returned.");
		FAIL;
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
Create image.
Create window which is not fully visible on the screen.
Call XMapWindow to make sure the window is viewable.
Call XGetSubImage with rectangle extending beyond edge of screen.
Verify XGetSubImage return value is null.
Verify that BadMatch error occurred.
Create window which is fully visible on the screen.
Call XMapWindow to make sure the window is viewable.
Call XGetSubImage with rectangle extending beyond edge of window.
Verify XGetSubImage return value is null.
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
Window	tmpw;

	resetvinf(VI_WIN);
	nextvinf(&vp);	/* use first visual */

/* Create image. */
	dest_image = makeimg(display, vp, ZPixmap);

/* Create window which is not fully visible on the screen. */
	w = makewinpos(display, vp, -W_STDWIDTH/2, -W_STDHEIGHT/2);

/* Call XMapWindow to make sure the window is viewable. */
	XMapWindow(display, w);

/* Call XGetSubImage with rectangle extending beyond edge of screen. */
	pre_xcall(w, &area1, AllPlanes, ZPixmap, &area1);
	image = XCALL;

/* Verify XGetSubImage return value is null. */
	if (image != (XImage *) 0) {
		report("Null image not returned.");
		FAIL;
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

/* Call XGetSubImage with rectangle extending beyond edge of window. */
	area2.width *= 2;
	area2.height *= 2;
	pre_xcall(w, &area2, AllPlanes, ZPixmap, &area2);
	image = XCALL;

/* Verify XGetSubImage return value is null. */
	if (image != (XImage *) 0) {
		report("Null image not returned.");
		FAIL;
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
>>ASSERTION Bad A
.ER BadValue format XYPixmap ZPixmap
>># HISTORY kieron Completed    Check format and pass ac
>># HISTORY peterc Completed    Incorporated RTCB3.
>># HISTORY peterc Completed    Engineering.
