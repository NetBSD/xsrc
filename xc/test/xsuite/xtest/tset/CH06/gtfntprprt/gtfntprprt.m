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
 * $XConsortium: gtfntprprt.m,v 1.8 94/04/17 21:05:36 rws Exp $
 */
>>TITLE XGetFontProperty CH06
Bool

XFontStruct	*font_struct;
Atom	atom;
unsigned long	*value_return;
>>SET startup fontstartup
>>SET cleanup fontcleanup
>>EXTERN
#include	"Xatom.h"
>>ASSERTION Good A
When the property specified by the
.A atom
argument
is defined,
then a call to xname returns
the value of the property
.A atom
in the
.S XFontStruct
named by the argument
.A font_struct
and returns
.S True .
>>STRATEGY
Retrieve properties that are known to be defined for the test fonts.
Verify that True is returned.
Verify that the value of the properties are correct.
>>CODE
unsigned long	val;
Bool	ret;
int 	i;
XFontProp	*fprop;
extern	XFontStruct	xtfont0;	/* Known good version */
extern	char	*xtfont0cpright;	/* Known good version */

	font_struct = XLoadQueryFont(Dsp, "xtfont0");
	if (font_struct == NULL || isdeleted()) {
		delete("Could not load font, check that xtest fonts are installed");
		return;
	}

	value_return = &val;

	for (i = 0; i < xtfont0.n_properties; i++) {
		fprop = &xtfont0.properties[i];
		atom = fprop->name;

		ret = XCALL;
		if (ret != True) {
			report("call did not return True for atom %s", atomname(atom));
			FAIL;
			continue;
		} else
			CHECK;

		if (atom == XA_COPYRIGHT) {
		char	*crstr;

			XSetErrorHandler(error_status);
			reseterr();
			crstr = XGetAtomName(Dsp, val);
			XSetErrorHandler(unexp_err);
			switch (geterr()) {
			case Success:
				break;
			case BadAtom:
				report("copyright string atom did not exist");
				FAIL;
				break;
			default:
				delete("Call to XGetAtomName failed");
				return;
			}

			if (strcmp(crstr, xtfont0cpright) == 0)
				CHECK;
			else {
				report("XA_COPYRIGHT was '%s',", crstr);
				report(" expecting '%s'", xtfont0cpright);
				FAIL;
			}

		} else {
			/* Compare value */
			if (fprop->card32 == val)
				CHECK;
			else {
				report("value of %s was %d, expecting %d",
					atomname(atom), val, fprop->card32);
				FAIL;
			}
		}
	}
	CHECKPASS(2*xtfont0.n_properties);

>>ASSERTION Good A
>># NOTE	kieron	Have to have defined fonts (bdf format)
>>#			loaded as the pre-defined properties (in X11/Xatom.h)
>>#			are likely, but not guaranteed, to be present on any
>>#			server.
When the property specified by the
.A atom
argument
is not defined,
then a call to xname returns
.S False .
>>STRATEGY
Use the XA_RGB_DEFAULT_MAP atom which is not defined in the xtest fonts,
(and is unlikely to be defined in a font..)
Verify that False is returned.
>>CODE
unsigned long	val;
Bool	ret;
int 	i;
extern	XFontStruct	xtfont0;	/* Known good version */

	font_struct = XLoadQueryFont(Dsp, "xtfont0");
	if (font_struct == NULL || isdeleted()) {
		delete("Could not load font, check that xtest fonts are installed");
		return;
	}

	value_return = &val;

	atom = XA_RGB_DEFAULT_MAP;

	ret = XCALL;

	if (ret != False)
		FAIL;
	else
		PASS;

>># HISTORY	kieron	Completed	Reformat and tidy to ca pass
