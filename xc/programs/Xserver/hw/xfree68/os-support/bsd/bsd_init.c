/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/bsd/bsd_init.c,v 3.7 1996/09/03 04:13:16 dawes Exp $ */
/*
 * Copyright 1992 by Rich Murphey <Rich@Rice.edu>
 * Copyright 1993 by David Wexelblat <dwex@goblin.org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the names of Rich Murphey and David Wexelblat 
 * not be used in advertising or publicity pertaining to distribution of 
 * the software without specific, written prior permission.  Rich Murphey and
 * David Wexelblat make no representations about the suitability of this 
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 *
 * RICH MURPHEY AND DAVID WEXELBLAT DISCLAIM ALL WARRANTIES WITH REGARD TO 
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS, IN NO EVENT SHALL RICH MURPHEY OR DAVID WEXELBLAT BE LIABLE FOR 
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER 
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF 
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN 
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */
/* $XConsortium: bsd_init.c /main/5 1995/11/13 05:53:59 kaleb $ */

#include "X.h"
#include "Xmd.h"
#include "input.h"
#include "scrnintstr.h"

#include "compiler.h"

#include "xf86.h"
#include "xf86Procs.h"
#include "xf86_OSlib.h"

extern void xf86VTRequest(
#if NeedFunctionPrototypes
	int
#endif
);

static Bool KeepTty = FALSE;
static int devGrfFd = -1;
static char *grfDevice = "/dev/grf0";
static int VTnum = -1;
static int initialVT = -1;

#define CHECK_DRIVER_MSG \
  "Check your kernel's console driver configuration and /dev entries"


/*
 * Functions to probe for the existance of a supported console driver.
 * Any function returns either a valid file descriptor (driver probed
 * succesfully), -1 (driver not found), or uses FatalError() if the
 * driver was found but proved to not support the required mode to run
 * an X server.
 */

typedef int (*xf86ConsOpen_t)(
#if NeedFunctionPrototypes
    void
#endif
);

void
xf86OpenConsole()
{
    int i, fd, temp;
    xf86ConsOpen_t *driver;
    
    if (serverGeneration == 1)
    {
	/* check if we are run with euid==0 */
	if (geteuid() != 0)
	{
	    FatalError("xf86OpenConsole: Server must be suid root\n");
	}

	if (!KeepTty)
	{
	    /*
	     * detaching the controlling tty solves problems of kbd character
	     * loss.  This is not interesting for CO driver, because it is 
	     * exclusive.
	     */
	    setpgrp(0, getpid());
	    if ((i = open("/dev/tty",O_RDWR)) >= 0)
	    {
		ioctl(i,TIOCNOTTY,(char *)0);
		close(i);
	    }
	}

	fd = open("/dev/kbd", O_RDWR|O_NONBLOCK, 0);
	if (fd < 0)
	{
	    FatalError(
		"%s: Can't open kbd device: %s\n\t%s\n",
		"xf86OpenConsole", "/dev/kbd", CHECK_DRIVER_MSG);
	}

	temp = 1;
	if (ioctl(fd, KIOCSDIRECT, &temp) < 0)
	{
	    close(fd);
	    FatalError("%s: Can't set kbd direct mode\n", "xf86OpenConsole");
	}
	temp = TR_UNTRANS_EVENT;
	if (ioctl(fd, KIOCTRANS, &temp) < 0)
	{
	    close(fd);
	    FatalError("%s: Can't set kbd translation\n", "xf86OpenConsole");
	}
	xf86Info.consoleFd = fd;
	fclose(stdin);

	/* open grf device */
	if (devGrfFd < 0) {
	    devGrfFd = open(grfDevice, O_RDWR, 0);

	    /* Check that a supported graphic driver was found */
	    if (devGrfFd < 0)
	    {
		FatalError(
		    "%s: Can't open grf device: %s\n\t%s\n",
		    "xf86OpenConsole", grfDevice, CHECK_DRIVER_MSG);
	    }
	    xf86Info.screenFd = devGrfFd;

	    if (ioctl(devGrfFd, GRFIOCON, 0) == -1)
		FatalError("xf68OpenGrfdev: Can't switch to grf mode\n");
	}

	xf86Config(FALSE); /* Read XF86Config */
    }
    else 
    {
	/* serverGeneration != 1 */
    }
    return;
}


void
xf86CloseConsole()
{
    int temp = 0;

    if (devGrfFd > -1) {
    	(void)ioctl(devGrfFd, GRFIOCOFF, 0);
    	close(devGrfFd);
    }
    (void)ioctl(xf86Info.consoleFd, KIOCSDIRECT, &temp);
    temp = TR_UNTRANS_EVENT;
    (void)ioctl(xf86Info.consoleFd, KIOCTRANS, &temp);
    close(xf86Info.consoleFd);

    return;
}

int
xf86ProcessArgument (argc, argv, i)
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
	if (!strcmp(argv[i], "-grfdev"))
	{
		if ((i + 1) < argc)
		{
			grfDevice = argv[i + 1];
			return(2);
		}
		return(1);
	}
	return(0);
}

void
xf86UseMsg()
{
	ErrorF("-grfdev device       use the specified grf device\n");
	ErrorF("-keeptty               ");
	ErrorF("don't detach controlling tty (for debugging only)\n");
	return;
}
