/*
 
Copyright (c) 1990, 1991  X Consortium

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
 * Copyright 1990, 1991 by UniSoft Group Limited.
 * 
 * Permission to use, copy, modify, distribute, and sell this software and
 * its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and that
 * both that copyright notice and this permission notice appear in
 * supporting documentation, and that the name of UniSoft not be
 * used in advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.  UniSoft
 * makes no representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 * $XConsortium: SendSup.c,v 1.13 94/04/17 21:01:23 rws Exp $
 */
/*
 * ***************************************************************************
 *  Copyright 1988 by Sequent Computer Systems, Inc., Portland, Oregon       *
 *                                                                           *
 *                                                                           *
 *                          All Rights Reserved                              *
 *                                                                           *
 *  Permission to use, copy, modify, and distribute this software and its    *
 *  documentation for any purpose and without fee is hereby granted,         *
 *  provided that the above copyright notice appears in all copies and that  *
 *  both that copyright notice and this permission notice appear in          *
 *  supporting documentation, and that the name of Sequent not be used       *
 *  in advertising or publicity pertaining to distribution or use of the     *
 *  software without specific, written prior permission.                     *
 *                                                                           *
 *  SEQUENT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING *
 *  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL *
 *  SEQUENT BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR  *
 *  ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,      *
 *  WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,   *
 *  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS      *
 *  SOFTWARE.                                                                *
 * ***************************************************************************
 */

#include "XstlibInt.h"
#include "DataMove.h"

#define SEMI_IMPLEMENTED(str) \
    {\
	char ebuf[132];\
	sprintf(ebuf,"\tSEMI_IMPLEMENTED (%s)\n",str);\
	Log_Debug(ebuf);\
    }

#define CHUNK	1024	/* allocation chunk for o/p buffer, power of 2 */

void
squeeze_me_in(cl, len)
	int cl;
	long len;
{
	unsigned long remaining, used;
	XstDisplay *dpy = Get_Display(cl);

	if (dpy->bufptr < dpy->buffer) {
		Log_Msg ("INTERNAL ERROR: Client %d has bufptr before buffer\n", cl);
		Delete ();
		/* NOTREACHED */
	}
	if (dpy->bufptr > dpy->bufmax) {
		Log_Msg ("INTERNAL ERROR: Client %d has bufptr off end of buffer\n", cl);
		Delete ();
		/* NOTREACHED */
	}
	used = dpy->bufptr - dpy->buffer;
	remaining = dpy->bufmax - dpy->bufptr;
	if (len > (long)remaining) {
		unsigned long total = ((used + len) + (CHUNK-1)) & (~(CHUNK-1));

		debug(3,"buffer expansion required: used = %ld, remaining = %ld, required = %ld, total = %ld\n", used, remaining, len, total);
		if ((dpy->buffer = (char *)Xstrealloc(dpy->buffer, total)) == NULL) {
			Log_Msg ("Could not expand o/p buffer to %d bytes\n", total);
			Delete ();
			/* NOTREACHED */
		}
		dpy->bufptr = dpy->buffer + used;
		dpy->bufmax = dpy->buffer + total;
	}
}

void
Send_Value_List(cl,rp,size,format)
int cl;
xReq *rp;
int size;
int format;
{
    int i;

    switch (format) {
    case 8:
	{
	unsigned char * valuePtr;
	int valueLen;

	valuePtr = (unsigned char *) (((unsigned char *) rp) + size);
	valueLen = ((int)(rp->length<<2)) - size;	/* bytes extra */ 
	if (valueLen < 0) {
	    if (Get_Test_Type(cl) != BAD_LENGTH) {
		Log_Msg("Send_Value_List: FATAL REQUEST LENGTH ERROR!!!\n");
		Log_Msg("\trequest type = %d, length = %d\n",rp->reqType,rp->length);
		Show_Req(rp);
		Delete();
		/*NOTREACHED*/
	    } else
		valueLen = 0;
	} else {
	    squeeze_me_in(cl, (long)valueLen);
	    bcopy(valuePtr,Get_Display(cl)->bufptr,valueLen);
	}
	Get_Display(cl)->bufptr += valueLen;
	}
	break;
    case 16:
	{
	    CARD16 * valuePtr;
	    int valueLen;

	    valuePtr = (CARD16 *) ((unsigned char *) rp + size);
	    valueLen = (((int)(rp->length<<2)) - size)/2;	/* shorts extra */ 

	    squeeze_me_in(cl,  ((long)valueLen) * 2L);
	    for(i=0;i<valueLen;i++) {
		send2(cl,*valuePtr);
		valuePtr++;
	    }
	}
	break;
    case 32:
	{
	    CARD32 * valuePtr;
	    int valueLen;

	    valuePtr = (CARD32 *) ((unsigned char *) rp + size);
	    valueLen = (((int)(rp->length<<2)) - size)/4;	/* longs extra */ 
	    squeeze_me_in(cl,  ((long)valueLen) * 4L);
	    for(i=0;i<valueLen;i++) {
		send4(cl,*valuePtr);
		valuePtr++;
	    }
	}
	break;
    default:
	DEFAULT_ERROR;
	break;
    }
}

void
Send_String16(cl,rp,size)
int cl;
xReq *rp;
int size;
{
    unsigned char *valuePtr;
    int valueLen;

    valuePtr = (unsigned char *) ((unsigned char *) rp + size);
    valueLen = ((int)(rp->length<<2)) - size;	/* bytes extra */ 
    if(valueLen < 1)
        return;
    squeeze_me_in(cl, (long)valueLen);
    bcopy((char *)valuePtr,Get_Display(cl)->bufptr,valueLen);
    Get_Display(cl)->bufptr += valueLen;
}

/*
 *	Routine: Send_CHAR2B - stuffs CHAR2B's in lsb format into a buffer 
 *               holding the bits for a request.  This is needed for 16
 *               bit font operations.
 *
 *	Input: cl - client
 *             rp - pointer to the request
 *             size - basic size of request
 *
 *	Output: output buffer has CHAR2B's in lsb format
 *
 *	Returns: nothing
 *
 *	Globals used:
 *
 *	Side Effects:
 *
 *	Methods:
 *
 */

void
Send_CHAR2B (cl, rp, size)
    int cl;
    xReq *rp;
    int size;
{
    unsigned short * valuePtr;
    int valueLen;
    int i;

    valuePtr = (unsigned short *) ((unsigned char *) rp + size);
    valueLen = (((int)(rp->length<<2)) - size)/2;	/* shorts extra */ 

    squeeze_me_in(cl, ((long)valueLen) * 2L);
    for (i = 0; i < valueLen; i++) {
	send2_lsb(cl,*valuePtr);
	valuePtr++;
    }
}

void
Send_TextItem8(cl,rp,size)
int cl;
xReq *rp;
int size;
{
    unsigned char *valuePtr;
    int valueLen;

    valuePtr = (unsigned char *) ((unsigned char *) rp + size);
    valueLen = ((int)(rp->length<<2)) - size;	/* bytes extra */ 
    if(valueLen < 1)
        return;
    squeeze_me_in(cl, (long)valueLen);
    bcopy((char *)valuePtr,Get_Display(cl)->bufptr,valueLen);
    Get_Display(cl)->bufptr += valueLen;
}

void
Send_TextItem16(cl,rp,size)
int cl;
xReq *rp;
int size;
  {
      unsigned char *valuePtr;
      int valueLen;
      int i;
  
      valuePtr = (unsigned char *) ((unsigned char *) rp + size);
      valueLen = ((int)(rp->length<<2)) - size;	/* bytes extra */ 
      if(valueLen < 4)
          return;
      squeeze_me_in(cl, (long)valueLen); /* Still needed? XXX ..sr */
      send1(cl,*valuePtr);                 /* n */
      valuePtr++;
      send1(cl,*valuePtr);                 /* delta */
      valuePtr++;
      for (i = 0; i < (valueLen-2)/2; i ++) { 
  	send2_lsb(cl, *((short *) valuePtr));  /* force it to send lsb order */
  	valuePtr += 2;
      }
  }

void
SendIt(cl,expected_bytes,length)
int cl;
int expected_bytes;
int length;
{
    int writelen;
    int actual_bytes;
    XstDisplay *dpy = Get_Display(cl);

    writelen = dpy->bufptr - dpy->buffer;	/* number of bytes to write */
    actual_bytes = max(expected_bytes, sizeof(xReq)); /* ensure something sent */
    if (Get_Test_Type(cl) == JUST_TOO_LONG) {
	/* try not to confuse servers too much on badly framed things */
	actual_bytes = max(writelen, actual_bytes);
    }
    Log_Debug2("SendIt: writelen is %d; actual bytes is %d; expected_bytes is %d.\n",
				writelen, actual_bytes, expected_bytes);
    /*
     * Next few lines altered as part of an effort to allow comprehensive
     * testing of BadLength errors.
     * We now write what's expected - this may be what was requested in the length 
     * field on the call to Send_Req - but make sure it's at least something.
     */
    /*
     *	Sanity check 
     */
    switch (Get_Test_Type(cl)) {
	case OPEN_DISPLAY:
	case SETUP:
	case GOOD:
	    if (writelen != expected_bytes)
		Log_Trace("SendIt: size of buffer (%d) doesn't agree with request length(%d)!\n",
				writelen, actual_bytes);
	    break;
	case TOO_LONG:
	    {
		Log_Debug3 ("TOO_LONG test, length field was %ld, %ld in buffer, but really send %d bytes\n",
			length, writelen, actual_bytes);
	    }
	    break;
    }

    if (dpy->fd < 0) Log_Trace("SendIt: fd closed");
  
    /* ensure that we've got the required number of bytes in the buffer.
     * Only likely to be required on TOO_LONG tests but useful check anyway.
     */
    squeeze_me_in(cl, actual_bytes);

     {
      long wrote;
      long leftbytes=actual_bytes;
      char *buf=dpy->buffer;
      while (leftbytes>0) {
  	wrote=write(ConnectionNumber(dpy),buf,leftbytes);
  	if (wrote == -1) {
  	    if (errno==EINTR || errno==EWOULDBLOCK || errno==EAGAIN)
  		continue;
  	    else
  		break;
  	}
  	leftbytes -= wrote;
  	buf += wrote;
      }
     }
    dpy->bufptr = dpy->buffer;
}
