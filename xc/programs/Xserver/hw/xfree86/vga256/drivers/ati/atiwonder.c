/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atiwonder.c,v 1.1.2.3 2000/05/14 02:02:18 tsi Exp $ */
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
/*
 * The ATI x8800 chips use special registers for their extended VGA features.
 * These registers are accessible through an index I/O port and a data I/O
 * port.  BIOS initialization stores the index port number in the Graphics
 * register bank (0x03CE), indices 0x50 and 0x51.  Unfortunately, for all but
 * the 18800-x series of adapters, these registers are write-only (a.k.a. black
 * holes).  On all but Mach64's, the index port number can be found in the
 * short integer at offset 0x10 in the BIOS.  For Mach64's, this driver will
 * use 0x01CE or 0x03CE as the index port number, depending on the I/O port
 * decoding used.  The data port number is one more than the index port number
 * (i.e. 0x01CF).  These ports differ slightly in their I/O behaviour from the
 * normal VGA ones:
 *
 *    write:  outw(0x01CE, (data << 8) | index);
 *    read:   outb(0x01CE, index);  data = inb(0x01CF);
 *
 * Two consecutive byte-writes to the data port will not work.  Furthermore an
 * index written to 0x01CE is usable only once.  Note also that the setting of
 * ATI extended registers (especially those with clock selection bits) should
 * be bracketed by a sequencer reset.
 *
 * The number of these extended VGA registers varies by chipset.  The 18800
 * series have 16, the 28800 series have 32, while Mach32's and Mach64's have
 * 64.  The last 16 on each have almost identical definitions.  Thus, the BIOS
 * (and this driver) sets up an indexing scheme whereby the last 16 extended
 * VGA registers are accessed at indices 0xB0 through 0xBF on all chipsets.
 */

#include "atichip.h"
#include "atidepth.h"
#include "atiio.h"
#include "atividmem.h"
#include "atiwonder.h"

/*
 * ATIVGAWonderSave --
 *
 * This function is called to save the VGA Wonder portion of the current video
 * state.
 */
void
ATIVGAWonderSave(ATIHWPtr save)
{
    save->b0 = ATIGetExtReg(0xB0U);
    save->b1 = ATIGetExtReg(0xB1U);
    save->b2 = ATIGetExtReg(0xB2U);
    save->b3 = ATIGetExtReg(0xB3U);
    save->b5 = ATIGetExtReg(0xB5U);
    save->b6 = ATIGetExtReg(0xB6U);
    save->b8 = ATIGetExtReg(0xB8U);
    save->b9 = ATIGetExtReg(0xB9U);
    save->ba = ATIGetExtReg(0xBAU);
    save->bd = ATIGetExtReg(0xBDU);
    if (ATIChip > ATI_CHIP_18800)
    {
        save->be = ATIGetExtReg(0xBEU);
        if (ATIChip >= ATI_CHIP_28800_2)
        {
            save->bf = ATIGetExtReg(0xBFU);
            save->a3 = ATIGetExtReg(0xA3U);
            save->a6 = ATIGetExtReg(0xA6U);
            save->a7 = ATIGetExtReg(0xA7U);
            save->ab = ATIGetExtReg(0xABU);
            save->ac = ATIGetExtReg(0xACU);
            save->ad = ATIGetExtReg(0xADU);
            save->ae = ATIGetExtReg(0xAEU);
        }
    }
}

/*
 * ATIVGAWonderInit --
 *
 * This function fills in the VGA Wonder portion of an ATIHWRec structure
 * occurrentce.
 */
void
ATIVGAWonderInit(DisplayModePtr mode)
{
    if (!mode)
    {
        /*
         * Fill in VGA Wonder data that is common to all video modes generated
         * by the server.
         */
        ATINewHWPtr->b3 = ATIGetExtReg(0xB3U) & 0x20U;
        if (ATIUsingPlanarModes)
            ATINewHWPtr->b6 = 0x40U;
        else
            ATINewHWPtr->b6 = 0x04U;
        if (ATIChip <= ATI_CHIP_18800)
            ATINewHWPtr->ba = 0x08U;
        else if (ATIChip >= ATI_CHIP_28800_2)
        {
            if (ATIvideoRam > 256)
                ATINewHWPtr->b6 |= 0x01U;
            ATINewHWPtr->bf = ATIGetExtReg(0xBFU) & 0x5FU;
            ATINewHWPtr->a3 = ATIGetExtReg(0xA3U) & 0x67U;
            ATINewHWPtr->ab = ATIGetExtReg(0xABU) & 0xE7U;
            ATINewHWPtr->ae = ATIGetExtReg(0xAEU) & 0xF0U;
        }
    }
    else
    {
        /* Set up default horizontal display enable skew */
        if ((ATIChip >= ATI_CHIP_28800_2) && (ATIChip <= ATI_CHIP_28800_6) &&
            !(mode->Flags & V_HSKEW))
        {
            /*
             * Modes using the higher clock frequencies need a non-zero Display
             * Enable Skew.  The following number has been empirically
             * determined to be somewhere between 4.2 and 4.7 MHz.
             */
#           define Display_Enable_Skew_Threshold 4500

            /* Set a reasonable default Display Enable Skew */
            mode->HSkew = mode->CrtcHSkew =
                ATIDivide(vga256InfoRec.clock[mode->Clock],
                    Display_Enable_Skew_Threshold, 0, 0);
        }
        mode->Flags |= V_HSKEW;

        /*
         * Fill in mode-specific VGA Wonder data.
         */
        ATINewHWPtr->b0 = 0x00U;
        if (!ATIUsingPlanarModes)
            ATINewHWPtr->b0 = 0x20U;
        if (ATIChip >= ATI_CHIP_28800_2)
        {
            if (ATIvideoRam > 512)
                ATINewHWPtr->b0 |= 0x08U;
            else if (ATIvideoRam > 256)
                ATINewHWPtr->b0 |= 0x10U;
        }
        else if (ATIUsingPlanarModes)
        {
            if (ATIvideoRam > 256)
                ATINewHWPtr->b0 |= 0x08U;
        }
        else
        {
            if (ATIvideoRam > 256)
                ATINewHWPtr->b0 |= 0x18U;
            else
                ATINewHWPtr->b0 |= 0x06U;
        }
        ATINewHWPtr->b1 = ATIGetExtReg(0xB1U) & 0x04U;
        ATINewHWPtr->b5 = 0x00U;
        ATINewHWPtr->b8 = ATIGetExtReg(0xB8U) & 0xC0U;
        ATINewHWPtr->b9 = ATIGetExtReg(0xB9U) & 0x7FU;
        ATINewHWPtr->bd = ATIGetExtReg(0xBDU) & 0x02U;
        if (ATIChip <= ATI_CHIP_18800)
            ATINewHWPtr->b2 = ATIGetExtReg(0xB2U) & 0xC0U;
        else
        {
            ATINewHWPtr->b2 = 0x00U;
            ATINewHWPtr->be = (ATIGetExtReg(0xBEU) & 0x30U) | 0x09U;
            if (ATIChip >= ATI_CHIP_28800_2)
            {
                ATINewHWPtr->a6 = (ATIGetExtReg(0xA6U) & 0x38U) | 0x04U;
                ATINewHWPtr->a7 = ATIGetExtReg(0xA7U) & 0xBEU;
                ATINewHWPtr->ac = ATIGetExtReg(0xACU) & 0x8EU;
            }
        }
        if (mode->Flags & V_INTERLACE)  /* Enable interlacing */
        {
            if (ATIChip <= ATI_CHIP_18800)
                ATINewHWPtr->b2 |= 0x01U;
            else
                ATINewHWPtr->be |= 0x02U;
        }
#if 0   /* This is no longer needed but is left in for reference */
        if (mode->Flags & V_DBLSCAN)
            ATINewHWPtr->b1 |= 0x08U;   /* Enable double scanning */
#endif
        if ((OFLG_ISSET(OPTION_CSYNC, &vga256InfoRec.options)) ||
            (mode->Flags & (V_CSYNC | V_PCSYNC)))
            ATINewHWPtr->bd |= 0x08U;   /* Enable composite synch */
        if (mode->Flags & V_NCSYNC)
            ATINewHWPtr->bd |= 0x09U;   /* Invert csync polarity */
        if (mode->CrtcHSkew > 0)
        {
            if (mode->CrtcHSkew <= 3)
                ATINewHWPtr->b5 |= 0x01U;
            else if (ATIChip >= ATI_CHIP_28800_2)
            switch ((mode->CrtcHSkew + 4) >> 3)
            {
                case 1:                 /* Use ATI override */
                    ATINewHWPtr->std.CRTC[3] &= ~0x60U;
                    ATINewHWPtr->b0 |= 0x01U;
                    break;
                case 2:                 /* Use ATI override */
                    ATINewHWPtr->std.CRTC[3] &= ~0x60U;
                    ATINewHWPtr->a6 |= 0x01U;
                    break;
                case 3:
                    ATINewHWPtr->std.CRTC[3] |= 0x60U;
                    break;
                case 4:
                    ATINewHWPtr->a7 |= 0x40U;
                    break;
                case 5:
                    ATINewHWPtr->ac |= 0x10U;
                    break;
                case 6:
                    ATINewHWPtr->ac |= 0x20U;
                    break;
                default:
                    break;
            }
        }
    }
}

/*
 * ATIVGAWonderRestore --
 *
 * This function loads the VGA Wonder portion of a video mode.
 */
void
ATIVGAWonderRestore(ATIHWPtr restore)
{
    if (ATIChip <= ATI_CHIP_18800)
        ATIModifyExtReg(0xB2U, -1, 0x00U, restore->b2);
    else
    {
        ATIModifyExtReg(0xBEU, -1, 0x00U, restore->be);
        if (ATIChip >= ATI_CHIP_28800_2)
        {
            ATIModifyExtReg(0xBFU, -1, 0x00U, restore->bf);
            ATIModifyExtReg(0xA3U, -1, 0x00U, restore->a3);
            ATIModifyExtReg(0xA6U, -1, 0x00U, restore->a6);
            ATIModifyExtReg(0xA7U, -1, 0x00U, restore->a7);
            ATIModifyExtReg(0xABU, -1, 0x00U, restore->ab);
            ATIModifyExtReg(0xACU, -1, 0x00U, restore->ac);
            ATIModifyExtReg(0xADU, -1, 0x00U, restore->ad);
            ATIModifyExtReg(0xAEU, -1, 0x00U, restore->ae);
        }
    }
    ATIModifyExtReg(0xB0U, -1, 0x00U, restore->b0);
    ATIModifyExtReg(0xB1U, -1, 0x00U, restore->b1);
    ATIModifyExtReg(0xB3U, -1, 0x00U, restore->b3);
    ATIModifyExtReg(0xB5U, -1, 0x00U, restore->b5);
    ATIModifyExtReg(0xB6U, -1, 0x00U, restore->b6);
    ATIModifyExtReg(0xB8U, -1, 0x00U, restore->b8);
    ATIModifyExtReg(0xB9U, -1, 0x00U, restore->b9);
    ATIModifyExtReg(0xBAU, -1, 0x00U, restore->ba);
    ATIModifyExtReg(0xBDU, -1, 0x00U, restore->bd);
}
