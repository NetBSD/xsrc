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
 * $XConsortium: stfntpth.m,v 1.11 94/04/17 21:05:50 rws Exp $
 */
>>TITLE XSetFontPath CH06
void

Display *display = Dsp;
char	**directories;
int 	ndirs;
>>EXTERN

/*
 * Startup and cleanup code attempt to save and restore the Font path
 * over the tests. 
 * This should not be needed now because saving and restoring the Font path
 * is done in startup/cleanup - but it is still done here in case that
 * scheme is ever changed.
 */
static	char	**savepath;
static	int 	savenum;
>>SET startup localstartup
static void
localstartup()
{
	startup();
	if(Dsp) {
		XSetErrorHandler(error_status);
		savepath = XGetFontPath(Dsp, &savenum);
		XSetErrorHandler(unexp_err);
	}
}
>>SET cleanup localcleanup
static void
localcleanup()
{
	if(Dsp) {
		if (savepath && savenum)
			XSetFontPath(Dsp, savepath, savenum);
		XSync(Dsp, 0);
	}
	cleanup();
}
>>ASSERTION Good A
When xname is called with the 
.A directories
argument specifying a list of directories in an operating system dependent
format,
then the directory search path for font lookup is set to
the list of directories 
in the order specified.
>>STRATEGY
Get TET variable XT_FONTPATH_GOOD
Set font path to this value.
Do simple check with XGetFontPath.
>>CODE
char	*fpathlist;
char	*fpathtmp;
char	*dirlist[MAX_DIRS];
char	*strtok();
char	**checkpath;
int 	nret;
int 	i;

	fpathlist = tet_getvar("XT_FONTPATH_GOOD");
	if (fpathlist == NULL || *fpathlist == '\0') {
		delete("XT_FONTPATH_GOOD not set in config file");
		return;
	}
	fpathtmp = (char *)calloc(strlen(fpathlist)+1, sizeof(char));
	strcpy(fpathtmp, fpathlist);

	for (i = 0; i < MAX_DIRS; i++) {
		dirlist[i] = strtok((i==0)? fpathtmp: (char*)0, SEP);
		if (dirlist[i] == NULL)
			break;
		debug(1, "dirlist entry %d - '%s'", i, dirlist[i]);
	}
	directories = dirlist;
	ndirs = i;

	if (ndirs <= 0) {
		delete("No components in supplied XT_FONTPATH_GOOD");
		return;
	}

	XCALL;

	checkpath = XGetFontPath(display, &nret);
	if (nret == ndirs)
		CHECK;
	else {
		report("Different number of directories returned");
		report("  was %d, expecting %d", nret, ndirs);
		FAIL;
	}

	if (ndirs < nret)
		nret = ndirs;
	for (i = 0; i < nret; i++) {
		debug(1, "got back list item '%s'", checkpath[i]);
		if (strcmp(dirlist[i], checkpath[i]) == 0)
			CHECK;
		else {
			report("Font path component did not match what was set");
			report("  was '%s', expecting '%s'", checkpath[i], dirlist[i]);
			FAIL;
		}
	}

	CHECKPASS(1+ndirs);
>>ASSERTION Good A
On a call to xname, the directory search 
path for font lookup is set for all clients.
>>STRATEGY
Open a second client.
Set font path in first client.
Get font path in the second client.
Verify that the font is the same in each client.
>>CODE
Display	*client2;
char	*fpathlist;
char	*fpathtmp;
char	*dirlist[MAX_DIRS];
char	*strtok();
char	**checkpath;
int 	nret;
int 	i;

	client2 = XOpenDisplay(DisplayString(display));
	if (client2 == NULL || isdeleted()) {
		delete("Could not open second client");
		return;
	}

	fpathlist = tet_getvar("XT_FONTPATH_GOOD");
	if (fpathlist == NULL || *fpathlist == '\0') {
		delete("XT_FONTPATH_GOOD not set in config file");
		return;
	}
	fpathtmp = (char *)calloc(strlen(fpathlist)+1, sizeof(char));
	strcpy(fpathtmp, fpathlist);

	for (i = 0; i < MAX_DIRS; i++) {
		dirlist[i] = strtok((i==0)? fpathtmp: (char*)0, SEP);
		if (dirlist[i] == NULL)
			break;
		debug(1, "dirlist entry %d - '%s'", i, dirlist[i]);
	}
	directories = dirlist;
	ndirs = i;

	if (ndirs <= 0) {
		delete("No components in supplied XT_FONTPATH_GOOD");
		return;
	}

	XCALL;

	checkpath = XGetFontPath(client2, &nret);
	if (nret == ndirs)
		CHECK;
	else {
		report("Different number of directories returned");
		report("  was %d, expecting %d", nret, ndirs);
		FAIL;
	}

	if (ndirs < nret)
		nret = ndirs;
	for (i = 0; i < nret; i++) {
		debug(1, "got back list item '%s'", checkpath[i]);
		if (strcmp(dirlist[i], checkpath[i]) == 0)
			CHECK;
		else {
			report("Font path component did not match what was set");
			report("  was '%s', expecting '%s'", checkpath[i], dirlist[i]);
			FAIL;
		}
	}

	CHECKPASS(1+ndirs);
>>ASSERTION Good B 1
On a call to XSetFontPath, the X server flushes all cached information
about fonts for which there are currently no explicit resource ID's
allocated.
>>ASSERTION Bad C
When the 
.A directories
argument is invalid for the operating system,
then a
.S BadValue 
error occurs.
>># This says nothing
>>#and the value returned is operating system dependent.
>>STRATEGY
Get TET variable XT_FONTPATH_BAD
If this is set to UNSUPPORTED, then no bad paths are possible
  result is UNSUPPORTED
Set font path to this value.
Verify that BadValue error is generated.
>>CODE BadValue
char	*fpathlist;
char	*dirlist[MAX_DIRS];
char	*strtok();
int 	i;

	fpathlist = tet_getvar("XT_FONTPATH_BAD");
	if (fpathlist == NULL || *fpathlist == '\0') {
		delete("XT_FONTPATH_BAD not set in config file");
		return;
	}
	if (strcmp(fpathlist, "UNSUPPORTED") == 0) {
		report("No bad paths are possible");
		tet_result(TET_UNSUPPORTED);
		return;
	}

	for (i = 0; i < MAX_DIRS; i++) {
		dirlist[i] = strtok((i==0)? fpathlist: (char*)0, SEP);
		if (dirlist[i] == NULL)
			break;
	}
	directories = dirlist;
	ndirs = i;

	if (ndirs <= 0) {
		delete("No components in supplied XT_FONTPATH_BAD");
		return;
	}

	XCALL;

	if (geterr() == BadValue)
		PASS;
	else
		FAIL;

>># HISTORY kieron Completed	Reformat and tidy to ca pass
