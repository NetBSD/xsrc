/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwarepolypnt[] =

    "Id: vmwarepolypnt.c,v 1.2 2001/01/26 23:32:16 yoel Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwarepolypnt.c,v 1.1 2001/04/05 19:29:44 dawes Exp $ */

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
vmwarePolyPoint(DrawablePtr pDrawable, GCPtr pGC, int mode, int npt,
    xPoint * pptInit)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
		    pGC->pScreen,
		    computeBBpoints(pDrawable, pGC, mode, npt, pptInit, &BB),
		    GC_OPS(pGC)->PolyPoint(pDrawable, pGC, mode, npt, pptInit));
}
