/* $XConsortium: mblkpstr.m,v 1.2 94/04/17 21:14:05 rws Exp $ */
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

>>TITLE XmbLookupString IM
int

XIC ic;
XKeyPressedEvent *event;
char *buffer_return;
int bytes_buffer;
KeySym *keysym_return;
Status *status_return;
>>EXTERN
static KeySym which_key = 0x062; /* XK_b */
static KeySym which_keycap = 0x042; /* XK_B */
>>SET startup localestartup
>>SET cleanup localecleanup
>>ASSERTION Good A
A call to xname returns the string from the input method specified in the
.A buffer_return
argument. If no string is returned the 
.A buffer_return 
argument remains unchanged.  The 
.S KeySym 
into which the 
.S KeyCode 
from the event was mapped is returned in the
.A keysym_return
argument if it is non-NULL and the 
.A status_return 
argument indicates that a 
.S KeySym 
was returned.  If both a string and a 
.S KeySym 
are returned, the 
.S KeySym 
value does not necessarily correspond to the string returned.
xname returns the length of the string in bytes.  The text is returned
in the encoding of the locale bound to the input method of the 
specified input context,
.A ic .
>>STRATEGY
For all locales, create an input method and 
for all supported styles create an input context,
Obtain the keycode corresponding to the keysym XK_b using XKeysymToKeycode.
Obtain the string and keysym bound to that keycode using xname.
Verify that the returned string is correct.
Obtain the string and keysym bound to that keycode using xname with state = ShiftMask.
Verify that the returned string is correct.
>>CODE
char *plocale;
XrmDatabase db = NULL;
XIM im = NULL;
Window win;
XFontSet fs = NULL;
int nstyles = 0;
XIMStyle which_style;
KeyCode kc;
XKeyEvent ke;
KeySym ks=0;
char buf[32];
int mlen;
Status status;
int res,cmplen;

	XrmInitialize();

	strcpy(buf,"Xtest uninitialiased string.");
	mlen = strlen(buf);

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

     	reset_ic_style(im);
		nstyles += n_ic_styles();
     	while(next_ic_style(&which_style))
    	{
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

			kc = XKeysymToKeycode(Dsp, which_key);

			ke.type = KeyPress;
			ke.display = Dsp;
			ke.keycode = kc;
			ke.state = 0;
			ke.window = win;

			event = &ke;
			buffer_return = (char *) buf;
			bytes_buffer = mlen;
			keysym_return = &ks;
			status_return = &status;

			res = XCALL;
			trace("Status = %d\n",status);
			if(ks != which_key)
			{
				report("%s() returned keysym %d instead of %d.", 
					TestName, (int) ks, which_key);
				FAIL;
			}
			else
				CHECK;

			if(res != 1)
			{
				report("%s() returned %d instead of 1.", TestName, res);
				FAIL;
			}
			else
				CHECK;

			cmplen = mlen;
			if(res>0 && res<mlen) cmplen = res;
			buf[cmplen] = '\0';

			if(strncmp(buffer_return, "b", cmplen) != 0)
			{
				report("%s() returned string \"%s\" instead of \"%s\".",
					TestName, buffer_return,"b");
				FAIL;
			}
			else
				CHECK;

			ke.state = ShiftMask;
			event = &ke;
			res = XCALL;
			trace("Status = %d\n",status);

			if(ks != which_keycap)
			{
				report("%s() returned keysym %d instead of %d.",
					TestName, (int) ks, which_keycap);
				FAIL;
			}
			else
				CHECK;

			cmplen = mlen;
			if(res>0 && res<mlen) cmplen = res;
			buf[cmplen] = '\0';

			if(strncmp(buffer_return, "B", cmplen) != 0)
			{
				report("%s() returned string \"%s\" instead of \"%s\".",
					TestName, buffer_return,"B");
				FAIL;
			}
			else
				CHECK;

     		ic_close(ic);
        	}
	}   /* nextlocale */
 	cleanup_locale(NULL,fs,im,db);

	CHECKPASS(4*nlocales()+6*nstyles);

>>ASSERTION Good A
When the
.S KeySym
corresponding to the
.A event
argument has been rebound, then
the bound string is returned, truncated to
.A bytes_buffer ,
in the
.A buffer_return
argument.
>>STRATEGY
For all locales, create an input method and 
for all supported styles create an input context,
Rebind the keysym XK_b to the string XtestRebound using XRebindKeysym.
Obtain the keycode bound to the XK_b keysym using XKeysymToKeycode.
Obtain the characters of the string to which the keycode is bound using xname.
Verify that the returned string is correct.
>>CODE
KeyCode kc;
static char	buf[]="XtestRebound";
char *plocale;
XrmDatabase db = NULL;
XIM im = NULL;
Window win;
XFontSet fs = NULL;
int nstyles = 0;
XIMStyle which_style;
XKeyEvent ke;
KeySym ks=0;
char buf_ret[20];
int	mlen = strlen(buf);
Status status;
int res,cmplen;

	XrmInitialize();

	kc = XKeysymToKeycode(Dsp, which_key);
	XRebindKeysym(Dsp, which_key, (KeySym *) NULL, 0, 
			(unsigned char *) buf, strlen(buf));

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

			ke.type = KeyPress;
			ke.display = Dsp;
			ke.keycode = kc;
			ke.state = 0;
			ke.window = win;

			event = &ke;
			buffer_return = (char *) buf_ret;
			bytes_buffer = mlen;
			keysym_return = &ks;
			status_return = &status;

			res = XCALL;
			trace("Status = %d\n",status);

			if(ks != which_key)
			{
				report("%s() returned keysym 0x%x instead of 0x%x.",
					TestName, (int) ks, which_key);
				FAIL;
			}
			else
				CHECK;

			if(res != mlen)
			{
				report("%s() returned %d instead of %d.",
					TestName, res, mlen);
				FAIL;
			}
			else
				CHECK;

			cmplen = mlen;
			if(res>0 && res<mlen) cmplen = res;
			buf[cmplen] = '\0';
			buffer_return[cmplen] = '\0';

			if(strncmp(buffer_return, buf, cmplen) != 0)
			{
				report("%s() returned string \"%s\" instead of \"%s\".",
					TestName, buffer_return, buf);
				FAIL;
			}
			else
				CHECK;

			ic_close(ic);
    	}
	}   /* nextlocale */
	cleanup_locale(NULL,fs,im,db);

	CHECKPASS(4*nlocales()+4*nstyles);

>>ASSERTION Bad A
If the input string to be returned is too large for the supplied
.A buffer_return
xname returns XBufferOverflow in the
.A status_return
argument.
>>STRATEGY
For all locales, create an input method and 
for all supported styles create an input context,
Rebind the keysym XK_b to the string Too_long_a_string using XRebindKeysym.
Obtain the keycode bound to the XK_b keysym using XKeysymToKeycode.
Set the bytes_buffer to the size of the string minus 2.  The status_return
argument should come back with the value, XBufferOverflow.
>>CODE
KeyCode kc;
static char buf[]="Too_long_a_string";
char *plocale;
XrmDatabase db = NULL;
XIM im = NULL;
Window win;
XFontSet fs = NULL;
int nstyles = 0;
XIMStyle which_style;
XKeyEvent ke;
KeySym ks=0;
static char	buf_ret[32];
int   mlen;
Status status;
int res;

	XrmInitialize();

	kc = XKeysymToKeycode(Dsp, which_key);
	XRebindKeysym(Dsp, which_key, (KeySym *) NULL, 0, 
		(unsigned char *) buf, strlen(buf));

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

			strcpy(buf,"Too_long_a_string");
			ke.type = KeyPress;
			ke.display = Dsp;
			ke.keycode = kc;
			ke.state = 0;
			ke.window = win;

			event = &ke;
			buffer_return = (char *) buf_ret;
			mlen = strlen(buf) - 2;
			bytes_buffer = mlen;
			keysym_return = &ks;
			status_return = &status;

			res = XCALL;
			trace("Status = %d\n",status);

			if(status != XBufferOverflow)
			{
				report("%s() returned status %d instead of %d",
					TestName,status,XBufferOverflow);
				FAIL;
			}
			else
				CHECK;

			if(res != mlen+2)
			{
				report("%s() returned %d instead of %d.",
					TestName, res, mlen+2);
				FAIL;
			}
			else
				CHECK;

			ic_close(ic);
		}
	}   /* nextlocale */
 	cleanup_locale(NULL,fs,im,db);

	CHECKPASS(4*nlocales()+3*nstyles);

>>ASSERTION Bad A
If no consistent input has been composed so far the value, XLookupNone,
is returned in 
.A status_return
and the contents of 
.A buffer_return
and
.A keysym_return
are not modified and xname returns zero.
>>STRATEGY
For all locales, create an input method and 
for all supported styles create an input context,
Call the keypress event with a keycode of zero.
Check the status_return argument, it should be set to XLookupNone.
Check the buffer_return, it should be unmodified.
Check the keysym_return, it should be unmodified.
>>CODE
KeySym ks_hold;
KeyCode kc;
static char buf[]="Don't tread on me";
char *plocale;
XrmDatabase db = NULL;
XIM im = NULL;
Window win;
XFontSet fs = NULL;
int nstyles = 0;
XIMStyle which_style;
XKeyEvent ke;
KeySym ks=0;
static char buf_ret[32];
int   mlen;
Status status;
int res,cmplen;

	ks_hold = which_key;
	kc = 0; 

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

			ke.type = KeyPress;
			ke.display = Dsp;
			ke.keycode = kc;
			ke.state = 0;
			ke.window = win;

			event = &ke;
			strcpy(buf_ret,buf);
			buffer_return = (char *) buf_ret;
			mlen = 32;
			bytes_buffer = mlen;
			ks = ks_hold;
			keysym_return = &ks;
			status_return = &status;

			res = XCALL;
			trace("Status = %d\n",status);

			if(status != XLookupNone)
			{
				report("%s() returned status %d instead of %d",
					TestName,status,XLookupNone);
				FAIL;
			}
			else
				CHECK;

			if(ks != which_key)
			{
				report("%s() modified keysym_return 0x%x should have stayed 0x%x.",
					TestName, (int) ks, which_key);
				FAIL;
			}
			else
				CHECK;

			if(res != 0)
			{
				report("%s() returned %d instead of 0.",
					TestName, res);
				FAIL;
			}
			else
				CHECK;

			if(strncmp(buffer_return, buf, mlen) != 0)
			{
				report("%s() modified the buffer_return argument \"%s\" should have stayed \"%s\".",
					TestName, buffer_return, buf);
				FAIL;
			}
			else
				CHECK;

     		ic_close(ic);
    	}
	}   /* nextlocale */
	cleanup_locale(NULL,fs,im,db);

	CHECKPASS(4*nlocales()+5*nstyles);
