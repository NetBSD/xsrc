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
 * $XConsortium: trnsltcrdn.m,v 1.8 94/04/17 21:03:22 rws Exp $
 */
>>TITLE XTranslateCoordinates CH03
Bool

Display	*display = Dsp;
Window	src_w;
Window	dest_w;
int	src_x = c_ap.x + 10;
int	src_y = c_ap.y + 8;
int	*dest_x_return	= &dest_x;
int	*dest_y_return	= &dest_y;
Window	*child_return	= &child;
>>EXTERN
static	struct	area	c_ap;
static	int	dest_x;
static	int	dest_y;
static	Window	child;

void transinit()
{
	tpstartup();

/* struct area of the child window */
	c_ap.x = 50;
	c_ap.y = 60;
	c_ap.width = 20;
	c_ap.height= 17;

	dest_x = -1;
	dest_y = -1;
	child  = ~0;
}

>>SET tpstartup transinit
>>ASSERTION Good A
A call to xname takes the
.A src_x
and
.A src_y
coordinates relative to the source window's origin and
converts them to be
relative to the destination window's origin and
returns these translated coordinates in 
.A dest_x_return
and
.A dest_y_return .
>># Note that I'm testing over a loop of border_widths. This is to ensure
>># the border width is taken into account in the calculation.		stuart
>>STRATEGY
For some values of border_width:
	Create a window to be the source window.
	Create a child window to be the destination window.
	Map both windows.
	Translate coordinates of the parent window to the child using xname.
	Verify the returned coordinates were as expected.
>>CODE
Bool	ret;
int	exp_x, exp_y;
int	border_width;

/* Set up child window position coordinates */

	for(border_width = 0; border_width < 5; border_width++) {
		src_w = defdraw(display, VI_WIN);
		XMapWindow(display, src_w);

	/* Child window: visual = CopyFromParent, and is mapped */
		dest_w = crechild(display, src_w, &c_ap);
		XSetWindowBorderWidth(display, dest_w, border_width);
		XSetWindowBorder(display, dest_w, W_FG);
		XMapWindow(display, dest_w);

/* Calculate expected return values */
		exp_x = src_x - c_ap.x - border_width;
		exp_y = src_y - c_ap.y - border_width;

		ret = XCALL;

		if (ret == False)
		{
			report("%s returned False when expecting True.",
				TestName);
			report("src_x=%d", src_x); delete("src_y=%d", src_y);
			report("exp_x=%d", exp_x); delete("exp_y=%d", exp_y);
			report("border_width=%d", border_width);
			FAIL;
		} else
		if ((dest_x != exp_x) || (dest_y != exp_y)) {
			report("%s did not return expected coordinates",
				TestName);
			report("Expected: (*dest_x_return)=%d, (*dest_y_return)=%d",
				exp_x, exp_y);
			report("Got: (*dest_x_return)=%d, (*dest_y_return)=%d",
				dest_x, dest_y);
			if( (dest_x == -1) || (dest_y == -1) )
				report("(*dest_x_return) and (*dest_y_return) were probably not set");

			FAIL;
		} else
			CHECK;

	}

	CHECKPASS(5);
>>ASSERTION Good C
When
.A src_w
and
.A dest_w
were not created on the same screen,
then a call to xname returns zero and 
.A dest_x_return
and
.A dest_y_return
are set to zero.
>>STRATEGY
If an alternate screen is supported:
	Create a mapped window on the default screen.
	Create a mapped window on the alternate screen.
	Verify xname returns zero.
	Verify that *dest_x_return is zero.
	Verify that *dest_y_return is zero.
>>CODE
int	ret;

        if (config.alt_screen == -1) {
                unsupported("No alternate screen supported");
                return;
        }

	src_w  = defdraw(display, VI_WIN);
	XMapWindow(display, src_w);
	dest_w = defdraw(display, VI_ALT_WIN);
	XMapWindow(display, dest_w);

	ret = XCALL;

	if (ret != 0)
	{
		report("%s did not return zero when", TestName);
		report("src_w and dest_w were on different screens.");
		report("Expecting: 0");
		report("Returned : %d", ret);
		FAIL;
	}
	else
		CHECK;

	if ( dest_x != 0  || dest_y != 0 )
	{
		report("%s did not set the dest_x and dest_y as expected.",
				TestName);
		report("Expecting: (*dest_x_return)=0, (*dest_y_return)=0");
		report("Returned : (*dest_x_return)=%d, (*dest_y_return)=%d",
			dest_x, dest_y);
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good A
When the coordinates are contained in a mapped child of
.A dest_w ,
then that child is returned in
.A child_return .
>>STRATEGY
For some values of border_width:
	Create a window to be the source and destination windows.
	Create a sub window.
	Map both windows.
	Translate coordinates of the window to the child using xname.
	Verify the child_return window was the sub window.
>>CODE
Window	exp_w;
Bool	ret;
int	border_width;

/* Set up child window position coordinates */

	for(border_width = 0; border_width < 5; border_width++) {
		src_w = defdraw(display, VI_WIN);
		XMapWindow(display, src_w);

		dest_w = src_w;

		exp_w = crechild(display, src_w, &c_ap);

		XSetWindowBorderWidth(display, exp_w, border_width); 
		XSetWindowBorder(display, exp_w, W_FG);
		XMapWindow(display, exp_w);

/* Calculate expected return values */
		ret = XCALL;

		if (ret == False)
		{
			report("%s returned False when expecting True.",
				TestName);
			report("border_width=%d", border_width);
			FAIL;
		} else
		if (child != exp_w)
		{
			report("%s did not return the expected window in (*child_return)",
				TestName);
			report("(*child_return) = %0x", child);
			report("expected window = %0x", exp_w);
			report("border_width=%d", border_width);
			FAIL;
		}
		else
			CHECK;

	}

	CHECKPASS(5);
>>ASSERTION Good A
When the coordinates are not contained in a mapped child of
.A dest_w ,
then
.A child_return
is set to
.S None .
>>STRATEGY
Create a mapped window to be the source and destination windows.
Create an overlapping mapped sibling window.
Translate coordinates of the window using xname.
Verify the child_return window was None.
Verify the returned coordinates were as expected.
>>CODE
XVisualInfo 	*vp;
XWindowAttributes	winatt;
Window	exp_w;
Bool	ret;

	src_w = defdraw(display, VI_WIN);
	XMapWindow(display, src_w);
	dest_w = src_w;

/* Get the root window relative postion of the first window */
	XGetWindowAttributes(display, src_w, &winatt);

/* Set the coordinates of the sibling window */
	c_ap.x += winatt.x;
	c_ap.y += winatt.y;

/* Make sibling window: mapped */
	resetvinf(VI_WIN); nextvinf(&vp);
	exp_w = makewinpos(display, vp,  c_ap.x, c_ap.y);

	ret = XCALL;

	if (ret == False)
	{
		report("%s returned False when expecting True.",
			TestName);
		FAIL;
	} else
	if (child != None)
	{
		report("When the source coordinates refered to a sibling window");
		report("%s did not set child_return to None",
			TestName);
		report("Window id returned: %0x", child);
		report("Source/Dest window: %0x", src_w);
		report("Sibling window    : %0x", exp_w);
		FAIL;
	}
	else
		CHECK;

/* Now test for a point on dest_w itself */
	src_x = 5;
	src_y = 5;
	child = ~0;	/* We must make sure xname actually sets this */

	ret = XCALL;

	if (ret == False)
	{
		report("%s returned False when expecting True.",
			TestName);
		FAIL;
	} else
	if (child != None)
	{
		report("When the source coordinates refered to the dest_w");
		report("%s did not set child_return to None",
			TestName);
		report("child_return      : %0x", child);
		report("Source/Dest window: %0x", src_w);
		report("Sibling window    : %0x", exp_w);
		FAIL;
	}
	else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Bad A
When a window argument does not name a valid window, then a
.S BadWindow
error occurs.
>>STRATEGY
For src_w and dest_w:
	Set the argument to a bad window.
	Verify a BadWindow error occurs.
>>CODE BadWindow
Window	goodwin;

	seterrdef();
	
	goodwin = defdraw(display, VI_WIN);

	src_w	= badwin(display);
	dest_w	= goodwin;
	XCALL;
	if (geterr() == BadWindow)
		CHECK;
	else
		FAIL;

	src_w	= goodwin;
	dest_w	= badwin(display);
	XCALL;
	if (geterr() == BadWindow)
		CHECK;
	else
		FAIL;

	CHECKPASS(2);
