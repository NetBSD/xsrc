/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/ati/atiaccel.c,v 1.6 2001/05/07 21:59:06 tsi Exp $ */
/*
 * Copyright 2001 by Marc Aurele La France (TSI @ UQV), tsi@xfree86.org
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

#include "atiaccel.h"
#include "atiadapter.h"
#include "atimach64.h"
#include "atistruct.h"

/*
 * ATIInitializeAcceleration --
 *
 * This function is called to initialise XAA on a screen.
 */
Bool
ATIInitializeAcceleration
(
    ScreenPtr   pScreen,
    ScrnInfoPtr pScreenInfo,
    ATIPtr      pATI
)
{
    BoxRec       ScreenArea;
    unsigned int nScanlines, maxScanlines;

    if (!pATI->OptionAccel)
        return TRUE;

    if (!(pATI->pXAAInfo = XAACreateInfoRec()))
        return FALSE;

    switch (pATI->Adapter)
    {
        case ATI_ADAPTER_MACH64:
            maxScanlines = ATIMach64AccelInit(pATI, pATI->pXAAInfo);
            break;

        default:
            maxScanlines = 0;
    }

    if (maxScanlines > 0)
    {
        ScreenArea.x1 = ScreenArea.y1 = 0;
        ScreenArea.x2 = pATI->displayWidth;
        nScanlines = pScreenInfo->videoRam * 1024 * 8 / pATI->displayWidth /
            pATI->bitsPerPixel;
        if (nScanlines > maxScanlines)
            nScanlines = maxScanlines;
        ScreenArea.y2 = (short int)nScanlines;
        xf86InitFBManager(pScreen, &ScreenArea);

        if (XAAInit(pScreen, pATI->pXAAInfo))
            return TRUE;
    }

    XAADestroyInfoRec(pATI->pXAAInfo);
    pATI->pXAAInfo = NULL;
    return FALSE;
}
