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
 * $XConsortium: chngpntrcn.m,v 1.6 94/04/17 21:06:14 rws Exp $
 */
>>TITLE XChangePointerControl CH07
void

Display	*display = Dsp;
Bool	do_accel = False;
Bool	do_threshold = False;
int 	accel_numerator;
int 	accel_denominator;
int 	threshold;
>>EXTERN

static int 	oan, oad;
static int 	othresh;

/*
 * Save the original pointer values for restoring at the end of the test.
 */
>>SET startup saveold
static void
saveold()
{
	startup();
	if(Dsp)
		XGetPointerControl(Dsp, &oan, &oad, &othresh);
}

>>SET cleanup restorepoint
static void
restorepoint()
{
	if(Dsp)
		XChangePointerControl(Dsp, True, True, oan, oad, othresh);
	cleanup();
}

>>ASSERTION Good A
When
.A do_accel
is
.S True ,
then the acceleration of the pointer is set to the fraction
.A accel_numerator
over
.A accel_denominator .
>>STRATEGY
Set do_accel to True.
Set numerator and denominator values.
Call xname.
Verify that acceleration values have been set.
>>CODE
int 	newnum, newden, newthresh;

	do_accel = True;
	accel_numerator = 12;
	accel_denominator = 11;

	XCALL;

	XGetPointerControl(display, &newnum, &newden, &newthresh);

	if (newnum == accel_numerator)
		CHECK;
	else {
		report("accel_numerator was %d, expecting %d", newnum, accel_numerator);
		FAIL;
	}
	if (newden == accel_denominator)
		CHECK;
	else {
		report("accel_denominator was %d, expecting %d", newden, accel_denominator);
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
When
.A do_threshold
is
.S True ,
then the threshold
is set to the value in the
.A threshold
argument.
>># the acceleration only takes effect for pointer motions of more than
>># .A threshold
>># pixels at once and only applies to the number of pixels moved over the
>># threshold value.
>>STRATEGY
Set do_threshold to True.
Set value for threshold.
Call xname.
Verify that threshold value is set correctly.
>>CODE
int 	newnum, newden, newthresh;

	do_threshold = True;
	threshold = 43;

	XCALL;

	XGetPointerControl(display, &newnum, &newden, &newthresh);

	if (newthresh == threshold)
		CHECK;
	else {
		report("threshold was %d, expecting %d", newthresh, threshold);
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good A
When
.A threshold
is \-1, then the threshold is set to the default.
>>STRATEGY
Set do_threshold to True.
Set threshold to -1.
Call xname.
Obtain default value for threshold.
Set threshold to new value. 
Call xname.
Verify that threshold value is set correctly.
Set threshold to -1.
Call xname.
Verify that threshold value is set to default value.
>>CODE
int 	newnum, newden, newthresh;
int 	defnum, defden, defthresh;

	do_threshold = True;
	threshold = -1;

	XCALL;

	XGetPointerControl(display, &defnum, &defden, &defthresh);

	threshold = defthresh + 1;

	XCALL;

	XGetPointerControl(display, &newnum, &newden, &newthresh);

	if (newthresh == threshold)
		CHECK;
	else {
		report("When setting a non-default threshold value,");
		delete("threshold was %d, expecting %d", newthresh, threshold);
		return;
	}

	threshold = -1;

	XCALL;

	XGetPointerControl(display, &newnum, &newden, &newthresh);

	if (newthresh == defthresh)
		CHECK;
	else {
		report("When setting default threshold value,");
		report("threshold was %d, expecting %d", newthresh, defthresh);
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
When
.A accel_numerator
is \-1, then the numerator of the acceleration
is set to the default.
>>STRATEGY
Set do_accel to True.
Set numerator to -1.
Call xname.
Obtain default value for numerator.
Set numerator to new value. 
Call xname.
Verify that numerator value is set correctly.
Set numerator to -1.
Call xname.
Verify that numerator value is set to default value.
>>CODE
int 	newnum, newden, newthresh;
int 	defnum, defden, defthresh;

	do_accel = True;
	accel_numerator = -1;
	accel_denominator = 11;

	XCALL;

	XGetPointerControl(display, &defnum, &defden, &defthresh);

	accel_numerator = defnum + 1;

	XCALL;

	XGetPointerControl(display, &newnum, &newden, &newthresh);

	if (newnum == accel_numerator)
		CHECK;
	else {
		report("When setting a non-default numerator value,");
		report("accel_numerator was %d, expecting %d", newnum, accel_numerator);
		FAIL;
	}

	accel_numerator = -1;

	XCALL;

	XGetPointerControl(display, &newnum, &newden, &newthresh);

	if (newnum == defnum)
		CHECK;
	else {
		report("When setting default numerator value,");
		report("accel_numerator was %d, expecting %d", newnum, defnum);
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
When
.A accel_denominator
is \-1, then the denominator of the acceleration is set to the default.
>>STRATEGY
Set do_accel to True.
Set denominator to -1.
Call xname.
Obtain default value for denominator.
Set denominator to new value. 
Call xname.
Verify that denominator value is set correctly.
Set denominator to -1.
Call xname.
Verify that denominator value is set to default value.
>>CODE
int 	newnum, newden, newthresh;
int 	defnum, defden, defthresh;

	do_accel = True;
	accel_numerator = 12;
	accel_denominator = -1;

	XCALL;

	XGetPointerControl(display, &defnum, &defden, &defthresh);

	accel_denominator = defden + 1;

	XCALL;

	XGetPointerControl(display, &newnum, &newden, &newthresh);

	if (newden == accel_denominator)
		CHECK;
	else {
		report("When setting a non-default denominator value,");
		report("accel_denominator was %d, expecting %d", newden, accel_denominator);
		FAIL;
	}

	accel_denominator = -1;

	XCALL;

	XGetPointerControl(display, &newnum, &newden, &newthresh);

	if (newden == defden)
		CHECK;
	else {
		report("When setting default denominator value,");
		report("accel_denominator was %d, expecting %d", newden, defden);
		FAIL;
	}

	CHECKPASS(2);
>>ASSERTION Good A
When the value of the
.A do_accel
or
.A do_threshold
arguments are
.S False ,
then the corresponding parameter is not changed.
>>STRATEGY
Get old values of parameters.
Call xname with do_accel and do_threshold False.
Verify no change to parameters.
>>CODE
int 	oldnum, oldden, oldthresh;
int 	newnum, newden, newthresh;

	XGetPointerControl(display, &oldnum, &oldden, &oldthresh);

	accel_numerator = oldnum+4;
	accel_denominator = oldden+5;
	threshold = oldthresh+2;

	do_accel = False;
	do_threshold = False;

	XCALL;

	XGetPointerControl(display, &newnum, &newden, &newthresh);

	if (newnum == oldnum)
		CHECK;
	else {
		report("accel_numerator was changed when do_accel was False");
		FAIL;
	}
	if (newden == oldden)
		CHECK;
	else {
		report("accel_denominator was changed when do_accel was False");
		FAIL;
	}
	if (newthresh == oldthresh)
		CHECK;
	else {
		report("threshold was changed when do_threshold was False");
		FAIL;
	}

	CHECKPASS(3);
>>ASSERTION Good A
When
.A threshold
is a negative number other than \-1, then a
.S BadValue
error occurs.
>>STRATEGY
Set do_threshold to True.
Set threshold to negative value other than -1.
Verify BadValue error.
>>CODE BadValue

	do_threshold = True;
	threshold = -3;

	XCALL;

	if (geterr() == BadValue)
		PASS;
>>ASSERTION Good A
When
.A accel_numerator
or
.A accel_denominator
are negative numbers other than \-1, then a
.S BadValue
error occurs.
>>STRATEGY
Set do_accel to True.
Set accel parameters to invalid values.
Verify BadValue error.
>>CODE BadValue

	do_accel = True;

	accel_denominator = 3;
	accel_numerator = -3;

	XCALL;

	if (geterr() == BadValue)
		CHECK;

	accel_denominator = -3;
	accel_numerator = 2;

	XCALL;

	if (geterr() == BadValue)
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good A
When the
.A accel_denominator
argument is zero, then a
.S BadValue
error occurs.
>>STRATEGY
Set do_accel to True.
Set accel_denominator to zero.
Verify that a BadValue error occurs.
>>CODE BadValue

	do_accel = True;
	accel_denominator = 0;
	accel_numerator = 1;

	XCALL;

	if (geterr() == BadValue)
		PASS;
>>ASSERTION Bad A
.ER Value do_accel True False
>>ASSERTION Bad A
.ER Value do_threshold True False
