/*
 
Copyright (c) 1990, 1991, 1992  X Consortium

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
 * Copyright 1990, 1991, 1992 by UniSoft Group Limited.
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
 * $XConsortium: linktbl.c,v 1.3 94/04/17 21:09:18 rws Exp $
 */

#include "xtest.h"

extern struct linkinfo EIsCrsrKy;
extern struct linkinfo EIsFnctnKy;
extern struct linkinfo EIsKypdKy;
extern struct linkinfo EIsMscFnctnKy;
extern struct linkinfo EIsMdfrKy;
extern struct linkinfo EIsPFKy;
extern struct linkinfo EXAddPxl;
extern struct linkinfo EXClpBx;
extern struct linkinfo EXCrtBtmpFrmD;
extern struct linkinfo EXCrtImg;
extern struct linkinfo EXCrtPxmpFrmB;
extern struct linkinfo EXCrtRgn;
extern struct linkinfo EXDltCntxt;
extern struct linkinfo EXDstryImg;
extern struct linkinfo EXDstryRgn;
extern struct linkinfo EXEmptyRgn;
extern struct linkinfo EXEqlRgn;
extern struct linkinfo EXFtchBffr;
extern struct linkinfo EXFtchByts;
extern struct linkinfo EXFndCntxt;
extern struct linkinfo EXGtDflt;
extern struct linkinfo EXGtPxl;
extern struct linkinfo EXGtVslInf;
extern struct linkinfo EXIntrsctRgn;
extern struct linkinfo EXKycdTKysym;
extern struct linkinfo EXKysymTKycd;
extern struct linkinfo EXKysymTStr;
extern struct linkinfo EXLkpKysym;
extern struct linkinfo EXLkpStr;
extern struct linkinfo EXMtchVslInf;
extern struct linkinfo EXOffstRgn;
extern struct linkinfo EXPrsClr;
extern struct linkinfo EXPrsGmtry;
extern struct linkinfo EXPntInRgn;
extern struct linkinfo EXPlygnRgn;
extern struct linkinfo EXPtPxl;
extern struct linkinfo EXRdBtmpFl;
extern struct linkinfo EXRbndKysym;
extern struct linkinfo EXRctInRgn;
extern struct linkinfo EXRfrshKybrdM;
extern struct linkinfo EXRsrcMngrStr;
extern struct linkinfo EXRttBffrs;
extern struct linkinfo EXSvCntxt;
extern struct linkinfo EXStRgn;
extern struct linkinfo EXShrnkRgn;
extern struct linkinfo EXStrBffr;
extern struct linkinfo EXStrByts;
extern struct linkinfo EXStrTKysym;
extern struct linkinfo EXSbImg;
extern struct linkinfo EXSbtrctRgn;
extern struct linkinfo EXUnnRctWthRg;
extern struct linkinfo EXUnnRgn;
extern struct linkinfo EXUnqCntxt;
extern struct linkinfo EXWrtBtmpFl;
extern struct linkinfo EXXrRgn;
extern struct linkinfo EXprmllc;
extern struct linkinfo EXrmDstryDtbs;
extern struct linkinfo EXrmGtFlDtbs;
extern struct linkinfo EXrmGtRsrc;
extern struct linkinfo EXrmGtStrDtbs;
extern struct linkinfo EXrmIntlz;
extern struct linkinfo EXrmMrgDtbss;
extern struct linkinfo EXrmPrsCmmnd;
extern struct linkinfo EXrmPtFlDtbs;
extern struct linkinfo EXrmPtLnRsrc;
extern struct linkinfo EXrmPtRsrc;
extern struct linkinfo EXrmPtStrRsrc;
extern struct linkinfo EXrmQGtRsrc;
extern struct linkinfo EXrmQGtSrchLs;
extern struct linkinfo EXrmQGtSrchRs;
extern struct linkinfo EXrmQPtRsrc;
extern struct linkinfo EXrmQPtStrRsr;
extern struct linkinfo EXrmQrkTStr;
extern struct linkinfo EXrmStrTBndng;
extern struct linkinfo EXrmStrTQrk;
extern struct linkinfo EXrmStrTQrkLs;
extern struct linkinfo EXrmUnqQrk;

struct linkinfo *linktbl[] = {
	&EIsCrsrKy,
	&EIsFnctnKy,
	&EIsKypdKy,
	&EIsMscFnctnKy,
	&EIsMdfrKy,
	&EIsPFKy,
	&EXAddPxl,
	&EXClpBx,
	&EXCrtBtmpFrmD,
	&EXCrtImg,
	&EXCrtPxmpFrmB,
	&EXCrtRgn,
	&EXDltCntxt,
	&EXDstryImg,
	&EXDstryRgn,
	&EXEmptyRgn,
	&EXEqlRgn,
	&EXFtchBffr,
	&EXFtchByts,
	&EXFndCntxt,
	&EXGtDflt,
	&EXGtPxl,
	&EXGtVslInf,
	&EXIntrsctRgn,
	&EXKycdTKysym,
	&EXKysymTKycd,
	&EXKysymTStr,
	&EXLkpKysym,
	&EXLkpStr,
	&EXMtchVslInf,
	&EXOffstRgn,
	&EXPrsClr,
	&EXPrsGmtry,
	&EXPntInRgn,
	&EXPlygnRgn,
	&EXPtPxl,
	&EXRdBtmpFl,
	&EXRbndKysym,
	&EXRctInRgn,
	&EXRfrshKybrdM,
	&EXRsrcMngrStr,
	&EXRttBffrs,
	&EXSvCntxt,
	&EXStRgn,
	&EXShrnkRgn,
	&EXStrBffr,
	&EXStrByts,
	&EXStrTKysym,
	&EXSbImg,
	&EXSbtrctRgn,
	&EXUnnRctWthRg,
	&EXUnnRgn,
	&EXUnqCntxt,
	&EXWrtBtmpFl,
	&EXXrRgn,
	&EXprmllc,
	&EXrmDstryDtbs,
	&EXrmGtFlDtbs,
	&EXrmGtRsrc,
	&EXrmGtStrDtbs,
	&EXrmIntlz,
	&EXrmMrgDtbss,
	&EXrmPrsCmmnd,
	&EXrmPtFlDtbs,
	&EXrmPtLnRsrc,
	&EXrmPtRsrc,
	&EXrmPtStrRsrc,
	&EXrmQGtRsrc,
	&EXrmQGtSrchLs,
	&EXrmQGtSrchRs,
	&EXrmQPtRsrc,
	&EXrmQPtStrRsr,
	&EXrmQrkTStr,
	&EXrmStrTBndng,
	&EXrmStrTQrk,
	&EXrmStrTQrkLs,
	&EXrmUnqQrk,
	0,
};
