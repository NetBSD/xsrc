/* $XConsortium: wctxtprchr.m,v 1.2 94/04/17 21:14:22 rws Exp $ */
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

#define MAX_CHARS	32
#define MAX_TEST_STR	5
static char *test_str_list[MAX_TEST_STR] = {
	"T",
	"Te",
	"Tes",
	"Test",
	"Te\nst",
};

>>TITLE XwcTextPerCharExtents LC 
Status
XwcTextPerCharExtents(font_set,pwctest,nwchars,ink_return,logical_return,array_size,pnchars,overall_ink_return,overall_logical_return)
XFontSet font_set;
wchar_t *pwctest;
int nwchars;
XRectangle *ink_return;
XRectangle *logical_return;
int array_size;
int *pnchars;
XRectangle *overall_ink_return;
XRectangle *overall_logical_return;
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
xname returns the ink and logical extents of each character in addition 
to the overall extents of a string.
>>STRATEGY
For every Locale specified by the user in the configuration file, create
each of the base font sets specified by the user, by calling
XCreateFontSet, then call XwcTextPerCharExtents to get the extents per
character and the overall extents.  Check to make sure the extents are
valid. Also make sure the overall extents are equal to the combined
extents of the characters.  Make sure the returned status is good.
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
int escapement;
int nbytes,nchars,maxescape;
Status status;
XRectangle ink[MAX_CHARS];
XRectangle logical[MAX_CHARS];
XRectangle overall_ink;
XRectangle overall_logical;
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
						nchars = 0;
		
						ink_return = ink;
						logical_return = logical;
						array_size = MAX_CHARS;
						pnchars = (int *)&nchars;
						overall_ink_return = (XRectangle *)&overall_ink;
						overall_logical_return = (XRectangle *)&overall_logical;
						status = XCALL;
						if(status == 0)
						{
							report("Illegal escapement, %d, for string %s",
								escapement,test_str_list[j]);
							FAIL;
						}
						else
						{
							trace("Extents for string %s, fontset %s, in locale %s",
								test_str_list[j],font_list,plocale);

							if(nchars <= 0)
							{
								report("No characters returned");
								FAIL;
							}
							else
							{
							int k;
							int wcnt, hcnt;
								wcnt = 0;
								hcnt = ink[0].height;
								trace("---Ink     [overall] wid=%d, ht=%d",
									overall_ink.width, overall_ink.height);
								for(k=0;k<nchars;k++)
									wcnt += ink[k].width;
								trace("---Ink     [char   ] wid=%d, ht=%d",
									wcnt,hcnt);
								if((wcnt > (int)overall_ink.width) || 
								   (hcnt > (int)overall_ink.height))
								{
									report("Ink extents of chars (%d,%d) is greater than overall extents (%d,%d) for string %s",
										wcnt,hcnt,
										overall_ink.width,
										overall_ink.height,
										test_str_list[j]);
									FAIL;
								}
								else
									CHECK;
							
								wcnt = 0;
								hcnt = logical[0].height;
								trace("---Logical [overall] wid=%d, ht=%d",
									overall_ink.width, overall_ink.height);
								for(k=0;k<nchars;k++)
									wcnt += logical[k].width;
								trace("---Logical [char   ] wid=%d, ht=%d",
									wcnt,hcnt);
								if((wcnt > (int)overall_logical.width) || 
								   (hcnt > (int)overall_logical.height))
								{
									report("Logical extents of chars (%d,%d) is greater than overall extents (%d,%d) for string %s",
										wcnt,hcnt,
										overall_logical.width,
										overall_logical.height,
										test_str_list[j]);
									FAIL;
								}
								else
									CHECK;
							}
							if((overall_ink.width > overall_logical.width) ||
							   (overall_ink.height > overall_logical.height))
							{
								report("Ink extents are greater than logical extents of fontset, %s in locale %s",
									font_list,plocale);
								FAIL;
							}
							else
								CHECK;
						}
					}
				}
				XFreeFontSet(dpy,pfs);
				XFreeStringList(missing_chars);
			}
		}
	}
	
	CHECKPASS(nlocales()+3*MAX_TEST_STR*nlocales()*nfontset());
