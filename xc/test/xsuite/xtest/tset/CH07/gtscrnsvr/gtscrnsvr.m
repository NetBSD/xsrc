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
 * $XConsortium: gtscrnsvr.m,v 1.5 94/04/17 21:06:36 rws Exp $
 */
>>TITLE XGetScreenSaver CH07
void

Display	*display = Dsp;
int 	*timeout_return = &tr;
int 	*interval_return = &ir;
int 	*prefer_blanking_return = &pbr;
int 	*allow_exposures_return = &aer;
>>EXTERN

static	int 	tr;
static	int 	ir;
static	int 	pbr;
static	int 	aer;

static	int 	origt;
static	int 	origi;
static	int 	origpb;
static	int 	origae;

>>SET startup savesaver
static void
savesaver()
{
	startup();
	if(Dsp)
		XGetScreenSaver(Dsp, &origt, &origi, &origpb, &origae);
}

>>SET cleanup resetsaver
static void
resetsaver()
{
	if(Dsp)
		XSetScreenSaver(Dsp, origt, origi, origpb, origae);
	cleanup();
}

>>ASSERTION Good A
A call to xname returns the current screen saver values.
>>STRATEGY
Set screen saver values.
Get screen saver values.
Verify that returned values are as set.
>>EXTERN
#define	TOUT	71
#define	INTERVAL	57
#define	BLANKING	PreferBlanking
#define	EXPOSURES	AllowExposures
>>CODE

	XSetScreenSaver(display, TOUT, INTERVAL, BLANKING, EXPOSURES);

	XCALL;

	if (*timeout_return == TOUT)
		CHECK;
	else {
		report("timeout_return was %d, expecting %d", *timeout_return, TOUT);
		FAIL;
	}
	if (*interval_return == INTERVAL)
		CHECK;
	else {
		report("interval_return was %d, expecting %d",
			*interval_return, INTERVAL);
		FAIL;
	}
	if (*prefer_blanking_return == BLANKING)
		CHECK;
	else {
		report("prefer_blanking_return was %d, expecting %d",
			*prefer_blanking_return, BLANKING);
		FAIL;
	}
	if (*allow_exposures_return == EXPOSURES)
		CHECK;
	else {
		report("allow_exposures_return was %d, expecting %d",
			*allow_exposures_return, EXPOSURES);
		FAIL;
	}

	CHECKPASS(4);
