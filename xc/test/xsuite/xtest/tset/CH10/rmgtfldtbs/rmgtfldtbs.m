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
 * $XConsortium: rmgtfldtbs.m,v 1.7 94/04/17 21:10:03 rws Exp $
 */
>>TITLE XrmGetFileDatabase CH10
XrmDatabase

char *filename;
>>SET startup rmstartup
>>INCLUDE ../rmptrsrc/fn.mc
>>EXTERN
#include <stdio.h>
#define XGFD_NON_EXIST "xgfd_nonfile"
#define XGFD_TEST_FILE "xgfd_tmpfile"

#define XGFD_T1_COUNT 23

static char *t1_lines[XGFD_T1_COUNT][5] = {
/* Capital 'T' is transformed to TAB in the prefix and conjunct. */
/* prefix, resource, conjunt, value, specifier */
{"",	"a.a",	":",	"one",	"a.a"	}, /* Normal Line */

{" ",	"a.b",	":",	"two",	"a.b"	}, /* pre-resource space */
{"T",	"a.c",	":",	"three","a.c"	}, /* pre-resource tab*/
{" T ",	"a.d",	":",	"four",	"a.d"	}, /* pre-resource ws */

{"",	"a.e",	" :",	"five",	"a.e"	}, /* post-resource space */
{"",	"a.f",	"T:",	"six",	"a.f"	}, /* post-resource tab */
{"",	"a.g",	" T :",	"seven","a.g"	}, /* post-resource ws */

{"",	"a.h",	": ",	"eight","a.h"	}, /* pre-value space */
{"",	"a.i",	":T",	"nine",	"a.i"	}, /* pre-value tab */
{"",	"a.j",	": T ",	"ten",	"a.j"	}, /* pre-value ws */

{"  ",	"a.k",	"   :  ","eleven","a.k"	}, /* multiple space */
{"TT",	"a.l",	"TT:TTTT","twelve","a.l"}, /* multiple tab */
{"T  T","a.m",	" T:T   ","thirteen","a.m"}, /* multiple ws */

{"",	".a.n",	":",	"fourteen","a.n"}, /* pre-resource binding . */
{"",	"*a.o",	":",	"fifteen", "b.a.o"}, /* pre-resource binding * */

{"",	"a*p",	":",	"sixteen", "a.z.p"}, /* inclusive binding * */
{"",	".a*q",	":",	"seventeen","a.z.q"}, /* inclusive binding * */

{"",	"*a*r",	":",	"eighteen","b.a.c.r"}, /* inclusive binding * */

{"",	"b",	":",	"nineteen","b"	}, /* lone component */
{"",	".c",	":",	"twenty", "c"	}, /* lone component */
{"",	"*z",	":",	"twenty one","b.z"	}, /* lone component */

{"",	"d",	":",	"Aa9! @#$%^&*()_+~", "d"}, /* complex value */

{"T ",	"a-z.r_9A-*a-8B","T: ","A* bZ_% !?", "a-z.r_9A-.z.a-8B"} };/* complex entry */

>>ASSERTION Good A
A call to xname opens the specified
.A filename ,
creates a new resource database, loads the database with the
resource specifications from the file in ResourceLine 
format, and returns the database.
>>STRATEGY
Create a test database file.
Write test lines to test database file.
Call xname to read the database.
Verify the database was non-NULL, and contained the test information.
Remove test database file.
>>CODE
FILE *f;
int	a,b;
char tbuf[256], *tptr;
XrmDatabase rdb;

/* Create a test database file. */
	f = fopen(XGFD_TEST_FILE, "w");
	if (f==(FILE *)NULL) {
		delete("fopen() call to write database failed");
		return;
	} else
		CHECK;

/* Write test lines to test database file. */
	for(a=0; a<XGFD_T1_COUNT; a++) {
		tptr=tbuf;
		for(b=0; b<4; b++) {
			tptr += xrm_tabulate(t1_lines[a][b], tptr);
			CHECK;
		}
		*tptr = '\0';
		(void) fprintf(f, "%s\n", tbuf);
	}
	fclose(f);

/* Call xname to read the database. */
	filename = XGFD_TEST_FILE;
	rdb = XCALL;

/* Verify the database was non-NULL, and contained the test information. */
	if (rdb == (XrmDatabase)NULL) {
		FAIL;
		report("%s returned a NULL database when expecting a database.",
			TestName);
	} else {
		for(a=0; a<XGFD_T1_COUNT; a++) {
			tptr=tbuf;
			tbuf[ xrm_tabulate(t1_lines[a][3], tptr) ] = '\0';
			if(xrm_check_entry(rdb, t1_lines[a][4], t1_lines[a][4],
				"String", tbuf)) {
				FAIL;
			} else
				CHECK;
		}
	}

	CHECKPASS(1 + XGFD_T1_COUNT*4 + XGFD_T1_COUNT);

/* Remove test database file. */
#ifndef TESTING
	(void) unlink(XGFD_TEST_FILE);	/* Compiling with CFLOCAL=-DTESTING  */
					/* allows inspection of the testfile */
#endif

>>ASSERTION Good A
When
.A filename
refers to a file that cannot be opened, then a call to xname returns
.S NULL .
>>STRATEGY
Call xname with a non-existant file.
Verify that NULL was returned.
>>CODE
XrmDatabase ret;

/* Call xname with a non-existant file. */
	filename = XGFD_NON_EXIST;
	ret = XCALL;

/* Verify that NULL was returned. */
	if( ret != (XrmDatabase)NULL) {
		FAIL;
		report("%s returned non-NULL with a non-existant filename.",
			TestName);
	} else
		CHECK;

	CHECKPASS(1);
