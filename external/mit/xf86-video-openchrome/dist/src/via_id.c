/*
 * Copyright 2005-2015 The Openchrome Project
 *                     [http://www.freedesktop.org/wiki/Openchrome]
 * Copyright 2004-2005 The Unichrome Project  [unichrome.sf.net]
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sub license,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * A big structure with card-ID information, plus some checking functions.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "via_driver.h"

static void
ViaDoubleCheckCLE266Revision(ScrnInfoPtr pScrn)
{
    vgaHWPtr hwp = VGAHWPTR(pScrn);
    VIAPtr pVia = VIAPTR(pScrn);
    /* Crtc 0x4F is only defined in CLE266Cx */
    CARD8 tmp = hwp->readCrtc(hwp, 0x4F);

    hwp->writeCrtc(hwp, 0x4F, 0x55);
    if (hwp->readCrtc(hwp, 0x4F) == 0x55) {
        if (CLE266_REV_IS_AX(pVia->ChipRev))
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "CLE266 Revision seems"
                       " to be Cx, yet %d was detected previously.\n",
                       pVia->ChipRev);
    } else {
        if (CLE266_REV_IS_CX(pVia->ChipRev))
            xf86DrvMsg(pScrn->scrnIndex, X_WARNING, "CLE266 Revision seems"
                       " to be Ax, yet %d was detected previously.\n",
                       pVia->ChipRev);
    }
    hwp->writeCrtc(hwp, 0x4F, tmp);
}
