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
 * $XConsortium: dltmdfrmpe.m,v 1.5 94/04/17 21:06:16 rws Exp $
 */
>>TITLE XDeleteModifiermapEntry CH07
XModifierKeymap *
xname
XModifierKeymap *modmap;
KeyCode	keycode_entry;
int 	modifier;
>>ASSERTION Good A
A call to xname
deletes the specified KeyCode,
.A keycode_entry ,
from the set that controls the specified
.A modifier
and returns a pointer to the modified
.S XModifierKeymap
structure.
>>STRATEGY
Create modifiermap.
Insert some entries into it.
Call xname to delete entries.
Verify by direct inspection that the entries are gone.
>>CODE
int 	minkc, maxkc;
int 	kc;
int 	rm1, rm2;
int 	set;
int 	mod;
KeyCode	*kcp;
static int 	mods[] = {
	ShiftMapIndex,
	LockMapIndex,
	ControlMapIndex,
	Mod1MapIndex,
	Mod2MapIndex,
	Mod3MapIndex,
	Mod4MapIndex,
	Mod5MapIndex,
};

	XDisplayKeycodes(Dsp, &minkc, &maxkc);
	kc = minkc;

	modmap = XNewModifiermap(0);

	modmap = XInsertModifiermapEntry(modmap, ++kc, ControlMapIndex);
	rm1 = kc;
	modmap = XInsertModifiermapEntry(modmap, ++kc, ControlMapIndex);
	modmap = XInsertModifiermapEntry(modmap, ++kc, ControlMapIndex);
	modmap = XInsertModifiermapEntry(modmap, ++kc, Mod2MapIndex);
	modmap = XInsertModifiermapEntry(modmap, ++kc, Mod2MapIndex);
	rm2 = kc;
	modmap = XInsertModifiermapEntry(modmap, ++kc, Mod2MapIndex);
	modmap = XInsertModifiermapEntry(modmap, ++kc, Mod2MapIndex);

	/* Remove two entries */
	keycode_entry = rm1;
	modifier = ControlMapIndex;
	modmap = XCALL;

	keycode_entry = rm2;
	modifier = Mod2MapIndex;
	modmap = XCALL;

	kcp = modmap->modifiermap;
	for (mod = 0; mod < NELEM(mods); mod++) {
		for (set = 0; set < modmap->max_keypermod; set++) {
			if (kcp[0] == rm1) {
				report("KeyCode %d still in map", rm1);
				FAIL;
			} else
				CHECK;
			if (kcp[0] == rm2) {
				report("KeyCode %d still in map", rm2);
				FAIL;
			} else
				CHECK;

			kcp++;
		}
	}

	CHECKPASS(2 * modmap->max_keypermod * NELEM(mods));
