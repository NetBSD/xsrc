/* $NetBSD: sfbbitblt.c,v 1.1 2004/01/18 05:21:41 rtr Exp $ */

/*
 * cfb copy area
 */

/*

Copyright (c) 1989  X Consortium

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

Author: Keith Packard

*/
/* $XConsortium: cfbbitblt.c,v 5.51 94/05/27 11:00:56 dpw Exp $ */

#include <stdio.h>

#ifndef PSZ
#define PSZ 8
#endif

#include	"X.h"
#include	"Xmd.h"
#include	"Xproto.h"
#include	"gcstruct.h"
#include	"windowstr.h"
#include	"scrnintstr.h"
#include	"pixmapstr.h"
#include	"regionstr.h"
#include	"cfb.h"
#include	"cfbmskbits.h"
#include	"cfb8bit.h"
#include	"fastblt.h"
#define MFB_CONSTS_ONLY
#include	"maskbits.h"

#include	"sfbmap.h"
#include	"sfb.h"

void
decSfbDoBitbltCopy(
    DrawablePtr	    /* pSrc */,
    DrawablePtr     /* pDst */,
    int		    /* alu */,
    RegionPtr	    /* prgnDst */,
    DDXPointPtr	    /* pptSrc */,
    unsigned long   /* planemask */
);

void
decSfbDoBitblt (pSrc, pDst, alu, prgnDst, pptSrc, planemask)
    DrawablePtr	    pSrc, pDst;
    int		    alu;
    RegionPtr	    prgnDst;
    DDXPointPtr	    pptSrc;
    unsigned long   planemask;
{
    void (*blt)() = cfbDoBitbltGeneral;
    if ((planemask & PMSK) == PMSK) {
	blt = decSfbDoBitbltCopy;
    }
    (*blt) (pSrc, pDst, alu, prgnDst, pptSrc, planemask);
}

RegionPtr
decSfbCopyArea(pSrcDrawable, pDstDrawable,
            pGC, srcx, srcy, width, height, dstx, dsty)
    register DrawablePtr pSrcDrawable;
    register DrawablePtr pDstDrawable;
    GCPtr pGC;
    int srcx, srcy;
    int width, height;
    int dstx, dsty;
{
    void (*blt)() = cfbDoBitblt;
    if (pSrcDrawable->type == DRAWABLE_WINDOW &&
	pDstDrawable->type == DRAWABLE_WINDOW)
	blt = decSfbDoBitblt;
    return cfbBitBlt (pSrcDrawable, pDstDrawable,
        pGC, srcx, srcy, width, height, dstx, dsty, blt, 0L);
}
