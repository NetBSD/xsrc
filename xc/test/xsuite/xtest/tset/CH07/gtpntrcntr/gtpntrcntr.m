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
 * $XConsortium: gtpntrcntr.m,v 1.4 94/04/17 21:06:34 rws Exp $
 */
>>TITLE XGetPointerControl CH07
void

Display	*display = Dsp;
int 	*accel_numerator_return = &num;
int 	*accel_denominator_return = &denom;
int 	*threshold_return = &thresh;
>>EXTERN

static int 	num;
static int 	denom;
static int 	thresh;

>>ASSERTION Good A
A call to xname returns the current acceleration multiplier
and acceleration threshold to
.A accel_numerator_return ,
.A accel_denominator_return
and
.A threshold_return .
>>STRATEGY
Set some values.
Call xname.
Verify values are as set.
>>CODE
int 	onum, odenom, othresh;
int 	val = 34;

	/* First get original values */
	accel_numerator_return = &onum;
	accel_denominator_return = &odenom;
	threshold_return = &othresh;
	XCALL;

	XChangePointerControl(display, True, True, val, val, val);
	if (isdeleted())
		return;

	accel_numerator_return = &num;
	accel_denominator_return = &denom;
	threshold_return = &thresh;
	XCALL;

	if (num == val)
		CHECK;
	else {
		report("accel_numerator_return was %d, expecting %d", num, val);
		FAIL;
	}
	if (denom == val)
		CHECK;
	else {
		report("accel_denominator_return was %d, expecting %d", denom, val);
		FAIL;
	}
	if (thresh == val)
		CHECK;
	else {
		report("threshold_return was %d, expecting %d", thresh, val);
		FAIL;
	}

	CHECKPASS(3);

	XChangePointerControl(display, True, True, onum, odenom, othresh);
	XSync(display, False);
