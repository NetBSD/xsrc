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
 * $XConsortium: frstrlst.m,v 1.7 94/04/17 21:08:31 rws Exp $
 */
>>TITLE XFreeStringList CH09
void
XFreeStringList(list)
char	**list = (char **) NULL;
>>ASSERTION Good A
A call to xname frees the memory allocated by a call to
.S XTextPropertyToStringList
or
.S XGetCommand . 
>>STRATEGY
Create a window using XCreateWindow.
Allocate a text property structure using XStringListToTextProperty.
Set the WM_COMMAND property using XSetCommand.
Obtain the value of the WM_COMMAND property using XGetCommand.
Obtain the strings from the XTextPropertyStructure using XTextPropertyToStringlist.
Release the memory allocated in the call to XGetCommand.
Release the memory allocated in the call to XTextPropertyToStringList.
>>CODE
char		*str1 = "TestString1";
char		*str2 = "TestString2";
char		*str3 = "TestString3";
int		argc = 3;
char		*argv[3];
int		rargc;
int		rargc1;
char		**rargv = (char **) NULL;
char		**rargv1 = (char **) NULL;
Window		w;
XVisualInfo	*vp;
XTextProperty	tp;

	argv[0] = str1;
	argv[1] = str2;
	argv[2] = str3;


	if( XStringListToTextProperty(argv, 3, &tp) == 0) {
		delete("XStringListToTextProperty() returned zero.");
		return;
	} else
		CHECK;

	if( XTextPropertyToStringList(&tp, &rargv, &rargc) == 0) {
		delete("XTextPropertyToStringList() returned zero.");
		return;
	} else
		CHECK;

	XFree((char*)tp.value);
	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(Dsp, vp);

	XSetCommand(Dsp, w, argv, argc);

	if(XGetCommand(Dsp, w, &rargv1, &rargc) == 0 ) {
		delete("XGetCommand() returned zero.");
		return;
	} else
		CHECK;

	XFreeStringList(rargv1);
	XFreeStringList(rargv);

	CHECKPASS(3);
