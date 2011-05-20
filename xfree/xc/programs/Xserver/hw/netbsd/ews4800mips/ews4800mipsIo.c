/* $XConsortium: sunIo.c,v 5.26.1.3 95/01/25 23:02:33 kaleb Exp $ */
/* $XFree86: xc/programs/Xserver/hw/sun/sunIo.c,v 3.1 1995/01/28 15:46:06 dawes Exp $ */
/*-
 * sunIo.c --
 *	Functions to handle input from the keyboard and mouse.
 *
 * Copyright (c) 1987 by the Regents of the University of California
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies.  The University of California
 * makes no representations about the suitability of this
 * software for any purpose.  It is provided "as is" without
 * express or implied warranty.
 *
 *
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

#include <stdio.h>

#define	NEED_EVENTS
#include "ews4800mips.h"
#include "mi.h"

void
ews4800mipsCleanupFd(int fd)
{
	int n;
	char buf[100];

	while (0 < (n = read(fd, buf, sizeof(buf)))) {
		;
	}
}

/*
 * ProcessInputEvents --
 *	Retrieve all waiting input events and pass them to DIX in their
 *	correct chronological order. Only reads from the system pointer
 *	and keyboard.
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Events are passed to the DIX layer.
 */
void
ProcessInputEvents(void)
{
	(void) mieqProcessInputEvents ();
	miPointerUpdate ();
}

/*
 * ews4800mipsEnqueueEvents
 *	When a SIGIO is received, read device hard events and
 *	enqueue them using the mi event queue
 */
void
ews4800mipsEnqueueEvents(void)
{
	ews4800mipsEvent *ptrEvents;    /* Current pointer event */
	ews4800mipsEvent *kbdEvents;    /* Current keyboard event */
	int numPtrEvents;	/* Number of remaining pointer events */
	int numKbdEvents;	/* Number of remaining keyboard events */
	int nPE;   	    	/* Original number of pointer events */
	int nKE;   	    	/* Original number of keyboard events */
	Bool PtrAgain;		/* need to (re)read */
	Bool KbdAgain;		/* need to (re)read */
	DeviceIntPtr pPointer;
	DeviceIntPtr pKeyboard;
	ews4800mipsKbdPrivPtr kbdPriv;
	ews4800mipsPtrPrivPtr ptrPriv;

	pPointer = (DeviceIntPtr)LookupPointerDevice();
	pKeyboard = (DeviceIntPtr)LookupKeyboardDevice();
	ptrPriv = (ews4800mipsPtrPrivPtr) pPointer->public.devicePrivate;
	kbdPriv = (ews4800mipsKbdPrivPtr) pKeyboard->public.devicePrivate;
	if (!pPointer->public.on || !pKeyboard->public.on)
		return;

	numPtrEvents = 0;
	PtrAgain = TRUE;
	numKbdEvents = 0;
	KbdAgain = TRUE;

	/*
	 * So long as one event from either device remains unprocess, we loop:
	 * Take the oldest remaining event and pass it to the proper module
	 * for processing. The DDXEvent will be sent to ProcessInput by the
	 * function called.
	 */
	while (1) {
		/*
		 * Get events from both the pointer and the keyboard,
		 * storing the number of events gotten in nPE and nKE
		 * and keeping the start of both arrays in pE and kE
		 */
		if ((numPtrEvents == 0) && PtrAgain) {
			ptrEvents = ews4800mipsMouseGetEvents(ptrPriv, &nPE,
			    &PtrAgain);
			numPtrEvents = nPE;
		}
		if ((numKbdEvents == 0) && KbdAgain) {
			kbdEvents = ews4800mipsKbdGetEvents(kbdPriv, &nKE,
			    &KbdAgain);
			numKbdEvents = nKE;
		}
		if ((numPtrEvents == 0) && (numKbdEvents == 0))
			break;
		if (numPtrEvents && numKbdEvents) {
			if (timespeccmp (&kbdEvents->time,
			    &ptrEvents->time, <)) {
				ews4800mipsKbdEnqueueEvent(pKeyboard,
				    kbdEvents);
				numKbdEvents--;
				kbdEvents++;
			} else {
				ews4800mipsMouseEnqueueEvent(pPointer,
				    ptrEvents);
				numPtrEvents--;
				ptrEvents++;
			}
		} else if (numKbdEvents) {
			ews4800mipsKbdEnqueueEvent(pKeyboard, kbdEvents);
			numKbdEvents--;
			kbdEvents++;
		} else {
			ews4800mipsMouseEnqueueEvent(pPointer, ptrEvents);
			numPtrEvents--;
			ptrEvents++;
		}
	}
}

/*
 * DDX - specific abort routine.  Called by AbortServer().
 */
void
AbortDDX(void)
{
	int i;
	ScreenPtr pScreen;
	ews4800mipsFbPtr pFb;
	DevicePtr devPtr;

	OsSignal(SIGIO, SIG_IGN);
	for (i = 0; i < screenInfo.numScreens; i++)
	{
		int mode = WSDISPLAYIO_MODE_EMUL;
		pScreen = screenInfo.screens[i];
		pFb = ews4800mipsGetScreenFb(pScreen);
		(*pScreen->SaveScreen)(pScreen, SCREEN_SAVER_OFF);
		ioctl(pFb->fd, WSDISPLAYIO_SMODE, &mode);
	}
}

/* Called by GiveUp(). */
void
ddxGiveUp(void)
{

	AbortDDX ();
}

int
ddxProcessArgument(int argc, char *argv[], int i)
{
	extern void UseMsg();

	if (strcmp (argv[i], "-debug") == 0) {	/* -debug */
		return 1;
	}
	if (strcmp (argv[i], "-dev") == 0) {	/* -dev /dev/mumble */
		if (++i >= argc)
			UseMsg ();
		return 2;
	}
	return 0;
}

void
ddxUseMsg(void)
{

	ews4800mipsErrorF(("-debug              disable non-blocking"
	    " console mode\n"));
	ews4800mipsErrorF(("-dev fn[:fn][:fn]   name of device[s] to open\n"));
}
