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
 * $XConsortium: rmprscmmnd.m,v 1.8 94/04/17 21:10:08 rws Exp $
 */
>>TITLE XrmParseCommand CH10
void

XrmDatabase *database = &xpc_db;
XrmOptionDescList table = xpc_table;
int table_count = XPC_TESTS;
char *name = "xtest";
int *argc_in_out = &xpc_argc;
char **argv_in_out = xpc_argv;
>>SET startup rmstartup
>>INCLUDE ../rmptrsrc/fn.mc
>>EXTERN
#define XPC_TESTS	12
#define XPC_MAXDBE	2
#define XPC_MAXAVE	6

/* XPC_DB_MODE means that failed database matches cause a failure, */
/* instead of deletion. */
#define XPC_DB_MODE	1

/* XPC_ARGC_MODE means that incorrect argc values returned cause a failure, */
/* instead of deletion. */
#define XPC_ARGC_MODE	2

/* XPC_NOREPORT_MODE suppresses the CHECKPASS call at the end of xpc_dotest. */
#define	XPC_NOREPORT_MODE	4

static XrmDatabase xpc_db;
static int xpc_argc;
static char *xpc_argv[XPC_MAXAVE];

/* Initial database contents */
static char *xpcdb_init[XPC_TESTS][XPC_MAXDBE][2] = {
	{ {NULL,NULL},	{NULL,NULL} },
	{ {NULL,NULL},	{NULL,NULL} },
	{ {NULL,NULL},	{NULL,NULL} },
	{ {NULL,NULL},	{NULL,NULL} },
	{ {"xtest.t5","i5"},	{NULL,NULL} },
	{ {"xtest.t6","i6"},	{"xtest.t1","i1"} },
	{ {"xtest.t7","i7"},	{"xtest.t1","i1"} },
	{ {"xtest.t1","i1"},	{"xtest.t2","i2"} },
	{ {"xtest.test","i9"},	{NULL,NULL} },
	{ {NULL,NULL},	{NULL,NULL} },
	{ {"xtest.tA","iA"},	{"xtest.tB","iB"} },
	{ {"xtest.tC","iC"},	{NULL,NULL} } };

/* Expected final database contents. */
static char *xpcdb_final[XPC_TESTS][XPC_MAXDBE][2] = {
	{ {"xtest.t1","v1"},	{NULL,NULL} },
	{ {"xtest.t2","bb"},	{NULL,NULL} },
	{ {"xtest.t3","a3"},	{NULL,NULL} },
	{ {"xtest.t4","a4"},	{NULL,NULL}},
	{ {"xtest.t5","i5"},	{"t","a5"} },
	{ {"xtest.t6","i6"},	{"xtest.t1","i1"} },
	{ {"xtest.t7","i7"},	{"xtest.t1","i1"} },
	{ {"xtest.t1","i1"},	{"xtest.t2","bb"} },
	{ {"xtesttest","v9"},	{"xtest.test","i9"} },
	{ {"xtest.tA","vA"},	{NULL,NULL} },
	{ {"xtest.tA","iA"},	{"xtest.tB","iB"} },
	{ {"xtest.tC","iC"},	{NULL,NULL} } };

/* Initial argc value. */
static int xpcac_init[XPC_TESTS] = {
	2, 2, 2, 3, 3, 3, 4, 5, 2, 2, 2, 2 };

/* Expected return argc value. */
static int xpcac_final[XPC_TESTS] = {
	1, 1, 1, 1, 1, 3, 4, 4, 1, 1, 2, 2 };

/* Initial argv vector contents */
static char *xpcav_init[XPC_TESTS][XPC_MAXAVE] = {
	{	"PROGNAME",	"aa",	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	"bb",	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	"cca3",	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	"dd",	"a4",	NULL,	NULL,	NULL },
	{	"PROGNAME",	"ee",	"t:a5",	NULL,	NULL,	NULL },
	{	"PROGNAME",	"ff",	"aa",	NULL,	NULL,	NULL },
	{	"PROGNAME",	"gg",	"left",	"aa",	NULL,	NULL },
	{	"PROGNAME",	"hh",	"left",	"aa",	"bb",	NULL },
	{	"PROGNAME",	"ii",	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	"jj",	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	"j",	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	"LL",	NULL,	NULL,	NULL,	NULL } };

/* Expected argv vector contents */
static char *xpcav_final[XPC_TESTS][XPC_MAXAVE] = {
	{	"PROGNAME",	NULL,	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	NULL,	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	NULL,	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	NULL,	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	NULL,	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	"ff",	"aa",	NULL,	NULL,	NULL },
	{	"PROGNAME",	"gg",	"left",	"aa",	NULL,	NULL },
	{	"PROGNAME",	"hh",	"left",	"aa",	NULL,	NULL },
	{	"PROGNAME",	NULL,	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	NULL,	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	"j",	NULL,	NULL,	NULL,	NULL },
	{	"PROGNAME",	"LL",	NULL,	NULL,	NULL,	NULL } };

/* Test XrmOptionDescRec table. */
static XrmOptionDescRec xpc_table[XPC_TESTS] = {
	{	"aa",	".t1",	XrmoptionNoArg,		(caddr_t)"v1" 	},
	{	"bb",	".t2",	XrmoptionIsArg,		(caddr_t)"v2"	},
	{	"cc",	".t3",	XrmoptionStickyArg,	(caddr_t)"v3"	},
	{	"dd",	".t4",	XrmoptionSepArg,	(caddr_t)"v4"	},
	{	"ee",	".t5",	XrmoptionResArg,	(caddr_t)"v5"	},
	{	"ff",	".t6",	XrmoptionSkipArg,	(caddr_t)"v6"	},
	{	"gg",	NULL,	XrmoptionSkipLine,	(caddr_t)NULL	},
	{	"hh",	NULL,	XrmoptionSkipNArgs,	(caddr_t)2	},
/* Adds prefix without binding test. */
	{	"ii",	"test",	XrmoptionNoArg,		(caddr_t)"v9"	},
/* Unambiguous match tests. */
	{	"jjj",	".tA",	XrmoptionNoArg,		(caddr_t)"vA"	},
	{	"jk",	".tB",	XrmoptionNoArg,		(caddr_t)"vB"	},
/* Case sensitivity test */
	{	"ll",	".tC",	XrmoptionNoArg,		(caddr_t)"vC"	} };

static int
xpc_dotest(n, mode)
int n;
int mode;
{
	int a;
	int pass=0;
	int fail=0;
/* Create new database to perform tests on. */
	xpc_db = xrm_create_database("");
	if (xpc_db == (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return(0);
	} else
		CHECK;

/* Load database with initial values, if any. */
	for(a=0; a<XPC_MAXDBE; a++) {
		CHECK;
		if (xpcdb_init[n][a][0] != NULL) {
			XrmPutStringResource(database,
				xpcdb_init[n][a][0],
				xpcdb_init[n][a][1]);
		}
	}

/* Initialise test argc and argv */
	xpc_argc = xpcac_init[n];
	for(a=0; a<XPC_MAXAVE; a++) {
		CHECK;
		xpc_argv[a]=xpcav_init[n][a];
	}

/* Call xname to parse the argc,argv pair. */
	XCALL;

/* Verify that argc was updated as expected. */
	if (xpc_argc != xpcac_final[n]) {
		if(mode & XPC_ARGC_MODE) {
			FAIL;
			report("%s did not set argc_in_out as expected.", TestName);
			report("Expected argc_in_out: %d", xpcac_final[n]);
			report("Returned argc_in_out: %d", xpc_argc);
		} else {
			CHECK;
			trace("%s did not set argc_in_out as expected.", TestName);
			trace("Expected argc_in_out: %d", xpcac_final[n]);
			trace("Returned argc_in_out: %d", xpc_argc);
		}
	} else
		CHECK;

/* Verify that argv was updated as expected. */
	for(a=0; a<xpcac_final[n]; a++) {
		if(xpc_argv[a] == NULL ||
			strcmp(xpc_argv[a], xpcav_final[n][a])) {
			if(mode & XPC_DB_MODE) {
				FAIL;
				report("Expected argv_in_out[%d]: %s", a,
					xpcav_final[n][a]);
				report("Returned argv_in_out[%d]: %s", a,
					xpc_argv[a]);
			} else {
				CHECK;
				trace("Expected argv_in_out[%d]: %s", a,
					xpcav_final[n][a]);
				trace("Returned argv_in_out[%d]: %s", a,
					xpc_argv[a]);
			}
		} else
			CHECK;
	}

/* Verify that the database was updated as expected. */
	for(a=0; a<XPC_MAXDBE; a++) {
		if(xpcdb_final[n][a][0]!=NULL) {
			if(xrm_check_entry(xpc_db,
				xpcdb_final[n][a][0], xpcdb_final[n][a][0],
				"String", xpcdb_final[n][a][1])) {
				if(mode & XPC_DB_MODE) {
					FAIL;
					report("%s did not update the database",
						TestName);
					report("as expected.");
				} else {
					CHECK;
					trace("%s did not update the database",
						TestName);
					trace("as expected.");
				}
			} else
				CHECK;
		} else
			CHECK;
	}

#ifdef	TESTING
	{	/* If the database contents are required for investigation,  */
		/* Compile with pmake CFLOCAL=-DTESTING to dump the database */
		char tf[20];
		sprintf(tf, "xpc_%d.%d", n, mode);
		XrmPutFileDatabase(xpc_db, tf);
		trace("Database dumped to '%s'", tf);
	}
#endif
	XrmDestroyDatabase(xpc_db);

	if (!(mode & XPC_NOREPORT_MODE)) {
		CHECKPASS(1+XPC_MAXDBE+XPC_MAXAVE+1+xpcac_final[n]+XPC_MAXDBE);
		return(0);
	} else {
		return(pass==(1+XPC_MAXDBE+XPC_MAXAVE+1+xpcac_final[n]+XPC_MAXDBE)?1:0);
	}
}

>>ASSERTION Good A
When an argument in
.A argv_in_out
matches an
.M option
within
.A table
whose
.M argKind
is
.S XrmoptionNoArg ,
then a call to xname 
stores the resource
.M specifier 
with value 
.M value
and representation type 
.M String
in the database
.A database
and removes the argument from
.A argv_in_out .
>>STRATEGY
Create new database to perform tests on.
Load database with initial values, if any.
Initialise test argc and argv
Call xname to parse the argc,argv pair.
Verify that argv was updated as expected.
Verify that the database was updated as expected.
>>CODE

	(void)xpc_dotest(0, XPC_DB_MODE);

>>ASSERTION Good A
When an argument in
.A argv_in_out
matches an
.M option
within
.A table
whose
.M argKind
is
.S XrmoptionIsArg ,
then a call to xname 
stores the resource
.M specifier 
with the value 
set to the argument
and representation type 
.M String
in the database
.A database
and removes the argument from
.A argv_in_out .
>>STRATEGY
Create new database to perform tests on.
Load database with initial values, if any.
Initialise test argc and argv
Call xname to parse the argc,argv pair.
Verify that argv was updated as expected.
Verify that the database was updated as expected.
>>CODE

	(void)xpc_dotest(1, XPC_DB_MODE);

>>ASSERTION Good A
When an argument in
.A argv_in_out
matches an
.M option
within
.A table
whose
.M argKind
is
.S XrmoptionStickyArg ,
then a call to xname stores the resource
.M specifier 
with the value set to 
the characters immediately following the argument
and representation type 
.M String
in the database
.A database
and removes the argument from
.A argv_in_out .
>>STRATEGY
Create new database to perform tests on.
Load database with initial values, if any.
Initialise test argc and argv
Call xname to parse the argc,argv pair.
Verify that argv was updated as expected.
Verify that the database was updated as expected.
>>CODE

	(void)xpc_dotest(2, XPC_DB_MODE);

>>ASSERTION Good A
When an argument in
.A argv_in_out
matches an
.M option
within
.A table
whose
.M argKind
is
.S XrmoptionSepArg ,
then a call to xname stores the resource
.M specifier 
with the value set to 
the next argument
and representation type 
.M String
in the database
.A database
and removes both arguments from
.A argv_in_out .
>>STRATEGY
Create new database to perform tests on.
Load database with initial values, if any.
Initialise test argc and argv
Call xname to parse the argc,argv pair.
Verify that argv was updated as expected.
Verify that the database was updated as expected.
>>CODE

	(void)xpc_dotest(3, XPC_DB_MODE);

>>ASSERTION Good A
When an argument in
.A argv_in_out
matches an
.M option
within
.A table
whose
.M argKind
is
.S XrmoptionResArg ,
then a call to xname stores the resource
and value specified by
the next argument
and representation type 
.M String
in the database
.A database
and removes both arguments from
.A argv_in_out .
>>STRATEGY
Create new database to perform tests on.
Load database with initial values, if any.
Initialise test argc and argv
Call xname to parse the argc,argv pair.
Verify that argv was updated as expected.
Verify that the database was updated as expected.
>>CODE

	(void)xpc_dotest(4, XPC_DB_MODE);

>>ASSERTION Good A
When an argument in
.A argv_in_out
matches an
.M option
within
.A table
whose
.M argKind
is
.S XrmoptionSkipArg ,
then a call to xname ignores the argument and the following argument.
>>STRATEGY
Create new database to perform tests on.
Load database with initial values, if any.
Initialise test argc and argv
Call xname to parse the argc,argv pair.
Verify that argv was updated as expected.
Verify that the database was updated as expected.
>>CODE

	(void)xpc_dotest(5, XPC_DB_MODE);

>>ASSERTION Good A
When an argument in
.A argv_in_out
matches an
.M option
within
.A table
whose
.M argKind
is
.S XrmoptionSkipLine ,
then a call to xname ignores the argument and the rest of
.A argv_in_out .
>>STRATEGY
Create new database to perform tests on.
Load database with initial values, if any.
Initialise test argc and argv
Call xname to parse the argc,argv pair.
Verify that argv was updated as expected.
Verify that the database was updated as expected.
>>CODE

	(void)xpc_dotest(6, XPC_DB_MODE);

>>ASSERTION Good A
When an argument in
.A argv_in_out
matches an
.M option
within
.A table
whose
.M argKind
is
.S XrmoptionSkipNArgs ,
then a call to xname ignores the argument and the following
.M value
arguments of
.A argv_in_out .
>>STRATEGY
Create new database to perform tests on.
Load database with initial values, if any.
Initialise test argc and argv
Call xname to parse the argc,argv pair.
Verify that argv was updated as expected.
Verify that the database was updated as expected.
>>CODE

	(void)xpc_dotest(7, XPC_DB_MODE);

>>ASSERTION Good A
On a call to xname, the
.A argc_in_out
argument is set to the remaining number of arguments that were not
parsed.
>>STRATEGY
Create new database to perform tests on.
Load database with initial values, if any.
Initialise test argc and argv
Call xname to parse the argc,argv pair.
Verify that argc was updated as expected.
>>CODE
int a;

	for(a=0; a<8; a++) {
		trace("testing parse table line %d",a);
		if(xpc_dotest(a, XPC_ARGC_MODE|XPC_NOREPORT_MODE)) {
			CHECK;
		} else
			FAIL;
	}

	CHECKPASS(8);

>>ASSERTION Good B 1
On a call to xname, the
.A name
is prefixed without a binding character to the
.M specifier
in the option
.A table
before storing the specification in the
.A database .
>># There has been clarification from MIT that the semantics are undefined
>># when the table member does not start with a binding.  The assertion
>># therefore does not have any interesting consequences.
>>#>># ***POTENTIAL GREY AREA***
>>#>># The specification says that no binding character is inserted, but
>>#>># it is. The specification _does_ say that each table member should
>>#>># start with a . or  * though, and I don't do this.... 
>>#>># If I did have to place a * or . at the start of the test table
>>#>># entry, then this assertion becomes untestable.
>>#>>STRATEGY
>>#Create new database to perform tests on.
>>#Load database with initial values, if any.
>>#Initialise test argc and argv
>>#Call xname to parse the argc,argv pair.
>>#Verify that argv was updated as expected.
>>#Verify that the database was updated as expected.
>>#>>CODE
>>#
>>#	(void)xpc_dotest(8, XPC_DB_MODE);
>>#
>>ASSERTION Good A
On a call to xname, any unambiguous abbreviation within
.A argv_in_out
for an
.M option
member of
.A table
is considered a match for the option.
>>STRATEGY
Create new database to perform tests on.
Load database with initial values, if any.
Initialise test argc and argv
Call xname to parse the argc,argv pair.
Verify that argc was updated as expected.
Verify that argv was updated as expected.
Verify that the database was updated as expected.
>>CODE

	if(xpc_dotest(9, XPC_DB_MODE|XPC_ARGC_MODE|XPC_NOREPORT_MODE)) {
		CHECK;
	} else {
		FAIL;
		report("Unambiguous abbreviation failed to match.");
	}

	if(xpc_dotest(10, XPC_DB_MODE|XPC_ARGC_MODE|XPC_NOREPORT_MODE)) {
		CHECK;
	} else {
		FAIL;
		report("Ambiguous abbreviation matched incorrectly.");
	}

	CHECKPASS(2);

>>ASSERTION Good A
On a call to xname, the case of arguments is significant.
>>STRATEGY
Create new database to perform tests on.
Load database with initial values, if any.
Initialise test argc and argv
Call xname to parse the argc,argv pair.
Verify that argc was updated as expected.
Verify that argv was updated as expected.
Verify that the database was updated as expected.
>>CODE

	(void)xpc_dotest(11, XPC_DB_MODE|XPC_ARGC_MODE);
