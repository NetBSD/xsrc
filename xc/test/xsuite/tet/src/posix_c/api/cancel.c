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

SCCS:   	@(#)cancel.c	1.9 03/09/92
NAME:		'C' API test purpose cancel functions
PRODUCT:	TET (Test Environment Toolkit)
AUTHOR:		Geoff Clare, UniSoft Ltd.
DATE CREATED:	26 July 1990
SYNOPSIS:

	void tet_delete(int test_no, char *reason);
	char *tet_reason(int test_no);
	void tet_delreas(int numtests);

DESCRIPTION:

	Tet_delete() marks the test purpose given by test_no as
	cancelled with the specified reason.  This causes the TCM
	to output an UNINITIATED result for that test purpose, and
	the corresponding function in tet_testlist[] is not called.
	The reason argument must point to static data as tet_delete()
	does not copy it internally, and the function calling
	tet_delete() will have returned before the reason string
	is examined by the TCM.

	If test_no has been cancelled tet_reason() returns the reason
	for cancellation, otherwise it returns NULL.

	Note that test purposes are numbered in the sequence of the
	tet_testlist[] array (starting at 1) regardless of execution
	sequence.

	Tet_delreas() is not part of the API.  It is called by the
	TCM to set up data for use by tet_delete() and tet_reason().

MODIFICATIONS:

	Geoff Clare, UniSoft Ltd, 6 Sept 1991
		Add tet_delreas() function.
		Change fixed MAX_TESTS to dynamic numtests.

		#### MIT X Test suite patch ####

	Stuart Boutell, UniSoft Ltd, 13 April 1992
		Added NULL declaration, because SunOS stdlib.h wrongly doesn't 
	define it.

************************************************************************/

#include <stdlib.h>
#include <tet_api.h>

/* SunOS stdlib.h workaround */
#ifndef  NULL
#define  NULL (0)
#endif

extern void	tet_error();

static int	numtests;
static char	**delreas;

void
tet_delete(test_no, reason)
int	test_no;
char	*reason;
{
	/* mark test as cancelled with specified reason */

	if (test_no > 0 && test_no <= numtests)
		delreas[test_no - 1] = reason;
}

char *
tet_reason(test_no)
int	test_no;
{
	/* If test has been cancelled return reason, else return NULL */

	if (test_no > 0 && test_no <= numtests)
		return delreas[test_no - 1];

	return NULL;
}

void
tet_delreas(ntests)
int ntests;
{
	/* Allocate space for pointers to hold deletion reasons */

	delreas = (char **)malloc((size_t)(ntests * sizeof(char *)));
	if (delreas == NULL)
	{
		tet_error(0, "malloc() failed in tet_delreas()");
		exit(EXIT_FAILURE);
	}
	else
	{
		numtests = ntests;
		for (ntests = 0; ntests < numtests; ntests++)
			delreas[ntests] = NULL;
	}
}
