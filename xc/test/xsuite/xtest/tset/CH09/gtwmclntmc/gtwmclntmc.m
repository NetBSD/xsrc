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
 * $XConsortium: gtwmclntmc.m,v 1.9 94/04/17 21:08:44 rws Exp $
 */
>>TITLE XGetWMClientMachine CH09
Status
XGetWMClientMachine(display, w, text_prop_return)
Display		*display = Dsp;
Window		w = DRW(Dsp);
XTextProperty	*text_prop_return = &textprop;
>>EXTERN
#include	"Xatom.h"
static XTextProperty	textprop = { (unsigned char *) 0, XA_STRING, 8, (unsigned long) 0 };
>>ASSERTION Good A
When the WM_CLIENT_MACHINE property
is set on the window 
.A w ,
then a call to xname stores
the data, which can be freed with XFree, in the
.M value
field, the type of the data in the
.M encoding
field, the format of the data in the
.M format
field, and the number of items of data in the
.M nitems
field of the 
.S XTextProperty 
structure named by the
.A text_prop_return
argument and returns non-zero.
>>STRATEGY
Create a window with XCreateWindow.
Set the property WM_CLIENT_MACHINE for the window with XSetWMClientMachine.
Obtain the value of the WM_CLIENT_MACHINE property with XGetWMClientMachine.
Verify that the function returned non-zero.
Verify that the encoding, format, value and nitems fields of the returned structure are correct.
Release the allocated memory using XFree.
>>CODE
Window	win;
char	*str1 = "Xtest Client Machine String";
char	*str[1];
Status	status;
char	**list_return;
int	count_return;
XTextProperty	tp, rtp;
XVisualInfo	*vp;

	str[0] = str1;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	if(XStringListToTextProperty(str, 1, &tp) == False) {
		delete("XStringListToTextProperty() Failed.");
		return;
	} else
		CHECK;

	XSetWMClientMachine(display, win, &tp);

	text_prop_return = &rtp;
	w = win;
	status = XCALL;

	if(status == False) {
		report("%s() returned False.", TestName);
		FAIL;
	} else
		CHECK;

	if(tp.encoding != rtp.encoding) {
		report("The encoding component of the XTextProperty was incorrect.");
		FAIL;
	} else
		CHECK;

	if(tp.format != rtp.format) {
		report("The format component of the XTextProperty was incorrect.");
		FAIL;
	} else
		CHECK;

	if(tp.nitems != rtp.nitems) {
		report("The nitems component of the XTextProperty was incorrect.");
		FAIL;
	} else
		CHECK;

	if(XTextPropertyToStringList( &rtp, &list_return, &count_return) == False) {
		delete("XTextPropertyToStringList() returned False.");
		return;
	} else
		CHECK;

	if (count_return != 1) {
		delete("XTextPropertyToStringList() count_return was %d instead of 1.", count_return);
		return;
	} else
		CHECK;

	if( strcmp(str1, list_return[0] ) != 0 ) {
		report("Value string was \"%s\" instead of \"%s\"", list_return[0], str1);
		FAIL;
	} else
		CHECK;

	XFree((char*)tp.value);
	XFreeStringList(list_return);

	CHECKPASS(8);

>>ASSERTION Good A
When the WM_CLIENT_MACHINE property
is not set on the window
.A w ,
then a call to xname sets the 
.M value
field to NULL, the 
.M encoding
field to None, the
.M format
field to 0 and the 
.M nitems
field to 0 of the
.S XTextProperty
structure named by the
.A text_prop_return
argument and returns zero.
>>STRATEGY
Create a window with XCreateWindow.
Get the value of the unset property WM_CLIENT_MACHINE with XGetWMClientMachine.
Verify that the call returned False.
Verify that in the returned XTextProperty structure the encoding field was none,
  the format component was 0, the nitems component was 0 and the
  value component was NULL.
>>CODE
Window	win;
Status	status;
XTextProperty	tp;
XVisualInfo	*vp;

	tp.value = (unsigned char *) "XTuninitialised\0";
	tp.encoding = XA_STRING;
	tp.format = 8;
	tp.nitems = 1 + strlen(tp.value);

	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(display, vp);

	text_prop_return = &tp;
	w = win;
	status = XCALL;

	if(status != False) {
		report("%s() did not return False.", TestName);
		FAIL;
	} else
		CHECK;

	if(tp.encoding != None) {
		report("The encoding component was not None.");
		FAIL;
	} else
		CHECK;
	
	if(tp.format != 0) {
		report("The format component was %d instead of 0", tp.format);
		FAIL;
	} else
		CHECK;

	if(tp.nitems != 0) {
		report("The nitems component was %lu instead of 0", tp.nitems);
		FAIL;
	} else
		CHECK;

	if(tp.value != (unsigned char *) NULL) {
		report("The value component was not NULL."); 
		FAIL;
	} else
		CHECK;

	CHECKPASS(5);

>>#
>># I cannot see how a BadAtom Can be generated.
>># Cal 28/05/91
>>#>>ASSERTION
>>#.ER BadAtom 
>>#
>>#
>>ASSERTION Bad A
.ER BadWindow
>># Kieron	Action	Review
