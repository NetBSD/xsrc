
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
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/sis/sis_accel2.c,v 1.1.2.2 1999/04/24 08:22:26 hohndel Exp $ */

#if 0
#define PDEBUG(arg)  arg 
#else
#define PDEBUG(arg) 
#endif

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
void SISNAME(SetupForScreenToScreenCopy)();
void SISNAME(SubsequentScreenToScreenCopy)();

void SISNAME(SetupForScreenToScreenColorExpand)();
void SISNAME(SubsequentScreenToScreenColorExpand)();
void SISNAME(SetupForScanlineScreenToScreenColorExpand)();
void SISNAME(SubsequentScanlineScreenToScreenColorExpand)();

void SISNAME(SetupFor8x8PatternColorExpand)();
void SISNAME(Subsequent8x8PatternColorExpand)();


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
       


    xf86AccelInfoRec.Flags =  BACKGROUND_OPERATIONS | 
	  (sisUseXAAcolorExp ? PIXMAP_CACHE : 0 ) |
	    HARDWARE_PATTERN_PROGRAMMED_BITS | 
                HARDWARE_PATTERN_PROGRAMMED_ORIGIN | 
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
    xf86GCInfoRec.PolyFillRectSolidFlags = NO_PLANEMASK ;

    /*
     * Install the low-level functions for drawing solid filled rectangles.
     */
    xf86AccelInfoRec.SetupForFillRectSolid = SISNAME(SetupForFillRectSolid);
    xf86AccelInfoRec.SubsequentFillRectSolid = SISNAME(SubsequentFillRectSolid);

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

    sisSETPATFGCOLOR(color);
    sisSETROP(sisPatALUConv[rop & 0xF]);
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

