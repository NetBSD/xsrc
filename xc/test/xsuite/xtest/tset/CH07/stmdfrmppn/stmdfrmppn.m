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
 * $XConsortium: stmdfrmppn.m,v 1.13 94/04/17 21:06:52 rws Exp $
 */
>>TITLE XSetModifierMapping CH07
int
xname
Display	*display = Dsp;
XModifierKeymap	*modmap;
>>SET return-value MappingSuccess
>>EXTERN

static	XModifierKeymap	*origmap;

static	int	Minkc, Maxkc;

>>SET startup savemap
static void
savemap()
{
	startup();
	if(Dsp) {
		origmap = XGetModifierMapping(Dsp);
		XDisplayKeycodes(Dsp, &Minkc, &Maxkc);
	}
}

>>SET cleanup cleanmap
static void
cleanmap()
{
	if(Dsp)
		XSetModifierMapping(Dsp, origmap);
	cleanup();
}

>>ASSERTION Good A
A succesful call to xname
specifies the KeyCodes of the keys that are to be used
as modifiers and returns
.S MappingSuccess .
>>STRATEGY
Set up a modifier map.
Call xname to set servers map.
Verify that MappingSuccess is returned.
Get current map with XGetModifierMapping.
Verify that the mapping has been set correctly.
>>CODE
int 	i;
int 	kpm;
XModifierKeymap	*newmap;

	/*
	 * Because some keycodes may not be usable as modifiers in a server
	 * dependent fashion, then we must take steps to avoid this.
	 * Therefore: get current modifiers and rearrange them.
	 */
	kpm = origmap->max_keypermod;
	modmap = XNewModifiermap(kpm);
	if (modmap == 0) {
		delete("Could not create new map");
		return;
	}

	for (i = 0; i < kpm*8; i++)
		modmap->modifiermap[i] = origmap->modifiermap[kpm*8-1 - i];

	XCALL;

	newmap = XGetModifierMapping(display);

	if (newmap->max_keypermod == modmap->max_keypermod)
		CHECK;
	else {
		report("max_keypermod was %d, expecting %d", newmap->max_keypermod,
			modmap->max_keypermod);
		FAIL;
	}
	for (i = 0; i < kpm*8; i++) {
		if (modmap->modifiermap[i] == newmap->modifiermap[i])
			CHECK;
		else {
			report("Modifier map was not set correctly");
			FAIL;
			break;
		}
	}
	CHECKPASS(1+kpm*8);

	XFreeModifiermap(newmap);
>>ASSERTION Good A
When a call to xname succeeds, then a
.S MappingNotify
event is generated.
>>STRATEGY
Call xname to set mapping.
Verify that a MappingNotify event is generated.
>>CODE
int 	n;
XEvent	ev;
XMappingEvent	good;

	modmap = origmap;

	XCALL;

	defsetevent(good, display, MappingNotify);
	good.window = None;	/* unused */
	good.request = MappingModifier;
	/* rest not used */

	n = getevent(display, &ev);
	if (n == 0 || ev.type != MappingNotify) {
		report("Expecting a MappingNotify event");
		FAIL;
		return;
	} else
		CHECK;

	if (checkevent((XEvent*)&good, &ev))
		FAIL;
	else
		CHECK;

	CHECKPASS(2);
>>ASSERTION def
The
.M modifiermap
member of the
.S XModifierKeymap
structure contains eight sets of
.M max_keypermod
KeyCodes, one for each modifier in the order
.S Shift ,
.S Lock ,
.S Control ,
.S Mod1 ,
.S Mod2 ,
.S Mod3 ,
.S Mod4 ,
and
.S Mod5 .
>>ASSERTION Good A
When a zero KeyCode occurs in a set, then it is ignored.
>>STRATEGY
>># This is not really true in any sense that we can test.
>># Check that 0 does not generate BadValue when used multiple times.
Set up a mapping with all keycodes zero.
Set mapping with xname.
Verify no BadValue error.
>># Verify that mapping did not change.
>>CODE
XModifierKeymap	*oldmap;
int 	i;

	oldmap = XGetModifierMapping(display);
	if (oldmap == 0) {
		delete("Could not get the old map");
		return;
	}

	modmap = XNewModifiermap(1);
	for (i = 0; i < 8; i++)
		modmap->modifiermap[i] = 0;

	XCALL;

	if (geterr() == Success)
		CHECK;

	CHECKPASS(1);

>>ASSERTION Bad C
When an implementation restriction on which keys can be used
as modifiers is violated,
then a call to xname returns
.S MappingFailed
and none of the modifiers are changed.
>>STRATEGY
Try in turn all possible keycodes.
If all return MappingSuccess:
  Report unsupported.
else
  Verify that MappingFailed is returned.
  Verify that modifier has not been set to this keycode.
>>CODE
int 	i;
int 	ret;
int 	found;
XModifierKeymap *newmap;
unsigned int	kc;

	if ((modmap = XNewModifiermap(1)) == 0) {
		delete("Failed to create new modifier map");
		return;
	}

	for (i = 0; i < 8; i++)
		modmap->modifiermap[i] = 0;

	found = 0;
	for (kc = Minkc; kc <= Maxkc; kc++) {
		modmap->modifiermap[0] = (KeyCode)kc;

		ret = XCALL;

		if (ret != MappingSuccess) {

			found = True;

			if (ret != MappingFailed) {
				report("Return value was %d, expecting MappingFailure", ret);
				FAIL;
				break;
			}
			newmap = XGetModifierMapping(display);

			if (newmap->modifiermap[0] == (KeyCode)kc) {
				report("An invalid keycode (%u) was set into the map", kc);
				FAIL;
				break;
			} else
				CHECK;
		} else
			CHECK;
	}

	if (!found)
		unsupported("All keycodes are acceptable as modifiers for this server");
	else
		CHECKPASS(Maxkc-Minkc+1);

>>ASSERTION Bad B 3
When the new KeyCodes specified for a modifier differ from those
currently defined and any of the
current or new keys for that modifier are
in the logically down state, then a call to xname returns
.S MappingBusy
and none of the modifiers are changed.
>>STRATEGY
If extension available:
  Get current modifier mapping (other tests may have changed it from origmap).
  Make a new modifier map by permuting the old.
  Check there is at least one non-zero keycode in it.
  Simulate pressing this key using extension.
  Call xname to set the new mod. map.
  Expect MappingBusy
  Release all keys.
  Get current mapping with XGetModifierMapping.
  Compare with that got at first.
  Remove key from map.
  Simulate pressing key not in the map.
  Call xname.
  Expect MappingBusy as one of current (at time of xname call) mod keys pressed.
  Release all keys.
  Call xname to set up map to not include this key.
  Expect MappingSuccess.
  Simulate pressing key not in the map.
  Call xname.
  Expect !MappingBusy as key not in current or new maps (at time of xname call).
  Release all keys.
else
  Report untested.
>>CODE
int 	i;
int 	kpm;
int	modkc;
XModifierKeymap	*newmap;
XModifierKeymap	*oldmap;
int	ret;
KeyCode *kcp;

	if (noext(0))
		return;
	else
		CHECK;

	oldmap = XGetModifierMapping(display);

	/*
	 * Because some keycodes may not be usable as modifiers in a server
	 * dependent fashion, then we must take steps to avoid this.
	 * Therefore: get current modifiers and rearrange them.
	 */
	kpm = oldmap->max_keypermod;
	modmap = XNewModifiermap(kpm);
	if (modmap == 0) {
		delete("Could not create new map");
		XFreeModifiermap(oldmap);
		return;
	}

	modkc = 0;
	for (i = 0; i < kpm*8; i++) {
		KeyCode kc = modmap->modifiermap[i] = oldmap->modifiermap[kpm*8-1 - i];

		if (!modkc && kc) {
			modkc = kc;
			kcp = &(modmap->modifiermap[i]);
		}
	}

	if (!modkc) {
		delete("Can't find a usable modifier key code.");
		XFreeModifiermap(oldmap);
		XFreeModifiermap(modmap);
		return;
	} else
		CHECK;

	keypress(display, modkc);

	ret = XCALL;

	relalldev();

	if (ret == MappingBusy)
		CHECK;
	else {
		report("Key %d did not cause MappingBusy when pressed.", modkc);
		FAIL;
	}
	/* now check no change occurred */
	newmap = XGetModifierMapping(display);

	if (newmap->max_keypermod == oldmap->max_keypermod)
		CHECK;
	else {
		report("max_keypermod was %d, expecting %d", newmap->max_keypermod,
			oldmap->max_keypermod);
		FAIL;
	}
	for (i = 0; i < kpm*8; i++) {
		if (oldmap->modifiermap[i] == newmap->modifiermap[i])
			CHECK;
		else {
			report("Modifier map was not set correctly");
			FAIL;
			break;
		}
	}

	/* use a key not in new map, but in current, and expect MappingBusy */
	*kcp = 0; /* remove modkc from map */
	keypress(display, modkc);
	ret = XCALL;
	relalldev();

	if (ret == MappingBusy)
		CHECK;
	else {
		report("Pressing key %d caused %s, not MappingBusy, though in old map.",
			modkc, (ret==MappingSuccess)?"MappingSuccess":"MappingFailed");
		FAIL;
	}
	/* make sure that key is not in map, now */
	ret = XCALL;
	if (ret != MappingSuccess) {
		delete("Can't install new mod. map not including key %d.", modkc);
		XFreeModifiermap(modmap);
		XFreeModifiermap(oldmap);
		XFreeModifiermap(newmap);
		return;
	} else
		CHECK;
	/* use a key not in the new or current map and expect other than MappingBusy */
	keypress(display, modkc);
	ret = XCALL;
	relalldev();

	if (ret != MappingBusy)
		CHECK;
	else {
		report("Pressing key %d caused MappingBusy though not in new or old map.", modkc);
		FAIL;
	}

	XFreeModifiermap(modmap);
	XFreeModifiermap(oldmap);
	XFreeModifiermap(newmap);

	CHECKPASS(7 + kpm*8);
>>SET return-value MappingSuccess
>>ASSERTION Bad B 3
.ER BadAlloc
>>ASSERTION Bad A
When a KeyCode is not in the range returned by
>># This was an error not spotted in the assertion reviews.
>># .F XDisplayCodes ,
.F XDisplayKeycodes ,
then a
.S BadValue
error occurs.
>>STRATEGY
Call XDisplayKeycodes to get range of valid keycodes.
Set up map with keycode less than the minimum value.
Call xname.
Verify that a BadValue error occurs.

Set up map with keycode greater than the maximum value (if possible).
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue
>>SET return-value MappingFailed
int 	i;

	modmap = XNewModifiermap(1);
	for (i = 0; i < 8*modmap->max_keypermod; i++)
		modmap->modifiermap[i] = 0;

	modmap->modifiermap[0] = Minkc-1;
	XCALL;

	if (geterr() == BadValue)
		CHECK;
	else
		FAIL;

	if (Maxkc+1 < 0xff) {
		modmap->modifiermap[0] = Maxkc+1;
		XCALL;

		if (geterr() == BadValue)
			CHECK;
		else
			FAIL;
	} else
		CHECK;

	CHECKPASS(2);
