/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/cygwin/cygwin_init.c,v 1.2 2000/08/11 23:59:48 dawes Exp $ */
/*
 * Copyright 1990,91 by Thomas Roell, Dinkelscherben, Germany
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 * Copyright 1999 by David Holland <davidh@iquest.net>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Thomas Roell and David Wexelblat 
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  Thomas Roell and
 * David Wexelblat makes no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * DAVID HOLLAND, THOMAS ROELL AND DAVID WEXELBLAT DISCLAIMS ALL 
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL 
 * DAVID HOLLAND, THOMAS ROELL OR DAVID WEXELBLAT BE LIABLE FOR 
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF 
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XConsortium: cygwin_init.c /main/4 1996/02/21 17:54:10 kaleb $ */

#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#include "X.h"
#include "Xmd.h"

#include "scrnintstr.h"
#include <errno.h>
#include <sys/mman.h>
#include "compiler.h"

#include "xf86.h"
#include "xf86Priv.h"
#include "xf86_OSlib.h"

static Bool KeepTty = FALSE;

static Bool Protect0 = FALSE;


char	fb_dev[PATH_MAX] = "/dev/conin";

#define MAX_SECONDS	60	
#define USEC_IN_SEC     (unsigned long)1000000


void
xf86OpenConsole()
{
    int fd;
    int i;
    MessageType from = X_PROBED;

    if (serverGeneration == 1) 
    {
    	/* check if we're run with euid==0 */
    	if (geteuid() != 0)
	{
      	    FatalError("xf86OpenConsole: Server must be suid root\n");
	}

	/* Protect page 0 to help find NULL dereferencing */
	/* mprotect() doesn't seem to work */
	if (Protect0)
	{
	    int fd = -1;

	    if ((fd = open("/dev/zero", O_RDONLY, 0)) < 0)
	    {
		xf86Msg(X_WARNING,
			"xf86OpenConsole: cannot open /dev/zero (%s)\n",
			strerror(errno));
	    }
	    else
	    {
		if ((int)mmap(0, 0x1000, PROT_NONE,
			      MAP_FIXED | MAP_SHARED, fd, 0) == -1)
		{
		    xf86Msg(X_WARNING,
			"xf86OpenConsole: failed to protect page 0 (%s)\n",
			strerror(errno));
		}
		close(fd);
	    }
	}

	if (!KeepTty)
    	{
    	    setpgrp();
	}

	if (((xf86Info.consoleFd = open(fb_dev, O_RDWR | O_NDELAY, 0)) < 0))
	{
            FatalError("xf86OpenConsole: Cannot open %s (%s)\n",
		       fb_dev, strerror(errno));
	}

    }
    return;
}

void xf86CloseConsole()
{

	int 	tmp;

	close(xf86Info.consoleFd);

	return;
}

int xf86ProcessArgument(argc, argv, i)
int argc;
char *argv[];
int i;
{
	/*
	 * Keep server from detaching from controlling tty.  This is useful 
	 * when debugging (so the server can receive keyboard signals.
	 */
	if (!strcmp(argv[i], "-keeptty"))
	{
		KeepTty = TRUE;
		return(1);
	}
	/*
	 * Undocumented flag to protect page 0 from read/write to help
	 * catch NULL pointer dereferences.  This is purely a debugging
	 * flag.
	 */
	if (!strcmp(argv[i], "-protect0"))
	{
		Protect0 = TRUE;
		return(1);
	}
	if (!strcmp(argv[i], "-dev")  && i+1 < argc) {
		strncpy(fb_dev, argv[i+1], PATH_MAX);
		fb_dev[PATH_MAX-1] = '\0';
		return(2);
	}


	return(0);
}

void xf86UseMsg()
{
	ErrorF("-ar1 <float> 	       Set autorepeat initiate time (sec)\n");
	ErrorF("                       (If not using XKB)\n");
	ErrorF("-ar2 <float>           Set autorepeat interval time (sec)\n");
	ErrorF("                       (If not using XKB)\n");
	ErrorF("-dev <fb>              FrameBuffer device\n");
	ErrorF("-keeptty               ");
	ErrorF("don't detach controlling tty (for debugging only)\n");
	return;
}

