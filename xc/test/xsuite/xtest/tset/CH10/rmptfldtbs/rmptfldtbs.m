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
 * $XConsortium: rmptfldtbs.m,v 1.7 94/04/17 21:10:09 rws Exp $
 */
>>TITLE XrmPutFileDatabase CH10
void

XrmDatabase database = (XrmDatabase)NULL;
char *stored_db = "xpfd_file";
>>SET startup rmstartup
>>INCLUDE ../rmptrsrc/fn.mc
>>EXTERN

#define XPFD_T1_COUNT	4
static char *t1_data[XPFD_T1_COUNT][3] = {
	{ "a.b.c",	"ONE",	"a.b.c" },
	{ "D.E.F",	"TWO",	"D.E.F"},
	{ "*Z",	"THREE",	"A.Z"	},
	{ ".e.f.g.h.i.j",	"*!++ X&",	"e.f.g.h.i.j"	} };

>>ASSERTION Good A
A call to xname stores the resource name and value pairs 
in the specified file
.A stored_db 
from the specified
.A database
in ResourceLine format.
>>STRATEGY
Create a new database containing the test information.
Call xname to write the database.
Call XrmGetFileDatabase to check the database was written out.
Check the retrieved database contents were as expected.
Remove created file.
>>CODE
int a;
XrmDatabase rdb;

/* Create a new database containing the test information. */
	for(a=0; a<XPFD_T1_COUNT; a++) {
		XrmPutStringResource(&database, t1_data[a][0], t1_data[a][1]);
		CHECK;
	}

/* Call xname to write the database. */
	unlink( stored_db );
	XCALL;

/* Call XrmGetFileDatabase to check the database was written out. */
	rdb = XrmGetFileDatabase( stored_db );
	if (rdb == (XrmDatabase)NULL) {
		FAIL;
		delete("XrmGetFileDatabase could not open the written database.");
		return;
	} else {
/* Check the retrieved database contents were as expected. */
		for(a=0; a<XPFD_T1_COUNT; a++) {
			if (xrm_check_entry(rdb, t1_data[a][2], t1_data[a][2],
				"String", t1_data[a][1])) {
				delete("Unexpected data item returned from read in database");
				report("%s may have failed.", TestName);
			} else
				CHECK;
		}
	}

	CHECKPASS(XPFD_T1_COUNT + XPFD_T1_COUNT);

/* Remove created file. */
#ifndef TESTING
	unlink( stored_db ); 	/* To examine test file, */
				/* use pmake CFLOCAL=-DTESTING */
#endif
