/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/radeon_accel.c,v 1.4 2000/11/18 19:37:12 tsi Exp $ */
/*
 * Copyright 2000 ATI Technologies Inc., Markham, Ontario, and
 *                VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation on the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial
 * portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL ATI, VA LINUX SYSTEMS AND/OR
 * THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Rickard E. Faith <faith@valinux.com>
 *   Alan Hourihane <ahourihane@valinux.com>
 *
 * Credits:
 *
 *   Thanks to Ani Joshi <ajoshi@shell.unixbox.com> for providing source
 *   code to his Radeon driver.  Portions of this file are based on the
 *   initialization code for that driver.
 *
 * References:
 *
 * !!!! FIXME !!!!
 *   RAGE 128 VR/ RAGE 128 GL Register Reference Manual (Technical
 *   Reference Manual P/N RRG-G04100-C Rev. 0.04), ATI Technologies: April
 *   1999.
 *
 *   RAGE 128 Software Development Manual (Technical Reference Manual P/N
 *   SDK-G04000 Rev. 0.01), ATI Technologies: June 1999.
 *
 * Notes on unimplemented XAA optimizations:
 *
 *   SetClipping:   This has been removed as XAA expects 16bit registers
 *                  for full clipping.
 *   TwoPointLine:  The Radeon supports this. Not Bresenham.
 *   DashedLine with non-power-of-two pattern length: Apparently, there is
 *                  no way to set the length of the pattern -- it is always
 *                  assumed to be 8 or 32 (or 1024?).
 *   ScreenToScreenColorExpandFill: See p. 4-17 of the Technical Reference
 *                  Manual where it states that monochrome expansion of frame
 *                  buffer data is not supported.
 *   CPUToScreenColorExpandFill, direct: The implementation here uses a hybrid
 *                  direct/indirect method.  If we had more data registers,
 *                  then we could do better.  If XAA supported a trigger write
 *                  address, the code would be simpler.
 *   Color8x8PatternFill: Apparently, an 8x8 color brush cannot take an 8x8
 *                  pattern from frame buffer memory.
 *   ImageWrites:   Same as CPUToScreenColorExpandFill
 *
 */

#define RADEON_IMAGEWRITE 0     /* Turned off by default - slower in accel */

				/* Driver data structures */
#include "radeon.h"
#include "radeon_reg.h"
#ifdef XF86DRI
#define _XF86DRI_SERVER_
#include "r128_dri.h"
#endif

				/* Line support */
#include "miline.h"

				/* X and server generic header files */
#include "xf86.h"

static struct {
    int rop;
    int pattern;
} RADEON_ROP[] = {
    { RADEON_ROP3_ZERO, RADEON_ROP3_ZERO }, /* GXclear        */
    { RADEON_ROP3_DSa,  RADEON_ROP3_DPa  }, /* Gxand          */
    { RADEON_ROP3_SDna, RADEON_ROP3_PDna }, /* GXandReverse   */
    { RADEON_ROP3_S,    RADEON_ROP3_P    }, /* GXcopy         */
    { RADEON_ROP3_DSna, RADEON_ROP3_DPna }, /* GXandInverted  */
    { RADEON_ROP3_D,    RADEON_ROP3_D    }, /* GXnoop         */
    { RADEON_ROP3_DSx,  RADEON_ROP3_DPx  }, /* GXxor          */
    { RADEON_ROP3_DSo,  RADEON_ROP3_DPo  }, /* GXor           */
    { RADEON_ROP3_DSon, RADEON_ROP3_DPon }, /* GXnor          */
    { RADEON_ROP3_DSxn, RADEON_ROP3_PDxn }, /* GXequiv        */
    { RADEON_ROP3_Dn,   RADEON_ROP3_Dn   }, /* GXinvert       */
    { RADEON_ROP3_SDno, RADEON_ROP3_PDno }, /* GXorReverse    */
    { RADEON_ROP3_Sn,   RADEON_ROP3_Pn   }, /* GXcopyInverted */
    { RADEON_ROP3_DSno, RADEON_ROP3_DPno }, /* GXorInverted   */
    { RADEON_ROP3_DSan, RADEON_ROP3_DPan }, /* GXnand         */
    { RADEON_ROP3_ONE,  RADEON_ROP3_ONE  }  /* GXset          */
};

/* Flush all dirty data in the Pixel Cache to memory. */
void RADEONEngineFlush(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int           i;

    OUTREGP(RADEON_RB2D_DSTCACHE_CTLSTAT, RADEON_RB2D_DC_FLUSH_ALL,
						~RADEON_RB2D_DC_FLUSH_ALL);
    for (i = 0; i < RADEON_TIMEOUT; i++) {
	if (!(INREG(RADEON_RB2D_DSTCACHE_CTLSTAT) & RADEON_RB2D_DC_BUSY)) break;
    }
}

/* Reset graphics card to known state. */
void RADEONEngineReset(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    CARD32        clock_cntl_index;
    CARD32        mclk_cntl;
    CARD32        rbbm_soft_reset;

    RADEONEngineFlush(pScrn);

    clock_cntl_index = INREG(RADEON_CLOCK_CNTL_INDEX);
    mclk_cntl        = INPLL(pScrn, RADEON_MCLK_CNTL);

    OUTPLL(RADEON_MCLK_CNTL, (mclk_cntl | 0x003f0000));

    rbbm_soft_reset   = INREG(RADEON_RBBM_SOFT_RESET);

    OUTREG(RADEON_RBBM_SOFT_RESET, rbbm_soft_reset |
				   RADEON_SOFT_RESET_CP |
				   RADEON_SOFT_RESET_HI |
				   RADEON_SOFT_RESET_SE |
				   RADEON_SOFT_RESET_RE |
				   RADEON_SOFT_RESET_PP |
				   RADEON_SOFT_RESET_E2 |
				   RADEON_SOFT_RESET_RB |
				   RADEON_SOFT_RESET_HDP);
    INREG(RADEON_RBBM_SOFT_RESET);
    OUTREG(RADEON_RBBM_SOFT_RESET, rbbm_soft_reset &
				 ~(RADEON_SOFT_RESET_CP |
				   RADEON_SOFT_RESET_HI |
				   RADEON_SOFT_RESET_SE |
				   RADEON_SOFT_RESET_RE |
				   RADEON_SOFT_RESET_PP |
				   RADEON_SOFT_RESET_E2 |
				   RADEON_SOFT_RESET_RB |
				   RADEON_SOFT_RESET_HDP));
    INREG(RADEON_RBBM_SOFT_RESET);

    OUTPLL(RADEON_MCLK_CNTL,        mclk_cntl);
    OUTREG(RADEON_CLOCK_CNTL_INDEX, clock_cntl_index);
    OUTREG(RADEON_RBBM_SOFT_RESET,  rbbm_soft_reset);

#ifdef XF86DRI
    if (RADEONCP_USE_RING_BUFFER(info->CPMode)) RADEONCPResetRing(pScrn);
#endif
}

/* The FIFO has 64 slots.  This routines waits until at least `entries' of
   these slots are empty. */
void RADEONWaitForFifoFunction(ScrnInfoPtr pScrn, int entries)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int           i;

    for (;;) {
	for (i = 0; i < RADEON_TIMEOUT; i++) {
	    info->fifo_slots =
		INREG(RADEON_RBBM_STATUS) & RADEON_RBBM_FIFOCNT_MASK;
	    if (info->fifo_slots >= entries) return;
	}
	RADEONTRACE(("FIFO timed out: %d entries, stat=0x%08x\n",
		     INREG(RADEON_RBBM_STATUS) & RADEON_RBBM_FIFOCNT_MASK,
		     INREG(RADEON_RBBM_STATUS)));
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "FIFO timed out, resetting engine...\n");
	RADEONEngineInit(pScrn);
#ifdef XF86DRI
	if (info->CP2D) RADEONCPStart(pScrn);
#endif
    }
}

/* Wait for the graphics engine to be completely idle: the FIFO has
   drained, the Pixel Cache is flushed, and the engine is idle.  This is a
   standard "sync" function that will make the hardware "quiescent". */
void RADEONWaitForIdle(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int           i;

    RADEONTRACE(("WaitForIdle (entering): %d entries, stat=0x%08x\n",
		     INREG(RADEON_RBBM_STATUS) & RADEON_RBBM_FIFOCNT_MASK,
		     INREG(RADEON_RBBM_STATUS)));

    RADEONWaitForFifoFunction(pScrn, 64);

    for (;;) {
	for (i = 0; i < RADEON_TIMEOUT; i++) {
	    if (!(INREG(RADEON_RBBM_STATUS) & RADEON_RBBM_ACTIVE)) {
		RADEONEngineFlush(pScrn);
		return;
	    }
	}
	RADEONTRACE(("Idle timed out: %d entries, stat=0x%08x\n",
		     INREG(RADEON_RBBM_STATUS) & RADEON_RBBM_FIFOCNT_MASK,
		     INREG(RADEON_RBBM_STATUS)));
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Idle timed out, resetting engine...\n");
	RADEONEngineInit(pScrn);
#ifdef XF86DRI
	if (info->CP2D) RADEONCPStart(pScrn);
#endif
    }
}

/* Setup for XAA SolidFill. */
static void RADEONSetupForSolidFill(ScrnInfoPtr pScrn,
				    int color, int rop, unsigned int planemask)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    RADEONWaitForFifo(pScrn, 4);
    OUTREG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				       | RADEON_GMC_BRUSH_SOLID_COLOR
				       | RADEON_GMC_SRC_DATATYPE_COLOR
				       | RADEON_ROP[rop].pattern));
    OUTREG(RADEON_DP_BRUSH_FRGD_CLR,  color);
    OUTREG(RADEON_DP_WRITE_MASK,      planemask);
    OUTREG(RADEON_DP_CNTL,            (RADEON_DST_X_LEFT_TO_RIGHT
				       | RADEON_DST_Y_TOP_TO_BOTTOM));
}

/* Subsequent XAA SolidFillRect.

   Tests: xtest CH06/fllrctngl, xterm
*/
static void  RADEONSubsequentSolidFillRect(ScrnInfoPtr pScrn,
					   int x, int y, int w, int h)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    RADEONWaitForFifo(pScrn, 2);
    OUTREG(RADEON_DST_Y_X,          (y << 16) | x);
    OUTREG(RADEON_DST_WIDTH_HEIGHT, (w << 16) | h);
}

/* Setup for XAA solid lines. */
static void RADEONSetupForSolidLine(ScrnInfoPtr pScrn,
				    int color, int rop, unsigned int planemask)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    RADEONWaitForFifo(pScrn, 3);
    OUTREG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				       | RADEON_GMC_BRUSH_SOLID_COLOR
				       | RADEON_GMC_SRC_DATATYPE_COLOR
				       | RADEON_ROP[rop].pattern));
    OUTREG(RADEON_DP_BRUSH_FRGD_CLR,  color);
    OUTREG(RADEON_DP_WRITE_MASK,      planemask);
}


/* Subsequent XAA solid TwoPointLine line.

   Tests: xtest CH06/drwln, ico, Mark Vojkovich's linetest program

   [See http://www.xfree86.org/devel/archives/devel/1999-Jun/0102.shtml for
   Mark Vojkovich's linetest program, posted 2Jun99 to devel@xfree86.org.]
*/
static void RADEONSubsequentSolidTwoPointLine(ScrnInfoPtr pScrn,
					      int xa, int ya, int xb, int yb,
					      int flags)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int           direction   = 0;

    if (xa < xb) direction |= RADEON_DST_X_DIR_LEFT_TO_RIGHT;
    if (ya < yb) direction |= RADEON_DST_Y_DIR_TOP_TO_BOTTOM;

    RADEONWaitForFifo(pScrn, 4);
    OUTREG(RADEON_DST_Y_X,                  (ya << 16) | xa);
    if (!(flags & OMIT_LAST))
	OUTREG(RADEON_DP_CNTL_XDIR_YDIR_YMAJOR, direction);
    OUTREG(RADEON_DST_LINE_START,           (ya << 16) | xa);
    OUTREG(RADEON_DST_LINE_END,             (yb << 16) | xb);
}

/* Subsequent XAA solid horizontal and vertical lines */
static void RADEONSubsequentSolidHorVertLine(ScrnInfoPtr pScrn,
					     int x, int y, int len, int dir )
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    RADEONWaitForFifo(pScrn, 1);
    OUTREG(RADEON_DP_CNTL, (RADEON_DST_X_LEFT_TO_RIGHT
			    | RADEON_DST_Y_TOP_TO_BOTTOM));

    if (dir == DEGREES_0) {
	RADEONSubsequentSolidFillRect(pScrn, x, y, len, 1);
    } else {
	RADEONSubsequentSolidFillRect(pScrn, x, y, 1, len);
    }
}

/* Setup for XAA dashed lines.

   Tests: xtest CH05/stdshs, XFree86/drwln

   NOTE: Since we can only accelerate lines with power-of-2 patterns of
   length <= 32.
*/
static void RADEONSetupForDashedLine(ScrnInfoPtr pScrn,
				     int fg, int bg,
				     int rop, unsigned int planemask,
				     int length, unsigned char *pattern)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    CARD32        pat         = *(CARD32 *)pattern;

    switch (length) {
    case  2: pat |= pat <<  2;  /* fall through */
    case  4: pat |= pat <<  4;  /* fall through */
    case  8: pat |= pat <<  8;  /* fall through */
    case 16: pat |= pat << 16;
    }

    RADEONWaitForFifo(pScrn, 5);
    OUTREG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				       | (bg == -1
					  ? RADEON_GMC_BRUSH_32x1_MONO_FG_LA
					  : RADEON_GMC_BRUSH_32x1_MONO_FG_BG)
				       | RADEON_ROP[rop].pattern
				       | RADEON_GMC_BYTE_LSB_TO_MSB));
    OUTREG(RADEON_DP_WRITE_MASK,      planemask);
    OUTREG(RADEON_DP_BRUSH_FRGD_CLR,  fg);
    OUTREG(RADEON_DP_BRUSH_BKGD_CLR,  bg);
    OUTREG(RADEON_BRUSH_DATA0,        pat);
}

/* Subsequent XAA dashed line. */
static void RADEONSubsequentDashedTwoPointLine(ScrnInfoPtr pScrn,
						int xa, int ya,
						int xb, int yb,
						int flags,
						int phase)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int           direction   = 0;

    if (xa < xb) direction |= RADEON_DST_X_DIR_LEFT_TO_RIGHT;
    if (ya < yb) direction |= RADEON_DST_Y_DIR_TOP_TO_BOTTOM;

    RADEONWaitForFifo(pScrn, 5);
    if (!(flags & OMIT_LAST))
	OUTREG(RADEON_DP_CNTL_XDIR_YDIR_YMAJOR, direction);
    OUTREG(RADEON_DST_Y_X,          (ya << 16) | xa);
    OUTREG(RADEON_BRUSH_Y_X,        (phase << 16) | phase);
    OUTREG(RADEON_DST_LINE_START,   (ya << 16) | xa);
    OUTREG(RADEON_DST_LINE_END,     (yb << 16) | xb);
}

/* Setup for XAA screen-to-screen copy.

   Tests: xtest CH06/fllrctngl (also tests transparency).
*/
static void RADEONSetupForScreenToScreenCopy(ScrnInfoPtr pScrn,
					     int xdir, int ydir, int rop,
					     unsigned int planemask,
					     int trans_color)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    info->xdir = xdir;
    info->ydir = ydir;
    RADEONWaitForFifo(pScrn, 3);
    OUTREG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				       | RADEON_GMC_BRUSH_NONE
				       | RADEON_GMC_SRC_DATATYPE_COLOR
				       | RADEON_ROP[rop].rop
				       | RADEON_DP_SRC_SOURCE_MEMORY));
    OUTREG(RADEON_DP_WRITE_MASK,      planemask);
    OUTREG(RADEON_DP_CNTL,            ((xdir >= 0
					? RADEON_DST_X_LEFT_TO_RIGHT
					: 0)
				       | (ydir >= 0
					  ? RADEON_DST_Y_TOP_TO_BOTTOM
					  : 0)));

    if (trans_color != -1) {
				/* Set up for transparency */
	RADEONWaitForFifo(pScrn, 3);
	OUTREG(RADEON_CLR_CMP_CLR_SRC, trans_color);
	OUTREG(RADEON_CLR_CMP_MASK,    RADEON_CLR_CMP_MSK);
	/* Mmmm, Seems as though the transparency compare is opposite to r128
	 * It should only draw when source != trans_color,
	 * this is the opposite of that. */
	OUTREG(RADEON_CLR_CMP_CNTL,    (RADEON_SRC_CMP_EQ_COLOR
					| RADEON_CLR_CMP_SRC_SOURCE));
    }
}

/* Subsequent XAA screen-to-screen copy. */
static void RADEONSubsequentScreenToScreenCopy(ScrnInfoPtr pScrn,
					       int xa, int ya,
					       int xb, int yb,
					       int w, int h)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    if (info->xdir < 0) xa += w - 1, xb += w - 1;
    if (info->ydir < 0) ya += h - 1, yb += h - 1;

    RADEONWaitForFifo(pScrn, 3);
    OUTREG(RADEON_SRC_Y_X,          (ya << 16) | xa);
    OUTREG(RADEON_DST_Y_X,          (yb << 16) | xb);
    OUTREG(RADEON_DST_HEIGHT_WIDTH, (h << 16) | w);
}

/* Setup for XAA mono 8x8 pattern color expansion.  Patterns with
   transparency use `bg == -1'.  This routine is only used if the XAA
   pixmap cache is turned on.

   Tests: xtest XFree86/fllrctngl (no other test will test this routine with
                                   both transparency and non-transparency)
*/
static void RADEONSetupForMono8x8PatternFill(ScrnInfoPtr pScrn,
					     int patternx, int patterny,
					     int fg, int bg, int rop,
					     unsigned int planemask)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    RADEONWaitForFifo(pScrn, 6);
    OUTREG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				       | (bg == -1
					  ? RADEON_GMC_BRUSH_8X8_MONO_FG_LA
					  : RADEON_GMC_BRUSH_8X8_MONO_FG_BG)
				       | RADEON_ROP[rop].pattern
				       | RADEON_GMC_BYTE_LSB_TO_MSB));
    OUTREG(RADEON_DP_WRITE_MASK,      planemask);
    OUTREG(RADEON_DP_BRUSH_FRGD_CLR,  fg);
    OUTREG(RADEON_DP_BRUSH_BKGD_CLR,  bg);
    OUTREG(RADEON_BRUSH_DATA0,        patternx);
    OUTREG(RADEON_BRUSH_DATA1,        patterny);
}

/* Subsequent XAA 8x8 pattern color expansion.  Because they are used in
   the setup function, `patternx' and `patterny' are not used here. */
static void RADEONSubsequentMono8x8PatternFillRect(ScrnInfoPtr pScrn,
						   int patternx, int patterny,
						   int x, int y, int w, int h)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    RADEONWaitForFifo(pScrn, 3);
    OUTREG(RADEON_BRUSH_Y_X,        (patterny << 8) | patternx);
    OUTREG(RADEON_DST_Y_X,          (y << 16) | x);
    OUTREG(RADEON_DST_HEIGHT_WIDTH, (h << 16) | w);
}

#if 0
/* Setup for XAA color 8x8 pattern fill.

   Tests: xtest XFree86/fllrctngl (with Mono8x8PatternFill off)
*/
static void RADEONSetupForColor8x8PatternFill(ScrnInfoPtr pScrn,
					      int patx, int paty,
					      int rop, unsigned int planemask,
					      int trans_color)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    ErrorF("Color8x8 %d %d %d\n", trans_color, patx, paty);

    RADEONWaitForFifo(pScrn, 3);
    OUTREG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				       | RADEON_GMC_BRUSH_8x8_COLOR
				       | RADEON_GMC_SRC_DATATYPE_COLOR
				       | RADEON_ROP[rop].pattern
				       | RADEON_DP_SRC_SOURCE_MEMORY));
    OUTREG(RADEON_DP_WRITE_MASK,      planemask);
    OUTREG(RADEON_SRC_Y_X, (paty << 16) | patx);

    if (trans_color != -1) {
				/* Set up for transparency */
	RADEONWaitForFifo(pScrn, 3);
	OUTREG(RADEON_CLR_CMP_CLR_SRC, trans_color);
	OUTREG(RADEON_CLR_CMP_MASK,    RADEON_CLR_CMP_MSK);
	/* Mmmm, Seems as though the transparency compare is opposite to r128
	 * It should only draw when source != trans_color,
	 * this is the opposite of that. */
	OUTREG(RADEON_CLR_CMP_CNTL,    (RADEON_SRC_CMP_EQ_COLOR
					| RADEON_CLR_CMP_SRC_SOURCE));
    }
}

/* Subsequent XAA 8x8 pattern color expansion. */
static void RADEONSubsequentColor8x8PatternFillRect(ScrnInfoPtr pScrn,
						    int patx, int paty,
						    int x, int y, int w, int h)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    ErrorF("Color8x8 %d,%d %d,%d %d %d\n", patx, paty, x, y, w, h);

    RADEONWaitForFifo(pScrn, 4);
    OUTREG(RADEON_BRUSH_Y_X, (paty << 16) | patx);
    OUTREG(RADEON_DST_Y_X, (y << 16) | x);
    OUTREG(RADEON_DST_HEIGHT_WIDTH, (h << 16) | w);
}
#endif

/* Setup for XAA indirect CPU-to-screen color expansion (indirect).
   Because of how the scratch buffer is initialized, this is really a
   mainstore-to-screen color expansion.  Transparency is supported when `bg
   == -1'.
   Implementing the hybrid indirect/direct scheme improved performance in a
   few areas:
*/
static void RADEONSetupForScanlineCPUToScreenColorExpandFill(ScrnInfoPtr pScrn,
							     int fg, int bg,
							     int rop,
							     unsigned int
							     planemask)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    RADEONWaitForFifo(pScrn, 4);
    OUTREG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				       | RADEON_GMC_DST_CLIPPING
				       | RADEON_GMC_BRUSH_NONE
				       | (bg == -1
					  ? RADEON_GMC_SRC_DATATYPE_MONO_FG_LA
					  : RADEON_GMC_SRC_DATATYPE_MONO_FG_BG)
				       | RADEON_ROP[rop].rop
				       | RADEON_GMC_BYTE_LSB_TO_MSB
				       | RADEON_DP_SRC_SOURCE_HOST_DATA));
    OUTREG(RADEON_DP_WRITE_MASK,      planemask);
    OUTREG(RADEON_DP_SRC_FRGD_CLR,    fg);
    OUTREG(RADEON_DP_SRC_BKGD_CLR,    bg);
}

/* Subsequent XAA indirect CPU-to-screen color expansion.  This is only
   called once for each rectangle. */
static void RADEONSubsequentScanlineCPUToScreenColorExpandFill(ScrnInfoPtr
							       pScrn,
							       int x, int y,
							       int w, int h,
							       int skipleft)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    info->scanline_h      = h;
    info->scanline_words  = (w + 31) >> 5;

    if ((info->scanline_words * h) <= 9) {
	/* Turn on direct for less than 9 dword colour expansion */
	info->scratch_buffer[0]
	    = (unsigned char *)(ADDRREG(RADEON_HOST_DATA_LAST)
				- (info->scanline_words - 1));
	info->scanline_direct = 1;
    } else {
	/* Use indirect for anything else */
	info->scratch_buffer[0]            = info->scratch_save;
	info->scanline_direct = 0;
    }

    RADEONWaitForFifo(pScrn, 4 + (info->scanline_direct ?
					(info->scanline_words * h) : 0) );
    OUTREG(RADEON_SC_TOP_LEFT,      (y << 16)       | ((x+skipleft) & 0xffff));
    /* MMmm, we don't need the -1 on both y+h or x+w, why ? */
    OUTREG(RADEON_SC_BOTTOM_RIGHT,  ((y+h) << 16)   | ((x+w) & 0xffff));
    OUTREG(RADEON_DST_Y_X,          (y << 16)       | (x & 0xffff));
    /* Have to pad the width here and use clipping engine */
    OUTREG(RADEON_DST_HEIGHT_WIDTH, (h << 16)       | ((w + 31) & ~31));
}

/* Subsequent XAA indirect CPU-to-screen color expandion.  This is called
   once for each scanline. */
static void RADEONSubsequentColorExpandScanline(ScrnInfoPtr pScrn, int bufno)
{
    RADEONInfoPtr   info        = RADEONPTR(pScrn);
    unsigned char   *RADEONMMIO = info->MMIO;
    CARD32          *p          = (CARD32 *)info->scratch_buffer[bufno];
    int             i;
    int             left        = info->scanline_words;
    volatile CARD32 *d;

    if (info->scanline_direct) return;
    --info->scanline_h;
    while (left) {
	if (left <= 8) {
	  /* Last scanline - finish write to DATA_LAST */
	  if (info->scanline_h == 0) {
	    RADEONWaitForFifo(pScrn, left);
				/* Unrolling doesn't improve performance */
	    for (d = ADDRREG(RADEON_HOST_DATA_LAST) - (left - 1); left; --left)
		*d++ = *p++;
	    return;
	  } else {
	    RADEONWaitForFifo(pScrn, left);
				/* Unrolling doesn't improve performance */
	    for (d = ADDRREG(RADEON_HOST_DATA7) - (left - 1); left; --left)
		*d++ = *p++;
	  }
	} else {
	    RADEONWaitForFifo(pScrn, 8);
				/* Unrolling doesn't improve performance */
	    for (d = ADDRREG(RADEON_HOST_DATA0), i = 0; i < 8; i++)
		*d++ = *p++;
	    left -= 8;
	}
    }
}

#if RADEON_IMAGEWRITE
/* Setup for XAA indirect image write. */
static void RADEONSetupForScanlineImageWrite(ScrnInfoPtr pScrn,
					     int rop,
					     unsigned int planemask,
					     int trans_color,
					     int bpp,
					     int depth)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    info->scanline_bpp = bpp;

    RADEONWaitForFifo(pScrn, 2);
    OUTREG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				       | RADEON_GMC_DST_CLIPPING
				       | RADEON_GMC_BRUSH_NONE
				       | RADEON_GMC_SRC_DATATYPE_COLOR
				       | RADEON_ROP[rop].rop
				       | RADEON_GMC_BYTE_LSB_TO_MSB
				       | RADEON_DP_SRC_SOURCE_HOST_DATA));
    OUTREG(RADEON_DP_WRITE_MASK,      planemask);

    if (trans_color != -1) {
				/* Set up for transparency */
	RADEONWaitForFifo(pScrn, 3);
	OUTREG(RADEON_CLR_CMP_CLR_SRC, trans_color);
	OUTREG(RADEON_CLR_CMP_MASK,    RADEON_CLR_CMP_MSK);
	/* Mmmm, Seems as though the transparency compare is opposite to r128
	 * It should only draw when source != trans_color,
	 * this is the opposite of that. */
	OUTREG(RADEON_CLR_CMP_CNTL,    (RADEON_SRC_CMP_EQ_COLOR
					| RADEON_CLR_CMP_SRC_SOURCE));
    }
}

/* Subsequent XAA indirect image write. This is only called once for each
   rectangle. */
static void RADEONSubsequentScanlineImageWriteRect(ScrnInfoPtr pScrn,
						   int x, int y,
						   int w, int h,
						   int skipleft)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int shift = 0; /* 32bpp */

    if (pScrn->bitsPerPixel == 8) shift = 3;
    else if (pScrn->bitsPerPixel == 16) shift = 1;

    info->scanline_h      = h;
    info->scanline_words  = (w * info->scanline_bpp + 31) >> 5;

    if ((info->scanline_words * h) <= 9) {
	/* Turn on direct for less than 9 dword colour expansion */
	info->scratch_buffer[0]
	    = (unsigned char *)(ADDRREG(RADEON_HOST_DATA_LAST)
				- (info->scanline_words - 1));
	info->scanline_direct = 1;
    } else {
	/* Use indirect for anything else */
	info->scratch_buffer[0]            = info->scratch_save;
	info->scanline_direct = 0;
    }

    RADEONWaitForFifo(pScrn, 4 + (info->scanline_direct ?
					(info->scanline_words * h) : 0) );
    OUTREG(RADEON_SC_TOP_LEFT,      (y << 16)       | ((x+skipleft) & 0xffff));
    /* MMmm, we don't need the -1 on both y+h or x+w, why ? */
    OUTREG(RADEON_SC_BOTTOM_RIGHT,  ((y+h) << 16)   | ((x+w) & 0xffff));
    OUTREG(RADEON_DST_Y_X,          (y << 16)       | (x & 0xffff));
    /* Have to pad the width here and use clipping engine */
    OUTREG(RADEON_DST_HEIGHT_WIDTH, (h << 16)       | ((w + shift) & ~shift));
}

/* Subsequent XAA indirect image write.  This is called once for each
   scanline. */
static void RADEONSubsequentImageWriteScanline(ScrnInfoPtr pScrn, int bufno)
{
    RADEONInfoPtr   info        = RADEONPTR(pScrn);
    unsigned char   *RADEONMMIO = info->MMIO;
    CARD32          *p          = (CARD32 *)info->scratch_buffer[bufno];
    int             i;
    int             left        = info->scanline_words;
    volatile CARD32 *d;

    if (info->scanline_direct) return;
    --info->scanline_h;
    while (left) {
	if (left <= 8) {
	  /* Last scanline - finish write to DATA_LAST */
	  if (info->scanline_h == 0) {
	    RADEONWaitForFifo(pScrn, left);
				/* Unrolling doesn't improve performance */
	    for (d = ADDRREG(RADEON_HOST_DATA_LAST) - (left - 1); left; --left)
		*d++ = *p++;
	    return;
	  } else {
	    RADEONWaitForFifo(pScrn, left);
				/* Unrolling doesn't improve performance */
	    for (d = ADDRREG(RADEON_HOST_DATA7) - (left - 1); left; --left)
		*d++ = *p++;
	  }
	} else {
	    RADEONWaitForFifo(pScrn, 8);
				/* Unrolling doesn't improve performance */
	    for (d = ADDRREG(RADEON_HOST_DATA0), i = 0; i < 8; i++)
		*d++ = *p++;
	    left -= 8;
	}
    }
}
#endif

/* Initialize the acceleration hardware. */
void RADEONEngineInit(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int pitch64;

    RADEONTRACE(("EngineInit (%d/%d)\n",
		 info->CurrentLayout.pixel_code,
		 info->CurrentLayout.bitsPerPixel));

    OUTREG(RADEON_SCALE_3D_CNTL, 0);
    RADEONEngineReset(pScrn);

    RADEONWaitForFifo(pScrn, 1);
    /* turn of all automatic flushing - we'll do it all */
    OUTREG(RADEON_RB2D_DSTCACHE_MODE, 0);

    switch (info->CurrentLayout.pixel_code) {
    case 8:  info->datatype = 2; break;
    case 15: info->datatype = 3; break;
    case 16: info->datatype = 4; break;
    case 24: info->datatype = 5; break;
    case 32: info->datatype = 6; break;
    default:
	RADEONTRACE(("Unknown depth/bpp = %d/%d (code = %d)\n",
		     info->CurrentLayout.depth,
		     info->CurrentLayout.bitsPerPixel,
		     info->CurrentLayout.pixel_code));
    }
    info->pitch = ((info->CurrentLayout.displayWidth / 8) *
		   (info->CurrentLayout.pixel_bytes == 3 ? 3 : 1));

    RADEONTRACE(("Pitch for acceleration = %d\n", info->pitch));

    pitch64 = ((pScrn->displayWidth * (pScrn->bitsPerPixel / 8) + 0x3f)) >> 6;

    RADEONWaitForFifo(pScrn, 1);
    OUTREG(RADEON_DEFAULT_OFFSET, (INREG(RADEON_DEFAULT_OFFSET) & 0xC0000000) |
				  (pitch64 << 22));

    RADEONWaitForFifo(pScrn, 1);
#if X_BYTE_ORDER == X_BIG_ENDIAN
    OUTREGP(RADEON_DP_DATATYPE,
	    RADEON_HOST_BIG_ENDIAN_EN, ~RADEON_HOST_BIG_ENDIAN_EN);
#else
    OUTREGP(RADEON_DP_DATATYPE, 0, ~RADEON_HOST_BIG_ENDIAN_EN);
#endif

    RADEONWaitForFifo(pScrn, 1);
    OUTREG(RADEON_DEFAULT_SC_BOTTOM_RIGHT, (RADEON_DEFAULT_SC_RIGHT_MAX
					    | RADEON_DEFAULT_SC_BOTTOM_MAX));
    info->dp_gui_master_cntl =
	((info->datatype << RADEON_GMC_DST_DATATYPE_SHIFT)
	 | RADEON_GMC_CLR_CMP_CNTL_DIS);
    RADEONWaitForFifo(pScrn, 1);
    OUTREG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				       | RADEON_GMC_BRUSH_SOLID_COLOR
				       | RADEON_GMC_SRC_DATATYPE_COLOR));

    RADEONWaitForFifo(pScrn, 7);
    OUTREG(RADEON_DST_LINE_START,    0);
    OUTREG(RADEON_DST_LINE_END,      0);
    OUTREG(RADEON_DP_BRUSH_FRGD_CLR, 0xffffffff);
    OUTREG(RADEON_DP_BRUSH_BKGD_CLR, 0x00000000);
    OUTREG(RADEON_DP_SRC_FRGD_CLR,   0xffffffff);
    OUTREG(RADEON_DP_SRC_BKGD_CLR,   0x00000000);
    OUTREG(RADEON_DP_WRITE_MASK,     0xffffffff);

    RADEONWaitForIdle(pScrn);
}

#ifdef XF86DRI
/* FIXME: When direct rendering is enabled, we should use the CP to
   draw 2D commands */
static void RADEONCPAccelInit(ScrnInfoPtr pScrn, XAAInfoRecPtr a)
{
    a->Flags                            = 0;

				/* Sync */
#if 1
    a->Sync                             = RADEONWaitForIdle;
#else
    a->Sync                             = RADEONCCEWaitForIdle;
#endif

}
#endif

static void RADEONMMIOAccelInit(ScrnInfoPtr pScrn, XAAInfoRecPtr a)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);

    a->Flags                            = (PIXMAP_CACHE
					   | OFFSCREEN_PIXMAPS
					   | LINEAR_FRAMEBUFFER);

				/* Sync */
    a->Sync                             = RADEONWaitForIdle;

				/* Solid Filled Rectangle */
    a->PolyFillRectSolidFlags           = 0;
    a->SetupForSolidFill                = RADEONSetupForSolidFill;
    a->SubsequentSolidFillRect          = RADEONSubsequentSolidFillRect;

				/* Screen-to-screen Copy */
    a->ScreenToScreenCopyFlags          = 0;
    a->SetupForScreenToScreenCopy       = RADEONSetupForScreenToScreenCopy;
    a->SubsequentScreenToScreenCopy     = RADEONSubsequentScreenToScreenCopy;

				/* Mono 8x8 Pattern Fill (Color Expand) */
    a->SetupForMono8x8PatternFill
	= RADEONSetupForMono8x8PatternFill;
    a->SubsequentMono8x8PatternFillRect
	= RADEONSubsequentMono8x8PatternFillRect;
    a->Mono8x8PatternFillFlags          = (HARDWARE_PATTERN_PROGRAMMED_BITS
					   | HARDWARE_PATTERN_PROGRAMMED_ORIGIN
					   | HARDWARE_PATTERN_SCREEN_ORIGIN
					   | BIT_ORDER_IN_BYTE_LSBFIRST);

				/* Indirect CPU-To-Screen Color Expand */
#if X_BYTE_ORDER == X_LITTLE_ENDIAN
    a->ScanlineCPUToScreenColorExpandFillFlags = LEFT_EDGE_CLIPPING
		/* RADEON gets upset, when using HOST provided data
		 * without a source rop. To show run 'xtest's drwarc */
					       | ROP_NEEDS_SOURCE
					       | LEFT_EDGE_CLIPPING_NEGATIVE_X;
#else
    a->ScanlineCPUToScreenColorExpandFillFlags = BIT_ORDER_IN_BYTE_MSBFIRST
		/* RADEON gets upset, when using HOST provided data
		 * without a source rop. To show run 'xtest's drwarc */
					       | ROP_NEEDS_SOURCE
					       | LEFT_EDGE_CLIPPING
					       | LEFT_EDGE_CLIPPING_NEGATIVE_X;
#endif
    a->NumScanlineColorExpandBuffers   = 1;
    a->ScanlineColorExpandBuffers      = info->scratch_buffer;
    info->scratch_save                 = xalloc(((pScrn->virtualX+31)/32*4)
					    + (pScrn->virtualX
					    * info->CurrentLayout.pixel_bytes));
    info->scratch_buffer[0]            = info->scratch_save;
    a->SetupForScanlineCPUToScreenColorExpandFill
	= RADEONSetupForScanlineCPUToScreenColorExpandFill;
    a->SubsequentScanlineCPUToScreenColorExpandFill
	= RADEONSubsequentScanlineCPUToScreenColorExpandFill;
    a->SubsequentColorExpandScanline   = RADEONSubsequentColorExpandScanline;

    a->SetupForSolidLine               = RADEONSetupForSolidLine;
    a->SubsequentSolidTwoPointLine     = RADEONSubsequentSolidTwoPointLine;
    a->SubsequentSolidHorVertLine      = RADEONSubsequentSolidHorVertLine;

    a->SetupForDashedLine              = RADEONSetupForDashedLine;
    a->SubsequentDashedTwoPointLine    = RADEONSubsequentDashedTwoPointLine;
    a->DashPatternMaxLength            = 32;
    a->DashedLineFlags                 = (LINE_PATTERN_LSBFIRST_LSBJUSTIFIED
					  | LINE_PATTERN_POWER_OF_2_ONLY);

#if RADEON_IMAGEWRITE
				/* ImageWrite */
    a->NumScanlineImageWriteBuffers    = 1;
    a->ScanlineImageWriteBuffers       = info->scratch_buffer;
    info->scratch_buffer[0]            = info->scratch_save;
    a->SetupForScanlineImageWrite      = RADEONSetupForScanlineImageWrite;
    a->SubsequentScanlineImageWriteRect
	= RADEONSubsequentScanlineImageWriteRect;
    a->SubsequentImageWriteScanline    = RADEONSubsequentImageWriteScanline;
    a->ScanlineImageWriteFlags           = CPU_TRANSFER_PAD_DWORD
		/* Performance tests show that we shouldn't use GXcopy for
		 * uploads as a memcpy is faster */
					 | NO_GXCOPY
		/* RADEON gets upset, when using HOST provided data
		 * without a source rop. To show run 'xtest's ptimg */
					 | ROP_NEEDS_SOURCE
					 | SCANLINE_PAD_DWORD
					 | LEFT_EDGE_CLIPPING
					 | LEFT_EDGE_CLIPPING_NEGATIVE_X;
#endif

#if 0
				/* Color 8x8 Pattern Fill */
    a->SetupForColor8x8PatternFill
	= RADEONSetupForColor8x8PatternFill;
    a->SubsequentColor8x8PatternFillRect
	= RADEONSubsequentColor8x8PatternFillRect;
    a->Color8x8PatternFillFlags          =
					    HARDWARE_PATTERN_PROGRAMMED_ORIGIN
					   | HARDWARE_PATTERN_SCREEN_ORIGIN
					   | BIT_ORDER_IN_BYTE_LSBFIRST;
#endif
}

/* Initialize XAA for supported acceleration and also initialize the
   graphics hardware for acceleration. */
Bool RADEONAccelInit(ScreenPtr pScreen)
{
    ScrnInfoPtr   pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info  = RADEONPTR(pScrn);
    XAAInfoRecPtr a;

    if (!(a = info->accel = XAACreateInfoRec())) return FALSE;

#ifdef XF86DRI
    /* FIXME: When direct rendering is enabled, we should use the CP to
       draw 2D commands */
    if (info->CP2D) RADEONCPAccelInit(pScrn, a);
    else
#endif
	RADEONMMIOAccelInit(pScrn, a);

    RADEONEngineInit(pScrn);
    return XAAInit(pScreen, a);
}
