/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/sol8_x86/sol8_kbdEv.c,v 1.1 1999/09/25 14:38:09 dawes Exp $ */
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
/* $XConsortium: std_kbdEv.c /main/4 1996/03/11 10:47:33 kaleb $ */

#include "X.h"
#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"


#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stream.h>
#include <sys/stropts.h>
#include <sys/vuid_event.h>
#include <sys/kbio.h>
#include <sys/kbd.h>
#include <termio.h>

extern sol8PostKbdEvent(Firm_event *event);

/* Lets try reading more than one keyboard event at a time in the 
 * hopes that this will be slightly more efficient. 
 * Or we could just try the MicroSoft method, and forget about
 * efficiency. :-)
 */
void
xf86KbdEvents()
{
	Firm_event	event[64];

	int nBytes, i;

/* I certainly hope its not possible to read partial events.  */

	if ((nBytes = read( xf86Info.kbdFd, (char *)event, sizeof(event)))
	    > 0)
	{
		for (i = 0; i < (nBytes / sizeof(Firm_event)); i++)
			sol8PostKbdEvent(&event[i]);
	}
}

