/* $XConsortium: xim_cb.c,v 1.2 94/04/17 21:01:49 rws Exp $ */
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

#include    <stdio.h>
#include    <string.h>

#include    "xtest.h"
#include    "Xlib.h"
#include    "Xutil.h"
#include		"Xresource.h"
#include    "xtestlib.h"
#include    "tet_api.h"
#include    "pixval.h"

#include	"ximtest.h"

extern Display *Dsp;

static	XIMCallback cb_rec[CB_MAX];
static	XIMStyle	xim_style;
static	XIM			xim_im;
static	XIC			xim_ic;
static	Window		xim_win;
static	XrmDatabase xim_db; 
static	XFontSet	xim_fs;

static XVaNestedList preedit_list = NULL;
static XVaNestedList status_list = NULL;

/****************************************************************/
/* these stacks are for the real data */
/* so that a comparison to expected data can be performed */ 
/* see xim_response.c for further explanation */
static cbstk_def cbstk_actual;

/****************************************************************/
/* some callbacks procedures */
XIMText *xim_copy_ximtext(pt)
	XIMText *pt;
{
	int cnt;
	XIMText *nt;

	if(pt == NULL)
		return(NULL);

	nt = (XIMText *)malloc(sizeof(XIMText));
	memcpy(nt,pt,sizeof(XIMText));

	/* copy over the other pieces */
	cnt = pt->length;
	nt->feedback = (XIMFeedback *)malloc(sizeof(XIMFeedback) * cnt);
	memcpy(nt->feedback,pt->feedback,cnt);

	if(pt->encoding_is_wchar)
	{
		nt->string.wide_char = (wchar_t *)malloc(sizeof(wchar_t) * cnt);
		memcpy(nt->string.wide_char,pt->string.wide_char,cnt);
	}
	else
	{
		nt->string.multi_byte = (char *)malloc(sizeof(char) * cnt);
		memcpy(nt->string.multi_byte,pt->string.multi_byte,cnt);
	}
	return(nt);
}

Pixmap xim_copy_pixmap(pm)
	Pixmap pm;
{
	report("copying pixmap not implemented");
	return(NULL);
}

void 
xim_cb_preedit_start(ic,client,call_data)
	XIC ic;
	int client;
	XPointer call_data;
{
	int which_cb;

	which_cb = CB_PE_START;

	if(client != which_cb)	
		report("Bad client data: preedit_start callback, expected %d, got %d",
			which_cb,client);
	if(ic == NULL)
		report("Null ic passed to preedit_start callback");
	if(call_data != NULL)
		report("Non-null call data in preedit_start callback");

	/* push this info onto the stack for later compare */
	/* call_data should have been NULL, force it */
	xim_response_push_cb(&cbstk_actual,which_cb,NULL);
}

void
xim_cb_preedit_draw(ic,client,call_data)
	XIC ic;
	int client;
	XIMPreeditDrawCallbackStruct *call_data;
{
	int which_cb;
	XIMPreeditDrawCallbackStruct *data;

	which_cb = CB_PE_DRAW;

	/* push this info onto the stack for later compare */
	/* copy the data first and the other allocated pieces */
	data = (XIMPreeditDrawCallbackStruct *)malloc(
		sizeof(XIMPreeditDrawCallbackStruct));

	memcpy(data,call_data,sizeof(XIMPreeditDrawCallbackStruct));
	/* allocate and copy the text */
	data->text = xim_copy_ximtext(call_data->text);

	xim_response_push_cb(&cbstk_actual,which_cb,data);

	if(client != which_cb)	
		report("Bad client data: preedit_draw callback, expected %d, got %d",
			which_cb,client);
	if(ic == NULL)
		report("Null ic passed to preedit_draw callback");
	if(call_data == NULL)
		report("Null call data in preedit_draw callback");
}

void
xim_cb_preedit_done(ic,client,call_data)
	XIC ic;
	int client;
	XPointer call_data;
{
	int which_cb;

	which_cb = CB_PE_DONE;

	if(client != which_cb)	
		report("Bad client data: preedit_done callback, expected %d, got %d",
			which_cb,client);
	if(ic == NULL)
		report("Null ic passed to preedit_done callback");
	if(call_data != NULL)
		report("Non-null call data in preedit_done callback");

	/* push this info onto the stack for later compare */
	/* call_data should have been NULL, force it */
	xim_response_push_cb(&cbstk_actual,which_cb,NULL);
}

void
xim_cb_preedit_caret(ic,client,call_data)
	XIC ic;
	int client;
	XPointer call_data;
{
	int which_cb;
	XIMPreeditCaretCallbackStruct *data;

	which_cb = CB_PE_CARET;
	/* push this info onto the stack for later compare */
	/* copy the data first and the other allocated pieces */
	data = (XIMPreeditCaretCallbackStruct *)malloc(
		sizeof(XIMPreeditCaretCallbackStruct));
	memcpy(data,call_data,sizeof(XIMPreeditCaretCallbackStruct));

	xim_response_push_cb(&cbstk_actual,which_cb,data);

	if(client != which_cb)	
		report("Bad client data: preedit_caret callback, expected %d, got %d",
			which_cb,client);
	if(ic == NULL)
		report("Null ic passed to preedit_caret callback");
	if(call_data == NULL)
		report("Null call data in preedit_caret callback");
}

/* some status callback procedures */
void
xim_cb_status_start(ic,client,call_data)
	XIC ic;
	int client;
	XPointer call_data;
{
	int which_cb;

	which_cb = CB_ST_START;

	if(client != which_cb)	
		report("Bad client data: status_start callback, expected %d, got %d",
			which_cb,client);
	if(ic == NULL)
		report("Null ic passed to status_start callback");
	if(call_data != NULL)
		report("Non-null call data in status_start callback");

	/* push this info onto the stack for later compare */
	/* call_data should have been NULL, force it */
	xim_response_push_cb(&cbstk_actual,which_cb,NULL);
}

void
xim_cb_status_draw(ic,client,call_data)
	XIC ic;
	int client;
	XIMStatusDrawCallbackStruct *call_data;
{
	int which_cb;
	XIMStatusDrawCallbackStruct *data;

	which_cb = CB_ST_DRAW;

	/* push this info onto the stack for later compare */
	/* copy the data first and the other allocated pieces */
	data = (XIMStatusDrawCallbackStruct *)malloc(
		sizeof(XIMStatusDrawCallbackStruct));
	memcpy(data,call_data,sizeof(XIMStatusDrawCallbackStruct));
	if(data->type == XIMTextType)
		data->data.text = xim_copy_ximtext(call_data->data.text);
	else
		data->data.bitmap = xim_copy_pixmap(call_data->data.bitmap);
	
	xim_response_push_cb(&cbstk_actual,which_cb,data);

	if(client != which_cb)	
		report("Bad client data: status_draw callback, expected %d, got %d",
			which_cb,client);
	if(ic == NULL)
		report("Null ic passed to status_draw callback");
	if(call_data == NULL)
		report("Null call data in status_draw callback");
}

void
xim_cb_status_done(ic,client,call_data)
	XIC ic;
	int client;
	XPointer call_data;
{
	int which_cb;

	which_cb = CB_ST_DONE;

	if(client != which_cb)	
		report("Bad client data: status_done callback, expected %d, got %d",
			which_cb,client);
	if(ic == NULL)
		report("Null ic passed to status_done callback");
	if(call_data != NULL)
		report("Non-null call data in status_done callback");

	/* push this info onto the stack for later compare */
	/* call_data should have been NULL, force it */
	xim_response_push_cb(&cbstk_actual,which_cb,NULL);
}

void
xim_cb_geom(ic,client,call_data)
	XIC ic;
	int client;
	XPointer call_data;
{
	int which_cb;

	which_cb = CB_GEOM;

	if(client != which_cb)	
		report("Bad client data: geometry callback, expected %d, got %d",
			which_cb,client);
	if(ic == NULL)
		report("Null IC passed to geometry callback");
	if(call_data != NULL)
		report("Non-null call data in geometry callback");

	/* push this info onto the stack for later compare */
	/* call_data should have been NULL, force it */
	xim_response_push_cb(&cbstk_actual,which_cb,NULL);
}

void
xim_cb_clean()
{
    int i;
    cbstk_def *pstk; 
 
	/* clean up the actual response stack */
    pstk = &cbstk_actual; 
    for(i=0;i<pstk->top;i++)
        xim_response_pop_cb(pstk);

	/* clean up the expected response stack */
	xim_response_clean_cb();
}

Bool xim_cb_compare()
{
	return(xim_response_compare(&cbstk_actual));
}

/**************************************************************/
/* open a connection to the IM server*/
/* establish a connection */
/* with the callback style */
/* set up the callbacks */


/* open an stimulus file */
/* open the response file */

/* outside loop is to cycle through the actions (stimuli) */
/* reading the corresponding response for each action */

/* go into a event wait loop and Filter Events as appropriate */
/* ask whether the callbacks are done */
/* if so, break out of the loop */

/* as we get an callback, test to see if the callback response */
/* corresponds to the expected response (both data and which callback) */
/* may save the current response file also */
/* need to be able to save to .im_sav if user is building the response */
/* files.  Need to create a configuration variable */

/* once the callbacks are done, we may decide to do one more compare */
/* in case of out of order problems */
/* The stack is then cleaned and we try again on the next stimulus set */
/* until the actions and responses are exhausted */
/* then tally the errors and close up */

/* close the stimilus file */
/* close the response file */
/* close the IC connection */
/* close the IM */
/* wrap the remaining issues and go home */


/**************************************************************/
/* open routines */

Bool xim_ic_setup(pwin,pfs)
	Window *pwin;
	XFontSet *pfs;
{
	int i;
	XVisualInfo *vp;
	Window win;
	char *fsname;
	int missing_cnt;
	char **missing_chars;
	char *defstr;
	XFontSet fs;

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

	/* set up the callback structs */
	for(i=0;i<CB_MAX;i++)
		cb_rec[i].client_data = (XPointer)i;
	cb_rec[CB_PE_START].callback = (XIMProc)xim_cb_preedit_start; 
	cb_rec[CB_PE_DONE].callback  = (XIMProc)xim_cb_preedit_done; 
	cb_rec[CB_PE_DRAW].callback  = (XIMProc)xim_cb_preedit_draw; 
	cb_rec[CB_PE_CARET].callback = (XIMProc)xim_cb_preedit_caret; 

	cb_rec[CB_ST_START].callback = (XIMProc)xim_cb_status_start; 
	cb_rec[CB_ST_DONE].callback  = (XIMProc)xim_cb_status_done; 
	cb_rec[CB_ST_DRAW].callback  = (XIMProc)xim_cb_status_draw; 

	cb_rec[CB_GEOM].callback	 = (XIMProc)xim_cb_geom; 

	*pwin = win;
	*pfs = fs;
	return(True);
}

XIC xim_ic_open(im,win,which_style)
	XIM im;
	Window win;
	XIMStyle which_style;
{
	XIMStyle pe_mask,st_mask;
	int dummy;
	XPoint spot_loc;
	XRectangle area;
	XIC ic;
	unsigned long filter;
	XWindowAttributes watts;
	XSetWindowAttributes setatts;
	char *pstr;

	trace("Creating input context input style, 0x%x",which_style);

	if(preedit_list != NULL)
		XFree(preedit_list);
	if(status_list != NULL)
		XFree(status_list);

	preedit_list = NULL;
	status_list = NULL;
	ic = NULL;

	pe_mask = XIMPreeditArea | XIMPreeditNothing | XIMPreeditNone |
			  XIMPreeditCallbacks | XIMPreeditPosition;

	st_mask = XIMStatusArea | XIMStatusNothing | XIMStatusNone |
			  XIMStatusCallbacks;

	if((which_style & pe_mask) == 0)
	{
		report("No input preedit styles specified");
		return(NULL);
	}

	if((which_style & st_mask) == 0)
	{
		report("No input status styles specified");
		return(NULL);
	}


	/* create a preedit list */
	spot_loc.x = 10;
	spot_loc.y = 10;
	area.x = 0;
	area.y = 0;
	area.width = 50;
	area.height = 50;
	preedit_list = XVaCreateNestedList(dummy,
			XNFontSet,xim_fs,
			XNArea,&area,
			XNSpotLocation,&spot_loc,
			XNPreeditStartCallback, &cb_rec[CB_PE_START],
			XNPreeditDrawCallback,  &cb_rec[CB_PE_DRAW],
			XNPreeditDoneCallback,  &cb_rec[CB_PE_DONE],
			XNPreeditCaretCallback, &cb_rec[CB_PE_CARET],
			NULL);

	/* create a status list */
	status_list = XVaCreateNestedList(dummy,
			XNArea,&area,
			XNFontSet,xim_fs,
			XNStatusStartCallback, &cb_rec[CB_ST_START],
			XNStatusDrawCallback,  &cb_rec[CB_ST_DRAW],
			XNStatusDoneCallback,  &cb_rec[CB_ST_DONE],
			NULL);

	/* now create the IC */
	ic = XCreateIC(im, 
			XNClientWindow, win, 
			XNInputStyle, which_style, 
			XNGeometryCallback,&cb_rec[CB_GEOM],
			XNStatusAttributes, status_list, 
			XNPreeditAttributes, preedit_list, 
			NULL);

	/* adjust the events that window should capture by looking */
	/* at the filter mask of the IC */
	filter = 0;
	pstr = XGetICValues(ic,XNFilterEvents,&filter,NULL);
	
	if(pstr != NULL)
		report("Can't fetch event filter, GetICValues failed, %s",pstr);

	XGetWindowAttributes(Dsp,win,&watts);
	setatts.event_mask = filter | watts.all_event_masks;
	XChangeWindowAttributes(Dsp,win,CWEventMask,&setatts);

	return(ic);
}

static Bool
xim_open_files(locale,pstyle)
	char *locale;
	XIMStyle *pstyle;
{
	char resp_locale[MAXIDLEN];
	XIMStyle style,resp_style;

	/* open stimulus file for the given locale and the get the styles */
	if(!xim_stimulus_open(locale,&style))
		return(False);

	if(!xim_response_open(locale,&resp_style))
		return(False);
	if(!ximconfig.save_im)
	{
		/* make sure that the styles match */
		/* between stimulus and response files */
		if(style != resp_style)
		{
			report("Mismatch of styles: in stimulus file, 0x%x, in response file 0x%x",
				style,resp_style);
			return(False);
		}
	}

	if(!xim_save_open(locale,style))
		return(False);

	if(!locale_set(locale))
	{
		report("Can't set locale to %s",locale);
		return(False);
	}

	*pstyle = style;
	return(True);
}

void
xim_ic_term()
{
	if(preedit_list != NULL)
		XFree(preedit_list);
	if(status_list != NULL)
		XFree(status_list);
	preedit_list = NULL;
	status_list = NULL;

	if(xim_ic != NULL)
		XDestroyIC(xim_ic);

	if(xim_fs != NULL)
		XFreeFontSet(Dsp,xim_fs);

	if(xim_im != NULL)
		XCloseIM(xim_im);

	if(xim_db != NULL)
		XrmDestroyDatabase(xim_db);

	xim_stimulus_close();
	xim_response_close();
	xim_save_close();

}

/* initialize all the ic information */
XIC xim_ic_init(locale)
	char *locale;
{
	xim_fs = NULL;
	xim_db = NULL;
	xim_im = NULL;
	xim_ic = NULL;

	if(!xim_open_files(locale,&xim_style))
		return(NULL);

	xim_db = rm_db_open();
	if(xim_db == NULL)
	{
		report("Can't open resource database");
		xim_ic_term();
		return(NULL);
	}

	xim_im = im_open(xim_db);
	if(xim_im == NULL)
	{
		report("Can't open IM");
		xim_ic_term();
		return(NULL);
	}

	if(!xim_ic_setup(&xim_win,&xim_fs))
	{
		report("Unable to setup ic"); 
		xim_ic_term();
		return(NULL);
	}

	xim_ic = xim_ic_open(xim_im,xim_win,xim_style);
	return(xim_ic);
}


/**************************************************************/
/* check the actual callback stack to see if the all the responses */
/* expected have been fulfilled */
Bool xim_done()
{
	return(xim_response_done(&cbstk_actual));
}
