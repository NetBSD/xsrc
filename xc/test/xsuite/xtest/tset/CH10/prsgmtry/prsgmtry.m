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
 * $XConsortium: prsgmtry.m,v 1.9 94/04/17 21:09:57 rws Exp $
 */
>>TITLE XParseGeometry CH10
int
XParseGeometry(parsestring, x_return, y_return, width_return, height_return)
char	*parsestring;
int	*x_return;
int	*y_return;
>># Documentation says 'int' for the width and height.  Changed to unsigned
>># here to match the header prototype which is consistant with the
>># use of unsigned for other width and height parameters.
unsigned int	*width_return;
unsigned int	*height_return;
>>ASSERTION Good A
>>#
>># .Ds
>># [=][<width>x<height>][{+-}<xoffset>{+-}<yoffset>],
>># [=][<\fIwidth\fP>x<\fIheight\fP>][{+-}<\fIxoffset\fP>{+-}<\fIyoffset\fP>] 
>># .De
>>#
When the
.A parsestring
argument is a string of the form 
.Ds
[=][<\fIwidth\fP>x<\fIheight\fP>][{+-}<\fIxoffset\fP>{+-}<\fIyoffset\fP>] 
.De
then a call to xname returns a bitwise OR of
.S "WidthValue | HeightValue" ,
.S "XValue | YValue" ,
.S XNegative
or
.S YNegative
depending on whether the height and width are specified, whether the offsets are specified
and on the sign of the specified offsets respectively.
>>STRATEGY
For geometry strings covering each case of interest
  Parse string with xname.
  Verify that correct flags are set.
  For each value
	If corresponding flag is set
	  Verify that value is set correctly.
	else
	  Verify that value is unchanged.
>>CODE
#define	DEFV	123
static struct	list {
	char	*pstr;	/* Parse string */
	int 	flags;	/* returned flags */
	int 	x;
	int 	y;
	unsigned int    width;
	unsigned int    height;
} list[] = {
	{"=10x20", WidthValue|HeightValue, DEFV, DEFV, 10, 20},
	{"=+10+20", XValue|YValue, 10, 20, DEFV, DEFV},
	{"=1x2-10-20", WidthValue|HeightValue|XValue|YValue|XNegative|YNegative, -10, -20, 1, 2},
	{"=1x2-0-0", WidthValue|HeightValue|XValue|YValue|XNegative|YNegative, 0, 0, 1, 2},
	{"=10x20+2-3", WidthValue|HeightValue|XValue|YValue|YNegative, 2, -3, 10, 20}
};
struct	list	*lp;
int		i;
int		xr;
int		yr;
unsigned int		wr;
unsigned int		hr;
int		result;

	x_return = &xr;
	y_return = &yr;
	width_return = &wr;
	height_return = &hr;

	/*
	 * Essentially we run through the list twice, the first time
	 * without the '=' the second time with.
	 */
	for(i=0; i < 2*NELEM(list); i++) {

		lp = &list[i%NELEM(list)];

		/* Set all values to some default */
		xr = yr = wr = hr = DEFV;

		/* Set parse string, and skip the '=' if this is first time through */
		parsestring = lp->pstr;
		if (i < NELEM(list))
			parsestring++;
		trace("parse string of \"%s\"", parsestring);
		result = XCALL;

		if(result != lp->flags) {
			report("%s() returned 0x%x instead of 0x%x for parsestring \"%s\".", TestName, result, lp->flags, parsestring);
			FAIL;
		} else
			CHECK;

		if (xr == lp->x)
			CHECK;
		else if (lp->x == DEFV) {
			report("x_return was unexpectedly altered to %d", xr);
			FAIL;
		} else {
			report("Returned x value was %d, expecting %d", xr, lp->x);
			FAIL;
		}
		if (yr == lp->y)
			CHECK;
		else if (lp->y == DEFV) {
			report("y_return was unexpectedly altered to %d", yr);
			FAIL;
		} else {
			report("Returned y value was %d, expecting %d", yr, lp->y);
			FAIL;
		}
		if (wr == lp->width)
			CHECK;
		else if (lp->width == DEFV) {
			report("width_return was unexpectedly altered to %u", wr);
			FAIL;
		} else {
			report("Returned width value was %u, expecting %u", wr, lp->width);
			FAIL;
		}
		if (hr == lp->height)
			CHECK;
		else if (lp->x == DEFV) {
			report("height_return was unexpectedly altered to %u", hr);
			FAIL;
		} else {
			report("Returned height value was %u, expecting %u", hr, lp->height);
			FAIL;
		}
	}

	CHECKPASS(5*2*NELEM(list));

>># COMMENT
>># Added the word "just" to this assertion.
>># Cal.
>>ASSERTION Good A
When the
.A parsestring
argument contains just xoffset and yoffset specifications, 
then a call to xname returns these values
in the
.A x_return
and
.A y_return
arguments respectively and the
.A width_return
and
.A height_return
arguments are not altered.
>>STRATEGY
Parse the string  "+32768-32768" using xname.
Verify that the call returns XValue | YValue | YNegative.
Verify that the x_return argument is set to 32768.
Verify that the y_return argument is set to -32768.
Verify that the width_return argument is unaltered.
Verify that the height_return argument is unaltered.
>>CODE
int		xr = -1;
int		yr = -1;
int		wr = -13;
int		hr = -666;
int		rres;
int		result;

	parsestring = "+32768-32768";
	x_return = &xr;
	y_return = &yr;
	width_return = (unsigned *)&wr;
	height_return = (unsigned *)&hr;

	result = XCALL;	
	if(result !=  (rres = XValue | YValue | YNegative)) {
		report("%s() returned %d instead of %d for parsestring \"%s\".", TestName, result, rres, parsestring);
		FAIL;
	} else
		CHECK;

	if(xr != 32768) {
		report("%s() returned %d in x_return instead of %d.", TestName, xr, 32768);
		FAIL;
	} else
		CHECK;

	if(yr != -32768) {
		report("%s() returned %d in y_return instead of %d.", TestName, yr, -32768);
		FAIL;
	} else
		CHECK;

	if(wr != -13) {
		report("%s() changed the width_return argument.", TestName);
		FAIL;
	} else
		CHECK;

	if(hr != -666) {
		report("%s() changed the height_return argument.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(5);

>># COMMENT
>># Added the word "just" to this assertion.
>># Cal.
>>ASSERTION Good A
When the
.A parsestring
argument contains just width and height specifications, 
then a call to xname returns these values
in the
.A width_return
and
.A height_return
arguments respectively and the
.A x_return
and
.A y_return
arguments are not altered.
>>STRATEGY
Parse the string  "16385x33768" using xname.
Verify that the call returns WidthValue | HeightValue.
Verify that the width_return argument is set to 16385.
Verify that the height_return argument is set to 33768.
Verify that the x_return argument is unaltered.
Verify that the y_return argument is unaltered.
>>CODE
int		xr = -77;
int		yr = -33;
unsigned int		wr = 16385;
unsigned int		hr = 33768;
int		rres;
int		result;

	parsestring = "16385x33768";
	x_return = &xr;
	y_return = &yr;
	width_return = &wr;
	height_return = &hr;

	result = XCALL;	
	if(result !=  (rres = WidthValue | HeightValue)) {
		report("%s() returned %d instead of %d for parsestring \"%s\".", TestName, result, rres, parsestring);
		FAIL;
	} else
		CHECK;

	if(wr != 16385) {
		report("%s() returned %u in width_return instead of %d.", TestName, wr, 16385);
		FAIL;
	} else
		CHECK;

	if(hr != 33768) {
		report("%s() returned %u in height_return instead of %d.", TestName, hr, 33768);
		FAIL;
	} else
		CHECK;

	if(xr != -77) {
		report("%s() changed the x_return argument.", TestName);
		FAIL;
	} else
		CHECK;

	if(yr != -33) {
		report("%s() changed the y_return argument.", TestName);
		FAIL;
	} else
		CHECK;

	CHECKPASS(5);

>># COMMENT
>># Maybe need an assertion which verifies that all 4 of the parameters are set correctly.
>># Cal.
