/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atiadjust.c,v 1.1.2.1 1998/02/01 16:41:40 robin Exp $ */
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

#include "atiadjust.h"
#include "atichip.h"
#include "aticonsole.h"
#include "aticrtc.h"
#include "atidepth.h"
#include "atiio.h"

#ifdef XFreeXDGA
#   define _XF86DGA_SERVER_
#   include "extensions/xf86dga.h"
#endif

/*
 * The display start address is expressed in units of 32-bit (VGA) or 64-bit
 * (accelerator) words where all planar modes are considered as 4bpp modes.
 * These functions ensure the start address does not exceed architectural
 * limits.  Also, to avoid colour changes while panning, these 32-bit or 64-bit
 * boundaries may not fall within a pixel.
 */

static int           ATIAdjustDepth;
static unsigned long ATIAdjustMask;
static int           ATIAdjustMaxX,
                     ATIAdjustMaxY;

/*
 * ATIAjustInit  --
 *
 * This function calculates values needed to speed up the setting of the
 * display start address.
 */
void
ATIAdjustInit(void)
{
    unsigned long MaxBase = 0;

    ATIAdjustDepth = (vga256InfoRec.bitsPerPixel + 7) >> 3;

    ATIAdjustMask = 64;
    while (ATIAdjustMask % (unsigned long)ATIAdjustDepth)
        ATIAdjustMask += 64;
    ATIAdjustMask =
        ~(((ATIAdjustMask / (unsigned long)ATIAdjustDepth) >> 3) - 1);

    switch (ATICRTC)
    {
        case ATI_CRTC_VGA:
            if (ATIChip >= ATI_CHIP_264CT)
            {
                MaxBase = GetBits(CRTC_OFFSET_VGA, CRTC_OFFSET_VGA) << 2;
                if (ATIUsingPlanarModes)
                    MaxBase <<= 1;
            }
            else if (!ATIChipHasVGAWonder)
                MaxBase = 0xFFFFU << 3;
            else if (ATIChip <= ATI_CHIP_28800_6)
                MaxBase = 0x3FFFFU << 3;
            else /* Mach32 & Mach64 */
                MaxBase = 0xFFFFFU << 3;
            break;

        case ATI_CRTC_MACH64:
            MaxBase = GetBits(CRTC_OFFSET, CRTC_OFFSET) << 3;
            break;
    }

    MaxBase = (MaxBase / (unsigned long)ATIAdjustDepth) | ~ATIAdjustMask;

    ATIAdjustMaxX = MaxBase % vga256InfoRec.displayWidth;
    ATIAdjustMaxY = MaxBase / vga256InfoRec.displayWidth;
}

/*
 * ATIAdjust --
 *
 * This function is used to initialize the SVGA Start Address - the first
 * displayed location in video memory.  This is used to implement the virtual
 * window.
 */
void
ATIAdjust(int x, int y)
{
    int Base;

    /*
     * Assume the caller has already done its homework in ensuring the physical
     * screen is still contained in the virtual resolution.
     */
    if (y >= ATIAdjustMaxY)
    {
        y = ATIAdjustMaxY;
        if (x > ATIAdjustMaxX)
            y--;
    }

    Base = ((((y * vga256InfoRec.displayWidth) + x) & ATIAdjustMask) *
            ATIAdjustDepth) >> 3;

    /* Unlock registers */
    ATIEnterLeave(ENTER);

    if ((ATICRTC == ATI_CRTC_VGA) && (ATIChip < ATI_CHIP_264CT))
    {
        PutReg(CRTX(vgaIOBase), 0x0CU, GetByte(Base, 1));
        PutReg(CRTX(vgaIOBase), 0x0DU, GetByte(Base, 0));

        if (ATIChipHasVGAWonder)
        {
            if (ATIChip <= ATI_CHIP_18800_1)
                ATIModifyExtReg(0xB0U, -1, 0x3FU, Base >> 10);
            else
            {
                ATIModifyExtReg(0xB0U, -1, 0xBFU, Base >> 10);
                ATIModifyExtReg(0xA3U, -1, 0xEFU, Base >> 13);

                /*
                 * I don't know if this also applies to Mach64's, but give it a
                 * shot...
                 */
                if (ATIChip >= ATI_CHIP_68800)
                    ATIModifyExtReg(0xADU, -1, 0xF3U, Base >> 16);
            }
        }
    }
    else
    {
        /*
         * On integrated controllers, there is only one set of CRTC control
         * bits, many of which are simultaneously accessible through both VGA
         * and accelerator I/O ports.  Given VGA's architectural limitations,
         * setting the CRTC's offset register to more than 256k needs to be
         * done through the accelerator port.
         */
        if (ATIUsingPlanarModes)
        {
            outl(ATIIOPortCRTC_OFF_PITCH,
                SetBits(vga256InfoRec.displayWidth >> 4, CRTC_PITCH) |
                    SetBits(Base, CRTC_OFFSET));
        }
        else
        {
            if (ATICRTC == ATI_CRTC_VGA)
                Base <<= 1;                     /* LSBit must be zero */
            outl(ATIIOPortCRTC_OFF_PITCH,
                SetBits(vga256InfoRec.displayWidth >> 3, CRTC_PITCH) |
                    SetBits(Base, CRTC_OFFSET));
        }
    }

#   ifdef XFreeXDGA
        if (!ATIUsing1bppModes &&
            (vga256InfoRec.directMode & XF86DGADirectGraphics))
            switch (ATICRTC)
            {
                case ATI_CRTC_VGA:
                    /* Wait until vertical retrace is in progress */
                    while (inb(GENS1(vgaIOBase)) & 0x08U);
                    while (!(inb(GENS1(vgaIOBase)) & 0x08U));
                    break;

                case ATI_CRTC_MACH64:
                    /* Wait until vertical retrace is in progress */
                    while (inb(ATIIOPortCRTC_INT_CNTL) & CRTC_VBLANK);
                    while (!(inb(ATIIOPortCRTC_INT_CNTL) & CRTC_VBLANK));
                    break;

                default:
                    break;
            }
#   endif
}
