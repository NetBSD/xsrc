/*
 * Copyright 2003 through 2004 by Marc Aurele La France (TSI @ UQV), tsi@xfree86.org
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of Marc Aurele La France not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Marc Aurele La France makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as-is" without express or implied warranty.
 *
 * MARC AURELE LA FRANCE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL MARC AURELE LA FRANCE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Copyright 1999-2000 Precision Insight, Inc., Cedar Park, Texas.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
/* 
 * DRI support by:
 *    Manuel Teira
 *    Leif Delgass <ldelgass@retinalburn.net>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ati.h"
#include "atichip.h"
#include "atimach64accel.h"
#include "atimach64io.h"
#include "atipriv.h"
#include "atiregs.h"

#ifdef XF86DRI_DEVEL
#include "mach64_common.h"
#endif

#include "miline.h"

/* Used to test MMIO cache integrity in ATIMach64Sync() */
#define TestRegisterCaching(_Register)                   \
  do {                                                   \
    if (RegisterIsCached(_Register) &&                   \
        (CacheSlot(_Register) != inm(_Register)))        \
    {                                                    \
        UncacheRegister(_Register);                      \
        xf86DrvMsg(pScreenInfo->scrnIndex, X_WARNING,    \
            #_Register " MMIO write cache disabled!\n"); \
    }                                                    \
  } while (0)

/*
 * X-to-Mach64 mix translation table.
 */
CARD8 ATIMach64ALU[16] =
{
    MIX_0,                       /* GXclear */
    MIX_AND,                     /* GXand */
    MIX_SRC_AND_NOT_DST,         /* GXandReverse */
    MIX_SRC,                     /* GXcopy */
    MIX_NOT_SRC_AND_DST,         /* GXandInverted */
    MIX_DST,                     /* GXnoop */
    MIX_XOR,                     /* GXxor */
    MIX_OR,                      /* GXor */
    MIX_NOR,                     /* GXnor */
    MIX_XNOR,                    /* GXequiv */
    MIX_NOT_DST,                 /* GXinvert */
    MIX_SRC_OR_NOT_DST,          /* GXorReverse */
    MIX_NOT_SRC,                 /* GXcopyInverted */
    MIX_NOT_SRC_OR_DST,          /* GXorInverted */
    MIX_NAND,                    /* GXnand */
    MIX_1                        /* GXset */
};

/*
 * ATIMach64ValidateClip --
 *
 * This function ensures the current scissor settings do not interfere with
 * the current draw request.
 */
void
ATIMach64ValidateClip
(
    ATIPtr pATI,
    int    sc_left,
    int    sc_right,
    int    sc_top,
    int    sc_bottom
)
{
    if ((sc_left < (int)pATI->sc_left) || (sc_right > (int)pATI->sc_right))
    {
        outf(SC_LEFT_RIGHT, pATI->sc_left_right);
        pATI->sc_left = pATI->NewHW.sc_left;
        pATI->sc_right = pATI->NewHW.sc_right;
    }

    if ((sc_top < (int)pATI->sc_top) || (sc_bottom > (int)pATI->sc_bottom))
    {
        outf(SC_TOP_BOTTOM, pATI->sc_top_bottom);
        pATI->sc_top = pATI->NewHW.sc_top;
        pATI->sc_bottom = pATI->NewHW.sc_bottom;
    }
}

static __inline__ void TestRegisterCachingDP(ScrnInfoPtr pScreenInfo);
static __inline__ void TestRegisterCachingXV(ScrnInfoPtr pScreenInfo);

/*
 * ATIMach64Sync --
 *
 * This is called to wait for the draw engine to become idle.
 */
void
ATIMach64Sync
(
    ScrnInfoPtr pScreenInfo
)
{
    ATIPtr pATI = ATIPTR(pScreenInfo);

#ifdef XF86DRI_DEVEL

    if ( pATI->directRenderingEnabled && pATI->NeedDRISync )
    {
	ATIHWPtr pATIHW = &pATI->NewHW;
	CARD32 offset;

	if (pATI->OptionMMIOCache) {
	    /* "Invalidate" the MMIO cache so the cache slots get updated */
	    UncacheRegister(SRC_CNTL);
	    UncacheRegister(SCALE_3D_CNTL);
	    UncacheRegister(HOST_CNTL);
	    UncacheRegister(PAT_CNTL);
	    UncacheRegister(SC_LEFT_RIGHT);
	    UncacheRegister(SC_TOP_BOTTOM);
	    UncacheRegister(DP_BKGD_CLR);
	    UncacheRegister(DP_FRGD_CLR);
	    UncacheRegister(DP_PIX_WIDTH);
	    UncacheRegister(DP_MIX);
	    UncacheRegister(CLR_CMP_CNTL);
	    UncacheRegister(TEX_SIZE_PITCH);
	}

	ATIDRIWaitForIdle(pATI);

	outr( BUS_CNTL, pATIHW->bus_cntl );

	/* DRI uses GUI_TRAJ_CNTL, which is a composite of 
	 * src_cntl, dst_cntl, pat_cntl, and host_cntl
	 */
	outf( SRC_CNTL, pATIHW->src_cntl );
	outf( DST_CNTL, pATIHW->dst_cntl );
	outf( PAT_CNTL, pATIHW->pat_cntl );
	outf( HOST_CNTL, pATIHW->host_cntl );

	outf( DST_OFF_PITCH, pATIHW->dst_off_pitch );
	outf( SRC_OFF_PITCH, pATIHW->src_off_pitch );
	outf( DP_SRC, pATIHW->dp_src );
	outf( DP_MIX, pATIHW->dp_mix );
	outf( DP_FRGD_CLR,  pATIHW->dp_frgd_clr );
	outf( DP_WRITE_MASK, pATIHW->dp_write_mask );
	outf( DP_PIX_WIDTH, pATIHW->dp_pix_width );

	outf( CLR_CMP_CNTL, pATIHW->clr_cmp_cntl );

	offset = TEX_LEVEL(pATIHW->tex_size_pitch);

	ATIMach64WaitForFIFO(pATI, 6);
	outf( ALPHA_TST_CNTL, 0 );
	outf( Z_CNTL, 0 );
	outf( SCALE_3D_CNTL, pATIHW->scale_3d_cntl );
	outf( TEX_0_OFF + offset, pATIHW->tex_offset );
	outf( TEX_SIZE_PITCH, pATIHW->tex_size_pitch );
	outf( TEX_CNTL, pATIHW->tex_cntl );

	ATIMach64WaitForFIFO(pATI, 2);
	outf( SC_LEFT_RIGHT,
	      SetWord(pATIHW->sc_right, 1) | SetWord(pATIHW->sc_left, 0) );
	outf( SC_TOP_BOTTOM,
	      SetWord(pATIHW->sc_bottom, 1) | SetWord(pATIHW->sc_top, 0) );

	if (pATI->OptionMMIOCache) {
	    /* Now that the cache slots reflect the register state, re-enable MMIO cache */
	    CacheRegister(SRC_CNTL);
	    CacheRegister(SCALE_3D_CNTL);
	    CacheRegister(HOST_CNTL);
	    CacheRegister(PAT_CNTL);
	    CacheRegister(SC_LEFT_RIGHT);
	    CacheRegister(SC_TOP_BOTTOM);
	    CacheRegister(DP_BKGD_CLR);
	    CacheRegister(DP_FRGD_CLR);
	    CacheRegister(DP_PIX_WIDTH);
	    CacheRegister(DP_MIX);
	    CacheRegister(CLR_CMP_CNTL);
	    CacheRegister(TEX_SIZE_PITCH);
	}

	ATIMach64WaitForIdle(pATI);

	if (pATI->OptionMMIOCache && pATI->OptionTestMMIOCache) {
	  
	    /* Only check registers we didn't restore */
	    TestRegisterCaching(PAT_REG0);
            TestRegisterCaching(PAT_REG1);

            TestRegisterCaching(CLR_CMP_CLR);
            TestRegisterCaching(CLR_CMP_MSK);

	    TestRegisterCachingXV(pScreenInfo);
         }
	pATI->NeedDRISync = FALSE;

    }
    else

#endif /* XF86DRI_DEVEL */
    {
      ATIMach64WaitForIdle(pATI);
      
      if (pATI->OptionMMIOCache && pATI->OptionTestMMIOCache)
      {
        /*
         * For debugging purposes, attempt to verify that each cached register
         * should actually be cached.
         */
        TestRegisterCachingDP(pScreenInfo);

        TestRegisterCachingXV(pScreenInfo);
      }
    }

#ifdef USE_EXA
    /* EXA sets pEXA->needsSync to FALSE on its own */
#endif


    if (pATI->Chip >= ATI_CHIP_264VTB)
    {
        /*
         * Flush the read-back cache (by turning on INVALIDATE_RB_CACHE),
         * otherwise the host might get stale data when reading through the
         * aperture.
         */
        outr(MEM_BUF_CNTL, pATI->NewHW.mem_buf_cntl);
    }

    /*
     * Note:
     * Before actually invalidating the read-back cache, the mach64 driver
     * was using the trick below which is buggy. The code is left here for
     * reference, DRI uses this trick and needs updating.
     *
     * For VTB's and later, the first CPU read of the framebuffer will return
     * zeroes, so do it here.  This appears to be due to some kind of engine
     * caching of framebuffer data I haven't found any way of disabling, or
     * otherwise circumventing.  Thanks to Mark Vojkovich for the suggestion.
     *
     * pATI = *(volatile ATIPtr *)pATI->pMemory;
     */
}

static __inline__ void
TestRegisterCachingDP(ScrnInfoPtr pScreenInfo)
{
    ATIPtr pATI = ATIPTR(pScreenInfo);

    TestRegisterCaching(SRC_CNTL);

    if (pATI->Chip >= ATI_CHIP_264GTPRO)
    {
        TestRegisterCaching(SCALE_3D_CNTL);
    }

    TestRegisterCaching(HOST_CNTL);

    TestRegisterCaching(PAT_REG0);
    TestRegisterCaching(PAT_REG1);
    TestRegisterCaching(PAT_CNTL);

    if (RegisterIsCached(SC_LEFT_RIGHT) &&      /* Special case */
        (CacheSlot(SC_LEFT_RIGHT) !=
         (SetWord(inm(SC_RIGHT), 1) | SetWord(inm(SC_LEFT), 0))))
    {
        UncacheRegister(SC_LEFT_RIGHT);
        xf86DrvMsg(pScreenInfo->scrnIndex, X_WARNING,
            "SC_LEFT_RIGHT write cache disabled!\n");
    }

    if (RegisterIsCached(SC_TOP_BOTTOM) &&      /* Special case */
        (CacheSlot(SC_TOP_BOTTOM) !=
         (SetWord(inm(SC_BOTTOM), 1) | SetWord(inm(SC_TOP), 0))))
    {
        UncacheRegister(SC_TOP_BOTTOM);
        xf86DrvMsg(pScreenInfo->scrnIndex, X_WARNING,
            "SC_TOP_BOTTOM write cache disabled!\n");
    }

    TestRegisterCaching(DP_BKGD_CLR);
    TestRegisterCaching(DP_FRGD_CLR);
    TestRegisterCaching(DP_PIX_WIDTH);
    TestRegisterCaching(DP_MIX);

    TestRegisterCaching(CLR_CMP_CLR);
    TestRegisterCaching(CLR_CMP_MSK);
    TestRegisterCaching(CLR_CMP_CNTL);

    if (pATI->Chip >= ATI_CHIP_264GTPRO)
    {
        TestRegisterCaching(TEX_SIZE_PITCH);
    }
}

static __inline__ void
TestRegisterCachingXV(ScrnInfoPtr pScreenInfo)
{
    ATIPtr pATI = ATIPTR(pScreenInfo);

    if (!pATI->Block1Base)
        return;

    TestRegisterCaching(OVERLAY_Y_X_START);
    TestRegisterCaching(OVERLAY_Y_X_END);

    TestRegisterCaching(OVERLAY_GRAPHICS_KEY_CLR);
    TestRegisterCaching(OVERLAY_GRAPHICS_KEY_MSK);

    TestRegisterCaching(OVERLAY_KEY_CNTL);

    TestRegisterCaching(OVERLAY_SCALE_INC);
    TestRegisterCaching(OVERLAY_SCALE_CNTL);

    TestRegisterCaching(SCALER_HEIGHT_WIDTH);

    TestRegisterCaching(SCALER_TEST);

    TestRegisterCaching(VIDEO_FORMAT);

    if (pATI->Chip < ATI_CHIP_264VTB)
    {
        TestRegisterCaching(BUF0_OFFSET);
        TestRegisterCaching(BUF0_PITCH);
        TestRegisterCaching(BUF1_OFFSET);
        TestRegisterCaching(BUF1_PITCH);

        return;
    }

    TestRegisterCaching(SCALER_BUF0_OFFSET);
    TestRegisterCaching(SCALER_BUF1_OFFSET);
    TestRegisterCaching(SCALER_BUF_PITCH);

    TestRegisterCaching(OVERLAY_EXCLUSIVE_HORZ);
    TestRegisterCaching(OVERLAY_EXCLUSIVE_VERT);

    if (pATI->Chip < ATI_CHIP_264GTPRO)
        return;

    TestRegisterCaching(SCALER_COLOUR_CNTL);

    TestRegisterCaching(SCALER_H_COEFF0);
    TestRegisterCaching(SCALER_H_COEFF1);
    TestRegisterCaching(SCALER_H_COEFF2);
    TestRegisterCaching(SCALER_H_COEFF3);
    TestRegisterCaching(SCALER_H_COEFF4);

    TestRegisterCaching(SCALER_BUF0_OFFSET_U);
    TestRegisterCaching(SCALER_BUF0_OFFSET_V);
    TestRegisterCaching(SCALER_BUF1_OFFSET_U);
    TestRegisterCaching(SCALER_BUF1_OFFSET_V);
}

