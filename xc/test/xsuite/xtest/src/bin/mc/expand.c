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
 * $XConsortium: expand.c,v 1.3 94/04/17 21:00:20 rws Exp $
 *
 * Author: Steve Ratcliffe, UniSoft Ltd.
 */

#include	"stdio.h"
#include	"mc.h"

#define	F_BANNER	"mepbanner.tmc"
#define	F_EXPAND	"mepexpand.tmc"

static	FILE	*FpBanner;
static	FILE	*FpExp;

void
mepcopyright(fp, buf)
FILE	*fp;
char	*buf;
{
static int 	firsttime = 1;

	while (newline(fp, buf) != NULL && !SECSTART(buf)) {
		if (strncmp(buf, " */", 3) == 0)
			strcpy(buf, " * \n");
		if (firsttime || strinstr(buf, "SCCS"))
			fputs(buf, FpBanner);
	}
	firsttime = 0;
}

void
mepecho(fp, buf)
FILE	*fp;
char	*buf;
{
	echo(fp, buf, FpExp);
}

/* Hooks */
/*ARGSUSED*/
void
mepstart(buf)
char	*buf;
{
	FpExp = cretmpfile(F_EXPAND);
	FpBanner = cretmpfile(F_BANNER);
}

/*ARGSUSED*/
void
mepend(buf)
char	*buf;
{
	(void) fprintf(FpBanner, " */\n");
	outfile(FpBanner);
	outfile(FpExp);
	fclose(FpExp);
}

void
mepset(buf)
char	*buf;
{
	fprintf(FpExp, ">>SET %s", buf);
}

void
mepcomment(buf)
char	*buf;
{
	fprintf(FpExp, "%s", buf);
}
