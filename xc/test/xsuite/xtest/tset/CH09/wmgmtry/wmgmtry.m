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

 * Copyright 1990, 1991 and UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of nd UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission. UniSoft
 * makes no representations about the suitability of this software for any
 * purpose. It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: wmgmtry.m,v 1.6 94/04/17 21:09:15 rws Exp $
 */

>>TITLE XWMGeometry CH09
int
XWMGeometry(display, screen, user_geom, def_geom, bwidth, hints, x_return, y_return, width_return, height_return, gravity_return);
Display		*display = Dsp;
int		screen = DefaultScreen(Dsp);
char		*user_geom = NULL;
char		*def_geom = NULL;
unsigned int	bwidth = 1;
XSizeHints	*hints = &size_hints;
int		*x_return = &dint;
int		*y_return = &dint;
int		*width_return = &dint;
int		*height_return = &dint;
int		*gravity_return = &dint;
>>EXTERN
#include	"Xatom.h"
int		dint;
XSizeHints	size_hints;

XSizeHints	szhints = {	0L,
				0,0,     
				0,0,    
				38,	29,    	/* min width, min height */
				400,	300,	/* max width, max height */
				17,	18,  	/* width inc, height inc */
				{0,0},  
				{0,0},  
				20,	10,	/* base width, base height */
				SouthWestGravity};

XSizeHints	szhints1 = { PAllHints, /* Deliberate lack of PBaseSize and PWinGravity */
				0,0,     
				0,0,    
				20,	30,    	/* min width, min height */
				400,	300,	/* max width, max height */
				17,	18,  	/* width inc, height inc */
				{0,0},  
				{0,0},  
				-1,	-2,	/* base width, base height */
				-1};

XSizeHints	szhints2 = {	0L,
				0,0,     
				0,0,    
				0,0,
				0,0,
				0,0,
				{0,0},  
				{0,0},  
				0,0,
				0};


>># NOTE
>># Having consulted the ICCCM v1.0 page 21 I have changed this
>># assertion to mention that :
>>#      returned width = hints->base_width + width*hints->width_inc
>>#      returned height = hints->base_height + height*hints->height_inc
>>#
>># Cal 23/5/91
>>#
>>ASSERTION Good A
A call to xname combines the geometry information given in the
.A user_geom ,
.A def_geom
and
.A hints
arguments and returns in
.A x_return
the x offset, in
.A y_return
the y offset, in
.A width_return
the width ( = hints->base_width + width*hints->width_inc)
, in
.A height_return
the height ( = hints->base_height + height*hints->height_inc)
and in
.A gravity_return
the gravity information specified by the geometry arguments.
>>STRATEGY
Call XWMGeometry with user_geom "10x12+30+40", def_geom = NULL,
    and an XSizeHints structure with min_width = 38, min_heght = 29,
    max_width = 400, max_height = 300, width_inc = 17, height_inc =18,
    base_width = 20 and base_height = 10.
Verify that the returned width is base_width  + (10 * width_inc)
Verify that the returned height is base_height + (12 * height_inc)
Verify that the returned x-coordinate is 30.
Verify that the returned y-coordinate is 40.

Call XWMGeometry with user_geom NULL, def_geom = "10x12+30+40"
    and an XSizeHints structure with min_width = 38, min_heght = 29,
    max_width = 400, max_height = 300, width_inc = 17, height_inc =18,
    base_width = 20 and base_height = 10.
Verify that the returned width is base_width  + (10 * width_inc)
Verify that the returned height is base_height + (12 * height_inc)
Verify that the returned x-coordinate is 30.
Verify that the returned y-coordinate is 40.

Call XWMGeometry with user_geom = "10x12", def_geom = "2x3+30+40"
    and an XSizeHints structure with min_width = 38, min_heght = 29,
    max_width = 400, max_height = 300, width_inc = 17, height_inc =18,
    base_width = 20 and base_height = 10.
Verify that the returned width is base_width  + (10 * width_inc)
Verify that the returned height is base_height + (12 * height_inc)
Verify that the returned x-coordinate is 30.
Verify that the returned y-coordinate is 40.

Call XWMGeometry with user_geom = "1x1", def_geom = NULL
    and an XSizeHints structure with min_width = 38, min_heght = 29,
    max_width = 400, max_height = 300, width_inc = 17, height_inc =18,
    base_width = 20 and base_height = 10.
Verify that the returned width is min_width
Verify that the returned height is min_height 

Call XWMGeometry with user_geom = "40x30", def_geom = NULL
    and an XSizeHints structure with min_width = 38, min_heght = 29,
    max_width = 400, max_height = 300, width_inc = 17, height_inc =18,
    base_width = 20 and base_height = 10.
Verify that the returned width is max_width
Verify that the returned height is max_height 

Call XWMGeometry with user_geom = "10x12-1-2", def_geom = NULL
    and an XSizeHints structure with min_width = 38, min_heght = 29,
    max_width = 400, max_height = 300, width_inc = 17, height_inc =18,
    base_width = 20 and base_height = 10.
Verify that the returned x-coordinate is DisplayWidth -1 - (base_width + 10 * width_inc) - 2 * bwidth
Verify that the returned y-coordinate is DisplayHeight -1 - (base_height + 10 * height_inc) - 2 * bwidth

>>CODE
int	rval;
int	xr, yr, wr, hr, gr;
int	x,  y,  w,  h,  g;

	bwidth = 10;
	szhints.flags= PAllHints|PBaseSize|PWinGravity;
	hints = &szhints;
	x_return = &xr;
	y_return = &yr;
	width_return = &wr;
	height_return = &hr;
	gravity_return = &gr;

	user_geom = "10x12+30+40";
	def_geom = NULL;
	rval = XCALL;

	w = szhints.base_width  + (10 * szhints.width_inc);
	h = szhints.base_height + (12 * szhints.height_inc);
	x = 30;
	y = 40;

	trace("user_geom = %s, def_geom = %s", user_geom == NULL ? "<NULL>": user_geom, def_geom == NULL ? "<NULL>": def_geom);

	if(w != wr) {
		report("Returned width was %d instead of %d", wr, w);
		FAIL;
	} else
		CHECK;

	if(h != hr) {
		report("Returned height was %d instead of %d", hr, h);
		FAIL;
	} else
		CHECK;

	if(x != xr) {
		report("Returned x-coordinate was %d instead of %d", xr, x);
		FAIL;
	} else
		CHECK;

	if(y != yr) {
		report("Returned y-coordinate was %d instead of %d", yr, y);
		FAIL;
	} else
		CHECK;

	user_geom = NULL;
	def_geom =  "10x12+30+40";
	rval = XCALL;

	w = szhints.base_width  + (10 * szhints.width_inc);
	h = szhints.base_height + (12 * szhints.height_inc);

	trace("user_geom = %s, def_geom = %s", user_geom == NULL ? "<NULL>": user_geom, def_geom == NULL ? "<NULL>": def_geom);

	if(w != wr) {
		report("Returned width was %d instead of %d", wr, w);
		FAIL;
	} else
		CHECK;

	if(h != hr) {
		report("Returned height was %d instead of %d", hr, h);
		FAIL;
	} else
		CHECK;

	if(x != xr) {
		report("Returned x-coordinate was %d instead of %d", xr, x);
		FAIL;
	} else
		CHECK;

	if(y != yr) {
		report("Returned y-coordinate was %d instead of %d", yr, y);
		FAIL;
	} else
		CHECK;

/* When no x,y is specified by user_geom, use the def_geom */
	user_geom = "10x12";
	def_geom = "2x3+30+40";
	rval = XCALL;

	w = szhints.base_width  + (10 * szhints.width_inc);
	h = szhints.base_height + (12 * szhints.height_inc);

	trace("user_geom = %s, def_geom = %s", user_geom == NULL ? "<NULL>": user_geom, def_geom == NULL ? "<NULL>": def_geom);

	if(w != wr) {
		report("Returned width was %d instead of %d", wr, w);
		FAIL;
	} else
		CHECK;

	if(h != hr) {
		report("Returned height was %d instead of %d", hr, h);
		FAIL;
	} else
		CHECK;

	if(x != xr) {
		report("Returned x-coordinate was %d instead of %d", xr, x);
		FAIL;
	} else
		CHECK;

	if(y != yr) {
		report("Returned y-coordinate was %d instead of %d", yr, y);
		FAIL;
	} else
		CHECK;

/* Ensure that the minimums hold */
	user_geom = "1x1";
	def_geom = NULL;
	rval = XCALL;

	w = szhints.min_width;
	h = szhints.min_height;

	trace("user_geom = %s, def_geom = %s", user_geom == NULL ? "<NULL>": user_geom, def_geom == NULL ? "<NULL>": def_geom);

	if(w != wr) {
		report("Returned width was %d instead of %d", wr, w);
		FAIL;
	} else
		CHECK;

	if(h != hr) {
		report("Returned height was %d instead of %d", hr, h);
		FAIL;
	} else
		CHECK;

/* Ensure that the maximums hold */
	user_geom = "40x30";
	def_geom = NULL;
	rval = XCALL;

	w = szhints.max_width;
	h = szhints.max_height;

	trace("user_geom = %s, def_geom = %s", user_geom == NULL ? "<NULL>": user_geom, def_geom == NULL ? "<NULL>": def_geom);

	if(w != wr) {
		report("Returned width was %d instead of %d", wr, w);
		FAIL;
	} else
		CHECK;

	if(h != hr) {
		report("Returned height was %d instead of %d", hr, h);
		FAIL;
	} else
		CHECK;

/* Check that border, width and height is taken into account with negative x & y */
	user_geom = "10x12-1-2";
	def_geom = NULL;
	rval = XCALL;

	w = szhints.base_width  + (10 * szhints.width_inc);
	h = szhints.base_height + (12 * szhints.height_inc);
	x = DisplayWidth(display, screen) + (-1) - w - 2 * bwidth;
	y = DisplayHeight(display, screen) + (-2) - h - 2 * bwidth;


	trace("user_geom = %s, def_geom = %s", user_geom == NULL ? "<NULL>": user_geom, def_geom == NULL ? "<NULL>": def_geom);

	if(x != xr) {
		report("Returned x-coordinate was %d instead of %d", xr, x);
		FAIL;
	} else
		CHECK;

	if(y != yr) {
		report("Returned y-coordinate was %d instead of %d", yr, y);
		FAIL;
	} else
		CHECK;

	CHECKPASS(18);

>>ASSERTION Good A
The value returned in the
.A gravity_return
information is one of 
.S NorthWestGravity ,
.S NorthGravity ,
.S NorthEastGravity ,
.S WestGravity ,
.S CenterGravity ,
.S EastGravity ,
.S SouthWestGravity ,
.S SouthGravity ,
or
.S SouthEastGravity .
>>STRATEGY
Call XWMGeometry with user_geom = "10x20-0-0", and an XSizeHints
  structure with gravity component set to SouthWestGravity.
Verify that the gravity returned is one of NorthWestGravity,
  NorthGravity , NorthEastGravity , WestGravity ,
  CenterGravity ,EastGravity , SouthWestGravity ,
  SouthGravity , SouthEastGravity .
>>CODE
int	rval;
int	xr, yr, wr, hr, gr;

	user_geom = "10x20-0-0";
	def_geom = NULL;
	bwidth = 1;
	szhints.flags= PAllHints|PBaseSize|PWinGravity;
	hints = &szhints;
	x_return = &xr;
	y_return = &yr;
	width_return = &wr;
	height_return = &hr;
	gravity_return = &gr;
	rval = XCALL;

	trace("user_geom = %s, def_geom = %s", user_geom == NULL ? "<NULL>": user_geom, def_geom == NULL ? "<NULL>": def_geom);

	if( (gr != NorthWestGravity) && (gr != NorthGravity) &&(gr != NorthEastGravity) &&
	    (gr != WestGravity) && (gr != CenterGravity) && (gr != EastGravity) &&
	    (gr != SouthWestGravity) && (gr != SouthGravity) && (gr != SouthEastGravity)) {
		report("Gravity value returned was invalid : %d", gr);
		FAIL;
	} else
		PASS;

>>ASSERTION Good A
A call to  xname  returns a  mask which  indicates
which information came from the
.A user_geom
argument and whether the
position is relative to the right and bottom edges which  is the OR of
none or any of
.S XValue ,
.S YValue ,
.S WidthValue ,
.S HeightValue ,
.S XNegative ,
or
.S YNegative .
>>STRATEGY
Call XWMGeometry with user_geom = "40x30-9-9"
Verify that the returned mask is (XValue | YValue | WidthValue |HeightValue | XNegative | YNegative)
Call XWMGeometry with user_geom = "-9+9"
Verify that the returned mask is (XValue | YValue | XNegative)
Call XWMGeometry with user_geom = "5x5"
Verify that the returned mask is (WidthValue | HeightValue)
Call XWMGeometry with user_geom = NULL
Verify that the returned mask is 0.
>>CODE
int	rval;
int	xr, yr, wr, hr, gr;

	user_geom = "40x30-9-9";
	def_geom = NULL;
	bwidth = 1;
	szhints.flags= PAllHints|PBaseSize|PWinGravity;
	hints = &szhints;
	x_return = &xr;
	y_return = &yr;
	width_return = &wr;
	height_return = &hr;
	gravity_return = &gr;
	rval = XCALL;

	if(rval != (XValue | YValue | WidthValue |HeightValue | XNegative | YNegative)) {
		report("Return value was 0x%x, instead of", rval);
		report("(XValue | YValue | XWidthValue | XHeightValue | XNegative | YNegative)");
		report("with user_geom = %s", user_geom);
		FAIL;
	} else
		CHECK;

	user_geom = NULL;
	rval = XCALL;

	if(rval != NoValue) {
		report("Return value was 0x%x, instead of NoValue with user_geom = %s", rval, user_geom);
		FAIL;
	} else
		CHECK;

	user_geom = "-9+9";
	rval = XCALL;

	if(rval != ( XValue | YValue | XNegative)) {
		report("Return value was 0x%x, instead of", rval);
		report("(XWidthValue | XHeightValue | XNegative)");
		report("with user_geom = %s", user_geom);
		FAIL;
	} else
		CHECK;

	user_geom = "5x5";
	rval = XCALL;

	if(rval != (WidthValue | HeightValue)) {
		report("Return value was 0x%x, instead of", rval);
		report("(XValue | YValue)");
		report("with user_geom = %s", user_geom);
		FAIL;
	} else
		CHECK;

	user_geom = NULL;
	rval = XCALL;

	if(rval != 0) {
		report("Return value was 0x%x, instead of 0", rval);
		report("with user_geom = %s", user_geom);
		FAIL;
	} else
		CHECK;

	CHECKPASS(5);
>>ASSERTION Good A
When the
.M base_width
component of the 
.S XSizeHints 
structure named by the
.A hints
argument
is not set, then the
.M min_width
component is used.
>>STRATEGY
Call XWMGeometry with user_geom = "10x12", def_geom = NULL and an XSizeHints
  structure with flags component = PAllHints , base_width = -1 base_height = -2,
  min_width = 20 and min_height = 30.
Verify that the returned width is min_width  + (10 * width_inc)
>>CODE
int	rval;
int	xr, yr, wr, hr, gr;
int	w;

	bwidth = 10;
	hints = &szhints1;
	x_return = &xr;
	y_return = &yr;
	width_return = &wr;
	height_return = &hr;
	gravity_return = &gr;

	user_geom = "10x12";
	def_geom = NULL;
	rval = XCALL;

	w = szhints1.min_width  + (10 * szhints1.width_inc);

	trace("user_geom = %s, def_geom = %s", user_geom == NULL ? "<NULL>": user_geom, def_geom == NULL ? "<NULL>": def_geom);

	if(w != wr) {
		report("Returned width was %d instead of %d", wr, w);
		FAIL;
	} else
		PASS;

>>ASSERTION Good A
When the
.M base_height
component of the 
.S XSizeHints 
structure named by the
.A hints
argument
is not set, then the
.M min_height
component is used.
>>STRATEGY
Call XWMGeometry with user_geom = "10x12", def_geom = NULL and an XSizeHints
  structure with flags component = PAllHints , base_width = -1 base_height = -2,
  min_width = 20 and min_height = 30.
Verify that the returned height is min_height  + (10 * height_inc)
>>CODE
int	rval;
int	xr, yr, wr, hr, gr;
int	h;

	bwidth = 10;
	hints = &szhints1;
	x_return = &xr;
	y_return = &yr;
	width_return = &wr;
	height_return = &hr;
	gravity_return = &gr;

	user_geom = "10x12";
	def_geom = NULL;
	rval = XCALL;

	h = szhints1.min_height  + (12 * szhints1.height_inc);

	trace("user_geom = %s, def_geom = %s", user_geom == NULL ? "<NULL>": user_geom, def_geom == NULL ? "<NULL>": def_geom);

	if(h != hr) {
		report("Returned height was %d instead of %d", hr, h);
		FAIL;
	} else
		PASS;

>>ASSERTION Good A
When the
.M min_width
component is not set in the
.S XSizeHints
structure named by the
.A hints
argument, then the
.M base_width
component is used.
>>STRATEGY
Call XWMGeometry with user_geom = "1x1", def_geom = NULL and an XSizeHints
  structure with flags component = PBaseSize | PResizeInc , min_width = 21,
  min_height = 31, width_inc = -1, height_inc = -1, base_height = 20 and
  base_width = 30.
Verify that the returned width is base_width.
>>CODE
int	rval;
int	xr, yr, wr, hr, gr;
int	w;

	bwidth = 10;

	szhints2.flags = PBaseSize | PResizeInc;
	szhints2.min_width = 21;
	szhints2.min_height = 31;
	szhints2.width_inc = -1;
	szhints2.height_inc = -1;
	szhints2.base_height = 20;
	szhints2.base_width = 30;

	hints = &szhints2;
	x_return = &xr;
	y_return = &yr;
	width_return = &wr;
	height_return = &hr;
	gravity_return = &gr;

	user_geom = "1x1";
	def_geom = NULL;
	rval = XCALL;

	w = szhints2.base_width;

	trace("user_geom = %s, def_geom = %s", user_geom == NULL ? "<NULL>": user_geom, def_geom == NULL ? "<NULL>": def_geom);

	if(w != wr) {
		report("Returned width was %d instead of %d", wr, w);
		FAIL;
	} else
		PASS;

>>ASSERTION Good A
When the
.M min_height
component is not set in the
.S XSizeHints
structure named by the
.A hints
argument, then the
.M base_height
component is used.
>>STRATEGY
Call XWMGeometry with user_geom = "1x1", def_geom = NULL and an XSizeHints
  structure with flags component = PBaseSize | PResizeInc , min_width = 21,
  min_height = 31, width_inc = -1, height_inc = -1, base_height = 20 and
  base_width = 30.
Verify that the returned height is base_height.
>>CODE
int	rval;
int	xr, yr, wr, hr, gr;
int	h;

	bwidth = 10;

	szhints2.flags = PBaseSize | PResizeInc;
	szhints2.min_width = 21;
	szhints2.min_height = 31;
	szhints2.width_inc = -1;
	szhints2.height_inc = -1;
	szhints2.base_height = 20;
	szhints2.base_width = 30;

	hints = &szhints2;
	x_return = &xr;
	y_return = &yr;
	width_return = &wr;
	height_return = &hr;
	gravity_return = &gr;

	user_geom = "1x1";
	def_geom = NULL;
	rval = XCALL;

	h = szhints2.base_height;

	trace("user_geom = %s, def_geom = %s", user_geom == NULL ? "<NULL>": user_geom, def_geom == NULL ? "<NULL>": def_geom);

	if(h != hr) {
		report("Returned height was %d instead of %d", hr, h);
		FAIL;
	} else
		PASS;


>>ASSERTION Good A
When neither the
.M base_width
nor
.M min_width
components of the
.S XSizeHints
structure named by the
.A hints
argument is set, then the value 0 is used.
>>STRATEGY
Call XWMGeometry with user_geom = "1x1", def_geom = NULL and an XSizeHints
  structure with flags component = PResizeInc , min_width = 21,
  min_height = 31, width_inc = -1, height_inc = -1, base_height = 20 and
  base_width = 30.
Verify that the returned width is 0.
>>CODE
int	rval;
int	xr, yr, wr, hr, gr;

	bwidth = 10;

	szhints2.flags = PResizeInc;
	szhints2.min_width = 21;
	szhints2.min_height = 31;
	szhints2.width_inc = -1;
	szhints2.height_inc = -1;
	szhints2.base_height = 20;
	szhints2.base_width = 30;

	hints = &szhints2;
	x_return = &xr;
	y_return = &yr;
	width_return = &wr;
	height_return = &hr;
	gravity_return = &gr;

	user_geom = "1x1";
	def_geom = NULL;
	rval = XCALL;

	trace("user_geom = %s, def_geom = %s", user_geom == NULL ? "<NULL>": user_geom, def_geom == NULL ? "<NULL>": def_geom);

	if(wr != 0) {
		report("Returned width was %d instead of 0", wr);
		FAIL;
	} else
		PASS;

>>ASSERTION Good A
When neither the
.M base_height
nor
.M min_height
components of the
.S XSizeHints
structure named by the
.A hints
argument is set, then the value 0 is used.
>>STRATEGY
Call XWMGeometry with user_geom = "1x1", def_geom = NULL and an XSizeHints
  structure with flags component = PResizeInc , min_width = 21,
  min_height = 31, width_inc = -1, height_inc = -1, base_height = 20 and
  base_width = 30.
Verify that the returned height is 0.
>>CODE
int	rval;
int	xr, yr, wr, hr, gr;

	bwidth = 10;

	szhints2.flags = PResizeInc;
	szhints2.min_width = 21;
	szhints2.min_height = 31;
	szhints2.width_inc = -1;
	szhints2.height_inc = -1;
	szhints2.base_height = 20;
	szhints2.base_width = 30;

	hints = &szhints2;
	x_return = &xr;
	y_return = &yr;
	width_return = &wr;
	height_return = &hr;
	gravity_return = &gr;

	user_geom = "1x1";
	def_geom = NULL;
	rval = XCALL;

	trace("user_geom = %s, def_geom = %s", user_geom == NULL ? "<NULL>": user_geom, def_geom == NULL ? "<NULL>": def_geom);

	if(hr != 0) {
		report("Returned height was %d instead of 0", hr);
		FAIL;
	} else
		PASS;

>># Kieron	Completed	Review
