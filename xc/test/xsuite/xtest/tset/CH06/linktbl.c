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
 * $XConsortium: linktbl.c,v 1.3 94/04/17 21:04:26 rws Exp $
 */

#include "xtest.h"

extern struct linkinfo EXClrAr;
extern struct linkinfo EXClrWdw;
extern struct linkinfo EXCpyAr;
extern struct linkinfo EXCpyPln;
extern struct linkinfo EXCrtFntCrsr;
extern struct linkinfo EXCrtGlyphCrs;
extern struct linkinfo EXCrtPxmpCrsr;
extern struct linkinfo EXDfnCrsr;
extern struct linkinfo EXDrwArc;
extern struct linkinfo EXDrwArcs;
extern struct linkinfo EXDrwImgStr;
extern struct linkinfo EXDrwImgStr16;
extern struct linkinfo EXDrwLn;
extern struct linkinfo EXDrwLns;
extern struct linkinfo EXDrwPnt;
extern struct linkinfo EXDrwPnts;
extern struct linkinfo EXDrwRctngl;
extern struct linkinfo EXDrwRctngls;
extern struct linkinfo EXDrwSgmnts;
extern struct linkinfo EXDrwStr;
extern struct linkinfo EXDrwStr16;
extern struct linkinfo EXDrwTxt;
extern struct linkinfo EXDrwTxt16;
extern struct linkinfo EXFllArc;
extern struct linkinfo EXFllArcs;
extern struct linkinfo EXFllPlygn;
extern struct linkinfo EXFllRctngl;
extern struct linkinfo EXFllRctngls;
extern struct linkinfo EXFrCrsr;
extern struct linkinfo EXFrFnt;
extern struct linkinfo EXFrFntInf;
extern struct linkinfo EXFrFntNms;
extern struct linkinfo EXFrFntPth;
extern struct linkinfo EXGtFntPth;
extern struct linkinfo EXGtFntPrprty;
extern struct linkinfo EXGtImg;
extern struct linkinfo EXGtSbImg;
extern struct linkinfo EXLstFnts;
extern struct linkinfo EXLstFntsWthI;
extern struct linkinfo EXLdFnt;
extern struct linkinfo EXLdQryFnt;
extern struct linkinfo EXPtImg;
extern struct linkinfo EXQryBstCrsr;
extern struct linkinfo EXQryFnt;
extern struct linkinfo EXQryTxtExtnt;
extern struct linkinfo EXQryTxtExt16;
extern struct linkinfo EXRclrCrsr;
extern struct linkinfo EXStFntPth;
extern struct linkinfo EXTxtExtnts;
extern struct linkinfo EXTxtExtnts16;
extern struct linkinfo EXTxtWdth;
extern struct linkinfo EXTxtWdth16;
extern struct linkinfo EXUndfnCrsr;
extern struct linkinfo EXUnldFnt;

struct linkinfo *linktbl[] = {
	&EXClrAr,
	&EXClrWdw,
	&EXCpyAr,
	&EXCpyPln,
	&EXCrtFntCrsr,
	&EXCrtGlyphCrs,
	&EXCrtPxmpCrsr,
	&EXDfnCrsr,
	&EXDrwArc,
	&EXDrwArcs,
	&EXDrwImgStr,
	&EXDrwImgStr16,
	&EXDrwLn,
	&EXDrwLns,
	&EXDrwPnt,
	&EXDrwPnts,
	&EXDrwRctngl,
	&EXDrwRctngls,
	&EXDrwSgmnts,
	&EXDrwStr,
	&EXDrwStr16,
	&EXDrwTxt,
	&EXDrwTxt16,
	&EXFllArc,
	&EXFllArcs,
	&EXFllPlygn,
	&EXFllRctngl,
	&EXFllRctngls,
	&EXFrCrsr,
	&EXFrFnt,
	&EXFrFntInf,
	&EXFrFntNms,
	&EXFrFntPth,
	&EXGtFntPth,
	&EXGtFntPrprty,
	&EXGtImg,
	&EXGtSbImg,
	&EXLstFnts,
	&EXLstFntsWthI,
	&EXLdFnt,
	&EXLdQryFnt,
	&EXPtImg,
	&EXQryBstCrsr,
	&EXQryFnt,
	&EXQryTxtExtnt,
	&EXQryTxtExt16,
	&EXRclrCrsr,
	&EXStFntPth,
	&EXTxtExtnts,
	&EXTxtExtnts16,
	&EXTxtWdth,
	&EXTxtWdth16,
	&EXUndfnCrsr,
	&EXUnldFnt,
	0,
};
