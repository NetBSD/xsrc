/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwareblt[] =

    "Id: vmwareblt.c,v 1.4 2001/01/27 00:28:15 bennett Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwareblt.c,v 1.3 2001/09/05 22:13:10 keithp Exp $ */

#include "X.h"
#include "fb.h"
#include "vmware.h"

void
vmwareDoBitblt(DrawablePtr  pSrc,
	       DrawablePtr  pDst,
	       GCPtr	    pGC,
	       BoxPtr	    pbox,
	       int	    nbox,
	       int	    dx,
	       int	    dy,
	       Bool	    reverse,
	       Bool	    upsidedown,
	       Pixel	    bitplane,
	       void	    *closure)
{
    VMWAREPtr pVMWARE;
    CARD8   alu;

    pVMWARE = VMWAREPTR(infoFromScreen(pSrc->pScreen));
    if (pGC)
	alu = pGC->alu;
    else
	alu = GXcopy;

    /* Send the commands */
    while (nbox--) {
	vmwareWriteWordToFIFO(pVMWARE, SVGA_CMD_RECT_ROP_COPY);
	vmwareWriteWordToFIFO(pVMWARE, pbox->x1 + dx);
	vmwareWriteWordToFIFO(pVMWARE, pbox->y1 + dy);
	vmwareWriteWordToFIFO(pVMWARE, pbox->x1);
	vmwareWriteWordToFIFO(pVMWARE, pbox->y1);
	vmwareWriteWordToFIFO(pVMWARE, pbox->x2 - pbox->x1);
	vmwareWriteWordToFIFO(pVMWARE, pbox->y2 - pbox->y1);
	vmwareWriteWordToFIFO(pVMWARE, alu);
	pbox++;
    }
}

RegionPtr
vmwareCopyArea(DrawablePtr pSrcDrawable,
DrawablePtr pDstDrawable,
GCPtr pGC, int srcx, int srcy, int width, int height, int dstx, int dsty)
{
    RegionPtr prgn;
    VMWAREPtr pVMWARE = VMWAREPTR(infoFromScreen(pGC->pScreen));

    TRACEPOINT
    if ((pVMWARE->vmwareCapability & SVGA_CAP_RECT_COPY) &&
	(pGC->alu == GXcopy || (pVMWARE->vmwareCapability & SVGA_CAP_RASTER_OP)) &&
	pSrcDrawable->type == DRAWABLE_WINDOW &&
	pDstDrawable->type == DRAWABLE_WINDOW &&
	(pGC->planemask & pVMWARE->Pmsk) == pVMWARE->Pmsk) {
	fbCopyProc  doBitBlt;
	BoxRec updateBB;
	BoxRec mouseBB;
	Bool hidden = pVMWARE->mouseHidden;

	updateBB.x1 = pDstDrawable->x + dstx;
	updateBB.y1 = pDstDrawable->y + dsty;
	updateBB.x2 = updateBB.x1 + width;
	updateBB.y2 = updateBB.y1 + height;
	mouseBB.x1 = MIN(pSrcDrawable->x + srcx, pDstDrawable->x + dstx);
	mouseBB.y1 = MIN(pSrcDrawable->y + srcy, pDstDrawable->y + dsty);
	mouseBB.x2 = MAX(pSrcDrawable->x + srcx, pDstDrawable->x + dstx) + width;
	mouseBB.y2 = MAX(pSrcDrawable->y + srcy, pDstDrawable->y + dsty) + height;
	doBitBlt = vmwareDoBitblt;
	if (!hidden) {
	    HIDE_CURSOR_ACCEL(pVMWARE, mouseBB);
	}
	prgn = fbDoCopy (pSrcDrawable, pDstDrawable, pGC, srcx, srcy, width,
			 height, dstx, dsty, doBitBlt, 0, 0);
	if (!hidden) {
	    SHOW_CURSOR(pVMWARE, mouseBB);
	}
	UPDATE_ACCEL_AREA(pVMWARE, updateBB);
    } else if (pDstDrawable->type == DRAWABLE_WINDOW ||
	pSrcDrawable->type == DRAWABLE_WINDOW) {
	if (pVMWARE->vmwareBBLevel == 0) {
	    BoxRec updateBB;
	    BoxRec mouseBB;

	    if (pDstDrawable->type == DRAWABLE_WINDOW &&
		pSrcDrawable->type != DRAWABLE_WINDOW) {
		updateBB.x1 = pDstDrawable->x + dstx;
		updateBB.y1 = pDstDrawable->y + dsty;
		updateBB.x2 = updateBB.x1 + width;
		updateBB.y2 = updateBB.y1 + height;
		mouseBB = updateBB;
	    } else if (pDstDrawable->type != DRAWABLE_WINDOW &&
		       pSrcDrawable->type == DRAWABLE_WINDOW) {
		updateBB.x1 = pSrcDrawable->x + srcx;
		updateBB.y1 = pSrcDrawable->y + srcy;
		updateBB.x2 = updateBB.x1 + width;
		updateBB.y2 = updateBB.y1 + height;
		mouseBB = updateBB;
	    } else {
		updateBB.x1 = pDstDrawable->x + dstx;
		updateBB.y1 = pDstDrawable->y + dsty;
		updateBB.x2 = updateBB.x1 + width;
		updateBB.y2 = updateBB.y1 + height;
		mouseBB.x1 = MIN(pSrcDrawable->x + srcx, pDstDrawable->x + dstx);
		mouseBB.y1 = MIN(pSrcDrawable->y + srcy, pDstDrawable->y + dsty);
		mouseBB.x2 = MAX(pSrcDrawable->x + srcx, pDstDrawable->x + dstx) + width;
		mouseBB.y2 = MAX(pSrcDrawable->y + srcy, pDstDrawable->y + dsty) + height;
	    }
	    HIDE_CURSOR(pVMWARE, mouseBB);
	    vmwareWaitForFB(pVMWARE);
	    pVMWARE->vmwareBBLevel++;
	    prgn =
		GC_OPS(pGC)->CopyArea(pSrcDrawable,
		pDstDrawable, pGC, srcx, srcy, width, height, dstx, dsty);
	    pVMWARE->vmwareBBLevel--;
	    if (pDstDrawable->type == DRAWABLE_WINDOW) {
		vmwareSendSVGACmdUpdate(pVMWARE, &updateBB);
	    }
	    SHOW_CURSOR(pVMWARE, mouseBB);
	} else {
	    vmwareWaitForFB(pVMWARE);
	    prgn =
		GC_OPS(pGC)->CopyArea(pSrcDrawable,
		pDstDrawable, pGC, srcx, srcy, width, height, dstx, dsty);
	}
    } else {
	prgn =
	    GC_OPS(pGC)->CopyArea(pSrcDrawable, pDstDrawable,
	    pGC, srcx, srcy, width, height, dstx, dsty);
    }
    return prgn;
}

RegionPtr
vmwareCopyPlane(DrawablePtr pSrcDrawable,
DrawablePtr pDstDrawable,
GCPtr pGC,
int srcx, int srcy, int width, int height, int dstx, int dsty, unsigned long bitPlane)
{
    RegionPtr prgn;
    VMWAREPtr pVMWARE = VMWAREPTR(infoFromScreen(pGC->pScreen));

    TRACEPOINT

    if (pDstDrawable->type == DRAWABLE_WINDOW ||
	pSrcDrawable->type == DRAWABLE_WINDOW) {
	if (pVMWARE->vmwareBBLevel == 0) {
	    BoxRec updateBB;
	    BoxRec mouseBB;

	    if (pDstDrawable->type == DRAWABLE_WINDOW &&
		pSrcDrawable->type != DRAWABLE_WINDOW) {
		updateBB.x1 = pDstDrawable->x + dstx;
		updateBB.y1 = pDstDrawable->y + dsty;
		updateBB.x2 = updateBB.x1 + width;
		updateBB.y2 = updateBB.y1 + height;
		mouseBB = updateBB;
	    } else if (pDstDrawable->type != DRAWABLE_WINDOW &&
		       pSrcDrawable->type == DRAWABLE_WINDOW) {
		updateBB.x1 = pSrcDrawable->x + srcx;
		updateBB.y1 = pSrcDrawable->y + srcy;
		updateBB.x2 = updateBB.x1 + width;
		updateBB.y2 = updateBB.y1 + height;
		mouseBB = updateBB;
	    } else {
		updateBB.x1 = pDstDrawable->x + dstx;
		updateBB.y1 = pDstDrawable->y + dsty;
		updateBB.x2 = updateBB.x1 + width;
		updateBB.y2 = updateBB.y1 + height;
		mouseBB.x1 = MIN(pSrcDrawable->x + srcx, pDstDrawable->x + dstx);
		mouseBB.y1 = MIN(pSrcDrawable->y + srcy, pDstDrawable->y + dsty);
		mouseBB.x2 = MAX(pSrcDrawable->x + srcx, pDstDrawable->x + dstx) + width;
		mouseBB.y2 = MAX(pSrcDrawable->y + srcy, pDstDrawable->y + dsty) + height;
	    }
	    HIDE_CURSOR(pVMWARE, mouseBB);
	    vmwareWaitForFB(pVMWARE);
	    pVMWARE->vmwareBBLevel++;
	    prgn = fbCopyPlane(pSrcDrawable,
			       pDstDrawable, pGC, srcx, srcy, width, height, dstx, dsty,
			       bitPlane);
	    pVMWARE->vmwareBBLevel--;
	    if (pDstDrawable->type == DRAWABLE_WINDOW) {
		vmwareSendSVGACmdUpdate(pVMWARE, &updateBB);
	    }
	    SHOW_CURSOR(pVMWARE, mouseBB);
	} else {
	    vmwareWaitForFB(pVMWARE);
	    prgn = fbCopyPlane(pSrcDrawable,
			       pDstDrawable, pGC, srcx, srcy, width, height, dstx, dsty,
			       bitPlane);
	}
    } else {
	prgn = fbCopyPlane(pSrcDrawable, pDstDrawable,
			   pGC, srcx, srcy, width, height, dstx, dsty, bitPlane);
    }
    return prgn;
}

