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
 * $XConsortium: bll.m,v 1.7 94/04/17 21:06:09 rws Exp $
 */
>>TITLE XBell CH07
void

Display	*display = Dsp;
int 	percent = 50;
>>ASSERTION Good D 3
If it is possible to ring a bell on the display:
When the percent argument is nonnegative, then
the volume at which the bell rings, where base is the base volume
of the bell, is
base \- [(base*percent)/100] + percent.
>>STRATEGY
Try out some nonnegative values.
>>CODE
int 	i;
static int 	vals[] = {
	0, 2, 12, 30, 45, 60, 100};

	for (i = 0; i < NELEM(vals); i++) {
		percent = vals[i];
		XCALL;
	}

	untested("Test for bell functionality present only");

>>ASSERTION Good D 3
If it is possible to ring a bell on the display:
When the percent argument is negative, then
the volume at which the bell rings, where base is the base volume
of the bell, is
base + [(base*percent)/100].
>>STRATEGY
Try out some negative values.
>>CODE
int 	i;
static int 	vals[] = {
	-2, -17, -30, -60, -100};

	for (i = 0; i < NELEM(vals); i++) {
		percent = vals[i];
		XCALL;
	}

	untested("Test for bell functionality present only");

>>ASSERTION Bad A
When the value for the percent argument is not in the range \-100 to 100
inclusive, then a
.S BadValue
error occurs.
>>STRATEGY
Call xname with invalid percent argument.
Verify that BadValue error occurs.
>>CODE BadValue

	percent = -101;
	XCALL;

	if (geterr() == BadValue)
		CHECK;

	percent = 141;
	XCALL;

	if (geterr() == BadValue)
		CHECK;

	CHECKPASS(2);
