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
 * $XConsortium: enblaccssc.m,v 1.6 94/04/17 21:06:18 rws Exp $
 */
>>TITLE XEnableAccessControl CH07
void

Display	*display = Dsp;
>>ASSERTION Good B 1
A call to xname
enables the use of the access control list at each connection setup.
>>STRATEGY
Get current host list.
Attempt to remove all hosts from list.
If this fails with BadAccess:
  Report that this host does not have permission to do this.
  (Will not have permission for the next part either).
  Result is untested.
Enable access control with xname.
Verify that state returned by XListHosts is EnableAccess.
Verify that new connections can not be made.
>>CODE
#if 0
XHostAddress	*oldhosts;
XHostAddress	*list;
Display	*newclient;
int 	nhosts;
int 	njunk;
Bool	state;
#endif

	untested("There is no portable way to test this assertion");

	/* -------- */

#if 0
	/*
	 * The following will work on some servers.  However on other
	 * it can result in the client not being able to reset the state
	 * to disabled, and no more can be done without manual intervention
	 * to the server.
	 */
	oldhosts = XListHosts(display, &nhosts, &state);

	CATCH_ERROR(display);
	XRemoveHosts(display, oldhosts, nhosts);
	RESTORE_ERROR(display);

	if (GET_ERROR(display) == BadAccess) {
		report("The client does not have permission to disable the acl");
		untested("  so this assertion cannot be tested.");
		return;
	}

	XCALL;

	list = XListHosts(display, &njunk, &state);
	if (state == EnableAccess)
		CHECK;
	else {
		report("Access control state was not EnableAccess (was %d)", state);
		FAIL;
	}

	newclient = XOpenDisplay(config.display);
	if (newclient == (Display*)0)
		CHECK;
	else {
		report("Connections could be made to host");
		FAIL;
	}

	CHECKPASS(2);

	/*
	 * It is quite possible that the following will fail.
	 */
	CATCH_ERROR(display);
	XAddHosts(display, oldhosts, nhosts);
	XDisableAccessControl(display);
	RESTORE_ERROR(display);

	XFree((char*)oldhosts);
	XFree((char*)list);
#endif
>>ASSERTION Good B 1
When an attempt is made to enable the use of the access control list
from a client that is not authorised in a server-dependent way
to do so, then a
.S BadAccess
error occurs.
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

		CATCH_ERROR(display);
		XDisableAccessControl(display);
		RESTORE_ERROR(display);
	}

