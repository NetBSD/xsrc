/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwareply1rct[] =

    "Id: vmwareply1rct.c,v 1.2 2001/01/26 23:32:16 yoel Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwareply1rct.c,v 1.1 2001/04/05 19:29:44 dawes Exp $ */

#include "vmware.h"

static void
computeBBpoly(DrawablePtr pDrawable,
    GCPtr pGC, int mode, int count, DDXPointPtr pptInit, BoxPtr pBB)
{
    int x, y;

    if (count <= 0)
	return;
    x = pptInit->x;
    y = pptInit->y;
    pBB->x1 = x;
    pBB->y1 = y;
    pBB->x2 = x + 1;
    pBB->y2 = y + 1;
    while (--count) {
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
vmwareFillPolygon(DrawablePtr pDrawable,
    GCPtr pGC, int shape, int mode, int count, DDXPointPtr pPts)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
		    pGC->pScreen,
		    computeBBpoly(pDrawable, pGC, mode, count, pPts, &BB),
		    GC_OPS(pGC)->FillPolygon(pDrawable, pGC, shape, mode,
			count, pPts));
}





