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
 * $XConsortium: linktbl.c,v 1.3 94/04/17 21:10:44 rws Exp $
 */

#include "xtest.h"

extern struct linkinfo EAllcClr;
extern struct linkinfo EAllcClrClls;
extern struct linkinfo EAllcClrPlns;
extern struct linkinfo EAllcNmdClr;
extern struct linkinfo EAllwEvnts;
extern struct linkinfo EBdRqst;
extern struct linkinfo EBll;
extern struct linkinfo EChngActvPntr;
extern struct linkinfo EChngGC;
extern struct linkinfo EChngHsts;
extern struct linkinfo EChngKybrdCnt;
extern struct linkinfo EChngKybrdMpp;
extern struct linkinfo EChngPntrCntr;
extern struct linkinfo EChngPrprty;
extern struct linkinfo EChngSvSt;
extern struct linkinfo EChngWdwAttrb;
extern struct linkinfo ECrcltWdw;
extern struct linkinfo EClrAr;
extern struct linkinfo EClsFnt;
extern struct linkinfo ECnfgrWdw;
extern struct linkinfo ECnvrtSlctn;
extern struct linkinfo ECpyAr;
extern struct linkinfo ECpyClrmpAndF;
extern struct linkinfo ECpyGC;
extern struct linkinfo ECpyPln;
extern struct linkinfo ECrtClrmp;
extern struct linkinfo ECrtCrsr;
extern struct linkinfo ECrtGC;
extern struct linkinfo ECrtGlyphCrsr;
extern struct linkinfo ECrtPxmp;
extern struct linkinfo ECrtWdw;
extern struct linkinfo EDltPrprty;
extern struct linkinfo EDstrySbws;
extern struct linkinfo EDstryWdw;
extern struct linkinfo EFllPly;
extern struct linkinfo EFrcScrnSvr;
extern struct linkinfo EFrClrmp;
extern struct linkinfo EFrClrs;
extern struct linkinfo EFrCrsr;
extern struct linkinfo EFrGC;
extern struct linkinfo EFrPxmp;
extern struct linkinfo EGtAtmNm;
extern struct linkinfo EGtFntPth;
extern struct linkinfo EGtGmtry;
extern struct linkinfo EGtImg;
extern struct linkinfo EGtInptFcs;
extern struct linkinfo EGtKybrdCntrl;
extern struct linkinfo EGtKybrdMppng;
extern struct linkinfo EGtMdfrMppng;
extern struct linkinfo EGtMtnEvnts;
extern struct linkinfo EGtPntrCntrl;
extern struct linkinfo EGtPntrMppng;
extern struct linkinfo EGtPrprty;
extern struct linkinfo EGtScrnSvr;
extern struct linkinfo EGtSlctnOwnr;
extern struct linkinfo EGtWdwAttrbts;
extern struct linkinfo EGrbBttn;
extern struct linkinfo EGrbKy;
extern struct linkinfo EGrbKybrd;
extern struct linkinfo EGrbPntr;
extern struct linkinfo EGrbSrvr;
extern struct linkinfo EImgTxt16;
extern struct linkinfo EImgTxt8;
extern struct linkinfo EInstllClrmp;
extern struct linkinfo EIntrnAtm;
extern struct linkinfo EKllClnt;
extern struct linkinfo ELstExtnsns;
extern struct linkinfo ELstFnts;
extern struct linkinfo ELstFntsWthIn;
extern struct linkinfo ELstHsts;
extern struct linkinfo ELstInstlldCl;
extern struct linkinfo ELstPrprts;
extern struct linkinfo ELkpClr;
extern struct linkinfo EMpSbws;
extern struct linkinfo EMpWdw;
extern struct linkinfo ENOprtn;
extern struct linkinfo EOpnDsply;
extern struct linkinfo EOpnFnt;
extern struct linkinfo EPlyArc;
extern struct linkinfo EPlyFllArc;
extern struct linkinfo EPlyFllRctngl;
extern struct linkinfo EPlyLn;
extern struct linkinfo EPlyPnt;
extern struct linkinfo EPlyRctngl;
extern struct linkinfo EPlySgmnt;
extern struct linkinfo EPlyTxt16;
extern struct linkinfo EPlyTxt8;
extern struct linkinfo EPtImg;
extern struct linkinfo EQryBstSz;
extern struct linkinfo EQryClrs;
extern struct linkinfo EQryExtnsn;
extern struct linkinfo EQryFnt;
extern struct linkinfo EQryKymp;
extern struct linkinfo EQryPntr;
extern struct linkinfo EQryTxtExtnts;
extern struct linkinfo EQryTr;
extern struct linkinfo ERclrCrsr;
extern struct linkinfo ERprntWdw;
extern struct linkinfo ERttPrprts;
extern struct linkinfo ESndEvnt;
extern struct linkinfo EStAccssCntrl;
extern struct linkinfo EStClpRctngls;
extern struct linkinfo EStClsDwnMd;
extern struct linkinfo EStDshs;
extern struct linkinfo EStFntPth;
extern struct linkinfo EStInptFcs;
extern struct linkinfo EStMdfrMppng;
extern struct linkinfo EStPntrMppng;
extern struct linkinfo EStScrnSvr;
extern struct linkinfo EStSlctnOwnr;
extern struct linkinfo EStrClrs;
extern struct linkinfo EStrNmdClr;
extern struct linkinfo ETrnsltCrds;
extern struct linkinfo EUngrbBttn;
extern struct linkinfo EUngrbKy;
extern struct linkinfo EUngrbKybrd;
extern struct linkinfo EUngrbPntr;
extern struct linkinfo EUngrbSrvr;
extern struct linkinfo EUnnstllClrmp;
extern struct linkinfo EUnmpSbws;
extern struct linkinfo EUnmpWdw;
extern struct linkinfo EWrpPntr;

struct linkinfo *linktbl[] = {
	&EAllcClr,
	&EAllcClrClls,
	&EAllcClrPlns,
	&EAllcNmdClr,
	&EAllwEvnts,
	&EBdRqst,
	&EBll,
	&EChngActvPntr,
	&EChngGC,
	&EChngHsts,
	&EChngKybrdCnt,
	&EChngKybrdMpp,
	&EChngPntrCntr,
	&EChngPrprty,
	&EChngSvSt,
	&EChngWdwAttrb,
	&ECrcltWdw,
	&EClrAr,
	&EClsFnt,
	&ECnfgrWdw,
	&ECnvrtSlctn,
	&ECpyAr,
	&ECpyClrmpAndF,
	&ECpyGC,
	&ECpyPln,
	&ECrtClrmp,
	&ECrtCrsr,
	&ECrtGC,
	&ECrtGlyphCrsr,
	&ECrtPxmp,
	&ECrtWdw,
	&EDltPrprty,
	&EDstrySbws,
	&EDstryWdw,
	&EFllPly,
	&EFrcScrnSvr,
	&EFrClrmp,
	&EFrClrs,
	&EFrCrsr,
	&EFrGC,
	&EFrPxmp,
	&EGtAtmNm,
	&EGtFntPth,
	&EGtGmtry,
	&EGtImg,
	&EGtInptFcs,
	&EGtKybrdCntrl,
	&EGtKybrdMppng,
	&EGtMdfrMppng,
	&EGtMtnEvnts,
	&EGtPntrCntrl,
	&EGtPntrMppng,
	&EGtPrprty,
	&EGtScrnSvr,
	&EGtSlctnOwnr,
	&EGtWdwAttrbts,
	&EGrbBttn,
	&EGrbKy,
	&EGrbKybrd,
	&EGrbPntr,
	&EGrbSrvr,
	&EImgTxt16,
	&EImgTxt8,
	&EInstllClrmp,
	&EIntrnAtm,
	&EKllClnt,
	&ELstExtnsns,
	&ELstFnts,
	&ELstFntsWthIn,
	&ELstHsts,
	&ELstInstlldCl,
	&ELstPrprts,
	&ELkpClr,
	&EMpSbws,
	&EMpWdw,
	&ENOprtn,
	&EOpnDsply,
	&EOpnFnt,
	&EPlyArc,
	&EPlyFllArc,
	&EPlyFllRctngl,
	&EPlyLn,
	&EPlyPnt,
	&EPlyRctngl,
	&EPlySgmnt,
	&EPlyTxt16,
	&EPlyTxt8,
	&EPtImg,
	&EQryBstSz,
	&EQryClrs,
	&EQryExtnsn,
	&EQryFnt,
	&EQryKymp,
	&EQryPntr,
	&EQryTxtExtnts,
	&EQryTr,
	&ERclrCrsr,
	&ERprntWdw,
	&ERttPrprts,
	&ESndEvnt,
	&EStAccssCntrl,
	&EStClpRctngls,
	&EStClsDwnMd,
	&EStDshs,
	&EStFntPth,
	&EStInptFcs,
	&EStMdfrMppng,
	&EStPntrMppng,
	&EStScrnSvr,
	&EStSlctnOwnr,
	&EStrClrs,
	&EStrNmdClr,
	&ETrnsltCrds,
	&EUngrbBttn,
	&EUngrbKy,
	&EUngrbKybrd,
	&EUngrbPntr,
	&EUngrbSrvr,
	&EUnnstllClrmp,
	&EUnmpSbws,
	&EUnmpWdw,
	&EWrpPntr,
	0,
};
