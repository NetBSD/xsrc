/* **********************************************************
 * Copyright (C) 1998-2001 VMware, Inc.
 * All Rights Reserved
 * **********************************************************/
#ifdef VMX86_DEVEL
char rcsId_vmwarecurs[] =

    "Id: vmwarecurs.c,v 1.5 2001/01/30 23:33:02 bennett Exp $";
#endif
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/vmware/vmwarecurs.c,v 1.4 2002/05/14 20:24:06 alanh Exp $ */

#include "vmware.h"
#include "bits2pixels.h"

static void
RedefineCursor(VMWAREPtr pVMWARE)
{
    int i;

    /* Define cursor */
    vmwareWriteWordToFIFO(pVMWARE, SVGA_CMD_DEFINE_CURSOR);
    vmwareWriteWordToFIFO(pVMWARE, MOUSE_ID);
    vmwareWriteWordToFIFO(pVMWARE, 0);          /* HotX/HotY seem to be zero? */
    vmwareWriteWordToFIFO(pVMWARE, 0);          /* HotX/HotY seem to be zero? */
    vmwareWriteWordToFIFO(pVMWARE, pVMWARE->CursorInfoRec->MaxWidth);
    vmwareWriteWordToFIFO(pVMWARE, pVMWARE->CursorInfoRec->MaxHeight);
    vmwareWriteWordToFIFO(pVMWARE, pVMWARE->bitsPerPixel);
    vmwareWriteWordToFIFO(pVMWARE, pVMWARE->bitsPerPixel);

    /*
     * Since we have AND and XOR masks rather than 'source' and 'mask',
     * color expand 'mask' with all zero as its foreground and all one as
     * its background.  This makes 'image & 0 ^ 'source' = source.  We
     * arange for 'image' & 1 ^ 'source' = 'image' below when we clip
     * 'source' below.
     */
    Raster_BitsToPixels((uint8 *) pVMWARE->hwcur.mask,
                        SVGA_BITMAP_INCREMENT(pVMWARE->CursorInfoRec->MaxWidth),
                        (uint8 *) pVMWARE->hwcur.maskPixmap,
                        SVGA_PIXMAP_INCREMENT(pVMWARE->CursorInfoRec->MaxWidth,
                                              pVMWARE->bitsPerPixel),
                        pVMWARE->bitsPerPixel / 8,
                        pVMWARE->CursorInfoRec->MaxWidth,
                        pVMWARE->CursorInfoRec->MaxHeight, 0, ~0);
    for (i = 0; i < SVGA_PIXMAP_SIZE(pVMWARE->CursorInfoRec->MaxWidth,
                                     pVMWARE->CursorInfoRec->MaxHeight,
                                     pVMWARE->bitsPerPixel); i++) {
	vmwareWriteWordToFIFO(pVMWARE, pVMWARE->hwcur.maskPixmap[i]);
    }

    Raster_BitsToPixels((uint8 *) pVMWARE->hwcur.source,
                        SVGA_BITMAP_INCREMENT(pVMWARE->CursorInfoRec->MaxWidth),
                        (uint8 *) pVMWARE->hwcur.sourcePixmap,
                        SVGA_PIXMAP_INCREMENT(pVMWARE->CursorInfoRec->MaxWidth,
                                              pVMWARE->bitsPerPixel),
                        pVMWARE->bitsPerPixel / 8,
                        pVMWARE->CursorInfoRec->MaxWidth,
                        pVMWARE->CursorInfoRec->MaxHeight,
                        pVMWARE->hwcur.fg, pVMWARE->hwcur.bg);

    /*
     * As pointed out above, we need to clip the expanded 'source' against
     * the expanded 'mask' since we actually have AND and XOR masks in the
     * virtual hardware.  Effectively, 'source' becomes a three color fg/bg/0
     * pixmap that XORs appropriately.
     */
    for (i = 0; i < SVGA_PIXMAP_SIZE(pVMWARE->CursorInfoRec->MaxWidth,
                                     pVMWARE->CursorInfoRec->MaxHeight,
                                     pVMWARE->bitsPerPixel); i++) {
        pVMWARE->hwcur.sourcePixmap[i] &= ~pVMWARE->hwcur.maskPixmap[i];
	vmwareWriteWordToFIFO(pVMWARE, pVMWARE->hwcur.sourcePixmap[i]);
    }

    /* Sync the FIFO, so that the definition preceeds any use of the cursor */
    vmwareWaitForFB(pVMWARE);
    pVMWARE->cursorDefined = TRUE;
}

static void
vmwareSetCursorColors(ScrnInfoPtr pScrn, int bg, int fg)
{
    VMWAREPtr pVMWARE = VMWAREPTR(pScrn);
    TRACEPOINT

    if (pVMWARE->hwcur.fg != fg || pVMWARE->hwcur.bg != bg) {
        VmwareLog(("SetCursorColors(0x%08x, 0x%08x)\n", bg, fg));
        pVMWARE->hwcur.fg = fg;
        pVMWARE->hwcur.bg = bg;
        RedefineCursor(pVMWARE);
    }
}

static void
vmwareLoadCursorImage(ScrnInfoPtr pScrn, unsigned char *src )
{
    VMWAREPtr pVMWARE = VMWAREPTR(pScrn);
    const int imageSize = SVGA_BITMAP_SIZE(pVMWARE->CursorInfoRec->MaxWidth,
                                           pVMWARE->CursorInfoRec->MaxHeight);
    TRACEPOINT

    memcpy(pVMWARE->hwcur.source, src, imageSize * sizeof(uint32));
    memcpy(pVMWARE->hwcur.mask,
           src + imageSize * sizeof(uint32), imageSize * sizeof(uint32));
    RedefineCursor(pVMWARE);
}

static void
vmwareShowCursor(ScrnInfoPtr pScrn)
{
    VMWAREPtr pVMWARE = VMWAREPTR(pScrn);
    TRACEPOINT

    if (pVMWARE->cursorDefined && !pVMWARE->cursorHidden) {
        vmwareWriteReg(pVMWARE, SVGA_REG_CURSOR_ID, MOUSE_ID);
        vmwareWriteReg(pVMWARE, SVGA_REG_CURSOR_X, pVMWARE->hwcur.x);
        vmwareWriteReg(pVMWARE, SVGA_REG_CURSOR_Y, pVMWARE->hwcur.y);
        vmwareWriteReg(pVMWARE, SVGA_REG_CURSOR_ON, SVGA_CURSOR_ON_SHOW);
    }
}

static void
vmwareHideCursor(ScrnInfoPtr pScrn)
{
    VMWAREPtr pVMWARE = VMWAREPTR(pScrn);
    TRACEPOINT

    vmwareWriteReg(pVMWARE, SVGA_REG_CURSOR_ID, MOUSE_ID);
    vmwareWriteReg(pVMWARE, SVGA_REG_CURSOR_ON, SVGA_CURSOR_ON_HIDE);
}

static void
vmwareSetCursorPosition(ScrnInfoPtr pScrn, int x, int y)
{
    VMWAREPtr pVMWARE = VMWAREPTR(pScrn);
    TRACEPOINT

    vmwareHideCursor(pScrn);

    /*
     * We're bad people.  We have no concept of a frame (VMWAREAdjustFrame()
     * is a NOP).  The hwcursor code expects us to be frame aware though, so
     * we have to do this.  I'm open to suggestions.  I tried not even
     * hooking AdjustFrame and it didn't help.
     */
    pVMWARE->hwcur.x = x + pScrn->frameX0;
    pVMWARE->hwcur.y = y + pScrn->frameY0;
    pVMWARE->hwcur.box.x1 = pVMWARE->hwcur.x;
    pVMWARE->hwcur.box.x2 = pVMWARE->hwcur.x + pVMWARE->CursorInfoRec->MaxWidth;
    pVMWARE->hwcur.box.y1 = pVMWARE->hwcur.y;
    pVMWARE->hwcur.box.y2 = pVMWARE->hwcur.y + pVMWARE->CursorInfoRec->MaxHeight;

    vmwareShowCursor(pScrn);
}

Bool
vmwareCursorInit(ScreenPtr pScreen)
{
    xf86CursorInfoPtr infoPtr;
    VMWAREPtr pVMWARE = VMWAREPTR(infoFromScreen(pScreen));
    TRACEPOINT

    /* Require cursor bypass for hwcursor.  Ignore deprecated FIFO hwcursor */
    if (!(pVMWARE->vmwareCapability & SVGA_CAP_CURSOR_BYPASS)) {
       return FALSE;
    }

    infoPtr = xf86CreateCursorInfoRec();
    if(!infoPtr) return FALSE;

    pVMWARE->CursorInfoRec = infoPtr;

    infoPtr->MaxWidth = MAX_CURS;
    infoPtr->MaxHeight = MAX_CURS;
    infoPtr->Flags = HARDWARE_CURSOR_BIT_ORDER_MSBFIRST |
                     HARDWARE_CURSOR_SOURCE_MASK_NOT_INTERLEAVED;
    infoPtr->SetCursorColors = vmwareSetCursorColors;
    infoPtr->SetCursorPosition = vmwareSetCursorPosition;
    infoPtr->LoadCursorImage = vmwareLoadCursorImage;
    infoPtr->HideCursor = vmwareHideCursor;
    infoPtr->ShowCursor = vmwareShowCursor;

    return(xf86InitCursor(pScreen, infoPtr));
}
