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
 * $XConsortium: rmqgtrsrc.m,v 1.8 94/04/17 21:10:18 rws Exp $
 */
>>TITLE XrmQGetResource CH10
Bool

XrmDatabase database = (XrmDatabase)NULL;
XrmNameList quark_name = name;
XrmClassList quark_class = class;
XrmRepresentation *quark_type_return = &type;
XrmValue *value_return = &value;
>>SET startup rmstartup
>>INCLUDE ../rmptrsrc/fn.mc
>>INCLUDE ../rmgtrsrc/common.mc
>>EXTERN
XrmName	name[10];
XrmClass	class[10];
XrmRepresentation	type;
XrmValue	value;
>>ASSERTION Good A
When
.A quark_name
is a list of quarks representing a fully qualified resource name
and
.A quark_class
is a list of quarks representing a fully qualified resource class,
and the 
.A database
contains one or more corresponding resources,
then a call to xname places
the representation type as a quark in
.A quark_type_return
and the value in
.A value_return
of the closest matching resource as defined by the matching rules,
and returns
.S True .
>>STRATEGY
>>CODE
int a;
XrmValue b;
Bool ret;

/* Create a database containing test information. */
	database = xrm_create_database("");
	if (database == (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

	for(a=0; a<XGR_T1_DATA; a++) {
		xrm_fill_value(&b, t1_data[a][2]);
		XrmPutResource(&database, t1_data[a][0], t1_data[a][1], &b );
		CHECK;
	}

#ifdef TESTING
	XrmPutFileDatabase(database, "xgr_one");
#endif

/* Interrogate database using xname. */
/* Verify that the returned type and value were as expected. */
	for(a=0; a<XGR_T1_TEST; a++) {
		XrmStringToNameList(t1_test[a][0], quark_name);
		XrmStringToClassList(t1_test[a][1], quark_class);
		type = (XrmRepresentation)0;
		value.addr = (caddr_t)NULL;
		ret = XCALL;
		if (ret==False) {
			FAIL;
			report("%s failed to match a database entry.",
				TestName);
			report("name :%s", t1_test[a][0]);
			report("class:%s", t1_test[a][1]);
			report("Possible diagnosis: %s", t1_test[a][4]);
		} else {
			CHECK;

			if (type == (XrmRepresentation)0 ||
				 type != XrmStringToRepresentation(t1_test[a][2])) {
				FAIL;
				report("%s failed to return expected type.",
					TestName);
				report("Expected type: '%s'", t1_test[a][2]);
				report("Returned type: '%s'",
					(type==(XrmRepresentation)0?
					"<NULL QUARK>":XrmQuarkToString(type)));
			} else
				CHECK;


			if (value.addr == NULL
				|| strncmp(t1_test[a][3], (char *)value.addr, strlen(t1_test[a][3]))){
				char tb[100];
				unsigned int l;

				if (value.addr == NULL) {
					strcpy(tb, "<NULL POINTER>");
					l = strlen(tb);
				} else {
					(void) strncpy(tb, (char*)value.addr, value.size);
					tb[value.size]='\0';
					l = value.size;
				}
				FAIL;
				report("%s failed to return expected value.",
					TestName);
				report("Expected value: '%s'", t1_test[a][3]);
				report("Returned value: '%.*s'", l, tb);
				report("Possible diagnosis: %s", t1_test[a][4]);
			} else
				CHECK;
		}
	}

	CHECKPASS(1 + XGR_T1_DATA + 3*XGR_T1_TEST);
>>ASSERTION Good A
When
.A quark_name
is a list of quarks representing a fully qualified resource name
and
.A quark_class
is a list of quarks representing a fully qualified resource class,
and the
.A database
does not contain a corresponding resource as defined by the matching rules,
then a call to xname returns
.S False .
>>STRATEGY
Create a database containing test information.
Interrogate database using xname.
Verify that the test examples were not found.
>>CODE
int a;
XrmValue b;
Bool ret;

/* Create a database containing test information. */
	database = xrm_create_database("");
	if (database == (XrmDatabase)NULL) {
		delete("Could not create test database.");
		return;
	} else
		CHECK;

	for(a=0; a<XGR_T1_DATA; a++) {
		xrm_fill_value(&b, t1_data[a][2]);
		XrmPutResource(&database, t1_data[a][0], t1_data[a][1], &b );
		CHECK;
	}

#ifdef TESTING
	XrmPutFileDatabase(database, "xgr_two");
#endif

/* Interrogate database using xname. */
/* Verify that the test examples were not found. */
	for(a=0; a<XGR_T2_TEST; a++) {
		XrmStringToNameList(t2_test[a][0], quark_name);
		XrmStringToClassList(t2_test[a][1], quark_class);
		type = (XrmRepresentation)0;
		value.addr = (caddr_t)NULL;
		ret = XCALL;
		if (ret==False) {
			CHECK;
		} else {
			char tb[100];
			unsigned int l;

			FAIL;
			report("%s returned a database match when a failure was expected.",
				TestName);
			report("Returned type: '%s'", (type==(XrmRepresentation)0)?
					"<NULL QUARK>":XrmRepresentationToString(type));

			if (value.addr == NULL) {
				strcpy(tb, "<NULL POINTER>");
				l = strlen(tb);
			} else {
				(void) strncpy(tb, (char*)value.addr, value.size);
				tb[value.size]='\0';
				l = value.size;
			}
			report("Returned value: '%.*s'", l, tb);
			report("Possible diagnosis: %s", t2_test[a][3]);
		}
	}

	CHECKPASS(1 + XGR_T1_DATA + XGR_T2_TEST);
