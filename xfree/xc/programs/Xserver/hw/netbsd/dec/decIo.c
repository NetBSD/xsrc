/*	$NetBSD: decIo.c,v 1.2 2011/05/24 21:22:28 jakllsch Exp $	*/

/* XConsortium: sunIo.c,v 5.26.1.3 95/01/25 23:02:33 kaleb Exp */
/* XFree86: xc/programs/Xserver/hw/sun/sunIo.c,v 3.1 1995/01/28 15:46:06 dawes Exp */
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

#define NEED_EVENTS
#include    "mi.h"
#include    "dec.h"

/*-
 *-----------------------------------------------------------------------
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
 *
 *-----------------------------------------------------------------------
 */
void
ProcessInputEvents ()
{
    (void) mieqProcessInputEvents ();
    miPointerUpdate ();
}

/*
 *-----------------------------------------------------------------------
 * decEnqueueEvents
 *	When a SIGIO is received, read device hard events and
 *	enqueue them using the mi event queue
 */

void decEnqueueEvents (
#if NeedFunctionPrototypes
    void
#endif
)
{
    struct wscons_event *ptrEvents,
			*kbdEvents;
    int		numPtrEvents, 	/* Number of remaining pointer events */
		numKbdEvents;   /* Number of remaining keyboard events */
    int		nPE,   	    	/* Original number of pointer events */
		nKE;   	    	/* Original number of keyboard events */
    Bool	PtrAgain,	/* need to (re)read */
		KbdAgain;	/* need to (re)read */
    DeviceIntPtr	pPointer;
    DeviceIntPtr	pKeyboard;
    decKbdPrivPtr       kbdPriv;
    decPtrPrivPtr       ptrPriv;

    pPointer = (DeviceIntPtr)LookupPointerDevice();
    pKeyboard = (DeviceIntPtr)LookupKeyboardDevice();
    ptrPriv = (decPtrPrivPtr) pPointer->public.devicePrivate;
    kbdPriv = (decKbdPrivPtr) pKeyboard->public.devicePrivate;
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
  	 * Get events from both the pointer and the keyboard, storing the number
	 * of events gotten in nPE and nKE and keeping the start of both arrays
 	 * in pE and kE
	 */
	if ((numPtrEvents == 0) && PtrAgain) {
	    ptrEvents = decMouseGetEvents (ptrPriv->fd, &nPE, &PtrAgain);
	    numPtrEvents = nPE;
	}
	if ((numKbdEvents == 0) && KbdAgain) {
	    kbdEvents = decKbdGetEvents (kbdPriv->fd, &nKE, &KbdAgain);
	    numKbdEvents = nKE;
	}
	if ((numPtrEvents == 0) && (numKbdEvents == 0))
	    break;
	if (numPtrEvents && numKbdEvents) {
	    if (timespeccmp (&kbdEvents->time, &ptrEvents->time, <)) {
	     	decKbdEnqueueEvent (pKeyboard, kbdEvents);
	     	numKbdEvents--;
	     	kbdEvents++;
	    } else {
		 decMouseEnqueueEvent (pPointer, ptrEvents);
		 numPtrEvents--;
		 ptrEvents++;
	    }
	} else if (numKbdEvents) {
	    decKbdEnqueueEvent (pKeyboard, kbdEvents);
	    numKbdEvents--;
	    kbdEvents++;
	} else {
	    decMouseEnqueueEvent (pPointer, ptrEvents);
	    numPtrEvents--;
	    ptrEvents++;
	}
    }
}

/*
 * DDX - specific abort routine.  Called by AbortServer().
 */
void
AbortDDX()
{
    int		i, mode;
    ScreenPtr	pScreen;
    DevicePtr	devPtr;

    mode = WSDISPLAYIO_MODE_EMUL;

    (void) OsSignal (SIGIO, SIG_IGN);
#if 0 /* XXX */
    devPtr = LookupKeyboardDevice();
    if (devPtr)
	(void) sunChangeKbdTranslation (((sunKbdPrivPtr)(devPtr->devicePrivate))->fd, FALSE);
    sunNonBlockConsoleOff ();
#endif
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	(*pScreen->SaveScreen) (pScreen, SCREEN_SAVER_OFF);
	decDisableCursor (pScreen);
	ioctl(decFbs[pScreen->myNum].fd, WSDISPLAYIO_SMODE, &mode);
    }
}

/* Called by GiveUp(). */
void
ddxGiveUp()
{
    AbortDDX ();
}

int
ddxProcessArgument (argc, argv, i)
    int	argc;
    char *argv[];
    int	i;
{
    extern void UseMsg();

    if (!strcmp(argv[i], "-noaccel")) {
	decAccelerate = 0;
	return 1;
    }
    if (!strcmp(argv[i], "-mouse")) {
	if (++i >= argc) UseMsg ();
	decPtrDev = argv[i];
	return 2;
    }
    if (!strcmp(argv[i], "-keyboard")) {
	if (++i >= argc) UseMsg ();
	decKbdDev = argv[i];
	return 2;
    }
    if (!strcmp(argv[i], "-swcursor")) {
        decSoftCursor = TRUE;
        return 1;
    }
    if (!strcmp(argv[i], "-hwcursor")) {
        decHardCursor = TRUE;
        return 1;
    }
    if (!strcmp(argv[i], "-depth")) {
	if (++i >= argc) UseMsg ();
    	decWantedDepth = atoi(argv[i]);
    	return 2;
    }
#if 0 /* XXX */
#ifndef XKB
    if (strcmp (argv[i], "-ar1") == 0) {	/* -ar1 int */
	if (++i >= argc) UseMsg ();
	sunAutoRepeatInitiate = 1000 * (long)atoi(argv[i]);
	if (sunAutoRepeatInitiate > 1000000)
	    sunAutoRepeatInitiate =  999000;
	return 2;
    }
    if (strcmp (argv[i], "-ar2") == 0) {	/* -ar2 int */
	if (++i >= argc) UseMsg ();
	sunAutoRepeatDelay = 1000 * (long)atoi(argv[i]);
	if (sunAutoRepeatDelay > 1000000)
	    sunAutoRepeatDelay =  999000;
	return 2;
    }
#endif
    if (strcmp (argv[i], "-swapLkeys") == 0) {	/* -swapLkeys */
	sunSwapLkeys = TRUE;
	return 1;
    }
#endif /* 0 XXX */
    if (strcmp (argv[i], "-debug") == 0) {	/* -debug */
	return 1;
    }
    if (strcmp (argv[i], "-dev") == 0) {	/* -dev /dev/mumble */
	if (++i >= argc) UseMsg ();
	return 2;
    }
    if (strcmp (argv[i], "-zaphod") == 0) {	/* -zaphod */
	decActiveZaphod = FALSE;
	return 1;
    }
#if 0 /* XXX */
    if (strcmp (argv[i], "-mono") == 0) {	/* -mono */
	return 1;
    }
    if (strcmp (argv[i], "-flipPixels") == 0) {	/* -flipPixels */
	sunFlipPixels = TRUE;
	return 1;
    }
    if (strcmp (argv[i], "-fbinfo") == 0) {	/* -fbinfo */
	sunFbInfo = TRUE;
	return 1;
    }
    if (strcmp (argv[i], "-kbd") == 0) {	/* -kbd */
	if (++i >= argc) UseMsg();
	return 2;
    }
    if (strcmp (argv[i], "-protect") == 0) {	/* -protect */
	if (++i >= argc) UseMsg();
	return 2;
    }
    if (strcmp (argv[i], "-cg4frob") == 0) {
	sunCG4Frob = TRUE;
	return 1;
    }
    if (strcmp (argv[i], "-noGX") == 0) {
	sunNoGX = TRUE;
	return 1;
    }
#endif /* 0 XXX */
    return 0;
}

void
ddxUseMsg()
{
#if 0 /* XXX */
#ifndef XKB
    ErrorF("-ar1 int            set autorepeat initiate time\n");
    ErrorF("-ar2 int            set autorepeat interval time\n");
#endif
    ErrorF("-swapLkeys          swap keysyms on L1..L10\n");
#endif /* 0 XXX */
    ErrorF("-debug              disable non-blocking console mode\n");
    ErrorF("-dev fn[:fn][:fn]   name of device[s] to open\n");
    ErrorF("-mouse fn           mouse device to open\n");
    ErrorF("-keyboard fn        keyboard device to open\n");
    ErrorF("-noaccel            disable SFB/TGA hardware accelleration\n");
    ErrorF("-swcursor           use a software cursor\n");
    ErrorF("-hwcursor           use a hardware cursor (overrides -swcursor)\n");
    ErrorF("-depth bpp          set desired depth (may not take effect)\n");
    ErrorF("-zaphod             disable active Zaphod mode\n");
#if 0 /* XXX */
    ErrorF("-mono               force monochrome-only screen\n");
    ErrorF("-fbinfo             tell more about the found frame buffer(s)\n");
#ifdef UNDOCUMENTED
    ErrorF("-cg4frob            don't use the mono plane of the cgfour\n");
    ErrorF("-noGX               treat the GX as a dumb frame buffer\n");
#endif
#endif /* 0 XXX */
}
