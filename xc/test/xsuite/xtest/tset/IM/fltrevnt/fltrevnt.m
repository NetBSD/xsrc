/* $XConsortium: fltrevnt.m,v 1.2 94/04/17 21:14:04 rws Exp $ */
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
#include "ximtest.h"

>>TITLE XFilterEvent IM
Bool

XEvent *event;
Window win;
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good B 3
A call to xname gives the input method server a chance to process an
.A event
If a filter is registered for the event for a given window,
.A win
The input method server will process the event, filter it and
tell the client that the event has been filtered.  The event should be
filtered if the event is being filtered for at least one ic which has
given window,
.A win
as the focus window.
>>STRATEGY
Read a set of stimuli from a stimulus file and compare actual response
to the expected responses.
>>CODE
char *plocale;
XIC ic;
XEvent ev;

   if(noext(False))
   {
      untested("Unable to run extension tests");
      return;
   }

   resetlocale();
   while(nextlocale(&plocale))
   {
		ic = xim_ic_init(plocale);
		if(ic != NULL) 
			CHECK;
		else
		{
			FAIL;
			continue;
		}

		/* zip through the stimuli (actions) and responses */
		while(xim_stimulus_read(ic))
		{
		int i, n;
			/* read the corresponding response */
			if(!xim_response_read())
			{
				report("Missing response for current action"); 
				break;
			}

			sleep(2);
			n = XEventsQueued(Dsp,QueuedAfterFlush);

			for(i=0;i<n;i++)
			{
				if(!XNextEvent(Dsp,&ev))
				{
					report("Missing event");
					FAIL;
				}
				/* let the IM server see the event first */
				if(!XFilterEvent(&ev,win))
					trace("Event num = %d, type = %d, not filtered",i,ev.type);
			}
				
			if(!xim_cb_compare())
				FAIL;
			else
				CHECK;
	
			/* clean up this set of responses */
			xim_cb_clean();
		}
		xim_ic_term(ic);
	}

	CHECKPASS(2*nlocales());
