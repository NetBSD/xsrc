/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/sis_accel.c,v 1.23 2002/01/10 19:05:43 eich Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"

#include "xf86PciInfo.h"
#include "xf86Pci.h"

#include "sis_regs.h"
#include "sis.h"
#include "xaarop.h"

Bool SiSAccelInit(ScreenPtr pScreen);
static void SiSSync(ScrnInfoPtr pScrn);
static void SiSSetupForFillRectSolid(ScrnInfoPtr pScrn, int color,
                int rop, unsigned int planemask);
static void SiSSubsequentFillRectSolid(ScrnInfoPtr pScrn, int x,
                int y, int w, int h);
static void SiSSetupForScreenToScreenCopy(ScrnInfoPtr pScrn,
                int xdir, int ydir, int rop, 
                unsigned int planemask, int transparency_color);
static void SiSSubsequentScreenToScreenCopy(ScrnInfoPtr pScrn,
                int x1, int y1, int x2,
                int y2, int w, int h);
static void SiSSetupForMono8x8PatternFill(ScrnInfoPtr pScrn, 
                int patternx, int patterny, int fg, int bg, 
                int rop, unsigned int planemask);
static void SiSSubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn, 
                int patternx, int patterny, int x, int y, 
                int w, int h);
#if 0
static void SiSSetupForScreenToScreenColorExpandFill (ScrnInfoPtr pScrn,
                int fg, int bg, 
                int rop, unsigned int planemask);
static void SiSSubsequentScreenToScreenColorExpandFill( ScrnInfoPtr pScrn,
                int x, int y, int w, int h,
                int srcx, int srcy, int offset );
#endif
static void SiSSetClippingRectangle ( ScrnInfoPtr pScrn,
                    int left, int top, int right, int bottom);
static void SiSDisableClipping (ScrnInfoPtr pScrn);
static void SiSSetupForSolidLine(ScrnInfoPtr pScrn, 
                int color, int rop, unsigned int planemask);
static void SiSSubsequentSolidTwoPointLine(ScrnInfoPtr pScrn,
        int x1, int y1, int x2, int y2, int flags);
static void SiSSubsequentSolidHorVertLine(ScrnInfoPtr pScrn,
        int x, int y, int len, int dir);


Bool 
SiSAccelInit(ScreenPtr pScreen)
{
    XAAInfoRecPtr infoPtr;
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    SISPtr pSiS = SISPTR(pScrn);
    BoxRec AvailFBArea;
    int offset, topFB;

    pSiS->AccelInfoPtr = infoPtr = XAACreateInfoRec();
    if (!infoPtr) 
        return FALSE;

    infoPtr->Flags = PIXMAP_CACHE |
             OFFSCREEN_PIXMAPS |
             LINEAR_FRAMEBUFFER;
 
    infoPtr->Sync = SiSSync;
    /* Clipping and lines only works on 5597 and 6326 
       for 1024, 2048, 4096 logical width */
    if  (pSiS->ValidWidth) { 
        infoPtr->SetClippingRectangle = SiSSetClippingRectangle;
        infoPtr->DisableClipping = SiSDisableClipping;
        infoPtr->ClippingFlags =  
                    HARDWARE_CLIP_SOLID_LINE | 
                    HARDWARE_CLIP_SCREEN_TO_SCREEN_COPY |
                    HARDWARE_CLIP_MONO_8x8_FILL |
                    HARDWARE_CLIP_SOLID_FILL  ;

    /* Solid Lines */               
    infoPtr->SolidLineFlags =   NO_PLANEMASK |
                    BIT_ORDER_IN_BYTE_MSBFIRST;

    infoPtr->SetupForSolidLine = SiSSetupForSolidLine;
    infoPtr->SubsequentSolidTwoPointLine = SiSSubsequentSolidTwoPointLine;
    infoPtr->SubsequentSolidHorVertLine = SiSSubsequentSolidHorVertLine;
    }

    infoPtr->SolidFillFlags = NO_PLANEMASK;
    infoPtr->SetupForSolidFill = SiSSetupForFillRectSolid;
    infoPtr->SubsequentSolidFillRect = SiSSubsequentFillRectSolid;
    
    infoPtr->ScreenToScreenCopyFlags = NO_TRANSPARENCY | NO_PLANEMASK;
    infoPtr->SetupForScreenToScreenCopy =   
                SiSSetupForScreenToScreenCopy;
    infoPtr->SubsequentScreenToScreenCopy =         
                SiSSubsequentScreenToScreenCopy;

    if (pScrn->bitsPerPixel != 24) {
        infoPtr->Mono8x8PatternFillFlags =
                    NO_PLANEMASK | 
                    HARDWARE_PATTERN_PROGRAMMED_BITS |
                    HARDWARE_PATTERN_PROGRAMMED_ORIGIN |
                    BIT_ORDER_IN_BYTE_MSBFIRST;
        infoPtr->SetupForMono8x8PatternFill =
                SiSSetupForMono8x8PatternFill;
        infoPtr->SubsequentMono8x8PatternFillRect = 
                SiSSubsequentMono8x8PatternFillRect;
    }

#if 0 /* Don't work until we implement skipleft */
    if (pScrn->bitsPerPixel != 24) {
        infoPtr->ScreenToScreenColorExpandFillFlags =  GXCOPY_ONLY | 
                    CPU_TRANSFER_PAD_DWORD |
                    SCANLINE_PAD_DWORD |
                    NO_PLANEMASK | 
                    HARDWARE_PATTERN_PROGRAMMED_BITS |
                    HARDWARE_PATTERN_PROGRAMMED_ORIGIN |
                    BIT_ORDER_IN_BYTE_MSBFIRST;

        infoPtr->SetupForScreenToScreenColorExpandFill =
                    SiSSetupForScreenToScreenColorExpandFill;
        infoPtr->SubsequentScreenToScreenColorExpandFill = 
                    SiSSubsequentScreenToScreenColorExpandFill;
    }
#endif

    AvailFBArea.x1 = 0;
    AvailFBArea.y1 = 0;
    AvailFBArea.x2 = pScrn->displayWidth;
    if (pSiS->HWCursor || pSiS->TurboQueue)
        offset = 262144;
    else 
        offset = 0;
    
    topFB = (pSiS->maxxfbmem >= (pSiS->FbMapSize - offset)) ?
	pSiS->maxxfbmem : pSiS->FbMapSize - offset;
    AvailFBArea.y2 = (topFB) / (pScrn->displayWidth *
                      pScrn->bitsPerPixel / 8);

    if (AvailFBArea.y2 < 0)
	AvailFBArea.y2 = 32767;

    xf86InitFBManager(pScreen, &AvailFBArea);

    return(XAAInit(pScreen, infoPtr));
}

static void 
SiSSync(ScrnInfoPtr pScrn) {
    SISPtr pSiS = SISPTR(pScrn);
    sisBLTSync;
}

static void 
SiSSetupForFillRectSolid(ScrnInfoPtr pScrn, int color, int rop, 
             unsigned int planemask)
{
    SISPtr pSiS = SISPTR(pScrn);

    sisBLTSync;
    sisSETFGCOLOR(color);
    sisSETBGCOLOR(color);
    sisSETROP(XAACopyROP[rop]);
    sisSETPITCH(pScrn->displayWidth * pScrn->bitsPerPixel / 8, 
            pScrn->displayWidth * pScrn->bitsPerPixel / 8);
    /*
     * If you don't support a write planemask, and have set the
     * appropriate flag, then the planemask can be safely ignored.
     * The same goes for the raster-op if only GXcopy is supported.
     */
    /*SETWRITEPLANEMASK(planemask);*/
}

static void 
SiSSubsequentFillRectSolid(ScrnInfoPtr pScrn, int x, int y, int w, int h)
{
    SISPtr pSiS = SISPTR(pScrn);
    int destaddr, op;

    destaddr = y * pScrn->displayWidth + x;
    op = sisCMDBLT | sisSRCBG | sisTOP2BOTTOM | sisLEFT2RIGHT;
    if (pSiS->ClipEnabled) 
        op |= sisCLIPINTRN | sisCLIPENABL;
    destaddr *= (pScrn->bitsPerPixel / 8);

    sisBLTSync;
    sisSETHEIGHTWIDTH(h-1, w * (pScrn->bitsPerPixel/8)-1);
    sisSETDSTADDR(destaddr);
    sisSETCMD(op);
}

static void 
SiSSetupForScreenToScreenCopy(ScrnInfoPtr pScrn, int xdir, int ydir, 
                int rop, unsigned int planemask,
                int transparency_color)
{
    SISPtr pSiS = SISPTR(pScrn);
    sisBLTSync;
    sisSETPITCH(pScrn->displayWidth * pScrn->bitsPerPixel / 8, 
            pScrn->displayWidth * pScrn->bitsPerPixel / 8);
    sisSETROP(XAACopyROP[rop]);
    pSiS->Xdirection = xdir;
    pSiS->Ydirection = ydir;
}

static void 
SiSSubsequentScreenToScreenCopy(ScrnInfoPtr pScrn, int x1, int y1, int x2, 
                int y2, int w, int h)
{
    SISPtr pSiS = SISPTR(pScrn);
    int srcaddr, destaddr;
    int op ;

    op = sisCMDBLT | sisSRCVIDEO;
    if (pSiS->Ydirection == -1) {
        op |= sisBOTTOM2TOP;
        srcaddr = (y1 + h - 1) * pScrn->displayWidth;
        destaddr = (y2 + h - 1) * pScrn->displayWidth;
    } else {
        op |= sisTOP2BOTTOM;
        srcaddr = y1 * pScrn->displayWidth;
        destaddr = y2 * pScrn->displayWidth;
    }
    if (pSiS->Xdirection == -1) {
        op |= sisRIGHT2LEFT;
        srcaddr += x1 + w - 1;
        destaddr += x2 + w - 1;
    } else {
        op |= sisLEFT2RIGHT;
        srcaddr += x1;
        destaddr += x2;
    }
    if (pSiS->ClipEnabled) 
        op |= sisCLIPINTRN | sisCLIPENABL;
    srcaddr *= (pScrn->bitsPerPixel/8);
    destaddr *= (pScrn->bitsPerPixel/8);
    if (((pScrn->bitsPerPixel/8)>1) && (pSiS->Xdirection == -1)) {
        srcaddr += (pScrn->bitsPerPixel/8)-1;
        destaddr += (pScrn->bitsPerPixel/8)-1;
    }

    sisBLTSync;
    sisSETSRCADDR(srcaddr);
    sisSETDSTADDR(destaddr);
    sisSETHEIGHTWIDTH(h-1, w * (pScrn->bitsPerPixel/8)-1);
    sisSETCMD(op);
}

static void 
SiSSetupForMono8x8PatternFill(ScrnInfoPtr pScrn, int patternx, int patterny, 
                int fg, int bg, int rop, unsigned int planemask)
{
    SISPtr pSiS = SISPTR(pScrn);
    unsigned int  *patternRegPtr;
    int  i;
    int  dstpitch;

    (void)XAAHelpPatternROP(pScrn, &fg, &bg, planemask, &rop);

    dstpitch = pScrn->displayWidth * pScrn->bitsPerPixel / 8 ;
    sisBLTSync;
    sisSETBGCOLOR(bg);
    sisSETFGCOLOR(fg);
    if (bg != -1) {
        sisSETROPBG(0xcc);  /* copy */
    } else {
        sisSETROPBG(0xAA);  /* dst */
    }
    sisSETROPFG(rop);
    sisSETPITCH(0, dstpitch);    
    sisSETSRCADDR(0);
    patternRegPtr =  (unsigned int *)sisSETPATREG();
    pSiS->sisPatternReg[0] = pSiS->sisPatternReg[2] = patternx ;
    pSiS->sisPatternReg[1] = pSiS->sisPatternReg[3] = patterny ;
    for ( i = 0 ; i < 16 /* sisPatternHeight */ ; ) {
        patternRegPtr[i++] = patternx ;
        patternRegPtr[i++] = patterny ;
    }
}

static void 
SiSSubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn, int patternx, 
                int patterny, int x, int y, int w, int h)
{
    SISPtr pSiS = SISPTR(pScrn);
    int         dstaddr;
    register unsigned char  *patternRegPtr ;
    register unsigned char  *srcPatternRegPtr ;    
    register unsigned int   *patternRegPtrL ;
    int     i, k ;
    unsigned short  tmp;
    int     shift ;
    int     op  = sisCMDCOLEXP | sisTOP2BOTTOM | sisLEFT2RIGHT | 
                  sisPATFG | sisSRCBG ;
    if (pSiS->ClipEnabled)
        op |= sisCLIPINTRN | sisCLIPENABL;

    dstaddr = ( y * pScrn->displayWidth + x ) * pScrn->bitsPerPixel / 8;
    sisBLTSync;
    patternRegPtr = sisSETPATREG();
    srcPatternRegPtr = (unsigned char *)pSiS->sisPatternReg ;
    shift = 8 - patternx ;
    for ( i = 0, k = patterny ; i < 8 ; i++, k++ ) {
        tmp = srcPatternRegPtr[k]<<8 | srcPatternRegPtr[k] ;
        tmp >>= shift ;
        patternRegPtr[i] = tmp & 0xff ;
    }
    patternRegPtrL = (unsigned int *)sisSETPATREG();
    for ( i = 2 ; i < 16 /* sisPatternHeight */; ) {
        patternRegPtrL[i++] = patternRegPtrL[0];
        patternRegPtrL[i++] = patternRegPtrL[1];
    }

    sisSETDSTADDR(dstaddr);
    sisSETHEIGHTWIDTH(h-1, w*(pScrn->bitsPerPixel/8)-1);
    sisSETCMD(op);
}

#if 0
/*
 * setup for screen-to-screen color expansion
 */
static void 
SiSSetupForScreenToScreenColorExpandFill (ScrnInfoPtr pScrn,
    int fg, int bg, 
    int rop, unsigned int planemask)
{
    SISPtr pSiS = SISPTR(pScrn);
    int isTransparent = (bg == -1);

    /*ErrorF("SISSetupScreenToScreenColorExpand()\n");*/

    /*
     * check transparency 
     */
    /* becareful with rop */

    sisBLTSync;
    if (isTransparent) {
        sisSETBGCOLOR(bg);
        sisSETFGCOLOR(fg);
        sisSETROPFG(0xf0);  /* pat copy */
        sisSETROPBG(0xAA);  /* dst */
    } else {
        sisSETBGCOLOR(bg);
        sisSETFGCOLOR(fg);
        sisSETROPFG(0xf0);  /* pat copy */
        sisSETROPBG(0xcc);  /* copy */
    }
}

/*
 * executing screen-to-screen color expansion
 */
static void 
SiSSubsequentScreenToScreenColorExpandFill( ScrnInfoPtr pScrn,
                int x, int y, int w, int h,
                int srcx, int srcy, int offset )
/* Offset needs to be taken into account. By now, is not used */
{
    SISPtr pSiS = SISPTR(pScrn);
    int destpitch = pScrn->displayWidth * pScrn->bitsPerPixel / 8 ;
    int srcaddr = srcy * destpitch *  + srcx ;
    int destaddr = y * destpitch + x * pScrn->bitsPerPixel / 8;
    int srcpitch ;
    int ww ;
    int widthTodo ;
    int op ;

    op  = sisCMDCOLEXP | sisTOP2BOTTOM | sisLEFT2RIGHT | sisPATFG | sisSRCBG | sisCMDENHCOLEXP ;
    if (pSiS->ClipEnabled)
        op |= sisCLIPINTRN | sisCLIPENABL;


/*    ErrorF("SISSubsequentScreenToScreenColorExpand()\n"); */
#define maxWidth 144
    /* can't expand more than maxWidth in one time.
       it's a work around for scanline greater than maxWidth 
     */
    destpitch = pScrn->displayWidth * pScrn->bitsPerPixel / 8 ;
    srcpitch =  ((w + 31)& ~31) /8 ;
    sisBLTSync;
    sisSETPITCH(srcpitch, destpitch);
    widthTodo = w ;
    do { 
        ww = widthTodo < maxWidth ? widthTodo : maxWidth ;
        sisSETDSTADDR(destaddr);
        sisSETSRCADDR(srcaddr);
        sisSETHEIGHTWIDTH(h-1, ww*(pScrn->bitsPerPixel / 8)-1);
        sisSETCMD(op);
        srcaddr += ww ;
        destaddr += ww*pScrn->bitsPerPixel / 8 ;
        widthTodo -= ww ;
    } while ( widthTodo > 0 ) ;
}
#endif

static void SiSSetClippingRectangle ( ScrnInfoPtr pScrn,
                int left, int top, int right, int bottom)
{
    SISPtr pSiS = SISPTR(pScrn);

    sisBLTSync;
    sisSETCLIPTOP(left,top);
    sisSETCLIPBOTTOM(right,bottom);
    pSiS->ClipEnabled = TRUE;
    
}

static void SiSDisableClipping (ScrnInfoPtr pScrn)
{
    SISPtr pSiS = SISPTR(pScrn);
    pSiS->ClipEnabled = FALSE;
}

static void SiSSetupForSolidLine(ScrnInfoPtr pScrn, 
                int color, int rop, unsigned int planemask)
{
    SISPtr pSiS = SISPTR(pScrn);

    sisBLTSync;
    sisSETFGCOLOR(color);
    sisSETBGCOLOR(0);
    sisSETROP(XAACopyROP[rop]);     /* dst */
}


static void SiSSubsequentSolidTwoPointLine(ScrnInfoPtr pScrn,
            int x1, int y1, int x2, int y2, int flags)

{
    SISPtr pSiS = SISPTR(pScrn);
    int op ;
    int major, minor, err,K1,K2, tmp;
    op = sisCMDLINE  | sisSRCFG;
    if ((flags & OMIT_LAST))
        op |= sisLASTPIX;
    if (pSiS->ClipEnabled) 
        op |= sisCLIPINTRN | sisCLIPENABL;
    if ((major = x2 - x1) <= 0) {
       major = -major;
    } else 
        op |= sisXINCREASE;;        
    if ((minor = y2 - y1) <= 0) {
       minor = -minor;
    } else 
        op |= sisYINCREASE;
    if (minor >= major) {
       tmp = minor; 
       minor = major; 
       major = tmp;
    }
    else 
        op |= sisXMAJOR;

    K1 = (minor - major)<<1;
    K2 = minor<<1;
    err = (minor<<1) - major;

    sisBLTSync;
    sisSETXStart(x1);
    sisSETYStart(y1);
    sisSETLineSteps((short)K1,(short)K2); 
    sisSETLineErrorTerm((short)err);
    sisSETLineMajorCount((short)major);
    sisSETCMD(op);
/*  sisBLTSync;*/
}


static void SiSSubsequentSolidHorVertLine(ScrnInfoPtr pScrn,
                                int x, int y, int len, int dir)
{
    SISPtr pSiS = SISPTR(pScrn);
    int destaddr, op;

    destaddr = y * pScrn->displayWidth + x;
    op = sisCMDBLT | sisSRCFG | sisTOP2BOTTOM | sisLEFT2RIGHT;
    if (pSiS->ClipEnabled)
        op |= sisCLIPINTRN | sisCLIPENABL;
    destaddr *= (pScrn->bitsPerPixel / 8);

    sisBLTSync;
    sisSETPITCH(pScrn->displayWidth * pScrn->bitsPerPixel / 8, 
        pScrn->displayWidth * pScrn->bitsPerPixel / 8);

    if(dir == DEGREES_0)
        sisSETHEIGHTWIDTH(0, len * (pScrn->bitsPerPixel>>3)-1);
    else
        sisSETHEIGHTWIDTH(len-1, (pScrn->bitsPerPixel>>3)-1 );

    sisSETDSTADDR(destaddr);
    sisSETCMD(op);
}
