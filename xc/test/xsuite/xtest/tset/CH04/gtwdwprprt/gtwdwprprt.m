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
 * $XConsortium: gtwdwprprt.m,v 1.10 94/04/17 21:03:33 rws Exp $
 */
>>TITLE XGetWindowProperty CH04
int

Display *display = Dsp;
Window w = defwin(display);
Atom property = XA_COPYRIGHT;
long long_offset = 2;
long long_length = 2;
Bool delete_prop = False;
Atom req_type = XA_STRING;
Atom *actual_type_return = &actual_type;
int *actual_format_return = &actual_format;
unsigned long *nitems_return = &nitems;
unsigned long *bytes_after_return = &bytes_after;
unsigned char **prop_return = &prop;
>>EXTERN
#include "Xatom.h"

static Atom actual_type;
static int actual_format;
static unsigned long nitems;
static unsigned long bytes_after;
static unsigned char *prop;

static void
set_vars()
{
	actual_type = 0;
	actual_format = -1;
	nitems = -1;
	bytes_after = -1;
	prop = (unsigned char *)NULL;
}

static int
check_values( ex_actual_type, ex_actual_format, ex_nitems, ex_bytes_after )
Atom ex_actual_type;
int ex_actual_format;
unsigned long ex_nitems;
unsigned long ex_bytes_after;
{
	int fail;
	int pass;

	fail=0;
	pass=0;

#ifdef TESTING_CHECK
	ex_actual_type = XA_RESOLUTION;
	ex_actual_format = 6;
	ex_nitems=256;
	ex_bytes_after=256;
#endif
	if (actual_type != ex_actual_type) {
		FAIL;
		report("%s returned an incorrect actual_type_return",
			TestName);
		report("Expected actual_type_return: %u (%s)", ex_actual_type,
			(ex_actual_type==None?"None":atomname(ex_actual_type)));
		report("Returned actual_type_return: %u (%s)",
			actual_type, atomname(actual_type));
	} else
		pass++;

	if (actual_format != ex_actual_format) {
		FAIL;
		report("%s returned an incorrect actual_format_return",
			TestName);
		report("Expected actual_format_return: %d", ex_actual_format);
		report("Returned actual_format_return: %d", actual_format);
	} else
		pass++;

	if (nitems != ex_nitems) {
		FAIL;
		report("%s returned an incorrect nitems_return",
			TestName);
		report("Expected nitems_return: %d", ex_nitems);
		report("Returned nitems_return: %d", nitems);
	} else
		pass++;

	if (bytes_after != ex_bytes_after) {
		FAIL;
		report("%s returned an incorrect bytes_after_return",
			TestName);
		report("Expected bytes_after_return: %d", ex_bytes_after);
		report("Returned bytes_after_return: %d", bytes_after);
	} else
		pass++;

	return((fail==0 && pass==4?1:0));
}

>>ASSERTION Good A
A successful call to xname returns
.A Success
and the actual type of the property, the actual format of the property,
the number of 8-bit, 16-bit, or 32-bit items transferred,
the number of bytes remaining to be read in the property,
and a pointer to the data actually returned.
>>STRATEGY
Create a window with a property.
Call xname to obtain the property information.
Verify that the returned information was correct.
>>CODE
int ret;
unsigned char cbuf[5];
char *data = "a tested property";

/* Create a window with a property. */
	XChangeProperty(display, w, property, XA_STRING, 8,
		PropModeReplace,(unsigned char *)data, strlen(data));

/* Call xname to obtain the property information. */
	set_vars();
	long_offset = 3;
	long_length = 1;
	delete_prop = False;
	ret = XCALL;

/* Verify that the returned information was correct. */
	if (ret != Success) {
		FAIL;
		report("%s returned %d (expecting Success (%d))", TestName,
			ret, Success);
	} else
		CHECK;

	if (check_values( XA_STRING, 8, (unsigned long)4, (unsigned long)1 )) {
		CHECK;
	} else 
		FAIL;

	(void) strncpy(cbuf, &(data[12]), 4);
	cbuf[4] = '\0';

	if (prop == (unsigned char *)NULL) {
		FAIL;
		report("%s returned an incorrect prop_return",
			TestName);
		report("Expected prop_return: unsigned char * pointer");
		report("Returned prop_return: NULL pointer");
	} else
		if (strncmp(prop, (char *)cbuf, 5) != 0) {
			FAIL;
			report("%s returned an incorrect prop_return",
				TestName);
			report("Expected prop_return: '%s'", cbuf);
			report("Returned prop_return: '%s'", prop);
		} else
			CHECK;

	CHECKPASS(3);
	
>>ASSERTION Good A
>># Test for both delete True/False
>># Ensure no PropertyNotify events were generated.	-stuart.
When the specified
.A property
does not exist for the specified window
.A w ,
then a call to xname returns
.S None
to
.A actual_type_return ,
zero to
.A actual_format_return
and
.A bytes_after_return ,
the
.A nitems_return
argument is empty, and the
.A delete
argument is ignored.
>>STRATEGY
Create a window with PropertyChangeMask events selected and no properties.
For delete_prop of True and False:
	Call xname to obtain the property information.
	Verify that the returned values were correct.
	Verify that no PropertyNotify events were generated.
>>CODE
int mode;
int ret;
XEvent ev;

/* Create a window with PropertyChangeMask events selected and no properties. */
	XDeleteProperty(display, w, property); /* ENSURE property is nuked. */
	XSync(display,True);
	XSelectInput(display, w, PropertyChangeMask);

/* For delete_prop of True and False: */
	for(mode=0; mode<2; mode++) {

/* 	Call xname to obtain the property information. */
		delete_prop = (mode==0?True:False);
		trace("delete_prop is %s", boolname(delete_prop));
		set_vars();
		long_offset = 3;
		long_length = 1;
		ret = XCALL;
		XSync(display, False);

/* 	Verify that the returned values were correct. */
		if (ret != Success) {
			FAIL;
			report("%s returned %d (expecting Success (%d))",
				TestName, ret, Success);
		} else
			CHECK;

		if ( check_values( None, 0,
			(unsigned long)0, (unsigned long)0 ) ) {
			CHECK;
		} else
			FAIL;

/* 	Verify that no PropertyNotify events were generated. */
		ret = getevent(display, &ev);
#ifdef TESTING
	ret++;
	ev.type = PropertyNotify;
#endif
		if (ret != 0) {
			FAIL;
			report("%s caused %d unexpected event%s", TestName,
					ret, (ret==1?"":"s"));
			do {
				report("event %s returned", eventname(ev.type));
			} while(getevent(display, &ev) != 0) ;
		} else
			CHECK;
	}

	CHECKPASS(2*3);
>>ASSERTION Good A
>># Ensure the last item by ensuring no PropertyNotify events were
>># generated. Test for both delete True and False.	-stuart.
When the specified
.A property
exists for the specified window
.A w
and the type does not match the specified
.A req_type ,
then a call to xname returns the actual property type to
.A actual_type_return ,
the actual property format to
.A actual_format_return ,
and the property length in bytes to
.A bytes_after_return ,
the
.A nitems_return
argument is empty, and the
.A delete
argument is ignored.
>>STRATEGY
Create a window with a property and PropertyChangeMask events selected.
For delete_prop of True and False:
	Call xname to obtain the property information.
	Verify that the returned values were correct.
	Verify that no PropertyNotify events were generated.
>>CODE
int mode;
int ret;
char *data = "a tested property";
XEvent ev;

/* Create a window with a property and PropertyChangeMask events selected. */
	XChangeProperty(display, w, property, XA_STRING, 8,
		PropModeReplace,(unsigned char *)data, strlen(data));
	XSync(display,True);
	XSelectInput(display, w, PropertyChangeMask);

/* For delete_prop of True and False: */
	for(mode=0; mode<2; mode++) {

/* 	Call xname to obtain the property information. */
		delete_prop = (mode==0?True:False);
		trace("delete_prop is %s", boolname(delete_prop));
		set_vars();
		long_offset = 3;
		long_length = 1;
		req_type = XA_INTEGER;
		ret = XCALL;
		XSync(display, False);

/* 	Verify that the returned values were correct. */
		if (ret != Success) {
			FAIL;
			report("%s returned %d (expecting Success (%d))",
				TestName, ret, Success);
		} else
			CHECK;

		if ( check_values( XA_STRING, 8,
			(unsigned long)0, (unsigned long)strlen(data) ) ) {
			CHECK;
		} else
			FAIL;

/* 	Verify that no PropertyNotify events were generated. */
		ret = getevent(display, &ev);
#ifdef TESTING
	ret++;
	ev.type = PropertyNotify;
#endif
		if (ret != 0) {
			FAIL;
			report("%s caused %d unexpected event%s", TestName,
					ret, (ret==1?"":"s"));
			do {
				report("event %s returned", eventname(ev.type));
			} while(getevent(display, &ev) != 0) ;
		} else
			CHECK;
	}

	CHECKPASS(2*3);
>>ASSERTION Good A
>># Ensure a) No PropertyNotify events generated
>># 	   b) That the property can be referenced again.
>>#	   c) tested for req_type of type and AnyPropertyType
>># Test this by using long_offset and long_length to test different
>># values for bytes_after_return and nitems_return.	-stuart.
>># There might be a better way of expressing the assertion...
When the specified
.A property
exists for the specified window
.A w ,
.A req_type
is set to the type of the property or
.S AnyPropertyType ,
and
.A delete
is set to
.S False ,
then a call to xname returns the actual
.A property
type to
.A actual_type_return ,
the actual
.A property
format to
.A actual_format_return ,
the number of trailing unread bytes in the
.A property
in
.A bytes_after_return ,
the number of 8/16/32 bit items in
.A nitems_return ,
the data is placed in
.A prop_return ,
where the data is sourced from four times
.A long_offset
bytes into the
.A property ,
and is the minimum of the remaining bytes
left in the
.A property
and four times
.A long_length
bytes long, and the
.A property
is not deleted.
>>STRATEGY
Create a window with testable properties.
For req_type is the required type and AnyPropertyType:
	Call xname to obtain the property information of a STRING property,
		with delete False.
	Verify that the returned values were correct.
	Verify that no PropertyNotify events were generated.
	Call xname to obtain the property information of an INTEGER property,
		with delete False.
	Verify that the returned values were correct.
	Verify that no PropertyNotify events were generated.
>>CODE
int ret;
int mode;
char *cdata = "a tested property";
>># Unsigned longs _must_ be 32 bits, or this won't work...
unsigned long idata[4];
XEvent ev;

/* Create a window with testable properties. */
	XChangeProperty(display, w, XA_COPYRIGHT, XA_STRING, 8,
		PropModeReplace, (unsigned char *)cdata, strlen(cdata));

	for( ret=0; ret<4; ret++ )
		idata[ret] = ret;
	XChangeProperty(display, w, XA_NOTICE, XA_INTEGER, 32,
		PropModeReplace, (unsigned char *)idata, 4);

	XSync(display, True);
	XSelectInput(display, w, PropertyChangeMask);

/* For req_type is the required type and AnyPropertyType: */
	for(mode=0; mode<2 ;mode++) {
	
		trace("Calling %s to obtain string information", TestName);
/* 	Call xname to obtain the property information of a STRING property, */
/* 		with delete False. */
		set_vars();
		property = XA_COPYRIGHT;
		long_length = 2; /* Attempt to read 8 bytes from */
		long_offset = 4; /* 16 bytes into the property.   */
				 /* We expect only one byte to be returned. */
		trace("req_type is %s", (mode==0?"STRING":"AnyPropertyType"));
		req_type=(mode==0?XA_STRING:AnyPropertyType);
		ret = XCALL;
		XSync(display, False);

/* 	Verify that the returned values were correct. */
		if (ret != Success) {
			FAIL;
			report("%s returned %d (expecting Success (%d))",
				TestName, ret, Success);
		} else
			CHECK;

		if ( check_values( XA_STRING, 8,
				(unsigned long)1, (unsigned long)0 ) ) {
			CHECK;
		} else
			FAIL;

		if (prop == (unsigned char *)NULL) {
			FAIL;
			report("%s returned an unexpected prop_return");
			report("Expected prop_return: unsigned char * pointer");
			report("Returned prop_return: NULL pointer");
		} else
			if (strncmp(prop, "y", 1) != 0) {
				FAIL;
				report("%s returned an unexpected prop_return");
				report("Expected prop_return: 'y'");
				report("Returned prop_return: '%s'", prop);
			} else
				CHECK;

/* 	Verify that no PropertyNotify events were generated. */
		ret = getevent(display, &ev);
		if (ret != 0) {
			FAIL;
			report("%s caused %d unexpected event%s", TestName,
					ret, (ret==1?"":"s"));
			do {
				report("event %s returned", eventname(ev.type));
			} while(getevent(display, &ev) != 0) ;
		} else
			CHECK;

		trace("Calling %s to obtain integer information", TestName);
/* 	Call xname to obtain the property information of an INTEGER property, */
/* 		with delete False. */
		set_vars();
		property = XA_NOTICE;
		long_length = 1; /* Attempt to read 4 bytes from */
		long_offset = 1; /* 4 bytes into the property.   */
				 /* We expect one integer to be returned. */
		trace("req_type is %s", (mode==0?"INTEGER":"AnyPropertyType"));
		req_type=(mode==0?XA_INTEGER:AnyPropertyType);
		ret = XCALL;
		XSync(display, False);

/* 	Verify that the returned values were correct. */
		if (ret != Success) {
			FAIL;
			report("%s returned %d (expecting Success (%d))",
				TestName, ret, Success);
		} else
			CHECK;

		if ( check_values( XA_INTEGER, 32,
			(unsigned long)1, (unsigned long)8 ) ) {
			CHECK;
		} else
			FAIL;

		if (prop == (unsigned char *)NULL) {
			FAIL;
			report("%s returned an unexpected prop_return");
			report("Expected prop_return: unsigned char * pointer");
			report("Returned prop_return: NULL pointer");
		} else
			if ( *(unsigned long *)prop != idata[1] ) {
				FAIL;
				report("%s returned an unexpected prop_return");
				report("Expected prop_return: %u", idata[1]);
				report("Returned prop_return: %u",
					*(unsigned long*)prop);
			} else
				CHECK;
		
/* 	Verify that no PropertyNotify events were generated. */
		ret = getevent(display, &ev);
		if (ret != 0) {
			FAIL;
			report("%s caused %d unexpected event%s", TestName,
					ret, (ret==1?"":"s"));
			do {
				report("event %s returned", eventname(ev.type));
			} while(getevent(display, &ev) != 0) ;
		} else
			CHECK;
	}

	CHECKPASS(2*8);

>>ASSERTION Good A
>># NEW, NON_MIT_REVIEWED ASSERTION
When the specified
.A property
exists for the specified window
.A w ,
.A req_type
is set to the type of the property or
.S AnyPropertyType ,
and
.A delete
is set to
.S True ,
and on a call to xname the number of unread bytes in the
.A property
returned to
.A bytes_after_return
is non-zero, then 
the actual
.A property
type is returned  to
.A actual_type_return ,
the actual
.A property
format to
.A actual_format_return ,
the number of trailing unread bytes in the
.A property
in
.A bytes_after_return ,
the number of 8/16/32 bit items in
.A nitems_return ,
the data is placed in
.A prop_return ,
where the data is sourced from four times
.A long_offset
bytes into the
.A property ,
and is the minimum of the remaining bytes
left in the
.A property
and four times
.A long_length
bytes long, and the
.A property
is not deleted.
>>STRATEGY
Create a window with testable properties.
For req_type is the required type and AnyPropertyType:
	Call xname to obtain the property information of a STRING property,
		with delete True.
	Verify that the returned values were correct.
	Verify that no PropertyNotify events were generated.
	Call xname to obtain the property information of an INTEGER property,
		with delete True.
	Verify that the returned values were correct.
	Verify that no PropertyNotify events were generated.
>>CODE
int ret;
int mode;
char *cdata = "a tested property";
>># Unsigned longs _must_ be 32 bits, or this won't work...
unsigned long idata[4];
XEvent ev;

/* Create a window with testable properties. */
	XChangeProperty(display, w, XA_COPYRIGHT, XA_STRING, 8,
		PropModeReplace, (unsigned char *)cdata, strlen(cdata));

	for( ret=0; ret<4; ret++ )
		idata[ret] = ret;
	XChangeProperty(display, w, XA_NOTICE, XA_INTEGER, 32,
		PropModeReplace, (unsigned char *)idata, 4);

	XSync(display, True);
	XSelectInput(display, w, PropertyChangeMask);

/* For req_type is the required type and AnyPropertyType: */
	for(mode=0; mode<2 ;mode++) {
	
		trace("Calling %s to obtain string information", TestName);
/* 	Call xname to obtain the property information of a STRING property, */
/* 		with delete True. */
		set_vars();
		property = XA_COPYRIGHT;
		long_length = 1; /* Attempt to read 4 bytes from */
		long_offset = 1; /* 4 bytes into the property.   */
				 /* We expect 4 bytes to be returned. */
		trace("req_type is %s", (mode==0?"STRING":"AnyPropertyType"));
		req_type=(mode==0?XA_STRING:AnyPropertyType);
		delete_prop=True;
		ret = XCALL;
		XSync(display, False);

/* 	Verify that the returned values were correct. */
		if (ret != Success) {
			FAIL;
			report("%s returned %d (expecting Success (%d))",
				TestName, ret, Success);
		} else
			CHECK;

		if ( check_values( XA_STRING, 8,
				(unsigned long)4, (unsigned long)9 ) ) {
			CHECK;
		} else
			FAIL;

		if (prop == (unsigned char *)NULL) {
			FAIL;
			report("%s returned an unexpected prop_return");
			report("Expected prop_return: unsigned char * pointer");
			report("Returned prop_return: NULL pointer");
		} else
			if (strncmp(prop, "sted", 1) != 0) {
				FAIL;
				report("%s returned an unexpected prop_return");
				report("Expected prop_return: 'sted'");
				report("Returned prop_return: '%s'", prop);
			} else
				CHECK;

/* 	Verify that no PropertyNotify events were generated. */
		ret = getevent(display, &ev);
		if (ret != 0) {
			FAIL;
			report("%s caused %d unexpected event%s", TestName,
					ret, (ret==1?"":"s"));
			do {
				report("event %s returned", eventname(ev.type));
			} while(getevent(display, &ev) != 0) ;
		} else
			CHECK;

		trace("Calling %s to obtain integer information", TestName);
/* 	Call xname to obtain the property information of an INTEGER property, */
/* 		with delete True. */
		set_vars();
		property = XA_NOTICE;
		long_length = 1; /* Attempt to read 4 bytes from */
		long_offset = 1; /* 4 bytes into the property.   */
				 /* We expect one integer to be returned. */
		trace("req_type is %s", (mode==0?"INTEGER":"AnyPropertyType"));
		req_type=(mode==0?XA_INTEGER:AnyPropertyType);
		delete_prop = True;
		ret = XCALL;
		XSync(display, False);

/* 	Verify that the returned values were correct. */
		if (ret != Success) {
			FAIL;
			report("%s returned %d (expecting Success (%d))",
				TestName, ret, Success);
		} else
			CHECK;

		if ( check_values( XA_INTEGER, 32,
			(unsigned long)1, (unsigned long)8 ) ) {
			CHECK;
		} else
			FAIL;

		if (prop == (unsigned char *)NULL) {
			FAIL;
			report("%s returned an unexpected prop_return");
			report("Expected prop_return: unsigned char * pointer");
			report("Returned prop_return: NULL pointer");
		} else
			if ( *(unsigned long *)prop != idata[1] ) {
				FAIL;
				report("%s returned an unexpected prop_return");
				report("Expected prop_return: %u", idata[1]);
				report("Returned prop_return: %u",
					*(unsigned long *)prop);
			} else
				CHECK;
		
/* 	Verify that no PropertyNotify events were generated. */
		ret = getevent(display, &ev);
		if (ret != 0) {
			FAIL;
			report("%s caused %d unexpected event%s", TestName,
					ret, (ret==1?"":"s"));
			do {
				report("event %s returned", eventname(ev.type));
			} while(getevent(display, &ev) != 0) ;
		} else
			CHECK;
	}

	CHECKPASS(2*8);

>>ASSERTION Good A
When the specified
.A property
exists for the specified window
.A w ,
.A req_type
is set to the type of the property or
.S AnyPropertyType ,
.A delete
is set to
.S True ,
and on a call to xname the number of unread bytes in the
.A property
returned to
.A bytes_after_return
is zero, then the property is deleted from the window
.A w
and a
.S PropertyNotify
event is generated on the specified window
.A w .
>>STRATEGY
Create a window with PropertyChangeMask events selected.
For req_type is the required type and AnyPropertyType:
	Create a property on the window.
	Call xname to obtain the property information,
		with delete True.
	Verify that the returned values were correct.
	Verify that a single PropertyNotify event was generated.
	Verify that the property has been deleted.
>>CODE
int mode;
int ret;
char *data="a tested property";
XEvent ev;

/* Create a window with PropertyChangeMask events selected. */
	XSelectInput(display, w, PropertyChangeMask);

/* For req_type is the required type and AnyPropertyType: */
	for(mode=0; mode<2 ;mode++) {

/* 	Create a property on the window. */
		XChangeProperty(display, w, XA_COPYRIGHT, XA_STRING, 8,
			PropModeReplace, (unsigned char *)data, strlen(data));
		XSync(display,True);

/* 	Call xname to obtain the property information, */
/* 		with delete True. */

		set_vars();

		delete_prop = True;
		req_type=(mode==0?XA_STRING:AnyPropertyType);
		long_offset = 3;
		long_length = 2;

		trace("delete_prop is %s", boolname(delete_prop));
		trace("req_type is %s", (mode==0?"STRING":"AnyPropertyType"));
		ret = XCALL;
		XSync(display, False);

/* 	Verify that the returned values were correct. */
		if (ret != Success) {
			FAIL;
			report("%s returned %d (expecting Success (%d))",
				TestName, ret, Success);
		} else
			CHECK;

		if ( check_values( XA_STRING, 8,
			(unsigned long)5, (unsigned long)0 ) ) {
			CHECK;
		} else
			FAIL;

/* 	Verify that a single PropertyNotify event was generated. */
		ret = getevent(display, &ev);
		if (ret != 1 || ev.type != PropertyNotify) {
			FAIL;
			report("%s caused %d events", TestName, ret);
			report("Expecting a single PropertyNotify event");
			while(ret != 0) {
				report("Returned: %s event",
					eventname(ev.type));
				ret = getevent(display, &ev);
			}
		} else {
			XEvent good;
			good.type = PropertyNotify;
			good.xproperty.type = PropertyNotify;
			good.xproperty.display = display;
			good.xproperty.serial = 0; /* Can't know */
			good.xproperty.send_event = False;
			good.xproperty.display = display;
			good.xproperty.window = w;
			good.xproperty.atom = XA_COPYRIGHT;
			good.xproperty.time = 0 ; /* Can't know */
			good.xproperty.state = PropertyDelete;

			if (checkevent(&good, &ev) != 0) {
				FAIL;
			} else
				CHECK;
		}

/* 	Verify that the property has been deleted. */
		delete_prop=False;
		ret = XCALL;
		
		if ((ret != Success) || !check_values(None, 0,
					(unsigned long)0, (unsigned long)0) ) {
			FAIL;
			report("Property was not deleted.");
		} else
			CHECK;
	}

	CHECKPASS(2*4);

>>ASSERTION Good A
A call to xname always allocates one extra byte in
.A prop_return 
and sets it to ASCII
.A NULL .
>>STRATEGY
Create a window with a property.
Call xname to obtain property information.
Verify that prop_return contained an ASCII NULL.
>>CODE
int ret;
char *data = "a tested property";

/* Create a window with a property. */
	XChangeProperty(display, w, property, XA_STRING, 8, PropModeReplace,
			(unsigned char *)data, strlen(data));

/* Call xname to obtain property information. */
	set_vars();
	long_offset=1;
	long_length=0;
	ret = XCALL;

	if (ret != Success) {
		delete("%s returned %d when expecting %d", ret, Success);
		return;
	} else
		CHECK;

	if (!check_values( XA_STRING, 8,
			(unsigned long)0, (unsigned long)13 )) {
		delete("%s returned unexpected values");
	} else
		CHECK;

/* Verify that prop_return contained an ASCII NULL. */

	if (prop == (unsigned char *)NULL) {
		FAIL;
		report("%s returned an unexpected prop_return", TestName);
		report("Expected prop_return: unsigned char * pointer");
		report("Returned prop_return: NULL pointer");
		return;
	} else
		CHECK;

	if (*prop != (unsigned char)'\0') {
		FAIL;
		report("%s did not allocate an ASCII NULL character beyond");
		report("the prop_return data.");
	} else
		CHECK;
 
	CHECKPASS(4);
>>ASSERTION Bad A
.ER BadWindow
>>ASSERTION Bad A
.ER BadAtom
>>ASSERTION Bad A
When xname is called with
.A long_offset
such that the offset lies beyond the end of the
.A property ,
then a
.S BadValue
error occurs.
>>STRATEGY
Create a window with a property and PropertyChangeMask events selected.
Call xname with a long_offset beyond the property end.
Verify that a BadValue error occurred.
>>CODE BadValue
char *data = "a tested property";

/* Create a window with a property and PropertyChangeMask events selected. */
	XChangeProperty(display, w, property, XA_STRING, 8,
		PropModeReplace,(unsigned char *)data, strlen(data));

	seterrdef();

/* Call xname with a long_offset beyond the property end. */
	long_offset=5;
	long_length=0;

	XCALL;

/* Verify that a BadValue error occurred. */
	if (geterr() != BadValue) {
		FAIL;
		report("%s did not generate a BadValue when long_offset was",
			TestName);
		report("beyond the length of the property");
	} else
		CHECK;

	CHECKPASS(1);
