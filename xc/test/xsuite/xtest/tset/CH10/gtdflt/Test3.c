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
 * $XConsortium: Test3.c,v 1.3 94/04/17 21:09:34 rws Exp $
 */

#include	<stdlib.h>
#include	"xtest.h"
#include	"Xlib.h"
#include	"Xutil.h"
#include	"tet_api.h"
#include	"xtestlib.h"
#include	"pixval.h"
#include	"Xatom.h"

extern	Display	*Dsp;

/* 
 * Dummy declarations which are normally inserted by mc.
 * Needed to prevent linkstart.c being included.
 */
char	*TestName = "XGetDefault";
int     tet_thistest;
struct tet_testlist tet_testlist[] = {
	NULL, 0
};
int 	ntests = sizeof(tet_testlist)/sizeof(struct tet_testlist)-1;

tet_main(argc, argv, envp)
int argc;	
char *argv[];
char *envp[];
{
int		pass = 0, fail = 0;
Display		*display;
char		*prog = "XTest";
char		*opt =  "testval32";
char		*tres = "VAL_b";
char		*res;
char		*pval="XTest.testval31:pval_a\nXTest.testval32:pval_b\nXTest.testval33:pval3\n";

	exec_startup();
	tpstartup();
	trace("Exec'd file ./Test3.");

	if(getenv("HOME") == (char *) NULL) {
		delete("Environment variable \"HOME\" is not set.");
		return;
	} else
		CHECK;

	XDeleteProperty(Dsp, RootWindow(Dsp, 0), XA_RESOURCE_MANAGER);
	XSync(Dsp, False);

	display = opendisplay();
	startcall(display);
	res = XGetDefault(display, prog, opt);
	endcall(display);

	if( res == (char *) NULL) {
		report("%s() returned NULL.", TestName);
		FAIL;
	} else {
		CHECK;
		if(strcmp(res, tres) != 0) {
			report("%s() with program \"%s\" and option \"%s\" returned \"%s\" instead of \"%s\".", TestName, prog, opt,  res, tres);
			FAIL;
		} else
			CHECK;
	}

	XChangeProperty (Dsp, RootWindow(Dsp, 0), XA_RESOURCE_MANAGER, XA_STRING, 8, PropModeReplace, (unsigned char *)pval, 1+strlen(pval));
	XSync(Dsp, False);

	display = opendisplay();
	tres = "pval_b";

	startcall(display);
	res = XGetDefault(display, prog, opt);
	endcall(display);

	if(res == (char *) NULL) {
		report("%s() returned NULL.", TestName);
		FAIL;
	} else {
		CHECK;

		if(strcmp(res, tres) != 0) {
			report("%s() with program \"%s\" and option \"%s\" returned \"%s\" instead of \"%s\"", TestName, prog, opt, res, tres);
			FAIL;
		} else
			CHECK;
	}


	CHECKPASS(5);
	tpcleanup();
	exec_cleanup();
}
