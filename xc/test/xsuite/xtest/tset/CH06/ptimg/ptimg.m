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
 * $XConsortium: ptimg.m,v 1.22 94/04/17 21:05:44 rws Exp $
 */
>>TITLE XPutImage CH06
void
XPutImage(display, d, gc, image, src_x, src_y, dest_x, dest_y, width, height)
Display *display = Dsp;
Drawable d;
GC gc;
>># Following was initialised incorrectly. Have removed it ..sr
XImage *image;
int src_x = 0;
int src_y = 0;
int dest_x = 0;
int dest_y = 0;
unsigned int width = 1;
unsigned int height = 1;
>>EXTERN
static struct area area;

/*
 * doxcall() -	set globals, remember dest and size, call xname
 */
static void
doxcall(win, sx, sy, dx, dy, w, h)
Window win;
int sx, sy, dx, dy;
unsigned int w, h;
{
	d = win;
	src_x = sx;
	src_y = sy;
	dest_x = dx;
	dest_y = dy;
	width = w;
	height = h;
	/* remember area */
	area.x = dest_x;
	area.y = dest_y;
	area.width = width;
	area.height = height;
}

#define	A_IMAGE	image
>>ASSERTION Good A
On a call to xname the section of
.A image
defined by the
.A src_x ,
.A src_y ,
.A width
and
.A height 
is drawn on the specified part of the
.A drawable .
>>STRATEGY
Create image in XYPixmap format.
Create drawable.
Call XPutImage.
Verify results.
Repeat for XYBitmap and ZPixmap.
Repeat for each visual.
>>CODE
XVisualInfo	*vp;
Window	w;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
/* Create image in XYPixmap format. */
		/* do XYPixmap testing */
		trace("XYPixmap");
		image = makeimg(display, vp, XYPixmap);
		dsetimg(image, W_BG);
/* Create drawable. */
		w = makewin(display, vp);
		gc = makegc(display, w);
		dset(display, w, W_FG);

/* Call XPutImage. */
		doxcall(w, 0, 0, 0, 0, image->width, image->height);
		XCALL;

/* Verify results. */
		if (checkarea(display, w, &area, W_BG, W_FG, CHECK_ALL) == False) {
			report("Area was not properly put.");
			FAIL;
		}
		else
			CHECK;

/* Repeat for XYBitmap and ZPixmap. */
		/* do ZPixmap testing */
		trace("ZPixmap");
		image = makeimg(display, vp, ZPixmap);
		dsetimg(image, W_BG);
		w = makewin(display, vp);
		gc = makegc(display, w);
		dset(display, w, W_FG);

		doxcall(w, image->width/2, image->height/2, 0, 0, image->width/4, image->height/4);
		XCALL;

		if (checkarea(display, w, &area, W_BG, W_FG, CHECK_ALL) == False) {
			report("Area was not properly put.");
			FAIL;
		}
		else
			CHECK;

		/* do XYBitmap testing */
		trace("XYBitmap");
		image = makeimg(display, vp, XYBitmap);
		dsetimg(image, 0L); /* so we'll fill with bg */
		w = makewin(display, vp);
		gc = makegc(display, w);
		dset(display, w, W_FG); /* so we can tell the difference */

		doxcall(w, 0, 0, 1, 1, image->width/2, image->height);
		XCALL;

		if (checkarea(display, w, &area, W_BG, W_FG, CHECK_ALL) == False) {
			report("Area was not properly put.");
			FAIL;
		}
		else
			CHECK;
/* Repeat for each visual. */
	}

	CHECKPASS(3*nvinf());
>>ASSERTION Good C
When the 
.A image
.M format
is
.S XYBitmap ,
then the
.M foreground
pixel in
.A gc
defines the source for the one bits in the image, and the
.M background
pixel defines the source for the zero bits.
>>STRATEGY
Create depth 1 image in XYBitmap format.
Create drawable.
Set all bits in image to zero.
Set every pixel in drawable to W_FG.
Call XPutImage.
Verify results.
Set every pixel in drawable to W_BG.
Call XPutImage.
Verify results.
Set all bits in image to one.
Set every pixel in drawable to W_FG.
Call XPutImage.
Verify results.
Set every pixel in drawable to W_BG.
Call XPutImage.
Verify results.
Repeat for each visual.
>>CODE
XVisualInfo	*vp;
Window	w;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {

/* Create image in XYBitmap format. */
		image = makeimg(display, vp, XYBitmap);

/* Create drawable. */
		w = makewin(display, vp);
		gc = makegc(display, w);

/* Set all bits in image to zero. */
		trace("Bits 0, drawable W_FG");
		dsetimg(image, 0L);

/* Set every pixel in drawable to W_FG. */
		dset(display, w, W_FG);

/* Call XPutImage. */
		doxcall(w, 0, 0, 0, 0, image->width, image->height);
		XCALL;

/* Verify results. */
		if (checkarea(display, w, &area, W_BG, W_BG, CHECK_ALL) == False) {
			report("Area was not properly put.");
			FAIL;
		}
		else
			CHECK;

/* Set every pixel in drawable to W_BG. */
		trace("Bits 0, drawable W_BG");
		dset(display, w, W_BG);

/* Call XPutImage. */
		doxcall(w, 0, 0, 0, 0, image->width, image->height);
		XCALL;

/* Verify results. */
		if (checkarea(display, w, &area, W_BG, W_BG, CHECK_ALL) == False) {
			report("Area was not properly put.");
			FAIL;
		}
		else
			CHECK;

/* Set all bits in image to one. */
		trace("Bits 1, drawable W_FG");
		dsetimg(image, 1);

/* Set every pixel in drawable to W_FG. */
		dset(display, w, W_FG);

/* Call XPutImage. */
		doxcall(w, 0, 0, 0, 0, image->width, image->height);
		XCALL;

/* Verify results. */
		if (checkarea(display, w, &area, W_FG, W_FG, CHECK_ALL) == False) {
			report("Area was not properly put.");
			FAIL;
		}
		else
			CHECK;

/* Set every pixel in drawable to W_BG. */
		trace("Bits 1, drawable W_BG");
		dset(display, w, W_BG);

/* Call XPutImage. */
		doxcall(w, 0, 0, 0, 0, image->width, image->height);
		XCALL;

/* Verify results. */
		if (checkarea(display, w, &area, W_FG, W_FG, CHECK_ALL) == False) {
			report("Area was not properly put.");
			FAIL;
		}
		else
			CHECK;
/* Repeat for each visual. */
	}

	CHECKPASS(4*nvinf());
>>ASSERTION gc
On a call to xname the GC components
.M function ,
.M plane-mask ,
.M subwindow-mode ,
.M clip-x-origin ,
.M clip-y-origin
and
.M clip-mask
are used.
>>ASSERTION gc
On a call to xname the GC mode-dependent components
.M foreground
and
.M background
are used.
>>#ADDED peterc	As per external review comments.
>>#NOTE	peterc	I don't think the spec is clear enough for testing this...
>>#NOTE kieron	Clarified with rws, but status of b.p.p. uncertain.
>>#		Protocol has 1,4,8,16,24,32 as allowable values, but Xlib
>>#		seems to allow 4,8,12,16,20,24,28,32.... Is 1 treated just
>>#		like XYPixmap with depth 1??? What about the other values?
>>#		Protocol allows it to be > depth with extra bits ignored!
>>ASSERTION Good A
If drawables with depth < 32 are supported:
When the image differs from the X server's format in bits-per-pixel,
then the image will be converted to that format before drawing on
the specified drawable.
>>STRATEGY
Call makeimg to create image in ZPixmap format using
server's format in bits-per-pixel, scanline-pad, byte-order,
and bit-order.
Call makeimg to create another image in ZPixmap format.
Set bits-per-pixel to be different from the server's format, but <= depth.
Allocate memory for image data, discarding old data and clearing new.
Write known pattern to both images, using min(normal b.p.p, abnormal b.p.p) bits.
Create drawable.
Call XPutImage with server-normal image.
Compare server-normal image with drawable.
Clear drawable to W_FG.
Call XPutImage with non-server-normal image.
Compare same image with drawable.
Compare other image with drawable.
Repeat for each visual.
>>CODE
XImage *im1, *im2;
XVisualInfo *vp;
Window w;
char *data;
int supported = 0;
int nbits;
int   i;
static int legal_bpps[] = {1, 4, 8, 16, 24, 32, 0};
#define ROUNDUP(nbytes, pad) ((((nbytes) + ((pad)-1)) / (pad)) * ((pad)>>3))

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
		if (vp->depth == 32)
			continue;

		supported++;

/* Call makeimg to create image in ZPixmap format using */
/* server's format in bits-per-pixel, scanline-pad, byte-order, */
/* and bit-order. */
		trace("ZPixmap, standard");
		im1 = makeimg(display, vp, ZPixmap);

		for (i = 0; legal_bpps[i]; i++) {
			if (legal_bpps[i] >= im1->depth &&
			legal_bpps[i] != im1->bits_per_pixel)
				break;
		}
		if (!legal_bpps[i]) {
			report("Could not find differing bits_per_pixel");
			FAIL;
			continue;
		}

/* Call makeimg to create another image in ZPixmap format. */
		im2 = makeimg(display, vp, ZPixmap);

/* Set bits-per-pixel to be different from the server's format, but >= depth. */
		im2->bits_per_pixel = legal_bpps[i];
		trace("ZPixmap. Non-server normal: depth=%d, bits_per_pixel=%d",im2->depth, im2->bits_per_pixel);

/* Allocate memory for image data, discarding old data and clearing new. */
		/* discard old data as provided by makeimg() */
		if (im2->data != (char *) 0)
			XFree(im2->data);
		/* allocate a little extra just to be safe */
		im2->bytes_per_line = ROUNDUP((im2->bits_per_pixel * im2->width), im2->bitmap_pad);
		data = (char *) malloc(im2->bytes_per_line * im2->height);
		if (data == (char *) 0) {
			delete("malloc() error");
			return;
		} else
			CHECK;
		im2->data = data;
		/* clear new data to 0L */
		dsetimg(im2, 0L);
/* Write known pattern to both images, using min(normal b.p.p, abnormal b.p.p) bits. */
		nbits = (im1->bits_per_pixel < im2->bits_per_pixel) ?
				im1->bits_per_pixel : im2->bits_per_pixel;
		trace("patterning im1 with %x", DEPTHMASK(nbits));
		patternimg(im1, DEPTHMASK(nbits));
		trace("patterning im2 with %x", DEPTHMASK(nbits));
		patternimg(im2, DEPTHMASK(nbits));
/* Create drawable. */
		w = makewin(display, vp);
		gc = makegc(display, w);
		dset(display, w, W_FG);
/* Call XPutImage with server-normal image. */
		image = im1;
		doxcall(w, 0, 0, 0, 0, image->width, image->height);
		XCALL;
/* Compare server-normal image with drawable. */
		if (compsavimage(display, w, image) == False) {
			report("Area (server-normal bits-per-pixel=%d) was not properly put.", image->bits_per_pixel);
			FAIL;
		}
		else
			CHECK;
/* Clear drawable to W_FG. */
		dset(display, w, W_FG);
/* Call XPutImage with non-server-normal image. */
		image = im2;
		doxcall(w, 0, 0, 0, 0, image->width, image->height);
		XCALL;
/* Compare same image with drawable. */
		if (compsavimage(display, w, image) == False) {
			report("Area (non-server-normal bits-per-pixel=%d) was not properly put.", image->bits_per_pixel);
			FAIL;
		}
		else
			CHECK;
/* Compare other image with drawable. */
		if (compsavimage(display, w, im1) == False) {
			report("Server-normal image differs from drawable.");
			FAIL;
		}
		else
			CHECK;
/* Repeat for each visual. */
	}

	if (supported == 0) {
		report("Drawables with changeable bits_per_pixel not supported");
		tet_result(TET_UNSUPPORTED);
		return;
	} else
		CHECK;

	CHECKPASS(1+4*supported);
>>#ADDED peterc	As per external review comments.
>>#NOTE	peterc	I don't think the spec is clear enough for testing this...
>>ASSERTION Good A
When the image differs from the X server's format in
scanline-pad,
byte-order,
or
bit-order,
then the image will be converted to that format before drawing on
the specified drawable.
>>STRATEGY
Call makeimg to create image in ZPixmap format using
server's format in bits-per-pixel, scanline-pad, byte-order,
and bit-order.
Call makeimg to create another image in ZPixmap format.
Set scanline-pad to 8 if it is 32, else set it to 32.
Toggle byte-order between LSBFirst and MSBFirst.
Toggle bit-order between LSBFirst and MSBFirst.
Allocate memory for image data, discarding old data and clearing new.
Write known pattern to both images, using as many bits as we can.
Create drawable.
Call XPutImage with server-normal image.
Compare same image with drawable.
Clear drawable to W_FG.
Call XPutImage with non-server-normal image.
Compare same image with drawable.
Compare other image with drawable.
Repeat for each visual.
>>CODE
XImage *im1, *im2;
XVisualInfo *vp;
Window w;
char *data;
int nbits;

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {

/* Call makeimg to create image in ZPixmap format using */
/* server's format in bits-per-pixel, scanline-pad, byte-order, */
/* and bit-order. */
		trace("ZPixmap, standard");
		im1 = makeimg(display, vp, ZPixmap);

/* Call makeimg to create another image in ZPixmap format. */
		im2 = makeimg(display, vp, ZPixmap);

/* Set scanline-pad to 8 if it is 32, else set it to 32. */
		if (im2->bitmap_unit == 32)
			im2->bitmap_unit = 8;
		else if (im2->bitmap_pad == 32)
			im2->bitmap_unit = 32;
/* Toggle byte-order between LSBFirst and MSBFirst. */
		if (im2->byte_order == LSBFirst)
			im2->byte_order = MSBFirst;
		else
			im2->byte_order = LSBFirst;
/* Toggle bit-order between LSBFirst and MSBFirst. */
		if (im2->bitmap_bit_order == LSBFirst)
			im2->bitmap_bit_order = MSBFirst;
		else
			im2->bitmap_bit_order = LSBFirst;

/* Allocate memory for image data, discarding old data and clearing new. */
		/* discard old data as provided by makeimg() */
		if (im2->data != (char *) 0)
			XFree(im2->data);
		/* allocate a little extra just to be safe */
		data = (char *) malloc(2 * im2->bytes_per_line * im2->height);
		if (data == (char *) 0) {
			delete("malloc() error");
			return;
		}
		else
			CHECK;
		im2->data = data;
		/* clear new data to 0L */
		dsetimg(im2, 0L);
/* Write known pattern to both images, using as many bits as we can. */
		nbits = (im2->bitmap_unit < im1->bitmap_unit) ?
			im2->bitmap_unit : im1->bitmap_unit;
		trace("patterning im1 with %x", DEPTHMASK(nbits));
		patternimg(im1, DEPTHMASK(nbits));
		trace("patterning im2 with %x", DEPTHMASK(nbits));
		patternimg(im2, DEPTHMASK(nbits));
/* Create drawable. */
		w = makewin(display, vp);
		gc = makegc(display, w);
		dset(display, w, W_FG);
/* Call XPutImage with server-normal image. */
		image = im1;
		doxcall(w, 0, 0, 0, 0, image->width, image->height);
		XCALL;
/* Compare same image with drawable. */
		if (compsavimage(display, w, image) == False) {
			report("Area (server-normal bits-per-pixel=%d) was not properly put.", image->bits_per_pixel);
			FAIL;
		}
		else
			CHECK;
/* Clear drawable to W_FG. */
		dset(display, w, W_FG);
/* Call XPutImage with non-server-normal image. */
		image = im2;
		doxcall(w, 0, 0, 0, 0, image->width, image->height);
		XCALL;
/* Compare same image with drawable. */
		if (compsavimage(display, w, image) == False) {
			report("Area (non-server-normal bits-per-pixel=%d) was not properly put.", image->bits_per_pixel);
			FAIL;
		}
		else
			CHECK;
/* Compare other image with drawable. */
		if (compsavimage(display, w, im1) == False) {
			report("Server-normal image differs from drawable.");
			FAIL;
		}
		else
			CHECK;

/* Repeat for each visual. */
	}

	CHECKPASS(4*nvinf());
>>ASSERTION Bad A
When the 
.A image
.M format
is
.S XYPixmap 
or 
.S ZPixmap
and the
.A image
.M depth
does not match the
.A drawable
.M depth ,
then a
.S BadMatch 
error occurs.
>>STRATEGY
Create drawable.
Create image in XYPixmap format with depth different from drawable using XCreateImage.
Allocate memory for image data.
Call XPutImage to write image to drawable.
Verify XPutImage generated BadMatch error.
Destroy image with XDestroyImage.
Create image in ZPixmap format with depth different from drawable using XCreateImage.
Allocate memory for image data.
Call XPutImage to write image to drawable.
Verify XPutImage generated BadMatch error.
Destroy image with XDestroyImage.
>>CODE BadMatch
XVisualInfo *vp;
Window w;
int image_depth;
char *data;

	/* choose first visual: any will do */
	resetvinf(VI_WIN_PIX); nextvinf(&vp);

	if (nvinf() == 0) {
		unsupported("No usable visuals, check XT_DEBUG_WINDOWS_ONLY and XT_DEBUG_PIXMAPS_ONLY");
		return;
	} else
		CHECK;
	/* ensured that there is at least one drawable */

/* Create drawable. */
	w = makewin(display, vp);
	gc = makegc(display, w);

/* Create image in XYPixmap format with depth different from drawable. */
	if (vp->depth == 1)
		image_depth = 8;
	else
		image_depth = 1;
	image = XCreateImage(display, vp->visual, image_depth, XYPixmap, 0, NULL, I_STDWIDTH, I_STDHEIGHT, BitmapPad(display), 0);
	if (image == (XImage *) 0) {
		delete("XCreateImage failed");
		return;
	}
	else
		CHECK;

/* Allocate memory for image data. */
	data = (char *) malloc(image->height * image->bytes_per_line * image->depth);
	if (data == (char *) 0) {
		delete("malloc() error");
		return;
	}
	else
		CHECK;
	image->data = data;

/* Call XPutImage to write image to drawable. */
	doxcall(w, 0, 0, 0, 0, image->width, image->height);
	XCALL;

/* Verify XPutImage generated BadMatch error. */
	if (geterr() != BadMatch)
		FAIL;
	else
		CHECK;

/* Destroy image with XDestroyImage. */
	XDestroyImage(image);

/* Create image in ZPixmap format with depth different from drawable. */
	image = XCreateImage(display, vp->visual, image_depth, ZPixmap, 0, NULL, I_STDWIDTH, I_STDHEIGHT, BitmapPad(display), 0);
	trace("Repeat for ZPixmap format image.");
	if (image == (XImage *) 0) {
		delete("XCreateImage failed");
		return;
	}
	else
		CHECK;

/* Allocate memory for image data. */
	data = (char *) malloc(image->bytes_per_line * image->height);
	if (data == (char *) 0) {
		delete("malloc() error");
		return;
	}
	else
		CHECK;
	image->data = data;

/* Call XPutImage to write image to drawable. */
	doxcall(w, 0, 0, 0, 0, image->width, image->height);
	XCALL;

/* Verify XPutImage generated BadMatch error. */
	if (geterr() != BadMatch)
		FAIL;
	else
		CHECK;

/* Destroy image with XDestroyImage. */
	XDestroyImage(image);

	CHECKPASS(7);
>>#
>># The following assertion and the corresponding test were commented out
>># during the alpha test period in response to bug report 0180.
>># The rationale for this is that (despite the statements in the X11R4 Xlib
>># specification) the Xlib XImage functions should not have to cope 
>># with garbage combinations.
>>#
>># >>ASSERTION Bad A
>># When the
>># .A image
>># .M format
>># is
>># .S XYBitmap 
>># and the
>># .A image
>># .M depth
>># is not one,
>># then a
>># .S BadMatch 
>># error occurs.
>># >>STRATEGY
>># Create drawable.
>># Create image in XYBitmap with depth 8 using XCreateImage.
>># Allocate memory for image data.
>># Call XPutImage to write image to drawable.
>># Verify XPutImage generated BadMatch error.
>># Destroy image with XDestroyImage.
>># >>CODE BadMatch
>># XVisualInfo *vp;
>># Window w;
>># int depth;
>># char *data;
>># 
>># 	/* choose first visual: any will do */
>># 	resetvinf(VI_WIN_PIX); nextvinf(&vp);
>># 
>># 	if (nvinf() == 0) {
>># 		unsupported("No usable visuals, check XT_DEBUG_WINDOWS_ONLY and XT_DEBUG_PIXMAPS_ONLY");
>># 		return;
>># 	} else
>># 		CHECK;
>># 	/* ensured that there is at least one drawable */
>># 
>># /* Create drawable. */
>># 	w = makewin(display, vp);
>># 	gc = makegc(display, w);
>># 
>># /* Create image in XYBitmap with depth 8. */
>># 	trace("XYBitmap image");
>># 	depth = 8;
>># 	image = XCreateImage(display, vp->visual, depth, XYBitmap, 0, NULL, I_STDWIDTH, I_STDHEIGHT, BitmapPad(display), 0);
>># 	if (image == (XImage *) 0) {
>># 		delete("XCreateImage failed");
>># 		return;
>># 	}
>># 	else
>># 		CHECK;
>># 
>># /* Allocate memory for image data. */
>># 	data = (char *) malloc(image->height * image->bytes_per_line * depth);
>># 	if (data == (char *) 0) {
>># 		delete("malloc() error");
>># 		return;
>># 	}
>># 	else
>># 		CHECK;
>># 	image->data = data;
>># 
>># /* Call XPutImage to write image to drawable. */
>># 	doxcall(w, 0, 0, 0, 0, image->width, image->height);
>># 	XCALL;
>># 
>># /* Verify XPutImage generated BadMatch error. */
>># 	if (geterr() != BadMatch)
>># 		FAIL;
>># 	else
>># 		CHECK;
>># 
>># /* Destroy image with XDestroyImage. */
>># 	XDestroyImage(image);
>># 
>># 	CHECKPASS(4);
>># 
>>ASSERTION Bad A
.ER BadDrawable
>>ASSERTION Bad A
.ER BadGC
>>ASSERTION Bad A
.ER BadMatch inputonly 
>>ASSERTION Bad A
.ER BadMatch gc-drawable-depth
>>ASSERTION Bad A
.ER BadMatch gc-drawable-screen
>>#
>># The following assertion and the corresponding test were commented out
>># during the alpha test period in response to bug report 0180.
>># The rationale for this is that (despite the statements in the X11R4 Xlib
>># specification) the Xlib XImage functions should not have to cope 
>># with garbage combinations.
>>#
>># >>ASSERTION Bad A
>># When the image 
>># .M format
>># is other than
>># .S XYBitmap ,
>># .S XYPixmap
>># or
>># .S ZPixmap ,
>># then a
>># .S BadValue
>># error occurs.
>># >>STRATEGY
>># Create drawable.
>># Create image in formats other than XYBitmap, XYPixmap, and ZPixmap with XCreateImage.
>># Allocate memory for image data.
>># Call XPutImage to write image to drawable.
>># Verify XPutImage generated BadValue error.
>># Destroy image with XDestroyImage.
>># >>EXTERN
>># static int  allowed[] = {XYBitmap, XYPixmap, ZPixmap};
>># >>CODE BadValue
>># XVisualInfo *vp;
>># Window w;
>># char *data;
>># long format[NM_LEN];
>># int i, n;
>># 
>># 	/* choose first visual: any will do */
>># 	resetvinf(VI_WIN_PIX); nextvinf(&vp);
>># 
>># 	if (nvinf() == 0) {
>># 		unsupported("No usable visuals, check XT_DEBUG_WINDOWS_ONLY and XT_DEBUG_PIXMAPS_ONLY");
>># 		return;
>># 	} else
>># 		CHECK;
>># 	/* ensured that there is at least one drawable */
>># 
>># /* Create drawable. */
>># 	w = makewin(display, vp);
>># 	gc = makegc(display, w);
>># 
>># /* Create image in formats other than XYBitmap, XYPixmap, and ZPixmap. */
>># 	if ((n=notmember(allowed, NELEM(allowed), format)) <= 0) {
>># 		delete("No bad formats found");
>># 		return;
>># 	} else if (n > NM_LEN) {
>># 		delete("notmember unexpectedly gave %d results (> %d)", n, NM_LEN);
>># 		return;
>># 	} else
>># 		CHECK;
>># 		
>># 	for(i=0; i<n; i++) {
>># 		trace("Bad format number %d of %d", i+1, n);
>># 		/* can't use makeimg as it checks for bad formats */
>># 		image = XCreateImage(display, vp->visual, vp->depth, (int)format[i],
>># 			0, NULL, I_STDWIDTH, I_STDHEIGHT, BitmapPad(display), 0);
>># 
>># 		if (image == (XImage *) 0) {
>># 			delete("XCreateImage failed.");
>># 			return;
>># 		} else
>># 			CHECK;
>># 
>># /* Allocate memory for image data. */
>># 		/* this hopefully allocates too much memory... */
>># 		/* better too much than too little */
>># 		data = (char *) malloc(2 * image->height * image->bytes_per_line * image->depth);
>># 		if (data == (char *) 0) {
>># 			delete("malloc() error");
>># 			return;
>># 		}
>># 		else
>># 			CHECK;
>># 		image->data = data;
>># 
>># /* Call XPutImage to write image to drawable. */
>># 		doxcall(w, 0, 0, 0, 0, image->width, image->height);
>># 		XCALL;
>># 
>># /* Verify XPutImage generated BadValue error. */
>># 		if (geterr() != BadValue) {
>># 			report("%s gave %s instead of BadValue with format %d",
>># 				TestName, errorname(geterr()), (int)format[i]);
>># 			FAIL;
>># 		} else
>># 			CHECK;
>># 
>># /* Destroy image with XDestroyImage. */
>># 		XDestroyImage(image);
>># 	}
>># 
>># 	CHECKPASS(2+3*n);
>># 
>># HISTORY peterc Completed    Incorporated RTCB3.
>># HISTORY peterc Completed    Engineering.
>># HISTORY kieron Completed    Re-Engineering and bug fixing after first snapshot.
>># HISTORY kieron Completed    Use ZPixmap where format doesn't matter or we're>>#				not checking all of them for speed-up.
