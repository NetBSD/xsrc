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
 * $XConsortium: gccomps.c,v 1.2 94/04/17 21:00:21 rws Exp $
 *
 * Author: Steve Ratcliffe, UniSoft Ltd.
 */


#include	<stdio.h>
#include	<string.h>

#include	"mc.h"

static	struct	gclookup {
	char	*gccompname;
	char	*include;
} gclookup[] = {
	/*
	 * Note:  If there are gccompname's that are initial strings of others
	 * then they must be afterwards.
	 */
	{"function", "function"},
	{"plane-mask", "plane-mask"},
	{"foreground", "foreground"},
	{"background", "background"},
	{"line-width", "line-width"},
	{"line-style", "line-style"},
	{"cap-style", "cap-style"},
	{"join-style", "join-style"},
	{"fill-style", "fill-style"},
	{"fill-rule", "fill-rule"},
	{"arc-mode", "arc-mode"},
	{"tile-stipple-x-origin", "ts-x-origin"},
	{"tile-stipple-y-origin", "ts-y-origin"},
	{"ts-x-origin", "ts-x-origin"},
	{"ts-y-origin", "ts-y-origin"},
	{"tile", "tile"},
	{"stipple", "stipple"},
	{"font", "font"},
	{"subwindow-mode", "subwindow-mode"},
	{"graphics-exposures", "graphics-exposures"},
	{"clip-x-origin", "clip-x-origin"},
	{"clip-y-origin", "clip-y-origin"},
	{"clip-mask", "clip-mask"},
	{"dash-offset", "dash-offset"},
	{"dash-list", "dash-list"},
	{"dashes", "dash-list"},
};

#define	NGCCOMP	(sizeof(gclookup)/sizeof(struct gclookup))

gccomps(fp, buf)
FILE	*fp;
char	*buf;
{
struct	gclookup	*lp;
char	*cp;

	while (newline(fp, buf) != NULL && !SECSTART(buf)) {
		if (strncmp(buf, ".M", 2) != 0)
			continue;

		cp = buf+3;
		while (*cp && *cp == ' ' || *cp == '\t')
			cp++;

		for (lp = gclookup; lp < gclookup+NGCCOMP; lp++) {
			if (strncmp(cp, lp->gccompname, strlen(lp->gccompname)) == 0)
				break;
		}

		if (lp == gclookup+NGCCOMP) {
			err("");
			(void) fprintf(stderr, "Unrecognised gc component name %s\n", cp);
			errexit();
		}

		(void) sprintf(buf, "gc/%.9s.mc\n", lp->include);
		includefile(buf, buf);
	}
}
