/* $XConsortium: vcrtnstdls.m,v 1.2 94/04/17 21:14:11 rws Exp $ */
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

>>TITLE XVaCreateNestedList IM
XVaNestedList

int dummy;
int endlist=0;
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A 
A call to xname builds and returns a pairwise list of parameters from an
variable length list.
>>STRATEGY
For all locales, build an empty variable list, verify that it is null.
>>CODE
char *plocale;
XVaNestedList va;

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

		va = XCALL;
		if(va == NULL)
			CHECK;
		else
		{
			report("%s() created a non-null nested list from no arguments",
				TestName);
			FAIL;
		}
		if(va != NULL)
			XFree(va);
	}   /* nextlocale */

	CHECKPASS(2*nlocales());

>>ASSERTION Good A 
A call to xname builds and returns a pair wise list of parameters from 
an variable length list.
>>STRATEGY
For all locales, build a variable list of one element, 
verify that a nested list is returned. 
>>CODE
char cmd[32];
int a1,cnt,tmp;
char *plocale;
XVaNestedList va;
char *p;
int *pi;

	strcpy(cmd,"a1");
	cnt = 1;
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

		a1 = cnt++;
		/* Can't use X C A L L since parameter list is variable */
		startcall(Dsp);
		if (isdeleted())
			return;
		va = XVaCreateNestedList(dummy,cmd,a1,endlist);
		endcall(Dsp);
		if (geterr() != Success) {
			report("Got %s, Expecting Success", errorname(geterr()));
			FAIL;
		}
		if(va != NULL)
			CHECK;
		else
		{
			report("%s() created a null nested list",
				TestName);
			FAIL;
		}

		/* see if the list looks correct */
		pi = (int *)va;
		p = (char *)*pi;
		if(strcmp((char *)p,cmd) != 0)
		{
			report("Command arg of Nested list is incorrect, expected %s",
				cmd);
			FAIL;
		}
		else
		{
			trace("VaNestedList command = %s",cmd);
			CHECK;
		}

		pi++;
		tmp = *pi;
		if(tmp != a1)
		{
			report("First arg of Nested list is incorrect, expected %d, got %d",
				a1,tmp);
			FAIL;
		}
		else
		{
			trace("VaNestedList a1 = %d",tmp);
			CHECK;
		}

		if(va != NULL)
			XFree(va);
	}   /* nextlocale */

	CHECKPASS(4*nlocales());

>>ASSERTION Good A 
A call to xname builds and returns a list of parameters from an variable
length list.
>>STRATEGY
For all locales, build a variable list of two elements, 
verify that a nested list is returned. 
>>CODE
char cmd[2][32];
int a[2],cnt;
char *plocale;
XVaNestedList va;
char *p;
int *pi;
int j,tmp;

	strcpy(cmd[0],"a[0]");
	strcpy(cmd[1],"a[1]");
	cnt = 1;
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

		a[0] = cnt++;
		a[1] = cnt++;
		/* Can't use X C A L L since parameter list is variable */
		startcall(Dsp);
		if (isdeleted())
			return;
		va = XVaCreateNestedList(dummy,
				cmd[0],a[0],
				cmd[1],a[1],
				endlist);
		endcall(Dsp);
		if (geterr() != Success) {
			report("Got %s, Expecting Success", errorname(geterr()));
			FAIL;
		}
		if(va != NULL)
			CHECK;
		else
		{
			report("%s() created a null nested list",
				TestName);
			FAIL;
		}

		pi = (int *)va;
		for(j=0;j<2;j++)
		{
			p = (char *)*pi;
			pi++;
			if(strcmp((char *)p,cmd[j]) != 0)
			{
				report("Command arg of Nested list is incorrect, expected %s",
					cmd[j]);
				FAIL;
			}
			else
			{
				trace("VaNestedList command = %s",cmd[j]);
				CHECK;
			}

			tmp = *pi;
			pi++;
			if(tmp != a[j])
			{
				report("%d arg of Nested list is incorrect, expected %d, got %d",
					j,a[j],tmp);
				FAIL;
			}
			else
			{
				trace("VaNestedList a[%d] = %d",j,tmp);
				CHECK;
			}
		}

		if(va != NULL)
			XFree(va);
	}   /* nextlocale */

	CHECKPASS(6*nlocales());

>>ASSERTION Good A 
A call to xname builds and returns a list of parameters from an variable
length list.
>>STRATEGY
For all locales, build a variable list of one element, and use this
argument as a parameter to a second nested list, verify that a nested 
list is returned. 
>>CODE
int a1,cnt;
char *plocale;
XVaNestedList va,va2;
char cmd[32];
char va_cmd[32];
char *p;
int *pi;
int tmp;

	cnt = 1;
	strcpy(cmd,"a1");
	strcpy(va_cmd,"va");
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

		a1 = cnt++;
		/* Can't use X C A L L since parameter list is variable */
		startcall(Dsp);
		if (isdeleted())
			return;
		va = XVaCreateNestedList(dummy,cmd,a1, endlist);
		endcall(Dsp);
		if (geterr() != Success) {
			report("Got %s, Expecting Success", errorname(geterr()));
			FAIL;
		}
		if(va != NULL)
			CHECK;
		else
		{
			report("%s() created a null nested list",TestName);
			FAIL;
			continue;
		}

		/* build the second nested list */
		startcall(Dsp);
		if (isdeleted())
			return;
		va2 = XVaCreateNestedList(dummy, va_cmd, va, endlist);
		endcall(Dsp);
		if (geterr() != Success) {
			report("Got %s, Expecting Success", errorname(geterr()));
			FAIL;
		}
		if(va2 != NULL)
			CHECK;
		else
		{
			report("%s() created a null nested list",TestName);
			FAIL;
		}

		/* see if the list looks correct */
		pi = (int *)va2;
		p = (char *)*pi;
		if(strcmp((char *)p,va_cmd) != 0)
		{
			report("Command arg of Nested list is incorrect, expected %s",
				va_cmd);
			FAIL;
		}
		else
		{
			trace("VaNestedList command = %s",va_cmd);
			CHECK;
		}

		pi++;
		if(*pi != (int)va)
		{
			report("Pointer to va is incorrect, expected 0x%x,got 0x%x",
				va,pi);
			FAIL;
		}
		else
			CHECK;

		/* this is another Nested List, go down and verify it */
		pi = (int *)*pi;
		p = (char *)*pi;
		if(strcmp((char *)p,cmd) != 0)
		{
			report("Command arg of Nested list is incorrect, expected %s",
				cmd);
			FAIL;
		}
		else
		{
			trace("VaNestedList command = %s",cmd);
			CHECK;
		}

		pi++;
		tmp = *pi;
		if(tmp != a1)
		{
			report("First arg of Nested list is incorrect, expected %d, got %d",
				a1,tmp);
			FAIL;
		}
		else
		{
			trace("VaNestedList a1 = %d",tmp);
			CHECK;
		}

		if(va2 != NULL)
			XFree(va2);
		if(va != NULL)
			XFree(va);
	}   /* nextlocale */

	CHECKPASS(7*nlocales());
