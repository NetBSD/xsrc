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
 * $XConsortium: linktbl.c,v 1.2 94/04/17 21:12:40 rws Exp $
 */

#include "xtest.h"

extern struct linkinfo EXAllwDvcEvnt;
extern struct linkinfo EXChngDvcCntr;
extern struct linkinfo EXChngFdbckCn;
extern struct linkinfo EXChngKybrdDv;
extern struct linkinfo EXChngDvcDntP;
extern struct linkinfo EXChngPntrDvc;
extern struct linkinfo EXChngDvcKyMp;
extern struct linkinfo EXClsDvc;
extern struct linkinfo EXDvcBll;
extern struct linkinfo EXGtDvcCntrl;
extern struct linkinfo EXGtFdbckCntr;
extern struct linkinfo EXGtDvcFcs;
extern struct linkinfo EXGtDvcDntPrp;
extern struct linkinfo EXGtExtnsnVrs;
extern struct linkinfo EXGrbDvc;
extern struct linkinfo EXGrbDvcBttn;
extern struct linkinfo EXGrbDvcKy;
extern struct linkinfo EXGtDvcBttnMp;
extern struct linkinfo EXGtDvcKyMppn;
extern struct linkinfo EXGtDvcMdfrMp;
extern struct linkinfo EXGtDvcMtnEvn;
extern struct linkinfo EXGtSlctdExtn;
extern struct linkinfo EXLstInptDvcs;
extern struct linkinfo EMscllns;
extern struct linkinfo EXOpnDvc;
extern struct linkinfo EXQryDvcStt;
extern struct linkinfo EXStDvcVltrs;
extern struct linkinfo EXStDvcBttnMp;
extern struct linkinfo EXStDvcFcs;
extern struct linkinfo EXStDvcMdfrMp;
extern struct linkinfo EXStDvcMd;
extern struct linkinfo EXSlctExtnsnE;
extern struct linkinfo EXSndExtnsnEv;
extern struct linkinfo EXUngrbDvc;
extern struct linkinfo EXUngrbDvcBtt;
extern struct linkinfo EXUngrbDvcKy;

struct linkinfo *linktbl[] = {
        &EXAllwDvcEvnt,
        &EXChngDvcCntr,
        &EXChngFdbckCn,
        &EXChngKybrdDv,
        &EXChngDvcDntP,
        &EXChngPntrDvc,
        &EXChngDvcKyMp,
        &EXClsDvc,
        &EXDvcBll,
        &EXGtDvcCntrl,
        &EXGtFdbckCntr,
        &EXGtDvcFcs,
        &EXGtDvcDntPrp,
        &EXGtExtnsnVrs,
        &EXGrbDvc,
        &EXGrbDvcBttn,
        &EXGrbDvcKy,
        &EXGtDvcBttnMp,
        &EXGtDvcKyMppn,
        &EXGtDvcMdfrMp,
        &EXGtDvcMtnEvn,
        &EXGtSlctdExtn,
        &EXLstInptDvcs,
        &EMscllns,
        &EXOpnDvc,
        &EXQryDvcStt,
        &EXStDvcVltrs,
        &EXStDvcBttnMp,
        &EXStDvcFcs,
        &EXStDvcMdfrMp,
        &EXStDvcMd,
        &EXSlctExtnsnE,
        &EXSndExtnsnEv,
        &EXUngrbDvc,
        &EXUngrbDvcBtt,
        &EXUngrbDvcKy,

	0,
};
