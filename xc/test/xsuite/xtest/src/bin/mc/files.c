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
 * $XConsortium: files.c,v 1.7 94/04/17 21:00:20 rws Exp $
 *
 * Author: Steve Ratcliffe, UniSoft Ltd.
 */

#include "stdio.h"
#include "mc.h"

#define	MC_LOC		"/xtest/lib/"

extern	char	*getenv();
extern	char	*Filename;
extern	int 	Lineno;

static	int 	Filenum;
static	char	*Filetemp[50];
static	int 	Filetind;

FILE	*opencopy();

/*
 * Open the first source file.
 */
FILE *
nextfile(sources)
struct	mclist	*sources;
{
FILE	*fp;
static	int 	firsttime = 1;

	Lineno = 0;
	if ((Filename = getmclistitem(sources, Filenum++)) == NULL) {
		if (firsttime)
			fp = stdin;
		else
			fp = NULL;
	} else {
		if ((fp = fopen(Filename, "r")) == NULL) {
			(void) fprintf(stderr, "Could not open %s\n", Filename);
			errexit();
		}
	}

	firsttime = 0;
	return(fp);
}

/*
 * Copy the contents of the corresponding skeleton file to the output.
 * The skeleton files have a .mc suffix.
 */
void
outcopy(file)
char	*file;
{
register FILE	*fp;
char 	buf[BUFSIZ];

	/*
	 * Look for a corresponding file with name lib/mc/{*}.mc .
	 */
	(void) sprintf(buf, "mc/%s", file);
	file = buf;
	if (strcmp(file+strlen(file)-3, ".mc") != 0) {
		/*
		 * For debugging only -- there is an error in the file name
		 * conventions.
		 */
		fprintf(stderr, "Internal error outcopy file does not end in .mc\n");
	}

	if ((fp = fopen(mcpath(file), "r")) != NULL) {
		outfile(fp);
		fclose(fp);
	} else {
		fprintf(stderr, "Could not open skeleton file %s\n", mcpath(file));
		errexit();
		/*NOTREACHED*/
	}
}

/*
 * Create a temporary file with the given name. Open a stream to it
 * and return the stream pointer.
 */
FILE *
cretmpfile(file)
char	*file;
{
register FILE	*fp;

	if ((fp = fopen(file, "w+")) == NULL) {
		(void) fprintf(stderr, "Could not open %s\n", file);
		errexit();
	}
	filetemp(file);
	return(fp);
}


/*
 * Combine all the files in the right order onto
 * stdout.
 */
outfile(fp)
FILE	*fp;
{
static FILE	*fout;
static int 	olineno = 0;
static char	*ofilename;
char	buf[BUFSIZ];
register int 	n;
extern	char	*OutFile;
extern	int 	pflag;
extern	int 	Cmdname;

	if (!fout) {
		if (OutFile) {
			if ((fout = fopen(OutFile, "w")) == NULL) {
				(void) fprintf(stderr, "Could not open %s for output\n", OutFile);
				errexit();
			}
			ofilename = OutFile;
		} else {
			fout = stdout;
			ofilename = "stdout";
		}
	}

	rewind(fp);

	if (pflag && Cmdname == CMD_MC) {
		while (fgets(buf, BUFSIZ, fp) != NULL) {
			olineno++;
			if (buf[0] == '>' && buf[1] == '>' && buf[2] == 'G')
				(void) sprintf(buf, "#line %d \"%s\"\n", olineno+1, ofilename);
			fputs(buf, fout);
		}
	} else {
		while ((n = fread(buf, sizeof(char), BUFSIZ, fp)) > 0)
			(void) fwrite(buf, n, 1, fout);
	}
}

remfiles()
{
int 	i;

	for (i = 0; i < Filetind; i++)
		(void) unlink(Filetemp[i]);
	Filetind = 0;
}

includefile(file, bp)
char	*file;
char	*bp;
{
char	*name;
char	*savline;
char	*savfilename;
char	*path;
char	*strtok();
FILE	*fp;
int 	savlineno;

	savline = mcstrdup(bp);
	file = mcstrdup(file);

	name = strtok(file, " \t\n");
	if (name == (char*)0) {
		err("Missing file name on include directive\n");
		errexit();
	}

	if ((fp = fopen(name, "r")) == NULL) {
		path = mcpath(name);
		if ((fp = fopen(path, "r")) == NULL) {
			err("");
			(void) fprintf(stderr, "Cannot open include file %s\n", name);
			errexit();
		}
	}

	savfilename = Filename;
	savlineno = Lineno;
	Filename = name;
	Lineno = 0;

	dohook(name, HOOK_INCSTART);
	dosections(fp, bp);
	(void) fclose(fp);
	dohook(name, HOOK_INCEND);

	Filename = savfilename;
	Lineno = savlineno;

	strcpy(bp, savline);
	free(savline);
	free(file);
}

/*
 * Mark the file name for deletion at the end.
 */
void
filetemp(name)
char	*name;
{
int 	i;

	for (i = 0; i < Filetind; i++) {
		if (strcmp(Filetemp[i], name) == 0)
			return;
	}
	Filetemp[Filetind++] = name;
}

/*
 * Return path name of file in the xtest lib directory.
 */
char *
mcpath(file)
char	*file;
{
char	*rp;
char	*path;
int 	size;

	rp = getenv("TET_ROOT");
	if (rp == NULL) {
		(void) fprintf(stderr, "TET_ROOT not set in environment\n");
		errexit();
	}
	size = strlen(rp)+strlen(MC_LOC)+strlen(file)+1;
	path = (char *)malloc((unsigned)size);
	if (path == NULL) {
		(void) fprintf(stderr, "Out of memory\n");
		errexit();
	}
	(void) strcpy(path, rp);
	(void) strcat(path, MC_LOC);
	(void) strcat(path, file);

	return(path);
}
