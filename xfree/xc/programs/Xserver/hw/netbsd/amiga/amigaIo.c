/* $XConsortium: amigaIo.c,v 5.25 94/04/17 20:29:40 rws Exp $ */
/*-
 * amigaIo.c --
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
NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL AMIGA BE  LI-
ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/

#define NEED_EVENTS
#include    "amiga.h"
#include    "mi.h"

Bool            amigaEmulateMiddleButton = FALSE;
Bool            amigaEmulateRightButton = FALSE;

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
 * amigaEnqueueEvents
 *	Was:
 *
 *	Now: We are called as wakeup handler.
 */

void amigaEnqueueEvents (
#if NeedFunctionPrototypes
    void
#endif
)
{
    Firm_event	*ptrEvents,    	/* Current pointer event */
		*kbdEvents;    	/* Current keyboard event */
    int		numPtrEvents, 	/* Number of remaining pointer events */
		numKbdEvents;   /* Number of remaining keyboard events */
    int		nPE,   	    	/* Original number of pointer events */
		nKE;   	    	/* Original number of keyboard events */
    Bool	PtrAgain,	/* need to (re)read */
		KbdAgain;	/* need to (re)read */
    DeviceIntPtr	pPointer;
    DeviceIntPtr	pKeyboard;
    amigaKbdPrivPtr       kbdPriv;
    amigaPtrPrivPtr       ptrPriv;

    pPointer = (DeviceIntPtr)LookupPointerDevice();
    pKeyboard = (DeviceIntPtr)LookupKeyboardDevice();
    ptrPriv = (amigaPtrPrivPtr) pPointer->public.devicePrivate;
    kbdPriv = (amigaKbdPrivPtr) pKeyboard->public.devicePrivate;
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
	    if (ptrPriv->mousetype == -1)
	    	ptrEvents = amigaMouseGetEvents (ptrPriv->fd, 
						pPointer->public.on,
						&nPE, &PtrAgain);
	    else
	    	ptrEvents = amigaSerGetEvents (ptrPriv->fd, 
						pPointer->public.on,
						&nPE, &PtrAgain);
	    numPtrEvents = nPE;
	}
	if ((numKbdEvents == 0) && KbdAgain) {
	    kbdEvents = amigaKbdGetEvents (kbdPriv->fd, pKeyboard->public.on,
						&nKE, &KbdAgain);
	    numKbdEvents = nKE;
	}
	if ((numPtrEvents == 0) && (numKbdEvents == 0))
	    break;
	if (numPtrEvents && numKbdEvents) {
	    if (timercmp (&kbdEvents->time, &ptrEvents->time, <)) {
		amigaKbdEnqueueEvent (pKeyboard, kbdEvents);
		numKbdEvents--;
		kbdEvents++;
	    } else {
		amigaMouseEnqueueEvent (pPointer, ptrEvents, numPtrEvents > 1 ? &ptrEvents [1] : NULL);
		numPtrEvents--;
		ptrEvents++;
	    }
	} else if (numKbdEvents) {
	    amigaKbdEnqueueEvent (pKeyboard, kbdEvents);
	    numKbdEvents--;
	    kbdEvents++;
	} else {
	    amigaMouseEnqueueEvent (pPointer, ptrEvents, numPtrEvents > 1 ? &ptrEvents [1] : NULL);
	    numPtrEvents--;
	    ptrEvents++;
	}
    }
}

/*
 * DDX - specific abort routine.  Called by AbortServer().
 */
void AbortDDX()
{
    int		i;
    ScreenPtr	pScreen;
    DevicePtr	devPtr;

    devPtr = LookupKeyboardDevice();
    if (devPtr && devPtr->devicePrivate)
      (void) amigaChangeKbdTranslation (((amigaKbdPrivPtr)(devPtr->devicePrivate))->fd, FALSE);
    amigaNonBlockConsoleOff ();
    for (i = 0; i < screenInfo.numScreens; i++)
    {
	pScreen = screenInfo.screens[i];
	if (pScreen->SaveScreen != NULL)
	   (*pScreen->SaveScreen) (pScreen, SCREEN_SAVER_OFF);
#ifdef GFX_CARD_SUPPORT
	amigaDisableCursor (pScreen);
#endif
    }
}

/* Called by GiveUp(). */
void
ddxGiveUp()
{
    AbortDDX ();
    exit (1);
}

int
ddxProcessArgument (argc, argv, i)
    int	argc;
    char *argv[];
    int	i;
{
    extern void UseMsg();
    extern Bool amigaDontZap;

#ifdef XKB
    int noxkb = 0, n;
    /*
     * peek in argv and see if -kb because noXkbExtension won't
     * get set until too late to useful here.
     */
    for (n = 1; n < argc; n++)
	if (strcmp (argv[n], "-kb") == 0)
	    noxkb = 1;

    if (noxkb)
#endif
    if (strcmp (argv[i], "-ar1") == 0) {	/* -ar1 int */
	if (++i >= argc) UseMsg ();
	amigaAutoRepeatInitiate = 1000 * (long)atoi(argv[i]);	/* cvt to usec */
	if (amigaAutoRepeatInitiate > 1000000)
	    amigaAutoRepeatInitiate =  999000;
	return 2;
    }
#ifdef XKB
    if (noxkb)
#endif
    if (strcmp (argv[i], "-ar2") == 0) {	/* -ar2 int */
	if (++i >= argc) UseMsg ();
	amigaAutoRepeatDelay = 1000 * (long)atoi(argv[i]);	/* cvt to usec */
	if (amigaAutoRepeatDelay > 1000000)
	    amigaAutoRepeatDelay =  999000;
	return 2;
    }
    if (strcmp (argv[i], "-debug") == 0) {	/* -debug */
	return 1;
    }
    if (strcmp (argv[i], "-dev") == 0) {	/* -dev /dev/mumble */
	if (++i >= argc) UseMsg ();
	return 2;
    }
    if (strcmp (argv[i], "-mono") == 0) {	/* -mono */
	return 1;
    }
    if (strcmp (argv[i], "-zaphod") == 0) {	/* -zaphod */
	amigaActiveZaphod = FALSE;
	return 1;
    }
    if (strcmp (argv[i], "-flipPixels") == 0) {	/* -flipPixels */
	amigaFlipPixels = TRUE;
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
    if (strcmp (argv[i], "-nozap") == 0 ||
	strcmp (argv[i], "-nopanickeys") == 0) {	/* -nozap */
	amigaDontZap = TRUE;
	return 1;
    }
#ifdef GFX_CARD_SUPPORT
    if (strcmp (argv[i], "-mode") == 0) {	/* -mode */
        if (++i >= argc) UseMsg();
	return 2;
    }
    if (strcmp (argv[i], "-noGX") == 0) {
	amigaNoGX = TRUE;
	return 1;
    }
    if (strcmp (argv[i], "-useHWC") == 0) {
	amigaUseHWC = TRUE;
	return 1;
    }
#endif /* GFX_CARD_SUPPORT */
#ifdef CV64_SUPPORT
    if (strcmp (argv[i], "-virtualW") == 0) {
	if (++i >= argc) UseMsg ();
	amigaVirtualWidth = atoi (argv[i]);
	return 2;
    }
    if (strcmp (argv[i], "-virtualH") == 0) {
	if (++i >= argc) UseMsg ();
	amigaVirtualHeight = atoi (argv[i]);
	return 2;
    }
#endif /* CV64_SUPPORT */
#ifdef AMIGA_CC_COLOR
    if (strcmp (argv[i], "-W") == 0 || strcmp (argv[i], "-width") == 0) { /* -W int */
	if (++i >= argc) UseMsg ();
	amigaCCWidth = (long)atoi(argv[i]);
	return 2;
    }
    if (strcmp (argv[i], "-H") == 0 || strcmp (argv[i], "-height") == 0) { /* -H int */
	if (++i >= argc) UseMsg ();
	amigaCCHeight = (long)atoi(argv[i]);
	return 2;
    }
    if (strcmp (argv[i], "-D") == 0 || strcmp (argv[i], "-depth") == 0) { /* -D int */
	if (++i >= argc) UseMsg ();
	amigaCCDepth = (long)atoi(argv[i]);
	return 2;
    }
    if (strcmp (argv[i], "-X") == 0) {          /* -X int */
	if (++i >= argc) UseMsg ();
	amigaCCXOffset = (long)atoi(argv[i]);
	return 2;
    }
    if (strcmp (argv[i], "-Y") == 0) {          /* -Y int */
	if (++i >= argc) UseMsg ();
	amigaCCYOffset = (long)atoi(argv[i]);
	return 2;
    }
#endif /*  AMIGA_CC_COLOR */
    if (strcmp (argv[i], "-emulateright") == 0 || strcmp (argv[i], "-3") == 0) {
	amigaEmulateRightButton = TRUE;
	return 1;
    }
    if (strcmp (argv[i], "-emulatemiddle") == 0 || strcmp (argv[i], "-2") == 0) {
	amigaEmulateMiddleButton = TRUE;
	return 1;
    }
    if (strcmp (argv[i], "-mouse") == 0) {
	if (++i >= argc) UseMsg ();
	amigaPtrPriv.mousename = argv[i];
	return 2;
    }

    return 0;
}

void
ddxUseMsg()
{
#ifndef XKB
    ErrorF("-ar1 int               set autorepeat initiate time\n");
    ErrorF("-ar2 int               set autorepeat interval time\n");
#endif
    ErrorF("-debug                 disable non-blocking console mode\n");
    ErrorF("-dev fn[:fn][:fn]      name of device[s] to open\n");
    ErrorF("-flipPixels            switch colors 0 and 1\n");
    ErrorF("-mono                  force monochrome-only screen\n");
    ErrorF("-zaphod                disable active Zaphod mode\n");
    ErrorF("-nozap                 disable CTRL-ALT-BACKSPACE key sequence\n");
    ErrorF("-nopanickeys           disable CTRL-ALT-BACKSPACE key sequence\n");
#ifdef GFX_CARD_SUPPORT
    ErrorF("-mode num              set board into videomode num\n");
    ErrorF("-noGX                  treat the GX as a dumb frame buffer\n");
    ErrorF("-useHWC                Use the Hardware Cursor (Gfx Board only)\n");
#endif
#ifdef CV64_SUPPORT
    ErrorF("-virtualW              virtual screen width (CV64 only)\n");
    ErrorF("-virtualH              virtual screen height (CV64 only)\n");
#endif
#ifdef AMIGA_CC_COLOR
    ErrorF("-width int             set display width in pixels (amigaNative only)\n");
    ErrorF("-height int            set display height in pixels (amigaNative only)\n");
    ErrorF("-depth int             set display depth in pixels (amigaNative only)\n");
    ErrorF("-X int                 set display X offset in pixels (amigaNative only)\n");
    ErrorF("-Y int                 set display Y offset in pixels (amigaNative only)\n");
#endif
    ErrorF("-emulatemiddle (or -2) Emulate middle button on 2-button mouse\n");
    ErrorF("-emulateright (or -3)  Emulate right button on 2-button mouse\n");

    ErrorF("-mouse fn              Use fn instead of /dev/mouse0\n");
}
