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

SCCS:   	@(#)llib-ltcm.c	1.9 03/09/92
NAME:		'C' API TCM lint library
PRODUCT:	TET (Test Environment Toolkit)
AUTHOR:		Geoff Clare, UniSoft Ltd.
DATE CREATED:	2 Aug 1990

DESCRIPTION:

	Lint library for use in linting applications which use the
	Test Case Manager (TCM) and API library.

MODIFICATIONS:
	
	Geoff Clare, UniSoft Ltd., 10 Oct 1990
		Remove const keywords.

	Geoff Clare, UniSoft Ltd., 10 Oct 1990
		Add tet_pname.

	Geoff Clare, UniSoft Ltd., 28 Nov 1990
		Add tet_nosigreset.

************************************************************************/

/* LINTLIBRARY */

#include "tet_api.h"

extern	void	(*tet_startup)();
extern	void	(*tet_cleanup)();
extern	struct tet_testlist tet_testlist[];

int main(argc, argv)
int argc;
char **argv;
{
	(*tet_startup)();
	(*tet_cleanup)();
	return tet_testlist[0].icref;
}

pid_t	tet_child;
void	tet_delete(test_no, reason) int test_no; char *reason; { }
int	tet_exec(f, a, e) char *f; char *a[], *e[]; { return 0; }
int	tet_fork(c, p, w, e) void (*c) (); void (*p) (); int w, e; { return 0; }
char *	tet_getvar(name) char *name; { return ""; }
void	tet_infoline(data) char *data; { }
int	tet_nosigreset;
char *	tet_pname;
char *	tet_reason(test_no) int test_no; { return ""; }
void	tet_result(result) int result; { }
void	tet_setblock() { }
void	tet_setcontext() { }
int	tet_thistest;
