/* $NetBSD: x68kIo.c,v 1.1.1.1 2016/06/09 09:07:59 mrg Exp $ */
/*-------------------------------------------------------------------------
 * Copyright (c) 1996 Yasushi Yamasaki
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Yasushi Yamasaki
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------*/

/*-
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

#include "x68k.h"
#include "mi.h"

static void x68kEnqueueEvents(void);

/*--------------------------------------------------------------------
 * function "x68kSigIOHandler"
 *
 *  purpose:  handles signals from input devices.
 *            enqueue inputs into mi event queue
 *  argument: (int)sig
 *  returns:  nothing
 *------------------------------------------------------------------*/
void
x68kSigIOHandler(int sig)
{
    int olderrno = errno;

    x68kEnqueueEvents();
    errno = olderrno;
}

/*--------------------------------------------------------------------
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
 *-----------------------------------------------------------------*/
void
ProcessInputEvents(void)
{
    (void) mieqProcessInputEvents ();
}

/*--------------------------------------------------------------------
 * x68kEnqueueEvents
 *	When a SIGIO is received, read device hard events and
 *	enqueue them using the mi event queue
 */

static void
x68kEnqueueEvents(void)
{
    Firm_event	*ptrEvents,    	/* Current pointer event */
		*kbdEvents;    	/* Current keyboard event */
    int		numPtrEvents, 	/* Number of remaining pointer events */
		numKbdEvents;   /* Number of remaining keyboard events */
    int		nPE,   	    	/* Original number of pointer events */
		nKE;   	    	/* Original number of keyboard events */
    Bool	PtrAgain,	/* need to (re)read */
		KbdAgain;	/* need to (re)read */
    DeviceIntPtr        pPointer;
    DeviceIntPtr	pKeyboard;
    X68kKbdPrivPtr      kbdPriv;
    X68kMousePrivPtr    ptrPriv;

    pPointer = x68kPointerDevice;
    pKeyboard = x68kKeyboardDevice;
    ptrPriv = (X68kMousePrivPtr) pPointer->public.devicePrivate;
    kbdPriv = (X68kKbdPrivPtr) pKeyboard->public.devicePrivate;
    if (!pPointer->public.on || !pKeyboard->public.on)
	return;

    numPtrEvents = 0;
    PtrAgain = TRUE;
    numKbdEvents = 0;
    KbdAgain = TRUE;
    ptrEvents = NULL;	/* XXX gcc */
    kbdEvents = NULL;	/* XXX gcc */

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
	    ptrEvents = x68kMouseGetEvents (ptrPriv->fd, &nPE, &PtrAgain);
	    numPtrEvents = nPE;
	}
	if ((numKbdEvents == 0) && KbdAgain) {
	    kbdEvents = x68kKbdGetEvents (kbdPriv->fd, &nKE, &KbdAgain);
	    numKbdEvents = nKE;
	}
	if ((numPtrEvents == 0) && (numKbdEvents == 0))
	    break;
	if (numPtrEvents && numKbdEvents) {
	    if (timercmp (&kbdEvents->time, &ptrEvents->time, <)) {
		x68kKbdEnqueueEvent (pKeyboard, kbdEvents);
		numKbdEvents--;
		kbdEvents++;
	    } else {
		x68kMouseEnqueueEvent (pPointer, ptrEvents);
		numPtrEvents--;
		ptrEvents++;
	    }
	} else if (numKbdEvents) {
	    x68kKbdEnqueueEvent (pKeyboard, kbdEvents);
	    numKbdEvents--;
	    kbdEvents++;
	} else {
	    x68kMouseEnqueueEvent (pPointer, ptrEvents);
	    numPtrEvents--;
	    ptrEvents++;
	}
    }
}

/* EOF x68kIo.c */
