/*
 
Copyright (c) 1990, 1991, 1992  X Consortium

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
 * Copyright 1990, 1991, 1992 by UniSoft Group Limited.
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
 * $XConsortium: devcntl.c,v 1.9 94/04/17 21:00:42 rws Exp $
 */

#include	"xtest.h"
#include	<Xlib.h>
#include	<Xutil.h>
#include	<extensions/XInput.h>
#include	"xtestlib.h"

#define	MAXBUT	256
#define	MAXKEY	256

static	struct	butsave {
	unsigned int 	button;
	Display	*disp;
} buttons[MAXBUT];
static	int 	butind;

static	struct	keysave {
	int 	key;
	Display	*disp;
} keys[MAXKEY];
static	int 	keyind;

/*
 * Simulate a button press, saving the press so it can be automatically
 * released later.
 */
void
buttonpress(disp, button)
Display *disp;
unsigned int     button;
{
	if (!SimulateButtonPressEvent(disp, button)) {
		delete("XTEST extension not configured or in use");
		return;
	}
	XSync(disp, False);
	debug(1, "Button press %d", button);

	buttons[butind].disp = disp;
	buttons[butind++].button = button;
}

void
devicebuttonpress(disp, dev, button)
Display *disp;
XDevice *dev;
unsigned int     button;
{
	if (!SimulateDeviceButtonPressEvent(disp, dev, button)) {
		delete("XTEST extension not configured or in use");
		return;
	}
	XSync(disp, False);
	debug(1, "Button press %d", button);

	buttons[butind].disp = disp;
	buttons[butind++].button = button;
}

/*
 * Simulate a button release, the corresponding saved event is removed.
 */
void
buttonrel(disp, button)
Display *disp;
unsigned int     button;
{
int 	i;

	if (!SimulateButtonReleaseEvent(disp, button)) {
		delete("XTEST extension not configured or in use");
		return;
	}
	XSync(disp, False);
	debug(1, "Button release %d", button);

	/*
	 * Remove the corresponding button down.
	 */
	for (i = butind-1; i >= 0; i--) {
		if (buttons[i].button == button) {
			buttons[i].disp = 0;
			break;
		}
	}
}

void
devicebuttonrel(disp, dev, button)
Display *disp;
XDevice *dev;
unsigned int     button;
{
int 	i;

	if (!SimulateDeviceButtonReleaseEvent(disp, dev, button)) {
		delete("XTEST extension not configured or in use");
		return;
	}
	XSync(disp, False);
	debug(1, "Button release %d", button);

	/*
	 * Remove the corresponding button down.
	 */
	for (i = butind-1; i >= 0; i--) {
		if (buttons[i].button == button) {
			buttons[i].disp = 0;
			break;
		}
	}
}

/*
 * Simulate a key press, saving the event so it can be automatically
 * released later.
 */
void
keypress(disp, key)
Display *disp;
int     key;
{
	if (key == NoSymbol)
		return;

	if (!SimulateKeyPressEvent(disp, key)) {
		delete("XTEST extension not configured or in use");
		return;
	}
	XSync(disp, False);
	debug(1, "Key press %d", key);

	keys[keyind].disp = disp;
	keys[keyind++].key = key;
}

void
devicekeypress(disp, dev, key)
Display *disp;
XDevice *dev;
int     key;
{
	if (key == NoSymbol)
		return;

	if (!SimulateDeviceKeyPressEvent(disp, dev, key)) {
		delete("XTEST extension not configured or in use");
		return;
	}
	XSync(disp, False);
	debug(1, "Key press %d", key);

	keys[keyind].disp = disp;
	keys[keyind++].key = key;
}

/*
 * Simulate a key release, the corresponding saved event is removed.
 */
void
keyrel(disp, key)
Display *disp;
int     key;
{
int 	i;

	if (key == NoSymbol)
		return;
	if (!SimulateKeyReleaseEvent(disp, key)) {
		delete("XTEST extension not configured or in use");
		return;
	}
	XSync(disp, False);
	debug(1, "Key release %d", key);

	/*
	 * Remove the corresponding key down.
	 */
	for (i = keyind-1; i >= 0; i--) {
		if (keys[i].key == key) {
			keys[i].disp = 0;
			break;
		}
	}
}

void
devicekeyrel(disp, dev, key)
Display *disp;
XDevice *dev;
int     key;
{
int 	i;

	if (key == NoSymbol)
		return;
	if (!SimulateDeviceKeyReleaseEvent(disp, dev, key)) {
		delete("XTEST extension not configured or in use");
		return;
	}
	XSync(disp, False);
	debug(1, "Key release %d", key);

	/*
	 * Remove the corresponding key down.
	 */
	for (i = keyind-1; i >= 0; i--) {
		if (keys[i].key == key) {
			keys[i].disp = 0;
			break;
		}
	}
}

/*
 * Release buttons pressed with buttonpress, in reverse order of pressing.
 */
void
relbuttons()
{
int 	i;

	for (i = butind-1; i >= 0; i--) {
		if (buttons[i].disp)
			buttonrel(buttons[i].disp, buttons[i].button);
	}
	butind = 0;
}

void
devicerelbuttons(dev)
XDevice *dev;
{
int 	i;

	for (i = butind-1; i >= 0; i--) {
		if (buttons[i].disp)
			devicebuttonrel(buttons[i].disp, dev, buttons[i].button);
	}
	butind = 0;
}

/*
 * Release keys pressed with keypress, in reverse order of pressing.
 */
void
relkeys()
{
int 	i;

	for (i = keyind-1; i >= 0; i--) {
		if (keys[i].disp)
			keyrel(keys[i].disp, keys[i].key);
	}
	keyind = 0;
}
devicerelkeys(dev)
XDevice *dev;
{
int 	i;

	for (i = keyind-1; i >= 0; i--) {
		if (keys[i].disp)
			devicekeyrel(keys[i].disp, dev, keys[i].key);
	}
	keyind = 0;
}

/*
 * Release all buttons and keys, buttons first.  It doesn't matter
 * if none are currently pressed (applies to all the release functions.)
 */
void
relalldev()
{
	relbuttons();
	relkeys();
}

#define	NMODS	8	/* Number of modifiers */

static	XModifierKeymap	*origmap;
static	XModifierKeymap	*curmap;
static	XModifierKeymap	*devcurmap;

/*
 * Given the number of modifiers that you want it returns a modifier
 * key mask with that number of bits set if possible.  Each bit is
 * a valid modifier for the server.  If necessary the modifer map is
 * set up with extra keycodes.
 * If the server is really limited in how many keycodes it allows
 * to be used as modifiers then you may not get as many modifiers as
 * you want.  You should insure that the test will still work in this
 * case.
 */
unsigned int
_wantmods(disp, dev, want)
Display	*disp;
XDevice *dev;
int 	want;
{
unsigned int 	mask;
int 	nmods;
int 	i;

	if (curmap == NULL)
	    if (dev == NULL)
		curmap = XGetModifierMapping(disp);
	    else
		curmap = XGetDeviceModifierMapping(disp, dev);
	if (curmap == NULL)
		return(0);

	mask = 0;
	nmods = 0;

	/*
	 * Find out what is currently available.
	 */
	for (i = 0; i < NMODS*curmap->max_keypermod; i++) {
		if (curmap->modifiermap[i])
			mask |= 1 << (i/curmap->max_keypermod);
	}

	/*
	 * If we want more modifiers than are currently in the map then
	 * try to add some.  There is no guarantee that this can be done though.
	 */
	nmods = bitcount(mask);
	if (nmods < want) {
		/*
		 * Exercise for the reader...
		 */
		untested("Unimplemented modmap expansion in wantmods");
	}

	/* I want, but I can't get */
	if (nmods < want)
		want = nmods;

	/*
	 * Put the wanted number of modifiers into the return mask
	 */
	for (i = 0; i < NMODS; i++) {
		if (mask & (1 << i))
			want--;
		if (want < 0)
			mask &= ~(1 << i);
	}

	return(mask);
}

unsigned int
wantdevmods(disp, dev, want)
Display	*disp;
XDevice *dev;
int 	want;
{
	return (_wantmods(disp, dev, want));
}

unsigned int
wantmods(disp, want)
Display	*disp;
int 	want;
{
	return (_wantmods(disp, NULL, want));
}

static void modthing(/* disp, mask */);
static void devmodthing(/* disp, dev, mask */);

/*
 * Simulate pressing a bunch of modifier keys.  The mask passed to this
 * function should be made of bits obtained by a previous call to wantmods,
 * to ensure that they have assigned keycodes.
 */
modpress(disp, mask)
Display	*disp;
unsigned int 	mask;
{
	modthing(disp, mask, True);
}

devmodpress(disp, dev, mask)
Display	*disp;
XDevice *dev;
unsigned int 	mask;
{
	devmodthing(disp, dev, mask, True);
}

/*
 * Simulate releasing a bunch of modifier keys.  The mask passed to this
 * function should be made of bits obtained by a previous call to wantmods,
 * to ensure that they have assigned keycodes.
 */
modrel(disp, mask)
Display	*disp;
unsigned int 	mask;
{
	modthing(disp, mask, False);
}

devmodrel(disp, dev, mask)
Display	*disp;
XDevice *dev;
unsigned int 	mask;
{
	devmodthing(disp, dev, mask, False);
}

static void
modthing(disp, mask, pressing)
Display	*disp;
unsigned int 	mask;
int pressing;
{
int 	mod;
int 	ent;
void	(*func)();

	if (curmap == NULL) {
		delete("Programming error: wantmods() not called");
		return;
	}

	if (pressing)
		func = keypress;
	else
		func = keyrel;

	for (mod = 0; mod < NMODS; mod++) {
		if (mask & (1 << mod))
			{
			for (ent=mod*curmap->max_keypermod; 
			     ent<(mod+1) *curmap->max_keypermod; ent++)
			    if (curmap->modifiermap[ent]) {
				(*func)(disp, curmap->modifiermap[ent]);
				break;
			    }
			/*
			(*func)(disp, curmap->modifiermap[mod*curmap->max_keypermod]);
			*/
			}
	}
}

static void
devmodthing(disp, dev, mask, pressing)
Display	*disp;
unsigned int 	mask;
int pressing;
{
int 	mod;
void	(*func)();

	if (curmap == NULL) {
		delete("Programming error: wantmods() not called");
		return;
	}

	if (dev)
	    if (pressing)
		func = devicekeypress;
	    else
		func = devicekeyrel;

	for (mod = 0; mod < NMODS; mod++) {
		if (mask & (1 << mod))
			(*func)(disp, dev, devcurmap->modifiermap[mod*devcurmap->max_keypermod]);
	}
}

/*
 * Check if a keycode corresponds to any of mods in a mask, returned by
 * wantmods.
 */
ismodkey(mask, kc)
unsigned int 	mask;
int kc;
{
int 	mod;

	if (curmap == NULL) {
		delete("Programming error: wantmods() not called");
		return False;
	}
	if (!mask || kc < 8 || kc > 255)
		return False;

	for (mod = 0; mod < NMODS; mod++) {
		if (mask & (1 << mod))
			if (curmap->modifiermap[mod*curmap->max_keypermod] == kc)
				return True;
	}
	return False;
}

/*
 * This routine should be called at the end of a test after any of the
 * device press routines have been called.
 */
restoredevstate()
{
extern	Display	*Dsp;

	relalldev();
	if (origmap)
		XSetModifierMapping(Dsp, origmap);
}

/*
 * Returns True if we don't want to do extended testing for any reason.
 */
noext(needbutton)
int 	needbutton;
{

	if (config.extensions == False) {
		untested("Extended testing not required");
		return True;
	} else if (IsExtTestAvailable() == False) {
		untested("Server does not support XTEST extension");
		untested("or test suite not configured to use XTEST extension");
		return True;
	} else if (needbutton && nbuttons() == 0) {
		untested("No buttons exist on the server");
		return True;
	}
	return False;
}


/*
 * Returns the number of physical buttons.
 */
nbuttons()
{
static int 	Nbuttons = -1;
unsigned	char	pmap[5];
extern	Display	*Dsp;

	if (Nbuttons == -1)
		Nbuttons = XGetPointerMapping(Dsp, pmap, sizeof(pmap));
	return Nbuttons;
}

/*
 * Returns a valid keycode for the server.  A different one is returned
 * every time (until it wraps round).
 */
getkeycode(display)
Display	*display;
{
static	int 	minkc, maxkc;
static	int 	curkey;

	XDisplayKeycodes(display, &minkc, &maxkc);
	if (minkc < 8)
		minkc = 8;
	if (!curkey)
		curkey = minkc;

	if (curkey > maxkc)
		curkey = minkc;

	return curkey++;
}
