/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/mga/mga_storm.c,v 1.1.2.17 1999/11/18 15:37:31 hohndel Exp $ */

#include "compiler.h"
#include "vga256.h"
#include "xf86.h"
#include "vga.h"
#include "vgaPCI.h"

#include "miline.h"

#include "xf86xaa.h"
#include "xf86local.h"

#include "mga.h"
#include "mga_reg.h"
#include "mga_map.h"
#include "mga_macros.h"

extern CARD32 MGAAtype[16];
extern CARD32 MGAAtypeNoBLK[16];

extern void MGAWriteBitmap();
extern void MGAFillRectStippled();
extern Bool MGAIsClipped;
extern Bool MGAUsePCIRetry;
extern Bool MGAUseBLKOpaqueExpansion;
extern Bool MGAIsMillennium2;
extern Bool MGATranscSolidFill;
extern int  MGAMaxFastBlitY;

static void MGANAME(SetupForScreenToScreenCopy)();
static void MGANAME(SubsequentScreenToScreenCopy)();
static void MGANAME(SubsequentScreenToScreenCopy_FastBlit)();
static void MGANAME(SubsequentScreenToScreenCopy_FastBlit_Broken)();
static void MGANAME(SetupForFillRectSolid)();
static void MGANAME(SubsequentFillRectSolid)();
static void MGANAME(SubsequentFillTrapezoidSolid)();
static void MGANAME(SubsequentBresenhamLine)();
static void MGANAME(SetupForCPUToScreenColorExpand)();
static void MGANAME(SubsequentCPUToScreenColorExpand)();
static void MGANAME(SetupForScreenToScreenColorExpand)();
static void MGANAME(SubsequentScreenToScreenColorExpand)();
static void MGANAME(SetupForDashedLine)();
static void MGANAME(SubsequentDashedBresenhamLine)();
static void MGANAME(SetupFor8x8PatternColorExpand)();
static void MGANAME(Subsequent8x8PatternColorExpand)();
static void MGANAME(Subsequent8x8PatternColorExpand_Additional)();

static CARD32 MGADashPattern[4];

void MGANAME(AccelInit)() 
{
    if(MGA_IS_2164(MGAchipset))
	MGAIsMillennium2 = TRUE;

    if(OFLG_ISSET(OPTION_PCI_RETRY, &vga256InfoRec.options))
    	MGAUsePCIRetry = TRUE; 

    MGAUseBLKOpaqueExpansion = (MGAchipset != PCI_CHIP_MGA1064 &&
    				!MGA_IS_GCLASS(MGAchipset));

    MGATranscSolidFill =       (MGA_IS_G200(MGAchipset) ||
				MGA_IS_2164(MGAchipset) ||
				MGA_IS_G400(MGAchipset));

    xf86AccelInfoRec.Flags = 	BACKGROUND_OPERATIONS | 
				COP_FRAMEBUFFER_CONCURRENCY |
                             	DO_NOT_BLIT_STIPPLES |  
                            	LINE_PATTERN_MSBFIRST_LSBJUSTIFIED |
				PIXMAP_CACHE | 
				HARDWARE_PATTERN_PROGRAMMED_BITS |
                             	HARDWARE_PATTERN_PROGRAMMED_ORIGIN |
                             	HARDWARE_PATTERN_SCREEN_ORIGIN |
                             	HARDWARE_PATTERN_BIT_ORDER_MSBFIRST |
                             	HARDWARE_PATTERN_MONO_TRANSPARENCY;

    /* sync */
    xf86AccelInfoRec.Sync = MGAStormSync;

    /* screen to screen copy */
    xf86GCInfoRec.CopyAreaFlags = NO_TRANSPARENCY;
    xf86AccelInfoRec.SetupForScreenToScreenCopy =
        			MGANAME(SetupForScreenToScreenCopy);
    xf86AccelInfoRec.SubsequentScreenToScreenCopy =
        			MGANAME(SubsequentScreenToScreenCopy);

    /* solid filled rectangles */
    xf86AccelInfoRec.SetupForFillRectSolid = 
				MGANAME(SetupForFillRectSolid);
    xf86AccelInfoRec.SubsequentFillRectSolid =  
				MGANAME(SubsequentFillRectSolid);

    /* solid trapezoids */
    xf86AccelInfoRec.SubsequentFillTrapezoidSolid =
    				MGANAME(SubsequentFillTrapezoidSolid);

    /* solid bresenham lines */
    xf86AccelInfoRec.SubsequentBresenhamLine = 
				MGANAME(SubsequentBresenhamLine);
    xf86AccelInfoRec.ErrorTermBits = 15;

    /* cpu to screen color expansion */
    xf86AccelInfoRec.ColorExpandFlags = SCANLINE_PAD_DWORD |
                                        CPU_TRANSFER_PAD_DWORD |
                                        BIT_ORDER_IN_BYTE_LSBFIRST |
					LEFT_EDGE_CLIPPING |
					LEFT_EDGE_CLIPPING_NEGATIVE_X; 
#ifdef __alpha__
    xf86AccelInfoRec.CPUToScreenColorExpandBase = (unsigned*)MGAMMIOBaseDENSE;
#else
    xf86AccelInfoRec.CPUToScreenColorExpandBase = (unsigned*)MGAMMIOBase;
#endif /* __alpha__ */
    xf86AccelInfoRec.CPUToScreenColorExpandRange = 0x1C00;

    xf86AccelInfoRec.SetupForCPUToScreenColorExpand = 
				MGANAME(SetupForCPUToScreenColorExpand);
    xf86AccelInfoRec.SubsequentCPUToScreenColorExpand = 
				MGANAME(SubsequentCPUToScreenColorExpand);

    xf86AccelInfoRec.SetupForScreenToScreenColorExpand =
    			MGANAME(SetupForScreenToScreenColorExpand);
    xf86AccelInfoRec.SubsequentScreenToScreenColorExpand =
    			MGANAME(SubsequentScreenToScreenColorExpand);

    /* dashed bresenham lines */
    xf86AccelInfoRec.SetupForDashedLine = MGANAME(SetupForDashedLine);
    xf86AccelInfoRec.SubsequentDashedBresenhamLine = 
				MGANAME(SubsequentDashedBresenhamLine);
    xf86AccelInfoRec.LinePatternBuffer = (void *)MGADashPattern;
    xf86AccelInfoRec.LinePatternMaxLength = 128;


    /* 8x8 pattern fills */
    xf86AccelInfoRec.SetupFor8x8PatternColorExpand =
    			MGANAME(SetupFor8x8PatternColorExpand);
    xf86AccelInfoRec.Subsequent8x8PatternColorExpand =
    			MGANAME(Subsequent8x8PatternColorExpand);
 

    /* replacements */
    xf86AccelInfoRec.WriteBitmap = MGAWriteBitmap;
    xf86AccelInfoRec.FillRectStippled = MGAFillRectStippled;
    xf86AccelInfoRec.FillRectOpaqueStippled = MGAFillRectStippled; 

    if (MGA_IS_G100(MGAchipset)) {
        /* the G100 gets unhappy if we use planemasks... */
	xf86GCInfoRec.CopyAreaFlags |= NO_PLANEMASK;
	xf86GCInfoRec.FillPolygonSolidFlags |= NO_PLANEMASK;

	xf86GCInfoRec.PolyRectangleSolidZeroWidthFlags |= NO_PLANEMASK;
	xf86GCInfoRec.PolyLineSolidZeroWidthFlags |= NO_PLANEMASK;
	xf86GCInfoRec.PolySegmentSolidZeroWidthFlags |= NO_PLANEMASK;
	xf86GCInfoRec.PolyLineDashedZeroWidthFlags |= NO_PLANEMASK;
	xf86GCInfoRec.PolySegmentDashedZeroWidthFlags |= NO_PLANEMASK;

	xf86GCInfoRec.PolyGlyphBltNonTEFlags |= NO_PLANEMASK;
	xf86GCInfoRec.ImageGlyphBltNonTEFlags |= NO_PLANEMASK;
	xf86GCInfoRec.PolyGlyphBltTEFlags |= NO_PLANEMASK;
	xf86GCInfoRec.ImageGlyphBltTEFlags |= NO_PLANEMASK;

	xf86GCInfoRec.FillSpansSolidFlags |= NO_PLANEMASK;
	xf86GCInfoRec.FillSpansTiledFlags |= NO_PLANEMASK;
	xf86GCInfoRec.FillSpansStippledFlags |= NO_PLANEMASK;
	xf86GCInfoRec.FillSpansOpaqueStippledFlags |= NO_PLANEMASK;

	xf86GCInfoRec.SecondaryPolyFillRectOpaqueStippledFlags |= NO_PLANEMASK;
	xf86GCInfoRec.SecondaryPolyFillRectStippledFlags |= NO_PLANEMASK;

	xf86GCInfoRec.PolyFillRectSolidFlags |= NO_PLANEMASK;
	xf86GCInfoRec.PolyFillRectTiledFlags |= NO_PLANEMASK;
	xf86GCInfoRec.PolyFillRectStippledFlags |= NO_PLANEMASK;
	xf86GCInfoRec.PolyFillRectOpaqueStippledFlags |= NO_PLANEMASK;

	xf86GCInfoRec.PolyFillArcSolidFlags |= NO_PLANEMASK;

	xf86AccelInfoRec.ColorExpandFlags |= NO_PLANEMASK;
    }

#if PSZ == 24
    /* Shotgun approach.  XAA should do some of these on it's own
	but it isn't getting all of them */
    xf86GCInfoRec.CopyAreaFlags |= NO_PLANEMASK;
    xf86GCInfoRec.FillPolygonSolidFlags |= NO_PLANEMASK;

    xf86GCInfoRec.PolyRectangleSolidZeroWidthFlags |= NO_PLANEMASK;
    xf86GCInfoRec.PolyLineSolidZeroWidthFlags |= NO_PLANEMASK;
    xf86GCInfoRec.PolySegmentSolidZeroWidthFlags |= NO_PLANEMASK;
    xf86GCInfoRec.PolyLineDashedZeroWidthFlags |= NO_PLANEMASK;
    xf86GCInfoRec.PolySegmentDashedZeroWidthFlags |= NO_PLANEMASK;

    xf86GCInfoRec.PolyGlyphBltNonTEFlags |= NO_PLANEMASK;
    xf86GCInfoRec.ImageGlyphBltNonTEFlags |= NO_PLANEMASK;
    xf86GCInfoRec.PolyGlyphBltTEFlags |= NO_PLANEMASK;
    xf86GCInfoRec.ImageGlyphBltTEFlags |= NO_PLANEMASK;

    xf86GCInfoRec.FillSpansSolidFlags |= NO_PLANEMASK;
    xf86GCInfoRec.FillSpansTiledFlags |= NO_PLANEMASK;
    xf86GCInfoRec.FillSpansStippledFlags |= NO_PLANEMASK;
    xf86GCInfoRec.FillSpansOpaqueStippledFlags |= NO_PLANEMASK;

    xf86GCInfoRec.SecondaryPolyFillRectOpaqueStippledFlags |= NO_PLANEMASK;
    xf86GCInfoRec.SecondaryPolyFillRectStippledFlags |= NO_PLANEMASK;

    xf86GCInfoRec.PolyFillRectSolidFlags |= NO_PLANEMASK;
    xf86GCInfoRec.PolyFillRectTiledFlags |= NO_PLANEMASK;
    xf86GCInfoRec.PolyFillRectStippledFlags |= NO_PLANEMASK;
    xf86GCInfoRec.PolyFillRectOpaqueStippledFlags |= NO_PLANEMASK;

    xf86GCInfoRec.PolyFillArcSolidFlags |= NO_PLANEMASK;

    xf86AccelInfoRec.ColorExpandFlags |= NO_PLANEMASK;
#endif

    /* pixmap cache */
    {
	int cacheStart, cacheEnd, maxFastBlitMem;

	maxFastBlitMem = (MGAinterleave ? 4096 : 2048) * 1024;
	cacheStart = (vga256InfoRec.virtualY * vga256InfoRec.displayWidth
                            + MGAydstorg) * PSZ / 8;

 	cacheEnd = cacheStart + (128 * vga256InfoRec.displayWidth * PSZ / 8);

	if(cacheEnd > (vga256InfoRec.videoRam * 1024))
	    cacheEnd = (vga256InfoRec.videoRam * 1024);

	/* Mystique only: Reserve 1K at top of memory for hardware cursor */
	if (OFLG_ISSET(OPTION_HW_CURSOR, &vga256InfoRec.options) &&
	 ((MGAchipset == PCI_CHIP_MGA1064) || (MGA_IS_GCLASS(MGAchipset))) &&
	    (cacheEnd > (vga256InfoRec.videoRam-1)*1024))
	{
	    cacheEnd = (vga256InfoRec.videoRam-1)*1024;
	}

	if(cacheEnd > maxFastBlitMem) {
	     MGAMaxFastBlitY = maxFastBlitMem / 
			(vga256InfoRec.displayWidth  * PSZ / 8);
    	}

   	xf86AccelInfoRec.PixmapCacheMemoryStart = cacheStart;  
    	xf86AccelInfoRec.PixmapCacheMemoryEnd = cacheEnd;
    }
}



#if PSZ == 8

/*
   GXclear, GXand, 
   GXandReverse, GXcopy,
   GXandInverted, GXnoop, 
   GXxor, GXor, 
   GXnor, GXequiv, 
   GXinvert, GXorReverse, 
   GXcopyInverted, GXorInverted, 
   GXnand, GXset
*/


CARD32 MGAAtype[16] = {
   MGADWG_RPL  | 0x00000000, MGADWG_RSTR | 0x00080000, 
   MGADWG_RSTR | 0x00040000, MGADWG_BLK  | 0x000c0000,
   MGADWG_RSTR | 0x00020000, MGADWG_RSTR | 0x000a0000, 
   MGADWG_RSTR | 0x00060000, MGADWG_RSTR | 0x000e0000,
   MGADWG_RSTR | 0x00010000, MGADWG_RSTR | 0x00090000, 
   MGADWG_RSTR | 0x00050000, MGADWG_RSTR | 0x000d0000,
   MGADWG_RPL  | 0x00030000, MGADWG_RSTR | 0x000b0000, 
   MGADWG_RSTR | 0x00070000, MGADWG_RPL  | 0x000f0000
};


CARD32 MGAAtypeNoBLK[16] = {
   MGADWG_RPL  | 0x00000000, MGADWG_RSTR | 0x00080000, 
   MGADWG_RSTR | 0x00040000, MGADWG_RPL  | 0x000c0000,
   MGADWG_RSTR | 0x00020000, MGADWG_RSTR | 0x000a0000, 
   MGADWG_RSTR | 0x00060000, MGADWG_RSTR | 0x000e0000,
   MGADWG_RSTR | 0x00010000, MGADWG_RSTR | 0x00090000, 
   MGADWG_RSTR | 0x00050000, MGADWG_RSTR | 0x000d0000,
   MGADWG_RPL  | 0x00030000, MGADWG_RSTR | 0x000b0000, 
   MGADWG_RSTR | 0x00070000, MGADWG_RPL  | 0x000f0000
};

Bool MGAIsClipped = FALSE;
Bool MGAUsePCIRetry = FALSE;
Bool MGAUseBLKOpaqueExpansion;
Bool MGAIsMillennium2 = FALSE;
int  MGAMaxFastBlitY = 0;
Bool MGATranscSolidFill = FALSE;
Bool MGAIsSDRAM = FALSE;

void 
MGAStormAccelInit() {
    /*
     * for SDRAM cards we cannot use block mode
     */
    if (MGAIsSDRAM)
    	MGAAtype[3] = MGAAtypeNoBLK[3];

    switch( vgaBitsPerPixel )
    {
    case 8:
    	Mga8AccelInit();
    	break;
    case 16:
    	Mga16AccelInit();
    	break;
    case 24:
    	Mga24AccelInit();
    	break;
    case 32:
    	Mga32AccelInit();
    	break;
    }
}

void 
MGAStormSync()
{
    if(MGAIsClipped) { 
	/* we don't need to sync after a cpu->screen color expand.
		we merely need to reset the clipping box */
	WAITFIFO(1);
    	OUTREG(MGAREG_CXBNDRY, 0xFFFF0000);
	MGAIsClipped = FALSE;
    } else while(MGAISBUSY());

    /* flush cache before a read (mga-1064g 5.1.6) */
    OUTREG8(MGAREG_CRTC_INDEX, 0); 
}

void 
MGAStormEngineInit()
{
    CARD32 maccess = 0;
    				
    if(MGA_IS_G100(MGAchipset))
    	maccess = 1 << 14;	/* enable JEDEC */
    
    switch( vgaBitsPerPixel )
    {
    case 8:
        break;
    case 16:
	/* set 16 bpp, turn off dithering, turn on 5:5:5 pixels */
        maccess |= 1 + (1 << 30) + (1 << 31);
        break;
    case 24:
        maccess |= 3;
        break;
    case 32:
        maccess |= 2;
        break;
    }

    WAITFIFO(8);
    OUTREG(MGAREG_PITCH, vga256InfoRec.displayWidth);
    OUTREG(MGAREG_YDSTORG, MGAydstorg);
    OUTREG(MGAREG_MACCESS, maccess);
    if( !(MGA_IS_G100(MGAchipset) && MGAIsSDRAM) )
	    OUTREG(MGAREG_PLNWT, ~0);
    OUTREG(MGAREG_OPMODE, MGAOPM_DMA_BLIT);

    /* put clipping in a know state */
    OUTREG(MGAREG_CXBNDRY, 0xFFFF0000);	/* (maxX << 16) | minX */ 
    OUTREG(MGAREG_YTOP, 0x00000000);	/* minPixelPointer */ 
    OUTREG(MGAREG_YBOT, 0x007FFFFF);	/* maxPixelPointer */ 

    if(MGA_IS_G200(MGAchipset) || MGA_IS_G400(MGAchipset)) {
	/* set the blit origins */
	WAITFIFO(2);
	OUTREG(0x2cb4, 0);
	OUTREG(0x2cb8, 0);
    }
}


#endif /* PSZ == 8 */




	/*********************************************\
	|            Screen-to-Screen Copy            |
	\*********************************************/

#define BLIT_LEFT	1
#define BLIT_UP		4

static CARD32 BltScanDirection;

void 
MGANAME(SetupForScreenToScreenCopy)(xdir, ydir, rop, planemask, trans_color)
    int xdir, ydir;
    int rop;
    unsigned planemask;
    int trans_color;
{
    xf86AccelInfoRec.SubsequentScreenToScreenCopy = 
		MGANAME(SubsequentScreenToScreenCopy);
    REPLICATE(planemask);
    BltScanDirection = 0;
    if(ydir == -1) BltScanDirection |= BLIT_UP;
    if(xdir == -1) BltScanDirection |= BLIT_LEFT;

    WAITFIFO(4);
    if(xdir == -1) {
    	OUTREG(MGAREG_DWGCTL, MGAAtypeNoBLK[rop] | MGADWG_SHIFTZERO | 
			MGADWG_BITBLT | MGADWG_BFCOL);
    } else {
	if(MGAusefbitblt && (rop == GXcopy)) {
	    if(MGAIsMillennium2)
	      xf86AccelInfoRec.SubsequentScreenToScreenCopy = 
		MGANAME(SubsequentScreenToScreenCopy_FastBlit);
	    else 
	      xf86AccelInfoRec.SubsequentScreenToScreenCopy = 
		MGANAME(SubsequentScreenToScreenCopy_FastBlit_Broken);
	}
    	OUTREG(MGAREG_DWGCTL, MGAAtypeNoBLK[rop] | MGADWG_SHIFTZERO | 
			MGADWG_BITBLT | MGADWG_BFCOL);
    }
    OUTREG(MGAREG_SGN, BltScanDirection);
#if PSZ != 24
    if(! MGA_IS_G100(MGAchipset))
	    OUTREG(MGAREG_PLNWT, planemask);
#endif
    OUTREG(MGAREG_AR5, ydir * xf86AccelInfoRec.FramebufferWidth);
}


void 
MGANAME(SubsequentScreenToScreenCopy)(srcX, srcY, dstX, dstY, w, h)
    int srcX, srcY, dstX, dstY, w, h;
{
    register int start, end;
 
    if(BltScanDirection & BLIT_UP) {
	srcY += h - 1;
	dstY += h - 1;
    }

    w--;
    start = end = XYADDRESS(srcX, srcY);

    if(BltScanDirection & BLIT_LEFT) start += w;
    else end += w; 
   
    WAITFIFO(4);
    OUTREG(MGAREG_AR0, end);
    OUTREG(MGAREG_AR3, start);
    OUTREG(MGAREG_FXBNDRY, ((dstX + w) << 16) | dstX);
    OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (dstY << 16) | h);
}

void 
MGANAME(SubsequentScreenToScreenCopy_FastBlit)(srcX, srcY, dstX, dstY, w, h)
    int srcX, srcY, dstX, dstY, w, h;
{
    register int start, end;

    if(BltScanDirection & BLIT_UP) {
        srcY += h - 1;
        dstY += h - 1;
    }

    w--;
    start = XYADDRESS(srcX, srcY);
    end = start + w;

    /* we assume the driver asserts screen pitches such that
	we can always use fastblit for scrolling */
    if(
#if PSZ == 32
        !((srcX ^ dstX) & 31)
#elif PSZ == 16
        !((srcX ^ dstX) & 63)
#else
        !((srcX ^ dstX) & 127)
#endif
    	) {
	if(MGAMaxFastBlitY) {
	   if(((srcY + h) > MGAMaxFastBlitY) ||
				((dstY + h) > MGAMaxFastBlitY)) 
	   goto FASTBLIT_BAILOUT;
	}
	
    	WAITFIFO(6);
    	OUTREG(MGAREG_DWGCTL, 0x040A400C);
    	OUTREG(MGAREG_AR0, end);
    	OUTREG(MGAREG_AR3, start);
    	OUTREG(MGAREG_FXBNDRY, ((dstX + w) << 16) | dstX);
    	OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (dstY << 16) | h);
    	OUTREG(MGAREG_DWGCTL, MGAAtypeNoBLK[GXcopy] | MGADWG_SHIFTZERO | 
			MGADWG_BITBLT | MGADWG_BFCOL);
	return;
    }  

FASTBLIT_BAILOUT:
   
    WAITFIFO(4);
    OUTREG(MGAREG_AR0, end);
    OUTREG(MGAREG_AR3, start);
    OUTREG(MGAREG_FXBNDRY, ((dstX + w) << 16) | dstX);
    OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (dstY << 16) | h);
}


/*  Workaround for fastblit bug in Millennium 1 */

void 
MGANAME(SubsequentScreenToScreenCopy_FastBlit_Broken)(srcX, srcY, dstX, dstY, w, h)
    int srcX, srcY, dstX, dstY, w, h;
{
    register int start, end;

    if(BltScanDirection & BLIT_UP) {
        srcY += h - 1;
        dstY += h - 1;
    }

    w--;
    start = XYADDRESS(srcX, srcY);
    end = start + w;

    /* we assume the driver asserts screen pitches such that
	we can always use fastblit for scrolling */
    if(
#if PSZ == 32
        !((srcX ^ dstX) & 31)
#elif PSZ == 16
        !((srcX ^ dstX) & 63)
#else
        !((srcX ^ dstX) & 127)
#endif
    	) {
    	int cxright, fxright = dstX + w;

	if(MGAMaxFastBlitY) {
	   if(((srcY + h) > MGAMaxFastBlitY) ||
				((dstY + h) > MGAMaxFastBlitY)) 
	   goto FASTBLIT_BAILOUT_BROKEN;
	}
	
#if PSZ == 8
        if( (dstX & (1 << 6)) && (((fxright >> 6) - (dstX >> 6)) & 7) == 7 ) {
            cxright = fxright, fxright |= 1 << 6;
#elif PSZ == 16
        if( (dstX & (1 << 5)) && (((fxright >> 5) - (dstX >> 5)) & 7) == 7 ) {
            cxright = fxright, fxright |= 1 << 5;
#elif PSZ == 24
        if( ((dstX * 3) & (1 << 6)) && 
                 ((((fxright * 3 + 2) >> 6) - ((dstX * 3) >> 6)) & 7) == 7 ) {
            cxright = fxright, fxright = ((fxright * 3 + 2) | (1 << 6)) / 3;
#elif PSZ == 32
        if( (dstX & (1 << 4)) && (((fxright >> 4) - (dstX >> 4)) & 7) == 7 ) {
            cxright = fxright, fxright |= 1 << 4;
#endif
	
    	    WAITFIFO(8);
            OUTREG(MGAREG_CXRIGHT, cxright);
    	    OUTREG(MGAREG_DWGCTL, 0x040A400C);
    	    OUTREG(MGAREG_AR0, end);
    	    OUTREG(MGAREG_AR3, start);
    	    OUTREG(MGAREG_FXBNDRY, (fxright << 16) | dstX);
    	    OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (dstY << 16) | h);
    	    OUTREG(MGAREG_DWGCTL, MGAAtypeNoBLK[GXcopy] | MGADWG_SHIFTZERO | 
			MGADWG_BITBLT | MGADWG_BFCOL);
            OUTREG(MGAREG_CXRIGHT, 0xFFFF);
	} else {
     	    WAITFIFO(6);
    	    OUTREG(MGAREG_DWGCTL, 0x040A400C);
    	    OUTREG(MGAREG_AR0, end);
    	    OUTREG(MGAREG_AR3, start);
    	    OUTREG(MGAREG_FXBNDRY, ((dstX + w) << 16) | dstX);
    	    OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (dstY << 16) | h);
    	    OUTREG(MGAREG_DWGCTL, MGAAtypeNoBLK[GXcopy] | MGADWG_SHIFTZERO | 
			MGADWG_BITBLT | MGADWG_BFCOL);
	}
	return;
    }  

FASTBLIT_BAILOUT_BROKEN:
   
    WAITFIFO(4);
    OUTREG(MGAREG_AR0, end);
    OUTREG(MGAREG_AR3, start);
    OUTREG(MGAREG_FXBNDRY, ((dstX + w) << 16) | dstX);
    OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (dstY << 16) | h);
}







	/*********************************************\
	|            Solid Filled Rectangles          |
	\*********************************************/

static CARD32 FilledRectCMD;
static CARD32 SolidLineCMD;

void 
MGANAME(SetupForFillRectSolid)(color, rop, planemask)
    int color, rop;
    unsigned planemask;
{
#if PSZ == 24
    if(!RGBEQUAL(color))
    FilledRectCMD = MGADWG_TRAP | MGADWG_SOLID | MGADWG_ARZERO | 
		    MGADWG_SGNZERO | MGADWG_SHIFTZERO | 
		    MGADWG_BMONOLEF | MGAAtypeNoBLK[rop];
    else
#endif
    FilledRectCMD = MGADWG_TRAP | MGADWG_SOLID | MGADWG_ARZERO | 
		    MGADWG_SGNZERO | MGADWG_SHIFTZERO | 
		    MGADWG_BMONOLEF | MGAAtype[rop];

    SolidLineCMD = MGADWG_LINE_OPEN | MGADWG_SOLID | MGADWG_SHIFTZERO | 
		   MGADWG_BFCOL | MGAAtypeNoBLK[rop];

    if(MGATranscSolidFill) FilledRectCMD |= MGADWG_TRANSC;

    REPLICATE24(color);
    REPLICATE(planemask);
    WAITFIFO(3);
    OUTREG(MGAREG_FCOL, color);
#if PSZ != 24
    if(! MGA_IS_G100(MGAchipset))
	    OUTREG(MGAREG_PLNWT, planemask);
#endif
    OUTREG(MGAREG_DWGCTL, FilledRectCMD);
}


void 
MGANAME(SubsequentFillRectSolid)(x, y, w, h)
    int x, y, w, h;
{
    WAITFIFO(2);
    OUTREG(MGAREG_FXBNDRY, ((x + w) << 16) | x);
    OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (y << 16) | h);
}

	/******************************\
	|       Solid Trapezoids       |
	\******************************/


void 
MGANAME(SubsequentFillTrapezoidSolid)(y, h, left, dxL, dyL, eL, 
						right, dxR, dyR, eR)
    int y, h, left, dxL, dyL, eL, right, dxR, dyR, eR;
{
    int sdxl = (dxL < 0);
    int ar2 = sdxl? dxL : -dxL;
    int sdxr = (dxR < 0);
    int ar5 = sdxr? dxR : -dxR;
    
    WAITFIFO(11);
    OUTREG(MGAREG_DWGCTL, FilledRectCMD & ~(MGADWG_ARZERO | MGADWG_SGNZERO));
    OUTREG(MGAREG_AR0, dyL);
    OUTREG(MGAREG_AR1, ar2 - eL);
    OUTREG(MGAREG_AR2, ar2);
    OUTREG(MGAREG_AR4, ar5 - eR);
    OUTREG(MGAREG_AR5, ar5);
    OUTREG(MGAREG_AR6, dyR);
    OUTREG(MGAREG_SGN, (sdxl << 1) | (sdxr << 5));
    OUTREG(MGAREG_FXBNDRY, ((right + 1) << 16) | left);
    OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (y << 16) | h);
    OUTREG(MGAREG_DWGCTL, FilledRectCMD);
}

	/********************************\
	|         Bresenham Lines        |
	\********************************/

static CARD32 MGAOctants[8] = { 1, 0, 5, 4, 3, 2, 7, 6 };

void
MGANAME(SubsequentBresenhamLine)(x1, y1, octant, err, e1, e2, length)
    int x1, y1, octant, err, e1, e2, length;
{
    if(MGAIsMillennium2 && !e1) {
    	WAITFIFO(2);
    	OUTREG(MGAREG_FXBNDRY, ((x1 + 1) << 16) | x1);
    	OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (y1 << 16) | length);
	return;
    }

    WAITFIFO(8);
    OUTREG(MGAREG_DWGCTL, SolidLineCMD);
    OUTREG(MGAREG_SGN, MGAOctants[octant & 7]);
    OUTREG(MGAREG_AR0, e1);
    OUTREG(MGAREG_AR1, err);
    OUTREG(MGAREG_AR2, e2);
    OUTREG(MGAREG_XDST, x1);
    OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (y1 << 16) | length);
    OUTREG(MGAREG_DWGCTL, FilledRectCMD);
}


	/********************************************\
	|        CPU to Screen Color Expansion       |
	\********************************************/

void 
MGANAME(SetupForCPUToScreenColorExpand)(bg, fg, rop, planemask)
    int bg, fg, rop;
    unsigned planemask;
{
    CARD32 mgaCMD = MGADWG_ILOAD | MGADWG_LINEAR | MGADWG_SGNZERO | 
			MGADWG_SHIFTZERO | MGADWG_BMONOLEF;
        
    REPLICATE24(fg);
    REPLICATE(planemask);

    if(bg == -1) {
#if PSZ == 24
    	if(!RGBEQUAL(fg))
            mgaCMD |= MGADWG_TRANSC | MGAAtypeNoBLK[rop];
	else
#endif
            mgaCMD |= MGADWG_TRANSC | MGAAtype[rop];

	WAITFIFO(3);
    } else {
#if PSZ == 24
	if(MGAUseBLKOpaqueExpansion && RGBEQUAL(fg) && RGBEQUAL(bg)) 
#else
	if(MGAUseBLKOpaqueExpansion) 
#endif
        	mgaCMD |= MGAAtype[rop];
	else
        	mgaCMD |= MGAAtypeNoBLK[rop];
        REPLICATE24(bg);
	WAITFIFO(4);
    	OUTREG(MGAREG_BCOL, bg);
    }

    OUTREG(MGAREG_FCOL, fg);
#if PSZ != 24
    if(! MGA_IS_G100(MGAchipset))
	    OUTREG(MGAREG_PLNWT, planemask);
#endif
    OUTREG(MGAREG_DWGCTL, mgaCMD);
}

void MGANAME(SubsequentCPUToScreenColorExpand)(x, y, w, h, skipleft)
    int x, y, w, h, skipleft;
{
    MGAIsClipped = TRUE;
    WAITFIFO(5);
    OUTREG(MGAREG_CXBNDRY, ((x + w - 1) << 16) | ((x + skipleft) & 0xFFFF));
    w = (w + 31) & ~31;     /* source is dword padded */
    OUTREG(MGAREG_AR0, (w * h) - 1);
    OUTREG(MGAREG_AR3, 0);  /* crashes occasionally without this */
    OUTREG(MGAREG_FXBNDRY, ((x + w - 1) << 16) | (x & 0xFFFF));
    OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (y << 16) | h);
}

	/***************************************\
	|   Screen to Screen Color Expansion	|
	\***************************************/

void 
MGANAME(SetupForScreenToScreenColorExpand)(bg, fg, rop, planemask)
    int bg, fg, rop;
    unsigned planemask;
{
    CARD32 mgaCMD = MGADWG_BITBLT | MGADWG_SGNZERO | MGADWG_SHIFTZERO;
        
    REPLICATE24(fg);
    REPLICATE(planemask);

    if(bg == -1) {
#if PSZ == 24
    	if(!RGBEQUAL(fg))
            mgaCMD |= MGADWG_TRANSC | MGAAtypeNoBLK[rop];
	else
#endif
            mgaCMD |= MGADWG_TRANSC | MGAAtype[rop];

	WAITFIFO(4);
    } else {
#if PSZ == 24
	if(MGAUseBLKOpaqueExpansion && RGBEQUAL(fg) && RGBEQUAL(bg)) 
#else
	if(MGAUseBLKOpaqueExpansion) 
#endif
        	mgaCMD |= MGAAtype[rop];
	else
        	mgaCMD |= MGAAtypeNoBLK[rop];
        REPLICATE24(bg);
	WAITFIFO(5);
    	OUTREG(MGAREG_BCOL, bg);
    }

    OUTREG(MGAREG_FCOL, fg);
#if PSZ != 24
    if(! MGA_IS_G100(MGAchipset))
	    OUTREG(MGAREG_PLNWT, planemask);
#endif
    OUTREG(MGAREG_AR5, xf86AccelInfoRec.FramebufferWidth * PSZ);
    OUTREG(MGAREG_DWGCTL, mgaCMD);
}

void MGANAME(SubsequentScreenToScreenColorExpand)(srcx, srcy, x, y, w, h)
    int srcx, srcy, x, y, w, h;
{
    register int start, end;

    start = (XYADDRESS(0, srcy) * PSZ) + srcx;
    end = start + w - 1;

    WAITFIFO(4);
    OUTREG(MGAREG_AR3, start);
    OUTREG(MGAREG_AR0, end);
    OUTREG(MGAREG_FXBNDRY, ((x + w - 1) << 16) | x);
    OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (y << 16) | h);
}


	/************************************\
	|      Dashed Bresenham Lines        |
	\************************************/

static CARD32 mgaStylelen;
static Bool NiceDashPattern;
static CARD32 NiceDashCMD;
static CARD32 DashCMD;

void
MGANAME(SetupForDashedLine)(fg, bg, rop, planemask, size)
  int fg, bg, rop;
  unsigned int planemask;
  int size;
{
    extern unsigned char byte_reversed[256];
    int dwords = (size + 31) >> 5;
    DashCMD = MGADWG_LINE_OPEN | MGADWG_BFCOL | MGAAtypeNoBLK[rop];

    mgaStylelen = size - 1;
    REPLICATE(fg);
    REPLICATE(planemask);

    /* We see if we can draw horizontal lines as 8x8 pattern fills.
	This is worthwhile since the pattern fills can use block mode
	and the default X pattern is 8 pixels long.  The forward pattern
	is the top scanline, the backwards pattern is the next one. */
    switch(size) {
	case 2:	MGADashPattern[0] |= MGADashPattern[0] << 2;
	case 4:	MGADashPattern[0] |= MGADashPattern[0] << 4;
	case 8:	MGADashPattern[0] &= 0xFF;
		MGADashPattern[0] |= byte_reversed[MGADashPattern[0]] << 16;
		MGADashPattern[0] |= MGADashPattern[0] << 8;
		NiceDashCMD = MGADWG_TRAP | MGADWG_ARZERO | MGADWG_SGNZERO | 
				 MGADWG_BMONOLEF;
     		NiceDashPattern = TRUE;
   		if(bg == -1) {
#if PSZ == 24
    		   if(!RGBEQUAL(fg))
            		NiceDashCMD |= MGADWG_TRANSC | MGAAtypeNoBLK[rop];
		   else
#endif
           		NiceDashCMD |= MGADWG_TRANSC | MGAAtype[rop];
    		} else {
#if PSZ == 24
		   if(MGAUseBLKOpaqueExpansion && RGBEQUAL(fg) && RGBEQUAL(bg)) 
#else
		   if(MGAUseBLKOpaqueExpansion) 
#endif
        		NiceDashCMD |= MGAAtype[rop];
		   else
        		NiceDashCMD |= MGAAtypeNoBLK[rop];
    		}
		break;
	default: NiceDashPattern = FALSE;
    }

    if(bg == -1) {
        DashCMD |= MGADWG_TRANSC;
	WAITFIFO(dwords + 3);
    } else {
        REPLICATE(bg);
	WAITFIFO(dwords + 4);
    	OUTREG(MGAREG_BCOL, bg);
    }

    OUTREG(MGAREG_DWGCTL, DashCMD);
#if PSZ != 24
    if(! MGA_IS_G100(MGAchipset))
	    OUTREG(MGAREG_PLNWT, planemask);
#endif
    OUTREG(MGAREG_FCOL, fg);

    switch (dwords) {
	case 4:  OUTREG(MGAREG_SRC3, MGADashPattern[3]);
	case 3:  OUTREG(MGAREG_SRC2, MGADashPattern[2]);
	case 2:	 OUTREG(MGAREG_SRC1, MGADashPattern[1]);
	default: OUTREG(MGAREG_SRC0, MGADashPattern[0]);
    }
}


void 
MGANAME(SubsequentDashedBresenhamLine)(x1, y1, octant, err, e1, e2, len ,start)
    int x1, y1, octant, err, e1, e2, len ,start;
{
    if(NiceDashPattern && !e1 && !(octant & YMAJOR)) {
    	WAITFIFO(5);
    	OUTREG(MGAREG_DWGCTL, NiceDashCMD);
	if(octant & XDECREASING) {
    	   OUTREG(MGAREG_SHIFT, ((-y1 & 0x07) << 4) | 
				((7 - start - x1) & 0x07)); 
   	   OUTREG(MGAREG_FXBNDRY, ((x1 + 1) << 16) | (x1 - len + 1));
    	} else {
    	   OUTREG(MGAREG_SHIFT, (((1 - y1) & 0x07) << 4) | 
				((start - x1) & 0x07)); 
     	   OUTREG(MGAREG_FXBNDRY, ((x1 + len) << 16) | x1);
	}	
    	OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (y1 << 16) | 1);
    	OUTREG(MGAREG_DWGCTL, DashCMD);
	return;
    }

    WAITFIFO(7);
    OUTREG(MGAREG_SHIFT, (mgaStylelen << 16 ) | (mgaStylelen - start));
    OUTREG(MGAREG_SGN, MGAOctants[octant & 7]);
    OUTREG(MGAREG_AR0, e1);
    OUTREG(MGAREG_AR1, err);
    OUTREG(MGAREG_AR2, e2);
    OUTREG(MGAREG_XDST, x1);
    OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (y1 << 16) | len);
}


   		/****************************\
		|    8x8 Rectangular Fill    |
		\****************************/

static CARD32 PatternRectCMD;

void 
MGANAME(SetupFor8x8PatternColorExpand)(patternx, patterny, bg, fg,
                                            rop, planemask)
    unsigned patternx, patterny, planemask;
    int bg, fg, rop;
{
    PatternRectCMD = MGADWG_TRAP | MGADWG_ARZERO | MGADWG_SGNZERO | 
						MGADWG_BMONOLEF;

    xf86AccelInfoRec.Subsequent8x8PatternColorExpand = 
		MGANAME(Subsequent8x8PatternColorExpand);

    REPLICATE24(fg);
    REPLICATE(planemask);
    
    if(bg == -1) {
#if PSZ == 24
    	if(!RGBEQUAL(fg))
            PatternRectCMD |= MGADWG_TRANSC | MGAAtypeNoBLK[rop];
	else
#endif
            PatternRectCMD |= MGADWG_TRANSC | MGAAtype[rop];

	WAITFIFO(5);
    } else {
#if PSZ == 24
	if(MGAUseBLKOpaqueExpansion && RGBEQUAL(fg) && RGBEQUAL(bg)) 
#else
	if(MGAUseBLKOpaqueExpansion) 
#endif
        	PatternRectCMD |= MGAAtype[rop];
	else
        	PatternRectCMD |= MGAAtypeNoBLK[rop];
        REPLICATE24(bg);
	WAITFIFO(6);
    	OUTREG(MGAREG_BCOL, bg);
    }

    OUTREG(MGAREG_FCOL, fg);
#if PSZ != 24
    if(! MGA_IS_G100(MGAchipset))
	    OUTREG(MGAREG_PLNWT, planemask);
#endif
    OUTREG(MGAREG_DWGCTL, PatternRectCMD);
    OUTREG(MGAREG_PAT0, patternx);
    OUTREG(MGAREG_PAT1, patterny);
}

void 
MGANAME(Subsequent8x8PatternColorExpand)(patternx, patterny, x, y, w, h)
    unsigned patternx, patterny;
    int x, y, w, h;
{
    WAITFIFO(3);
    OUTREG(MGAREG_SHIFT, (patterny << 4) | patternx);
    OUTREG(MGAREG_FXBNDRY, ((x + w) << 16) | x);
    OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (y << 16) | h);
    xf86AccelInfoRec.Subsequent8x8PatternColorExpand = 
		MGANAME(Subsequent8x8PatternColorExpand_Additional);
}

void 
MGANAME(Subsequent8x8PatternColorExpand_Additional)(patternx, patterny, x, y, w, h)
    unsigned patternx, patterny;
    int x, y, w, h;
{
    WAITFIFO(2);
    OUTREG(MGAREG_FXBNDRY, ((x + w) << 16) | x);
    OUTREG(MGAREG_YDSTLEN + MGAREG_EXEC, (y << 16) | h);
}






