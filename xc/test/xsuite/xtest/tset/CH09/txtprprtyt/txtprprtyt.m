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
 * $XConsortium: txtprprtyt.m,v 1.9 94/04/17 21:09:14 rws Exp $
 */
>>TITLE XTextPropertyToStringList CH09
Status
XTextPropertyToStringList(text_prop, list_return, count_return)
XTextProperty	*text_prop;
char		***list_return;
int		*count_return;
>>EXTERN
#include	"Xatom.h"
char		*nullstr = "<NULL>";
char		*str1 = "Xtest_._.";
char		*str2 = "test._";
char		*str3 = "string._..";
>>ASSERTION Good A
When the
.M encoding
component of the
.A text_prop
argument is STRING and the
.M format
component is 8, then
a call to xname returns in the
.A list_string
argument the list of strings representing the null-separated
elements of the
.S XTextProperty 
structure named by the
.A text_prop
argument, returns in
.A count_return
the number of strings and returns non-zero.
>>STRATEGY
Create an XTextProperty structure.
Extract the strings using XTextPropertyToStringList.
Verify that the call did not return zero.
Verify that the returned string count is correct.
Verify that the returned string list is correct.
>>CODE
Status		status;
XTextProperty	tp;
char		**list;
int		count;

	tp.value = (unsigned char *) "Xtest_._.\0test._\0string._..";
	tp.nitems = 27;
	tp.format = 8;
	tp.encoding = XA_STRING;
	text_prop = &tp;
	list_return = &list;
	count_return = &count;
	status = XCALL;

	if(status == 0) {
		delete("%s() returned zero.", TestName);
		return;
	} else
		CHECK;

	if(count != 3) {
		report("The returned number of strings was %d instead of %d", count, 3);
		FAIL;
	} else {
		CHECK;

		if(strcmp(list[0], str1) != 0 ||strcmp(list[1], str2) != 0 || strcmp(list[2], str3)) {
			report("The returned strings were:");
			report("\"%s\"", list[0] == NULL ? nullstr : list[0]);
			report("\"%s\"", list[1] == NULL ? nullstr : list[1]);
			report("\"%s\"", list[2] == NULL ? nullstr : list[2]);
			report("instead of:");
			report("\"%s\"", str1);
			report("\"%s\"", str2);
			report("\"%s\"", str3);
		} else
			CHECK;

	}

	XFreeStringList(list);
	CHECKPASS(3);

>>ASSERTION Good A
When the
.M encoding
component of the
.A text_prop
argument is not STRING, then
a call to xname sets no return values and returns zero.
>>STRATEGY
Create an XTextProperty structure with encoding component set to XA_WINDOW.
Call XTextPropertyToStringList.
Verify that the call returned zero.
Verify that the list_return argument was not changed.
Verify that the count_return argument was not changed.
>>CODE
Status		status;
XTextProperty	tp;
char		**list = (char **) -1;
int		count = -1;

	tp.value = (unsigned char *) "Xtest_._.\0test._\0string._..";
	tp.nitems = 27;
	tp.format = 8;
	tp.encoding = XA_WINDOW;
	text_prop = &tp;
	list_return = &list;
	count_return = &count;
	status = XCALL;

	if(status != 0) {
		report("%s() did not return zero.", TestName);
		FAIL;
	} else
		CHECK;

	if(list != (char**) -1) {
		report("The list_return argument was altered.");
		FAIL;
	} else
		CHECK;

	if(count != -1) {
		report("The count_return argument was altered.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Good A
When the
.A format
component of the
.A text_prop
argument is not 8, then
a call to xname sets no return values and returns zero.
>>STRATEGY
Create an XTextProperty structure with format component set to 16.
Call XTextPropertyToStringList.
Verify that the call returned zero.
Verify that the list_return argument was not changed.
Verify that the count_return argument was not changed.
>>CODE
Status		status;
XTextProperty	tp;
char		**list = (char **) -1;
int		count = -1;

	tp.value = (unsigned char *) "Xtest_._.\0test._\0string._..";
	tp.nitems = 27;
	tp.format = 16;
	tp.encoding = XA_STRING;
	text_prop = &tp;
	list_return = &list;
	count_return = &count;
	status = XCALL;

	if(status != 0) {
		report("%s() did not return zero.", TestName);
		FAIL;
	} else
		CHECK;

	if(list != (char**) -1) {
		report("The list_return argument was altered.");
		FAIL;
	} else
		CHECK;

	if(count != -1) {
		report("The count_return argument was altered.");
		FAIL;
	} else
		CHECK;

	CHECKPASS(3);

>>ASSERTION Bad B 1
When insufficient memory is available for the list and its elements, 
then a call to xname sets no return values and returns zero.
>># Kieron	Completed	Review
