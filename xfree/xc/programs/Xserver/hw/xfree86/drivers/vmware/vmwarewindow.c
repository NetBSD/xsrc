/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwarewindow[] =

    "Id: vmwarewindow.c,v 1.4 2001/01/27 00:28:15 bennett Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwarewindow.c,v 1.2 2001/05/16 06:48:12 keithp Exp $ */

#include "vmware.h"

void
vmwareCopyWindow(WindowPtr pWin, DDXPointRec ptOldOrg, RegionPtr prgnSrc)
{
    RegionRec rgnDst;
    int dx, dy;
    WindowPtr pwinRoot;
    BoxPtr pBB;

    VMWAREPtr pVMWARE = VMWAREPTR(infoFromScreen(pWin->drawable.pScreen));

    TRACEPOINT
    if (!(pVMWARE->vmwareCapability & SVGA_CAP_RECT_COPY)) {
	pVMWARE->ScrnFuncs.CopyWindow(pWin, ptOldOrg, prgnSrc);
	return;
    }
    pwinRoot = WindowTable[pWin->drawable.pScreen->myNum];
    REGION_INIT(pWin->drawable.pScreen, &rgnDst, NullBox, 0);
    dx = ptOldOrg.x - pWin->drawable.x;
    dy = ptOldOrg.y - pWin->drawable.y;
    REGION_TRANSLATE(pWin->drawable.pScreen, prgnSrc, -dx, -dy);
    REGION_INTERSECT(pWin->drawable.pScreen, &rgnDst, &pWin->borderClip,
	prgnSrc);
    pBB = REGION_EXTENTS(pWin->drawable.pScreen, &rgnDst);
    HIDE_CURSOR_ACCEL(pVMWARE, *pBB);
    
    fbCopyRegion ((DrawablePtr) pwinRoot, (DrawablePtr) pwinRoot,
		  0,
		  &rgnDst, dx, dy, vmwareDoBitblt, 0, 0);

    SHOW_CURSOR(pVMWARE, *pBB);
    UPDATE_ACCEL_AREA(pVMWARE, *pBB);
    REGION_UNINIT(pWin->drawable.pScreen, &rgnDst);
}

static void
accelPaintWindow(VMWAREPtr pVMWARE, WindowPtr pWin, RegionPtr pRegion, int what, BoxPtr pBB)
{
    Pixel pixel;
    RegionRec drawRegion;
    RegionRec BBRegion;
    BoxPtr pbox;
    int nbox;

    if (what == PW_BACKGROUND && pWin->backgroundState == BackgroundPixel) {
	pixel = pWin->background.pixel;
    } else if (what == PW_BORDER && pWin->borderIsPixel) {
	pixel = pWin->border.pixel;
    } else {
	/*
	 * The caller has guaranteed that this case has been excluded,
	 * but the compiler doesn't know that.  So this line is just to
	 * eliminate the compiler warning.
	 */
	pixel = 0;
    }

    REGION_INIT(pWin->drawable.pScreen, &drawRegion, NullBox, 0);
    REGION_INIT(pWin->drawable.pScreen, &BBRegion, pBB, 1);
    REGION_INTERSECT(pWin->drawable.pScreen, &drawRegion, pRegion, &BBRegion);
    pbox = REGION_RECTS(&drawRegion);
    nbox = REGION_NUM_RECTS(&drawRegion);
    while (nbox) {
	vmwareWriteWordToFIFO(pVMWARE, SVGA_CMD_RECT_FILL);
	vmwareWriteWordToFIFO(pVMWARE, pixel);
	vmwareWriteWordToFIFO(pVMWARE, pbox->x1);
	vmwareWriteWordToFIFO(pVMWARE, pbox->y1);
	vmwareWriteWordToFIFO(pVMWARE, pbox->x2 - pbox->x1);
	vmwareWriteWordToFIFO(pVMWARE, pbox->y2 - pbox->y1);
	pbox++;
	nbox--;
    }
    REGION_UNINIT(pWin->drawable.pScreen, &BBRegion);
    REGION_UNINIT(pWin->drawable.pScreen, &drawRegion);
}

void
vmwarePaintWindow(WindowPtr pWin, RegionPtr pRegion, int what)
{
    VMWAREPtr pVMWARE = VMWAREPTR(infoFromScreen(pWin->drawable.pScreen));

    TRACEPOINT
    /* Accelerate solid fills */
    if ((pVMWARE->vmwareCapability & SVGA_CAP_RECT_FILL) && 
	((what == PW_BACKGROUND && pWin->backgroundState == BackgroundPixel)
	    || (what == PW_BORDER && pWin->borderIsPixel))) {
	BoxPtr pBB;
	Bool hidden = pVMWARE->mouseHidden;

	pBB = REGION_EXTENTS(pWin->drawable.pScreen, pRegion);
	if (!hidden) {
	    HIDE_CURSOR(pVMWARE, *pBB);
	}
	accelPaintWindow(pVMWARE, pWin, pRegion, what, pBB);
	if (!hidden) {
	    SHOW_CURSOR(pVMWARE, *pBB);
	}
	UPDATE_ACCEL_AREA(pVMWARE, *pBB);
	/* vmwareWaitForFB(); */ /* XXX */
	return;
    }
    if (pVMWARE->vmwareBBLevel == 0) {
	BoxPtr pBB;

	pBB = REGION_EXTENTS(pWin->drawable.pScreen, pRegion);
	HIDE_CURSOR(pVMWARE, *pBB);
	vmwareWaitForFB(pVMWARE);
	pVMWARE->vmwareBBLevel++;
	pVMWARE->ScrnFuncs.PaintWindowBackground(pWin, pRegion, what);
	pVMWARE->vmwareBBLevel--;
	vmwareSendSVGACmdUpdate(pVMWARE, pBB);
	SHOW_CURSOR(pVMWARE, *pBB);
    } else {
	VmwareLog(("vmwarePaintWindow not called at top level\n"));
	vmwareWaitForFB(pVMWARE);
	pVMWARE->ScrnFuncs.PaintWindowBackground(pWin, pRegion, what);
    }
}
