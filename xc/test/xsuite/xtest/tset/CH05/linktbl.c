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
 * $XConsortium: linktbl.c,v 1.3 94/04/17 21:03:40 rws Exp $
 */

#include "xtest.h"

extern struct linkinfo EXAllcClr;
extern struct linkinfo EXAllcClrClls;
extern struct linkinfo EXAllcClrPlns;
extern struct linkinfo EXAllcNmdClr;
extern struct linkinfo EXChngGC;
extern struct linkinfo EXCpyClrmpAnd;
extern struct linkinfo EXCpyGC;
extern struct linkinfo EXCrtClrmp;
extern struct linkinfo EXCrtGC;
extern struct linkinfo EXCrtPxmp;
extern struct linkinfo EXFrClrmp;
extern struct linkinfo EXFrClrs;
extern struct linkinfo EXFrGC;
extern struct linkinfo EXFrPxmp;
extern struct linkinfo EXGCntxtFrmGC;
extern struct linkinfo EXGtGCVls;
extern struct linkinfo EXLkpClr;
extern struct linkinfo EXQryBstSz;
extern struct linkinfo EXQryBstStppl;
extern struct linkinfo EXQryBstTl;
extern struct linkinfo EXQryClr;
extern struct linkinfo EXQryClrs;
extern struct linkinfo EXStArcMd;
extern struct linkinfo EXStBg;
extern struct linkinfo EXStClpMsk;
extern struct linkinfo EXStClpOrgn;
extern struct linkinfo EXStClpRctngl;
extern struct linkinfo EXStDshs;
extern struct linkinfo EXStFllRl;
extern struct linkinfo EXStFllStyl;
extern struct linkinfo EXStFnt;
extern struct linkinfo EXStFrgrnd;
extern struct linkinfo EXStFnctn;
extern struct linkinfo EXStGrphcsExp;
extern struct linkinfo EXStLnAttrbts;
extern struct linkinfo EXStPlnMsk;
extern struct linkinfo EXStStt;
extern struct linkinfo EXStStppl;
extern struct linkinfo EXStSbwMd;
extern struct linkinfo EXStTSOrgn;
extern struct linkinfo EXStTl;
extern struct linkinfo EXStWdwClrmp;
extern struct linkinfo EXStrClr;
extern struct linkinfo EXStrClrs;
extern struct linkinfo EXStrNmdClr;

struct linkinfo *linktbl[] = {
	&EXAllcClr,
	&EXAllcClrClls,
	&EXAllcClrPlns,
	&EXAllcNmdClr,
	&EXChngGC,
	&EXCpyClrmpAnd,
	&EXCpyGC,
	&EXCrtClrmp,
	&EXCrtGC,
	&EXCrtPxmp,
	&EXFrClrmp,
	&EXFrClrs,
	&EXFrGC,
	&EXFrPxmp,
	&EXGCntxtFrmGC,
	&EXGtGCVls,
	&EXLkpClr,
	&EXQryBstSz,
	&EXQryBstStppl,
	&EXQryBstTl,
	&EXQryClr,
	&EXQryClrs,
	&EXStArcMd,
	&EXStBg,
	&EXStClpMsk,
	&EXStClpOrgn,
	&EXStClpRctngl,
	&EXStDshs,
	&EXStFllRl,
	&EXStFllStyl,
	&EXStFnt,
	&EXStFrgrnd,
	&EXStFnctn,
	&EXStGrphcsExp,
	&EXStLnAttrbts,
	&EXStPlnMsk,
	&EXStStt,
	&EXStStppl,
	&EXStSbwMd,
	&EXStTSOrgn,
	&EXStTl,
	&EXStWdwClrmp,
	&EXStrClr,
	&EXStrClrs,
	&EXStrNmdClr,
	0,
};
