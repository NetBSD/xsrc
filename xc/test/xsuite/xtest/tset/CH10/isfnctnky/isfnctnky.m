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
 * $XConsortium: isfnctnky.m,v 1.5 94/04/17 21:09:40 rws Exp $
 */
>>TITLE IsFunctionKey CH10

IsFunctionKey(keysym)
KeySym	keysym;
>>EXTERN
#define XK_LATIN1
#define XK_MISCELLANY
#include	"keysymdef.h"
#undef	XK_MISCELLANY
#undef XK_LATIN1
>>ASSERTION Good A
When the
.A keysym
argument is a function key, then invocation
of the xname macro returns
.S True .
>>STRATEGY
For each function key KeySym:
   Verify that xname returns True.
>>CODE
static KeySym ks[] = {
			XK_F1,
			XK_F2,
			XK_F3,
			XK_F4,
			XK_F5,
			XK_F6,
			XK_F7,
			XK_F8,
			XK_F9,
			XK_F10,
			XK_F11,
			XK_L1,
			XK_F12,
			XK_L2,
			XK_F13,
			XK_L3,
			XK_F14,
			XK_L4,
			XK_F15,
			XK_L5,
			XK_F16,
			XK_L6,
			XK_F17,
			XK_L7,
			XK_F18,
			XK_L8,
			XK_F19,
			XK_L9,
			XK_F20,
			XK_L10,
			XK_F21,
			XK_R1,
			XK_F22,
			XK_R2,
			XK_F23,
			XK_R3,
			XK_F24,
			XK_R4,
			XK_F25,
			XK_R5,
			XK_F26,
			XK_R6,
			XK_F27,
			XK_R7,
			XK_F28,
			XK_R8,
			XK_F29,
			XK_R9,
			XK_F30,
			XK_R10,
			XK_F31,
			XK_R11,
			XK_F32,
			XK_R12,
			XK_R13,
			XK_F33,
			XK_F34,
			XK_R14,
			XK_F35,
			XK_R15,
			0 };
KeySym	*ksp;
Bool	res;

	for(ksp = ks; *ksp; ksp++) {
		keysym = *ksp;
		res = XCALL;
		if(res != True) {
			char	*kstr = XKeysymToString(*ksp);

			report("%s() did not return True for KeySym XK_%s (value %lu).",
				TestName, kstr != (char *) NULL ? kstr : "<KeySym Undefined>", *ksp);
			FAIL;
		} else
			CHECK;

	}

	CHECKPASS(NELEM(ks) - 1);

>>ASSERTION Good A
When the
.A keysym
argument is not a function key, then
invocation of the xname macro returns
.S False .
>>STRATEGY
Verify that xname returns False for the KeySym XK_A.
>>CODE
Bool	res;

	keysym = XK_A;
	res = XCALL;
	if(res != False) {
		char	*kstr = XKeysymToString(keysym);

		report("%s() did not return False for KeySym XK_%s (value %lu).",
			TestName, kstr != (char *) NULL ? kstr : "<KeySym Undefined>", keysym);
		FAIL;
	} else
		CHECK;

	CHECKPASS(1);
