/* $XConsortium: gtimvls.m,v 1.2 94/04/17 21:13:57 rws Exp $ */
/*

Copyright (c) 1993  X Consortium

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
 * Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.
 *
 *                   All Rights Reserved
 *
 * Permission  to  use,  copy,  modify,  and  distribute   this
 * software  and  its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright no-
 * tice  appear  in all copies and that both that copyright no-
 * tice and this permission notice appear in  supporting  docu-
 * mentation,  and  that the name of Sun not be used in
 * advertising or publicity pertaining to distribution  of  the
 * software  without specific prior written permission. Sun 
 * makes no representations about the suitability of this
 * software for any purpose. It is provided "as is" without any
 * express or implied warranty.
 *
 * SUN DISCLAIMS ALL WARRANTIES WITH REGARD TO  THIS  SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FIT-
 * NESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL SUN BE  LI-
 * ABLE  FOR  ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
 * ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,  DATA  OR
 * PROFITS,  WHETHER  IN  AN  ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION  WITH
 * THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
>>EXTERN
#include <locale.h>
#include <ximtest.h>

unsigned long all_styles = XIMPreeditArea|XIMPreeditCallbacks|\
			XIMPreeditPosition|XIMPreeditNothing|XIMPreeditNone|\
         XIMStatusArea|XIMStatusCallbacks|XIMStatusNothing|\
         XIMStatusNone;

>>TITLE XGetIMValues IM	
char *

XIM im = NULL;
char *im_name = XNQueryInputStyle;
XIMStyles **pstyle = NULL;
int end_of_list = NULL;
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
A call to xname presents a variable argument list programming interface 
for querying properties or features of the specified input method. This 
function returns NULL if successful, otherwise it returns the name of the 
first argument that could not be obtained.  The only standard argument 
defined by Xlib is XNQueryInputStyle.
>>STRATEGY
For all locales, open an input method, call XGetIMValues using 
XNQueryInputStyle and check the return value for non-null, also check
the styles returned for valid entries and non-zero.
>>CODE
char *plocale;
char *ret;
unsigned long mask = 0;
unsigned long val = 0;
XrmDatabase db = NULL;
XIMStyles *style = NULL;

	XrmInitialize();

	resetlocale();
	while(nextlocale(&plocale))
	{
		if (locale_set(plocale))
			CHECK;
		else
		{
			report("Couldn't set locale.");
			FAIL;
			continue;
		}

		cleanup_locale(style,NULL,im,db);

		db = rm_db_open();
		if(db != NULL)
			CHECK;
		else
		{
			report("Couldn't open database.");
			FAIL;
			continue;
		}

		im = im_open(db);
		if(im != NULL)
			CHECK;
		else
		{
			report("Couldn't open imput method.");
			FAIL;
			continue;
		}

		pstyle = (XIMStyles **)&style;
		ret = XCALL; 
		if(ret != NULL)
		{
			report("%s failed to return all IM values requested",
					TestName);
			report("  starting with value, %s", ret);
			FAIL;
		}
		else
		{
			/* check all the styles which were returned */
			/* if negative, zero, or greater than number of bits */
			if(style->count_styles <= 0l || style->count_styles >= 32)
			{
				report("Illegal number of input styles returned by %s",
					TestName);
				FAIL;
			}
			else
			{
				/* see if any illegal bits are set */
				mask = all_styles | *style->supported_styles;
				val = mask ^ all_styles;
				if(val != 0)
				{
					report("Illegal styles supported, 0x%x, in locale %s.",
						*style->supported_styles,plocale);
					report("    Legal values 0x%x",all_styles);
					FAIL;
				}
				else
					CHECK;
			}				
		}

	}	/* nextlocale */
	cleanup_locale(style,NULL,im,db);

	CHECKPASS(4*nlocales());
