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
 * $XConsortium: sttxtprprt.m,v 1.10 94/04/17 21:09:00 rws Exp $
 */
>>TITLE XSetTextProperty CH09
void
XSetTextProperty(display, w, text_prop, property)
Display		*display = Dsp;
Window		w = DRW(Dsp);
XTextProperty	*text_prop = &textprop;
Atom		property = XA_WM_NAME;
>>EXTERN
#include	"Xatom.h"
static XTextProperty	textprop = { (unsigned char *) 0, XA_STRING, 8, (unsigned long) 0 };
>>ASSERTION Good A
A call to xname sets the property, specified by
the
.A property
argument, for the window
.A w
to be of data, type, format and number of items as specified by the
.M value
field, the
.M encoding
field, the
.M format
field, and the
.M nitems
field of the
.S XTextProperty 
structure named by the
.A text_prop
argument.
>>STRATEGY
Create a window using XCreateWindow
Set the property WM_NAME with XSetTextProperty
Verify that the property values were all set correctly with XGetTextProperty
>>CODE
Window	win;
char	*str[2];
char	*str1 = "Xtest test string1";
char	*str2 = "Xtest test string2";
int	nitems;
Status	status;
char	**list_return;
int	count_return;
XTextProperty	rtp, tp;
XVisualInfo	*vp;

	str[0] = str1;
	str[1] = str2;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	if(XStringListToTextProperty(str, 2, &tp) == False) {
		delete("XStringListToTextProperty() Failed.");
		return;
	} else
		CHECK;

	w = win;
	text_prop = &tp;
	XCALL;

	w = win;
	status = XGetTextProperty(display, w, &rtp, property);

	if (status == False) {
		delete("Could not retrieve the WM_NAME property with XGetTextProperty.");
		return;
	} else
		CHECK;

	if(rtp.encoding != XA_STRING) {
		report("The WM_NAME property was not of type STRING");
		FAIL;
	} else
		CHECK;

	if(rtp.format != 8) {
		report("The WM_NAME property format was %d instead of 8", rtp.format);
		FAIL;
	} else
		CHECK;

	nitems = strlen(str1) + strlen(str2) + 2 - 1;
	if(rtp.nitems != nitems) {
		report("The nitems component was %lu instead of %d", rtp.nitems, nitems);
		FAIL;
	} else
		CHECK;

	if(XTextPropertyToStringList( &rtp, &list_return, &count_return) == False) {
		delete("XTextPropertyToStringList() returned False.");
		return;
	} else
		CHECK;

	if (count_return != 2) {
		delete("XTextPropertyToStringList() count_return was %d instead of 2.", count_return);
		return;
	} else
		CHECK;

	if( (strcmp(str1, list_return[0]) != 0) || (strcmp(str2, list_return[1]) != 0) ) {
		report("Value strings were:");
		report("\"%s\" and \"%s\"", list_return[0], list_return[1]);
		report("Instead of:");
		report("\"%s\" and \"%s\"", str1, str2);
		FAIL;
	} else
		CHECK;

	XFree((char*)rtp.value);
	XFreeStringList(list_return);
	CHECKPASS(8);


>>ASSERTION Bad B 1
.ER BadAlloc
>>ASSERTION Bad A
.ER BadAtom
>>ASSERTION Bad A
.ER BadWindow
>>ASSERTION Bad A
When the
.M format
component of the
.S XTextProperty
structure named by the
.A text_prop
argument is other than 8, 16 or 32 then a
.S BadValue
error occurs.
>>STRATEGY
Create a window with XCreateWindow.
Create a TextPropertyStructure with format component set to {0,1,7,15,31}.
Set the WM_NAME property with XSetTextProperty.
Verify that a BadValue error occurs each time.
>>EXTERN
static int bad_ones[] = {0, 1, 7, 15, 31};
>>CODE BadValue
Window	win;
char	*str1 = "Xtest test string1";
char	*str2 = "Xtest test string2";
char	*str[2];
XTextProperty	tp;
XVisualInfo	*vp;
int	i;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	str[0] = str1;
	str[1] = str2;

	if(XStringListToTextProperty(str, 2, &tp) == False) {
		delete("XStringListToTextProperty() Failed.");
		return;
	} else
		CHECK;

	for(i=0; i < NELEM(bad_ones); i++) {
		tp.format = bad_ones[i];
		w = win;
		text_prop = &tp;
		XCALL;

		if(geterr() != BadValue) 
			FAIL;
		else
			CHECK;
	}

	CHECKPASS(NELEM(bad_ones) + 1);


>># What if the encoding or nitems fields are garbage? Servers crash?
>># 	Yep, seem to.	-	kieron
>># Completed	Kieron		Review
