/*
 * Copyright 1990 Open Software Foundation (OSF)
 * Copyright 1990 Unix International (UI)
 * Copyright 1990 X/Open Company Limited (X/Open)
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of OSF, UI or X/Open not be used in 
 * advertising or publicity pertaining to distribution of the software 
 * without specific, written prior permission.  OSF, UI and X/Open make 
 * no representations about the suitability of this software for any purpose.  
 * It is provided "as is" without express or implied warranty.
 *
 * OSF, UI and X/Open DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, 
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO 
 * EVENT SHALL OSF, UI or X/Open BE LIABLE FOR ANY SPECIAL, INDIRECT OR 
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF 
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR 
 * PERFORMANCE OF THIS SOFTWARE.
 */

/************************************************************************

SCCS:   	@(#)tet_fork.c	1.9 03/09/92
NAME:		'C' API fork process function
PRODUCT:	TET (Test Environment Toolkit)
AUTHOR:		Geoff Clare, UniSoft Ltd.
DATE CREATED:	26 July 1990
SYNOPSIS:

	int tet_fork(void (*childproc)(), void (*parentproc)(),
		     int waittime, int exitvals);

	pid_t tet_child;

	int tet_killw(pid_t child, unsigned timeout);
	char *tet_errname(int errno_val);
	char *tet_signame(int signum);

DESCRIPTION:

	Tet_fork() forks a child process and calls (*childproc)() in the
	child and (*parentproc)() (if != TET_NULLFP) in the parent.  The
	child calls tet_setcontext() to distinguish it's results file
	output from the parent's.  Calls to tet_setblock() are made in
	the parent to separate output made before, during and after
	execution of the child process.

	Waittime controls whether, and for how long, tet_fork() waits for
	the child after (*parentproc)() has returned.  If waittime < 0 no
	wait is done and the child is killed if it is still alive.  If
	waittime is zero tet_fork() waits indefinitely, otherwise the
	wait is timed out after waittime seconds.  If the child is going
	to be waited for, signals which are being caught in the parent
	are set to SIG_DFL in the child so unexpected signals will come
	through to the wait status.

	Exitvals is a bit mask of valid child exit codes.  Tet_fork()
	returns the child exit code (if valid), otherwise -1.  If
	(*childproc)() returns rather than exiting, or no wait was done
	then tet_fork() returns 0.  If tet_fork() returns -1 it first
	writes an information line and an UNRESOLVED result code to the
	execution results file.

	In the parent process tet_child is set to the PID of the child
	process.

	Tet_killw() is not part of the API.  It is used by other functions
	in the API to kill a child process (with SIGTERM) and wait for it.
	If the wait times out it will retry the kill() (but using SIGKILL)
	and the waitpid() a fixed number of times.  If the wait fails for
	any other reason -1 is returned.  If the child exits or was not
	there (ECHILD) then 0 is returned.  If the kill() fails and errno
	is ESRCH the wait is done anyway (to reap a possible zombie).  On
	return the value of errno is restored to the value set by the
	failed system call.

	Note that, because of the retries, the time spent in this routine
	may be much longer than the timeout given.

	Tet_errname() and tet_signame() are not part of the API.  They are
	used by API functions to obtain names for the standard errno values
	and signals respectively.

MODIFICATIONS:

	Geoff Clare, UniSoft Ltd., 6 Sept 1990
		Add tet_setblock() calls in parent.

	Geoff Clare, UniSoft Ltd., 5 Dec 1991
		Add arguments to alrm() and sig_term().

************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <tet_api.h>

#define	KILLWAIT	10
#define KILLTRIES	5

static	int	alrm_flag;
static	char	buf[256];

pid_t	tet_child;
char *	tet_errname();
char *	tet_signame();

/* ARGSUSED */
static void
alrm(sig)
int sig;
{
	alrm_flag++;
}

/* ARGSUSED */
static void
sig_term(sig)
int sig;
{
	/* clean up on receipt of SIGTERM, but arrange for wait
	   status still to show termination by SIGTERM */

	struct sigaction sa;

	if (tet_child > 0)
		(void) tet_killw(tet_child, KILLWAIT);

	sa.sa_handler = SIG_DFL;
	sa.sa_flags = 0; 
	(void) sigemptyset(&sa.sa_mask); 
	(void) sigaction(SIGTERM, &sa, (struct sigaction *)NULL);
	(void) kill(getpid(), SIGTERM);
}

int
tet_fork(childproc, parentproc, waittime, exitvals)
void (*childproc) ();
void (*parentproc) ();
int	waittime;
int	exitvals;
{
	int	rtval, err, status, i;
	pid_t	savchild;
	struct sigaction new_sa, old_sa; 

	(void) fflush(stdout);
	(void) fflush(stderr);

	/* Save old value of tet_child in case of recursive calls
	   to tet_fork().  RESTORE tet_child BEFORE ALL RETURNS. */
	savchild = tet_child;

	switch (tet_child = fork())
	{
	
	case -1:
		(void) sprintf(buf,
			"fork() failed in tet_fork() - errno %d (%s)",
			errno, tet_errname(errno));
		tet_infoline(buf);
		tet_result(TET_UNRESOLVED);
		tet_child = savchild;
		return -1;

	case 0:
		/* child process */

		if (waittime >= 0)
		{
			/* set signals which were caught in parent to default */

			/* NSIG is not provided by POSIX.1:  it must be
			   defined via an extra feature-test macro, or
			   on the compiler command line */
			for (i = 1; i < NSIG; i++)
			{
				if (sigaction(i, (struct sigaction *)NULL,
					&new_sa) != -1 &&
					new_sa.sa_handler != SIG_DFL &&
					new_sa.sa_handler != SIG_IGN)
				{
					new_sa.sa_handler = SIG_DFL;
					(void) sigaction(i, &new_sa,
						(struct sigaction *)NULL);
				}
			}
		}

		/* change context to distinguish output from parent's */
		tet_setcontext();

		/* call child function, and if it returns exit with code 0 */
		(*childproc) ();
		exit(0);
	}

	/* parent process */

	/* if SIGTERM is set to default (e.g. if this tet_fork() was
	   called from a child), catch it so we can propagate tet_killw() */
	if (sigaction(SIGTERM, (struct sigaction *)NULL, &new_sa) != -1 &&
		new_sa.sa_handler == SIG_DFL)
	{
		new_sa.sa_handler = sig_term;
		(void) sigaction(SIGTERM, &new_sa, (struct sigaction *)NULL);
	}

	if (parentproc != NULL)
	{
		tet_setblock();
		(*parentproc) ();
	}

	tet_setblock();

	/* negative waittime means no wait required (i.e. parentproc does
	   the wait, or the child is to be killed if still around) */

	if (waittime < 0)
	{
		(void) tet_killw(tet_child, KILLWAIT);
		tet_child = savchild;
		return 0;
	}

	/* wait for child, with timeout if required */

	if (waittime > 0)
	{
		new_sa.sa_handler = alrm; 
		new_sa.sa_flags = 0; 
		(void) sigemptyset(&new_sa.sa_mask); 
		(void) sigaction(SIGALRM, &new_sa, &old_sa);
		alrm_flag = 0; 
		(void) alarm((unsigned)waittime);
	}

	rtval = waitpid(tet_child, &status, WUNTRACED);
	err = errno; 

	if (waittime > 0)
	{
		(void) alarm(0); 
		(void) sigaction(SIGALRM, &old_sa, (struct sigaction *)NULL);
	}

	/* check child wait status shows valid exit code, if not
	   report wait status and give UNRESOLVED result */

	if (rtval == -1)
	{
		if (alrm_flag > 0)
			(void) sprintf(buf, "child process timed out");
		else
			(void) sprintf(buf, "waitpid() failed - errno %d (%s)",
				err, tet_errname(err));
		tet_infoline(buf);
		(void) tet_killw(tet_child, KILLWAIT);
	}
	else if (WIFEXITED(status))
	{
		status = WEXITSTATUS(status);

		if ((status & ~exitvals) == 0)
		{
			/* Valid exit code */

			tet_child = savchild;
			return status;
		}
		else
		{
			(void) sprintf(buf,
				"child process gave unexpected exit code %d",
				status);
			tet_infoline(buf);
		}
	}
	else if (WIFSIGNALED(status))
	{
		status = WTERMSIG(status);
		(void) sprintf(buf,
			"child process was terminated by signal %d (%s)",
			status, tet_signame(status));
		tet_infoline(buf);
	}
	else if (WIFSTOPPED(status))
	{
		status = WSTOPSIG(status);
		(void) sprintf(buf,
			"child process was stopped by signal %d (%s)",
			status, tet_signame(status));
		tet_infoline(buf);
		(void) tet_killw(tet_child, KILLWAIT);
	}
	else
	{
		(void) sprintf(buf,
			"child process returned bad wait status (%#x)", status);
		tet_infoline(buf);
	}

	tet_result(TET_UNRESOLVED);

	tet_child = savchild;
	return -1;
}

int
tet_killw(child, timeout)
pid_t child;
unsigned timeout;
{
	/* kill child and wait for it (with timeout) */

	pid_t	pid;
	int	sig = SIGTERM;
	int	ret = -1;
	int	err, count, status;
	struct sigaction new_sa, old_sa; 

	new_sa.sa_handler = alrm; 
	new_sa.sa_flags = 0; 
	(void) sigemptyset(&new_sa.sa_mask); 
	(void) sigaction(SIGALRM, &new_sa, &old_sa);

	for (count = 0; count < KILLTRIES; count++)
	{
		if (kill(child, sig) == -1 && errno != ESRCH)
		{
			err = errno;
			break;
		}

		alrm_flag = 0; 
		(void) alarm(timeout);
		pid = waitpid(child, &status, 0);
		err = errno;
		(void) alarm(0); 

		if (pid == child)
		{
			ret = 0;
			break;
		}
		if (pid == -1 && alrm_flag == 0 && errno != ECHILD)
			break;
		
		sig = SIGKILL; /* use a stronger signal on repeats */
	}

	(void) sigaction(SIGALRM, &old_sa, (struct sigaction *)NULL);

	errno = err;
	return ret;
}

char *
tet_errname(err)
int err;
{
	/* look up name for given errno value */

	int	i;
	static struct {
		int num;
		char *name;
	} err_table[] = {
		E2BIG,		"E2BIG",
		EACCES,		"EACCES",
		EAGAIN,		"EAGAIN",
		EBADF,		"EBADF",
		EBUSY,		"EBUSY",
		ECHILD,		"ECHILD",
		EDEADLK,	"EDEADLK",
		EDOM,		"EDOM",
		EEXIST,		"EEXIST",
		EFAULT,		"EFAULT",
		EFBIG,		"EFBIG",
		EINTR,		"EINTR",
		EINVAL,		"EINVAL",
		EIO,		"EIO",
		EISDIR,		"EISDIR",
		EMFILE,		"EMFILE",
		EMLINK,		"EMLINK",
		ENAMETOOLONG,	"ENAMETOOLONG",
		ENFILE,		"ENFILE",
		ENODEV,		"ENODEV",
		ENOENT,		"ENOENT",
		ENOEXEC,	"ENOEXEC",
		ENOLCK,		"ENOLCK",
		ENOMEM,		"ENOMEM",
		ENOSPC,		"ENOSPC",
		ENOSYS,		"ENOSYS",
		ENOTDIR,	"ENOTDIR",
		ENOTEMPTY,	"ENOTEMPTY",
		ENOTTY,		"ENOTTY",
		ENXIO,		"ENXIO",
		EPERM,		"EPERM",
		EPIPE,		"EPIPE",
		ERANGE,		"ERANGE",
		EROFS,		"EROFS",
		ESPIPE,		"ESPIPE",
		ESRCH,		"ESRCH",
		EXDEV,		"EXDEV",
#ifdef EIDRM
		EIDRM,		"EIDRM",
#endif
#ifdef ENOMSG
		ENOMSG,		"ENOMSG",
#endif
#ifdef ENOTBLK
		ENOTBLK,	"ENOTBLK",
#endif
#ifdef ETXTBSY
		ETXTBSY,	"ETXTBSY",
#endif
		0,		NULL
	};

	for (i = 0; err_table[i].name != NULL; i++)
	{
		if (err_table[i].num == err)
			return err_table[i].name;
	}

	return "unknown errno";
}

char *
tet_signame(sig)
int sig;
{
	/* look up name for given signal number */

	/* the table must contain standard signals only - a return
	   value not starting with "SIG" is taken to indicate a
	   non-standard signal */

	int	i;
	static	struct {
		int num;
		char *name;
	} sig_table[] = {
		SIGABRT,	"SIGABRT",
		SIGALRM,	"SIGALRM",
		SIGCHLD,	"SIGCHLD",
		SIGCONT,	"SIGCONT",
		SIGFPE,		"SIGFPE",
		SIGHUP,		"SIGHUP",
		SIGILL,		"SIGILL",
		SIGINT,		"SIGINT",
		SIGKILL,	"SIGKILL",
		SIGPIPE,	"SIGPIPE",
		SIGQUIT,	"SIGQUIT",
		SIGSEGV,	"SIGSEGV",
		SIGSTOP,	"SIGSTOP",
		SIGTERM,	"SIGTERM",
		SIGTSTP,	"SIGTSTP",
		SIGTTIN,	"SIGTTIN",
		SIGTTOU,	"SIGTTOU",
		SIGUSR1,	"SIGUSR1",
		SIGUSR2,	"SIGUSR2",
		0,		NULL
	};


	for (i = 0; sig_table[i].name != NULL; i++)
	{
		if (sig_table[i].num == sig)
			return sig_table[i].name;
	}

	return "unknown signal";
}
