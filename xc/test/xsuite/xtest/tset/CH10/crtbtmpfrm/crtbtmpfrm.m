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
 * $XConsortium: crtbtmpfrm.m,v 1.10 94/04/17 21:09:23 rws Exp $
 */
>>TITLE XCreateBitmapFromData CH10
Pixmap

Display *display = Dsp;
Drawable d = defwin(display);
char *data = (char*)cbfd_one;
unsigned int width=10;
unsigned int height=20;
>>EXTERN
/* data for a "checkerboard" 10x20 bitmap. */
static unsigned char cbfd_one[]= {
	0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00,
	0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0x1f, 0x00, 0xe0, 0x03, 0xe0, 0x03,
	0xe0, 0x03, 0xe0, 0x03, 0xe0, 0x03, 0xe0, 0x03, 0xe0, 0x03, 0xe0, 0x03,
	0xe0, 0x03, 0xe0, 0x03};

>>ASSERTION Good A
A call to xname returns a bitmap containing the
.A data
of width
.A width
and height
.A height .
>>STRATEGY
Call xname to create the bitmap from the data.
Use XGetGeometry to check pixmap.
Verify the pixmap was as expected.
>>CODE
Pixmap ret;
Window wt_r;
int it_r;
unsigned int width_ret, height_ret;
unsigned int uit_r;
unsigned int depth_ret;
struct area ar;

/* Call xname to create the bitmap from the data. */
	ret=XCALL;

/* Use XGetGeometry to check pixmap. */
	if(!XGetGeometry(display, ret, &wt_r, &it_r, &it_r,
		&width_ret, &height_ret, &uit_r, &depth_ret)) {
		delete("XGetGeometry failed.");
		return;
	} else
		CHECK;

#ifdef TESTING
	depth_ret = 2;
	width_ret++;
	height_ret++;
#endif

	if(width_ret!=width || height_ret!=height) {
		FAIL;
		report("%s created a pixmap of unexpected geometry.",
			TestName);
		report("Expected width, height: %d,%d", width, height);
		report("Returned width, height: %d,%d", width_ret, height_ret);
	} else
		CHECK;

	if(depth_ret != 1) {
		FAIL;
		report("%s created a pixmap of unexpected depth.",
			TestName);
		report("Expected depth: 1");
		report("Returned depth: %u", depth_ret);
	} else
		CHECK;

/* Verify the pixmap was as expected. */
	setarea(&ar,0,0,5,10);
	if(!checkarea(display, ret, &ar, W_FG, 0, CHECK_IN)) {
		FAIL;
		report("%s did not return the expected pixmap.", TestName);
		trace("Top right quarter was incorrect");
	} else
		CHECK;

	setarea(&ar,5,0,5,10);
	if(!checkarea(display, ret, &ar, W_BG, 0, CHECK_IN)) {
		FAIL;
		report("%s did not return the expected pixmap.", TestName);
		trace("Top left quarter was incorrect");
	} else
		CHECK;

	setarea(&ar,0,10,5,10);
	if(!checkarea(display, ret, &ar, W_BG, 0, CHECK_IN)) {
		FAIL;
		report("%s did not return the expected pixmap.", TestName);
		trace("Bottom right quarter was incorrect");
	} else
		CHECK;

	setarea(&ar,5,10,5,10);
	if(!checkarea(display, ret, &ar, W_FG, 0, CHECK_IN)) {
		FAIL;
		report("%s did not return the expected pixmap.", TestName);
		trace("Bottom Left quarter was incorrect");
	} else
		CHECK;

	CHECKPASS(7);

>>ASSERTION Bad B 1
When sufficient temporary storage cannot be allocated,
then a call to xname returns
.S None . 
>>ASSERTION Bad B
.ER BadAlloc
>>ASSERTION Bad A
When a drawable argument does not name a valid drawable, then one or more
.S BadDrawable
errors, one or more
.S BadGC
errors or both types of error occur.
>>STRATEGY
Create a bad drawable by creating and destroying a window.
Call test function using bad drawable as the drawable argument.
Verify that a BadDrawable and BadGC error occurs.
>>CODE BadDrawable

	seterrdef();

	A_DRAWABLE = (Drawable)badwin(A_DISPLAY);

>>SET no-error-status-check
	XCALL;

	if (geterr() == BadDrawable || geterr() == BadGC)
		PASS;
	else
		FAIL;
