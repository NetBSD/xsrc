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
 * $XConsortium: stscrnsvr.m,v 1.6 94/04/17 21:06:54 rws Exp $
 */
>>TITLE XSetScreenSaver CH07
void

Display	*display = Dsp;
int 	timeout = 34;
int 	interval = 12;
int 	prefer_blanking = PreferBlanking;
int 	allow_exposures = AllowExposures;
>>EXTERN

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

>>ASSERTION Good B 3
When the
.A timeout
argument is greater than zero, then the screen saver is enabled
and the value of
.A timeout
specifies the time in seconds, until the screen saver is activated.
>>STRATEGY
Touch test with timeout greater than zero.
>>CODE

	timeout = 10;

	XCALL;

	untested("Touch test with timeout greater than zero");
>>ASSERTION Good B 3
When
.A timeout
is 0, then the screen saver is disabled.
>>STRATEGY
Touch test for timeout zero.
>>CODE

	timeout = 0;
	XCALL;

	untested("Touch test for timeout zero");
>>ASSERTION Good B 3
When
.A timeout
is 0 and the screen saver is activated, then a call to xname
does not deactivate the screen saver.
>>ASSERTION Good B 3
When
.A timeout
is \-1, then the default timeout is restored.
>>STRATEGY
Touch test for timeout of -1.
>>CODE

	timeout = -1;
	XCALL;

	untested("Touch test for timeout of -1");
>>ASSERTION Good B 3
If the server-dependent screen saver method uses periodic change:
The interval argument serves as a hint about how long the change period
should be, and zero hints that no periodic change should be made.
>>STRATEGY
Touch test for interval, including zero.
>>CODE

	interval = 3;
	XCALL;

	interval = 0;
	XCALL;

	untested("Touch test for interval, including zero");
>>ASSERTION Good B 3
When
.A interval
is \-1, then the default interval is restored.
>>STRATEGY
Touch test for default interval.
>>CODE

	interval = -1;
	XCALL;

	untested("Touch test for default interval");
>>ASSERTION Good B 3
When no input from devices is generated
for the specified number of
.A timeout
seconds once the screen saver is enabled,
then the screen saver is activated.
>>ASSERTION Good D 3
If the hardware supports video blanking:
When
.A prefer_blanking
is
.S PreferBlanking
and the screen saver is subsequently activated,
then the screen goes blank.
>>STRATEGY
Touch test for PreferBlanking.
>>CODE

	prefer_blanking = PreferBlanking;
	XCALL;

	untested("Touch test for PreferBlanking");
>>ASSERTION Good B 3
When
.A prefer_blanking
is
.S DontPreferBlanking
and
.A allow_exposures
is
.S AllowExposures
or the screen can be regenerated without sending
.S Expose
events and the screen saver is subsequently activated,
then the screen is altered in an implementation defined way that avoids
phosphor burn.
>>STRATEGY
Touch test for DontPreferBlanking.
>>CODE

	prefer_blanking = DontPreferBlanking;
	allow_exposures = AllowExposures;
	XCALL;

	untested("Touch test for DontPreferBlanking");
>>ASSERTION Good B 3
When
.A prefer_blanking
is
.S DefaultBlanking ,
then the default value for the server is restored.
>>STRATEGY
Touch test for DefaultBlanking.
>>CODE

	prefer_blanking = DefaultBlanking;
	XCALL;

	untested("Touch test for DefaultBlanking");
>>ASSERTION Good B 3
When
.A allow_exposures
is
.S DontAllowExposures ,
and the screen can not be blanked or regenerated without sending
.S Expose
events and the screen saver would subsequently be activated,
then the screen's state does not change
and the screen saver is not activated.
>>STRATEGY
Touch test for DontAllowExposures.
>>CODE

	allow_exposures = DontAllowExposures;
	XCALL;

	untested("Touch test for DontAllowExposures");
>>ASSERTION Good B 3
When
.A allow_exposures
is
.S DefaultExposures ,
then the servers default value is restored.
>>STRATEGY
Touch test for DefaultExposures.
>>CODE

	allow_exposures = DefaultExposures;
	XCALL;

	untested("Touch test for DefaultExposures");
>>ASSERTION Good B 3
The screen saver is deactivated,
and all screen states are restored at the next
keyboard or pointer input or at the next call to
.S XForceScreenSaver
with mode
.S ScreenSaverReset .
>>ASSERTION Bad A
When
.A timeout
is a negative number other than \-1, then a
.S BadValue
error occurs.
>>STRATEGY
Call xname with timeout of -2.
Verify that a BadValue error occurs.
>>CODE BadValue

	timeout = -2;
	XCALL;

	if (geterr() == BadValue)
		PASS;
>>ASSERTION Bad A
When
.A interval
is a negative number other than \-1, then a
.S BadValue
error occurs.
>>STRATEGY
Call xname with interval of -2.
Verify that a BadValue error occurs.
>>CODE BadValue

	interval = -2;
	XCALL;

	if (geterr() == BadValue)
		PASS;
>>ASSERTION Bad A
.ER BadValue prefer_blanking DontPreferBlanking PreferBlanking DefaultBlanking
>>ASSERTION Bad A
.ER BadValue allow_exposures DontAllowExposures AllowExposures DefaultExposures
