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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_init.c,v 1.5 2000/09/26 15:56:50 tsi Exp $ */

/*
 * Authors:
 *   Daryll Strauss <daryll@precisioninsight.com>
 *
 */

#ifdef GLX_DIRECT_RENDERING

#include <X11/Xlibint.h>
#include "xf86dri.h"
#include "tdfx_dri.h"
#include "fxdrv.h"


#ifdef DEBUG_LOCKING
char *prevLockFile = 0;
int prevLockLine = 0;
#endif

static void
performMagic(__DRIscreenPrivate * driScrnPriv)
{
    tdfxScreenPrivate *gPriv = (tdfxScreenPrivate *) driScrnPriv->private;
    TDFXDRIPtr gDRIPriv = (TDFXDRIPtr) driScrnPriv->pDevPriv;

    gPriv->regs.handle = gDRIPriv->regs;
    gPriv->regs.size = gDRIPriv->regsSize;
    gPriv->deviceID = gDRIPriv->deviceID;
    gPriv->width = gDRIPriv->width;
    gPriv->height = gDRIPriv->height;
    gPriv->mem = gDRIPriv->mem;
    gPriv->cpp = gDRIPriv->cpp;
    gPriv->stride = gDRIPriv->stride;
    gPriv->fifoOffset = gDRIPriv->fifoOffset;
    gPriv->fifoSize = gDRIPriv->fifoSize;
    gPriv->fbOffset = gDRIPriv->fbOffset;
    gPriv->backOffset = gDRIPriv->backOffset;
    gPriv->depthOffset = gDRIPriv->depthOffset;
    gPriv->textureOffset = gDRIPriv->textureOffset;
    gPriv->textureSize = gDRIPriv->textureSize;
}

GLboolean
tdfxMapAllRegions(__DRIscreenPrivate * driScrnPriv)
{
    tdfxScreenPrivate *gPriv = (tdfxScreenPrivate *) driScrnPriv->private;

    /* First, pick apart pDevPriv & friends */
    performMagic(driScrnPriv);

    if (drmMap(driScrnPriv->fd, gPriv->regs.handle, gPriv->regs.size,
               &gPriv->regs.map)) {
        return GL_FALSE;
    }

    return GL_TRUE;
}

void
tdfxUnmapAllRegions(__DRIscreenPrivate * driScrnPriv)
{
    tdfxScreenPrivate *gPriv = (tdfxScreenPrivate *) driScrnPriv->private;

    drmUnmap(gPriv->regs.map, gPriv->regs.size);
}


#endif
