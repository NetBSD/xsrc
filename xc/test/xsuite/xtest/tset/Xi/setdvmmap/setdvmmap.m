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
 * Copyright 1993 by the Hewlett-Packard Company.
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the names of HP, and UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  HP, and UniSoft
 * make no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: setdvmmap.m,v 1.12 94/09/06 20:52:51 dpw Exp $
 */
>>TITLE XSetDeviceModifierMapping XINPUT
int
xname
Display	*display = Dsp;
XDevice *device;
XModifierKeymap	*modmap;
>>SET return-value MappingSuccess
>>EXTERN

extern ExtDeviceInfo Devs;
extern int MinKeyCode, MaxKeyCode;

static	XModifierKeymap	*origmap;

>>SET startup savemap
static void
savemap()
{

	startup();
	if (!Setup_Extension_DeviceInfo(ModMask))
	    {
	    return;
	    }
	SelectExtensionEvents (Dsp, DefaultRootWindow(Dsp));
	XSync(Dsp,0);
	device = Devs.Mod;
	if(Dsp) {
		origmap = XGetDeviceModifierMapping(Dsp, device);
	}
}

>>SET cleanup cleanmap
static void
cleanmap()
{
	if(Dsp && Devs.Key)
		XSetDeviceModifierMapping(Dsp, Devs.Mod, origmap);
	XSync(Dsp,0);
	cleanup();
}

>>ASSERTION Good B 3
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

	if (!Devs.Mod) {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
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

	device = Devs.Key;
	XCALL;

	newmap = XGetDeviceModifierMapping(display, device);

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
>>ASSERTION Good B 3
When a call to xname succeeds, then a
.S DeviceMappingNotify
event is generated.
>>STRATEGY
Call xname to set mapping.
Verify that a DeviceMappingNotify event is generated.
>>CODE
int 	n, dmn;
XEvent	ev;
XDeviceMappingEvent	good;
XEventClass dmnc;

	if (!Devs.Mod) {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	modmap = origmap;

	device = Devs.Mod;
	DeviceMappingNotify(Devs.Mod, dmn, dmnc);
	XCALL;

	defsetevent(good, display, dmn);
	good.window = None;	/* unused */
	good.request = MappingModifier;
	good.deviceid = Devs.Key->device_id;
	/* rest not used */

	n = getevent(display, &ev);
	if (n == 0 || ev.type != dmn) {
		report("Expecting a devicemappingnotify event");
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
>>ASSERTION Good B 3
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

	if (!Devs.Mod) {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Mod;
	oldmap = XGetDeviceModifierMapping(display, device);
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

	if (!Devs.Mod) {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Mod;
	if ((modmap = XNewModifiermap(1)) == 0) {
		delete("Failed to create new modifier map");
		return;
	}

	for (i = 0; i < 8; i++)
		modmap->modifiermap[i] = 0;

	found = 0;
	for (kc = MinKeyCode; kc <= MaxKeyCode; kc++) {
		modmap->modifiermap[0] = (KeyCode)kc;

		ret = XCALL;

		if (ret != MappingSuccess) {

			found = True;

			if (ret != MappingFailed) {
				report("Return value was %d, expecting MappingFailure", ret);
				FAIL;
				break;
			}
			newmap = XGetDeviceModifierMapping(display, device);

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
		CHECKPASS(MaxKeyCode-MinKeyCode+1);

>>ASSERTION Bad B 3
When the new KeyCodes specified for a modifier differ from those
currently defined and any of the
current or new keys for that modifier are
in the logically down state, then a call to xname returns
.S MappingBusy
and none of the modifiers are changed.
>>STRATEGY
If the XTest extension is present, press one of the keys to be used as one
of the modifiers.  Then change the mapping.  Verify that a MappingBusy
error was returned.
>>CODE
int 	i, ret;
int 	kpm;
XModifierKeymap	*newmap;

	if (!Devs.Mod) {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	if (noext(1)) {
		return;
	}

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
		{
		modmap->modifiermap[i] = origmap->modifiermap[kpm*8-1 - i];
		devicekeypress (display, Devs.Mod, modmap->modifiermap[i]);
		}


	device = Devs.Mod;
	ret = XCALL;
	if (ret != MappingBusy) {
		report("Return value was %d, expecting MappingBusy", ret);
		FAIL;
	}
	else
		PASS;
	for (i = 0; i < kpm*8; i++)
		{
		modmap->modifiermap[i] = origmap->modifiermap[kpm*8-1 - i];
		devicekeyrel (display, Devs.Mod, modmap->modifiermap[i]);
		}
	devicerelkeys (Devs.Mod);

>>ASSERTION Bad B 3
.ER BadAlloc
>>ASSERTION Bad B 3
When a KeyCode is not in the range returned by
XListInputDevices,
then a
.S BadValue
error occurs.
>>STRATEGY
Set up map with keycode less than the minimum value.
Call xname.
Verify that a BadValue error occurs.

Set up map with keycode greater than the maximum value (if possible).
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue
int 	i, ret;

	if (!Devs.Mod) {
	    untested("%s: No input extension key device.\n", TestName);
	    return;
	    }
	device = Devs.Mod;
	modmap = XNewModifiermap(1);
	for (i = 0; i < 8*modmap->max_keypermod; i++)
		modmap->modifiermap[i] = 0;

	modmap->modifiermap[0] = MinKeyCode-1;
	ret = XCALL;
	XSync(Dsp,0);

	if (geterr() == BadValue)
		CHECK;
	else
		FAIL;

	if (MaxKeyCode+1 < 0xff) {
		modmap->modifiermap[0] = MaxKeyCode+1;
		ret = XCALL;

		if (geterr() == BadValue)
			CHECK;
		else
			FAIL;
	} else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Bad B 3
If an invalid device is specified, a BadDevice error occurs.
>>STRATEGY
Specifiy an invalid device.
>>CODE baddevice
int ret;
XID baddevice;
XDevice bogus;
int ximajor, first, err;

	if (!XQueryExtension (display, INAME, &ximajor, &first, &err)) {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

	BadDevice(display, baddevice);
	bogus.device_id = -1;
	device = &bogus;
	modmap = XNewModifiermap(1);
	ret = XCALL;

	if (geterr() == baddevice)
		PASS;
	else
		FAIL;
