/*
 * Copyright 1999, 2000 ATI Technologies Inc., Markham, Ontario,
 *                      Precision Insight, Inc., Cedar Park, Texas, and
 *                      VA Linux Systems Inc., Fremont, California.
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
 * NON-INFRINGEMENT.  IN NO EVENT SHALL ATI, PRECISION INSIGHT, VA LINUX
 * SYSTEMS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/*
 * Authors:
 *   Rickard E. Faith <faith@valinux.com>
 *   Kevin E. Martin <martin@valinux.com>
 *   Alan Hourihane <alanh@fairlite.demon.co.uk>
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
 * Notes on unimplemented XAA optimizations:
 *
 *   SetClipping:   The Rage128 doesn't support the full 16bit registers needed
 *                  for XAA clip rect support.
 *   SolidFillTrap: This will probably work if we can compute the correct
 *                  Bresenham error values.
 *   TwoPointLine:  The Rage 128 supports Bresenham lines instead.
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
 * (Alan Hourihane) Update. We now use purely indirect and clip the full
 *                  rectangle. Seems as the direct method has some problems
 *                  with this, although this indirect method is much faster
 *                  than the old method of setting up the engine per scanline.
 *                  This code was the basis of the Radeon work we did.
 *   Color8x8PatternFill: Apparently, an 8x8 color brush cannot take an 8x8
 *                  pattern from frame buffer memory.
 *   ImageWrites:   See CPUToScreenColorExpandFill.
 *
 */

#define R128_TRAPEZOIDS 0       /* Trapezoids don't work               */

				/* Driver data structures */
#include <errno.h>

#include "r128.h"
#include "r128_reg.h"
#include "r128_probe.h"
#ifdef R128DRI
#include "r128_sarea.h"
#define _XF86DRI_SERVER_
#include "r128_dri.h"
#include "r128_common.h"
#endif

				/* Line support */
#include "miline.h"

				/* X and server generic header files */
#include "xf86.h"


/* Flush all dirty data in the Pixel Cache to memory. */
void R128EngineFlush(ScrnInfoPtr pScrn)
{
    R128InfoPtr   info      = R128PTR(pScrn);
    unsigned char *R128MMIO = info->MMIO;
    int           i;

    OUTREGP(R128_PC_NGUI_CTLSTAT, R128_PC_FLUSH_ALL, ~R128_PC_FLUSH_ALL);
    for (i = 0; i < R128_TIMEOUT; i++) {
	if (!(INREG(R128_PC_NGUI_CTLSTAT) & R128_PC_BUSY)) break;
    }
}

/* Reset graphics card to known state. */
void R128EngineReset(ScrnInfoPtr pScrn)
{
    R128InfoPtr   info      = R128PTR(pScrn);
    unsigned char *R128MMIO = info->MMIO;
    uint32_t      clock_cntl_index;
    uint32_t      mclk_cntl;
    uint32_t      gen_reset_cntl;

    R128EngineFlush(pScrn);

    clock_cntl_index = INREG(R128_CLOCK_CNTL_INDEX);
    mclk_cntl        = INPLL(pScrn, R128_MCLK_CNTL);

    OUTPLL(R128_MCLK_CNTL, mclk_cntl | R128_FORCE_GCP | R128_FORCE_PIPE3D_CP);

    gen_reset_cntl   = INREG(R128_GEN_RESET_CNTL);

    OUTREG(R128_GEN_RESET_CNTL, gen_reset_cntl | R128_SOFT_RESET_GUI);
    INREG(R128_GEN_RESET_CNTL);
    OUTREG(R128_GEN_RESET_CNTL,
	gen_reset_cntl & (uint32_t)(~R128_SOFT_RESET_GUI));
    INREG(R128_GEN_RESET_CNTL);

    OUTPLL(R128_MCLK_CNTL,        mclk_cntl);
    OUTREG(R128_CLOCK_CNTL_INDEX, clock_cntl_index);
    OUTREG(R128_GEN_RESET_CNTL,   gen_reset_cntl);
}

/* The FIFO has 64 slots.  This routines waits until at least `entries' of
   these slots are empty. */
void R128WaitForFifoFunction(ScrnInfoPtr pScrn, int entries)
{
    R128InfoPtr   info      = R128PTR(pScrn);
    unsigned char *R128MMIO = info->MMIO;
    int           i;

    for (;;) {
	for (i = 0; i < R128_TIMEOUT; i++) {
	    info->fifo_slots = INREG(R128_GUI_STAT) & R128_GUI_FIFOCNT_MASK;
	    if (info->fifo_slots >= entries) return;
	}

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                    "FIFO timed out: %lu entries, "
                    "stat = 0x%08lx, probe = 0x%08lx\n",
                    INREG(R128_GUI_STAT) & R128_GUI_FIFOCNT_MASK,
                    INREG(R128_GUI_STAT),
                    INREG(R128_GUI_PROBE)));
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "FIFO timed out, resetting engine...\n");
	R128EngineReset(pScrn);
#ifdef R128DRI
	R128CCE_RESET(pScrn, info);
	if (info->directRenderingEnabled) {
	    R128CCE_START(pScrn, info);
	}
#endif
    }
}

/* Wait for the graphics engine to be completely idle: the FIFO has
   drained, the Pixel Cache is flushed, and the engine is idle.  This is a
   standard "sync" function that will make the hardware "quiescent". */
void R128WaitForIdle(ScrnInfoPtr pScrn)
{
    R128InfoPtr   info      = R128PTR(pScrn);
    unsigned char *R128MMIO = info->MMIO;
    int           i;

    R128WaitForFifoFunction(pScrn, 64);

    for (;;) {
	for (i = 0; i < R128_TIMEOUT; i++) {
	    if (!(INREG(R128_GUI_STAT) & R128_GUI_ACTIVE)) {
		R128EngineFlush(pScrn);
		return;
	    }
	}

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Idle timed out: %lu entries, "
                        "stat = 0x%08lx, probe = 0x%08lx\n",
                        INREG(R128_GUI_STAT) & R128_GUI_FIFOCNT_MASK,
                        INREG(R128_GUI_STAT),
                        INREG(R128_GUI_PROBE)));
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Idle timed out, resetting engine...\n");
#ifdef R128DRI
        R128CCE_STOP(pScrn, info);
#endif
	R128EngineReset(pScrn);
#ifdef R128DRI
	R128CCE_RESET(pScrn, info);
	if (info->directRenderingEnabled) {
	    R128CCE_START(pScrn, info);
	}
#endif
    }
}

#ifdef R128DRI
/* Wait until the CCE is completely idle: the FIFO has drained and the
 * CCE is idle.
 */
void R128CCEWaitForIdle(ScrnInfoPtr pScrn)
{
    R128InfoPtr info = R128PTR(pScrn);
    int         ret, i;

    FLUSH_RING();

    for (;;) {
        i = 0;
        do {
            ret = drmCommandNone(info->drmFD, DRM_R128_CCE_IDLE);
        } while ( ret && errno == EBUSY && i++ < (R128_IDLE_RETRY * R128_IDLE_RETRY) );

	if (ret && ret != -EBUSY) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "%s: CCE idle %d\n", __FUNCTION__, ret);
	}

	if (i > R128_IDLE_RETRY) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		       "%s: (DEBUG) CCE idle took i = %d\n", __FUNCTION__, i);
	}

	if (ret == 0) return;

	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Idle timed out, resetting engine...\n");
	R128CCE_STOP(pScrn, info);
	R128EngineReset(pScrn);

	/* Always restart the engine when doing CCE 2D acceleration */
	R128CCE_RESET(pScrn, info);
	R128CCE_START(pScrn, info);
    }
}

int R128CCEStop(ScrnInfoPtr pScrn)
{
    R128InfoPtr    info = R128PTR(pScrn);
    drmR128CCEStop stop;
    int            ret, i;

    stop.flush = 1;
    stop.idle  = 1;

    ret = drmCommandWrite( info->drmFD, DRM_R128_CCE_STOP,
                           &stop, sizeof(drmR128CCEStop) );

    if ( ret == 0 ) {
        return 0;
    } else if ( errno != EBUSY ) {
        return -errno;
    }

    stop.flush = 0;

    i = 0;
    do {
        ret = drmCommandWrite( info->drmFD, DRM_R128_CCE_STOP,
                               &stop, sizeof(drmR128CCEStop) );
    } while ( ret && errno == EBUSY && i++ < R128_IDLE_RETRY );

    if ( ret == 0 ) {
        return 0;
    } else if ( errno != EBUSY ) {
        return -errno;
    }

    stop.idle = 0;

    if ( drmCommandWrite( info->drmFD, DRM_R128_CCE_STOP,
                          &stop, sizeof(drmR128CCEStop) )) {
        return -errno;
    } else {
        return 0;
    }
}

#endif


/* Initialize the acceleration hardware. */
void R128EngineInit(ScrnInfoPtr pScrn)
{
    R128InfoPtr   info      = R128PTR(pScrn);
    unsigned char *R128MMIO = info->MMIO;

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "EngineInit (%d/%d)\n",
                        info->CurrentLayout.pixel_code,
                        info->CurrentLayout.bitsPerPixel));

    OUTREG(R128_SCALE_3D_CNTL, 0);
    R128EngineReset(pScrn);

    switch (info->CurrentLayout.pixel_code) {
    case 8:  info->datatype = 2; break;
    case 15: info->datatype = 3; break;
    case 16: info->datatype = 4; break;
    case 24: info->datatype = 5; break;
    case 32: info->datatype = 6; break;
    default:
    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Unknown depth/bpp = %d/%d (code = %d)\n",
                        info->CurrentLayout.depth,
                        info->CurrentLayout.bitsPerPixel,
                        info->CurrentLayout.pixel_code));
    }
    info->pitch = (info->CurrentLayout.displayWidth / 8) * (info->CurrentLayout.pixel_bytes == 3 ? 3 : 1);

    DEBUG(xf86DrvMsg(pScrn->scrnIndex, X_INFO,
                        "Pitch for acceleration = %d\n", info->pitch));

    R128WaitForFifo(pScrn, 2);
    OUTREG(R128_DEFAULT_OFFSET, pScrn->fbOffset);
    OUTREG(R128_DEFAULT_PITCH,  info->pitch);

    R128WaitForFifo(pScrn, 4);
    OUTREG(R128_AUX_SC_CNTL,             0);
    OUTREG(R128_DEFAULT_SC_BOTTOM_RIGHT, (R128_DEFAULT_SC_RIGHT_MAX
					  | R128_DEFAULT_SC_BOTTOM_MAX));
    OUTREG(R128_SC_TOP_LEFT,             0);
    OUTREG(R128_SC_BOTTOM_RIGHT,         (R128_DEFAULT_SC_RIGHT_MAX
					  | R128_DEFAULT_SC_BOTTOM_MAX));

    info->dp_gui_master_cntl = ((info->datatype << R128_GMC_DST_DATATYPE_SHIFT)
				| R128_GMC_CLR_CMP_CNTL_DIS
				| R128_GMC_AUX_CLIP_DIS);
    R128WaitForFifo(pScrn, 1);
    OUTREG(R128_DP_GUI_MASTER_CNTL, (info->dp_gui_master_cntl
				     | R128_GMC_BRUSH_SOLID_COLOR
				     | R128_GMC_SRC_DATATYPE_COLOR));

    R128WaitForFifo(pScrn, 8);
    OUTREG(R128_DST_BRES_ERR,      0);
    OUTREG(R128_DST_BRES_INC,      0);
    OUTREG(R128_DST_BRES_DEC,      0);
    OUTREG(R128_DP_BRUSH_FRGD_CLR, 0xffffffff);
    OUTREG(R128_DP_BRUSH_BKGD_CLR, 0x00000000);
    OUTREG(R128_DP_SRC_FRGD_CLR,   0xffffffff);
    OUTREG(R128_DP_SRC_BKGD_CLR,   0x00000000);
    OUTREG(R128_DP_WRITE_MASK,     0xffffffff);

    R128WaitForFifo(pScrn, 1);

#if X_BYTE_ORDER == X_BIG_ENDIAN
    /* FIXME: this is a kludge for texture uploads in the 3D driver. Look at
     * how the radeon driver handles HOST_DATA_SWAP if you want to implement
     * CCE ImageWrite acceleration or anything needing this bit */
#ifdef R128DRI
    if (info->directRenderingEnabled)
	OUTREGP(R128_DP_DATATYPE, 0, ~R128_HOST_BIG_ENDIAN_EN);
    else
#endif
	OUTREGP(R128_DP_DATATYPE,
		R128_HOST_BIG_ENDIAN_EN, ~R128_HOST_BIG_ENDIAN_EN);
#else /* X_LITTLE_ENDIAN */
    OUTREGP(R128_DP_DATATYPE, 0, ~R128_HOST_BIG_ENDIAN_EN);
#endif

#ifdef R128DRI
    info->sc_left         = 0x00000000;
    info->sc_right        = R128_DEFAULT_SC_RIGHT_MAX;
    info->sc_top          = 0x00000000;
    info->sc_bottom       = R128_DEFAULT_SC_BOTTOM_MAX;

    info->re_top_left     = 0x00000000;
    info->re_width_height = ((0x7ff << R128_RE_WIDTH_SHIFT) |
			     (0x7ff << R128_RE_HEIGHT_SHIFT));

    info->aux_sc_cntl     = 0x00000000;
#endif

    R128WaitForIdle(pScrn);
}

#ifdef R128DRI

/* Get an indirect buffer for the CCE 2D acceleration commands.
 */
drmBufPtr R128CCEGetBuffer( ScrnInfoPtr pScrn )
{
    R128InfoPtr   info = R128PTR(pScrn);
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
    dma.request_size = R128_BUFFER_SIZE;
    dma.request_list = &indx;
    dma.request_sizes = &size;
    dma.granted_count = 0;

    while ( 1 ) {
	do {
	    ret = drmDMA( info->drmFD, &dma );
	    if ( ret && ret != -EAGAIN ) {
		xf86DrvMsg( pScrn->scrnIndex, X_ERROR,
			    "%s: CCE GetBuffer %d\n", __FUNCTION__, ret );
	    }
	} while ( ( ret == -EAGAIN ) && ( i++ < R128_TIMEOUT ) );

	if ( ret == 0 ) {
	    buf = &info->buffers->list[indx];
	    buf->used = 0;
	    if ( R128_VERBOSE ) {
		xf86DrvMsg( pScrn->scrnIndex, X_INFO,
			    "   GetBuffer returning %d\n", buf->idx );
	    }
	    return buf;
	}

	xf86DrvMsg( pScrn->scrnIndex, X_ERROR,
		    "GetBuffer timed out, resetting engine...\n");
	R128EngineReset( pScrn );
	/* R128EngineRestore( pScrn ); FIXME ??? */

	/* Always restart the engine when doing CCE 2D acceleration */
	R128CCE_RESET( pScrn, info );
	R128CCE_START( pScrn, info );
    }
}

/* Flush the indirect buffer to the kernel for submission to the card.
 */
void R128CCEFlushIndirect( ScrnInfoPtr pScrn, int discard )
{
    R128InfoPtr   info = R128PTR(pScrn);
    drmBufPtr buffer = info->indirectBuffer;
    int start = info->indirectStart;
    drmR128Indirect indirect;

    if ( !buffer )
	return;

    if ( (start == buffer->used) && !discard )
        return;

    indirect.idx = buffer->idx;
    indirect.start = start;
    indirect.end = buffer->used;
    indirect.discard = discard;

    drmCommandWriteRead( info->drmFD, DRM_R128_INDIRECT,
                         &indirect, sizeof(drmR128Indirect));

    if ( discard )
        buffer = info->indirectBuffer = R128CCEGetBuffer( pScrn );

    /* pad to an even number of dwords */
    if (buffer->used & 7)
        buffer->used = ( buffer->used+7 ) & ~7;

    info->indirectStart = buffer->used;
}

/* Flush and release the indirect buffer.
 */
void R128CCEReleaseIndirect( ScrnInfoPtr pScrn )
{
    R128InfoPtr   info = R128PTR(pScrn);
    drmBufPtr buffer = info->indirectBuffer;
    int start = info->indirectStart;
    drmR128Indirect indirect;

    info->indirectBuffer = NULL;
    info->indirectStart = 0;

    if ( !buffer )
	return;

    indirect.idx = buffer->idx;
    indirect.start = start;
    indirect.end = buffer->used;
    indirect.discard = 1;

    drmCommandWriteRead( info->drmFD, DRM_R128_INDIRECT,
                         &indirect, sizeof(drmR128Indirect));
}

#endif

void R128CopySwap(uint8_t *dst, uint8_t *src, unsigned int size, int swap)
{
    switch(swap) {
    case APER_0_BIG_ENDIAN_32BPP_SWAP:
	{
	    unsigned int *d = (unsigned int *)dst;
	    unsigned int *s = (unsigned int *)src;
	    unsigned int nwords = size >> 2;

	    for (; nwords > 0; --nwords, ++d, ++s)
#ifdef __powerpc__
		asm volatile("stwbrx %0,0,%1" : : "r" (*s), "r" (d));
#else
		*d = ((*s >> 24) & 0xff) | ((*s >> 8) & 0xff00)
			| ((*s & 0xff00) << 8) | ((*s & 0xff) << 24);
#endif
	    return;
	}
    case APER_0_BIG_ENDIAN_16BPP_SWAP:
	{
	    unsigned short *d = (unsigned short *)dst;
	    unsigned short *s = (unsigned short *)src;
	    unsigned int nwords = size >> 1;

	    for (; nwords > 0; --nwords, ++d, ++s)
#ifdef __powerpc__
		asm volatile("sthbrx %0,0,%1" : : "r" (*s), "r" (d));
#else
	        *d = (*s >> 8) | (*s << 8);
#endif
	    return;
	}
    }
    if (src != dst)
	memcpy(dst, src, size);
}
