/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwarepush[] =

    "Id: vmwarepush.c,v 1.2 2001/01/26 23:32:16 yoel Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwarepush.c,v 1.1 2001/04/05 19:29:44 dawes Exp $ */

#include "vmware.h"

static __inline void
vmwarePushToBox(BoxPtr BB, const GCPtr pGC, PixmapPtr pBitmap,
    DrawablePtr pDrawable, int dx, int dy, int xOrg, int yOrg)
{
    BB->x1 =
	MAX(pDrawable->x + xOrg, (REGION_EXTENTS(pGC->pScreen,
	    &((WindowPtr) pDrawable)->winSize))->x1);
    BB->y1 =
	MAX(pDrawable->y + yOrg, (REGION_EXTENTS(pGC->pScreen,
	    &((WindowPtr) pDrawable)->winSize))->y1);
    BB->x2 =
	MIN(pDrawable->x + xOrg + dx, (REGION_EXTENTS(pGC->pScreen,
	    &((WindowPtr) pDrawable)->winSize))->x2);
    BB->y2 =
	MIN(pDrawable->y + yOrg + dy, (REGION_EXTENTS(pGC->pScreen,
	    &((WindowPtr) pDrawable)->winSize))->y2);
}

void
vmwarePushPixels(GCPtr pGC,
    PixmapPtr pBitmap,
    DrawablePtr pDrawable, int dx, int dy, int xOrg, int yOrg)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
    		    pGC->pScreen,
		    vmwarePushToBox(&BB, pGC, pBitmap, pDrawable, dx, dy, xOrg, yOrg),
		    GC_OPS(pGC)->PushPixels(pGC, pBitmap, pDrawable, dx, dy, xOrg, yOrg));
}
