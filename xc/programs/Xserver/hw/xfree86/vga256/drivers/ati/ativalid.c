/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/ativalid.c,v 1.1.2.2 1999/07/05 09:07:36 hohndel Exp $ */
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

#include "atiadapter.h"
#include "atichip.h"
#include "aticrtc.h"
#include "atiregs.h"
#include "ativalid.h"

/*
 * NOTE:  The numbers in here should eventually be related to the appropriate
 *        bit-field #define's.
 */

/*
 * ATIValidMode --
 *
 * This checks for hardware-related limits on mode timings.  This assumes
 * xf86CheckMode has already done some basic consistency checks.
 */
int
ATIValidMode(DisplayModePtr mode, const Bool verbose, const int Flag)
{
    int Limit;
    int VDisplay = mode->VDisplay;
    int VTotal = mode->VTotal;

    if ((mode->Flags & V_DBLSCAN) && (ATILCDPanelID < 0))
    {
        VDisplay <<= 1;
        VTotal <<= 1;
    }

    switch (ATICRTC)
    {
        case ATI_CRTC_VGA:
            if ((mode->HDisplay >= 2056) || (mode->HTotal >= 2088))
            {
                if (verbose)
                    ErrorF("Mode \"%s\" is too wide.  Deleted.\n", mode->name);
                return MODE_HSYNC;
            }

            if ((mode->Flags & V_INTERLACE) && (ATIChip < ATI_CHIP_264CT))
            {
                VDisplay >>= 1;
                VTotal >>= 1;
            }

            if ((VDisplay > 2048) || (VTotal > 2050))
            {
                if (verbose)
                    ErrorF("Mode \"%s\" is too high.  Deleted.\n", mode->name);
                return MODE_VSYNC;
            }

            if (ATIAdapter != ATI_ADAPTER_VGA)
                break;

            if (mode->Flags & V_INTERLACE)
            {
                if (verbose)
                    ErrorF("Interlaced modes not supported by generic VGA."
                           "  Mode \"%s\" deleted.\n", mode->name);
                return MODE_VSYNC;
            }

            if ((VDisplay > 1024) || (VTotal > 1025))
            {
                if (verbose)
                    ErrorF("Mode \"%s\" is too high for generic VGA."
                           "  Deleted.\n", mode->name);
                return MODE_VSYNC;
            }
            break;

        case ATI_CRTC_MACH64:
            Limit = (GetBits(CRTC_H_TOTAL, CRTC_H_TOTAL) + 1) << 3;
            if (ATIChip < ATI_CHIP_264VT)
                Limit >>= 1;            /* CRTC_H_TOTAL is 1 bit narrower */
            if (mode->HTotal > Limit)
            {
                if (verbose)
                    ErrorF("Mode \"%s\" is too wide.  Deleted.\n", mode->name);
                return MODE_HSYNC;
            }

            if ((mode->HTotal >> 3) == (mode->HDisplay >> 3))
            {
                if (verbose)
                    ErrorF("Horizontal sync pulse too narrow.  Mode \"%s\""
                           " deleted.\n", mode->name);
                return MODE_HSYNC;
            }

            if (VTotal > ((int)GetBits(CRTC_V_TOTAL, CRTC_V_TOTAL) + 1))
            {
                if (verbose)
                    ErrorF("Mode \"%s\" is too high.  Deleted.\n", mode->name);
                return MODE_VSYNC;
            }

            /*
             * ATI finally fixed accelerated doublescanning in the 264VT and
             * later.  On 88800's, the bit is documented to exist, but only
             * doubles the vertical timings.  On the 264CT & 264ET, the bit is
             * ignored.
             */
            if ((ATIChip < ATI_CHIP_264VT) && (mode->Flags & V_DBLSCAN))
            {
                if (verbose)
                    ErrorF("The %s does not support accelerated doublescanned"
                           " modes.\n Mode \"%s\" deleted.\n",
                        ATIChipNames[ATIChip], mode->name);
                return MODE_VSYNC;
            }

            break;

        default:
            break;
    }

    /* Compare mode against panel dimensions */
    if ((ATILCDPanelID >= 0) &&
        ((mode->HDisplay > ATILCDHorizontal) ||
         (mode->VDisplay > ATILCDVertical)))
    {
        if (verbose)
            ErrorF("Mode \"%s\" exceeds panel dimensions.  Deleted.\n",
                mode->name);
        return MODE_BAD;
    }

    return MODE_OK;
}
