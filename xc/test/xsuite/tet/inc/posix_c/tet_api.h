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

SCCS:   	@(#)tet_api.h	1.9 03/09/92
NAME:		'C' API header
PRODUCT:	TET (Test Environment Toolkit)
AUTHOR:		Geoff Clare, UniSoft Ltd.
DATE CREATED:	23 July 1990
CONTENTS:

	struct tet_testlist definition
	result code values for use with tet_result()
	TET_NULLFP null function pointer for use with tet_fork()
	declarations/prototypes for all API interfaces

MODIFICATIONS:

	Geoff Clare, UniSoft Ltd., 10 Oct 1990
		Remove const keywords.

	Geoff Clare, UniSoft Ltd., 18 Oct 1990
		Add tet_pname.

	Geoff Clare, UniSoft Ltd., 28 Nov 1990
		Add tet_nosigreset.

	Geoff Clare, UniSoft Ltd., 21 June 1991
		Make tet_fork() prototype consistent with definition.

************************************************************************/

#include <sys/types.h>	/* for pid_t */

/* argument values for tet_result() */

#define TET_PASS	0
#define TET_FAIL	1
#define TET_UNRESOLVED	2
#define TET_NOTINUSE	3
#define TET_UNSUPPORTED	4
#define TET_UNTESTED	5
#define TET_UNINITIATED	6
#define TET_NORESULT	7

#define TET_NULLFP	((void (*)())0)	/* for use as argument to tet_fork() */

#if (__STDC__ > 0)

struct tet_testlist {
	void (*testfunc)(void);
	int icref;
};

extern void	tet_delete(int, char *);
extern int	tet_exec(char *, char *[], char *[]);
extern int	tet_fork(void (*)(), void (*)(), int, int);
extern char *	tet_getvar(char *);
extern void	tet_infoline(char *);
extern char *	tet_reason(int);
extern void	tet_result(int);
extern void	tet_setblock(void);
extern void	tet_setcontext(void);

#else /* !(__STDC__ > 0) */

struct tet_testlist {
	void (*testfunc)();
	int icref;
};

extern void	tet_delete();
extern int	tet_exec();
extern int	tet_fork();
extern char *	tet_getvar();
extern void	tet_infoline();
extern char *	tet_reason();
extern void	tet_result();
extern void	tet_setblock();
extern void	tet_setcontext();

#endif /* !(__STDC__ > 0) */

extern int	tet_thistest;
extern pid_t	tet_child;
extern char	*tet_pname;
extern int	tet_nosigreset;
