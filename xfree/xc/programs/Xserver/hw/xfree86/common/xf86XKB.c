/* $XConsortium: xf86XKB.c /main/4 1996/02/04 09:28:04 kaleb $ */
/************************************************************
Copyright (c) 1993 by Silicon Graphics Computer Systems, Inc.

Permission to use, copy, modify, and distribute this
software and its documentation for any purpose and without
fee is hereby granted, provided that the above copyright
notice appear in all copies and that both that copyright
notice and this permission notice appear in supporting
documentation, and that the name of Silicon Graphics not be 
used in advertising or publicity pertaining to distribution 
of the software without specific prior written permission.
Silicon Graphics makes no representation about the suitability 
of this software for any purpose. It is provided "as is"
without any express or implied warranty.

SILICON GRAPHICS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS 
SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SILICON
GRAPHICS BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, 
DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE 
OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
THE USE OR PERFORMANCE OF THIS SOFTWARE.

********************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/common/xf86XKB.c,v 3.9 2000/05/23 04:47:41 dawes Exp $ */

#include <stdio.h>
#define	NEED_EVENTS 1
#include <X11/X.h>
#include <X11/Xproto.h>
#include <X11/keysym.h>
#include "inputstr.h"
#include "scrnintstr.h"
#include "windowstr.h"
#include "XI.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#define XF86_OS_PRIVS
#include "xf86_OSlib.h"

#include "XKBsrv.h"

#ifdef AMOEBA
#define LED_CAP	IOP_LED_CAP
#define LED_NUM	IOP_LED_NUM
#define LED_SCR	IOP_LED_SCROLL
#endif

#ifdef MINIX
#define LED_CAP KBD_LEDS_CAPS
#define LED_NUM KBD_LEDS_NUM
#define LED_SCR KBD_LEDS_SCROLL
#endif

void
xf86InitXkb(void)
{
}

void
XkbDDXUpdateIndicators(DeviceIntPtr pXDev,CARD32 new)
{
    CARD32 old;
#if defined (__sparc__)
    static int kbdSun = -1;
    
    if (kbdSun == -1) {
	if ((xf86Info.xkbmodel && !strcmp(xf86Info.xkbmodel, "sun"))
	    || (xf86Info.xkbrules && !strcmp(xf86Info.xkbrules, "sun")))
	    kbdSun = 1;
	else
	    kbdSun = 0;
	}
	if (kbdSun) {
	    old = new;
	    new = 0;
	    if (old & 0x08) new |= XLED1;
	    if (old & 0x04) new |= XLED3;
	    if (old & 0x02) new |= XLED4;
	    if (old & 0x01) new |= XLED2;
	}
#endif /* defined (__sparc__) */
#ifdef DEBUG
/*    if (xkbDebugFlags)*/
        ErrorF("XkbDDXUpdateIndicators(...,0x%x) -- XFree86 version\n",new);
#endif
#ifdef LED_CAP
    old= new;
    new= 0;
    if (old&XLED1)	new|= LED_CAP;
    if (old&XLED2)	new|= LED_NUM;
    if (old&XLED3)	new|= LED_SCR;
#ifdef LED_COMP
    if (old&XLED4)	new|= LED_COMP;
#endif
#endif
    xf86SetKbdLeds(new);
    return;
}

void
XkbDDXUpdateDeviceIndicators(	DeviceIntPtr		dev,
				XkbSrvLedInfoPtr 	sli,
				CARD32 			new)
{
    if (sli->fb.kf==dev->kbdfeed)
	XkbDDXUpdateIndicators(dev,new);
    else if (sli->class==KbdFeedbackClass) {
	KbdFeedbackPtr	kf;
	kf= sli->fb.kf;
	if (kf && kf->CtrlProc) {
	    (*kf->CtrlProc)(dev,&kf->ctrl);
	}
    }
    else if (sli->class==LedFeedbackClass) {
	LedFeedbackPtr	lf;
	lf= sli->fb.lf;
	if (lf && lf->CtrlProc) {
	    (*lf->CtrlProc)(dev,&lf->ctrl);
	}
    }
    return;
}
