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
 * $XConsortium: lstfntswth.m,v 1.11 94/04/17 21:05:43 rws Exp $
 */
>>TITLE XListFontsWithInfo CH06
char **

Display	*display = Dsp;
char	*patternarg;
int 	maxnames = 9999;
int 	*count_return = &count;
XFontStruct	**info_return = &finfo;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>EXTERN
#include    <ctype.h>
#include    <string.h>

static	int 	count;
static	XFontStruct *finfo;

extern	XFontStruct	xtfont0;
extern	char	*xtfont0cpright;
extern	struct	fontinfo	fontinfo[];
extern	int 	nfontinfo;

static lowerstring(str)
char    *str;
{
	for (; *str; str++) {
		if (isupper(*str))
			tolower(*str);
	}
}

>>ASSERTION Good A
When the
.A pattern
argument
is a string in ISO Latin-1 encoding terminated with ASCII nul,
then a call to xname returns an array of
strings terminated with ASCII nul which are available font names 
that match the
.A pattern
argument
and returns the number of fonts in the
.A count_return
argument
and their associated font information 
excluding the per-character metrics
in the 
.A info_return
argument.
>>STRATEGY
Set patternarg to "xtfont0"
Call XListFontsWithInfo.
Verify that returned count was 1.
Verify that the returned names pointer was non-NULL.
Lower-case the returned string.
Verify that xtfont0 was returned.
Verify that font information is correct.
>>CODE
char    **names;

	patternarg = "xtfont0";
	names = XCALL;

	if (count != 1) {
		report("count_return was %d, expecting 1", count);
		FAIL;
	} else
		CHECK;

	if (names == NULL) {
		report("NULL was returned");
		FAIL;
		return;
	} else
		CHECK;

	lowerstring(*names);
	if (strcmp(*names, patternarg) != 0) {
		report("returned name was '%s', expecting '%s'", *names, patternarg);
		FAIL;
	} else
		CHECK;

	xtfont0.per_char = NULL;
	if (checkfsp(finfo, &xtfont0, xtfont0cpright))
		CHECK;
	else {
		report("font information was incorrect");
		FAIL;
	}

	CHECKPASS(4);

>>ASSERTION Good A
Upper and lower case character in the
.A pattern
argument refer to the same font.
>>STRATEGY
Try matching with XtFoNt0
Call XListFontsWithInfo.
Verify that returned count was 1.
Verify that the returned names pointer was non-NULL.
Lower-case the returned string.
Verify that xtfont0 was returned.
Verify that the font information was correct.
>>CODE
char	**names;

	patternarg = "XtFoNt0";
	names = XCALL;

	if (count != 1) {
		report("count_return was %d, expecting 1", count);
		FAIL;
	} else
		CHECK;

	if (names == NULL) {
		report("NULL was returned");
		FAIL;
		return;
	} else
		CHECK;

	lowerstring(*names);
	patternarg = "xtfont0";
	if (strcmp(*names, patternarg) != 0) {
		report("returned name was '%s', expecting '%s'", *names, patternarg);
		FAIL;
	} else
		CHECK;

	xtfont0.per_char = NULL;
	if (checkfsp(finfo, &xtfont0, xtfont0cpright))
		CHECK;
	else {
		report("font infomation was incorrect");
		FAIL;
	}

	CHECKPASS(4);
>>ASSERTION Good A
Each asterisk (*) in the string is a wildcard for any number of characters.
>>STRATEGY
Set patternarg to "x*t*t*"
Call XListFontsWithInfo.
Verify that at least all the xtest fonts are returned, and
that any other returned string matches the patternarg.
>>CODE
char	**names;
char	*curname;
int 	i;
int 	j;

	patternarg = "x*t*t*";
	names = XCALL;

	if (count < XT_NFONTS) {
		report("returned count was %d, expecting at least %d", count,XT_NFONTS);
		FAIL;
	} else
		CHECK;

	if (names == NULL) {
		report("returned name pointer was NULL");
		FAIL;
		return;
	} else
		CHECK;

	for (i = 0; i < count; i++) {
		curname = names[i];
		lowerstring(curname);
		for (j = 0; j < nfontinfo; j++) {
			if (strcmp(curname, fontinfo[j].name) == 0) {
				/* Check if we have already seen this one */
				if (fontinfo[j].flag) {
					report("name %s repeated in list", curname);
					FAIL;
				} else {
					fontinfo[j].flag = 1;

					/* Check font infomation */
					trace("checking font %s", curname);
					fontinfo[j].fontstruct->per_char = NULL;
					if (checkfsp(&finfo[i], fontinfo[j].fontstruct,
							*fontinfo[j].string)) {
						CHECK;
					} else {
						report("font information was incorrect");
						FAIL;
					}
				}
				break;
			}
		}
		if (j == nfontinfo) {
		char	*cp;

			/* It wasn't one of ours... */
			trace("extra font name %s found", curname);
			cp = strchr(curname, 'x');
			if (cp == NULL) {
				report("Found name %s that did not match patternarg");
				FAIL;
			} else if ((cp = strchr(cp, 't')) == NULL) {
				report("Found name %s that did not match patternarg");
				FAIL;
			} else if ((cp = strchr(cp, 't')) == NULL) {
				report("Found name %s that did not match patternarg");
				FAIL;
			} else {
				CHECK;
			}
		}
	}
	for (i = 0; i < nfontinfo; i++) {
		if (fontinfo[i].flag == 0) {
			report("xtest font '%s' was not found", fontinfo[i].name);
			FAIL;
		} else {
			/* reset to zero for next test */
			fontinfo[i].flag = 0;
			CHECK;
		}
	}

	CHECKPASS(2+count+nfontinfo);
>>ASSERTION Good A
Each question mark (?) in the string is a wildcard for a single character.
>>STRATEGY
Set patternarg to "x?f?nt?"
Call XListFontsWithInfo.
Verify that returned count is at least XT_NFONTS
Verify that the xtest font names are returned.
Verify that any other name returned matches the patternarg.
Verify that font information is correct for the xtest fonts.
>>CODE
char	**names;
char	*curname;
int 	i;
int 	j;

	patternarg = "x?f?nt?";
	names = XCALL;

	if (count < XT_NFONTS) {
		report("returned count was %d, expecting at least %d", count,XT_NFONTS);
		FAIL;
	} else
		CHECK;

	if (names == NULL) {
		report("returned name pointer was NULL");
		FAIL;
		return;
	} else
		CHECK;

	for (i = 0; i < count; i++) {
		curname = names[i];
		lowerstring(curname);
		for (j = 0; j < nfontinfo; j++) {
			if (strcmp(curname, fontinfo[j].name) == 0) {
				/* Check if we have already seen this one */
				if (fontinfo[j].flag) {
					report("name %s repeated in list", curname);
					FAIL;
				} else {
					fontinfo[j].flag++;

					trace("checking font %s", curname);
					fontinfo[j].fontstruct->per_char = NULL;
					if (checkfsp(&finfo[i], fontinfo[j].fontstruct,
							*fontinfo[j].string)) {
						CHECK;
					} else {
						report("font information was incorrect");
						FAIL;
					}
				}
				break;
			}
		}
		if (j == nfontinfo) {
		int 	len;

			/* It wasn't one of ours... */
			trace("extra font name %s found", curname);
			len = strlen(patternarg);
			if (strlen(curname) != len) {
				report("name '%s' did not match patternarg");
				FAIL;
			} else {
				for (j = 0; j < len; j++) {
					if (patternarg[j] == '?')
						continue;
					if (curname[j] != patternarg[j]) {
						report("name '%s' did not match patternarg");
						FAIL;
						break;
					}
				}
				if (j == len)
					CHECK;
			}
		}
	}
	for (i = 0; i < nfontinfo; i++) {
		if (fontinfo[i].flag == 0) {
			report("xtest font '%s' was not found", fontinfo[i].name);
			FAIL;
		} else {
			/* reset to zero for next test */
			fontinfo[i].flag = 0;
			CHECK;
		}
	}
	CHECKPASS(2+count+nfontinfo);

>>ASSERTION Good A
The number of fonts returned in the count_return argument will not
exceed maxnames.
>>STRATEGY
Set maxnames to 1
Set patternarg to "*"
Verify that only one name was returned.
>>CODE
char	**names;

	maxnames = 1;
	patternarg = "*";

	names = XCALL;

	if (count > maxnames) {
		report("returned count was %d, not expecting it to exceed %d", count, maxnames);
		FAIL;
	} else {
		PASS;
	}

>>ASSERTION Bad A
When there are no available font names that
match the pattern
argument,
then a call to xname
returns NULL.
>>STRATEGY
Set patternarg to a bad name.
Call XListFontsWithInfo.
Verify tht NULL is returned.
>>CODE
char	**names;

	patternarg = "xtfont non existant name";

	names = XCALL;

	if (names != NULL) {
		report("returned pointer was not NULL");
		FAIL;
	} else
		PASS;
>>ASSERTION Bad A
.ER BadAlloc 
>># HISTORY kieron Completed	Reformat and tidy to ca pass
