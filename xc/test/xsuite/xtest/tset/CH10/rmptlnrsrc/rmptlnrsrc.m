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
 * $XConsortium: rmptlnrsrc.m,v 1.9 94/04/17 21:10:10 rws Exp $
 */
>>TITLE XrmPutLineResource CH10
void

XrmDatabase *database = &xplr_database;
char *line;
>>SET startup rmstartup
>>EXTERN
static XrmDatabase xplr_database;

#define XPLR_T1_COUNT 23

static char *t1_lines[XPLR_T1_COUNT][5] = {
/* Capital 'T' is transformed to TAB in the prefix and conjunct. */
/* prefix, resource, conjunt, value, specifier, newvalue */
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

>>INCLUDE ../rmptrsrc/fn.mc
>>ASSERTION Good A
When 
.A line
is in 
ResourceLine format,
then a call to xname adds a resource name and value pair from the specified
.A line
to the specified
.A database .
>>STRATEGY
Create empty database for testing.
Call xname to add test lines to the database.
Verify the test lines were added to the database as expected.
Call xname to test comments.
Verify that comments did not affect the database contents.
>>CODE
int i,j,k;
char tbuf[256], *tptr;

/* Create empty database for testing. */
	xplr_database=xrm_create_database("");
	if (xplr_database = (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

/* Call xname to add test lines to the database. */
	for(i=0; i<XPLR_T1_COUNT; i++) {

		tptr=tbuf;
		for(j=0; j<4; j++) {
			tptr += xrm_tabulate(t1_lines[i][j], tptr);
			CHECK;
		}
		*tptr = '\0';
		trace("line: %s", tbuf);

		line=tbuf;
		XCALL;
	}
		
/* Verify the test lines were added to the database as expected. */
	for(i=0; i<XPLR_T1_COUNT; i++) {
		tptr=tbuf;
		k = xrm_tabulate(t1_lines[i][3], tptr);
		tbuf[k] = '\0';
		if(xrm_check_entry(xplr_database,
			t1_lines[i][4], t1_lines[i][4],
			"String", tbuf)) {
			FAIL;
		} else
			CHECK;
	}

/* Call xname to test comments. */
	tbuf[0]= '!';
	tptr=tbuf+1;
	tptr += xrm_tabulate(t1_lines[1][1], tptr);
	tptr += xrm_tabulate(":COMMENTS EFFECT DATABASE", tptr);
	*tptr = '\0';

	line=tbuf;
	XCALL;
		
/* Verify that comments did not affect the database contents. */
	if(xrm_check_entry(xplr_database,
		t1_lines[1][4], t1_lines[1][4],
		"String", t1_lines[1][3])) {
		FAIL;
		report("On a call to %s, resources within comments affect",
			TestName);
		report("the resource database.");
	} else
		CHECK;

	CHECKPASS(1 + 4*XPLR_T1_COUNT + XPLR_T1_COUNT + 1);

#ifdef TESTING
	XrmPutFileDatabase(xplr_database, "xplr_one");
#endif

	XrmDestroyDatabase(xplr_database);

>>ASSERTION Good A
When the 
.A database
contains an entry for the resource name specified by
.A line ,
then a call to xname replaces the resource value in the
.A database
with the resource value associated with
.A line .
>>STRATEGY
Create empty database for testing.
Call xname to add test lines to the database.
Call xname to replace the database entries.
Verify the database entries were replaced as expected.
>>CODE
int i,j,k;
char tbuf[256], *tptr;

/* Create empty database for testing. */
	xplr_database=xrm_create_database("");
	if (xplr_database = (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

/* Call xname to add test lines to the database. */
	for(i=0; i<XPLR_T1_COUNT; i++) {
		tptr=tbuf;
		for(j=0; j<3; j++) {
			tptr += xrm_tabulate(t1_lines[i][j], tptr);
			CHECK;
		}
		tptr += xrm_tabulate("testvalue", tptr);
		*tptr = '\0';

		line=tbuf;
		XCALL;
	}

/* Call xname to replace the database entries. */
	for(i=0; i<XPLR_T1_COUNT; i++) {
		tptr=tbuf;
		for(j=0; j<4; j++) {
			tptr += xrm_tabulate(t1_lines[i][j], tptr);
			CHECK;
		}
		*tptr = '\0';

		line=tbuf;
		XCALL;
	}

/* Verify the database entries were replaced as expected. */
	for(i=0; i<XPLR_T1_COUNT; i++) {
		tptr=tbuf;
		k = xrm_tabulate(t1_lines[i][3], tptr);
		tbuf[k] = '\0';
		if(xrm_check_entry(xplr_database,
			t1_lines[i][4], t1_lines[i][4],
			"String", tbuf)) {
			FAIL;
		} else
			CHECK;
	}

	CHECKPASS( 1 + 3*XPLR_T1_COUNT + 4*XPLR_T1_COUNT + XPLR_T1_COUNT );

#ifdef TESTING
	XrmPutFileDatabase(xplr_database, "xplr_two");
#endif

	XrmDestroyDatabase(xplr_database);

>>ASSERTION Good A
When 
.A line
is in 
ResourceLine format and 
.A database
is NULL, 
then a call to xname
creates a new database, adds a resource name and value pair from the specified
.A line
to the database, and
returns a pointer to the database in
.A database .
>>STRATEGY
Call xname to add data to a NULL database.
Verify that the database was created, and the data was added as expected.
>>CODE

/* Call xname to add data to a NULL database. */
	xplr_database = (XrmDatabase)NULL;
	line="one.two.three:testing";
	XCALL;
/* Verify that the database was created, and the data was added as expected. */
	if (xplr_database== (XrmDatabase)NULL) {
		FAIL;
		report("%s did not create a new database when called with",
			TestName);
		report("*database=(XrmDatabase)NULL");
	} else {
		CHECK;
		if (xrm_check_entry(xplr_database,
			"one.two.three", "ONE.TWO.THREE",
			"String", "testing")) {
			FAIL;
			report("%s did not add to the database as expected.",
				TestName);
		} else
			CHECK;
	}
	
	CHECKPASS(2);

#ifdef TESTING
	XrmPutFileDatabase(xplr_database, "xplr_three");
#endif

	XrmDestroyDatabase(xplr_database);

>>ASSERTION Good A
>># This has been thoroughly covered by test one, however we perform a 
>># further  simple test.
On a call to xname, any white space before or after the name
or colon in the
.A line
argument is ignored.
>>STRATEGY
Create empty database for testing.
Call xname to add an entry with white space around the name and colon.
Verify that the white space had no effect upon the entry.
>>CODE
char tbuf[256];

/* Create empty database for testing. */
	xplr_database=xrm_create_database("");
	if (xplr_database = (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

/* Call xname to add an entry with white space around the name and colon. */
	tbuf[xrm_tabulate("T a.b.c T:  Ttest", tbuf)] = '\0';
	line=tbuf;
	XCALL;

/* Verify that the white space had no effect upon the entry. */
	if (xrm_check_entry(xplr_database,
		"a.b.c", "a.b.c",
		"String", "test")) {
		FAIL;
		report("%s did not add to the database as expected.",
			TestName);
		report("White space effects database entries");
	} else
		CHECK;

	CHECKPASS(2);

#ifdef TESTING
	XrmPutFileDatabase(xplr_database, "xplr_four");
#endif

	XrmDestroyDatabase(xplr_database);
	
>>ASSERTION Good A
On a call to xname, the processing of
.A line
is terminated at a new-line or an ASCII nul character.
>>STRATEGY
Create empty database for testing.
Create a test string containing a new-line character.
Call xname to add the data to the database.
Verify that only a single entry was added to the database.
Create a test string containing a nul character.
Call xname to add the data to the database.
Verify that only a single entry was added to the database.
>>CODE
char tbuf[256], *tptr;
char *type_ret;
XrmValue value_ret;
int k;

/* Create empty database for testing. */
	xplr_database=xrm_create_database("");
	if (xplr_database = (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

/* Create a test string containing a new-line character. */
	tptr = tbuf;
	tptr += xrm_tabulate("one:test", tbuf);
	*tptr++ = '\n';
	tptr += xrm_tabulate("two:another test", tptr);
	*tptr='\0';

/* Call xname to add the data to the database. */
	line = tbuf;
	trace("test string: '%s'", line);
	XCALL;

/* Verify that only a single entry was added to the database. */
	if (xrm_check_entry(xplr_database,
		"one", "one", "String", "test")) {
		FAIL;
	} else
		CHECK;

	if (XrmGetResource(xplr_database, "two", "two",
		&type_ret, &value_ret)) {
		FAIL;
		report("Unexpected entry indicates that a newline");
		report("does not terminate line processing for %s",
			TestName);
		trace("'two:%s' was unexpectedly present in the database.",
			(value_ret.addr!=(char *)NULL?
			(char *)value_ret.addr:"<NULL_POINTER"));
	} else
		CHECK;

/* Create a test string containing a nul character. */
	tptr = tbuf;
	tptr += xrm_tabulate("three:test", tbuf);
	*tptr++ = '\0';
	tptr += xrm_tabulate("four:another test", tptr);
	*tptr='\0';

/* Call xname to add the data to the database. */
	line = tbuf;
	trace("test string: '%s'", line);
	XCALL;

/* Verify that only a single entry was added to the database. */
	if (xrm_check_entry(xplr_database,
		"three", "three", "String", "test")) {
		FAIL;
	} else
		CHECK;

	if (XrmGetResource(xplr_database, "four", "four",
		&type_ret, &value_ret)) {
		FAIL;
		report("Unexpected entry indicates that an ASCII nul ");
		report("does not terminate line processing for %s",
			TestName);
	} else
		CHECK;

	CHECKPASS(5);

#ifdef TESTING
	XrmPutFileDatabase(xplr_database, "xplr_five");
#endif

	XrmDestroyDatabase(xplr_database);
	
>>ASSERTION Good A
When the
.A line
argument contains
.M \\\\n ,
then on a call to xname this is interpreted as the new-line character.
>>STRATEGY
Create empty database for testing.
Call xname to add an entry including a \n in the value.
Verify that the \n was treated as an embedded newline.
>>CODE
char tbuf[256];
char tstr[4];

/* Create empty database for testing. */
	xplr_database=xrm_create_database("");
	if (xplr_database = (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

/* Call xname to add an entry including a \n in the value. */
	tbuf[xrm_tabulate("one.two.three:t\\n!", tbuf)] = '\0';
	line=tbuf;
	trace("test string: '%s'", line);
	XCALL;

/* Verify that the \n was treated as an embedded newline. */
	tstr[0]='t';
	tstr[1]='\n';
	tstr[2]='!';
	tstr[3]='\0';
	if (xrm_check_entry(xplr_database,
		"one.two.three", "One.Two.Three",
		"String", tstr)) {
		FAIL;
		report("%s did not add to the database as expected.",
			TestName);
		report("\\n was not treated as an embedded character.");
		report("(whitespace in values represents \\n)");
	} else
		CHECK;

	CHECKPASS(2);

#ifdef TESTING
	XrmPutFileDatabase(xplr_database, "xplr_six");
#endif

	XrmDestroyDatabase(xplr_database);
	
>>ASSERTION Good A
When the 
.A line
argument contains
.M \\\\nnn
where n is a digit from zero to seven, then on a call to xname this is
interpreted as a single byte containing the corresponding octal character.
>>STRATEGY
Create empty database for testing.
Call xname to add an entry including a \100 in the value.
Verify that the \100 was treated as a character .
>>CODE
char tbuf[256];
char tstr[4];

/* Create empty database for testing. */
	xplr_database=xrm_create_database("");
	if (xplr_database = (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

/* Call xname to add an entry including a \100 in the value. */
	tbuf[xrm_tabulate("one.two.three:t\\100!", tbuf)] = '\0';
	line=tbuf;
	trace("test string: '%s'", line);
	XCALL;

/* Verify that the \100 was treated as a character . */
	tstr[0]='t';
	tstr[1]=(char)0x40;
	tstr[2]='!';
	tstr[3]='\0';
	if (xrm_check_entry(xplr_database,
		"one.two.three", "One.Two.Three",
		"String", tstr)) {
		FAIL;
		report("%s did not add to the database as expected.",
			TestName);
		report("\\nnn was not treated as an embedded character.");
	} else
		CHECK;

	CHECKPASS(2);

#ifdef TESTING
	XrmPutFileDatabase(xplr_database, "xplr_seven");
#endif

	XrmDestroyDatabase(xplr_database);
