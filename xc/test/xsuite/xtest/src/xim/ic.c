/* $XConsortium: ic.c,v 1.2 94/04/17 21:01:44 rws Exp $ */
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

#include    "xtest.h"
#include    "Xlib.h"
#include    "Xutil.h"
#include    "Xresource.h"
#include    "xtestlib.h"
#include    "tet_api.h"
#include    "pixval.h"
#include    <string.h>
#include    "ximtest.h"

#define IM_GEOM_CALLBACKS	1

#define ICCB_START  0
#define ICCB_DONE   1
#define ICCB_DRAW   2
#define ICCB_CARET  3
#define ICCB_MAX    4

int iccb_preedit_cnt[ICCB_MAX];
int iccb_status_cnt[ICCB_MAX];
int iccb_geom_cnt;

extern Display *Dsp;

static XIMStyles *style = NULL;
static int num_styles;
static int cur_style;
static XFontSet fs;
static XIMCallback pecb_start,pecb_draw,pecb_done,pecb_caret;
static XIMCallback stcb_start,stcb_draw,stcb_done;
static XIMCallback gmcb;

static XVaNestedList preedit_list = NULL;
static XVaNestedList status_list = NULL;


/* some callbacks procedures */
void
iccb_preedit_start(ic,client,call_data)
	XIC ic;
	XPointer client;
	XPointer call_data;
{
	iccb_preedit_cnt[ICCB_START]++;
	if(ic == NULL)
	{
		report("Null ic passed to preedit_start callback");
		return;
	}
	if(call_data != NULL)
	{
		report("Non-null call data in preedit_start callback");
		return;
	}
}

void
iccb_preedit_draw(ic,client,call_data)
	XIC ic;
	XPointer client;
	XPointer call_data;
{
	iccb_preedit_cnt[ICCB_DRAW]++;
	if(ic == NULL)
	{
		report("Null ic passed to preedit_draw callback");
		return;
	}
	if(call_data == NULL)
	{
		report("Null call data in preedit_draw callback");
		return;
	}
}

void
iccb_preedit_done(ic,client,call_data)
	XIC ic;
	XPointer client;
	XPointer call_data;
{
	iccb_preedit_cnt[ICCB_DONE]++;
	if(ic == NULL)
	{
		report("Null ic passed to preedit_done callback");
		return;
	}
	if(call_data != NULL)
	{
		report("Non-null call data in preedit_done callback");
		return;
	}
}

void
iccb_preedit_caret(ic,client,call_data)
	XIC ic;
	XPointer client;
	XPointer call_data;
{
	iccb_preedit_cnt[ICCB_CARET]++;
	if(ic == NULL)
	{
		report("Null ic passed to preedit_caret callback");
		return;
	}
	if(call_data == NULL)
	{
		report("Null call data in preedit_caret callback");
		return;
	}
}

/* some status callback procedures */
void
iccb_status_start(ic,client,call_data)
	XIC ic;
	XPointer client;
	XPointer call_data;
{
	iccb_status_cnt[ICCB_START]++;
	if(ic == NULL)
	{
		report("Null ic passed to status_start callback");
		return;
	}
	if(call_data != NULL)
	{
		report("Non-null call data in status_start callback");
		return;
	}
}

void
iccb_status_draw(ic,client,call_data)
	XIC ic;
	XPointer client;
	XPointer call_data;
{
	iccb_status_cnt[ICCB_DRAW]++;
	if(ic == NULL)
	{
		report("Null ic passed to status_draw callback");
		return;
	}
	if(call_data == NULL)
	{
		report("Null call data in status_draw callback");
		return;
	}
}

void
iccb_status_done(ic,client,call_data)
	XIC ic;
	XPointer client;
	XPointer call_data;
{
	iccb_status_cnt[ICCB_DONE]++;
	if(ic == NULL)
	{
		report("Null ic passed to status_done callback");
		return;
	}
	if(call_data != NULL)
	{
		report("Non-null call data in status_done callback");
		return;
	}
}

void
iccb_geom(ic,client,call_data)
	XIC ic;
	XPointer client;
	XPointer call_data;
{
	iccb_geom_cnt++;
	if(ic == NULL)
	{
		report("Null IC passed to geometry callback");
		return;
	}
	if(call_data != NULL)
	{
		report("Non-null call data in geometry callback");
		return;
	}
}

XrmDatabase rm_db_open()
{
    XrmDatabase db;

    /* create a resource database */
    db = XrmGetStringDatabase("");
    if(db == (XrmDatabase)NULL)
    {   
		delete("Could not create target database.");
		UNTESTED;
		return(NULL);
    }   
    trace("Opened resource database");
    return(db);
}
 
XIM im_open(db)
	XrmDatabase db;
{
	XIM im;

	/* open an input method */
	im = XOpenIM(Dsp,db,NULL,NULL);
	if(im == NULL)
	{
		report("Unable to open an input method");
		UNTESTED;
		return(NULL);
	}
	return(im);
}

Bool ic_setup(pwin,pfs)
	Window *pwin;
	XFontSet *pfs;
{
	XVisualInfo *vp;
	Window win;
	char *fsname;
	int missing_cnt;
	char **missing_chars;
	char *defstr;

	fs = NULL;
	/* open a window */
	resetvinf(VI_WIN);
	nextvinf(&vp);
	win = makewin(Dsp,vp);
	if(win == NULL)
	{
		report("Unable to openwin window");
		UNTESTED;
		return(False);
	}

	/* create a Fontset */
	resetfontset();
	nextfontset(&fsname);
	fs = XCreateFontSet(Dsp,fsname,&missing_chars,&missing_cnt,&defstr);
	if(fs == NULL)
	{
		report("Unable to open fontset, %s",fsname);
		UNTESTED;
		return(False);
	}

	pecb_start.callback = (XIMProc)iccb_preedit_start;
	pecb_start.client_data = (XPointer)IM_PE_CALLBACKS;
	pecb_draw.callback = (XIMProc)iccb_preedit_draw;
	pecb_draw.client_data = (XPointer)IM_PE_CALLBACKS;
	pecb_done.callback = (XIMProc)iccb_preedit_done;
	pecb_done.client_data = (XPointer)IM_PE_CALLBACKS;
	pecb_caret.callback = (XIMProc)iccb_preedit_caret;
	pecb_caret.client_data = (XPointer)IM_PE_CALLBACKS;

	stcb_start.callback = (XIMProc)iccb_status_start;
	stcb_start.client_data = (XPointer)IM_STATUS_CALLBACKS;
	stcb_draw.callback = (XIMProc)iccb_status_draw;
	stcb_draw.client_data = (XPointer)IM_STATUS_CALLBACKS;
	stcb_done.callback = (XIMProc)iccb_status_done;
	stcb_done.client_data = (XPointer)IM_STATUS_CALLBACKS;

	gmcb.callback = (XIMProc)iccb_geom;
	gmcb.client_data = (XPointer)IM_GEOM_CALLBACKS;

	*pwin = win;
	*pfs = fs;
	return(True);
}

Bool reset_ic_style(im)
	XIM im;
{
	char *pval;

	if(style != NULL)
		XFree(style);
	/* get the input styles */
	pval = XGetIMValues(im,XNQueryInputStyle,&style,NULL);
	cur_style = 0;
	num_styles = style->count_styles;
	trace("There are %d styles for this IM",num_styles);
	return(True);
}

Bool next_ic_style(which_style)
	XIMStyle *which_style;
{
	int i;

	for(i=0;i<ICCB_MAX;i++)
	{
		iccb_preedit_cnt[i] = 0;
		iccb_status_cnt[i] = 0;
	}
	iccb_geom_cnt = 0;

	if(cur_style >= num_styles)
		return(False);
	*which_style = style->supported_styles[cur_style++]; 

	return(True);
}

/*
 * Returns the number of times that next_ic_style will succeed. Only valid
 * after a call to reset_ic_style().
 */
int
n_ic_styles()
{
   return(num_styles);
}

XIC ic_open(im,win,which_style)
	XIM im;
	Window win;
	XIMStyle which_style;
{
	int pe_cnt,st_cnt,dummy;
	XPoint spot_loc;
	XRectangle area;
	XIC ic;

	trace("Creating input context input style, 0x%x",which_style);

	if(preedit_list != NULL)
		XFree(preedit_list);
	if(status_list != NULL)
		XFree(status_list);

	preedit_list = NULL;
	status_list = NULL;
	pe_cnt = 0;
	st_cnt = 0;
	ic = NULL;

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
		return(NULL);
	}

	if(pe_cnt == 0)
	{
		report("No input preedit styles specified");
		return(NULL);
	}

	if(st_cnt > 1)
	{
		report("Too many status styles (%d) specified, style = 0x%x",
			st_cnt,which_style);
		return(NULL);
	}

	if(pe_cnt > 1)
	{
		report("Too many preedit styles (%d) specified, style = 0x%x",
			pe_cnt,which_style);
		return(NULL);
	}

	if(status_list == NULL && preedit_list == NULL)
	{
		ic = XCreateIC(im, 
					XNClientWindow, win, 
					XNInputStyle, which_style, 
					XNResourceName, "im_rname",
					XNResourceClass, "IM_rclass",
					XNGeometryCallback,&gmcb,
					NULL);
	}
	else if(status_list == NULL)
	{
		ic = XCreateIC(im, 
					XNClientWindow, win, 
					XNInputStyle, which_style, 
					XNResourceName, "im_rname",
					XNResourceClass, "IM_rclass",
					XNGeometryCallback,&gmcb,
					XNPreeditAttributes, preedit_list, 
					NULL);
	}
	else if(preedit_list == NULL)
	{
		ic = XCreateIC(im, 
					XNClientWindow, win, 
					XNInputStyle, which_style, 
					XNResourceName, "im_rname",
					XNResourceClass, "IM_rclass",
					XNGeometryCallback,&gmcb,
					XNStatusAttributes, status_list, 
					NULL);
	}
	else
	{
		ic = XCreateIC(im, 
					XNClientWindow, win, 
					XNInputStyle, which_style, 
					XNResourceName, "im_rname",
					XNResourceClass, "IM_rclass",
					XNGeometryCallback,&gmcb,
					XNStatusAttributes, status_list, 
					XNPreeditAttributes, preedit_list, 
					NULL);
	}
	trace("Returning IC = 0x%x,pe_cnt = %d, st_cnt = %d",
		ic,pe_cnt,st_cnt);
	trace("    Preedit list = 0x%x, Status list = 0x%x",
		preedit_list,status_list);

	return(ic);
}

void ic_close(ic)
	XIC ic;
{
	if(preedit_list != NULL)
		XFree(preedit_list);
	if(status_list != NULL)
		XFree(status_list);
	preedit_list = NULL;
	status_list = NULL;
	if(ic != NULL)
		XDestroyIC(ic);
}

ic_get_cb(pe_cb,status_cb,geom_cb)
	XIMCallback pe_cb[];
	XIMCallback status_cb[];
	XIMCallback *geom_cb;
{
	
    pe_cb[ICCB_START].callback = iccb_preedit_start;
    pe_cb[ICCB_DONE].callback  = iccb_preedit_done;
    pe_cb[ICCB_DRAW].callback  = iccb_preedit_draw;
    pe_cb[ICCB_CARET].callback = iccb_preedit_caret;

    pe_cb[ICCB_START].client_data = (XPointer)IM_PE_CALLBACKS; 
    pe_cb[ICCB_DONE].client_data  = (XPointer)IM_PE_CALLBACKS; 
    pe_cb[ICCB_DRAW].client_data  = (XPointer)IM_PE_CALLBACKS;
    pe_cb[ICCB_CARET].client_data = (XPointer)IM_PE_CALLBACKS;


    status_cb[ICCB_START].callback = iccb_status_start;
    status_cb[ICCB_DONE].callback  = iccb_status_done;
    status_cb[ICCB_DRAW].callback  = iccb_status_draw;
    status_cb[ICCB_CARET].callback = NULL;

    status_cb[ICCB_START].client_data = (XPointer)IM_STATUS_CALLBACKS;
    status_cb[ICCB_DONE].client_data  = (XPointer)IM_STATUS_CALLBACKS;
    status_cb[ICCB_DRAW].client_data  = (XPointer)IM_STATUS_CALLBACKS;
    status_cb[ICCB_CARET].client_data = NULL;

    geom_cb->callback = iccb_geom;
    geom_cb->client_data = (XPointer)IM_GEOM_CALLBACKS;
}
