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
 * $XConsortium: linktbl.c,v 1.2 94/04/17 21:13:19 rws Exp $
 */

#include "xtest.h"

extern struct linkinfo EAllwDvcEvnts;
extern struct linkinfo EChngDvcCntrl;
extern struct linkinfo EChngDvcDntPr;
extern struct linkinfo EChngFdbckCnt;
extern struct linkinfo EChngKybrdDvc;
extern struct linkinfo EChngPntrDvc;
extern struct linkinfo EChngDvcKyMpp;
extern struct linkinfo EClsDvc;
extern struct linkinfo EDvcBll;
extern struct linkinfo EGtDvcMdfrMpp;
extern struct linkinfo EGtFdbckCntrl;
extern struct linkinfo EGtExtnsnVrsn;
extern struct linkinfo EGrbDvc;
extern struct linkinfo EGrbDvcBttn;
extern struct linkinfo EGrbDvcKy;
extern struct linkinfo EGtDvcBttnMpp;
extern struct linkinfo EGtDvcCntrl;
extern struct linkinfo EGtDvcFcs;
extern struct linkinfo EGtDvcMtnEvnt;
extern struct linkinfo EGtDvcDntPrpg;
extern struct linkinfo EGtDvcKyMppng;
extern struct linkinfo EGtSlctdExtns;
extern struct linkinfo ELstInptDvcs;
extern struct linkinfo EMscllns;
extern struct linkinfo ESlctExtnsnEv;
extern struct linkinfo EOpnDvc;
extern struct linkinfo EQryDvcStt;
extern struct linkinfo EStDvcVltrs;
extern struct linkinfo EStDvcBttnMpp;
extern struct linkinfo EStDvcFcs;
extern struct linkinfo EStDvcMdfrMpp;
extern struct linkinfo EStDvcMd;
extern struct linkinfo ESndExtnsnEvn;
extern struct linkinfo EUngrbDvc;
extern struct linkinfo EUngrbDvcBttn;
extern struct linkinfo EUngrbDvcKy;

struct linkinfo *linktbl[] = {
        &EAllwDvcEvnts,
        &EChngDvcCntrl,
        &EChngDvcDntPr,
        &EChngFdbckCnt,
        &EChngKybrdDvc,
        &EChngPntrDvc,
        &EChngDvcKyMpp,
        &EClsDvc,
        &EDvcBll,
        &EGtDvcMdfrMpp,
        &EGtFdbckCntrl,
        &EGtExtnsnVrsn,
        &EGrbDvc,
        &EGrbDvcBttn,
        &EGrbDvcKy,
        &EGtDvcBttnMpp,
        &EGtDvcCntrl,
        &EGtDvcFcs,
        &EGtDvcMtnEvnt,
        &EGtDvcDntPrpg,
        &EGtDvcKyMppng,
        &EGtSlctdExtns,
        &ELstInptDvcs,
        &EMscllns,
        &ESlctExtnsnEv,
	&EOpnDvc,
	&EQryDvcStt,
	&EStDvcVltrs,
	&EStDvcBttnMpp,
	&EStDvcFcs,
	&EStDvcMdfrMpp,
	&EStDvcMd,
	&ESndExtnsnEvn,
	&EUngrbDvc,
	&EUngrbDvcBttn,
	&EUngrbDvcKy,
	0,
};
