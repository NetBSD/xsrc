/* $XConsortium: xim_comp.c,v 1.3 94/04/17 21:01:49 rws Exp $ */
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

/***********************************************************/
/* save routines for response routines */

int xim_comp_pixmap(pxe,pxa)
	Pixmap pxe;
	Pixmap pxa;
{
	int errcnt = 0;

	return(errcnt);
}

int xim_comp_wcstr(cnt,pwce,pwca)
	int cnt;
	wchar_t *pwce;
	wchar_t *pwca;
{
	int i;
	int errcnt = 0;

	for(i=0;i<cnt;i++)
	{
		if(*pwce != *pwca)
		{
			report("WCstr: string at %d differs, expected 0x%0x, actual 0x%0x",
				i,*pwce,*pwca);
			errcnt++;
		}
		pwce++;
		pwca++;
	}
	return(errcnt);
}

int xim_comp_mbstr(cnt,pmbe,pmba)
	int cnt;
	unsigned char *pmbe;
	unsigned char *pmba;
{
	int i;
	int errcnt = 0;

	for(i=0;i<cnt;i++)
	{
		if(*pmbe != *pmba)
		{
			report("MBstr: string at %d differs, expected 0x%0x, actual 0x%0x",
				i,*pmbe,*pmba);
			errcnt++;
		}
		pmbe++;
		pmba++;
	}
	return(errcnt);
}

int xim_comp_feedback(cnt,pfe,pfa)
	int cnt;
	XIMFeedback *pfe;
	XIMFeedback *pfa;
{
	int i;
	int errcnt = 0;

	for(i=0;i<cnt;i++)
	{
		if(*pfe != *pfa)
		{
			report("Feedback: feedback at %d differs, expected %d, actual %d",
				i,*pfe,*pfa);
			errcnt++;
		}	
		pfe++;
		pfa++;
	}
	return(errcnt);
}

int xim_comp_ximtext(pte,pta)
	XIMText *pte;
	XIMText *pta;
{
	int errcnt = 0;
	int num;

	if(pte == NULL)
	{
		report("XIMText data not saved in expected case");
		return(++errcnt);
	}

	if(pta == NULL)
	{
		report("XIMText data not saved in actual case");
		return(++errcnt);
	}

	num = (int)pte->length;

	if(pte->length != pta->length)
	{
		report("XIMText: Lengths differ, expected %d, actual %d",
			pte->length,pta->length);
		errcnt++;
	}

	if(pte->encoding_is_wchar != pta->encoding_is_wchar)
	{
		report("XIMText: Encoding_is_wchar differs, expected %d, actual %d",
			pte->encoding_is_wchar,pta->encoding_is_wchar);
		errcnt++;
	}

	errcnt += xim_comp_feedback(num,pte->feedback,pta->feedback);
	if(pte->encoding_is_wchar)
		errcnt += xim_comp_wcstr(num,
			pte->string.wide_char,pta->string.wide_char);
	else
		errcnt += xim_comp_mbstr(num,
			pte->string.multi_byte,pta->string.multi_byte);

	return(errcnt);
}

int xim_comp_pe_draw(pde,pda)
	XIMPreeditDrawCallbackStruct *pde;
	XIMPreeditDrawCallbackStruct *pda;
{
	int errcnt = 0;

	if(pde == NULL)
		return(++errcnt);
	if(pda == NULL)
		return(++errcnt);

	if(pde->caret != pda->caret)
	{
		report("PE_DRAW: caret fields differ, expected %d, actual %d",
			pde->caret,pda->caret);
		errcnt++;
	}

	if(pde->chg_first != pda->chg_first)
	{
		report("PE_DRAW: chg_first fields differ, expected %d, actual %d",
			pde->chg_first,pda->chg_first);
		errcnt++;
	}

	if(pde->chg_length != pda->chg_length)
	{
		report("PE_DRAW: chg_length fields differ, expected %d, actual %d",
			pde->chg_length,pda->chg_length);
		errcnt++;
	}

	errcnt += xim_comp_ximtext(pde->text,pda->text); 

	return(errcnt);
}

int xim_comp_pe_caret(pde,pda)
	XIMPreeditCaretCallbackStruct *pde;
	XIMPreeditCaretCallbackStruct *pda;
{
	int errcnt = 0;

	if(pde == NULL)
		return(++errcnt);
	if(pda == NULL)
		return(++errcnt);

	if(pde->position != pda->position)
	{
		report("PE_CARET: position fields differ, expected %d, actual %d",
			pde->position,pda->position);
		errcnt++;
	}

	if(pde->direction != pda->direction)
	{
		report("PE_CARET: direction fields differ, expected %d, actual %d",
			pde->direction,pda->direction);
		errcnt++;
	}

	if(pde->style != pda->style)
	{
		report("PE_CARET: style fields differ, expected %d, actual %d",
			pde->style,pda->style);
		errcnt++;
	}

	return(errcnt);
}


int xim_comp_st_draw(pde,pda)
	XIMStatusDrawCallbackStruct *pde;
	XIMStatusDrawCallbackStruct *pda;
{
	int errcnt = 0;

	if(pde == NULL)
		return(++errcnt);
	if(pda == NULL)
		return(++errcnt);

	if(pde->type != pda->type)
	{
		report("ST_DRAW: type fields differ, expected %d, actual %d",
			pde->type,pda->type);
		errcnt++;
	}

	if(pde->type == XIMTextType)
		errcnt += xim_comp_ximtext(pde->data.text,pda->data.text);
	else
		errcnt += xim_comp_pixmap(pde->data.bitmap,pda->data.bitmap);

	return(errcnt);
}

Bool xim_compare(pe,pa)
	cbstk_def *pe;			/* stack of expected responses */
	cbstk_def *pa;			/* stack of actual responses */
{
	int i,cnt;
	int errcnt = 0;

	cnt = pe->top;
	if(pe->top != pa->top)
	{
		report("Callback Stacks are of different size, expected %d, actual %d",
			pe->top,pa->top);
		errcnt++;
		if(cnt > pa->top)
			cnt = pa->top;
	}

	for(i=0;i<cnt;i++)
	{
		if(pe->stack[i] != pa->stack[i])
		{
			report("Callbacks differ at index = %d, expected = %d, actual = %d",
				i,pe->stack[i],pa->stack[i]);
			errcnt++;
		}
		else
		{
			if(pe->data[i] != NULL)
			{
				switch(pe->stack[i])
				{   
					case CB_GEOM:
					case CB_ST_START:
					case CB_ST_DONE:
					case CB_PE_START:
					case CB_PE_DONE:
						report("Got data for callback %d, none expected",
							pe->stack[i]);
						errcnt++;
						break;
					case CB_PE_DRAW:
						errcnt += xim_comp_pe_draw(pe->data[i],
							pa->data[i]);
						break;
					case CB_PE_CARET:
						errcnt += xim_comp_pe_caret(pe->data[i],
							pa->data[i]);
						break;
						break;
					case CB_ST_DRAW:
						errcnt += xim_comp_st_draw(pe->data[i],
							pa->data[i]);
						break;
					default:
						report("Illegal callback value, %d, at index %d",
							pe->stack[i],i);
						errcnt++;
						break;
				}
			}
		}
	}
	return((errcnt == 0));
}

int xim_wsneq(s1,s2,cnt)
	wchar_t *s1;
	wchar_t *s2;
	int cnt;
{
	int i;

	for(i=0;i<cnt;i++)
	{
		if(*s1 != *s2)
			return 0;
		if(!*s1)
			return 1;
		s1++;
		s2++;
	}
	return 1;
}
