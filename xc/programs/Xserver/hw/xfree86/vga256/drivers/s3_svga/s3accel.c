/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/s3_svga/s3accel.c,v 1.1.2.4 1998/10/21 13:38:00 dawes Exp $ */

/*
 *
 * Copyright 1996-1997 The XFree86 Project, Inc.
 *
 *
 *	Written by Mark Vojkovich (mvojkovi@ucsd.edu)
 *
 */

#define PSZ 8

#include "xf86.h"
#include "xf86_OSlib.h"
#include "vga256.h"
#include "vga.h"
#include "xf86xaa.h"


#include "s3.h"
#include "s3reg.h"

static void S3Sync();
static void S3SetupForScreenToScreenCopy();
static void S3SubsequentScreenToScreenCopy();
static void S3SetupForFillRectSolid();
static void S3SubsequentFillRectSolid();
static void S3SubsequentBresenhamLine();
static void S3SetupForDashedLine();
static void S3SubsequentDashedBresenhamLine32();
static void S3SetupForFill8x8Pattern();
static void S3SubsequentFill8x8Pattern();
static void S3SetupForCPUToScreenColorExpand();
static void S3FillRectStippledCPUToScreenColorExpand();
static void S3WriteBitmapCPUToScreenColorExpand32();
#ifdef S3_NEWMMIO
 static void S3SubsequentCPUToScreenColorExpand32();
#else
 static void S3SubsequentDashedBresenhamLine16();
 static void S3SetupForScanlineScreenToScreenColorExpand(); 
 static void S3SubsequentScanlineScreenToScreenColorExpand16();
 static void S3SubsequentScanlineScreenToScreenColorExpand32();
 static void S3WriteBitmapCPUToScreenColorExpand16();
#endif

static Bool Transfer32 = FALSE;
static Bool ColorExpandBug = FALSE;
static unsigned char ScratchBuffer[512];
static unsigned char SwappedBytes[256];
static CARD32 ShiftMasks[32];

#define MAX_LINE_PATTERN_LENGTH 512
#define LINE_PATTERN_START	((MAX_LINE_PATTERN_LENGTH >> 5) - 1)
static CARD32 DashPattern[MAX_LINE_PATTERN_LENGTH >> 5];

void
#ifdef S3_NEWMMIO
S3AccelInit_NewMMIO() 
#else
S3AccelInit() 
#endif
{
#ifndef S3_NEWMMIO
    if(S3_x64_SERIES(s3ChipId))
#endif
	Transfer32 = TRUE; 

    if(S3_x68_SERIES(s3ChipId))
	ColorExpandBug = TRUE;

    xf86AccelInfoRec.Flags = BACKGROUND_OPERATIONS  | PIXMAP_CACHE |
				COP_FRAMEBUFFER_CONCURRENCY | 
				LINE_PATTERN_MSBFIRST_MSBJUSTIFIED |
				HARDWARE_PATTERN_NOT_LINEAR;

    if(s3Bpp != 3) 
	xf86AccelInfoRec.Flags |= HARDWARE_PATTERN_TRANSPARENCY;	
	
    xf86AccelInfoRec.Sync = S3Sync;

    /* copy area */
    if((s3Bpp == 3) || S3_911_SERIES(s3ChipId))
       xf86GCInfoRec.CopyAreaFlags = NO_TRANSPARENCY;
    else   
       xf86GCInfoRec.CopyAreaFlags = TRANSPARENCY_GXCOPY;

    xf86AccelInfoRec.SetupForScreenToScreenCopy =
        			S3SetupForScreenToScreenCopy;
    xf86AccelInfoRec.SubsequentScreenToScreenCopy =
        			S3SubsequentScreenToScreenCopy;

    /* filled rects */
    xf86AccelInfoRec.SetupForFillRectSolid = S3SetupForFillRectSolid;
    xf86AccelInfoRec.SubsequentFillRectSolid = S3SubsequentFillRectSolid; 


    /* lines */
    xf86AccelInfoRec.SubsequentBresenhamLine = S3SubsequentBresenhamLine;
    if(S3_911_SERIES(s3ChipId)) 
    	xf86AccelInfoRec.ErrorTermBits = 11;
    else
    	xf86AccelInfoRec.ErrorTermBits = 12;

    /* dashed lines */
    if(s3Bpp != 3) {
    	xf86AccelInfoRec.SetupForDashedLine = S3SetupForDashedLine;
    	xf86AccelInfoRec.LinePatternBuffer = (void*)DashPattern;     
    	xf86AccelInfoRec.LinePatternMaxLength = MAX_LINE_PATTERN_LENGTH;
#ifndef S3_NEWMMIO
	if(!Transfer32)
       	    xf86AccelInfoRec.SubsequentDashedBresenhamLine = 
				S3SubsequentDashedBresenhamLine16; 
	else
#endif
       	    xf86AccelInfoRec.SubsequentDashedBresenhamLine = 
				S3SubsequentDashedBresenhamLine32; 
    }

    /* 8x8 pattern fills */
    /*  A hardware bug in some S3's at 32bpp is worked-around in 
	the XAA pixmap code */
    if(!S3_911_SERIES(s3ChipId)) {
       xf86AccelInfoRec.SetupForFill8x8Pattern = S3SetupForFill8x8Pattern;
       xf86AccelInfoRec.SubsequentFill8x8Pattern = S3SubsequentFill8x8Pattern;
    }


    /* Color Expand */
#ifdef S3_NEWMMIO 
    xf86AccelInfoRec.SetupForCPUToScreenColorExpand =  
				S3SetupForCPUToScreenColorExpand;
    xf86AccelInfoRec.SubsequentCPUToScreenColorExpand =
				S3SubsequentCPUToScreenColorExpand32;

    xf86AccelInfoRec.CPUToScreenColorExpandBase = (void*) &IMG_TRANS;
    xf86AccelInfoRec.CPUToScreenColorExpandRange = 0x8000;

    xf86AccelInfoRec.ColorExpandFlags = CPU_TRANSFER_PAD_DWORD |
					BIT_ORDER_IN_BYTE_MSBFIRST |
					SCANLINE_PAD_DWORD;

#else
    xf86AccelInfoRec.SetupForScanlineScreenToScreenColorExpand = 
			S3SetupForScanlineScreenToScreenColorExpand;
    if(Transfer32) {
	xf86AccelInfoRec.SubsequentScanlineScreenToScreenColorExpand =
			S3SubsequentScanlineScreenToScreenColorExpand32;
    } else {
	xf86AccelInfoRec.SubsequentScanlineScreenToScreenColorExpand =
			S3SubsequentScanlineScreenToScreenColorExpand16;
    }

    xf86AccelInfoRec.ColorExpandFlags = BIT_ORDER_IN_BYTE_MSBFIRST;

    xf86AccelInfoRec.ScratchBufferAddr = 1;
    xf86AccelInfoRec.ScratchBufferSize = 512;
    xf86AccelInfoRec.ScratchBufferBase = (void*)ScratchBuffer;
    xf86AccelInfoRec.PingPongBuffers = 1;

#endif


    /* Stippled rect replacements */

    xf86AccelInfoRec.FillRectOpaqueStippled =
                S3FillRectStippledCPUToScreenColorExpand;
    xf86AccelInfoRec.FillRectStippled =
                S3FillRectStippledCPUToScreenColorExpand;


    /* Write Bitmap replacements */
#ifndef S3_NEWMMIO
    if(!Transfer32)
     	xf86AccelInfoRec.WriteBitmap =
		S3WriteBitmapCPUToScreenColorExpand16;
    else
#endif
      	xf86AccelInfoRec.WriteBitmap =
		S3WriteBitmapCPUToScreenColorExpand32;


    /* pixmap cache */    
    xf86AccelInfoRec.PixmapCacheMemoryStart = 
		vga256InfoRec.virtualY * s3BppDisplayWidth;  
    xf86AccelInfoRec.PixmapCacheMemoryEnd = 
		(vga256InfoRec.videoRam * 1024) - s3CursorBytes;

    {
	int i,j;
	register unsigned char newbyte;
	register unsigned char oldbyte;

    	for(i = 0; i < 256; i++) {
	    oldbyte = i;
	    newbyte = 0;
	    j = 8;
	    while(j--) {
	        newbyte <<= 1;	
		if(oldbyte & 0x01) newbyte++;
		oldbyte >>= 1;
	    }
	    SwappedBytes[i] = newbyte;
    	}

	for(i = 0; i < 32; i++)
	    ShiftMasks[i] = (1 << i) - 1;
    }

}

		/******************\
		|	Sync	   |
		\******************/

void S3Sync() {
    WaitIdle();
}


	/***************************************\
	|	Screen-to-Screen Copy		|
	\***************************************/


static unsigned short BltDirection;
static int TransColor = -1;

static
void S3SetupForScreenToScreenCopy(xdir, ydir, rop, planemask, trans_color)
    int xdir, ydir;
    int rop;
    unsigned planemask;
    int trans_color;
{
    BltDirection = CMD_BITBLT | DRAW | WRTDATA;

    if(xdir == 1) BltDirection |= INC_X;
    if(ydir == 1) BltDirection |= INC_Y;

    TransColor = trans_color;
   
    WaitQueue16_32(3,4);
    SET_PIX_CNTL(0);
    SET_FRGD_MIX(FSS_BITBLT | s3alu[rop]);
    SET_WRT_MASK(planemask);
}


static
void S3SubsequentScreenToScreenCopy(srcX, srcY, destX, destY, w, h)
    int srcX, srcY, destX, destY, w, h;
{
    w--; h--;
	
    if(!(BltDirection & INC_Y)) {
        srcY += h;
	destY += h;
    } 

    if(!(BltDirection & INC_X)) {
        srcX += w;
	destX += w;
    } 

    if(TransColor == -1) {
    	WaitQueue(7); 
    	SET_CURPT((short)srcX,(short)srcY);              
    	SET_DESTSTP((short)destX, (short)destY);
    	SET_AXIS_PCNT((short)w,(short)h);
    	SET_CMD(BltDirection);
    } else {
        WaitQueue16_32(2,3);
	SET_MULT_MISC(CMD_REG_WIDTH | 0x0100); /* enable compare */
        SET_COLOR_CMP(TransColor);

     	WaitQueue(8); 
   	SET_CURPT((short)srcX,(short)srcY);              
    	SET_DESTSTP((short)destX, (short)destY);
    	SET_AXIS_PCNT((short)w,(short)h);
    	SET_CMD(BltDirection);
	SET_MULT_MISC(CMD_REG_WIDTH);	/* disable compare */
    } 
}

	/***********************\
	|	Solid Rects	|
	\***********************/


static
void S3SetupForFillRectSolid(color, rop, planemask)
    int color, rop;
    unsigned planemask;
{
    WaitQueue16_32(4,6);
    SET_PIX_CNTL(0);
    SET_FRGD_COLOR(color);
    SET_FRGD_MIX(FSS_FRGDCOL | s3alu[rop]);
    SET_WRT_MASK(planemask);
}

static
void S3SubsequentFillRectSolid(x, y, w, h)
    int x, y, w, h;
{
    WaitQueue(5);
    SET_CURPT((short)x, (short)y);
    SET_AXIS_PCNT(w - 1, h - 1);
    SET_CMD(CMD_RECT | DRAW | INC_X | INC_Y  | WRTDATA);
}

	/***********************\
	|	Lines		|
	\***********************/

#include "miline.h"

static
void S3SubsequentBresenhamLine(x1, y1, octant, err, e1, e2, length)
    int x1, y1, octant, err, e1, e2, length;
{
    unsigned short cmd;

    /* Note: 
	We rely on the fact that XAA will never send us a horizontal line
    */
    if(e1) { 
	cmd = CMD_LINE | DRAW | WRTDATA | LASTPIX;

    	if(octant & YMAJOR) cmd |= YMAJAXIS;
    	if(!(octant & XDECREASING)) cmd |= INC_X;
    	if(!(octant & YDECREASING)) cmd |= INC_Y;

   	WaitQueue(7);
   	SET_CURPT((short)x1, (short)y1);
   	SET_ERR_TERM((short)err);
    	SET_DESTSTP((short)e2, (short)e1);
    	SET_MAJ_AXIS_PCNT((short)length);
   	SET_CMD(cmd);
    } else { /* vertical line */
     	WaitQueue(4);
    	SET_CURPT((short)x1, (short)y1);
    	SET_MAJ_AXIS_PCNT((short)length - 1);
    	SET_CMD(CMD_LINE | DRAW | LINETYPE | WRTDATA | VECDIR_270);
    }
}



	/*******************************\
	| 	8x8 Fill Patterns	|
	\*******************************/

static
void S3SetupForFill8x8Pattern(patternx, patterny, rop, planemask, trans_col)
    int patternx, patterny, rop, planemask, trans_col;
{
    TransColor = trans_col;
    WaitQueue16_32(3,4);
    SET_PIX_CNTL(0);
    SET_FRGD_MIX(FSS_BITBLT | s3alu[rop]);
    SET_WRT_MASK(planemask);
}

static
void S3SubsequentFill8x8Pattern(patternx, patterny, x, y, w, h)
    int patternx, patterny, x, y, w, h;
{
    if(TransColor == -1) {
    	WaitQueue(7);
    	SET_CURPT((short)patternx, (short)patterny); 
    	SET_DESTSTP((short)x, (short)y);
    	SET_AXIS_PCNT(w - 1, h - 1);
    	SET_CMD(CMD_PFILL | DRAW | INC_Y | INC_X | WRTDATA);
    } else {
    	WaitQueue16_32(2,3);
    	SET_MULT_MISC(CMD_REG_WIDTH | 0x0100); /* enable compare */
    	SET_COLOR_CMP(TransColor);

    	WaitQueue(8);
    	SET_CURPT((short)patternx, (short)patterny); 
    	SET_DESTSTP((short)x, (short)y);
    	SET_AXIS_PCNT(w - 1, h - 1);
    	SET_CMD(CMD_PFILL | DRAW | INC_Y | INC_X | WRTDATA);
    	SET_MULT_MISC(CMD_REG_WIDTH);	/* disable compare */
   }
}

	/***************************************\
	|    CPU to Screen Color Expansion 	|
	\***************************************/


static
void S3SetupForCPUToScreenColorExpand(bg, fg, rop, planemask)
    int bg, fg, rop;
    unsigned planemask;
{
    WaitQueue16_32(3, 4);
    if(bg == -1) {  
	if(ColorExpandBug) {
    	  SET_MIX(FSS_FRGDCOL | s3alu[rop], BSS_BKGDCOL | MIX_XOR); 
    	  SET_BKGD_COLOR(0);
	} else
    	  SET_MIX(FSS_FRGDCOL | s3alu[rop], BSS_BKGDCOL | MIX_DST); 
    } else {
   	SET_MIX(FSS_FRGDCOL | s3alu[rop], BSS_BKGDCOL | s3alu[rop]); 
    	SET_BKGD_COLOR(bg);
    }

    WaitQueue16_32(3, 5);
    SET_FRGD_COLOR(fg);
    SET_WRT_MASK(planemask);
    SET_PIX_CNTL(MIXSEL_EXPPC);
}


#ifdef S3_NEWMMIO

static
void S3SubsequentCPUToScreenColorExpand32(x, y, w, h, skipleft)
    int x, y, w, h, skipleft;
{
    WaitQueue(4);
    SET_CURPT((short)x, (short)y); 
    SET_AXIS_PCNT((short)w - 1, (short)h - 1);

    WaitIdle();
    SET_CMD(CMD_RECT | BYTSEQ | _32BIT | PCDATA | DRAW | PLANAR |
					INC_Y | INC_X | WRTDATA);
}

#endif

	/***********************************************\
	|	Indirect Color Expansion Hack		|
	\***********************************************/

#ifndef S3_NEWMMIO

static int ScanlineWordCount;

static
void S3SetupForScanlineScreenToScreenColorExpand(x, y, w, h, bg, fg,
						 rop, planemask)
   int x, y, w, h, bg, fg, rop, planemask;
{
    S3SetupForCPUToScreenColorExpand(bg, fg, rop, planemask);

    WaitQueue(4);
    SET_CURPT((short)x, (short)y); 
    SET_AXIS_PCNT((short)w - 1, (short)h - 1);

    if(Transfer32) {
    	ScanlineWordCount = (w + 31) >> 5; 

    	WaitIdle();
    	SET_CMD(CMD_RECT | BYTSEQ | _32BIT | PCDATA | DRAW | PLANAR |
					INC_Y | INC_X | WRTDATA);
    } else {
    	ScanlineWordCount = (w + 15) >> 4;

    	WaitIdle();
    	SET_CMD(CMD_RECT | BYTSEQ | _16BIT | PCDATA | DRAW | PLANAR |
					INC_Y | INC_X | WRTDATA);
    }
}

static
void S3SubsequentScanlineScreenToScreenColorExpand16(int srcaddr)
{
    register unsigned short *ptr = (unsigned short*)ScratchBuffer;
    register int count = ScanlineWordCount;

    while(count--)
	SET_PIX_TRANS_W(*(ptr++)); 
}

static
void S3SubsequentScanlineScreenToScreenColorExpand32(int srcaddr)
{
    register CARD32 *ptr = (CARD32*)ScratchBuffer;
    register int count = ScanlineWordCount;

    while(count--)
	SET_PIX_TRANS_L(*(ptr++)); 
}

#endif

	/***********************\
	|	Dashed Lines	|
	\***********************/


static int DashPatternSize;
static Bool NicePattern;

static
void S3SetupForDashedLine(fg, bg, rop, planemask, size)
    int fg, bg, rop, planemask, size;
{
    S3SetupForCPUToScreenColorExpand(bg, fg, rop, planemask);

    NicePattern = FALSE;

    if(size <= 32) {
    	register CARD32 scratch = DashPattern[LINE_PATTERN_START];
	if(size & (size - 1)) {
	  	while(size < 16) {
		   scratch |= (scratch >> size);
		   size <<= 1;
	  	}
		scratch |= (scratch >> size);
	 	DashPattern[LINE_PATTERN_START] = scratch;
	 } else { 
          	switch(size) {
	  	   case 2:	scratch |= scratch >> 2;
	  	   case 4:	scratch |= scratch >> 4;
	  	   case 8:	scratch |= scratch >> 8;
	  	   case 16:	scratch |= scratch >> 16;
	 			DashPattern[LINE_PATTERN_START] = scratch;
		   case 32:	NicePattern = TRUE;
	  	   default:	break;	
		}		
        }
    }
    DashPatternSize = size;
}


static void 
S3SubsequentDashedBresenhamLine32(x1, y1, octant, err, e1, e2, length, start)
    int x1, y1, octant, err, e1, e2, length, start;
{
    register int count = (length + 31) >> 5;
    register CARD32 pattern;
 
    if(e1) {
    	unsigned short cmd = _32BIT | PLANAR | WRTDATA | DRAW | PCDATA | 
			LASTPIX | CMD_LINE;

       	if(octant & YMAJOR) cmd |= YMAJAXIS;
    	if(!(octant & XDECREASING)) cmd |= INC_X;
    	if(!(octant & YDECREASING)) cmd |= INC_Y;

    	WaitQueue(7);
    	SET_CURPT((short)x1, (short)y1);
    	SET_ERR_TERM((short)err);
    	SET_DESTSTP((short)e2, (short)e1);
    	SET_MAJ_AXIS_PCNT((short)length);
    	SET_CMD(cmd);
    } else {
	if (octant & YMAJOR){
     	    WaitQueue(4);
    	    SET_CURPT((short)x1, (short)y1);
    	    SET_MAJ_AXIS_PCNT((short)length - 1);

   	    if(octant & YDECREASING) {   
    	    	SET_CMD(_32BIT | PLANAR | WRTDATA | DRAW | PCDATA |
				 CMD_LINE | LINETYPE | VECDIR_090);
            } else {
    	    	SET_CMD(_32BIT | PLANAR | WRTDATA | DRAW | PCDATA | 
				CMD_LINE | LINETYPE | VECDIR_270);
	    }
	} else {
    	    if(octant & XDECREASING) { 
 		WaitQueue(4);
    	    	SET_CURPT((short)x1, (short)y1);
    	    	SET_MAJ_AXIS_PCNT((short)length - 1);
    	    	SET_CMD(_32BIT | PLANAR | WRTDATA | DRAW | PCDATA |
				 CMD_LINE | LINETYPE | VECDIR_180);
            } else { 	/* he he */
    		WaitQueue(4);
    		SET_CURPT((short)x1, (short)y1); 
    		SET_AXIS_PCNT((short)length - 1, 0);

    		WaitIdle();
    		SET_CMD(_32BIT | PLANAR | WRTDATA | DRAW | PCDATA | 
				CMD_RECT | INC_Y | INC_X);
	    } 
	}
    }

    if(NicePattern) {
#ifdef S3_NEWMMIO
 	register CARD32* dest = (CARD32*)&IMG_TRANS;
#endif
	pattern = (start) ? 
	     (DashPattern[LINE_PATTERN_START] << start) |
 		(DashPattern[LINE_PATTERN_START] >> (32 - start)) :
			DashPattern[LINE_PATTERN_START];

#ifdef S3_NEWMMIO
	while(count & ~0x03) {
		dest[0] = dest[1] = dest[2] = dest[3] = pattern;
		dest += 4;
		count -= 4;
	}	
	switch(count) {
		case 1: dest[0] = pattern; 
			break;
		case 2: dest[0] = dest[1] = pattern; 
			break;
		case 3: dest[0] = dest[1] = dest[2] = pattern; 
			break;
	}
#else
	while(count--)
	    SET_PIX_TRANS_L(pattern);
#endif
    } else if(DashPatternSize < 32) {
	register int offset = start;

	 while(count--) {
	    SET_PIX_TRANS_L((DashPattern[LINE_PATTERN_START] << offset) | 
	   	(DashPattern[LINE_PATTERN_START] >> (DashPatternSize-offset)));
	    offset += 32;
	    while(offset > DashPatternSize)
		offset -= DashPatternSize;
	 }
    } else { 
        int offset = start;
        register unsigned char* srcp = (unsigned char*)(DashPattern) + 
                                        (MAX_LINE_PATTERN_LENGTH >> 3) - 1;
        register CARD32* scratch;
        int scratch2, shift;
                        
        while(count--) {
                shift = DashPatternSize - offset;
                scratch = (CARD32*)(srcp - (offset >> 3) - 3);
                scratch2 = offset & 0x07;

                if(shift & ~31) {
                   if(scratch2) {
                      pattern = (*scratch << scratch2) |
                                (*(scratch - 1) >> (32 - scratch2));
                   } else 
                       pattern = *scratch; 
                } else {
                    pattern = (*((CARD32*)(srcp - 3)) >> shift) | 
                        (*scratch << scratch2);
                }
                SET_PIX_TRANS_L(pattern);
                offset += 32;
                while(offset >= DashPatternSize) 
                    offset -= DashPatternSize;
	}
    }
}

#ifndef S3_NEWMMIO

static
void S3SubsequentDashedBresenhamLine16(x1, y1, octant, err, e1, e2, length,
							start)
    int x1, y1, octant, err, e1, e2, length, start;
{
    register int count = (length + 15) >> 4;
    register CARD32 pattern;
    Bool plus_one;

    plus_one = (count & 0x01);
    count >>= 1;

    if(e1) {
   	unsigned short cmd = _16BIT | PLANAR | WRTDATA | DRAW | PCDATA | 
			LASTPIX | CMD_LINE;


       	if(octant & YMAJOR) cmd |= YMAJAXIS;
    	if(!(octant & XDECREASING)) cmd |= INC_X;
    	if(!(octant & YDECREASING)) cmd |= INC_Y;

    	WaitQueue(7);
    	SET_CURPT((short)x1, (short)y1);
    	SET_ERR_TERM((short)err);
    	SET_DESTSTP((short)e2, (short)e1);
    	SET_MAJ_AXIS_PCNT((short)length);
    	SET_CMD(cmd);
    } else {
	if (octant & YMAJOR){
     	    WaitQueue(4);
    	    SET_CURPT((short)x1, (short)y1);
    	    SET_MAJ_AXIS_PCNT((short)length - 1);

   	    if(octant & YDECREASING) {   
    	    	SET_CMD(_16BIT | PLANAR | WRTDATA | DRAW | PCDATA | 
				CMD_LINE | LINETYPE | VECDIR_090);
            } else {
    	    	SET_CMD(_16BIT | PLANAR | WRTDATA | DRAW | PCDATA | 
				CMD_LINE | LINETYPE | VECDIR_270);
	    }
	} else {
    	    if(octant & XDECREASING) { 
 		WaitQueue(4);
    	    	SET_CURPT((short)x1, (short)y1);
    	    	SET_MAJ_AXIS_PCNT((short)length - 1);
    	    	SET_CMD(_16BIT | PLANAR | WRTDATA | DRAW | PCDATA | 
				CMD_LINE | LINETYPE | VECDIR_180);
            } else { 	/* he he */
    		WaitQueue(4);
    		SET_CURPT((short)x1, (short)y1); 
    		SET_AXIS_PCNT((short)length - 1, 0);

    		WaitIdle();
    		SET_CMD(_16BIT | PLANAR | WRTDATA | DRAW | PCDATA | 
				CMD_RECT | INC_Y | INC_X);
	    } 
	}
    }

    if(NicePattern) {
	   pattern = (start) ? 
	     (DashPattern[LINE_PATTERN_START] << start) |
 		(DashPattern[LINE_PATTERN_START] >> (32 - start)) :
			DashPattern[LINE_PATTERN_START];

	   while(count--) {
	   	SET_PIX_TRANS_W(pattern >> 16);
	   	SET_PIX_TRANS_W(pattern);
	   }
	   if(plus_one)
	   	SET_PIX_TRANS_W(pattern >> 16);

    } else if(DashPatternSize < 32) {
	register int offset = start;

	while(count--) {
		pattern = (DashPattern[LINE_PATTERN_START] << offset) | 
		 (DashPattern[LINE_PATTERN_START] >> (DashPatternSize-offset));
	    	SET_PIX_TRANS_W(pattern >> 16);
		offset += 32;
		while(offset > DashPatternSize)
		    offset -= DashPatternSize;
	    	SET_PIX_TRANS_W(pattern);
	}
 	if(plus_one) 
		SET_PIX_TRANS_W(((DashPattern[LINE_PATTERN_START] << offset) | 
			(DashPattern[LINE_PATTERN_START] >> 
				(DashPatternSize - offset))) >> 16);
    } else {
	int offset = start;
	register unsigned char* srcp = (unsigned char*)(DashPattern) + 
					(MAX_LINE_PATTERN_LENGTH >> 3) - 1;
	register CARD32* scratch;
	int scratch2, shift;

	while(count--) {
	   	shift = DashPatternSize - offset;
		scratch = (CARD32*)(srcp - (offset >> 3) - 3);
		scratch2 = offset & 0x07;

		if(shift & ~31) {
		   if(scratch2) {
		      pattern =	(*scratch << scratch2) |
				(*(scratch - 1) >> (32 - scratch2));
		   } else 
		       pattern = *scratch; 
		} else {
		    pattern = (*((CARD32*)(srcp - 3)) >> shift) | 
			(*scratch << scratch2);

		}
	   	SET_PIX_TRANS_W(pattern >> 16);
		offset += 32;
		while(offset >= DashPatternSize) 
		    offset -= DashPatternSize;
	   	SET_PIX_TRANS_W(pattern);
	}
	if(plus_one) {	
	   	shift = DashPatternSize - offset;
		scratch = (CARD32*)(srcp - (offset >> 3) - 3);
		scratch2 = offset & 0x07;

		if(shift & ~31) {
		   if(scratch2) {
		      pattern =	(*scratch << scratch2) |
				(*(scratch - 1) >> (32 - scratch2));
		   } else 
		       pattern = *scratch; 
		} else {
		    pattern = (*((CARD32*)(srcp - 3)) >> shift) | 
			(*scratch << scratch2);

		}
	   	SET_PIX_TRANS_W(pattern >> 16);
	}
    }
}

#endif

	/***************************************\
	|	Some static helper functions	|
	\***************************************/


#if defined(__GNUC__) && defined(__i386__)
static __inline__ CARD32 reverse_bitorder(CARD32 data) {
#if defined(Lynx) || (defined(SYSV) || defined(SVR4)) && !defined(ACK_ASSEMBLER) || defined(__ELF__)
	__asm__(
		"movl $0,%%ecx\n"
		"movb %%al,%%cl\n"
		"movb SwappedBytes(%%ecx),%%al\n"
		"movb %%ah,%%cl\n"
		"movb SwappedBytes(%%ecx),%%ah\n"
		"roll $16,%%eax\n"
		"movb %%al,%%cl\n"
		"movb SwappedBytes(%%ecx),%%al\n"
		"movb %%ah,%%cl\n"
		"movb SwappedBytes(%%ecx),%%ah\n"
		"roll $16,%%eax\n"
		: "=a" (data) : "0" (data)
		: "cx"
		);
#else
	__asm__(
		"movl $0,%%ecx\n"
		"movb %%al,%%cl\n"
		"movb _SwappedBytes(%%ecx),%%al\n"
		"movb %%ah,%%cl\n"
		"movb _SwappedBytes(%%ecx),%%ah\n"
		"roll $16,%%eax\n"
		"movb %%al,%%cl\n"
		"movb _SwappedBytes(%%ecx),%%al\n"
		"movb %%ah,%%cl\n"
		"movb _SwappedBytes(%%ecx),%%ah\n"
		"roll $16,%%eax\n"
		: "=a" (data) : "0" (data)
		: "cx"
		);
#endif
	return data;
}
#else
static __inline__ CARD32 reverse_bitorder(CARD32 data) {
    unsigned char* kludge = (unsigned char*)&data;

    kludge[0] = SwappedBytes[kludge[0]];
    kludge[1] = SwappedBytes[kludge[1]];
    kludge[2] = SwappedBytes[kludge[2]];
    kludge[3] = SwappedBytes[kludge[3]];
	
    return data;	
}
#endif

#ifdef S3_NEWMMIO
static __inline__ SetDWORDS(dest, value, dwords)
   register CARD32* dest;
   register CARD32 value;
   register int dwords;
{
    while(dwords & ~0x03) {
	dest[0] = dest[1] = dest[2] = dest[3] = value;
	dest += 4;
	dwords -= 4;
    }	
    switch(dwords) {
	case 1: dest[0] = value; 
		break;
	case 2: dest[0] = dest[1] = value; 
		break;
	case 3: dest[0] = dest[1] = dest[2] = value; 
		break;
    }
}
#endif


	/***********************************************\
	|	Stippled Rectangle Replacements 	|
	\***********************************************/



static void
S3FillStippledCPUToScreenColorExpand32(x, y, w, h, src, srcwidth,
stipplewidth, stippleheight, srcx, srcy)
    int x, y, w, h;
    unsigned char *src;
    int srcwidth;
    int stipplewidth, stippleheight;
    int srcx, srcy;
{
    unsigned char *srcp;
    int dwords, count; 

    WaitQueue(4);
    SET_CURPT((short)x, (short)y); 
    SET_AXIS_PCNT((short)w - 1, (short)h - 1);

    dwords = (w + 31) >> 5;    
    srcp = (srcwidth * srcy) + src;

    WaitIdle();	
    SET_CMD(CMD_RECT | BYTSEQ | _32BIT | PCDATA | DRAW | PLANAR |
					INC_Y | INC_X | WRTDATA);


    if(!((stipplewidth > 32) || (stipplewidth & (stipplewidth - 1)))) { 
    	CARD32 pattern;
    	register unsigned char* kludge = (unsigned char*)(&pattern);

	while(h--) {
	   switch(stipplewidth) {
		case 32:
	   	    pattern = *((CARD32*)srcp);  
		    break;
	      	case 16:
		    kludge[0] = kludge[2] = srcp[0];
		    kludge[1] = kludge[3] = srcp[1];
		    break;
	      	case 8:
		    kludge[0] = kludge[1] = kludge[2] = kludge[3] = srcp[0];
		    break;
	      	case 4:
		    kludge[0] = kludge[1] = kludge[2] = kludge[3] = 
				(srcp[0] & 0x0F);
		    pattern |= (pattern << 4);
		    break;
		case 2:
		    kludge[0] = kludge[1] = kludge[2] = kludge[3] = 
				(srcp[0] & 0x03);
		    pattern |= (pattern << 2);
		    pattern |= (pattern << 4);
		    break;
		default:	/* case 1: */
		    if(srcp[0] & 0x01) 
			pattern = 0xffffffff;
		    else
			pattern = 0x00000000;
		    break;
	   }

	   if(srcx) 
		pattern = (pattern >> srcx) | (pattern << (32 - srcx));           
	   pattern = reverse_bitorder(pattern);

#ifdef S3_NEWMMIO
   	   SetDWORDS((CARD32*)&IMG_TRANS, pattern, dwords); 
#else
	   count = dwords;
	   while(count--)
	   	    SET_PIX_TRANS_L(pattern);
#endif
	   srcy++;
	   srcp += srcwidth;
	   if (srcy >= stippleheight) {
	     srcy = 0;
	     srcp = src;
	   }
	}
    } else if(stipplewidth < 32) {
	register int width, offset;
	register CARD32 pattern;

	while(h--) {
	   width = stipplewidth;
	   pattern = *((CARD32*)srcp) & ShiftMasks[width];  
	   while(!(width & ~15)) {
		pattern |= (pattern << width);
		width <<= 1;	
	   }
	   pattern |= (pattern << width);
 

	   offset = srcx;

	   count = dwords;
	   while(count--) {
	   	SET_PIX_TRANS_L(reverse_bitorder((pattern >> offset) | 
				(pattern << (width - offset))));
		offset += 32;
		while(offset >= width) 
		    offset -= width;
	   }

	   srcy++;
	   srcp += srcwidth;
	   if (srcy >= stippleheight) {
	     srcy = 0;
	     srcp = src;
	   }
	}
    } else {
	register CARD32* scratch;
	register CARD32 pattern;
	int shift, offset, scratch2;

	while(h--) {
	   count = dwords;
	   offset = srcx;
	   	   	
	   while(count--) {
	   	shift = stipplewidth - offset;
		scratch = (CARD32*)(srcp + (offset >> 3));
		scratch2 = offset & 0x07;

		if(shift & ~31) {
		   if(scratch2) {
		      pattern = (*scratch >> scratch2) |
			(scratch[1] << (32 - scratch2));
		   } else 
		       pattern = *scratch; 
		} else {
		    pattern = (*((CARD32*)srcp) << shift) |
			((*scratch >> scratch2) & ShiftMasks[shift]);
		}
	   	SET_PIX_TRANS_L(reverse_bitorder(pattern));
		offset += 32;
		while(offset >= stipplewidth) 
		    offset -= stipplewidth;
	   }	

	   srcy++;
	   srcp += srcwidth;
	   if (srcy >= stippleheight) {
	     srcy = 0;
	     srcp = src;
	   }
	}
    }
}

#ifndef S3_NEWMMIO

static void
S3FillStippledCPUToScreenColorExpand16(x, y, w, h, src, srcwidth,
stipplewidth, stippleheight, srcx, srcy)
    int x, y, w, h;
    unsigned char *src;
    int srcwidth;
    int stipplewidth, stippleheight;
    int srcx, srcy;
{
    unsigned char *srcp;
    int dwords, count; 
    Bool PlusOne;

    WaitQueue(4);
    SET_CURPT((short)x, (short)y); 
    SET_AXIS_PCNT((short)w - 1, (short)h - 1);

    dwords = (w + 15) >> 4;  
    PlusOne = (dwords & 0x01);
    dwords >>= 1;  
    srcp = (srcwidth * srcy) + src;

    WaitIdle();	
    	SET_CMD(CMD_RECT | BYTSEQ | _16BIT | PCDATA | DRAW | PLANAR |
					INC_Y | INC_X | WRTDATA);

    if(!((stipplewidth > 32) || (stipplewidth & (stipplewidth - 1)))) { 
    	CARD32 pattern;
    	register unsigned char* kludge = (unsigned char*)(&pattern);

	while(h--) {
	   switch(stipplewidth) {
		case 32:
	   	    pattern = *((CARD32*)srcp);  
		    break;
	      	case 16:
		    kludge[0] = kludge[2] = srcp[0];
		    kludge[1] = kludge[3] = srcp[1];
		    break;
	      	case 8:
		    kludge[0] = kludge[1] = kludge[2] = kludge[3] = srcp[0];
		    break;
	      	case 4:
		    kludge[0] = kludge[1] = kludge[2] = kludge[3] = 
				(srcp[0] & 0x0F);
		    pattern |= (pattern << 4);
		    break;
		case 2:
		    kludge[0] = kludge[1] = kludge[2] = kludge[3] = 
				(srcp[0] & 0x03);
		    pattern |= (pattern << 2);
		    pattern |= (pattern << 4);
		    break;
		default:	/* case 1: */
		    if(srcp[0] & 0x01) 
			pattern = 0xffffffff;
		    else
			pattern = 0x00000000;
		    break;
	   }

	   if(srcx) 
		pattern = (pattern >> srcx) | (pattern << (32 - srcx));           
	   pattern = reverse_bitorder(pattern);

	   count = dwords;
	   while(count--) {
	   	SET_PIX_TRANS_W(pattern);
	   	SET_PIX_TRANS_W(*((unsigned short*)(&kludge[2])));
	   }
	   if(PlusOne)
	   	SET_PIX_TRANS_W(pattern);


	   srcy++;
	   srcp += srcwidth;
	   if (srcy >= stippleheight) {
	     srcy = 0;
	     srcp = src;
	   }
	}
    } else if(stipplewidth < 32) {
	int width, offset;
	register CARD32 pattern2;
	register CARD32 pattern;

	while(h--) {
	   width = stipplewidth;
	   pattern = *((CARD32*)srcp) & ShiftMasks[width];  
	   while(!(width & ~15)) {
		pattern |= (pattern << width);
		width <<= 1;	
	   }
	   pattern |= (pattern << width);
 
	   offset = srcx;

	   count = dwords;
	   while(count--) {
		pattern2 = reverse_bitorder((pattern >> offset) | 
				(pattern << (width - offset)));
	   	SET_PIX_TRANS_W(pattern2); 
		offset += 32;
		while(offset >= width) 
		    offset -= width;
	   	SET_PIX_TRANS_W(pattern2 >> 16);
	   }
	   if(PlusOne) 
	   	SET_PIX_TRANS_W(reverse_bitorder((pattern >> offset) | 
				(pattern << (width - offset))));

	   srcy++;
	   srcp += srcwidth;
	   if (srcy >= stippleheight) {
	     srcy = 0;
	     srcp = src;
	   }
	}
    } else {
	register CARD32* scratch;
	register CARD32 pattern;
	int shift, offset, scratch2;

	while(h--) {
	   count = dwords;
	   offset = srcx;
	   	   	
	   while(count--) {
	   	shift = stipplewidth - offset;
		scratch = (CARD32*)(srcp + (offset >> 3));
		scratch2 = offset & 0x07;

		if(shift & ~31) {
		   if(scratch2) {
		      pattern = (*scratch >> scratch2) |
			(*(scratch + 1) << (32 - scratch2));
		   } else 
		       pattern = *scratch; 
		} else {
		    pattern = (*((CARD32*)srcp) << shift) |
			((*scratch >> scratch2) & ShiftMasks[shift]);
		}
		pattern = reverse_bitorder(pattern);
	   	SET_PIX_TRANS_W(pattern);
		offset += 32;
		while(offset >= stipplewidth) 
		    offset -= stipplewidth;
	   	SET_PIX_TRANS_W(pattern >> 16);
	   }
	   if(PlusOne) {
	   	shift = stipplewidth - offset;
		scratch = (CARD32*)(srcp + (offset >> 3));
		scratch2 = offset & 0x07;

		if(shift & ~31) {
		   if(scratch2) {
		      pattern = (*scratch >> scratch2) |
			(*(scratch + 1) << (32 - scratch2));
		   } else 
		       pattern = *scratch; 
		} else {
		    pattern = (*((CARD32*)srcp) << shift) |
			((*scratch >> scratch2) & ShiftMasks[shift]);
		}
	   	SET_PIX_TRANS_W(reverse_bitorder(pattern));
	   }	

	   srcy++;
	   srcp += srcwidth;
	   if (srcy >= stippleheight) {
	     srcy = 0;
	     srcp = src;
	   }
	}
    } 

}

#endif

static void
S3FillRectStippledCPUToScreenColorExpand(pDrawable, pGC, nBoxInit, pBoxInit)
    DrawablePtr pDrawable;
    GCPtr pGC;
    int nBoxInit;		/* number of rectangles to fill */
    BoxPtr pBoxInit;		/* Pointer to first rectangle to fill */
{
    PixmapPtr pPixmap;		/* Pixmap of the area to draw */
    int rectWidth;		/* Width of the rect to be drawn */
    int rectHeight;		/* Height of the rect to be drawn */
    BoxPtr pBox;		/* current rectangle to fill */
    int nBox;			/* Number of rectangles to fill */
    int xoffset, yoffset;
    Bool AlreadySetup = FALSE;

    pPixmap = pGC->stipple;

    for (nBox = nBoxInit, pBox = pBoxInit; nBox > 0; nBox--, pBox++) {

	rectWidth = pBox->x2 - pBox->x1;
	rectHeight = pBox->y2 - pBox->y1;

	if ((rectWidth > 0) && (rectHeight > 0)) {
	    if(!AlreadySetup) {
    		S3SetupForCPUToScreenColorExpand(
	    		(pGC->fillStyle == FillStippled) ? -1 : pGC->bgPixel, 
			pGC->fgPixel, pGC->alu, pGC->planemask);
		AlreadySetup = TRUE;
	    }

	    xoffset = (pBox->x1 - (pGC->patOrg.x + pDrawable->x))
	        % pPixmap->drawable.width;
	    if (xoffset < 0)
	        xoffset += pPixmap->drawable.width;
	    yoffset = (pBox->y1 - (pGC->patOrg.y + pDrawable->y))
	        % pPixmap->drawable.height;
	    if (yoffset < 0)
	        yoffset += pPixmap->drawable.height;
#ifndef S3_NEWMMIO
	    if(!Transfer32)
	      S3FillStippledCPUToScreenColorExpand16(
	        pBox->x1, pBox->y1, rectWidth, rectHeight,
	        pPixmap->devPrivate.ptr, pPixmap->devKind,
	        pPixmap->drawable.width, pPixmap->drawable.height,
	        xoffset, yoffset);
	    else
#endif
	      S3FillStippledCPUToScreenColorExpand32(
	        pBox->x1, pBox->y1, rectWidth, rectHeight,
	        pPixmap->devPrivate.ptr, pPixmap->devKind,
	        pPixmap->drawable.width, pPixmap->drawable.height,
	        xoffset, yoffset);
	}
    }	/* end for loop through each rectangle to draw */

}
	
	/***************************************\
	|	Write Bitmap Replacement	|
	\***************************************/


static void 
S3WriteBitmapCPUToScreenColorExpand32(x, y, w, h, src, srcwidth, srcx,
srcy, bg, fg, rop, planemask)
    int x, y, w, h;
    unsigned char *src;
    int srcwidth;
    int srcx, srcy;
    int bg, fg;
    int rop;
    unsigned int planemask;
{
    unsigned char *srcp;
    int dwords, shift;
    register int count; 
    register CARD32* pattern;

    S3SetupForCPUToScreenColorExpand(bg, fg, rop, planemask);

    WaitQueue(4);
    SET_CURPT((short)x, (short)y); 
    SET_AXIS_PCNT((short)w - 1, (short)h - 1);

    dwords = (w + 31) >> 5;
    srcp = (srcwidth * srcy) + (srcx >> 3) + src;    
    
    WaitIdle();	
    SET_CMD(CMD_RECT | BYTSEQ | _32BIT | PCDATA | DRAW | PLANAR |
					INC_Y | INC_X | WRTDATA);

    if(shift = srcx & 0x07) {
    	while(h--) {
	    count = dwords;
	    pattern = (CARD32*)srcp;
	    while(count--) {
	   	SET_PIX_TRANS_L(reverse_bitorder((*pattern >> shift) | 
				(*(pattern + 1) << (32 - shift))));
		pattern++;
	    }
	    srcp += srcwidth;
    	}    
    } else {
    	while(h--) {
	    count = dwords;
	    pattern = (CARD32*)srcp;
	    while(count--) 
	   	SET_PIX_TRANS_L(reverse_bitorder(*(pattern++)));
	    srcp += srcwidth;
    	}    
    }
}

#ifndef S3_NEWMMIO

static
void S3WriteBitmapCPUToScreenColorExpand16(x, y, w, h, src, srcwidth, srcx,
srcy, bg, fg, rop, planemask)
    int x, y, w, h;
    unsigned char *src;
    int srcwidth;
    int srcx, srcy;
    int bg, fg;
    int rop;
    unsigned int planemask;
{
    unsigned char *srcp;
    int dwords, count, shift; 
    register CARD32* pattern;
    register CARD32 pattern2;
    Bool PlusOne;

    S3SetupForCPUToScreenColorExpand(bg, fg, rop, planemask);

    WaitQueue(4);
    SET_CURPT((short)x, (short)y); 
    SET_AXIS_PCNT((short)w - 1, (short)h - 1);

    dwords = (w + 15) >> 4;  
    PlusOne = (dwords & 0x01);
    dwords >>= 1;
    srcp = (srcwidth * srcy) + (srcx >> 3) + src;  

    WaitIdle();	
    SET_CMD(CMD_RECT | BYTSEQ | _16BIT | PCDATA | DRAW | PLANAR |
					INC_Y | INC_X | WRTDATA);

    if((shift = srcx & 0x07)) {
    	while(h--) {
	    count = dwords;
	    pattern = (CARD32*)srcp;
	    while(count--) {
		pattern2 = reverse_bitorder((*pattern >> shift) | 
				(*(pattern + 1) << (32 - shift)));
	   	SET_PIX_TRANS_W(pattern2);
		pattern++;
	   	SET_PIX_TRANS_W(pattern2 >> 16);
	    }
	    if(PlusOne) 
	   	SET_PIX_TRANS_W(reverse_bitorder((*pattern >> shift) | 
				(*(pattern + 1) << (32 - shift))));
	    srcp += srcwidth;
    	}    
    } else {
    	while(h--) {
	    count = dwords;
	    pattern = (CARD32*)srcp;
	    while(count--) { 
		pattern2 = reverse_bitorder(*(pattern++));
	   	SET_PIX_TRANS_W(pattern2);
	   	SET_PIX_TRANS_W(pattern2 >> 16);
	    }
	    if(PlusOne)
	   	SET_PIX_TRANS_W(reverse_bitorder(*(pattern)));
	    srcp += srcwidth;
    	}    
    }
}

#endif

