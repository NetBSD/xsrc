/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/aticmap.c,v 1.1.2.1 1998/02/01 16:41:45 robin Exp $ */
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

#include "aticmap.h"
#include "aticrtc.h"
#include "atiio.h"
#include "cfb.h"
#include "xf86Priv.h"

#ifdef XFreeXDGA
#   define _XF86DGA_SERVER_
#   include "extensions/xf86dga.h"
#   define ATIVTSema \
    ( \
        xf86VTSema || \
        (vga256InfoRec.directMode & XF86DGAHasColormap) || \
        ((vga256InfoRec.directMode & \
            (XF86DGADirectGraphics | XF86DGADirectColormap)) == \
            XF86DGADirectGraphics) \
    )
#else
#   define ATIVTSema (xf86VTSema)
#endif

/*
 * ATIStoreColours --
 *
 * All of the VGA layer's colourmap handling is used except for this function.
 * ... And I *know* there's lazy spellings in here, so don't razz me about it,
 * OK?
 */
void
ATIStoreColours(ColormapPtr ColourMap, int NumberOfColours, xColorItem *Colour)
{
#ifndef XF86VGA16
    xColorItem DirectColour[256];
#endif
    int Index, OverScanLUTSlot, NewOverScanLUTSlot;
    int LUTDistance, OverScanLUTDistance;
    unsigned char *LUTEntry;

    /* This is only done for installed colourmaps */
    if (vgaCheckColorMap(ColourMap))
        return;

#ifndef XF86VGA16
    /* Translate colours for TrueColor and DirectColor visuals */
    if ((ColourMap->pVisual->class | DynamicClass) == DirectColor)
    {
        NumberOfColours = cfbExpandDirectColors(ColourMap, NumberOfColours,
            Colour, DirectColour);
        Colour = DirectColour;
    }
#endif

    /* Update the DAC's LUT and our copy of it */
    for (Index = 0;  Index < NumberOfColours;  Index++)
    {
        LUTEntry = &ATINewHWPtr->std.DAC[Colour[Index].pixel * 3];
        LUTEntry[0] = Colour[Index].red >> (16 - xf86weight.red);
        LUTEntry[1] = Colour[Index].green >> (16 - xf86weight.green);
        LUTEntry[2] = Colour[Index].blue >> (16 - xf86weight.blue);

        if (ATIVTSema)
        {
            outb(ATIIOPortDAC_WRITE, Colour[Index].pixel);
            DACDelay;
            outb(ATIIOPortDAC_DATA, LUTEntry[0]);
            DACDelay;
            outb(ATIIOPortDAC_DATA, LUTEntry[1]);
            DACDelay;
            outb(ATIIOPortDAC_DATA, LUTEntry[2]);
            DACDelay;
        }
    }

    if (ATICRTC != ATI_CRTC_VGA)
        return;

    /* Check if the overscan LUT slot has been redefined */
    OverScanLUTSlot = ATINewHWPtr->std.Attribute[OVERSCAN];
    LUTEntry = &ATINewHWPtr->std.DAC[OverScanLUTSlot * 3];
    if (!LUTEntry[0] && !LUTEntry[1] && !LUTEntry[2])
        return;

    NewOverScanLUTSlot = OverScanLUTSlot;
    OverScanLUTDistance =
        (LUTEntry[0] * LUTEntry[0]) +
        (LUTEntry[1] * LUTEntry[1]) +
        (LUTEntry[2] * LUTEntry[2]);

    /* Find the entry closest to black */
    for (Index = 256;  (--Index >= 0) && OverScanLUTDistance;  )
    {
        if (Index == OverScanLUTSlot)
            continue;

        LUTEntry = &ATINewHWPtr->std.DAC[Index * 3];
        LUTDistance =
            (LUTEntry[0] * LUTEntry[0]) +
            (LUTEntry[1] * LUTEntry[1]) +
            (LUTEntry[2] * LUTEntry[2]);
        if (LUTDistance >= OverScanLUTDistance)
            continue;

        NewOverScanLUTSlot = Index;
        OverScanLUTDistance = LUTDistance;
    }

    /* Check for change */
    if (NewOverScanLUTSlot == OverScanLUTSlot)
        return;

    ATINewHWPtr->std.Attribute[OVERSCAN] = NewOverScanLUTSlot;

    /* "On screen"? */
    if (!ATIVTSema)
        return;

    /* Tell VGA CRTC where the new overscan entry is */
    (void) inb(GENS1(vgaIOBase));
    outb(ATTRX, OVERSCAN | 0x20U);
    outb(ATTRX, NewOverScanLUTSlot);
}
