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

#ifndef lint
static	char sccsid[] = "@(#)TCM 1.9 03/09/92";
#endif

/************************************************************************

SCCS:   	@(#)tcm.c	1.9 03/09/92
NAME:		'C' Test Case Manager
PRODUCT:	TET (Test Environment Toolkit)
AUTHOR:		Geoff Clare, UniSoft Ltd.
DATE CREATED:	23 July 1990
SYNOPSIS:

	int main(int argc, char **argv);
	int tet_thistest;

DESCRIPTION:

	Main() is the main program for the Test Case Manager (TCM).
	The command line contains the invocable components (IC's) to be
	executed.  All test purposes in the user-supplied tet_testlist[]
	array with an icref element corresponding to one of the requested
	IC's are called, preceded by a call to the function pointed to
	by tet_startup and followed by tet_cleanup (if each is not NULL).
	The TCM reads configuration variables from the file specified
	by the environment variable TET_CONFIG, provides handling of
	unexpected signals, and reports the start and end of IC's and
	the start of test purposes.

	Tet_thistest is set to the test purpose number during execution
	of each test purpose.  Test purposes are numbered in the sequence
	of the tet_testlist[] array (starting at 1) regardless of
	execution sequence.

MODIFICATIONS:

	Geoff Clare, UniSoft Ltd, 6 Sept 1990
		New results file format.
		If no arguments assume all ICs.
		Move tet_error() to resfile.c.
		Change handling of ICs with zero TPs.

	Geoff Clare, UniSoft Ltd, 28 Sept 1990
		When doing all ICs start from lowest defined IC instead of 1.
	
	Geoff Clare, UniSoft Ltd, 28 Nov 1990
		Add tet_nosigreset.
	
	Geoff Clare, UniSoft Ltd, 6 Sept 1991
		Add call to tet_delreas().

************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <setjmp.h>
#include <tet_api.h>

#define	VERSION		"1.9"
#define ARGS_COUNT	0
#define ARGS_EXECUTE	1
#define	KILLWAIT	10

extern	void	(*tet_startup)();
extern	void	(*tet_cleanup)();
extern	struct tet_testlist
		tet_testlist[];

extern	void	tet_openres();
extern	void	tet_config();
extern	void	tet_delreas();
extern	void	tet_tcmstart();
extern	void	tet_icstart();
extern	void	tet_icend();
extern	void	tet_tpstart();
extern	void	tet_tpend();
extern	void	tet_error();
extern	int	tet_killw();
extern	char *	tet_errname();
extern	char *	tet_signame();
extern	long	tet_block;

static	pid_t	toppid;
static	int	sigreset = 1;
static	sigjmp_buf skipjmp;
static	int	procargs();
static	int	count_tps();
static	int	callfuncs();
static	void	sigskip();
static	void	sigabandon();
static	void	sigterm();
static	void	setsigs();

static	char	buf[256];

char *	tet_pname;
int	tet_thistest = 0;
int	tet_nosigreset = 0;

/* ARGSUSED */
int
main(argc, argv)
int	argc;
char	**argv;
{
	tet_pname = argv[0]; /* used by tet_error() */

	/* save current PID for checking when unexpected signals
	   cause a longjmp() to main test loop */
	toppid = getpid();

	/* open execution results file (do first, so tet_error() can use it) */
	tet_openres();

	/* read in configuration variables */
	tet_config();

	/* set up data for use by tet_delete() and tet_reason() */
	tet_delreas(count_tps(-1));

	/* set current context to process ID */
	tet_setcontext();

	/* count number of IC's to be executed and output on TCM Start line */
	tet_tcmstart(VERSION, procargs(argv, ARGS_COUNT));

	/* unexpected signals are fatal during startup */
	setsigs(sigabandon);

	/* call user-supplied startup function */
	if (tet_startup != NULL)
		(*tet_startup)();
	
	/* see whether tet_nosigreset was set in startup function */
	if (tet_nosigreset)
		sigreset = 0;
	
	/* execute IC's specified on command line */
	(void) procargs(argv, ARGS_EXECUTE);

	/* unexpected signals are fatal during cleanup */
	setsigs(sigabandon);

	/* call user-supplied cleanup function */
	if (tet_cleanup != NULL)
	{
		tet_thistest = 0;
		tet_block = 0;
		tet_setblock();
		(*tet_cleanup)();
	}

	exit(EXIT_SUCCESS);

	/* NOTREACHED */
}

static int
procargs(argv, action)
char **argv;
int action;
{
	/*
	 * Process the IC list given on the command line.  If action is
	 * ARGS_COUNT simply return an IC count.  If action is ARGS_EXECUTE
	 * then execute each IC.
	 */

	char *cp;
	struct tet_testlist *tp;
	int ic_no, ic_first, ic_last, ic_count, testcnt;
	static char *default_args[] = { "arg0", "all", NULL };

	/*
	 * Each argument contains an invocable component list of the
	 * form: 1,3-5,7,11,... 
	 *
	 * If any argument is "all" then all components later than
	 * the last in the previous argument are executed.
	 *
	 * Default if no arguments are given is "all".
	 */

	if (argv[1] == NULL)
		argv = default_args;

	ic_last = -1;
	ic_count = 0;
	while (*++argv != NULL)
	{
		if (strncmp(*argv, "all", (size_t)3) == 0)
		{
			if (ic_last < 0)
			{
				/* no previous ICs executed - start at lowest */
				ic_first = tet_testlist[0].icref;
				for (tp = tet_testlist;
						tp->testfunc != NULL; tp++)
				{
					if (tp->icref < ic_first)
						ic_first = tp->icref;
				}
			}
			else
				/* carry on from last IC executed */
				ic_first = ic_last + 1;

			ic_last = ic_first;
			for (tp = tet_testlist; tp->testfunc != NULL; tp++)
			{
				if (tp->icref > ic_last)
					ic_last = tp->icref;
			}

			for (ic_no = ic_first; ic_no <= ic_last; ic_no++)
			{
				testcnt = count_tps(ic_no);

				/* skip ICs with zero TPs */
				if (testcnt == 0)
					continue;

				ic_count++;
				if (action == ARGS_EXECUTE)
				{
					/* output IC start message */
					tet_icstart(ic_no, testcnt);

					testcnt = callfuncs(ic_no);

					/* output IC end message */
					tet_icend(ic_no, testcnt);
				}
			}
		}
		else
		{
			for (cp = *argv; *cp != '\0'; cp++)
			{
				ic_first = atoi(cp);
				while(*cp != ',' && *cp != '-' && *cp != '\0')
					cp++;
				if (*cp == '-')
				{
					ic_last = atoi(++cp);
					while(*cp != ',' && *cp != '\0')
						cp++;
				}
				else
					ic_last = ic_first;

				for (ic_no = ic_first;
						ic_no <= ic_last; ic_no++)
				{
					testcnt = count_tps(ic_no);

					/* skip ICs with zero TPs */
					if (testcnt == 0)
					{
						if (action == ARGS_EXECUTE &&
						   (ic_no == ic_first ||
						    ic_no == ic_last))
						{
							(void) sprintf(buf,
				    "IC %d is not defined for this test case",
								ic_no);
							tet_error(0, buf);
						}
						continue;
					}

					ic_count++;
					if (action == ARGS_EXECUTE)
					{
						/* output IC start message */
						tet_icstart(ic_no, testcnt);

						testcnt = callfuncs(ic_no);

						/* output IC end message */
						tet_icend(ic_no, testcnt);
					}
				}

				if (*cp == '\0')
					break;
			}
		}
	}

	return ic_count;
}

static int
count_tps(ic_no)
int ic_no;
{
	/* count number of tests in specified invocable component */
	/* or in all ICs if ic_no < 0 */

	struct tet_testlist *tp;
	int testcnt = 0;

	for (tp = tet_testlist; tp->testfunc != NULL; tp++)
	{
		if (ic_no < 0 || tp->icref == ic_no)
			testcnt++;
	}

	return testcnt;
}

static int
callfuncs(ic_no)
int ic_no;
{
	/* call each test function in specified invocable component */
	/* return count of test functions called */

	struct tet_testlist *tp;
	int test_no, testcnt;
	int signum;

	testcnt = 0;
	for (test_no = 1, tp = tet_testlist;
			tp->testfunc != NULL; test_no++, tp++)
	{
		if (tp->icref != ic_no)
			continue;
		else if ((signum = sigsetjmp(skipjmp, 1)) != 0)
		{
			(void) sprintf(buf,
				"unexpected signal %d (%s) received",
				signum-1, tet_signame(signum-1));
			tet_infoline(buf);
			tet_result(TET_UNRESOLVED);
			if (tet_child > 0)
			{
				(void) tet_killw(tet_child, KILLWAIT);
				tet_child = 0;
			}

			/* if this is not the top level process, don't fall
			   through to TCM test purpose loop! */
			if (getpid() != toppid)
				exit(EXIT_FAILURE);

			if (signum-1 == SIGTERM)
				sigterm();

			/* output unresolved result and go on to next test */
			tet_tpend(test_no);
			testcnt++;
			continue;
		}

		/* output test start message */
		tet_tpstart(test_no);

		/* set global current test purpose indicator */
		tet_thistest = test_no;

		if (tet_reason(test_no) != NULL)
		{
			tet_infoline(tet_reason(test_no));
			tet_result(TET_UNINITIATED);
		}
		else
		{
			/* unexpected signals skip current test
			   NOTE: this is done before every test function
			   to ensure no "local" signal handlers are left
			   in place when skipping to the next test.
			   This safety feature can be disabled by setting
			   tet_nosigreset in the tet_startup function,
			   in which case unexpected signals in later
			   tests could go unnoticed. */
			if (sigreset)
				setsigs(sigskip);

			/* call the user-supplied test function */
			(*tp->testfunc)();
		}

		/* output test result */
		tet_tpend(test_no);

		testcnt++;
	}

	return testcnt;
}

static	void
sigabandon(sig)
int	sig;
{
	static	char	mbuf[132];

	if (sig == SIGTERM)
		sigterm();

	(void) sprintf(mbuf,
		"Abandoning testset: caught unexpected signal %d (%s)",
		sig, tet_signame(sig));
	tet_error(0, mbuf);
	exit(EXIT_FAILURE);
}

static	void
sigterm()
{
	/*  Cleanup and exit if SIGTERM received */

	char *msg = "Abandoning test case: received signal SIGTERM";

	tet_error(0, msg);
	if (tet_child > 0)
	{
		(void) tet_killw(tet_child, KILLWAIT);
		tet_child = 0;
	}
	/* call user-supplied cleanup function */
	if (tet_cleanup != NULL)
		(*tet_cleanup)();

	exit(EXIT_FAILURE);
}

static	void
sigskip(sig)
int sig;
{
	/*
	 * Catch unexpected signals and longjmp() to skipjmp where
	 * the setjmp() return value will be (sig+1).
	 */

	siglongjmp(skipjmp, sig+1);
}


static void
sig_init(var, set)
char *var;
sigset_t *set;
{
	/* initialise signal set from list in specified variable */

	/* note that this routine uses strtok() which will alter the
	   contents of the list variable */

	char	*list, *sname;
	int	snum;

	(void) sigemptyset(set);

	list = tet_getvar(var);
	if (list == NULL)
		return;

	for (sname = strtok(list, ", "); sname != NULL;
					sname = strtok((char *) NULL, ", "))
	{
		snum = atoi(sname);

		/* Check it's not a standard signal */
		if (strncmp(tet_signame(snum), "SIG", (size_t)3) == 0)
		{
			(void) sprintf(buf,
			    "warning: illegal entry \"%s\" in %s ignored",
			    sname, var);
			tet_error(0, buf);
		}
		else if (sigaddset(set, snum) == -1)
		{
			(void) sprintf(buf,
			    "warning: sigaddset() failed on entry \"%s\" in %s",
			    sname, var);
			tet_error(0, buf);
		}
	}
}

static	void
setsigs(func)
void	(*func)();
{
	/*
	 * Sets all signals except SIGKILL, SIGSTOP and SIGCHLD
	 * to be caught by "func", except that signals specified in
	 * TET_SIG_IGN are ignored and signals specified in
	 * TET_SIG_LEAVE are left alone.
	 */

	int i;
	struct sigaction sig;
	static sigset_t	sig_ign;
	static sigset_t	sig_leave;
	static int	init_done = 0;

	if (!init_done)
	{
		sig_init("TET_SIG_IGN", &sig_ign);
		sig_init("TET_SIG_LEAVE", &sig_leave);
		init_done = 1;
	}

	/* NSIG is not provided by POSIX.1:  it must be defined via
	   an extra feature-test macro, or on the compiler command line */
	for (i = 1; i < NSIG; i++)
	{
		if (i == SIGKILL || i == SIGSTOP || i == SIGCHLD
					|| sigismember(&sig_leave, i))
			continue;
		
		if (sigismember(&sig_ign, i))
			sig.sa_handler = SIG_IGN;
		else
			sig.sa_handler = func;
		sig.sa_flags = 0;
		(void) sigemptyset(&sig.sa_mask);
		(void) sigaction(i, &sig, (struct sigaction *)NULL);
	}
}
