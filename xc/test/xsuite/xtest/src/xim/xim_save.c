/* $XConsortium: xim_save.c,v 1.2 94/04/17 21:01:50 rws Exp $ */
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
#include	"Xresource.h"
#include    "xtestlib.h"
#include    "tet_api.h"
#include    "pixval.h"

#include	"ximtest.h"

extern Display *Dsp;

extern  int     tet_thistest;
extern  struct  tet_testlist tet_testlist[];

extern	int     Errnum; /* Number of error record */

/****************************************************************/
/* duplicated in actions.c */
 
static char *keys_style[] = {
    "AREA",
    "CALLBACK",
    "NOTHING",
    "NONE",
    "POSITION",
};
static int nkeys_style = sizeof(keys_style) / sizeof(char *);
static int pe_style_val[] = {
    XIMPreeditArea,
    XIMPreeditCallbacks,
    XIMPreeditNothing,
    XIMPreeditNone,
    XIMPreeditPosition,
};
static int status_style_val[] = {
    XIMStatusArea,
    XIMStatusCallbacks,
    XIMStatusNothing,
    XIMStatusNone,
	0,
};

/****************************************************************/
/* duplicated in response.c */

static char *keys_response[] = {
	"VERSION",
	"X_RELEASE",
	"PREEDIT_STYLE",
	"STATUS_STYLE",
	"RESPONSE",
};
static int nkeys_response = sizeof(keys_response) / sizeof(char *);

static char *keys_cbname[] = {
	"PREEDIT_START",
	"PREEDIT_DONE",
	"PREEDIT_DRAW",
	"PREEDIT_CARET",
	"STATUS_START",
	"STATUS_DONE",
	"STATUS_DRAW",
	"GEOMETRY",
};
static int nkeys_cbname = sizeof(keys_cbname) / sizeof(char *);


static char *keys_ximtext[] = {
	"LENGTH",
	"FEEDBACK",
	"IS_WCHAR",
	"STRING",
};
static int nkeys_ximtext = sizeof(keys_ximtext) / sizeof(char *);


static char *keys_pe_draw[] = {
	"CARET",
	"FIRST",
	"LENGTH",
	"TEXT",
};
static int nkeys_pe_draw = sizeof(keys_pe_draw) / sizeof(char *);


static char *keys_pe_caret[] = {
	"POSITION",
	"DIRECTION",
	"STYLE",
};
static int nkeys_pe_caret = sizeof(keys_pe_caret) / sizeof(char *);


static char *keys_st_draw[] = {
	"TYPE",
	"DATA",
};
static int nkeys_st_draw = sizeof(keys_st_draw) / sizeof(char *);

/***********************************************************/
/* save routines for response routines */

#define MAX_INDENT	70
#define INDENT_STEP	4
static char spaces[MAX_INDENT] = 
/*          1         2         3         4         5         6         7*/
/*01234567890123456789012345678901234567890123456789012345678901234567890*/
 "                                                                      ";
/* this little gizmo works backwards, the indent level is actually */
/* the distance from the end of the spaces string */
/* Hence, an indent_level == MAX_INDENT is no indentation */
/*        an indent_level == 0 is an indentation of MAX_INDENT spaces */ 
static int indent_level = MAX_INDENT-1-INDENT_STEP;

static FILE *xim_save_fp = NULL;

void xim_save_pixmap(fp,px)
	FILE *fp;
	Pixmap px;
{
	fprintf(fp,"%s # Pixmap saving not ready yet\n",
		&spaces[indent_level]);
}

void xim_save_wcstr(fp,pwc,cnt)
	FILE *fp;
	wchar_t *pwc;
	int cnt;
{
	int i;

	for(i=0;i<cnt;i++)
	{
		if((i % 10) == 0)
			fprintf(fp,"%s", &spaces[indent_level]);
		fprintf(fp,"0x%02x ",*pwc);
		pwc++;
		if(((i+1) % 10) == 0)
			fprintf(fp,"\n");
	}
}

void xim_save_mbstr(fp,pmb,cnt)
	FILE *fp;
	unsigned char *pmb;
	int cnt;
{
	int i;

	for(i=0;i<cnt;i++)
	{
		if((i % 10) == 0)
			fprintf(fp,"%s", &spaces[indent_level]);
		fprintf(fp,"0x%02x ",*pmb);
		pmb++;
		if(((i+1) % 10) == 0)
			fprintf(fp,"\n");
	}
}

void xim_save_feedback(fp,pfb,cnt)
	FILE *fp;
	XIMFeedback *pfb;
	int cnt;
{
	int i;

	for(i=0;i<cnt;i++)
	{
		if((i % 10) == 0)
			fprintf(fp,"%s", &spaces[indent_level]);
		fprintf(fp,"0x%02x ",(unsigned int)*pfb);
		pfb++;
		if(((i+1) % 10) == 0)
			fprintf(fp,"\n");
	}
}

void xim_save_ximtext(fp,pt)
	FILE *fp;
	XIMText *pt;
{
	int num;

	if(pt == NULL)
	{
		report("No XIMText data to save");
		return;
	}

	num = (int)pt->length;

	fprintf(fp,"%s%s %d\n", &spaces[indent_level],
		keys_ximtext[XIM_TEXT_LENGTH], num);

	fprintf(fp,"%s%s %d\n", &spaces[indent_level],
		keys_ximtext[XIM_TEXT_IS_WCHAR], pt->encoding_is_wchar);


	fprintf(fp,"%s%s {\n", &spaces[indent_level],
		keys_ximtext[XIM_TEXT_FEEDBACK]);

	indent_level -= INDENT_STEP;
	xim_save_feedback(fp,pt->feedback,num);
	indent_level += INDENT_STEP;
	fprintf(fp,"\n%s}\n",&spaces[indent_level]);


	fprintf(fp,"%s%s {\n", &spaces[indent_level],
		keys_ximtext[XIM_TEXT_STRING]);

	indent_level -= INDENT_STEP;
	if(pt->encoding_is_wchar)
		xim_save_wcstr(fp,pt->string.wide_char,num);
	else
		xim_save_mbstr(fp,pt->string.multi_byte,num);
	indent_level += INDENT_STEP;
	fprintf(fp,"\n%s}\n",&spaces[indent_level]);

	fprintf(fp,"\n");
}

void xim_save_pe_draw(fp,pd)
	FILE *fp;
	XIMPreeditDrawCallbackStruct *pd;
{
	fprintf(fp,"%s%s %d\n", &spaces[indent_level],
		keys_pe_draw[PE_DRAW_CARET], pd->caret);

	fprintf(fp,"%s%s %d\n", &spaces[indent_level],
		keys_pe_draw[PE_DRAW_FIRST], pd->chg_first);

	fprintf(fp,"%s%s %d\n", &spaces[indent_level],
		keys_pe_draw[PE_DRAW_LENGTH], pd->chg_length);

	fprintf(fp,"%s%s {\n",&spaces[indent_level],
		keys_pe_draw[PE_DRAW_TEXT]);

	indent_level -= INDENT_STEP;
	xim_save_ximtext(fp,pd->text);
	indent_level += INDENT_STEP;

	fprintf(fp,"%s}\n",&spaces[indent_level]);

	fprintf(fp,"\n");
}


void xim_save_pe_caret(fp,pd)
	FILE *fp;
	XIMPreeditCaretCallbackStruct *pd;
{
	fprintf(fp,"%s%s %d\n", &spaces[indent_level],
		keys_pe_caret[PE_CARET_POSITION], pd->position);

	fprintf(fp,"%s%s %d\n", &spaces[indent_level],
		keys_pe_caret[PE_CARET_DIRECTION], pd->direction);

	fprintf(fp,"%s%s %d\n", &spaces[indent_level],
		keys_pe_caret[PE_CARET_STYLE], pd->style);
}


void xim_save_st_draw(fp,pd)
	FILE *fp;
	XIMStatusDrawCallbackStruct *pd;
{
	fprintf(fp,"%s%s %d\n", &spaces[indent_level],
		keys_st_draw[ST_DRAW_TYPE], pd->type);

	fprintf(fp,"%s%s {\n", &spaces[indent_level],
		keys_st_draw[ST_DRAW_DATA]);

	indent_level -= INDENT_STEP;
	if(pd->type == XIMTextType)
		xim_save_ximtext(fp,pd->data.text);
	else
		xim_save_pixmap(fp,pd->data.bitmap);
	indent_level += INDENT_STEP;

	fprintf(fp,"%s}\n",&spaces[indent_level]);
}

void xim_save_cb(fp,pstk)
	FILE *fp;	
	cbstk_def *pstk;
{
	int i;

	if(fp == NULL)
		return;

	for(i=0;i<pstk->top;i++)
	{
		if(pstk->stack[i] < 0 || pstk->stack[i] >= CB_MAX)
		{
			report("Invalid Callback index %d",pstk->stack[i]);
			continue;
		}

		if(pstk->data[i] == NULL)
		{
			fprintf(fp,"%s%s\n",
				&spaces[indent_level],keys_cbname[pstk->stack[i]]);
		}
		else
		{
			fprintf(fp,"%s%s {\n",
				&spaces[indent_level],keys_cbname[pstk->stack[i]]);
			indent_level -= INDENT_STEP;
		}
		switch(pstk->stack[i])
		{
			case CB_PE_DRAW:
				xim_save_pe_draw(fp,pstk->data[i]);
				break;
			case CB_PE_CARET:
				xim_save_pe_caret(fp,pstk->data[i]);
				break;
			case CB_ST_DRAW:
				xim_save_st_draw(fp,pstk->data[i]);
				break;
			default:
				/* nothing to record, no data */
				if(pstk->data[i] != NULL)
				{
					report("Callback, %s, on has data on the stack - none expected",
						keys_cbname[pstk->stack[i]]);
				}
				break;
		}
		if(pstk->data[i] != NULL)
		{
			indent_level += INDENT_STEP;
			fprintf(fp,"%s}\n",&spaces[indent_level]);
		}
	}
}

void xim_save_response(pstk)
	cbstk_def *pstk;
{
	FILE *fp;

	if(xim_save_fp == NULL)
		return;

	fp = xim_save_fp;

	fprintf(fp,"%s%s {\n",&spaces[indent_level],
		keys_response[RESPONSE_KEY_RESPONSE]);

	indent_level -= INDENT_STEP;
	xim_save_cb(xim_save_fp,pstk);
	indent_level += INDENT_STEP;

	fprintf(fp,"%s}\n",&spaces[indent_level]);

	fprintf(fp,"\n");
}

Bool xim_save_get_style(style,pe,st)
	XIMStyle style;
	int *pe;
	int *st;
{
	int i;

	*pe = -1;
	*st = -1;

	for(i=0;i<nkeys_style;i++)
	{
		if(style & pe_style_val[i])
			*pe = i;
		if(style & status_style_val[i])
			*st = i;
	}
	
	if(*pe == -1 || *st == -1)
		return(False);
}

void xim_save_header(fp,style)
	FILE *fp;
	XIMStyle style;
{
	int pe,st;

	/* version */
	fprintf(fp,"%s %s\n",
		keys_response[RESPONSE_KEY_VERSION],
		RESPONSE_VERSION);

	/* XRelease */
	fprintf(fp,"%s %s\n",
		keys_response[RESPONSE_KEY_XRELEASE],
		RESPONSE_XRELEASE);

	if(!xim_save_get_style(style,&pe,&st))
		report("Unknown Style, 0x%x",style);
	else
	{
		/* Preedit Style */
		fprintf(fp,"%s %s\n",
			keys_response[RESPONSE_KEY_PE_STYLE],
			keys_style[pe]);
		
		/* Status Style */
		fprintf(fp,"%s %s\n",
			keys_response[RESPONSE_KEY_STATUS_STYLE],
			keys_style[st]);
	}

	/* a little space */
	fprintf(fp,"\n");

	/* Start the indentor */
	indent_level = MAX_INDENT - 1 - INDENT_STEP ;
}

Bool xim_save_open(plocale,style)
	char *plocale;
	XIMStyle style;
{
	int testnum;
	char fname[MAXFNAME];
	char *pext;

	/* decide if we are saving as a master or for comparison */
	if(ximconfig.save_im == 0)
		pext = IM_FNAME_DATA;
	else
		pext = IM_FNAME_SAVE;

	/* build a file name */
	/* form is im<test#>.<locale>.response.im_sav */
    testnum = tet_testlist[tet_thistest-1].icref;
	sprintf(fname,"%s%d.%s.%s.%s",
		IM_FNAME_PREFIX,
		testnum,
		plocale,
		IM_FNAME_RESPONSE,
		pext);

	/* figure out which file to open */
	xim_save_fp = fopen(fname,"w");
	if(xim_save_fp == NULL)
	{
		report("Could not open %s to save responses",fname);
		return(False);
	}

	/* write the header */
	fprintf(xim_save_fp,
		"# Response file, %s, saved automatically\n",fname);
	xim_save_header(xim_save_fp,style);

	return(True);
}

void xim_save_close()
{
	if(xim_save_fp != NULL)
		fclose(xim_save_fp);
	xim_save_fp = NULL;
}
