/* $NetBSD: hpcKbd.c,v 1.1 2000/05/06 06:01:49 takemura Exp $	*/
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

#define NEED_EVENTS
#include "hpc.h"
#include "keysym.h"
#include <stdio.h>
#include <sys/time.h>
#include <dev/pckbc/pckbdreg.h>

#define MIN_KEYCODE	8	/* necessary to avoid the mouse buttons */ /* XXX */
#define MAX_KEYCODE	255	/* limited by the protocol */ /* XXX */

extern KeySymsRec hpcKeySyms[];
extern hpcModmapRec *hpcModMaps[];

/*
 * hpcBell --
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
hpcBell(percent, device, ctrl, unused)
    int		    percent;	    /* Percentage of full volume */
    DeviceIntPtr    device;	    /* Keyboard to ring */
    pointer	    ctrl;
    int		    unused;
{
#if 0
    KeybdCtrl*      kctrl = (KeybdCtrl*) ctrl;
    hpcKbdPrivPtr   pPriv = (hpcKbdPrivPtr) device->public.devicePrivate;
    struct wsconsio_bell_data wbd;

    if (percent == 0 || kctrl->bell == 0)
 	return;

    wbd.wbd_flags = WSCONSIO_BELLDATA_VOLUME | WSCONSIO_BELLDATA_PITCH |
	WSCONSIO_BELLDATA_PERIOD;
    wbd.wbd_volume = percent;
    wbd.wbd_pitch = kctrl->bell_pitch;
    wbd.wbd_period = kctrl->bell_duration;

    if (ioctl (pPriv->fd, WSCONSIO_COMPLEXBELL, &wbd) == -1) {
 	Error("Failed to activate bell");
	return;
    }
#endif
}

/*
 * hpcKbdCtrl --
 *	Alter some of the keyboard control parameters
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Some...
 */
static void
hpcKbdCtrl(device, ctrl)
    DeviceIntPtr    device;	    /* Keyboard to alter */
    KeybdCtrl*	    ctrl;
{
    hpcKbdPrivPtr pPriv = (hpcKbdPrivPtr) device->public.devicePrivate;

    if (pPriv->fd < 0) return;

    /* Bell info change needs nothing done here. */
}

/*
 * hpcKbdProc --
 *	Handle the initialization, etc. of a keyboard.
 *
 * Results:
 *	None.
 */
int
hpcKbdProc(device, what)
    DeviceIntPtr  device;	/* Keyboard to manipulate */
    int	    	  what;	    	/* What to do to it */
{
    int i;
    DevicePtr pKeyboard = (DevicePtr) device;
    hpcKbdPrivPtr pPriv;
    KeybdCtrl*	ctrl = &device->kbdfeed->ctrl;
    extern int XkbDfltRepeatDelay, XkbDfltRepeatInterval;
    struct termios tkbdtty;

    static CARD8 *workingModMap = NULL;
    static KeySymsRec *workingKeySyms;

    switch (what) {
    case DEVICE_INIT:
	if (pKeyboard != LookupKeyboardDevice()) {
	    ErrorF ("Cannot open non-system keyboard\n");
	    return (!Success);
	}

	if (!workingKeySyms) {
	    workingKeySyms = &hpcKeySyms[hpcKbdPriv.type];

#if MIN_KEYCODE > 0
	    if (workingKeySyms->minKeyCode < MIN_KEYCODE) {
		workingKeySyms->minKeyCode += MIN_KEYCODE;
		workingKeySyms->maxKeyCode += MIN_KEYCODE;
	    }
#endif
	    if (workingKeySyms->maxKeyCode > MAX_KEYCODE)
		workingKeySyms->maxKeyCode = MAX_KEYCODE;
	}

	if (!workingModMap) {
	    workingModMap=(CARD8 *)xalloc(MAP_LENGTH);
	    (void) memset(workingModMap, 0, MAP_LENGTH);
	    for (i = 0; hpcModMaps[hpcKbdPriv.type][i].key != 0; i++)
		workingModMap[hpcModMaps[hpcKbdPriv.type][i].key + MIN_KEYCODE] =
		    hpcModMaps[hpcKbdPriv.type][i].modifiers;
	}

	(void) memset ((void *) defaultKeyboardControl.autoRepeats,
			~0, sizeof defaultKeyboardControl.autoRepeats);

#if 0 /* XXX */
#ifdef XKB
	if (noXkbExtension) {
	    hpcAutoRepeatInitiate = XkbDfltRepeatDelay * 1000;
	    hpcAutoRepeatDelay = XkbDfltRepeatInterval * 1000;
#endif
	autoRepeatKeyDown = 0;
#ifdef XKB
	}
#endif
#endif /* 0 XXX */
	pKeyboard->devicePrivate = (pointer)&hpcKbdPriv;
	pKeyboard->on = FALSE;

	InitKeyboardDeviceStruct(pKeyboard,
				 workingKeySyms, workingModMap,
				 hpcBell, hpcKbdCtrl);
	break;

    case DEVICE_ON:
	pPriv = (hpcKbdPrivPtr)pKeyboard->devicePrivate;
	switch (pPriv->devtype) {
	case HPC_KBDDEV_RAW:
	    pPriv->xlatestat = HPC_KBDXSTAT_INIT;
	    if (ioctl(pPriv->fd, KDSKBMODE, K_RAW) < 0) {
		FatalError("Can't set keyboard mode\n");
	    }
	    tcgetattr(pPriv->fd, &pPriv->kbdtty);
	    tkbdtty = pPriv->kbdtty;
	    tkbdtty.c_iflag = IGNPAR | IGNBRK;
	    tkbdtty.c_oflag = 0;
	    tkbdtty.c_cflag = CREAD | CS8;
	    tkbdtty.c_lflag = 0;
	    tkbdtty.c_cc[VTIME] = 0;
	    tkbdtty.c_cc[VMIN] = 1;
	    cfsetispeed(&tkbdtty, 9600);
	    cfsetospeed(&tkbdtty, 9600);
	    tcsetattr(pPriv->fd, TCSANOW, &tkbdtty);
	    break;
	case HPC_KBDDEV_WSKBD:
	case HPC_KBDDEV_WSMUX:
	}
	hpcCleanupFd(pPriv->fd);
	AddEnabledDevice(pPriv->fd);
	pKeyboard->on = TRUE;
	break;

    case DEVICE_CLOSE:
    case DEVICE_OFF:
	pPriv = (hpcKbdPrivPtr)pKeyboard->devicePrivate;
	RemoveEnabledDevice(pPriv->fd);
	pKeyboard->on = FALSE;
	switch (pPriv->devtype) {
	case HPC_KBDDEV_RAW:
	    if (ioctl(pPriv->fd, KDSKBMODE, K_XLATE) < 0) {
		Error("Can't set keyboard mode\n");
	    }
	    tcsetattr(pPriv->fd, TCSANOW, &pPriv->kbdtty);
	    break;
	case HPC_KBDDEV_WSKBD:
	case HPC_KBDDEV_WSMUX:
	}
	break;
    default:
	FatalError("Unknown keyboard operation\n");
    }
    return Success;
}

/*
 * hpcKbdGetEvents --
 *	Return the events waiting in the wings for the given keyboard.
 *
 * Results:
 *	A pointer to an array of hpcEvents or (hpcEvent *)0 if no events
 *	The number of events contained in the array.
 *	A boolean as to whether more events might be available.
 *
 * Side Effects:
 *	None.
 */
hpcEvent*
hpcKbdGetEvents(pPriv, pNumEvents, pAgain)
    hpcKbdPrivPtr pPriv;
    int*	pNumEvents;
    Bool*	pAgain;
{
    int fd;
    int	nBytes;	    /* number of bytes of events available. */
    u_char c;
    static hpcEvent evBuf[MAXEVENTS];   /* Buffer for hpcEvents */

    fd = pPriv->fd;
    switch (pPriv->devtype) {
    case HPC_KBDDEV_RAW:
	    if (read(fd, &c, 1) < 0) {
		if (errno == EWOULDBLOCK) {
		    *pNumEvents = 0;
		    *pAgain = FALSE;
		} else {
		    Error ("Reading keyboard");
		    FatalError ("Could not read the keyboard");
		}
	    } else {
		*pAgain = TRUE;
		*pNumEvents = 0;
		switch (pPriv->xlatestat) {
		case HPC_KBDXSTAT_INIT:
		    if (c == KBR_EXTENDED0) {
			pPriv->xlatestat = HPC_KBDXSTAT_EXT0;
		    } else
		    if (c == KBR_EXTENDED1) {
			pPriv->xlatestat = HPC_KBDXSTAT_EXT1;
		    } else {
			*pNumEvents = 1;
			evBuf[0].value = (c & 0x7f);
		    }
		    break;
		case HPC_KBDXSTAT_EXT0:
		    *pNumEvents = 1;
		    evBuf[0].value = (c | 0x80) ;
		    pPriv->xlatestat = HPC_KBDXSTAT_INIT;
		    break;
		case HPC_KBDXSTAT_EXT1:
		    if (c == 0x1d) {
			pPriv->xlatestat = HPC_KBDXSTAT_EXT1_1D;
		    } else
		    if (c == 0x9d) {
			pPriv->xlatestat = HPC_KBDXSTAT_EXT1_9D;
		    } else {
			ErrorF("hpcKbdGetEvents: unexpected input"
			       " %02x, stat=%d", c, pPriv->xlatestat);
			pPriv->xlatestat = HPC_KBDXSTAT_INIT;
		    }
		    break;
		case HPC_KBDXSTAT_EXT1_1D:
		case HPC_KBDXSTAT_EXT1_9D:
		    if (c == 0x45 || c == (0x45 | 0x80)) {
			*pNumEvents = 1;
			evBuf[0].value = 0x7f;
		    } else {
			ErrorF("hpcKbdGetEvents: unexpected input %02x, stat=%d",
			       c, pPriv->xlatestat);
		    }
		    pPriv->xlatestat = HPC_KBDXSTAT_INIT;
		    break;
		default:
		    FatalError("hpcKbdGetEvents: invalid xlate status");
		    break;
		}
		if (*pNumEvents != 0) {
		    struct timeval tv;
		    gettimeofday(&tv, NULL);
		    evBuf[0].type = (c & 0x80) ?
				WSCONS_EVENT_KEY_UP : WSCONS_EVENT_KEY_DOWN;
		    TIMEVAL_TO_TIMESPEC(&tv, &evBuf[0].time);
		}
	    }
	    break;
    case HPC_KBDDEV_WSKBD:
    case HPC_KBDDEV_WSMUX:
	if ((nBytes = read(fd, evBuf, sizeof(evBuf))) == -1) {
	    if (errno == EWOULDBLOCK) {
		*pNumEvents = 0;
		*pAgain = FALSE;
	    } else {
		Error ("Reading keyboard");
		FatalError ("Could not read the keyboard");
	    }
	} else {
	    *pNumEvents = nBytes / sizeof (hpcEvent);
	    *pAgain = (nBytes == sizeof (evBuf));
	}
	break;
    }

    return evBuf;
}

/*
 * hpcKbdEnqueueEvent --
 */
void
hpcKbdEnqueueEvent (device, fe)
    DeviceIntPtr  device;
    hpcEvent	  *fe;
{
    xEvent		xE;
    BYTE		keycode;
    CARD8		keyModifiers;

    keycode = (fe->value & 0xff) + MIN_KEYCODE;

    keyModifiers = device->key->modifierMap[keycode];
#if 0 /* XXX */
#ifdef XKB
    if (noXkbExtension) {
#endif
    if (autoRepeatKeyDown && (keyModifiers == 0) &&
	((fe->value == VKEY_DOWN) || (keycode == autoRepeatEvent.u.u.detail))) {
	/*
	 * Kill AutoRepeater on any real non-modifier key down, or auto key up
	 */
	autoRepeatKeyDown = 0;
    }
#ifdef XKB
    }
#endif
#endif /* 0 XXX */
    xE.u.keyButtonPointer.time = TSTOMILLI(fe->time);
    xE.u.u.type = ((fe->type == WSCONS_EVENT_KEY_UP) ? KeyRelease : KeyPress);
    xE.u.u.detail = keycode;
#if 0 /* XXX */
#ifdef XKB
    if (noXkbExtension) {
#endif
    if (DoSpecialKeys(device, &xE, fe))
	return;
#ifdef XKB
    }
#endif /* ! XKB */
#endif /* 0 XXX */
    mieqEnqueue (&xE);
}

/*ARGSUSED*/
Bool LegalModifier(key, pDev)
    unsigned int key;
    DevicePtr	pDev;
{
    return TRUE;
}
