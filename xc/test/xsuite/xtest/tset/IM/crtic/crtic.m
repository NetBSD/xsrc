/* $XConsortium: crtic.m,v 1.2 94/04/17 21:14:01 rws Exp $ */
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

>>TITLE XCreateIC IM
XIC

XIM im;
char *ic_win = XNClientWindow;
Window win;
char *ic_style = XNInputStyle;
XIMStyle which_style;
char *ic_preedit = XNPreeditAttributes;
XVaNestedList preedit_list;
char *ic_status = XNStatusAttributes;
XVaNestedList status_list;
int end_list = NULL;
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
A call to xname opens a input context,
.A ic,
to the input method server
.A im

>>STRATEGY

>>EXTERN
static char *style_name[] = {
	"Preedit Area",
	"Preedit Callbacks",
	"Preedit Position",
	"Preedit Nothing",
	"Preedit None",
	"", "", "",
	"Status Area",
	"Status Callbacks",
	"Status Nothing",
	"Status None",
};


/* some callbacks procedures */
static void
preedit_start()
{
}

static void
preedit_draw()
{
}

static void
preedit_done()
{
}

static void
preedit_caret()
{
}

/* some status callback procedures */
static void
status_start()
{
}

static void
status_draw()
{
}

static void
status_done()
{
}

>>CODE
XrmDatabase db;
char *plocale;
XFontSet fs;
XIMStyles *style;
XPoint spot_loc;
XIMCallback pecb_start,pecb_draw,pecb_done,pecb_caret;
XIMCallback stcb_start,stcb_draw,stcb_done;
int dummy;
XRectangle area;
int nstyles = 0; 
XIC ic;
int pe_cnt,st_cnt;

	XrmInitialize();

	db = NULL;
	im = NULL;
	fs = NULL;
	style = NULL;

	pecb_start.callback = (XIMProc)preedit_start;
	pecb_start.client_data = (XPointer)IM_PE_CALLBACKS;
	pecb_draw.callback = (XIMProc)preedit_draw;
	pecb_draw.client_data = (XPointer)IM_PE_CALLBACKS;
	pecb_done.callback = (XIMProc)preedit_done;
	pecb_done.client_data = (XPointer)IM_PE_CALLBACKS;
	pecb_caret.callback = (XIMProc)preedit_caret;
	pecb_caret.client_data = (XPointer)IM_PE_CALLBACKS;

	stcb_start.callback = (XIMProc)status_start;
	stcb_start.client_data = (XPointer)IM_STATUS_CALLBACKS;
	stcb_draw.callback = (XIMProc)status_draw;
	stcb_draw.client_data = (XPointer)IM_STATUS_CALLBACKS;
	stcb_done.callback = (XIMProc)status_done;
	stcb_done.client_data = (XPointer)IM_STATUS_CALLBACKS;

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

		cleanup_locale(style,fs,im,db);

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
			report("Couldn't open input method.");
			FAIL;
			continue;
		}

		if(ic_setup(&win,&fs))
			CHECK;
		else
		{
			report("Couldn't setup input styles.");
			FAIL;
			continue;
		}

		/* get the input styles */
		reset_ic_style(im);
		nstyles += n_ic_styles();
		while(next_ic_style(&which_style))
		{
			preedit_list = NULL;
			status_list = NULL;
			pe_cnt = 0;
			st_cnt = 0;

			if(which_style & XIMPreeditCallbacks)
			{
				pe_cnt++;
				preedit_list = XVaCreateNestedList(dummy,
						XNPreeditStartCallback, &pecb_start,
						XNPreeditDrawCallback,  &pecb_draw,
						XNPreeditDoneCallback,  &pecb_done,
						XNPreeditCaretCallback, &pecb_caret,
						NULL);
			}

			if(which_style & XIMPreeditPosition)
			{
				pe_cnt++;
				spot_loc.x = 10;
				spot_loc.y = 10;
				preedit_list = XVaCreateNestedList(dummy,
						XNSpotLocation,&spot_loc,
						XNFontSet,fs,
						NULL);
			}

			if(which_style & XIMPreeditArea)
			{
				pe_cnt++;
				area.x = 0;
				area.y = 0;
				area.width = 50;
				area.height = 50;
				preedit_list = XVaCreateNestedList(dummy,
						XNArea,&area,
						XNFontSet,fs,
						NULL);
			}

			if(which_style & XIMPreeditNothing)
				pe_cnt++;

			if(which_style & XIMPreeditNone)
				pe_cnt++;

			if(which_style & XIMStatusArea)
			{
				st_cnt++;
				/* create a status list */
				area.x = 0;
				area.y = 0;
				area.width = 50;
				area.height = 50;
				status_list = XVaCreateNestedList(dummy,
						XNArea,&area,
						XNFontSet,fs,
						NULL);
			}

			if(which_style & XIMStatusCallbacks)
			{
				st_cnt++;
				status_list = XVaCreateNestedList(dummy,
						XNStatusStartCallback, stcb_start,
						XNStatusDrawCallback,  stcb_draw,
						XNStatusDoneCallback,  stcb_done,
						NULL);
			}

			if(which_style & XIMStatusNothing)
				st_cnt++;

			if(which_style & XIMStatusNone)
				st_cnt++;

			if(st_cnt == 0)
			{
				report("No input status styles specified");
				FAIL;
				continue;
			}

			if(pe_cnt == 0)
			{
				report("No input preedit styles specified");
				FAIL;
				continue;
			}

			if(st_cnt > 1)
			{
				report("Too many status styles (%d) specified.", st_cnt);
				FAIL;
				continue;
			}

			if(pe_cnt > 1)
			{
				report("Too many preedit styles (%d) specified.", pe_cnt);
				FAIL;
				continue;
			}

			startcall(Dsp);
			if (isdeleted())
				return;
			if (preedit_list == NULL)
				if (status_list == NULL)
            	ic = XCreateIC(im, ic_win, win, ic_style, which_style, NULL);
				else ic = XCreateIC(im, ic_win, win, ic_style, which_style, 
											ic_status, status_list, NULL);
			else
			{ 
				if (status_list == NULL)
            	ic = XCreateIC(im, ic_win, win, ic_style, which_style,
										ic_preedit, preedit_list, NULL);
				else ic = XCreateIC(im, ic_win, win, ic_style, which_style, 
										ic_preedit, preedit_list, 
										ic_status, status_list, NULL);
			}
			endcall(Dsp);
			if (geterr() != Success)
			{
				report("Got %s, Expecting Success", errorname(geterr()));
				FAIL;
			}     

			if(ic == NULL)
			{
				report("Unable to create input context for locale, %s",
					plocale);
				FAIL;
			}
			else
			{
				CHECK;
				XDestroyIC(ic);
			}
			if(preedit_list != NULL)
				XFree(preedit_list);
			if(status_list != NULL)
				XFree(status_list);
		}
	}   /* nextlocale */
	cleanup_locale(style,fs,im,db);

   CHECKPASS(4*nlocales()+nstyles);
