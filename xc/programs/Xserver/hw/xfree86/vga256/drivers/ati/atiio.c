/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atiio.c,v 1.1.2.3 2000/05/14 02:02:15 tsi Exp $ */
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
#include "atiio.h"
#include "vga.h"

/* The following are port numbers that are determined by ATIProbe */
CARD16 ATIIOPortVGAWonder = 0x01CEU;

CARD16 ATIIOPortCRTC_H_TOTAL_DISP, ATIIOPortCRTC_H_SYNC_STRT_WID,
       ATIIOPortCRTC_V_TOTAL_DISP, ATIIOPortCRTC_V_SYNC_STRT_WID,
       ATIIOPortCRTC_OFF_PITCH, ATIIOPortCRTC_INT_CNTL, ATIIOPortCRTC_GEN_CNTL,
       ATIIOPortDSP_CONFIG, ATIIOPortDSP_ON_OFF, ATIIOPortOVR_CLR,
       ATIIOPortOVR_WID_LEFT_RIGHT, ATIIOPortOVR_WID_TOP_BOTTOM,
       ATIIOPortTV_OUT_INDEX, ATIIOPortCLOCK_CNTL, ATIIOPortTV_OUT_DATA,
       ATIIOPortBUS_CNTL, ATIIOPortLCD_INDEX, ATIIOPortLCD_DATA,
       ATIIOPortMEM_INFO, ATIIOPortMEM_VGA_WP_SEL, ATIIOPortMEM_VGA_RP_SEL,
       ATIIOPortDAC_REGS, ATIIOPortDAC_CNTL,
       ATIIOPortHORZ_STRETCHING, ATIIOPortVERT_STRETCHING,
       ATIIOPortGEN_TEST_CNTL, ATIIOPortLCD_GEN_CTRL, ATIIOPortCONFIG_CNTL;

/* These port numbers are to be determined by ATISave & ATIRestore */
CARD16 ATIIOPortDAC_MASK, ATIIOPortDAC_DATA,
       ATIIOPortDAC_READ, ATIIOPortDAC_WRITE;

/* I/O decoding definitions */
CARD16 ATIIOBase;
CARD8 ATIIODecoding;

CARD8 ATIB2Reg = 0;             /* The B2 mirror */
CARD8 ATIVGAOffset = 0x80U;     /* Low index for ATIIOPortVGAWonder */

/*
 * ATISetVGAIOBase --
 *
 * This sets vgaIOBase according to the value of the passed value of the
 * miscellaneous output register.
 */
void
ATISetVGAIOBase(const CARD8 misc)
{
    vgaIOBase = (misc & 0x01U) ? ColourIOBase : MonochromeIOBase;
}

/*
 * ATIModifyExtReg --
 *
 * This function is called to modify certain bits in an ATI extended VGA
 * register while preserving its other bits.  The function will not write the
 * register if it turns out its value would not change.  This helps prevent
 * server hangs on older adapters.
 */
void
ATIModifyExtReg(const CARD8 Index, int Current_Value,
                const CARD8 Current_Mask, CARD8 New_Value)
{
    /* Possibly retrieve the current value */
    if (Current_Value < 0)
        Current_Value = ATIGetExtReg(Index);

    /* Compute new value */
    New_Value &= (CARD8)(~Current_Mask);
    New_Value |= Current_Value & Current_Mask;

    /* Check if value will be changed */
    if (Current_Value == New_Value)
        return;

    /*
     * The following is taken from ATI's VGA Wonder programmer's reference
     * manual which says that this is needed to "ensure the proper state of the
     * 8/16 bit ROM toggle".  I suspect a timing glitch appeared in the 18800
     * after its die was cast.  18800-1 and later chips do not exhibit this
     * problem.
     */
    if ((ATIChip <= ATI_CHIP_18800) && (Index == 0xB2U) &&
       ((New_Value ^ 0x40U) & Current_Value & 0x40U))
    {
        CARD8 misc = inb(R_GENMO);
        CARD8 bb = ATIGetExtReg(0xBBU);

        outb(GENMO, (misc & 0xF3U) | 0x04U | ((bb & 0x10U) >> 1));
        Current_Value &= (CARD8)(~0x40U);
        ATIPutExtReg(0xB2U, Current_Value);
        ATIDelay(5);
        outb(GENMO, misc);
        ATIDelay(5);
        if (Current_Value != New_Value)
            ATIPutExtReg(0xB2U, New_Value);
    }
    else
        ATIPutExtReg(Index, New_Value);
}

/*
 * ATIAccessMach64PLLReg --
 *
 * This function sets up the addressing required to access, for read or write,
 * a 264xT's PLL registers.
 */
void
ATIAccessMach64PLLReg(const CARD8 Index, const Bool Write)
{
    CARD8 clock_cntl1 = inb(ATIIOPortCLOCK_CNTL + 1) &
        ~GetByte(PLL_WR_EN | PLL_ADDR, 1);

    /* Set PLL register to be read or written */
    outb(ATIIOPortCLOCK_CNTL + 1, clock_cntl1 |
        GetByte(SetBits(Index, PLL_ADDR) | SetBits(Write, PLL_WR_EN), 1));
}
