/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atigetmode.c,v 1.1.2.1 1998/02/01 16:41:53 robin Exp $ */
/*
 * Copyright 1997,1998 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
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

#include "atiadapter.h"
#include "atichip.h"
#include "aticlock.h"
#include "aticonsole.h"
#include "atigetmode.h"
#include "atiio.h"

/*
 * ATIGetMode --
 *
 * This function will read the current SVGA register settings and produce a
 * filled-in DisplayModeRec containing the current mode.
 */
void
ATIGetMode(DisplayModePtr mode)
{
    int ShiftCount = 0;
    CARD8 misc;
    CARD8 crt00, crt01, crt03, crt04, crt05, crt06, crt07, crt09,
          crt10, crt11, crt12, crt17;
    CARD8 a6 = 0, a7 = 0, ac = 0,
          b0 = 0, b1 = 0, b2 = 0, b5 = 0, b8 = 0, b9 = 0, bd = 0, be = 0;
    CARD8 crtc_gen_cntl0 = 0;

    /*
     * Unlock registers.
     */
    ATIEnterLeave(ENTER);

    /* Initialize */
    mode->Clock = 0;

    /*
     * First, get the needed register values.
     */
    misc = inb(R_GENMO);
    ATISetVGAIOBase(misc);

    crt00 = GetReg(CRTX(vgaIOBase), 0x00U);
    crt01 = GetReg(CRTX(vgaIOBase), 0x01U);
    crt03 = GetReg(CRTX(vgaIOBase), 0x03U);
    crt04 = GetReg(CRTX(vgaIOBase), 0x04U);
    crt05 = GetReg(CRTX(vgaIOBase), 0x05U);
    crt06 = GetReg(CRTX(vgaIOBase), 0x06U);
    crt07 = GetReg(CRTX(vgaIOBase), 0x07U);
    crt09 = GetReg(CRTX(vgaIOBase), 0x09U);
    crt10 = GetReg(CRTX(vgaIOBase), 0x10U);
    crt11 = GetReg(CRTX(vgaIOBase), 0x11U);
    crt12 = GetReg(CRTX(vgaIOBase), 0x12U);
    crt17 = GetReg(CRTX(vgaIOBase), 0x17U);

    if (ATIChip >= ATI_CHIP_264CT)
        crtc_gen_cntl0 = inb(ATIIOPortCRTC_GEN_CNTL);

    if (ATIChipHasVGAWonder)
    {
        b0 = ATIGetExtReg(0xB0U);
        b1 = ATIGetExtReg(0xB1U);
        b5 = ATIGetExtReg(0xB5U);
        b8 = ATIGetExtReg(0xB8U);
        b9 = ATIGetExtReg(0xB9U);
        bd = ATIGetExtReg(0xBDU);
        if (ATIChip <= ATI_CHIP_18800)
            b2 = ATIGetExtReg(0xB2U);
        else
        {
            be = ATIGetExtReg(0xBEU);
            if (ATIChip >= ATI_CHIP_28800_2)
            {
                a6 = ATIGetExtReg(0xA6U);
                a7 = ATIGetExtReg(0xA7U);
                ac = ATIGetExtReg(0xACU);
            }
        }

        /* Set clock number */
        mode->Clock = (b8 & 0xC0U) >> 3;        /* Clock divider */
        if (ATIChip <= ATI_CHIP_18800)
            mode->Clock |= (b2 & 0x40U) >> 4;
        else
        {
            if (ATIAdapter != ATI_ADAPTER_V4)
            {
                mode->Clock |= (b9 & 0x02U) << 1;
                mode->Clock <<= 1;
            }
            mode->Clock |= (be & 0x10U) >> 2;
        }
    }
    mode->Clock |= (misc & 0x0CU) >> 2;         /* VGA clock select */
    mode->Clock = ATIClockUnMap[mode->Clock & 0x0FU] | (mode->Clock & ~0x0FU);
    if (ATIProgrammableClock == ATI_CLOCK_FIXED)
        mode->SynthClock = vga256InfoRec.clock[mode->Clock];
    else
        /*
         * TODO:  Read clock generator registers.  But this'll do for now.
         */
        mode->SynthClock = ATIDivide(ATIBIOSClocks[mode->Clock & 0x0FU] * 10,
            (mode->Clock >> 4) + 1, 0, 0);

    /*
     * Set horizontal display end.
     */
    mode->CrtcHDisplay = crt01;
    mode->HDisplay = (crt01 + 1) << 3;

    /*
     * Set horizontal synch pulse start.
     */
    mode->CrtcHSyncStart = crt04;
    mode->HSyncStart = crt04 << 3;

    /*
     * Set horizontal synch pulse end.
     */
    crt05 = (crt04 & 0xE0U) | (crt05 & 0x1FU);
    if (crt05 <= crt04)
        crt05 += 0x20U;
    mode->CrtcHSyncEnd = crt05;
    mode->HSyncEnd = crt05 << 3;

    /*
     * Set horizontal total.
     */
    mode->CrtcHTotal = crt00;
    mode->HTotal = (crt00 + 5) << 3;

    /*
     * Set horizontal display enable skew.
     */
    mode->HSkew = (crt03 & 0x60U) >> 2;
    if (ATIChipHasVGAWonder)
    {
        /* Assume ATI extended VGA registers override standard VGA */
        if (b5 & 0x01U)
            mode->HSkew = 1;
        if (b0 & 0x01U)
            mode->HSkew = 1 << 3;
        if (a6 & 0x01U)
            mode->HSkew = 2 << 3;
        if (a7 & 0x40U)
            mode->HSkew = 4 << 3;
        if (ac & 0x10U)
            mode->HSkew = 5 << 3;
        if (ac & 0x20U)
            mode->HSkew = 6 << 3;
    }
    mode->CrtcHSkew = mode->HSkew;

    mode->CrtcHAdjusted = TRUE;

    /*
     * Set vertical display end.
     */
    mode->CrtcVDisplay = ((crt07 & 0x40U) << 3) | ((crt07 & 0x02U) << 7) |
        crt12;
    mode->VDisplay = mode->CrtcVDisplay + 1;

    /*
     * Set vertical synch pulse start.
     */
    mode->CrtcVSyncStart = mode->VSyncStart = ((crt07 & 0x80U) << 2) |
        ((crt07 & 0x04U) << 6) | crt10;

    /*
     * Set vertical synch pulse end.
     */
    mode->VSyncEnd = (mode->VSyncStart & 0x3F0U) | (crt11 & 0x0FU);
    if (mode->VSyncEnd <= mode->VSyncStart)
        mode->VSyncEnd += 0x10U;
    mode->CrtcVSyncEnd = mode->VSyncEnd;

    /*
     * Set vertical total.
     */
    mode->CrtcVTotal = ((crt07 & 0x20U) << 4) | ((crt07 & 0x01U) << 8) | crt06;
    mode->VTotal = mode->CrtcVTotal + 2;

    mode->CrtcVAdjusted = TRUE;

    /*
     * Set flags.
     */
    if (misc & 0x40U)
        mode->Flags = V_NHSYNC;
    else
        mode->Flags = V_PHSYNC;
    if (misc & 0x80U)
        mode->Flags |= V_NVSYNC;
    else
        mode->Flags |= V_PVSYNC;
    /*
     * Triple-, quad-, etc scans not yet supported.
     */
    if (crt09 & 0x9FU)
        mode->Flags |= V_DBLSCAN;
    if (mode->HSkew)
        mode->Flags |= V_HSKEW;
    if (ATIChipHasVGAWonder)
    {
        if (ATIChip <= ATI_CHIP_18800)
        {
            if (b2 & 0x01U)
                mode->Flags |= V_INTERLACE;
        }
        else
        {
            if (be & 0x02U)
                mode->Flags |= V_INTERLACE;
        }
        if (b1 & 0x08U)
            mode->Flags |= V_DBLSCAN;
        if ((bd & 0x09U) == 0x09U)
            mode->Flags |= V_NCSYNC;
        else if (bd & 0x08U)
            mode->Flags |= V_PCSYNC;
    }
    if (ATIChip >= ATI_CHIP_264CT)
    {
        if (crtc_gen_cntl0 & CRTC_DBL_SCAN_EN)
            mode->Flags |= V_DBLSCAN;
        if (crtc_gen_cntl0 & CRTC_INTERLACE_EN)
            mode->Flags |= V_INTERLACE;
        if (crtc_gen_cntl0 & CRTC_CSYNC_EN)
            mode->Flags |= V_PCSYNC;
    }

    /*
     * Adjust vertical timings.
     */
    if ((ATIChip < ATI_CHIP_264CT) && (mode->Flags & V_INTERLACE))
        ShiftCount++;
    if (mode->Flags & V_DBLSCAN)
        ShiftCount--;
    if (b1 & 0x40U)
        ShiftCount--;
    if (crt17 & 0x04U)
        ShiftCount++;
    if (ShiftCount > 0)
    {
        mode->VDisplay <<= ShiftCount;
        mode->VSyncStart <<= ShiftCount;
        mode->VSyncEnd <<= ShiftCount;
        mode->VTotal <<= ShiftCount;
    }
    else if (ShiftCount < 0)
    {
        mode->VDisplay >>= -ShiftCount;
        mode->VSyncStart >>= -ShiftCount;
        mode->VSyncEnd >>= -ShiftCount;
        mode->VTotal >>= -ShiftCount;
    }
}
