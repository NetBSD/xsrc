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
 * $XConsortium: ShowErr.c,v 1.6 94/04/17 21:01:25 rws Exp $
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

void
Show_Err(mp)
xError *mp;
{
	if (mp->errorCode > FirstExtensionError) {
	    Show_Ext_Err(mp);
	    return;
	}

	switch (mp->errorCode) {
	case BadRequest:
		BPRINTF1("Request:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadValue:
		BPRINTF1("Value:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tresourceID = 0x%08x\n",(long) mp->resourceID);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadWindow:
		BPRINTF1("Window:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tresourceID = 0x%08x\n",mp->resourceID);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadPixmap:
		BPRINTF1("Pixmap:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tresourceID = 0x%08x\n",mp->resourceID);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadAtom:
		BPRINTF1("Atom:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tresourceID = 0x%08x\n",mp->resourceID);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadCursor:
		BPRINTF1("Cursor:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tresourceID = 0x%08x\n",mp->resourceID);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadFont:
		BPRINTF1("Font:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tresourceID = 0x%08x\n",mp->resourceID);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadMatch:
		BPRINTF1("Match:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadDrawable:
		BPRINTF1("Drawable:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tresourceID = 0x%08x\n",mp->resourceID);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadAccess:
		BPRINTF1("Access:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadAlloc:
		BPRINTF1("Alloc:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadColor:
		BPRINTF1("Color:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tresourceID = 0x%08x\n",mp->resourceID);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadGC:
		BPRINTF1("GC:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tresourceID = 0x%08x\n",mp->resourceID);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadIDChoice:
		BPRINTF1("IDChoice:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tresourceID = 0x%08x\n",mp->resourceID);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadName:
		BPRINTF1("Name:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadLength:
		BPRINTF1("Length:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	case BadImplementation:
		BPRINTF1("Implementation:\n");
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	default:
		BPRINTF1("UNKNOWN ERROR TYPE:\n");
		BPRINTF2("\terrorCode = %ld\n",(long) mp->errorCode);
		BPRINTF2("\tsequenceNumber = %d\n",mp->sequenceNumber);
		BPRINTF2("\tminorCode = %d\n",mp->minorCode);
		BPRINTF2("\tmajorCode = %d\n",mp->majorCode);
		break;
	}
}
