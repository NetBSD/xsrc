/* $Xorg: daemon.c,v 1.4 2001/02/09 02:05:40 xorgcvs Exp $ */
/*

Copyright 1988, 1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/
/* $XFree86: xc/programs/xdm/daemon.c,v 3.17 2001/12/14 20:01:21 dawes Exp $ */

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 */

#include <X11/Xos.h>

#if defined(SVR4) || defined(USG)
#include <termios.h>
#else
#include <sys/ioctl.h>
#endif
#if defined(__osf__) || defined(linux) || defined(__GNU__) || defined(__CYGWIN__)
#define setpgrp setpgid
#endif
#ifdef hpux
#include <sys/ptyio.h>
#endif
#include <errno.h>
#include <sys/types.h>
#ifdef X_NOT_POSIX
#define Pid_t int
#else
#define Pid_t pid_t
#endif

#include <stdlib.h>

#include "dm.h"
#include "dm_error.h"

void
BecomeOrphan (void)
{
    Pid_t child_id;
#ifndef CSRG_BASED
    int stat;
#endif

    /*
     * fork so that the process goes into the background automatically. Also
     * has a nice side effect of having the child process get inherited by
     * init (pid 1).
     * Separate the child into its own process group before the parent
     * exits.  This eliminates the possibility that the child might get
     * killed when the init script that's running xdm exits.
     */

    child_id = fork();
    switch (child_id) {
    case 0:
	/* child */
	break;
    case -1:
	/* error */
	LogError("daemon fork failed, errno = %d\n", errno);
	break;

    default:
	/* parent */

#ifndef CSRG_BASED
#if defined(SVR4) || defined(__QNXNTO__)
	stat = setpgid(child_id, child_id);
	/* This gets error EPERM.  Why? */
#else
#if defined(SYSV)
	stat = 0;	/* don't know how to set child's process group */
#else
	stat = setpgrp(child_id, child_id);
	if (stat != 0)
	    LogError("setting process grp for daemon failed, errno = %d\n",
		     errno);
#endif
#endif
#endif /* !CSRG_BASED */
	exit (0);
    }
}

void
BecomeDaemon (void)
{
#ifndef CSRG_BASED
    register int i;

    /*
     * Close standard file descriptors and get rid of controlling tty
     */

#if defined(SYSV) || defined(SVR4) || defined(__QNXNTO__)
    setpgrp ();
#else
    setpgrp (0, getpid());
#endif

    close (0); 
    close (1);
    close (2);

#ifndef __EMX__
#if !((defined(SYSV) || defined(SVR4)) && defined(i386)) && !defined(__CYGWIN__)
    if ((i = open ("/dev/tty", O_RDWR)) >= 0) {	/* did open succeed? */
#if defined(USG) && defined(TCCLRCTTY)
	int zero = 0;
	(void) ioctl (i, TCCLRCTTY, &zero);
#else
#if (defined(SYSV) || defined(SVR4)) && defined(TIOCTTY)
	int zero = 0;
	(void) ioctl (i, TIOCTTY, &zero);
#else
	(void) ioctl (i, TIOCNOTTY, (char *) 0);    /* detach, BSD style */
#endif
#endif
	(void) close (i);
    }
#endif /* !((SYSV || SVR4) && i386) */
#endif /* !__EMX__ */

    /*
     * Set up the standard file descriptors.
     */
    (void) open ("/", O_RDONLY);	/* root inode already in core */
    (void) dup2 (0, 1);
    (void) dup2 (0, 2);
#else
    daemon (0, 0);
#endif /* CSRG_BASED */
}
