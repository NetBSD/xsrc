/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/et4000/tseng_colexp.h,v 1.1.2.1 1998/02/01 16:42:12 robin Exp $ */
/*
 * Tseng acceleration interface -- color expansion primitives.
 */

#ifndef _TSENG_COLEXP_H
#define _TSENG_COLEXP_H

extern void TsengSetupForScanlineScreenToScreenColorExpand(
    int x, int y,
    int w, int h,
    int bg, int fg,
    int rop,
    unsigned int planemask);

extern void TsengSubsequentScanlineScreenToScreenColorExpand(
    int srcaddr);

extern void TsengSetupForScanlineCPUToScreenColorExpand(
    int x, int y,
    int w, int h,
    int bg, int fg,
    int rop,
    unsigned int planemask);

extern void TsengSubsequentScanlineCPUToScreenColorExpand(
    int srcaddr);

extern void TsengSetupForCPUToScreenColorExpand(
    int bg, int fg,
    int rop,
    unsigned int planemask);

extern void TsengSubsequentCPUToScreenColorExpand(
    int x, int y,
    int w, int h,
    int skipleft);

extern void TsengSetupForScreenToScreenColorExpand(
    int bg, int fg,
    int rop,
    unsigned int planemask);

extern void TsengSubsequentScreenToScreenColorExpand(
    int srcx, int srcy,
    int x, int y,
    int w, int h);

extern void ET6KWriteBitmap(
    int x, int y, int w, int h,
    unsigned char *src, int srcwidth,
    int srcx, int srcy,
    int bg, int fg, int rop,
    unsigned int planemask);

extern void W32WriteBitmap(
    int x, int y, int w, int h,
    unsigned char *src, int srcwidth,
    int srcx, int srcy,
    int bg, int fg, int rop,
    unsigned int planemask);

extern void W32ImageTextTECPUToScreenColorExpand(
    DrawablePtr pDrawable,
    GC *pGC,
    int xInit, int yInit,
    int nglyph,
    CharInfoPtr *ppci,
    unsigned char *pglyphBase);

extern void W32PolyTextTECPUToScreenColorExpand(
    DrawablePtr pDrawable,
    GC *pGC,
    int xInit, int yInit,
    int nglyph,
    CharInfoPtr *ppci,
    unsigned char *pglyphBase);

extern void TsengScanlineScreenToScreenFillStippledRect(
    DrawablePtr pDrawable,
    GCPtr pGC,
    int nBoxInit,
    BoxPtr pBoxInit);

extern void TsengSubsequentScanlineScreenToScreenFillStippledRect(
    int x, int y, int w, int h,
    unsigned char *src,
    int srcwidth,
    int stipplewidth, int stippleheight,
    int srcx, int srcy);

extern void TsengScanlineCPUToScreenFillStippledRect(
    DrawablePtr pDrawable,
    GCPtr pGC,
    int nBoxInit,
    BoxPtr pBoxInit);

extern void TsengSubsequentScanlineCPUToScreenFillStippledRect(
    int x, int y, int w, int h,
    unsigned char *src,
    int srcwidth,
    int stipplewidth, int stippleheight,
    int srcx, int srcy);
                    
#endif
