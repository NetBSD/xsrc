/*
 
Copyright (c) 1990, 1991  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.

 *
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: mc.h,v 1.7 94/04/17 21:00:23 rws Exp $
 *
 * Author: Steve Ratcliffe, UniSoft Ltd.
 */

#ifndef EXIT_FAILURE
#define EXIT_FAILURE 1
#endif

#define	MAXLINE	1024

/*
 * Defines for the directives that introduce each section.
 */
#define D_HEADER	">>TITLE"
#define D_ASSERTION	">>ASSERTION"
#define D_STRATEGY	">>STRATEGY"
#define D_CODE	">>CODE"
#define D_MAKE	">>MAKE"
#define D_EXTERN	">>EXTERN"
#define	D_INCLUDE	">>INCLUDE"
#define D_CFILE		">>CFILES"
#define D_COMMENT	">>#"
#define D_COMMENT_LEN	3
#define D_SET	">>SET"

#define SECSTART(buf)	((buf)[0] == '>' && (buf)[2] != '#')

#define XCALLSYM	"XCALL"
#define XNAMESYM	"xname"

#define	ASLENGTH	60

struct	settings	{
	char	*startup;
	char	*cleanup;
	char	*tpstartup;
	char	*tpcleanup;
	short 	needgcflush;
	char	*display;	/* Name of the display variable if there is one */
	short	failreturn;	/* return after a failure */
	char	*valreturn;	/* Symbol value that should be returned */
	char	*beginfunc;	/* Function to call at beginning of TP */
	char	*endfunc;	/* Function to call at end of TP */
	int 	macro;		/* Has a macro version */
	char	*macroname;	/* Name of the macro version */
	short	noerrcheck;	/* No error status check */
};

struct	state	{
	char	*name;		/* test name */
	char	*chap;		/* associated chapter or section */
	int 	assertion;	/* assertion number */
	char	*type;		/* type code */
	short	category;	/* category code */
	char	*reason;	/* reason code string for categories B, D */
	short	sectype;	/* Current section we are in */
	short	defaultreq;	/* Default strat/code is needed */
	short	discardtest;/* We are discarding this assertion */
	short	err;		/* State within a .ER assertion */
	short	skipsec;	/* Skip some sections */
	short	abortafter;	/* stop after this many sections */
	short	xproto;		/* This is an xproto test */
};

#define	ER_NONE	0	/* No error */
#define	ER_NORM	1	/* Normal error */
#define	ER_VALUE	2	/* A Value error */

/*
 * Commands.
 */
#define	CMD_MEXPAND	0
#define	CMD_MC	1
#define	CMD_MKMF 2
#define	CMD_MA	3
#define	CMD_MAS	4

/*
 * Section types.
 */
#define	SEC_COPYRIGHT	0
#define	SEC_HEADER	1
#define	SEC_ASSERTION	2
#define	SEC_DEFASSERT	3
#define	SEC_STRATEGY	4
#define	SEC_CODE	5
#define	SEC_EXTERNCODE	6
#define	SEC_FILE	7
#define	SEC_MAKE	8
#define	NSEC	9

/*
 * Hook types.  These allow commands to obtain information about
 * various things that happen.
 */
#define	HOOK_START	0
#define	HOOK_END	1
#define	HOOK_INCSTART	2
#define	HOOK_INCEND	3
#define	HOOK_SET	4
#define	HOOK_COMMENT	5
#define	NHOOK	6

struct	mclist {
	int 	num;	/* Number used */
	int 	size;	/* Number available */
	char	*items[1];/* strings */
	/* Really longer */
};

/*
 * Some temporary file names.
 */
#define	F_TVAL	"Mval.tmc"		/* Value assertion */
#define	F_TVCODE	"Mvcode.tmc"	/* BadValue code */
#define	F_TDEFCODE	"Mdefcode.tmc"	/* default code */

/*
 * Category codes
 */
#define	CAT_NONE	'\0'
#define	CAT_A	'A'
#define	CAT_B	'B'
#define	CAT_C	'C'
#define	CAT_D	'D'
#define	CAT_DEF	'-'

#define	SEPS	" \t\n"
#define	ARGSEP	" \t\n*[]();"

#define	NELEM(A)	(sizeof(A)/sizeof(A[0]))

#include	"mcproto.h"
