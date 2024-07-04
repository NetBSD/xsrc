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

/*
 * xdm - display manager daemon
 * Author:  Keith Packard, MIT X Consortium
 *
 * server.c - manage the X server
 */

#include	"dm.h"
#include	"dm_error.h"
#include 	"dm_socket.h"

#include	<X11/Xlib.h>
#include	<X11/Xos.h>
#include	<stdio.h>
#include	<signal.h>
#include	<errno.h>
#include 	<sys/socket.h>

static int receivedUsr1;

static int serverPause (unsigned t, pid_t serverPid);

/* ARGSUSED */
static void
CatchUsr1 (int n)
{
    int olderrno = errno;

    Debug ("display manager caught SIGUSR1\n");
    ++receivedUsr1;
    errno = olderrno;
}

const char *_SysErrorMsg (int n)
{
    const char *s = strerror(n);
    return (s ? s : "unknown error");
}

static int
StartServerOnce (struct display *d)
{
    char	**f;
    char	**argv;
    char	arg[1024];
    pid_t	pid;

    Debug ("StartServer for %s\n", d->name);
    receivedUsr1 = 0;
    (void) Signal (SIGUSR1, CatchUsr1);
    argv = d->argv;
    switch (pid = fork ()) {
    case 0:
	CleanUpChild ();
#ifdef XDMCP
	/* The chooser socket is not closed by CleanUpChild() */
	DestroyWellKnownSockets();
#endif
	if (d->authFile) {
	    snprintf (arg, sizeof(arg), "-auth %s", d->authFile);
	    argv = parseArgs (argv, arg);
	}
	if (!argv) {
	    LogError ("StartServer: no arguments\n");
	    sleep ((unsigned) d->openDelay);
	    exit (UNMANAGE_DISPLAY);
	}
	for (f = argv; *f; f++)
	    Debug ("'%s' ", *f);
	Debug ("\n");
	/*
	 * give the server SIGUSR1 ignored,
	 * it will notice that and send SIGUSR1
	 * when ready
	 */
	(void) Signal (SIGUSR1, SIG_IGN);
	(void) execv (argv[0], argv);
	LogError ("server %s cannot be executed\n",
			argv[0]);
	sleep ((unsigned) d->openDelay);
	exit (REMANAGE_DISPLAY);
    case -1:
	LogError ("fork failed, sleeping\n");
	return 0;
    default:
	break;
    }
    Debug ("Server Started %d\n", pid);
    d->serverPid = pid;
    if (serverPause ((unsigned) d->openDelay, pid))
	return FALSE;
    return TRUE;
}

int
StartServer (struct display *d)
{
    int	i;
    int	ret = FALSE;

    i = 0;
    while (d->serverAttempts == 0 || i < d->serverAttempts)
    {
	if ((ret = StartServerOnce (d)) == TRUE)
	    break;
	sleep (d->openDelay);
	i++;
    }
    return ret;
}

/*
 * sleep for t seconds, return 1 if the server is dead when
 * the sleep finishes, 0 else
 */

static Jmp_buf	pauseAbort;
static int	serverPauseRet;

/* ARGSUSED */
_X_NORETURN
static void
serverPauseAbort (int n)
{
    Longjmp (pauseAbort, 1);
}

/* ARGSUSED */
_X_NORETURN
static void
serverPauseUsr1 (int n)
{
    Debug ("display manager paused til SIGUSR1\n");
    ++receivedUsr1;
    Longjmp (pauseAbort, 1);
}

static int
serverPause (unsigned t, pid_t serverPid)
{
    pid_t	pid;

    serverPauseRet = 0;
    if (!Setjmp (pauseAbort)) {
	(void) Signal (SIGALRM, serverPauseAbort);
	(void) Signal (SIGUSR1, serverPauseUsr1);
	if (!receivedUsr1)
	    (void) alarm (t);
	else
	    Debug ("Already received USR1\n");
	for (;;) {
	    /*
	     * wait() is unsafe.  Other Xserver or xdm processes may
	     * exit at this time and this will remove the wait status.
	     * This means the main loop will not restart the display.
	     */
	    if (!receivedUsr1)
		pid = waitpid (serverPid, (int *) 0, 0);
	    else
		pid = waitpid (serverPid, (int *) 0, WNOHANG);

	    if (pid == serverPid ||
	       (pid == -1 && errno == ECHILD))
	    {
		Debug ("Server dead\n");
		serverPauseRet = 1;
		break;
	    }

	    if (pid == 0) {
		Debug ("Server alive and kicking\n");
		break;
	    }
	}
    }
    (void) alarm ((unsigned) 0);
    (void) Signal (SIGALRM, SIG_DFL);
    (void) Signal (SIGUSR1, CatchUsr1);
    if (serverPauseRet) {
	Debug ("Server died\n");
	LogError ("server unexpectedly died\n");
    }
    return serverPauseRet;
}


/*
 * this code is complicated by some TCP failings.  On
 * many systems, the connect will occasionally hang forever,
 * this trouble is avoided by setting up a timeout to Longjmp
 * out of the connect (possibly leaving piles of garbage around
 * inside Xlib) and give up, terminating the server.
 */

static Jmp_buf	openAbort;

/* ARGSUSED */
_X_NORETURN
static void
abortOpen (int n)
{
	Longjmp (openAbort, 1);
}

#ifdef XDMCP


static void
GetRemoteAddress (struct display *d, int fd)
{
    char    buf[512];
    int	    len = sizeof (buf);

    free (d->peer);
    getpeername (fd, (struct sockaddr *) buf, (void *)&len);
    d->peerlen = 0;
    if (len)
    {
	d->peer = malloc (len);
	if (d->peer)
	{
	    memcpy(d->peer, buf, len);
	    d->peerlen = len;
	}
    }
    Debug ("Got remote address %s %d\n", d->name, d->peerlen);
}

#endif /* XDMCP */

static int
openErrorHandler (Display *dpy)
{
    LogError ("IO Error in XOpenDisplay\n");
    exit (OPENFAILED_DISPLAY);
    /*NOTREACHED*/
    return(0);
}

int
WaitForServer (struct display *d)
{
    static int i;

    for (i = 0; i < (d->openRepeat > 0 ? d->openRepeat : 1); i++) {
	(void) Signal (SIGALRM, abortOpen);
	(void) alarm ((unsigned) d->openTimeout);
	if (!Setjmp (openAbort)) {
	    Debug ("Before XOpenDisplay(%s)\n", d->name);
	    errno = 0;
	    (void) XSetIOErrorHandler (openErrorHandler);
	    d->dpy = XOpenDisplay (d->name);
	    (void) alarm ((unsigned) 0);
	    (void) Signal (SIGALRM, SIG_DFL);
	    (void) XSetIOErrorHandler ((int (*)(Display *)) 0);
	    Debug ("After XOpenDisplay(%s)\n", d->name);
	    if (d->dpy) {
#ifdef XDMCP
		if (d->displayType.location == Foreign)
		    GetRemoteAddress (d, ConnectionNumber (d->dpy));
#endif
		RegisterCloseOnFork (ConnectionNumber (d->dpy));
		(void) fcntl (ConnectionNumber (d->dpy), F_SETFD, 0);
		return 1;
	    } else {
		Debug ("OpenDisplay failed %d (%s) on \"%s\"\n",
		       errno, strerror (errno), d->name);
	    }
	    Debug ("waiting for server to start %d\n", i);
	    sleep ((unsigned) d->openDelay);
	} else {
	    Debug ("hung in open, aborting\n");
	    LogError ("Hung in XOpenDisplay(%s), aborting\n", d->name);
	    (void) Signal (SIGALRM, SIG_DFL);
	    break;
	}
    }
    Debug ("giving up on server\n");
    LogError ("server open failed for %s, giving up\n", d->name);
    return 0;
}

void
ResetServer (struct display *d)
{
    if (d->dpy && d->displayType.origin != FromXDMCP)
	pseudoReset (d->dpy);
}

static Jmp_buf	pingTime;

_X_NORETURN
static void
PingLost (void)
{
    Longjmp (pingTime, 1);
}

/* ARGSUSED */
static int
PingLostIOErr (Display *dpy)
{
    PingLost();
    return 0;
}

/* ARGSUSED */
_X_NORETURN
static void
PingLostSig (int n)
{
    PingLost();
}

int
PingServer (struct display *d, Display *alternateDpy)
{
    int	    (*oldError)(Display *);
    void  (*oldSig)(int);
    int	    oldAlarm;
    static Display *aDpy;

    aDpy = (alternateDpy != NULL ? alternateDpy : d->dpy);
    oldError = XSetIOErrorHandler (PingLostIOErr);
    oldAlarm = alarm (0);
    oldSig = Signal (SIGALRM, PingLostSig);
    (void) alarm (d->pingTimeout * 60);
    if (!Setjmp (pingTime))
    {
	Debug ("Ping server\n");
	XSync (aDpy, 0);
    }
    else
    {
	Debug ("Server dead\n");
	(void) alarm (0);
	(void) Signal (SIGALRM, SIG_DFL);
	XSetIOErrorHandler (oldError);
	return 0;
    }
    (void) alarm (0);
    (void) Signal (SIGALRM, oldSig);
    (void) alarm (oldAlarm);
    Debug ("Server alive\n");
    XSetIOErrorHandler (oldError);
    return 1;
}
