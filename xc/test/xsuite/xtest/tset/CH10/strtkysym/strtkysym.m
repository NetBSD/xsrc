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
 * $XConsortium: strtkysym.m,v 1.6 94/04/17 21:10:35 rws Exp $
 */
>>TITLE XStringToKeysym CH10
KeySym
XStringToKeysym(string)
char	*string;
>>EXTERN
#define XK_MISCELLANY
#include "keysymdef.h"
#undef  XK_MISCELLANY
>>ASSERTION Good A
A call to xname returns the
.S KeySym
value for the
.S KeySym 
name specified by the
null-terminated
.A string
argument.
>>STRATEGY
Obtain the KeySym corresponding to the string "Escape" using XStringToKeysym.
Verify that the returned value is XK_Escape.
>>CODE
KeySym	ks;
char	*tstr = "Escape";

	string = tstr;
	ks = XCALL;
	if(ks != XK_Escape) {
		report("%s() returned KeySym %lu instead of XK_Escape (%lu) for \"%s\".",
			TestName, (long) ks, (long) XK_Escape, tstr);
		FAIL;
	} else
		PASS;

>>ASSERTION Good A
When the specified string does not match a valid KeySym, then
a call to xname returns
.S NoSymbol . 
>>STRATEGY
Obtain the KeySym matching the string "XTest_No_Sym" using XStringToKeysym.
Verify that the returned value is NoSymbol.
>>CODE
KeySym	ks;
char	*tstr = "XTest_No_Sym";

	string = tstr;
	ks = XCALL;
	if(ks != NoSymbol) {
		report("%s() returned KeySym %lu instead of NoSymbol (%lu) for \"%s\".",
			TestName, (long) ks, (long) NoSymbol, tstr);
		FAIL;
	} else
		PASS;

