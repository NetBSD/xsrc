/* $NetBSD: x68kKbd.c,v 1.2 2016/08/30 07:50:55 mrg Exp $ */
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

#include "x68k.h"
#include "mi.h"

#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include "screenint.h"
#include "inputstr.h"
#include "eventstr.h"
#include "misc.h"
#include "scrnintstr.h"
#include "servermd.h"

#include <X11/extensions/XKB.h>
#include "xkbsrv.h"

#define MIN_KEYCODE     7       /* necessary to avoid the mouse buttons */
#define MAX_KEYCODE     255     /* limited by the protocol */

X68kKbdPriv x68kKbdPriv;
DeviceIntPtr x68kKeyboardDevice = NULL;

static void x68kInitModMap(KeySymsRec *, CARD8 *);
static void x68kInitKbdNames(XkbRMLVOSet *, X68kKbdPrivPtr);
static void x68kKbdRingBell(DeviceIntPtr, int, int);
static void x68kKbdBell(int, DeviceIntPtr, pointer, int);
static void x68kKbdCtrl(DeviceIntPtr, KeybdCtrl *);
static void x68kSetLeds(X68kKbdPrivPtr, u_char);

/*------------------------------------------------------------------------
 * x68kKbdProc --
 *	Handle the initialization, etc. of a keyboard.
 *
 * Results:
 *	None.
 *
 *----------------------------------------------------------------------*/
int
x68kKbdProc(DeviceIntPtr pDev, 	/* Keyboard to manipulate */
            int what)	    	/* What to do to it */
{
    DevicePtr pKeyboard = &pDev->public;
    CARD8 x68kModMap[MAP_LENGTH];
    int mode;
    XkbRMLVOSet rmlvo;
    
    switch (what) {
        case DEVICE_INIT:
            pKeyboard->devicePrivate = (pointer)&x68kKbdPriv;
            if( (x68kKbdPriv.fd = open("/dev/kbd", O_RDONLY)) == -1 ) {
                ErrorF("Can't open keyboard device");
                return !Success;
            }
            pKeyboard->on = FALSE;
            x68kInitModMap(x68kKeySyms, x68kModMap);

            x68kInitKbdNames(&rmlvo, pKeyboard->devicePrivate);
#if 0 /* XXX How should we setup XKB maps for non PS/2 keyboard!? */
            InitKeyboardDeviceStruct(pDev, &rmlvo,
                                     x68kKbdBell, x68kKbdCtrl);
#else
            InitKeyboardDeviceStruct(pDev, NULL,
                                     x68kKbdBell, x68kKbdCtrl);
	    XkbApplyMappingChange(pDev, x68kKeySyms,
		x68kKeySyms->minKeyCode,
		x68kKeySyms->maxKeyCode - x68kKeySyms->minKeyCode + 1,
		x68kModMap, serverClient);
#endif
            break;

        case DEVICE_ON:
            mode = 1;
            if ( fcntl(x68kKbdPriv.fd, F_SETOWN, getpid()) == -1 ||
                 fcntl(x68kKbdPriv.fd, F_SETFL, O_NONBLOCK|O_ASYNC) == -1 ||
                 ioctl(x68kKbdPriv.fd, KIOCSDIRECT, &mode) == -1 ) {
                ErrorF("Async keyboard I/O failed");
                return !Success;
            }
	    x68kSetLeds(&x68kKbdPriv, (u_char)x68kKbdPriv.leds);
            (void) AddEnabledDevice(x68kKbdPriv.fd);
            pKeyboard->on = TRUE;
            break;

        case DEVICE_CLOSE:
        case DEVICE_OFF:
            RemoveEnabledDevice(x68kKbdPriv.fd);
            pKeyboard->on = FALSE;
            break;
        default:
            FatalError("Unknown keyboard operation\n");
    }
    return Success;
}

/*-------------------------------------------------------------------------
 * function "x68kInitModMap"
 *
 *  purpose:  initialize modmap with keysym table
 *  argument: (KeySymsRec *)x68kKeySyms : keysym table
 *            (CARD8 *)x68kModMap       : result
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
static void
x68kInitModMap(KeySymsRec *KeySyms, CARD8 *x68kModMap)
{
    int i;
    
    for (i = 0; i < MAP_LENGTH; i++)
        x68kModMap[i] = NoSymbol;
    if (KeySyms->minKeyCode < MIN_KEYCODE) {
        KeySyms->minKeyCode += MIN_KEYCODE;
        KeySyms->maxKeyCode += MIN_KEYCODE;
    }
#if 0
    if (KeySyms->maxKeyCode > MAX_KEYCODE)
        KeySyms->maxKeyCode += MAX_KEYCODE;
#endif
    for (i = KeySyms->minKeyCode;
         i < KeySyms->maxKeyCode; i++) {
        switch (KeySyms->map[(i-KeySyms->minKeyCode)*4]) {
            case XK_Shift_L:
            case XK_Shift_R:
                x68kModMap[i] = ShiftMask;
                break;
            case XK_Control_L:
            case XK_Control_R:
                x68kModMap[i] = ControlMask;
                break;
            case XK_Alt_L:
            case XK_Alt_R:
                x68kModMap[i] = Mod1Mask;
                break;
            case XK_Meta_L:
            case XK_Meta_R:
                x68kModMap[i] = Mod2Mask;
                break;
            case XK_Caps_Lock:
                x68kModMap[i] = LockMask;
                break;
        }
    }
}

/*-------------------------------------------------------------------------
 * function "x68kInitKbdNames"
 *
 *  purpose:  store xkb database names
 *  argument: (XkbRMLVOSet *)rmlvo
 *            (X68kKbdPrivPtr)pKbd
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
static void
x68kInitKbdNames(XkbRMLVOSet *rmlvo, X68kKbdPrivPtr pKbd)
{
#if 0 /* XXX How should we setup XKB maps for non PS/2 keyboard!? */
    rmlvo->rules = "base";
    rmlvo->model = "x68k";
    switch (pKbd->type) {
    case X68K_KB_STANDARD:
        rmlvo->layout = "jp(standard)";
        break;
    case X68K_KB_ASCII:
        rmlvo->layout = "jp(ascii)";
        break;
    }
    rmlvo->variant = "basic";
    rmlvo->options = "";
#else
    rmlvo->rules = "base";
    rmlvo->model = NULL;
    rmlvo->layout = NULL;
    rmlvo->variant = NULL;
    rmlvo->options = NULL;
#endif
}
    
/*-
 *-----------------------------------------------------------------------
 * x68kKbdGetEvents --
 *	Return the events waiting in the wings for the given keyboard.
 *
 * Results:
 *	A pointer to an array of Firm_events or (Firm_event *)0 if no events
 *	The number of events contained in the array.
 *	A boolean as to whether more events might be available.
 *
 * Side Effects:
 *	None.
 *-----------------------------------------------------------------------
 */
Firm_event *
x68kKbdGetEvents(int fd, int *pNumEvents, Bool *pAgain)
{
    int nBytes; 	    /* number of bytes of events available. */
    static Firm_event evBuf[X68K_MAXEVENTS];   /* Buffer for Firm_events */

    if ((nBytes = read (fd, evBuf, sizeof(evBuf))) == -1) {
	if (errno == EWOULDBLOCK) {
	    *pNumEvents = 0;
	    *pAgain = FALSE;
	} else {
	    ErrorF("Reading keyboard");
	    FatalError ("Could not read the keyboard");
	}
    } else {
	*pNumEvents = nBytes / sizeof (Firm_event);
	*pAgain = (nBytes == sizeof (evBuf));
    }
    return evBuf;
}

/*-
 *-----------------------------------------------------------------------
 * x68kKbdEnqueueEvent --
 *
 *-----------------------------------------------------------------------
 */
void
x68kKbdEnqueueEvent(DeviceIntPtr pDev, Firm_event *fe)
{
    BYTE		keycode;
    int			type;
    int			i, nevents;

    type = ((fe->value == VKEY_UP) ? KeyRelease : KeyPress);
    keycode = (fe->id & 0x7f) + MIN_KEYCODE;
    nevents = GetKeyboardEvents(x68kEvents, pDev, type, keycode);
    for (i = 0; i < nevents; i++)
	mieqEnqueue(pDev, &x68kEvents[i]);
}

/*-
 *-----------------------------------------------------------------------
 * x68kKbdBell --
 *	Ring the terminal/keyboard bell
 *
 * Results:
 *	Ring the keyboard bell for an amount of time proportional to
 *	"loudness."
 *
 * Side Effects:
 *	None, really...
 *
 *-----------------------------------------------------------------------
 */

static void
x68kKbdRingBell(DeviceIntPtr pDev, int volume, int duration)
{
    int		    kbdCmd;   	    /* Command to give keyboard */
    X68kKbdPrivPtr  pPriv = (X68kKbdPrivPtr)pDev->public.devicePrivate;
 
    if (volume == 0)
 	return;

    kbdCmd = KBD_CMD_BELL;
    if (ioctl (pPriv->fd, KIOCCMD, &kbdCmd) == -1) {
 	ErrorF("Failed to activate bell");
	return;
    }
    usleep (duration * 1000);
    kbdCmd = KBD_CMD_NOBELL;
    if (ioctl (pPriv->fd, KIOCCMD, &kbdCmd) == -1)
	ErrorF("Failed to deactivate bell");
}

static void
x68kKbdBell(int volume, DeviceIntPtr pDev, pointer ctrl, int unused)
{
    KeybdCtrl*      kctrl = (KeybdCtrl*) ctrl;
 
    if (kctrl->bell == 0)
 	return;

    x68kKbdRingBell(pDev, volume, kctrl->bell_duration);
}

void
DDXRingBell(int volume, int pitch, int duration)
{
    DeviceIntPtr	pKeyboard;

    pKeyboard = x68kKeyboardDevice;
    if (pKeyboard != NULL)
	x68kKbdRingBell(pKeyboard, volume, duration);
}

/*-
 *-----------------------------------------------------------------------
 * x68kKbdCtrl --
 *	Alter some of the keyboard control parameters
 *
 * Results:
 *	None.
 *
 * Side Effects:
 *	Some...
 *
 *-----------------------------------------------------------------------
 */
#define	XKB_LED_ZENKAKU		0x40
#define	XKB_LED_HIRAGANA	0x20
#define	XKB_LED_INSERT		0x10
#define	XKB_LED_CAPS_LOCK	0x08
#define	XKB_LED_CODE_INPUT	0x04
#define	XKB_LED_ROMAJI		0x02
#define	XKB_LED_KANA_LOCK	0x01

static void
x68kKbdCtrl(DeviceIntPtr pDev, KeybdCtrl *ctrl)
{
    X68kKbdPrivPtr pPriv = (X68kKbdPrivPtr)pDev->public.devicePrivate;

#ifdef XKB
    if (pPriv->leds != ctrl->leds) {
        x68kSetLeds(pPriv, (u_char)ctrl->leds);
	pPriv->leds = ctrl->leds;
    }
#endif
}

/*-------------------------------------------------------------------------
 * function "x68kSetLeds"
 *
 *  purpose:  set keyboard leds to specified state
 *  argument: (X68kKbdPrivPtr)pPriv
 *            (u_char)data;
 *  returns:  nothing
 *-----------------------------------------------------------------------*/
static void
x68kSetLeds(X68kKbdPrivPtr pPriv, u_char data)
{
    /* bit sequence of led indicator in xkb and hardware are same */
    if (ioctl(pPriv->fd, KIOCSLED, &data) == -1)
        ErrorF("Failed to set keyboard lights");
}    

Bool
LegalModifier(unsigned int key, DeviceIntPtr pDev)
{
    return TRUE;
}

/* EOF x68kKbd.c */
