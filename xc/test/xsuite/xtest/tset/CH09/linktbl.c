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
 * $XConsortium: linktbl.c,v 1.3 94/04/17 21:08:25 rws Exp $
 */

#include "xtest.h"

extern struct linkinfo EXAllcClssHnt;
extern struct linkinfo EXAllcIcnSz;
extern struct linkinfo EXAllcSzHnts;
extern struct linkinfo EXAllcStndrdC;
extern struct linkinfo EXAllcWMHnts;
extern struct linkinfo EXFtchNm;
extern struct linkinfo EXFrStrLst;
extern struct linkinfo EXGtClssHnt;
extern struct linkinfo EXGtCmmnd;
extern struct linkinfo EXGtIcnNm;
extern struct linkinfo EXGtIcnSzs;
extern struct linkinfo EXGtRGBClrmps;
extern struct linkinfo EXGtTxtPrprty;
extern struct linkinfo EXGtTrnsntFrH;
extern struct linkinfo EXGtWMClntMch;
extern struct linkinfo EXGtWMClrmpWd;
extern struct linkinfo EXGtWMHnts;
extern struct linkinfo EXGtWMIcnNm;
extern struct linkinfo EXGtWMNm;
extern struct linkinfo EXGtWMNrmlHnt;
extern struct linkinfo EXGtWMPrtcls;
extern struct linkinfo EXGtWMSzHnts;
extern struct linkinfo EXIcnfyWdw;
extern struct linkinfo EXRcnfgrWMWdw;
extern struct linkinfo EXStClssHnt;
extern struct linkinfo EXStCmmnd;
extern struct linkinfo EXStIcnNm;
extern struct linkinfo EXStIcnSzs;
extern struct linkinfo EXStRGBClrmps;
extern struct linkinfo EXStTxtPrprty;
extern struct linkinfo EXStTrnsntFrH;
extern struct linkinfo EXStWMClntMch;
extern struct linkinfo EXStWMClrmpWd;
extern struct linkinfo EXStWMHnts;
extern struct linkinfo EXStWMIcnNm;
extern struct linkinfo EXStWMNm;
extern struct linkinfo EXStWMNrmlHnt;
extern struct linkinfo EXStWMPrprts;
extern struct linkinfo EXStWMPrtcls;
extern struct linkinfo EXStWMSzHnts;
extern struct linkinfo EXStrNm;
extern struct linkinfo EXStrLstTTxtP;
extern struct linkinfo EXTxtPrprtyTS;
extern struct linkinfo EXWMGmtry;
extern struct linkinfo EXWthdrwWdw;

struct linkinfo *linktbl[] = {
	&EXAllcClssHnt,
	&EXAllcIcnSz,
	&EXAllcSzHnts,
	&EXAllcStndrdC,
	&EXAllcWMHnts,
	&EXFtchNm,
	&EXFrStrLst,
	&EXGtClssHnt,
	&EXGtCmmnd,
	&EXGtIcnNm,
	&EXGtIcnSzs,
	&EXGtRGBClrmps,
	&EXGtTxtPrprty,
	&EXGtTrnsntFrH,
	&EXGtWMClntMch,
	&EXGtWMClrmpWd,
	&EXGtWMHnts,
	&EXGtWMIcnNm,
	&EXGtWMNm,
	&EXGtWMNrmlHnt,
	&EXGtWMPrtcls,
	&EXGtWMSzHnts,
	&EXIcnfyWdw,
	&EXRcnfgrWMWdw,
	&EXStClssHnt,
	&EXStCmmnd,
	&EXStIcnNm,
	&EXStIcnSzs,
	&EXStRGBClrmps,
	&EXStTxtPrprty,
	&EXStTrnsntFrH,
	&EXStWMClntMch,
	&EXStWMClrmpWd,
	&EXStWMHnts,
	&EXStWMIcnNm,
	&EXStWMNm,
	&EXStWMNrmlHnt,
	&EXStWMPrprts,
	&EXStWMPrtcls,
	&EXStWMSzHnts,
	&EXStrNm,
	&EXStrLstTTxtP,
	&EXTxtPrprtyTS,
	&EXWMGmtry,
	&EXWthdrwWdw,
	0,
};
