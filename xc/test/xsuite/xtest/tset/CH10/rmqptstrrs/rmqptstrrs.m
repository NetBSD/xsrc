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
 * $XConsortium: rmqptstrrs.m,v 1.8 94/04/17 21:10:22 rws Exp $
 */
>>TITLE XrmQPutStringResource CH10
void

XrmDatabase *database = &xqpsr_database;
XrmBindingList bindings = xqpsr_bindings;
XrmQuarkList quarks = xqpsr_quarks;
char *value;
>>SET startup rmstartup
>>EXTERN
static XrmDatabase xqpsr_database;
static XrmBinding xqpsr_bindings[10];
static XrmQuark xqpsr_quarks[10];

>>INCLUDE ../rmptrsrc/common.mc
>>ASSERTION Good A
A call to xname adds a resource
specified by
.A bindings
and
.A quarks
with the specified
.A value
to the specified
.A database
with a
.M String
representation type.
>>STRATEGY
Create an empty test database.
Call xname to add database entries.
Call XrmGetResource to verify the database entries were added.
>>CODE
int i,j;

/* Create an empty test database. */
	xqpsr_database = xrm_create_database("");
	if (xqpsr_database = (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

/* Call xname to add database entries. */
/* Call XrmGetResource to verify the database entries were added. */
	for(i=0; i<XRM_T1_TESTS; i++) {
		for(j=0; qt1_specifiers[i][j] != (char *)NULL; j++) {
			xqpsr_quarks[j]=XrmStringToQuark(qt1_specifiers[i][j]);
			xqpsr_bindings[j]=qt1_bindings[i][j];
		}
		xqpsr_quarks[j]=(XrmQuark)0;
		value= t1_values[i];
		XCALL;
		if(xrm_check_entry(xqpsr_database,
			t1_fspecs[i], t1_fclasses[i],
			"String" , t1_values[i])) {
			FAIL;
		} else
			CHECK;
	}

#ifdef TESTING
	XrmPutFileDatabase(xqpsr_database, "xqpsr_one");
#endif

	CHECKPASS(1+XRM_T1_TESTS);

	XrmDestroyDatabase(xqpsr_database);

>>ASSERTION Good A
When the 
.A database
contains an entry for the resource name specified by
.A bindings
and
.A quarks ,
then a call to xname replaces the resource value in the
.A database
with
.A value ,
and the resource type 
with
.M String .
>>STRATEGY
Create an empty test database.
Call xname to add a database entry.
Call xname to replace a database entry.
Verify the database entry was updated as expected.
>>CODE
	int i,j;

/* Create an empty test database. */
	xqpsr_database = xrm_create_database("");
	if (xqpsr_database = (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

/* Call xname to add a database entry. */
/* Call xname to replace a database entry. */
	for(i=0; i<2; i++) {
		for(j=0; qt2_specifier[j] != (char *)NULL; j++) {
			xqpsr_quarks[j]=XrmStringToQuark(qt2_specifier[j]);
			xqpsr_bindings[j]=qt2_bindings[j];
		}
		xqpsr_quarks[j]=(XrmQuark)0;
		value=t2_values[i];
		XCALL;
		CHECK;
	}

/* Verify the database entry was updated as expected. */
	if(xrm_check_entry(xqpsr_database,
		t2_fullspec, t2_fullclass,
		"String", t2_values[1])) {
			FAIL;
			report("%s did not update the database contents as expected.",
				TestName);
		} else
			CHECK;

	CHECKPASS(4);

#ifdef TESTING
	XrmPutFileDatabase(xqpsr_database, "xqpsr_two");
#endif

	XrmDestroyDatabase(xqpsr_database);

>>ASSERTION Good A
When
.A database
is NULL, then a call to xname
creates a new database,
adds a resource
specified by
.A bindings
and
.A quarks
with the specified
.A value
to the database
with a
.M String
representation type, and returns a pointer to the database in
.A database .
>>STRATEGY
Call xname to add data to a NULL database.
Verify that the database was created, and the data was added as expected.
>>CODE
int j;

/* Call xname to add data to a NULL database. */
	xqpsr_database = (XrmDatabase)NULL;
	for(j=0; qt2_specifier[j] != (char *)NULL; j++) {
		xqpsr_quarks[j]=XrmStringToQuark(qt2_specifier[j]);
		xqpsr_bindings[j]=qt2_bindings[j];
	}
	xqpsr_quarks[j]=(XrmQuark)0;
	value=t2_values[0];
	XCALL;

/* Verify that the database was created, and the data was added as expected. */
	if (xqpsr_database== (XrmDatabase)NULL) {
		FAIL;
		report("%s did not create a new database when called with",
			TestName);
		report("*database=(XrmDatabase)NULL");
	} else {
		CHECK;
		if(xrm_check_entry(xqpsr_database,
			t2_fullspec, t2_fullclass,
			"String", t2_values[0])) {
				FAIL;
				report("%s did not add to the database as expected.",
					TestName);
			} else
				CHECK;
	}

	CHECKPASS(2);

#ifdef TESTING
	XrmPutFileDatabase(xqpsr_database, "xqpsr_three");
#endif

	XrmDestroyDatabase(xqpsr_database);
