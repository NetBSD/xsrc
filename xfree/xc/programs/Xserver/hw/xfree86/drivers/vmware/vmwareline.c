/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwareline[] =

    "Id: vmwareline.c,v 1.2 2001/01/26 23:32:16 yoel Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwareline.c,v 1.1 2001/04/05 19:29:44 dawes Exp $ */

#include "vmware.h"

static void
computeBBpoints(DrawablePtr pDrawable,
    GCPtr pGC, int mode, int npt, DDXPointPtr pptInit, BoxPtr pBB)
{
    int x, y;

    if (npt <= 0)
	return;
    x = pptInit->x;
    y = pptInit->y;
    pBB->x1 = x;
    pBB->y1 = y;
    pBB->x2 = x + 1;
    pBB->y2 = y + 1;
    while (--npt) {
	pptInit++;
	if (mode == CoordModeOrigin) {
	    x = pptInit->x;
	    y = pptInit->y;
	} else {
	    x += pptInit->x;
	    y += pptInit->y;
	}
	if (x < pBB->x1)
	    pBB->x1 = x;
	if (y < pBB->y1)
	    pBB->y1 = y;
	if (x + 1 > pBB->x2)
	    pBB->x2 = x + 1;
	if (y + 1 > pBB->y2)
	    pBB->y2 = y + 1;
    }
    pBB->x1 =
	MAX(pDrawable->x + pBB->x1 - pGC->lineWidth,
	(REGION_EXTENTS(pGC->pScreen,
&((WindowPtr) pDrawable)->winSize))->x1);
    pBB->y1 =
	MAX(pDrawable->y + pBB->y1 - pGC->lineWidth,
	(REGION_EXTENTS(pGC->pScreen,
&((WindowPtr) pDrawable)->winSize))->y1);
    pBB->x2 =
	MIN(pDrawable->x + pBB->x2 + pGC->lineWidth,
	(REGION_EXTENTS(pGC->pScreen,
&((WindowPtr) pDrawable)->winSize))->x2);
    pBB->y2 =
	MIN(pDrawable->y + pBB->y2 + pGC->lineWidth,
	(REGION_EXTENTS(pGC->pScreen,
&((WindowPtr) pDrawable)->winSize))->y2);
}

void
vmwarePolylines(DrawablePtr pDrawable,
    GCPtr pGC, int mode, int npt, DDXPointPtr pPts)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
		    pGC->pScreen,
		    computeBBpoints(pDrawable, pGC, mode, npt, pPts, &BB),
		    GC_OPS(pGC)->Polylines(pDrawable, pGC, mode, npt, pPts));
}

static void
computeBBsegments(DrawablePtr pDrawable,
    GCPtr pGC, int nseg, xSegment * pSegs, BoxPtr pBB)
{
    if (nseg <= 0)
	return;
    pBB->x1 = pSegs->x1;
    pBB->y1 = pSegs->y1;
    pBB->x2 = pSegs->x1 + 1;
    pBB->y2 = pSegs->y1 + 1;
    if (pSegs->x2 < pBB->x1)
	pBB->x1 = pSegs->x2;
    if (pSegs->y2 < pBB->y1)
	pBB->y1 = pSegs->y2;
    if (pSegs->x2 + 1 > pBB->x2)
	pBB->x2 = pSegs->x2 + 1;
    if (pSegs->y2 + 1 > pBB->y2)
	pBB->y2 = pSegs->y2 + 1;
    while (--nseg) {
	pSegs++;
	if (pSegs->x1 < pBB->x1)
	    pBB->x1 = pSegs->x1;
	if (pSegs->y1 < pBB->y1)
	    pBB->y1 = pSegs->y1;
	if (pSegs->x1 + 1 > pBB->x2)
	    pBB->x2 = pSegs->x1 + 1;
	if (pSegs->y1 + 1 > pBB->y2)
	    pBB->y2 = pSegs->y1 + 1;
	if (pSegs->x2 < pBB->x1)
	    pBB->x1 = pSegs->x2;
	if (pSegs->y2 < pBB->y1)
	    pBB->y1 = pSegs->y2;
	if (pSegs->x2 + 1 > pBB->x2)
	    pBB->x2 = pSegs->x2 + 1;
	if (pSegs->y2 + 1 > pBB->y2)
	    pBB->y2 = pSegs->y2 + 1;
    }
    pBB->x1 =
	MAX(pDrawable->x + pBB->x1 - pGC->lineWidth,
	(REGION_EXTENTS(pGC->pScreen,
&((WindowPtr) pDrawable)->winSize))->x1);
    pBB->y1 =
	MAX(pDrawable->y + pBB->y1 - pGC->lineWidth,
	(REGION_EXTENTS(pGC->pScreen,
&((WindowPtr) pDrawable)->winSize))->y1);
    pBB->x2 =
	MIN(pDrawable->x + pBB->x2 + pGC->lineWidth,
	(REGION_EXTENTS(pGC->pScreen,
&((WindowPtr) pDrawable)->winSize))->x2);
    pBB->y2 =
	MIN(pDrawable->y + pBB->y2 + pGC->lineWidth,
	(REGION_EXTENTS(pGC->pScreen,
&((WindowPtr) pDrawable)->winSize))->y2);
}

void
vmwarePolySegment(DrawablePtr pDrawable,
    GCPtr pGC, int nseg, xSegment * pSegs)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
		    pGC->pScreen,
		    computeBBsegments(pDrawable, pGC, nseg, pSegs, &BB),
		    GC_OPS(pGC)->PolySegment(pDrawable, pGC, nseg, pSegs));
}
