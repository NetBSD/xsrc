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

SCCS:   	@(#)config.c	1.9 03/09/92
NAME:		'C' API configuration variable functions
PRODUCT:	TET (Test Environment Toolkit)
AUTHOR:		Geoff Clare, UniSoft Ltd.
DATE CREATED:	27 July 1990
SYNOPSIS:

	char *tet_getvar(char *name);

	void tet_config(void);
	int  tet_putenv(char *envstr);

DESCRIPTION:

	Tet_getvar() obtains the value of the named configuration
	variable.  It returns NULL if the variable has not been set.

	Tet_config() is not part of the API.  It is used by other
	API functions to read configuration variables from the file
	specified by the communication variable TET_CONFIG and makes
	them available to tet_getvar().  If the file cannot be opened
	or read an error message is produced and no variables will be
	set.  Tests using tet_getvar() are expected to report the
	NULL value returned in this case.

	Tet_putenv() is not part of the API: it is used by API
	functions to set environment variables.

MODIFICATIONS:
	
	Geoff Clare, UniSoft Ltd, 15 Jan 1992
		Fix memory leak if realloc() moved data.

************************************************************************/

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <tet_api.h>

extern char **	environ;
extern void	tet_error();

static char *	vardata = NULL;
static char **	varptrs = NULL;

char *
tet_getvar(name)
char *name;
{
	/* return value of specified configuration variable */

	char **cur;
	char *cp;
	size_t len;

	if (varptrs == NULL || vardata == NULL)
		return ((char *) NULL);

	/* varptrs is an array of strings of the form: "VAR=value" */
	len = strlen(name);
	for (cur = varptrs; *cur != NULL; cur++)
	{
		cp = *cur;
		if (strncmp(cp, name, len) == 0 && cp[len] == '=')
			return &cp[len+1];
	}

	return ((char *) NULL);
}

void
tet_config()
{
	/* read config variables from TET_CONFIG file */

	char *file, *cp, **varp;
	int fd, err, cnt;
	size_t nbyte;
	struct stat statb;
	char buf[256];

	/* open and read in entire config file */

	file = getenv("TET_CONFIG");
	if (file == NULL || *file == '\0')
		return;

	if ((fd = open(file, O_RDONLY)) < 0)
	{
		err = errno;
		(void) sprintf(buf, "could not open config file \"%s\"",
			file);
		tet_error(err, buf);
		return;
	}

	if (fstat(fd, &statb) == -1)
	{
		err = errno;
		(void) sprintf(buf, "fstat() failed on config file \"%s\"",
			file);
		tet_error(err, buf);
		(void) close(fd);
		return;
	}

	if ((nbyte = statb.st_size) == 0)
	{
		(void) close(fd);
		return;
	}

	/* In case this routine has been called before, free the old data */
	if (vardata != NULL)
		free((void *)vardata);

	vardata = (char *)malloc((size_t)(nbyte + 1));
	if (vardata == NULL)
	{
		tet_error(0, "malloc() failed");
		(void) close(fd);
		return;
	}

	if (read(fd, (void *)vardata, nbyte) != nbyte)
	{
		err = errno;
		(void) sprintf(buf, "read() failed on config file \"%s\"",
			file);
		tet_error(err, buf);
		(void) close(fd);
		return;
	}
	vardata[nbyte] = '\0'; /* in case of incomplete last line */

	(void) close(fd);

	/* count number of variables and replace newlines with nulls */

	cnt = 0;
	cp = &vardata[nbyte-1];
	if (*cp == '\n')
		*cp = '\0';
	while (--cp > vardata)
	{
		if (*cp == '\n')
		{
			/* don't count comment lines and blank lines */
			if (cp[1] != '#' && cp[1] != '\0')
				cnt++;
			*cp = '\0';
		}
	}
	if (*cp == '\n')
		*cp = '\0';
	else if (*cp != '#')
		cnt++;
	cnt++; /* allow for null terminator */

	/* set up pointers to each "variable=value" string */

	if (varptrs != NULL)
		free((void *)varptrs);

	varptrs = (char **)malloc((size_t) (cnt * sizeof(char *)));
	if (varptrs == NULL)
	{
		tet_error(0, "malloc() failed");
		return;
	}

	varp = varptrs;
	while (cnt > 1)
	{
		while (*cp == '\0')
			cp++;
		if (*cp != '#')
		{
			*varp++ = cp;
			cnt--;
		}
		while (*cp != '\0')
			cp++;
	}

	/* add terminating NULL pointer */

	*varp = NULL;
}

int
tet_putenv(envstr)
char *envstr;
{
	/*
	 * This routine mimics putenv(), and is provided purely
	 * because putenv() is not in POSIX.1
	 */

	char **newenv, **cur, *envname;
	int n, count = 0;
	static char **allocp = NULL;

	if (environ == NULL)
	{
		newenv = (char **)malloc((size_t)(2*sizeof(char *)));
		if (newenv == NULL) 
			return -1;

		newenv[0] = envstr;
		newenv[1] = NULL;
		environ = newenv;
		allocp = newenv;
		return 0;
	}

	cur = environ;

	while (*cur != NULL)
	{
		count++;
		envname = *cur;
		n = strcspn(envstr, "=");
		if (strncmp(envname, envstr, (size_t) n) || envname[n] != '=')
			cur++;
		else
		{
			*cur = envstr;
			return 0;
		}
	}
	
	/*
	 * If we previously allocated this environment enlarge it using
	 * realloc(), otherwise allocate a new one and copy it over.
	 * Note that only the last malloc()/realloc() pointer is saved, so
	 * if environ has since been changed the old space will be wasted.
	 */

	if (environ == allocp)
		newenv = (char **) realloc((void *) environ, 
				(size_t) ((count+2)*sizeof(char *)));
	else
		newenv = (char **) malloc((size_t) ((count+2)*sizeof(char *)));

	if (newenv == NULL) 
		return -1;
	
	if (environ != allocp)
	{
		for (n = 0; environ[n] != NULL; n++)
			newenv[n] = environ[n];
	}
	newenv[count] = envstr;
	newenv[count+1] = NULL;
	environ = newenv;
	allocp = newenv;

	return 0;
}
