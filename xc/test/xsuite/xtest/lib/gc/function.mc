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
 * $XConsortium: function.mc,v 1.12 94/04/17 21:14:44 rws Exp $
 */
>>EXTERN

#ifdef A_DRAWABLE2
#define	A_DRAW	A_DRAWABLE2
#else
#define A_DRAW	A_DRAWABLE
#endif

static	int 	f_pix_x = -1;
static	int 	f_pix_y = -1;

static void
functest(functype)
int 	functype;
{
XVisualInfo	*vp;
unsigned long	srcpix;
unsigned long	destpix;
unsigned long	expected;
unsigned long	actualpix;
unsigned long	maxpix;
int 	pass = 0, fail = 0;

	trace("Function %s", gcfunctionname(functype));

	for (resetvinf(VI_WIN_PIX); nextvinf(&vp); ) {
#ifdef A_DRAWABLE2
		winpair(A_DISPLAY, vp, &A_DRAWABLE, &A_DRAWABLE2);
#if T_XCopyPlane
		dset(A_DISPLAY, A_DRAWABLE, ~0L);
#else
		dset(A_DISPLAY, A_DRAWABLE, W_FG);
#endif
#else
		A_DRAW = makewin(A_DISPLAY, vp);
#endif
		A_GC = makegc(A_DISPLAY, A_DRAW);
#ifdef A_IMAGE
		A_IMAGE = makeimg(A_DISPLAY, vp, ZPixmap);
		dsetimg(A_IMAGE, W_FG);
#endif

		/*
		 * If this is the first call find a pixel that is drawn
		 * that can be used for all the tests.
		 */
		if (f_pix_x == -1) {
			XCALL;
			setfuncpixel(A_DISPLAY, A_DRAW, &f_pix_x, &f_pix_y);
			dclear(A_DISPLAY, A_DRAW);
			if (f_pix_x == -1) {
				report("Nothing was drawn with a gc function of GXcopy");
				/*
				 * If we were actually testing GXcopy then the test has
				 * failed (in a way), so say FAIL.
				 */
				if (functype == GXcopy)
					FAIL;
				else
					delete("Setup error in functest");
				return;
			}
		}

		XSetFunction(A_DISPLAY, A_GC, functype);

		/* Try out a few more values without taking too long */
		maxpix = 3;

		for (destpix = 0; destpix <= maxpix; destpix++) {
			for (srcpix = 0; srcpix <= 1; srcpix++) {

				trace("src=%d, dest=%d", srcpix, destpix);

				/*
				 * The test on depth shouldn't be necessary, however a common
				 * bug exists where pixels are not truncated on mono screens.
				 * The bug is diagnosed in another test so causing failures here
				 * as well would be confusing.
				 */
				if (vp->depth == 1)
					dset(A_DISPLAY, A_DRAW, destpix&1);
				else
					dset(A_DISPLAY, A_DRAW, destpix);
#if T_XCopyArea
				dset(A_DISPLAY, A_DRAWABLE, srcpix);
#else
#if T_XPutImage
				dsetimg(A_IMAGE, srcpix);
#else
				XSetForeground(A_DISPLAY, A_GC, srcpix);
#endif
#endif
				XCALL;

				switch (functype) {
				case GXclear:
					expected = 0;
					break;
				case GXand:
					expected = srcpix & destpix;
					break;
				case GXandReverse:
					expected = srcpix & ~destpix;
					break;
				case GXcopy:
					expected = srcpix;
					break;
				case GXandInverted:
					expected = ~srcpix & destpix;
					break;
				case GXnoop:
					expected = destpix;
					break;
				case GXxor:
					expected = srcpix ^ destpix;
					break;
				case GXor:
					expected = srcpix | destpix;
					break;
				case GXnor:
					expected = ~srcpix & ~destpix;
					break;
				case GXequiv:
					expected = ~srcpix ^ destpix;
					break;
				case GXinvert:
					expected = ~destpix;
					break;
				case GXorReverse:
					expected = srcpix | ~destpix;
					break;
				case GXcopyInverted:
					expected = ~srcpix;
					break;
				case GXorInverted:
					expected = ~srcpix | destpix;
					break;
				case GXnand:
					expected = ~srcpix | ~destpix;
					break;
				case GXset:
					expected = ~0;
					break;
				default:
					report("Bad value given to functest");
					tet_result(TET_UNRESOLVED);
					break;
				}

				expected &= DEPTHMASK(vp->depth);
				actualpix = getpixel(A_DISPLAY, A_DRAW, f_pix_x, f_pix_y);

				debug(3, "src %d, dst %d, expect %d", srcpix, destpix,expected);

				if (actualpix == expected)
					CHECK;
				else {
					report("%s fail expected %d, got %d", gcfunctionname(functype), expected, actualpix);
					FAIL;
				}
			}
		}

	}

	CHECKPASS(2*(maxpix+1)*nvinf());
}

>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXclear ,
then the destination becomes
(dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXclear.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is 0.
>>CODE

	functest(GXclear);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXand ,
then the destination becomes
((src AND dst) AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXand.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is src&dest.
>>CODE

	functest(GXand);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXandReverse ,
then the destination becomes
((src AND (NOT dst)) AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXandReverse.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is src&~dest.
>>CODE

	functest(GXandReverse);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXcopy ,
then the destination becomes
(src AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXcopy.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is src.
>>CODE

	functest(GXcopy);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXandInverted ,
then the destination becomes
(((NOT src) AND dst) AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXandInverted.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is ~src&dest.
>>CODE

	functest(GXandInverted);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXnoop ,
then the destination becomes
(dst AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXnoop.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is dest.
>>CODE

	functest(GXnoop);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXxor ,
then the destination becomes
((src XOR dst) AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXxor.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is src^dest.
>>CODE

	functest(GXxor);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXor ,
then the destination becomes
((src OR dst) AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXor.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is src|dest.
>>CODE

	functest(GXor);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXnor ,
then the destination becomes
(((NOT src) AND (NOT dst)) AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXnor.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is ~src&~dest.
>>CODE

	functest(GXnor);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXequiv ,
then the destination becomes
(((NOT src) XOR dst) AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXequiv.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is ~src^dest.
>>CODE

	functest(GXequiv);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXinvert ,
then the destination becomes
((NOT dst) AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXinvert.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is ~dest.
>>CODE

	functest(GXinvert);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXorReverse ,
then the destination becomes
((src OR (NOT dst)) AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXorReverse.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is src|~dest.
>>CODE

	functest(GXorReverse);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXcopyInverted ,
then the destination becomes
((NOT src) AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXcopyInverted.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is ~src.
>>CODE

	functest(GXcopyInverted);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXorInverted ,
then the destination becomes
(((NOT src) OR dst) AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXorInverted.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is ~src|dest.
>>CODE

	functest(GXorInverted);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXnand ,
then the destination becomes
(((NOT src) OR (NOT dst)) AND
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXnand.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is ~src|~dest.
>>CODE

	functest(GXnand);
>>ASSERTION Good A
When a graphics operation is performed and the source pixel is src and
the destination pixel is dst
and
.M function
is
.S GXset ,
then the destination becomes
(
.M plane_mask )
OR (dst AND (NOT
.M plane_mask )).
>>STRATEGY
Find a point (f_pix_x, f_pix_y) that is drawn by the graphics operation.

Set function component of GC to GXset.
For destination pixel values dst = 0, 1, 2, 3
    For source pixel values of src = 0, 1
	Set all points in drawable to destination pixel value.
	Set foreground component of GC to source pixel value.
	  (or fill source drawable/image with source pixel value in the cases
	   of XCopyArea or XPutImage, while XCopyPlane always has it ~0 to
	   force a foreground fill).
	Do graphics operation.
	Verify that pixel value at f_pix_x, f_pix_y is ~0.
>>CODE

	functest(GXset);
>># HISTORY	kieron	Completed	Use ZPixmap images, fix for XCopyPlane
