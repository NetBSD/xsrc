/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/sol8_x86/sol8_io.c,v 1.2 1999/10/13 04:21:36 dawes Exp $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany
 * Copyright 1993 by David Dawes <dawes@xfree86.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Thomas Roell and David Dawes 
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  Thomas Roell and
 * David Dawes makes no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * THOMAS ROELL AND DAVID DAWES DISCLAIMS ALL WARRANTIES WITH REGARD TO 
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS, IN NO EVENT SHALL THOMAS ROELL OR DAVID DAWES BE LIABLE FOR 
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF 
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XConsortium: sysv_io.c /main/8 1996/10/19 18:08:06 kaleb $ */

#include "X.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"
#include "xf86Xinput.h"
#include "xf86OSmouse.h"

void
xf86SoundKbdBell(int loudness, int pitch, int duration)
{
	int	kbdCmd;

	if (loudness && pitch)
	{
		kbdCmd = KBD_CMD_BELL;
		if (ioctl (xf86Info.kbdFd, KIOCCMD, &kbdCmd) == -1) {
			ErrorF("Failed to activate bell\n");
			return;
		}

		usleep(xf86Info.bell_duration * loudness * 20);

		kbdCmd = KBD_CMD_NOBELL;
		if (ioctl (xf86Info.kbdFd, KIOCCMD, &kbdCmd) == -1)
			ErrorF ("Failed to deactivate bell\n");
	}
}

void
xf86SetKbdLeds(int leds)
{
	if( ioctl(xf86Info.kbdFd, KIOCSLED, &leds) < 0 )
		ErrorF("Failed to set Keyboard LED's\n");
}
