/* -*- mode: c; c-basic-offset: 3 -*-
 *
 * Copyright 2000 VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_screen.c,v 1.1 2001/03/21 16:14:28 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *
 */

#include <X11/Xlibint.h>

#include "tdfx_dri.h"
#include "tdfx_context.h"
#include "tdfx_vb.h"
#include "tdfx_tris.h"
#include "tdfx_pipeline.h"


#ifdef DEBUG_LOCKING
char *prevLockFile = 0;
int prevLockLine = 0;
#endif


GLboolean tdfxCreateScreen( __DRIscreenPrivate *sPriv )
{
   tdfxScreenPrivate *fxScreen;
   TDFXDRIPtr fxDRIPriv = (TDFXDRIPtr) sPriv->pDevPriv;

   /* Allocate the private area */
   fxScreen = (tdfxScreenPrivate *) Xmalloc( sizeof(tdfxScreenPrivate) );
   if ( !fxScreen )
      return GL_FALSE;

   fxScreen->driScrnPriv = sPriv;
   sPriv->private = (void *) fxScreen;

   fxScreen->regs.handle	= fxDRIPriv->regs;
   fxScreen->regs.size		= fxDRIPriv->regsSize;
   fxScreen->deviceID		= fxDRIPriv->deviceID;
   fxScreen->width		= fxDRIPriv->width;
   fxScreen->height		= fxDRIPriv->height;
   fxScreen->mem		= fxDRIPriv->mem;
   fxScreen->cpp		= fxDRIPriv->cpp;
   fxScreen->stride		= fxDRIPriv->stride;
   fxScreen->fifoOffset		= fxDRIPriv->fifoOffset;
   fxScreen->fifoSize		= fxDRIPriv->fifoSize;
   fxScreen->fbOffset		= fxDRIPriv->fbOffset;
   fxScreen->backOffset		= fxDRIPriv->backOffset;
   fxScreen->depthOffset	= fxDRIPriv->depthOffset;
   fxScreen->textureOffset	= fxDRIPriv->textureOffset;
   fxScreen->textureSize	= fxDRIPriv->textureSize;
   fxScreen->sarea_priv_offset	= fxDRIPriv->sarea_priv_offset;

   if ( drmMap( sPriv->fd, fxScreen->regs.handle,
		fxScreen->regs.size, &fxScreen->regs.map ) ) {
      return GL_FALSE;
   }

   tdfxDDSetupInit();
   tdfxDDTriangleFuncsInit();
   tdfxDDFastPathInit();

   tdfxDDGlideExtensionsInit();

   return GL_TRUE;
}

void tdfxDestroyScreen( __DRIscreenPrivate *sPriv )
{
   tdfxScreenPrivate *fxScreen = (tdfxScreenPrivate *) sPriv->private;

   if ( fxScreen ) {
      drmUnmap( fxScreen->regs.map, fxScreen->regs.size );

      Xfree( fxScreen );
      sPriv->private = NULL;
   }
}
