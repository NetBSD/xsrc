/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mga_xmesa.c,v 1.12 2001/04/10 16:07:50 dawes Exp $ */
/*
 * Copyright 2000-2001 VA Linux Systems, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Whitwell <keithw@valinux.com>
 */

#ifdef GLX_DIRECT_RENDERING

#include <X11/Xlibint.h>
#include <stdio.h>

#include "drm.h"
#include "mga_xmesa.h"
#include "context.h"
#include "vbxform.h"
#include "matrix.h"
#include "mmath.h"
#include "simple_list.h"
#include "mem.h"

#include "mgadd.h"
#include "mgastate.h"
#include "mgatex.h"
#include "mgaspan.h"
#include "mgatris.h"
#include "mgapipeline.h"
#include "mgabuffers.h"
#include "mgapixel.h"

#include "xf86dri.h"
#include "mga_xmesa.h"

#include "mga_dri.h"


#ifndef MGA_DEBUG
int MGA_DEBUG = (0
/*		 | DEBUG_ALWAYS_SYNC */
/*		 | DEBUG_VERBOSE_MSG */
/*		 | DEBUG_VERBOSE_LRU */
/*		 | DEBUG_VERBOSE_DRI */
/*		 | DEBUG_VERBOSE_IOCTL */
/*		 | DEBUG_VERBOSE_2D */
/*		 | DEBUG_VERBOSE_FALLBACK */
   );
#endif


static mgaContextPtr      mgaCtx = 0;


/* These functions are accessed externally to the driver:
 *
 * XMesaInitDriver
 * XMesaResetDriver
 * XMesaCreateVisual
 * XMesaDestroyVisual
 * XMesaCreateContext
 * XMesaDestroyContext
 * XMesaCreateWindowBuffer
 * XMesaCreatePixmapBuffer
 * XMesaDestroyBuffer
 * XMesaSwapBuffers
 * XMesaMakeCurrent
 *
 */

GLboolean XMesaInitDriver(__DRIscreenPrivate *sPriv)
{
   mgaScreenPrivate *mgaScreen;
   MGADRIPtr         serverInfo = (MGADRIPtr)sPriv->pDevPriv;

   if (MGA_DEBUG&DEBUG_VERBOSE_DRI)
      fprintf(stderr, "XMesaInitDriver\n");

   /* Check the DRI version */
   {
      int major, minor, patch;
      if (XF86DRIQueryVersion(sPriv->display, &major, &minor, &patch)) {
         if (major != 4 || minor < 0) {
            char msg[1000];
            sprintf(msg, "MGA DRI driver expected DRI version 4.0.x but got version %d.%d.%d", major, minor, patch);
            __driMesaMessage(msg);
            return GL_FALSE;
         }
      }
   }

   /* Check that the DDX driver version is compatible */
   if (sPriv->ddxMajor != 1 ||
       sPriv->ddxMinor < 0) {
      char msg[1000];
      sprintf(msg, "MGA DRI driver expected DDX driver version 1.0.x but got version %d.%d.%d", sPriv->ddxMajor, sPriv->ddxMinor, sPriv->ddxPatch);
      __driMesaMessage(msg);
      return GL_FALSE;
   }

   /* Check that the DRM driver version is compatible */
   if (sPriv->drmMajor != 3 ||
       sPriv->drmMinor < 0) {
      char msg[1000];
      sprintf(msg, "MGA DRI driver expected DRM driver version 3.0.x but got version %d.%d.%d", sPriv->drmMajor, sPriv->drmMinor, sPriv->drmPatch);
      __driMesaMessage(msg);
      return GL_FALSE;
   }


   /* Allocate the private area */
   mgaScreen = (mgaScreenPrivate *)MALLOC(sizeof(mgaScreenPrivate));
   if (!mgaScreen) {
      __driMesaMessage("Couldn't malloc screen struct");
      return GL_FALSE;
   }

   mgaScreen->sPriv = sPriv;
   sPriv->private = (void *)mgaScreen;

   if (serverInfo->chipset != MGA_CARD_TYPE_G200 &&
       serverInfo->chipset != MGA_CARD_TYPE_G400) {
      XFree(mgaScreen);
      sPriv->private = NULL;
      __driMesaMessage("Unrecognized chipset");
      return GL_FALSE;
   }


   mgaScreen->chipset = serverInfo->chipset;
   mgaScreen->width = serverInfo->width;
   mgaScreen->height = serverInfo->height;
   mgaScreen->mem = serverInfo->mem;
   mgaScreen->cpp = serverInfo->cpp;

   mgaScreen->agpMode = serverInfo->agpMode;

   mgaScreen->frontPitch = serverInfo->frontPitch;
   mgaScreen->frontOffset = serverInfo->frontOffset;
   mgaScreen->backOffset = serverInfo->backOffset;
   mgaScreen->backPitch  =  serverInfo->backPitch;
   mgaScreen->depthOffset = serverInfo->depthOffset;
   mgaScreen->depthPitch  =  serverInfo->depthPitch;

   mgaScreen->mmio.handle = serverInfo->registers.handle;
   mgaScreen->mmio.size = serverInfo->registers.size;
   if ( drmMap( sPriv->fd,
		mgaScreen->mmio.handle, mgaScreen->mmio.size,
		&mgaScreen->mmio.map ) < 0 ) {
      FREE( mgaScreen );
      sPriv->private = NULL;
      __driMesaMessage( "Couldn't map MMIO registers" );
      return GL_FALSE;
   }

#if 0
   mgaScreen->status.handle = serverInfo->status.handle;
   mgaScreen->status.size = serverInfo->status.size;
   if ( drmMap( sPriv->fd,
		mgaScreen->status.handle, mgaScreen->status.size,
		&mgaScreen->status.map ) < 0 ) {
      drmUnmap( mgaScreen->mmio.map, mgaScreen->mmio.size );
      FREE( mgaScreen );
      sPriv->private = NULL;
      __driMesaMessage( "Couldn't map status page" );
      return GL_FALSE;
   }
#endif

   mgaScreen->primary.handle = serverInfo->primary.handle;
   mgaScreen->primary.size = serverInfo->primary.size;
   mgaScreen->buffers.handle = serverInfo->buffers.handle;
   mgaScreen->buffers.size = serverInfo->buffers.size;

#if 0
   mgaScreen->agp.handle = serverInfo->agp;
   mgaScreen->agp.size = serverInfo->agpSize;

   if (drmMap(sPriv->fd,
	      mgaScreen->agp.handle,
	      mgaScreen->agp.size,
	      (drmAddress *)&mgaScreen->agp.map) != 0)
   {
      Xfree(mgaScreen);
      sPriv->private = NULL;
      __driMesaMessage("Couldn't map agp region");
      return GL_FALSE;
   }
#endif

   mgaScreen->textureOffset[MGA_CARD_HEAP] = serverInfo->textureOffset;
   mgaScreen->textureOffset[MGA_AGP_HEAP] = (serverInfo->agpTextureOffset |
					     PDEA_pagpxfer_enable | 1);

   mgaScreen->textureSize[MGA_CARD_HEAP] = serverInfo->textureSize;
   mgaScreen->textureSize[MGA_AGP_HEAP] = serverInfo->agpTextureSize;

   mgaScreen->logTextureGranularity[MGA_CARD_HEAP] =
      serverInfo->logTextureGranularity;
   mgaScreen->logTextureGranularity[MGA_AGP_HEAP] =
      serverInfo->logAgpTextureGranularity;

   mgaScreen->texVirtual[MGA_CARD_HEAP] = (char *)(mgaScreen->sPriv->pFB +
					   serverInfo->textureOffset);
#if 0
   mgaScreen->texVirtual[MGA_AGP_HEAP] = (mgaScreen->agp.map +
					  serverInfo->agpTextureOffset);
#endif

   mgaScreen->mAccess = serverInfo->mAccess;

   /* For calculating setupdma addresses.
    */
   mgaScreen->dmaOffset = serverInfo->buffers.handle;

   mgaScreen->bufs = drmMapBufs(sPriv->fd);
   if (!mgaScreen->bufs) {
      /*drmUnmap(mgaScreen->agp_tex.map, mgaScreen->agp_tex.size);*/
      XFree(mgaScreen);
      sPriv->private = NULL;
      __driMesaMessage("Couldn't map dma buffers");
      return GL_FALSE;
   }
   mgaScreen->sarea_priv_offset = serverInfo->sarea_priv_offset;

   mgaDDFastPathInit();
   mgaDDEltPathInit();
   mgaDDTrifuncInit();
   mgaDDSetupInit();

   return GL_TRUE;
}


/* Accessed by dlsym from dri_mesa_init.c
 */
void XMesaResetDriver(__DRIscreenPrivate *sPriv)
{
   mgaScreenPrivate *mgaScreen = (mgaScreenPrivate *) sPriv->private;

   if (MGA_DEBUG&DEBUG_VERBOSE_DRI)
      fprintf(stderr, "XMesaResetDriver\n");

   /*drmUnmap(mgaScreen->agp_tex.map, mgaScreen->agp_tex.size);*/
   Xfree(mgaScreen);
   sPriv->private = NULL;
}


GLvisual *XMesaCreateVisual(Display *dpy,
                            __DRIscreenPrivate *driScrnPriv,
                            const XVisualInfo *visinfo,
                            const __GLXvisualConfig *config)
{
   if (MGA_DEBUG&DEBUG_VERBOSE_DRI)
      fprintf(stderr, "XMesaCreateVisual\n");

   /* Drivers may change the args to _mesa_create_visual() in order to
    * setup special visuals.
    */
   return _mesa_create_visual( config->rgba,
                               config->doubleBuffer,
                               config->stereo,
                               _mesa_bitcount(visinfo->red_mask),
                               _mesa_bitcount(visinfo->green_mask),
                               _mesa_bitcount(visinfo->blue_mask),
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


GLboolean XMesaCreateContext( Display *dpy, GLvisual *mesaVis,
                              __DRIcontextPrivate *driContextPriv )
{
   int i;
   GLcontext *ctx;
   mgaContextPtr mmesa;
   __DRIscreenPrivate *sPriv = driContextPriv->driScreenPriv;
   mgaScreenPrivate *mgaScreen = (mgaScreenPrivate *)sPriv->private;
   MGASAREAPrivPtr saPriv = (MGASAREAPrivPtr)(((char*)sPriv->pSAREA)+
					      mgaScreen->sarea_priv_offset);

   if (MGA_DEBUG&DEBUG_VERBOSE_DRI)
      fprintf(stderr, "XMesaCreateContext\n");

   mmesa = (mgaContextPtr)CALLOC(sizeof(mgaContext));
   if (!mmesa) {
      return GL_FALSE;
   }

   ctx = driContextPriv->mesaContext;

   mmesa->display = dpy;
   mmesa->hHWContext = driContextPriv->hHWContext;
   mmesa->driFd = sPriv->fd;
   mmesa->driHwLock = &sPriv->pSAREA->lock;

   mmesa->mgaScreen = mgaScreen;
   mmesa->driScreen = sPriv;
   mmesa->sarea = saPriv;
   mmesa->glBuffer = NULL;

   make_empty_list(&mmesa->SwappedOut);

   mmesa->lastTexHeap = mgaScreen->texVirtual[MGA_AGP_HEAP] ? 2 : 1;

   for (i = 0 ; i < mmesa->lastTexHeap ; i++) {
      mmesa->texHeap[i] = mmInit( 0, mgaScreen->textureSize[i]);
      make_empty_list(&mmesa->TexObjList[i]);
   }

   /* Set the maximum texture size small enough that we can guarentee
    * that both texture units can bind a maximal texture and have them
    * on the card at once.
    */
   {
      int nr = 2;

      if (mgaScreen->chipset == MGA_CARD_TYPE_G200)
	 nr = 1;

      if (mgaScreen->textureSize[0] < nr*1024*1024) {
	 ctx->Const.MaxTextureLevels = 9;
	 ctx->Const.MaxTextureSize = 1<<8;
      } else if (mgaScreen->textureSize[0] < nr*4*1024*1024) {
	 ctx->Const.MaxTextureLevels = 10;
	 ctx->Const.MaxTextureSize = 1<<9;
      } else {
	 ctx->Const.MaxTextureLevels = 11;
	 ctx->Const.MaxTextureSize = 1<<10;
      }
   }

   mmesa->hw_stencil = mesaVis->StencilBits && mesaVis->DepthBits == 24;

   switch (mesaVis->DepthBits) {
   case 16:
      mmesa->depth_scale = 1.0/(GLdouble)0xffff;
      mmesa->depth_clear_mask = ~0;
      mmesa->ClearDepth = 0xffff;
      break;
   case 24:
      mmesa->depth_scale = 1.0/(GLdouble)0xffffff;
      if (mmesa->hw_stencil) {
	 mmesa->depth_clear_mask = 0xffffff00;
	 mmesa->stencil_clear_mask = 0x000000ff;
      } else
	 mmesa->depth_clear_mask = ~0;
      mmesa->ClearDepth = 0xffffff00;
      break;
   case 32:
      mmesa->depth_scale = 1.0/(GLdouble)0xffffffff;
      mmesa->depth_clear_mask = ~0;
      mmesa->ClearDepth = 0xffffffff;
      break;
   };

   mmesa->canDoStipple = GL_FALSE;
   mmesa->renderindex = -1;		/* impossible value */
   mmesa->new_state = ~0;
   mmesa->dirty = ~0;
   mmesa->warp_pipe = 0;
   mmesa->CurrentTexObj[0] = 0;
   mmesa->CurrentTexObj[1] = 0;

   mmesa->texAge[0] = 0;
   mmesa->texAge[1] = 0;

#if 0
   mmesa->status = (GLuint *)mmesa->mgaScreen->status.map;
#endif
   mmesa->primary_offset = mmesa->mgaScreen->primary.handle;

   ctx->DriverCtx = (void *) mmesa;
   mmesa->glCtx = ctx;

   mgaDDExtensionsInit( ctx );

   mgaDDInitStateFuncs( ctx );
   mgaDDInitTextureFuncs( ctx );
   mgaDDInitSpanFuncs( ctx );
   mgaDDInitDriverFuncs( ctx );
   mgaDDInitIoctlFuncs( ctx );
/*   mgaDDInitPixelFuncs( ctx );*/

   ctx->Driver.TriangleCaps = (DD_TRI_CULL|
			       DD_TRI_LIGHT_TWOSIDE|
			       DD_TRI_STIPPLE|
			       DD_TRI_OFFSET);

   /* Ask mesa to clip fog coordinates for us.
    */
   ctx->TriangleCaps |= DD_CLIP_FOG_COORD;

   if (ctx->VB)
      mgaDDRegisterVB( ctx->VB );

   if (ctx->NrPipelineStages)
      ctx->NrPipelineStages =
	 mgaDDRegisterPipelineStages(ctx->PipelineStage,
				      ctx->PipelineStage,
				      ctx->NrPipelineStages);

   mgaInitState( mmesa );

   driContextPriv->driverPrivate = (void *) mmesa;

   return GL_TRUE;
}

void XMesaDestroyContext(__DRIcontextPrivate *driContextPriv)
{
   mgaContextPtr mmesa = (mgaContextPtr) driContextPriv->driverPrivate;

   if (MGA_DEBUG&DEBUG_VERBOSE_DRI)
      fprintf(stderr, "XMesaDestroyContext\n");

   if (mmesa) {
      Xfree(mmesa);
      driContextPriv->driverPrivate = NULL;
   }
}


GLframebuffer *XMesaCreateWindowBuffer( Display *dpy,
                                        __DRIscreenPrivate *driScrnPriv,
                                        __DRIdrawablePrivate *driDrawPriv,
                                        GLvisual *mesaVis)
{
   GLboolean swStencil = mesaVis->StencilBits > 0 && mesaVis->DepthBits != 24;

   if (MGA_DEBUG&DEBUG_VERBOSE_DRI)
      fprintf(stderr, "XMesaCreateWindowBuffer\n");

   return gl_create_framebuffer(mesaVis,
                                GL_FALSE,  /* software depth buffer? */
                                swStencil,
                                mesaVis->AccumRedBits > 0,
                                GL_FALSE   /* software alpha buffer/ */
                                );
}


GLframebuffer *XMesaCreatePixmapBuffer( Display *dpy,
                                        __DRIscreenPrivate *driScrnPriv,
                                        __DRIdrawablePrivate *driDrawPriv,
                                        GLvisual *mesaVis)
{
#if 0
   /* Different drivers may have different combinations of hardware and
    * software ancillary buffers.
    */
   return gl_create_framebuffer(mesaVis,
                                GL_FALSE,  /* software depth buffer? */
                                mesaVis->StencilBits > 0,
                                mesaVis->AccumRedBits > 0,
                                mesaVis->AlphaBits > 0
                                );
#else
   return NULL;  /* not implemented yet */
#endif
}


void XMesaSwapBuffers(__DRIdrawablePrivate *driDrawPriv)
{
   /* XXX should do swap according to the buffer, not the context! */
   mgaContextPtr mmesa = mgaCtx;
   FLUSH_VB( mmesa->glCtx, "swap buffers" );
   mgaSwapBuffers(mmesa);
}


GLboolean XMesaUnbindContext(__DRIcontextPrivate *driContextPriv)
{
   mgaContextPtr mmesa = (mgaContextPtr) driContextPriv->driverPrivate;
   if (mmesa)
      mmesa->dirty = ~0;

   return GL_TRUE;
}

GLboolean
XMesaOpenFullScreen(__DRIcontextPrivate *driContextPriv)
{
    return GL_TRUE;
}

GLboolean
XMesaCloseFullScreen(__DRIcontextPrivate *driContextPriv)
{
    return GL_TRUE;
}


/* This looks buggy to me - the 'b' variable isn't used anywhere...
 * Hmm - It seems that the drawable is already hooked in to
 * driDrawablePriv.
 *
 * But why are we doing context initialization here???
 */
GLboolean XMesaMakeCurrent(__DRIcontextPrivate *driContextPriv,
                           __DRIdrawablePrivate *driDrawPriv,
                           __DRIdrawablePrivate *driReadPriv)
{
   if (driContextPriv) {
      mgaCtx = (mgaContextPtr) driContextPriv->driverPrivate;

      gl_make_current2(mgaCtx->glCtx, driDrawPriv->mesaBuffer, driReadPriv->mesaBuffer);

      if (mgaCtx->driDrawable != driDrawPriv) {
	 mgaCtx->driDrawable = driDrawPriv;
	 mgaCtx->dirty = ~0;
	 mgaCtx->dirty_cliprects = (MGA_FRONT|MGA_BACK);
      }

      if (!mgaCtx->glCtx->Viewport.Width)
	 gl_Viewport(mgaCtx->glCtx, 0, 0, driDrawPriv->w, driDrawPriv->h);

   }
   else {
      gl_make_current(0,0);
      mgaCtx = NULL;
   }

   return GL_TRUE;
}


void mgaGetLock( mgaContextPtr mmesa, GLuint flags )
{
   __DRIdrawablePrivate *dPriv = mmesa->driDrawable;
   MGASAREAPrivPtr sarea = mmesa->sarea;
   int me = mmesa->hHWContext;
   int i;

   drmGetLock(mmesa->driFd, mmesa->hHWContext, flags);

   if (*(dPriv->pStamp) != mmesa->lastStamp) {
      mmesa->lastStamp = *(dPriv->pStamp);
      mmesa->setupdone = 0;
      mmesa->dirty_cliprects = (MGA_FRONT|MGA_BACK);
      mgaUpdateRects( mmesa, (MGA_FRONT|MGA_BACK) );
   }

   mmesa->dirty |= MGA_UPLOAD_CONTEXT | MGA_UPLOAD_CLIPRECTS;

    mmesa->sarea->dirty |= MGA_UPLOAD_CONTEXT;

   if (sarea->ctxOwner != me) {
      mmesa->dirty |= (MGA_UPLOAD_CONTEXT | MGA_UPLOAD_TEX0 |
		       MGA_UPLOAD_TEX1 | MGA_UPLOAD_PIPE);
      sarea->ctxOwner=me;
   }

   for (i = 0 ; i < mmesa->lastTexHeap ; i++)
      if (sarea->texAge[i] != mmesa->texAge[i])
	 mgaAgeTextures( mmesa, i );

   sarea->last_quiescent = -1;	/* just kill it for now */
}




#endif
