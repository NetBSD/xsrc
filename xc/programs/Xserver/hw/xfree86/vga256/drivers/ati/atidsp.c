/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atidsp.c,v 1.1.2.3 1999/07/05 09:07:33 hohndel Exp $ */
/*
 * Copyright 1997 through 1999 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
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

#include "atichip.h"
#include "aticlock.h"
#include "atidepth.h"
#include "atidsp.h"
#include "atiio.h"
#include "atividmem.h"

/* Various memory-related things needed to set DSP registers */
static int ATIXCLKFeedbackDivider,
           ATIXCLKReferenceDivider,
           ATIXCLKPostDivider;

static CARD16 ATIXCLKMaxRASDelay,
              ATIXCLKPageFaultDelay,
              ATIDisplayLoopLatency,
              ATIDisplayFIFODepth;

/*
 * ATIDSPProbe --
 *
 * This function initializes global variables used to set DSP registers on a
 * VT-B or later.  It is called by ATIProbe.
 */
Bool
ATIDSPProbe(void)
{
    CARD32 IO_Value;
    int trp;

    /* Set DSP register port numbers */
    ATIIOPortDSP_CONFIG = ATIIOPort(DSP_CONFIG);
    ATIIOPortDSP_ON_OFF = ATIIOPort(DSP_ON_OFF);

    /*
     * VT-B's and later have additional post-dividers that are not powers of
     * two.
     */
    ATIClockDescriptor->NumD = 8;

    /* Retrieve XCLK settings */
    IO_Value = ATIGetMach64PLLReg(PLL_XCLK_CNTL);
    ATIXCLKPostDivider = GetBits(IO_Value, PLL_XCLK_SRC_SEL);
    ATIXCLKReferenceDivider = ATIClockDescriptor->MinM;
    switch (ATIXCLKPostDivider)
    {
        case 0: case 1: case 2: case 3:
            break;

        case 4:
            ATIXCLKReferenceDivider *= 3;
            ATIXCLKPostDivider = 0;
            break;

        default:
            ErrorF("Unsupported XCLK source: %d", ATIXCLKPostDivider);
            return FALSE;
    }

    ATIXCLKPostDivider -= GetBits(IO_Value, PLL_MFB_TIMES_4_2B);
    ATIXCLKFeedbackDivider = ATIGetMach64PLLReg(PLL_MCLK_FB_DIV);

    /* Compute maximum RAS delay and friends */
    IO_Value = inl(ATIIOPortMEM_INFO);
    trp = GetBits(IO_Value, CTL_MEM_TRP);
    ATIXCLKPageFaultDelay = GetBits(IO_Value, CTL_MEM_TRCD) +
        GetBits(IO_Value, CTL_MEM_TCRD) + trp + 2;
    ATIXCLKMaxRASDelay = GetBits(IO_Value, CTL_MEM_TRAS) + trp + 2;
    ATIDisplayFIFODepth = 32;

    if (ATIChip < ATI_CHIP_264VT4)
    {
        ATIXCLKPageFaultDelay += 2;
        ATIXCLKMaxRASDelay += 3;
        ATIDisplayFIFODepth = 24;
    }

    switch (ATIMemoryType)
    {
        case MEM_264_DRAM:
            if (ATIvideoRam <= 1024)
                ATIDisplayLoopLatency = 10;
            else
            {
                ATIDisplayLoopLatency = 8;
                ATIXCLKPageFaultDelay += 2;
            }
            break;

        case MEM_264_EDO:
        case MEM_264_PSEUDO_EDO:
            if (ATIvideoRam <= 1024)
                ATIDisplayLoopLatency = 9;
            else
            {
                ATIDisplayLoopLatency = 8;
                ATIXCLKPageFaultDelay++;
            }
            break;

        case MEM_264_SDRAM:
            if (ATIvideoRam <= 1024)
                ATIDisplayLoopLatency = 11;
            else
            {
                ATIDisplayLoopLatency = 10;
                ATIXCLKPageFaultDelay++;
            }
            break;

        case MEM_264_SGRAM:
            ATIDisplayLoopLatency = 8;
            ATIXCLKPageFaultDelay += 3;
            break;

        default:                /* Set maximums */
            ATIDisplayLoopLatency = 11;
            ATIXCLKPageFaultDelay += 3;
            break;
    }

    if (ATIXCLKMaxRASDelay <= ATIXCLKPageFaultDelay)
        ATIXCLKMaxRASDelay = ATIXCLKPageFaultDelay + 1;

    return TRUE;
}

/*
 * ATIDSPSave --
 *
 * This function is called by ATISave() to remember DSP register values on VT-B
 * and later controllers.
 */
void
ATIDSPSave(ATIHWPtr save)
{
    save->dsp_on_off = inl(ATIIOPortDSP_ON_OFF);
    save->dsp_config = inl(ATIIOPortDSP_CONFIG);
}


/*
 * ATIDSPInit --
 *
 * This function sets up DSP register values for a VTB or later.  Note that
 * this would be slightly different if VCLK 0 or 1 were used for the mode
 * instead.  In that case, this function would set VGA_DSP_CONFIG and
 * VGA_DSP_ON_OFF, would have to zero out DSP_CONFIG and DSP_ON_OFF, and would
 * have to consider that VGA_DSP_CONFIG is partitioned slightly differently
 * than DSP_CONFIG.
 */
void
ATIDSPInit(DisplayModePtr mode)
{
    int Multiplier, Divider;
    int dsp_precision, dsp_on, dsp_off, dsp_xclks;
    int tmp, vshift, xshift;
    int RASMultiplier = ATIXCLKMaxRASDelay, RASDivider = 1;

#   define Maximum_DSP_PRECISION ((int)GetBits(DSP_PRECISION, DSP_PRECISION))

    /* Compute a memory-to-screen bandwidth ratio */
    Multiplier = /* ATINewHWPtr->ReferenceDivider * */ ATIXCLKFeedbackDivider *
        ATIClockDescriptor->PostDividers[ATINewHWPtr->PostDivider];
    Divider = ATINewHWPtr->FeedbackDivider /* * ATIXCLKReferenceDivider */;
    if (!ATIUsingPlanarModes)
        Divider *= vga256InfoRec.bitsPerPixel / 4;
    /* Start by assuming a display FIFO width of 32 bits */
    vshift = (5 - 2) - ATIXCLKPostDivider;
    if (ATINewHWPtr->crtc != ATI_CRTC_VGA)
        vshift++;               /* Nope, it's 64 bits wide */

    if (ATILCDPanelID >= 0)
    {
        /* Compensate for horizontal stretching */
        Multiplier *= ATILCDHorizontal;
        Divider *= mode->HDisplay & ~7;
        RASMultiplier *= ATILCDHorizontal;
        RASDivider *= mode->HDisplay & ~7;
    }

    /* Determine dsp_precision first */
    tmp = ATIDivide(Multiplier * ATIDisplayFIFODepth, Divider, vshift, 1);
    for (dsp_precision = -5;  tmp;  dsp_precision++)
        tmp >>= 1;
    if (dsp_precision < 0)
        dsp_precision = 0;
    else if (dsp_precision > Maximum_DSP_PRECISION)
        dsp_precision = Maximum_DSP_PRECISION;

    xshift = 6 - dsp_precision;
    vshift += xshift;

    /* Move on to dsp_off */
    dsp_off = ATIDivide(Multiplier * (ATIDisplayFIFODepth - 1), Divider,
        vshift, -1) - ATIDivide(1, 1, vshift - xshift, 1);

    /* Next is dsp_on */
    if ((ATINewHWPtr->crtc == ATI_CRTC_VGA) && (dsp_precision < 3))
    {
        /*
         * TODO:  I don't yet know why something like this appears necessary.
         *        But I don't have time to explore this right now.
         */
        dsp_on = ATIDivide(Multiplier * 5, Divider, vshift + 2, 1);
    }
    else
    {
        dsp_on = ATIDivide(Multiplier, Divider, vshift, 1);
        tmp = ATIDivide(RASMultiplier, RASDivider, xshift, 1);
        if (dsp_on < tmp)
            dsp_on = tmp;
        dsp_on += (tmp * 2) + ATIDivide(ATIXCLKPageFaultDelay, 1, xshift, 1);
    }

    /* Last but not least:  dsp_xclks */
    dsp_xclks = ATIDivide(Multiplier, Divider, vshift + 5, 1);

    /* Build DSP register contents */
    ATINewHWPtr->dsp_on_off = SetBits(dsp_on, DSP_ON) |
        SetBits(dsp_off, DSP_OFF);
    ATINewHWPtr->dsp_config = SetBits(dsp_precision, DSP_PRECISION) |
        SetBits(dsp_xclks, DSP_XCLKS_PER_QW) |
        SetBits(ATIDisplayLoopLatency, DSP_LOOP_LATENCY);
}

/*
 * ATIDSPRestore --
 *
 * This function is called by ATIRestore to set DSP registers on VT-B and later
 * controllers.
 */
void
ATIDSPRestore(ATIHWPtr restore)
{
    outl(ATIIOPortDSP_ON_OFF, restore->dsp_on_off);
    outl(ATIIOPortDSP_CONFIG, restore->dsp_config);
}
