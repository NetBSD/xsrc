/* $XConsortium: response.c,v 1.2 94/04/17 21:01:47 rws Exp $ */

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

static FILE *fp_response = NULL;

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
};

/****************************************************************/
static char *keys_response[] = {
	"VERSION",
	"X_RELEASE",
	"PREEDIT_STYLE",
	"STATUS_STYLE",
	"RESPONSE",
};
static int nkeys_response = sizeof(keys_response) / sizeof(char *);

/* first line of the respones file for spillover from header reading */
static char response_line[MAXLINELEN];
static char *presponse = response_line;

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

/* the callback stack is used for storing the responses expected */
/* the callback data stack is a pointer to the data for the callback */
/* some callbacks don't receive data from the IM server so the */
/* field will be left null.  The datastack is in lockstep with the */
/* callback stack. The top is always pointing to next available slot */
static cbstk_def cbstk;


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

/****************************************************************/
/*
 * Open a xim response file and read the header
 * Returns false if can't read the xim response file
 */
Bool xim_response_open(plocale,style)
	char *plocale;			/* name of the locale */
	XIMStyle *style;
{
	int testnum;		/* invocable component */
	char fname[MAXFNAME];
	char str[MAXLINELEN];
	int key;
	Bool got_responses;
	int pe_style,status_style,tstyle;
	char *pstr;
	char id[MAXIDLEN];

	/* if we are in saving mode, then no need to open response file */
	if(ximconfig.save_im)
		return(True);

	/* build a file name */
	/* form is im<test#>.<locale>.response.im_sav */
    testnum = tet_testlist[tet_thistest-1].icref;
	sprintf(fname,"%s%d.%s.%s.%s",
		IM_FNAME_PREFIX,
		testnum,
		plocale,
		IM_FNAME_RESPONSE,
		IM_FNAME_SAVE);

	fp_response = fopen(fname,"r");
	if(fp_response == NULL)
	{
		report("Can't open file, %s\n",fname);
		return(False);
	}
	got_responses = False;
	response_line[0] = '\0';
	presponse = response_line;
	pe_style = 0;
	status_style = 0;

	cbstk.top = 0;

	/* read the header info */ 
	/* header must come before any responses */
	while(!feof(fp_response) && !got_responses)
	{
		fgets(str,MAXLINELEN,fp_response);
		/* get rid of comments */
		pstr = strchr(str,'#');
		if(pstr != NULL)
			*pstr = '\0';

		/* get rid of white space */	
		pstr = str;
		parse_skwhite(&pstr); 
		if(*pstr == '\0')
			continue;

		/* get the keyword */
		if(*pstr == '{')
			break;
		if(!parse_getid(&pstr,id,True))
		{
			report("Badly formed response file: missing identifier\n    %s",str);
			continue;
		}

		/* uses the most of the same topmost commands as the response file */
		key = parse_find_key(id,keys_response,nkeys_response);
		if(key == -1)
		{
			report("Unknown keyword, %s, in:\n    %s",id,str);
			continue;
		}
		
		
		id[0] = '\0';
		parse_skwhite(&pstr);
		if(*pstr != '\0')
		{
			if(!parse_getid(&pstr,id,True))
				parse_skwhite(&pstr);
		}
		switch(key)
		{
			case RESPONSE_KEY_VERSION:
				if(strcmp(id,RESPONSE_VERSION) != 0)
				{
					report("Bad version number in %s: got %s, expected %s\n",
						fname,id,RESPONSE_VERSION);
					return(False);
				}
				break;
			case RESPONSE_KEY_XRELEASE:
				if(strcmp(id,RESPONSE_XRELEASE) != 0)
				{
					report("Bad XRelease number in %s: got %s, expected %s\n",
						fname,id,RESPONSE_XRELEASE);
					return(False);
				}
				break;
			case RESPONSE_KEY_PE_STYLE:
				key = parse_find_key(id,keys_style,nkeys_style);
				if(key == -1)
					report("Unknown style %s in:\n%s",id,str);
				else
					pe_style = pe_style_val[key];
				break;
			case RESPONSE_KEY_STATUS_STYLE:
				key = parse_find_key(id,keys_style,nkeys_style-1);
				if(key == -1)
					report("Unknown style %s in:\n%s",id,str);
				else
					status_style = status_style_val[key];
				break;
			case RESPONSE_KEY_RESPONSE:
				got_responses = True;
				break;
			default:
				report("Unknown key %d",key);
				break;
		}	
	}

	/* should have read up to the response section */
	/* save this last line for the first response */
	strcpy(response_line,str);
	presponse = response_line;

	*style = pe_style | status_style;

	return(True);
}

void xim_response_push_cb(pstk,cb,data)
	cbstk_def *pstk;
	int cb;		/* Callback index */
	char *data;
{
	if(pstk == NULL)
		return;

	if(pstk->top >= MAX_CB_RESPONSE)
	{
		report("Programming error: Overflow on callback stack");
		return;
	}
	pstk->stack[pstk->top] = cb;
	pstk->data[pstk->top] = data;
	pstk->top++;
}

void xim_response_pop_cb(pstk)
	cbstk_def *pstk;
{
	XIMText *pt;
	XIMPreeditDrawCallbackStruct *ped;
	XIMStatusDrawCallbackStruct *psd;

	if(pstk == NULL)
		return;

	if(pstk->top <= 0)
		return;

	pstk->top--;

	/* free any callback data */
	if(pstk->data[pstk->top] != NULL)
	{
		switch(pstk->stack[pstk->top])
		{
			case CB_PE_DRAW:
				ped = (XIMPreeditDrawCallbackStruct *)
						pstk->data[pstk->top];
				pt = ped->text;
				if(pt != NULL)
				{
					/* free the feedback array */
					if(pt->feedback != NULL)
						free(pt->feedback);

					/* free the XIM string */
					if(pt->encoding_is_wchar)
					{
						if(pt->string.wide_char != NULL)
							free(pt->string.wide_char);
					}
					else
					{
						if(pt->string.multi_byte != NULL)
							free(pt->string.multi_byte);
					}
						
					/* free the XIMText structure */
					free(pt);
				}
				break;
			case CB_PE_CARET:
				/* no other structs to free */ 
				break;
			case CB_ST_DRAW:
				psd = (XIMStatusDrawCallbackStruct *)
						pstk->data[pstk->top];
				if(psd->type == XIMTextType)
				{
					if(psd->data.text != NULL)
						free(psd->data.text);
				}
				else
				{
					if(psd->data.bitmap != NULL)
						free(psd->data.bitmap);
				}
				break;
			default:
				report("Callback, %s, has should not have any data",
					keys_cbname[pstk->stack[pstk->top]]);
				break;
		}

		free(pstk->data[pstk->top]);
	}
}

/* read the feedback bits for a XIM Text string */
/* (one long feedback per byte) */
static unsigned long *read_feedback(cnt)
	int cnt;			/* number of feedbacks expected */
{
	int num,n;
	Bool data_end,in_data;
	unsigned long *pf,*pn;
	char *tstr;

	if(cnt <= 0)
	{
		report("Missing count for reading feedback array");
		return(NULL);
	}

	/* allocate some space */
	pf = (unsigned long *)malloc(cnt * sizeof(unsigned long)); 
	pn = pf;

	n = 0;
	in_data = False;
	data_end = False;
	while(!feof(fp_response) && !data_end)
	{
		if(*presponse == '\0')
		{
			fgets(response_line,MAXLINELEN,fp_response);
			presponse = response_line;	
		}

		tstr = strchr(presponse,'#');
		if(tstr != NULL)
			*tstr = '\0';
		parse_skwhite(&presponse);
		if(*presponse == '\0')
			continue;

		if(!in_data)
		{
			/* expecting { */
			if(*presponse == '{')
			{
				in_data = True;
				presponse++;
			}
		}
		else
		{
			if(*presponse == '}')
			{
				presponse++;
				data_end = True;		/* we're done */
				in_data = False;
				continue;
			}

			/* read an individual response */
			if(*presponse == '0' && 
				(*(presponse+1) == 'x' || *(presponse+1) == 'X'))
				presponse += 2;
			if(!parse_gethex(&presponse,&num))
			{
				report("Missing feedback in:\n>>%s",response_line);
				response_line[0] = '\0';
				presponse = response_line;
				continue;
			}
			else
			{
				n++;
				if(n <= cnt)
				{	
					*pn = num;
					pn++;
				}
				else
					report("Overflow of feedback data, expecting %d, got %d",
						cnt,n);
			}
		}
		parse_skwhite(&presponse);
	}

	return(pf);
}

/* read the string as a wide character string */
static wchar_t *read_wcstr(cnt)
	int cnt;			/* number of feedbacks expected */
{
	int n,num;
	Bool data_end,in_data;
	unsigned long *pf;
	char *tstr;
	wchar_t *pwc,*pn;

	/* allocate some space */
	pwc = (wchar_t *)malloc(cnt * sizeof(wchar_t)); 
	pn = pwc;

	n = 0;
	in_data = False;
	data_end = False;
	while(!feof(fp_response) && !data_end)
	{
		if(*presponse == '\0')
		{
			fgets(response_line,MAXLINELEN,fp_response);
			presponse = response_line;	
		}

		tstr = strchr(presponse,'#');
		if(tstr != NULL)
			*tstr = '\0';
		parse_skwhite(&presponse);
		if(*presponse == '\0')
			continue;

		if(!in_data)
		{
			/* expecting { */
			if(*presponse == '{')
			{
				in_data = True;
				presponse++;
			}
		}
		else
		{
			if(*presponse == '}')
			{
				presponse++;
				data_end = True;		/* we're done */
				in_data = False;
				continue;
			}

			/* read an individual response */
			if(*presponse == '0' && 
				(*(presponse+1) == 'x' || *(presponse+1) == 'X'))
				presponse += 2;
			if(!parse_gethex(&presponse,&num))
			{
				report("Missing wide character string in:\n>>%s",
					response_line);
				response_line[0] = '\0';
				presponse = response_line;
				continue;
			}
			else
			{
				n++;
				if(n <= cnt)
				{
					*pn = (wchar_t)num;
					pn++;
				}
				else
					report("Overflow of wctext data, expecting %d, got %d",
						cnt,n);
			}
		}
		parse_skwhite(&presponse);
	}

	return(pwc);
}

/* read the string as a multi-byte character string */
static char *read_mbstr(cnt)
	int cnt;			/* number of mb chars expected */
{
	int n,num;
	Bool data_end,in_data;
	char *p,*pn;
	char *tstr;

	/* allocate some space */
	p = (char *)malloc(cnt * sizeof(char)); 
	pn = p;

	n = 0;
	in_data = False;
	data_end = False;
	while(!feof(fp_response) && !data_end)
	{
		if(*presponse == '\0')
		{
			fgets(response_line,MAXLINELEN,fp_response);
			presponse = response_line;	
		}

		tstr = strchr(presponse,'#');
		if(tstr != NULL)
			*tstr = '\0';
		parse_skwhite(&presponse);
		if(*presponse == '\0')
			continue;

		if(!in_data)
		{
			/* expecting { */
			if(*presponse == '{')
			{
				in_data = True;
				presponse++;
			}
		}
		else
		{
			if(*presponse == '}')
			{
				presponse++;
				data_end = True;		/* we're done */
				in_data = False;
				continue;
			}

			/* read an individual response */
			if(*presponse == '0' && 
				(*(presponse+1) == 'x' || *(presponse+1) == 'X'))
				presponse += 2;
			if(!parse_gethex(&presponse,&num))
			{
				report("Missing multi-byte string in:\n>>%s",
					response_line);
				response_line[0] = '\0';
				presponse = response_line;
				continue;
			}
			else
			{
				n++;
				if(n <= cnt)
				{
					*pn = (char)num;
					pn++;
				}
				else
					report("Data for mbtext exceedes the count, expecting %d, got %d",
						cnt,n); 
			}
		}
		parse_skwhite(&presponse);
	}

	return(p);
}

/* read the XIMText data from a response file */
static XIMText *read_ximtext()
{
	Bool data_end,in_data;
	XIMText *pt;
	int key,num,n;
	char *tstr;
	char id[MAXIDLEN];

	/* allocate some space */
	pt = (XIMText *)malloc(sizeof(XIMText)); 
	pt->length = 0;
	pt->feedback = 0;

	in_data = False;
	data_end = False;
	while(!feof(fp_response) && !data_end)
	{
		if(*presponse == '\0')
		{
			fgets(response_line,MAXLINELEN,fp_response);
			presponse = response_line;	
		}

		tstr = strchr(presponse,'#');
		if(tstr != NULL)
			*tstr = '\0';
		parse_skwhite(&presponse);
		if(*presponse == '\0')
			continue;

		if(!in_data)
		{
			/* expecting { */
			if(*presponse == '{')
			{
				in_data = True;
				presponse++;
			}
			parse_skwhite(&presponse);
		}
		else
		{
			if(*presponse == '}')
			{
				presponse++;
				data_end = True;		/* we're done */
				in_data = False;
				continue;
			}

			/* read an individual response */
			if(!parse_getid(&presponse,id,True))
			{
				report("Missing response name in:\n>>%s",response_line);
				response_line[0] = '\0';
				presponse = response_line;
				continue;
			}
			key = parse_find_key(id,keys_ximtext,nkeys_ximtext);
			if(key == -1)
			{
				report("Unknown response key %s in:\n>>%s",
					id,response_line);
				response_line[0] = '\0';
				presponse = response_line;
				continue;
			}

			parse_skwhite(&presponse);

			/* got a xim text field */
			switch(key)
			{
				case XIM_TEXT_LENGTH:
					if(!parse_getnum(&presponse,&num))
						report("Missing length for key XIMText in:\n>>%s",
							response_line);
					else
						pt->length = num;	
					break;
				case XIM_TEXT_FEEDBACK:
					pt->feedback = read_feedback(pt->length);
					break;
				case XIM_TEXT_IS_WCHAR:
					if(!parse_getnum(&presponse,&num))
						report("Missing length for key XIMText in:\n>>%s",
							response_line);
					else
						pt->encoding_is_wchar = (num == 0) ? False : True;
					break;
				case XIM_TEXT_STRING:
					if(pt->encoding_is_wchar)
						pt->string.wide_char  = read_wcstr(pt->length);
					else
						pt->string.multi_byte = read_mbstr(pt->length);
					break;
				default:
					report("Error in response file: unknown XimText field, %s\n>>%s",
						id,response_line);
					break;
			}
		}
		parse_skwhite(&presponse);
	}

	return(pt);
}

/* read the callback data from a response file for */
/* the preedit_draw callback and stuff it into the appropriate record */ 
static char *read_pe_draw()
{
	Bool cb_end,in_cb;
	int key,num;
	char *tstr;
	char id[MAXIDLEN];
	XIMPreeditDrawCallbackStruct *pd;

	/* allocate some space and init some fields */
	pd = (XIMPreeditDrawCallbackStruct *)malloc(
			sizeof(XIMPreeditDrawCallbackStruct)); 

	pd->caret = 0;
	pd->chg_first = 0;
	pd->chg_length = 0;
	pd->text = NULL;

	in_cb = False;
	cb_end = False;
	while(!feof(fp_response) && !cb_end)
	{
		if(*presponse == '\0')
		{
			fgets(response_line,MAXLINELEN,fp_response);
			presponse = response_line;
		}

		tstr = strchr(presponse,'#');
		if(tstr != NULL)
			*tstr = '\0';
		parse_skwhite(&presponse);
		if(*presponse == '\0')
			continue;

		if(!in_cb)
		{
			/* expecting { */
			if(*presponse == '{')
			{
				in_cb = True;
				presponse++;
			}
			parse_skwhite(&presponse);
		}
		else
		{
			if(*presponse == '}')
			{
				presponse++;
				cb_end = True;		/* we're done */
				in_cb = False;
				parse_skwhite(&presponse);
				continue;
			}

			/* read an individual response */
			if(!parse_getid(&presponse,id,True))
			{
				report("Missing response name in:\n>>%s",response_line);
				response_line[0] = '\0';
				presponse = response_line;
				continue;
			}
			key = parse_find_key(id,keys_pe_draw,nkeys_pe_draw);
			if(key == -1)
			{
				report("Unknown response key %s in:\n>>%s",
					id,response_line);
				response_line[0] = '\0';
				presponse = response_line;
				continue;
			}

			parse_skwhite(&presponse);
			/* got a xim text field */
			switch(key)
			{
				case PE_DRAW_CARET:
					if(!parse_getnum(&presponse,&num))
					{
						report("Bad Response file: missing caret offset\n>>%s",
							response_line);
						response_line[0] = '\0';
						presponse = response_line;
						continue;
					}
					pd->caret = num;
					break;
				case PE_DRAW_FIRST:
					if(!parse_getnum(&presponse,&num))
					{
						report("Bad Response file: missing first change position \n>>%s",
							response_line);
						response_line[0] = '\0';
						presponse = response_line;
						continue;
					}
					pd->chg_first = num;
					break;
				case PE_DRAW_LENGTH:
					if(!parse_getnum(&presponse,&num))
					{
						report("Bad Response file: missing change length\n>>%s",
							response_line);
						response_line[0] = '\0';
						presponse = response_line;
						continue;
					}
					pd->chg_length = num;
					break;
				case PE_DRAW_TEXT:
					pd->text = read_ximtext();
					break;
				default:
					report("Error in response file: unknown PE_DRAW field, %s\n>>%s",
						id,response_line);
					break;
			}
		}
		parse_skwhite(&presponse);
	}

	response_line[0] = '\0';
	return((char *)pd);
}

/* read the callback data from a response file for */
/* the preedit_caret callback and stuff it into the appropriate record */ 
static char *read_pe_caret()
{
	Bool cb_end,in_cb;
	int key,num;
	char *tstr;
	char id[MAXIDLEN];
	XIMPreeditCaretCallbackStruct *pd;

	/* allocate some space and init some fields */
	pd = (XIMPreeditCaretCallbackStruct *)malloc(
			sizeof(XIMPreeditCaretCallbackStruct)); 

	pd->position = 0;
	pd->direction = 0;
	pd->style = 0;

	in_cb = False;
	cb_end = False;
	while(!feof(fp_response) && !cb_end)
	{
		if(*presponse == '\0')
		{
			fgets(response_line,MAXLINELEN,fp_response);
			presponse = response_line;	
		}

		tstr = strchr(presponse,'#');
		if(tstr != NULL)
			*tstr = '\0';
		parse_skwhite(&presponse);
		if(*presponse == '\0')
			continue;

		if(!in_cb)
		{
			/* expecting { */
			if(*presponse == '{')
			{
				in_cb = True;
				presponse++;
			}
			parse_skwhite(&presponse);
		}
		else
		{
			if(*presponse == '}')
			{
				presponse++;
				cb_end = True;		/* we're done */
				in_cb = False;
				parse_skwhite(&presponse);
				continue;
			}

			/* read an individual response */
			if(!parse_getid(&presponse,id,True))
			{
				report("Missing response name in:\n>>%s",response_line);
				response_line[0] = '\0';
				presponse = response_line;
				continue;
			}
			key = parse_find_key(id,keys_pe_caret,nkeys_pe_caret);
			if(key == -1)
			{
				report("Unknown response key %s in:\n>>%s",
					id,response_line);
				response_line[0] = '\0';
				presponse = response_line;
				continue;
			}
			parse_skwhite(&presponse);
			switch(key)
			{
				case PE_CARET_POSITION:
					if(!parse_getnum(&presponse,&num))
					{
						report("Bad Response file: missing caret position\n>>%s",
							response_line);
						response_line[0] = '\0';
						presponse = response_line;
						continue;
					}
					pd->position= num;
					break;
				case PE_CARET_DIRECTION:
					if(!parse_getnum(&presponse,&num))
					{
						report("Bad Response file: missing caret direction\n>>%s",
							response_line);
						response_line[0] = '\0';
						presponse = response_line;
						continue;
					}
					pd->direction = num;
					break;
				case PE_CARET_STYLE:
					if(!parse_getnum(&presponse,&num))
					{
						report("Bad Response file: missing caret style\n>>%s",
							response_line);
						response_line[0] = '\0';
						presponse = response_line;
						continue;
					}
					pd->style = num;
					break;
				default:
					report("Error in response file: unknown PE_CARET field, %s\n>>%s",
						id,response_line);
					break;
			}
		}
		parse_skwhite(&presponse);
	}

	response_line[0] = '\0';
	return((char *)pd);
}

static Pixmap read_pixmap()
{
	Bool data_end,in_data;
	XIMText *pt;
	int key,num;
	char *tstr;
	char id[MAXIDLEN];

	/* allocate some space */
	pt = (XIMText *)malloc(sizeof(XIMText)); 
	pt->length = 0;
	pt->feedback = 0;

	fprintf(stderr,"Read pixmap not ready yet\n");

	in_data = False;
	data_end = False;
	while(!feof(fp_response) && !data_end)
	{
		if(*presponse == '\0')
		{
			fgets(response_line,MAXLINELEN,fp_response);
			presponse = response_line;	
		}

		tstr = strchr(presponse,'#');
		if(tstr != NULL)
			*tstr = '\0';
		parse_skwhite(&presponse);
		if(*presponse == '\0')
			continue;

		if(!in_data)
		{
			/* expecting { */
			if(*presponse == '{')
			{
				in_data = True;
				presponse++;
			}
			parse_skwhite(&presponse);
		}
		else
		{
			if(*presponse == '}')
			{
				presponse++;
				data_end = True;		/* we're done */
				in_data = False;
				continue;
			}
			/* just eat everything for now */
			while(!isspace(*presponse) && *presponse != '}')
				presponse++;
		}
		parse_skwhite(&presponse);
	}

	return(NULL);
}

/* read the callback data from a response file for */
/* the status_draw callback and stuff it into the appropriate record */ 
static char *read_st_draw()
{
	Bool cb_end,in_cb;
	int key,num;
	char *tstr;
	char id[MAXIDLEN];
	XIMStatusDrawCallbackStruct *pd;

	/* allocate some space and init some fields */
	pd = (XIMStatusDrawCallbackStruct *)malloc(
			sizeof(XIMStatusDrawCallbackStruct)); 

	pd->type = 0;
	pd->data.text = NULL;

	in_cb = False;
	cb_end = False;
	while(!feof(fp_response) && !cb_end)
	{
		if(*presponse == '\0')
		{
			fgets(response_line,MAXLINELEN,fp_response);
			presponse = response_line;	
		}

		tstr = strchr(presponse,'#');
		if(tstr != NULL)
			*tstr = '\0';
		parse_skwhite(&presponse);
		if(*presponse == '\0')
			continue;

		if(!in_cb)
		{
			/* expecting { */
			if(*presponse == '{')
			{
				in_cb = True;
				presponse++;
			}
			parse_skwhite(&presponse);
		}
		else
		{
			if(*presponse == '}')
			{
				presponse++;
				cb_end = True;		/* we're done */
				in_cb = False;
				parse_skwhite(&presponse);
				continue;
			}

			/* read an individual response */
			if(!parse_getid(&presponse,id,True))
			{
				report("Missing response name in:\n>>%s",response_line);
				response_line[0] = '\0';
				presponse = response_line;
				continue;
			}
			key = parse_find_key(id,keys_st_draw,nkeys_st_draw);
			if(key == -1)
			{
				report("Unknown response key %s in:\n>>%s",
					id,response_line);
				response_line[0] = '\0';
				presponse = response_line;
				continue;
			}

			parse_skwhite(&presponse);
			/* got a xim text field */
			switch(key)
			{
				case ST_DRAW_TYPE:
					if(!parse_getnum(&presponse,&num))
					{
						report("Bad Response file: missing status draw type \n>>%s",
							response_line);
						response_line[0] = '\0';
						presponse = response_line;
						continue;
					}
					pd->type = num;
					break;
				case ST_DRAW_DATA:
					if(pd->type == XIMTextType)
						pd->data.text = read_ximtext();
					else
						pd->data.bitmap = read_pixmap();
					break;
				default:
					report("Error in response file: unknown ST_DRAW field, %s\n>>%s",
						id,response_line);
					break;
			}
		}
		parse_skwhite(&presponse);
	}

	response_line[0] = '\0';
	return((char *)pd);
}

/*
 * Read response file and get consequences (or suffer the ...)
 */
Bool xim_response_read()
{
	Bool response_end;
	Bool got_response_key;
	Bool in_response;
	char str[MAXLINELEN];
	char *pdata,*tstr;
	char id[MAXIDLEN];
	int key;

	/* if we are in saving mode, then no need to continue */
	if(ximconfig.save_im)
		return(True);

	if(fp_response == NULL)
		return(False);

	/* read a consequence */
	response_end		= False;
	in_response			= False;
	got_response_key	= False;
	while(!feof(fp_response) && !response_end)
	{
		if(*presponse == '\0')
		{
			fgets(response_line,MAXLINELEN,fp_response);
			presponse = response_line;
		}

		tstr = strchr(presponse,'#');
		if(tstr != NULL)
			*tstr = '\0';
		parse_skwhite(&presponse);
		if(*presponse == '\0')
			continue;

		if(!in_response)
		{
			/* expecting keyword RESPONSE, followed by { */
			if(!got_response_key)
			{
				if(!parse_getid(&presponse,id,True))
					report("Missing keyword");
				else
				{
					if(strcmp(id,keys_response[RESPONSE_KEY_RESPONSE]) != 0)
						report("Unknown Keyword %s",id);
					else
						got_response_key = True;
				}	
			}	

			parse_skwhite(&presponse);
			if(got_response_key)
			{
				if(*presponse == '{')
				{
					in_response = True;
					got_response_key = False;
					presponse++;
				}
			}
			parse_skwhite(&presponse);
		}
		else
		{
			if(*presponse == '}')
			{
				presponse++;
				in_response = False;
				response_end = True;		/* we're done let's take a */
										/* look at the results */
				got_response_key = False;
				parse_skwhite(&presponse);
				continue;
			}
			/* read an individual response */
			if(!parse_getid(&presponse,id,True))
			{
				report("Missing response name in:\n>>%s",response_line);
				response_line[0] = '\0';
				presponse = response_line;
				continue;
			}
			key = parse_find_key(id,keys_cbname,nkeys_cbname);
			if(key == -1)
			{
				report("Unknown response key %s in:\n>>%s",
					id,response_line);
				response_line[0] = '\0';
				presponse = response_line;
				continue;
			}

			parse_skwhite(&presponse);
			/* got a call back name */
			switch(key)
			{
				case CB_PE_START:
				case CB_PE_DONE:
					xim_response_push_cb(&cbstk,key,NULL);
					break;
				case CB_PE_DRAW:
					pdata = read_pe_draw(presponse);
					xim_response_push_cb(&cbstk,key,pdata);
					break;
				case CB_PE_CARET:
					pdata = read_pe_caret(presponse);
					xim_response_push_cb(&cbstk,key,pdata);
					break;

				case CB_ST_START:
				case CB_ST_DONE:
					xim_response_push_cb(&cbstk,key,NULL);
					break;
				case CB_ST_DRAW:
					pdata = read_st_draw(presponse);
					xim_response_push_cb(&cbstk,key,pdata);
					break;

				case CB_GEOM:
					xim_response_push_cb(&cbstk,key,NULL);
					break;
				default:
					break;
			}
		}
		parse_skwhite(&presponse);
	}
	response_line[0] = '\0';
	presponse = response_line;
	if(feof(fp_response))
		return(False);
	else
		return(True);
}

/*
 * Close an xim response file 
 */
void xim_response_close()
{
	if(fp_response == NULL)
		return;
			
	fclose(fp_response);
}


/*******************************************************************/
void xim_response_clean_cb()
{
	int i;
	cbstk_def *pstk;

	pstk = &cbstk;
	for(i=0;i<pstk->top;i++)
		xim_response_pop_cb(pstk);

	pstk->top = 0;
}

/* check to see if we have collected all the responses we expect */
Bool xim_response_done(pstk)
	cbstk_def *pstk;
{
	if(pstk->top == cbstk.top)
		return(True);
	else
		return(False);
}

/* compare actual responses vs. expected responses */
Bool xim_response_compare(astk)
	cbstk_def *astk;
{
	cbstk_def *pstk;

	if(ximconfig.save_im)
	{
		xim_save_response(astk);
		report("Saving responses, no compares performed");
		return(False);
	}

	pstk = &cbstk;
	xim_save_response(pstk);

	/* simple test to start with */
	if(astk->top != pstk->top)
	{
		report("Number of responses differs, expected=%d, actual=%d",
			pstk->top,astk->top); 
		return(False);
	}

	if(!xim_compare(pstk,astk))
	{
		report("Compare failed between expected and actual responses");
		return(False);
	}
	else
		return(True);
}


/*******************************************************************/
/* test saving of response files */
void xim_response_save_test()
{
	xim_save_response(&cbstk);
}
