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

SCCS:   	@(#)resfile.c	1.9 03/09/92
NAME:		'C' API results file functions
PRODUCT:	TET (Test Environment Toolkit)
AUTHOR:		Geoff Clare, UniSoft Ltd.
DATE CREATED:	25 July 1990
SYNOPSIS:

	void tet_infoline(char *data);
	void tet_result(int result);
	void tet_setcontext(void);
	void tet_setblock(void);

	void tet_openres(void);
	void tet_tcmstart(char *version, int no_ics);
	void tet_icstart(int ic_num, int no_tests);
	void tet_icend(int ic_num, int no_tests);
	void tet_tpstart(int tp_num);
	void tet_tpend(int tp_num);
	void tet_error(int errno_val, char *msg);
	long tet_context;
	long tet_block;

DESCRIPTION:

	Tet_infoline() outputs the specified line of data to the
	execution results file, prefixed by the current context, block
	and sequence numbers.  The data should not contain any newline
	characters.

	Tet_result() specifies the result code which is to be entered
	in the execution results file for the current test purpose.
	It stores the result in a temporary file which is later read
	by tet_tpend().

	Tet_setcontext() sets the current context to the current
	process ID and resets the block and sequence numbers to 1.

	Tet_setblock() increments the current block number and
	resets the sequence number to 1.

	Tet_openres(), tet_tcmstart(), tet_icstart(), tet_icend(),
	tet_tpstart() and tet_tpend() are not part of the API: they are
	used by the TCM to open the execution results file and to output
	TCM start lines, IC start and end lines and test purpose start and
	result lines to it.

	Tet_error() is not part of the API.  It is used by API functions
	to report errors to stderr and the results file.

	Tet_context and tet_block are not part of the API: they are used
	by API functions to access the current context and block numbers.

MODIFICATIONS:
	
	Geoff Clare, UniSoft Ltd, 8 Aug 1990
		Add support for result codes file.
	
	Geoff Clare, UniSoft Ltd, 6 Sept 1990
		New results file format, using tet_jrnl.h.
		No longer need to check tet_context in tet_result().
		Add tet_error().

	Geoff Clare, UniSoft Ltd, 28 Sept 1990
		Remove spaces around IBCs in output.
		Skip blank lines in tet_code file.

	Geoff Clare, UniSoft Ltd, 1 Oct 1990
		Change invalid result code handling.
		Fix problem with blank line skipping in tet_code and add
			skipping of comment lines.
		Ensure tet_infoline() outputs a single line by translating
			any newlines in the input data to tabs.

	Geoff Clare, UniSoft Ltd, 16 Oct 1991
		Set up call to (*tet_cleanup)() same way as in tcm.c.

************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <tet_api.h>
#include <tet_jrnl.h>

#define IBC	'|'	/* inter-block character */

extern void	(*tet_cleanup)();
extern int	tet_putenv();
extern char *	tet_errname();
extern int	tet_thistest;

static char *	resfile = "tet_xres";
static char *	rescode_dflt = "tet_code";
static long	sequence = 0;
static int	resfd = -1;
static int	tmpfd = -1;
static char	buf[512];
static char	outbuf[512];

void	tet_error();
long	tet_activity = -1;
long	tet_context = 0;
long	tet_block = 0;

struct rescode_tab {
	char *name;
	int code;
	int abrt;
} restab_dflt[] = {
	/* default names for TET defined codes */
	"PASS",		TET_PASS,	0,
	"FAIL",		TET_FAIL,	0,
	"UNRESOLVED",	TET_UNRESOLVED,	0,
	"NOTINUSE",	TET_NOTINUSE,	0,
	"UNSUPPORTED",	TET_UNSUPPORTED,0,
	"UNTESTED",	TET_UNTESTED,	0,
	"UNINITIATED",	TET_UNINITIATED,0,
	"NORESULT",	TET_NORESULT,	0,
	NULL,		-1,		0
};

void
tet_openres()
{
	/* Create the execution results file and open in append mode */

	int err;
	mode_t resmode = S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH;
	static char envstr[40];

	(void) unlink(resfile);
	resfd = open(resfile, O_WRONLY|O_CREAT|O_TRUNC|O_APPEND, resmode);
	if (resfd < 0)
	{
		err = errno;
		(void) sprintf(buf, "cannot open results file \"%s\"", resfile);
		tet_error(err, buf);
		exit(EXIT_FAILURE);
	}

	/* put fd in environment to be picked up in exec'ed programs */
	(void) sprintf(envstr, "TET_RESFD=%d", resfd);
	if (tet_putenv(envstr) != 0)
	{
		tet_error(0, "tet_putenv() failed when setting TET_RESFD");
		exit(EXIT_FAILURE);
	}
}

static void
output(mtype, fields, data)
int mtype;
char *fields;
char *data;
{
	/* all execution results file output comes through here */

	char *cp;
	int i, j, len, lenp, err;

	if (resfd < 0)
	{
		/* assume this is an exec'ed program - pick up result
		   fd from environment */
		
		cp = getenv("TET_RESFD");
		if (cp == NULL || *cp == '\0')
		{
			tet_error(0, "TET_RESFD not set in environment");
			exit(EXIT_FAILURE);
		}

		resfd = atoi(cp);
	}

	if (tet_activity < 0)
	{
		/* set tet_activity from environment */
		cp = getenv("TET_ACTIVITY");
		if (cp == NULL || *cp == '\0')
			tet_activity = 0;
		else
			tet_activity = atol(cp);
	}

	if (data == NULL)
		data = "";

	(void) sprintf(outbuf, "%d%c%ld%s%s%c", mtype, IBC,
		tet_activity, (fields[0] == '\0' ? "" : " "), fields, IBC);

	lenp = strlen(outbuf);
	len = lenp + strlen(data) + 1;
	if (len > sizeof(outbuf))
	{
		len = sizeof(outbuf);
		tet_error(0, "warning: results file line truncated - prefix:");
		tet_error(0, outbuf);
	}
	for (i = lenp, j = 0; i < len-1; i++, j++)
	{
		/* output must not contain newlines: translate to tabs */
		if ((outbuf[i] = data[j]) == '\n')
			outbuf[i] = '\t';
	}
	outbuf[len-1] = '\n';
	if (write(resfd, (void *)outbuf, (size_t)len) != len)
	{
		err = errno;
		(void) sprintf(buf,
			"error writing to results file \"%s\"", resfile);
		tet_error(err, buf);
		exit(EXIT_FAILURE);
	}
}

static int
skipline(linebuf)
char *linebuf;
{
	/* return true if input line is to be skipped (comment or blank line) */

	char *cp;

	for (cp = linebuf; *cp == ' ' || *cp == '\t'; cp++)
		;
	return (*cp == '#' || *cp == '\n' || *cp == '\0');
}

static int
read_codes(tabp)
struct rescode_tab **tabp;
{
	/* get result codes from tet_code file */

	FILE *fp;
	struct rescode_tab *restab;
	char *fname, *cp, *endp;
	int err, line, nres, ires;

	/* file name is specified by TET_CODE communication variable */
	fname = getenv("TET_CODE");
	if (fname == NULL || *fname == '\0')
		fname = rescode_dflt;

	if ((fp = fopen(fname, "r")) == NULL)
	{
		/* don't complain if using default file */
		if (fname == rescode_dflt)
			return -1;

		err = errno;
		(void) sprintf(buf, "could not open result code file \"%s\"",
			fname);
		tet_error(err, buf);
		return -1;
	}

	/* count the number of result codes in the file */
	nres = 0;
	while (fgets(buf, sizeof(buf), fp) != NULL)
	{
		if (!skipline(buf))
			nres++;
	}
	
	/* allocate space for structures */
	*tabp = (struct rescode_tab *)malloc(
			(size_t)((nres + 1) * sizeof(**tabp)));
	if (*tabp == NULL)
	{
		tet_error(0, "malloc() failed in read_codes()");
		(void) fclose(fp);
		return -1;
	}
	restab = *tabp;

	/* now read each line and place the info in the structure */
	rewind(fp);
	ires = -1;
	for (line = 1; ires < nres && fgets(buf, sizeof(buf), fp) != NULL;
		line++)
	{
		if (skipline(buf))
			continue;

		ires++;

		/* first field is code number */
		restab[ires].code = atoi(buf);

		/* put temporary values in other fields */
		restab[ires].name = "INVALID RESULT";
		restab[ires].abrt = 0;

		/* second field is quote-delimited name */
		cp = strchr(buf, '"');
		if (cp != NULL)
		{
			cp++;
			endp = strchr(cp, '"');
		}
		if (cp == NULL || endp == NULL)
		{
			(void) sprintf(buf,
				"quotes missing on line %d of \"%s\"",
				line, fname);
			tet_error(0, buf);
			continue;
		}
		restab[ires].name = (char *)malloc((size_t)(endp-cp+1));
		if (restab[ires].name == NULL)
		{
			tet_error(0, "malloc() failed in read_codes()");
			break;
		}
		(void) strncpy(restab[ires].name, cp, (size_t)(endp-cp));
		restab[ires].name[endp-cp] = '\0';

		/* last field is action: Continue (default) or Abort */
		endp++;
		while (*endp == ' ' || *endp == '\t')
			endp++;
		if (*endp == '\n' || strcmp(endp, "Continue\n") == 0)
			restab[ires].abrt = 0;
		else if (strcmp(endp, "Abort\n") == 0)
			restab[ires].abrt = 1;
		else
		{
			(void) sprintf(buf,
				"invalid action field on line %d of \"%s\"",
				line, fname);
			tet_error(0, buf);
		}
	}

	/* add null terminator */
	restab[++ires].name = NULL;

	(void) fclose(fp);
	return 0;
}

static char *
get_code(result, abortflag)
int result;
int *abortflag;
{
	/* look up result code in rescodes structure, return name if
	   found, otherwise NULL.  If abortflag is not NULL then set
	   (*abortflag) to true if corresponding action is to abort */

	char *res;
	int i;
	static struct rescode_tab *restab = NULL;

	if (restab == NULL)
	{
		if (read_codes(&restab) != 0)
			restab = restab_dflt;
	}

	res = NULL;
	for (i = 0; restab[i].name != NULL; i++)
	{
		if (restab[i].code == result)
		{
			if (abortflag != NULL)
				*abortflag = restab[i].abrt;
			res = restab[i].name;
			break;
		}
	}

	/* if not found in supplied file, use default name */

	if (res == NULL && restab != restab_dflt)
	{
		for (i = 0; restab_dflt[i].name != NULL; i++)
		{
			if (restab_dflt[i].code == result)
			{
				if (abortflag != NULL)
					*abortflag = restab_dflt[i].abrt;
				res = restab_dflt[i].name;
				break;
			}
		}
	}

	return res;
}

static char *
curtime()
{
	/* return string containing current time */

	time_t t;
	struct tm *tp;
	static char s[10];

	(void) time(&t);
	tp = localtime(&t);
	(void) sprintf(s, "%02d:%02d:%02d", tp->tm_hour,
		tp->tm_min, tp->tm_sec);

	return s;
}

void
tet_infoline(data)
char *data;
{
	/* output an information line to the results file */

	if (tet_context == 0)
		tet_setcontext();

	if (data == NULL)
		data = "(null pointer)";

	(void) sprintf(buf, "%ld %ld %ld %ld", tet_thistest, tet_context,
		tet_block, sequence++);
	
	output(TET_JNL_TC_INFO, buf, data);
}

void
tet_result(result)
int result;
{
	/*
	 * Look up supplied code in results code file to check it's valid.
	 * Write the result code to a temporary result file to be picked
	 * up later by tet_tpend().  This mechanism is used rather than
	 * writing directly to the execution results file to ensure that only
	 * one result code appears there.
	 */
	
	char *resname;

	resname = get_code(result, (int *)NULL);
	if (resname == NULL)
	{
		(void) sprintf(buf,
			"INVALID result code %d passed to tet_result()",
			result);
		tet_error(0, buf);
		result = TET_NORESULT;
	}

	if (tmpfd < 0)
	{
		/* assume this is an exec'ed program - pick up temp result
		   fd from environment */
		
		char *cp = getenv("TET_TMPRESFD");
		if (cp == NULL || *cp == '\0')
		{
			tet_error(0, "TET_TMPRESFD not set in environment");
			exit(EXIT_FAILURE);
		}

		tmpfd = atoi(cp);
	}

	if (write(tmpfd, (void *)&result, sizeof(result)) != sizeof(result))
	{
		tet_error(errno, "write() failed on temporary result file");
		exit(EXIT_FAILURE);
	}
}

void
tet_tcmstart(versn, no_ics)
char *versn;
int no_ics;
{
	/* output "TCM Start" line to execution results file */

	(void) sprintf(buf, "%s %d", versn, no_ics);
	output(TET_JNL_TCM_START, buf, "TCM Start");
}

void
tet_icstart(ic_num, no_tests)
int ic_num;
int no_tests;
{
	/* output "IC Start" line to execution results file */

	(void) sprintf(buf, "%d %d %s", ic_num, no_tests, curtime());
	output(TET_JNL_IC_START, buf, "IC Start");
}

void
tet_icend(ic_num, no_tests)
int ic_num;
int no_tests;
{
	/* output "IC End" line to execution results file */

	(void) sprintf(buf, "%d %d %s", ic_num, no_tests, curtime());
	output(TET_JNL_IC_END, buf, "IC End");
}

void
tet_tpstart(tp_num)
int tp_num;
{
	/*
	 * This routine sets the current block to 1 and outputs a "TP Start"
	 * line to the execution results file.  It also opens the temporary
	 * result file for use by tet_result() and tet_tpend().
	 */

	static char *tmpres = ".tmpres";
	static char envstr[40];

	tet_block = 1;
	sequence = 1;

	(void) sprintf(buf, "%d %s", tp_num, curtime());
	output(TET_JNL_TP_START, buf, "TP Start");

	/* open temporary result file */
	tmpfd = open(tmpres, O_CREAT|O_TRUNC|O_APPEND|O_RDWR, S_IRWXU);
	if (tmpfd < 0)
	{
		tet_error(errno, "could not create temporary result file");
		return;
	}
	(void) unlink(tmpres);

	/* put fd in environment to be picked up by tet_result() in
	   exec'ed programs */
	(void) sprintf(envstr, "TET_TMPRESFD=%d", tmpfd);
	if (tet_putenv(envstr) != 0)
		tet_error(0, "tet_putenv() failed setting TET_TMPRESFD");
}

void
tet_tpend(tp_num)
int tp_num;
{
	/*
	 * output a "TP Result" line to the execution results file,
	 * based on the result code(s) written to a temporary result
	 * file by tet_result()
	 */

	char *res;
	int have_result, result, nextres;
	int abrt = 0;

	/* rewind temporary result file */
	if (lseek(tmpfd, (off_t)0, SEEK_SET) != 0)
	{
		tet_error(errno, "failed to rewind temporary result file");

		/* fall through: no results will be read */
	}

	/* read result code(s) from temporary file - if more than one
	   has been written, choose the one with the highest priority */

	have_result = 0;
	while (read(tmpfd, (void *)&nextres, sizeof(nextres)) ==
							sizeof(nextres))
	{
		/* if it's the first result, take it (for now) */
		if (!have_result)
		{
			result = nextres;
			have_result = 1;
			continue;
		}

		/* decide if this result supercedes any previous ones */
		switch (nextres)
		{
		case TET_PASS :
			/* lowest priority */
			break;

		case TET_FAIL :
			/* highest priority */
			result = nextres;
			break;

		case TET_UNRESOLVED :
		case TET_UNINITIATED :
			/* high priority */
			if (result != TET_FAIL)
				result = nextres;
			break;

		case TET_NORESULT :
			/* output by tet_result() for invalid result codes,
			   and so must supercede everything that isn't some
			   sort of definite failure */
			if (result != TET_FAIL && result != TET_UNRESOLVED &&
			    result != TET_UNINITIATED)
				result = nextres;
			break;

		case TET_UNSUPPORTED :
		case TET_NOTINUSE :	
		case TET_UNTESTED :
			/* low priority */
			if (result == TET_PASS)
				result = nextres;
			break;

		default :
			/* user-supplied codes: middle priority */
			if (result == TET_PASS || result == TET_UNSUPPORTED ||
			    result == TET_NOTINUSE || result == TET_UNTESTED)
				result = nextres;
		}
	}

	if (!have_result)
	{
		result = TET_NORESULT;
		res = "NORESULT";
	}
	else
	{
		res = get_code(result, &abrt);
		if (res == NULL)
		{
			/* This should never happen, as the codes have
			   already been validated by tet_result().  It is
			   not a serious problem - the name is only there
			   to make the results file more readable */
			res = "(NO RESULT NAME)";
		}
	}

	(void) sprintf(buf, "%d %d %s", tp_num, result, curtime());
	output(TET_JNL_TP_RESULT, buf, res);

	/* Abort is done here rather than in tet_result() since the
	   latter may have been called in a child.  Test purposes
	   should not assume that tet_result() will not return when
	   called with an abort code. */
	if (abrt)
	{
		(void) sprintf(buf, "ABORT on result code %d \"%s\"",
			result, res);
		output(TET_JNL_TCM_INFO, "", buf);

		if (tet_cleanup != NULL)
		{
			tet_thistest = 0;
			tet_block = 0;
			tet_setblock();
			(*tet_cleanup)();
		}
		exit(EXIT_FAILURE);
	}

	(void) close(tmpfd);
	(void) tet_putenv("TET_TMPRESFD=");
}

void
tet_setcontext()
{
	/* Set current context to process ID and reset block & sequence */

	pid_t pid = getpid();

	if (tet_context != pid)
	{
		tet_context = pid;
		tet_block = 1;
		sequence = 1;
	}
}

void
tet_setblock()
{
	/* Increment current block & reset sequence number within block */

	tet_block++;
	sequence = 1;
}

void
tet_error(errno_val, msg)
int errno_val;
char *msg;
{
	/* print error message on stderr and in results file */

	size_t len;
	extern char *tet_pname;

	(void) fprintf(stderr, "%s: %s\n", tet_pname, msg);
	if (errno_val > 0)
		(void) fprintf(stderr, "%s: errno = %d (%s)\n", tet_pname,
			errno_val, tet_errname(errno_val));

	/* don't use output() to write results file - it calls tet_error() */

	if (resfd >= 0)
	{
		if (errno_val > 0)
			(void) sprintf(outbuf,
				"%d%c%ld%c%s: errno = %d (%s)\n",
				TET_JNL_TCM_INFO, IBC, tet_activity, IBC, msg,
				errno_val, tet_errname(errno_val));
		else
			(void) sprintf(outbuf, "%d%c%ld%c%s\n",
				TET_JNL_TCM_INFO, IBC, tet_activity, IBC, msg);

		len = strlen(outbuf);
		if (write(resfd, (void *)outbuf, len) != len)
		{
			errno_val = errno;
			(void) fprintf(stderr,
				"%s: error writing to results file \"%s\"\n",
				tet_pname, resfile);
			(void) fprintf(stderr, "%s: errno = %d (%s)\n",
				tet_pname, errno_val, tet_errname(errno_val));
			exit(EXIT_FAILURE);
		}
	}
}
