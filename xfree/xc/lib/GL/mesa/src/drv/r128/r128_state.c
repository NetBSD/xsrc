/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_state.c,v 1.6 2000/12/12 17:17:07 dawes Exp $ */
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
 *   Gareth Hughes <gareth@valinux.com>
 *   Keith Whitwell <keithw@valinux.com>
 *
 */

#include "r128_context.h"
#include "r128_state.h"
#include "r128_ioctl.h"
#include "r128_tris.h"
#include "r128_vb.h"
#include "r128_tex.h"

#include "mmath.h"
#include "pb.h"
#include "enums.h"


/* =============================================================
 * Alpha blending
 */

static void r128UpdateAlphaMode( GLcontext *ctx )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );
   CARD32 a = r128ctx->setup.misc_3d_state_cntl_reg;
   CARD32 t = r128ctx->setup.tex_cntl_c;

   if ( ctx->Color.AlphaEnabled ) {
      GLubyte ref = ctx->Color.AlphaRef;

      a &= ~(R128_ALPHA_TEST_MASK | R128_REF_ALPHA_MASK);

      switch ( ctx->Color.AlphaFunc ) {
      case GL_NEVER:
	 a |= R128_ALPHA_TEST_NEVER;
	 ref = 0;
	 break;
      case GL_LESS:
	 a |= R128_ALPHA_TEST_LESS;
         break;
      case GL_LEQUAL:
	 a |= R128_ALPHA_TEST_LESSEQUAL;
	 break;
      case GL_EQUAL:
	 a |= R128_ALPHA_TEST_EQUAL;
	 break;
      case GL_GEQUAL:
	 a |= R128_ALPHA_TEST_GREATEREQUAL;
	 break;
      case GL_GREATER:
	 a |= R128_ALPHA_TEST_GREATER;
	 break;
      case GL_NOTEQUAL:
	 a |= R128_ALPHA_TEST_NEQUAL;
	 break;
      case GL_ALWAYS:
	 a |= R128_ALPHA_TEST_ALWAYS;
	 break;
      }

      a |= ref & R128_REF_ALPHA_MASK;
      t |= R128_ALPHA_TEST_ENABLE;
   } else {
      t &= ~R128_ALPHA_TEST_ENABLE;
   }

   if ( ctx->Color.BlendEnabled ) {
      a &= ~(R128_ALPHA_BLEND_SRC_MASK | R128_ALPHA_BLEND_DST_MASK);

      switch ( ctx->Color.BlendSrcRGB ) {
      case GL_ZERO:
	 a |= R128_ALPHA_BLEND_SRC_ZERO;
	 break;
      case GL_ONE:
	 a |= R128_ALPHA_BLEND_SRC_ONE;
	 break;
      case GL_DST_COLOR:
	 a |= R128_ALPHA_BLEND_SRC_DESTCOLOR;
	 break;
      case GL_ONE_MINUS_DST_COLOR:
	 a |= R128_ALPHA_BLEND_SRC_INVDESTCOLOR;
	 break;
      case GL_SRC_ALPHA:
	 a |= R128_ALPHA_BLEND_SRC_SRCALPHA;
	 break;
      case GL_ONE_MINUS_SRC_ALPHA:
	 a |= R128_ALPHA_BLEND_SRC_INVSRCALPHA;
	 break;
      case GL_DST_ALPHA:
	 a |= R128_ALPHA_BLEND_SRC_DESTALPHA;
	 break;
      case GL_ONE_MINUS_DST_ALPHA:
	 a |= R128_ALPHA_BLEND_SRC_INVDESTALPHA;
	 break;
      case GL_SRC_ALPHA_SATURATE:
	 a |= R128_ALPHA_BLEND_SRC_SRCALPHASAT;
	 break;
      }

      switch ( ctx->Color.BlendDstRGB ) {
      case GL_ZERO:
	 a |= R128_ALPHA_BLEND_DST_ZERO;
	 break;
      case GL_ONE:
	 a |= R128_ALPHA_BLEND_DST_ONE;
	 break;
      case GL_SRC_COLOR:
	 a |= R128_ALPHA_BLEND_DST_SRCCOLOR;
	 break;
      case GL_ONE_MINUS_SRC_COLOR:
	 a |= R128_ALPHA_BLEND_DST_INVSRCCOLOR;
	 break;
      case GL_SRC_ALPHA:
	 a |= R128_ALPHA_BLEND_DST_SRCALPHA;
	 break;
      case GL_ONE_MINUS_SRC_ALPHA:
	 a |= R128_ALPHA_BLEND_DST_INVSRCALPHA;
	 break;
      case GL_DST_ALPHA:
	 a |= R128_ALPHA_BLEND_DST_DESTALPHA;
	 break;
      case GL_ONE_MINUS_DST_ALPHA:
	 a |= R128_ALPHA_BLEND_DST_INVDESTALPHA;
	 break;
      }

      t |=  R128_ALPHA_ENABLE;
   } else {
      t &= ~R128_ALPHA_ENABLE;
   }

   if ( r128ctx->setup.misc_3d_state_cntl_reg != a ) {
      r128ctx->setup.misc_3d_state_cntl_reg = a;
      r128ctx->dirty |= R128_UPLOAD_CONTEXT | R128_UPLOAD_MASKS;
   }
   if ( r128ctx->setup.tex_cntl_c != t ) {
      r128ctx->setup.tex_cntl_c = t;
      r128ctx->dirty |= R128_UPLOAD_CONTEXT | R128_UPLOAD_MASKS;
   }
}

static void r128DDAlphaFunc( GLcontext *ctx, GLenum func, GLclampf ref )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   FLUSH_BATCH( r128ctx );
   r128ctx->new_state |= R128_NEW_ALPHA;
}

static void r128DDBlendEquation( GLcontext *ctx, GLenum mode )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   FLUSH_BATCH( r128ctx );
   r128ctx->new_state |= R128_NEW_ALPHA;
}

static void r128DDBlendFunc( GLcontext *ctx, GLenum sfactor, GLenum dfactor )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   FLUSH_BATCH( r128ctx );
   r128ctx->new_state |= R128_NEW_ALPHA;
}

static void r128DDBlendFuncSeparate( GLcontext *ctx,
				     GLenum sfactorRGB, GLenum dfactorRGB,
				     GLenum sfactorA, GLenum dfactorA )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   FLUSH_BATCH( r128ctx );
   r128ctx->new_state |= R128_NEW_ALPHA;
}


/* =============================================================
 * Depth testing
 */

static void r128UpdateZMode( GLcontext *ctx )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );
   CARD32 z = r128ctx->setup.z_sten_cntl_c;
   CARD32 t = r128ctx->setup.tex_cntl_c;

   if ( ctx->Depth.Test ) {
      z &= ~R128_Z_TEST_MASK;

      switch ( ctx->Depth.Func ) {
      case GL_NEVER:
	 z |= R128_Z_TEST_NEVER;
	 break;
      case GL_ALWAYS:
	 z |= R128_Z_TEST_ALWAYS;
	 break;
      case GL_LESS:
	 z |= R128_Z_TEST_LESS;
	 break;
      case GL_LEQUAL:
	 z |= R128_Z_TEST_LESSEQUAL;
	 break;
      case GL_EQUAL:
	 z |= R128_Z_TEST_EQUAL;
	 break;
      case GL_GEQUAL:
	 z |= R128_Z_TEST_GREATEREQUAL;
	 break;
      case GL_GREATER:
	 z |= R128_Z_TEST_GREATER;
	 break;
      case GL_NOTEQUAL:
	 z |= R128_Z_TEST_NEQUAL;
	 break;
      }

      t |=  R128_Z_ENABLE;
   } else {
      t &= ~R128_Z_ENABLE;
   }

   if ( ctx->Depth.Mask ) {
      t |=  R128_Z_WRITE_ENABLE;
   } else {
      t &= ~R128_Z_WRITE_ENABLE;
   }

   if ( r128ctx->setup.z_sten_cntl_c != z ) {
      r128ctx->setup.z_sten_cntl_c = z;
      r128ctx->dirty |= R128_UPLOAD_CONTEXT;
   }
   if ( r128ctx->setup.tex_cntl_c != t ) {
      r128ctx->setup.tex_cntl_c = t;
      r128ctx->dirty |= R128_UPLOAD_CONTEXT;
   }
}

static void r128DDDepthFunc( GLcontext *ctx, GLenum func )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   FLUSH_BATCH( r128ctx );
   r128ctx->new_state |= R128_NEW_DEPTH;
}

static void r128DDDepthMask( GLcontext *ctx, GLboolean flag )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   FLUSH_BATCH( r128ctx );
   r128ctx->new_state |= R128_NEW_DEPTH;
}

static void r128DDClearDepth( GLcontext *ctx, GLclampd d )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   switch ( r128ctx->DepthSize ) {
   case 16:
      r128ctx->ClearDepth = d * 0x0000ffff;
      break;
   case 24:
      r128ctx->ClearDepth = d * 0x00ffffff;
      break;
   case 32:
      r128ctx->ClearDepth = d * 0xffffffff;
      break;
   }
}


/* =============================================================
 * Fog
 */

static void r128UpdateFogAttrib( GLcontext *ctx )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );
   CARD32 t = r128ctx->setup.tex_cntl_c;
   GLubyte c[4];
   CARD32 col;

   if ( ctx->FogMode == FOG_FRAGMENT ) {
      t |=  R128_FOG_ENABLE;
   } else {
      t &= ~R128_FOG_ENABLE;
   }

   FLOAT_RGBA_TO_UBYTE_RGBA( c, ctx->Fog.Color );
   col = r128PackColor( 32, c[0], c[1], c[2], c[3] );

   if ( r128ctx->setup.fog_color_c != col ) {
      r128ctx->setup.fog_color_c = col;
      r128ctx->dirty |= R128_UPLOAD_CONTEXT;
   }
   if ( r128ctx->setup.tex_cntl_c != t ) {
      r128ctx->setup.tex_cntl_c = t;
      r128ctx->dirty |= R128_UPLOAD_CONTEXT;
   }
}

static void r128DDFogfv( GLcontext *ctx, GLenum pname, const GLfloat *param )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   FLUSH_BATCH( r128ctx );
   r128ctx->new_state |= R128_NEW_FOG;
}


/* =============================================================
 * Clipping
 */

static void r128UpdateClipping( GLcontext *ctx )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   if ( r128ctx->driDrawable ) {
      __DRIdrawablePrivate *drawable = r128ctx->driDrawable;
      int x1 = 0;
      int y1 = 0;
      int x2 = r128ctx->driDrawable->w - 1;
      int y2 = r128ctx->driDrawable->h - 1;

      if ( ctx->Scissor.Enabled ) {
	 if ( ctx->Scissor.X > x1 ) {
	    x1 = ctx->Scissor.X;
	 }
	 if ( drawable->h - ctx->Scissor.Y - ctx->Scissor.Height > y1 ) {
	    y1 = drawable->h - ctx->Scissor.Y - ctx->Scissor.Height;
	 }
	 if ( ctx->Scissor.X + ctx->Scissor.Width - 1 < x2 ) {
	    x2 = ctx->Scissor.X + ctx->Scissor.Width - 1;
	 }
	 if ( drawable->h - ctx->Scissor.Y - 1 < y2 ) {
	    y2 = drawable->h - ctx->Scissor.Y - 1;
	 }
      }

      x1 += r128ctx->driDrawable->x;
      y1 += r128ctx->driDrawable->y;
      x2 += r128ctx->driDrawable->x;
      y2 += r128ctx->driDrawable->y;

      if ( 0 ) {
	 fprintf( stderr, "%s: drawable %3d %3d %3d %3d\n",
		  __FUNCTION__,
		  r128ctx->driDrawable->x,
		  r128ctx->driDrawable->y,
		  r128ctx->driDrawable->w,
		  r128ctx->driDrawable->h );
	 fprintf( stderr, "%s: draw buf %3d %3d %3d %3d\n",
		  __FUNCTION__,
		  ctx->DrawBuffer->Xmin,
		  ctx->DrawBuffer->Ymin,
		  ctx->DrawBuffer->Xmax,
		  ctx->DrawBuffer->Ymax );
	 fprintf( stderr, "%s:  scissor %3d %3d %3d %3d\n",
		  __FUNCTION__,
		  ctx->Scissor.X,
		  ctx->Scissor.Y,
		  ctx->Scissor.Width,
		  ctx->Scissor.Height );
	 fprintf( stderr, "%s:    final %3d %3d %3d %3d\n",
		  __FUNCTION__, x1, y1, x2, y2 );
	 fprintf( stderr, "\n" );
      }

      r128ctx->setup.sc_top_left_c = ((x1 << 0) |
				      (y1 << 16));
      r128ctx->setup.sc_bottom_right_c = ((x2 << 0) |
					  (y2 << 16));

      r128ctx->dirty |= R128_UPLOAD_CONTEXT;
   }
}

static void r128DDScissor( GLcontext *ctx,
			   GLint x, GLint y, GLsizei w, GLsizei h )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   FLUSH_BATCH( r128ctx );
   r128ctx->new_state |= R128_NEW_CLIP;
}


/* =============================================================
 * Culling
 */

static void r128UpdateCull( GLcontext *ctx )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );
   CARD32 f = r128ctx->setup.pm4_vc_fpu_setup;

   f &= ~R128_FRONT_DIR_MASK;

   switch ( ctx->Polygon.FrontFace ) {
   case GL_CW:
      f |= R128_FRONT_DIR_CW;
      break;
   case GL_CCW:
      f |= R128_FRONT_DIR_CCW;
      break;
   }

   f |= R128_BACKFACE_SOLID | R128_FRONTFACE_SOLID;

   if ( ctx->Polygon.CullFlag && ctx->PB->primitive == GL_POLYGON ) {
      switch ( ctx->Polygon.CullFaceMode ) {
      case GL_FRONT:
	 f &= ~R128_FRONTFACE_SOLID;
	 break;
      case GL_BACK:
	 f &= ~R128_BACKFACE_SOLID;
	 break;
      case GL_FRONT_AND_BACK:
	 f &= ~(R128_BACKFACE_SOLID |
		R128_FRONTFACE_SOLID);
	 break;
      }
   }

   if ( r128ctx->setup.pm4_vc_fpu_setup != f ) {
      r128ctx->setup.pm4_vc_fpu_setup = f;
      r128ctx->dirty |= R128_UPLOAD_CONTEXT | R128_UPLOAD_SETUP;
   }
}

static void r128DDCullFace( GLcontext *ctx, GLenum mode )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   FLUSH_BATCH( r128ctx );
   r128ctx->new_state |= R128_NEW_CULL;
}

static void r128DDFrontFace( GLcontext *ctx, GLenum mode )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   FLUSH_BATCH( r128ctx );
   r128ctx->new_state |= R128_NEW_CULL;
}


/* =============================================================
 * Masks
 */

static void r128UpdateMasks( GLcontext *ctx )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   GLuint mask = r128PackColor( r128ctx->r128Screen->bpp,
				ctx->Color.ColorMask[RCOMP],
				ctx->Color.ColorMask[GCOMP],
				ctx->Color.ColorMask[BCOMP],
				ctx->Color.ColorMask[ACOMP] );

   if ( r128ctx->setup.plane_3d_mask_c != mask ) {
      r128ctx->setup.plane_3d_mask_c = mask;
      r128ctx->dirty |= R128_UPLOAD_CONTEXT | R128_UPLOAD_MASKS;
   }
}

static GLboolean r128DDColorMask( GLcontext *ctx,
				  GLboolean r, GLboolean g,
				  GLboolean b, GLboolean a )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   FLUSH_BATCH( r128ctx );
   r128ctx->new_state |= R128_NEW_MASKS;

   return GL_TRUE;
}


/* =============================================================
 * Rendering attributes
 *
 * We really don't want to recalculate all this every time we bind a
 * texture.  These things shouldn't change all that often, so it makes
 * sense to break them out of the core texture state update routines.
 */

static void r128UpdateRenderAttrib( GLcontext *ctx )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );
   CARD32 t = r128ctx->setup.tex_cntl_c;
   CARD32 bias = r128ctx->lod_bias & 0xff;;

   t &= ~R128_LOD_BIAS_MASK;
   t |= (bias << R128_LOD_BIAS_SHIFT);

   if ( ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR ) {
      t |=  R128_SPEC_LIGHT_ENABLE;
   } else {
      t &= ~R128_SPEC_LIGHT_ENABLE;
   }

   if ( ctx->Color.DitherFlag ) {
      t |=  R128_DITHER_ENABLE;
   } else {
      t &= ~R128_DITHER_ENABLE;
   }

   if ( r128ctx->setup.tex_cntl_c != t ) {
      r128ctx->setup.tex_cntl_c = t;
      r128ctx->dirty |= R128_UPLOAD_CONTEXT;
   }
}

static void r128DDLightModelfv( GLcontext *ctx, GLenum pname,
				const GLfloat *param )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   if ( pname == GL_LIGHT_MODEL_COLOR_CONTROL ) {
      FLUSH_BATCH( r128ctx );
      r128ctx->new_state |= R128_NEW_RENDER;
   }
}

static void r128DDShadeModel( GLcontext *ctx, GLenum mode )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );
   CARD32 s = r128ctx->setup.pm4_vc_fpu_setup;

   s &= ~R128_FPU_COLOR_MASK;

   switch ( mode ) {
   case GL_FLAT:
      s |= R128_FPU_COLOR_FLAT;
      break;
   case GL_SMOOTH:
      s |= R128_FPU_COLOR_GOURAUD;
      break;
   default:
      return;
   }

   if ( r128ctx->setup.pm4_vc_fpu_setup != s ) {
      FLUSH_BATCH( r128ctx );
      r128ctx->setup.pm4_vc_fpu_setup = s;

      r128ctx->new_state |= R128_NEW_CONTEXT;
      r128ctx->dirty |= R128_UPLOAD_SETUP;
   }
}


/* =============================================================
 * Window position
 */

void r128UpdateWindow( GLcontext *ctx )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );
   int x = r128ctx->driDrawable->x;
   int y = r128ctx->driDrawable->y;

   r128ctx->setup.window_xy_offset = ((y << R128_WINDOW_Y_SHIFT) |
				      (x << R128_WINDOW_X_SHIFT));

   r128ctx->dirty |= R128_UPLOAD_CONTEXT | R128_UPLOAD_WINDOW;
}


/* =============================================================
 * Miscellaneous
 */

static void r128DDClearColor( GLcontext *ctx,
			      GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   r128ctx->ClearColor = r128PackColor( r128ctx->r128Screen->bpp,
					r, g, b, a );
}

static void r128DDColor( GLcontext *ctx,
			 GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   r128ctx->Color = r128PackColor( r128ctx->r128Screen->bpp,
				   r, g, b, a );
}

static void r128DDLogicOpCode( GLcontext *ctx, GLenum opcode )
{
   if ( ctx->Color.ColorLogicOpEnabled ) {
      r128ContextPtr r128ctx = R128_CONTEXT( ctx );

      FLUSH_BATCH( r128ctx );

      if ( opcode == GL_COPY ) {
         r128ctx->Fallback &= ~R128_FALLBACK_LOGICOP;
      } else {
         r128ctx->Fallback |= R128_FALLBACK_LOGICOP;
      }
   }
}

static GLboolean r128DDSetDrawBuffer( GLcontext *ctx, GLenum mode )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );
   int found = GL_TRUE;

   FLUSH_BATCH( r128ctx );

   if ( r128ctx->DrawBuffer != mode ) {
      r128ctx->DrawBuffer = mode;
      r128ctx->Fallback &= ~R128_FALLBACK_DRAW_BUFFER;

      switch ( mode ) {
      case GL_FRONT_LEFT:
	 r128ctx->drawX      = r128ctx->r128Screen->frontX;
	 r128ctx->drawY      = r128ctx->r128Screen->frontY;
	 r128ctx->drawOffset = r128ctx->r128Screen->frontOffset;
	 r128ctx->drawPitch  = r128ctx->r128Screen->frontPitch;
	 r128ctx->readX      = r128ctx->r128Screen->frontX;
	 r128ctx->readY      = r128ctx->r128Screen->frontY;
	 break;
      case GL_BACK_LEFT:
	 r128ctx->drawX      = r128ctx->r128Screen->backX;
	 r128ctx->drawY      = r128ctx->r128Screen->backY;
	 r128ctx->drawOffset = r128ctx->r128Screen->backOffset;
	 r128ctx->drawPitch  = r128ctx->r128Screen->backPitch;
	 r128ctx->readX      = r128ctx->r128Screen->backX;
	 r128ctx->readY      = r128ctx->r128Screen->backY;
	 break;
      default:
	 r128ctx->Fallback |= R128_FALLBACK_DRAW_BUFFER;
	 found = GL_FALSE;
	 break;
      }

      r128ctx->setup.dst_pitch_offset_c = (((r128ctx->drawPitch/8) << 21) |
					   (r128ctx->drawOffset >> 5));
      r128ctx->new_state |= R128_NEW_WINDOW;
   }

   return found;
}

static void r128DDSetReadBuffer( GLcontext *ctx,
				 GLframebuffer *colorBuffer,
				 GLenum mode )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   r128ctx->Fallback &= ~R128_FALLBACK_READ_BUFFER;

   switch ( mode ) {
   case GL_FRONT_LEFT:
      r128ctx->readOffset = r128ctx->r128Screen->frontOffset;
      r128ctx->readPitch  = r128ctx->r128Screen->frontPitch;
      r128ctx->readX      = r128ctx->r128Screen->frontX;
      r128ctx->readY      = r128ctx->r128Screen->frontY;
      break;
   case GL_BACK_LEFT:
      r128ctx->readOffset = r128ctx->r128Screen->backOffset;
      r128ctx->readPitch  = r128ctx->r128Screen->backPitch;
      r128ctx->readX      = r128ctx->r128Screen->backX;
      r128ctx->readY      = r128ctx->r128Screen->backY;
      break;
   default:
      r128ctx->Fallback |= R128_FALLBACK_READ_BUFFER;
      break;
   }
}


/* =============================================================
 * Polygon stipple
 */

static void r128DDPolygonStipple( GLcontext *ctx, const GLubyte *mask )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );
   GLuint *stipple = (GLuint *)mask;

   FLUSH_BATCH( r128ctx );
   ctx->Driver.TriangleCaps |= DD_TRI_STIPPLE;

   r128ctx->setup.dp_gui_master_cntl_c &= ~R128_GMC_BRUSH_NONE;

   if ( ctx->Polygon.StippleFlag && ctx->PB->primitive == GL_POLYGON ) {
      r128ctx->setup.dp_gui_master_cntl_c |= R128_GMC_BRUSH_32x32_MONO_FG_LA;
   } else {
      r128ctx->setup.dp_gui_master_cntl_c |= R128_GMC_BRUSH_SOLID_COLOR;
   }

   LOCK_HARDWARE( r128ctx );

   drmR128PolygonStipple( r128ctx->driFd, stipple );

   UNLOCK_HARDWARE( r128ctx );

   r128ctx->new_state |= R128_NEW_CONTEXT;
   r128ctx->dirty |= R128_UPLOAD_CONTEXT;
}


/* =============================================================
 * State enable/disable
 */

static void r128DDEnable( GLcontext *ctx, GLenum cap, GLboolean state )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   if ( R128_DEBUG & DEBUG_VERBOSE_API ) {
      fprintf( stderr, "%s( %s = %s )\n",
	       __FUNCTION__, gl_lookup_enum_by_nr( cap ),
	       state ? "GL_TRUE" : "GL_FALSE" );
   }

   switch ( cap ) {
   case GL_ALPHA_TEST:
   case GL_BLEND:
      FLUSH_BATCH( r128ctx );
      r128ctx->new_state |= R128_NEW_ALPHA;
      break;

   case GL_CULL_FACE:
      FLUSH_BATCH( r128ctx );
      r128ctx->new_state |= R128_NEW_CULL;
      break;

   case GL_DEPTH_TEST:
      FLUSH_BATCH( r128ctx );
      r128ctx->new_state |= R128_NEW_DEPTH;
      break;

   case GL_DITHER:
      FLUSH_BATCH( r128ctx );
      r128ctx->new_state |= R128_NEW_RENDER;
      break;

   case GL_FOG:
      FLUSH_BATCH( r128ctx );
      r128ctx->new_state |= R128_NEW_FOG;
      break;

   case GL_INDEX_LOGIC_OP:
   case GL_COLOR_LOGIC_OP:
      FLUSH_BATCH( r128ctx );
      if ( state && ctx->Color.LogicOp != GL_COPY ) {
         r128ctx->Fallback |= R128_FALLBACK_LOGICOP;
      } else {
         r128ctx->Fallback &= ~R128_FALLBACK_LOGICOP;
      }
      break;

   case GL_SCISSOR_TEST:
      FLUSH_BATCH( r128ctx );
      r128ctx->scissor = state;
      r128ctx->new_state |= R128_NEW_CLIP;
      break;

   case GL_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_3D:
      FLUSH_BATCH( r128ctx );
      r128ctx->new_state |= R128_NEW_TEXTURE;
      break;

   case GL_POLYGON_STIPPLE:
      if ( (ctx->Driver.TriangleCaps & DD_TRI_STIPPLE) &&
	   ctx->PB->primitive == GL_POLYGON )
      {
	 FLUSH_BATCH( r128ctx );
	 r128ctx->setup.dp_gui_master_cntl_c &= ~R128_GMC_BRUSH_NONE;
	 if ( state ) {
	    r128ctx->setup.dp_gui_master_cntl_c |=
	       R128_GMC_BRUSH_32x32_MONO_FG_LA;
	 } else {
	    r128ctx->setup.dp_gui_master_cntl_c |=
	       R128_GMC_BRUSH_SOLID_COLOR;
	 }
	 r128ctx->new_state |= R128_NEW_CONTEXT;
	 r128ctx->dirty |= R128_UPLOAD_CONTEXT;
      }
      break;

   default:
      return;
   }
}


/* =============================================================
 * State initialization, management
 */

static void r128DDPrintDirty( const char *msg, GLuint state )
{
   fprintf( stderr,
	    "%s: (0x%x) %s%s%s%s%s%s%s%s%s%s%s\n",
	    msg,
	    state,
	    (state & R128_UPLOAD_CORE)		? "core, " : "",
	    (state & R128_UPLOAD_CONTEXT)	? "context, " : "",
	    (state & R128_UPLOAD_SETUP)		? "setup, " : "",
	    (state & R128_UPLOAD_TEX0)		? "tex0, " : "",
	    (state & R128_UPLOAD_TEX1)		? "tex1, " : "",
	    (state & R128_UPLOAD_TEX0IMAGES)	? "tex0 images, " : "",
	    (state & R128_UPLOAD_TEX1IMAGES)	? "tex1 images, " : "",
	    (state & R128_UPLOAD_MASKS)		? "masks, " : "",
	    (state & R128_UPLOAD_WINDOW)	? "window, " : "",
	    (state & R128_UPLOAD_CLIPRECTS)	? "cliprects, " : "",
	    (state & R128_REQUIRE_QUIESCENCE)	? "quiescence, " : "" );
}

/*
 * Load the current context's state into the hardware.
 *
 * NOTE: Be VERY careful about ensuring the context state is marked for
 * upload, the only place it shouldn't be uploaded is when the setup
 * state has changed in ReducedPrimitiveChange as this comes right after
 * a state update.
 *
 * Blits of any type should always upload the context and masks after
 * they are done.
 */
void r128EmitHwStateLocked( r128ContextPtr r128ctx )
{
   R128SAREAPrivPtr sarea = r128ctx->sarea;
   r128_context_regs_t *regs = &(r128ctx->setup);
   r128TexObjPtr t0 = r128ctx->CurrentTexObj[0];
   r128TexObjPtr t1 = r128ctx->CurrentTexObj[1];

   if ( R128_DEBUG & DEBUG_VERBOSE_MSG ) {
      r128DDPrintDirty( "r128EmitHwStateLocked", r128ctx->dirty );
   }

   if ( r128ctx->dirty & R128_UPLOAD_TEX0IMAGES ) {
      if ( t0 ) r128UploadTexImages( r128ctx, t0 );
      r128ctx->dirty &= ~R128_UPLOAD_TEX0IMAGES;
   }
   if ( r128ctx->dirty & R128_UPLOAD_TEX1IMAGES ) {
      if ( t1 ) r128UploadTexImages( r128ctx, t1 );
      r128ctx->dirty &= ~R128_UPLOAD_TEX1IMAGES;
   }

   if ( r128ctx->dirty & (R128_UPLOAD_CONTEXT |
			  R128_UPLOAD_SETUP |
			  R128_UPLOAD_MASKS |
			  R128_UPLOAD_WINDOW |
			  R128_UPLOAD_CORE |
			  R128_UPLOAD_TEX0) ) {
      memcpy( &sarea->ContextState, regs, sizeof(sarea->ContextState) );
   }

   if ( (r128ctx->dirty & R128_UPLOAD_TEX0) && t0 ) {
      memcpy( &sarea->TexState[0], &t0->setup, sizeof(sarea->TexState[0]) );
   }

   if ( (r128ctx->dirty & R128_UPLOAD_TEX1) && t1 ) {
      memcpy( &sarea->TexState[1], &t1->setup, sizeof(sarea->TexState[1]) );
   }

   sarea->vertsize = r128ctx->vertsize;
   sarea->vc_format = r128ctx->vc_format;

   /* Turn off the texture cache flushing */
   r128ctx->setup.tex_cntl_c &= ~R128_TEX_CACHE_FLUSH;

   sarea->dirty |= r128ctx->dirty;
   r128ctx->dirty &= R128_UPLOAD_CLIPRECTS;
}

static void r128DDPrintState( const char *msg, GLuint flags )
{
   fprintf( stderr,
	    "%s: (0x%x) %s%s%s%s%s%s%s%s%s\n",
	    msg,
	    flags,
	    (flags & R128_NEW_CONTEXT)	? "context, " : "",
	    (flags & R128_NEW_ALPHA)	? "alpha, " : "",
	    (flags & R128_NEW_DEPTH)	? "depth, " : "",
	    (flags & R128_NEW_FOG)	? "fog, " : "",
	    (flags & R128_NEW_CLIP)	? "clip, " : "",
	    (flags & R128_NEW_TEXTURE)	? "texture, " : "",
	    (flags & R128_NEW_CULL)	? "cull, " : "",
	    (flags & R128_NEW_MASKS)	? "masks, " : "",
	    (flags & R128_NEW_WINDOW)	? "window, " : "" );
}

/* Update the hardware state */
void r128DDUpdateHWState( GLcontext *ctx )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );
   int new_state = r128ctx->new_state;

   if ( new_state )
   {
      FLUSH_BATCH( r128ctx );

      r128ctx->new_state = 0;

      if ( R128_DEBUG & DEBUG_VERBOSE_MSG )
	 r128DDPrintState( "r128UpdateHwState", new_state );

      /* Update the various parts of the context's state.
       */
      if ( new_state & R128_NEW_ALPHA )
	 r128UpdateAlphaMode( ctx );

      if ( new_state & R128_NEW_DEPTH )
	 r128UpdateZMode( ctx );

      if ( new_state & R128_NEW_FOG )
	 r128UpdateFogAttrib( ctx );

      if ( new_state & R128_NEW_CLIP )
	 r128UpdateClipping( ctx );

      if ( new_state & R128_NEW_CULL )
	 r128UpdateCull( ctx );

      if ( new_state & R128_NEW_MASKS )
	 r128UpdateMasks( ctx );

      if ( new_state & R128_NEW_RENDER )
	 r128UpdateRenderAttrib( ctx );

      if ( new_state & R128_NEW_WINDOW )
	 r128UpdateWindow( ctx );

      if ( new_state & R128_NEW_TEXTURE )
	 r128UpdateTextureState( ctx );
   }
}

/* This is called when Mesa switches between rendering triangle
 * primitives (such as GL_POLYGON, GL_QUADS, GL_TRIANGLE_STRIP, etc),
 * and lines, points and bitmaps.
 *
 * As the r128 uses triangles to render lines and points, it is
 * necessary to turn off hardware culling when rendering these
 * primitives.
 */
static void r128DDReducedPrimitiveChange( GLcontext *ctx, GLenum prim )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );
   CARD32 f = r128ctx->setup.pm4_vc_fpu_setup;

   f |= R128_BACKFACE_SOLID | R128_FRONTFACE_SOLID;

   if ( ctx->Polygon.CullFlag && ctx->PB->primitive == GL_POLYGON ) {
      switch ( ctx->Polygon.CullFaceMode ) {
      case GL_FRONT:
	 f &= ~R128_FRONTFACE_SOLID;
	 break;
      case GL_BACK:
	 f &= ~R128_BACKFACE_SOLID;
	 break;
      case GL_FRONT_AND_BACK:
	 f &= ~(R128_BACKFACE_SOLID |
		R128_FRONTFACE_SOLID);
	 break;
      }
   }

   if ( r128ctx->setup.pm4_vc_fpu_setup != f ) {
      FLUSH_BATCH( r128ctx );
      r128ctx->setup.pm4_vc_fpu_setup = f;

      /* NOTE: Only upload the setup state, everything else has been
       * uploaded by the usual means already.
       */
      r128ctx->dirty |= R128_UPLOAD_SETUP;
   }
}


#define INTERESTED (~(NEW_MODELVIEW |		\
		      NEW_PROJECTION |		\
		      NEW_TEXTURE_MATRIX |	\
		      NEW_USER_CLIP |		\
		      NEW_CLIENT_STATE))

void r128DDUpdateState( GLcontext *ctx )
{
   r128ContextPtr r128ctx = R128_CONTEXT( ctx );

   if ( ctx->NewState & INTERESTED ) {
      r128DDChooseRenderState( ctx );
      r128DDChooseRasterSetupFunc( ctx );
   }

   /* Need to do this here to detect texture fallbacks before
    * setting triangle functions.
    */
   if ( r128ctx->new_state & R128_NEW_TEXTURE ) {
      r128DDUpdateHWState( ctx );
   }

   if ( !r128ctx->Fallback ) {
      ctx->IndirectTriangles &= ~DD_SW_RASTERIZE;
      ctx->IndirectTriangles |= r128ctx->IndirectTriangles;

      ctx->Driver.PointsFunc     = r128ctx->PointsFunc;
      ctx->Driver.LineFunc       = r128ctx->LineFunc;
      ctx->Driver.TriangleFunc   = r128ctx->TriangleFunc;
      ctx->Driver.QuadFunc       = r128ctx->QuadFunc;
   }
}


/* Initialize the context's hardware state */
void r128DDInitState( r128ContextPtr r128ctx )
{
   int dst_bpp, depth_bpp;
   CARD32 bias;

   switch ( r128ctx->r128Screen->bpp ) {
   case 16: dst_bpp = R128_GMC_DST_16BPP;   break;
   case 32: dst_bpp = R128_GMC_DST_32BPP;   break;
   default:
      fprintf( stderr, "Error: Unsupported pixel depth %d... exiting\n",
	       r128ctx->r128Screen->bpp );
      exit( -1 );
   }

   switch ( r128ctx->DepthSize ) {
   case 16:
      r128ctx->ClearDepth = 0x0000ffff;
      depth_bpp = R128_Z_PIX_WIDTH_16;
      r128ctx->depth_scale = 1.0 / 65536.0;
      break;
   case 24:
      r128ctx->ClearDepth = 0x00ffffff;
      depth_bpp = R128_Z_PIX_WIDTH_24;
      r128ctx->depth_scale = 1.0 / 16777216.0;
      break;
   case 32:
      r128ctx->ClearDepth = 0xffffffff;
      depth_bpp = R128_Z_PIX_WIDTH_32;
      r128ctx->depth_scale = 1.0 / 4294967296.0;
      break;
   default:
      fprintf( stderr, "Error: Unsupported depth %d... exiting\n",
	       r128ctx->DepthSize );
      exit( -1 );
   }

   r128ctx->ClearColor = 0x00000000;

   r128ctx->RenderIndex  = R128_FALLBACK_BIT;
   r128ctx->PointsFunc   = NULL;
   r128ctx->LineFunc     = NULL;
   r128ctx->TriangleFunc = NULL;
   r128ctx->QuadFunc     = NULL;

   r128ctx->IndirectTriangles = 0;
   r128ctx->Fallback = 0;

   if ( r128ctx->glCtx->Visual->DBflag ) {
      r128ctx->DrawBuffer = GL_BACK_LEFT;
      r128ctx->drawOffset = r128ctx->readOffset =
	 r128ctx->r128Screen->backOffset;
      r128ctx->drawPitch = r128ctx->readPitch =
	 r128ctx->r128Screen->backPitch;
   } else {
      r128ctx->DrawBuffer = GL_FRONT_LEFT;
      r128ctx->drawOffset = r128ctx->readOffset =
	 r128ctx->r128Screen->frontOffset;
      r128ctx->drawPitch = r128ctx->readPitch =
	 r128ctx->r128Screen->frontPitch;
   }

   /* Harware state:
    */
   r128ctx->setup.dst_pitch_offset_c = (((r128ctx->drawPitch/8) << 21) |
					(r128ctx->drawOffset >> 5));

   r128ctx->setup.dp_gui_master_cntl_c = (R128_GMC_DST_PITCH_OFFSET_CNTL |
					  R128_GMC_DST_CLIPPING |
					  R128_GMC_BRUSH_SOLID_COLOR |
					  dst_bpp |
					  R128_GMC_SRC_DATATYPE_COLOR |
					  R128_GMC_BYTE_MSB_TO_LSB |
					  R128_GMC_CONVERSION_TEMP_6500 |
					  R128_ROP3_S |
					  R128_DP_SRC_SOURCE_MEMORY |
					  R128_GMC_3D_FCN_EN |
					  R128_GMC_CLR_CMP_CNTL_DIS |
					  R128_GMC_AUX_CLIP_DIS |
					  R128_GMC_WR_MSK_DIS);

   r128ctx->setup.sc_top_left_c     = 0x00000000;
   r128ctx->setup.sc_bottom_right_c = 0x1fff1fff;

   r128ctx->setup.z_offset_c = r128ctx->r128Screen->depthOffset;
   r128ctx->setup.z_pitch_c = ((r128ctx->r128Screen->depthPitch >> 3) |
			       R128_Z_TILE);

   r128ctx->setup.z_sten_cntl_c = (depth_bpp |
				   R128_Z_TEST_LESS |
				   R128_STENCIL_TEST_ALWAYS |
				   R128_STENCIL_S_FAIL_KEEP |
				   R128_STENCIL_ZPASS_KEEP |
				   R128_STENCIL_ZFAIL_KEEP);

   bias = r128ctx->lod_bias & 0xff;
   r128ctx->setup.tex_cntl_c = (R128_Z_WRITE_ENABLE |
				R128_SHADE_ENABLE |
				R128_DITHER_ENABLE |
				R128_ALPHA_IN_TEX_COMPLETE_A |
				R128_LIGHT_DIS |
				R128_ALPHA_LIGHT_DIS |
				R128_TEX_CACHE_FLUSH |
				(bias << R128_LOD_BIAS_SHIFT));

   r128ctx->setup.misc_3d_state_cntl_reg = (R128_MISC_SCALE_3D_TEXMAP_SHADE |
					    R128_MISC_SCALE_PIX_REPLICATE |
					    R128_ALPHA_COMB_ADD_CLAMP |
					    R128_FOG_VERTEX |
					    R128_ALPHA_BLEND_SRC_ONE |
					    R128_ALPHA_BLEND_DST_ZERO |
					    R128_ALPHA_TEST_ALWAYS);

   r128ctx->setup.texture_clr_cmp_clr_c = 0x00000000;
   r128ctx->setup.texture_clr_cmp_msk_c = 0xffffffff;

   r128ctx->setup.fog_color_c = 0x00000000;

   r128ctx->setup.pm4_vc_fpu_setup = (R128_FRONT_DIR_CCW |
				      R128_BACKFACE_SOLID |
				      R128_FRONTFACE_SOLID |
				      R128_FPU_COLOR_GOURAUD |
				      R128_FPU_SUB_PIX_4BITS |
				      R128_FPU_MODE_3D |
				      R128_TRAP_BITS_DISABLE |
				      R128_XFACTOR_2 |
				      R128_YFACTOR_2 |
				      R128_FLAT_SHADE_VERTEX_OGL |
				      R128_FPU_ROUND_TRUNCATE |
				      R128_WM_SEL_8DW);

   r128ctx->setup.setup_cntl = (R128_COLOR_GOURAUD |
				R128_PRIM_TYPE_TRI |
				R128_TEXTURE_ST_MULT_W |
				R128_STARTING_VERTEX_1 |
				R128_ENDING_VERTEX_3 |
				R128_SU_POLY_LINE_NOT_LAST |
				R128_SUB_PIX_4BITS);

   r128ctx->setup.tex_size_pitch_c = 0x00000000;
   r128ctx->setup.constant_color_c = 0x00ffffff;

   r128ctx->setup.dp_write_mask   = 0xffffffff;
   r128ctx->setup.sten_ref_mask_c = 0xffff0000;
   r128ctx->setup.plane_3d_mask_c = 0xffffffff;

   r128ctx->setup.window_xy_offset = 0x00000000;

   r128ctx->setup.scale_3d_cntl = (R128_SCALE_DITHER_TABLE |
				   R128_TEX_CACHE_SIZE_FULL |
				   R128_DITHER_INIT_RESET |
				   R128_SCALE_3D_TEXMAP_SHADE |
				   R128_SCALE_PIX_REPLICATE |
				   R128_ALPHA_COMB_ADD_CLAMP |
				   R128_FOG_VERTEX |
				   R128_ALPHA_BLEND_SRC_ONE |
				   R128_ALPHA_BLEND_DST_ZERO |
				   R128_ALPHA_TEST_ALWAYS |
				   R128_COMPOSITE_SHADOW_CMP_EQUAL |
				   R128_TEX_MAP_ALPHA_IN_TEXTURE |
				   R128_TEX_CACHE_LINE_SIZE_4QW);

   r128ctx->new_state = R128_NEW_ALL;
}

/* Initialize the driver's state functions */
void r128DDInitStateFuncs( GLcontext *ctx )
{
   ctx->Driver.UpdateState		= r128DDUpdateState;

   ctx->Driver.ClearIndex		= NULL;
   ctx->Driver.ClearColor		= r128DDClearColor;
   ctx->Driver.Index			= NULL;
   ctx->Driver.Color			= r128DDColor;
   ctx->Driver.SetDrawBuffer		= r128DDSetDrawBuffer;
   ctx->Driver.SetReadBuffer		= r128DDSetReadBuffer;

   ctx->Driver.IndexMask		= NULL;
   ctx->Driver.ColorMask		= r128DDColorMask;
   ctx->Driver.LogicOp			= NULL;
   ctx->Driver.Dither			= NULL;

   ctx->Driver.NearFar			= NULL;

   ctx->Driver.RenderStart		= r128DDUpdateHWState;
   ctx->Driver.RenderFinish		= NULL;
   ctx->Driver.RasterSetup		= NULL;

   ctx->Driver.RenderVBClippedTab	= NULL;
   ctx->Driver.RenderVBCulledTab	= NULL;
   ctx->Driver.RenderVBRawTab		= NULL;

   ctx->Driver.ReducedPrimitiveChange	= r128DDReducedPrimitiveChange;
   ctx->Driver.MultipassFunc		= NULL;

   ctx->Driver.AlphaFunc		= r128DDAlphaFunc;
   ctx->Driver.BlendEquation		= r128DDBlendEquation;
   ctx->Driver.BlendFunc		= r128DDBlendFunc;
   ctx->Driver.BlendFuncSeparate	= r128DDBlendFuncSeparate;
   ctx->Driver.ClearDepth		= r128DDClearDepth;
   ctx->Driver.CullFace			= r128DDCullFace;
   ctx->Driver.FrontFace		= r128DDFrontFace;
   ctx->Driver.DepthFunc		= r128DDDepthFunc;
   ctx->Driver.DepthMask		= r128DDDepthMask;
   ctx->Driver.DepthRange		= NULL;
   ctx->Driver.Enable			= r128DDEnable;
   ctx->Driver.Fogfv			= r128DDFogfv;
   ctx->Driver.Hint			= NULL;
   ctx->Driver.Lightfv			= NULL;
   ctx->Driver.LightModelfv		= r128DDLightModelfv;
   ctx->Driver.LogicOpcode		= r128DDLogicOpCode;
   ctx->Driver.PolygonMode		= NULL;
   ctx->Driver.PolygonStipple		= r128DDPolygonStipple;
   ctx->Driver.Scissor			= r128DDScissor;
   ctx->Driver.ShadeModel		= r128DDShadeModel;
   ctx->Driver.ClearStencil		= NULL;
   ctx->Driver.StencilFunc		= NULL;
   ctx->Driver.StencilMask		= NULL;
   ctx->Driver.StencilOp		= NULL;
   ctx->Driver.Viewport			= NULL;
}
