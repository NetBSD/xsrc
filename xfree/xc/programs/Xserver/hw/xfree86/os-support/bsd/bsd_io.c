/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bsd/bsd_io.c,v 3.18 2000/11/06 19:24:08 dawes Exp $ */
/*
 * Copyright 1992 by Rich Murphey <Rich@Rice.edu>
 * Copyright 1993 by David Dawes <dawes@xfree86.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Rich Murphey and David Dawes 
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  Rich Murphey and
 * David Dawes make no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * RICH MURPHEY AND DAVID DAWES DISCLAIM ALL WARRANTIES WITH REGARD TO 
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS, IN NO EVENT SHALL RICH MURPHEY OR DAVID DAWES BE LIABLE FOR 
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF 
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XConsortium: bsd_io.c /main/11 1996/10/19 18:06:07 kaleb $ */

#define NEED_EVENTS
#include "X.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

void
xf86SoundKbdBell(int loudness, int pitch, int duration)
{
    	if (loudness && pitch)
	{
#ifdef PCCONS_SUPPORT
		int data[2];
#endif

	    	switch (xf86Info.consType) {

#ifdef PCCONS_SUPPORT
	    	case PCCONS:
		    	data[0] = pitch;
		    	data[1] = (duration * loudness) / 50;
		    	ioctl(xf86Info.consoleFd, CONSOLE_X_BELL, data);
			break;
#endif
#if defined (SYSCONS_SUPPORT) || defined (PCVT_SUPPORT)
	    	case SYSCONS:
		case PCVT:
			ioctl(xf86Info.consoleFd, KDMKTONE,
			      ((1193190 / pitch) & 0xffff) |
			      (((unsigned long)duration*loudness/50)<<16));
			break;
#endif
	    	}
	}
}

void
xf86SetKbdLeds(int leds)
{
	switch (xf86Info.consType) {

	case PCCONS:
		break;
#if defined (SYSCONS_SUPPORT) || defined (PCVT_SUPPORT)
	case SYSCONS:
	case PCVT:
		ioctl(xf86Info.consoleFd, KDSETLED, leds);
		break;
#endif
	}
}

int
xf86GetKbdLeds()
{
	int leds = 0;

	switch (xf86Info.consType) {

	case PCCONS:
		break;
#if defined (SYSCONS_SUPPORT) || defined (PCVT_SUPPORT)
	case SYSCONS:
	case PCVT:
		ioctl(xf86Info.consoleFd, KDGETLED, &leds);
		break;
#endif
	}
	return(leds);
}

void
xf86SetKbdRepeat(char rad)
{
	switch (xf86Info.consType) {

	case PCCONS:
		break;
#if defined (SYSCONS_SUPPORT) || defined (PCVT_SUPPORT)
	case SYSCONS:
	case PCVT:
		ioctl(xf86Info.consoleFd, KDSETRAD, rad);
		break;
#endif
	}
}

static struct termio kbdtty;

void
xf86KbdInit()
{
	switch (xf86Info.consType) {

#if defined(PCCONS_SUPPORT) || defined(SYSCONS_SUPPORT) || defined (PCVT_SUPPORT)
	case PCCONS:
	case SYSCONS:
	case PCVT:
		tcgetattr(xf86Info.consoleFd, &kbdtty);
		break;
#endif
	}
}

int
xf86KbdOn()
{
	struct termios nTty;

	switch (xf86Info.consType) {

#if defined(SYSCONS_SUPPORT) || defined(PCCONS_SUPPORT) || defined(PCVT_SUPPORT)
	case SYSCONS:
	case PCCONS:
	case PCVT:
		nTty = kbdtty;
		nTty.c_iflag = IGNPAR | IGNBRK;
		nTty.c_oflag = 0;
		nTty.c_cflag = CREAD | CS8;
		nTty.c_lflag = 0;
		nTty.c_cc[VTIME] = 0;
		nTty.c_cc[VMIN] = 1;
		cfsetispeed(&nTty, 9600);
		cfsetospeed(&nTty, 9600);
		tcsetattr(xf86Info.consoleFd, TCSANOW, &nTty);

#if defined (SYSCONS_SUPPORT) || defined (PCVT_SUPPORT)
		ioctl(xf86Info.consoleFd, KDSKBMODE, K_RAW);
#endif
		break;
#endif
#ifdef WSCONS_SUPPORT
	case WSCONS:
		return xf86Info.kbdFd;
#endif
	}
	return(xf86Info.consoleFd);
}

int
xf86KbdOff()
{
	switch (xf86Info.consType) {

#if defined (SYSCONS_SUPPORT) || defined (PCVT_SUPPORT)
	case SYSCONS:
	case PCVT:
		ioctl(xf86Info.consoleFd, KDSKBMODE, K_XLATE);
		/* FALL THROUGH */
#endif
#if defined(SYSCONS_SUPPORT) || defined(PCCONS_SUPPORT) || defined(PCVT_SUPPORT)
	case PCCONS:
		tcsetattr(xf86Info.consoleFd, TCSANOW, &kbdtty);
		break;
#endif
	}
	return(xf86Info.consoleFd);
}

