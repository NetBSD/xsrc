/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_xmesa.c,v 1.12.4.1 2002/01/27 19:18:39 dawes Exp $ */
/**************************************************************************

Copyright 1999, 2000 ATI Technologies Inc. and Precision Insight, Inc.,
                                               Cedar Park, Texas.
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
ATI, PRECISION INSIGHT AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
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

/* r128 Mesa driver includes */
#include "r128_context.h"
#include "r128_ioctl.h"
#include "r128_state.h"
#include "r128_tex.h"

/* Mesa src includes */
#include "context.h"
#include "simple_list.h"
#include "mmath.h"

extern void __driRegisterExtensions( void );

static r128ContextPtr r128Ctx = NULL;


/* Initialize the driver specific screen private data.
 */
GLboolean XMesaInitDriver( __DRIscreenPrivate *sPriv )
{
   sPriv->private = (void *) r128CreateScreen( sPriv );

   /* Check the DRI version */
   {
      int major, minor, patch;
      if ( XF86DRIQueryVersion( sPriv->display, &major, &minor, &patch ) ) {
         if ( major != 4 || minor < 0 ) {
            char msg[1000];
            sprintf( msg, "R128 DRI driver expected DRI version 4.0.x but got version %d.%d.%d", major, minor, patch );
            __driMesaMessage( msg );
            return GL_FALSE;
         }
      }
   }

   /* Check that the DDX driver version is compatible */
   if ( sPriv->ddxMajor != 4 ||
	sPriv->ddxMinor < 0 ) {
      char msg[1000];
      sprintf( msg, "R128 DRI driver expected DDX driver version 4.0.x but got version %d.%d.%d", sPriv->ddxMajor, sPriv->ddxMinor, sPriv->ddxPatch );
      __driMesaMessage( msg );
      return GL_FALSE;
   }

   /* Check that the DRM driver version is compatible */
   if ( sPriv->drmMajor != 2 ||
	sPriv->drmMinor < 2 ) {
      char msg[1000];
      sprintf( msg, "R128 DRI driver expected DRM driver version 2.2 or greater but got version %d.%d.%d", sPriv->drmMajor, sPriv->drmMinor, sPriv->drmPatch );
      __driMesaMessage( msg );
      return GL_FALSE;
   }

   if ( !sPriv->private ) {
      r128DestroyScreen( sPriv );
      return GL_FALSE;
   }

   return GL_TRUE;
}

/* Reset the driver specific screen private data.
 */
void XMesaResetDriver( __DRIscreenPrivate *sPriv )
{
   r128DestroyScreen( sPriv );
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
			       0 /* num samples */ );
}

/* Create and initialize the Mesa and driver specific context data.
 */
GLboolean XMesaCreateContext( Display *dpy, GLvisual *mesaVis,
			      __DRIcontextPrivate *driContextPriv )
{
   return r128CreateContext( dpy, mesaVis, driContextPriv );
}

/* Destroy the Mesa and driver specific context data.
 */
void XMesaDestroyContext(__DRIcontextPrivate *driContextPriv)
{
   r128ContextPtr rmesa = (r128ContextPtr)driContextPriv->driverPrivate;

   if ( rmesa == r128Ctx ) r128Ctx = NULL;
   r128DestroyContext(rmesa);
}

/* Create and initialize the Mesa and driver specific pixmap buffer
 * data.
 */
GLframebuffer *XMesaCreateWindowBuffer( Display *dpy,
					__DRIscreenPrivate *driScrnPriv,
					__DRIdrawablePrivate *driDrawPriv,
					GLvisual *mesaVis )
{
   GLboolean swStencil;
   swStencil = (mesaVis->StencilBits > 0) /* && (mesaVis->RedBits < 8) */ ;
   return gl_create_framebuffer( mesaVis,
				 GL_FALSE,  /* software depth buffer? */
				 swStencil,
				 mesaVis->AccumRedBits > 0,
				 GL_FALSE   /* software alpha buffer? */ );
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

/* Copy the back color buffer to the front color buffer */
void XMesaSwapBuffers( __DRIdrawablePrivate *driDrawPriv )
{
   /* FIXME: This assumes buffer is currently bound to a context.  This
    * needs to be able to swap buffers when not currently bound.  Also,
    * this needs to swap according to buffer, and NOT according to
    * context!
    */
   if ( r128Ctx == NULL )
      return;

    /* Only swap buffers when a back buffer exists.
     */
   if ( r128Ctx->glCtx->Visual->DBflag ) {
      FLUSH_VB( r128Ctx->glCtx, "swap buffers" );
      if ( !r128Ctx->doPageFlip ) {
	 r128SwapBuffers( r128Ctx );
      } else {
	 r128PageFlip( r128Ctx );
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
      r128ContextPtr rmesa = (r128ContextPtr)driContextPriv->driverPrivate;

      r128Ctx = r128MakeCurrent( r128Ctx, rmesa, driDrawPriv );

      gl_make_current2( r128Ctx->glCtx,
			driDrawPriv->mesaBuffer,
			driReadPriv->mesaBuffer );

      if ( r128Ctx->driDrawable != driDrawPriv ) {
	 r128Ctx->driDrawable = driDrawPriv;
	 r128Ctx->dirty = R128_UPLOAD_ALL;
      }

      /* GH: We need this to correctly calculate the window offset
       * and aux scissor rects.
       */
      r128Ctx->new_state = R128_NEW_WINDOW | R128_NEW_CLIP;

      if ( !r128Ctx->glCtx->Viewport.Width ) {
	 gl_Viewport( r128Ctx->glCtx, 0, 0, driDrawPriv->w, driDrawPriv->h );
      }
   } else {
      gl_make_current( 0, 0 );
      r128Ctx = NULL;
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
 * This is where we'd register new extension functions with the
 * dispatcher.
 */
void __driRegisterExtensions( void )
{
}

/* Initialize the fullscreen mode.
 */
GLboolean
XMesaOpenFullScreen( __DRIcontextPrivate *driContextPriv )
{
#if 0
   r128ContextPtr rmesa = (r128ContextPtr)driContextPriv->driverPrivate;
   GLint ret;

   /* FIXME: Do we need to check this?
    */
   if ( !r128Ctx->glCtx->Visual->DBflag )
      return GL_TRUE;

   LOCK_HARDWARE( rmesa );
   r128WaitForIdleLocked( rmesa );

   /* Ignore errors.  If this fails, we simply don't do page flipping.
    */
   ret = drmR128FullScreen( rmesa->driFd, GL_TRUE );

   UNLOCK_HARDWARE( rmesa );

   rmesa->doPageFlip = ( ret == 0 );
#endif

   return GL_TRUE;
}

/* Shut down the fullscreen mode.
 */
GLboolean
XMesaCloseFullScreen( __DRIcontextPrivate *driContextPriv )
{
#if 0
   r128ContextPtr rmesa = (r128ContextPtr)driContextPriv->driverPrivate;

   LOCK_HARDWARE( rmesa );
   r128WaitForIdleLocked( rmesa );

   /* Don't care if this fails, we're not page flipping anymore.
    */
   drmR128FullScreen( rmesa->driFd, GL_FALSE );

   UNLOCK_HARDWARE( rmesa );

   rmesa->doPageFlip = GL_FALSE;
   rmesa->currentPage = 0;
#endif

   return GL_TRUE;
}

#endif
