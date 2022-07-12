/*
 * Copyright 1997 through 2004 by Marc Aurele La France (TSI @ UQV), tsi@xfree86.org
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86.h"
#include "atichip.h"
#include "atistruct.h"
#include "ativalid.h"


/*
 * ATIValidMode --
 *
 * This checks for hardware-related limits on mode timings.
 */
ModeStatus
ATIValidMode
(
    SCRN_ARG_TYPE arg,
    DisplayModePtr pMode,
    Bool Verbose,
    int flags
)
{
    SCRN_INFO_PTR(arg);
    ATIPtr      pATI        = ATIPTR(pScreenInfo);
    int         HBlankWidth, HAdjust, VScan, VInterlace;

    if (flags & MODECHECK_FINAL)
    {
        return MODE_OK;
    }

    {
        int maxHValue, maxVValue;

        maxHValue = (MaxBits(CRTC_H_TOTAL) + 1) << 3;
        if (pATI->Chip < ATI_CHIP_264VT)
        {
            /* CRTC_H_TOTAL is one bit narrower */
            maxHValue >>= 1;
        }
        if (pMode->HTotal > maxHValue)
            return MODE_BAD_HVALUE;

        maxVValue = MaxBits(CRTC_V_TOTAL) + 1;
        if (pMode->VTotal > maxVValue)
            return MODE_BAD_VVALUE;
    }

    /*
     * The following is done for every mode in the monitor section that
     * survives the common layer's basic checks.
     */
    if (pMode->VScan <= 1)
        VScan = 1;
    else
        VScan = pMode->VScan;

    if (pMode->Flags & V_DBLSCAN)
        VScan <<= 1;

    if (pATI->OptionPanelDisplay && (pATI->LCDPanelID >= 0))
    {
        if ((pMode->CrtcHDisplay > pATI->LCDHorizontal) ||
            (pMode->CrtcVDisplay > pATI->LCDVertical))
            return MODE_PANEL;

        if (!pATI->OptionLCDSync || (pMode->type & M_T_BUILTIN))
        {
            if ((pMode->HDisplay > pATI->LCDHorizontal) ||
                (pMode->VDisplay > pATI->LCDVertical))
                return MODE_PANEL;

            return MODE_OK;
        }

        /*
         * Adjust effective timings for monitor checks.  Here the modeline
         * clock is ignored.  Horizontal timings are scaled by the stretch
         * ratio used for the displayed area.  The vertical porch is scaled by
         * the native resolution's aspect ratio.  This seems rather arbitrary,
         * and it is, but it does make all applicable VESA modes sync on a
         * panel after stretching.  This has the unfortunate, but necessary,
         * side-effect of changing the mode's horizontal sync and vertical
         * refresh rates.  With some exceptions, this tends to increase the
         * mode's horizontal sync rate, and decrease its vertical refresh rate.
         */
        pMode->SynthClock = pATI->LCDClock;

        pMode->CrtcHTotal = pMode->CrtcHBlankEnd =
            ATIDivide(pMode->CrtcHTotal * pATI->LCDHorizontal,
                pMode->CrtcHDisplay, -3, 1) << 3;
        pMode->CrtcHSyncEnd =
            ATIDivide(pMode->CrtcHSyncEnd * pATI->LCDHorizontal,
                pMode->CrtcHDisplay, -3, 1) << 3;
        pMode->CrtcHSyncStart =
            ATIDivide(pMode->CrtcHSyncStart * pATI->LCDHorizontal,
                pMode->CrtcHDisplay, -3, -1) << 3;
        pMode->CrtcHDisplay = pMode->CrtcHBlankStart = pATI->LCDHorizontal;

        pMode->CrtcVTotal = pMode->CrtcVBlankEnd =
            ATIDivide((pMode->CrtcVTotal - pMode->CrtcVDisplay) *
                pATI->LCDVertical, pATI->LCDHorizontal, 0, 1) +
                pATI->LCDVertical;
        pMode->CrtcVSyncEnd =
            ATIDivide((pMode->CrtcVSyncEnd - pMode->CrtcVDisplay) *
                pATI->LCDVertical, pATI->LCDHorizontal, 0, 1) +
                pATI->LCDVertical;
        pMode->CrtcVSyncStart =
            ATIDivide((pMode->CrtcVSyncStart - pMode->CrtcVDisplay) *
                pATI->LCDVertical, pATI->LCDHorizontal, 0, -1) +
                pATI->LCDVertical;
        pMode->CrtcVDisplay = pMode->CrtcVBlankStart = pATI->LCDVertical;

        /*
         * The CRTC only stretches the mode's displayed area, not its porches.
         * Reverse-engineer the mode's timings back into the user specified
         * values so that the stretched mode is produced when the CRTC is
         * eventually programmed.  The reverse-engineered mode is then checked
         * against CRTC limits below.
         */
        pMode->Clock = pATI->LCDClock;

        HAdjust = pATI->LCDHorizontal - pMode->HDisplay;
#       define ATIReverseHorizontal(_x) \
            (pMode->_x - HAdjust)

        pMode->HSyncStart = ATIReverseHorizontal(CrtcHSyncStart);
        pMode->HSyncEnd = ATIReverseHorizontal(CrtcHSyncEnd);
        pMode->HTotal = ATIReverseHorizontal(CrtcHTotal);

        VInterlace = GetBits(pMode->Flags, V_INTERLACE) + 1;
#       define ATIReverseVertical(_y) \
            ((((pMode->_y - pATI->LCDVertical) * VInterlace) / VScan) + \
             pMode->VDisplay)

        pMode->VSyncStart = ATIReverseVertical(CrtcVSyncStart);
        pMode->VSyncEnd = ATIReverseVertical(CrtcVSyncEnd);
        pMode->VTotal = ATIReverseVertical(CrtcVTotal);

#       undef ATIReverseHorizontal
#       undef ATIReverseVertical
    }

    HBlankWidth = (pMode->HTotal >> 3) - (pMode->HDisplay >> 3);
    if (!HBlankWidth)
        return MODE_HBLANK_NARROW;

    {
            if (VScan > 2)
                return MODE_NO_VSCAN;
    }

    return MODE_OK;
}
