/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/r128/r128_accel.c,v 1.1.2.4 2000/01/28 03:35:54 martin Exp $ */
/**************************************************************************

Copyright 1999 ATI Technologies Inc. and Precision Insight, Inc.,
                                         Cedar Park, Texas. 
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, PRECISION INSIGHT AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Rickard E. Faith <faith@precisioninsight.com>
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *
 * Credits:
 *
 *   Thanks to Alan Hourihane <alanh@fairlite.demon..co.uk> and SuSE for
 *   providing source code to their 3.3.x Rage 128 driver.  Portions of
 *   this file are based on the acceleration code for that driver.
 *
 * References:
 *
 *   RAGE 128 VR/ RAGE 128 GL Register Reference Manual (Technical
 *   Reference Manual P/N RRG-G04100-C Rev. 0.04), ATI Technologies: April
 *   1999.
 *
 *   RAGE 128 Software Development Manual (Technical Reference Manual P/N
 *   SDK-G04000 Rev. 0.01), ATI Technologies: June 1999.
 *
 * $PI: xc/programs/Xserver/hw/xfree86/vga256/drivers/r128/r128_accel.c,v 1.11 1999/10/21 20:47:31 faith Exp $
 */

#include "compiler.h"
#include "vga256.h"
#include "xf86.h"
#include "vga.h"
#include "vgaPCI.h"

#include "xf86xaa.h"

#include "xf86Priv.h"
#include "xf86_Config.h"
#include "xf86_PCI.h"

#include "r128.h"
#include "r128_reg.h"

static struct {
    int rop;
    int pattern;
} R128_ROP[] = {
    { R128_ROP3_ZERO, R128_ROP3_ZERO }, /* GXclear        */
    { R128_ROP3_DSa,  R128_ROP3_DPa  }, /* Gxand          */
    { R128_ROP3_SDna, R128_ROP3_PDna }, /* GXandReverse   */
    { R128_ROP3_S,    R128_ROP3_P    }, /* GXcopy         */
    { R128_ROP3_DSna, R128_ROP3_DPna }, /* GXandInverted  */
    { R128_ROP3_D,    R128_ROP3_D    }, /* GXnoop         */
    { R128_ROP3_DSx,  R128_ROP3_DPx  }, /* GXxor          */
    { R128_ROP3_DSo,  R128_ROP3_DPo  }, /* GXor           */
    { R128_ROP3_DSon, R128_ROP3_DPon }, /* GXnor          */
    { R128_ROP3_DSxn, R128_ROP3_PDxn }, /* GXequiv        */
    { R128_ROP3_Dn,   R128_ROP3_Dn   }, /* GXinvert       */
    { R128_ROP3_SDno, R128_ROP3_PDno }, /* GXorReverse    */
    { R128_ROP3_Sn,   R128_ROP3_Pn   }, /* GXcopyInverted */
    { R128_ROP3_DSno, R128_ROP3_DPno }, /* GXorInverted   */
    { R128_ROP3_DSan, R128_ROP3_DPan }, /* GXnand         */
    { R128_ROP3_ONE,  R128_ROP3_ONE  }  /* GXset          */
};

/* Flush all dirty data in the Pixel Cache to memory. */
static void R128EngineFlush(void)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    int           i;
    
    OUTREGP(R128_PC_NGUI_CTLSTAT, R128_PC_FLUSH_ALL, ~R128_PC_FLUSH_ALL);
    for (i = 0; i < R128_TIMEOUT; i++) {
	if (!(INREG(R128_PC_NGUI_CTLSTAT) & R128_PC_BUSY)) break;
    }
}

/* Reset graphics card to known state. */
static void R128EngineReset(void)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    CARD32        clock_cntl_index;
    CARD32        mclk_cntl;
    CARD32        gen_reset_cntl;
	
    R128EngineFlush();
    
    clock_cntl_index = INREG(R128_CLOCK_CNTL_INDEX);
    mclk_cntl        = INPLL(R128_MCLK_CNTL);

    OUTPLL(R128_MCLK_CNTL, mclk_cntl | R128_FORCE_GCP | R128_FORCE_PIPE3D_CPP);

    gen_reset_cntl   = INREG(R128_GEN_RESET_CNTL);

    OUTREG(R128_GEN_RESET_CNTL, gen_reset_cntl | R128_SOFT_RESET_GUI);
    INREG(R128_GEN_RESET_CNTL);
    OUTREG(R128_GEN_RESET_CNTL, gen_reset_cntl & ~R128_SOFT_RESET_GUI);
    INREG(R128_GEN_RESET_CNTL);

    OUTPLL(R128_MCLK_CNTL,        mclk_cntl);
    OUTREG(R128_CLOCK_CNTL_INDEX, clock_cntl_index);
    OUTREG(R128_GEN_RESET_CNTL,   gen_reset_cntl);
}

/* The FIFO has 64 slots.  This routines waits until at least `entries' of
   these slots are empty. */
static void R128WaitForFifo(int entries)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    int           i;

    for (;;) {
	for (i = 0; i < R128_TIMEOUT; i++) {
	    if ((INREG(R128_GUI_STAT) & R128_GUI_FIFOCNT_MASK) >= entries)
		return;
	}
	R128DEBUG(("FIFO timed out: %d entries, stat=0x%08x, probe=0x%08x\n",
		   INREG(R128_GUI_STAT) & R128_GUI_FIFOCNT_MASK,
		   INREG(R128_GUI_STAT),
		   INREG(R128_GUI_PROBE)));
	R128VERBOSE(("FIFO timed out, resetting engine...\n"));
	R128EngineReset();
    }
}

/* Wait for the graphics engine to be completely idle: the FIFO has
   drained, the Pixel Cache is flushed, and the engine is idle.  This is a
   standard "sync" function that will make the hardware "quiescent". */
static void R128WaitForIdle(void)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;
    int           i;

    R128WaitForFifo(64);

    for (;;) {
	for (i = 0; i < R128_TIMEOUT; i++) {
	    if (!(INREG(R128_GUI_STAT) & R128_GUI_ACTIVE)) {
		R128EngineFlush();
		return;
	    }
	}
	R128DEBUG(("Idle timed out: %d entries, stat=0x%08x, probe=0x%08x\n",
		   INREG(R128_GUI_STAT) & R128_GUI_FIFOCNT_MASK,
		   INREG(R128_GUI_STAT),
		   INREG(R128_GUI_PROBE)));
	R128VERBOSE(("Idle timed out, resetting engine...\n"));
	R128EngineReset();
    }
}

/* Setup for XAA FillRectSolid. */
static void R128SetupForFillRectSolid(int color, int rop,
				      unsigned int planemask)
{
    R128InfoPtr   info      = R128PTR();
    unsigned char *R128MMIO = info->MMIO;

    R128WaitForFifo(4);
    OUTREG(R128_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				     | R128_GMC_BRUSH_SOLID_COLOR
				     | R128_GMC_SRC_DATATYPE_COLOR
				     | R128_ROP[rop].pattern
				     | R128_GMC_CLR_CMP_CNTL_DIS
				     | R128_AUX_CLIP_DIS));
    OUTREG(R128_DP_BRUSH_FRGD_CLR,  color);
    OUTREG(R128_DP_WRITE_MASK,      planemask);
    OUTREG(R128_DP_CNTL,            (R128_DST_X_LEFT_TO_RIGHT
				     | R128_DST_Y_TOP_TO_BOTTOM));
}

/* Subsequent XAA FillRectSolid. */
static void  R128SubsequentFillRectSolid(int x, int y, int w, int h)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;

    R128WaitForFifo(2);
    OUTREG(R128_DST_Y_X,          (y << 16) | x);
    OUTREG(R128_DST_WIDTH_HEIGHT, (w << 16) | h);
}

/* Setup for XAA screen-to-screen copy.  Transparency isn't handled in
   XFree86 3.3.x, but will probably be implemented in the XFree86 4.0
   version of this driver.  Further, making clipping available to XAA will
   also improve performance.  For XFree86 3.3.x, no clipping is used. */
static void R128SetupForScreenToScreenCopy(int xdir, int ydir, int rop,
					   unsigned int planemask,
					   int transparency_color)
{
    R128InfoPtr   info      = R128PTR();
    unsigned char *R128MMIO = info->MMIO;



    info->xdir = xdir;
    info->ydir = ydir;
    R128WaitForFifo(3);
    OUTREG(R128_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				     | R128_GMC_BRUSH_SOLID_COLOR
				     | R128_GMC_SRC_DATATYPE_COLOR
				     | R128_ROP[rop].rop
				     | R128_DP_SRC_SOURCE_MEMORY
				     | R128_GMC_CLR_CMP_CNTL_DIS
				     | R128_AUX_CLIP_DIS));
    OUTREG(R128_DP_WRITE_MASK,      planemask);
    OUTREG(R128_DP_CNTL,            ((xdir >= 0 ? R128_DST_X_LEFT_TO_RIGHT : 0)
				     | (ydir >= 0
					? R128_DST_Y_TOP_TO_BOTTOM
					: 0)));
}

/* Subsequent XAA screen-to-screen copy. */
static void R128SubsequentScreenToScreenCopy(int x1, int y1,
					     int x2, int y2,
					     int w, int h)
{
    R128InfoPtr   info      = R128PTR();
    unsigned char *R128MMIO = info->MMIO;

    if (info->xdir < 0) x1 += w - 1, x2 += w - 1;
    if (info->ydir < 0) y1 += h - 1, y2 += h - 1;
    
    R128WaitForFifo(3);
    OUTREG(R128_SRC_Y_X,          (y1 << 16) | x1);
    OUTREG(R128_DST_Y_X,          (y2 << 16) | x2);
    OUTREG(R128_DST_HEIGHT_WIDTH, (h << 16) | w);
}

/* Setup for XAA 8x8 pattern color expansion.  Patterns with transparency
   use `bg == -1'.  This routine is only used if the XAA pixmap cache is
   turned on.

   x11perf -osrect100:
                               1024x768@60Hz   1024x768@60Hz
			                8bpp           32bpp
   not used:                     35200.0/sec     12000.0/sec
   used:                         95500.0/sec     30100.0/sec 

   x11perf -srect100 (tests transparency):
                               1024x768@60Hz   1024x768@60Hz
			                8bpp           32bpp
   not used:                       585.0/sec        78.9/sec
   used:                         95800.0/sec     30400.0/sec
*/
static void R128SetupFor8x8PatternColorExpand(int patternx, int patterny,
					      int bg, int fg, int rop,
					      unsigned int planemask)
{
    R128InfoPtr   info           = R128PTR();
    unsigned char *R128MMIO      = info->MMIO;

    R128WaitForFifo(6);
    OUTREG(R128_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				     | (bg == -1
					? R128_GMC_BRUSH_8X8_MONO_FG_LA
					: R128_GMC_BRUSH_8X8_MONO_FG_BG)
				     | R128_ROP[rop].pattern
				     | R128_GMC_BYTE_LSB_TO_MSB
				     | R128_GMC_CLR_CMP_CNTL_DIS
				     | R128_AUX_CLIP_DIS));
    OUTREG(R128_DP_WRITE_MASK,      planemask);
    OUTREG(R128_DP_BRUSH_FRGD_CLR,  fg);
    OUTREG(R128_DP_BRUSH_BKGD_CLR,  bg);
    OUTREG(R128_BRUSH_DATA0,        patternx);
    OUTREG(R128_BRUSH_DATA1,        patterny);
}

/* Subsequent XAA 8x8 pattern color expansion.  Because they are used in
   the setup function, `patternx' and `patterny' are not used here. */
static void R128Subsequent8x8PatternColorExpand(int patternx, int patterny,
						int x, int y, int w, int h)
{
    unsigned char *R128MMIO = R128PTR()->MMIO;

    R128WaitForFifo(3);
    OUTREG(R128_BRUSH_Y_X,        (patterny << 8) | patternx);
    OUTREG(R128_DST_Y_X,          (y << 16) | x);
    OUTREG(R128_DST_HEIGHT_WIDTH, (h << 16) | w);
}

/* Setup for XAA scanline screen-to-screen color expansion.  Because of how
   the scratch buffer is initialized, this is really a mainstore-to-screen
   color expansion.  Transparency is supported when `bg == -1'.  For
   XFree86 4.0, the implementation of CPUToScreenColorExpand should be
   explored, although this will require changes in the XAA architecture to
   ensure that the last word of host data is written to
   R128_HOST_DATA_LAST. */
static void R128SetupForScanlineScreenToScreenColorExpand(int x, int y,
							  int w, int h,
							  int bg, int fg,
							  int rop,
							  unsigned planemask)
{
    R128InfoPtr   info           = R128PTR();
    unsigned char *R128MMIO      = info->MMIO;

    info->scanline_y     = y;
    info->scanline_x     = x;
    info->scanline_h_w   = (1 << 16) | w;
    info->scanline_words = (w + 31) / 32;

    R128WaitForFifo(4);
    OUTREG(R128_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				     | R128_GMC_BRUSH_1X8_COLOR
				     | (bg == -1
					? R128_GMC_SRC_DATATYPE_MONO_FG_LA
					: R128_GMC_SRC_DATATYPE_MONO_FG_BG)
				     | R128_ROP[rop].rop
				     | R128_GMC_BYTE_LSB_TO_MSB
				     | R128_DP_SRC_SOURCE_HOST_DATA
				     | R128_GMC_CLR_CMP_CNTL_DIS
				     | R128_AUX_CLIP_DIS));
    OUTREG(R128_DP_WRITE_MASK,      planemask);
    OUTREG(R128_DP_SRC_FRGD_CLR,    fg);
    OUTREG(R128_DP_SRC_BKGD_CLR,    bg);
}

/* Subsequent XAA scanline screen-to-screen color expansion.  Performance
   measures for this routine with and without R128_FAST_COLOR_EXPAND
   dramatically highlight why waiting for FIFO slots should be avoided as
   much as possible.

   x11perf -ftext:
                               1024x768@60Hz   1024x768@60Hz
			                8bpp           32bpp
   not used:                    379000.0/sec    507000.0/sec
   R128_FAST_COLOR_EXPAND = 0:  445000.0/sec    445000.0/sec
   R128_FAST_COLOR_EXPAND = 1: 1070000.0/sec   1070000.0/sec
*/
#define R128_FAST_COLOR_EXPAND 1
static void R128SubsequentScanlineScreenToScreenColorExpand(int srcAddr)
{
    R128InfoPtr     info           = R128PTR();
    unsigned char   *R128MMIO      = info->MMIO;
    CARD32          *p             = (CARD32 *)info->scratch_buffer;
    int             i;
#if R128_FAST_COLOR_EXPAND
    int             left           = info->scanline_words;
    volatile CARD32 *d;
#endif

    R128WaitForFifo(2);
    OUTREG(R128_DST_Y_X,            ((info->scanline_y++ << 16)
				     | info->scanline_x));
    OUTREG(R128_DST_HEIGHT_WIDTH,   info->scanline_h_w);

				/* Correct for new XAA offset calculation */
    p += ((srcAddr/8-1)/4);

#if R128_FAST_COLOR_EXPAND
    while (left) {
	if (left <= 9) {
	    R128WaitForFifo(left);
				/* Unrolling doesn't improve performance */
	    for (d = ADDRREG(R128_HOST_DATA_LAST) - (left - 1); left; --left)
		*d++ = *p++;
	} else {
	    R128WaitForFifo(8);
				/* Unrolling doesn't improve performance */
	    for (d = ADDRREG(R128_HOST_DATA0), i = 0; i < 8; i++)
		*d++ = *p++;
	    left -= 8;
	}
    }
#else
    for (i = 1; i < info->scanline_words; i++) {
	R128WaitForFifo(1);
	OUTREG(R128_HOST_DATA0, *p++);
    }
    R128WaitForFifo(1);
    OUTREG(R128_HOST_DATA_LAST, *p);
#endif
}

/* Initialize the acceleration hardware. */
static void R128EngineInit(void)
{
    R128InfoPtr   info      = R128PTR();
    unsigned char *R128MMIO = info->MMIO;

    R128DEBUG(("EngineInit (%d/%d)\n",
	       info->pixel_depth, info->pixel_bytes * 8));

    OUTREG(R128_SCALE_3D_CNTL, 0);
    R128EngineReset();

    switch (info->pixel_depth) {
    case 8:  info->datatype = 2; break;
    case 15: info->datatype = 3; break;
    case 16: info->datatype = 4; break;
    case 24: info->datatype = 5; break;
    case 32: info->datatype = 6; break;
    default:
	R128FATAL(("Unknown depth/bpp = %d/%d\n",
		   info->pixel_depth, info->pixel_bytes * 8));
    }
    info->pitch = (info->virtual_x / 8) * (info->pixel_bytes == 3 ? 3 : 1);

    R128DEBUG(("Pitch for acceleration = %d\n", info->pitch));

    R128WaitForFifo(2);
    OUTREG(R128_DEFAULT_OFFSET, 0);
    OUTREG(R128_DEFAULT_PITCH,  info->pitch);

    R128WaitForFifo(2);
    OUTREG(R128_AUX_SC_CNTL, 0); /* Needed? */
    OUTREG(R128_DEFAULT_SC_BOTTOM_RIGHT,
	   R128_DEFAULT_SC_RIGHT_MAX | R128_DEFAULT_SC_BOTTOM_MAX);

    info->dp_gui_master_cntl = info->datatype << R128_GMC_DST_DATATYPE_SHIFT;
    R128WaitForFifo(1);
    OUTREG(R128_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				     | R128_GMC_BRUSH_SOLID_COLOR
				     | R128_GMC_SRC_DATATYPE_COLOR));

    R128WaitForFifo(8);
    OUTREG(R128_DST_BRES_ERR,      0);
    OUTREG(R128_DST_BRES_INC,      0);
    OUTREG(R128_DST_BRES_DEC,      0);
    OUTREG(R128_DP_BRUSH_FRGD_CLR, 0xffffffff);
    OUTREG(R128_DP_BRUSH_BKGD_CLR, 0x00000000);
    OUTREG(R128_DP_SRC_FRGD_CLR,   0xffffffff);
    OUTREG(R128_DP_SRC_BKGD_CLR,   0x00000000);
    OUTREG(R128_DP_WRITE_MASK,     0xffffffff);

    R128WaitForIdle();
}

/* Initialize XAA for supported acceleration and also initialize the
   graphics hardware for acceleration. */
void R128AccelInit(void)
{
    R128InfoPtr          info      = R128PTR();
    xf86AccelInfoRecType *a        = &xf86AccelInfoRec;
    xf86GCInfoRecType    *g        = &xf86GCInfoRec;
    
    a->Flags = (BACKGROUND_OPERATIONS
		| PIXMAP_CACHE
		| COP_FRAMEBUFFER_CONCURRENCY
		| HARDWARE_PATTERN_PROGRAMMED_BITS
		| HARDWARE_PATTERN_PROGRAMMED_ORIGIN
		| HARDWARE_PATTERN_SCREEN_ORIGIN
		| HARDWARE_PATTERN_MONO_TRANSPARENCY);

				/* Sync */
    a->Sync                            = R128WaitForIdle;

				/* Solid Filled Rectangle */
    g->PolyFillRectSolidFlags          = 0;
    a->SetupForFillRectSolid           = R128SetupForFillRectSolid;
    a->SubsequentFillRectSolid         = R128SubsequentFillRectSolid;

				/* Screen-to-screen Copy */
    g->CopyAreaFlags                   = NO_TRANSPARENCY;
    a->SetupForScreenToScreenCopy      = R128SetupForScreenToScreenCopy;
    a->SubsequentScreenToScreenCopy    = R128SubsequentScreenToScreenCopy;

				/* 8x8 Pattern Color Expand */
    a->SetupFor8x8PatternColorExpand   = R128SetupFor8x8PatternColorExpand;
    a->Subsequent8x8PatternColorExpand = R128Subsequent8x8PatternColorExpand;
    a->ColorExpandFlags                = BIT_ORDER_IN_BYTE_LSBFIRST;

				/* Scanline Screen-To-Screen Color Expand */
    a->ScratchBufferAddr               = 1;
    a->ScratchBufferSize               = sizeof(info->scratch_buffer);
    a->ScratchBufferBase               = info->scratch_buffer;
    a->PingPongBuffers                 = 1;

    a->SetupForScanlineScreenToScreenColorExpand =
	R128SetupForScanlineScreenToScreenColorExpand;
    a->SubsequentScanlineScreenToScreenColorExpand =
	R128SubsequentScanlineScreenToScreenColorExpand;

				/* Pixmap Cache */
                                /* x11perf -copypixpix100
				   without: 14000.0/sec
				   with:    13300.0/sec
				   
				   This enables other routines, and is
				   generally beneficial.
				 */
    if (info->cursor_end) {
	a->PixmapCacheMemoryStart      = R128_ALIGN(info->cursor_end, 16);
    } else {
	a->PixmapCacheMemoryStart      = R128_ALIGN(info->virtual_y
						    * info->virtual_x
						    * info->pixel_bytes,
						    16);
    } 
    a->PixmapCacheMemoryEnd            = info->video_ram * 1024;

    R128EngineInit();
}
