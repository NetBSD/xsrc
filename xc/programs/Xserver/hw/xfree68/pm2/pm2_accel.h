/* $XFree86: xc/programs/Xserver/hw/xfree68/pm2/pm2_accel.h,v 1.1.2.2 1999/06/02 12:08:27 hohndel Exp $ */
#ifndef PM2_ACCEL_H
#define PM2_ACCEL_H

#include <asm/system.h>
#include "glint.h"
#include "glint_regs.h"
#include "pm2im.h"

/* Synchronize with the Permedia2 (Wait until all its ops are finished) */
#define PM2_WAIT_IDLE()	\
	GLINT_WAIT(2); \
	GLINT_WRITE_REG(FM_PassSyncTag, FilterMode); \
	GLINT_WRITE_REG(0, GlintSync); \
	do {while (GLINT_READ_REG(OutFIFOWords) == 0){rmb();} \
	} while (GLINT_READ_REG(OutputFIFO) != (GLINT_TAG(0x18,0x08)>>3))

extern ScrnInfoRec fbdevInfoRec;

/* pm2_accel.c */
extern void Permedia2SetupForScreenToScreenCopy(int, int, int, unsigned, int);
extern void Permedia2SubsequentScreenToScreenCopy(int, int, int, int, int, int);
extern void Permedia2SetupForScreenToScreenCopy32bpp(int, int, int, unsigned, int);
extern void Permedia2SubsequentScreenToScreenCopy32bpp(int, int, int, int, int, int);
extern void Permedia2ImageWrite(int, unsigned, int, int, int, int);
extern void Permedia2SetupForFillRectSolid(int, int, unsigned);
extern void Permedia2SubsequentFillRectSolid(int, int, int, int);
extern void Permedia2SetupForFillRectStipple(int, int, int, int, int, unsigned, int);
extern void Permedia2SubsequentFillRectStipple(int, int, int, int);
extern void Permedia2SubsequentFillRectSolidBitmask(int, int, int, int);
extern void Permedia2SetupForFillRectSolidBitmask(int, int, int, int, unsigned, int);

/* pm2blt.c */
extern void pm2fbDoBitBlt(DrawablePtr, DrawablePtr, GCPtr, RegionPtr, DDXPointPtr, unsigned long);
extern void pm2fbGetImage(DrawablePtr, int, int, int, int, unsigned int, unsigned long, char*);
extern RegionPtr pm2fbCopyArea(register DrawablePtr, register DrawablePtr, GC*, int, int, int, int, int, int);

/* pm2fbdev.c */
extern pointer GLINTMMIOBase;	
extern int pm2fbMaxX, pm2fbMaxY, pm2fbVirtX, pm2fbVirtY;
extern int UsePCIRetry, Bppshift, pprod;
extern int pm2fbPixmapIndex;

extern void (*pm2fbSetupForScreenToScreenCopy)(int, int, int, unsigned, int);
extern void (*pm2fbSubsequentScreenToScreenCopy)(int, int, int, int, int, int);

/* pm2frect.c */
extern void pm2fbPolyFillRect(DrawablePtr, register GCPtr, int, xRectangle*);

/* pm2fs.c */
extern void pm2fbSolidFSpans(DrawablePtr, GCPtr, int, DDXPointPtr, int*, int);
extern void pm2fbTiledFSpans(DrawablePtr, GCPtr, int, DDXPointPtr, int*, int);
extern void pm2fbStipFSpans(DrawablePtr, GCPtr, int, DDXPointPtr, int*, int);
extern void pm2fbOStipFSpans(DrawablePtr, GCPtr, int, DDXPointPtr, int*, int);

/* pm2gc.c */
extern void pm2fbInitGC();
extern Bool pm2fbCreateGC(register GCPtr);

/* pm2im.c */
extern void (*pm2fbImageReadFunc)(int, int, int, int, char*, int, int, int, unsigned long);
extern void (*pm2fbImageWriteFunc)(int, int, int, int, char*, int, int, int, int, unsigned long);
extern void pm2fbImageInit();
extern void pm2fbImageStippleFunc(int, int, int, int, char*, int, int, int, Pixel, Pixel, int, unsigned long, int);

/* pm2orect.c */
extern void pm2fbPolyRectangle(DrawablePtr, GCPtr, int, xRectangle*);

/* pm2pcach.c */
extern CacheInfoPtr pm2fbCacheInfo;

extern void pm2fbCacheInit(int, int);
extern void pm2fbCacheFreeSlot(PixmapPtr);
extern void pm2fbIncrementCacheLRU(int);
extern int pm2fbCacheTile(PixmapPtr);
extern int pm2fbCacheStipple(PixmapPtr, int);
extern int pm2fbCacheOpStipple(PixmapPtr, int, int);
extern Bool pm2fbCachableTile(PixmapPtr);
extern Bool pm2fbCachableStipple(PixmapPtr);
extern Bool pm2fbCachableOpStipple(PixmapPtr);

/* pm2pntwn.c */
extern void pm2fbPaintWindow(WindowPtr, RegionPtr, int);

/* pm2win.c */
extern void pm2fbCopyWindow(WindowPtr, DDXPointRec, RegionPtr);

#endif
