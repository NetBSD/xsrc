/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwarefillarc[] =

    "Id: vmwarefillarc.c,v 1.2 2001/01/26 23:32:16 yoel Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwarefillarc.c,v 1.1 2001/04/05 19:29:44 dawes Exp $ */

#include "vmware.h"

static void
computeBBarc(DrawablePtr pDrawable,
    GCPtr pGC, int narcs, xArc * parcs, BoxPtr pBB)
{
    if (narcs <= 0)
	return;
    pBB->x1 = parcs->x;
    pBB->y1 = parcs->y;
    pBB->x2 = parcs->x + parcs->width;
    pBB->y2 = parcs->y + parcs->height;
    while (--narcs) {
	parcs++;
	if (parcs->x < pBB->x1)
	    pBB->x1 = parcs->x;
	if (parcs->y < pBB->y1)
	    pBB->y1 = parcs->y;
	if ((parcs->x + parcs->width) > pBB->x2)
	    pBB->x2 = parcs->x + parcs->width;
	if ((parcs->y + parcs->height) > pBB->y2)
	    pBB->y2 = parcs->y + parcs->height;
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
vmwarePolyFillArc(DrawablePtr pDrawable, GCPtr pGC, int narcs, xArc * parcs)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
                    pGC->pScreen,
		    computeBBarc(pDrawable, pGC, narcs, parcs, &BB),
		    GC_OPS(pGC)->PolyFillArc(pDrawable, pGC, narcs, parcs));
}
