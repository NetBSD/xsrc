/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwarerect[] =

    "Id: vmwarerect.c,v 1.2 2001/01/26 23:32:16 yoel Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwarerect.c,v 1.2 2001/05/16 06:48:12 keithp Exp $ */

#include "X.h"
#include "fb.h"
#include "vmware.h"

static void
computeBBrect(DrawablePtr pDrawable,
    GCPtr pGC, int nrectFill, xRectangle * prectInit, BoxPtr pBB)
{
    if (nrectFill <= 0)
	return;
    pBB->x1 = prectInit->x;
    pBB->y1 = prectInit->y;
    pBB->x2 = prectInit->x + prectInit->width + 1;
    pBB->y2 = prectInit->y + prectInit->height + 1;
    while (--nrectFill) {
	prectInit++;
	if (prectInit->x < pBB->x1)
	    pBB->x1 = prectInit->x;
	if (prectInit->y < pBB->y1)
	    pBB->y1 = prectInit->y;
	if ((prectInit->x + prectInit->width + 1) > pBB->x2)
	    pBB->x2 = prectInit->x + prectInit->width + 1;
	if ((prectInit->y + prectInit->height + 1) > pBB->y2)
	    pBB->y2 = prectInit->y + prectInit->height + 1;
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
vmwarePolyRectangle(DrawablePtr pDrawable,
    GCPtr pGC, int nrectFill, xRectangle * prectInit)
{
    TRACEPOINT
    
    GC_FUNC_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
    		    pGC->pScreen,
		    computeBBrect(pDrawable, pGC, nrectFill, prectInit, &BB),
		    GC_OPS(pGC)->PolyRectangle(pDrawable, pGC, nrectFill, prectInit));
}

static void
accelFillRectSolid(VMWAREPtr pVMWARE, DrawablePtr pDrawable, GCPtr pGC, int nBox, BoxPtr pBox)
{
    while (nBox) {
	vmwareWriteWordToFIFO(pVMWARE, SVGA_CMD_RECT_ROP_FILL);
	vmwareWriteWordToFIFO(pVMWARE, pGC->fgPixel);
	vmwareWriteWordToFIFO(pVMWARE, pBox->x1);
	vmwareWriteWordToFIFO(pVMWARE, pBox->y1);
	vmwareWriteWordToFIFO(pVMWARE, pBox->x2 - pBox->x1);
	vmwareWriteWordToFIFO(pVMWARE, pBox->y2 - pBox->y1);
	vmwareWriteWordToFIFO(pVMWARE, pGC->alu);
	pBox++;
	nBox--;
    }
}

#define NUM_STACK_RECTS	1024

static void
accelPolyFillRect(VMWAREPtr pVMWARE, DrawablePtr pDrawable,
    GCPtr pGC, int nrectFill, xRectangle * prectInit)
{
    xRectangle *prect;
    RegionPtr prgnClip;
    BoxPtr pbox;
    BoxPtr pboxClipped;
    BoxPtr pboxClippedBase;
    BoxPtr pextent;
    BoxRec stackRects[NUM_STACK_RECTS];
    int numRects;
    int n;
    int xorg, yorg;

    prgnClip = fbGetCompositeClip(pGC);

    prect = prectInit;
    xorg = pDrawable->x;
    yorg = pDrawable->y;
    if (xorg || yorg) {
	prect = prectInit;
	n = nrectFill;
	while (n--) {
	    prect->x += xorg;
	    prect->y += yorg;
	    prect++;
	}
    }

    prect = prectInit;

    numRects = REGION_NUM_RECTS(prgnClip) * nrectFill;
    if (numRects > NUM_STACK_RECTS) {
	pboxClippedBase = (BoxPtr) ALLOCATE_LOCAL(numRects * sizeof(BoxRec));
	if (!pboxClippedBase)
	    return;
    } else
	pboxClippedBase = stackRects;

    pboxClipped = pboxClippedBase;

    if (REGION_NUM_RECTS(prgnClip) == 1) {
	int x1, y1, x2, y2, bx2, by2;

	pextent = REGION_RECTS(prgnClip);
	x1 = pextent->x1;
	y1 = pextent->y1;
	x2 = pextent->x2;
	y2 = pextent->y2;
	while (nrectFill--) {
	    if ((pboxClipped->x1 = prect->x) < x1)
		pboxClipped->x1 = x1;

	    if ((pboxClipped->y1 = prect->y) < y1)
		pboxClipped->y1 = y1;

	    bx2 = (int)prect->x + (int)prect->width;
	    if (bx2 > x2)
		bx2 = x2;
	    pboxClipped->x2 = bx2;

	    by2 = (int)prect->y + (int)prect->height;
	    if (by2 > y2)
		by2 = y2;
	    pboxClipped->y2 = by2;

	    prect++;
	    if ((pboxClipped->x1 < pboxClipped->x2) &&
		(pboxClipped->y1 < pboxClipped->y2)) {
		pboxClipped++;
	    }
	}
    } else {
	int x1, y1, x2, y2, bx2, by2;

	pextent = REGION_EXTENTS(pGC->pScreen, prgnClip);
	x1 = pextent->x1;
	y1 = pextent->y1;
	x2 = pextent->x2;
	y2 = pextent->y2;
	while (nrectFill--) {
	    BoxRec box;

	    if ((box.x1 = prect->x) < x1)
		box.x1 = x1;

	    if ((box.y1 = prect->y) < y1)
		box.y1 = y1;

	    bx2 = (int)prect->x + (int)prect->width;
	    if (bx2 > x2)
		bx2 = x2;
	    box.x2 = bx2;

	    by2 = (int)prect->y + (int)prect->height;
	    if (by2 > y2)
		by2 = y2;
	    box.y2 = by2;

	    prect++;

	    if ((box.x1 >= box.x2) || (box.y1 >= box.y2))
		continue;

	    n = REGION_NUM_RECTS(prgnClip);
	    pbox = REGION_RECTS(prgnClip);

	    /* clip the rectangle to each box in the clip region
	     * this is logically equivalent to calling Intersect()
	     */
	    while (n--) {
		pboxClipped->x1 = max(box.x1, pbox->x1);
		pboxClipped->y1 = max(box.y1, pbox->y1);
		pboxClipped->x2 = min(box.x2, pbox->x2);
		pboxClipped->y2 = min(box.y2, pbox->y2);
		pbox++;

		/* see if clipping left anything */
		if (pboxClipped->x1 < pboxClipped->x2 &&
		    pboxClipped->y1 < pboxClipped->y2) {
		    pboxClipped++;
		}
	    }
	}
    }
    if (pboxClipped != pboxClippedBase)
	accelFillRectSolid(pVMWARE, pDrawable, pGC,
	    pboxClipped - pboxClippedBase, pboxClippedBase);
    if (pboxClippedBase != stackRects)
	DEALLOCATE_LOCAL(pboxClippedBase);
}

static void
computeBBfillrect(DrawablePtr pDrawable,
    GCPtr pGC, int nrectFill, xRectangle * prectInit, BoxPtr pBB)
{
    if (nrectFill <= 0)
	return;
    pBB->x1 = prectInit->x;
    pBB->y1 = prectInit->y;
    pBB->x2 = prectInit->x + prectInit->width;
    pBB->y2 = prectInit->y + prectInit->height;
    while (--nrectFill) {
	prectInit++;
	if (prectInit->x < pBB->x1)
	    pBB->x1 = prectInit->x;
	if (prectInit->y < pBB->y1)
	    pBB->y1 = prectInit->y;
	if ((prectInit->x + prectInit->width) > pBB->x2)
	    pBB->x2 = prectInit->x + prectInit->width;
	if ((prectInit->y + prectInit->height) > pBB->y2)
	    pBB->y2 = prectInit->y + prectInit->height;
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
vmwarePolyFillRect(DrawablePtr pDrawable,
    GCPtr pGC, int nrectFill, xRectangle * prectInit)
{
    TRACEPOINT
    
    GC_FUNC_ACCEL_WRAPPER(pDrawable->type == DRAWABLE_WINDOW,
    			  pGC->pScreen,
			  computeBBfillrect(pDrawable, pGC, nrectFill, prectInit, &BB),
		          (pVMWARE->vmwareCapability & SVGA_CAP_RECT_FILL) &&
		           (pGC->alu == GXcopy || (pVMWARE->vmwareCapability & SVGA_CAP_RASTER_OP)) &&
		           pGC->fillStyle == FillSolid && ((pGC->planemask & pVMWARE->Pmsk) == pVMWARE->Pmsk),
			  accelPolyFillRect(pVMWARE, pDrawable, pGC, nrectFill, prectInit),
		          GC_OPS(pGC)->PolyFillRect(pDrawable, pGC, nrectFill, prectInit));
}

