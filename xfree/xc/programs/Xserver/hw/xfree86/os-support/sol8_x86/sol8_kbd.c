/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/sol8_x86/sol8_kbd.c,v 1.3 1999/11/19 13:55:03 hohndel Exp $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany
 * Copyright 1993 by David Dawes <dawes@XFree86.org> 
 * Copyright 1999 by David Holland <davidh@iquest.net)
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Thomas Roell, David Dawes, and
 * David Holland not be used in advertising or publicity pertaining 
 * to distribution of the software without specific, written prior 
 * permission.  Thomas Roell, David Dawes, and David Holland
 *  makes no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * THOMAS ROELL, DAVID DAWES, AND DAVID HOLLAND DISCLAIMS ALL WARRANTIES 
 * WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL THOMAS ROELL, DAVID DAWES,
 * OR DAVID HOLLAND BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL 
 * DAMAGES OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF 
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "X.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

#include <sys/kbd.h>

void sol8_setkbdinitiate(double value);
void sol8_setkbdrepeat(double value);

static	int	sol8_otranslation = -1;
static	int	sol8_odirect = -1;

int
xf86GetKbdLeds()
{
	int leds;

	ioctl(xf86Info.kbdFd, KIOCGLED, &leds);
	return(leds);
}

void
xf86SetKbdRepeat(char rad)
{

	sol8_setkbdinitiate( (double) xf86Info.kbdDelay / 1000.0 );
	sol8_setkbdrepeat( (double) xf86Info.kbdRate / 1000.0 );
	return;
}


/*
 * Save initial keyboard state.  This is called at the start of each server
 * generation.
 *
 * Solx86_8:	Should determine keyboard type. 
 * 		Should save keyboard state.
 */

void xf86KbdInit()
{
	int	ktype, klayout;
	
	if(xf86Info.kbdFd < 0) {
		xf86Info.kbdFd = open("/dev/kbd", O_RDWR|O_NONBLOCK);
		if(xf86Info.kbdFd < 0) {
			FatalError("Unable to open keyboard: /dev/kbd\n");
		}
	}

/* 
 * None of the followin should ever fail.  If it does, something is broke 
 * (IMO) - DWH 8/21/99
 */

	if( ioctl(xf86Info.kbdFd, KIOCTYPE, &ktype) < 0) {
		FatalError("Unable to determine keyboard type: %d\n", errno);
	}

	if( ioctl(xf86Info.kbdFd, KIOCLAYOUT, &klayout) < 0) {
		FatalError("Unable to determine keyboard layout: %d\n", errno);
	}

	if( ioctl(xf86Info.kbdFd, KIOCGTRANS, &sol8_otranslation) < 0) {
		FatalError("Unable to determine keyboard translation mode\n");
	}

	if( ioctl(xf86Info.kbdFd, KIOCGDIRECT, &sol8_odirect) < 0) {
		FatalError("Unable to determine keyboard direct setting\n");
	}

	return;

}

int xf86KbdOn(void)
{
	int	tmp = 1;

	if(ioctl(xf86Info.kbdFd, KIOCSDIRECT, &tmp) == -1) {
		FatalError("Setting keyboard direct mode on\n");
		return -1; 
	}

	/* Setup translation */

	tmp = TR_UNTRANS_EVENT;

	if(ioctl(xf86Info.kbdFd, KIOCTRANS, &tmp) == -1) {
		FatalError("Setting keyboard translation\n");
		return -1;
	}
	return(xf86Info.kbdFd);
}

int
xf86KbdOff()
{

	if(sol8_otranslation != -1) {
		if( ioctl(xf86Info.kbdFd, KIOCTRANS, &sol8_otranslation) < 0) {
			FatalError("Unable to restore keyboard translation mode\n");
		}
	}

	if(sol8_odirect != 0) {
		if( ioctl(xf86Info.kbdFd, KIOCSDIRECT, &sol8_odirect) < 0 ){
			FatalError("Unable to restore keyboard direct setting\n");
		}
	}

	return(xf86Info.kbdFd);
}
