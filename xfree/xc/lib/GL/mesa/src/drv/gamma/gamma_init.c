/* $XFree86: xc/lib/GL/mesa/src/drv/gamma/gamma_init.c,v 1.2 2000/02/23 04:46:43 martin Exp $ */
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

/*
 * Authors:
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *
 */

#ifdef GLX_DIRECT_RENDERING

#include <X11/Xlibint.h>
#include "xf86dri.h"
#include "glint_dri.h"
#include "gamma_init.h"

static void performMagic(__DRIscreenPrivate *driScrnPriv)
{
    gammaScreenPrivate *gPriv = (gammaScreenPrivate *)driScrnPriv->private;
    GLINTDRIPtr         gDRIPriv = (GLINTDRIPtr)driScrnPriv->pDevPriv;

    gPriv->regionCount  = 4;	/* Magic number.  Can we fix this? */
    
    gPriv->regions = Xmalloc(gPriv->regionCount * sizeof(gammaRegion));

    gPriv->regions[0].handle = gDRIPriv->hControlRegs0;
    gPriv->regions[0].size   = gDRIPriv->sizeControlRegs0;
    gPriv->regions[1].handle = gDRIPriv->hControlRegs1;
    gPriv->regions[1].size   = gDRIPriv->sizeControlRegs1;
    gPriv->regions[2].handle = gDRIPriv->hControlRegs2;
    gPriv->regions[2].size   = gDRIPriv->sizeControlRegs2;
    gPriv->regions[3].handle = gDRIPriv->hControlRegs3;
    gPriv->regions[3].size   = gDRIPriv->sizeControlRegs3;
}

GLboolean gammaMapAllRegions(__DRIscreenPrivate *driScrnPriv)
{
    gammaScreenPrivate *gPriv = (gammaScreenPrivate *)driScrnPriv->private;
    int i;
    
    /* First, pick apart pDevPriv & friends */
    performMagic(driScrnPriv);

    /* Next, map all the regions */
    for (i = 0; i < gPriv->regionCount; i++) {
	if (drmMap(driScrnPriv->fd,
		   gPriv->regions[i].handle,
		   gPriv->regions[i].size,
		   &gPriv->regions[i].map)) {
	    while (--i > 0) {
		(void)drmUnmap(gPriv->regions[i].map, gPriv->regions[i].size);
	    }
	    return GL_FALSE;
	}
    }

    /* Get the list of dma buffers */
    gPriv->bufs = drmMapBufs(driScrnPriv->fd);
    if (!gPriv->bufs) {
	while (gPriv->regionCount > 0) {
	    (void)drmUnmap(gPriv->regions[gPriv->regionCount].map,
			   gPriv->regions[gPriv->regionCount].size);
	    gPriv->regionCount--;
	}
	return GL_FALSE;
    }

    return GL_TRUE;
}

void gammaUnmapAllRegions(__DRIscreenPrivate *driScrnPriv)
{
    gammaScreenPrivate *gPriv = (gammaScreenPrivate *)driScrnPriv->private;
    
    /* First, unmap the dma buffers */
    drmUnmapBufs(gPriv->bufs);

    /* Next, unmap all the regions */
    while (gPriv->regionCount > 0) {
	(void)drmUnmap(gPriv->regions[gPriv->regionCount].map,
		       gPriv->regions[gPriv->regionCount].size);
	gPriv->regionCount--;
    }
    Xfree(gPriv->regions);
}

#endif
