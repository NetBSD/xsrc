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
 * $XConsortium: main.c,v 1.9 94/04/17 21:00:22 rws Exp $
 *
 * Author: Steve Ratcliffe, UniSoft Ltd.
 */

#include	<stdio.h>
#include	<sys/types.h>
#include	<signal.h>
#include	<ctype.h>

#include "mc.h"

extern	char	*strtok();
extern	char	*strrchr();

/*
 * The input buffer
 */
char	Ibuf[MAXLINE];
int 	Lineno;
char	*Filename;

char	*OutFile;	/* Output file name */

FILE	*FpSource = stdin;

int 	aflag;
int 	dflag;
int 	hflag;
int 	lflag;
int 	pflag;
int 	sflag;
char	*sopt;
int 	mflag;

static	void	sigclean();
static	void	setupaslist();

struct	mclist *Sources;
int 	Cmdname;

struct	state	State;

/*
 * This table decodes argv[0] to the correct command. Induvidual getopt
 * strings and usage messages are supplied for each.  If all else fails
 * the -c option can be used to get a particular command.
 * All commands should if applicable take -a for a assertion list,
 * -o for an output file.  Also unadvertised options of -d for debug
 * and -c to change the command.
 */
struct	cmdinfo	{
	int 	cmd;
	char	*name;
	char	*opts;
	char	*usage;
} cmdinfo[] = {
	{ CMD_MEXPAND, "mexpand", "c:a:o:d",
		"mexpand [-a a_list] [-o <output-file>]" },
	{ CMD_MC, "mc",	"c:a:o:dlsmp",
		"mc [-a a_list] [-o <output-file>] [-l] [-m] [-s] [-p]" },
	{ CMD_MKMF, "mmkf", "c:o:ds:",
		"mmkf [-o <output-file>] [-s sections]" },
	{ CMD_MA, "ma", "c:a:o:dhpsm",
		"ma [-a a_list] [-o <output-file>] [-h] [-s] [-p] [-m]" },
	{ CMD_MAS, "mas", "c:a:o:dm",
		"mas [-a a_list] [-o <output-file>] [-m]" },
};

/*
 * If argv[0] is not recognised then this is used to suggest the use
 * of -c.  (Or fix the links!)
 */
static struct	cmdinfo	defcmd = {
	CMD_MC,
	"mtool",
	"c:o:d",
	"mtool -c <command> [-o <output-file>] [other-options]"
};

main(argc, argv)
int 	argc;
char	**argv;
{
int 	c;
int 	i;
char	*cp;
struct	cmdinfo	*cip;
extern	int 	optind;
extern	char	*optarg;

newcmd:
	cp = strrchr(argv[0], '/');
	if (cp == NULL)
		cp = argv[0];
	else
		cp++;
	for (i = 0; i < NELEM(cmdinfo); i++) {
		if (strcmp(cp, cmdinfo[i].name) == 0) {
			cip = cmdinfo+i;
			break;
		}
	}
	if (i == NELEM(cmdinfo))
		cip = &defcmd;

	Cmdname = cip->cmd;

	while ((c = getopt(argc, argv, cip->opts)) != EOF) {
		switch (c) {
		case 'a':
			aflag++;
			setupaslist(optarg);
			break;
		case 'd':
			dflag++;
			break;
		case 'h':
			hflag++;
			break;
		case 'l':
			lflag++;
			break;
		case 'm':
			mflag++;
			break;
		case 'p':
			pflag++;
			break;
		case 's':
			sflag++;
			sopt = optarg;
			break;
		case 'o':
			OutFile = optarg;
			break;
		case 'c':
			argv[0] = optarg;
			goto newcmd;
		case '?':
			(void) fprintf(stderr, "Invalid option\n");
			(void) fprintf(stderr, "%s\n", cip->usage);
			errexit();
			break;
		}
	}

	signal(SIGINT, sigclean);
	signal(SIGHUP, sigclean);
	signal(SIGTERM, sigclean);

	/*
	 * Collect the other arguments which sould be names of combined
	 * source files.
	 */
	Sources = createmclist();
	for (; optind < argc; optind++)
		Sources = addmclist(Sources, argv[optind]);

	State.name = "";
	State.chap = "";

	dohook((char*)0, HOOK_START);

	while ((FpSource = nextfile(Sources)) != NULL) {
		dosections(FpSource, Ibuf);
		dodefaults(Ibuf);
		(void) fclose(FpSource);
	}

	dohook((char*)0, HOOK_END);

	remfiles();
	exit(0);
}

static char	putbackbuf[MAXLINE];

char *
newline(fp, buf)
FILE	*fp;
char	*buf;
{
char	*res;
extern	int 	Outputon;

	if (*putbackbuf) {
		strcpy(buf, putbackbuf);
		putbackbuf[0] = '\0';
		return(buf);
	}

	while ((res = fgets(buf, MAXLINE, fp)) != NULL) {
		
		Lineno++;

		if (buf[0] == '>') {

			if (strncmp(buf, D_COMMENT, D_COMMENT_LEN) == 0) {
				dohook(buf, HOOK_COMMENT);
				continue;
			}

			if (strncmp(buf, D_SET, strlen(D_SET)) == 0) {
				dohook(buf+strlen(D_SET)+1, HOOK_SET);
				setcmd(buf);
				continue;
			}
		}
		if (buf[0] == '#') {
			if (!hashcmd(buf))
				continue;
		}
		if (Outputon == 0)
			continue;
		break;
	}

	if (res)
		expandxname(res);
	return(res);
}

/*
 * Push back a line of input.
 */
putbackline(line)
char	*line;
{
	if (*putbackbuf) {
		err("Internal error: one line already pushed back\n");
		errexit();
	}
	(void) strcpy(putbackbuf, line);
}

/*
 * Expand occurences of xname in assertion or strategy sections to
 * the name of the test.
 */
/*
 * Since this routine depends on assertion/strategy state so much
 * then perhaps this is the wrong place to do this.
 */
void
expandxname(line)
char	*line;
{
static	char	buf[MAXLINE];
char	*cp;
char	*np;

	if (State.sectype != SEC_ASSERTION && 
	    State.sectype != SEC_DEFASSERT &&
	    State.sectype != SEC_STRATEGY)
		return;

	for (np = buf, cp = line; *cp; ) {
		if (*cp == 'x') {
			if (strncmp(cp, "xname", 5) == 0) {
				if (State.sectype == SEC_ASSERTION ||
				    State.sectype == SEC_DEFASSERT) {
					if (cp != line)
						*np++ = '\n';
					(void) strcpy(np, ".F "); np += 3;
				}
				(void) strcpy(np, State.name); np += strlen(State.name);
				cp += 5;/* len of xname XXX */
				if (State.sectype == SEC_ASSERTION ||
				    State.sectype == SEC_DEFASSERT) {
					if (ispunct(*cp)) {
						*np++ = ' ';
						*np++ = *cp++;
						*np++ = '\n';
					} else {
						*np++ = '\n';
					}
					while (isspace(*cp))
						cp++;
				}
			} else if (strncmp(cp, "xerrlist", 8) == 0) {
				/* This should not occur in strategies but no reason to check */
				np += erralternates(np);
				cp += 8;	/* len of xerrlist */
				while (*cp == ' ' || *cp == '\n' || *cp == ',')	/* XXX */
					cp++;
			} else {
				*np++ = *cp++;
			}
		} else {
			*np++ = *cp++;
		}
	}
	*np++ = '\0';
	(void) strcpy(line, buf);
}

/*
 * Print out an error message with preceeding line and file information.
 */
err(mess)
char	*mess;
{
	fprintf(stderr, "%s: line %d: %s", Filename? Filename: "<stdin>", Lineno, mess);
}

/*
 * Tidy up and exit.
 */
errexit()
{
	remfiles();
	exit(EXIT_FAILURE);
}

/*
 * Action to take when terminated by a signal such as SIGINT.  For
 * unexpected signals no action is taken to aid debuging.
 */
/*ARGSUSED*/
static void
sigclean(sig)
int 	sig;
{
	errexit();
}

/*
 * Copy a string using malloced storage.
 */
char *
mcstrdup(s)
char	*s;
{
char	*bp;

	if (s == 0)
		return(s);

	bp = (char*)malloc((unsigned)strlen(s)+1);
	if (bp)
		(void) strcpy(bp, s);

	return(bp);
}

/*
 * Find a s2 within s1.
 */
char *
strinstr(s1, s2)
char	*s1;
char	*s2;
{
register char	*cp;
register int 	c;
int 	len;

	len = strlen(s2);
	if (len == 0)
		return(s1);

	c = *s2;

	for (cp = s1; *cp; cp++) {
		if (*cp == c) {
			if (strncmp(cp, s2, len) == 0) {
				return(cp);
			}
		}
	}

	return(NULL);
}

#define	MAXICLIST	100
static	struct	aslist	{
	short	begin;
	short	end;
} aslist[MAXICLIST];
static	int 	aslind;

static void
setupaslist(list)
char	*list;
{
char	*cp;

	for (cp = strtok(list, ",\t\n "); cp; cp = strtok((char*)0, ",\t\n ")) {
		if (*cp == '-')
			aslist[aslind].begin = 0;
		else
			aslist[aslind].begin = atoi(cp);
		while (*cp && *cp != '-')
			cp++;

		if (*cp == '-') {
			if (*++cp)
				aslist[aslind].end = atoi(cp);
			else
				aslist[aslind].end = 9999;
		} else
			aslist[aslind].end = aslist[aslind].begin;
		aslind++;
	}
}

int
isassertwanted(a)
int 	a;
{
int 	i;

	if (aslind == 0)
		return(1);

	for (i = 0; i < aslind; i++) {
		if (a >= aslist[i].begin && a <= aslist[i].end)
			return(1);
	}
	return(0);
}
