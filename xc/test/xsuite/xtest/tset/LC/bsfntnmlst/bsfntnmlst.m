/* $XConsortium: bsfntnmlst.m,v 1.1 94/01/29 16:02:22 rws Exp $ */
/*
 * Copyright 1993 by Sun Microsystems, Inc. Mountain View, CA.
 *
 *                   All Rights Reserved
 *
 * Permission  to  use,  copy,  modify,  and  distribute   this
 * software  and  its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright no-
 * tice  appear  in all copies and that both that copyright no-
 * tice and this permission notice appear in  supporting  docu-
 * mentation,  and  that the names of Sun or MIT not be used in
 * advertising or publicity pertaining to distribution  of  the
 * software  without specific prior written permission. Sun and
 * M.I.T. make no representations about the suitability of this
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

>>TITLE XBaseFontNameListOfFontSet LC 
char *
XBaseFontNameListOfFontSet (font_set)
XFontSet font_set;
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
Given a font set, xname, returns the name of the base font name list
used when the XFontSet was created.  
>>STRATEGY
For every Locale specified by the user in the configuration file, create
each of the base font sets specified by the user, by calling
XCreateFontSet, then call XBaseFontNameListOfFontSet to get the
base font name, this name should be the same name supplied to the
XCreateFontSet call. 
>>CODE
Display *dpy;
char *plocale;
char *font_list;
XFontSet pfs;
char *fontset;
char *defstr;
int missing_cnt;
char **missing_chars;
char *base_font_name;

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
				trace("    default string %s",defstr);
				trace("    %d missing chars",missing_cnt);
				font_set = pfs;

				base_font_name = XCALL;
				if(strcmp(base_font_name,font_list) != 0)
				{
					report("Base font set %s not same as fontset name %s",
						base_font_name,font_list);
					FAIL;
				}
				else
				{
					trace("Base font set %s matches %s",
						base_font_name,font_list);
					CHECK;
				}
				XFreeFontSet(dpy,pfs);
				XFreeStringList(missing_chars);
			}
		}
	}
	
	CHECKPASS(nlocales()+nlocales()*nfontset());
