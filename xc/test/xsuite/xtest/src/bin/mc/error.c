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
 * $XConsortium: error.c,v 1.7 94/04/17 21:00:19 rws Exp $
 *
 * Author: Steve Ratcliffe, UniSoft Ltd.
 */


#include <stdio.h>

#include "mc.h"

extern	struct	state	State;

static	char	*elist[] = {
	"Access grab", "EAcc1.mc",
	"Access colormap-free", "EAcc2.mc",
	"Access colormap-store", "EAcc3.mc",
	"Access acl", "EAcc4.mc",
	"Access select", "EAcc5.mc",
	"Alloc", "EAll.mc",
	"Atom", "EAto.mc",
	"Color", "ECol.mc",
	"Cursor", "ECur.mc",
	"Drawable", "EDra.mc",
	"Font bad-fontable", "EFon2.mc",
	"Font bad-font", "EFon1.mc",
	"GC", "EGC.mc",
	"Match inputonly", "EMat1.mc",
	"Match gc-drawable-depth", "EMat2.mc",
	"Match gc-drawable-screen", "EMat3.mc",
	"Match wininputonly", "EMat4.mc",
	"Name font", "ENam1.mc",
	"Name colour", "ENam2.mc",
	"Pixmap", "EPix.mc",
	"Value", "EVal.mc",
	"Window", "EWin.mc",
	(char*)0
};

#define	MAXALTS	128

static	char	errfile[32];
static	char	*Alts[MAXALTS];
static	int 	Nalts;

errtext(buf)
char	*buf;
{
char	**mp;
char	*savline;
char	*type;
char	*strtok();
static	char	*sep = " ,\t";

	State.err = ER_NORM;

	type = buf+3;
	if (type[strlen(type)-1] == '\n')
		type[strlen(type)-1] = '\0';

	while (type[0] == ' ' || type[0] == '\t')
		type++;

	/* Skip over any initial 'Bad' */
	if (strncmp(type, "Bad", 3) == 0)
		type += 3;

	for (mp = elist; *mp; mp++) {
		if (strncmp(*mp, type, strlen(*mp)) == 0)
		    break;
	}

	if (*mp == NULL) {
		err("Bad .ER error code");
		(void) fprintf(stderr, " (%s)\n", type);
		errexit();
	}


	/*
	 * This is only used on BadPixmap etc. but we do it for all.
	 */
	savline = mcstrdup(type);
	Alts[0] = strtok(savline, sep);
	for (Nalts = 1; Nalts < MAXALTS; Nalts++) {
		if ((Alts[Nalts] = strtok((char*)0, sep)) == NULL)
			break;
	}

	(void) strcpy(errfile, "error/");
	(void) strcat(errfile, *(mp+1));

	/* BadValue is a special case to be dealt with */
	if (strncmp(type, "Value", 5) == 0) {
		valerror(buf);
	} else {
		State.abortafter = 1;
		includefile(errfile, buf);
	}
}

static int 	wasmasktype;

valerror(buf)
char	*buf;
{
int 	i;
FILE	*fp;

	State.err = ER_VALUE;

	fp = cretmpfile(F_TVAL);

	(void) fprintf(fp, ">>ASSERTION Bad A\n");
	(void) fprintf(fp, "When the value of\n.A %s\n", Alts[1]);

	i = 2;
	if (strcmp(Alts[i], "mask") == 0) {
		wasmasktype = 1;
		i++;
	} else
		wasmasktype = 0;

	if (wasmasktype)
		(void) fprintf(fp, "is not a bitwise combination of\n");
	else
		(void) fprintf(fp, "is other than\n");

	for (; i < Nalts; i++) {
		(void) fprintf(fp, ".S %s", Alts[i]);
		if (i == Nalts-2)
			(void) fprintf(fp, "%s", "\nor\n");
		else
			(void) fprintf(fp, "%s", " ,\n");
	}

	(void) fprintf(fp, "then a\n.S BadValue\nerror occurs.\n");

	(void) fprintf(fp, ">>EXTERN\n\n");
	(void) fprintf(fp, "/* Value list for use in test t%03d */\n", State.assertion+1);
	(void) fprintf(fp, "static %s	%svallist[] = {\n",
		wasmasktype? "unsigned long": "int ", Alts[1]);


	for (i = (wasmasktype)? 3: 2; i < Nalts; i++) {
		(void) fprintf(fp, "\t%s,\n", Alts[i]);
	}

	(void) fprintf(fp, "};\n\n");
	(void) fclose(fp);

	includefile(F_TVAL, buf);
}

valerrdefs()
{
char	line[MAXLINE];

	/*
	 * Do the define bits.
	 */
	line[0] = '\0';
	(void) strcat(line, "#undef\tVALUE_ARG\n");
	(void) sprintf(line+strlen(line), "#define\tVALUE_ARG %s\n", Alts[1]);
	(void) strcat(line, "#undef\tVALUE_LIST\n");
	(void) sprintf(line+strlen(line), "#define\tVALUE_LIST %svallist\n", Alts[1]);
	(void) sprintf(line+strlen(line), "#undef NOTMEMTYPE\n");
	(void) sprintf(line+strlen(line), "#define NOTMEMTYPE %s\n",
		(wasmasktype)? "unsigned": "");
	(void) strcat(line, "#undef\tNOTMEMBER\n");
	if (wasmasktype)
		(void) strcat(line, "#define\tNOTMEMBER notmaskmember\n");
	else
		(void) strcat(line, "#define\tNOTMEMBER notmember\n");

	putbackline(line);
}


/*
 * List out all the alternatives that have been defined for this error.
 * This allows you to do things like 'a valid Pixmap or None'.
 */
erralternates(out)
char	*out;
{
int 	i;
char	*word;

	*out = '\0';
	for (i = 0; i < Nalts; i++) {
		word = Alts[i];
		(void) strcat(out, word);

		if (i < Nalts-2)
			(void) strcat(out, ",\n.S ");
		if (i == Nalts-2)
			(void) strcat(out, "\nor\n.S ");
	}
	if (Nalts > 1)
		(void) strcat(out, " ");
	(void) strcat(out, ",\n");

	return(strlen(out));
}

/*
 * If there has not been any user supplied code then use the default
 * error code in the file.
 */
errcode(bp)
char	*bp;
{
	if (State.err != ER_VALUE)
		State.skipsec = 2;
	includefile(errfile, bp);
}

