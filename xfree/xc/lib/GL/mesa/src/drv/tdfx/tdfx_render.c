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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_render.c,v 1.1 2001/03/21 16:14:28 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Brian Paul <brianp@valinux.com>
 *
 */

#include "dri_glide.h"
#include "tdfx_context.h"
#include "tdfx_render.h"
#include "tdfx_state.h"
#include "tdfx_texman.h"

/* Clear the color and/or depth buffers.
 */
static GLbitfield tdfxDDClear( GLcontext *ctx,
			       GLbitfield mask, GLboolean all,
			       GLint x, GLint y, GLint width, GLint height )
{
   tdfxContextPtr fxMesa = (tdfxContextPtr) ctx->DriverCtx;
   GLbitfield softwareMask = mask & (DD_ACCUM_BIT);
   const GLuint stencil_size =
      fxMesa->haveHwStencil ? fxMesa->glVis->StencilBits : 0;

#if 0
   GLuint flags;

   FLUSH_BATCH( fxMesa );

   if ( mask & DD_FRONT_LEFT_BIT ) {
      flags |= TDFX_FRONT;
      mask &= ~DD_FRONT_LEFT_BIT;
   }

   if ( mask & DD_BACK_LEFT_BIT ) {
      flags |= TDFX_BACK;
      mask &= ~DD_BACK_LEFT_BIT;
   }

   if ( (mask & DD_DEPTH_BIT) && ctx->Depth.Mask ) {
      flags |= TDFX_DEPTH;
      mask &= ~DD_DEPTH_BIT;
   }

   if ( (mask & DD_STENCIL_BIT) && fxMesa->haveHwStencil ) {
      clear.flags |= TDFX_STENCIL;
      mask &= ~DD_STENCIL_BIT;
   }

   if ( !flags )
      return mask;

   LOCK_HARDWARE( fxMesa );


   if ( flags & TDFX_FRONT ) {

   }

   if ( flags & TDFX_BACK ) {

   }

   if ( flags & TDFX_DEPTH ) {

   }

   if ( flags & TDFX_STENCIL ) {
      grStencilMask( fxMesa->Stencil.WriteMask );
      /* set stencil ref value = desired clear value */
      grStencilFunc( GR_CMP_ALWAYS, fxMesa->Stencil.Clear, 0xff );
      grStencilOp( GR_STENCILOP_REPLACE,
		   GR_STENCILOP_REPLACE,
		   GR_STENCILOP_REPLACE );
      grEnable( GR_STENCIL_MODE_EXT );

      if ( ctx->Stencil.Enabled ) {
	 grStencilOp( fxMesa->Stencil.FailFunc,
		      fxMesa->Stencil.ZFailFunc,
		      fxMesa->Stencil.ZPassFunc );
	 grStencilMask( fxMesa->Stencil.WriteMask );
	 grStencilFunc( fxMesa->Stencil.Function,
			fxMesa->Stencil.RefValue,
			fxMesa->Stencil.ValueMask );
	 grEnable_NoLock( GR_STENCIL_MODE_EXT );
      } else {
	 grDisable( GR_STENCIL_MODE_EXT );
      }

   }
   UNLOCK_HARDWARE( fxMesa );

#else

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %d, %d, %d, %d )\n",
	       __FUNCTION__, (int) x, (int) y, (int) width, (int) height );
   }

   /* Need this check to respond to glScissor and clipping updates */
   if ((fxMesa->new_state & (TDFX_NEW_CLIP | TDFX_NEW_DEPTH)) ||
       (fxMesa->dirty & TDFX_UPLOAD_COLOR_MASK)) {
      tdfxDDUpdateHwState(ctx);
   }

   /* we can't clear accum buffers */
   mask &= ~(DD_ACCUM_BIT);

   if (mask & DD_STENCIL_BIT) {
      if (!fxMesa->haveHwStencil || ctx->Stencil.WriteMask != 0xff) {
         /* Napalm seems to have trouble with stencil write masks != 0xff */
         /* do stencil clear in software */
         mask &= ~(DD_STENCIL_BIT);
         softwareMask |= DD_STENCIL_BIT;
      }
   }

   if (fxMesa->glVis->RedBits != 8) {
      /* can only do color masking if running in 24/32bpp on Napalm */
      if (ctx->Color.ColorMask[RCOMP] != ctx->Color.ColorMask[GCOMP] ||
          ctx->Color.ColorMask[GCOMP] != ctx->Color.ColorMask[BCOMP]) {
         softwareMask |= (mask & (DD_FRONT_LEFT_BIT | DD_BACK_LEFT_BIT));
         mask &= ~(DD_FRONT_LEFT_BIT | DD_BACK_LEFT_BIT);
      }
   }

   if (fxMesa->haveHwStencil) {
      /*
       * If we want to clear stencil, it must be enabled
       * in the HW, even if the stencil test is not enabled
       * in the OGL state.
       */
      LOCK_HARDWARE(fxMesa);
      if (mask & DD_STENCIL_BIT) {
	 FX_grStencilMask_NoLock(/*ctx->Stencil.WriteMask*/ 0xff);
	 /* set stencil ref value = desired clear value */
	 FX_grStencilFunc_NoLock(GR_CMP_ALWAYS, ctx->Stencil.Clear, 0xff);
	 FX_grStencilOp_NoLock(GR_STENCILOP_REPLACE,
			       GR_STENCILOP_REPLACE, GR_STENCILOP_REPLACE);
	 FX_grEnable_NoLock(GR_STENCIL_MODE_EXT);
      }
      else {
	 FX_grDisable_NoLock(GR_STENCIL_MODE_EXT);
      }
      UNLOCK_HARDWARE(fxMesa);
   }

   /*
    * FIXME: This is just plain ugly...
    */
   BEGIN_CLIP_LOOP(fxMesa);
   {
      /*
       * This could probably be done fancier but doing each possible case
       * explicitly is less error prone.
       */
      switch (mask & ~DD_STENCIL_BIT) {
      case DD_BACK_LEFT_BIT | DD_DEPTH_BIT:
	 /* back buffer & depth */
	 FX_grColorMaskv_NoLock(ctx, true4); /* work around Voodoo3 bug */
	 FX_grDepthMask_NoLock(FXTRUE);
	 FX_grRenderBuffer_NoLock(GR_BUFFER_BACKBUFFER);
	 if (stencil_size > 0) {
            FX_grBufferClearExt_NoLock(fxMesa->Color.ClearColor,
				       fxMesa->Color.ClearAlpha,
				       fxMesa->Depth.Clear,
                                       (FxU32) ctx->Stencil.Clear);
         }
	 else
            FX_grBufferClear_NoLock(fxMesa->Color.ClearColor,
				    fxMesa->Color.ClearAlpha,
                                    fxMesa->Depth.Clear);
	 if (!ctx->Depth.Mask || !ctx->Depth.Test) {
            FX_grDepthMask_NoLock(FXFALSE);
	 }
	 break;
      case DD_FRONT_LEFT_BIT | DD_DEPTH_BIT:
	 /* XXX it appears that the depth buffer isn't cleared when
	  * glRenderBuffer(GR_BUFFER_FRONTBUFFER) is set.
	  * This is a work-around/
	  */
	 /* clear depth */
	 FX_grDepthMask_NoLock(FXTRUE);
	 FX_grRenderBuffer_NoLock(GR_BUFFER_BACKBUFFER);
	 FX_grColorMaskv_NoLock(ctx, false4);
	 if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->Color.ClearColor,
                                       fxMesa->Color.ClearAlpha,
				       fxMesa->Depth.Clear,
                                       (FxU32) ctx->Stencil.Clear);
	 else
            FX_grBufferClear_NoLock(fxMesa->Color.ClearColor,
                                    fxMesa->Color.ClearAlpha,
                                    fxMesa->Depth.Clear);
	 /* clear front */
	 FX_grColorMaskv_NoLock(ctx, true4);
	 FX_grRenderBuffer_NoLock(GR_BUFFER_FRONTBUFFER);
	 if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->Color.ClearColor,
                                       fxMesa->Color.ClearAlpha,
				       fxMesa->Depth.Clear,
                                       (FxU32) ctx->Stencil.Clear);
	 else
            FX_grBufferClear_NoLock(fxMesa->Color.ClearColor,
                                    fxMesa->Color.ClearAlpha,
                                    fxMesa->Depth.Clear);
	 if (!ctx->Depth.Mask || !ctx->Depth.Test) {
            FX_grDepthMask_NoLock(FXFALSE);
	 }
	 break;
      case DD_BACK_LEFT_BIT:
	 /* back buffer only */
	 FX_grDepthMask_NoLock(FXFALSE);
	 FX_grRenderBuffer_NoLock(GR_BUFFER_BACKBUFFER);
	 if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->Color.ClearColor,
                                       fxMesa->Color.ClearAlpha,
				       fxMesa->Depth.Clear,
                                       (FxU32) ctx->Stencil.Clear);
	 else
            FX_grBufferClear_NoLock(fxMesa->Color.ClearColor,
                                    fxMesa->Color.ClearAlpha,
                                    fxMesa->Depth.Clear);
	 if (ctx->Depth.Mask && ctx->Depth.Test) {
            FX_grDepthMask_NoLock(FXTRUE);
	 }
	 break;
      case DD_FRONT_LEFT_BIT:
	 /* front buffer only */
	 FX_grDepthMask_NoLock(FXFALSE);
	 FX_grRenderBuffer_NoLock(GR_BUFFER_FRONTBUFFER);
	 if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->Color.ClearColor,
                                       fxMesa->Color.ClearAlpha,
				       fxMesa->Depth.Clear,
                                       (FxU32) ctx->Stencil.Clear);
	 else
            FX_grBufferClear_NoLock(fxMesa->Color.ClearColor,
                                    fxMesa->Color.ClearAlpha,
                                    fxMesa->Depth.Clear);
	 if (ctx->Depth.Mask && ctx->Depth.Test) {
            FX_grDepthMask_NoLock(FXTRUE);
	 }
	 break;
      case DD_FRONT_LEFT_BIT | DD_BACK_LEFT_BIT:
	 /* front and back */
	 FX_grDepthMask_NoLock(FXFALSE);
	 FX_grRenderBuffer_NoLock(GR_BUFFER_BACKBUFFER);
	 if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->Color.ClearColor,
                                       fxMesa->Color.ClearAlpha,
				       fxMesa->Depth.Clear,
                                       (FxU32) ctx->Stencil.Clear);
	 else
            FX_grBufferClear_NoLock(fxMesa->Color.ClearColor,
                                    fxMesa->Color.ClearAlpha,
                                    fxMesa->Depth.Clear);
	 FX_grRenderBuffer_NoLock(GR_BUFFER_FRONTBUFFER);
	 if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->Color.ClearColor,
                                       fxMesa->Color.ClearAlpha,
				       fxMesa->Depth.Clear,
                                       (FxU32) ctx->Stencil.Clear);
	 else
            FX_grBufferClear_NoLock(fxMesa->Color.ClearColor,
                                    fxMesa->Color.ClearAlpha,
                                    fxMesa->Depth.Clear);
	 if (ctx->Depth.Mask && ctx->Depth.Test) {
            FX_grDepthMask_NoLock(FXTRUE);
	 }
	 break;
      case DD_FRONT_LEFT_BIT | DD_BACK_LEFT_BIT | DD_DEPTH_BIT:
	 /* clear front */
	 FX_grDepthMask_NoLock(FXFALSE);
	 FX_grRenderBuffer_NoLock(GR_BUFFER_FRONTBUFFER);
	 if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->Color.ClearColor,
                                       fxMesa->Color.ClearAlpha,
				       fxMesa->Depth.Clear,
                                       (FxU32) ctx->Stencil.Clear);
	 else
            FX_grBufferClear_NoLock(fxMesa->Color.ClearColor,
                                    fxMesa->Color.ClearAlpha,
                                    fxMesa->Depth.Clear);
	 /* clear back and depth */
	 FX_grDepthMask_NoLock(FXTRUE);
	 FX_grRenderBuffer_NoLock(GR_BUFFER_BACKBUFFER);
	 if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->Color.ClearColor,
                                       fxMesa->Color.ClearAlpha,
				       fxMesa->Depth.Clear,
                                       (FxU32) ctx->Stencil.Clear);
	 else
            FX_grBufferClear_NoLock(fxMesa->Color.ClearColor,
                                    fxMesa->Color.ClearAlpha,
                                    fxMesa->Depth.Clear);
	 if (!ctx->Depth.Mask || !ctx->Depth.Mask) {
            FX_grDepthMask_NoLock(FXFALSE);
	 }
	 break;
      case DD_DEPTH_BIT:
	 /* just the depth buffer */
	 FX_grRenderBuffer_NoLock(GR_BUFFER_BACKBUFFER);
	 FX_grColorMaskv_NoLock(ctx, false4);
	 FX_grDepthMask_NoLock(FXTRUE);
	 if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->Color.ClearColor,
                                       fxMesa->Color.ClearAlpha,
				       fxMesa->Depth.Clear,
                                       (FxU32) ctx->Stencil.Clear);
	 else
            FX_grBufferClear_NoLock(fxMesa->Color.ClearColor,
                                    fxMesa->Color.ClearAlpha,
                                    fxMesa->Depth.Clear);
	 FX_grColorMaskv_NoLock(ctx, true4);
	 if (ctx->Color.DrawDestMask & FRONT_LEFT_BIT)
            FX_grRenderBuffer_NoLock(GR_BUFFER_FRONTBUFFER);
	 if (!ctx->Depth.Test || !ctx->Depth.Mask)
	    FX_grDepthMask_NoLock(FXFALSE);
	 break;
      default:
         /* clear no color buffers or depth buffer but might clear stencil */
	 if (stencil_size > 0 && (mask & DD_STENCIL_BIT)) {
            /* XXX need this RenderBuffer call to work around Glide bug */
            FX_grRenderBuffer_NoLock(GR_BUFFER_BACKBUFFER);
            FX_grDepthMask_NoLock(FXFALSE);
            FX_grColorMaskv_NoLock(ctx, false4);
            FX_grBufferClearExt_NoLock(fxMesa->Color.ClearColor,
                                       fxMesa->Color.ClearAlpha,
				       fxMesa->Depth.Clear,
                                       (FxU32) ctx->Stencil.Clear);
            if (ctx->Depth.Mask && ctx->Depth.Test) {
               FX_grDepthMask_NoLock(FXTRUE);
            }
            FX_grColorMaskv_NoLock(ctx, true4);
            if (ctx->Color.DrawDestMask & FRONT_LEFT_BIT)
               FX_grRenderBuffer_NoLock(GR_BUFFER_FRONTBUFFER);
         }
      }
   }
   END_CLIP_LOOP(fxMesa);

   if (fxMesa->haveHwStencil && (mask & DD_STENCIL_BIT)) {
      /* We changed the stencil state above.  Signal that we need to
       * upload it again.
       */
      fxMesa->dirty |= TDFX_UPLOAD_STENCIL;
   }

#endif
   return softwareMask;
}



static void tdfxDDFinish( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   FLUSH_BATCH( fxMesa );

   LOCK_HARDWARE( fxMesa );
   grFinish();
   UNLOCK_HARDWARE( fxMesa );
}

static void tdfxDDFlush( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   FLUSH_BATCH( fxMesa );

   LOCK_HARDWARE( fxMesa );
   grFlush();
   UNLOCK_HARDWARE( fxMesa );
}



static const char *texSource(int k)
{
   switch (k) {
      case GR_CMBX_ZERO:
         return "GR_CMBX_ZERO";
      case GR_CMBX_TEXTURE_ALPHA:
         return "GR_CMBX_TEXTURE_ALPHA";
      case GR_CMBX_ALOCAL:
         return "GR_CMBX_ALOCAL";
      case GR_CMBX_AOTHER:
         return "GR_CMBX_AOTHER";
      case GR_CMBX_B:
         return "GR_CMBX_B";
      case GR_CMBX_CONSTANT_ALPHA:
         return "GR_CMBX_CONSTANT_ALPHA";
      case GR_CMBX_CONSTANT_COLOR:
         return "GR_CMBX_CONSTANT_COLOR";
      case GR_CMBX_DETAIL_FACTOR:
         return "GR_CMBX_DETAIL_FACTOR";
      case GR_CMBX_ITALPHA:
         return "GR_CMBX_ITALPHA";
      case GR_CMBX_ITRGB:
         return "GR_CMBX_ITRGB";
      case GR_CMBX_LOCAL_TEXTURE_ALPHA:
         return "GR_CMBX_LOCAL_TEXTURE_ALPHA";
      case GR_CMBX_LOCAL_TEXTURE_RGB:
         return "GR_CMBX_LOCAL_TEXTURE_RGB";
      case GR_CMBX_LOD_FRAC:
         return "GR_CMBX_LOD_FRAC";
      case GR_CMBX_OTHER_TEXTURE_ALPHA:
         return "GR_CMBX_OTHER_TEXTURE_ALPHA";
      case GR_CMBX_OTHER_TEXTURE_RGB:
         return "GR_CMBX_OTHER_TEXTURE_RGB";
      case GR_CMBX_TEXTURE_RGB:
         return "GR_CMBX_TEXTURE_RGB";
      case GR_CMBX_TMU_CALPHA:
         return "GR_CMBX_TMU_CALPHA";
      case GR_CMBX_TMU_CCOLOR:
         return "GR_CMBX_TMU_CCOLOR";
      default:
         return "";
   }
}

static const char *texMode(int k)
{
   switch (k) {
      case GR_FUNC_MODE_ZERO:
         return "GR_FUNC_MODE_ZERO";
      case GR_FUNC_MODE_X:
         return "GR_FUNC_MODE_X";
      case GR_FUNC_MODE_ONE_MINUS_X:
         return "GR_FUNC_MODE_ONE_MINUS_X";
      case GR_FUNC_MODE_NEGATIVE_X:
         return "GR_FUNC_MODE_NEGATIVE_X";
      case GR_FUNC_MODE_X_MINUS_HALF:
         return "GR_FUNC_MODE_X_MINUS_HALF";
      default:
         return "";
   }
}

static const char *texInvert(int k)
{
   return k ? "FXTRUE" : "FXFALSE";
}

static void uploadTextureEnv( tdfxContextPtr fxMesa )
{
   if (TDFX_IS_NAPALM(fxMesa)) {
      int unit;
      for (unit = 0; unit < TDFX_NUM_TMU; unit++) {
#if 0
         printf("upload env %d\n", unit);
         printf("   cSourceA = %s\t", texSource(fxMesa->TexCombineExt[unit].Color.SourceA));
         printf("     cModeA = %s\n", texMode(fxMesa->TexCombineExt[unit].Color.ModeA));
         printf("   cSourceB = %s\t", texSource(fxMesa->TexCombineExt[unit].Color.SourceB));
         printf("     cModeB = %s\n", texMode(fxMesa->TexCombineExt[unit].Color.ModeB));
         printf("   cSourceC = %s\t", texSource(fxMesa->TexCombineExt[unit].Color.SourceC));
         printf("   cInvertC = %s\n", texInvert(fxMesa->TexCombineExt[unit].Color.InvertC));
         printf("   cSourceD = %s\t", texSource(fxMesa->TexCombineExt[unit].Color.SourceD));
         printf("   cInvertD = %s\n", texInvert(fxMesa->TexCombineExt[unit].Color.InvertD));
         printf("     cShift = %d\t", fxMesa->TexCombineExt[unit].Color.Shift);
         printf("    cInvert = %d\n", fxMesa->TexCombineExt[unit].Color.Invert);
         printf("   aSourceA = %s\t", texSource(fxMesa->TexCombineExt[unit].Alpha.SourceA));
         printf("     aModeA = %s\n", texMode(fxMesa->TexCombineExt[unit].Alpha.ModeA));
         printf("   aSourceB = %s\t", texSource(fxMesa->TexCombineExt[unit].Alpha.SourceB));
         printf("     aModeB = %s\n", texMode(fxMesa->TexCombineExt[unit].Alpha.ModeB));
         printf("   aSourceC = %s\t", texSource(fxMesa->TexCombineExt[unit].Alpha.SourceC));
         printf("   aInvertC = %s\n", texInvert(fxMesa->TexCombineExt[unit].Alpha.InvertC));
         printf("   aSourceD = %s\t", texSource(fxMesa->TexCombineExt[unit].Alpha.SourceD));
         printf("   aInvertD = %s\n", texInvert(fxMesa->TexCombineExt[unit].Alpha.InvertD));
         printf("     aShift = %d\t", fxMesa->TexCombineExt[unit].Alpha.Shift);
         printf("    aInvert = %d\n", fxMesa->TexCombineExt[unit].Alpha.Invert);
         printf("      Color = 0x%08x\n", fxMesa->TexCombineExt[unit].EnvColor);
#endif
         (*grTexColorCombineExtProc)(TDFX_TMU0 + unit,
                                     fxMesa->TexCombineExt[unit].Color.SourceA,
                                     fxMesa->TexCombineExt[unit].Color.ModeA,
                                     fxMesa->TexCombineExt[unit].Color.SourceB,
                                     fxMesa->TexCombineExt[unit].Color.ModeB,
                                     fxMesa->TexCombineExt[unit].Color.SourceC,
                                     fxMesa->TexCombineExt[unit].Color.InvertC,
                                     fxMesa->TexCombineExt[unit].Color.SourceD,
                                     fxMesa->TexCombineExt[unit].Color.InvertD,
                                     fxMesa->TexCombineExt[unit].Color.Shift,
                                     fxMesa->TexCombineExt[unit].Color.Invert);
         (*grTexAlphaCombineExtProc)(TDFX_TMU0 + unit,
                                     fxMesa->TexCombineExt[unit].Alpha.SourceA,
                                     fxMesa->TexCombineExt[unit].Alpha.ModeA,
                                     fxMesa->TexCombineExt[unit].Alpha.SourceB,
                                     fxMesa->TexCombineExt[unit].Alpha.ModeB,
                                     fxMesa->TexCombineExt[unit].Alpha.SourceC,
                                     fxMesa->TexCombineExt[unit].Alpha.InvertC,
                                     fxMesa->TexCombineExt[unit].Alpha.SourceD,
                                     fxMesa->TexCombineExt[unit].Alpha.InvertD,
                                     fxMesa->TexCombineExt[unit].Alpha.Shift,
                                     fxMesa->TexCombineExt[unit].Alpha.Invert);
         (*grConstantColorValueExtProc)(TDFX_TMU0 + unit,
                                        fxMesa->TexCombineExt[unit].EnvColor);
      }
   }
   else {
      /* Voodoo3 */
      int unit;
      for (unit = 0; unit < TDFX_NUM_TMU; unit++) {
         struct tdfx_texcombine *comb = &fxMesa->TexCombine[unit];
         FX_grTexCombine_NoLock(TDFX_TMU0 + unit,
                                comb->FunctionRGB,
                                comb->FactorRGB,
                                comb->FunctionAlpha,
                                comb->FactorAlpha,
                                comb->InvertRGB,
                                comb->InvertAlpha);
      }
   }
}


static void uploadTextureParams( tdfxContextPtr fxMesa )
{
   int unit;
   for (unit = 0; unit < TDFX_NUM_TMU; unit++) {
      const struct tdfx_texparams *p = &fxMesa->TexParams[unit];
      /*
      printf("upload params %d\n", unit);
      printf("   clamp %x %x\n", env->sClamp, env->tClamp);
      printf("   filter %x %x\n", env->minFilt, env->magFilt);
      printf("   mipmap %x %x\n", env->mmMode, env->LODblend);
      printf("   lod bias %f\n", env->LodBias);
      */
      FX_grTexClampMode_NoLock(GR_TMU0 + unit, p->sClamp, p->tClamp);
      FX_grTexFilterMode_NoLock(GR_TMU0 + unit, p->minFilt, p->magFilt);
      FX_grTexMipMapMode_NoLock(GR_TMU0 + unit, p->mmMode, p->LODblend);
      FX_grTexLodBiasValue_NoLock(GR_TMU0 + unit, p->LodBias);
   }
}


static void uploadTextureSource( tdfxContextPtr fxMesa )
{
   int unit;
   for (unit = 0; unit < TDFX_NUM_TMU; unit++) {
      const struct tdfx_texsource *src = &fxMesa->TexSource[unit];
      /*
      printf("upload source %d @ %d %p\n", unit, src->StartAddress, src->Info);
      */
      if (src->Info) {
         /*
         printf("  smallLodLog2=%d largeLodLog2=%d ar=%d format=%d data=%p\n",
                src->Info->smallLodLog2, src->Info->largeLodLog2,
                src->Info->aspectRatioLog2, src->Info->format,
                src->Info->data);
         */
         FX_grTexSource_NoLock(GR_TMU0 + unit,
                               src->StartAddress,
                               src->EvenOdd,
                               src->Info);
      }
   }
}


static void uploadTextureImages( tdfxContextPtr fxMesa )
{
   GLcontext *ctx = fxMesa->glCtx;
   int unit;
   for ( unit = 0 ; unit < TDFX_NUM_TMU ; unit++ ) {
      if ( ctx->Texture.Unit[unit].ReallyEnabled == TEXTURE0_2D ) {
         struct gl_texture_object *tObj = ctx->Texture.Unit[unit].CurrentD[2];
         tdfxTexObjPtr t = TDFX_TEXTURE_DATA(tObj);
         if ( t && t->reloadImages ) {
            tdfxTMDownloadTextureLocked( fxMesa, tObj );
            t->reloadImages = GL_FALSE;
         }
      }
   }
}



/*
 * If scissoring is enabled, compute intersection of scissor region
 * with all X clip rects, resulting in new cliprect list.
 * If number of cliprects is zero or one, call grClipWindow to setup
 * the clip region.  Otherwise we'll call grClipWindow inside the
 * BEGIN_CLIP_LOOP macro.
 */
void tdfxUploadClipping( tdfxContextPtr fxMesa )
{
   __DRIdrawablePrivate *dPriv = fxMesa->driDrawable;

   assert(dPriv);

   if (fxMesa->numClipRects == 0) {
      /* all drawing clipped away */
      grClipWindow(0, 0, 0, 0);
   }
   else if (fxMesa->numClipRects == 1) {
      grClipWindow(fxMesa->pClipRects[0].x1,
                   fxMesa->screen_height - fxMesa->pClipRects[0].y2,
                   fxMesa->pClipRects[0].x2,
                   fxMesa->screen_height - fxMesa->pClipRects[0].y1);
   }
   /* else, we'll do a cliprect loop around all drawing */

   grDRIPosition( dPriv->x, dPriv->y, dPriv->w, dPriv->h,
		  fxMesa->numClipRects, fxMesa->pClipRects );
}


void tdfxEmitHwStateLocked( tdfxContextPtr fxMesa )
{
   if ( !fxMesa->dirty )
      return;

   if ( fxMesa->dirty & TDFX_UPLOAD_COLOR_COMBINE ) {
      if (TDFX_IS_NAPALM(fxMesa)) {
         (*grColorCombineExtProc)(fxMesa->ColorCombineExt.SourceA,
                                  fxMesa->ColorCombineExt.ModeA,
                                  fxMesa->ColorCombineExt.SourceB,
                                  fxMesa->ColorCombineExt.ModeB,
                                  fxMesa->ColorCombineExt.SourceC,
                                  fxMesa->ColorCombineExt.InvertC,
                                  fxMesa->ColorCombineExt.SourceD,
                                  fxMesa->ColorCombineExt.InvertD,
                                  fxMesa->ColorCombineExt.Shift,
                                  fxMesa->ColorCombineExt.Invert);
      }
      else {
         /* Voodoo 3 */
         grColorCombine( fxMesa->ColorCombine.Function,
                         fxMesa->ColorCombine.Factor,
                         fxMesa->ColorCombine.Local,
                         fxMesa->ColorCombine.Other,
                         fxMesa->ColorCombine.Invert );
      }
      fxMesa->dirty &= ~TDFX_UPLOAD_COLOR_COMBINE;
   }
   if ( fxMesa->dirty & TDFX_UPLOAD_ALPHA_COMBINE ) {
      if (TDFX_IS_NAPALM(fxMesa)) {
         (*grAlphaCombineExtProc)(fxMesa->AlphaCombineExt.SourceA,
                                  fxMesa->AlphaCombineExt.ModeA,
                                  fxMesa->AlphaCombineExt.SourceB,
                                  fxMesa->AlphaCombineExt.ModeB,
                                  fxMesa->AlphaCombineExt.SourceC,
                                  fxMesa->AlphaCombineExt.InvertC,
                                  fxMesa->AlphaCombineExt.SourceD,
                                  fxMesa->AlphaCombineExt.InvertD,
                                  fxMesa->AlphaCombineExt.Shift,
                                  fxMesa->AlphaCombineExt.Invert);
      }
      else {
         /* Voodoo 3 */
         grAlphaCombine( fxMesa->AlphaCombine.Function,
                         fxMesa->AlphaCombine.Factor,
                         fxMesa->AlphaCombine.Local,
                         fxMesa->AlphaCombine.Other,
                         fxMesa->AlphaCombine.Invert );
      }
      fxMesa->dirty &= ~TDFX_UPLOAD_ALPHA_COMBINE;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_RENDER_BUFFER ) {
      grRenderBuffer( fxMesa->DrawBuffer );
      fxMesa->dirty &= ~TDFX_UPLOAD_RENDER_BUFFER;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_STIPPLE) {
      grStipplePattern( fxMesa->Stipple.Pattern );
      grStippleMode( fxMesa->Stipple.Mode );
      fxMesa->dirty &= ~TDFX_UPLOAD_STIPPLE;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_ALPHA_TEST ) {
      grAlphaTestFunction( fxMesa->Color.AlphaFunc );
      fxMesa->dirty &= ~TDFX_UPLOAD_ALPHA_TEST;
   }
   if ( fxMesa->dirty & TDFX_UPLOAD_ALPHA_REF ) {
      grAlphaTestReferenceValue( fxMesa->Color.AlphaRef );
      fxMesa->dirty &= ~TDFX_UPLOAD_ALPHA_REF;
   }
   if ( fxMesa->dirty & TDFX_UPLOAD_BLEND_FUNC ) {
      if (grAlphaBlendFunctionExtProc) {
         (*grAlphaBlendFunctionExtProc)( fxMesa->Color.BlendSrcRGB,
                                         fxMesa->Color.BlendDstRGB,
                                         GR_BLEND_OP_ADD,
                                         fxMesa->Color.BlendSrcA,
                                         fxMesa->Color.BlendDstA,
                                         GR_BLEND_OP_ADD );
      }
      else {
         grAlphaBlendFunction( fxMesa->Color.BlendSrcRGB,
                               fxMesa->Color.BlendDstRGB,
                               fxMesa->Color.BlendSrcA,
                               fxMesa->Color.BlendDstA );
      }
      fxMesa->dirty &= ~TDFX_UPLOAD_BLEND_FUNC;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_DEPTH_MODE ) {
      grDepthBufferMode( fxMesa->Depth.Mode );
      fxMesa->dirty &= ~TDFX_UPLOAD_DEPTH_MODE;
   }
   if ( fxMesa->dirty & TDFX_UPLOAD_DEPTH_BIAS ) {
      grDepthBiasLevel( fxMesa->Depth.Bias );
      fxMesa->dirty &= ~TDFX_UPLOAD_DEPTH_BIAS;
   }
   if ( fxMesa->dirty & TDFX_UPLOAD_DEPTH_FUNC ) {
      grDepthBufferFunction( fxMesa->Depth.Func );
      fxMesa->dirty &= ~TDFX_UPLOAD_DEPTH_FUNC;
   }
   if ( fxMesa->dirty & TDFX_UPLOAD_DEPTH_MASK ) {
      grDepthMask( fxMesa->Depth.Mask );
      fxMesa->dirty &= ~TDFX_UPLOAD_DEPTH_MASK;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_DITHER) {
      grDitherMode( fxMesa->Color.Dither );
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_FOG_MODE ) {
      grFogMode( fxMesa->Fog.Mode );
      fxMesa->dirty &= ~TDFX_UPLOAD_FOG_MODE;
   }
   if ( fxMesa->dirty & TDFX_UPLOAD_FOG_COLOR ) {
      grFogColorValue( fxMesa->Fog.Color );
      fxMesa->dirty &= ~TDFX_UPLOAD_FOG_COLOR;
   }
   if ( fxMesa->dirty & TDFX_UPLOAD_FOG_TABLE ) {
      grFogTable( fxMesa->Fog.Table );
      fxMesa->dirty &= ~TDFX_UPLOAD_FOG_TABLE;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_CULL ) {
      grCullMode( fxMesa->CullMode );
      fxMesa->dirty &= ~TDFX_UPLOAD_CULL;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_CLIP ) {
      tdfxUploadClipping( fxMesa );
      fxMesa->dirty &= ~TDFX_UPLOAD_CLIP;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_COLOR_MASK ) {
      if ( grColorMaskExtProc && fxMesa->glCtx->Visual->RedBits == 8) {
	 grColorMaskExtProc( fxMesa->Color.ColorMask[RCOMP],
			     fxMesa->Color.ColorMask[GCOMP],
			     fxMesa->Color.ColorMask[BCOMP],
			     fxMesa->Color.ColorMask[ACOMP] );
      } else {
	 grColorMask( fxMesa->Color.ColorMask[RCOMP] ||
		      fxMesa->Color.ColorMask[GCOMP] ||
		      fxMesa->Color.ColorMask[BCOMP],
		      fxMesa->Color.ColorMask[ACOMP] );
      }
      fxMesa->dirty &= ~TDFX_UPLOAD_COLOR_MASK;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_CONSTANT_COLOR ) {
      grConstantColorValue( fxMesa->Color.MonoColor );
      fxMesa->dirty &= ~TDFX_UPLOAD_CONSTANT_COLOR;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_LINE ) {
      if (fxMesa->glCtx->Line.SmoothFlag && fxMesa->glCtx->Line.Width == 1.0)
         grEnable(GR_AA_ORDERED);
      else
         grDisable(GR_AA_ORDERED);
      fxMesa->dirty &= ~TDFX_UPLOAD_LINE;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_STENCIL ) {
      if (fxMesa->glCtx->Stencil.Enabled) {
         grEnable(GR_STENCIL_MODE_EXT);
         FX_grStencilOp_NoLock(fxMesa->Stencil.FailFunc,
                               fxMesa->Stencil.ZFailFunc,
                               fxMesa->Stencil.ZPassFunc);
         FX_grStencilFunc_NoLock(fxMesa->Stencil.Function,
                                 fxMesa->Stencil.RefValue,
                                 fxMesa->Stencil.ValueMask);
         FX_grStencilMask_NoLock(fxMesa->Stencil.WriteMask);
      }
      else {
         grDisable(GR_STENCIL_MODE_EXT);
      }
      fxMesa->dirty &= ~TDFX_UPLOAD_STENCIL;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_VERTEX_LAYOUT ) {
      grGlideSetVertexLayout( fxMesa->layout[fxMesa->vertexFormat] );
      fxMesa->dirty &= ~TDFX_UPLOAD_VERTEX_LAYOUT;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_TEXTURE_ENV ) {
      uploadTextureEnv(fxMesa);
      fxMesa->dirty &= ~TDFX_UPLOAD_TEXTURE_ENV;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_TEXTURE_PARAMS ) {
      uploadTextureParams(fxMesa);
      fxMesa->dirty &= ~TDFX_UPLOAD_TEXTURE_PARAMS;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_TEXTURE_PALETTE ) {
      if (fxMesa->TexPalette.Data) {
         grTexDownloadTable(fxMesa->TexPalette.Type, fxMesa->TexPalette.Data);
      }
      fxMesa->dirty &= ~TDFX_UPLOAD_TEXTURE_PALETTE;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_TEXTURE_SOURCE ) {
      uploadTextureSource(fxMesa);
      fxMesa->dirty &= ~TDFX_UPLOAD_TEXTURE_SOURCE;
   }

   if ( fxMesa->dirty & TDFX_UPLOAD_TEXTURE_IMAGES ) {
      uploadTextureImages(fxMesa);
      fxMesa->dirty &= ~TDFX_UPLOAD_TEXTURE_IMAGES;
   }

   fxMesa->dirty = 0;
}



void tdfxDDInitRenderFuncs( GLcontext *ctx )
{
   ctx->Driver.Clear		= tdfxDDClear;
   ctx->Driver.Finish		= tdfxDDFinish;
   ctx->Driver.Flush		= tdfxDDFlush;
}
