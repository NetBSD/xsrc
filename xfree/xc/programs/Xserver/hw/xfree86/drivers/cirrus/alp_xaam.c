/* (c) Itai Nahshon */

/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/cirrus/alp_xaam.c,v 1.4 2000/12/06 15:35:15 eich Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "compiler.h"

#include "xf86Pci.h"
#include "xf86PciInfo.h"

#include "vgaHW.h"

#include "cir.h"
#define _ALP_PRIVATE_
#include "alp.h"

#ifdef DEBUG
#define minb(p) \
        ErrorF("minb(%X)\n", p),\
        MMIO_IN8(pCir->IOBase, (p))
#define moutb(p,v) \
        ErrorF("moutb(%X)\n", p),\
	MMIO_OUT8(pCir->IOBase, (p),(v))
#define minl(p) \
        ErrorF("minl(%X)\n", p),\
        MMIO_IN32(pCir->IOBase, (p))
#define moutl(p,v) \
        ErrorF("moutl(%X)\n", p),\
	MMIO_OUT32(pCir->IOBase, (p),(v))
#else
#define minb(p) MMIO_IN8(pCir->IOBase, (p))
#define moutb(p,v) MMIO_OUT8(pCir->IOBase, (p),(v))
#define minl(p) MMIO_IN32(pCir->IOBase, (p))
#define moutl(p,v) MMIO_OUT32(pCir->IOBase, (p),(v))
#endif

#define WAIT while(minb(0x40) & pCir->chip.alp->waitMsk){};
#define WAIT_1 while(minb(0x40) & 0x1){};

static void AlpSync(ScrnInfoPtr pScrn)
{
    CirPtr pCir = CIRPTR(pScrn);
#ifdef ALP_DEBUG
	ErrorF("AlpSync mm\n");
#endif
	WAIT_1;
	return;
}

static void
AlpSetupForScreenToScreenCopy(ScrnInfoPtr pScrn, int xdir, int ydir, int rop,
								unsigned int planemask, int trans_color)
{
	CirPtr pCir = CIRPTR(pScrn);
	int pitch = pCir->pitch;

	WAIT;

#ifdef ALP_DEBUG
	ErrorF("AlpSetupForScreenToScreenCopy xdir=%d ydir=%d rop=%x planemask=%x trans_color=%x\n",
			xdir, ydir, rop, planemask, trans_color);
#endif
	moutl(0x0C, (pitch << 16) | pitch); 

}

static void
AlpSubsequentScreenToScreenCopy(ScrnInfoPtr pScrn, int x1, int y1, int x2,
								int y2, int w, int h)
{
	CirPtr pCir = CIRPTR(pScrn);
	int source, dest;
	int  hh, ww;
	int decrement = 0;
	int pitch = pCir->pitch;

	ww = ((w * pScrn->bitsPerPixel / 8) - 1) & 0x1fff;
	hh = (h - 1) & 0x1fff;
	dest = y2 * pitch + x2 * pScrn->bitsPerPixel / 8;
	source = y1 * pitch + x1 * pScrn->bitsPerPixel / 8;
	if (dest > source) {
		decrement = 1;
		dest += hh * pitch + ww;
		source += hh * pitch + ww;
	}

	WAIT;

	/* Width / Height */
	moutl(0x08, (hh << 16) | ww);
	/* source */
	moutl(0x14, source & 0x3fffff);
	moutl(0x18, 0x0d0000 | decrement);

	/* dest */
	write_mem_barrier();
	moutl(0x10, dest & 0x3fffff);

#ifdef ALP_DEBUG
	ErrorF("AlpSubsequentScreenToScreenCopy x1=%d y1=%d x2=%d y2=%d w=%d h=%d\n",
			x1, y1, x2, y2, w, h);
	ErrorF("AlpSubsequentScreenToScreenCopy s=%d d=%d ww=%d hh=%d\n",
			source, dest, ww, hh);
#endif
	if (!pCir->chip.alp->autoStart)
	  moutb(0x40,0x02);
	write_mem_barrier();
}


static void
AlpSetupForSolidFill(ScrnInfoPtr pScrn, int color, int rop,
						unsigned int planemask)
{
	CirPtr pCir = CIRPTR(pScrn);
	int pitch = pCir->pitch;

	WAIT;

#ifdef ALP_DEBUG
	ErrorF("AlpSetupForSolidFill color=%x rop=%x planemask=%x\n",
			color, rop, planemask);
#endif

	moutl(0x04, color & 0xffffff);

	/* Set dest pitch */
	moutl(0x0C, pitch & 0x1fff);
	moutl(0x18, (0xC0|((pScrn->bitsPerPixel - 8) << 1)) | 0x040d0000);
}

static void
AlpSubsequentSolidFillRect(ScrnInfoPtr pScrn, int x, int y, int w, int h)
{
	int dest;
	int hh, ww;
	CirPtr pCir = CIRPTR(pScrn);
	int pitch = pCir->pitch;

	ww = ((w * pScrn->bitsPerPixel / 8) - 1) & 0x1fff;
	hh = (h - 1) & 0x7ff;
	dest = y * pitch + x * pScrn->bitsPerPixel / 8;

	WAIT;

	/* Width / Height */
	write_mem_barrier();
	moutl(0x08, (hh << 16) | ww);

#ifdef ALP_DEBUG
	ErrorF("AlpSubsequentSolidFillRect x=%d y=%d w=%d h=%d\n",
			x, y, w, h);
#endif
	/* dest */
	moutl(0x10, (dest & 0x3fffff));

	if (!pCir->chip.alp->autoStart)
	  moutb(0x40,0x02);
	write_mem_barrier();
}

Bool
AlpXAAInitMMIO(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	CirPtr pCir = CIRPTR(pScrn);
	XAAInfoRecPtr XAAPtr;

#ifdef ALP_DEBUG
	ErrorF("AlpXAAInit mm\n");
#endif

	XAAPtr = XAACreateInfoRec();
	if (!XAAPtr) return FALSE;

	XAAPtr->SetupForScreenToScreenCopy = AlpSetupForScreenToScreenCopy;
	XAAPtr->SubsequentScreenToScreenCopy = AlpSubsequentScreenToScreenCopy;
	XAAPtr->ScreenToScreenCopyFlags = GXCOPY_ONLY|NO_TRANSPARENCY|NO_PLANEMASK;

	XAAPtr->SetupForSolidFill = AlpSetupForSolidFill;
	XAAPtr->SubsequentSolidFillRect = AlpSubsequentSolidFillRect;
	XAAPtr->SubsequentSolidFillTrap = NULL;
	XAAPtr->SolidFillFlags = GXCOPY_ONLY|NO_TRANSPARENCY|NO_PLANEMASK;

	XAAPtr->Sync = AlpSync;

	if (pCir->Chipset != PCI_CHIP_GD7548)
	{
	  moutb(0x3CE, 0x0E); /* enable writes to gr33 */
	  moutb(0x3CF, 0x20); /* enable writes to gr33 */
        }

	if (pCir->properties & ACCEL_AUTOSTART) {
	  moutb(0x40, 0x80); /* enable autostart */
	  pCir->chip.alp->waitMsk = 0x10;
	  pCir->chip.alp->autoStart = TRUE;
	} else {
	  pCir->chip.alp->waitMsk = 0x1;
	  pCir->chip.alp->autoStart = FALSE;
	}

	if (!XAAInit(pScreen, XAAPtr))
		return FALSE;

	return TRUE;
}

