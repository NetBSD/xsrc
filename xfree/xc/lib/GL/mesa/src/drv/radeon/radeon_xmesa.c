/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_xmesa.c,v 1.4 2001/04/10 16:07:53 dawes Exp $ */
/**************************************************************************

Copyright 2000, 2001 ATI Technologies Inc., Ontario, Canada, and
                     VA Linux Systems Inc., Fremont, California.

All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, VA LINUX SYSTEMS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#ifdef GLX_DIRECT_RENDERING

/* Radeon Mesa driver includes */
#include "radeon_context.h"
#include "radeon_ioctl.h"
#include "radeon_state.h"
#include "radeon_tex.h"

/* Mesa src includes */
#include "context.h"
#include "simple_list.h"
#include "mmath.h"

extern void __driRegisterExtensions( void );

static radeonContextPtr radeonCtx = NULL;


/* Initialize the driver specific screen private data.
 */
GLboolean XMesaInitDriver( __DRIscreenPrivate *sPriv )
{
   sPriv->private = (void *) radeonCreateScreen( sPriv );

   /* Check the DRI version */
   {
      int major, minor, patch;
      if ( XF86DRIQueryVersion( sPriv->display, &major, &minor, &patch ) ) {
         if ( major != 4 || minor < 0 ) {
            char msg[128];
            sprintf( msg, "RADEON DRI driver expected DRI version 4.0.x but got version %d.%d.%d", major, minor, patch );
            __driMesaMessage( msg );
            return GL_FALSE;
         }
      }
   }

   /* Check that the DDX driver version is compatible */
   if ( sPriv->ddxMajor != 4 ||
	sPriv->ddxMinor < 0 ) {
      char msg[128];
      sprintf( msg, "RADEON DRI driver expected DDX driver version 4.0.x but got version %d.%d.%d", sPriv->ddxMajor, sPriv->ddxMinor, sPriv->ddxPatch );
      __driMesaMessage( msg );
      return GL_FALSE;
   }

   /* Check that the DRM driver version is compatible */
   if ( sPriv->drmMajor != 1 ||
	sPriv->drmMinor < 1 ) {
      char msg[128];
      sprintf( msg, "RADEON DRI driver expected DRM driver version 1.0.x but got version %d.%d.%d", sPriv->drmMajor, sPriv->drmMinor, sPriv->drmPatch );
      __driMesaMessage( msg );
      return GL_FALSE;
   }

   if ( !sPriv->private ) {
      radeonDestroyScreen( sPriv );
      return GL_FALSE;
   }

   return GL_TRUE;
}

/* Reset the driver specific screen private data.
 */
void XMesaResetDriver( __DRIscreenPrivate *sPriv )
{
   radeonDestroyScreen( sPriv );
}

/* Create and initialize the Mesa and driver specific visual data.
 */
GLvisual *XMesaCreateVisual( Display *dpy,
			     __DRIscreenPrivate *driScrnPriv,
			     const XVisualInfo *visinfo,
			     const __GLXvisualConfig *config )
{
   /* Drivers may change the args to _mesa_create_visual() in order to
    * setup special visuals.
     */
   return _mesa_create_visual( config->rgba,
			       config->doubleBuffer,
			       config->stereo,
			       _mesa_bitcount( visinfo->red_mask ),
			       _mesa_bitcount( visinfo->green_mask ),
			       _mesa_bitcount( visinfo->blue_mask ),
			       config->alphaSize,
			       0, /* index bits */
			       config->depthSize,
			       config->stencilSize,
			       config->accumRedSize,
			       config->accumGreenSize,
			       config->accumBlueSize,
			       config->accumAlphaSize,
			       0 /* num samples */);
}

/* Create and initialize the Mesa and driver specific context data.
 */
GLboolean XMesaCreateContext( Display *dpy, GLvisual *mesaVis,
			      __DRIcontextPrivate *driContextPriv )
{
   return radeonCreateContext( dpy, mesaVis, driContextPriv );
}

/* Destroy the Mesa and driver specific context data.
 */
void XMesaDestroyContext( __DRIcontextPrivate *driContextPriv )
{
   radeonContextPtr rmesa = (radeonContextPtr)driContextPriv->driverPrivate;

   if ( rmesa == (void *)radeonCtx) radeonCtx = NULL;
   radeonDestroyContext( rmesa );
}

/* Create and initialize the Mesa and driver specific pixmap buffer
 * data.
 */
GLframebuffer *XMesaCreateWindowBuffer( Display *dpy,
					__DRIscreenPrivate *driScrnPriv,
					__DRIdrawablePrivate *driDrawPriv,
					GLvisual *mesaVis )
{
   return gl_create_framebuffer( mesaVis,
				 GL_FALSE, /* software depth buffer? */
				 mesaVis->StencilBits > 0,
				 mesaVis->AccumRedBits > 0,
				 GL_FALSE  /* software alpha buffer? */ );
}

/* Create and initialize the Mesa and driver specific pixmap buffer
 * data.
 */
GLframebuffer *XMesaCreatePixmapBuffer( Display *dpy,
					__DRIscreenPrivate *driScrnPriv,
					__DRIdrawablePrivate *driDrawPriv,
					GLvisual *mesaVis )
{
#if 0
   /* Different drivers may have different combinations of hardware and
    * software ancillary buffers.
    */
   return gl_create_framebuffer( mesaVis,
				 GL_FALSE,  /* software depth buffer? */
				 mesaVis->StencilBits > 0,
				 mesaVis->AccumRedBits > 0,
				 mesaVis->AlphaBits > 0 );
#else
   return NULL;  /* not implemented yet */
#endif
}

/* Copy the back color buffer to the front color buffer.
 */
void XMesaSwapBuffers( __DRIdrawablePrivate *driDrawPriv )
{
   /* FIXME: This assumes buffer is currently bound to a context.  This
    * needs to be able to swap buffers when not currently bound.  Also,
    * this needs to swap according to buffer, and NOT according to
    * context!
     */
   if ( radeonCtx == NULL ) return;

   /* Only swap buffers when a back buffer exists.
     */
   if ( radeonCtx->glCtx->Visual->DBflag ) {
      FLUSH_VB( radeonCtx->glCtx, "swap buffers" );
      if ( !radeonCtx->doPageFlip ) {
	 radeonSwapBuffers( radeonCtx );
      } else {
	 radeonPageFlip( radeonCtx );
      }
   }
}

/* Force the context `c' to be the current context and associate with it
 * buffer `b'.
 */
GLboolean XMesaMakeCurrent( __DRIcontextPrivate *driContextPriv,
			    __DRIdrawablePrivate *driDrawPriv,
			    __DRIdrawablePrivate *driReadPriv )
{
   if ( driContextPriv ) {
      radeonContextPtr rmesa = (radeonContextPtr)driContextPriv->driverPrivate;

      radeonCtx = radeonMakeCurrent( radeonCtx, rmesa, driDrawPriv );

      gl_make_current2( radeonCtx->glCtx,
			driDrawPriv->mesaBuffer,
			driReadPriv->mesaBuffer );

      if ( radeonCtx->driDrawable != driDrawPriv ) {
	 radeonCtx->driDrawable = driDrawPriv;
	 radeonCtx->dirty = RADEON_UPLOAD_ALL;
      }

      /* GH: We need this to correctly calculate the window offset
       * and aux scissor rects.
       */
      radeonCtx->new_state = RADEON_NEW_WINDOW | RADEON_NEW_CLIP;

      if ( !radeonCtx->glCtx->Viewport.Width ) {
	 gl_Viewport( radeonCtx->glCtx, 0, 0, driDrawPriv->w, driDrawPriv->h );
      }
   } else {
      gl_make_current( 0, 0 );
      radeonCtx = NULL;
   }

   return GL_TRUE;
}

/* Force the context `c' to be unbound from its buffer.
 */
GLboolean XMesaUnbindContext( __DRIcontextPrivate *driContextPriv )
{
   return GL_TRUE;
}

/* This function is called by libGL.so as soon as libGL.so is loaded.
 * This is where we'd register new extension functions with the dispatcher.
 */
void __driRegisterExtensions( void )
{
}

/* Initialize the fullscreen mode.
 */
GLboolean
XMesaOpenFullScreen( __DRIcontextPrivate *driContextPriv )
{
   radeonContextPtr rmesa = (radeonContextPtr)driContextPriv->driverPrivate;
   GLint ret;

   /* FIXME: Do we need to check this?
    */
   if ( !radeonCtx->glCtx->Visual->DBflag )
      return GL_TRUE;

   LOCK_HARDWARE( rmesa );
   radeonWaitForIdleLocked( rmesa );

   /* Ignore errors.  If this fails, we simply don't do page flipping.
    */
   ret = drmRadeonFullScreen( rmesa->driFd, GL_TRUE );

   UNLOCK_HARDWARE( rmesa );

   rmesa->doPageFlip = ( ret == 0 );

   return GL_TRUE;
}

/* Shut down the fullscreen mode.
 */
GLboolean
XMesaCloseFullScreen( __DRIcontextPrivate *driContextPriv )
{
   radeonContextPtr rmesa = (radeonContextPtr)driContextPriv->driverPrivate;

   LOCK_HARDWARE( rmesa );
   radeonWaitForIdleLocked( rmesa );

   /* Don't care if this fails, we're not page flipping anymore.
    */
   drmRadeonFullScreen( rmesa->driFd, GL_FALSE );

   UNLOCK_HARDWARE( rmesa );

   rmesa->doPageFlip = GL_FALSE;
   rmesa->currentPage = 0;

   return GL_TRUE;
}

#endif
