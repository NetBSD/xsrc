/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bsd/bsd_mo.c,v 3.11 1996/08/23 11:04:37 dawes Exp $ */
/*
 * Copyright 1992 by Rich Murphey <Rich@Rice.edu>
 * Copyright 1993 by David Dawes <dawes@physics.su.oz.au>
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
/* $XConsortium: bsd_io.c /main/6 1995/11/13 05:54:02 kaleb $ */

#define NEED_EVENTS
#include "X.h"
#include "Xproto.h"
#include "inputstr.h"
#include "scrnintstr.h"

#include "compiler.h"

#include "xf86Procs.h"
#include "xf86_OSlib.h"
#include "xf86_Config.h"

void xf86SoundKbdBell(loudness, pitch, duration)
int loudness;
int pitch;
int duration;
{
	struct kbdbell kb;

    	if (loudness && pitch && duration)
	{
		kb.volume = loudness;
		kb.pitch = pitch;
		kb.duration = duration;

		ioctl(xf86Info.consoleFd, KIOCRINGBELL, &kb);
	}
}

void xf86SetKbdLeds(leds)
int leds;
{
}

int xf86GetKbdLeds()
{
	int leds = 0;

	return(leds);
}

#if NeedFunctionPrototypes
void xf86SetKbdRepeat(char rad)
#else
void xf86SetKbdRepeat(rad)
char rad;
#endif
{
}

static struct termio kbdtty;
/*
 * Don't know why, but the HADES keyboard gives some times key relase
 * of keys which are not pressed. X get confused about this. Therfore
 * this hack with k_pressed.
 */
static char k_pressed[128];

void xf86KbdInit()
{
    int i;

    /* Assume all keys are released at startup */
    for (i = 0; i < 128; i++) k_pressed[i] = 0;
}

int xf86KbdOn()
{
	return(xf86Info.consoleFd);
}

int xf86KbdOff()
{
	return(xf86Info.consoleFd);
}

void xf86MouseInit(mouse)
MouseDevPtr mouse;
{
	return;
}

#define MAXEVENTS	128

void xf86KbdEvents()
{
    Firm_event eventBuf[MAXEVENTS];
    unsigned char keycode;
    int nBytes, i;

    if ((nBytes = read( xf86Info.consoleFd, (char *)eventBuf, sizeof(eventBuf)))
        > 0)
    {
	for (i = 0; i < nBytes / sizeof(Firm_event); i++) {
	    keycode = eventBuf[i].id & 0xff;
	    if (eventBuf[i].value != k_pressed[keycode]) {
	        xf86PostKbdEvent(keycode | ((~eventBuf[i].value & 0x01) << 7));
		k_pressed[keycode] = eventBuf[i].value;
	    }
	}
    }
}

int xf86MouseOn(mouse)
MouseDevPtr mouse;
{
	if ((mouse->mseFd = open(mouse->mseDevice, O_RDWR | O_NDELAY)) < 0)
	{
		if (xf86AllowMouseOpenFail) {
			ErrorF("Cannot open mouse (%s) - Continuing...\n",
				strerror(errno));
			return(-2);
		}
		FatalError("Cannot open mouse (%s)\n", strerror(errno));
	}

	xf86SetupMouse(mouse);

	/* Flush any pending input */
	tcflush(mouse->mseFd, TCIFLUSH);

	return(mouse->mseFd);
}

int xf86MouseOff(mouse, doclose)
MouseDevPtr mouse;
Bool doclose;
{
	int oldfd;

	if ((oldfd = mouse->mseFd) >= 0)
	{
		if (mouse->mseType == P_LOGI)
		{
			write(mouse->mseFd, "U", 1);
		}
		if (mouse->oldBaudRate > 0) {
		    xf86SetMouseSpeed(mouse,
				      mouse->baudRate,
				      mouse->oldBaudRate,
				      xf86MouseCflags[mouse->mseType]);
		}
		close(mouse->mseFd);
		oldfd = mouse->mseFd;
		mouse->mseFd = -1;
	}
	return(oldfd);
}

void xf86MouseEvents(mouse)
    MouseDevPtr	mouse;
{
	unsigned char rBuf[MAXEVENTS * sizeof(Firm_event)];
	int nBytes;

	if ((nBytes = read(mouse->mseFd, (char *)rBuf, sizeof(rBuf))) > 0)
	{
		xf86MouseProtocol(mouse->device, rBuf, nBytes);
	}
}


static Bool not_a_tty = FALSE;

void xf86SetMouseSpeed(mouse, old, new, cflag)
MouseDevPtr mouse;
int old;
int new;
unsigned cflag;
{
	struct termios tty;
	char *c;

	if (not_a_tty)
		return;

	if (tcgetattr(mouse->mseFd, &tty) < 0)
	{
		not_a_tty = TRUE;
		ErrorF("Warning: %s unable to get status of mouse fd (%s)\n",
		       mouse->mseDevice, strerror(errno));
		return;
	}

	/* this will query the initial baudrate only once */
	if (mouse->oldBaudRate < 0) { 
	   switch (cfgetispeed(&tty)) 
	      {
	      case B9600: 
		 mouse->oldBaudRate = 9600;
		 break;
	      case B4800: 
		 mouse->oldBaudRate = 4800;
		 break;
	      case B2400: 
		 mouse->oldBaudRate = 2400;
		 break;
	      case B1200: 
	      default:
		 mouse->oldBaudRate = 1200;
		 break;
	      }
	}

	tty.c_iflag = IGNBRK | IGNPAR;
	tty.c_oflag = 0;
	tty.c_lflag = 0;
	tty.c_cflag = (tcflag_t)cflag;
	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 1;

	switch (old)
	{
	case 9600:
		cfsetispeed(&tty, B9600);
		cfsetospeed(&tty, B9600);
		break;
	case 4800:
		cfsetispeed(&tty, B4800);
		cfsetospeed(&tty, B4800);
		break;
	case 2400:
		cfsetispeed(&tty, B2400);
		cfsetospeed(&tty, B2400);
		break;
	case 1200:
	default:
		cfsetispeed(&tty, B1200);
		cfsetospeed(&tty, B1200);
	}

	if (tcsetattr(mouse->mseFd, TCSADRAIN, &tty) < 0)
	{
		if (xf86AllowMouseOpenFail) {
			ErrorF("Unable to set status of mouse fd (%s) - Continuing...\n",
			       strerror(errno));
			return;
		}
		xf86FatalError("Unable to set status of mouse fd (%s)\n",
			       strerror(errno));
	}

	switch (new)
	{
	case 9600:
		c = "*q";
		cfsetispeed(&tty, B9600);
		cfsetospeed(&tty, B9600);
		break;
	case 4800:
		c = "*p";
		cfsetispeed(&tty, B4800);
		cfsetospeed(&tty, B4800);
		break;
	case 2400:
		c = "*o";
		cfsetispeed(&tty, B2400);
		cfsetospeed(&tty, B2400);
		break;
	case 1200:
	default:
		c = "*n";
		cfsetispeed(&tty, B1200);
		cfsetospeed(&tty, B1200);
	}

	if (mouse->mseType == P_LOGIMAN || mouse->mseType == P_LOGI)
	{
		if (write(mouse->mseFd, c, 2) != 2)
		{
			if (xf86AllowMouseOpenFail) {
				ErrorF("Unable to write to mouse fd (%s) - Continuing...\n",
				       strerror(errno));
				return;
			}
			xf86FatalError("Unable to write to mouse fd (%s)\n",
				       strerror(errno));
		}
	}
	usleep(100000);

	if (tcsetattr(mouse->mseFd, TCSADRAIN, &tty) < 0)
	{
		if (xf86AllowMouseOpenFail) {
			ErrorF("Unable to set status of mouse fd (%s) - Continuing...\n",
			       strerror(errno));
			return;
		}
		xf86FatalError("Unable to set status of mouse fd (%s)\n",
			       strerror(errno));
	}
}

int
xf86FlushInput(fd)
int fd;
{
        return tcflush(fd, TCIFLUSH);
}

