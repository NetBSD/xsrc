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
 * $XConsortium: rmstrtqrkl.m,v 1.5 94/04/17 21:10:26 rws Exp $
 */
>>TITLE XrmStringToQuarkList CH10
void

char *string;
XrmQuarkList quarks_return = (XrmQuarkList)xstql_ret;
>>SET startup rmstartup
>>EXTERN
#define XSTQL_MAX_RETURN 10
static XrmQuark xstql_ret[XSTQL_MAX_RETURN];

static char *xstql_s1[]={
	"xstql_test",
	"ena",
	"deo",
	"tria"};

static char *xstql_s1sep[]={
	".",
	".",
	".",
	""};

static int xstql_ns1=NELEM(xstql_s1);

static char *xstql_s2[]={
	"XStql_test",
	"une",
	"deux",
	"trois",
	"erm",
	"five"};
static char *xstql_s2sep[]={
	"*",
	".",
	"*",
	"*",
	".",
	""};
static int xstql_ns2=NELEM(xstql_s2);

static char *xstql_s3[]={
	"XStqL_test.",
	"jack.",
	"and.",
	"jill"};
static int xstql_ns3=NELEM(xstql_s3);

static void
xstqlclear(value)
XrmQuark value;
{
	int i;

/* Clear the return array, to avoid rogue results. */
	for(i=0; i<XSTQL_MAX_RETURN; i++) 
		xstql_ret[i]=value;

}

>>ASSERTION Good A
When
.A string
is a valid null-terminated resource name format string, then a call to xname
returns a quark list in
.A quarks_return
corresponding to the components of
.A string .
>>STRATEGY
Create the test string.
Call xname to obtain quark list.
Verify that the quark list returned was as expected.
>>CODE
int  t;
char *buffer1;
char *buffer2;

	buffer1 = (char *)malloc( (size_t)256);
	if (buffer1==(char *)NULL) {
		delete("Could not malloc 256 bytes for buffer1.");
		return;
	} else
		CHECK;

	buffer2 = (char *)malloc( (size_t)4096);
	if (buffer2==(char *)NULL) {
		delete("Could not malloc 4096 bytes for buffer2.");
		return;
	} else
		CHECK;

	*buffer1='\0'; /* Make the buffers empty strings. */
	*buffer2='\0';

/* Create the test string. */
	for(t=0; t<xstql_ns1; t++) {
		(void) strcat(buffer1, xstql_s1[t]);
		(void) strcat(buffer1, xstql_s1sep[t]);
	}
	trace("Testing with '%s'", buffer1);

	xstqlclear((XrmQuark)0);

/* Call xname to obtain quark list. */
	string= buffer1;
	XCALL;

#ifdef TESTING
	xstql_ret[0]=0;
	xstql_ret[1]=XrmStringToQuark("TESTING");
#endif

/* Verify that the quark list returned was as expected. */
	for(t=0; t<xstql_ns1; t++) {
		char *ts;

		ts = XrmQuarkToString( xstql_ret[t] );
		if (ts == (char *)NULL) {
			FAIL;
			report("Quark array[%d]=%d. Represents: NO STRING (expecting '%s')",
				t, xstql_ret[t], xstql_s1[t]);
			strcat(buffer2,"<NO STRING>");
		} else
		{
			if(strcmp(xstql_s1[t], ts)) {
				FAIL;
				report("Quark array[%d]=%d. Represents: '%s' (expecting '%s')",
					t, xstql_ret[t], ts, xstql_s1[t]);
			} else {
				CHECK;
				trace("Quark array[%d]=%d. Represents: '%s' as expected.",
					t, xstql_ret[t], ts );
			}
			(void) strcat(buffer2, ts);
		}
		(void) strcat(buffer2, " ");
	}

	if (fail) {
		report("%s did not split the string into the quarks as expected",
			TestName);
		report("String passed to %s: '%s'", TestName, buffer1);
		report("Quark array represents: %s", buffer2);
	} else
		CHECK;

	CHECKPASS(2+xstql_ns1+1);
	free(buffer1);
	free(buffer2);

>>ASSERTION Good A
On a call to xname, the
.A string
is separated into components according to the
positions of periods and asterisks.
>>STRATEGY
Create the test string.
Call xname to obtain quark list.
Verify that the quark list returned was as expected.
>>CODE
int  t;
char *buffer1;
char *buffer2;

	buffer1 = (char *)malloc( (size_t)256);
	if (buffer1==(char *)NULL) {
		delete("Could not malloc 256 bytes for buffer1.");
		return;
	} else
		CHECK;

	buffer2 = (char *)malloc( (size_t)4096);
	if (buffer2==(char *)NULL) {
		delete("Could not malloc 4096 bytes for buffer2.");
		return;
	} else
		CHECK;

	*buffer1='\0'; /* Make the buffers empty strings. */
	*buffer2='\0';

/* Create the test string. */
	for(t=0; t<xstql_ns2; t++) {
		(void) strcat(buffer1, xstql_s2[t]);
		(void) strcat(buffer1, xstql_s2sep[t]);
	}
	trace("Testing with '%s'", buffer1);

	xstqlclear((XrmQuark)0);

/* Call xname to obtain quark list. */
	string= buffer1;
	XCALL;

#ifdef TESTING
	xstql_ret[0]=0;
	xstql_ret[1]=XrmStringToQuark("TESTING");
#endif

/* Verify that the quark list returned was as expected. */
	for(t=0; t<xstql_ns2; t++) {
		char *ts;

		ts = XrmQuarkToString( xstql_ret[t] );
		if (ts == (char *)NULL) {
			FAIL;
			report("Quark array[%d]=%d. Represents: NO STRING (expecting '%s')",
				t, xstql_ret[t], xstql_s2[t]);
			strcat(buffer2,"<NO STRING>");
		} else
		{
			if(strcmp(xstql_s2[t], ts)) {
				FAIL;
				report("Quark array[%d]=%d. Represents: '%s' (expecting '%s')",
					t, xstql_ret[t], ts, xstql_s2[t]);
			} else {
				CHECK;
				trace("Quark array[%d]=%d. Represents: '%s' as expected.",
					t, xstql_ret[t], ts );
			}
			(void) strcat(buffer2, ts);
		}
		(void) strcat(buffer2, " ");
	}

	if (fail) {
		report("%s did not split the string into the quarks as expected",
			TestName);
		report("String passed to %s: '%s'", TestName, buffer1);
		report("Quark array represents: '%s'", buffer2);
	} else
		CHECK;

	CHECKPASS(2+xstql_ns2+1);
	free(buffer1);
	free(buffer2);

>>ASSERTION Good A
On a call to xname, the
.A quarks_return
list is terminated with a zero.
>>STRATEGY
Create the test string.
Set return buffer with unique quark value.
Call xname to obtain quark list.
Verify that the quark list was zero terminated.
>>CODE
char *buffer1;
XrmQuark unq;
int t;

	buffer1 = (char *)malloc( (size_t)256);
	if (buffer1==(char *)NULL) {
		delete("Could not malloc 256 bytes for buffer1.");
		return;
	} else
		CHECK;

	*buffer1='\0';

/* Create the test string. */
	for(t=0; t<xstql_ns3; t++) {
		(void) strcat(buffer1, xstql_s3[t]);
	}
	trace("Testing with '%s'", buffer1);

/* Set return buffer with unique quark value. */
	unq=XrmUniqueQuark();
	xstqlclear(unq);

/* Call xname to obtain quark list. */
	string= buffer1;
	XCALL;

/* Verify that the quark list was zero terminated. */
	t=0;
	while(t<XSTQL_MAX_RETURN
		&& xstql_ret[t] != unq
		&& xstql_ret[t] != (XrmQuark)0) {
		t++;
	}

	if (t==XSTQL_MAX_RETURN) {
		int i;
		FAIL;
		report("%s did not return the quark array as expected.",
			TestName);
		report("At least %d quarks returned.", XSTQL_MAX_RETURN);
		for (i=0; i<XSTQL_MAX_RETURN; i++)
			report("Quark array[%d]=%d", i, xstql_ret[i]);
	} else {
		if (xstql_ret[t] == unq) {
			FAIL;
			report("%s did not terminate the array with a zero.",
				TestName);
		} else
			CHECK;
	}

	CHECKPASS(2);
	free(buffer1);
