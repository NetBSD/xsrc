/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis530_accel.c,v 1.1 2000/02/12 20:45:33 dawes Exp $ */

/*
 *
 *	Acceleration for SiS530 SiS620.
 *	It is done in a separate file because the register formats are 
 *      very different from the previous chips.
 *
 *
 *	
 *	Xavier Ducoin <x.ducoin@lectra.com>
 */

/*#define DEBUG*/
#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"

#include "xf86PciInfo.h"
#include "xf86Pci.h"

#include "miline.h"

#include "sis_regs2.h"
#include "sis.h"


static void SiS2Sync(ScrnInfoPtr pScrn);
static void SiS2SetupForFillRectSolid(ScrnInfoPtr pScrn, int color,
				int rop, unsigned int planemask);
static void SiS2SubsequentFillRectSolid(ScrnInfoPtr pScrn, int x,
				int y, int w, int h);
static void SiS2SubsequentScreenToScreenCopy(ScrnInfoPtr pScrn,
				int x1, int y1, int x2,
				int y2, int w, int h);
static void SiS2SetupForScreenToScreenCopy(ScrnInfoPtr pScrn,
				int xdir, int ydir, int rop, 
                                unsigned int planemask,
				int transparency_color);
static void SiS2SetupForMono8x8PatternFill(ScrnInfoPtr pScrn, 
				int patternx, int patterny, int fg, int bg, 
				int rop, unsigned int planemask);
static void SiS2SubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn, 
				int patternx, int patterny, int x, int y, 
				int w, int h);
static void SiS2SetupForScreenToScreenColorExpandFill(ScrnInfoPtr pScrn,
				int fg, int bg, int rop, 
				unsigned int planemask);
#if 0
static void SiS2SubsequentScreenToScreenColorExpandFill(ScrnInfoPtr pScrn,
				int x, int y, int w, int h,
				int srcx, int srcy, int skipleft);
#endif
static void SiS2SubsequentScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
				int x, int y, int w, int h, int skipleft);
static void SiS2SubsequentColorExpandScanline(ScrnInfoPtr pScrn, int bufno);




static void
SiS2InitializeAccelerator(ScrnInfoPtr pScrn)
{
}

Bool
SiS530AccelInit(ScreenPtr pScreen)
{
    XAAInfoRecPtr infoPtr;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SISPtr pSiS = SISPTR(pScrn);
    BoxRec AvailFBArea;
    int offset=0;
    int OffscreenAvailable, BLTPatternOffscreenSize;

    pSiS->AccelInfoPtr = infoPtr = XAACreateInfoRec();
    if (!infoPtr) return FALSE;

    SiS2InitializeAccelerator(pScrn);

    /* fill out infoPtr here */
    infoPtr->Flags = PIXMAP_CACHE |
		     OFFSCREEN_PIXMAPS |
		     LINEAR_FRAMEBUFFER;
 
    /* sync */
    infoPtr->Sync = SiS2Sync;


    /* solid fills */
    infoPtr->SolidFillFlags = NO_PLANEMASK;
    infoPtr->SetupForSolidFill = SiS2SetupForFillRectSolid;
    infoPtr->SubsequentSolidFillRect = SiS2SubsequentFillRectSolid;

    
    /* screen to screen copy */
    infoPtr->ScreenToScreenCopyFlags = NO_TRANSPARENCY | NO_PLANEMASK;
    infoPtr->SetupForScreenToScreenCopy = 	
				SiS2SetupForScreenToScreenCopy;
    infoPtr->SubsequentScreenToScreenCopy = 		
				SiS2SubsequentScreenToScreenCopy;


    /* 8x8 mono patterns */
    infoPtr->Mono8x8PatternFillFlags =  NO_PLANEMASK | 
                HARDWARE_PATTERN_SCREEN_ORIGIN | 
		HARDWARE_PATTERN_PROGRAMMED_BITS |
                HARDWARE_PATTERN_PROGRAMMED_ORIGIN |
                NO_TRANSPARENCY | /* transp does not work right now */ 
		BIT_ORDER_IN_BYTE_MSBFIRST;

    infoPtr->SetupForMono8x8PatternFill =
				SiS2SetupForMono8x8PatternFill;
    infoPtr->SubsequentMono8x8PatternFillRect = 
				SiS2SubsequentMono8x8PatternFillRect;

    /* screen to screen color expansion */
#if 0
    /* don't know how to specify the with, height of the
       source bitmap for the replication along Width, Height of destination area
       */
    infoPtr->ScreenToScreenColorExpandFillFlags = 
                 BIT_ORDER_IN_BYTE_MSBFIRST |
                 SCANLINE_PAD_DWORD |
	         /*GXCOPY_ONLY | be careful not fully tested */
	         TRANSPARENCY_ONLY | /* opaque does not work right now */
		 NO_PLANEMASK;

    infoPtr->SetupForScreenToScreenColorExpandFill = 
                        SiS2SetupForScreenToScreenColorExpandFill;
    infoPtr->SubsequentScreenToScreenColorExpandFill = 
                        SiS2SubsequentScreenToScreenColorExpandFill;
#endif

    if (pSiS->TurboQueue) offset = 32768;
    if (pSiS->HWCursor) offset = 16384;
    if (pSiS->HWCursor && pSiS->TurboQueue) offset = 65536;

    /* CPU To screen color expansion indirect method */
    OffscreenAvailable = pSiS->FbMapSize - pScrn->displayWidth * pScrn->virtualY
                                                   * (pScrn->bitsPerPixel / 8) ;
    OffscreenAvailable -= offset;
    BLTPatternOffscreenSize = 2 * (((pScrn->virtualX + 31)/32) * 4);
    if (OffscreenAvailable < BLTPatternOffscreenSize) {
      xf86DrvMsgVerb(pScrn->scrnIndex, X_WARNING, 0,
		     "Not enough off-screen video memory for expand color.\n");
    }
    else {
      infoPtr->ScanlineCPUToScreenColorExpandFillFlags = 
	                 SYNC_AFTER_COLOR_EXPAND | 
                         BIT_ORDER_IN_BYTE_MSBFIRST |
			 SCANLINE_PAD_DWORD |
	                 /*GXCOPY_ONLY | not fully tested */
	                 TRANSPARENCY_ONLY | /* opaque doesn't work*/
			 NO_PLANEMASK;

      pSiS->XAAScanlineColorExpandBuffers[0] =
	pSiS->FbBase + pSiS->FbMapSize - offset - 
	(((pScrn->virtualX + 31)/32) * 4);
      pSiS->XAAScanlineColorExpandBuffers[1] =
	pSiS->FbBase + pSiS->FbMapSize - offset - 
	(((pScrn->virtualX + 31)/32) * 4)*2;

      infoPtr->NumScanlineColorExpandBuffers = 2;
      infoPtr->ScanlineColorExpandBuffers = 
                         pSiS->XAAScanlineColorExpandBuffers;
      offset += BLTPatternOffscreenSize;

      infoPtr->SetupForScanlineCPUToScreenColorExpandFill =
	    SiS2SetupForScreenToScreenColorExpandFill;
      infoPtr->SubsequentScanlineCPUToScreenColorExpandFill =
	    SiS2SubsequentScanlineCPUToScreenColorExpandFill;
      infoPtr->SubsequentColorExpandScanline =
	    SiS2SubsequentColorExpandScanline;
    }

    AvailFBArea.x1 = 0;
    AvailFBArea.y1 = 0;
    AvailFBArea.x2 = pScrn->displayWidth;
    AvailFBArea.y2 = (pSiS->FbMapSize - offset) / (pScrn->displayWidth *
					    pScrn->bitsPerPixel / 8);

    xf86InitFBManager(pScreen, &AvailFBArea);

    return(XAAInit(pScreen, infoPtr));
}

static void 
SiS2Sync(ScrnInfoPtr pScrn) {
    SISPtr pSiS = SISPTR(pScrn);
    sisBLTSync;
}

static int sisALUConv[] =
{
    0x00,	/* dest = 0; 		0,	GXclear, 	0 */
    0x88,	/* dest &= src; 	DSa,	GXand,		0x1 */
    0x44,	/* dest = src & ~dest; 	SDna,	GXandReverse, 	0x2 */
    0xCC,	/* dest = src; 		S,	GXcopy, 	0x3 */
    0x22,	/* dest &= ~src; 	DSna,	GXandInverted, 	0x4 */
    0xAA,	/* dest = dest; 	D,	GXnoop, 	0x5 */
    0x66,	/* dest = ^src; 	DSx,	GXxor, 		0x6 */
    0xEE,	/* dest |= src; 	DSo,	GXor, 		0x7 */
    0x11,	/* dest = ~src & ~dest;	DSon,	GXnor, 		0x8 */
    0x99,	/* dest ^= ~src ;	DSxn,	GXequiv, 	0x9 */
    0x55,	/* dest = ~dest; 	Dn,	GXInvert, 	0xA */
    0xDD,	/* dest = src|~dest ;	SDno,	GXorReverse, 	0xB */
    0x33,	/* dest = ~src; 	Sn,	GXcopyInverted, 0xC */
    0xBB,	/* dest |= ~src; 	DSno,	GXorInverted, 	0xD */
    0x77,	/* dest = ~src|~dest;	DSan,	GXnand, 	0xE */
    0xFF,	/* dest = 0xFF; 	1,	GXset, 		0xF */
};
/* same ROP but with Pattern as Source */
static int sisPatALUConv[] =
{
    0x00,	/* dest = 0; 		0,	GXclear, 	0 */
    0xA0,	/* dest &= src; 	DPa,	GXand,		0x1 */
    0x50,	/* dest = src & ~dest; 	PDna,	GXandReverse, 	0x2 */
    0xF0,	/* dest = src; 		P,	GXcopy, 	0x3 */
    0x0A,	/* dest &= ~src; 	DPna,	GXandInverted, 	0x4 */
    0xAA,	/* dest = dest; 	D,	GXnoop, 	0x5 */
    0x5A,	/* dest = ^src; 	DPx,	GXxor, 		0x6 */
    0xFA,	/* dest |= src; 	DPo,	GXor, 		0x7 */
    0x05,	/* dest = ~src & ~dest;	DPon,	GXnor, 		0x8 */
    0xA5,	/* dest ^= ~src ;	DPxn,	GXequiv, 	0x9 */
    0x55,	/* dest = ~dest; 	Dn,	GXInvert, 	0xA */
    0xF5,	/* dest = src|~dest ;	PDno,	GXorReverse, 	0xB */
    0x0F,	/* dest = ~src; 	Pn,	GXcopyInverted, 0xC */
    0xAF,	/* dest |= ~src; 	DPno,	GXorInverted, 	0xD */
    0x5F,	/* dest = ~src|~dest;	DPan,	GXnand, 	0xE */
    0xFF,	/* dest = 0xFF; 	1,	GXset, 		0xF */
};

static void 
SiS2SetupForFillRectSolid(ScrnInfoPtr pScrn, int color, int rop, 
			 unsigned int planemask)
{
    SISPtr pSiS = SISPTR(pScrn);

    PDEBUG(ErrorF("SiS2SetupForFillRectSolid()\n"));
    sisBLTWAIT;
    sisSETPATFGCOLOR(color);
    sisSETROP(sisPatALUConv[rop & 0xF]);
    sisSETPITCH(pScrn->displayWidth * pScrn->bitsPerPixel / 8, 
		pScrn->displayWidth * pScrn->bitsPerPixel / 8);
    sisSETDSTHEIGHT(-1);	/* disable merge clipping */

    sisSETSRCADDR(0);
    sisSETDSTADDR(0);
    sisSETSRCXSRCY(0,0);

}

static void 
SiS2SubsequentFillRectSolid(ScrnInfoPtr pScrn, int x, int y, int w, int h)
{
    SISPtr pSiS = SISPTR(pScrn);
    int op;

    op = sisCMDBLT | sisTOP2BOTTOM | sisLEFT2RIGHT | sisROP;

    sisSETHEIGHTWIDTH(h, w);
    sisSETDSTXDSTY(x,y);

    sisSETCMD(op);

}

static void 
SiS2SetupForScreenToScreenCopy(ScrnInfoPtr pScrn, int xdir, int ydir, 
				int rop, unsigned int planemask,
				int transparency_color)
{
    SISPtr pSiS = SISPTR(pScrn);

    PDEBUG(ErrorF("SiS2SetupForScreenToScreenCopy()\n"));
    sisBLTWAIT;
    sisSETPITCH(pScrn->displayWidth * pScrn->bitsPerPixel / 8, 
		pScrn->displayWidth * pScrn->bitsPerPixel / 8);
    sisSETROP(sisALUConv[rop & 0xF]);
    sisSETDSTHEIGHT(-1);	/* disable merge clipping */
    sisSETSRCADDR(0);
    sisSETDSTADDR(0);
    pSiS->Xdirection = xdir;
    pSiS->Ydirection = ydir;
}

static void 
SiS2SubsequentScreenToScreenCopy(ScrnInfoPtr pScrn, int x1, int y1, int x2, 
				int y2, int w, int h)
{
    SISPtr pSiS = SISPTR(pScrn);
    int op ;

    op = sisCMDBLT | sisSRCVIDEO | sisROP;
    if (pSiS->Ydirection == -1) {
	op |= sisBOTTOM2TOP;
        y1 += h-1;
	y2 += h-1;
    } else {
	op |= sisTOP2BOTTOM;
    }
    if (pSiS->Xdirection == -1) {
	op |= sisRIGHT2LEFT;
	x1 += w-1;
	x2 += w-1;
    } else {
	op |= sisLEFT2RIGHT;
    }

    sisSETHEIGHTWIDTH(h, w);
    sisSETSRCXSRCY(x1,y1);
    sisSETDSTXDSTY(x2,y2);

    sisSETCMD(op);
}

static void 
SiS2SetupForMono8x8PatternFill(ScrnInfoPtr pScrn, int patternx, int patterny, 
				int fg, int bg, int rop, unsigned int planemask)
{
    SISPtr pSiS = SISPTR(pScrn);
    unsigned int	*patternRegPtr ;
    int 	dstpitch;
    int 	isTransparent = ( bg == -1 );
    int 	op  = sisCMDCOLEXP | sisTOP2BOTTOM | sisLEFT2RIGHT | 
	              sisPATMASK;

    PDEBUG(ErrorF("SiS2SetupFor8x8PatternColorExpand()\n"));

    dstpitch = pScrn->displayWidth * pScrn->bitsPerPixel / 8 ;
    /*
     * check transparency 
     */
    /* becareful with rop */
    if (isTransparent) {
      op |= sisTRANSPARENT;
      PDEBUG(ErrorF("doesn't work right now: should never be called\n"));
    }
	
    sisBLTWAIT;
    sisSETPATBGCOLOR(bg);
    sisSETPATFGCOLOR(fg);
    sisSETROP(sisPatALUConv[rop & 0xF]);	/* pat copy */

    sisSETPITCH(1, dstpitch);
    sisSETSRCADDR(0);
    sisSETDSTADDR(0);
    sisSETSRCXSRCY(0,0);
    sisSETDSTHEIGHT(-1);	/* disable merge clipping */
    pSiS->CommandReg = op | sisROP ;

    patternRegPtr =  (unsigned int *)sisSETPATMASKREG();
    patternRegPtr[0] = patternx ;
    patternRegPtr[1] = patterny ;
}

static void 
SiS2SubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn, int patternx, 
				int patterny, int x, int y, int w, int h)
{
    SISPtr pSiS = SISPTR(pScrn);
    /* 
     *    what do I need to do with patternx, patterny ?
     */

    sisSETDSTXDSTY(x,y);
    sisSETHEIGHTWIDTH(h, w);

    sisSETCMD(pSiS->CommandReg);
}

static void 
SiS2SetupForScreenToScreenColorExpandFill(ScrnInfoPtr pScrn,
				int fg, int bg, int rop, 
				unsigned int planemask)
{
    SISPtr pSiS = SISPTR(pScrn);

    int isTransparent = ( bg == -1 );
    int	op ;

    PDEBUG(ErrorF("SiS2SetupScreenToScreenColorExpand()\n"));

    op  = sisCMDENHCOLEXP | sisTOP2BOTTOM | sisLEFT2RIGHT;

    if (isTransparent) {
      op |= sisTRANSPARENT;
    } 
	
    sisBLTWAIT;
    sisSETPATBGCOLOR(bg);
    sisSETPATFGCOLOR(fg);
    /* becareful with rop */
    sisSETROP(sisPatALUConv[rop & 0xF]);	

    sisSETDSTADDR(0);
    sisSETSRCADDR(0);
    sisSETSRCXSRCY(0,0);
    sisSETDSTHEIGHT(-1);	/* disable merge clipping */

    pSiS->CommandReg = op | sisROP;

}
#if 0
static void 
SiS2SubsequentScreenToScreenColorExpandFill(ScrnInfoPtr pScrn,
				int x, int y, int w, int h,
				int srcx, int srcy, int skipleft)
{
    SISPtr pSiS = SISPTR(pScrn);

    int pitch = pScrn->displayWidth * pScrn->bitsPerPixel / 8;

    sisBLTWAIT;
    sisSETPITCH(pitch, pitch);
    sisSETHEIGHTWIDTH(h, w);
    sisSETSRCXSRCY(srcx, srcy);
    sisSETDSTXDSTY(x, y);
    sisSETCMD(pSiS->CommandReg);
}
#endif
void
SiS2SubsequentScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
    int x, int y, int w, int h, int skipleft)
{
    SISPtr pSiS = SISPTR(pScrn);
    int pitch = pScrn->displayWidth * pScrn->bitsPerPixel / 8;
    int srcpitch =  ((w + 31)& ~31) /8 ;

    PDEBUG(ErrorF("SiS2SubsequentScanlineCPUToScreenColorExpandFill\n"));

    sisBLTWAIT;
    sisSETDSTXDSTY(x,y);
    sisSETHEIGHTWIDTH(1, w);
    sisSETPITCH(srcpitch, pitch);    

    pSiS->DstX = x;
    pSiS->DstY = y;

}

void
SiS2SubsequentColorExpandScanline(ScrnInfoPtr pScrn,
    int bufno)
{
    SISPtr pSiS = SISPTR(pScrn);
    int srcaddr;

    PDEBUG(ErrorF("SiS2SubsequentColorExpandScanline\n"));

    srcaddr = pSiS->XAAScanlineColorExpandBuffers[bufno] - pSiS->FbBase;

    sisBLTWAIT;
    sisSETSRCADDR(srcaddr);
    sisSETDSTXDSTY(pSiS->DstX, pSiS->DstY);
    sisSETCMD(pSiS->CommandReg);
    sisBLTWAIT;
    pSiS->DstY++;
}










