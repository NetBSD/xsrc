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
 * $XConsortium: kysymtstr.m,v 1.5 94/04/17 21:09:49 rws Exp $
 */
>>TITLE XKeysymToString CH10
char *
XKeysymToString(keysym)
KeySym	keysym;
>>EXTERN
#define XK_MISCELLANY
#include "keysymdef.h"
#undef  XK_MISCELLANY
>>ASSERTION Good A
A call to xname returns 
as a null-terminated string
the name of the
.S KeySym
specified by the
.A keysym
argument.
>>STRATEGY
Obtain the string corresponding to the KeySym XK_BackSpace using xname.
Verify that the returned string was "BackSpace".
>>CODE
KeySym	ks = XK_BackSpace;
char	*value = "BackSpace";
char	*res;

	keysym = ks;
	res = XCALL;	

	if( res == (char *) NULL) {
		report("%s() returned NULL for KeySym XK_BackSpace.", TestName);
		FAIL;
	} else {
		CHECK;
		if(strcmp(res, value) != 0) {
			report("%s() returned \"%s\" instead of \"%s\" for KeySym XK_Backspace.",
				TestName, res, value);
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS(2);

>>ASSERTION Good A	
When the
.S KeySym
specified by the
.A keysym
argument
is not defined, then a call to xname returns NULL.
>>STRATEGY
Obtain the string corresponding to the KeySym NoSymbol using xname.
Verify that the returned string was NULL
>>CODE
KeySym	ks = NoSymbol;
char	*res;

	keysym = ks;
	res = XCALL;	

	if( res != (char *) NULL) {
		report("%s() returned \"%s\" instead of NULL for KeySym NoSymbol.",
			TestName, res);
		FAIL;
	} else
		PASS;
