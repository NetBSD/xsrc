/* $XConsortium: gticvls.m,v 1.2 94/04/17 21:14:03 rws Exp $ */
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
	unsigned int style_mask;
} ic_list_def;

>>TITLE XGetICValues IM
char *

XIC ic;
char *ic_name;
ic_val_def *ic_val;
int endlist = 0;
>>EXTERN

static char *pe_names[] = {
    "N/A",
    "Area",
    "Callbacks",
    "Callbacks/Area",
    "Position",
    "Position/Area",
    "Position/Callbacks",
    "Position/Callbacks/Area",
    "Nothing",
    "Nothing/Area",
    "Nothing/Callbacks",
    "Nothing/Callbacks/Area",
    "Nothing/Position",
    "Nothing/Position/Area",
    "Nothing/Position/Callbacks",
    "Nothing/Position/Callbacks/Area",
    "None",
    "None/Area",
    "None/Callbacks",
    "None/Callbacks/Area",
    "None/Position",
    "None/Position/Area",
    "None/Position/Callbacks",
    "None/Position/Callbacks/Area",
    "None/Nothing",
    "None/Nothing/Area",
    "None/Nothing/Callbacks",
    "None/Nothing/Callbacks/Area",
    "None/Nothing/Position",
    "None/Nothing/Position/Area",
    "None/Nothing/Position/Callbacks",
    "None/Nothing/Position/Callbacks/Area",
};

static char *st_names[] = {
    "N/A",
    "Area",
    "Callbacks",
    "Callbacks/Area",
    "Nothing",
    "Nothing/Area",
    "Nothing/Callbacks",
    "Nothing/Callbacks/Area",
    "None",
    "None/Area",
    "None/Callbacks",
    "None/Callbacks/Area",
    "None/Nothing",
    "None/Nothing/Area",
    "None/Nothing/Callbacks",
    "None/Nothing/Callbacks/Area",
};

static int pe_mask = XIMPreeditArea |
                     XIMPreeditCallbacks |
                     XIMPreeditPosition  |
                     XIMPreeditNothing  |
                     XIMPreeditNone;

static int st_mask = XIMStatusArea |
                     XIMStatusCallbacks |
                     XIMStatusNothing   |
                     XIMStatusNone;
 
static char style_strname[128];
static char *get_style_str(which_style)
	XIMStyle which_style;
{
	int which_pe,which_st;

	which_pe = (which_style & pe_mask);
	which_st = (which_style & st_mask) >> 8;
	sprintf(style_strname,"Preedit:%s, Status:%s\n",
            	pe_names[which_pe],
		st_names[which_st]);
	return(style_strname);
}
 

static Bool
echo_val(type,name,return_val)
int type;
char *name;
ic_val_def *return_val;
{
	/* echo the returned value */
	switch(type)
	{
		case ICV_WINDOW:
			trace("Value returned for %s: 0x%x",name,return_val->win);
			break;
		case ICV_STYLE:
			trace("Value returned for %s: 0x%x",name,return_val->style);
			break;
		case ICV_STR:
			trace("Value returned for %s: %s",name,return_val->str);
			break;
		case ICV_RECT:
			trace("Value returned for %s: (%d,%d) (%d,%d)",
				name,
				return_val->rect.x,
				return_val->rect.y,
				return_val->rect.width, 
				return_val->rect.height);
			break;
		case ICV_PT:
			trace("Value returned for %s: (%d,%d)",
				name,
				return_val->pt.x,
				return_val->pt.y);
			break;
		case ICV_LONG:
			trace("Value returned for %s: %d",name,return_val->val_long);
			break;
		case ICV_INT:
			trace("Value returned for %s: %d",name,return_val->val_int);
			break;
		case ICV_CMAP:
			trace("Value returned for %s: 0x%x",name,return_val->cmap);
			break;
		case ICV_PMAP:
			trace("Value returned for %s: 0x%x",name,return_val->pmap);
			break;
		case ICV_FS:
			trace("Value returned for %s: 0x%x",name,return_val->fs);
			break;
		case ICV_CURSOR:
			trace("Value returned for %s: 0x%x",name,return_val->cur);
			break;
		case ICV_CB:
			trace("Value returned for %s: (0x%x,%d)",name,
				return_val->cb.callback,
				return_val->cb.client_data);
			break;
		case ICV_ATT:
			report("Programming error in test: should never get type attribute");
			return(False);
		case ICV_ATOM:
			trace("Value returned for %s: %d",name,return_val->atom);
			break;
		default:
			report("Unknown IC value type");
			return(False);
	}
	return(True);
}

static att_def pe_att;
static att_def st_att;

#define ICCB_START	0
#define ICCB_DONE	1
#define ICCB_DRAW	2
#define ICCB_CARET	3
#define ICCB_MAX	4

static XIMCallback cbp[ICCB_MAX];
static XIMCallback cbs[ICCB_MAX];
static XIMCallback cbg;

#define PE_A	XIMPreeditArea 
#define PE_CB	XIMPreeditCallbacks
#define PE_POS	XIMPreeditPosition
#define PE_NOT	XIMPreeditNothing
#define PE_N	XIMPreeditNone

#define S_A		XIMStatusArea
#define	S_CB	XIMStatusCallbacks
#define S_NOT	XIMStatusNothing
#define S_N		XIMStatusNone

#define PE_ALL	PE_A | PE_CB | PE_POS | PE_NOT | PE_N
#define S_ALL	S_A | S_CB | S_NOT | S_N
#define IM_ALL	PE_ALL | S_ALL
#define PE_MOST	PE_A | PE_CB | PE_POS | PE_NOT
#define S_MOST	S_A | S_CB | S_NOT 
#define IM_MOST	PE_MOST | S_MOST
#define PE_STD  PE_POS | PE_A | PE_NOT
#define S_STD   S_A | S_NOT
#define IM_STD  PE_STD | S_STD 

static ic_list_def ic_list[] = {
	{ XNInputStyle,			ICV_STYLE,	IM_ALL},
	{ XNClientWindow, 		ICV_WINDOW,	PE_CB | PE_POS | PE_A | S_ALL},
	{ XNFocusWindow, 		ICV_WINDOW,	IM_MOST},
	{ XNResourceName,		ICV_STR,	IM_STD},
	{ XNResourceClass,		ICV_STR,	IM_STD},
	{ XNGeometryCallback,	ICV_CB,		PE_A | S_A | S_NOT},
	{ XNFilterEvents, 		ICV_LONG,	PE_MOST | S_ALL},
};

>>ASSERTION Good A
A call to xname fetches the values specified by 
.A ic_names
into the variable 
.A ic_val
associated with the input context,
.A ic.
XICValues returns NULL if no error occured, otherwise it returns
a pointer to the first argument that could not be set.
>>STRATEGY
For all locales, create an input method and for all supported styles
create an input context, then fetch the general ic values.  
>>CODE
ic_val_def icv,*picv;
Display *dpy;
int num_ic;
char *plocale;
XrmDatabase db = NULL;
XIM im = NULL;
XFontSet fs = NULL;
Window win;
XIMStyle which_style;
int nstyles = 0;
int ncheck = 0;
char *pstr;
ic_val_def *ppicv;
ic_list_def *il;

	dpy = Dsp;
	picv = &icv;

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
				/*  can we fetch this parameter? */ 
				if(!(il->style_mask & which_style))
					continue;
				ncheck++;

				ic_name = il->name;
				ic_val = (ic_val_def *)&picv;
				pstr = XCALL;
				if(pstr != NULL && *pstr != '\0')
				{
					report("%s() returns non-null result (style: %s), %s",
						TestName,get_style_str(which_style),pstr);	
					FAIL;
				}
				else
				{
					if(il->type == ICV_STR||il->type == ICV_PT||il->type == ICV_RECT)
						ppicv = picv;
					else
						ppicv = (ic_val_def *)&picv;
					if(echo_val(il->type,il->name,ppicv))
						CHECK;
					else
					{
						report("No Match.");
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
	{ XNArea, 				ICV_RECT,	PE_POS | PE_A},
	{ XNAreaNeeded, 		ICV_RECT,	PE_A },
	{ XNSpotLocation, 		ICV_PT,		PE_POS },
	{ XNColormap, 			ICV_CMAP,	PE_STD},
	{ XNForeground,			ICV_LONG,	PE_STD},
	{ XNBackground,			ICV_LONG,	PE_STD},
	{ XNBackgroundPixmap,	ICV_PMAP,	PE_STD},
	{ XNLineSpace,			ICV_INT,	PE_STD},
	{ XNCursor,				ICV_CURSOR,	PE_STD},
	{ XNPreeditStartCallback,ICV_CB,	PE_CB},
	{ XNPreeditDoneCallback, ICV_CB,	PE_CB},
	{ XNPreeditDrawCallback, ICV_CB,	PE_CB},
	{ XNPreeditCaretCallback, ICV_CB,	PE_CB},
};


>>ASSERTION Good A
A call to xname fetches the values specified by 
.A ic_names
into the variable 
.A ic_val
associated with the input context,
.A ic.
XICValues returns NULL if no error occured, otherwise it returns
a pointer to the first argument that could not be set.
>>STRATEGY
For all locales, create an input method and for all supported styles
create an input context, then fetch the preedit ic values. 
>>CODE
ic_val_def icv,*picv;
Display *dpy;
char *plocale;
XrmDatabase db = NULL;
XIM im = NULL;
XFontSet fs = NULL;
Window win;
XIMStyle which_style;
int nstyles = 0;
int ncheck = 0;
char *pstr;
ic_val_def *ppicv;
ic_list_def *ils;
int dummy;
att_def *att;
char name_sub[128];
char name[128];

	dpy = Dsp;

	picv = &icv;

	ic_get_cb(cbp,cbs,&cbg);

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

				att->va = XVaCreateNestedList(dummy,ils->name,&picv,NULL);	
				ic_name = name;
				ic_val = att->va;

				pstr = XCALL;
				if(ils->type == ICV_STR||ils->type == ICV_PT||ils->type == ICV_RECT
					||ils->type == ICV_CB)
					ppicv = picv;
				else
					ppicv = (ic_val_def *)&picv;

				if(pstr != NULL && *pstr != '\0')
				{
					report("%s() returns non-null result (style: %s), %s",
						TestName,get_style_str(which_style),pstr);	
					FAIL;
				}
				else
				{
					sprintf(name_sub,"%s - %s",name,ils->name);
					if(ppicv == NULL)
					{
						report("%s() unable to fetch result for %s",
							TestName,name_sub);
						FAIL;
					}
					else
					{
						if(echo_val(ils->type,name_sub,ppicv))
							CHECK;
						else
						{
							report("Mo match.");
							FAIL;
						}
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
	{ XNArea, 				ICV_RECT,	S_A},
	{ XNAreaNeeded, 		ICV_RECT,	S_A },
	{ XNColormap, 			ICV_CMAP,	S_STD},
	{ XNForeground,			ICV_LONG,	S_STD},
	{ XNBackground,			ICV_LONG,	S_STD},
	{ XNBackgroundPixmap,	ICV_PMAP,	S_STD},
	{ XNLineSpace,			ICV_INT,	S_STD},
	{ XNCursor,				ICV_CURSOR,	S_STD},
	{ XNStatusStartCallback, ICV_CB,	S_CB},
	{ XNStatusDoneCallback, ICV_CB,		S_CB},
	{ XNStatusDrawCallback, ICV_CB,		S_CB},
};

>>ASSERTION Good A
A call to xname fetches the values specified by 
.A ic_names
into the variable 
.A ic_val
associated with the input context,
.A ic.
XICValues returns NULL if no error occured, otherwise it returns
a pointer to the first argument that could not be set.
>>STRATEGY
For all locales, create an input method and for all supported styles
create an input context, then fetch the status ic values.  
>>CODE
ic_val_def icv,*picv;
Display *dpy;
char *plocale;
XrmDatabase db = NULL;
XIM im = NULL;
XFontSet fs = NULL;
Window win;
XIMStyle which_style;
int nstyles = 0;
int ncheck = 0;
char *pstr;
ic_val_def *ppicv;
ic_list_def *ils;
int dummy;
att_def *att;
char name_sub[128];
char name[128];

	dpy = Dsp;

	ic_get_cb(cbp,cbs,&cbg);

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
        		report("Unable to create input context for locale, %s", plocale);
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

				att->va = XVaCreateNestedList(dummy,ils->name,&picv,NULL);	
				ic_name = name;
				ic_val = att->va;

				pstr = XCALL;
				if(ils->type == ICV_STR		||
				   ils->type == ICV_PT		||
				   ils->type == ICV_RECT		||
				   ils->type == ICV_CB)
					ppicv = picv;
				else
					ppicv = (ic_val_def *)&picv;

				if(pstr != NULL && *pstr != '\0')
				{
					report("%s() returns non-null result (style: %s), %s",
						TestName,get_style_str(which_style),pstr);	
					FAIL;
				}
				else
				{
					sprintf(name_sub,"%s - %s",name,ils->name);
					if(ppicv == NULL)
					{
						report("%s() unable to fetch result for %s",
							TestName,name_sub);
						FAIL;
					}
					else
					{
						if(echo_val(ils->type,name_sub,ppicv))
							CHECK;
						else
						{
							report("No match.");
							FAIL;
						}
					}
				}
			}
			ic_close(ic);
		}
	}   /* nextlocale */
	cleanup_locale(NULL,fs,im,db);

	CHECKPASS(4*nlocales()+nstyles+ncheck);
