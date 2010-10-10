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

/*
 * Modified from  hpcKbd.c of Xhpc 
 */

#define NEED_EVENTS
#include "mi.h"
#include "dreamcast.h"
#include "keysym.h"
#include <stdio.h>
#include <sys/time.h>

extern KeySymsRec dreamcastKeySyms[];
extern dreamcastModmapRec *dreamcastModMaps[];

/*
 * dreamcastBell --
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
dreamcastBell(int percent, DeviceIntPtr device, pointer ctrl, int unused)
/*  int		    percent;	       Percentage of full volume */
/*  DeviceIntPtr    device;	       Keyboard to ring */
{
	/* None */
}

/*
 * dreamcastKbdCtrl --
 *	Alter some of the keyboard control parameters
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Some...
 */
static void
dreamcastKbdCtrl(DeviceIntPtr device, KeybdCtrl *ctrl)
/*  DeviceIntPtr    device;	       Keyboard to alter */
{
    dreamcastKbdPrivPtr pPriv = (dreamcastKbdPrivPtr) device->public.devicePrivate;

    if (pPriv->fd < 0) return;

    /* Bell info change needs nothing done here. */
}

/*
 * dreamcastKbdProc --
 *	Handle the initialization, etc. of a keyboard.
 *
 * Results:
 *	None.
 */
int
dreamcastKbdProc(DeviceIntPtr device, int what)
/*    DeviceIntPtr  device;	   Keyboard to manipulate */
/*    int    	  what;	    	   What to do to it */
{
    int i;
    DevicePtr pKeyboard = (DevicePtr) device;
    dreamcastKbdPrivPtr pPriv;

    static CARD8 *workingModMap = NULL;
    static KeySymsRec *workingKeySyms;

    switch (what) {
    case DEVICE_INIT:
	if (pKeyboard != LookupKeyboardDevice()) {
	    dreamcastErrorF (("Cannot open non-system keyboard\n"));
	    return (!Success);
	}

	if (!workingKeySyms) {
	    workingKeySyms = &dreamcastKeySyms[dreamcastKbdPriv.layout];

#if MIN_KEYCODE > 0
	    if (workingKeySyms->minKeyCode < MIN_KEYCODE) {
		workingKeySyms->minKeyCode += MIN_KEYCODE;
		workingKeySyms->maxKeyCode += MIN_KEYCODE;
	    }
#endif
	}

	if (!workingModMap) {
	    workingModMap=(CARD8 *)xalloc(MAP_LENGTH);
	    (void) memset(workingModMap, 0, MAP_LENGTH);
	    for (i = 0; dreamcastModMaps[dreamcastKbdPriv.layout][i].key != 0; i++)
		workingModMap[dreamcastModMaps[dreamcastKbdPriv.layout][i].key + MIN_KEYCODE] =
		    dreamcastModMaps[dreamcastKbdPriv.layout][i].modifiers;
	}

	(void) memset ((void *) defaultKeyboardControl.autoRepeats,
			~0, sizeof defaultKeyboardControl.autoRepeats);

	pKeyboard->devicePrivate = (pointer)&dreamcastKbdPriv;
	pKeyboard->on = FALSE;

	InitKeyboardDeviceStruct(pKeyboard,
				 workingKeySyms, workingModMap,
				 dreamcastBell, dreamcastKbdCtrl);
	break;

    case DEVICE_ON:
	pPriv = (dreamcastKbdPrivPtr)pKeyboard->devicePrivate;
	dreamcastCleanupFd(pPriv->fd);
#ifdef WSKBDIO_SETVERSION
	{
	    int version = WSKBDIO_EVENT_VERSION;
	    if (ioctl(pPriv->fd, WSKBDIO_SETVERSION, &version) == -1) {
		Error ("dreamcastKbdProc ioctl WSKBDIO_SETVERSION");
		return !Success;
	    }
	}
#endif
	AddEnabledDevice(pPriv->fd);
	pKeyboard->on = TRUE;
	break;

    case DEVICE_CLOSE:
    case DEVICE_OFF:
	pPriv = (dreamcastKbdPrivPtr)pKeyboard->devicePrivate;
	RemoveEnabledDevice(pPriv->fd);
	pKeyboard->on = FALSE;
	break;
    default:
	dreamcastFatalError(("Unknown keyboard operation\n"));
    }
    return Success;
}

/*
 * dreamcastKbdGetEvents --
 *	Return the events waiting in the wings for the given keyboard.
 *
 * Results:
 *	A pointer to an array of dreamcastEvents or (dreamcastEvent *)0 if no events
 *	The number of events contained in the array.
 *	A boolean as to whether more events might be available.
 *
 * Side Effects:
 *	None.
 */
dreamcastEvent*
dreamcastKbdGetEvents(pPriv, pNumEvents, pAgain)
    dreamcastKbdPrivPtr pPriv;
    int*	pNumEvents;
    Bool*	pAgain;
{
    int fd;
    int	nBytes;	    /* number of bytes of events available. */
    static dreamcastEvent evBuf[MAXEVENTS];   /* Buffer for dreamcastEvents */

    fd = pPriv->fd;
	if ((nBytes = read(fd, evBuf, sizeof(evBuf))) == -1) {
	    if (errno == EWOULDBLOCK) {
		*pNumEvents = 0;
		*pAgain = FALSE;
	    } else {
		dreamcastError ("Reading keyboard");
		dreamcastFatalError (("Could not read the keyboard"));
	    }
	} else {
	    *pNumEvents = nBytes / sizeof (dreamcastEvent);
	    *pAgain = (nBytes == sizeof (evBuf));
	}
    return evBuf;
}

/*
 * dreamcastKbdEnqueueEvent --
 */
void
dreamcastKbdEnqueueEvent (device, fe)
    DeviceIntPtr  device;
    dreamcastEvent	  *fe;
{
    xEvent		xE;
    BYTE		keycode;
    CARD8		keyModifiers;

    keycode = (fe->value & 0xff) + MIN_KEYCODE;

    keyModifiers = device->key->modifierMap[keycode];
    xE.u.keyButtonPointer.time = TSTOMILLI(fe->time);
    xE.u.u.type = ((fe->type == WSCONS_EVENT_KEY_UP) ? KeyRelease : KeyPress);
    xE.u.u.detail = keycode;

    mieqEnqueue (&xE);
}

/*ARGSUSED*/
Bool LegalModifier(key, pDev)
    unsigned int key;
    DevicePtr	pDev;
{
    return TRUE;
}
