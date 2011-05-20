/* $XConsortium: sunKbd.c,v 5.47 94/08/16 13:45:30 dpw Exp $ */
/*-
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 */

/************************************************************
Copyright 1987 by Sun Microsystems, Inc. Mountain View, CA.

                    All Rights Reserved

Permission  to  use,  copy,  modify,  and  distribute   this
software  and  its documentation for any purpose and without
fee is hereby granted, provided that the above copyright no-
tice  appear  in all copies and that both that copyright no-
tice and this permission notice appear in  supporting  docu-
mentation,  and  that the names of Sun or X Consortium
not be used in advertising or publicity pertaining to
distribution  of  the software  without specific prior
written permission. Sun and X Consortium make no
representations about the suitability of this software for
any purpose. It is provided "as is" without any express or
implied warranty.

SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#define	NEED_EVENTS
#include "ews4800mips.h"
#include "keysym.h"
#include "mi.h"
#include <stdio.h>
#include <sys/time.h>
#include <X11/extensions/XKB.h>

extern KeySymsRec ews4800mipsKeySyms[];
extern ews4800mipsModmapRec *ews4800mipsModMaps[];

/*
 * ews4800mipsBell --
 *	Ring the terminal/keyboard bell
 *
 * Results:
 *	Ring the keyboard bell for an amount of time proportional to
 *	"loudness."
 *
 * Side Effects:
 *	None, really...
 */
static void
ews4800mipsBell(int percent, /* Percentage of full volume */
    DeviceIntPtr device, pointer ctrl, int unused)
{
	/* None */
}

/*
 * ews4800mipsKbdCtrl --
 *	Alter some of the keyboard control parameters
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Some...
 */
static void
ews4800mipsKbdCtrl(DeviceIntPtr device, KeybdCtrl *ctrl)
{

	ews4800mipsKbdPrivPtr pPriv =
	    (ews4800mipsKbdPrivPtr)device->public.devicePrivate;

	if (pPriv->fd < 0)
		return;

	/* Bell info change needs nothing done here. */
}

/*
 * ews4800mipsKbdProc --
 *	Handle the initialization, etc. of a keyboard.
 *
 * Results:
 *	None.
 */
int
ews4800mipsKbdProc(DeviceIntPtr  device, int what)
{
	int i;
	DevicePtr pKeyboard = (DevicePtr) device;
	ews4800mipsKbdPrivPtr pPriv;
	KeybdCtrl *ctrl = &device->kbdfeed->ctrl;
	extern int XkbDfltRepeatDelay, XkbDfltRepeatInterval;
	struct termios tkbdtty;

	static CARD8 *workingModMap = NULL;
	static KeySymsRec *workingKeySyms;

	switch (what) {
	case DEVICE_INIT:
		if (pKeyboard != LookupKeyboardDevice()) {
			ews4800mipsErrorF(("Cannot open non-system"
			    " keyboard\n"));
			return (!Success);
		}

		if (!workingKeySyms) {
			workingKeySyms =
			    &ews4800mipsKeySyms[ews4800mipsKbdPriv.layout];
			if (workingKeySyms->minKeyCode < XkbMinLegalKeyCode) {
				workingKeySyms->minKeyCode +=
				    XkbMinLegalKeyCode;
				workingKeySyms->maxKeyCode +=
				    XkbMinLegalKeyCode;
			}
		}

		if (!workingModMap) {
			int layout = ews4800mipsKbdPriv.layout;
			ews4800mipsModmapRec *rec = ews4800mipsModMaps[layout];
			workingModMap=(CARD8 *)xalloc(MAP_LENGTH);
			memset(workingModMap, 0, MAP_LENGTH);
			for (i = 0; rec[i].key != 0; i++)
				workingModMap[rec[i].key + XkbMinLegalKeyCode] =
				    rec[i].modifiers;
		}

		(void) memset ((void *) defaultKeyboardControl.autoRepeats,
		    ~0, sizeof defaultKeyboardControl.autoRepeats);

		pKeyboard->devicePrivate = (pointer)&ews4800mipsKbdPriv;
		pKeyboard->on = FALSE;

		InitKeyboardDeviceStruct(pKeyboard,
		    workingKeySyms, workingModMap,
		    ews4800mipsBell, ews4800mipsKbdCtrl);
		break;

	case DEVICE_ON:
		pPriv = (ews4800mipsKbdPrivPtr)pKeyboard->devicePrivate;
		ews4800mipsCleanupFd(pPriv->fd);
#ifdef WSKBDIO_SETVERSION
		{
			int version = WSKBDIO_EVENT_VERSION;
			if (ioctl(pPriv->fd, WSKBDIO_SETVERSION, &version) == -1) {
				Error ("ews4800mipsKbdProc ioctl WSKBDIO_SETVERSION");
				return !Success;
			}
		}
#endif
		AddEnabledDevice(pPriv->fd);
		pKeyboard->on = TRUE;
		break;

	case DEVICE_CLOSE:
	case DEVICE_OFF:
		pPriv = (ews4800mipsKbdPrivPtr)pKeyboard->devicePrivate;
		RemoveEnabledDevice(pPriv->fd);
		pKeyboard->on = FALSE;
		break;
	default:
		ews4800mipsFatalError(("Unknown keyboard operation\n"));
	}

	return Success;
}

/*
 * ews4800mipsKbdGetEvents --
 *	Return the events waiting in the wings for the given keyboard.
 *
 * Results:
 *	A pointer to an array of ews4800mipsEvents or
 *	(ews4800mipsEvent *)0 if no events The number of events
 *	contained in the array.
 *	A boolean as to whether more events might be available.
 *
 * Side Effects:
 *	None.
 */
ews4800mipsEvent*
ews4800mipsKbdGetEvents(ews4800mipsKbdPrivPtr pPriv, int *pNumEvents,
    Bool *pAgain)
{
	static ews4800mipsEvent evBuf[MAXEVENTS];
	int fd, n;
	u_char c, c2;

	fd = pPriv->fd;
	if ((n = read(fd, evBuf, sizeof(evBuf))) == -1) {
		if (errno == EWOULDBLOCK) {
			*pNumEvents = 0;
			*pAgain = FALSE;
		} else {
			ews4800mipsError("Reading keyboard");
			ews4800mipsFatalError(("Could not read the keyboard"));
		}
	} else {
		*pNumEvents = n / sizeof(ews4800mipsEvent);
		*pAgain = n == sizeof(evBuf);
	}

	return evBuf;
}

void
ews4800mipsKbdEnqueueEvent(DeviceIntPtr device, ews4800mipsEvent *fe)
{
	xEvent event;

	event.u.keyButtonPointer.time = TSTOMILLI(fe->time);
	event.u.u.type =
	    (fe->type == WSCONS_EVENT_KEY_UP) ? KeyRelease : KeyPress;
	event.u.u.detail = (fe->value & 0xff) + XkbMinLegalKeyCode;

	mieqEnqueue(&event);
}

Bool LegalModifier(unsigned int key, DevicePtr pDev)
{

	return TRUE;
}
