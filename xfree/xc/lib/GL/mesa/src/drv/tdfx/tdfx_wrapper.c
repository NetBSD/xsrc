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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_wrapper.c,v 1.2 2001/08/18 02:51:07 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Brian Paul <brianp@valinux.com>
 *
 */

#include <stdlib.h>
#include <string.h>

#include "tdfx_context.h"


FxI32
FX_grGetInteger_NoLock(tdfxContextPtr fxMesa, FxU32 pname)
{
   switch (pname) {
   case FX_FOG_TABLE_ENTRIES:
   case FX_GLIDE_STATE_SIZE:
   case FX_LFB_PIXEL_PIPE:
   case FX_PENDING_BUFFERSWAPS:
   case FX_TEXTURE_ALIGN:
   case GR_STATS_PIXELS_DEPTHFUNC_FAIL:
   case GR_STATS_PIXELS_IN:
   case GR_STATS_PIXELS_OUT:
   {
      FxI32 result;
      FxU32 grname = pname;
      fxMesa->Glide.grGet(grname, 4, &result);
      return result;
   }
   case FX_ZDEPTH_MAX:
   {
      FxI32 zvals[2];
      fxMesa->Glide.grGet(GR_ZDEPTH_MIN_MAX, 8, zvals);
      return zvals[0];
   }
   default:
      if (MESA_VERBOSE & VERBOSE_DRIVER) {
	 fprintf(stderr, "Wrong parameter in FX_grGetInteger!\n");
      }
   }

   return 0;
}


FxI32
FX_grGetInteger(tdfxContextPtr fxMesa, FxU32 pname)
{
   int result;
   LOCK_HARDWARE(fxMesa);
   result = FX_grGetInteger_NoLock(fxMesa, pname);
   UNLOCK_HARDWARE(fxMesa);
   return result;
}


const char *
FX_grGetString(tdfxContextPtr fxMesa, FxU32 pname)
{
   const char *s;
   LOCK_HARDWARE(fxMesa);
   s = fxMesa->Glide.grGetString(pname);
   UNLOCK_HARDWARE(fxMesa);
   return s;
}



/* Wrapper for grColorMask() and grColorMaskExt().
 */
void
FX_grColorMask(GLcontext *ctx, GLboolean r, GLboolean g,
               GLboolean b, GLboolean a)
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   LOCK_HARDWARE(fxMesa);
   if (ctx->Visual->RedBits == 8) {
      /* 32bpp mode */
      ASSERT( fxMesa->Glide.grColorMaskExt );
      fxMesa->Glide.grColorMaskExt(r, g, b, a);
   }
   else {
      /* 16 bpp mode */
      /* we never have an alpha buffer */
      fxMesa->Glide.grColorMask(r || g || b, GL_FALSE);
   }
   UNLOCK_HARDWARE(fxMesa);
}


void
FX_grColorMask_NoLock(GLcontext *ctx, GLboolean r, GLboolean g,
                      GLboolean b, GLboolean a)
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   if (ctx->Visual->RedBits == 8) {
      /* 32bpp mode */
      ASSERT( fxMesa->Glide.grColorMaskExt );
      fxMesa->Glide.grColorMaskExt(r, g, b, a);
   }
   else {
      /* 16 bpp mode */
      /* we never have an alpha buffer */
      fxMesa->Glide.grColorMask(r || g || b, GL_FALSE);
   }
}


/* As above, but pass the mask as an array
 */
void
FX_grColorMaskv(GLcontext *ctx, const GLboolean rgba[4])
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   LOCK_HARDWARE(fxMesa);
   if (ctx->Visual->RedBits == 8) {
      /* 32bpp mode */
      ASSERT( fxMesa->Glide.grColorMaskExt );
      fxMesa->Glide.grColorMaskExt(rgba[RCOMP], rgba[GCOMP],
			 rgba[BCOMP], rgba[ACOMP]);
   }
   else {
      /* 16 bpp mode */
      /* we never have an alpha buffer */
      fxMesa->Glide.grColorMask(rgba[RCOMP] || rgba[GCOMP] || rgba[BCOMP], GL_FALSE);
   }
   UNLOCK_HARDWARE(fxMesa);
}

void
FX_grColorMaskv_NoLock(GLcontext *ctx, const GLboolean rgba[4])
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   if (ctx->Visual->RedBits == 8) {
      /* 32bpp mode */
      ASSERT( fxMesa->Glide.grColorMaskExt );
      fxMesa->Glide.grColorMaskExt(rgba[RCOMP], rgba[GCOMP],
                                   rgba[BCOMP], rgba[ACOMP]);
   }
   else {
      /* 16 bpp mode */
      /* we never have an alpha buffer */
      fxMesa->Glide.grColorMask(rgba[RCOMP] || rgba[GCOMP] || rgba[BCOMP], GL_FALSE);
   }
}



FxBool
FX_grLfbLock(tdfxContextPtr fxMesa, GrLock_t type, GrBuffer_t buffer,
             GrLfbWriteMode_t writeMode, GrOriginLocation_t origin,
             FxBool pixelPipeline, GrLfbInfo_t * info)
{
   FxBool result;

   LOCK_HARDWARE(fxMesa);
   result = fxMesa->Glide.grLfbLock(type, buffer, writeMode,
                                    origin, pixelPipeline, info);
   UNLOCK_HARDWARE(fxMesa);
   return result;
}

FxU32
FX_grTexTextureMemRequired(tdfxContextPtr fxMesa, FxU32 evenOdd, GrTexInfo * info)
{
   FxU32 result;

   LOCK_HARDWARE(fxMesa);
   result = fxMesa->Glide.grTexTextureMemRequired(evenOdd, info);
   UNLOCK_HARDWARE(fxMesa);
   return result;
}

FxU32
FX_grTexMinAddress(tdfxContextPtr fxMesa, GrChipID_t tmu)
{
   FxU32 result;

   LOCK_HARDWARE(fxMesa);
   result = fxMesa->Glide.grTexMinAddress(tmu);
   UNLOCK_HARDWARE(fxMesa);
   return result;
}

extern FxU32
FX_grTexMaxAddress(tdfxContextPtr fxMesa, GrChipID_t tmu)
{
   FxU32 result;

   LOCK_HARDWARE(fxMesa);
   result = fxMesa->Glide.grTexMaxAddress(tmu);
   UNLOCK_HARDWARE(fxMesa);
   return result;
}


int
FX_getFogTableSize(tdfxContextPtr fxMesa)
{
   int result;
   LOCK_HARDWARE(fxMesa);
   fxMesa->Glide.grGet(GR_FOG_TABLE_ENTRIES, sizeof(int), (void *) &result);
   UNLOCK_HARDWARE(fxMesa);
   return result;
}

int
FX_getGrStateSize(tdfxContextPtr fxMesa)
{
   int result;
   LOCK_HARDWARE(fxMesa);
   fxMesa->Glide.grGet(GR_GLIDE_STATE_SIZE, sizeof(int), (void *) &result);
   UNLOCK_HARDWARE(fxMesa);
   return result;
}

void
FX_grAADrawLine(tdfxContextPtr fxMesa, GrVertex * a, GrVertex * b)
{
   /* ToDo */
   BEGIN_CLIP_LOOP(fxMesa);
   fxMesa->Glide.grDrawLine(a, b);
   END_CLIP_LOOP(fxMesa);
}

void
FX_grAADrawPoint(tdfxContextPtr fxMesa, GrVertex * a)
{
   BEGIN_CLIP_LOOP(fxMesa);
   fxMesa->Glide.grDrawPoint(a);
   END_CLIP_LOOP(fxMesa);
}

void
FX_grDrawPolygonVertexList(tdfxContextPtr fxMesa, int n, GrVertex * verts)
{
   BEGIN_CLIP_LOOP(fxMesa);
   fxMesa->Glide.grDrawVertexArrayContiguous(GR_POLYGON, n, verts, sizeof(GrVertex));
   END_CLIP_LOOP(fxMesa);
}

#if TDFX_USE_PARGB
void
FX_setupGrVertexLayout(tdfxContextPtr fxMesa)
{
   LOCK_HARDWARE(fxMesa);
   fxMesa->Glide.grReset(GR_VERTEX_PARAMETER);

   fxMesa->Glide.grCoordinateSpace(GR_WINDOW_COORDS);
   fxMesa->Glide.grVertexLayout(GR_PARAM_XY, GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_PARGB, GR_VERTEX_PARGB_OFFSET << 2, GR_PARAM_ENABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_Q, GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_Z, GR_VERTEX_OOZ_OFFSET << 2, GR_PARAM_ENABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_ST0, GR_VERTEX_SOW_TMU0_OFFSET << 2, GR_PARAM_ENABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_Q0, GR_VERTEX_OOW_TMU0_OFFSET << 2, GR_PARAM_DISABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_ST1, GR_VERTEX_SOW_TMU1_OFFSET << 2, GR_PARAM_DISABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_Q1, GR_VERTEX_OOW_TMU1_OFFSET << 2, GR_PARAM_DISABLE);
   UNLOCK_HARDWARE(fxMesa);
}
#else /* TDFX_USE_PARGB */
void
FX_setupGrVertexLayout(tdfxContextPtr fxMesa)
{
   LOCK_HARDWARE(fxMesa);
   fxMesa->Glide.grReset(GR_VERTEX_PARAMETER);

   fxMesa->Glide.grCoordinateSpace(GR_WINDOW_COORDS);
   fxMesa->Glide.grVertexLayout(GR_PARAM_XY, GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_RGB, GR_VERTEX_R_OFFSET << 2, GR_PARAM_ENABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_A, GR_VERTEX_A_OFFSET << 2, GR_PARAM_ENABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_Q, GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_Z, GR_VERTEX_OOZ_OFFSET << 2, GR_PARAM_ENABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_ST0, GR_VERTEX_SOW_TMU0_OFFSET << 2, GR_PARAM_ENABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_Q0, GR_VERTEX_OOW_TMU0_OFFSET << 2, GR_PARAM_DISABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_ST1, GR_VERTEX_SOW_TMU1_OFFSET << 2, GR_PARAM_DISABLE);
   fxMesa->Glide.grVertexLayout(GR_PARAM_Q1, GR_VERTEX_OOW_TMU1_OFFSET << 2, GR_PARAM_DISABLE);
   UNLOCK_HARDWARE(fxMesa);
}
#endif /* TDFX_USE_PARGB */

void
FX_grHints_NoLock(tdfxContextPtr fxMesa, GrHint_t hintType, FxU32 hintMask)
{
   switch (hintType) {
   case GR_HINT_STWHINT:
   {
      if (hintMask & GR_STWHINT_W_DIFF_TMU0)
	 fxMesa->Glide.grVertexLayout(GR_PARAM_Q0,
                                      GR_VERTEX_OOW_TMU0_OFFSET << 2,
                                      GR_PARAM_ENABLE);
      else
	 fxMesa->Glide.grVertexLayout(GR_PARAM_Q0,
                                      GR_VERTEX_OOW_TMU0_OFFSET << 2,
                                      GR_PARAM_DISABLE);

      if (hintMask & GR_STWHINT_ST_DIFF_TMU1)
	 fxMesa->Glide.grVertexLayout(GR_PARAM_ST1,
                                      GR_VERTEX_SOW_TMU1_OFFSET << 2,
                                      GR_PARAM_ENABLE);
      else
	 fxMesa->Glide.grVertexLayout(GR_PARAM_ST1,
                                      GR_VERTEX_SOW_TMU1_OFFSET << 2,
                                      GR_PARAM_DISABLE);

      if (hintMask & GR_STWHINT_W_DIFF_TMU1)
	 fxMesa->Glide.grVertexLayout(GR_PARAM_Q1,
                                      GR_VERTEX_OOW_TMU1_OFFSET << 2,
                                      GR_PARAM_ENABLE);
      else
	 fxMesa->Glide.grVertexLayout(GR_PARAM_Q1,
                                      GR_VERTEX_OOW_TMU1_OFFSET << 2,
                                      GR_PARAM_DISABLE);

   }
   }
}

void
FX_grHints(tdfxContextPtr fxMesa, GrHint_t hintType, FxU32 hintMask)
{
   LOCK_HARDWARE(fxMesa);
   FX_grHints_NoLock(fxMesa, hintType, hintMask);
   UNLOCK_HARDWARE(fxMesa);
}

/* It appears to me that this function is needed either way. */
FX_GrContext_t
FX_grSstWinOpen(tdfxContextPtr fxMesa,
                FxU32 hWnd,
                GrScreenResolution_t screen_resolution,
                GrScreenRefresh_t refresh_rate,
                GrColorFormat_t color_format,
                GrOriginLocation_t origin_location,
                int nColBuffers, int nAuxBuffers)
{
   FX_GrContext_t i;
   LOCK_HARDWARE(fxMesa);
   i = fxMesa->Glide.grSstWinOpen(hWnd,
                                  screen_resolution,
                                  refresh_rate,
                                  color_format, origin_location,
                                  nColBuffers, nAuxBuffers);

   /*
     fprintf(stderr,
     "grSstWinOpen( win %d res %d ref %d fmt %d\n"
     "              org %d ncol %d naux %d )\n"
     " ==> %d\n",
     hWnd,
     screen_resolution,
     refresh_rate,
     color_format,
     origin_location,
     nColBuffers,
     nAuxBuffers,
     i);
   */
   UNLOCK_HARDWARE(fxMesa);
   return i;
}
