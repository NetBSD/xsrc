/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/atiload.c,v 1.1 2000/10/13 13:27:00 tsi Exp $ */
/*
 * Copyright 2000 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
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

#ifdef XFree86LOADER

#include "ati.h"
#include "atiload.h"
#include "atistruct.h"

/*
 * ATILoadModule --
 *
 * Load a specific module and register its main entry with the loader.
 */
static Bool
ATILoadModule
(
    ScrnInfoPtr pScreenInfo,
    const char *Module,
    const char *Symbol
)
{
    if (!xf86LoadSubModule(pScreenInfo, Module))
        return FALSE;

    xf86LoaderReqSymbols(Symbol, NULL);

    return TRUE;
}

/*
 * ATILoadModules --
 *
 * This function loads other modules required for a screen.
 */
Bool
ATILoadModules
(
    ScrnInfoPtr pScreenInfo,
    ATIPtr      pATI
)
{
    /*
     * Tell loader about symbols from other modules that this module might
     * refer to.
     */
    LoaderRefSymbols(

#ifndef AVOID_CPIO

        "xf1bppScreenInit",
        "xf4bppScreenInit",

#endif /* AVOID_CPIO */

        "cfbScreenInit",
        "cfb16ScreenInit",
        "cfb24ScreenInit",
        "cfb32ScreenInit",
        "ShadowFBInit",
        "XAACreateInfoRec",
        "XAADestroyInfoRec",
        "XAAInit",
        NULL);

    /* Load shadow frame buffer code if needed */
    if (pATI->OptionShadowFB &&
        !ATILoadModule(pScreenInfo, "shadowfb", "ShadowFBInit"))
        return FALSE;

    /* Load XAA if needed */
    if (pATI->OptionAccel)
    {
        if (!ATILoadModule(pScreenInfo, "xaa", "XAAInit"))
            return FALSE;

        /* Require more XAA symbols */
        xf86LoaderReqSymbols("XAACreateInfoRec", "XAADestroyInfoRec", NULL);
    }

    /* Load depth-specific entry points */
    switch (pATI->bitsPerPixel)
    {

#ifndef AVOID_CPIO

        case 1:
            return ATILoadModule(pScreenInfo, "xf1bpp", "xf1bppScreenInit");

        case 4:
            return ATILoadModule(pScreenInfo, "xf4bpp", "xf4bppScreenInit");

#endif /* AVOID_CPIO */

        case 8:
            return ATILoadModule(pScreenInfo, "cfb", "cfbScreenInit");

        case 16:
            return ATILoadModule(pScreenInfo, "cfb16", "cfb16ScreenInit");

        case 24:
            return ATILoadModule(pScreenInfo, "cfb24", "cfb24ScreenInit");

        case 32:
            return ATILoadModule(pScreenInfo, "cfb32", "cfb32ScreenInit");

        default:
            return FALSE;
    }
}

#endif /* XFree86LOADER */
