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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_xmesa.c,v 1.12 2001/03/21 16:14:28 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Brian Paul <brianp@valinux.com>
 *
 */

#ifdef GLX_DIRECT_RENDERING

#include <X11/Xlibint.h>

#include "context.h"
#include "matrix.h"
#include "mmath.h"
#include "vbxform.h"

#include "dri_glide.h"

#include "tdfx_context.h"
#include "tdfx_render.h"
#include "tdfx_state.h"
#include "tdfx_texman.h"


GLboolean
XMesaInitDriver( __DRIscreenPrivate *sPriv )
{
   int major, minor, patch;
   char msg[1024];

   if ( TDFX_DEBUG & DEBUG_VERBOSE_DRI ) {
      fprintf( stderr, "%s( %p )\n", __FUNCTION__, sPriv );
   }

   /* Check the DRI version */
   if ( XF86DRIQueryVersion( sPriv->display, &major, &minor, &patch ) ) {
      if ( major != 4 ||
	   minor < 0 ) {
	 sprintf( msg,
		  "3dfx DRI driver expected DRI version 4.0.x "
		  "but got version %d.%d.%d",
		  major, minor, patch );
	 __driMesaMessage( msg );
	 return GL_FALSE;
      }
   }

   /* Check that the DDX driver version is compatible */
   if ( sPriv->ddxMajor != 1 ||
	sPriv->ddxMinor < 0 ) {
      sprintf( msg,
	       "3dfx DRI driver expected DDX driver version 1.0.x "
	       "but got version %d.%d.%d",
	       sPriv->ddxMajor, sPriv->ddxMinor, sPriv->ddxPatch );
      __driMesaMessage( msg );
      return GL_FALSE;
   }

   /* Check that the DRM driver version is compatible */
   if ( sPriv->drmMajor != 1 ||
	sPriv->drmMinor < 0 ) {
      sprintf( msg,
	       "3dfx DRI driver expected DRM driver version 1.0.x "
	       "but got version %d.%d.%d",
	       sPriv->drmMajor, sPriv->drmMinor, sPriv->drmPatch );
      __driMesaMessage( msg );
      return GL_FALSE;
   }

   if ( !tdfxCreateScreen( sPriv ) ) {
      tdfxDestroyScreen( sPriv );
      return GL_FALSE;
   }

   return GL_TRUE;
}


void
XMesaResetDriver( __DRIscreenPrivate *sPriv )
{
   if ( TDFX_DEBUG & DEBUG_VERBOSE_DRI ) {
      fprintf( stderr, "%s( %p )\n", __FUNCTION__, sPriv );
   }

   tdfxDestroyScreen( sPriv );
}


GLvisual *
XMesaCreateVisual( Display * dpy,
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


GLboolean
XMesaCreateContext( Display *dpy, GLvisual *mesaVis,
		    __DRIcontextPrivate *driContextPriv )
{
   if ( TDFX_DEBUG & DEBUG_VERBOSE_DRI ) {
      fprintf( stderr, "%s( %p )\n", __FUNCTION__, driContextPriv );
   }

   return tdfxCreateContext( dpy, mesaVis, driContextPriv );
}


void
XMesaDestroyContext( __DRIcontextPrivate *driContextPriv )
{
   tdfxContextPtr fxMesa;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_DRI ) {
      fprintf( stderr, "%s( %p )\n", __FUNCTION__, driContextPriv );
   }

   fxMesa = (tdfxContextPtr) driContextPriv->driverPrivate;
   tdfxDestroyContext( fxMesa );
   driContextPriv->driverPrivate = NULL;
}


GLframebuffer *
XMesaCreateWindowBuffer( Display *dpy,
			 __DRIscreenPrivate *driScrnPriv,
			 __DRIdrawablePrivate *driDrawPriv,
			 GLvisual *mesaVis )
{
   return gl_create_framebuffer( mesaVis,
				 GL_FALSE, /* software depth buffer? */
				 mesaVis->StencilBits > 0,
				 mesaVis->AccumRedBits > 0,
				 GL_FALSE /* software alpha channel? */ );
}


GLframebuffer *
XMesaCreatePixmapBuffer( Display *dpy,
			 __DRIscreenPrivate *driScrnPriv,
			 __DRIdrawablePrivate *driDrawPriv,
			 GLvisual *mesaVis )
{
#if 0
   /* Different drivers may have different combinations of hardware and
    * software ancillary buffers.
    */
   return gl_create_framebuffer( mesaVis,
				 GL_FALSE, /* software depth buffer? */
				 mesaVis->StencilBits > 0,
				 mesaVis->AccumRedBits > 0,
				 mesaVis->AlphaBits > 0 );
#else
   return NULL;                /* not implemented yet */
#endif
}

void
XMesaSwapBuffers( __DRIdrawablePrivate *driDrawPriv )
{
   GET_CURRENT_CONTEXT(ctx);
   tdfxContextPtr fxMesa = 0;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_DRI ) {
      fprintf( stderr, "%s( %p )\n", __FUNCTION__, driDrawPriv );
   }

   if ( !driDrawPriv->mesaBuffer->Visual->DBflag )
      return; /* can't swap a single-buffered window */

   /* If the current context's drawable matches the given drawable
    * we have to do a glFinish (per the GLX spec).
    */
   if ( ctx ) {
      __DRIdrawablePrivate *curDrawPriv;
      fxMesa = TDFX_CONTEXT(ctx);
      curDrawPriv = fxMesa->driContext->driDrawablePriv;

      if ( curDrawPriv == driDrawPriv ) {
	 /* swapping window bound to current context, flush first */
	 FLUSH_VB( ctx, "swap buffers" );
	 LOCK_HARDWARE( fxMesa );
      }
      else {
         /* find the fxMesa context previously bound to the window */
	 fxMesa = (tdfxContextPtr) driDrawPriv->driContextPriv->driverPrivate;
         if (!fxMesa)
            return;
	 LOCK_HARDWARE( fxMesa );
	 grSstSelect( fxMesa->Glide.Board );
	 grGlideSetState( (GrState *) fxMesa->Glide.State );
      }
   }

#ifdef STATS
   {
      int stalls;
      static int prevStalls = 0;

      stalls = grFifoGetStalls();

      fprintf( stderr, "%s:\n", __FUNCTION__ );
      if ( stalls != prevStalls ) {
	 fprintf( stderr, "    %d stalls occurred\n",
		  stalls - prevStalls );
	 prevStalls = stalls;
      }
      if ( fxMesa && fxMesa->texSwaps ) {
	 fprintf( stderr, "    %d texture swaps occurred\n",
		  fxMesa->texSwaps );
	 fxMesa->texSwaps = 0;
      }
   }
#endif

   if (fxMesa->scissoredClipRects) {
      /* restore clip rects without scissor box */
      grDRIPosition( driDrawPriv->x, driDrawPriv->y,
                     driDrawPriv->w, driDrawPriv->h,
                     driDrawPriv->numClipRects, driDrawPriv->pClipRects );
   }

   grDRIBufferSwap( fxMesa->Glide.SwapInterval );

   if (fxMesa->scissoredClipRects) {
      /* restore clip rects WITH scissor box */
      grDRIPosition( driDrawPriv->x, driDrawPriv->y,
                     driDrawPriv->w, driDrawPriv->h,
                     fxMesa->numClipRects, fxMesa->pClipRects );
   }


#if 0
   {
      FxI32 result;
      do {
	 result = FX_grGetInteger( FX_PENDING_BUFFERSWAPS );
      } while ( result > fxMesa->maxPendingSwapBuffers );
   }
#endif

   fxMesa->stats.swapBuffer++;

   if (ctx) {
      if (ctx->DriverCtx != fxMesa) {
         fxMesa = TDFX_CONTEXT(ctx);
	 grSstSelect( fxMesa->Glide.Board );
	 grGlideSetState( (GrState *) fxMesa->Glide.State );
      }
      UNLOCK_HARDWARE( fxMesa );
   }
}


GLboolean
XMesaUnbindContext( __DRIcontextPrivate *driContextPriv )
{
   GET_CURRENT_CONTEXT(ctx);

   if ( TDFX_DEBUG & DEBUG_VERBOSE_DRI ) {
      fprintf( stderr, "%s( %p )\n", __FUNCTION__, driContextPriv );
   }

   if ( driContextPriv && driContextPriv->mesaContext == ctx ) {
      tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
      FX_grGlideGetState( fxMesa, (GrState *) fxMesa->Glide.State );
   }
   return GL_TRUE;
}


GLboolean
XMesaMakeCurrent( __DRIcontextPrivate *driContextPriv,
	          __DRIdrawablePrivate *driDrawPriv,
	          __DRIdrawablePrivate *driReadPriv )
{
   if ( TDFX_DEBUG & DEBUG_VERBOSE_DRI ) {
      fprintf( stderr, "%s( %p )\n", __FUNCTION__, driContextPriv );
   }

   if ( driContextPriv ) {
      tdfxContextPtr fxMesa = (tdfxContextPtr) driContextPriv->driverPrivate;
      GLcontext *ctx = fxMesa->glCtx;

      if ( fxMesa->driDrawable != driDrawPriv ) {
	 fxMesa->driDrawable = driDrawPriv;
	 fxMesa->dirty = ~0;
      }

      if ( !fxMesa->Glide.Initialized ) {
	 if ( !tdfxInitContext( driDrawPriv, fxMesa ) )
	    return GL_FALSE;

	 LOCK_HARDWARE( fxMesa );

	 /* FIXME: Force loading of window information */
	 fxMesa->width = 0;
         tdfxUpdateClipping(ctx);
         tdfxUploadClipping(fxMesa);

	 UNLOCK_HARDWARE( fxMesa );
      } else {
	 LOCK_HARDWARE( fxMesa );

	 grSstSelect( fxMesa->Glide.Board );
	 grGlideSetState( fxMesa->Glide.State );

         tdfxUpdateClipping(ctx);
         tdfxUploadClipping(fxMesa);

	 UNLOCK_HARDWARE( fxMesa );
      }

      assert( ctx == driContextPriv->mesaContext );

      gl_make_current2( ctx, driDrawPriv->mesaBuffer,
			driReadPriv->mesaBuffer );

      if ( !ctx->Viewport.Width ) {
	 gl_Viewport( ctx, 0, 0, driDrawPriv->w, driDrawPriv->h );
      }
   } else {
      gl_make_current( 0, 0 );
   }

   return GL_TRUE;
}


GLboolean
XMesaOpenFullScreen(__DRIcontextPrivate *driContextPriv)
{
   if ( 0 )
      fprintf( stderr, "***** XMesaOpenFullScreen *****\n" );
#if 0 /* When new glide3 calls exist */
   return (GLboolean)grDRISetupFullScreen( GL_TRUE );
#else
   return GL_TRUE;
#endif
}


GLboolean
XMesaCloseFullScreen(__DRIcontextPrivate *driContextPriv)
{
   if ( 0 )
      fprintf( stderr, "***** XMesaCloseFullScreen *****\n" );
#if 0 /* When new glide3 calls exist */
   return (GLboolean)grDRISetupFullScreen( GL_FALSE );
#else
   return GL_TRUE;
#endif
}


/* Silence compiler warnings.
 */
extern void __driRegisterExtensions( void );

/* This function is called by libGL.so as soon as libGL.so is loaded.
 * This is where we'd register new extension functions with the dispatcher.
 */
void __driRegisterExtensions( void )
{
#if 0
   /* Example.  Also look in tdfx_dd.c for more details. */
   {
      const int _gloffset_FooBarEXT = 555; /* just an example number! */
      if ( _glapi_add_entrypoint( "glFooBarEXT", _gloffset_FooBarEXT ) ) {
	 void *f = glXGetProcAddressARB( "glFooBarEXT" );
	 assert( f );
      }
   }
#endif
}


#endif
