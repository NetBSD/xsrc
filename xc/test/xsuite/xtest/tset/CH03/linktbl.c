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
 * $XConsortium: linktbl.c,v 1.3 94/04/17 21:02:50 rws Exp $
 */

#include "xtest.h"

extern struct linkinfo EXChngWdwAttr;
extern struct linkinfo EXCrcltSbws;
extern struct linkinfo EXCrcltSbwsDw;
extern struct linkinfo EXCrcltSbwsUp;
extern struct linkinfo EXCnfgrWdw;
extern struct linkinfo EXCrtSmplWdw;
extern struct linkinfo EXCrtWdw;
extern struct linkinfo EXDstrySbws;
extern struct linkinfo EXDstryWdw;
extern struct linkinfo EXLwrWdw;
extern struct linkinfo EXMpRsd;
extern struct linkinfo EXMpSbws;
extern struct linkinfo EXMpWdw;
extern struct linkinfo EXMvRszWdw;
extern struct linkinfo EXMvWdw;
extern struct linkinfo EXRsWdw;
extern struct linkinfo EXRszWdw;
extern struct linkinfo EXRstckWdws;
extern struct linkinfo EXStWdwBg;
extern struct linkinfo EXStWdwBgPxmp;
extern struct linkinfo EXStWdwBrdr;
extern struct linkinfo EXStWdwBrdrPx;
extern struct linkinfo EXStWdwBrdrWd;
extern struct linkinfo EXTrnsltCrdnt;
extern struct linkinfo EXUnmpSbws;
extern struct linkinfo EXUnmpWdw;
extern struct linkinfo EXVslIDFrmVsl;

struct linkinfo *linktbl[] = {
	&EXChngWdwAttr,
	&EXCrcltSbws,
	&EXCrcltSbwsDw,
	&EXCrcltSbwsUp,
	&EXCnfgrWdw,
	&EXCrtSmplWdw,
	&EXCrtWdw,
	&EXDstrySbws,
	&EXDstryWdw,
	&EXLwrWdw,
	&EXMpRsd,
	&EXMpSbws,
	&EXMpWdw,
	&EXMvRszWdw,
	&EXMvWdw,
	&EXRsWdw,
	&EXRszWdw,
	&EXRstckWdws,
	&EXStWdwBg,
	&EXStWdwBgPxmp,
	&EXStWdwBrdr,
	&EXStWdwBrdrPx,
	&EXStWdwBrdrWd,
	&EXTrnsltCrdnt,
	&EXUnmpSbws,
	&EXUnmpWdw,
	&EXVslIDFrmVsl,
	0,
};
