
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
 * $XConsortium: sections.c,v 1.14 94/04/17 21:00:24 rws Exp $
 *
 * Author: Steve Ratcliffe, UniSoft Ltd.
 */

#include	<stdio.h>
#include	<ctype.h>
#include	"string.h"

#include	"mc.h"

extern	int 	Cmdname;
extern	struct	state	State;
extern	struct	settings Settings;

extern	int 	mflag;

/*
 * What needs to be defaulted.  Note that it is sort of assumed that
 * code follows strategy even though this is not tested at present.
 */
#define	NEED_STRAT	0x1
#define	NEED_CODE	0x2

#if 0
/* Template for sections */
	,		/* copyright */
	,		/* header */
	,		/* assertion */
	,		/* defassertion */
	,		/* strategy */
	,		/* code */
	,		/* extern code */
	,		/* extra files */
	,		/* make files */
#endif
#if 0
/* Template for hooks */
	,		/* start */
	,		/* end */
	,		/* include start hook */
	,		/* include end hook */
	,		/* set hook */
	,		/* comment hook */
#endif

void	(*secsw[][NSEC])() = {
	{	/* mexpand */
	mepcopyright,		/* copyright */
	mepecho,		/* header */
	mepecho,		/* assertion */
	mepecho,		/* defassertion */
	mepecho,		/* strategy */
	mepecho,		/* code */
	mepecho,		/* extern code */
	mepecho,		/* make lines */
	mepecho,		/* make files */
	},
	{	/* mc */
	mccopyright,	/* copyright */
	mcheader,		/* header */
	mcassertion,		/* assertion */
	mcdefassertion,		/* defassertion */
	mcstrategy,		/* strategy */
	mccode,			/* code */
	mcexterncode,		/* extern code */
	skip,			/* extra files */
	skip,			/* make files */
	},
	{	/* mmkf */
	mmcopyright,		/* copyright */
	mmheader,		/* header */
	skip,		/* assertion */
	skip,		/* defassertion */
	skip,		/* strategy */
	skip,		/* code */
	skip,		/* extern code */
	mmcfiles,		/* extra files */
	mmmake,		/* make section */
	},
	{	/* ma */
	macopyright,		/* copyright */
	maheader,		/* header */
	maassertion,		/* assertion */
	madefassertion,		/* defassertion */
	skip,		/* strategy */
	skip,		/* code */
	skip,		/* extern code */
	skip,		/* extra files */
	skip,		/* make files */
	},
	{	/* mas */
	mascopyright,		/* copyright */
	masheader,		/* header */
	masassertion,		/* assertion */
	masassertion,		/* defassertion */
	masstrategy,		/* strategy */
	skip,		/* code */
	skip,		/* extern code */
	skip,		/* extra files */
	skip,		/* make files */
	},
};

void	(*hooksw[][NHOOK])() = {
	{
	mepstart,	/* start */
	mepend,		/* end */
	0,		/* include start hook */
	0,		/* include end hook */
	mepset,		/* set hook */
	mepcomment,	/* comment hook */
	},
	{
	mcstart,		/* start */
	mcend,			/* end */
	mcincstart,		/* include start hook */
	mcincend,		/* include end hook */
	0,			/* set hook */
	0,			/* comment hook */
	},
	{
	mmstart,		/* start */
	mmend,		/* end */
	mmincstart,		/* include start hook */
	0,		/* include end hook */
	0,		/* set hook */
	0,		/* comment hook */
	},
	{
	mastart,		/* start */
	maend,		/* end */
	0,		/* include start hook */
	0,		/* include end hook */
	0,		/* set hook */
	macomment,		/* comment hook */
	},
	{
	masstart,		/* start */
	masend,		/* end */
	0,		/* include start hook */
	0,		/* include end hook */
	0,		/* set hook */
	0,		/* comment hook */
	},
};

#define	NCMDS	(sizeof(secsw)/(NSEC*sizeof(void (*)())))

struct	secname {
	char	*name;
	int 	sec;
} secname[] = {
	{D_HEADER, SEC_HEADER},
	{D_ASSERTION, SEC_ASSERTION},
	{D_STRATEGY, SEC_STRATEGY},
	{D_CODE, SEC_CODE},
	{D_EXTERN, SEC_EXTERNCODE},
	{D_CFILE, SEC_FILE},
	{D_MAKE, SEC_MAKE},
};

/*
 * Loop through all sections and branch out to the appropriate strategy
 * routines.
 */
dosections(fp, buf)
FILE	*fp;
char	*buf;
{
struct	secname	*sp;
int 	sec;

	if (State.skipsec == 0)
		do1sec(fp, buf, SEC_COPYRIGHT);

	while (State.skipsec > 0) {
		State.skipsec--;
		skip(fp, buf);
	}

	while (!feof(fp)) {

		sec = -1;
		for (sp = secname; sp < secname+NELEM(secname); sp++) {
			if (strncmp(buf, sp->name, strlen(sp->name)) == 0) {
				sec = sp->sec;
				break;
			}
		}

		if (sec == -1) {
			if (strncmp(buf, D_INCLUDE, strlen(D_INCLUDE)) == 0) {
				includefile(buf+strlen(D_INCLUDE), buf);
				newline(fp, buf);
				continue;
			} else {
				err("Bad directive\n");
				errexit();
			}
		}

		/* XXX Temp because of the push-back line problem */
		State.sectype = sec;

		if (sec == SEC_ASSERTION) {
			assertion(fp, buf);
		} else {
			do1sec(fp, buf, sec);
		}
		if (State.abortafter > 0) {
			if (--State.abortafter == 0)
				break;
		}
	}
}

/*
 * Do an assertion.  There are three types of assertion regular, def
 * and gc.  The gc type are sorted out by gccomps the others
 * branch to their own strategies.
 */
assertion(fp, buf)
FILE	*fp;
char	*buf;
{
char	*line;
char	*str;
char	*type;
int 	reason;
static char	*reasons[] = {
	"Temporarily can't be implemented",
	"There is no known portable test method for this assertion",
	"The statement in the X11 specification is not specific enough to write a test",
	"There is no known reliable test method for this assertion",
	"Testing the assertion would require setup procedures that involve an unreasonable amount of effort by the user of the test suite.",
	"Testing the assertion would require an unreasonable amount of time or resources on most systems",
	"Creating a test would require an unreasonable amount of test development time.",
};

	/* Check for default error sections */
	dodefaults(buf);
	State.assertion++;

	line = mcstrdup(buf);	/* Must not be freed */
	str = strtok(line, SEPS);
	type = strtok((char*)0, SEPS);
	/*
	 * When in an included error file then the type cannot be allowed
	 * to overrule the type that the programmer has selected. This is
	 * because it affects things like Status returns.
	 */
	if (State.err == ER_NONE)
		State.type = type;

	if (type == NULL)
		type = "NOT-SET"; 

	if (strcmp(type, "def") == 0) {
		State.category = CAT_DEF;
		do1sec(fp, buf, SEC_DEFASSERT);
	} else if (strcmp(type, "gc") == 0) {
		/*
		 * Since this is not a real assertion then undo the
		 * state changes above.
		 */
		State.assertion--;
		gccomps(fp, buf);
	} else {
		str = strtok((char*)0, SEPS);
		if (str == NULL)
			str = "";
		State.category = *str;
		if (State.category == CAT_B || State.category == CAT_D) {
			State.reason = NULL;
			str = strtok((char*)0, SEPS);
			if (str && isdigit(*str)) {
				reason = atoi(str);
				if (reason < NELEM(reasons))
					State.reason = reasons[reason];
			}
		}

		/* XXX Temp. to get expansion */
		State.sectype = SEC_ASSERTION;
		/*
		 * Look ahead to see if there is an error.  This is probably
		 * slightly better than the way it used to be done.
		 */
		line = mcstrdup(buf);
		if (newline(fp, buf) && strncmp(buf, ".ER", 3) == 0) {
			/* Back out of the assertion so far */
			State.assertion--;
			errtext(buf);
			newline(fp, buf);
		} else {
			putbackline(buf);
			(void) strcpy(buf, line);
			State.defaultreq = NEED_STRAT|NEED_CODE;
			do1sec(fp, buf, SEC_ASSERTION);
		}
		free(line);
	}
}

/*
 * Do one section.
 */
do1sec(fp, buf, sec)
FILE	*fp;
char	*buf;
int 	sec;
{
char	*line;
char	*str;
int	i;

	State.sectype = sec;

	if (sec >= NSEC) {
		(void) fprintf(stderr, "Internal error: invalid command\n");
		errexit();
	}

	/*
	 * Any special action that needs to be done at the global level
	 * for a section.
	 */
	switch (sec) {
	case SEC_STRATEGY:
		State.defaultreq &= ~NEED_STRAT;
		if (State.discardtest) {
			skip(fp, buf);
			return;
		}
		break;
	case SEC_CODE:
		State.defaultreq &= ~NEED_CODE;
		if (State.discardtest) {
			skip(fp, buf);
			return;
		}

		if (State.err == ER_VALUE)
			valerrdefs();

		State.err = 0;
		break;
	case SEC_HEADER:
		line = mcstrdup(buf);
		if ((str = strtok(line+strlen(D_HEADER), SEPS)) == NULL)
			str = "";
		/*
		 * If the name has changed, then reset the assertion number to
		 * zero.
		 */
		if (State.name == NULL || strcmp(str, State.name) != 0)
			State.assertion = 0;
		State.name = str;
		State.chap = strtok((char*)0, SEPS);
		if (State.name == NULL)
			State.name = "NoName";
		if (State.chap == NULL)
			State.chap = "";

		/*
		 * If we are in macro mode then either take an explicit macro
		 * name if given, or construct the name by removing the leading
		 * letter 'X'.
		 */
		if (mflag) {
			if (Settings.macroname)
				State.name = Settings.macroname;
			if (State.name[0] == 'X')
				State.name++;
		}

		/* Check for xprotocol test */
		i = strlen(State.chap);
		if (i > 5 && strcmp(State.chap + i - 5, "PROTO") == 0)
			State.xproto = 1;

		break;
	case SEC_ASSERTION:
	case SEC_DEFASSERT:
		/*
		 * Are we interested in this assertion?  If not then just skip
		 * code and strategy until the next assertion.
		 */
		if (!isassertwanted(State.assertion)) {
			State.defaultreq = 0;
			State.discardtest = 1;
			skip(fp, buf);
			return;
		}
		State.discardtest = 0;
		break;
	}

	if (secsw[Cmdname][sec])
		(*secsw[Cmdname][sec])(fp, buf);
}

/*
 * Switch out to the command specific hook command.
 */
dohook(buf, hook)
char	*buf;
int 	hook;
{
	if (hook >= NHOOK) {
		(void) fprintf(stderr, "Internal error: invalid hook\n");
		errexit();
	}
	if (Cmdname >= NCMDS) {
		(void) fprintf(stderr, "Internal error: command not implemented\n");
		errexit();
	}

	if (hooksw[Cmdname][hook])
		(*hooksw[Cmdname][hook])(buf);
}

/*
 * Skip over this section.
 */
void
skip(fp, buf)
FILE	*fp;
char	*buf;
{
	while (newline(fp, buf) != NULL && !SECSTART(buf))
		;
}

/*
 * Copy the complete section straight to the output, including the
 * section start line.
 */
void
echo(fp, buf, fpout)
FILE	*fp;
char	*buf;
FILE	*fpout;
{
	fwrite(buf, strlen(buf), 1, fpout);
	echon(fp, buf, fpout);
}

/*
 * Copy this section straight to the output, without the section start
 * line.
 */
void
echon(fp, buf, fpout)
FILE	*fp;
char	*buf;
FILE	*fpout;
{
	while (newline(fp, buf) != NULL && !SECSTART(buf))
		fwrite(buf, strlen(buf), 1, fpout);
}

/*
 * At this point we insert any default code that needs inserting.
 */
dodefaults(buf)
char	*buf;
{
FILE	*fp;
int 	needed;

	/*
	 * If there is any supplied strategy or code then there are no defaults.
	 */
	if (!State.defaultreq)
		return;
	needed = State.defaultreq;
	State.defaultreq = 0;

	if (State.err) {
		errcode(buf);
		State.err = 0;
		return;
	}

	/* Default code stub */
	fp = cretmpfile(F_TDEFCODE);

	switch (State.category) {
	case CAT_B: case CAT_D:	/* Untested */
		if (needed & NEED_STRAT) {
			(void) fprintf(fp, ">>STRATEGY\n");
			(void) fprintf(fp, "Report UNTESTED\n");
		}
		if (needed & NEED_CODE) {
			(void) fprintf(fp, ">>CODE\n");
			(void) fprintf(fp, "\treport(\"%s\");\n", State.reason);
			(void) fprintf(fp, "\tUNTESTED;\n");
		}
		break;
	default:	/* No code written */
		if (needed & NEED_STRAT) {
			(void) fprintf(fp, ">>STRATEGY\n");
			if (needed & NEED_CODE)
				(void) fprintf(fp, "Report that no code has been written for this test.\n");
			else
				(void) fprintf(fp, "No strategy has been written for this test\n");
		}
		if (needed & NEED_CODE) {
			(void) fprintf(fp, ">>CODE\n");
			(void) fprintf(fp, "\treport(\"No code written for this assertion.\");\n");
		}
		break;
	}
	fclose(fp);

	includefile(F_TDEFCODE, buf);
}
