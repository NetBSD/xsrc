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
 * $XConsortium: ShowSup.c,v 1.8 94/04/17 21:01:32 rws Exp $
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

static char scratchbuf[PRTBUFSIZ];  /* ? */

/* 
   intent:	 format value list of request for output
   input:
                 rp -   pointer to request
		 size - size of basic request type (without size of
		        value list).
		 format - one of
		    FORMAT8 - 8 bit quantity
		    FORMAT16 - 16 bit quantity
		    FORMAT32 - 32 bit quantity
		    FORMATtimecoord - TIMECOORD from protocol spec
		    FORMATrgb - RGB from protocol spec
		    FORMATpoint - POINT from protocol spec
		    FORMATrectangle - RECTANGLE from protocol spec
		    FORMATarc - ARC from protocol spec
		    FORMATcoloritem - COLORITEM from protocol spec
   output:	 
   global input: 
   side effects: value list is formatted for output in nice human-readable
                 form.
   methods:	 Compute number of bytes to print:  length<<2 - size.
                 Bytes per item is implicit in FORMAT type.
                 Then the number of items to print is:
                      (bytes to print) / (bytes per item)
                 
*/

Show_Value_List_Req (rp, size, format)
xReq * rp;
int     size;
int	format;
{
	Reset_Some();
	switch (format) {
	case FORMAT8:  {
		unsigned char  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (unsigned char *) ((unsigned char *) rp + size);
		valueLen = (rp -> length << 2) - size;/* bytes extra */
		for (i = 0; i < (valueLen); i++) {
			Log_Some("\tvalue[%d] = 0x%x, %d\n", i, *valuePtr, *valuePtr);
			valuePtr++;
		}
		}
		break;
	case FORMAT16:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp -> length << 2) - size;/* bytes extra */
		for (i = 0; i < (valueLen / 2); i++) {
			Log_Some("\tvalue[%d] = 0x%x, %d\n", i, *valuePtr, *valuePtr);
			valuePtr++;
		}
		}
		break;
        case FORMAT32:  {
		CARD32  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD32 *) ((unsigned char *) rp + size);
		valueLen = (rp -> length << 2) - size;/* bytes extra */
		for (i = 0; i < (valueLen / 4); i++) {
			Log_Some("\tvalue[%d] = 0x%lx, %ld\n", i, *valuePtr, *valuePtr);
			valuePtr++;
		}
		}
		break;
	case FORMATtimecoord:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp -> length << 2) - size;/* bytes extra */
		for (i = 0; i < (valueLen / 8); i++) {
			Log_Some("\ttime[%d] = %ld\tx[%d] = %d\ty[%d] = %d\n", i, *((CARD32 *)valuePtr), i, *(valuePtr+2), i, *(valuePtr+3));
			valuePtr += 4;
		}
		}
		break;
	case FORMATrgb:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp -> length << 2) - size;/* bytes extra */
		for (i = 0; i < (valueLen / 8); i++) {
			Log_Some("\tred[%d] = %d\tgreen[%d] = %d\tblue[%d] = %d\n", i, *valuePtr, i, *(valuePtr+1), i, *(valuePtr+2));
			valuePtr += 4;
		}
		}
		break;
	case FORMATpoint:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp -> length << 2) - size;/* bytes extra */
		for (i = 0; i < (valueLen / 4); i++) {
			Log_Some("\tx, y [%d] = %d, %d\n",
				 i, *valuePtr, *(valuePtr+1));
			valuePtr += 2;
		}
		}
		break;
	case FORMATrectangle:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp -> length << 2) - size;/* bytes extra */
		for (i = 0; i < (valueLen / 8); i++) {
			Log_Some("\tx, y, width, height [%d] = %d, %d, %d, %d\n", i, *valuePtr, *(valuePtr+1), *(valuePtr+2), *(valuePtr+3));
			valuePtr += 4;
		}
		}
		break;
	case FORMATarc:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp -> length << 2) - size;/* bytes extra */
		for (i = 0; i < (valueLen / 12); i++) {
			Log_Some("\tx, y, width, height,angle1, angle2 [%d] = %d, %d, %d, %d, %d, %d\n", i, *valuePtr, *(valuePtr+1), *(valuePtr+2), *(valuePtr+3), *(valuePtr+4), *(valuePtr+5));
			valuePtr += 6;
		}
		}
		break;
	case FORMATcoloritem:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp -> length << 2) - size;/* bytes extra */
		for (i = 0; i < (valueLen / 12); i++) {
			Log_Some("\tpixel, red, grn, blue, rest [%d] = %ld, %d, %d, %d, %d\n", i, *((CARD32 *)valuePtr), *(valuePtr+2), *(valuePtr+3), *(valuePtr+4), *(valuePtr+5));
			valuePtr += 6;
		}
		}
		break;
	default:
		DEFAULT_ERROR;
	}
}


/* 
   intent:	 format value list of reply for output
   input:
                 rp -   pointer to reply
		 size - size of basic reply type (without size of
		        value list).
		 format - one of
		    FORMAT8 - 8 bit quantity
		    FORMAT16 - 16 bit quantity
		    FORMAT32 - 32 bit quantity
		    FORMATtimecoord - TIMECOORD from protocol spec
		    FORMATrgb - RGB from protocol spec
		    FORMATpoint - POINT from protocol spec
		    FORMATrectangle - RECTANGLE from protocol spec
		    FORMATarc - ARC from protocol spec
		    FORMATcoloritem - COLORITEM from protocol spec
   output:	 
   global input: 
   side effects: value list is formatted for output in nice human-readable
                 form.
   methods:	 Compute number of bytes to print:  length<<2, since
                 all replies are the same length, so the length field
		 just tells you the size of the value list.
                 Bytes per item is implicit in FORMAT type.
                 Then the number of items to print is:
                      (bytes to print) / (bytes per item)

                 
*/

Show_Value_List_Rep (rp, size, format)
xReply  *rp;
int     size;
int	format;
{
	Reset_Some();
	switch(format){
	case FORMAT8:  {
		unsigned char  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (unsigned char *) ((unsigned char *) rp + size);
		valueLen = (rp->generic.length << 2); /* bytes extra */
		for (i = 0; i < (valueLen); i++) {
			Log_Some("\tvalue[%d] = 0x%x, %d\n", i, *valuePtr, *valuePtr);
			valuePtr++;
		}
		}
		break;
	case FORMAT16:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp->generic.length << 2); /* bytes extra */
		for (i = 0; i < (valueLen / 2); i++) {
			Log_Some("\tvalue[%d] = 0x%x, %d\n", i, *valuePtr, *valuePtr);
			valuePtr++;
		}
		}
		break;
	case FORMAT32:  {
		CARD32  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD32 *) ((unsigned char *) rp + size);
		valueLen = (rp->generic.length << 2); /* bytes extra */
		for (i = 0; i < (valueLen / 4); i++) {
			Log_Some("\tvalue[%d] = 0x%lx, %ld\n", i, *valuePtr, *valuePtr);
			valuePtr++;
		}
		}
		break;
	case FORMATtimecoord:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp->generic.length << 2); /* bytes extra */
		for (i = 0; i < (valueLen / 8); i++) {
			Log_Some("\ttime[%d] = %ld\tx[%d] = %d\ty[%d] = %d\n", i, *((CARD32 *)valuePtr), i, *(valuePtr+2), i, *(valuePtr+3));
			valuePtr += 4;
		}
		}
		break;
	case FORMATrgb:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp->generic.length << 2); /* bytes extra */
		for (i = 0; i < (valueLen / 8); i++) {
			Log_Some("\tred[%d] = %d\tgreen[%d] = %d\tblue[%d] = %d\n", i, *valuePtr, i, *(valuePtr+1), i, *(valuePtr+2));
			valuePtr += 4;
		}
		}
		break;
	case FORMATpoint:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp->generic.length << 2); /* bytes extra */
		for (i = 0; i < (valueLen / 4); i++) {
			Log_Some("\tx, y [%d] = %d, %d\n",
				 i, *valuePtr, *(valuePtr+1));
			valuePtr += 2;
		}
		}
		break;
	case FORMATrectangle:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp->generic.length << 2); /* bytes extra */
		for (i = 0; i < (valueLen / 8); i++) {
			Log_Some("\tx, y, width, height [%d] = %d, %d, %d, %d\n", i, *valuePtr, *(valuePtr+1), *(valuePtr+2), *(valuePtr+3));
			valuePtr += 4;
		}
		}
		break;
	case FORMATarc:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp->generic.length << 2); /* bytes extra */
		for (i = 0; i < (valueLen / 12); i++) {
			Log_Some("\tx, y, width, height,angle1, angle2 [%d] = %d, %d, %d, %d, %d, %d\n", i, *valuePtr, *(valuePtr+1), *(valuePtr+2), *(valuePtr+3), *(valuePtr+4), *(valuePtr+5));
			valuePtr += 6;
		}
		}
		break;
	case FORMATcoloritem:  {
		CARD16  *valuePtr;
		int     valueLen;
		int     i;

		valuePtr = (CARD16 *) ((unsigned char *) rp + size);
		valueLen = (rp->generic.length << 2); /* bytes extra */
		for (i = 0; i < (valueLen / 12); i++) {
			Log_Some("\tpixel, red, grn, blue, rest [%d] = %ld, %d, %d, %d, %d\n", i, *((CARD32 *)valuePtr), *(valuePtr+2), *(valuePtr+3), *(valuePtr+4), *(valuePtr+5));
			valuePtr += 6;
		    }
	    }
	break;
	case FORMATcharinfo:  {
	    CARD16  *valuePtr;
	    int     valueLen;
	    int     i;

	    valuePtr = (CARD16 *) ((unsigned char *) rp + size);
	    valueLen = (rp->generic.length << 2); /* bytes extra */
	    for (i = 0; i < (valueLen / 12); i++) {
		Log_Some("\tcharinfo %d, left-side-bearing = %d, right-side-bearing = %d, character-width = %d, ascent = %d, descent = %d, attributes = 0x%x\n", i, *valuePtr, *(valuePtr+1), *(valuePtr+2), *(valuePtr+3), *(valuePtr+4), *(valuePtr+5));
		valuePtr += 6;
	    }
	}
	break;
	default:
		DEFAULT_ERROR;
	}
}


/* 
   intent:	 display a counted string in a value list
   input:
                 rp -   pointer to reply
		 size - size of basic reply type (without size of
		        value list).
		 length - number of characters in the string
   output:	 
   global input: 
   side effects: value list is formatted for output in nice human-readable
                 form.
   methods:	 would be good to think through how to optimize the way
                 it's done now, since it's a little silly.
*/

Show_String8 (rp, size, length)
xReq * rp;
int     size;
int	length;
{
	unsigned char  *valuePtr;
	int     i;

	Reset_Some();
	valuePtr = (unsigned char *) ((unsigned char *) rp + size);
	Log_Some("\tvalue = \"");
	for (i = 0; i < length; i++) {
		Log_Some("%c", *valuePtr);
		valuePtr++;
	}
	Log_Some("\"\n");
}

/* 
   intent:	 display a set of counted strings in a value list
   input:
                 cp -     pointer to first of counted strings
		 nstrs -  number of counted strings
		 nbytes - total number of bytes passed in (not yet used) -
		          put in for possible consistency check, by adding
			  up bytes as you cycle through the strings.
		 label -  string describing what each counted string is,
		          e.g. "FontName"
   output:	 
   global input: 
   side effects: value list is formatted for output in nice human-readable
                 form.
   methods:	 buffer that's passed in is expected to look like:

                 +---+--------------+---+--------------+-----+
                 | n | n characters | m | m characters | ... |
		 +---+--------------+---+--------------+-----+

		 method is to copy each chunk into a temporary buffer,
		 copy \0 to end, for example

		 +-----------------+
		 | n characters \0 |
		 +-----------------+

		 and print it to scratchbuf
                 
*/

void
Show_Strs(cp, nstrs, nbytes, label)
unsigned char *cp;
int nstrs;
int nbytes;
char *label;
{
    int i;
    int len;

	Reset_Some();
    for(i = 0; i < nstrs; i++) {
	len = *cp;
	bcopy (cp + 1, scratchbuf, len);
	scratchbuf[len] = '\0';
	Log_Some( "\t%s[%d] = \"%s\"\n", label, i, scratchbuf);
	cp += len + 1;
    }
    return;
}

/* 
   intent:	 format value list of reply for output, for case where
                 there is at least one more value list following this one.
   input:
                 rp -   pointer to reply
		 nval - number of values in value list
		 size - size of basic reply type (without size of
		        value list).
		 format - one of
		    FORMATfontprop - FONTPROP from protocol spec
		    FORMATcharinfo - CHARINFO from protocol spec
   output:	 
   global input: 
   side effects: value list is formatted for output in nice human-readable
                 form.
   methods:	 Bytes per item is implicit in FORMAT type.
*/

Show_Value_List_nRep (rp, nval, size, format)
xReply  *rp;
int     nval;
int     size;
int	format;
{
	Reset_Some();
	switch(format){
	case FORMATfontprop:  {
		CARD32  *valuePtr;
		int     i;
		
		if (nval > 0) {
		    valuePtr = (CARD32 *) ((CARD32 *) rp + size);
		    for (i = 0; i < nval; i++) {
			Log_Some("\tfontprop %d, name = 0x%lx, value = 0x%lx\n", i, *valuePtr, *(valuePtr+1));
			valuePtr += 2;
		    }
		}
		else if (nval == 0)
		    Log_Some( "\tno fontprops\n");
		else 
		    Log_Some( "\tERROR - number of fontprops is less than 0\n");
	    }
	    break;
	case FORMATcharinfo:  {
		CARD16 *valuePtr;
		int     i;

		if (nval > 0) {
		    valuePtr = (CARD16 *) ((CARD16 *) rp + size);
		    for (i = 0; i < nval; i++) {
			Log_Some("\tcharinfo %d, left-side-bearing = %d, right-side-bearing = %d, character-width = %d, ascent = %d, descent = %d, attributes = 0x%x\n", i, *valuePtr, *(valuePtr+1), *(valuePtr+2), *(valuePtr+3), *(valuePtr+4), *(valuePtr+5));
			valuePtr += 6;
		    }
		}
		else if (nval == 0)
		    Log_Some( "\tno charinfos\n");
		else 
		    Log_Some( "\tERROR - number of charinfos is less than 0\n");
		break;
	default:
		DEFAULT_ERROR;
	}
    }
}
