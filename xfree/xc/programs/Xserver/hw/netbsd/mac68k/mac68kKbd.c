/*
 * Copyright 1996 Scott Reynolds
 * Copyright (C) 1994 Bradley A. Grantham and Lawrence A. Kesteloot
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The names of the Alice Group or any of its members may not be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE ALICE GROUP ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE ALICE GROUP BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "mac68k.h"
#include "Xproto.h"
#include "inputstr.h"

#define MIN_KEYCODE	8	/* necessary to avoid the mouse buttons */
#define MAX_KEYCODE	255	/* limited by the protocol */

extern MacModmapRec	*macModMaps[];	/* Borrowed from A/UX code */
extern KeySymsRec	macKeySyms[];	/* Borrowed from A/UX code */


void mac68k_bell()
{
	static int itefd = (-1);

        if (itefd == (-1))
                if ((itefd = open( "/dev/ttye0", O_RDONLY )) == (-1))
			itefd = (-2);

	if (itefd >= 0)
		ioctl(itefd, ITEIOC_RINGBELL);
}


void mac68k_kbdctrl()
{
}


#if NeedFunctionPrototypes
int mac68k_kbdproc (
	DevicePtr	kbd,
	int		what)
#else
int mac68k_kdbproc (kbd, what)
	DevicePtr	kbd;	/* Keyboard to manipulate */
	int		what;	/* What to do to it */
#endif
{
	static CARD8 *workingModMap = NULL;
	static KeySymsRec *workingKeySyms;
	int i;

	switch (what) {
	case DEVICE_INIT:
		if (kbd != LookupKeyboardDevice()) {
			ErrorF ("Cannot open non-system keyboard\n");
			return (!Success);
		}
	    
		if (!workingKeySyms) {
			workingKeySyms = macKeySyms;

			if (workingKeySyms->minKeyCode < MIN_KEYCODE) {
				workingKeySyms->minKeyCode += MIN_KEYCODE;
				workingKeySyms->maxKeyCode += MIN_KEYCODE;
			}
#if 0
			/*
			 * maxKeyCode is eight bits long, MAX_KEYCODE is
			 * 255.  This check is unneeded and gcc catches
			 * it as a warning.
			 */
			if (workingKeySyms->maxKeyCode > MAX_KEYCODE)
				workingKeySyms->maxKeyCode = MAX_KEYCODE;
#endif
		}

		if (!workingModMap) {
			workingModMap = (CARD8 *)xalloc(MAP_LENGTH);
			(void) memset(workingModMap, 0, MAP_LENGTH);
			for (i = 0; macModMaps[1][i].key != 0; i++)
				workingModMap[macModMaps[1][i].key+MIN_KEYCODE]
				    = macModMaps[1][i].modifiers;
		}

		kbd->on = FALSE;

		InitKeyboardDeviceStruct(kbd, workingKeySyms,
			workingModMap, mac68k_bell, mac68k_kbdctrl);
		break;

	case DEVICE_ON:
		kbd->on = TRUE;
		break;

	case DEVICE_OFF:
		kbd->on = FALSE;
		break;

	case DEVICE_CLOSE:
		/* nothing! */
		break;

	default:
		FatalError("Unknown keyboard operation\n");
	}

	return Success;
}


void mac68k_getkbd(
	void)
{
	int index;

	/* Find out where there is a keyboard */

	/* FatalError("Cannot run X server without a keyboard.\n"); */
}


void mac68k_processkbd(
	DeviceIntPtr	kbd,
	adb_event_t	*event)
{
	xEvent	xev;

	mac_lasteventtime = xev.u.keyButtonPointer.time =
		TVTOMILLI(event->timestamp);

	xev.u.u.type = (event->u.k.key & 0x80) ? KeyRelease : KeyPress;
	xev.u.u.detail = (event->u.k.key & 0x7f) + MIN_KEYCODE;

	mieqEnqueue(&xev);
}


Bool LegalModifier(
	unsigned int key,
	DevicePtr dev)
{
	return(TRUE);
}
