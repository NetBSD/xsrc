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
 * $XConsortium: tpstartup.c,v 1.7 94/04/17 21:01:41 rws Exp $
 */

#include "stdio.h"
#include "unistd.h"
#include "string.h"
#include "tet_api.h"
#include "xtest.h"
#include "Xlib.h"
#include "Xutil.h"
#include "xtestlib.h"
#include "XstlibInt.h"

/*
 * Actions to take at the beginning of a test purpose.
 * This version of tpstartup.c is for the X protocol test suite.
 */
void
tpstartup()
{
}

/*
 * Actions to take at the end of a test purpose.
 */
void
tpcleanup()
{
}

static char *savedfontpath = NULL;

static char *
put_in_commas(rep)
xGetFontPathReply *rep;
{
	char *p = NULL; /* for now, need Xstrealloc() etc. */
	unsigned int total_len;
	int npaths;
	char *endptr;
	CARD8 *fromptr;
	char *toptr;
	int reqlen = sizeof(xGetFontPathReply) + (int) (rep->length << 2);

	if (reqlen < sizeof(xGetFontPathReply)) {
		Log_Del ("Current server fontpath returned with bad length (%d)\n", reqlen);
		Free_Reply(rep);
		return NULL;
	}
	for (total_len=npaths=0,
		    fromptr=(CARD8 *)(((char *)rep)+sizeof(xGetFontPathReply));
		    npaths < (int)rep->nPaths;
		    npaths++) {
		total_len += *fromptr;
		fromptr += *fromptr + 1;
	}
	total_len += (npaths * sizeof(char)); /* commas + final nul */
	Log_Debug("Server's initial fontpath required %d bytes and had %d components\n", total_len, npaths);
	if (total_len <= 1 || npaths == 0) {
		Free_Reply(rep);
		return NULL;
	}
	p = (char *) Xstmalloc(total_len);
	if (p == NULL) {
		Log_Del ("Could not allocate %d bytes to store server's initial fontpath\n", total_len);
		Free_Reply(rep);
		return NULL;
	}
	endptr = p + total_len - 1;
	for (toptr=p,fromptr=(CARD8 *)(((char *)rep)+sizeof(xGetFontPathReply));
		    npaths > 0;
		    npaths--) {
		CARD8 len = *fromptr;

		bcopy(fromptr+1, toptr, (unsigned int)len);
		fromptr += len + 1;
		toptr += len;
		*toptr++ = ',';
	}
	*endptr = '\0'; /* stamp on last comma to terminate the string. */

	Free_Reply(rep);
	return p;
}

static char *
getfontpath(client)
int client;
{
	xReq *req;
	xGetFontPathReply *rep;

	req = (xReq *) Make_Req(client, X_GetFontPath);
	Send_Req(client, (xReq *) req);
	Log_Trace("client %d sent startup GetFontPath request\n", client);
	if ((rep = (xGetFontPathReply *) Expect_Reply(client, X_GetFontPath)) == NULL) {
		Log_Del("Failed to receive startup GetFontPath reply\n");
		Free_Req(req);
		return NULL;
	}  else  {
		Log_Trace("client %d received startup GetFontPath reply\n", client);
	}
	(void) Expect_Nothing(client);
	Free_Req(req);

	return put_in_commas(rep);
}

static void
setfontpath(client,prevpath)
int client;
char *prevpath;
{
	xReq *req;
	char *commaptr;
	CARD8 n;
	CARD16 nf;

	req = (xReq *) Make_Req(client, X_SetFontPath);
	req = Clear_Counted_Value(req);
	((xSetFontPathReq *)req)->nFonts = 0;
	/* don't touch nFonts until all Add_Counted_Value calls done
	   as it uses nFonts as a count of bytes added. We must start
	   with it zero and only set it to the actual value after all of
	   value bytes added.
	*/

	for (n=nf=0, commaptr=prevpath; commaptr && *commaptr;) {
		char *p = SearchString(commaptr, ',');
		int i;

		if (p != NULL)
			*p = '\0';
		n = strlen(commaptr);
		if (n > 0) {
			req = Add_Counted_Value(req, n);
			for (i=n; i-- > 0; commaptr++)
				req = Add_Counted_Value(req, *commaptr);
			nf++;
		}
		if (p != NULL) {
			if (commaptr != p) {
				Log_Del("INTERNAL ERROR in fontsetting\n");
				return;
			}
			*commaptr++ = ',';
		}
	}
	/* must do this as Add_Counted_Value uses nFonts as byte count */
	((xSetFontPathReq *)req)->nFonts = nf;
	Log_Debug("Set font path to '%s': %d components\n", prevpath, nf);

	Send_Req(client, (xReq *) req);
	Log_Trace("client %d sent startup SetFontPath request\n", client);
	(void) Expect_Nothing(client);

	Free_Req(req);
}

/*
 * Actions to take at the beginning of a test purpose.
 * This version of tpstartup.c is for the X protocol test suite.
 * Special version to set font path and create long lived client.
 */
void
tpfontstartup()
{
	/*
	 * Reset SIGALRM signals to be caught in case the TCM has messed 
	 * with the signal settings. This only needs to be done in 
	 * tpfontstartup(), not in startup(), because in the default case
	 * we don't make any protocol requests in this process (parent), 
	 * so the timer is not even switched on in the parent.
	 * Normally, all the action happens only in the child process.
	 */
	Set_Init_Timer();
	Create_Client(LONG_LIVED_CLIENT);
	savedfontpath = getfontpath(LONG_LIVED_CLIENT);
	Log_Trace("Server's initial fontpath was '%s'\n",
		(savedfontpath == NULL) ? "<Nothing>" : savedfontpath);
	if (config.fontpath == NULL || *config.fontpath == '\0') {
		Log_Del("No, or empty, XT_FONTPATH set\n");
		return;
	}
	setfontpath(LONG_LIVED_CLIENT, config.fontpath);
}

/*
 * Actions to take at the end of a test purpose.
 * Special version to reset font path and destroy long lived client.
 */
void
tpfontcleanup()
{
	setfontpath(LONG_LIVED_CLIENT, savedfontpath);
	if (savedfontpath != NULL)
		free(savedfontpath);
	Destroy_Client(LONG_LIVED_CLIENT);
}
