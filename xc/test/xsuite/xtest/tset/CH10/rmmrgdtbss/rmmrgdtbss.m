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
 * $XConsortium: rmmrgdtbss.m,v 1.5 94/04/17 21:10:07 rws Exp $
 */
>>TITLE XrmMergeDatabases CH10
void

XrmDatabase source_db;
XrmDatabase *target_db = &target;
>>SET startup rmstartup
>>INCLUDE ../rmptrsrc/fn.mc
>>EXTERN
static XrmDatabase target;

#define XMD_SPEC 0
#define XMD_TYPE 1
#define XMD_VALUE 2
#define XMD_FSPEC 3
#define XMD_FCLASS 4
#define XMD_XTYPE 5
#define XMD_XVALUE 6

/* Test database data */
#define XMD_SIZE 3
static char	*d1_data[XMD_SIZE][5] = {
	/* spec, type, value, fullspec, fullclass */
	{ "one.data", "String", "one", "one.data", "One.Data" },
	{ "one.Misc", "Nylon",  "two", "one.misc", "One.Misc" },
	{ "one*type", "Cotton", "three", "one.type", "One.Type" } };

static char	*d2_data[XMD_SIZE][5] = {
	/* spec, type, value, fullspec, fullclass */
	{ "two.data", "Thread", "four", "two.data", "Two.Data" },
	{ "Two.star", "Lycra",  "five", "two.star", "Two.Star" },
	{ "two*Halt", "Silk",   "six", "two.halt", "Two.Halt" } };

static char *d3_data[XMD_SIZE][7] = {
	/* spec, type, value, fullspec, fullclass, exptype, expvalue */
/* These should replace test data in the target */
	{ "one.data", "Thread", "seven", "one.data", "One.Data", "Thread", "seven" },
	{ "one.Misc", "Lycra",  "eight", "one.misc", "One.Misc", "Lycra",  "eight" },
/* These is new data from the source. */
	{ "two.odd",	"Silk", 	"nine",	"two.odd",	"Two.Odd",	"Silk", "nine" } };

/* This defines from which point in d1_data entries the data is expected to
	remain in the target after the merge in test 3. */
#define XMD_UNREPLACED 2

>>ASSERTION Good A
A call to xname
merges the contents of
.A source_db
into
.A target_db .
>>STRATEGY
Create two test databases.
Add test data to the test databases.
Call xname to merge the databases.
Verify that the target database contains all the test data.
>>CODE
int a;
XrmValue val;

/* Create two test databases. */
	source_db = xrm_create_database("");
	if(source_db == (XrmDatabase)NULL) {
		delete("Could not create source database.");
		return;
	} else
		CHECK;

	target = xrm_create_database("");
	if(target == (XrmDatabase)NULL) {
		delete("Could not create target database.");
		return;
	} else
		CHECK;

/* Add test data to the test databases. */
	for(a=0; a<XMD_SIZE; a++) {
		CHECK;
		xrm_fill_value( &val, d1_data[a][XMD_VALUE] );
		XrmPutResource(&source_db,
			d1_data[a][XMD_SPEC],
			d1_data[a][XMD_TYPE],
			&val);
		
		xrm_fill_value( &val, d2_data[a][XMD_VALUE] );
		XrmPutResource(target_db,
			d2_data[a][XMD_SPEC],
			d2_data[a][XMD_TYPE],
			&val);
	}

/* Call xname to merge the databases. */
	XCALL;

/* Verify that the target database contains all the test data. */
	for(a=0; a<XMD_SIZE; a++) {

		if (xrm_check_entry(target,
			d1_data[a][XMD_FSPEC],
			d1_data[a][XMD_FCLASS],
			d1_data[a][XMD_TYPE],
			d1_data[a][XMD_VALUE])) {
			FAIL;
			report("%s did not merge in the source database correctly",
				TestName);
		} else
			CHECK;

		if (xrm_check_entry(target,
			d2_data[a][XMD_FSPEC],
			d2_data[a][XMD_FCLASS],
			d2_data[a][XMD_TYPE],
			d2_data[a][XMD_VALUE])) {
			FAIL;
			report("%s did not preserve the target database correctly",
				TestName);
		} else
			CHECK;
	}

	CHECKPASS(2 + XMD_SIZE + XMD_SIZE*2);

	XrmDestroyDatabase(target);
	
>>ASSERTION Good B 1
On a call to xname, the source database
.A source_db
is destroyed.
>>#
>># There is no portable way to verify that a resource database has been
>># destroyed.
>>#
>>ASSERTION Good A
When a resource is a member of both
.A target_db
and
.A source_db ,
then the value and type of the resource in
.A target_db
is overwritten with the corresponding value and type from the resource in
.A source_db .
>>STRATEGY
Create two test databases.
Add test data to the test databases.
Call xname to merge the databases.
Verify that the overlapping source database entries replaced the
	corresponding target database entries, and that unique source
	database entries were merged correctly.
Verify that unique target database entries remained following the merge.
>>CODE
int a;
XrmValue val;

/* Create two test databases. */
	source_db = xrm_create_database("");
	if(source_db == (XrmDatabase)NULL) {
		delete("Could not create source database.");
		return;
	} else
		CHECK;

	target = xrm_create_database("");
	if(target == (XrmDatabase)NULL) {
		delete("Could not create target database.");
		return;
	} else
		CHECK;

/* Add test data to the test databases. */
	for(a=0; a<XMD_SIZE; a++) {
		CHECK;
		
		xrm_fill_value( &val, d3_data[a][XMD_VALUE] );
		XrmPutResource(&source_db,
			d3_data[a][XMD_SPEC],
			d3_data[a][XMD_TYPE],
			&val);

		xrm_fill_value( &val, d1_data[a][XMD_VALUE] );
		XrmPutResource(target_db,
			d1_data[a][XMD_SPEC],
			d1_data[a][XMD_TYPE],
			&val);
	}

/* Call xname to merge the databases. */
	XCALL;

/* Verify that the overlapping source database entries replaced the */
/* 	corresponding target database entries, and that unique source */
/* 	database entries were merged correctly. */
	for(a=0; a<XMD_SIZE; a++) {
		if (xrm_check_entry(target,
			d3_data[a][XMD_FSPEC],
			d3_data[a][XMD_FCLASS],
			d3_data[a][XMD_XTYPE],
			d3_data[a][XMD_XVALUE])) {
			FAIL;
			report("%s did not merge the databases correctly",
				TestName);
		} else
			CHECK;
	}

/* Verify that unique target database entries remained following the merge. */
	for(a=XMD_UNREPLACED; a<XMD_SIZE; a++) {
		if (xrm_check_entry(target,
			d1_data[a][XMD_FSPEC],
			d1_data[a][XMD_FCLASS],
			d1_data[a][XMD_TYPE],
			d1_data[a][XMD_VALUE])) {
			FAIL;
			report("%s did not preserve the target database correctly",
				TestName);
		} else
			CHECK;
	}

	CHECKPASS(2 + XMD_SIZE + XMD_SIZE*2 - XMD_UNREPLACED) ;

	XrmDestroyDatabase(target);
	
