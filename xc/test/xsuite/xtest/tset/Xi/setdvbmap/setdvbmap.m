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
 * $XConsortium: setdvbmap.m,v 1.8 94/09/06 20:53:38 dpw Exp $
 */
>>TITLE XSetDeviceButtonMapping XINPUT
>>SET return-value MappingSuccess
int

Display	*display = Dsp;
XDevice *device = Devs.Button;
unsigned char	*map = Map;
int 	nmap = 255;
>>EXTERN

/* Maximum button number allowed. */
#define	MAXBUTTON	255

/*
 * MAPSIZE must be at least one greater than the maximum number of buttons
 * allowed.
 */
#define	MAPSIZE	256
static	unsigned char	Map[MAPSIZE];
static 	XID baddevice; 

static	int 	nbtns;
extern ExtDeviceInfo Devs;

/*
 * Set up the number of buttons.  Also set the nmap value to nbtns (
 * the test may later override this).
 */
>>SET begin-function getnbutton
static	void
getnbutton()
{
	if (!Devs.Button)
	    return;
	device = Devs.Button;
	nbtns = XGetDeviceButtonMapping(Dsp, device, Map, MAPSIZE);
	nmap = nbtns;
	if (isdeleted())
		delete("XGetDeviceButtonMapping failed");
}

/*
 * Save and restore the old map.
 */
static unsigned char	oldmap[MAPSIZE];
>>SET startup savemap
static void
savemap()
{
	startup();
	if (!Setup_Extension_DeviceInfo(BtnMask))
	    {
	    return;
	    }
	device = Devs.Button;
	if(Dsp)
		nbtns = XGetDeviceButtonMapping(Dsp, device, oldmap, MAPSIZE);
}

>>SET cleanup restoremap
static void
restoremap()
{
	device = Devs.Button;
	if(Dsp && Devs.Button)
		{
		(void) XSetDeviceButtonMapping(Dsp, device, oldmap, nbtns);
		XSync(Dsp,0);
		}
	cleanup();
}

>>ASSERTION Good B 3
A successful call to xname sets the pointer mapping for the physical buttons
to the
.A nmap
logical button numbers specified in the array
.A map
and returns
.S MappingSuccess .
>>STRATEGY
Get number of buttons.
Set up a pointer mapping.
Set pointer mapping with xname.
Verify return value.
Get pointer mapping with XGetDeviceButtonMapping.
Verify that pointer mapping is as set.
>>CODE
unsigned char	newmap[MAPSIZE];
int 	i;

	if (!Devs.Button) {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	/*
	 * Cycle the current mapping around.
	 */
	for (i = 0; i < nbtns; i++) {
		map[i] = map[i] + 1;
		if (map[i] > MAXBUTTON)
			map[i] = 1;
	}

	device = Devs.Button;

	XCALL;

	(void) XGetDeviceButtonMapping(display, device, newmap, MAPSIZE);
	if (isdeleted()) {
		delete("Could not get device button mapping");
		return;
	}

	for (i = 0; i < nbtns; i++) {
		if (map[i] == newmap[i])
			CHECK;
		else {
			report("Mapping not set correctly in position %d", i);
			report("  was %u, expecting %u", (unsigned)newmap[i], (unsigned)map[i]);
			FAIL;
		}
	}

	CHECKPASS(nbtns);
>>ASSERTION Good B 3
When a call to xname is successful, then a
.S DeviceMappingNotify
event is generated.
>>STRATEGY
Call xname.
Verify that a DeviceMappingNotify event is generated.
>>CODE
XEvent	ev;
XDeviceMappingEvent	good;
int 	i,n,dbmn;
XEventClass dbmnc;

	if (!Devs.Button) {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	device = Devs.Button;
	DeviceMappingNotify(Devs.Button, dbmn, dbmnc);
	XSelectExtensionEvent (display, DefaultRootWindow(Dsp), &dbmnc, 1);
	XSync(display,0);
	XCALL;
	XSync(display,0);

	n = getevent(display, &ev);
	if (n == 0 || ev.type != dbmn) {
		report("Expecting a DeviceMappingNotify event, received %s", n? eventname(ev.type): "no event");
		FAIL;
	} else
		CHECK;

	defsetevent(good, display, dbmn);
	good.window = None;	/* Not used */
	good.request = MappingPointer;
	good.deviceid = Devs.Button->device_id;
	if (checkevent((XEvent*)&good, &ev))
		FAIL;
	else
		CHECK;

	CHECKPASS(2);
>>ASSERTION Good B 3
When an element of
.A map
is zero, then the corresponding physical button is
disabled.
>>STRATEGY
If extension available:
  Create and map a window.
  Select DeviceButtonPress on it.
  For i in 1..nbtns
    Set map[i-1] to 0.
    Call xname.
    Discard event queue.
    Simulate button i press with extension.
    Release all buttons etc.
    Check no button press event received.
    Restore map[i-1].
else
  Report untested.
>>CODE
int	i;
unsigned char zmap[255];
Window	win;
XID dbp, dbpclass;

	if (!Devs.Button) {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	if (noext(nbtns))
		return;
	else
		CHECK;

	if (nbtns<1 || nbtns>255) {
		report("Protocol limit of 1..255 buttons exceeded (%d).", nbtns);
		return;
	} else
		CHECK;

	for(i=0; i<nbtns; i++)
		zmap[i] = map[i];
	win = defwin(display);
	device = Devs.Button;
	DeviceButtonPress(device, dbp, dbpclass);
	(void) warppointer(display, win, 2,2);
	XSelectExtensionEvent(display, win, &dbpclass, 1);

	for(i=0; i<nbtns; i++) {
		unsigned char mapsave = zmap[i];
		int ret;
		XEvent ev;

		zmap[i] = 0; /* disable button i+1 */
		map = zmap;
		ret = XCALL;

		if (ret != MappingSuccess) {
			delete("Couldn't set zero entry for button %d.", i+1);
			return;
		} else
			CHECK;

		XSync(display, True); /* discard event queue */
		_startcall(display);	/* set error handler etc. */
		devicebuttonpress(display, device, (unsigned int)i+1);
		devicebuttonrel(display, device, (unsigned int)i+1);
		XSync(display,0);
		relalldev();
		_endcall(display);
		if (geterr() != Success) {
			delete("Couldn't simulate pressing button %d.", i+1);
			return;
		} else
			CHECK;
		if (!XPending(display))
		    CHECK;
		else while(XPending(display))
			{
			XNextEvent (display, &ev);
			if (ev.type == dbp)
			    {
			    report("Got event after pressing disabled button %d.", i+1);
			    FAIL;
			    }
			}
		zmap[i] = mapsave; /* restore button i+1 */
	}
	devicerelbuttons(device);
	CHECKPASS(nbtns*3 + 2);

>>ASSERTION Good B 3
Elements of the
.A map
array are not restricted in
value by the number of physical buttons.
>>STRATEGY
Set up map array with button number higher than number of physical buttons.
Call xname.
Verify no error.
>>CODE
int 	i;

	if (!Devs.Button) {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	for (i = 0; i < nbtns; i++)
		map[i] = i;

	map[0] = nbtns+1;

	device = Devs.Button;
	XCALL;

	if (geterr() == Success)
		PASS;
	else
		FAIL;
>>ASSERTION Good B 3
When any of the buttons to be altered are logically in the down state,
then a call to xname returns
.S MappingBusy ,
and the mapping is not changed.
>>STRATEGY
If the xtest extension is available, press one of the buttons.  Then
try to change the map.  Verify that a status of MappingBusy was returned.
>>CODE
int i, ret;

	if (!Devs.Button) {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	if (noext(0))
	    return;
	getnbutton();
	/*
	 * Cycle the current mapping around.
	 */
	for (i = 0; i < nbtns; i++) {
		map[i] = map[i] + 1;
		if (map[i] > MAXBUTTON)
			map[i] = 1;
	}
	devicebuttonpress(display, Devs.Button, Button1);
	devicebuttonpress(display, Devs.Button, Button2);
	XSync(display,0);
	device = Devs.Button;
	ret = XCALL;

	if (ret == MappingBusy)
		PASS;
	else
		FAIL;
	devicebuttonrel(display, Devs.Button, Button1);
	devicebuttonrel(display, Devs.Button, Button2);
	devicerelbuttons(device);

>>ASSERTION Bad B 3
When
.A nmap
is not the same as the length that
.F XGetDeviceButtonMapping
would return,
then a
.S BadValue
error occurs.
>>STRATEGY
Set nmap to incorrect value.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue
int 	i;

	if (!Devs.Button) {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	if (nbtns < 254)
	    nmap = nbtns + 2;
	else if (nbtns > 1)
	    nmap = nbtns - 1;
	else
	    {
	    untested("%s: Bad # buttons on extension device.\n", TestName);
	    return;
	    }
	for (i = 0; i < nmap; i++)
		map[i] = i;	/* MAPSIZE is large enough to allow this */
	device = Devs.Button;
	XCALL;

	if (geterr() == BadValue)
		PASS;
	else
		FAIL;	/* done already */
>>ASSERTION Bad C
If there is more than one button:
When two elements of
.A map
have the same non-zero value,
then a
.S BadValue
error occurs.
>>STRATEGY
If less than two buttons
  Report unsupported.
Set up a map with two elements the same.
Call xname.
Verify that a BadValue error occurs.
>>CODE BadValue
int 	i;

	if (!Devs.Button) {
	    untested("%s: No input extension button device.\n", TestName);
	    return;
	    }
	getnbutton();
	if (nbtns < 2) {
		unsupported("There are less than two buttons");
		return;
	}

	for (i = 0; i < nmap; i++)
		map[i] = i;

	map[0] = map[1];
	device = Devs.Button;
	XCALL;

	if (geterr() == BadValue)
		PASS;
	else
		FAIL;

>>ASSERTION Bad B 3
If xname is invoked with an invalid device, a BadDevice error
will result.
>>STRATEGY
Make the call with an invalid device.
>>CODE baddevice
XDevice nodevice;
int ximajor, first, err;

	if (!XQueryExtension (display, INAME, &ximajor, &first, &err))
	    {
	    untested("%s: Input extension not supported.\n", TestName);
	    return;
	    }

	BadDevice (display, baddevice);
	nodevice.device_id = -1;
	device = &nodevice;

	XCALL;

	if (geterr() == baddevice)
		CHECK;
	else
		FAIL;

	CHECKPASS(1);
