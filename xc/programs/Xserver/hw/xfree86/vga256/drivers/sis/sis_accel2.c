
/*
 *
 *	Acceleration for SiS530 SiS620 SiS300.
 *	It is done in a separate file because the register formats are 
 *      very different from the previous chips.
 *
 *
 *	
 *	Xavier Ducoin <x.ducoin@lectra.com>
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/sis/sis_accel2.c,v 1.1.2.6 1999/12/21 07:43:43 hohndel Exp $ */

#if 0
#define PDEBUG(arg)  arg 
#else
#define PDEBUG(arg) 
#endif

#define ENABLE_LINE		1
#define FORCE_CLIPPING		0
#define ENABLE_MULTISLINE	0
#define ENABLE_TRAPEZOID	0

#include "vga256.h"
#include "xf86.h"
#include "vga.h"
#define XCONFIG_FLAGS_ONLY
#include "xf86_Config.h"

#include "xf86xaa.h"

#include "sis_driver.h"

#if (defined(__STDC__) && !defined(UNIXCPP)) || defined(ANSICPP)
#define CATNAME(prefix,subname) prefix##subname
#else
#define CATNAME(prefix,subname) prefix/**/subname
#endif

#include "sis_Blitter2.h"
#define SISNAME(subname) CATNAME(SIS2,subname)


extern Bool sisUseXAAcolorExp ;

void SISNAME(Sync)();
void SISNAME(SetupForFillRectSolid)();
void SISNAME(SubsequentFillRectSolid)();

#if ENABLE_TRAPEZOID
void SISNAME(SubsequentFillTrapezoidSolid)();
#endif

void SISNAME(SetupForScreenToScreenCopy)();
void SISNAME(SubsequentScreenToScreenCopy)();

void SISNAME(SetupForScreenToScreenColorExpand)();
void SISNAME(SubsequentScreenToScreenColorExpand)();
void SISNAME(SetupForScanlineScreenToScreenColorExpand)();
void SISNAME(SubsequentScanlineScreenToScreenColorExpand)();

void SISNAME(SetupFor8x8PatternColorExpand)();
void SISNAME(Subsequent8x8PatternColorExpand)();

#if ENABLE_LINE
void SISNAME(SetClippingRectangle)();
void SISNAME(SubsequentTwoPointLine)();
void SISNAME(SetupForDashedLine)();
void SISNAME(SubsequentDashedTwoPointLine)();

unsigned int sisLinePatternBuffer;
int sisLinePatternMaxLength = 32;
int sisPatternSize = 0;
unsigned int sisPatternMask;
int sisClippingSet = 0;
unsigned int sisLineop;
#endif

#if ENABLE_MULTISLINE
extern void xf86FillSpans();
void SISNAME(FillSpansSolid)();
#endif

/*
 * The following function sets up the supported acceleration. Call it
 * from the FbInit() function in the SVGA driver, or before ScreenInit
 * in a monolithic server.
 */
void SISNAME(AccelInit)()
{
    int cacheStart, cacheEnd;
    int sisCursorSize = sisHWCursor ? 16384 : 0 ; 
    int sisTurboQueueSize = sisTurboQueue ? 32768 : 0; 
    int offscreen_available ;
    int sisBLTPatternAddress ;
    int sisBLTPatternOffscreenSize ;

    /* sisTurboQueueSize is 32k, but it's aligned on a 32k boundary */
    if (sisHWCursor && sisTurboQueue) {sisCursorSize = 32768;}

    /*
     * If you want to disable acceleration, just don't modify anything
     * in the AccelInfoRec.
     */

    /*
     * Set up the main acceleration flags.
     * Usually, you will want to use BACKGROUND_OPERATIONS,
     * and if you have ScreenToScreenCopy, use the PIXMAP_CACHE.
     *
     * If the chip is restricted in the screen-to-screen BitBLT
     * directions it supports, you can indicate that here:
     *
     * ONLY_TWO_BITBLT_DIRECTIONS indicates that xdir must be equal to ydir.
     * ONLY_LEFT_TO_RIGHT_BITBLT indicates that the xdir must be 1.
     */

    /* Disable the PIXMAP CACHE in no linear because XAA high level does not
     * work with video in banked mode.
     * May be in the future we could restore the PIXMAP CACHE even in banked
     * mode
     */
       

    /* Yeou: SiS 530/620 and SiS 300 2D engine do not support 24bpp */ 	

    if (((SISchipset == SIS530) || (SISfamily == SIS300)) && 
       (vga256InfoRec.bitsPerPixel == 24)) return;

    xf86AccelInfoRec.Flags =  BACKGROUND_OPERATIONS | 
	  (sisUseXAAcolorExp ? PIXMAP_CACHE : 0 ) |
	    HARDWARE_PATTERN_PROGRAMMED_BITS | 
                HARDWARE_PATTERN_PROGRAMMED_ORIGIN | 
#if ENABLE_LINE
		TWO_POINT_LINE_NOT_LAST |
		HARDWARE_CLIP_LINE |
		LINE_PATTERN_MSBFIRST_MSBJUSTIFIED |
#endif
		    HARDWARE_PATTERN_BIT_ORDER_MSBFIRST 
               /* | HARDWARE_PATTERN_MONO_TRANSPARENCY // does not work right now */ ;

    /*
     * The following line installs a "Sync" function, that waits for
     * all coprocessor operations to complete.
     */
    xf86AccelInfoRec.Sync = SISNAME(Sync);

    /*
     * We want to set up the FillRectSolid primitive for filling a solid
     * rectangle. First we set up the flags for the graphics operation.
     * It may include GXCOPY_ONLY, NO_PLANEMASK, and RGB_EQUAL.
     */
    xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK;

    /*
     * Install the low-level functions for drawing solid filled rectangles.
     */
    xf86AccelInfoRec.SetupForFillRectSolid = SISNAME(SetupForFillRectSolid);
    xf86AccelInfoRec.SubsequentFillRectSolid = SISNAME(SubsequentFillRectSolid);

#if ENABLE_TRAPEZOID
    xf86AccelInfoRec.ErrorTermBits = 21;
    xf86AccelInfoRec.SubsequentFillTrapezoidSolid
	= SISNAME(SubsequentFillTrapezoidSolid);
#endif

#if ENABLE_LINE
    xf86AccelInfoRec.LinePatternBuffer = &sisLinePatternBuffer;
    xf86AccelInfoRec.LinePatternMaxLength = sisLinePatternMaxLength;
    xf86AccelInfoRec.SetClippingRectangle = SISNAME(SetClippingRectangle);
    xf86AccelInfoRec.SetupForDashedLine = SISNAME(SetupForDashedLine);
    xf86AccelInfoRec.SubsequentDashedTwoPointLine =
	SISNAME(SubsequentDashedTwoPointLine);
    xf86AccelInfoRec.SubsequentTwoPointLine = SISNAME(SubsequentTwoPointLine);
#endif

#if ENABLE_MULTISLINE
    xf86GCInfoRec.FillSpansSolidFlags = NO_PLANEMASK;
    xf86GCInfoRec.FillSpansSolid = xf86FillSpans;
    xf86AccelInfoRec.FillSpansSolid = SISNAME(FillSpansSolid);
#endif

    /*
     * We also want to set up the ScreenToScreenCopy (BitBLT) primitive for
     * copying a rectangular area from one location on the screen to
     * another. First we set up the restrictions. In this case, we
     * don't handle transparency color compare. Other allowed flags are
     * GXCOPY_ONLY and NO_PLANEMASK.
     */
    xf86GCInfoRec.CopyAreaFlags = NO_TRANSPARENCY | NO_PLANEMASK;
    
    /*
     * Install the low-level functions for screen-to-screen copy.
     */
    xf86AccelInfoRec.SetupForScreenToScreenCopy =
        SISNAME(SetupForScreenToScreenCopy);
    xf86AccelInfoRec.SubsequentScreenToScreenCopy =
        SISNAME(SubsequentScreenToScreenCopy);

    /* Color Expansion */
    if (vga256InfoRec.bitsPerPixel != 24) {
	/* the enhanced color expansion is not supported
	 * by the engine in 16M-color graphic mode.
	 */
	xf86AccelInfoRec.ColorExpandFlags = VIDEO_SOURCE_GRANULARITY_DWORD |
	                BIT_ORDER_IN_BYTE_MSBFIRST |
			SCANLINE_PAD_DWORD |
	                /*GXCOPY_ONLY | not fully tested */
	                ONLY_TRANSPARENCY_SUPPORTED | /* opaque doesn't work right now */
			NO_PLANEMASK;
	if ( sisUseXAAcolorExp ) {
#if 0	
	  /* don't know how to test it.
	     ScanlineScreenToScreenColorExpand always called
	     */
	    xf86AccelInfoRec.SetupForScreenToScreenColorExpand = 
		SISNAME(SetupForScreenToScreenColorExpand);
	    xf86AccelInfoRec.SubsequentScreenToScreenColorExpand = 
		SISNAME(SubsequentScreenToScreenColorExpand);
#endif
	    xf86AccelInfoRec.SetupForScanlineScreenToScreenColorExpand =
		SISNAME(SetupForScanlineScreenToScreenColorExpand);
	    xf86AccelInfoRec.SubsequentScanlineScreenToScreenColorExpand =
		SISNAME(SubsequentScanlineScreenToScreenColorExpand);
		
	    offscreen_available = vga256InfoRec.videoRam * 1024 - 
		vga256InfoRec.displayWidth * vga256InfoRec.virtualY
		    * (vgaBitsPerPixel / 8) ;
		    
	    offscreen_available = offscreen_available - sisCursorSize - sisTurboQueueSize;
	    sisBLTPatternOffscreenSize = 1024 ;
	
	    if (offscreen_available < sisBLTPatternOffscreenSize) {
		ErrorF("%s %s: Not enough off-screen video"
		       " memory for expand color.\n",
		       XCONFIG_PROBED, vga256InfoRec.name);
		sisBLTPatternOffscreenSize = 0 ;
		PDEBUG(ErrorF("offscreen_available: %d\n", offscreen_available ));
	    }
	    else {
		sisBLTPatternAddress = vga256InfoRec.videoRam * 1024 
		    - sisCursorSize - sisTurboQueueSize - sisBLTPatternOffscreenSize;
		xf86AccelInfoRec.ScratchBufferAddr=sisBLTPatternAddress;
		xf86AccelInfoRec.ScratchBufferSize=sisBLTPatternOffscreenSize;
		PDEBUG(ErrorF("sisBLTPatternAddress at 0x0%x\n", sisBLTPatternAddress ));
	    }
	}	
	/*
	 * 8x8 color expand pattern fill
	 */
	xf86AccelInfoRec.SetupFor8x8PatternColorExpand =
	    SISNAME(SetupFor8x8PatternColorExpand);
	xf86AccelInfoRec.Subsequent8x8PatternColorExpand =
	    SISNAME(Subsequent8x8PatternColorExpand);
    }
     /*
     * Finally, we set up the video memory space available to the pixmap
     * cache. In this case, all memory from the end of the virtual screen
     * to the end of video memory minus 1K, can be used. If you haven't
     * enabled the PIXMAP_CACHE flag, then these lines can be omitted.
     */
    if (sisUseXAAcolorExp) {
	cacheStart =
	    vga256InfoRec.virtualY * vga256InfoRec.displayWidth
		* vga256InfoRec.bitsPerPixel / 8;
	cacheEnd =
	    vga256InfoRec.videoRam * 1024 - 1024 - sisBLTPatternOffscreenSize -
		sisCursorSize - sisTurboQueueSize; 

	xf86InitPixmapCache(&vga256InfoRec, cacheStart, cacheEnd);
    PDEBUG(ErrorF("virtualY = %d \ndisplayWidth %d \nbitsPerPixel %d \n",
		  "Visible framebuffer size: %dK\n",  
		  vga256InfoRec.displayWidth,
		  vga256InfoRec.virtualY,
		  vga256InfoRec.bitsPerPixel,
		  cacheStart/1024));
    PDEBUG(ErrorF("Pixmap cache from top - %dK to top - %dK. Size %dK\n",   
		  vga256InfoRec.videoRam-(cacheStart/1024),
		  vga256InfoRec.videoRam-(cacheEnd/1024),
		  (cacheEnd-cacheStart)/1024));
    }
    /* 
     * Now set variables often used
     *
     */
    
}

/*
 * This is the implementation of the Sync() function.
 */
void SISNAME(Sync)() {

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


static int sisROP=0;
void SISNAME(SetupForFillRectSolid)(color, rop, planemask)
    int color, rop;
    unsigned planemask;
{

    PDEBUG(ErrorF("SetupForFillRectSolid(%d, %d, %d)\n",color,rop,planemask));

    sisSETPATFGCOLOR(color);
    sisSETROP(sisPatALUConv[rop & 0xF]);
    sisSETAGPBASE();
    sisSETPITCH(vga256InfoRec.displayWidth * vgaBytesPerPixel, 
		vga256InfoRec.displayWidth * vgaBytesPerPixel);
    sisSETDSTHEIGHT(-1);	/* disable merge clipping */

    sisSETSRCADDR(0);
    sisSETDSTADDR(0);
    sisSETSRCXSRCY(0,0);

}

void SISNAME(SubsequentFillRectSolid)(x, y, w, h)
    int x, y, w, h;
{
    int op;

    PDEBUG(ErrorF("SubsequentFillRectSolid(%d, %d, %d, %d)\n", x, y, w, h));

    op = sisCMDBLT | sisTOP2BOTTOM | sisLEFT2RIGHT | sisROP;

    sisSETHEIGHTWIDTH(h, w);
    sisSETDSTXDSTY(x,y);

    sisSETCMD(op);
}

static int blitxdir, blitydir;
 
void SISNAME(SetupForScreenToScreenCopy)(xdir, ydir, rop, planemask,
transparency_color)
    int xdir, ydir;
    int rop;
    unsigned planemask;
    int transparency_color;
{
    PDEBUG(ErrorF("SetupForScreenToScreenCopy()\n"));

    sisSETAGPBASE();
    sisSETPITCH(vga256InfoRec.displayWidth * vgaBytesPerPixel, 
		vga256InfoRec.displayWidth * vgaBytesPerPixel);
    sisSETROP(sisALUConv[rop & 0xF]);
    sisSETDSTHEIGHT(-1);	/* disable merge clipping */

    sisSETSRCADDR(0);
    sisSETDSTADDR(0);
    blitxdir = xdir;
    blitydir = ydir;
}

void SISNAME(SubsequentScreenToScreenCopy)(x1, y1, x2, y2, w, h)
    int x1, y1, x2, y2, w, h;
{
    int op ;

    PDEBUG(ErrorF("SubsequentScreenToScreenCopy()\n"));
    /*
     * If the direction is "decreasing", the chip wants the addresses
     * to be at the other end, so we must be aware of that in our
     * calculations.
     */
    op = sisCMDBLT | sisSRCVIDEO | sisROP;
    if (blitydir == -1) {
	op |= sisBOTTOM2TOP;
        y1 += h-1;
	y2 += h-1;
    } else {
	op |= sisTOP2BOTTOM;
    }
    if (blitxdir == -1) {
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

static int sisColExp_op ;
void SISNAME(SetupForScreenToScreenColorExpand)(bg, fg, rop, planemask)
    int bg, fg, rop;
    unsigned planemask;
{
    int isTransparent = ( bg == -1 );
    int	op ;

    PDEBUG(ErrorF("SISSetupScreenToScreenColorExpand()\n"));

    op  = sisCMDENHCOLEXP | sisTOP2BOTTOM | sisLEFT2RIGHT;

    if (isTransparent) {
      op |= sisTRANSPARENT;
    } 
	
    sisSETPATBGCOLOR(bg);
    sisSETPATFGCOLOR(fg);
    /* becareful with rop */
    sisSETROP(sisPatALUConv[rop & 0xF]);	

    sisSETDSTADDR(0);
    sisSETSRCADDR(0);
    sisSETDSTHEIGHT(-1);	/* disable merge clipping */

    sisColExp_op = op | sisROP;
}

void SISNAME(SubsequentScreenToScreenColorExpand)(srcx, srcy, x, y, w, h)
    int srcx, srcy, x, y, w, h;
{
    int destpitch = vga256InfoRec.displayWidth * vgaBytesPerPixel ;
    int srcpitch ;

    PDEBUG(ErrorF("SISSubsequentScreenToScreenColorExpand()\n"));
    srcpitch =  ((w + 31)& ~31) /8 ;
    sisSETAGPBASE();
    sisSETPITCH(srcpitch, destpitch);
    sisSETHEIGHTWIDTH(h, w);
    sisSETSRCXSRCY(srcx, srcy);
    sisSETDSTXDSTY(x, y);
    sisSETCMD(sisColExp_op);

}

static int sisDstY, sisDstX;
void SISNAME(SetupForScanlineScreenToScreenColorExpand)(x, y, w, h, bg, fg, rop,
planemask)
    int x, y, w, h, bg, fg, rop;
    unsigned int planemask;
{
    int isTransparent = ( bg == -1 );
    int	op ;
    int pitch = vga256InfoRec.displayWidth * vgaBytesPerPixel ;
    int	srcpitch;

    PDEBUG(ErrorF("SISSetupForScanlineScreenToScreenColorExpand()\n"));

    op  = sisTOP2BOTTOM | sisLEFT2RIGHT | sisCMDENHCOLEXP;

    if (isTransparent) {
      op |= sisTRANSPARENT;
    } 
	
    sisSETPATBGCOLOR(bg);
    sisSETPATFGCOLOR(fg);
    /* becareful with rop */
    sisSETROP(sisPatALUConv[rop & 0xF]);	

    sisColExp_op = op | sisROP;

    sisSETDSTADDR(0);
    sisSETSRCADDR(0);
    sisSETSRCXSRCY(0,0);
    sisSETDSTXDSTY(x,y);
    sisSETDSTHEIGHT(-1);	/* disable merge clipping */
    sisSETHEIGHTWIDTH(1, w);
    srcpitch =  ((w + 31)& ~31) /8 ;
    sisSETAGPBASE();
    sisSETPITCH(srcpitch, pitch);    

    sisDstX = x;
    sisDstY = y;
}

void SISNAME(SubsequentScanlineScreenToScreenColorExpand)(srcaddr)
    int srcaddr;
{
    PDEBUG(ErrorF("SISSubsequentScanlineScreenToScreenColorExpand()\n"));
    sisSETSRCADDR(srcaddr/8);
    sisSETDSTXDSTY(sisDstX, sisDstY);
    sisSETCMD(sisColExp_op);
    sisDstY++;
}

void SISNAME(SetupFor8x8PatternColorExpand)(patternx, patterny, bg, fg,
                                            rop, planemask)
    unsigned patternx, patterny, planemask;
    int bg, fg, rop;
{
    unsigned int	*patternRegPtr ;
    int	       	i ;
    int 	dstpitch;
    int 	isTransparent = ( bg == -1 );
    int 	op  = sisCMDCOLEXP | sisTOP2BOTTOM | sisLEFT2RIGHT | 
	              sisPATMASK;
    
    PDEBUG(ErrorF("SetupFor8x8PatternColorExpand()\n"));

    dstpitch = vga256InfoRec.displayWidth * vgaBytesPerPixel ;

    if (isTransparent) {
      op |= sisTRANSPARENT;
    } 
	
    sisSETPATBGCOLOR(bg);
    sisSETPATFGCOLOR(fg);
    /* becareful with rop */
    sisSETROP(sisPatALUConv[rop & 0xF]);

    sisSETAGPBASE();
    sisSETPITCH(1, dstpitch);
    sisSETSRCADDR(0);
    sisSETDSTADDR(0);
    sisSETSRCXSRCY(0,0);
    sisSETDSTHEIGHT(-1);	/* disable merge clipping */
    sisColExp_op = op | sisROP ;

    patternRegPtr =  (unsigned int *)sisSETPATMASKREG();
    patternRegPtr[0] = patternx ;
    patternRegPtr[1] = patterny ;
}

void SISNAME(Subsequent8x8PatternColorExpand)(patternx, patterny, x, y, w, h)
    unsigned patternx, patterny;
    int x, y, w, h;
{
  /* 
   * what do I need to do with patternx, patterny ?
   */
    PDEBUG(ErrorF("SISSubsequent8x8PatternColorExpand(%d %d %d %d %d %d)\n",
	   patternx, patterny, x, y, w, h));
    
    sisSETDSTXDSTY(x,y);
    sisSETHEIGHTWIDTH(h, w);

    sisSETCMD(sisColExp_op);
}

#if ENABLE_TRAPEZOID
void SISNAME(SubsequentFillTrapezoidSolid)(
	int ytop,
	int height,
	int left,
	int dxL, int dyL,
	int eL,
	int right,
	int dxR, int dyR,
	int eR)
{
	int op,bL,bR, t;

	/**/
	if (dxL >= 0)
		bL = left - 1;
	else {
		t = height * dxL / dyL;
		if (t < 0)
			bL = left + t - 1;
		else
			bL = left - t - 1;
	}

	if (dxR <= 0)
		bR = right + 1;
	else {
		t = height * dxR / dyR;
		if (t < 0)
			bR = right - t + 1;
		else
			bR = right + t + 1;
	}
        sisSETCLIPTOP(bL, ytop-1);
        sisSETCLIPBOTTOM(bR, ytop+height);
	/**/
#if 0
	/***/
	if ((dxL > 0) && (dxR < 0)) {
		int l, r;
		l = left + abs(height * dxL / dyL);
		r = right - abs(height * dxR / dyR);
		if (l > r) {
			dxR = (l - right)*dyR/height;
		}
		if (dxR > dyR)
			eR = dyR - dxR;
		else
			eR = dxR - dyR;
	}
	/***/
#endif
	sisSETTRAPEZOIDHY(height,ytop);
	sisSETTRAPEZOIDX(left,right+1);

	op = sisCMDTRAPEZOIDFILL | sisROP | sisCLIPENABL;

	if (dyR >= 0)
		op |= sisRIGHTYINC;
	else
		dyR = -dyR;

	if (dxR >= 0)
		op |= sisRIGHTXINC;
	else
		dxR = -dxR;

	if (dxR >= dyR) {
		op |= sisRIGHTXMAJOR;
		eR = (dxR - eR)*dyR/dxR + dyR;
		while (eR >= 0)
			eR -= dxR;
			
	} 

	sisSETRIGHTDELTA(dxR,dyR);

	if (dyL >= 0)
		op |= sisLEFTYINC;
	else
		dyL = -dyL;

	if (dxL >= 0)
		op |= sisLEFTXINC;
	else
		dxL = -dxL;


	if (dxL >= dyL) {
		op |= sisLEFTXMAJOR;
		eL = (dxL - eL)*dyL/dxL + dyL;
		while (eL >= 0)
			eL -= dxL;
	}

	sisSETLEFTDELTA(dxL, dyL);

	sisSETLEFTERR(eL);
	sisSETRIGHTERR(eR);

	sisSETCMD(op);
}
#endif

#if ENABLE_LINE

void SISNAME(SubsequentTwoPointLine)(x1, y1, x2, y2, bias)
    int x1, y1, x2, y2, bias;
{
	int op;

	sisSETSRCXSRCY(y1, x1);
	sisSETDSTXDSTY(y2, x2);
	sisSETLINEPERIODCOUNT(0, 1);
	sisSETLINESTYLE1(0x80000000);

	op = sisCMDLINE | sisLINESTYLEENABLE | sisROP;

	if (bias & 0x100)
		op |= sisLASTPIXELNOTDRAW;
	if (sisClippingSet)
		op |= sisCLIPENABL;
	sisSETCMD(op);
	sisClippingSet = 0;
}

void SISNAME(SetupForDashedLine)(
	int fg,
        int bg,
        int rop,
        unsigned planemask,
        int size)
{
	int 	isTransparent = ( bg == -1 );
	int 	op,i;

	sisSETAGPBASE();
	sisSETDSTADDR(0);
	sisSETPITCH(vga256InfoRec.displayWidth * vgaBytesPerPixel, 
		vga256InfoRec.displayWidth * vgaBytesPerPixel);
	sisSETDSTHEIGHT(-1);	/* disable merge clipping */
	sisSETLINEPERIODCOUNT(size-1, 1);
	sisPatternSize = size;
	sisPatternMask = 0;
	for (i=0;i<size;i++) {
		sisPatternMask >>= 1;
		sisPatternMask |= 0x80000000;
	}
	sisSETPATFGCOLOR(fg);
	sisSETPATBGCOLOR(bg);

	sisLineop = sisCMDLINE | sisLINESTYLEENABLE | sisPatALUConv[rop & 0xF];
	if (isTransparent) {
		sisLineop |= sisTRANSPARENT;
	} 
}

void SISNAME(SubsequentDashedTwoPointLine)(
        int x1,
        int y1,
        int x2,
        int y2,
        int bias,
        int offset)
{
	unsigned int op;
	unsigned int pat;

	sisSETSRCXSRCY(y1, x1);
	sisSETDSTXDSTY(y2, x2);
	pat = (sisLinePatternBuffer << offset) 
		| (sisLinePatternBuffer >> (sisPatternSize - offset));
	pat &= sisPatternMask;
	sisSETLINESTYLE1(pat);

	op = sisLineop;
	if (bias & 0x100)
		op |= sisLASTPIXELNOTDRAW; /* last pixel does NOT drawn */
#if FORCE_CLIPPING
	if (!sisClippingSet) {
		int xb1,yb1,xb2,yb2;
		if (x1 > x2) {
			xb1 = x2 -1;
			xb2 = x1;
		} else {
			xb1 = x1 -1;
			xb2 = x2;
		}

		if (y1 > y2) {
			yb1 = y2 -1;
			yb2 = y1;
		} else {
			yb1 = y1 -1;
			yb2 = y2;
		}
		SISNAME(SetClippingRectangle)(xb1, yb1, xb2, yb2);
	}
#endif
	if (sisClippingSet)
		op |= sisCLIPENABL;
	sisSETCMD(op);
	sisClippingSet = 0;
}

void SISNAME(SetClippingRectangle)(
	int x1,
	int y1,
	int x2,
	int y2)
{

	sisSETCLIPTOP(x1, y1);
	sisSETCLIPBOTTOM(x2+1, y2+1);
	sisClippingSet = 1;
}

#endif

#if ENABLE_MULTISLINE
void SISNAME(FillSpansSolid)(
        int             nInit,
        DDXPointPtr     pptInit,
        int             *pwidthInit,
        int             fSorted,
        int             fg,
        int             rop)
{
	int y,next_y,line2base,maxline,i,total,p,n,op;

	if (!nInit)
		return;

	if (!fSorted) {
		SISNAME(SetupForFillRectSolid)(fg, rop, 0);
		for (i=0;i<nInit;i++) {
			SISNAME(SubsequentFillRectSolid)(
				pptInit[i].x,pptInit[i].y,pwidthInit[i],1);
		}
	}

	switch (xf86AccelInfoRec.BitsPerPixel) {
	case 8:  maxline = 48; line2base = 0x8340;
		break;
	case 16: maxline = 32; line2base = 0x8380;
		break;
	case 32:
	default:
		 maxline = 32; line2base = 0x8400;
		break;
	}

	/* setup */
	sisSETAGPBASE();
	sisSETDSTADDR(0);
	sisSETPITCH(0,
		vga256InfoRec.displayWidth * vgaBytesPerPixel);
	sisSETDSTHEIGHT(-1);	/* disable merge clipping */
	sisSETPATFGCOLOR(fg);
	op = sisCMDMULTISLINE | sisTOP2BOTTOM | sisPatALUConv[rop & 0xF];
	total = nInit;
	p = 0;
	n = 0;
	next_y = y = pptInit[p].y;
	while (total) {
		if (n == 0)
			sisSETMULTISLINE0(pptInit[p].x,
				pptInit[p].x + pwidthInit[p]);
		else if (n == 1)
			sisSETMULTISLINE1(pptInit[p].x,
				pptInit[p].x + pwidthInit[p]);
		else
			sisSETMULTISLINEN(line2base,n,pptInit[p].x,
				pptInit[p].x + pwidthInit[p]);
		total--;p++;next_y++;n++;
		if (!total || (n >= maxline)) {
			sisSETLINECNT(y,n);
			sisSETCMD(op);
			next_y = y = pptInit[p].y;
			n = 0;
			continue;
		}

		if (pptInit[p].y != next_y) {
			sisSETLINECNT(y,n);
			sisSETCMD(op);
			next_y = y = pptInit[p].y;
			n = 0;
		}
	}
	if (xf86AccelInfoRec.Flags & BACKGROUND_OPERATIONS)
		xf86AccelInfoRec.Sync();
}
#endif
