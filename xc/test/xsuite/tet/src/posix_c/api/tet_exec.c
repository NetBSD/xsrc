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

SCCS:   	@(#)tet_exec.c	1.9 03/09/92
NAME:		'C' API exec process function
PRODUCT:	TET (Test Environment Toolkit)
AUTHOR:		Geoff Clare, UniSoft Ltd.
DATE CREATED:	31 July 1990
SYNOPSIS:

	int tet_exec(const char *file, char *const argv[],
	 	     char *const envp[]);

DESCRIPTION:

	Tet_exec() is used to execute the specified file in the same
	way as the POSIX.1 execve() interface except that it passes
	extra information on the command line and in the environment for
	use by a main() provided with the API.  This then calls the
	user-supplied tet_main() routine with the argv[] which was
	passed to tet_exec().

	Tet_exec() does not return if successful, but may return -1 if
	either execve() or malloc() fails.

MODIFICATIONS:
	
	Geoff Clare, UniSoft Ltd., 6 Sept 1990
		Pass tet_activity on command line.

	Geoff Clare, UniSoft Ltd., 10 Oct 1990
		Remove const keywords.

************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <tet_api.h>

#define TET_CONFIG	"TET_CONFIG"
#define TET_RESFD	"TET_RESFD"
#define TET_TMPRESFD	"TET_TMPRESFD"

extern long	tet_activity;
extern long	tet_context;
extern long	tet_block;

static char	buf[4][25];

int
tet_exec(file, argv, envp)
char *file;
char *argv[];
char *envp[];
{
	char **newargv, **newenvp, *cp;
	int cnt, config_ok, resfd_ok, tmpfd_ok;

	/* allocate new argv array, with room for four extra args plus
	   NULL terminator */

	for (cnt = 0; argv[cnt] != NULL; cnt++)
		;
	cnt += 5;
	newargv = (char **) malloc((size_t)(cnt * sizeof(char *)));
	if (newargv == NULL)
		return -1;

	/* first 4 args are TP number, current activity, context and block */

	(void) sprintf(buf[0], "%d", tet_thistest);
	(void) sprintf(buf[1], "%ld", tet_activity);
	(void) sprintf(buf[2], "%ld", tet_context);
	(void) sprintf(buf[3], "%ld", tet_block);
	newargv[0] = buf[0];
	newargv[1] = buf[1];
	newargv[2] = buf[2];
	newargv[3] = buf[3];

	/* copy remaining args from argv[] and add NULL terminator */

	for (cnt = 4; *argv != NULL; cnt++, argv++)
		newargv[cnt] = *argv;
	newargv[cnt] = NULL;

	/* ensure TET_CONFIG, TET_RESFD and TET_TMPRESFD are in environment */

	config_ok = resfd_ok = tmpfd_ok = 0;
	for (cnt = 0; envp[cnt] != NULL; cnt++)
	{
		if (strncmp(envp[cnt], TET_CONFIG,
				(size_t)(sizeof(TET_CONFIG)-1)) == 0 &&
				envp[cnt][sizeof(TET_CONFIG)] == '=')
			config_ok++;
		else if (strncmp(envp[cnt], TET_RESFD,
				(size_t)(sizeof(TET_RESFD)-1)) == 0 &&
				envp[cnt][sizeof(TET_RESFD)] == '=')
			resfd_ok++;
		else if (strncmp(envp[cnt], TET_TMPRESFD,
				(size_t)(sizeof(TET_TMPRESFD)-1)) == 0 &&
				envp[cnt][sizeof(TET_TMPRESFD)] == '=')
			tmpfd_ok++;
	}

	if (config_ok && resfd_ok && tmpfd_ok)
		newenvp = (char **) envp;
	else
	{
		/* allocate new envp array and add missing variables */

		if (!config_ok)
			cnt++;
		if (!resfd_ok)
			cnt++;
		if (!tmpfd_ok)
			cnt++;
		cnt++;
		newenvp = (char **) malloc((size_t)(cnt * sizeof(char *)));
		if (newenvp == NULL)
			return -1;

		for (cnt = 0; *envp != NULL; cnt++, envp++)
			newenvp[cnt] = *envp;
		if (!config_ok && (cp = getenv(TET_CONFIG)) != NULL)
		{
			cp -= sizeof(TET_CONFIG);
			newenvp[cnt++] = cp;
		}
		if (!resfd_ok && (cp = getenv(TET_RESFD)) != NULL)
		{
			cp -= sizeof(TET_RESFD);
			newenvp[cnt++] = cp;
		}
		if (!tmpfd_ok && (cp = getenv(TET_TMPRESFD)) != NULL)
		{
			cp -= sizeof(TET_TMPRESFD);
			newenvp[cnt++] = cp;
		}
		newenvp[cnt] = NULL;
	}

	return execve(file, newargv, newenvp);
}
