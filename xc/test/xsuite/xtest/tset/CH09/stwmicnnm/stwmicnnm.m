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
 * $XConsortium: stwmicnnm.m,v 1.9 94/04/17 21:09:03 rws Exp $
 */
>>TITLE XSetWMIconName CH09
void
XSetWMName(display, w, text_prop)
Display		*display = Dsp;
Window		w = DRW(Dsp);
XTextProperty	*text_prop = &textprop;
>>EXTERN
#include	"Xatom.h"
static XTextProperty	textprop = { (unsigned char *) 0, XA_STRING, 8, (unsigned long) 0 };
>>ASSERTION Good A
A call to xname sets the
.S WM_ICON_NAME
property for the window
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
Create a window with XCreateWindow.
Set the WM_ICON_NAME property with XSetWMIconName.
Obtain the WM_ICON_NAME property using XGetTextProperty.
Verify that the property format was correct.
Verify that the property type was correct.
Verify that the propery value was correct.
Free allocated property with XFree.
>>CODE
Window	win;
char	*str1 = "Xtest test string1";
char	*str2 = "Xtest test string2";
char	*str[2];
Status	status;
char	**list_return;
int	count_return;
XTextProperty	tp, rtp;
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

	if(XGetWMIconName(display, win, &rtp) == False) {
		delete("XGetWMIconName() returned False.");
		return;
	} else
		CHECK;

	if(XGetTextProperty(display, win, &rtp, XA_WM_ICON_NAME) == False) {
		delete("XGetTextProperty() returned False.");
		return;
	} else
		CHECK;

	if(tp.encoding != rtp.encoding) {
		report("The encoding component of the XTextProperty was incorrect.");
		FAIL;
	} else
		CHECK;

	if(tp.format != rtp.format) {
		report("The format component of the XTextProperty was %d instead of %d.", rtp.format, tp.format);
		FAIL;
	} else
		CHECK;

	if(tp.nitems != rtp.nitems) {
		report("The nitems component of the XTextProperty was %lu instead of %lu.", rtp.nitems, tp.nitems);
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

	if( (strncmp(str1, list_return[0], strlen(str1)) != 0) || (strncmp(str2, list_return[1], strlen(str2)) != 0) ) {
		report("Value strings were:");
		report("\"%s\" and \"%s\"", list_return[0], list_return[1]);
		report("Instead of:");
		report("\"%s\" and \"%s\"", str1, str2);
		FAIL;
	} else
		CHECK;

	XFree((char*)tp.value);
	XFree((char*)rtp.value);
	XFreeStringList(list_return);

	CHECKPASS(9);

>>ASSERTION Bad A
.ER BadAlloc
>>ASSERTION Bad A
>># Note that 0L is used as the invalid ID. This is not known to be an acceptable strategy.
>># Cal 23/04/91.	Use -1L instead, we know the top 3 bits of a valid XID
>>#			are always 0.	Kieron, 30/04/91
When the
.M encoding
component of the
.S XTextProperty
named by the
.A text_prop
argument does not name a valid atom, then a
.S BadAtom
error occurs.
>>STRATEGY
Create a window with XCreateWindow.
Create an XTextProperty structure with XStringListToTextProperty
Set the encoding component of the structure to -1L.
Set the WM_ICON_NAME property with XSetWMIconName
Verify that a BadAtom error occurred.
>>CODE BadAtom
Window	win;
char	*str1 = "Xtest test string1";
char	*str2 = "Xtest test string2";
char	*str[2];
XTextProperty	tp;
XVisualInfo	*vp;

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

	w = win;
	tp.encoding = (Atom) -1L;
	text_prop = &tp;

	XCALL;

	if(geterr() != BadAtom) 
		FAIL;
	else
		CHECK;

	CHECKPASS(2);

>>ASSERTION Bad A
.ER BadWindow
>>ASSERTION Bad A
When the
.M format
component of the
.S XTextProperty
structure named by the
.A text_prop
argument is other than 8, 16 or 32, then a
.S BadValue
error occurs.
>>STRATEGY
Create a window with XCreateWindow.
Create a TextProperty structure with format {0, 1, 7, 15, 31}.
with XStringListToTextProperty.
Set the WM_ICON_NAME property with XSetWMIconName.
Verify that a BadValue error occurs.
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

