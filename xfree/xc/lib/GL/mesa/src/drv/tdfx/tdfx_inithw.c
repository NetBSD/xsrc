/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_inithw.c,v 1.7 2000/12/08 19:36:23 alanh Exp $ */

/*
 * Authors:
 *   Daryll Strauss <daryll@precisioninsight.com>
 *
 */

#ifdef GLX_DIRECT_RENDERING

#include <glide.h>
#include "fxdrv.h"


GLboolean
tdfxInitHW(__DRIdrawablePrivate * driDrawPriv, fxMesaContext fxMesa)
{
    /* KW: Would be nice to make one of these a member of the other.
     */
    __DRIscreenPrivate *driScrnPriv = driDrawPriv->driScreenPriv;
    tdfxScreenPrivate *sPriv = (tdfxScreenPrivate *) driScrnPriv->private;

#ifdef DEBUG_LOCKING
    fprintf(stderr, "Debug locking enabled\n");
#endif

    if (fxMesa->initDone)
        return GL_TRUE;

    fxMesa->width = driDrawPriv->w;
    fxMesa->height = driDrawPriv->h;

    /* We have to use a light lock here, because we can't do any glide
       operations yet. No use of FX_* functions in this function. */
    DRM_LIGHT_LOCK(driScrnPriv->fd, &driScrnPriv->pSAREA->lock,
                   driDrawPriv->driContextPriv->hHWContext);
    FX_grGlideInit_NoLock();

    fxMesa->board = 0;
    FX_grSstSelect_NoLock(fxMesa->board);

    if (sPriv->deviceID == 0x3)
        fxMesa->haveTwoTMUs = GL_FALSE;
    else
        fxMesa->haveTwoTMUs = GL_TRUE;

    fxMesa->glideContext =
        FX_grSstWinOpen_NoLock((FxU32) - 1, GR_RESOLUTION_NONE,
                               GR_REFRESH_NONE, GR_COLORFORMAT_ABGR,
                               GR_ORIGIN_LOWER_LEFT, 2, 1);

    grDRIResetSAREA();
    DRM_UNLOCK(driScrnPriv->fd, &driScrnPriv->pSAREA->lock,
               driDrawPriv->driContextPriv->hHWContext);

    fxMesa->needClip = 1;

    if (!fxMesa->glideContext || !fxDDInitFxMesaContext(fxMesa))
        return GL_FALSE;

    fxMesa->initDone = GL_TRUE;

    return GL_TRUE;
}

#endif
