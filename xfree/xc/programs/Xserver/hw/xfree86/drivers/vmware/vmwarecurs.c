/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwarecurs[] =

    "Id: vmwarecurs.c,v 1.5 2001/01/30 23:33:02 bennett Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwarecurs.c,v 1.1 2001/04/05 19:29:44 dawes Exp $ */

#include "vmware.h"
#include "cursorstr.h"
#include "bits2pixels.h"

typedef struct _VMwareCursPriv {
    uint8 *bSource;
    uint8 *bMask;
    uint8 *pSource;
    uint8 *pMask;
} VMwareCursPriv;

static int vmwareCursGeneration = -1;
static CursorPtr vmwareSaveCursors[MAXSCREENS];

static miPointerSpriteFuncRec vmwarePointerSpriteFuncs = {
    vmwareRealizeCursor,
    vmwareUnrealizeCursor,
    vmwareSetCursor,
    vmwareMoveCursor,
};

Bool
vmwareCursorInit(char *pm, 
		 ScreenPtr pScr)
{
    TRACEPOINT
    if (vmwareCursGeneration != serverGeneration) {
	miPointerScreenFuncPtr xf86scrn = xf86GetPointerScreenFuncs();
	
	pScr->QueryBestSize = vmwareQueryBestSize;
	if (!(miPointerInitialize(pScr, &vmwarePointerSpriteFuncs,
	    xf86scrn, FALSE)))
	    return FALSE;

	pScr->RecolorCursor = vmwareRecolorCursor;
	vmwareCursGeneration = serverGeneration;
    }
    return TRUE;
}

static void ZeroBits(uint8 *buf, int preserveBits, int totalBits) {
    while (preserveBits > 8) {
	buf++;
	preserveBits -= 8;
	totalBits -= 8;
    }
    *buf &= ~(~0 << preserveBits);
    buf++;
    preserveBits -= 8;
    totalBits -= 8;
    while (totalBits > 0) {
	*buf++ = 0;
	totalBits -= 8;
    }
}

Bool
vmwareRealizeCursor(ScreenPtr pScr, 
		    CursorPtr pCurs)
{
    pointer *pPriv = &pCurs->bits->devPriv[pScr->myNum];
    VMwareCursPriv *cursPriv;
    int i;
    uint8 *bits;
    VMWAREPtr pVMWARE;

    TRACEPOINT
    if (pCurs->bits->refcnt > 1)
	return TRUE;
    cursPriv = (VMwareCursPriv *) xcalloc(1, sizeof(VMwareCursPriv));
    if (!cursPriv)
	return FALSE;
    cursPriv->bSource = (uint8 *) xcalloc(1, PixmapBytePad(pCurs->bits->width, 1) * pCurs->bits->height);
    if (!cursPriv->bSource) {
	xfree(cursPriv);
	return FALSE;
    }
    pVMWARE = VMWAREPTR(infoFromScreen(pScr));

    memcpy(cursPriv->bSource, pCurs->bits->source, PixmapBytePad(pCurs->bits->width, 1) * pCurs->bits->height);
    bits = cursPriv->bSource;
    /* Handle cursors that do not have zero bits beyond the right edge of each scanline */
    for (i = 0; i < pCurs->bits->height; i++) {
	ZeroBits(bits, pCurs->bits->width, PixmapBytePad(pCurs->bits->width, 1) * 8);
	bits += PixmapBytePad(pCurs->bits->width, 1);
    }
    /* Raster_BitsToPixels expects most significant bit first */
    for (i = 0; i < PixmapBytePad(pCurs->bits->width, 1) * pCurs->bits->height; i++) {
	cursPriv->bSource[i] = 
	    (cursPriv->bSource[i] & 0x01) << 7 |
	    (cursPriv->bSource[i] & 0x02) << 5 |
	    (cursPriv->bSource[i] & 0x04) << 3 |
	    (cursPriv->bSource[i] & 0x08) << 1 |
	    (cursPriv->bSource[i] & 0x10) >> 1 |
	    (cursPriv->bSource[i] & 0x20) >> 3 |
	    (cursPriv->bSource[i] & 0x40) >> 5 |
	    (cursPriv->bSource[i] & 0x80) >> 7;
    }
    cursPriv->bMask = (uint8 *) xcalloc(1, PixmapBytePad(pCurs->bits->width, 1) * pCurs->bits->height);
    if (!cursPriv->bMask) {
	xfree(cursPriv->bSource);
	xfree(cursPriv);
	return FALSE;
    }
    memcpy(cursPriv->bMask, pCurs->bits->mask, PixmapBytePad(pCurs->bits->width, 1) * pCurs->bits->height);
    bits = cursPriv->bMask;
    /* Handle cursors that do not have zero bits beyond the right edge of each scanline */
    for (i = 0; i < pCurs->bits->height; i++) {
	ZeroBits(bits, pCurs->bits->width, PixmapBytePad(pCurs->bits->width, 1) * 8);
	bits += PixmapBytePad(pCurs->bits->width, 1);
    }
    for (i = 0; i < PixmapBytePad(pCurs->bits->width, 1) * pCurs->bits->height; i++) {
	cursPriv->bMask[i] =
	    (cursPriv->bMask[i] & 0x01) << 7 |
	    (cursPriv->bMask[i] & 0x02) << 5 |
	    (cursPriv->bMask[i] & 0x04) << 3 |
	    (cursPriv->bMask[i] & 0x08) << 1 |
	    (cursPriv->bMask[i] & 0x10) >> 1 |
	    (cursPriv->bMask[i] & 0x20) >> 3 |
	    (cursPriv->bMask[i] & 0x40) >> 5 |
	    (cursPriv->bMask[i] & 0x80) >> 7;
    }
    cursPriv->pSource = (uint8 *) xcalloc(4, SVGA_PIXMAP_SIZE(pCurs->bits->width, pCurs->bits->height, pVMWARE->bitsPerPixel));
    if (!cursPriv->pSource) {
	xfree(cursPriv->bMask);
	xfree(cursPriv->bSource);
	xfree(cursPriv);
	return FALSE;
    }
    cursPriv->pMask = (uint8 *) xcalloc(4, SVGA_PIXMAP_SIZE(pCurs->bits->width, pCurs->bits->height, pVMWARE->bitsPerPixel));
    if (!cursPriv->pMask) {
	xfree(cursPriv->pSource);
	xfree(cursPriv->bMask);
	xfree(cursPriv->bSource);
	xfree(cursPriv);
	return FALSE;
    }
    Raster_BitsToPixels(cursPriv->bMask, PixmapBytePad(pCurs->bits->width, 1),
			cursPriv->pMask, 4 * SVGA_PIXMAP_SCANLINE_SIZE(pCurs->bits->width, pVMWARE->bitsPerPixel),
			pVMWARE->bitsPerPixel / 8, 
			pCurs->bits->width, pCurs->bits->height, 0, ~0);
    *pPriv = (pointer) cursPriv;
    return TRUE;
}

Bool
vmwareUnrealizeCursor(ScreenPtr pScr, 
		      CursorPtr pCurs)
{
    VMwareCursPriv *cursPriv;

    TRACEPOINT
    if (pCurs->bits->refcnt <= 1 &&
	(cursPriv = pCurs->bits->devPriv[pScr->myNum])) {
	xfree(cursPriv->pMask);
	xfree(cursPriv->pSource);
	xfree(cursPriv->bMask);
	xfree(cursPriv->bSource);
	xfree(cursPriv);
    }
    return TRUE;
}

static void
vmwareLoadCursor(ScreenPtr pScr, 
		 CursorPtr pCurs, 
		 int x, 
		 int y)
{
    VMWAREPtr pVMWARE;

    TRACEPOINT

    if (!pCurs)
	return;

    pVMWARE = VMWAREPTR(infoFromScreen(pScr));

    pVMWARE->Mouse.Width = pCurs->bits->width;
    pVMWARE->Mouse.Height = pCurs->bits->height;
    pVMWARE->Mouse.XHot = pCurs->bits->xhot;
    pVMWARE->Mouse.YHot = pCurs->bits->yhot;
    vmwareRecolorCursor(pScr, pCurs, TRUE);
    vmwareMoveCursor(pScr, x, y);

    if (!(pVMWARE->vmwareCapability & SVGA_CAP_CURSOR_BYPASS)) {
	vmwareWriteWordToFIFO(pVMWARE, SVGA_CMD_DISPLAY_CURSOR);
	vmwareWriteWordToFIFO(pVMWARE, MOUSE_ID);
	vmwareWriteWordToFIFO(pVMWARE, 1);
    }
}

void
vmwareSetCursor(ScreenPtr pScr, 
		CursorPtr pCurs, 
		int x, 
		int y)
{
    int index = pScr->myNum;

    TRACEPOINT
    
    if (!pCurs)
	return;

    vmwareSaveCursors[index] = pCurs;
    vmwareLoadCursor(pScr, pCurs, x, y);
}

void
vmwareRestoreCursor(ScreenPtr pScr)
{
    int index = pScr->myNum;
    int x, y;

    TRACEPOINT
    
    miPointerPosition(&x, &y);
    vmwareLoadCursor(pScr, vmwareSaveCursors[index], x, y);
}

void
vmwareMoveCursor(ScreenPtr pScr,
		 int x, 
		 int y)
{
    VMWAREPtr pVMWARE;
    
    TRACEPOINT

    pVMWARE = VMWAREPTR(infoFromScreen(pScr));

    pVMWARE->Mouse.Box.x1 = x - pVMWARE->Mouse.XHot;
    if (pVMWARE->Mouse.Box.x1 < 0) pVMWARE->Mouse.Box.x1 = 0;
    pVMWARE->Mouse.Box.y1 = y - pVMWARE->Mouse.YHot;
    if (pVMWARE->Mouse.Box.y1 < 0) pVMWARE->Mouse.Box.y1 = 0;

    pVMWARE->Mouse.Box.x2 = pVMWARE->Mouse.Box.x1 + pVMWARE->Mouse.Width;
    pVMWARE->Mouse.Box.y2 = pVMWARE->Mouse.Box.y1 + pVMWARE->Mouse.Height;

    if (pVMWARE->cursorDefined && !pVMWARE->mouseHidden) {
	if (pVMWARE->vmwareCapability & SVGA_CAP_CURSOR_BYPASS) {
	    vmwareWriteReg(pVMWARE, SVGA_REG_CURSOR_ID, MOUSE_ID);
	    vmwareWriteReg(pVMWARE, SVGA_REG_CURSOR_X, x);
	    vmwareWriteReg(pVMWARE, SVGA_REG_CURSOR_Y, y);
	    vmwareWriteReg(pVMWARE, SVGA_REG_CURSOR_ON, 1);
	} else {
	    vmwareWriteWordToFIFO(pVMWARE, SVGA_CMD_MOVE_CURSOR);
	    vmwareWriteWordToFIFO(pVMWARE, x);
	    vmwareWriteWordToFIFO(pVMWARE, y);
	    UPDATE_ACCEL_AREA(pVMWARE, pVMWARE->Mouse.Box);
	}
    }
}

void
vmwareRecolorCursor(ScreenPtr pScr, 
		    CursorPtr pCurs, 
		    Bool displayed)
{
    xColorItem	sourceColor;
    xColorItem	maskColor;
    VMwareCursPriv *cursPriv = pCurs->bits->devPriv[pScr->myNum];
    uint32 *b1, *b2;
    int i;
    VMWAREPtr pVMWARE;

    TRACEPOINT

    if (!displayed)
	return;

    pVMWARE = VMWAREPTR(infoFromScreen(pScr));

    sourceColor.red = pCurs->foreRed;
    sourceColor.green = pCurs->foreGreen;
    sourceColor.blue = pCurs->foreBlue;

    maskColor.red = pCurs->backRed;
    maskColor.green = pCurs->backGreen;
    maskColor.blue = pCurs->backBlue;

    if (pScr->rootDepth > 8) 
    {
	sourceColor.pixel = (sourceColor.red >> (16 - pVMWARE->weight.red)) << pVMWARE->offset.red |
	    (sourceColor.green >> (16 - pVMWARE->weight.green)) << pVMWARE->offset.green |
	    (sourceColor.blue >> (16 - pVMWARE->weight.blue)) << pVMWARE->offset.blue;
	maskColor.pixel = (maskColor.red >> (16 - pVMWARE->weight.red)) << pVMWARE->offset.red |
	    (maskColor.green >> (16 - pVMWARE->weight.green)) << pVMWARE->offset.green |
	    (maskColor.blue >> (16 - pVMWARE->weight.blue)) << pVMWARE->offset.blue;
    } 
    else
    {
	ColormapPtr	pmap;

	pmap = miInstalledMaps[pScr->myNum];
    
	FakeAllocColor(pmap, &sourceColor);
	FakeAllocColor(pmap, &maskColor);

	FakeFreeColor(pmap, sourceColor.pixel);
	FakeFreeColor(pmap, maskColor.pixel);

	maskColor.red = maskColor.red >> 8;
	maskColor.green = maskColor.green >> 8;
	maskColor.blue = maskColor.blue >> 8;

	sourceColor.red = sourceColor.red >> 8;
	sourceColor.green = sourceColor.green >> 8;
	sourceColor.blue = sourceColor.blue >> 8;
    }

    /* Calculate XOR mask */
    Raster_BitsToPixels(cursPriv->bSource, PixmapBytePad(pCurs->bits->width, 1),
			cursPriv->pSource, 
			4 * SVGA_PIXMAP_SCANLINE_SIZE(pCurs->bits->width, pVMWARE->bitsPerPixel), 
			pVMWARE->bitsPerPixel / 8,
			pCurs->bits->width, pCurs->bits->height, sourceColor.pixel, maskColor.pixel);
    b1 = (uint32 *) cursPriv->pSource;
    b2 = (uint32 *) cursPriv->pMask;
    for (i = 0; i < SVGA_PIXMAP_SIZE(pCurs->bits->width, pCurs->bits->height, pVMWARE->bitsPerPixel); i++) {
	*b1++ &= ~*b2++;
    }

    /* Define cursor */
    vmwareWriteWordToFIFO(pVMWARE, SVGA_CMD_DEFINE_CURSOR);
    vmwareWriteWordToFIFO(pVMWARE, MOUSE_ID);
    vmwareWriteWordToFIFO(pVMWARE, pCurs->bits->xhot);
    vmwareWriteWordToFIFO(pVMWARE, pCurs->bits->yhot);
    vmwareWriteWordToFIFO(pVMWARE, pCurs->bits->width);
    vmwareWriteWordToFIFO(pVMWARE, pCurs->bits->height);
    vmwareWriteWordToFIFO(pVMWARE, pVMWARE->bitsPerPixel);
    vmwareWriteWordToFIFO(pVMWARE, pVMWARE->bitsPerPixel);
    b1 = (uint32 *) cursPriv->pSource;
    b2 = (uint32 *) cursPriv->pMask;
    for (i = 0; i < SVGA_PIXMAP_SIZE(pCurs->bits->width, pCurs->bits->height, pVMWARE->bitsPerPixel); i++) {
	vmwareWriteWordToFIFO(pVMWARE, *b2++);
    }
    for (i = 0; i < SVGA_PIXMAP_SIZE(pCurs->bits->width, pCurs->bits->height, pVMWARE->bitsPerPixel); i++) {
      vmwareWriteWordToFIFO(pVMWARE, *b1++);
	/*	vmwareWriteWordToFIFO(~0); */
    }
    pVMWARE->checkCursorColor = FALSE;

    /* Sync the FIFO, so that the definition preceeds any use of the cursor */
    UPDATE_ACCEL_AREA(pVMWARE, pVMWARE->Mouse.Box);
    vmwareWaitForFB(pVMWARE);
    pVMWARE->cursorDefined = TRUE;
}

void
vmwareRenewCursorColor(ScreenPtr pScr)
{
    TRACEPOINT

    if (vmwareSaveCursors[pScr->myNum])
	vmwareRecolorCursor(pScr, vmwareSaveCursors[pScr->myNum], TRUE);
}

void
vmwareBlockHandler(i, blockData, pTimeout, pReadmask)
    int i;
    pointer blockData;
    pointer pTimeout;
    pointer pReadmask;
{
    VMWAREPtr pVMWARE;

    TRACEPOINT

    pVMWARE = VMWAREPTR(xf86Screens[i]);
    if (pVMWARE->checkCursorColor)
	vmwareRenewCursorColor(screenInfo.screens[i]);
}

void
vmwareQueryBestSize(class, pwidth, pheight, pScr)
    int class;
    unsigned short *pwidth;
    unsigned short *pheight;
    ScreenPtr pScr;
{
    TRACEPOINT

    if (*pwidth > 0) {

	switch (class) {

	case CursorShape:
	    if (*pwidth > 64)
		*pwidth = 64;
	    if (*pheight > 64)
		*pheight = 64;
	    break;
	default:
	    mfbQueryBestSize(class, pwidth, pheight, pScr);
	    break;
	}
    }
}

void
vmwareCursorOff(VMWAREPtr pVMWARE)
{
    TRACEPOINT

    if (pVMWARE->cursorDefined) {
	if (pVMWARE->vmwareCapability & SVGA_CAP_CURSOR_BYPASS) {
	    vmwareWriteReg(pVMWARE, SVGA_REG_CURSOR_ID, MOUSE_ID);
	    vmwareWriteReg(pVMWARE, SVGA_REG_CURSOR_ON, 0);
	} else {
	    vmwareWriteWordToFIFO(pVMWARE, SVGA_CMD_DISPLAY_CURSOR);
	    vmwareWriteWordToFIFO(pVMWARE, MOUSE_ID);
	    vmwareWriteWordToFIFO(pVMWARE, 0);
	    UPDATE_ACCEL_AREA(pVMWARE, pVMWARE->Mouse.Box);
	}
    }
}

void
vmwareClearSavedCursor(int scr_index)
{
    TRACEPOINT

    vmwareSaveCursors[scr_index] = NULL;
}


