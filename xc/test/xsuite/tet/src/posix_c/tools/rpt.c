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
 *
 */

#ifndef lint
static char sccsid[] = "@(#)RPT	1.9 03/09/92";
#endif

/************************************************************************

SCCS:          @(#)rpt.c	1.9 03/09/92
NAME:          rpt.c
PRODUCT:       TET (Test Environment Toolkit)
AUTHOR:        Carlos 
DATE CREATED:  14 May 1991
CONTENTS:

MODIFICATIONS:

               Made the code reasonably lint-able.
               David G. Sawyer, Unisoft Ltd, July 1991.

	       Added close() of pipe in parent so filter gets EOF.
	       Removed spurious COMPLAIN line and journal argument to filter.
               Geoff Clare, Unisoft Ltd, 14 Oct 1991.

************************************************************************/
#include <tcc_env.h>
#include <tcc_mac.h>
#include <tet_jrnl.h>
#include <limits.h>

#define MAXLEN 1024

/* Function Prototypes */
#if __STDC__

int	getline( char *, FILE *);
void 	cant_get_file(char *, char *);
void 	tet_shutdown();

#else

int 	getline();
void	cant_get_file();
void	tet_shutdown();

#endif


int main(argc, argv)
int argc;
char *argv[];
{

	char journal[MAXLEN], filter[MAXLEN];
	int fd[2];
	int child;
	void cant_get_file();
	FILE *fp;
  	char tmp[MAXLEN];
  	int getline();
  	int wstatus;
  	int exec_res;

  	if (argc != 3)
  	{
		(void) fprintf(stderr, " Usage: rpt <journal> <filter>\n\n");
		exit(1);
  	}

	(void) strcpy(journal, argv[1]);	/* copy 'journal' name */
	(void) strcpy(filter, argv[2]);	/* copy 'filter'  name */

	if (access(journal, R_OK))	/* check read perm for jrnl */
	{
		if (errno == EACCES)
			cant_get_file(journal, "Permission denied");
		else
			cant_get_file(journal, "Incorrect file name");
	}

	if (access(filter, X_OK))	/* check 'x' perm for filter */
	{
		if (errno == EACCES)
			cant_get_file(filter, "Permission denied");
		else
			cant_get_file(filter, "Incorrect file name");
	}

	BAIL_OUT_ON(pipe(fd),"pipe(fd)");

	if ((child = fork()) == 0)
	{
		BAIL_OUT_ON(dup2(fd[0], 0), "dup2(fd[0])");
		BAIL_OUT_ON(close(fd[0]), "close(fd[0])");
		BAIL_OUT_ON(close(fd[1]), "close(fd[1])");

		exec_res = execlp(filter, filter, (char *)0 );

		BAIL_OUT_ON(exec_res, "execlp of 'filter' failed");
	}

	BAIL_OUT_ON(close(fd[0]), "close(fd[0]) (parent)");

	fp = fopen(journal, "r");
	while (getline(tmp, fp) > 0)
		(void) write(fd[1], (void *) tmp, strlen(tmp));
	(void) fclose(fp);

	BAIL_OUT_ON(close(fd[1]), "close(fd[1]) (parent)");

	(void) waitpid((pid_t) child, &wstatus, 0);

	exit(SUCCESS);

	/* NOTREACHED */
}


int getline(s, fp)
char s[]; 
FILE *fp;
{
	int c, i;

	i = 0;
	while ((c = getc(fp)) != '\n' && c != EOF)
		s[i++] = c;

	if (c == '\n')
		s[i++] = c;

	s[i] = '\0';

	return(i);
}

void cant_get_file(str1, str2)
char *str1, *str2;
{
	(void) fprintf(stderr, "\nreport_gen: access error for file %s: %s\n\n", 		str1, str2);
	exit(2);

}

/*
 *	fatal error routine
 */
void tet_shutdown()
{
	/* if we're FATAL close down and exit */
	exit(-1);
}
