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
 * $XConsortium: lstdpths.m,v 1.6 94/04/17 21:02:28 rws Exp $
 */
>>TITLE XListDepths CH02
int *
XListDepths(display, screen_number, count_return)
Display	*display = Dsp;
int	screen_number = DefaultScreen(Dsp);
int	*count_return;
>>ASSERTION Good B 1
A call to xname returns an array which can be freed with
.S XFree ,
of all depths which are available on the screen
.A screen_number
and whose size is returned through the
.A count_return
argument.
>>STRATEGY
Obtain the list of supported depths using xname.
For each number in the list:
  Create a pixmap of that depth.
  Verify that no error occurred.
For numbers not in the list of depths:
  Create a pixmap of the depth.
  Verify that it was not successful.
Free the list using XFree.
>>CODE
int	 	number;
int		*dlst;
int		*dptr;
int		i;
XVisualInfo	vp;
Pixmap	pixm;
long	notmem[NM_LEN];

	count_return = &number;
	dptr = dlst = XCALL;
	for(; dptr < &dlst[number] ; dptr++) {
		trace("Depth %d", *dptr);
		startcall(display);
		if (isdeleted())
			return;
		pixm = XCreatePixmap(display, DRW(display), 1 , 1, *dptr);
		endcall(display);
		if(geterr() != Success) {
			report("Got %s, Expecting Success", errorname(geterr()));
			FAIL;
		} else {
			CHECK;
			XFreePixmap(display, pixm);
		}
	}	

	if(notmember(dlst, number, notmem) == 0) {
		delete("notmember failed.");
		return;
	} else
		CHECK;

	for (i = 0; i < NM_LEN; i++) {
		startcall(display);
		if (isdeleted())
			return;
		/* notmem is a long, and is being cast to an int */
		pixm = XCreatePixmap(display, DRW(display), 1 , 1, (int)notmem[i]);
		endcall(display);
		if(geterr() == Success) {
			XFreePixmap(display, pixm);
			report("Got Success unexpectedly for depth %d.", (int)notmem[i]);
			FAIL;
		} else
			CHECK;
	}	

	XFree((char*)dlst);
	CHECKPASS(1+number+NM_LEN);

>>ASSERTION Good A
When
.A screen_number
is not a valid screen, then a call to xname
does not set the
.A count_return
argument and returns NULL.
>>STRATEGY
List the depths of an invalid screen using XListDepths.
Verify that the function returns NULL.
Verify that the count_return argument was unchanged.
>>CODE
int	number = -1;
int	*dlst;

	count_return = &number;
	screen_number = -1;
	dlst = XCALL;

	if(dlst != (int *) NULL) {
		report("%s() did not return NULL.", TestName);
		FAIL;
	} else
		CHECK;

	if( number != -1) {
		report("The count_return argument was modified.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(2);


>>ASSERTION Bad B 1
When sufficient memory cannot be allocated for the
returned array, then a call to xname
does not set the
.A count_return
argument and returns NULL.
