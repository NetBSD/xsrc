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
 * $XConsortium: rdbtmpfl.m,v 1.12 94/04/17 21:10:01 rws Exp $
 */
>>TITLE XReadBitmapFile CH10
int

Display *display = Dsp;
Drawable d = (Drawable)defwin(display);
char *filename = xrbf_name;
unsigned int *width_return = &width;
unsigned int *height_return = &height;
Pixmap *bitmap_return = &bitmap;
int *x_hot_return = &x_hot;
int *y_hot_return = &y_hot;
>>EXTERN
#include <stdio.h>

/* Function return variables. */
static unsigned int width;
static unsigned int height;
static Pixmap bitmap;
static int x_hot;
static int y_hot;

/* Temporary bitmap filename. */
static char *xrbf_name = "xrbf_temp";
static char *xrbf_bad_name = "xrbf_nofile";

/* Valid bitmap file without hotspot definition. */
static char *xrbf_one[] = {
	"#define test_width 16",
	"#define test_height 8",
	"static char test_bits[] = {",
	"0x80, 0x00, 0x3f, 0xfe, 0x04, 0x07, 0x88, 0x41,",
	"0xfc, 0x09, 0x00, 0x80, 0x40, 0x20, 0x08, 0x04};",
} ;
static int xrbf_n_one = NELEM(xrbf_one);

/* Valid bitmap file with hotspot definition. */
static char *xrbf_two[] = {
	"#define test_width 16",
	"#define test_height 8",
	"#define test_x_hot 5",
	"#define test_y_hot 6",
	"static char test_bits[] = {",
	"0x80, 0x00, 0x3f, 0xfe, 0x04, 0x07, 0x88, 0x41,",
	"0xfc, 0x09, 0x00, 0x80, 0x40, 0x20, 0x08, 0x04};",
} ;
static int xrbf_n_two = NELEM(xrbf_two);

/* Invalid bitmap file. */
static char *xrbf_three[] = {
	"#define not bit map data",
	"static char data_bits[]={",
	"};",
};
static int xrbf_n_three = NELEM(xrbf_three);

static char *xrbf_verify_array[8]={
	"0000000100000000",
	"1111110001111111",
	"0010000011100000",
	"0001000110000010",
	"0011111110010000",
	"0000000000000001",
	"0000001000000100",
	"0001000000100000",
};


static int
xrbf_create(file, data, elements)
char *file;
char **data;
int elements;
{
	FILE *fp;
	int a;

	fp = fopen(file, "w");
	if (fp == (FILE *)NULL) {
		delete("Could not create temporary bitmap file '%s'", file);
		return -1;
	}

	a = 0;
	while( a<elements ) {
		(void) fprintf(fp, "%s\n", data[a++]);
	}

	fclose(fp);
	return 0;
}

static void
xrbf_b_start() {
	tpstartup();
	(void) xrbf_create(xrbf_name, xrbf_one, xrbf_n_one);
}


static void
xrbf_b_end()  {
	(void) unlink(xrbf_name);
	tpcleanup();
}

static int
xrbf_check(pm, array, w, h)
Pixmap pm;
char **array;
int w;
int h;
{
	int lh, lw, bad;
	unsigned long exp;

	bad = 0;
	for(lh=0; lh<h; lh++) {
		for(lw=0; lw<w; lw++) {
			int pix;
			exp = (array[lh][lw]=='0'?W_BG:W_FG);
			pix = checkpixel(display, pm, lw, lh, exp);
			if ( !pix ) {
				trace("Bad Pixel at %d,%d", lw, lh);
				bad++;
			}
		}
	}

	return bad;
}

>>ASSERTION Good A
When the file
.A filename
is readable and in the X11 bitmap format, then a call to xname returns
.S BitmapSuccess . 
>>SET return-value BitmapSuccess
>>STRATEGY
Create a valid, readable, bitmap file.
Call xname to read the bitmap file.
Verify that BitmapSuccess was returned.
>>CODE
int ret;

/* Create a valid, readable, bitmap file. */
	if (xrbf_create(filename, xrbf_two, xrbf_n_two)) {
		return;
	} else
		CHECK;

/* Call xname to read the bitmap file. */
	XCALL;

	(void) unlink(filename);
	CHECKPASS(1);

>>ASSERTION Good A
When the file
.A filename
is readable and in the X11 bitmap format, then a call to xname
returns the bitmap's height and width as read
from the file in
.A width_return
and
.A height_return ,
and a pixmap containing the bitmap with the bitmap's height and width, on
the same screen as the
.A drawable ,
in
.A bitmap_return .
>>STRATEGY
Create a valid, readable, bitmap file.
Call xname to read the bitmap file.
Verify that the width was returned as expected.
Verify that the height was returned as expected.
Call XGetGeometry to obtain the bitmap width, height and depth.
Verify that the bitmap was of the correct height, width and depth.
Verify that the bitmap contents were correct.
Verify that the bitmap was created on the correct screen.
>>CODE
Window w_tmp, rw_ret, rw2_ret;
int i_tmp;
unsigned int h_ret, w_ret, d_ret, ui_tmp;

/* Create a valid, readable, bitmap file. */
	if (xrbf_create(filename, xrbf_one, xrbf_n_one)) {
		return;
	} else
		CHECK;

/* Call xname to read the bitmap file. */
	width = 0;
	height = 0;
	bitmap = (Pixmap)NULL;

	XCALL;

/* Verify that the width was returned as expected. */
	if (width != 16) {
		FAIL;
		report("%s did not return the expected width",
			TestName);
		report("Expected width: 16");
		report("Returned width: %d", width);
	} else
		CHECK;

/* Verify that the height was returned as expected. */
	if (height != 8) {
		FAIL;
		report("%s did not return the expected height",
			TestName);
		report("Expected height: 8");
		report("Returned height: %d", height);
	} else
		CHECK;

/* Call XGetGeometry to obtain the bitmap width, height and depth. */
	(void)XGetGeometry(display, bitmap, &rw_ret, &i_tmp, &i_tmp,
		&w_ret, &h_ret, &ui_tmp, &d_ret);

/* Verify that the bitmap was of the correct height, width and depth. */
	if (w_ret != 16 || h_ret != 8 || d_ret != 1) {
		FAIL;
		report("%s created a pixmap of unexpected dimensions.",
			TestName);
		report("Expected width :16; Returned width :%u", w_ret);
		report("Expected height: 8; Returned height:%u", h_ret);
		report("Expected depth : 1; Returned depth :%u", d_ret);
	} else
		CHECK;

/* Verify that the bitmap contents were correct. */
	if (xrbf_check(bitmap, xrbf_verify_array, 16, 8)) {
		FAIL;
		report("%s did not generate the expected pixmap.",
			TestName);
	} else
		CHECK;

/* Verify that the bitmap was created on the correct screen. */
	(void)XGetGeometry(display, d, &rw2_ret, &i_tmp, &i_tmp,
		&ui_tmp, &ui_tmp, &ui_tmp, &ui_tmp);
	if(rw2_ret != rw_ret) {
		FAIL;
		report("%s did not generate the pixmap on the expected screen.",
			TestName);
		report("Root window for drawable and bitmap differ.");
	} else
		CHECK;
		
	(void) unlink(filename);

	CHECKPASS(6);

>>ASSERTION Good A
When the file
.A filename
is readable, and in the X11 bitmap format, and contains 
.M name_x_hot
and
.M name_y_hot ,
then a call to xname returns the value of
.M name_x_hot
to
.A x_hot_return ,
and the value of
.M name_y_hot
to
.A y_hot_return .
>>STRATEGY
Create a valid, readable, bitmap file containing a hotspot.
Call xname to read the bitmap file.
Verify that the x_hot_return was returned as expected.
Verify that the y_hot_return was returned as expected.
>>CODE
Window w_tmp;
int i_tmp;
unsigned int h_ret, w_ret, d_ret, ui_tmp;

/* Create a valid, readable, bitmap file containing a hotspot. */
	if (xrbf_create(filename, xrbf_two, xrbf_n_two)) {
		return;
	} else
		CHECK;

/* Call xname to read the bitmap file. */
	x_hot = 0;
	y_hot = 0;
	XCALL;

/* Verify that the x_hot_return was returned as expected. */
	if (x_hot != 5) {
		FAIL;
		report("%s did not return the expected x_hot_return",
			TestName);
		report("Expected x_hot_return: 5");
		report("Returned x_hot_return: %d", x_hot);
	} else
		CHECK;

/* Verify that the y_hot_return was returned as expected. */
	if (y_hot != 6) {
		FAIL;
		report("%s did not return the expected y_hot_return",
			TestName);
		report("Expected y_hot_return: 6");
		report("Returned y_hot_return: %d", y_hot);
	} else
		CHECK;

	(void) unlink(filename);

	CHECKPASS(3);

>>ASSERTION Good A
When the file
.A filename
is readable, and in the X11 bitmap format, and does not contain
.M name_x_hot
and
.M name_y_hot ,
then a call to xname returns -1 to
.A x_hot_return
and
.A y_hot_return .
>>STRATEGY
Create a valid, readable, bitmap file containing no hotspot.
Call xname to read the bitmap file.
Verify that the x_hot_return was returned as expected.
Verify that the y_hot_return was returned as expected.
>>CODE
Window w_tmp;
int i_tmp;
unsigned int h_ret, w_ret, d_ret, ui_tmp;

/* Create a valid, readable, bitmap file containing no hotspot. */
	if (xrbf_create(filename, xrbf_one, xrbf_n_one)) {
		return;
	} else
		CHECK;

/* Call xname to read the bitmap file. */
	x_hot = 0;
	y_hot = 0;
	XCALL;

/* Verify that the x_hot_return was returned as expected. */
	if (x_hot != -1) {
		FAIL;
		report("%s did not return the expected x_hot_return",
			TestName);
		report("Expected x_hot_return: -1");
		report("Returned x_hot_return: %d", x_hot);
	} else
		CHECK;

/* Verify that the y_hot_return was returned as expected. */
	if (y_hot != -1) {
		FAIL;
		report("%s did not return the expected y_hot_return",
			TestName);
		report("Expected y_hot_return: -1");
		report("Returned y_hot_return: %d", y_hot);
	} else
		CHECK;

	(void) unlink(filename);

	CHECKPASS(3);
>>ASSERTION Bad A
When the file
.A filename
cannot be opened, then a call to xname returns 
.S BitmapOpenFailed .
>>STRATEGY
Call xname with a non-existant bitmap filename.
Verify that BitmapOpenFailed was returned.
>>CODE
int ret;

/* Call xname with a non-existant bitmap filename. */
	filename = xrbf_bad_name;
	ret = XCALL;

/* Verify that BitmapOpenFailed was returned. */
	if (ret != BitmapOpenFailed) {
		FAIL;
		report("%s did not return correct value with a nonexistant",
			TestName);
		report("bitmap file.");
		report("Expected: %d (BitmapOpenFailed)");
		report("Returned: %d", ret);
	} else
		CHECK;

	CHECKPASS(1);

>>ASSERTION Bad A
When the file
.A filename
is readable and does not contain valid bitmap data, then a call to
xname returns
.S BitmapFileInvalid .
>>STRATEGY
Create an invalid, bitmap file.
Call xname to read the bitmap file.
Verify that BitmapFileInvalid was returned.
>>CODE
int ret;

/* Create an invalid, bitmap file. */
	if (xrbf_create(filename, xrbf_three, xrbf_n_three)) {
		return;
	} else
		CHECK;

/* Call xname to read the bitmap file. */
	ret = XCALL;

/* Verify that BitmapFileInvalid was returned. */
	if (ret != BitmapFileInvalid) {
		FAIL;
		report("%s did not return correct value with an invalid",
			TestName);
		report("bitmap file.");
		report("Expected: %d (BitmapFileInvalid)");
		report("Returned: %d", ret);
	} else
		CHECK;

	(void) unlink(filename);

	CHECKPASS(2);

>>ASSERTION Bad B 1
When insufficient memory is allocated, then a call to xname returns
.S BitmapNoMemory .
>>ASSERTION Bad A
.ER BadAlloc
>>SET tpstartup xrbf_b_start
>>SET tpcleanup xrbf_b_end
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
