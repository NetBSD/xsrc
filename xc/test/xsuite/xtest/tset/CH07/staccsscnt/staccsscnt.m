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
 * $XConsortium: staccsscnt.m,v 1.5 94/04/17 21:06:49 rws Exp $
 */
>>TITLE XSetAccessControl CH07
void

Display	*display = Dsp;
int 	mode;
>>ASSERTION Good B 1
When
.A mode
is
.S EnableAccess ,
then the use of the access control list is enabled at each connection setup.
>>ASSERTION Good B 1
When
.A mode
is
.S DisableAccess ,
then the use of the access control list is disabled at each connection setup.
>>STRATEGY
Get current host list.
Attempt to remove all hosts from list.
If this fails with BadAccess:
  Report that this host does not have permission to do this.
  (Will not have permission for the next part either).
  Result is untested.
Disable access control with xname.
Verify that state returned by XListHosts is DisableAccess.
Verify that a new connection can be made.
>>CODE
XHostAddress	*oldhosts;
XHostAddress	*list;
Display	*newclient;
int 	nhosts;
int 	njunk;
Bool	state;

	oldhosts = XListHosts(display, &nhosts, &state);

	CATCH_ERROR(display);
	XRemoveHosts(display, oldhosts, nhosts);
	RESTORE_ERROR(display);

	if (GET_ERROR(display) == BadAccess) {
		report("The client does not have permission to disable the acl");
		untested("  so this assertion cannot be tested.");
		return;
	}

	mode = DisableAccess;
	XCALL;

	list = XListHosts(display, &njunk, &state);
	if (state == DisableAccess)
		CHECK;
	else {
		report("Access control state was not DisableAccess (was %d)", state);
		FAIL;
	}

	newclient = XOpenDisplay(config.display);
	if (newclient != (Display*)0)
		CHECK;
	else {
		report("Connections could not be made to host");
		FAIL;
	}

	CHECKPASS(2);

	XAddHosts(display, oldhosts, nhosts);
	XFree((char*)oldhosts);
	XFree((char*)list);

>>ASSERTION Bad A
.ER BadAccess acl
>>STRATEGY
Call xname.
If error occurs.
  If error is BadAccess
	Report Pass.
  else
	Report Fail.
else
  Report client is authorised.
  Result is untested.
>>CODE

>>SET no-error-status-check
	XCALL;

	if (geterr() != Success) {
		if (geterr() == BadAccess)
			PASS;
		else {
			report("Expecting BadAccess, was %s", errorname(geterr()));
			FAIL;
		}
	} else {
		untested("This client is authorised to disable the access list");
		untested("  so the assertion cannot be tested");
	}

>>ASSERTION Bad A
.ER BadValue mode EnableAccess DisableAccess
