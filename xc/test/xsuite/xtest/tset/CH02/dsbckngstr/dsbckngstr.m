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
 * $XConsortium: dsbckngstr.m,v 1.8 94/04/17 21:02:16 rws Exp $
 */
>>SET   macro
>>TITLE XDoesBackingStore CH02
int
XDoesBackingStore(screen)
Screen	*screen = DefaultScreenOfDisplay(Dsp);
>>EXTERN
static char	*wm = "WhenMapped";
static char	*al = "Always";
static char	*nu = "NotUseful";
static char	er[9];
static char *
bs(bs)
int bs;
{
	switch(bs) {
	case WhenMapped :
		return wm;
	case Always :
		return al;
	case NotUseful :
		return nu;
	default :
		sprintf(er, "%d", bs);
		return er;
	}
}
>>ASSERTION Good A
A call to xname returns 
.S WhenMapped , 
.S NotUseful , 
or
.S Always 
to indicate whether the screen
.A screen
supports backing stores.
>>STRATEGY
Obtain the level of support for backing store using xname.
>>CODE
int	dbs;
int	cdbs;

	switch(config.does_backing_store) {
	case 0:
		cdbs = NotUseful;
		break;
	case 1:
		cdbs = WhenMapped;
		break;
	case 2:
		cdbs = Always;
		break;
	default:
		delete("XT_DOES_BACKING_STORE was not set to 0, 1 or 2");
		return;
	}

	dbs = XCALL;
	if(cdbs !=  dbs) {
		report("%s() returned %s (%d) instead of %s (%d).\n", TestName, bs(dbs), dbs, bs(cdbs), cdbs);
		FAIL;		
	} else
		PASS;
