/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_state.c,v 1.2 2001/03/21 16:14:25 dawes Exp $ */
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
 *   Keith Whitwell <keithw@valinux.com>
 *
 */

#include "radeon_context.h"
#include "radeon_state.h"
#include "radeon_ioctl.h"
#include "radeon_tris.h"
#include "radeon_vb.h"
#include "radeon_tex.h"

#include "mmath.h"
#include "pb.h"
#include "enums.h"


/* =============================================================
 * Alpha blending
 */

static void radeonUpdateAlphaMode( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   GLuint a = rmesa->setup.pp_misc;
   GLuint p = rmesa->setup.pp_cntl;
   GLuint b = rmesa->setup.rb3d_blendcntl;
   GLuint c = rmesa->setup.rb3d_cntl;

   if ( ctx->Color.AlphaEnabled ) {
      GLubyte ref = ctx->Color.AlphaRef;

      a &= ~(RADEON_ALPHA_TEST_OP_MASK | RADEON_REF_ALPHA_MASK);

      switch ( ctx->Color.AlphaFunc ) {
      case GL_NEVER:
	 a |= RADEON_ALPHA_TEST_FAIL;
	 break;
      case GL_LESS:
	 a |= RADEON_ALPHA_TEST_LESS;
	 break;
      case GL_LEQUAL:
	 a |= RADEON_ALPHA_TEST_LEQUAL;
	 break;
      case GL_EQUAL:
	 a |= RADEON_ALPHA_TEST_EQUAL;
	 break;
      case GL_GEQUAL:
	 a |= RADEON_ALPHA_TEST_GEQUAL;
	 break;
      case GL_GREATER:
	 a |= RADEON_ALPHA_TEST_GREATER;
	 break;
      case GL_NOTEQUAL:
	 a |= RADEON_ALPHA_TEST_NEQUAL;
	 break;
      case GL_ALWAYS:
	 a |= RADEON_ALPHA_TEST_PASS;
	 break;
      }

      a |= (ref & RADEON_REF_ALPHA_MASK);
      p |= RADEON_ALPHA_TEST_ENABLE;
   } else {
      p &= ~RADEON_ALPHA_TEST_ENABLE;
   }

   if ( ctx->Color.BlendEnabled ) {
      b &= ~(RADEON_SRC_BLEND_MASK | RADEON_DST_BLEND_MASK);

      switch ( ctx->Color.BlendSrcRGB ) {
      case GL_ZERO:
	 b |= RADEON_SRC_BLEND_GL_ZERO;
	 break;
      case GL_ONE:
	 b |= RADEON_SRC_BLEND_GL_ONE;
	 break;
      case GL_DST_COLOR:
	 b |= RADEON_SRC_BLEND_GL_DST_COLOR;
	 break;
      case GL_ONE_MINUS_DST_COLOR:
	 b |= RADEON_SRC_BLEND_GL_ONE_MINUS_DST_COLOR;
	 break;
      case GL_SRC_ALPHA:
	 b |= RADEON_SRC_BLEND_GL_SRC_ALPHA;
	 break;
      case GL_ONE_MINUS_SRC_ALPHA:
	 b |= RADEON_SRC_BLEND_GL_ONE_MINUS_SRC_ALPHA;
	 break;
      case GL_DST_ALPHA:
	 b |= RADEON_SRC_BLEND_GL_DST_ALPHA;
	 break;
      case GL_ONE_MINUS_DST_ALPHA:
	 b |= RADEON_SRC_BLEND_GL_ONE_MINUS_DST_ALPHA;
	 break;
      case GL_SRC_ALPHA_SATURATE:
	 b |= RADEON_SRC_BLEND_GL_SRC_ALPHA_SATURATE;
	 break;
      }

      switch ( ctx->Color.BlendDstRGB ) {
      case GL_ZERO:
	 b |= RADEON_DST_BLEND_GL_ZERO;
	 break;
      case GL_ONE:
	 b |= RADEON_DST_BLEND_GL_ONE;
	 break;
      case GL_SRC_COLOR:
	 b |= RADEON_DST_BLEND_GL_SRC_COLOR;
	 break;
      case GL_ONE_MINUS_SRC_COLOR:
	 b |= RADEON_DST_BLEND_GL_ONE_MINUS_SRC_COLOR;
	 break;
      case GL_SRC_ALPHA:
	 b |= RADEON_DST_BLEND_GL_SRC_ALPHA;
	 break;
      case GL_ONE_MINUS_SRC_ALPHA:
	 b |= RADEON_DST_BLEND_GL_ONE_MINUS_SRC_ALPHA;
	 break;
      case GL_DST_ALPHA:
	 b |= RADEON_DST_BLEND_GL_DST_ALPHA;
	 break;
      case GL_ONE_MINUS_DST_ALPHA:
	 b |= RADEON_DST_BLEND_GL_ONE_MINUS_DST_ALPHA;
	 break;
      }

      c |=  RADEON_ALPHA_BLEND_ENABLE;
   } else {
      c &= ~RADEON_ALPHA_BLEND_ENABLE;
   }

   if ( rmesa->setup.pp_misc != a ) {
      rmesa->setup.pp_misc = a;
      rmesa->dirty |= RADEON_UPLOAD_CONTEXT | RADEON_UPLOAD_MASKS;
   }
   if ( rmesa->setup.pp_cntl != p ) {
      rmesa->setup.pp_cntl = p;
      rmesa->dirty |= RADEON_UPLOAD_CONTEXT | RADEON_UPLOAD_MASKS;
   }
   if ( rmesa->setup.rb3d_blendcntl != b ) {
      rmesa->setup.rb3d_blendcntl = b;
      rmesa->dirty |= RADEON_UPLOAD_CONTEXT | RADEON_UPLOAD_MASKS;
   }
   if ( rmesa->setup.rb3d_cntl != c ) {
      rmesa->setup.rb3d_cntl = c;
      rmesa->dirty |= RADEON_UPLOAD_CONTEXT | RADEON_UPLOAD_MASKS;
   }
}

static void radeonDDAlphaFunc( GLcontext *ctx, GLenum func, GLclampf ref )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   FLUSH_BATCH( rmesa );
   rmesa->new_state |= RADEON_NEW_ALPHA;
}

static void radeonDDBlendEquation( GLcontext *ctx, GLenum mode )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   FLUSH_BATCH( rmesa );
   rmesa->new_state |= RADEON_NEW_ALPHA;

   if (ctx->Color.ColorLogicOpEnabled && ctx->Color.LogicOp != GL_COPY)
      rmesa->Fallback |= RADEON_FALLBACK_LOGICOP;
   else
      rmesa->Fallback &= ~RADEON_FALLBACK_LOGICOP;
}

static void radeonDDBlendFunc( GLcontext *ctx, GLenum sfactor, GLenum dfactor )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   FLUSH_BATCH( rmesa );
   rmesa->new_state |= RADEON_NEW_ALPHA;
}

static void radeonDDBlendFuncSeparate( GLcontext *ctx,
				       GLenum sfactorRGB, GLenum dfactorRGB,
				       GLenum sfactorA, GLenum dfactorA )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   FLUSH_BATCH( rmesa );
   rmesa->new_state |= RADEON_NEW_ALPHA;
}


/* =============================================================
 * Depth testing
 */

static void radeonUpdateZMode( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   GLuint z = rmesa->setup.rb3d_zstencilcntl;
   GLuint c = rmesa->setup.rb3d_cntl;

   if ( ctx->Depth.Test ) {
      z &= ~RADEON_Z_TEST_MASK;

      switch ( ctx->Depth.Func ) {
      case GL_NEVER:
	 z |= RADEON_Z_TEST_NEVER;
	 break;
      case GL_ALWAYS:
	 z |= RADEON_Z_TEST_ALWAYS;
	 break;
      case GL_LESS:
	 z |= RADEON_Z_TEST_LESS;
	 break;
      case GL_LEQUAL:
	 z |= RADEON_Z_TEST_LEQUAL;
	 break;
      case GL_EQUAL:
	 z |= RADEON_Z_TEST_EQUAL;
	 break;
      case GL_GEQUAL:
	 z |= RADEON_Z_TEST_GEQUAL;
	 break;
      case GL_GREATER:
	 z |= RADEON_Z_TEST_GREATER;
	 break;
      case GL_NOTEQUAL:
	 z |= RADEON_Z_TEST_NEQUAL;
	 break;
      }

      c |=  RADEON_Z_ENABLE;
   } else {
      c &= ~RADEON_Z_ENABLE;
   }

   if ( ctx->Depth.Mask ) {
      z |=  RADEON_Z_WRITE_ENABLE;
   } else {
      z &= ~RADEON_Z_WRITE_ENABLE;
   }

   if ( rmesa->setup.rb3d_zstencilcntl != z ) {
      rmesa->setup.rb3d_zstencilcntl = z;
      rmesa->dirty |= RADEON_UPLOAD_CONTEXT;
   }
   if ( rmesa->setup.rb3d_cntl != c ) {
      rmesa->setup.rb3d_cntl = c;
      rmesa->dirty |= RADEON_UPLOAD_CONTEXT;
   }
}

static void radeonDDDepthFunc( GLcontext *ctx, GLenum func )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   FLUSH_BATCH( rmesa );
   rmesa->new_state |= RADEON_NEW_DEPTH;
}

static void radeonDDDepthMask( GLcontext *ctx, GLboolean flag )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   FLUSH_BATCH( rmesa );
   rmesa->new_state |= RADEON_NEW_DEPTH;
}

static void radeonDDClearDepth( GLcontext *ctx, GLclampd d )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   switch ( rmesa->setup.rb3d_zstencilcntl & RADEON_DEPTH_FORMAT_MASK ) {
   case RADEON_DEPTH_FORMAT_16BIT_INT_Z:
      rmesa->ClearDepth = d * 0x0000ffff;
      break;
   case RADEON_DEPTH_FORMAT_24BIT_INT_Z:
      rmesa->ClearDepth = d * 0x00ffffff;
      break;
   }
}


/* =============================================================
 * Fog
 */

static void radeonUpdateFogAttrib( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   GLuint p = rmesa->setup.pp_cntl;
   GLubyte c[4];
   GLuint col;

   if ( ctx->FogMode == FOG_FRAGMENT ) {
      p |=  RADEON_FOG_ENABLE;
   } else {
      p &= ~RADEON_FOG_ENABLE;
   }

   FLOAT_RGB_TO_UBYTE_RGB( c, ctx->Fog.Color );
   col = radeonPackColor( 4, c[0], c[1], c[2], 0 );

   if ( rmesa->setup.pp_fog_color != col ) {
      rmesa->setup.pp_fog_color = col;
      rmesa->dirty |= RADEON_UPLOAD_CONTEXT;
   }
   if ( rmesa->setup.pp_cntl != p ) {
      rmesa->setup.pp_cntl = p;
      rmesa->dirty |= RADEON_UPLOAD_CONTEXT;
   }
}

static void radeonDDFogfv( GLcontext *ctx, GLenum pname, const GLfloat *param )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   FLUSH_BATCH( rmesa );
   rmesa->new_state |= RADEON_NEW_FOG;
}


/* =============================================================
 * Clipping
 */

static void radeonUpdateClipping( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   if ( rmesa->driDrawable ) {
      __DRIdrawablePrivate *drawable = rmesa->driDrawable;
      int x = 0;
      int y = 0;
      int w = drawable->w - 1;
      int h = drawable->h - 1;

      if ( ctx->Scissor.Enabled ) {
	 if ( ctx->Scissor.X > x ) {
	    x = ctx->Scissor.X;
	 }
	 if ( drawable->h - ctx->Scissor.Y - ctx->Scissor.Height > y ) {
	    y = drawable->h - ctx->Scissor.Y - ctx->Scissor.Height;
	 }
	 if ( ctx->Scissor.X + ctx->Scissor.Width - 1 < w ) {
	    w = ctx->Scissor.X + ctx->Scissor.Width - 1;
	 }
	 if ( drawable->h - ctx->Scissor.Y - 1 < h ) {
	    h = drawable->h - ctx->Scissor.Y - 1;
	 }
      }

      rmesa->scissor_rect.x1 = x + rmesa->driDrawable->x;
      rmesa->scissor_rect.y1 = y + rmesa->driDrawable->y;
      rmesa->scissor_rect.x2 = w + rmesa->driDrawable->x + 1;
      rmesa->scissor_rect.y2 = h + rmesa->driDrawable->y + 1;

      rmesa->dirty |= RADEON_UPLOAD_CLIPRECTS;
   }
}

static void radeonDDScissor( GLcontext *ctx,
			     GLint x, GLint y, GLsizei w, GLsizei h )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   FLUSH_BATCH( rmesa );
   rmesa->new_state |= RADEON_NEW_CLIP;
}


/* =============================================================
 * Culling
 */

static void radeonUpdateCull( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   GLuint s = rmesa->setup.se_cntl;

   s &= ~RADEON_FFACE_CULL_DIR_MASK;

   switch ( ctx->Polygon.FrontFace ) {
   case GL_CW:
      s |= RADEON_FFACE_CULL_CW;
      break;
   case GL_CCW:
      s |= RADEON_FFACE_CULL_CCW;
      break;
   }

   s |= RADEON_FFACE_SOLID | RADEON_BFACE_SOLID;

   if ( ctx->Polygon.CullFlag && ctx->PB->primitive == GL_POLYGON ) {
      switch ( ctx->Polygon.CullFaceMode ) {
      case GL_FRONT:
	 s &= ~RADEON_FFACE_SOLID;
	 break;
      case GL_BACK:
	 s &= ~RADEON_BFACE_SOLID;
	 break;
      case GL_FRONT_AND_BACK:
	 s &= ~(RADEON_FFACE_SOLID | RADEON_BFACE_SOLID);
	 break;
      }
   }

   if ( rmesa->setup.se_cntl != s ) {
      rmesa->setup.se_cntl = s;
      rmesa->dirty |= RADEON_UPLOAD_CONTEXT | RADEON_UPLOAD_SETUP;
   }
}

static void radeonDDCullFace( GLcontext *ctx, GLenum mode )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   FLUSH_BATCH( rmesa );
   rmesa->new_state |= RADEON_NEW_CULL;
}

static void radeonDDFrontFace( GLcontext *ctx, GLenum mode )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   FLUSH_BATCH( rmesa );
   rmesa->new_state |= RADEON_NEW_CULL;
}


/* =============================================================
 * Masks
 */

static void radeonUpdateMasks( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   GLuint mask = radeonPackColor( rmesa->radeonScreen->cpp,
				  ctx->Color.ColorMask[RCOMP],
				  ctx->Color.ColorMask[GCOMP],
				  ctx->Color.ColorMask[BCOMP],
				  ctx->Color.ColorMask[ACOMP] );

   if ( rmesa->setup.rb3d_planemask != mask ) {
      rmesa->setup.rb3d_planemask = mask;
      rmesa->dirty |=  RADEON_UPLOAD_CONTEXT | RADEON_UPLOAD_MASKS;
   }
}

static GLboolean radeonDDColorMask( GLcontext *ctx,
				    GLboolean r, GLboolean g,
				    GLboolean b, GLboolean a )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   FLUSH_BATCH( rmesa );
   rmesa->new_state |= RADEON_NEW_MASKS;

   return GL_FALSE; /* This forces the software paths to do colormasking. */
                    /* This function will return void when we use Mesa 3.5 */
}


/* =============================================================
 * Rendering attributes
 *
 * We really don't want to recalculate all this every time we bind a
 * texture.  These things shouldn't change all that often, so it makes
 * sense to break them out of the core texture state update routines.
 */

static void radeonDDLightModelfv( GLcontext *ctx, GLenum pname,
				  const GLfloat *param )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   if ( pname == GL_LIGHT_MODEL_COLOR_CONTROL ) {
      GLuint p = rmesa->setup.pp_cntl;

      FLUSH_BATCH( rmesa );

      if ( ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR &&
           ctx->Light.Enabled && ctx->Texture.ReallyEnabled) {
	 p |=  RADEON_SPECULAR_ENABLE;
      } else {
	 p &= ~RADEON_SPECULAR_ENABLE;
      }

      if ( rmesa->setup.pp_cntl != p ) {
	 rmesa->setup.pp_cntl = p;
	 rmesa->dirty |= RADEON_UPLOAD_CONTEXT;
      }
   }
}

static void radeonDDShadeModel( GLcontext *ctx, GLenum mode )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   GLuint s = rmesa->setup.se_cntl;

   s &= ~(RADEON_DIFFUSE_SHADE_MASK |
	  RADEON_ALPHA_SHADE_MASK |
	  RADEON_SPECULAR_SHADE_MASK |
	  RADEON_FOG_SHADE_MASK);

   switch ( mode ) {
   case GL_FLAT:
      s |= (RADEON_DIFFUSE_SHADE_FLAT |
	    RADEON_ALPHA_SHADE_FLAT |
	    RADEON_SPECULAR_SHADE_FLAT |
	    RADEON_FOG_SHADE_FLAT);
      break;
   case GL_SMOOTH:
      s |= (RADEON_DIFFUSE_SHADE_GOURAUD |
	    RADEON_ALPHA_SHADE_GOURAUD |
	    RADEON_SPECULAR_SHADE_GOURAUD |
	    RADEON_FOG_SHADE_GOURAUD);
      break;
   default:
      return;
   }

   if ( rmesa->setup.se_cntl != s ) {
      FLUSH_BATCH( rmesa );
      rmesa->setup.se_cntl = s;

      rmesa->new_state |= RADEON_NEW_CONTEXT;
      rmesa->dirty |= RADEON_UPLOAD_SETUP;
   }
}


/* =============================================================
 * Window position
 */

void radeonUpdateWindow( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   __DRIdrawablePrivate *dPriv = rmesa->driDrawable;
   GLfloat xoffset = (GLfloat)dPriv->x;
   GLfloat yoffset = (GLfloat)dPriv->y + dPriv->h;
   const GLfloat one = 1.0;
   GLuint m = rmesa->setup.re_misc;
   GLuint sx, sy;

   rmesa->setup.se_vport_xscale  = *(GLuint *)&one;
   rmesa->setup.se_vport_xoffset = *(GLuint *)&xoffset;
   rmesa->setup.se_vport_yscale  = *(GLuint *)&one;
   rmesa->setup.se_vport_yoffset = *(GLuint *)&yoffset;
   rmesa->setup.se_vport_zscale  = *(GLuint *)&rmesa->depth_scale;
   rmesa->setup.se_vport_zoffset = 0x00000000;

   /* Update polygon stipple offsets */
   m &= ~(RADEON_STIPPLE_X_OFFSET_MASK |
	  RADEON_STIPPLE_Y_OFFSET_MASK);

   /* add magic offsets, then invert */
   sx = 31 - ((rmesa->driDrawable->x - 1) & RADEON_STIPPLE_COORD_MASK);
   sy = 31 - ((rmesa->driDrawable->y + rmesa->driDrawable->h - 1)
              & RADEON_STIPPLE_COORD_MASK);

   m |= ((sx << RADEON_STIPPLE_X_OFFSET_SHIFT) |
	 (sy << RADEON_STIPPLE_Y_OFFSET_SHIFT));

   if ( rmesa->setup.re_misc != m ) {
      rmesa->setup.re_misc = m;
      rmesa->dirty |= RADEON_UPLOAD_MISC;
   }

   rmesa->dirty |= RADEON_UPLOAD_VIEWPORT;
}


/* =============================================================
 * Miscellaneous
 */

static void radeonDDClearColor( GLcontext *ctx,
				GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   rmesa->ClearColor = radeonPackColor( rmesa->radeonScreen->cpp,
					r, g, b, a );
}

static void radeonDDColor( GLcontext *ctx,
			   GLubyte r, GLubyte g, GLubyte b, GLubyte a )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   rmesa->Color = radeonPackColor( rmesa->radeonScreen->cpp,
				   r, g, b, a );
}

static void radeonDDLogicOpCode( GLcontext *ctx, GLenum opcode )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   if ( ctx->Color.ColorLogicOpEnabled ) {
      FLUSH_BATCH( rmesa );

      /* FIXME: We can do color logic ops.
       */
      if ( opcode == GL_COPY ) {
	 rmesa->Fallback &= ~RADEON_FALLBACK_LOGICOP;
      } else {
	 rmesa->Fallback |= RADEON_FALLBACK_LOGICOP;
      }
   }
   else
      rmesa->Fallback &= ~RADEON_FALLBACK_LOGICOP;
}

static GLboolean radeonDDSetDrawBuffer( GLcontext *ctx, GLenum mode )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   int found = GL_TRUE;

   FLUSH_BATCH( rmesa );

   if ( rmesa->DrawBuffer != mode ) {
      rmesa->DrawBuffer = mode;
      rmesa->Fallback &= ~RADEON_FALLBACK_DRAW_BUFFER;

      switch ( mode ) {
      case GL_FRONT_LEFT:
	 rmesa->drawOffset = rmesa->radeonScreen->frontOffset;
	 rmesa->drawPitch  = rmesa->radeonScreen->frontPitch;
	 break;
      case GL_BACK_LEFT:
	 rmesa->drawOffset = rmesa->radeonScreen->backOffset;
	 rmesa->drawPitch  = rmesa->radeonScreen->backPitch;
	 break;
      default:
	 rmesa->Fallback |= RADEON_FALLBACK_DRAW_BUFFER;
	 found = GL_FALSE;
	 break;
      }

      rmesa->setup.rb3d_coloroffset =
	 (rmesa->drawOffset & RADEON_COLOROFFSET_MASK);
      rmesa->setup.rb3d_colorpitch = rmesa->drawPitch;

      rmesa->new_state |= RADEON_NEW_WINDOW;
   }

   return found;
}

static void radeonDDSetReadBuffer( GLcontext *ctx,
				   GLframebuffer *colorBuffer,
				   GLenum mode )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   rmesa->Fallback &= ~RADEON_FALLBACK_READ_BUFFER;

   switch ( mode ) {
   case GL_FRONT_LEFT:
      rmesa->readOffset = rmesa->radeonScreen->frontOffset;
      rmesa->readPitch  = rmesa->radeonScreen->frontPitch;
      break;
   case GL_BACK_LEFT:
      rmesa->readOffset = rmesa->radeonScreen->backOffset;
      rmesa->readPitch  = rmesa->radeonScreen->backPitch;
      break;
   default:
      rmesa->Fallback |= RADEON_FALLBACK_READ_BUFFER;
      break;
   }
}


/* =============================================================
 * Polygon stipple
 */

static void radeonDDPolygonStipple( GLcontext *ctx, const GLubyte *mask )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   GLuint i, stipple[32];

   /* must flip pattern upside down */
   for (i = 0; i < 32; i++) {
      stipple[31 - i] = ((GLuint *) mask)[i];
   }

   FLUSH_BATCH( rmesa );

   if ( ctx->Polygon.StippleFlag && ctx->PB->primitive == GL_POLYGON ) {
      rmesa->setup.pp_cntl |=  RADEON_STIPPLE_ENABLE;
   } else {
      rmesa->setup.pp_cntl &= ~RADEON_STIPPLE_ENABLE;
   }

   LOCK_HARDWARE( rmesa );

   /* FIXME: Use window x,y offsets into stipple RAM.
    */
   drmRadeonPolygonStipple( rmesa->driFd, stipple );

   UNLOCK_HARDWARE( rmesa );

   rmesa->new_state |= RADEON_NEW_CONTEXT;
   rmesa->dirty |= RADEON_UPLOAD_CONTEXT;
}


/* =============================================================
 * State enable/disable
 */

static void radeonDDEnable( GLcontext *ctx, GLenum cap, GLboolean state )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   switch ( cap ) {
   case GL_ALPHA_TEST:
      FLUSH_BATCH( rmesa );
      rmesa->new_state |= RADEON_NEW_ALPHA;
      break;

   case GL_BLEND:
      FLUSH_BATCH( rmesa );
      rmesa->new_state |= RADEON_NEW_ALPHA;

      if (ctx->Color.ColorLogicOpEnabled && ctx->Color.LogicOp != GL_COPY)
	 rmesa->Fallback |= RADEON_FALLBACK_LOGICOP;
      else
	 rmesa->Fallback &= ~RADEON_FALLBACK_LOGICOP;
      break;

   case GL_CULL_FACE:
      FLUSH_BATCH( rmesa );
      rmesa->new_state |= RADEON_NEW_CULL;
      break;

   case GL_DEPTH_TEST:
      FLUSH_BATCH( rmesa );
      rmesa->new_state |= RADEON_NEW_DEPTH;
      break;

   case GL_DITHER:
      do {
	 GLuint r = rmesa->setup.rb3d_cntl;
	 FLUSH_BATCH( rmesa );

	 if ( ctx->Color.DitherFlag ) {
	    r |=  RADEON_DITHER_ENABLE;
	 } else {
	    r &= ~RADEON_DITHER_ENABLE;
	 }

	 if ( rmesa->setup.rb3d_cntl != r ) {
	    rmesa->setup.rb3d_cntl = r;
	    rmesa->dirty |= RADEON_UPLOAD_CONTEXT;
	 }
      } while (0);
      break;

   case GL_FOG:
      FLUSH_BATCH( rmesa );
      rmesa->new_state |= RADEON_NEW_FOG;
      break;

   case GL_COLOR_LOGIC_OP:
      FLUSH_BATCH( rmesa );
      if ( state && ctx->Color.LogicOp != GL_COPY ) {
	 rmesa->Fallback |= RADEON_FALLBACK_LOGICOP;
      } else {
	 rmesa->Fallback &= ~RADEON_FALLBACK_LOGICOP;
      }
      break;

   case GL_LIGHTING:
      {
         GLuint p = rmesa->setup.pp_cntl;
         if ( ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR &&
              ctx->Light.Enabled && ctx->Texture.ReallyEnabled) {
            p |=  RADEON_SPECULAR_ENABLE;
         } else {
            p &= ~RADEON_SPECULAR_ENABLE;
         }
         if ( rmesa->setup.pp_cntl != p ) {
            rmesa->setup.pp_cntl = p;
            rmesa->dirty |= RADEON_UPLOAD_CONTEXT;
         }
         break;
      }

   case GL_SCISSOR_TEST:
      FLUSH_BATCH( rmesa );
      rmesa->scissor = state;
      rmesa->new_state |= RADEON_NEW_CLIP;
      break;

   case GL_TEXTURE_1D:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_3D:
      FLUSH_BATCH( rmesa );
      rmesa->new_state |= RADEON_NEW_TEXTURE;
      break;

   case GL_POLYGON_STIPPLE:
      if ( ctx->PB->primitive == GL_POLYGON ) {
	 FLUSH_BATCH( rmesa );
	 if ( state ) {
	    rmesa->setup.pp_cntl |=  RADEON_STIPPLE_ENABLE;
	 } else {
	    rmesa->setup.pp_cntl &= ~RADEON_STIPPLE_ENABLE;
	 }
	 rmesa->new_state |= RADEON_NEW_CONTEXT;
	 rmesa->dirty |= RADEON_UPLOAD_CONTEXT;
      }
      break;

   default:
      return;
   }
}


/* =============================================================
 * State initialization, management
 */

static void radeonDDPrintDirty( const char *msg, GLuint state )
{
   fprintf( stderr,
	    "%s: (0x%x) %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
	    msg,
	    state,
	    (state & RADEON_UPLOAD_CONTEXT)     ? "context, " : "",
	    (state & RADEON_UPLOAD_VERTFMT)     ? "vertfmt, " : "",
	    (state & RADEON_UPLOAD_LINE)        ? "line, " : "",
	    (state & RADEON_UPLOAD_BUMPMAP)     ? "bumpmap, " : "",
	    (state & RADEON_UPLOAD_MASKS)       ? "masks, " : "",
	    (state & RADEON_UPLOAD_VIEWPORT)    ? "viewport, " : "",
	    (state & RADEON_UPLOAD_SETUP)       ? "setup, " : "",
	    (state & RADEON_UPLOAD_TCL)         ? "tcl, " : "",
	    (state & RADEON_UPLOAD_MISC)        ? "misc, " : "",
	    (state & RADEON_UPLOAD_TEX0)        ? "tex0, " : "",
	    (state & RADEON_UPLOAD_TEX1)        ? "tex1, " : "",
	    (state & RADEON_UPLOAD_TEX2)        ? "tex2, " : "",
	    (state & RADEON_UPLOAD_TEX0IMAGES)  ? "tex0 images, " : "",
	    (state & RADEON_UPLOAD_TEX1IMAGES)  ? "tex1 images, " : "",
	    (state & RADEON_UPLOAD_TEX2IMAGES)  ? "tex2 images, " : "",
	    (state & RADEON_UPLOAD_CLIPRECTS)   ? "cliprects, " : "",
	    (state & RADEON_REQUIRE_QUIESCENCE) ? "quiescence, " : "" );
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
void radeonEmitHwStateLocked( radeonContextPtr rmesa )
{
   RADEONSAREAPrivPtr sarea = rmesa->sarea;
   radeon_context_regs_t *regs = &(rmesa->setup);
   radeonTexObjPtr t0 = rmesa->CurrentTexObj[0];
   radeonTexObjPtr t1 = rmesa->CurrentTexObj[1];

   if ( RADEON_DEBUG & DEBUG_VERBOSE_MSG ) {
      radeonDDPrintDirty( "radeonEmitHwStateLocked", rmesa->dirty );
   }

   if ( (rmesa->dirty & RADEON_UPLOAD_TEX0IMAGES) && t0 ) {
      radeonUploadTexImages( rmesa, t0 );
      rmesa->dirty &= ~RADEON_UPLOAD_TEX0IMAGES;
   }
   if ( (rmesa->dirty & RADEON_UPLOAD_TEX1IMAGES) && t1 ) {
      radeonUploadTexImages( rmesa, t1 );
      rmesa->dirty &= ~RADEON_UPLOAD_TEX1IMAGES;
   }
   if ( rmesa->dirty & RADEON_UPLOAD_TEX2IMAGES ) {
      /* FIXME: Enable the third texture unit... */
      rmesa->dirty &= ~RADEON_UPLOAD_TEX2IMAGES;
   }

   if ( rmesa->dirty & (RADEON_UPLOAD_CONTEXT |
			RADEON_UPLOAD_VERTFMT |
			RADEON_UPLOAD_LINE |
			RADEON_UPLOAD_BUMPMAP |
			RADEON_UPLOAD_MASKS |
			RADEON_UPLOAD_VIEWPORT |
			RADEON_UPLOAD_SETUP |
			RADEON_UPLOAD_TCL |
			RADEON_UPLOAD_MISC) ) {
      memcpy( &sarea->ContextState, regs, sizeof(sarea->ContextState) );
   }

   /* Assemble the texture state, combining the texture object and
    * texture environment state into the hardware texture unit state.
    */
   if ( (rmesa->dirty & RADEON_UPLOAD_TEX0) && t0 ) {
      radeon_texture_regs_t *tex = &sarea->TexState[0];

      tex->pp_txfilter = t0->pp_txfilter | rmesa->lod_bias[0] << 8;
      tex->pp_txformat = t0->pp_txformat | RADEON_TXFORMAT_ST_ROUTE_STQ0;
      tex->pp_txoffset = t0->pp_txoffset;

      tex->pp_txcblend = rmesa->color_combine[0];
      tex->pp_txablend = rmesa->alpha_combine[0];
      tex->pp_tfactor = rmesa->env_color[0];

      tex->pp_border_color = t0->pp_border_color;
   }

   if ( (rmesa->dirty & RADEON_UPLOAD_TEX1) && t1 ) {
      radeon_texture_regs_t *tex = &sarea->TexState[1];

      tex->pp_txfilter = t1->pp_txfilter | rmesa->lod_bias[1] << 8;
      tex->pp_txformat = t1->pp_txformat | RADEON_TXFORMAT_ST_ROUTE_STQ1;
      tex->pp_txoffset = t1->pp_txoffset;

      tex->pp_txcblend = rmesa->color_combine[1];
      tex->pp_txablend = rmesa->alpha_combine[1];
      tex->pp_tfactor = rmesa->env_color[1];

      tex->pp_border_color = t1->pp_border_color;
   }

   if ( rmesa->dirty & RADEON_UPLOAD_TEX2 ) {
      /* FIXME: Enable the third texture unit... */
      memset( &sarea->TexState[2], 0, sizeof(sarea->TexState[2]) );
   }

   sarea->vertsize = rmesa->vertsize;
   sarea->vc_format = rmesa->vc_format;

   sarea->dirty |= rmesa->dirty;
   rmesa->dirty &= RADEON_UPLOAD_CLIPRECTS;
}

static void radeonDDPrintState( const char *msg, GLuint flags )
{
   fprintf( stderr,
	    "%s: (0x%x) %s%s%s%s%s%s%s%s%s\n",
	    msg,
	    flags,
	    (flags & RADEON_NEW_CONTEXT)	? "context, " : "",
	    (flags & RADEON_NEW_ALPHA)		? "alpha, " : "",
	    (flags & RADEON_NEW_DEPTH)		? "depth, " : "",
	    (flags & RADEON_NEW_FOG)		? "fog, " : "",
	    (flags & RADEON_NEW_CLIP)		? "clip, " : "",
	    (flags & RADEON_NEW_TEXTURE)	? "texture, " : "",
	    (flags & RADEON_NEW_CULL)		? "cull, " : "",
	    (flags & RADEON_NEW_MASKS)		? "masks, " : "",
	    (flags & RADEON_NEW_WINDOW)		? "window, " : "" );
}

void radeonDDUpdateHWState( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   int new_state = rmesa->new_state;

   if ( new_state ) {
      FLUSH_BATCH( rmesa );

      rmesa->new_state = 0;

      if ( RADEON_DEBUG & DEBUG_VERBOSE_MSG )
	 radeonDDPrintState( "radeonUpdateHwState", new_state );

      /* Update the various parts of the context's state.
       */
      if ( new_state & RADEON_NEW_ALPHA )
	 radeonUpdateAlphaMode( ctx );

      if ( new_state & RADEON_NEW_DEPTH )
	 radeonUpdateZMode( ctx );

      if ( new_state & RADEON_NEW_FOG )
	 radeonUpdateFogAttrib( ctx );

      if ( new_state & RADEON_NEW_CLIP )
	 radeonUpdateClipping( ctx );

      if ( new_state & RADEON_NEW_CULL )
	 radeonUpdateCull( ctx );

      if ( new_state & RADEON_NEW_MASKS )
	 radeonUpdateMasks( ctx );

      if ( new_state & RADEON_NEW_WINDOW )
	 radeonUpdateWindow( ctx );

      if ( new_state & RADEON_NEW_TEXTURE )
	 radeonUpdateTextureState( ctx );
   }
}

/* This is called when Mesa switches between rendering triangle
 * primitives (such as GL_POLYGON, GL_QUADS, GL_TRIANGLE_STRIP, etc),
 * and lines, points and bitmaps.
 *
 * As the radeon uses triangles to render lines and points, it is
 * necessary to turn off hardware culling when rendering these
 * primitives.
 */
static void radeonDDReducedPrimitiveChange( GLcontext *ctx, GLenum prim )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);
   GLuint s = rmesa->setup.se_cntl;

   s |= RADEON_FFACE_SOLID | RADEON_BFACE_SOLID;

   if ( ctx->Polygon.CullFlag && ctx->PB->primitive == GL_POLYGON ) {
      switch ( ctx->Polygon.CullFaceMode ) {
      case GL_FRONT:
	 s &= ~RADEON_FFACE_SOLID;
	 break;
      case GL_BACK:
	 s &= ~RADEON_BFACE_SOLID;
	 break;
      case GL_FRONT_AND_BACK:
	 s &= ~(RADEON_FFACE_SOLID | RADEON_BFACE_SOLID);
	 break;
      }
   }

   if ( rmesa->setup.se_cntl != s ) {
      FLUSH_BATCH( rmesa );
      rmesa->setup.se_cntl = s;

      /* NOTE: Only upload the setup state, everything else has been
       * uploaded by the usual means already.  Also, note that this is
       * an optimization (see comment in the kernel's radeon_state.c),
       * which will not be necessary when/if we use the Radeon's
       * native point/line support.
       */
      rmesa->dirty |= RADEON_UPLOAD_SETUP;
   }
}


#define INTERESTED (~(NEW_MODELVIEW |		\
		      NEW_PROJECTION |		\
		      NEW_TEXTURE_MATRIX |	\
		      NEW_USER_CLIP |		\
		      NEW_CLIENT_STATE))

void radeonDDUpdateState( GLcontext *ctx )
{
   radeonContextPtr rmesa = RADEON_CONTEXT(ctx);

   if ( ctx->NewState & INTERESTED ) {
      radeonDDChooseRenderState( ctx );
      radeonDDChooseRasterSetupFunc( ctx );
   }

   /* Need to do this here to detect texture fallbacks before
    * setting triangle functions.
    * GH: Do we need this anymore?  The Radeon doesn't really have
    * texturing fallbacks like the r128...
    */
   if ( rmesa->new_state & RADEON_NEW_TEXTURE ) {
      radeonDDUpdateHWState( ctx );
   }

   if ( !rmesa->Fallback ) {
      ctx->IndirectTriangles &= ~DD_SW_RASTERIZE;
      ctx->IndirectTriangles |= rmesa->IndirectTriangles;

      ctx->Driver.PointsFunc	= rmesa->PointsFunc;
      ctx->Driver.LineFunc	= rmesa->LineFunc;
      ctx->Driver.TriangleFunc	= rmesa->TriangleFunc;
      ctx->Driver.QuadFunc	= rmesa->QuadFunc;
   }
}


/* Initialize the context's hardware state.
 */
void radeonDDInitState( radeonContextPtr rmesa )
{
   GLuint color_fmt, depth_fmt;

   switch ( rmesa->radeonScreen->cpp ) {
   case 2:
      color_fmt = RADEON_COLOR_FORMAT_RGB565;
      break;
   case 4:
      color_fmt = RADEON_COLOR_FORMAT_ARGB8888;
      break;
   default:
      fprintf( stderr, "Error: Unsupported pixel depth... exiting\n" );
      exit( -1 );
   }

   rmesa->ClearColor = 0x00000000;

   switch ( rmesa->glCtx->Visual->DepthBits ) {
   case 16:
      rmesa->ClearDepth = 0x0000ffff;
      rmesa->DepthMask= 0xffffffff;
      depth_fmt = RADEON_DEPTH_FORMAT_16BIT_INT_Z;
      rmesa->depth_scale = 1.0 / (GLfloat)0xffff;
      break;
   case 24:
      rmesa->ClearDepth = 0x00ffffff;
      rmesa->DepthMask= 0x00ffffff;
      depth_fmt = RADEON_DEPTH_FORMAT_24BIT_INT_Z;
      rmesa->depth_scale = 1.0 / (GLfloat)0xffffff;
      break;
   default:
      fprintf( stderr, "Error: Unsupported depth %d... exiting\n",
	       rmesa->glCtx->Visual->DepthBits );
      exit( -1 );
   }

   rmesa->RenderIndex	= RADEON_FALLBACK_BIT;
   rmesa->PointsFunc	= NULL;
   rmesa->LineFunc	= NULL;
   rmesa->TriangleFunc	= NULL;
   rmesa->QuadFunc	= NULL;

   rmesa->IndirectTriangles = 0;
   rmesa->Fallback = 0;

   if ( rmesa->glCtx->Visual->DBflag ) {
      rmesa->DrawBuffer = GL_BACK_LEFT;
      rmesa->drawOffset = rmesa->readOffset = rmesa->radeonScreen->backOffset;
      rmesa->drawPitch  = rmesa->readPitch  = rmesa->radeonScreen->backPitch;
   } else {
      rmesa->DrawBuffer = GL_FRONT_LEFT;
      rmesa->drawOffset = rmesa->readOffset = rmesa->radeonScreen->frontOffset;
      rmesa->drawPitch  = rmesa->readPitch  = rmesa->radeonScreen->frontPitch;
   }

   /* Harware state:
    */
   rmesa->setup.pp_misc = (RADEON_ALPHA_TEST_PASS |
			   RADEON_CHROMA_FUNC_FAIL |
			   RADEON_CHROMA_KEY_NEAREST |
			   RADEON_SHADOW_FUNC_EQUAL |
			   RADEON_SHADOW_PASS_1 |
			   RADEON_RIGHT_HAND_CUBE_OGL);

   rmesa->setup.pp_fog_color = ((0x00000000 & RADEON_FOG_COLOR_MASK) |
				RADEON_FOG_VERTEX |
				RADEON_FOG_USE_DEPTH);

   rmesa->setup.re_solid_color = 0x00000000;

   rmesa->setup.rb3d_blendcntl = (RADEON_SRC_BLEND_GL_ONE |
				  RADEON_DST_BLEND_GL_ZERO );

   rmesa->setup.rb3d_depthoffset = rmesa->radeonScreen->depthOffset;

   rmesa->setup.rb3d_depthpitch = ((rmesa->radeonScreen->depthPitch &
				    RADEON_DEPTHPITCH_MASK) |
				   RADEON_DEPTH_ENDIAN_NO_SWAP);

   rmesa->setup.rb3d_zstencilcntl = (depth_fmt |
				     RADEON_Z_TEST_LESS |
				     RADEON_STENCIL_TEST_ALWAYS |
				     RADEON_STENCIL_S_FAIL_KEEP |
				     RADEON_STENCIL_ZPASS_KEEP |
				     RADEON_STENCIL_ZFAIL_KEEP |
				     RADEON_Z_WRITE_ENABLE);

   rmesa->setup.pp_cntl = (RADEON_SCISSOR_ENABLE |
			   RADEON_ANTI_ALIAS_NONE);

   rmesa->setup.rb3d_cntl = (RADEON_PLANE_MASK_ENABLE |
			     color_fmt |
			     RADEON_ZBLOCK16);

   rmesa->setup.rb3d_coloroffset = (rmesa->drawOffset &
				    RADEON_COLOROFFSET_MASK);

   rmesa->setup.re_width_height = ((0x7ff << RADEON_RE_WIDTH_SHIFT) |
				   (0x7ff << RADEON_RE_HEIGHT_SHIFT));

   rmesa->setup.rb3d_colorpitch = ((rmesa->drawPitch &
				    RADEON_COLORPITCH_MASK) |
				   RADEON_COLOR_ENDIAN_NO_SWAP);

   rmesa->setup.se_cntl = (RADEON_FFACE_CULL_CW |
			   RADEON_BFACE_SOLID |
			   RADEON_FFACE_SOLID |
			   RADEON_FLAT_SHADE_VTX_LAST |
			   RADEON_DIFFUSE_SHADE_GOURAUD |
			   RADEON_ALPHA_SHADE_GOURAUD |
			   RADEON_SPECULAR_SHADE_GOURAUD |
			   RADEON_FOG_SHADE_GOURAUD |
			   RADEON_VPORT_XY_XFORM_ENABLE |
			   RADEON_VPORT_Z_XFORM_ENABLE |
			   RADEON_VTX_PIX_CENTER_OGL |
			   RADEON_ROUND_MODE_TRUNC |
			   RADEON_ROUND_PREC_8TH_PIX);

   rmesa->setup.se_coord_fmt = (RADEON_VTX_XY_PRE_MULT_1_OVER_W0 |
				RADEON_VTX_Z_PRE_MULT_1_OVER_W0 |
				RADEON_TEX1_W_ROUTING_USE_Q1);

   rmesa->setup.re_line_pattern = ((0x0000 & RADEON_LINE_PATTERN_MASK) |
				   (0 << RADEON_LINE_REPEAT_COUNT_SHIFT) |
				   (0 << RADEON_LINE_PATTERN_START_SHIFT) |
				   RADEON_LINE_PATTERN_LITTLE_BIT_ORDER);

   rmesa->setup.re_line_state = ((0 << RADEON_LINE_CURRENT_PTR_SHIFT) |
				 (0 << RADEON_LINE_CURRENT_COUNT_SHIFT));

   rmesa->setup.se_line_width = 0x0000000;

   rmesa->setup.pp_lum_matrix = 0x00000000;

   rmesa->setup.pp_rot_matrix_0 = 0x00000000;
   rmesa->setup.pp_rot_matrix_1 = 0x00000000;

   rmesa->setup.rb3d_stencilrefmask = ((0x00 << RADEON_STENCIL_REF_SHIFT) |
				       (0xff << RADEON_STENCIL_MASK_SHIFT) |
				       (0xff << RADEON_STENCIL_WRITEMASK_SHIFT));

   rmesa->setup.rb3d_ropcntl   = 0x00000000;
   rmesa->setup.rb3d_planemask = 0xffffffff;

   rmesa->setup.se_vport_xscale  = 0x00000000;
   rmesa->setup.se_vport_xoffset = 0x00000000;
   rmesa->setup.se_vport_yscale  = 0x00000000;
   rmesa->setup.se_vport_yoffset = 0x00000000;
   rmesa->setup.se_vport_zscale  = 0x00000000;
   rmesa->setup.se_vport_zoffset = 0x00000000;

   rmesa->setup.se_cntl_status = (RADEON_VC_NO_SWAP |
				  RADEON_TCL_BYPASS);

#ifdef TCL_ENABLE
   /* FIXME: Obviously these need to be properly initialized */
   rmesa->setup.se_tcl_material_emmissive.red	= 0x00000000;
   rmesa->setup.se_tcl_material_emmissive.green	= 0x00000000;
   rmesa->setup.se_tcl_material_emmissive.blue	= 0x00000000;
   rmesa->setup.se_tcl_material_emmissive.alpha	= 0x00000000;

   rmesa->setup.se_tcl_material_ambient.red	= 0x00000000;
   rmesa->setup.se_tcl_material_ambient.green	= 0x00000000;
   rmesa->setup.se_tcl_material_ambient.blue	= 0x00000000;
   rmesa->setup.se_tcl_material_ambient.alpha	= 0x00000000;

   rmesa->setup.se_tcl_material_diffuse.red	= 0x00000000;
   rmesa->setup.se_tcl_material_diffuse.green	= 0x00000000;
   rmesa->setup.se_tcl_material_diffuse.blue	= 0x00000000;
   rmesa->setup.se_tcl_material_diffuse.alpha	= 0x00000000;

   rmesa->setup.se_tcl_material_specular.red	= 0x00000000;
   rmesa->setup.se_tcl_material_specular.green	= 0x00000000;
   rmesa->setup.se_tcl_material_specular.blue	= 0x00000000;
   rmesa->setup.se_tcl_material_specular.alpha	= 0x00000000;

   rmesa->setup.se_tcl_shininess		= 0x00000000;
   rmesa->setup.se_tcl_output_vtx_fmt		= 0x00000000;
   rmesa->setup.se_tcl_output_vtx_sel		= 0x00000000;
   rmesa->setup.se_tcl_matrix_select_0		= 0x00000000;
   rmesa->setup.se_tcl_matrix_select_1		= 0x00000000;
   rmesa->setup.se_tcl_ucp_vert_blend_ctl	= 0x00000000;
   rmesa->setup.se_tcl_texture_proc_ctl		= 0x00000000;
   rmesa->setup.se_tcl_light_model_ctl		= 0x00000000;
   for ( i = 0 ; i < 4 ; i++ ) {
      rmesa->setup.se_tcl_per_light_ctl[i]	= 0x00000000;
   }
#endif

   rmesa->setup.re_top_left = ((0 << RADEON_RE_LEFT_SHIFT) |
			       (0 << RADEON_RE_TOP_SHIFT) );

   rmesa->setup.re_misc = ((0 << RADEON_STIPPLE_X_OFFSET_SHIFT) |
			   (0 << RADEON_STIPPLE_Y_OFFSET_SHIFT) |
			   RADEON_STIPPLE_BIG_BIT_ORDER);

   rmesa->env_color[0] = 0x00000000;
   rmesa->env_color[1] = 0x00000000;
   rmesa->env_color[2] = 0x00000000;

   rmesa->new_state = RADEON_NEW_ALL;
}

/* Initialize the driver's state functions.
 */
void radeonDDInitStateFuncs( GLcontext *ctx )
{
   ctx->Driver.UpdateState		= radeonDDUpdateState;

   ctx->Driver.ClearIndex		= NULL;
   ctx->Driver.ClearColor		= radeonDDClearColor;
   ctx->Driver.Index			= NULL;
   ctx->Driver.Color			= radeonDDColor;
   ctx->Driver.SetDrawBuffer		= radeonDDSetDrawBuffer;
   ctx->Driver.SetReadBuffer		= radeonDDSetReadBuffer;

   ctx->Driver.IndexMask		= NULL;
   ctx->Driver.ColorMask		= radeonDDColorMask;
   ctx->Driver.LogicOp			= NULL;
   ctx->Driver.Dither			= NULL;

   ctx->Driver.NearFar			= NULL;

   ctx->Driver.RenderStart		= radeonDDUpdateHWState;
   ctx->Driver.RenderFinish		= NULL;
   ctx->Driver.RasterSetup		= NULL;

   ctx->Driver.RenderVBClippedTab	= NULL;
   ctx->Driver.RenderVBCulledTab	= NULL;
   ctx->Driver.RenderVBRawTab		= NULL;

   ctx->Driver.ReducedPrimitiveChange	= radeonDDReducedPrimitiveChange;
   ctx->Driver.MultipassFunc		= NULL;

   ctx->Driver.AlphaFunc		= radeonDDAlphaFunc;
   ctx->Driver.BlendEquation		= radeonDDBlendEquation;
   ctx->Driver.BlendFunc		= radeonDDBlendFunc;
   ctx->Driver.BlendFuncSeparate	= radeonDDBlendFuncSeparate;
   ctx->Driver.ClearDepth		= radeonDDClearDepth;
   ctx->Driver.CullFace			= radeonDDCullFace;
   ctx->Driver.FrontFace		= radeonDDFrontFace;
   ctx->Driver.DepthFunc		= radeonDDDepthFunc;
   ctx->Driver.DepthMask		= radeonDDDepthMask;
   ctx->Driver.DepthRange		= NULL;
   ctx->Driver.Enable			= radeonDDEnable;
   ctx->Driver.Fogfv			= radeonDDFogfv;
   ctx->Driver.Hint			= NULL;
   ctx->Driver.Lightfv			= NULL;
   ctx->Driver.LightModelfv		= radeonDDLightModelfv;
   ctx->Driver.LogicOpcode		= radeonDDLogicOpCode;
   ctx->Driver.PolygonMode		= NULL;
   ctx->Driver.PolygonStipple		= radeonDDPolygonStipple;
   ctx->Driver.Scissor			= radeonDDScissor;
   ctx->Driver.ShadeModel		= radeonDDShadeModel;
   ctx->Driver.ClearStencil		= NULL;
   ctx->Driver.StencilFunc		= NULL;
   ctx->Driver.StencilMask		= NULL;
   ctx->Driver.StencilOp		= NULL;
   ctx->Driver.Viewport			= NULL;
}
