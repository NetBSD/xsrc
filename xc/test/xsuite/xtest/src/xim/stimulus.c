/* $XConsortium: stimulus.c,v 1.2 94/04/17 21:01:48 rws Exp $ */
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

static FILE *fp_stimulus = NULL;

/****************************************************************/

static char *keys_stimulus[] = {
	"VERSION",
	"X_RELEASE",
	"PREEDIT_STYLE",
	"STATUS_STYLE",
	"ACTION",
};
static int nkeys_stimulus = sizeof(keys_stimulus) / sizeof(char *);

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

/* first line of the stimulus for spillover from header reading */
static char stimulus_line[MAXLINELEN];
static char *pstimulus;

static char *keys_subact[] = {
	"KEY",
	"MB_RESET",
	"WC_RESET",
};
static int nkeys_subact = sizeof(keys_subact) / sizeof(char *);

/****************************************************************/

/*
 * Open a xim input/stimulus file and read the header
 * Returns false if can't read the xim stimulus file
 */
Bool xim_stimulus_open(plocale,style)
	char *plocale;			/* name of the locale */
	XIMStyle *style;
{
	int testnum;		/* invocable component */
	char fname[MAXFNAME];
	char id[MAXIDLEN];
	char str[MAXLINELEN];
	int key;
	Bool got_stimulus;
	int pe_style,status_style,tstyle;
	char *tstr,*pstr;

	/* build a file name */
	/* form is im<test#>.<locale>.stimulus */
    testnum = tet_testlist[tet_thistest-1].icref;
	sprintf(fname,"%s%d.%s.%s",
		IM_FNAME_PREFIX,
		testnum,
		plocale,
		IM_FNAME_ACTION);

	fp_stimulus = fopen(fname,"r");
	if(fp_stimulus == NULL)
	{
		report("Can't open %s",fname);
		return(False);
	}

	got_stimulus = False;
	stimulus_line[0] = '\0';
	pstimulus = stimulus_line;

	pe_style = 0;
	status_style = 0;

	/* read the header info */ 
	/* header must come before any stimulus */
	while(!feof(fp_stimulus) && !got_stimulus)
	{
		fgets(str,MAXLINELEN,fp_stimulus);
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
			report("Badly formed stimulus file: missing identifier\n    %s",str);
			continue;
		}

		key = parse_find_key(id,keys_stimulus,nkeys_stimulus);
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
			case ACTION_KEY_VERSION:
				if(strcmp(id,ACTION_VERSION) != 0)
				{
					report("Bad version number in %s: got %s, expected %s\n",
						fname,id,ACTION_VERSION);
					return(False);
				}
				break;
			case ACTION_KEY_XRELEASE:
				if(strcmp(id,ACTION_XRELEASE) != 0)
				{
					report("Bad XRelease number in %s: got %s, expected %s\n",
						fname,id,ACTION_XRELEASE);
					return(False);
				}
				break;
			case ACTION_KEY_PE_STYLE:
				key = parse_find_key(id,keys_style,nkeys_style);
				if(key == -1)
					report("Unknown style %s in:\n%s",id,str);
				else
					pe_style = pe_style_val[key];
				break;
			case ACTION_KEY_STATUS_STYLE:
				key = parse_find_key(id,keys_style,nkeys_style-1);
				if(key == -1)
					report("Unknown style %s in:\n%s",id,str);
				else
					status_style = status_style_val[key];
				break;
			case ACTION_KEY_ACTION:
				got_stimulus = True;
				break;
			default:
				report("Unknown key %d",key);
				break;
		}	
	}

	/* should have read up to the stimulus section */
	/* save this last line for the first stimulus */
	strcpy(stimulus_line,str);

	*style = pe_style | status_style;

	return(True);
}

/*
 * Read a stimulus file and emit stimuli
 */
Bool xim_stimulus_read(ic)
	XIC ic;
{
	char str[MAXLINELEN];
	char *pstr,*tstr;
	char id[MAXIDLEN];
	Bool in_stimulus,got_stimulus_key,stimulus_end;
	int key,keycode;
	char *pe_str;
	wchar_t *pe_wstr;
	Window win,old_win;
	int revert_to;
	char *p;

	if(fp_stimulus == NULL)
		return(False);

	pstr = XGetICValues(ic,XNFocusWindow,&win,NULL);
	if(pstr != NULL)
		return(False);

	/* read an stimulus */
	stimulus_end = False;
	in_stimulus = False;
	got_stimulus_key = False;
	pstr = stimulus_line;
	while(!feof(fp_stimulus) && !stimulus_end)
	{
		if(*pstr == '\0')
			fgets(stimulus_line,MAXLINELEN,fp_stimulus);
		pstr = stimulus_line;

		tstr = strchr(pstr,'#'); 
		if(tstr != NULL)
			*tstr = '\0';

		parse_skwhite(&pstr);
		if(*pstr == '\0')
			continue;

		if(!in_stimulus)
		{
			/* expecting keyword ACTION, followed by { */
			if(!got_stimulus_key)
			{
				if(!parse_getid(&pstr,id,True))
					report("Missing Action keyword");
				else
				{
					if(strcmp(id,keys_stimulus[ACTION_KEY_ACTION]) != 0)
						report("Unknown Keyword %s",id);
					else
						got_stimulus_key = True;
				}	
			}	

			parse_skwhite(&pstr);
			if(got_stimulus_key)
			{
				if(*pstr == '{')
				{
					in_stimulus = True;
					got_stimulus_key = False;
					pstr++;
				}
			}
			parse_skwhite(&pstr);
		}
		else
		{
			if(*pstr == '}')
			{
				pstr++;
				in_stimulus = False;
				stimulus_end = True;		/* we're done let's take a */
										/* look at the results */
				got_stimulus_key = False;
				parse_skwhite(&pstr);
				continue;
			}
			/* read an individual stimulus */
			if(!parse_getid(&pstr,id,True))
			{
				report("Missing stimulus name in:\n>>%s",stimulus_line);
				*pstr = '\0';
				continue;
			}
			key = parse_find_key(id,keys_subact,nkeys_subact);
			if(key == -1)
			{
				report("Unknown Action key %s in:\n>>%s",
					id,stimulus_line);
				*pstr = '\0';
				continue;
			}

			parse_skwhite(&pstr);
			switch(key)
			{
				case SUBACT_KEY:
					/* read the key and package into an event to ship */
					if(*pstr == '0' && 
						(*(pstr+1) == 'x' || *(pstr+1) == 'X'))
					{
						pstr += 2;
						if(!parse_gethex(&pstr,&keycode))
							report("Badly formed keycode");
					}
					else if(!parse_gethex(&pstr,&keycode))
					{
						if(*pstr == '\'')
							pstr++;
						keycode = *(pstr++);
						if(*pstr == '\'')
							pstr++;
					}
					/* save current focus */
					XGetInputFocus(Dsp,&old_win,&revert_to);
					XSetInputFocus(Dsp,win,RevertToNone,CurrentTime);
					keypress(Dsp,keycode);	/* send the event */
					/* go back to original */
					XSetInputFocus(Dsp,old_win,revert_to,CurrentTime);
					break;
				case SUBACT_MBRESET:
					/* reset the ic, assumes only one ic */
					pe_str = XmbResetIC(ic);
					break;
				case SUBACT_WCRESET:
					pe_wstr = XwcResetIC(ic);
					break;
				default:
					break;
			}

			parse_skwhite(&pstr);
		}
	}

	stimulus_line[0] = '\0';
	if(feof(fp_stimulus))
		return(False);
	else
		return(True);
}

/*
 * Close the xim stimuli file 
 */
void xim_stimulus_close()
{
	if(fp_stimulus == NULL)
		return;
			
	fclose(fp_stimulus);
}
