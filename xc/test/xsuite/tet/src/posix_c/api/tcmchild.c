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
static	char sccsid[] = "@(#)tcmchild 1.9 03/09/92";
#endif

/************************************************************************

SCCS:   	@(#)tcmchild.c	1.9 03/09/92
NAME:		'C' API executed process main function
PRODUCT:	TET (Test Environment Toolkit)
AUTHOR:		Geoff Clare, UniSoft Ltd.
DATE CREATED:	31 July 1990
SYNOPSIS:

	extern int tet_main(int argc, char *argv[]);

	int main(int argc, char **argv);
	int tet_thistest;

DESCRIPTION:

	The main() routine provided here is for use in processes
	executed by tet_exec().  The user-supplied tet_main() function
	is called with the arguments passed to tet_exec().  Configuration
	variables are read in from the file specified in the environment
	by TET_CONFIG.  The variable tet_thistest is set to its value in
	the calling process.  The current context is preserved from the
	calling process, the block number is incremented and sequence
	number reset to 1.

MODIFICATIONS:

	Geoff Clare, UniSoft Ltd., 6 Sept 1990
		Pass tet_activity on command line.
		Add tet_cleanup definition.
		Move tet_error() to resfile.c.
	
	Geoff Clare, UniSoft Ltd., 10 Oct 1990
		Rename file and change sccsid.

************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <tet_api.h>

extern int	tet_main();
extern void	tet_config();
extern void	tet_error();
extern char *	tet_errname();

extern long	tet_activity;
extern long	tet_context;
extern long	tet_block;

char *	tet_pname;
int	tet_thistest;

/* This is to satisfy the reference in resfile.c (called on Abort rescode) */
void (*tet_cleanup)() = NULL;

int
main(argc, argv)
int argc;
char **argv;
{
	/* the first four arguments are TP number, current activity,
	   context and block */
	
	if (argc < 5)
	{
		tet_error(0, "argument count wrong: process must be executed by tet_exec()");
		exit(EXIT_FAILURE);
	}
	tet_thistest = atoi(argv[0]);
	tet_activity = atol(argv[1]);
	tet_context = atol(argv[2]);
	tet_block = atol(argv[3]);

	tet_setblock();

	tet_pname = argv[4]; /* used by tet_error() */

	/* read configuration variables */
	tet_config();

	/* the remaining arguments are passed to tet_main */

	exit(tet_main(argc-4, &argv[4]));

	/* NOTREACHED */
}
