/* $XConsortium: sticvls.m,v 1.2 94/04/17 21:14:02 rws Exp $ */
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

typedef struct {
	int cnt;
	char *name;
	union ICV *val;
	XVaNestedList va;
	struct ICL *list;
} att_def;

typedef union ICV {
	Window win;
	XIMStyle style;
	char str[256];	
	XRectangle rect;
	XPoint pt;
	Colormap cmap;
	Pixmap pmap;
	XFontSet fs;
	unsigned long val_long;
	int val_int;
	Cursor cur;
	XIMCallback cb; 
	att_def att;
	Atom atom;
} ic_val_def;

typedef struct ICL {
	char *name;
	int type;
#define ICV_WINDOW		0
#define ICV_STYLE		1
#define ICV_STR			2
#define ICV_RECT		3
#define ICV_PT			4
#define ICV_CMAP		5
#define ICV_PMAP		6
#define	ICV_LONG		7
#define ICV_FS			8
#define ICV_INT			9
#define ICV_CURSOR		10	
#define ICV_CB			11	
#define ICV_ATT			12
#define ICV_ATOM		13
	ic_val_def *val;
	unsigned int style_mask;
} ic_list_def;

>>TITLE XSetICValues IM
char *

XIC ic;
char *ic_name;
ic_val_def *ic_val;
int endlist = 0;
>>EXTERN

static Bool
check_val(type,name,base_val,return_val)
int type;
char *name;
ic_val_def *base_val;
ic_val_def *return_val;
{
	/* check the base_value returned in ic_base_val */
	switch(type)
	{
		case ICV_WINDOW:
			if(return_val->win != base_val->win)
			{
				report("Returned value does not match expected value for %s",name);
				report("     returned 0x%x",return_val->win);
				report("     expected 0x%x",base_val->win);
				return(False);
			}
			break;
		case ICV_STYLE:
			if(return_val->style != base_val->style)
			{
				report("Returned value does not match expected value for %s",name);
				report("     returned 0x%x",return_val->style);
				report("     expected 0x%x",base_val->style);
				return(False);
			}
			break;
		case ICV_STR:
			if(strcmp(return_val->str,base_val->str) != 0)
			{
				report("Returned value does not match expected value for %s",name);
				report("     returned %s",return_val->str);
				report("     expected %s",base_val->str);
				return(False);
			}
			break;
		case ICV_RECT:
			if((return_val->rect.x != base_val->rect.x) ||
			   (return_val->rect.y != base_val->rect.y) ||
			   (return_val->rect.width != base_val->rect.width) ||
			   (return_val->rect.height != base_val->rect.height))
			{
				report("Returned value does not match expected value for %s",name);
				report("     returned (%d,%d) (%d,%d)",
					return_val->rect.x,
					return_val->rect.y,
					return_val->rect.width, 
					return_val->rect.height);
				report("     expected (%d,%d) (%d,%d)",
					base_val->rect.x, 
					base_val->rect.y,
					base_val->rect.width, 
					base_val->rect.height);
				return(False);
			}
			break;
		case ICV_PT:
			if((return_val->pt.x != base_val->pt.x) ||
			   (return_val->pt.y != base_val->pt.y))
			{
				report("Returned value does not match expected value for %s",name);
				report("     returned (%d,%d)",
					return_val->pt.x,return_val->pt.y);
				report("     expected (%d,%d)",
					base_val->pt.x,base_val->pt.y);
				return(False);
			}
			break;
		case ICV_CMAP:
			if(return_val->cmap != base_val->cmap)
			{
				report("Returned value does not match expected value for %s",name);
				report("     returned 0x%x",return_val->cmap);
				report("     expected 0x%x",base_val->cmap);
				return(False);
			}
			break;
		case ICV_LONG:
			if(return_val->val_long != base_val->val_long)
			{
				report("Returned value does not match expected value for %s",name);
				report("     returned %d",return_val->val_long);
				report("     expected %d",base_val->val_long);
				return(False);
			}
			break;
		case ICV_ATOM:
			if(return_val->atom != base_val->atom)
			{
				report("Returned value does not match expected value for %s",name);
				report("     returned %d",return_val->atom);
				report("     expected %d",base_val->atom);
				return(False);
			}
			break;
		case ICV_PMAP:
			if(return_val->pmap != base_val->pmap)
			{
				report("Returned value does not match expected value for %s",name);
				report("     returned 0x%x",return_val->pmap);
				report("     expected 0x%x",base_val->pmap);
				return(False);
			}
			break;
		case ICV_FS:
			if(return_val->fs != base_val->fs)
			{
				report("Returned value does not match expected value for %s",name);
				report("     returned 0x%x",return_val->fs);
				report("     expected 0x%x",base_val->fs);
				return(False);
			}
			break;
		case ICV_INT:
			if(return_val->val_int != base_val->val_int)
			{
				report("Returned value does not match expected value for %s",name);
				report("     returned %d",return_val->val_int);
				report("     expected %d",base_val->val_int);
				return(False);
			}
			break;
		case ICV_CURSOR:
			if(return_val->cur != base_val->cur)
			{
				report("Returned base_value for %s, 0x%x does not match expected base_value 0x%x",
					name,return_val->cur,base_val->cur);
				return(False);
			}
			break;
		case ICV_CB:
			if((return_val->cb.callback != base_val->cb.callback) ||
			   (return_val->cb.client_data != base_val->cb.client_data))
			{
				report("Returned value does not match expected value for %s",name);
				report("     returned (0x%x,%c)",
					return_val->cb.callback,
					return_val->cb.client_data);
				report("     expected (0x%x,%c)",
					base_val->cb.callback,
					base_val->cb.client_data);
				return(False);
			}
			break;
		case ICV_ATT:
			report("Programming error in test: should get type attribute");
			return(False);
		default:
			report("Unknown IC value type");
			return(False);
	}
	return(True);
}

 
static Window win;
static XIMStyle which_style;
Colormap cmap = None;
static Pixmap pmap = None;
static char rname[] = "im_rname";
static char rclass[] = "IM_rclass";
static unsigned long fg = 1L;
static unsigned long bg = 0L;
static XRectangle area = {0, 0, 20, 20};
static XRectangle area_needed = {0, 10, 50, 20};
static XPoint spot = {20, 11};
static unsigned long filter = 5;
static int linesp = 7;
static Cursor cur = None;

static att_def pe_att;
static att_def st_att;

#define ICCB_START	0
#define ICCB_DONE	1
#define ICCB_DRAW	2
#define ICCB_CARET	3
#define ICCB_MAX	4

static XIMCallback cbp[ICCB_MAX];
static XIMCallback cbs[ICCB_MAX];
static XIMCallback geom;

#define PE_NONE 0
#define PE_A	XIMPreeditArea 
#define PE_CB	XIMPreeditCallbacks
#define PE_POS	XIMPreeditPosition
#define PE_NOT	XIMPreeditPosition
#define PE_N	XIMPreeditNone
#define PE_STD  PE_POS | PE_A | PE_NOT
#define PE_MOST	PE_A | PE_CB | PE_POS | PE_NOT
#define PE_ALL	PE_A | PE_CB | PE_POS | PE_NOT | PE_N

#define S_NONE	0
#define S_A		XIMStatusArea
#define	S_CB	XIMStatusCallbacks
#define S_NOT	XIMStatusNothing
#define S_N		XIMStatusNone
#define S_STD   S_A | S_NOT
#define S_MOST	S_A | S_CB | S_NOT 
#define S_ALL	S_A | S_CB | S_NOT | S_N

#define IM_MOST	PE_MOST | S_MOST
#define IM_STD 	PE_STD | S_STD 
#define IM_ALL	PE_ALL | S_ALL
#define IM_NONE	PE_NONE | S_NONE

static ic_list_def ic_list[] = {
	{ XNInputStyle,		ICV_STYLE,	(ic_val_def *)&which_style,IM_NONE},
	{ XNClientWindow, 		ICV_WINDOW,	(ic_val_def *)&win,		IM_NONE},
	{ XNFocusWindow, 		ICV_WINDOW,	(ic_val_def *)&win,			IM_MOST},
	{ XNResourceName,		ICV_STR,	(ic_val_def *)&rname[0],		IM_STD},
	{ XNResourceClass,		ICV_STR,	(ic_val_def *)&rclass[0],	IM_STD},
	{ XNFilterEvents, 		ICV_LONG,	(ic_val_def *)&filter,	IM_NONE},
};

>>ASSERTION Good A
A call to xname sets the values specified by 
.A ic_names
into the variable 
.A ic_val
associated with the input context,
.A ic.
XICValues returns NULL if no error occured, otherwise it returns
a pointer to the first argument that could not be set.
>>STRATEGY
For all locales, create an input method and for all supported styles
create an input context, for all generic ic values set the value, then
get the value to see if was set correctly. 
>>CODE
Display *dpy;
char *plocale;
XIM im = NULL;
XFontSet fs = NULL;
XrmDatabase db = NULL;
int nstyles = 0;
int ncheck = 0;
char *pstr;
ic_val_def icv,*picv,*ret_icv;
int num_ic;
ic_list_def *il;
int *val;

	dpy = Dsp;
	
	num_ic = sizeof(ic_list) / sizeof(ic_list_def);

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

		cleanup_locale(NULL,fs,im,db);

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
		int i;

	  		ic = ic_open(im,win,which_style);
     		if(ic != NULL)
				CHECK;
			else
     		{
	     		report("Unable to create input context for locale, %s",
              			plocale);
				FAIL;
        		continue;
     		}
			/* loop through all the IC values, fetching them */
			for(i=0;i<num_ic;i++)
			{
				il = (ic_list_def *)&ic_list[i];
				/* make sure that we can fetch this parameter */ 
				if(!(il->style_mask & which_style))
					continue;
				ncheck++;

				ic_name = il->name;
				if(il->type == ICV_LONG	||
					il->type == ICV_ATOM	||
				   il->type == ICV_WINDOW ||
				   il->type == ICV_INT)
				{
					val = (int*)il->val;
					val = (int*)*val;
     				ic_val = (ic_val_def *)val; 
				}
				else
     				ic_val = il->val; 

				pstr = XCALL;
				if(pstr != NULL && *pstr != '\0')
				{
					report("%s() returns non-null result, %s",
						TestName,pstr);	
					FAIL;
				}
				else
				{
					/* fetch the values */
					ret_icv = &icv;
					pstr = XGetICValues(ic,ic_name,&ret_icv,NULL);
					if(pstr != NULL && *pstr != '\0')
					{
						report("XGetICValues returns non-null result, %s",
							pstr);	
						FAIL;
					}

					if(il->type == ICV_STR		||
					   il->type == ICV_PT		||
					   il->type == ICV_RECT		||
					   il->type == ICV_CMAP		||
					   il->type == ICV_PMAP		||
					   il->type == ICV_CURSOR	||
					   il->type == ICV_CB)
						picv = ret_icv;
					else
						picv = (ic_val_def *)&ret_icv;

					if(picv == NULL)
					{
						report("XGetICValues returns null value for %s",
							ic_name);
						FAIL;
					}
					else
					{
						if(check_val(il->type,il->name,il->val,picv))
							CHECK;
						else
							FAIL;
					}
				}
			}
     		ic_close(ic);
     	}
	}   /* nextlocale */
	cleanup_locale(NULL,fs,im,db);

	CHECKPASS(4*nlocales()+nstyles+ncheck);

>>EXTERN
static ic_list_def preedit_list[] = {
	{ XNArea,ICV_RECT,(ic_val_def *)&area,PE_POS | PE_A},
	{ XNColormap,ICV_CMAP,(ic_val_def *)&cmap,PE_STD},
	{ XNForeground,ICV_LONG,(ic_val_def *)&fg,PE_STD},
	{ XNBackground,ICV_LONG,(ic_val_def *)&bg,PE_STD},
	{ XNBackgroundPixmap,ICV_PMAP,(ic_val_def *)&pmap,PE_STD},
	{ XNCursor,ICV_CURSOR,(ic_val_def *)&cur,PE_STD},
	{ XNPreeditStartCallback,ICV_CB,(ic_val_def *)&cbp[ICCB_START],PE_CB},
	{ XNPreeditDoneCallback,ICV_CB,(ic_val_def *)&cbp[ICCB_DONE],PE_CB},
	{ XNPreeditDrawCallback,ICV_CB,(ic_val_def *)&cbp[ICCB_DRAW],PE_CB},
	{ XNPreeditCaretCallback,ICV_CB,(ic_val_def *)&cbp[ICCB_CARET],PE_CB},
};
>>ASSERTION Good A
A call to xname sets the values specified by 
.A ic_names
into the variable 
.A ic_val
associated with the input context,
.A ic.
XICValues returns NULL if no error occured, otherwise it returns
a pointer to the first argument that could not be set.
>>STRATEGY
For all locales, create an input method and for all supported styles
create an input context, for all the preedit ic values set the value, then
get the value to see if was set correctly. 
>>CODE
Display *dpy;
char *plocale;
XIM im = NULL;
XFontSet fs = NULL;
XrmDatabase db = NULL;
int nstyles = 0;
int ncheck = 0;
char *pstr;
ic_val_def icv,*picv,*ret_icv;
ic_list_def *ils;
int type,dummy;
att_def *att,ret_att;
char name_sub[128];
char name[128];
int *val;

	dpy = Dsp;

	ic_get_cb(cbp,cbs,&geom);

	pe_att.cnt = sizeof(preedit_list) / sizeof(ic_list_def);
	pe_att.list = (ic_list_def *)&preedit_list;

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

		cleanup_locale(NULL,fs,im,db);

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
		int j;

	  		ic = ic_open(im,win,which_style);
     		if(ic != NULL)
				CHECK;
			else
     		{
	     		report("Unable to create input context for locale, %s",
              			plocale);
				FAIL;
        		continue;
     		}

     		strcpy(name,XNPreeditAttributes);
 
     		/* loop through all the IC values, fetching them */
     		att = (att_def *)&pe_att;
     		for(j=0;j<att->cnt;j++)
     		{
	     		ils = (ic_list_def *)&att->list[j];
        		if(!(ils->style_mask & which_style))
	     			continue;
				ncheck++;
 
				if(ils->type == ICV_LONG	|| 
				   ils->type == ICV_ATOM	||
				   ils->type == ICV_WINDOW	||
				   ils->type == ICV_INT)
				{
					val = (int*)ils->val;
     				att->va = XVaCreateNestedList(dummy,ils->name,*val,NULL);
				}
				else
     				att->va = XVaCreateNestedList(dummy,ils->name,ils->val,NULL);

        		ic_name = name;
        		ic_val = att->va;

				pstr = XCALL;
				if(pstr != NULL && *pstr != '\0')
				{
					report("%s() returns non-null result, %s",
						TestName,pstr);	
					FAIL;
				}
				else
				{
					/* fetch the values */
					ret_icv = &icv;
        			ret_att.va = XVaCreateNestedList(dummy,ils->name,&ret_icv,NULL);
					pstr = XGetICValues(ic,ic_name,ret_att.va,NULL);
					if(pstr != NULL && *pstr != '\0')
					{
						report("XGetICValues returns non-null result, %s",
							pstr);	
						FAIL;
					}

					if(ils->type == ICV_STR		||
					   ils->type == ICV_PT		||
					   ils->type == ICV_RECT	||
					   ils->type == ICV_CMAP	||
					   ils->type == ICV_PMAP	||
					   ils->type == ICV_CURSOR	||
					   ils->type == ICV_CB)
						picv = ret_icv;
					else
						picv = (ic_val_def *)&ret_icv;

					if(picv == NULL)
					{
						report("XGetICValues returns null value for %s",
							ic_name);
						FAIL;
					}
					else
					{
						if(check_val(ils->type,ils->name,ils->val,picv))
							CHECK;
						else
							FAIL;
					}
				}
			}
    		ic_close(ic);
		}
	}   /* nextlocale */
 	cleanup_locale(NULL,fs,im,db);

	CHECKPASS(4*nlocales()+nstyles+ncheck);

>>EXTERN
static ic_list_def status_list[] = {
	{ XNArea,ICV_RECT,(ic_val_def *)&area,S_A},
	{ XNAreaNeeded,ICV_RECT,(ic_val_def *)&area,S_A },
	{ XNColormap,ICV_CMAP,(ic_val_def *)&cmap,S_A},
	{ XNForeground,ICV_LONG,(ic_val_def *)&fg,S_STD},
	{ XNBackground,ICV_LONG,(ic_val_def *)&bg,S_STD},
	{ XNBackgroundPixmap,ICV_PMAP,(ic_val_def *)&pmap,S_A},
	{ XNLineSpace,ICV_INT,(ic_val_def *)&linesp,S_A},
	{ XNCursor,ICV_CURSOR,(ic_val_def *)&cur,S_A},
	{ XNStatusStartCallback,ICV_CB,(ic_val_def *)&cbs[ICCB_START],S_CB},
	{ XNStatusDoneCallback,ICV_CB,(ic_val_def *)&cbs[ICCB_DONE],S_CB},
	{ XNStatusDrawCallback,ICV_CB,(ic_val_def *)&cbs[ICCB_DRAW],S_CB},
};

>>ASSERTION Good A
A call to xname sets the values specified by 
.A ic_names
into the variable 
.A ic_val
associated with the input context,
.A ic.
XICValues returns NULL if no error occured, otherwise it returns
a pointer to the first argument that could not be set.
>>STRATEGY
For all locales, create an input method and for all supported styles
create an input context, for all the status ic values set the value, then
get the value to see if was set correctly. 
>>CODE
Display *dpy;
char *plocale;
XIM im = NULL;
XFontSet fs = NULL;
XrmDatabase db = NULL;
char *pstr;
ic_val_def icv,*picv,*ret_icv;
int nstyles = 0;
int ncheck = 0;
ic_list_def *ils;
int type,dummy;
att_def *att,ret_att;
char name_sub[128];
char name[128];
int *val;

	dpy = Dsp;

	ic_get_cb(cbp,cbs,&geom);

	st_att.cnt = sizeof(status_list) / sizeof(ic_list_def);
	st_att.list = (ic_list_def *)&status_list;

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

		cleanup_locale(NULL,fs,im,db);

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
		int j;

	 		ic = ic_open(im,win,which_style);
    		if(ic != NULL)
				CHECK;
			else
     		{
	    		report("Unable to create input context for locale, %s",
              			plocale);
				FAIL;
        		continue;
     		}
 
     		strcpy(name,XNStatusAttributes);
 
     		/* loop through all the IC values, fetching them */
    		att = (att_def *)&st_att;
     		for(j=0;j<att->cnt;j++)
     		{
       		ils = (ic_list_def *)&att->list[j];
        		if(!(ils->style_mask & which_style))
	        		continue;
				ncheck++;
 
				if(ils->type == ICV_LONG	|| 
				   ils->type == ICV_ATOM	||
				   ils->type == ICV_WINDOW	||
				   ils->type == ICV_INT)
				{
					val = (int*)ils->val;
    				att->va = XVaCreateNestedList(dummy,ils->name,*val,NULL);
				}
				else
     				att->va = XVaCreateNestedList(dummy,ils->name,ils->val,NULL);
        		ic_name = name;
        		ic_val = att->va;

				pstr = XCALL;
				if(pstr != NULL && *pstr != '\0')
				{
					report("%s() returns non-null result, %s",
						TestName,pstr);	
					FAIL;
				}
				else
				{
					/* fetch the values */
					ret_icv = &icv;
       			ret_att.va = XVaCreateNestedList(dummy,ils->name,&ret_icv,NULL);
					pstr = XGetICValues(ic,ic_name,ret_att.va,NULL);
					if(pstr != NULL && *pstr != '\0')
					{
						report("XGetICValues returns non-null result, %s",
							pstr);	
						FAIL;
					}
trace("%s",ils->name);

					if(ils->type == ICV_STR		||
					   ils->type == ICV_PT		||
					   ils->type == ICV_RECT	||
					   ils->type == ICV_CMAP	|| 
					   ils->type == ICV_PMAP	||
					   ils->type == ICV_CURSOR	||
					   ils->type == ICV_CB)
						picv = ret_icv;
					else
						picv = (ic_val_def *)&ret_icv;

					if(picv == NULL)
					{
						report("XGetICValues returns null value for %s",
							ic_name);
						FAIL;
					}
					else
					{
						if(check_val(ils->type,ils->name,ils->val,picv))
							CHECK;
						else
							FAIL;
					}
				}
			}
    		ic_close(ic);
		}
	}   /* nextlocale */
 	cleanup_locale(NULL,fs,im,db);

	CHECKPASS(4*nlocales()+nstyles+ncheck);

>>EXTERN
#define ICCB_START  0
#define ICCB_DONE   1
#define ICCB_DRAW   2
#define ICCB_CARET  3
#define ICCB_MAX    4
 
extern int iccb_preedit_cnt[ICCB_MAX];
extern int iccb_status_cnt[ICCB_MAX];
 
>>ASSERTION Good A
When an input context is created a StatusStartCallback callback is 
called and when the input context is destroyed or when it loses focus 
a StatusDoneCallback callback is called.
>>STRATEGY
For all locales, create an input method and 
for status callback style, create an input context, 
check the counter to see if the StatusStartCallback has been called,
check the counter to see if the StatusDoneCallback has been called. 
>>CODE
Display *dpy;
char *plocale;
XIM im = NULL;
XFontSet fs = NULL;
XrmDatabase db = NULL;
int ncheck = 0;
char *pstr;
int *val;
int cur_cnt;
XEvent ev;

	dpy = Dsp;

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

		cleanup_locale(NULL,fs,im,db);

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
    	while(next_ic_style(&which_style))
    	{
		int n;

			if(!(which_style & XIMStatusCallbacks))
				continue;
			ncheck++;

     		ic = ic_open(im,win,which_style);
     		if(ic != NULL)
				CHECK;
			else
     		{
	     		report("Unable to create input context for locale, %s",
             			plocale);
				FAIL;
        		continue;
     		}

			cur_cnt = iccb_status_cnt[ICCB_START];

			sleep(2);
			n = XEventsQueued(Dsp,QueuedAfterReading);

			while(n > 0)
			{
				XNextEvent(Dsp,&ev);	
				XFilterEvent(&ev,win);
				n--;
			}

			trace("Create - Counts: (%d,%d,%d,%d)",
				iccb_status_cnt[0],
				iccb_status_cnt[1],
				iccb_status_cnt[2],
				iccb_status_cnt[3]);

			if(cur_cnt+1 != iccb_status_cnt[ICCB_START])
			{
				report("StatusStartCallback not called after IC is created");
				FAIL;
			}
			else
				CHECK;

			XUnsetICFocus(ic);

			cur_cnt = iccb_status_cnt[ICCB_DONE];

			sleep(2);
			n = XEventsQueued(Dsp,QueuedAfterReading);
			while(n > 0)
			{
				XNextEvent(Dsp,&ev);	
				XFilterEvent(&ev,win);
				n--;
			}

			trace("Destroy - Counts: (%d,%d,%d,%d)",
				iccb_status_cnt[0],
				iccb_status_cnt[1],
				iccb_status_cnt[2],
				iccb_status_cnt[3]);

			if(cur_cnt+1 != iccb_status_cnt[ICCB_DONE])
			{
				report("StatusDoneCallback not called when ic is destroyed");
				FAIL;
			}
			else
				CHECK;

			ic_close(ic);
		}
	}   /* nextlocale */
 	cleanup_locale(NULL,fs,im,db);

	CHECKPASS(4*nlocales()+3*ncheck);
