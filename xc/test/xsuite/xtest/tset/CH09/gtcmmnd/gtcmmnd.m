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
 * $XConsortium: gtcmmnd.m,v 1.8 94/04/17 21:08:38 rws Exp $
 */
>>TITLE XGetCommand CH09
Status
XGetCommand(display, w, argv_return, argc_return)
Display		*display = Dsp;
Window		w = DRW(Dsp);
char		***argv_return = &argvdef;
int		*argc_return = &argcdef;
>>EXTERN
#include	"Xatom.h"
char		**argvdef;
int		argcdef;
>>ASSERTION Good A
When the WM_COMMAND property is set for the window
.A w ,
is of
.M type
STRING, and is of
.M format
8, then a call to xname returns the property string list,
which can be freed with XFree,
in the
.A argv_return
argument and the number of strings in the
.A argc_return
argument and returns non-zero.
>>STRATEGY
Set the WM_COMMAND property using XSetCommand.
Obtain the value of the WM_COMMAND property using XGetCommand.
Verify that the call did not return zero.
Verify that the number and value of the returned strings is correct.
>>CODE
XVisualInfo	*vp;
Status	status;
char	*nullstr = "<NULL>";
char	**strpp, *strp;
char	*str1 = "XTest string 1____";
char	*str2 = "XTest string 2__";
char	*str3 = "XTest string 3___";
int	nstrs = 3;
char	*prop[3];
char	**rstrings = (char**) NULL;
int	rcount = 0;
int	i;

	prop[0] = str1;
	prop[1] = str2;
	prop[2] = str3;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	w = makewin(display, vp);

	XSetCommand(display, w, prop, nstrs);

	argv_return = &rstrings;
	argc_return = &rcount;
	status = XCALL;

	if(status == False) {
		delete("XGetCommand() returned False.");
		return;
	} else
		CHECK;

	if(rstrings == (char **) NULL) {
		report("Returned list of strings was NULL");
		FAIL;
	} else {
		CHECK;

		if(rcount != nstrs) {
			report("%d strings were returned instead of %d", rcount, nstrs);
			FAIL;
		} else {
			CHECK;

			for(i=0, strpp = rstrings; i< nstrs; i++, strpp++) {
				strp = (*strpp == NULL ? nullstr : *strpp);

				if(strcmp(strp, prop[i]) != 0) {
					report("String %d was \"%s\" instead of \"%s\"", i, strp, prop[i]);
					FAIL;
				} else
					CHECK;
			}

		}
		XFreeStringList(rstrings);
	}

	CHECKPASS(nstrs + 3);

>>ASSERTION Good A
When the WM_COMMAND property is not set for the window
.A w ,
or is not of
.M type
STRING, or is not of
.M format
8, then a call to xname returns zero.
>>STRATEGY
Create a window with XCreateWindow.
Obtain the value of the unset WM_ICON_SIZES property using XGetCommand.
Verify that the function returned zero.

Create a window with XCreateWindow.
Set the WM_COMMAND property to have format 16 type STRING using XChangeProperty.
Obtain the value of the WM_COMMAND property using XGetCommand.
Verify that the call returned zero

Create a window with XCreateWindow.
Set the WM_COMMAND property to have format 8 and type ATOM using XChangeProperty.
Obtain the value of the WM_COMMAND property using XGetCommand.
Verify that the call returned zero.
>>CODE
Status		status;
char		*s = "XTestString1";
XVisualInfo	*vp;
char		**rstrings = (char **) NULL;
int		rcount = 0;

	resetvinf(VI_WIN);
	nextvinf(&vp);
	argv_return = &rstrings;
	argc_return = &rcount;

	w = makewin(display, vp);

/* unset property */

	status = XCALL;

	if(status != False) {
		report("%s() did not return False when the property was unset.", TestName);
		FAIL;
	} else
		CHECK;


	w = makewin(display, vp);

/* format 16 */
	XChangeProperty(display, w, XA_WM_COMMAND, XA_STRING, 16, PropModeReplace, (unsigned char *) s, strlen(s) );

	status = XCALL;

	if(status != False) {
		report("%s() did not return False when property was of format 16.", TestName);
		FAIL;
	} else
		CHECK;


	w = makewin(display, vp);

/* type ATOM */
	XChangeProperty(display, w, XA_WM_COMMAND, XA_ATOM, 8, PropModeReplace, (unsigned char *) s, strlen(s) );

	status = XCALL;

	if(status != False) {
		report("%s() did not return False when property was of type Atom.", TestName);
		FAIL;
	} else
		CHECK;


	CHECKPASS(3);

>>ASSERTION Bad B 1
When insufficient memory is available to contain the string list,
then a call to xname returns a zero status.
>># Kieron	Completed	Review
