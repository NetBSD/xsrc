/* $XConsortium: wcdrwimgst.m,v 1.2 94/04/17 21:14:26 rws Exp $ */
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

>>TITLE XwcDrawImageString LC 
void
XmbDrawImageString(display,d,font_set,gc,x,y,wstring,length) 
Display	*display = Dsp;
Drawable d;
XFontSet font_set;
GC		gc;
int 	x = 4;
int 	y = 20;
wchar_t *wstring = wstr;
int		length;
>>EXTERN
static char *str = "A bCdElMnO";
static wchar_t wstr[128];
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
A call to xname first fills the destination rectangle with the
.M background
pixel value and next draws a
.A wstring
of 8-bit characters, selected from the
.A gc 's
.M font ,
using the
.M foreground
pixel value.
>>STRATEGY
Reverse foreground and background pixel values in the gc.
For each font
  Set the font into the gc.
  Draw string
  Pixel verify.
>>CODE
Display *dpy;
char *plocale;
XVisualInfo *vp;
char *font_list;
XFontSet pfs;
char *fontset;
char *defstr;
int missing_cnt;
char **missing_chars;

	dpy = Dsp;
	length = strlen(str);
	mbstowcs(wstr,str,length);

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

		if (!linklocale(plocale))
		{
			untested("Couldn't create data link.");
			FAIL;
			continue;
		}

		resetvinf(VI_WIN_PIX); 
		if(nextvinf(&vp))
		{
			d = makewin(display, vp);
			gc = makegc(display, d);

			XSetForeground(display,gc,W_BG);
			XSetBackground(display,gc,W_FG);

			/* cycle through the fontsets */
			resetfontset();
			while(nextfontset(&font_list))
			{
				trace("Font Set %s", font_list);
				pfs = XCreateFontSet(dpy,font_list,&missing_chars,
					&missing_cnt,&defstr);
				if(pfs == NULL)
				{
					report("XCreateFontSet unable to create fontset, %s",
						font_list);
					FAIL;
					continue;
				}

				font_set = pfs;
				XCALL;
				PIXCHECK(display, d);
				dclear(display, d);

				XFreeFontSet(dpy,pfs);
				XFreeStringList(missing_chars);
			}	/* nextvinf */
		}	/* nextfontset */
	}	/* nextlocale */
	unlinklocales();

	CHECKPASS(nlocales()+nlocales()*nfontset());
