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
 * $XConsortium: rfrshkybrd.m,v 1.10 94/04/17 21:10:02 rws Exp $
 */
>>TITLE XRefreshKeyboardMapping CH10

XRefreshKeyboadMapping(event_map)
XMappingEvent	*event_map;
>>EXTERN
#define XK_LATIN1
#include	"keysymdef.h"
#undef XK_LATIN1

static void
cpymap(dest, src)
	XModifierKeymap *dest;
	XModifierKeymap *src;
{
	int i;
	int n;

	for(i=0,n=src->max_keypermod*8; i<n; i++)
		dest->modifiermap[i] = src->modifiermap[i];
	dest->max_keypermod = src->max_keypermod;
}
>>ASSERTION Good A
When the
.M request
component of the
.A event_map
argument is
.S MappingKeyboard ,
then a call to xname modifies the keymap information based on that specified
by the
.A event_map
argument.
>>STRATEGY
Map the keycode which maps to KeySym XK_a onto XK_b using XChangeKeyboardMapping.
Verify that a MappingNotify event is generated.
Update the keymap information with xname.
Obtain the KeySym corresponding to the keycode.
Verify that KeySym is XK_b.
Reset the keycode to map onto XK_a using XChangeKeyboardMapping.
>>CODE
XEvent		ev;
KeyCode		kc;
KeySym		ks, res, old, new;
XKeyEvent	ke;

	old = XK_a;
	new = XK_b;

	if((kc = XKeysymToKeycode(Dsp, old)) == 0) {
		delete("XKeysymToKeycode() returned 0 for KeySym %lu.", (long) old);
		return;
	} else
		CHECK;

	XChangeKeyboardMapping(Dsp, kc, 1, &new, 1);
	XSync(Dsp, False);
	
	if( getevent(Dsp, &ev) == 0) {
		delete("Did not get an event after an XChangeKeyboardMapping()");
		XChangeKeyboardMapping(Dsp, kc, 1, &old, 1);
		return;
	} else
		CHECK;

	if(ev.type != MappingNotify) {
		delete("Got a %s event instead of a %s.", eventname(ev.type), eventname(MappingNotify));
		XChangeKeyboardMapping(Dsp, kc, 1, &old, 1);
		return;
	} else
		CHECK;

	if(ev.xmapping.request != MappingKeyboard) {
		delete("The request field was %d instead of %d (MappingKeyboard)",
				ev.xmapping.request, MappingKeyboard);
		XChangeKeyboardMapping(Dsp, kc, 1, &old, 1);
		return;
	} else
		CHECK;

	event_map = &ev.xmapping;
	XCALL;

	if((res = XKeycodeToKeysym(Dsp, kc, 0)) == NoSymbol) {
		delete("XKeycodeToKeysym() returned NoSymbol for KeyCode %lu.", (long) kc);
		XChangeKeyboardMapping(Dsp, kc, 1, &old, 1);
		return;
	} else
		CHECK;

	if( res != new) {
		report("%s() mapped KeyCode %lu to KeySym %lu instead of %lu.", TestName, (long) kc, (long) res, (long) new);
		FAIL;
	} else
		CHECK;

	XChangeKeyboardMapping(Dsp, kc, 1, &old, 1);
	CHECKPASS(6);

>>ASSERTION Good B 1
When the
.M request
component of the
.A event_map
argument is
.S MappingModifier ,
then a call to xname modifies the keymap information based on that specified
by the
.A event_map
argument.
>>STRATEGY
Obtain a new modifier map using XNewModifiermap.
Initialise it to have the current, valid, values.
Set the modifier map to the new map using XSetModifierMapping.
Await the mapping event.
Update the modifier mappings using xname.
Release the map storage using XFreeModifiermap.
>>CODE
int		i;
XEvent		ev;
KeyCode		*kcptr;
KeyCode		*tkcptr;
XModifierKeymap	*savemap = XGetModifierMapping(Dsp);
XModifierKeymap	*mmap;

	if(savemap == (XModifierKeymap *) NULL) {
		delete("XGetModifierMapping() returned NULL.");
		return;
	} else
		CHECK;

	mmap = XNewModifiermap(savemap->max_keypermod);

	if(mmap == (XModifierKeymap *) NULL) {
		delete("XNewModifiermap() returned NULL.");
		return;
	} else
		CHECK;

	cpymap(mmap, savemap);

	if(XSetModifierMapping(Dsp, mmap) != MappingSuccess) {
		delete("XSetModifierMapping() did not return MappingSuccess.");
		return;
	} else
		CHECK;

	XSync(Dsp, False);
	
	if( getevent(Dsp, &ev) == 0) {
		delete("Did not get an event after an XSetModifierMapping()");
		XSetModifierMapping(Dsp, savemap);
		return;
	} else
		CHECK;

	if(ev.type != MappingNotify) {
		delete("Got a %s event instead of a %s.", eventname(ev.type), eventname(MappingNotify));
		XSetModifierMapping(Dsp, savemap);
		return;
	} else
		CHECK;

	if(ev.xmapping.request != MappingModifier) {
		delete("The request field was %d instead of %d (MappingModifier)",
				ev.xmapping.request, MappingModifier);
		XSetModifierMapping(Dsp, savemap);
		return;
	} else
		CHECK;

	event_map = &ev.xmapping;
	XCALL;

	XSetModifierMapping(Dsp, savemap);

	XFreeModifiermap(mmap);
	XFreeModifiermap(savemap);
	CHECKUNTESTED(6);

>>ASSERTION Good B 1
When the
.M request
component of the
.A event_map
argument is other than
.S MappingKeyboard
or
.S MappingModifier ,
then a call to xname does not modify the keymap information.
>>STRATEGY
Obtain a new modifier map using XNewModifiermap.
Initialise it to have the current, valid, values.
Set the modifier map to the new map using XSetModifierMapping.
Await the mapping event.
Set the request component of the event structure to MappingModifier + MappingKeyboard + MappingPointer
Update the modifier mappings using xname.
>>CODE
int		i;
int		set;
XEvent		ev;
XModifierKeymap	*savemap = XGetModifierMapping(Dsp);
XModifierKeymap	*mmap1;
KeyCode		minkc;
KeyCode		maxkc;
KeyCode		*kcp;

	if(savemap == (XModifierKeymap *) NULL) {
		delete("XGetModifierMapping() returned NULL.");
		return;
	} else
		CHECK;

	mmap1 = XNewModifiermap(savemap->max_keypermod);

	if(mmap1 == (XModifierKeymap *) NULL) {
		delete("XNewModifiermap() returned NULL.");
		return;
	} else
		CHECK;

	cpymap(mmap1, savemap);

	if(XSetModifierMapping(Dsp, mmap1) != MappingSuccess) {
		delete("XSetModifierMapping() did not return MappingSuccess.");
		return;
	} else
		CHECK;

	XSync(Dsp, False);

	if( getevent(Dsp, &ev) == 0) {
		delete("Did not get an event after an XSetModifierMapping()");
		XSetModifierMapping(Dsp, savemap);
		return;
	} else
		CHECK;

	if(ev.type != MappingNotify) {
		delete("Got a %s event instead of a %s.", eventname(ev.type), eventname(MappingNotify));
		XSetModifierMapping(Dsp, savemap);
		return;
	} else
		CHECK;

	ev.xmapping.request = MappingModifier + MappingKeyboard + 1;
	event_map = &ev.xmapping;
	XCALL;

	XSetModifierMapping(Dsp, savemap);

	XFreeModifiermap(mmap1);
	XFreeModifiermap(savemap);
	CHECKUNTESTED(5);
