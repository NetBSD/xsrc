/* $XConsortium: wctxtextnt.m,v 1.2 94/04/17 21:14:20 rws Exp $ */
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

#define MAX_TEST_STR	4
static char *test_str_list[MAX_TEST_STR] = {
	"T",
	"Te",
	"Tes",
	"Test",
};

>>TITLE XwcTextExtents LC 
int
XwcTextExtents(font_set,pwctest,nwchars,overall_ink,overall_logical)
XFontSet font_set;
wchar_t *pwctest;
int nwchars;
XRectangle *overall_ink;
XRectangle *overall_logical;
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
xname returns the overall ink and logical extents in addtion to the 
escapement of a string for a given fontset.
>>STRATEGY
For every Locale specified by the user in the configuration file, create
each of the base font sets specified by the user, by calling
XCreateFontSet, then call XwcTextExtents to get the overall ink and 
logical extents of a string.  Check the extents to make sure that they are
positive and that the ink extents are equal to or smaller than the 
logical extents. 
>>CODE
Display *dpy;
char *plocale;
char *fontset;
char *font_list;
XFontSet pfs;
char *defstr;
int missing_cnt;
char **missing_chars;
XFontSetExtents *ext;
int escapement,nbytes,maxescape;
XRectangle ink,logical;
wchar_t wcharstr[512];

	resetlocale();
	dpy = Dsp;
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

		/* cycle through the fontsets */
		resetfontset();
		while(nextfontset(&font_list))
		{
			pfs = XCreateFontSet(dpy,font_list,&missing_chars,
				&missing_cnt,&defstr);
			if(pfs == NULL)
			{
				report("XCreateFontSet unable to create fontset, %s",
					font_list);
				FAIL;
			}
			else
			{
				trace("Created Font Set %s", font_list);
				font_set = pfs;

				ext = XExtentsOfFontSet(pfs);
				if(ext == NULL)
				{
					report("Extents of fontset %s returns null record for locale, %s",
						font_list,plocale);
					FAIL;
				}
				else
				{
				int j;
					trace("Extents for fontset, %s, in locale %s:",
						font_list,plocale);
					trace("    Max Ink     Extents - (%d,%d) w=%d, h=%d",
						ext->max_ink_extent.x,
						ext->max_ink_extent.y,
						ext->max_ink_extent.width,
						ext->max_ink_extent.height);
					trace("    Max Logical Extents - (%d,%d) w=%d, h=%d",
						ext->max_logical_extent.x,
						ext->max_logical_extent.y,
						ext->max_logical_extent.width,
						ext->max_logical_extent.height);

					maxescape = 0;
					for(j=0;j<MAX_TEST_STR;j++)
					{
						nwchars = mbstowcs(wcharstr,test_str_list[j],128);
						pwctest = wcharstr;

						overall_ink = (XRectangle *)&ink;
						overall_logical = (XRectangle *)&logical;
						escapement = XCALL;
						if(escapement <= 0)
						{
							report("Illegal escapement, %d, for string %s",
								escapement,test_str_list[j]);
							FAIL;
							continue;
						}
						else
						{
							trace("Extents for fontset, %s, in locale %s:",
								font_list,plocale);
							trace("    Ink     Overall Extents - (%d,%d) w=%d, h=%d",
								ink.x,ink.y,ink.width,ink.height);
							trace("    Logical Overall Extents - (%d,%d) w=%d, h=%d",
								logical.x,logical.y,
								logical.width,logical.height);
							CHECK;
						}
						if(escapement < maxescape)
						{
						/* as long as the number of characters */
						/* is increasing, the escapment should */
						/* be increasing */
							trace("Escapement size decrease %d max=%d for longer string, %s",
								escapement,maxescape,test_str_list[j]);

							FAIL;
						}
						else
							CHECK;
						if(escapement > maxescape)
								maxescape = escapement;
						trace("Escapement %d for string, %s",
								escapement,test_str_list[j]);

						/* make sure the extents are valid */
						if(ink.width <= 0 || ink.height <= 0)
						{
							report("Bad ink extents for fontset %s",
									font_list);
							FAIL;
						}
						if(logical.width <= 0 || logical.height <= 0)
						{
							report("Bad logical extents for fontset %s",
								font_list);
							FAIL;
						}
						if((ink.width  > logical.width) ||
							   (ink.height > logical.height))
						{
							report("Ink extents are greater than logical extents of fontset, %s in locale %s",
								font_list,plocale);
							FAIL;
						}
					}
				}
				XFreeFontSet(dpy,pfs);
				XFreeStringList(missing_chars);
			}
		}
	}

	CHECKPASS(nlocales()+2*MAX_TEST_STR*nlocales()*nfontset());
