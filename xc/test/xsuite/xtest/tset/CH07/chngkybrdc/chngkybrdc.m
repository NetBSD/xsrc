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
 * $XConsortium: chngkybrdc.m,v 1.11 94/04/17 21:06:11 rws Exp $
 */
>>TITLE XChangeKeyboardControl CH07
void

Display	*display = Dsp;
unsigned long	value_mask = KBBellPercent;
XKeyboardControl	*values = &Kcval;
>>EXTERN

/* For the argument */
static XKeyboardControl Kcval;

>>EXTERN

/*
 * Functions to save and restore the old keyboard control values across TP's
 * This also makes the current values available for use in tests.
 */
static XKeyboardState	oldstate;

static void
savekey()
{
	startup();
	if(Dsp)
		XGetKeyboardControl(Dsp, &oldstate);
}
>>SET startup savekey

static void
restorekey()
{
XKeyboardControl	ctr;
int 	led;
int 	key;
int 	minkc, maxkc;

	if(!Dsp) {
		cleanup();
		return;
	}

	/*
	 * It is not easy to restore a previously saved keyboard state.
	 */
	ctr.key_click_percent = oldstate.key_click_percent;
	ctr.bell_percent = oldstate.bell_percent;
	ctr.bell_pitch = oldstate.bell_pitch;
	ctr.bell_duration = oldstate.bell_duration;
	XChangeKeyboardControl(Dsp, KBKeyClickPercent|KBBellPercent|KBBellPitch|KBBellDuration, &ctr);

	ctr.auto_repeat_mode = oldstate.global_auto_repeat;
	XChangeKeyboardControl(Dsp, KBAutoRepeatMode, &ctr);
	XFlush(Dsp);

	for (led = 1; led <= 32; led++) {
		ctr.led = led;
		ctr.led_mode = (oldstate.led_mask & (1<<(led-1)))? LedModeOn:LedModeOff;
		XChangeKeyboardControl(Dsp, KBLed|KBLedMode, &ctr);
	}
	XFlush(Dsp);

	XDisplayKeycodes(Dsp, &minkc, &maxkc);
	for (key = minkc; key < maxkc; key++) {
		ctr.key = key;
		ctr.auto_repeat_mode = (oldstate.auto_repeats[key/8] & (1<<(key%8)))?
			AutoRepeatModeOn: AutoRepeatModeOff;
		XChangeKeyboardControl(Dsp, KBAutoRepeatMode|KBKey, &ctr);
		XFlush(Dsp);
	}
	cleanup();
}
>>SET cleanup restorekey

>>ASSERTION Good A
If the key click loudness can be set:
When the
.S KBKeyClickPercent
bit is set in
.A value_mask ,
then the volume of the key click is set
to the value specified in
.M key_click_percent ,
which is between 0, signifying off,
and 100, signifying loud.
>>STRATEGY
Set value_mask to KBKeyClickPercent.
Set key_click_percent value.
Call xname.
Get new keyboard state.
Verify that change occurred.
>>CODE
int 	i;
XKeyboardState	newks;
static int 	vals[] = {
	0, 100, 50};

	value_mask = KBKeyClickPercent;
	for (i = 0; i < NELEM(vals); i++) {

		values->key_click_percent = vals[i];
		XCALL;

		XGetKeyboardControl(display, &newks);

		if (newks.key_click_percent == vals[i])
			CHECK;
		else {
			report("Key click percent was %d, expecting %d",
				newks.key_click_percent, vals[i]);
			FAIL;
		}
	}

	CHECKPASS(NELEM(vals));
>>ASSERTION Good B 2
When the
.S KBKeyClickPercent
bit is set in
.A value_mask
and
.M key_click_percent
is \-1, then the default key click volume is restored.
>>STRATEGY
Set value_mask to KBKeyClickPercent.
Set key_click_percent to -1.
Call xname.
UNTESTED.
>>CODE

	/*
	 * Since the default isn't specified there is no useful test that
	 * can be done.  However touch test with -1 to make sure that it
	 * doesn't blow up.
	 */
	value_mask = KBKeyClickPercent;
	values->key_click_percent = -1;

	XCALL;

	report("There is no complete test method, but a touch test was performed");

	UNTESTED;
>>ASSERTION Good A
If the bell loudness can be set:
When the
.S KBBellPercent
bit is set in
.A value_mask ,
then the base volume of the bell is set to the value specified in
.M bell_percent ,
which is between 0, signifying off, to 100, signifying loud.
>>STRATEGY
Set value_mask to KBBellPercent.
Set bell_percent value.
Call xname.
Get new keyboard state.
Verify that change occurred.
>>CODE
int 	i;
XKeyboardState	newks;
static int 	vals[] = {
	0, 12, 20, 32, 47, 61, 100};

	value_mask = KBBellPercent;
	for (i = 0; i < NELEM(vals); i++) {

		values->bell_percent = vals[i];
		XCALL;

		/* Do the bell so that you can check audibly if you wish */
		XBell(display, 0);

		XGetKeyboardControl(display, &newks);

		if (newks.bell_percent == vals[i])
			CHECK;
		else {
			report("Key bell percent was %d, expecting %d",
				newks.key_click_percent, vals[i]);
			FAIL;
		}
	}

	/*
	 * Allow time for bells to happen before going on to next test.
	 */
	sleep(2);

	CHECKPASS(NELEM(vals));
>>ASSERTION Good B 2
When the
.S KBBellPercent
bit is set in
.A value_mask
and
.M bell_percent
is \-1, then the base volume of the bell is set to the default.
>>STRATEGY
Set value_mask to KBBellPercent.
Set bell_percent value to -1.
Call xname.
UNTESTED.
>>CODE

	value_mask = KBBellPercent;
	values->bell_percent = -1;

	XCALL;

	report("There is no complete test method, but a touch test was performed");

	UNTESTED;
>>ASSERTION Good A
If the bell pitch can be set:
When the
.S KBBellPitch
bit is set in
.A value_mask ,
then the bell pitch in Hz is set to the value specified in
.M bell_pitch .
>>STRATEGY
Set value_mask to KBBellPitch.
Set bell_pitch value.
Call xname.
Get new keyboard state.
Verify that change occurred.
>>CODE
int 	i;
XKeyboardState	newks;
static int 	vals[] = {
	440, 528, 400, 294, 300, 800, 640};

	value_mask = KBBellPitch;
	for (i = 0; i < NELEM(vals); i++) {

		values->bell_pitch = vals[i];
		XCALL;

		XBell(Dsp, 0);

		XGetKeyboardControl(display, &newks);

		if (newks.bell_pitch == vals[i])
			CHECK;
		else {
			report("Bell pitch was %d, expecting %d",
				newks.bell_pitch, vals[i]);
			FAIL;
		}
	}

	/*
	 * Allow time for bells to happen before going on to next test.
	 */
	sleep(2);

	CHECKPASS(NELEM(vals));
>>ASSERTION Good B 2
When the
.S KBBellPitch
bit is set in
.A value_mask
and
.M bell_pitch
is \-1, then the bell pitch is set to the default.
>>STRATEGY
Set value_mask to KBBellPitch.
Set bell_pitch value to -1.
Call xname.
UNTESTED.
>>CODE

	value_mask = KBBellPitch;
	values->bell_pitch = -1;

	XCALL;

	report("There is no complete test method, but a touch test was performed");

	UNTESTED;
>>ASSERTION Good A
If the bell duration can be set:
When the
.S KBBellDuration
bit is set in
.A value_mask ,
then the bell duration in milliseconds is set to the value specified in
.M bell_duration .
>>STRATEGY
Set value_mask to KBBellDuration.
Set bell_duration value to -1.
Call xname.
Get new keyboard state.
Verify that change occurred.
>>CODE
int 	i;
XKeyboardState	newks;
static int 	vals[] = {
	10, 200, 600, 1000, 50};

	value_mask = KBBellDuration;
	for (i = 0; i < NELEM(vals); i++) {

		values->bell_duration = vals[i];
		XCALL;

		XBell(display, 0);
		XGetKeyboardControl(display, &newks);

		if (newks.bell_duration == vals[i])
			CHECK;
		else {
			report("Bell duration was %d, expecting %d",
				newks.bell_duration, vals[i]);
			FAIL;
		}
	}

	/*
	 * Allow time for bells to happen before going on to next test.
	 */
	sleep(5);

	CHECKPASS(NELEM(vals));
>>ASSERTION Good B 2
When the
.S KBBellDuration
bit is set in
.A value_mask
and
.M bell_duration
is \-1, then the bell duration is set to the default.
>>STRATEGY
Set value_mask to KBBellDuration.
Set bell_duration value to -1.
Call xname.
UNTESTED.
>>CODE

	value_mask = KBBellDuration;
	values->bell_duration = -1;

	XCALL;

	report("There is no complete test method, but a touch test was performed");

	UNTESTED;
>>ASSERTION Good A
If
.SM LED s
are supported:
When both
.S KBLed
and
.S KBLedMode
are specified, then the state of the
.SM LED
specified by
.M led
is changed to the state specified by
.M led_mode .
>>STRATEGY
Set value_mask to KBLed | KBLedMode.
Set led value.
Set led_mode value.
Call xname.
Get new keyboard state.
Verify that change occurred.
>>EXTERN
#define	NLEDS	32	/* Number of leds that the protocol allows */
#define	NLEDMASK 0xffffffff	/* Mask of all leds */
>>CODE
int 	i, j;
unsigned long	onmask;
XKeyboardState	newks;
static int 	vals[] = {
	1, 3, 2, 4, 20, NLEDS};

	value_mask = KBLed|KBLedMode;
	onmask = 0;
	for (i = 1; i <= NLEDS; i++) {

		values->led_mode = LedModeOff;
		for (j = 0; j < NELEM(vals); j++) {
			if (vals[j] == i) {
				onmask |= (1L<<(i-1));
				values->led_mode = LedModeOn;
				break;
			}
		}
		values->led = i;

		XCALL;
	}

	XGetKeyboardControl(display, &newks);

	if (newks.led_mask == onmask)
		CHECK;
	else {
		report("Led mask was %lx, expecting %lx",
			newks.led_mask, onmask);
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good A
If
.SM LED s
are supported:
When
.S KBLedMode
is specified and
.S KBLed
is not specified, then the state of all
.SM LED s
is changed to the mode specified by
.M led_mode .
>>STRATEGY
Set value_mask to KBLed.
Set led_mode value.
Call xname.
Get new keyboard state.
Verify that change occurred.
>>CODE
XKeyboardState	newks;

	value_mask = KBLedMode;
	values->led_mode = LedModeOn;

	XCALL;

	XGetKeyboardControl(display, &newks);

	if (newks.led_mask == (unsigned)NLEDMASK)
		CHECK;
	else {
		report("All leds were not set to correct value");
		report("  led_mask was 0x%x, expecting 0x%x", newks.led_mask, NLEDMASK);
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Good A
When
.S KBKey
and
.S KBAutoRepeatMode
are specified, then the auto repeat mode of
.M key
is changed to the mode specified by
.M auto_repeat_mode .
>>STRATEGY
Set value_mask to KBKey | KBAutoRepeatMode.
Get current value for key.
Set auto_repeat_mode to opposite of current value.
Call xname.
Verify that auto_repeat_mode for the key has changed.
>>CODE
int 	i;
int 	minkc, maxkc;
int 	onoff;
XKeyboardState	newks;

#define	NKEYSTOTRY	12

	value_mask = KBKey | KBAutoRepeatMode;
	XDisplayKeycodes(display, &minkc, &maxkc);
	/*
	 * Try out the first few keycodes.
	 */
	for (i = minkc; i < minkc+NKEYSTOTRY; i++) {

		values->key = i;
		onoff = (oldstate.auto_repeats[i/8] & (1<<(i%8)))?
			AutoRepeatModeOff: AutoRepeatModeOn;
		values->auto_repeat_mode = onoff;
		XCALL;

		XGetKeyboardControl(display, &newks);

		if (((newks.auto_repeats[i/8] & (1<<(i%8))) != 0) == ((onoff==AutoRepeatModeOn)? 1: 0))
			CHECK;
		else {
			report("Key auto repeat was not set to %d", onoff);
			FAIL;
		}
	}

	CHECKPASS(NKEYSTOTRY);
>>ASSERTION Good A
When
.S KBAutoRepeatMode
is specified and
.S KBKey
is not specified,
then the per-key settings are unchanged and the auto repeat mode for
the whole keyboard is changed to that specified by
.M auto_repeat_mode .
>>STRATEGY
Set value_mask to KBAutoRepeatMode.
Get current global auto repeat mode.
Set auto_repeat_mode to different value.
Call xname.
Verify that global auto repeat mode changed.
>>CODE
XKeyboardState	newks;
int 	mode;

	value_mask = KBAutoRepeatMode;
	mode = (oldstate.global_auto_repeat==AutoRepeatModeOn)?
		AutoRepeatModeOff: AutoRepeatModeOn;
	values->auto_repeat_mode = mode;

	XCALL;

	XGetKeyboardControl(display, &newks);

	if (newks.global_auto_repeat == mode)
		CHECK;
	else {
		report("global auto repeat mode was not set");
		FAIL;
	}

	CHECKPASS(1);
>>ASSERTION Bad A
When the
.S KBKeyClickPercent
bit is set in
.A value_mask
and
.M key_click_percent
is not between 0 and 100 inclusive
or \-1, then a
.S BadValue
error occurs.
>>STRATEGY
Set value_mask to KBKeyClickPercent.
Set out of range key_click_percent.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue
int 	i;
static	int 	badvals[] = {
	-100, -2, 101, 203};

	value_mask = KBKeyClickPercent;
	for (i = 0; i < NELEM(badvals); i++) {
		values->key_click_percent = badvals[i];
		XCALL;

		if (geterr() == BadValue)
			CHECK;
		else {
			report("No BadValue for key_click_percent of %d", badvals[i]);
			FAIL;
		}
	}
	CHECKPASS(NELEM(badvals));
>>ASSERTION Bad A
When the
.S KBBellPercent
bit is set in
.A value_mask
and
.M bell_percent
is not between 0 and 100 inclusive
or \-1, then a
.S BadValue
error occurs.
>>STRATEGY
Set value_mask to KBBellPercent.
Set out of range bell_percent.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue
int 	i;
static	int 	badvals[] = {
	-100, -2, 101, 203};

	value_mask = KBBellPercent;
	for (i = 0; i < NELEM(badvals); i++) {
		values->bell_percent = badvals[i];
		XCALL;

		if (geterr() == BadValue)
			CHECK;
		else {
			report("No BadValue for bell_percent of %d", badvals[i]);
			FAIL;
		}
	}
	CHECKPASS(NELEM(badvals));
>>ASSERTION Bad A
When the
.S KBBellPitch
bit is set in
.A value_mask
and
.M bell_pitch
is a negative number other than \-1, then a
.S BadValue
error occurs.
>>STRATEGY
Set value_mask to KBBellPitch.
Set out of range bell_pitch.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue
int 	i;
static	int 	badvals[] = {
	-9012, -100, -2};

	value_mask = KBBellPitch;
	for (i = 0; i < NELEM(badvals); i++) {
		values->bell_pitch = badvals[i];
		XCALL;

		if (geterr() == BadValue)
			CHECK;
		else {
			report("No BadValue for bell_pitch of %d", badvals[i]);
			FAIL;
		}
	}
	CHECKPASS(NELEM(badvals));
>>ASSERTION Bad A
When the
.S KBBellDuration
bit is set in
.A value_mask
and
.M bell_duration
is a negative number other than \-1, then a
.S BadValue
error occurs.
>>STRATEGY
Set value_mask to KBBellDuration.
Set out of range bell_duration.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue
int 	i;
static	int 	badvals[] = {
	-3456, -100, -2};

	value_mask = KBBellDuration;
	for (i = 0; i < NELEM(badvals); i++) {
		values->bell_duration = badvals[i];
		XCALL;

		if (geterr() == BadValue)
			CHECK;
		else {
			report("No BadValue for bell_duration of %d", badvals[i]);
			FAIL;
		}
	}
	CHECKPASS(NELEM(badvals));
>>ASSERTION Bad A
When
.S KBLed
is specified
and
.S KBLedMode
is not specified, then a
.S BadMatch
error occurs.
>>STRATEGY
Set value_mask to KBLed.
Call xname.
Verify that a BadMatch error occurs.
>>CODE BadMatch

	value_mask = KBLed;
	values->led = 1;

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;	/* safe */
>>ASSERTION Bad A
When
.S KBKey
is specified and
.S KBAutoRepeatMode
is not specified,
then a
.S BadMatch
error occurs.
>>STRATEGY
Set value_mask to KBKey.
Call xname.
Verify that a BadMatch error occurs.
>>CODE BadMatch

	value_mask = KBKey;

	XCALL;

	if (geterr() == BadMatch)
		PASS;
	else
		FAIL;

>>ASSERTION Bad A
When the value of
.M led_mode
is other than
.S LedModeOn
or
.S LedModeOff ,
then a
.S BadValue
error occurs.
>>STRATEGY
Set value_mask to KBLedMode.
Set led_mode to other than LedModeOn or LedModeOff.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue
static	int 	lmodes[] = {
	LedModeOn, LedModeOff};
long	badvals[NM_LEN];
int 	i, n;

	n = notmember(lmodes, NELEM(lmodes), badvals);

	value_mask = KBLedMode;

	for (i = 0; i < n; i++) {
		values->led_mode = badvals[i];

		XCALL;

		if (geterr() == BadValue)
			CHECK;
		else {
			report("A led_mode of %d did not give BadValue", badvals[i]);
		}
	}
	CHECKPASS(n);
>>ASSERTION Bad A
When
the value of
.M auto_repeat_mode
is other than
.S AutoRepeatModeOff ,
.S AutoRepeatModeOn
or
.S AutoRepeatModeDefault ,
then a
.S BadValue
error occurs.
>>STRATEGY
Set value_mask to KBAutoRepeatMode.
Set auto_repeat_mode to invalid value.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue
static	int 	armodes[] = {
	AutoRepeatModeOff, AutoRepeatModeOn, AutoRepeatModeDefault};
long	badvals[NM_LEN];
int 	i, n;

	n = notmember(armodes, NELEM(armodes), badvals);

	value_mask = KBAutoRepeatMode;

	for (i = 0; i < n; i++) {
		values->auto_repeat_mode = badvals[i];

		XCALL;

		if (geterr() == BadValue)
			CHECK;
		else {
			report("A auto_repeat_mode of %d did not give a BadValue", badvals[i]);
		}
	}
	CHECKPASS(n);
>># Removed because there is no guarantee that Xlib will pass the bad bits
>># through.
>>#ASSERTION Bad A
>>#.ER Value value_mask mask KBKeyClickPercent KBBellPercent KBBellPitch KBBellDuration KBLed KBLedMode KBKey KBAutoRepeatMode
