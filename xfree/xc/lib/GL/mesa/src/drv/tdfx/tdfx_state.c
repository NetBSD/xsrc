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
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_state.c,v 1.2 2001/05/02 15:06:04 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Brian Paul <brianp@valinux.com>
 *
 */

#include "types.h"
#include "pb.h"

#include "dri_glide.h"

#include "tdfx_context.h"
#include "tdfx_state.h"
#include "tdfx_vb.h"
#include "tdfx_tex.h"
#include "tdfx_texman.h"
#include "tdfx_tris.h"
#include "tdfx_render.h"



/* =============================================================
 * Alpha blending
 */

static void tdfxUpdateAlphaMode( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   GrCmpFnc_t func;
   GrAlphaBlendFnc_t srcRGB, dstRGB, srcA, dstA;
   GrAlpha_t ref = ctx->Color.AlphaRef;
   const int hasAlpha = ctx->Visual->AlphaBits > 0;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s()\n", __FUNCTION__ );
   }

   if ( ctx->Color.AlphaEnabled ) {
      switch ( ctx->Color.AlphaFunc ) {
      case GL_NEVER:
	 func = GR_CMP_NEVER;
	 break;
      case GL_LESS:
	 func = GR_CMP_LESS;
         break;
      case GL_LEQUAL:
	 func = GR_CMP_LEQUAL;
	 break;
      case GL_EQUAL:
	 func = GR_CMP_EQUAL;
	 break;
      case GL_GEQUAL:
	 func = GR_CMP_GEQUAL;
	 break;
      case GL_GREATER:
	 func = GR_CMP_GREATER;
	 break;
      case GL_NOTEQUAL:
	 func = GR_CMP_NOTEQUAL;
	 break;
      case GL_ALWAYS:
      default:
	 func = GR_CMP_ALWAYS;
	 break;
      }
   } else {
      func = GR_CMP_ALWAYS;
   }

   if ( ctx->Color.BlendEnabled
        && (fxMesa->Fallback & TDFX_FALLBACK_BLEND) == 0 ) {
      switch ( ctx->Color.BlendSrcRGB ) {
      case GL_ZERO:
	 srcRGB = GR_BLEND_ZERO;
	 break;
      case GL_ONE:
	 srcRGB = GR_BLEND_ONE;
	 break;
      case GL_DST_COLOR:
	 srcRGB = GR_BLEND_DST_COLOR;
	 break;
      case GL_ONE_MINUS_DST_COLOR:
	 srcRGB = GR_BLEND_ONE_MINUS_DST_COLOR;
	 break;
      case GL_SRC_ALPHA:
	 srcRGB = GR_BLEND_SRC_ALPHA;
	 break;
      case GL_ONE_MINUS_SRC_ALPHA:
	 srcRGB = GR_BLEND_ONE_MINUS_SRC_ALPHA;
	 break;
      case GL_DST_ALPHA:
	 srcRGB = hasAlpha ? GR_BLEND_DST_ALPHA : GR_BLEND_ONE;
	 break;
      case GL_ONE_MINUS_DST_ALPHA:
	 srcRGB = hasAlpha ? GR_BLEND_ONE_MINUS_DST_ALPHA : GR_BLEND_ZERO;
	 break;
      case GL_SRC_ALPHA_SATURATE:
	 srcRGB = hasAlpha ? GR_BLEND_ALPHA_SATURATE : GR_BLEND_ZERO;
	 break;
      default:
	 srcRGB = GR_BLEND_ONE;
      }

      switch ( ctx->Color.BlendSrcA ) {
      case GL_ZERO:
	 srcA = GR_BLEND_ZERO;
	 break;
      case GL_ONE:
	 srcA = GR_BLEND_ONE;
	 break;
      case GL_DST_COLOR:		/* Napalm only */
	 srcA = hasAlpha ? GR_BLEND_DST_ALPHA : GR_BLEND_ONE;
	 break;
      case GL_ONE_MINUS_DST_COLOR:	/* Napalm only */
	 srcA = hasAlpha ? GR_BLEND_ONE_MINUS_DST_ALPHA : GR_BLEND_ZERO;
	 break;
      case GL_SRC_ALPHA:		/* Napalm only */
	 srcA = GR_BLEND_SRC_ALPHA;
	 break;
      case GL_ONE_MINUS_SRC_ALPHA:	/* Napalm only */
	 srcA = GR_BLEND_ONE_MINUS_SRC_ALPHA;
	 break;
      case GL_DST_ALPHA:		/* Napalm only */
	 srcA = hasAlpha ? GR_BLEND_DST_ALPHA : GR_BLEND_ONE;
	 break;
      case GL_ONE_MINUS_DST_ALPHA:	/* Napalm only */
	 srcA = hasAlpha ? GR_BLEND_ONE_MINUS_DST_ALPHA : GR_BLEND_ZERO;
	 break;
      case GL_SRC_ALPHA_SATURATE:
         srcA = GR_BLEND_ONE;
	 break;
      default:
	 srcA = GR_BLEND_ONE;
      }

      switch ( ctx->Color.BlendDstRGB ) {
      case GL_ZERO:
	 dstRGB = GR_BLEND_ZERO;
	 break;
      case GL_ONE:
	 dstRGB = GR_BLEND_ONE;
	 break;
      case GL_SRC_COLOR:
	 dstRGB = GR_BLEND_SRC_COLOR;
	 break;
      case GL_ONE_MINUS_SRC_COLOR:
	 dstRGB = GR_BLEND_ONE_MINUS_SRC_COLOR;
	 break;
      case GL_SRC_ALPHA:
	 dstRGB = GR_BLEND_SRC_ALPHA;
	 break;
      case GL_ONE_MINUS_SRC_ALPHA:
	 dstRGB = GR_BLEND_ONE_MINUS_SRC_ALPHA;
	 break;
      case GL_DST_ALPHA:
	 dstRGB = hasAlpha ? GR_BLEND_DST_ALPHA : GR_BLEND_ONE;
	 break;
      case GL_ONE_MINUS_DST_ALPHA:
	 dstRGB = hasAlpha ? GR_BLEND_ONE_MINUS_DST_ALPHA : GR_BLEND_ZERO;
	 break;
      default:
	 dstRGB = GR_BLEND_ZERO;
      }

      switch ( ctx->Color.BlendDstA ) {
      case GL_ZERO:
	 dstA = GR_BLEND_ZERO;
	 break;
      case GL_ONE:
	 dstA = GR_BLEND_ONE;
	 break;
      case GL_SRC_COLOR:		/* Napalm only */
	 dstA = GR_BLEND_SRC_ALPHA;
	 break;
      case GL_ONE_MINUS_SRC_COLOR:	/* Napalm only */
	 dstA = GR_BLEND_ONE_MINUS_SRC_ALPHA;
	 break;
      case GL_SRC_ALPHA:		/* Napalm only */
	 dstA = GR_BLEND_SRC_ALPHA;
	 break;
      case GL_ONE_MINUS_SRC_ALPHA:	/* Napalm only */
	 dstA = GR_BLEND_ONE_MINUS_SRC_ALPHA;
	 break;
      case GL_DST_ALPHA:		/* Napalm only */
	 dstA = hasAlpha ? GR_BLEND_DST_ALPHA : GR_BLEND_ONE;
	 break;
      case GL_ONE_MINUS_DST_ALPHA:	/* Napalm only */
	 dstA = hasAlpha ? GR_BLEND_ONE_MINUS_DST_ALPHA : GR_BLEND_ZERO;
	 break;
      default:
	 dstA = GR_BLEND_ZERO;
      }
   } else {
      /* blend disabled */
      srcRGB = GR_BLEND_ONE;
      dstRGB = GR_BLEND_ZERO;
      srcA = GR_BLEND_ONE;
      dstA = GR_BLEND_ZERO;
   }

   if ( fxMesa->Color.AlphaFunc != func ) {
      fxMesa->Color.AlphaFunc = func;
      fxMesa->dirty |= TDFX_UPLOAD_ALPHA_TEST;
   }
   if ( fxMesa->Color.AlphaRef != ref ) {
      fxMesa->Color.AlphaRef = ref;
      fxMesa->dirty |= TDFX_UPLOAD_ALPHA_REF;
   }

   if ( fxMesa->Color.BlendSrcRGB != srcRGB ||
	fxMesa->Color.BlendDstRGB != dstRGB ||
	fxMesa->Color.BlendSrcA != srcA ||
	fxMesa->Color.BlendDstA != dstA )
   {
      fxMesa->Color.BlendSrcRGB = srcRGB;
      fxMesa->Color.BlendDstRGB = dstRGB;
      fxMesa->Color.BlendSrcA = srcA;
      fxMesa->Color.BlendDstA = dstA;
      fxMesa->dirty |= TDFX_UPLOAD_BLEND_FUNC;
   }
}

static void tdfxDDAlphaFunc( GLcontext *ctx, GLenum func, GLclampf ref )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );

   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_ALPHA;
}

static void tdfxDDBlendEquation( GLcontext *ctx, GLenum mode )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );

   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_ALPHA;

   if (ctx->Color.ColorLogicOpEnabled && ctx->Color.LogicOp != GL_COPY)
      fxMesa->Fallback |= TDFX_FALLBACK_LOGICOP;
   else
      fxMesa->Fallback &= ~TDFX_FALLBACK_LOGICOP;
}

static void tdfxDDBlendFunc( GLcontext *ctx, GLenum sfactor, GLenum dfactor )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );

   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_ALPHA;

   /*
    * XXX - Voodoo5 seems to suffer from precision problems in some
    * blend modes.  To pass all the conformance tests we'd have to
    * fall back to software for many modes.  Revisit someday.
    */
}

static void tdfxDDBlendFuncSeparate( GLcontext *ctx,
				     GLenum sfactorRGB, GLenum dfactorRGB,
				     GLenum sfactorA, GLenum dfactorA )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );

   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_ALPHA;
}

/* =============================================================
 * Stipple
 */

static void tdfxUpdateStipple( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );
   GrStippleMode_t mode = GR_STIPPLE_DISABLE;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s()\n", __FUNCTION__ );
   }

   FLUSH_BATCH( fxMesa );

   if (ctx->Polygon.StippleFlag) {
      mode = GR_STIPPLE_PATTERN;
   }

   if ( fxMesa->Stipple.Mode != mode ) {
      fxMesa->Stipple.Mode = mode;
      fxMesa->dirty |= TDFX_UPLOAD_STIPPLE;
   }
}


/* =============================================================
 * Depth testing
 */

static void tdfxUpdateZMode( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );
   GrCmpFnc_t func;
   FxI32 bias;
   FxBool mask;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s()\n", __FUNCTION__ );
   }

   bias = (FxI32) (ctx->Polygon.OffsetUnits * TDFX_DEPTH_BIAS_SCALE);

   if ( ctx->Depth.Test ) {
      switch ( ctx->Depth.Func ) {
      case GL_NEVER:
	 func = GR_CMP_NEVER;
	 break;
      case GL_LESS:
	 func = GR_CMP_LESS;
         break;
      case GL_LEQUAL:
	 func = GR_CMP_LEQUAL;
	 break;
      case GL_EQUAL:
	 func = GR_CMP_EQUAL;
	 break;
      case GL_GEQUAL:
	 func = GR_CMP_GEQUAL;
	 break;
      case GL_GREATER:
	 func = GR_CMP_GREATER;
	 break;
      case GL_NOTEQUAL:
	 func = GR_CMP_NOTEQUAL;
	 break;
      case GL_ALWAYS:
      default:
	 func = GR_CMP_ALWAYS;
	 break;
      }

      if ( ctx->Depth.Mask ) {
         mask = FXTRUE;
      }
      else {
         mask = FXFALSE;
      }
   }
   else {
      /* depth testing disabled */
      func = GR_CMP_ALWAYS;  /* fragments always pass */
      mask = FXFALSE;        /* zbuffer is not touched */
   }

   fxMesa->Depth.Clear = (FxU32) (((1 << fxMesa->glVis->DepthBits) - 1)
                                  * ctx->Depth.Clear);

   if ( fxMesa->Depth.Bias != bias ) {
      fxMesa->Depth.Bias = bias;
      fxMesa->dirty |= TDFX_UPLOAD_DEPTH_BIAS;
   }
   if ( fxMesa->Depth.Func != func ) {
      fxMesa->Depth.Func = func;
      fxMesa->dirty |= TDFX_UPLOAD_DEPTH_FUNC | TDFX_UPLOAD_DEPTH_MASK;
   }
   if ( fxMesa->Depth.Mask != mask ) {
      fxMesa->Depth.Mask = mask;
      fxMesa->dirty |= TDFX_UPLOAD_DEPTH_MASK;
   }
}

static void tdfxDDDepthFunc( GLcontext *ctx, GLenum func )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );

   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_DEPTH;
}

static void tdfxDDDepthMask( GLcontext *ctx, GLboolean flag )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );

   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_DEPTH;
}

static void tdfxDDClearDepth( GLcontext *ctx, GLclampd d )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );

   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_DEPTH;
}



/* =============================================================
 * Stencil
 */


/* Evaluate all stencil state and make the Glide calls.
 */
static GrStencil_t convertGLStencilOp( GLenum op )
{
   switch ( op ) {
   case GL_KEEP:
      return GR_STENCILOP_KEEP;
   case GL_ZERO:
      return GR_STENCILOP_ZERO;
   case GL_REPLACE:
      return GR_STENCILOP_REPLACE;
   case GL_INCR:
      return GR_STENCILOP_INCR_CLAMP;
   case GL_DECR:
      return GR_STENCILOP_DECR_CLAMP;
   case GL_INVERT:
      return GR_STENCILOP_INVERT;
   case GL_INCR_WRAP_EXT:
      return GR_STENCILOP_INCR_WRAP;
   case GL_DECR_WRAP_EXT:
      return GR_STENCILOP_DECR_WRAP;
   default:
      gl_problem( NULL, "bad stencil op in convertGLStencilOp" );
   }
   return GR_STENCILOP_KEEP;   /* never get, silence compiler warning */
}


static void tdfxUpdateStencil( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s()\n", __FUNCTION__ );
   }

   if (fxMesa->haveHwStencil) {
      if (ctx->Stencil.Enabled) {
         fxMesa->Stencil.Function = ctx->Stencil.Function - GL_NEVER;
         fxMesa->Stencil.RefValue = ctx->Stencil.Ref;
         fxMesa->Stencil.ValueMask = ctx->Stencil.ValueMask;
         fxMesa->Stencil.WriteMask = ctx->Stencil.WriteMask;
         fxMesa->Stencil.FailFunc = convertGLStencilOp(ctx->Stencil.FailFunc);
         fxMesa->Stencil.ZFailFunc =convertGLStencilOp(ctx->Stencil.ZFailFunc);
         fxMesa->Stencil.ZPassFunc =convertGLStencilOp(ctx->Stencil.ZPassFunc);
         fxMesa->Stencil.Clear = ctx->Stencil.Clear & 0xff;
      }
      fxMesa->dirty |= TDFX_UPLOAD_STENCIL;
   }
}


static void tdfxDDStencilFunc( GLcontext *ctx, GLenum func,
			       GLint ref, GLuint mask )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_STENCIL;
}

static void tdfxDDStencilMask( GLcontext *ctx, GLuint mask )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_STENCIL;
}

static void tdfxDDStencilOp( GLcontext *ctx, GLenum sfail,
			     GLenum zfail, GLenum zpass )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_STENCIL;
}


/* =============================================================
 * Fog - orthographic fog still not working
 */

static void tdfxUpdateFogAttrib( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   GrFogMode_t mode;
   GrColor_t color;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s()\n", __FUNCTION__ );
   }

   if ( ctx->Fog.Enabled ) {
      mode = GR_FOG_WITH_TABLE_ON_Q;
   } else {
      mode = GR_FOG_DISABLE;
   }

   color = TDFXPACKCOLOR888((GLubyte)(ctx->Fog.Color[0]*255.0F),
			    (GLubyte)(ctx->Fog.Color[1]*255.0F),
			    (GLubyte)(ctx->Fog.Color[2]*255.0F));

   if ( fxMesa->Fog.Mode != mode ) {
      fxMesa->Fog.Mode = mode;
      fxMesa->dirty |= TDFX_UPLOAD_FOG_MODE;
   }
   if ( fxMesa->Fog.Color != color ) {
      fxMesa->Fog.Color = color;
      fxMesa->dirty |= TDFX_UPLOAD_FOG_COLOR;
   }
   if ( fxMesa->Fog.TableMode != ctx->Fog.Mode ||
	fxMesa->Fog.Density != ctx->Fog.Density ||
	fxMesa->Fog.Near != ctx->Fog.Start ||
	fxMesa->Fog.Far != ctx->Fog.End )
   {
      switch( ctx->Fog.Mode ) {
      case GL_EXP:
	 guFogGenerateExp( fxMesa->Fog.Table, ctx->Fog.Density );
	 break;
      case GL_EXP2:
	 guFogGenerateExp2( fxMesa->Fog.Table, ctx->Fog.Density );
	 break;
      case GL_LINEAR:
	 guFogGenerateLinear( fxMesa->Fog.Table,
			      ctx->Fog.Start, ctx->Fog.End );
	 break;
      }

      fxMesa->Fog.TableMode = ctx->Fog.Mode;
      fxMesa->Fog.Density = ctx->Fog.Density;
      fxMesa->Fog.Near = ctx->Fog.Start;
      fxMesa->Fog.Far = ctx->Fog.End;
      fxMesa->dirty |= TDFX_UPLOAD_FOG_TABLE;
   }
}

static void tdfxDDFogfv( GLcontext *ctx, GLenum pname, const GLfloat *param )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_FOG;
}


/* =============================================================
 * Clipping
 */

static int intersect_rect( XF86DRIClipRectPtr out,
			   const XF86DRIClipRectPtr a,
			   const XF86DRIClipRectPtr b)
{
   *out = *a;
   if (b->x1 > out->x1) out->x1 = b->x1;
   if (b->y1 > out->y1) out->y1 = b->y1;
   if (b->x2 < out->x2) out->x2 = b->x2;
   if (b->y2 < out->y2) out->y2 = b->y2;
   if (out->x1 >= out->x2) return 0;
   if (out->y1 >= out->y2) return 0;
   return 1;
}


/*
 * Examine XF86 cliprect list and scissor state to recompute our
 * cliprect list.
 */
void tdfxUpdateClipping( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   __DRIdrawablePrivate *dPriv = fxMesa->driDrawable;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s()\n", __FUNCTION__ );
   }

   assert(ctx);
   assert(fxMesa);
   assert(dPriv);

   if ( dPriv->x != fxMesa->x_offset || dPriv->y != fxMesa->y_offset ||
	dPriv->w != fxMesa->width || dPriv->h != fxMesa->height ) {
      fxMesa->x_offset = dPriv->x;
      fxMesa->y_offset = dPriv->y;
      fxMesa->width = dPriv->w;
      fxMesa->height = dPriv->h;
      fxMesa->y_delta =
	 fxMesa->screen_height - fxMesa->y_offset - fxMesa->height;
   }

   if (fxMesa->scissoredClipRects && fxMesa->pClipRects) {
      free(fxMesa->pClipRects);
   }

   if (ctx->Scissor.Enabled) {
      /* intersect OpenGL scissor box with all cliprects to make a new
       * list of cliprects.
       */
      XF86DRIClipRectRec scissor;
      int x1 = ctx->Scissor.X + fxMesa->x_offset;
      int y1 = fxMesa->screen_height - fxMesa->y_delta
             - ctx->Scissor.Y - ctx->Scissor.Height;
      int x2 = x1 + ctx->Scissor.Width;
      int y2 = y1 + ctx->Scissor.Height;
      scissor.x1 = MAX2(x1, 0);
      scissor.y1 = MAX2(y1, 0);
      scissor.x2 = MAX2(x2, 0);
      scissor.y2 = MAX2(y2, 0);

      assert(scissor.x2 >= scissor.x1);
      assert(scissor.y2 >= scissor.y1);

      fxMesa->pClipRects = malloc(dPriv->numClipRects
                                  * sizeof(XF86DRIClipRectRec));
      if (fxMesa->pClipRects) {
         int i;
         fxMesa->numClipRects = 0;
         for (i = 0; i < dPriv->numClipRects; i++) {
            if (intersect_rect(&fxMesa->pClipRects[fxMesa->numClipRects],
                               &scissor, &dPriv->pClipRects[i])) {
               fxMesa->numClipRects++;
            }
         }
         fxMesa->scissoredClipRects = GL_TRUE;
      }
      else {
         /* out of memory, forgo scissor */
         fxMesa->numClipRects = dPriv->numClipRects;
         fxMesa->pClipRects = dPriv->pClipRects;
         fxMesa->scissoredClipRects = GL_FALSE;
      }
   }
   else {
      fxMesa->numClipRects = dPriv->numClipRects;
      fxMesa->pClipRects = dPriv->pClipRects;
      fxMesa->scissoredClipRects = GL_FALSE;
   }

   fxMesa->dirty |= TDFX_UPLOAD_CLIP;
}



/* =============================================================
 * Culling
 */

static void tdfxUpdateCull( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   GrCullMode_t mode = GR_CULL_DISABLE;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s()\n", __FUNCTION__ );
   }

   if ( ctx->Polygon.CullFlag &&
        (ctx->PB->primitive == GL_POLYGON ||
         ctx->PB->primitive == GL_BITMAP) ) {
      switch ( ctx->Polygon.CullFaceMode ) {
      case GL_FRONT:
	 if ( ctx->Polygon.FrontFace == GL_CCW ) {
	    mode = GR_CULL_POSITIVE;
	 } else {
	    mode = GR_CULL_NEGATIVE;
	 }
	 break;

      case GL_BACK:
	 if ( ctx->Polygon.FrontFace == GL_CCW ) {
	    mode = GR_CULL_NEGATIVE;
	 } else {
	    mode = GR_CULL_POSITIVE;
	 }
	 break;

      case GL_FRONT_AND_BACK:
      default:
	 mode = GR_CULL_DISABLE;
	 break;
      }
   }

   if ( fxMesa->CullMode != mode ) {
      fxMesa->CullMode = mode;
      fxMesa->dirty |= TDFX_UPLOAD_CULL;
   }
}

static void tdfxDDCullFace( GLcontext *ctx, GLenum mode )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );

   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_CULL;
}

static void tdfxDDFrontFace( GLcontext *ctx, GLenum mode )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );

   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_CULL;
}


/* =============================================================
 * Line drawing.
 */

static void tdfxUpdateLine( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s()\n", __FUNCTION__ );
   }

   FLUSH_BATCH( fxMesa );
   fxMesa->dirty |= TDFX_UPLOAD_LINE;
}


static void tdfxDDLineWidth( GLcontext *ctx, GLfloat width )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );
   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_LINE;
}



/* =============================================================
 * Color Attributes
 */

static void tdfxDDLogicOp( GLcontext *ctx, GLenum opcode )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   if (ctx->Color.ColorLogicOpEnabled)
   {
      FLUSH_BATCH( fxMesa );

      if (opcode == GL_COPY)
	 fxMesa->Fallback &= ~TDFX_FALLBACK_LOGICOP;
      else
	 fxMesa->Fallback |= TDFX_FALLBACK_LOGICOP;
   }
   else
      fxMesa->Fallback &= ~TDFX_FALLBACK_LOGICOP;
}


static GLboolean tdfxDDColorMask( GLcontext *ctx,
				  GLboolean r, GLboolean g,
				  GLboolean b, GLboolean a )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   FLUSH_BATCH( fxMesa );

   if ( fxMesa->Color.ColorMask[RCOMP] != r ||
	fxMesa->Color.ColorMask[GCOMP] != g ||
	fxMesa->Color.ColorMask[BCOMP] != b ||
	fxMesa->Color.ColorMask[ACOMP] != a ) {
      fxMesa->Color.ColorMask[RCOMP] = r;
      fxMesa->Color.ColorMask[GCOMP] = g;
      fxMesa->Color.ColorMask[BCOMP] = b;
      fxMesa->Color.ColorMask[ACOMP] = a;
      fxMesa->dirty |= TDFX_UPLOAD_COLOR_MASK;

      if (ctx->Visual->RedBits < 8) {
         /* Can't do RGB colormasking in 16bpp mode. */
         /* We can completely ignore the alpha mask. */
         if (r != g || g != b) {
            fxMesa->Fallback |= TDFX_FALLBACK_COLORMASK;
         }
         else {
            fxMesa->Fallback &= ~TDFX_FALLBACK_COLORMASK;
         }
      }
   }

   return GL_FALSE; /* This forces the software paths to do colormasking. */
                    /* This function will return void when we use Mesa 3.5 */
}

static void tdfxDDColor( GLcontext *ctx,
			 GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   GrColor_t color;

   FLUSH_BATCH( fxMesa );

   color = tdfxPackColor( fxMesa->fxScreen->cpp, r, g, b, a );

   if ( fxMesa->Color.MonoColor != color ) {
      fxMesa->Color.MonoColor = color;
      fxMesa->dirty |= TDFX_UPLOAD_CONSTANT_COLOR;
   }
}

static void tdfxDDClearColor( GLcontext *ctx,
			      GLubyte red, GLubyte green,
			      GLubyte blue, GLubyte alpha )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   FLUSH_BATCH( fxMesa );

   fxMesa->Color.ClearColor = TDFXPACKCOLOR888( red, green, blue );
   fxMesa->Color.ClearAlpha = alpha;
}


/* =============================================================
 * Light Model
 */

static void tdfxDDLightModelfv( GLcontext *ctx, GLenum pname,
				const GLfloat *param )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   if ( pname == GL_LIGHT_MODEL_COLOR_CONTROL ) {
      FLUSH_BATCH( fxMesa );

      fxMesa->Fallback &= ~TDFX_FALLBACK_SPECULAR;

      if ( ctx->Light.Enabled &&
	   ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR ) {
	 fxMesa->Fallback |= TDFX_FALLBACK_SPECULAR;
      }
   }
}

static void tdfxDDShadeModel( GLcontext *ctx, GLenum mode )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   /* FIXME: Can we implement native flat shading? */
   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_TEXTURE;
}


/* =============================================================
 * Scissor
 */

static void
tdfxDDScissor(GLcontext * ctx, GLint x, GLint y, GLsizei w, GLsizei h)
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_CLIP;
}

/* =============================================================
 * Render
 */

static void tdfxUpdateRenderAttrib( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   FLUSH_BATCH( fxMesa );
   fxMesa->dirty |= TDFX_UPLOAD_RENDER_BUFFER;
}

/* =============================================================
 * Viewport
 */

static void tdfxUpdateViewport( GLcontext *ctx )
{
   /* XXX: Implement this when we're doing clip coordinates */
   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s()\n", __FUNCTION__ );
   }
}


static void tdfxDDViewport( GLcontext *ctx, GLint x, GLint y,
			    GLsizei w, GLsizei h )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_VIEWPORT;
}


static void tdfxDDNearFar( GLcontext *ctx, GLfloat nearVal, GLfloat farVal )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   FLUSH_BATCH( fxMesa );
   fxMesa->new_state |= TDFX_NEW_VIEWPORT;
}


/* =============================================================
 * State enable/disable
 */

static void tdfxDDEnable( GLcontext *ctx, GLenum cap, GLboolean state )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );

   switch ( cap ) {
   case GL_ALPHA_TEST:
      FLUSH_BATCH( fxMesa );
      fxMesa->new_state |= TDFX_NEW_ALPHA;
      break;

   case GL_BLEND:
      FLUSH_BATCH( fxMesa );
      fxMesa->new_state |= TDFX_NEW_ALPHA;

      if (ctx->Color.ColorLogicOpEnabled && ctx->Color.LogicOp != GL_COPY)
	 fxMesa->Fallback |= TDFX_FALLBACK_LOGICOP;
      else
	 fxMesa->Fallback &= ~TDFX_FALLBACK_LOGICOP;
      break;

   case GL_CULL_FACE:
      FLUSH_BATCH( fxMesa );
      fxMesa->new_state |= TDFX_NEW_CULL;
      break;

   case GL_DEPTH_TEST:
      FLUSH_BATCH( fxMesa );
      fxMesa->new_state |= TDFX_NEW_DEPTH;
      break;

   case GL_DITHER:
      FLUSH_BATCH( fxMesa );
      if ( state ) {
	 fxMesa->Color.Dither = GR_DITHER_2x2;
      } else {
	 fxMesa->Color.Dither = GR_DITHER_DISABLE;
      }
      fxMesa->dirty |= TDFX_UPLOAD_DITHER;
      break;

   case GL_FOG:
      FLUSH_BATCH( fxMesa );
      fxMesa->new_state |= TDFX_NEW_FOG;
      break;

   case GL_COLOR_LOGIC_OP:
      FLUSH_BATCH( fxMesa );
      if ( state && ctx->Color.LogicOp != GL_COPY ) {
         fxMesa->Fallback |= TDFX_FALLBACK_LOGICOP;
      } else {
         fxMesa->Fallback &= ~TDFX_FALLBACK_LOGICOP;
      }
      break;

   case GL_LINE_SMOOTH:
      FLUSH_BATCH( fxMesa );
      fxMesa->new_state |= TDFX_NEW_LINE;
      break;

   case GL_POLYGON_STIPPLE:
      FLUSH_BATCH(fxMesa);
      fxMesa->new_state |= TDFX_NEW_STIPPLE;
      break;

   case GL_SCISSOR_TEST:
      FLUSH_BATCH( fxMesa );
      fxMesa->new_state |= TDFX_NEW_CLIP;
      break;

   case GL_STENCIL_TEST:
      FLUSH_BATCH( fxMesa );
      if (fxMesa->haveHwStencil)
	 fxMesa->new_state |= TDFX_NEW_STENCIL;
      else if (state)
	 fxMesa->Fallback |= TDFX_FALLBACK_STENCIL;
      else
	 fxMesa->Fallback &= ~TDFX_FALLBACK_STENCIL;
      break;

   case GL_TEXTURE_1D:
   case GL_TEXTURE_3D:
      if (state)
	 fxMesa->Fallback |= TDFX_FALLBACK_TEXTURE;
      else
	 fxMesa->Fallback &= ~TDFX_FALLBACK_TEXTURE;
      fxMesa->new_state |= TDFX_NEW_TEXTURE;
      break;

   case GL_TEXTURE_2D:
      FLUSH_BATCH( fxMesa );
      fxMesa->new_state |= TDFX_NEW_TEXTURE;
      break;

   default:
      return;
   }
}



/* Set the buffer used for drawing */
/* XXX support for separate read/draw buffers hasn't been tested */
static GLboolean tdfxDDSetDrawBuffer( GLcontext *ctx, GLenum mode )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s()\n", __FUNCTION__ );
   }

   FLUSH_BATCH( fxMesa );

   fxMesa->Fallback &= ~TDFX_FALLBACK_BUFFER;

   switch ( mode ) {
   case GL_FRONT_LEFT:
      fxMesa->DrawBuffer = GR_BUFFER_FRONTBUFFER;
      fxMesa->new_state |= TDFX_NEW_RENDER;
      return GL_TRUE;

   case GL_BACK_LEFT:
      fxMesa->DrawBuffer = GR_BUFFER_BACKBUFFER;
      fxMesa->new_state |= TDFX_NEW_RENDER;
      return GL_TRUE;

   case GL_NONE:
      FX_grColorMaskv( ctx, false4 );
      return GL_TRUE;

   default:
      fxMesa->Fallback |= TDFX_FALLBACK_BUFFER;
      return GL_FALSE;
   }
}


/* Set the buffer used for reading */
/* XXX support for separate read/draw buffers hasn't been tested */
static void tdfxDDSetReadBuffer( GLcontext *ctx,
				 GLframebuffer *buffer, GLenum mode )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   (void) buffer;

   FLUSH_BATCH( fxMesa );

   fxMesa->Fallback &= ~TDFX_FALLBACK_BUFFER;

   switch ( mode ) {
   case GL_FRONT_LEFT:
      fxMesa->ReadBuffer = GR_BUFFER_FRONTBUFFER;
      break;

   case GL_BACK_LEFT:
      fxMesa->ReadBuffer = GR_BUFFER_BACKBUFFER;
      break;

   default:
      fxMesa->Fallback |= TDFX_FALLBACK_BUFFER;
      break;
   }
}

/* =============================================================
 * Polygon stipple
 */

static void tdfxDDPolygonStipple( GLcontext *ctx, const GLubyte *mask )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   const GLubyte *m = mask;
   GLubyte q[4];
   int i,j,k;
   int active = (ctx->Polygon.StippleFlag && ctx->PB->primitive == GL_POLYGON);

   FLUSH_BATCH(fxMesa);

   if (active) {
      ctx->Driver.TriangleCaps |= DD_TRI_STIPPLE;
   }

   q[0] = mask[0];
   q[1] = mask[4];
   q[2] = mask[8];
   q[3] = mask[12];

   for (k = 0 ; k < 8 ; k++)
      for (j = 0 ; j < 4; j++)
	 for (i = 0 ; i < 4 ; i++,m++) {
	    if (*m != q[j]) {
	       ctx->Driver.TriangleCaps &= ~DD_TRI_STIPPLE;
               fxMesa->Stipple.Pattern = 0xffffffff; /* ensure all pixels on */
	       return;
	    }
         }

   /* We can do it, so flag an upload of the stipple pattern */
   fxMesa->Stipple.Pattern = ( (q[0] << 0) |
                               (q[1] << 8) |
                               (q[2] << 16) |
                               (q[3] << 24) );
   fxMesa->dirty |= TDFX_UPLOAD_STIPPLE;
}

/* Always called between RenderStart and RenderFinish --> We already
 * hold the lock.
 */
static void tdfxDDReducedPrimitiveChange( GLcontext *ctx, GLenum prim )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT( ctx );

   FLUSH_BATCH( fxMesa );

   tdfxUpdateCull(ctx);
   if ( fxMesa->dirty & TDFX_UPLOAD_CULL ) {
      grCullMode( fxMesa->CullMode );
      fxMesa->dirty &= ~TDFX_UPLOAD_CULL;
   }

   tdfxUpdateStipple(ctx);
   if ( fxMesa->dirty & TDFX_UPLOAD_STIPPLE ) {
      grStipplePattern ( fxMesa->Stipple.Pattern );
      grStippleMode ( fxMesa->Stipple.Mode );
      fxMesa->dirty &= ~TDFX_UPLOAD_STIPPLE;
   }
}



static void tdfxDDPrintState( const char *msg, GLuint flags )
{
   fprintf( stderr,
	    "%s: (0x%x) %s%s%s%s%s%s%s%s%s%s%s%s%s\n",
	    msg,
	    flags,
	    (flags & TDFX_NEW_COLOR) ? "color, " : "",
	    (flags & TDFX_NEW_ALPHA) ? "alpha, " : "",
	    (flags & TDFX_NEW_DEPTH) ? "depth, " : "",
	    (flags & TDFX_NEW_RENDER) ? "render, " : "",
	    (flags & TDFX_NEW_FOG) ? "fog, " : "",
	    (flags & TDFX_NEW_STENCIL) ? "stencil, " : "",
	    (flags & TDFX_NEW_STIPPLE) ? "stipple, " : "",
	    (flags & TDFX_NEW_CLIP) ? "clip, " : "",
	    (flags & TDFX_NEW_VIEWPORT) ? "viewport, " : "",
	    (flags & TDFX_NEW_CULL) ? "cull, " : "",
	    (flags & TDFX_NEW_GLIDE) ? "glide, " : "",
	    (flags & TDFX_NEW_TEXTURE) ? "texture, " : "",
	    (flags & TDFX_NEW_CONTEXT) ? "context, " : "");
}



void tdfxDDUpdateHwState( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);
   int new_state = fxMesa->new_state;

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s()\n", __FUNCTION__ );
   }

   if ( new_state )
   {
      FLUSH_BATCH( fxMesa );

      fxMesa->new_state = 0;

      if ( 0 )
	 tdfxDDPrintState( "tdfxUpdateHwState", new_state );

      /* Update the various parts of the context's state.
       */
      if ( new_state & TDFX_NEW_ALPHA ) {
	 tdfxUpdateAlphaMode( ctx );
      }

      if ( new_state & TDFX_NEW_DEPTH )
	 tdfxUpdateZMode( ctx );

      if ( new_state & TDFX_NEW_FOG )
	 tdfxUpdateFogAttrib( ctx );

      if ( new_state & TDFX_NEW_CLIP )
	 tdfxUpdateClipping( ctx );

      if ( new_state & TDFX_NEW_STIPPLE )
	 tdfxUpdateStipple( ctx );

      if ( new_state & TDFX_NEW_CULL )
	 tdfxUpdateCull( ctx );

      if ( new_state & TDFX_NEW_LINE )
         tdfxUpdateLine( ctx );

      if ( new_state & TDFX_NEW_VIEWPORT )
	 tdfxUpdateViewport( ctx );

      if ( new_state & TDFX_NEW_RENDER )
	 tdfxUpdateRenderAttrib( ctx );

      if ( new_state & TDFX_NEW_STENCIL )
         tdfxUpdateStencil( ctx );

      if ( new_state & TDFX_NEW_TEXTURE ) {
	 tdfxUpdateTextureState( ctx );
      }
      else if ( new_state & TDFX_NEW_TEXTURE_BIND ) {
	 tdfxUpdateTextureBinding( ctx );
      }
   }

   if ( 0 ) {
      FxI32 bias = (FxI32) (ctx->Polygon.OffsetUnits * TDFX_DEPTH_BIAS_SCALE);

      if ( fxMesa->Depth.Bias != bias ) {
	 fxMesa->Depth.Bias = bias;
	 fxMesa->dirty |= TDFX_UPLOAD_DEPTH_BIAS;
      }
   }

   if ( fxMesa->dirty ) {
      LOCK_HARDWARE( fxMesa );
      tdfxEmitHwStateLocked( fxMesa );
      UNLOCK_HARDWARE( fxMesa );
   }
}


static void tdfxDDRenderStart( GLcontext *ctx )
{
   tdfxDDUpdateHwState( ctx );
   LOCK_HARDWARE( TDFX_CONTEXT(ctx) );
}

static void tdfxDDRenderFinish( GLcontext *ctx )
{
   UNLOCK_HARDWARE( TDFX_CONTEXT(ctx) );
}

#define INTERESTED (~(NEW_MODELVIEW |		\
		      NEW_PROJECTION |		\
		      NEW_TEXTURE_MATRIX |	\
		      NEW_USER_CLIP |		\
		      NEW_CLIENT_STATE |	\
		      NEW_TEXTURE_ENABLE))

static void tdfxDDUpdateState( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   if ( TDFX_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s()\n", __FUNCTION__ );
   }

   /* Have to do this here to detect texture, line fallbacks in time:
    */
   if ( fxMesa->new_state & (TDFX_NEW_TEXTURE | TDFX_NEW_LINE) )
      tdfxDDUpdateHwState( ctx );

   if ( ctx->NewState & INTERESTED ) {
      tdfxDDChooseRenderState( ctx );
   }

   /* The choise of vertex setup function only depends on whether fog
    * and/or texturing is enabled.
    */
   if ( ctx->NewState & (NEW_FOG | NEW_TEXTURE_ENABLE | NEW_TEXTURING)) {
      tdfxDDChooseRasterSetupFunc( ctx );
   }

   if ( 0 )
      fprintf( stderr, "fallback %x indirect %x\n",
	       fxMesa->Fallback, fxMesa->IndirectTriangles );

   if ( fxMesa->Fallback ) {
      ctx->IndirectTriangles |= ctx->TriangleCaps;
   }
   else {
      ctx->IndirectTriangles &= ~DD_SW_RASTERIZE;
      ctx->IndirectTriangles |= fxMesa->IndirectTriangles;

      ctx->Driver.PointsFunc	= fxMesa->PointsFunc;
      ctx->Driver.LineFunc	= fxMesa->LineFunc;
      ctx->Driver.TriangleFunc	= fxMesa->TriangleFunc;
      ctx->Driver.QuadFunc	= fxMesa->QuadFunc;
      ctx->Driver.RenderVBRawTab = fxMesa->RenderVBRawTab;
   }
}



/* Initialize the context's Glide state mirror.  These values will be
 * used as Glide function call parameters when the time comes.
 */
void tdfxInitState( tdfxContextPtr fxMesa )
{
   GLcontext *ctx = fxMesa->glCtx;
   GLint i;

   fxMesa->ColorCombine.Function	= GR_COMBINE_FUNCTION_LOCAL;
   fxMesa->ColorCombine.Factor		= GR_COMBINE_FACTOR_NONE;
   fxMesa->ColorCombine.Local		= GR_COMBINE_LOCAL_ITERATED;
   fxMesa->ColorCombine.Other		= GR_COMBINE_OTHER_NONE;
   fxMesa->ColorCombine.Invert		= FXFALSE;
   fxMesa->AlphaCombine.Function	= GR_COMBINE_FUNCTION_LOCAL;
   fxMesa->AlphaCombine.Factor		= GR_COMBINE_FACTOR_NONE;
   fxMesa->AlphaCombine.Local		= GR_COMBINE_LOCAL_ITERATED;
   fxMesa->AlphaCombine.Other		= GR_COMBINE_OTHER_NONE;
   fxMesa->AlphaCombine.Invert		= FXFALSE;

   fxMesa->ColorCombineExt.SourceA	= GR_CMBX_ITRGB;
   fxMesa->ColorCombineExt.ModeA	= GR_FUNC_MODE_X;
   fxMesa->ColorCombineExt.SourceB	= GR_CMBX_ZERO;
   fxMesa->ColorCombineExt.ModeB	= GR_FUNC_MODE_ZERO;
   fxMesa->ColorCombineExt.SourceC	= GR_CMBX_ZERO;
   fxMesa->ColorCombineExt.InvertC	= FXTRUE;
   fxMesa->ColorCombineExt.SourceD	= GR_CMBX_ZERO;
   fxMesa->ColorCombineExt.InvertD	= FXFALSE;
   fxMesa->ColorCombineExt.Shift	= 0;
   fxMesa->ColorCombineExt.Invert	= FXFALSE;
   fxMesa->AlphaCombineExt.SourceA	= GR_CMBX_ITALPHA;
   fxMesa->AlphaCombineExt.ModeA	= GR_FUNC_MODE_X;
   fxMesa->AlphaCombineExt.SourceB	= GR_CMBX_ZERO;
   fxMesa->AlphaCombineExt.ModeB	= GR_FUNC_MODE_ZERO;
   fxMesa->AlphaCombineExt.SourceC	= GR_CMBX_ZERO;
   fxMesa->AlphaCombineExt.InvertC	= FXTRUE;
   fxMesa->AlphaCombineExt.SourceD	= GR_CMBX_ZERO;
   fxMesa->AlphaCombineExt.InvertD	= FXFALSE;
   fxMesa->AlphaCombineExt.Shift	= 0;
   fxMesa->AlphaCombineExt.Invert	= FXFALSE;

   fxMesa->sScale0 = fxMesa->tScale0 = 1.0;
   fxMesa->sScale1 = fxMesa->tScale1 = 1.0;

   fxMesa->TexPalette.Type = 0;
   fxMesa->TexPalette.Data = NULL;

   for ( i = 0 ; i < TDFX_NUM_TMU ; i++ ) {
      fxMesa->TexSource[i].StartAddress	= 0;
      fxMesa->TexSource[i].EvenOdd	= GR_MIPMAPLEVELMASK_EVEN;
      fxMesa->TexSource[i].Info		= NULL;

      fxMesa->TexCombine[i].FunctionRGB		= 0;
      fxMesa->TexCombine[i].FactorRGB		= 0;
      fxMesa->TexCombine[i].FunctionAlpha	= 0;
      fxMesa->TexCombine[i].FactorAlpha		= 0;
      fxMesa->TexCombine[i].InvertRGB		= FXFALSE;
      fxMesa->TexCombine[i].InvertAlpha		= FXFALSE;

      fxMesa->TexCombineExt[i].Alpha.SourceA	= 0;
      /* XXX more state to init here */
      fxMesa->TexCombineExt[i].Color.SourceA	= 0;
      fxMesa->TexCombineExt[i].EnvColor        = 0x0;

      fxMesa->TexParams[i].sClamp 	= GR_TEXTURECLAMP_WRAP;
      fxMesa->TexParams[i].tClamp	= GR_TEXTURECLAMP_WRAP;
      fxMesa->TexParams[i].minFilt	= GR_TEXTUREFILTER_POINT_SAMPLED;
      fxMesa->TexParams[i].magFilt	= GR_TEXTUREFILTER_BILINEAR;
      fxMesa->TexParams[i].mmMode	= GR_MIPMAP_DISABLE;
      fxMesa->TexParams[i].LODblend	= FXFALSE;
      fxMesa->TexParams[i].LodBias	= 0.0;

      fxMesa->TexState.EnvMode[i]	= ~0;
      fxMesa->TexState.TexFormat[i]	= ~0;
      fxMesa->TexState.Enabled		= 0;
   }

   if ( ctx->Visual->DBflag) {
      fxMesa->DrawBuffer		= GR_BUFFER_BACKBUFFER;
      fxMesa->ReadBuffer		= GR_BUFFER_BACKBUFFER;
   } else {
      fxMesa->DrawBuffer		= GR_BUFFER_FRONTBUFFER;
      fxMesa->ReadBuffer		= GR_BUFFER_FRONTBUFFER;
   }

   fxMesa->Color.ClearColor		= 0x00000000;
   fxMesa->Color.ClearAlpha		= 0x00;
   fxMesa->Color.ColorMask[RCOMP]	= FXTRUE;
   fxMesa->Color.ColorMask[BCOMP]	= FXTRUE;
   fxMesa->Color.ColorMask[GCOMP]	= FXTRUE;
   fxMesa->Color.ColorMask[ACOMP]	= FXTRUE;
   fxMesa->Color.MonoColor		= 0xffffffff;

   fxMesa->Color.AlphaFunc		= GR_CMP_ALWAYS;
   fxMesa->Color.AlphaRef		= 0x00;
   fxMesa->Color.BlendSrcRGB		= GR_BLEND_ONE;
   fxMesa->Color.BlendDstRGB		= GR_BLEND_ZERO;
   fxMesa->Color.BlendSrcA		= GR_BLEND_ONE;
   fxMesa->Color.BlendSrcA		= GR_BLEND_ZERO;

   fxMesa->Color.Dither			= GR_DITHER_2x2;

   if ( fxMesa->glVis->DepthBits > 0 ) {
      fxMesa->Depth.Mode		= GR_DEPTHBUFFER_ZBUFFER;
   } else {
      fxMesa->Depth.Mode		= GR_DEPTHBUFFER_DISABLE;
   }
   fxMesa->Depth.Bias			= 0;
   fxMesa->Depth.Func			= GR_CMP_LESS;
   fxMesa->Depth.Clear			= 0; /* computed later */
   fxMesa->Depth.Mask			= FXTRUE;


   fxMesa->Fog.Mode			= GR_FOG_DISABLE;
   fxMesa->Fog.Color			= 0x00000000;
   fxMesa->Fog.Table			= NULL;
   fxMesa->Fog.Density			= 1.0;
   fxMesa->Fog.Near			= 1.0;
   fxMesa->Fog.Far			= 1.0;

   fxMesa->Stencil.Function		= GR_CMP_ALWAYS;
   fxMesa->Stencil.RefValue		= 0;
   fxMesa->Stencil.ValueMask		= 0xff;
   fxMesa->Stencil.WriteMask		= 0xff;
   fxMesa->Stencil.FailFunc		= 0;
   fxMesa->Stencil.ZFailFunc		= 0;
   fxMesa->Stencil.ZPassFunc		= 0;
   fxMesa->Stencil.Clear		= 0;

   fxMesa->Stipple.Mode                 = GR_STIPPLE_DISABLE;
   fxMesa->Stipple.Pattern              = 0xffffffff;

   fxMesa->Scissor.minX			= 0;
   fxMesa->Scissor.minY			= 0;
   fxMesa->Scissor.maxX			= 0;
   fxMesa->Scissor.maxY			= 0;

   fxMesa->Viewport.Mode		= GR_WINDOW_COORDS;
   fxMesa->Viewport.X			= 0;
   fxMesa->Viewport.Y			= 0;
   fxMesa->Viewport.Width		= 0;
   fxMesa->Viewport.Height		= 0;
   fxMesa->Viewport.Near		= 0.0;
   fxMesa->Viewport.Far			= 0.0;

   fxMesa->CullMode			= GR_CULL_DISABLE;

   fxMesa->Glide.ColorFormat		= GR_COLORFORMAT_ABGR;
   fxMesa->Glide.Origin			= GR_ORIGIN_LOWER_LEFT;
   fxMesa->Glide.Initialized		= FXFALSE;
}



void tdfxDDInitStateFuncs( GLcontext *ctx )
{
   tdfxContextPtr fxMesa = TDFX_CONTEXT(ctx);

   ctx->Driver.UpdateState		= tdfxDDUpdateState;

   ctx->Driver.ClearIndex		= NULL;
   ctx->Driver.ClearColor		= tdfxDDClearColor;
   ctx->Driver.Index			= NULL;
   ctx->Driver.Color			= tdfxDDColor;
   ctx->Driver.SetDrawBuffer		= tdfxDDSetDrawBuffer;
   ctx->Driver.SetReadBuffer		= tdfxDDSetReadBuffer;

   ctx->Driver.IndexMask		= NULL;
   ctx->Driver.ColorMask		= tdfxDDColorMask;

   ctx->Driver.NearFar			= tdfxDDNearFar;

   ctx->Driver.RenderStart		= tdfxDDRenderStart;
   ctx->Driver.RenderFinish		= tdfxDDRenderFinish;
   ctx->Driver.RasterSetup		= NULL;

   ctx->Driver.RenderVBClippedTab	= NULL;
   ctx->Driver.RenderVBCulledTab	= NULL;
   ctx->Driver.RenderVBRawTab		= NULL;

   ctx->Driver.ReducedPrimitiveChange	= tdfxDDReducedPrimitiveChange;
   ctx->Driver.MultipassFunc		= NULL;

   ctx->Driver.AlphaFunc		= tdfxDDAlphaFunc;
   ctx->Driver.BlendEquation		= tdfxDDBlendEquation;
   ctx->Driver.BlendFunc		= tdfxDDBlendFunc;
   ctx->Driver.BlendFuncSeparate	= tdfxDDBlendFuncSeparate;
   ctx->Driver.ClearDepth		= tdfxDDClearDepth;
   ctx->Driver.ClearStencil		= NULL;
   ctx->Driver.CullFace			= tdfxDDCullFace;
   ctx->Driver.FrontFace		= tdfxDDFrontFace;
   ctx->Driver.DepthFunc		= tdfxDDDepthFunc;
   ctx->Driver.DepthMask		= tdfxDDDepthMask;
   ctx->Driver.DepthRange		= NULL;
   ctx->Driver.Enable			= tdfxDDEnable;
   ctx->Driver.Fogfv			= tdfxDDFogfv;
   ctx->Driver.Hint			= NULL;
   ctx->Driver.Lightfv			= NULL;
   ctx->Driver.LightModelfv		= tdfxDDLightModelfv;
   ctx->Driver.LineStipple		= NULL;
   ctx->Driver.LineWidth		= tdfxDDLineWidth;
   ctx->Driver.LogicOpcode		= tdfxDDLogicOp;
#if 0
   ctx->Driver.PolygonMode		= NULL;
#endif
   ctx->Driver.PolygonStipple		= tdfxDDPolygonStipple;
   ctx->Driver.Scissor			= tdfxDDScissor;
   ctx->Driver.ShadeModel		= tdfxDDShadeModel;

   if ( fxMesa->haveHwStencil ) {
      ctx->Driver.StencilFunc		= tdfxDDStencilFunc;
      ctx->Driver.StencilMask		= tdfxDDStencilMask;
      ctx->Driver.StencilOp		= tdfxDDStencilOp;
   } else {
      ctx->Driver.StencilFunc		= NULL;
      ctx->Driver.StencilMask		= NULL;
      ctx->Driver.StencilOp		= NULL;
   }

   ctx->Driver.Viewport			= tdfxDDViewport;
}
