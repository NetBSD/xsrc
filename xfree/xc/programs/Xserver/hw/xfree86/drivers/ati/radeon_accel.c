/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/radeon_accel.c,v 1.23 2001/11/24 14:38:19 tsi Exp $ */
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

				/* Driver data structures */
#include "radeon.h"
#include "radeon_probe.h"
#include "radeon_reg.h"
#include "radeon_version.h"
#ifdef XF86DRI
#define _XF86DRI_SERVER_
#include "radeon_dri.h"
#include "radeon_sarea.h"
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

extern int gRADEONEntityIndex;

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
	RADEONEngineReset(pScrn);
	RADEONEngineRestore(pScrn);
#ifdef XF86DRI
	RADEONCP_RESET(pScrn, info);
	if (info->directRenderingEnabled) {
	    RADEONCP_START(pScrn, info);
	}
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
	RADEONEngineReset(pScrn);
	RADEONEngineRestore(pScrn);
#ifdef XF86DRI
	RADEONCP_RESET(pScrn, info);
	if (info->directRenderingEnabled) {
	    RADEONCP_START(pScrn, info);
	}
#endif
    }
}

#ifdef XF86DRI
/* Wait until the CP is completely idle: the FIFO has drained and the
 * CP is idle.
 */
static void RADEONCPWaitForIdle(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    int         ret;
    int         i    = 0;

    FLUSH_RING();

    for (;;) {
	do {
	    ret = drmRadeonWaitForIdleCP(info->drmFD);
	    if (ret && ret != -EBUSY) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			   "%s: CP idle %d\n", __FUNCTION__, ret);
	    }
	} while ((ret == -EBUSY) && (i++ < RADEON_TIMEOUT));

	if (ret == 0) return;

	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Idle timed out, resetting engine...\n");
	RADEONEngineReset(pScrn);
	RADEONEngineRestore(pScrn);

	/* Always restart the engine when doing CP 2D acceleration */
	RADEONCP_RESET(pScrn, info);
	RADEONCP_START(pScrn, info);
    }
}
#endif

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
    CARD32        host_path_cntl;

    RADEONEngineFlush(pScrn);

    clock_cntl_index = INREG(RADEON_CLOCK_CNTL_INDEX);
    mclk_cntl        = INPLL(pScrn, RADEON_MCLK_CNTL);

    OUTPLL(RADEON_MCLK_CNTL, (mclk_cntl |
			      RADEON_FORCEON_MCLKA |
			      RADEON_FORCEON_MCLKB |
			      RADEON_FORCEON_YCLKA |
			      RADEON_FORCEON_YCLKB |
			      RADEON_FORCEON_MC |
			      RADEON_FORCEON_AIC));

    /*Soft resetting HDP thru RBBM_SOFT_RESET register can
      cause some unexpected behaviour on some machines.
      Here we use RADEON_HOST_PATH_CNTL to reset it.*/ 
    host_path_cntl   = INREG(RADEON_HOST_PATH_CNTL);

    rbbm_soft_reset   = INREG(RADEON_RBBM_SOFT_RESET);

    OUTREG(RADEON_RBBM_SOFT_RESET, rbbm_soft_reset |
				   RADEON_SOFT_RESET_CP |
				   RADEON_SOFT_RESET_HI |
				   RADEON_SOFT_RESET_SE |
				   RADEON_SOFT_RESET_RE |
				   RADEON_SOFT_RESET_PP |
				   RADEON_SOFT_RESET_E2 |
				   RADEON_SOFT_RESET_RB );
    INREG(RADEON_RBBM_SOFT_RESET);
    OUTREG(RADEON_RBBM_SOFT_RESET, rbbm_soft_reset & (CARD32)
				 ~(RADEON_SOFT_RESET_CP |
				   RADEON_SOFT_RESET_HI |
				   RADEON_SOFT_RESET_SE |
				   RADEON_SOFT_RESET_RE |
				   RADEON_SOFT_RESET_PP |
				   RADEON_SOFT_RESET_E2 |
				   RADEON_SOFT_RESET_RB ));
    INREG(RADEON_RBBM_SOFT_RESET);

    OUTREG(RADEON_HOST_PATH_CNTL, host_path_cntl | RADEON_HDP_SOFT_RESET);
    INREG(RADEON_HOST_PATH_CNTL);
    OUTREG(RADEON_HOST_PATH_CNTL, host_path_cntl);

    OUTREG(RADEON_RBBM_SOFT_RESET,  rbbm_soft_reset);

    OUTREG(RADEON_CLOCK_CNTL_INDEX, clock_cntl_index);
    OUTPLL(RADEON_MCLK_CNTL,        mclk_cntl);

}

/* Restore the acceleration hardware to its previous state. */
void RADEONEngineRestore(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    int           pitch64;

    RADEONTRACE(("EngineRestore (%d/%d)\n",
		 info->CurrentLayout.pixel_code,
		 info->CurrentLayout.bitsPerPixel));

    RADEONWaitForFifo(pScrn, 1);
    /* turn of all automatic flushing - we'll do it all */
    OUTREG(RADEON_RB2D_DSTCACHE_MODE, 0);

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

    /* restore SURFACE_CNTL */
    OUTREG(RADEON_SURFACE_CNTL, info->ModeReg.surface_cntl);

    RADEONWaitForFifo(pScrn, 1);
    OUTREG(RADEON_DEFAULT_SC_BOTTOM_RIGHT, (RADEON_DEFAULT_SC_RIGHT_MAX
					    | RADEON_DEFAULT_SC_BOTTOM_MAX));
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

/* This callback is required for multiheader cards using XAA */
static
void RADEONRestoreAccelState(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    CARD32 pitch64;
    RADEONEntPtr pRADEONEnt;
    DevUnion* pPriv;

    pPriv = xf86GetEntityPrivate(pScrn->entityList[0],
            gRADEONEntityIndex);
    pRADEONEnt = pPriv->ptr;
    if(pRADEONEnt->IsDRIEnabled)
    {
        /*not working yet*/
        /*
        RADEONInfoPtr info0 = RADEONPTR(pRADEONEnt->pPrimaryScrn);
        RADEONCP_TO_MMIO(pRADEONEnt->pPrimaryScrn, info0);
        */
    }
    pitch64 = ((pScrn->displayWidth * (pScrn->bitsPerPixel / 8) + 0x3f)) >> 6;

    OUTREG(RADEON_DEFAULT_OFFSET, (pScrn->fbOffset>>10) |
				  (pitch64 << 22));

    /* FIXME: May need to restore other things, 
       like BKGD_CLK FG_CLK...*/

    RADEONWaitForIdle(pScrn);

}

/* This callback is required for multiheader cards using XAA */
#ifdef XF86DRI
static
void RADEONRestoreCPAccelState(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;
    /*xf86DrvMsg(pScrn->scrnIndex, X_INFO, "===>RestoreCP\n");*/

    RADEONWaitForFifo(pScrn, 1);
    OUTREG( RADEON_DEFAULT_OFFSET, info->frontPitchOffset);

    RADEONWaitForIdle(pScrn);

    /*Not working yet*/
    /*
    RADEONMMIO_TO_CP(pScrn, info);
    */
   
    /* FIXME: May need to restore other things, 
       like BKGD_CLK FG_CLK...*/

}
#endif 

/* Initialize the acceleration hardware. */
static void RADEONEngineInit(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    unsigned char *RADEONMMIO = info->MMIO;

    RADEONTRACE(("EngineInit (%d/%d)\n",
		 info->CurrentLayout.pixel_code,
		 info->CurrentLayout.bitsPerPixel));

    OUTREG(RADEON_RB3D_CNTL, 0);
#if defined(__powerpc__)
#if defined(XF86_DRI)
    if(!info->directRenderinEnabled)
#endif
    {
	OUTREG(RADEON_MC_FB_LOCATION, 0xffff0000);
    	OUTREG(RADEON_MC_AGP_LOCATION, 0xfffff000);
    }
#endif
    RADEONEngineReset(pScrn);

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

    info->dp_gui_master_cntl =
	((info->datatype << RADEON_GMC_DST_DATATYPE_SHIFT)
	 | RADEON_GMC_CLR_CMP_CNTL_DIS);

#ifdef XF86DRI
    info->sc_left         = 0x00000000;
    info->sc_right        = RADEON_DEFAULT_SC_RIGHT_MAX;
    info->sc_top          = 0x00000000;
    info->sc_bottom       = RADEON_DEFAULT_SC_BOTTOM_MAX;

    info->re_top_left     = 0x00000000;
    info->re_width_height = ((0x7ff << RADEON_RE_WIDTH_SHIFT) |
			     (0x7ff << RADEON_RE_HEIGHT_SHIFT));

    info->aux_sc_cntl     = 0x00000000;
#endif

    RADEONEngineRestore(pScrn);
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

/* unlike r128, Radeon don't have the Last-Pel controlling bit in DP_CNTL_XDIR_YDIR_YMAJOR
   for line drawing, so we have to do it using our own extrapolation routine*/
static void LastLinePel(int *X1, int *Y1, int *X2, int *Y2)
{
	int tg, deltax, deltay;
	int xa = *X1, ya = *Y1, xb = *X2, yb = *Y2;

	deltax = xb - xa;
	deltay = yb - ya;

    	if(deltax == 0) 
		tg = 40;
       	else
		tg = labs((deltay<<4) / deltax);

	if((tg >= 7) && (tg <= 39))
	{
		if(deltax > 0)xb++;
		else xb--;
		if(deltay > 0)yb++;
		else yb--;
	}
	else
	{
		if(labs(deltax) > labs(deltay))
		{
			if(deltax > 0) xb++;
			else xb--;
		}
		else
		{
			if(deltay > 0) yb++;
			else yb--;
		}
		   
	}
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

    if (!(flags & OMIT_LAST))
		LastLinePel(&xa, &ya, &xb, &yb);

    RADEONWaitForFifo(pScrn, 2);
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
    CARD32        pat         = *(CARD32 *)(pointer)pattern;

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

    RADEONWaitForFifo(pScrn, 3);
    if (!(flags & OMIT_LAST))
		LastLinePel(&xa, &ya, &xb, &yb);

    OUTREG(RADEON_DST_LINE_START,   (ya << 16) | xa);
	OUTREG(RADEON_DST_LINE_PATCOUNT, phase);
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

    if ((trans_color != -1) || (info->XAAForceTransBlit == TRUE)) {
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

#ifdef __alpha__
    /* always use indirect for Alpha */
    if (0)
#else
    if ((info->scanline_words * h) <= 9)
#endif
    {
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

/* Subsequent XAA indirect CPU-to-screen color expansion.  This is called
   once for each scanline. */
static void RADEONSubsequentColorExpandScanline(ScrnInfoPtr pScrn, int bufno)
{
    RADEONInfoPtr   info        = RADEONPTR(pScrn);
    unsigned char   *RADEONMMIO = info->MMIO;
    CARD32          *p          = (pointer)info->scratch_buffer[bufno];
    int             i;
    int             left        = info->scanline_words;
    volatile CARD32 *d;

    if (info->scanline_direct) return;
    --info->scanline_h;
    while (left) {
	write_mem_barrier();
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
    int           shift       = 0; /* 32bpp */

    if (pScrn->bitsPerPixel == 8) shift = 3;
    else if (pScrn->bitsPerPixel == 16) shift = 1;

    info->scanline_h      = h;
    info->scanline_words  = (w * info->scanline_bpp + 31) >> 5;

#ifdef __alpha__
    /* always use indirect for Alpha */
    if (0)
#else
    if ((info->scanline_words * h) <= 9)
#endif
    {
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
    CARD32          *p          = (pointer)info->scratch_buffer[bufno];
    int             i;
    int             left        = info->scanline_words;
    volatile CARD32 *d;

    if (info->scanline_direct) return;
    --info->scanline_h;
    while (left) {
	write_mem_barrier();
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

static void RADEONSetClippingRectangle(ScrnInfoPtr pScrn,
							  int xa, int ya, int xb, int yb)
{
	RADEONInfoPtr   info        = RADEONPTR(pScrn);
	unsigned char *RADEONMMIO = info->MMIO;
	unsigned long tmp = 0;

	if(xa < 0)
	{
		tmp = -xa;
		tmp |= RADEON_SC_SIGN_MASK_LO;
	}
	else tmp = xa;

	if(ya < 0)
	{
		tmp |= ((-ya) << 16);
		tmp |= RADEON_SC_SIGN_MASK_HI;
	}
	else tmp |= (ya << 16);

	OUTREG(RADEON_SC_TOP_LEFT, tmp);

	if(xb < 0)
	{
		tmp = -xb;
		tmp |= RADEON_SC_SIGN_MASK_LO;
	}
	else tmp = xb;

	if(yb < 0)
	{
		tmp |= ((-yb) << 16);
		tmp |= RADEON_SC_SIGN_MASK_HI;
	}
	else tmp |= (yb << 16);
    OUTREG(RADEON_SC_BOTTOM_RIGHT, tmp);

    OUTREG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl | RADEON_GMC_DST_CLIPPING));

}

static void
RADEONDisableClipping(ScrnInfoPtr pScrn)
{
	RADEONInfoPtr   info        = RADEONPTR(pScrn);
	unsigned char *RADEONMMIO = info->MMIO;

	OUTREG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl & ~(CARD32)RADEON_GMC_DST_CLIPPING));

	OUTREG(RADEON_SC_TOP_LEFT, 0);
	OUTREG(RADEON_SC_BOTTOM_RIGHT, INREG(RADEON_DEFAULT_SC_BOTTOM_RIGHT));

}



/* ================================================================
 * CP-based 2D acceleration
 */
#ifdef XF86DRI

/* Setup for XAA SolidFill. */
static void RADEONCPSetupForSolidFill(ScrnInfoPtr pScrn,
				      int color, int rop,
				      unsigned int planemask)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    RING_LOCALS;

    RADEONCP_REFRESH( pScrn, info );

    BEGIN_RING( 8 );

    OUT_RING_REG( RADEON_DP_GUI_MASTER_CNTL,
		  (info->dp_gui_master_cntl
		   | RADEON_GMC_BRUSH_SOLID_COLOR
		   | RADEON_GMC_SRC_DATATYPE_COLOR
		   | RADEON_ROP[rop].pattern) );

    OUT_RING_REG( RADEON_DP_BRUSH_FRGD_CLR,  color );
    OUT_RING_REG( RADEON_DP_WRITE_MASK,	     planemask );
    OUT_RING_REG( RADEON_DP_CNTL,	     (RADEON_DST_X_LEFT_TO_RIGHT |
					      RADEON_DST_Y_TOP_TO_BOTTOM) );
    ADVANCE_RING();
}

/* Subsequent XAA SolidFillRect.

   Tests: xtest CH06/fllrctngl, xterm
*/
static void RADEONCPSubsequentSolidFillRect(ScrnInfoPtr pScrn,
					    int x, int y, int w, int h)
{
    RADEONInfoPtr info  = RADEONPTR(pScrn);
    RING_LOCALS;

    RADEONCP_REFRESH( pScrn, info );

    BEGIN_RING( 4 );

    OUT_RING_REG( RADEON_DST_Y_X,          (y << 16) | x );
    OUT_RING_REG( RADEON_DST_WIDTH_HEIGHT, (w << 16) | h );

    ADVANCE_RING();
}

/* Setup for XAA screen-to-screen copy.

   Tests: xtest CH06/fllrctngl (also tests transparency).
*/
static void RADEONCPSetupForScreenToScreenCopy(ScrnInfoPtr pScrn,
					       int xdir, int ydir, int rop,
					       unsigned int planemask,
					       int trans_color)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    RING_LOCALS;

    RADEONCP_REFRESH( pScrn, info );

    info->xdir = xdir;
    info->ydir = ydir;

    BEGIN_RING( 6 );

    OUT_RING_REG( RADEON_DP_GUI_MASTER_CNTL,
		  (info->dp_gui_master_cntl
		   | RADEON_GMC_BRUSH_NONE
		   | RADEON_GMC_SRC_DATATYPE_COLOR
		   | RADEON_ROP[rop].rop
		   | RADEON_DP_SRC_SOURCE_MEMORY) );

    OUT_RING_REG( RADEON_DP_WRITE_MASK, planemask );
    OUT_RING_REG( RADEON_DP_CNTL,
		  ((xdir >= 0 ? RADEON_DST_X_LEFT_TO_RIGHT : 0) |
		   (ydir >= 0 ? RADEON_DST_Y_TOP_TO_BOTTOM : 0)) );

    ADVANCE_RING();

    if ((trans_color != -1) || (info->XAAForceTransBlit == TRUE)) {
	BEGIN_RING( 6 );

	OUT_RING_REG( RADEON_CLR_CMP_CLR_SRC, trans_color );
	OUT_RING_REG( RADEON_CLR_CMP_MASK,    RADEON_CLR_CMP_MSK );
	/* Mmmm, Seems as though the transparency compare is opposite to r128
	 * It should only draw when source != trans_color,
	 * this is the opposite of that. */
	OUT_RING_REG( RADEON_CLR_CMP_CNTL,    (RADEON_SRC_CMP_EQ_COLOR |
					       RADEON_CLR_CMP_SRC_SOURCE) );

	ADVANCE_RING();
    }
}

/* Subsequent XAA screen-to-screen copy. */
static void RADEONCPSubsequentScreenToScreenCopy(ScrnInfoPtr pScrn,
						 int xa, int ya,
						 int xb, int yb,
						 int w, int h)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    RING_LOCALS;

    RADEONCP_REFRESH( pScrn, info );

    if (info->xdir < 0) xa += w - 1, xb += w - 1;
    if (info->ydir < 0) ya += h - 1, yb += h - 1;

    BEGIN_RING( 6 );

    OUT_RING_REG( RADEON_SRC_Y_X,          (ya << 16) | xa );
    OUT_RING_REG( RADEON_DST_Y_X,          (yb << 16) | xb );
    OUT_RING_REG( RADEON_DST_HEIGHT_WIDTH, (h << 16) | w );

    ADVANCE_RING();
}

/* Setup for XAA solid lines. */
static void RADEONCPSetupForSolidLine(ScrnInfoPtr pScrn,
				    int color, int rop, unsigned int planemask)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);

    RING_LOCALS;
    BEGIN_RING( 6 );
    OUT_RING_REG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
										   | RADEON_GMC_BRUSH_SOLID_COLOR
										   | RADEON_GMC_SRC_DATATYPE_COLOR
										   | RADEON_ROP[rop].pattern));
    OUT_RING_REG(RADEON_DP_BRUSH_FRGD_CLR,  color);
    OUT_RING_REG(RADEON_DP_WRITE_MASK,      planemask);

    ADVANCE_RING();

}


/* Subsequent XAA solid TwoPointLine line.

   Tests: xtest CH06/drwln, ico, Mark Vojkovich's linetest program

   [See http://www.xfree86.org/devel/archives/devel/1999-Jun/0102.shtml for
   Mark Vojkovich's linetest program, posted 2Jun99 to devel@xfree86.org.]
*/
static void RADEONCPSubsequentSolidTwoPointLine(ScrnInfoPtr pScrn,
					      int xa, int ya, int xb, int yb,
					      int flags)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);

    RING_LOCALS;
    BEGIN_RING( 4 );

    if (!(flags & OMIT_LAST))
	{
		LastLinePel(&xa, &ya, &xb, &yb);
	}

	OUT_RING_REG(RADEON_DST_LINE_START,           (ya << 16) | xa);
	OUT_RING_REG(RADEON_DST_LINE_END,             (yb << 16) | xb);

	ADVANCE_RING();

}

/* Subsequent XAA solid horizontal and vertical lines */
static void RADEONCPSubsequentSolidHorVertLine(ScrnInfoPtr pScrn,
					     int x, int y, int len, int dir )
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);

    RING_LOCALS;
    BEGIN_RING( 6 );

    OUT_RING_REG(RADEON_DP_CNTL, (RADEON_DST_X_LEFT_TO_RIGHT
			    | RADEON_DST_Y_TOP_TO_BOTTOM));
	
    if (dir == DEGREES_0) 
	{
		OUT_RING_REG( RADEON_DST_Y_X,          (y << 16) | x );
		OUT_RING_REG( RADEON_DST_WIDTH_HEIGHT, (len << 16) | 1 );		
    } 
	else 
	{
		OUT_RING_REG( RADEON_DST_Y_X,          (y << 16) | x );
		OUT_RING_REG( RADEON_DST_WIDTH_HEIGHT, (1 << 16) | len );		
    }
    ADVANCE_RING();
}


/* Setup for XAA dashed lines.

   Tests: xtest CH05/stdshs, XFree86/drwln

   NOTE: Since we can only accelerate lines with power-of-2 patterns of
   length <= 32.
*/
static void RADEONCPSetupForDashedLine(ScrnInfoPtr pScrn,
				     int fg, int bg,
				     int rop, unsigned int planemask,
				     int length, unsigned char *pattern)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);
    CARD32        pat         = *(CARD32 *)(pointer)pattern;

    RING_LOCALS;

    switch (length) {
    case  2: pat |= pat <<  2;  /* fall through */
    case  4: pat |= pat <<  4;  /* fall through */
    case  8: pat |= pat <<  8;  /* fall through */
    case 16: pat |= pat << 16;
    }

    BEGIN_RING( 12 );
    OUT_RING_REG(RADEON_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
									 | (bg == -1
										? RADEON_GMC_BRUSH_32x1_MONO_FG_LA
										: RADEON_GMC_BRUSH_32x1_MONO_FG_BG)
									 | RADEON_ROP[rop].pattern
									 | RADEON_GMC_BYTE_LSB_TO_MSB));
    OUT_RING_REG(RADEON_DP_WRITE_MASK,      planemask);
    OUT_RING_REG(RADEON_DP_BRUSH_FRGD_CLR,  fg);
    if(bg != -1) OUT_RING_REG(RADEON_DP_BRUSH_BKGD_CLR,  bg);
    OUT_RING_REG(RADEON_BRUSH_DATA0, pat);
	ADVANCE_RING();

}

/* Subsequent XAA dashed line. */
static void RADEONCPSubsequentDashedTwoPointLine(ScrnInfoPtr pScrn,
					       int xa, int ya,
					       int xb, int yb,
					       int flags,
					       int phase)
{
    RADEONInfoPtr info        = RADEONPTR(pScrn);

    RING_LOCALS;

    if (!(flags & OMIT_LAST))
	{
		LastLinePel(&xa, &ya, &xb, &yb);
	}

	BEGIN_RING(6);
    OUT_RING_REG(RADEON_DST_LINE_START,   (ya << 16) | xa);
	OUT_RING_REG(RADEON_DST_LINE_PATCOUNT, phase);
    OUT_RING_REG(RADEON_DST_LINE_END,     (yb << 16) | xb);
	ADVANCE_RING();
}


static void RADEONCPSetClippingRectangle(ScrnInfoPtr pScrn,
							  int xa, int ya, int xb, int yb)
{
	RADEONInfoPtr   info        = RADEONPTR(pScrn);
	unsigned long tmp1 = 0, tmp2 = 0;

	if(xa < 0)
	{
		tmp1 = -xa;
		tmp1 |= RADEON_SC_SIGN_MASK_LO;
	}
	else tmp1 = xa;

	if(ya < 0)
	{
		tmp1 |= ((-ya) << 16);
		tmp1 |= RADEON_SC_SIGN_MASK_HI;
	}
	else tmp1 |= (ya << 16);

	if(xb < 0)
	{
		tmp2 = -xb;
		tmp2 |= RADEON_SC_SIGN_MASK_LO;
	}
	else tmp2 = xb;

	if(yb < 0)
	{
		tmp2 |= ((-yb) << 16);
		tmp2 |= RADEON_SC_SIGN_MASK_HI;
	}
	else tmp2 |= (yb << 16);

	{
	
	RING_LOCALS;
	BEGIN_RING( 3 );
	OUT_RING_REG(RADEON_SC_TOP_LEFT, tmp1);
	OUT_RING_REG(RADEON_SC_BOTTOM_RIGHT, tmp2);
	OUT_RING_REG(RADEON_DP_GUI_MASTER_CNTL, 
				 (info->dp_gui_master_cntl | RADEON_GMC_DST_CLIPPING));

	ADVANCE_RING();
	}
}

static void
RADEONCPDisableClipping(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr   info        = RADEONPTR(pScrn);
	/* unsigned char *RADEONMMIO = info->MMIO;*/
    RING_LOCALS;
    BEGIN_RING( 3 );
    OUT_RING_REG(RADEON_DP_GUI_MASTER_CNTL, 
					 (info->dp_gui_master_cntl & ~(CARD32)(RADEON_GMC_DST_CLIPPING)));
	OUT_RING_REG(RADEON_SC_TOP_LEFT, 0);
    OUT_RING_REG(RADEON_SC_BOTTOM_RIGHT, (RADEON_DEFAULT_SC_RIGHT_MAX
					    | RADEON_DEFAULT_SC_BOTTOM_MAX));

	ADVANCE_RING();

}


/* Point the DST_PITCH_OFFSET register at the current buffer.  This
 * allows us to interact with the back and depth buffers.  All CP 2D
 * acceleration commands use the DST_PITCH_OFFSET register.
 */
void RADEONSelectBuffer(ScrnInfoPtr pScrn, int buffer)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    RING_LOCALS;

    switch (buffer) {
    case RADEON_BACK:
	info->dst_pitch_offset = info->backPitchOffset;
	break;
    case RADEON_DEPTH:
	info->dst_pitch_offset = info->depthPitchOffset;
	break;
    default:
    case RADEON_FRONT:
	info->dst_pitch_offset = info->frontPitchOffset;
	break;
    }

    BEGIN_RING( 2 );

    OUT_RING_REG( RADEON_DEFAULT_OFFSET, info->dst_pitch_offset );

    ADVANCE_RING();
}

/* Get an indirect buffer for the CP 2D acceleration commands.
 */
drmBufPtr RADEONCPGetBuffer( ScrnInfoPtr pScrn )
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    drmDMAReq dma;
    drmBufPtr buf = NULL;
    int indx = 0;
    int size = 0;
    int ret, i = 0;

#if 0
    /* FIXME: pScrn->pScreen has not been initialized when this is first
       called from RADEONSelectBuffer via RADEONDRICPInit.  We could use
       the screen index from pScrn, which is initialized, and then get
       the screen from screenInfo.screens[index], but that is a hack. */
    dma.context = DRIGetContext(pScrn->pScreen);
#else
    dma.context = 0x00000001; /* This is the X server's context */
#endif
    dma.send_count = 0;
    dma.send_list = NULL;
    dma.send_sizes = NULL;
    dma.flags = 0;
    dma.request_count = 1;
    dma.request_size = RADEON_BUFFER_SIZE;
    dma.request_list = &indx;
    dma.request_sizes = &size;
    dma.granted_count = 0;

    while ( 1 ) {
	do {
	    ret = drmDMA( info->drmFD, &dma );
	    if ( ret && ret != -EBUSY ) {
		xf86DrvMsg( pScrn->scrnIndex, X_ERROR,
			    "%s: CP GetBuffer %d\n", __FUNCTION__, ret );
	    }
	} while ( ( ret == -EBUSY ) && ( i++ < RADEON_TIMEOUT ) );

	if ( ret == 0 ) {
	    buf = &info->buffers->list[indx];
	    buf->used = 0;
	    if ( RADEON_VERBOSE ) {
		xf86DrvMsg( pScrn->scrnIndex, X_INFO,
			    "   GetBuffer returning %d\n", buf->idx );
	    }
	    return buf;
	}

	xf86DrvMsg( pScrn->scrnIndex, X_ERROR,
		    "GetBuffer timed out, resetting engine...\n");
	RADEONEngineReset( pScrn );
	RADEONEngineRestore( pScrn );

	/* Always restart the engine when doing CP 2D acceleration */
	RADEONCP_RESET( pScrn, info );
	RADEONCP_START( pScrn, info );
    }
}

/* Flush the indirect buffer to the kernel for submission to the card.
 */
void RADEONCPFlushIndirect( ScrnInfoPtr pScrn )
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    drmBufPtr buffer = info->indirectBuffer;
    int start = info->indirectStart;
    int discard;

    if ( !buffer )
	return;

    if ( start == buffer->used )
	return;

    discard = ( buffer->used + RING_THRESHOLD > buffer->total );

    drmRadeonFlushIndirectBuffer( info->drmFD, buffer->idx,
				  start, buffer->used, discard );

    if ( discard ) {
	info->indirectBuffer = RADEONCPGetBuffer( pScrn );
	info->indirectStart = 0;
    } else {
	info->indirectStart = buffer->used;
    }
}

/* Flush and release the indirect buffer.
 */
void RADEONCPReleaseIndirect( ScrnInfoPtr pScrn )
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    drmBufPtr buffer = info->indirectBuffer;
    int start = info->indirectStart;

    info->indirectBuffer = NULL;
    info->indirectStart = 0;

    if ( !buffer )
	return;

    drmRadeonFlushIndirectBuffer( info->drmFD, buffer->idx,
				  start, buffer->used, 1 );
}

static void RADEONCPAccelInit(ScrnInfoPtr pScrn, XAAInfoRecPtr a)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);

#if 1
    a->Flags                            = (PIXMAP_CACHE
					   | OFFSCREEN_PIXMAPS
					   | LINEAR_FRAMEBUFFER);
#else
    a->Flags                            = 0; /* GH: Do we really need this? */
#endif

				/* Sync */
    a->Sync                             = RADEONCPWaitForIdle;

    /* If direct rendering is disabled, then do not enable any CP
       acceleration routines */
    if (!info->directRenderingEnabled) return;

				/* Solid Filled Rectangle */
    a->PolyFillRectSolidFlags           = 0;
    a->SetupForSolidFill                = RADEONCPSetupForSolidFill;
    a->SubsequentSolidFillRect          = RADEONCPSubsequentSolidFillRect;

				/* Screen-to-screen Copy */
    a->ScreenToScreenCopyFlags          = 0;
    a->SetupForScreenToScreenCopy       = RADEONCPSetupForScreenToScreenCopy;
    a->SubsequentScreenToScreenCopy     = RADEONCPSubsequentScreenToScreenCopy;


    a->SetupForDashedLine              = RADEONCPSetupForDashedLine;
    a->SubsequentDashedTwoPointLine    = RADEONCPSubsequentDashedTwoPointLine;
    a->DashPatternMaxLength            = 32;
	/*ROP3 doesn't seem to work properly for dashedline with GXinvert*/
    a->DashedLineFlags                 = (LINE_PATTERN_LSBFIRST_LSBJUSTIFIED
										  | LINE_PATTERN_POWER_OF_2_ONLY 
										  | ROP_NEEDS_SOURCE); 


	a->SolidLineFlags 		= 0;
    a->SetupForSolidLine               = RADEONCPSetupForSolidLine;
    a->SubsequentSolidTwoPointLine     = RADEONCPSubsequentSolidTwoPointLine;
    a->SubsequentSolidHorVertLine      = RADEONCPSubsequentSolidHorVertLine;
    a->SubsequentSolidBresenhamLine 	= NULL;

    /* clipping */
    a->SetClippingRectangle = RADEONCPSetClippingRectangle;
    a->DisableClipping = RADEONCPDisableClipping;
    a->ClippingFlags = 	HARDWARE_CLIP_SOLID_LINE  |
		HARDWARE_CLIP_DASHED_LINE |
		/*HARDWARE_CLIP_SOLID_FILL  |*/ /* seems very slow with this on ???*/
		HARDWARE_CLIP_MONO_8x8_FILL |
		HARDWARE_CLIP_SCREEN_TO_SCREEN_COPY;

    if(!info->IsSecondary && xf86IsEntityShared(pScrn->entityList[0]))
        a->RestoreAccelState           = RADEONRestoreCPAccelState;

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
	/*ROP3 doesn't seem to work properly for dashedline with GXinvert*/
    a->DashedLineFlags                 = (LINE_PATTERN_LSBFIRST_LSBJUSTIFIED
										  | LINE_PATTERN_POWER_OF_2_ONLY 
										  | ROP_NEEDS_SOURCE); 

    /* clipping, note without this, 
       all line accelerations will not be called */
    a->SetClippingRectangle = RADEONSetClippingRectangle;
    a->DisableClipping = RADEONDisableClipping;
    a->ClippingFlags = 	HARDWARE_CLIP_SOLID_LINE  |
		HARDWARE_CLIP_DASHED_LINE |
		/*HARDWARE_CLIP_SOLID_FILL  |*/ /* seems very slow with this on ???*/
		HARDWARE_CLIP_MONO_8x8_FILL |
		HARDWARE_CLIP_SCREEN_TO_SCREEN_COPY;


    if(xf86IsEntityShared(pScrn->entityList[0]))
    {
        DevUnion* pPriv;
        RADEONEntPtr pRADEONEnt;
        pPriv = xf86GetEntityPrivate(pScrn->entityList[0],
                gRADEONEntityIndex);
        pRADEONEnt = pPriv->ptr;
        
        /*if there are more than one devices sharing this entity, we
          have to assign this call back, otherwise the XAA will be
          disabled */
        if(pRADEONEnt->HasSecondary || pRADEONEnt->BypassSecondary)
           a->RestoreAccelState           = RADEONRestoreAccelState;
    }

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

    if (!(a = info->accel = XAACreateInfoRec())) 
    {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, 
            "XAACreateInfoRec Error\n");
        return FALSE;
    }
#ifdef XF86DRI
    if (info->directRenderingEnabled)
	RADEONCPAccelInit(pScrn, a);
    else
#endif
	RADEONMMIOAccelInit(pScrn, a);

    RADEONEngineInit(pScrn);
    
    if(!XAAInit(pScreen, a))
    {
        xf86DrvMsg(pScrn->scrnIndex, X_ERROR, 
            "XAAInit Error\n");
        return FALSE;
    }    
    return TRUE;
}
