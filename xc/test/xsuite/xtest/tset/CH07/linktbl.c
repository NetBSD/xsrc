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
 * $XConsortium: linktbl.c,v 1.3 94/04/17 21:06:00 rws Exp $
 */

#include "xtest.h"

extern struct linkinfo EXActvtScrnSv;
extern struct linkinfo EXAddHst;
extern struct linkinfo EXAddHsts;
extern struct linkinfo EXAddTSvSt;
extern struct linkinfo EXAllwEvnts;
extern struct linkinfo EXAtRptOff;
extern struct linkinfo EXAtRptOn;
extern struct linkinfo EXBll;
extern struct linkinfo EXChngActvPnt;
extern struct linkinfo EXChngKybrdCn;
extern struct linkinfo EXChngKybrdMp;
extern struct linkinfo EXChngPntrCnt;
extern struct linkinfo EXChngSvSt;
extern struct linkinfo EXDltMdfrmpEn;
extern struct linkinfo EXDsblAccssCn;
extern struct linkinfo EXDsplyKycds;
extern struct linkinfo EXEnblAccssCn;
extern struct linkinfo EXFrcScrnSvr;
extern struct linkinfo EXFrMdfrmp;
extern struct linkinfo EXGtInptFcs;
extern struct linkinfo EXGtKybrdCntr;
extern struct linkinfo EXGtKybrdMppn;
extern struct linkinfo EXGtMdfrMppng;
extern struct linkinfo EXGtPntrCntrl;
extern struct linkinfo EXGtPntrMppng;
extern struct linkinfo EXGtScrnSvr;
extern struct linkinfo EXGrbBttn;
extern struct linkinfo EXGrbKy;
extern struct linkinfo EXGrbKybrd;
extern struct linkinfo EXGrbPntr;
extern struct linkinfo EXGrbSrvr;
extern struct linkinfo EXInsrtMdfrmp;
extern struct linkinfo EXInstllClrmp;
extern struct linkinfo EXKllClnt;
extern struct linkinfo EXLstHsts;
extern struct linkinfo EXLstInstlldC;
extern struct linkinfo EXNwMdfrmp;
extern struct linkinfo EXQryKymp;
extern struct linkinfo EXRmvFrmSvSt;
extern struct linkinfo EXRmvHst;
extern struct linkinfo EXRmvHsts;
extern struct linkinfo EXRprntWdw;
extern struct linkinfo EXRstScrnSvr;
extern struct linkinfo EXStAccssCntr;
extern struct linkinfo EXStClsDwnMd;
extern struct linkinfo EXStInptFcs;
extern struct linkinfo EXStMdfrMppng;
extern struct linkinfo EXStPntrMppng;
extern struct linkinfo EXStScrnSvr;
extern struct linkinfo EXUngrbBttn;
extern struct linkinfo EXUngrbKy;
extern struct linkinfo EXUngrbKybrd;
extern struct linkinfo EXUngrbPntr;
extern struct linkinfo EXUngrbSrvr;
extern struct linkinfo EXUnnstllClrm;
extern struct linkinfo EXWrpPntr;

struct linkinfo *linktbl[] = {
	&EXActvtScrnSv,
	&EXAddHst,
	&EXAddHsts,
	&EXAddTSvSt,
	&EXAllwEvnts,
	&EXAtRptOff,
	&EXAtRptOn,
	&EXBll,
	&EXChngActvPnt,
	&EXChngKybrdCn,
	&EXChngKybrdMp,
	&EXChngPntrCnt,
	&EXChngSvSt,
	&EXDltMdfrmpEn,
	&EXDsblAccssCn,
	&EXDsplyKycds,
	&EXEnblAccssCn,
	&EXFrcScrnSvr,
	&EXFrMdfrmp,
	&EXGtInptFcs,
	&EXGtKybrdCntr,
	&EXGtKybrdMppn,
	&EXGtMdfrMppng,
	&EXGtPntrCntrl,
	&EXGtPntrMppng,
	&EXGtScrnSvr,
	&EXGrbBttn,
	&EXGrbKy,
	&EXGrbKybrd,
	&EXGrbPntr,
	&EXGrbSrvr,
	&EXInsrtMdfrmp,
	&EXInstllClrmp,
	&EXKllClnt,
	&EXLstHsts,
	&EXLstInstlldC,
	&EXNwMdfrmp,
	&EXQryKymp,
	&EXRmvFrmSvSt,
	&EXRmvHst,
	&EXRmvHsts,
	&EXRprntWdw,
	&EXRstScrnSvr,
	&EXStAccssCntr,
	&EXStClsDwnMd,
	&EXStInptFcs,
	&EXStMdfrMppng,
	&EXStPntrMppng,
	&EXStScrnSvr,
	&EXUngrbBttn,
	&EXUngrbKy,
	&EXUngrbKybrd,
	&EXUngrbPntr,
	&EXUngrbSrvr,
	&EXUnnstllClrm,
	&EXWrpPntr,
	0,
};
