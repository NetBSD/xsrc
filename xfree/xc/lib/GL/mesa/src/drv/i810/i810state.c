/* $XFree86: xc/lib/GL/mesa/src/drv/i810/i810state.c,v 1.6 2001/03/21 16:14:21 dawes Exp $ */

#include <stdio.h>

#include "types.h"
#include "enums.h"
#include "pb.h"
#include "dd.h"

#include "mm.h"
#include "i810dd.h"
#include "i810context.h"
#include "i810state.h"
#include "i810tex.h"
#include "i810log.h"
#include "i810vb.h"
#include "i810tris.h"
#include "i810ioctl.h"

  

static __inline__ GLuint i810PackColor(GLuint format, 
				       GLubyte r, GLubyte g, 
				       GLubyte b, GLubyte a)
{
   switch (format) {
   case DV_PF_555:
      return I810PACKCOLOR1555(r,g,b,a);
   case DV_PF_565:
      return I810PACKCOLOR565(r,g,b);
   default:
      fprintf(stderr, "unknown format %d\n", (int)format);
      return 0;
   }
}


static void i810DDAlphaFunc(GLcontext *ctx, GLenum func, GLclampf ref)
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);
   CARD32 a = (ZA_UPDATE_ALPHAFUNC|ZA_UPDATE_ALPHAREF);

   FLUSH_BATCH(imesa);

   switch (ctx->Color.AlphaFunc) {
   case GL_NEVER:    a |= ZA_ALPHA_NEVER;    break;
   case GL_LESS:     a |= ZA_ALPHA_LESS;     break;
   case GL_GEQUAL:   a |= ZA_ALPHA_GEQUAL;   break;
   case GL_LEQUAL:   a |= ZA_ALPHA_LEQUAL;   break;
   case GL_GREATER:  a |= ZA_ALPHA_GREATER;  break;
   case GL_NOTEQUAL: a |= ZA_ALPHA_NOTEQUAL; break;
   case GL_EQUAL:    a |= ZA_ALPHA_EQUAL;    break;
   case GL_ALWAYS:   a |= ZA_ALPHA_ALWAYS;   break;
   default: return;
   }

   a |= ctx->Color.AlphaRef << ZA_ALPHAREF_SHIFT;

   imesa->dirty |= I810_UPLOAD_CTX;
   imesa->Setup[I810_CTXREG_ZA] &= ~(ZA_ALPHA_MASK|ZA_ALPHAREF_MASK);
   imesa->Setup[I810_CTXREG_ZA] |= a;
}

static void i810DDBlendEquation(GLcontext *ctx, GLenum mode) 
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);

   if (mode != GL_FUNC_ADD_EXT) {
      ctx->Color.BlendEquation = GL_FUNC_ADD_EXT;
      if (0) fprintf(stderr, "Unsupported blend equation: %s\n", 
		     gl_lookup_enum_by_nr(mode));
   }

   if (ctx->Color.ColorLogicOpEnabled && ctx->Color.LogicOp != GL_COPY)
      imesa->Fallback |= I810_FALLBACK_LOGICOP;
   else
      imesa->Fallback &= ~I810_FALLBACK_LOGICOP;
}

static void i810DDBlendFunc(GLcontext *ctx, GLenum sfactor, GLenum dfactor)
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);
   GLuint a = SDM_UPDATE_SRC_BLEND | SDM_UPDATE_DST_BLEND;
   FLUSH_BATCH(imesa);

   switch (ctx->Color.BlendSrcRGB) {
   case GL_ZERO:                a |= SDM_SRC_ZERO; break;
   case GL_SRC_ALPHA:           a |= SDM_SRC_SRC_ALPHA; break;
   case GL_ONE:                 a |= SDM_SRC_ONE; break;
   case GL_DST_COLOR:           a |= SDM_SRC_DST_COLOR; break;
   case GL_ONE_MINUS_DST_COLOR: a |= SDM_SRC_INV_DST_COLOR; break;
   case GL_ONE_MINUS_SRC_ALPHA: a |= SDM_SRC_INV_SRC_ALPHA; break;
   case GL_DST_ALPHA:           a |= SDM_SRC_ONE; break;
   case GL_ONE_MINUS_DST_ALPHA: a |= SDM_SRC_ZERO; break;
   case GL_SRC_ALPHA_SATURATE:  a |= SDM_SRC_SRC_ALPHA; break;
   default: return;
   }

   switch (ctx->Color.BlendDstRGB) {
   case GL_SRC_ALPHA:           a |= SDM_DST_SRC_ALPHA; break;
   case GL_ONE_MINUS_SRC_ALPHA: a |= SDM_DST_INV_SRC_ALPHA; break;
   case GL_ZERO:                a |= SDM_DST_ZERO; break;
   case GL_ONE:                 a |= SDM_DST_ONE; break;
   case GL_SRC_COLOR:           a |= SDM_DST_SRC_COLOR; break;
   case GL_ONE_MINUS_SRC_COLOR: a |= SDM_DST_INV_SRC_COLOR; break;
   case GL_DST_ALPHA:           a |= SDM_DST_ONE; break;
   case GL_ONE_MINUS_DST_ALPHA: a |= SDM_DST_ZERO; break;
   default: return;      
   }  

   imesa->dirty |= I810_UPLOAD_CTX;
   imesa->Setup[I810_CTXREG_SDM] &= ~(SDM_SRC_MASK|SDM_DST_MASK);
   imesa->Setup[I810_CTXREG_SDM] |= a;
}


/* Shouldn't be called as the extension is disabled.
 */
static void i810DDBlendFuncSeparate( GLcontext *ctx, GLenum sfactorRGB, 
				     GLenum dfactorRGB, GLenum sfactorA,
				     GLenum dfactorA )
{
   if (dfactorRGB != dfactorA || sfactorRGB != sfactorA) {
      gl_error( ctx, GL_INVALID_OPERATION, "glBlendEquation (disabled)");
   }

   i810DDBlendFunc( ctx, sfactorRGB, dfactorRGB );
}



static void i810DDDepthFunc(GLcontext *ctx, GLenum func)
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);
   int zmode;

   FLUSH_BATCH(imesa);

   switch(func)  { 
   case GL_NEVER: zmode = LCS_Z_NEVER; break;
   case GL_ALWAYS: zmode = LCS_Z_ALWAYS; break;
   case GL_LESS: zmode = LCS_Z_LESS; break; 
   case GL_LEQUAL: zmode = LCS_Z_LEQUAL; break;
   case GL_EQUAL: zmode = LCS_Z_EQUAL; break;
   case GL_GREATER: zmode = LCS_Z_GREATER; break;
   case GL_GEQUAL: zmode = LCS_Z_GEQUAL; break;
   case GL_NOTEQUAL: zmode = LCS_Z_NOTEQUAL; break;
   default: return;
   }
   
   imesa->Setup[I810_CTXREG_LCS] &= ~LCS_Z_MASK;
   imesa->Setup[I810_CTXREG_LCS] |= LCS_UPDATE_ZMODE | zmode;   
   imesa->dirty |= I810_UPLOAD_CTX;
}

static void i810DDDepthMask(GLcontext *ctx, GLboolean flag)
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);

   FLUSH_BATCH(imesa);

   imesa->dirty |= I810_UPLOAD_CTX;
   imesa->Setup[I810_CTXREG_B2] &= ~B2_ZB_WRITE_ENABLE;

   if (flag)
     imesa->Setup[I810_CTXREG_B2] |= B2_ZB_WRITE_ENABLE;
}


/* =============================================================
 * Polygon stipple 
 * 
 * The i810 supports a 4x4 stipple natively, GL wants 32x32. 
 * Fortunately stipple is usually a repeating pattern.  Could
 * also consider using a multitexturing mechanism for this, but
 * that has real issues, too.
 */
static void i810DDPolygonStipple( GLcontext *ctx, const GLubyte *mask )
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);
   const GLubyte *m = mask;
   GLubyte p[4];
   int i,j,k;
   int active = (ctx->Polygon.StippleFlag && ctx->PB->primitive == GL_POLYGON);
   GLuint newMask;

   FLUSH_BATCH(imesa);
   ctx->Driver.TriangleCaps |= DD_TRI_STIPPLE;

   if (active) {
      imesa->dirty |= I810_UPLOAD_CTX;
      imesa->Setup[I810_CTXREG_ST1] &= ~ST1_ENABLE;
   }

   p[0] = mask[12] & 0xf; p[0] |= p[0] << 4;
   p[1] = mask[8] & 0xf; p[1] |= p[1] << 4;
   p[2] = mask[4] & 0xf; p[2] |= p[2] << 4;
   p[3] = mask[0] & 0xf; p[3] |= p[3] << 4;

   for (k = 0 ; k < 8 ; k++)
      for (j = 0 ; j < 4; j++) 
	 for (i = 0 ; i < 4 ; i++) 
	    if (*m++ != p[j]) {
	       ctx->Driver.TriangleCaps &= ~DD_TRI_STIPPLE;
	       return;
	    }

   newMask = ((p[0] & 0xf) << 0) |
             ((p[1] & 0xf) << 4) |
             ((p[2] & 0xf) << 8) |
             ((p[3] & 0xf) << 12);
   if (newMask == 0xffff) {
      /* do opaque stipple in software for conformance */
      ctx->Driver.TriangleCaps &= ~DD_TRI_STIPPLE;
      return;
   }

   imesa->Setup[I810_CTXREG_ST1] &= ~0xffff;
   imesa->Setup[I810_CTXREG_ST1] |= newMask;

   if (active)
      imesa->Setup[I810_CTXREG_ST1] |= ST1_ENABLE;
}



/* =============================================================
 * Hardware clipping
 */


static void i810DDScissor( GLcontext *ctx, GLint x, GLint y, 
			   GLsizei w, GLsizei h )
{ 
   i810ContextPtr imesa = I810_CONTEXT(ctx);

   FLUSH_BATCH(imesa);
   imesa->scissor_rect.x1 = x;
   imesa->scissor_rect.y1 = imesa->driDrawable->h - (y+h);
   imesa->scissor_rect.x2 = x+w;
   imesa->scissor_rect.y2 = imesa->driDrawable->h - y;


   if (I810_DEBUG&DEBUG_VERBOSE_2D) 
      fprintf(stderr, "SET SCISSOR %d,%d-%d,%d\n", 
	      imesa->scissor_rect.x1,
	      imesa->scissor_rect.y1,
	      imesa->scissor_rect.x2,
	      imesa->scissor_rect.y2);


   imesa->dirty |= I810_UPLOAD_CLIPRECTS;
}


static void i810DDDither(GLcontext *ctx, GLboolean enable)
{
}

static void i810DDLogicOp( GLcontext *ctx, GLenum opcode )
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);

   if (ctx->Color.ColorLogicOpEnabled) 
   {
      FLUSH_BATCH( imesa );
   
      if (opcode == GL_COPY)
	 imesa->Fallback &= ~I810_FALLBACK_LOGICOP;
      else
	 imesa->Fallback |= I810_FALLBACK_LOGICOP;
   }
   else 
      imesa->Fallback &= ~I810_FALLBACK_LOGICOP;
}

static GLboolean i810DDSetDrawBuffer(GLcontext *ctx, GLenum mode )
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);

   FLUSH_BATCH(imesa);

   imesa->Fallback &= ~I810_FALLBACK_DRAW_BUFFER;

   if (mode == GL_FRONT_LEFT) 
   {
      imesa->drawMap = (char *)imesa->driScreen->pFB;
      imesa->readMap = (char *)imesa->driScreen->pFB;
      imesa->BufferSetup[I810_DESTREG_DI1] = (imesa->i810Screen->fbOffset | 
					      imesa->i810Screen->backPitchBits);
      imesa->dirty |= I810_UPLOAD_BUFFERS;
      i810XMesaSetFrontClipRects( imesa );
      return GL_TRUE;
   } 
   else if (mode == GL_BACK_LEFT) 
   {
      imesa->drawMap = imesa->i810Screen->back.map;
      imesa->readMap = imesa->i810Screen->back.map;
      imesa->BufferSetup[I810_DESTREG_DI1] = (imesa->i810Screen->backOffset | 
					      imesa->i810Screen->backPitchBits);
      imesa->dirty |= I810_UPLOAD_BUFFERS;
      i810XMesaSetBackClipRects( imesa );
      return GL_TRUE;
   }

   imesa->Fallback |= I810_FALLBACK_DRAW_BUFFER;
   return GL_FALSE;
}

static void i810DDSetReadBuffer(GLcontext *ctx, GLframebuffer *colorBuffer,
				GLenum mode )
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);

   if (mode == GL_FRONT_LEFT) 
   {
      imesa->readMap = (char *)imesa->driScreen->pFB;
      imesa->Fallback &= ~I810_FALLBACK_READ_BUFFER;
   } 
   else if (mode == GL_BACK_LEFT) 
   {
      imesa->readMap = imesa->i810Screen->back.map;
      imesa->Fallback &= ~I810_FALLBACK_READ_BUFFER;
   }
   else
      imesa->Fallback |= I810_FALLBACK_READ_BUFFER;
}



static void i810DDSetColor(GLcontext *ctx, 
			   GLubyte r, GLubyte g,
			   GLubyte b, GLubyte a )
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);
   imesa->MonoColor = i810PackColor( imesa->i810Screen->fbFormat, r, g, b, a );
}


static void i810DDClearColor(GLcontext *ctx, 
			     GLubyte r, GLubyte g,
			     GLubyte b, GLubyte a )
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);
   imesa->ClearColor = i810PackColor( imesa->i810Screen->fbFormat, r, g, b, a );
}


/* =============================================================
 * Culling - the i810 isn't quite as clean here as the rest of
 *           its interfaces, but it's not bad.
 */
static void i810DDCullFaceFrontFace(GLcontext *ctx, GLenum unused)
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);
   GLuint mode = LCS_CULL_BOTH;
   
   FLUSH_BATCH(imesa);

   if (ctx->Polygon.CullFaceMode != GL_FRONT_AND_BACK) {
      mode = LCS_CULL_CW;
      if (ctx->Polygon.CullFaceMode == GL_FRONT)
	 mode ^= (LCS_CULL_CW ^ LCS_CULL_CCW);
      if (ctx->Polygon.FrontFace != GL_CCW)
	 mode ^= (LCS_CULL_CW ^ LCS_CULL_CCW);
   }

   imesa->LcsCullMode = mode;

   if (ctx->Polygon.CullFlag && ctx->PB->primitive == GL_POLYGON)
   {
      imesa->dirty |= I810_UPLOAD_CTX;
      imesa->Setup[I810_CTXREG_LCS] &= ~LCS_CULL_MASK;
      imesa->Setup[I810_CTXREG_LCS] |= mode;
   }
}


static void i810DDReducedPrimitiveChange( GLcontext *ctx, GLenum prim )
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);
   FLUSH_BATCH(imesa);

   imesa->dirty |= I810_UPLOAD_CTX;
   imesa->Setup[I810_CTXREG_LCS] &= ~LCS_CULL_MASK;
   imesa->Setup[I810_CTXREG_ST1] &= ~ST1_ENABLE;
   imesa->Setup[I810_CTXREG_AA] &= ~AA_ENABLE;
   imesa->vertex_prim = PR_TRIANGLES;

   switch (ctx->PB->primitive) {
   case GL_POLYGON:
      if (ctx->Polygon.StippleFlag && 
	  (ctx->Driver.TriangleCaps & DD_TRI_STIPPLE))
	 imesa->Setup[I810_CTXREG_ST1] |= ST1_ENABLE;
      if (ctx->Polygon.CullFlag) 
	 imesa->Setup[I810_CTXREG_LCS] |= imesa->LcsCullMode;
      else 
	 imesa->Setup[I810_CTXREG_LCS] |= LCS_CULL_DISABLE;
      if (ctx->Polygon.SmoothFlag)
	 imesa->Setup[I810_CTXREG_AA] |= AA_ENABLE;
      break;
   case GL_LINES:
      if (ctx->Line.SmoothFlag) 
	 imesa->Setup[I810_CTXREG_AA] |= AA_ENABLE;  
      imesa->Setup[I810_CTXREG_LCS] |= LCS_CULL_DISABLE;
      imesa->vertex_prim = PR_LINES;
      break;
   case GL_POINTS:
      if (ctx->Point.SmoothFlag)
	 imesa->Setup[I810_CTXREG_AA] |= AA_ENABLE;      
      imesa->Setup[I810_CTXREG_LCS] |= LCS_CULL_DISABLE;
      break;
   default:
      imesa->Setup[I810_CTXREG_LCS] |= LCS_CULL_DISABLE;
      break;
   }
}


static void i810DDLineWidth( GLcontext *ctx, GLfloat widthf )
{
   i810ContextPtr imesa = I810_CONTEXT( ctx );
   int width = (int)widthf;

   if (width > 3) width = 3;
   if (width < 1) width = 1;
   
   imesa->Setup[I810_CTXREG_LCS] &= ~LCS_LINEWIDTH_3_0;

   if (width & 1) imesa->Setup[I810_CTXREG_LCS] |= LCS_LINEWIDTH_1_0;
   if (width & 2) imesa->Setup[I810_CTXREG_LCS] |= LCS_LINEWIDTH_2_0;

   imesa->dirty |= I810_UPLOAD_CTX;
}

/* =============================================================
 * Color masks
 */

/* Mesa calls this from the wrong place - it is called a very large
 * number of redundant times.
 *
 * Colormask can be simulated by multipass or multitexture techniques.
 */
static GLboolean i810DDColorMask(GLcontext *ctx, 
				 GLboolean r, GLboolean g, 
				 GLboolean b, GLboolean a )
{
   i810ContextPtr imesa = I810_CONTEXT( ctx );
   GLuint tmp;

   if (r && g && b)
      imesa->Fallback &= ~I810_FALLBACK_COLORMASK;
   else
      imesa->Fallback |= I810_FALLBACK_COLORMASK;

   tmp = imesa->Setup[I810_CTXREG_B2] |
      (B2_FB_WRITE_ENABLE | B2_UPDATE_FB_WRITE_ENABLE);

   if (tmp != imesa->Setup[I810_CTXREG_B2]) {
      FLUSH_BATCH(imesa);
      imesa->Setup[I810_CTXREG_B2] = tmp;
      imesa->dirty |= I810_UPLOAD_CTX;
   }

   return GL_FALSE;  /* makes s/w path always do s/w masking */
}

/* Seperate specular not fully implemented in hardware...  Needs
 * some interaction with material state?  Just punt to software
 * in all cases?
 */
static void i810DDLightModelfv(GLcontext *ctx, GLenum pname, 
			      const GLfloat *param)
{
   if (pname == GL_LIGHT_MODEL_COLOR_CONTROL) 
   {            
      i810ContextPtr imesa = I810_CONTEXT( ctx );
      FLUSH_BATCH(imesa);

      imesa->Fallback &= ~I810_FALLBACK_SPECULAR;
      if (ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)
	 imesa->Fallback |= I810_FALLBACK_SPECULAR;
   }
}

/* The 815 has it...
 */
static void i810DDLightModelfv_i815(GLcontext *ctx, GLenum pname, 
			      const GLfloat *param)
{
   if (pname == GL_LIGHT_MODEL_COLOR_CONTROL) 
   {            
      i810ContextPtr imesa = I810_CONTEXT( ctx );
      FLUSH_BATCH(imesa);
      
      imesa->Setup[I810_CTXREG_B1] &= ~B1_SPEC_ENABLE;

      if (ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)
	 imesa->Setup[I810_CTXREG_B1] |= B1_SPEC_ENABLE;
   }
}
  

/* =============================================================
 * Fog
 */

static void i810DDFogfv(GLcontext *ctx, GLenum pname, const GLfloat *param)
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);

   if (pname == GL_FOG_COLOR) {
      GLuint color = (((GLubyte)(ctx->Fog.Color[0]*255.0F) << 16) |
		      ((GLubyte)(ctx->Fog.Color[1]*255.0F) << 8) |
		      ((GLubyte)(ctx->Fog.Color[2]*255.0F) << 0));

      imesa->dirty |= I810_UPLOAD_CTX;
      imesa->Setup[I810_CTXREG_FOG] = ((GFX_OP_FOG_COLOR | color) &
				      ~FOG_RESERVED_MASK);
   }
}


/* =============================================================
 */

static void i810DDEnable(GLcontext *ctx, GLenum cap, GLboolean state)
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);

   switch(cap) {
   case GL_ALPHA_TEST:
      FLUSH_BATCH(imesa);
      imesa->dirty |= I810_UPLOAD_CTX;
      imesa->Setup[I810_CTXREG_B1] &= ~B1_ALPHA_TEST_ENABLE;
      if (state)
	 imesa->Setup[I810_CTXREG_B1] |= B1_ALPHA_TEST_ENABLE;
      break;
   case GL_BLEND:
      FLUSH_BATCH(imesa);
      imesa->dirty |= I810_UPLOAD_CTX;
      imesa->Setup[I810_CTXREG_B1] &= ~B1_BLEND_ENABLE;
      if (state)
	 imesa->Setup[I810_CTXREG_B1] |= B1_BLEND_ENABLE;

      /* For some reason enable(GL_BLEND) affects ColorLogicOpEnabled.
       */
      if (ctx->Color.ColorLogicOpEnabled && ctx->Color.LogicOp != GL_COPY)
	 imesa->Fallback |= I810_FALLBACK_LOGICOP;
      else
	 imesa->Fallback &= ~I810_FALLBACK_LOGICOP;
      break;
   case GL_DEPTH_TEST:
      FLUSH_BATCH(imesa);
      imesa->dirty |= I810_UPLOAD_CTX;
      imesa->Setup[I810_CTXREG_B1] &= ~B1_Z_TEST_ENABLE;
      if (state)
	 imesa->Setup[I810_CTXREG_B1] |= B1_Z_TEST_ENABLE;
      break;
   case GL_SCISSOR_TEST:
      FLUSH_BATCH(imesa);
      imesa->scissor = state;
      imesa->dirty |= I810_UPLOAD_CLIPRECTS;
      break;
   case GL_POLYGON_STIPPLE:      
      if ((ctx->Driver.TriangleCaps & DD_TRI_STIPPLE) &&
	  ctx->PB->primitive == GL_POLYGON) 
      {
	 FLUSH_BATCH(imesa);
	 imesa->dirty |= I810_UPLOAD_CTX;
	 imesa->Setup[I810_CTXREG_ST1] &= ~ST1_ENABLE;
	 if (state)
	    imesa->Setup[I810_CTXREG_ST1] |= ST1_ENABLE;
      }
      break;
   case GL_LINE_SMOOTH:
      if (ctx->PB->primitive == GL_LINE) {
	 FLUSH_BATCH(imesa);
	 imesa->dirty |= I810_UPLOAD_CTX;
	 imesa->Setup[I810_CTXREG_AA] &= ~AA_ENABLE;
	 imesa->Setup[I810_CTXREG_LCS] &= ~LCS_LINEWIDTH_0_5;
	 if (state) {
	    imesa->Setup[I810_CTXREG_AA] |= AA_ENABLE;
	    imesa->Setup[I810_CTXREG_LCS] |= LCS_LINEWIDTH_0_5;
	 }
      }
      break;
   case GL_POINT_SMOOTH:
      if (ctx->PB->primitive == GL_POINT) {
	 FLUSH_BATCH(imesa);
	 imesa->dirty |= I810_UPLOAD_CTX;
	 imesa->Setup[I810_CTXREG_AA] &= ~AA_ENABLE;
	 if (state) 
	    imesa->Setup[I810_CTXREG_AA] |= AA_ENABLE;
      }
      break;
   case GL_POLYGON_SMOOTH:
      if (ctx->PB->primitive == GL_POLYGON) {
	 FLUSH_BATCH(imesa);
	 imesa->dirty |= I810_UPLOAD_CTX;
	 imesa->Setup[I810_CTXREG_AA] &= ~AA_ENABLE;
	 if (state) 
	    imesa->Setup[I810_CTXREG_AA] |= AA_ENABLE;
      }
      break;
   case GL_FOG:
      FLUSH_BATCH(imesa);
      imesa->dirty |= I810_UPLOAD_CTX;
      imesa->Setup[I810_CTXREG_B1] &= ~B1_FOG_ENABLE;
      if (state)
	 imesa->Setup[I810_CTXREG_B1] |= B1_FOG_ENABLE;
      break;
   case GL_CULL_FACE:
      if (ctx->PB->primitive == GL_POLYGON) {
	 FLUSH_BATCH(imesa);
	 imesa->dirty |= I810_UPLOAD_CTX;
	 imesa->Setup[I810_CTXREG_LCS] &= ~LCS_CULL_MASK;
	 if (state)
	    imesa->Setup[I810_CTXREG_LCS] |= imesa->LcsCullMode;
	 else
	    imesa->Setup[I810_CTXREG_LCS] |= LCS_CULL_DISABLE;
      }
      break;
   case GL_TEXTURE_1D:      
   case GL_TEXTURE_3D:      
      FLUSH_BATCH(imesa);
      imesa->new_state |= I810_NEW_TEXTURE;
      break;
   case GL_TEXTURE_2D:      
      FLUSH_BATCH(imesa);
      imesa->new_state |= I810_NEW_TEXTURE;
      break;
   case GL_COLOR_LOGIC_OP:
      FLUSH_BATCH( imesa );
      imesa->Fallback &= ~I810_FALLBACK_LOGICOP;
      if (state && ctx->Color.LogicOp != GL_COPY)
	 imesa->Fallback |= I810_FALLBACK_LOGICOP;
      break;
   default:
      ; 
   }    
}



/* =============================================================
 */


void i810DDUpdateHwState( GLcontext *ctx )
{
   i810ContextPtr imesa = I810_CONTEXT(ctx);

   if (imesa->new_state & I810_NEW_TEXTURE) {
      FLUSH_BATCH(imesa);
      i810UpdateTextureState( ctx );
   }

   imesa->new_state = 0;
}


void i810EmitDrawingRectangle( i810ContextPtr imesa )
{
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;
   i810ScreenPrivate *i810Screen = imesa->i810Screen;

   int x0 = imesa->drawX;
   int y0 = imesa->drawY;
   int x1 = x0 + dPriv->w;
   int y1 = y0 + dPriv->h;


   /* Coordinate origin of the window - may be offscreen.
    */
   imesa->BufferSetup[I810_DESTREG_DR4] = ((y0<<16) | 
					   (((unsigned)x0)&0xFFFF));
  
   /* Clip to screen.
    */
   if (x0 < 0) x0 = 0;
   if (y0 < 0) y0 = 0;
   if (x1 > i810Screen->width-1) x1 = i810Screen->width-1;
   if (y1 > i810Screen->height-1) y1 = i810Screen->height-1;


   /* Onscreen drawing rectangle.
    */
   imesa->BufferSetup[I810_DESTREG_DR2] = ((y0<<16) | x0);
   imesa->BufferSetup[I810_DESTREG_DR3] = (((y1+1)<<16) | (x1+1));
   imesa->dirty |= I810_UPLOAD_BUFFERS;
}


static void i810DDPrintDirty( const char *msg, GLuint state )
{
   fprintf(stderr, "%s (0x%x): %s%s%s%s%s\n",	   
	   msg,
	   (unsigned int) state,
	   (state & I810_UPLOAD_TEX0IMAGE)  ? "upload-tex0, " : "",
	   (state & I810_UPLOAD_TEX1IMAGE)  ? "upload-tex1, " : "",
	   (state & I810_UPLOAD_CTX)        ? "upload-ctx, " : "",
	   (state & I810_UPLOAD_BUFFERS)    ? "upload-bufs, " : "",
	   (state & I810_UPLOAD_CLIPRECTS)  ? "upload-cliprects, " : ""
	   );
}


/* Push the state into the sarea and/or texture memory.
 */
void i810EmitHwStateLocked( i810ContextPtr imesa )
{
   if (I810_DEBUG & DEBUG_VERBOSE_API)
      i810DDPrintDirty( "\n\n\ni810EmitHwStateLocked", imesa->dirty );

   if (imesa->dirty & ~I810_UPLOAD_CLIPRECTS)
   {
      if ((imesa->dirty & I810_UPLOAD_TEX0IMAGE) && imesa->CurrentTexObj[0])
	 i810UploadTexImages(imesa, imesa->CurrentTexObj[0]);
   
      if ((imesa->dirty & I810_UPLOAD_TEX1IMAGE) && imesa->CurrentTexObj[1])
	 i810UploadTexImages(imesa, imesa->CurrentTexObj[1]);
  
      if (imesa->dirty & I810_UPLOAD_CTX)
	 memcpy( imesa->sarea->ContextState, 
		 imesa->Setup, 
		 sizeof(imesa->Setup) );

      if ((imesa->dirty & I810_UPLOAD_TEX0) && imesa->CurrentTexObj[0]) {
	 imesa->sarea->dirty |= I810_UPLOAD_TEX0;
	 memcpy(imesa->sarea->TexState[0],
		imesa->CurrentTexObj[0]->Setup,
		sizeof(imesa->sarea->TexState[0]));
      }

      if ((imesa->dirty & I810_UPLOAD_TEX1) && imesa->CurrentTexObj[1]) {
	 imesa->sarea->dirty |= I810_UPLOAD_TEX1;
	 memcpy(imesa->sarea->TexState[1],
		imesa->CurrentTexObj[1]->Setup,
		sizeof(imesa->sarea->TexState[1]));
      }
      
      if (imesa->dirty & I810_UPLOAD_BUFFERS) 
	 memcpy( imesa->sarea->BufferState, 
		 imesa->BufferSetup, 
		 sizeof(imesa->BufferSetup) );

      imesa->sarea->dirty |= (imesa->dirty & 
			      ~(I810_UPLOAD_TEX1|I810_UPLOAD_TEX0));
      imesa->dirty &= I810_UPLOAD_CLIPRECTS;
   }
}



void i810DDInitState( i810ContextPtr imesa )
{
   i810ScreenPrivate *i810Screen = imesa->i810Screen;

   memset(imesa->Setup, 0, sizeof(imesa->Setup));

   imesa->Setup[I810_CTXREG_VF] = I810_VFMT_T0;

   imesa->Setup[I810_CTXREG_MT] = (GFX_OP_MAP_TEXELS |
				   MT_UPDATE_TEXEL1_STATE |
				   MT_TEXEL1_COORD1 |
				   MT_TEXEL1_MAP1 |
				   MT_TEXEL1_DISABLE |
				   MT_UPDATE_TEXEL0_STATE |
				   MT_TEXEL0_COORD0 |
				   MT_TEXEL0_MAP0 |
				   MT_TEXEL0_DISABLE);

   imesa->Setup[I810_CTXREG_MC0] = ( GFX_OP_MAP_COLOR_STAGES |
				     MC_STAGE_0 |
				     MC_UPDATE_DEST |
				     MC_DEST_CURRENT |
				     MC_UPDATE_ARG1 |
				     MC_ARG1_ITERATED_COLOR | 
				     MC_ARG1_DONT_REPLICATE_ALPHA |
				     MC_ARG1_DONT_INVERT |
				     MC_UPDATE_ARG2 |
				     MC_ARG2_ONE |
				     MC_ARG2_DONT_REPLICATE_ALPHA |
				     MC_ARG2_DONT_INVERT |
				     MC_UPDATE_OP |
				     MC_OP_ARG1 );
				     
   imesa->Setup[I810_CTXREG_MC1] = ( GFX_OP_MAP_COLOR_STAGES |
				     MC_STAGE_1 |
				     MC_UPDATE_DEST |
				     MC_DEST_CURRENT |
				     MC_UPDATE_ARG1 |
				     MC_ARG1_ONE | 
				     MC_ARG1_DONT_REPLICATE_ALPHA |
				     MC_ARG1_DONT_INVERT |
				     MC_UPDATE_ARG2 |
				     MC_ARG2_ONE |
				     MC_ARG2_DONT_REPLICATE_ALPHA |
				     MC_ARG2_DONT_INVERT |
				     MC_UPDATE_OP |
				     MC_OP_DISABLE );
				     

   imesa->Setup[I810_CTXREG_MC2] = ( GFX_OP_MAP_COLOR_STAGES |
				     MC_STAGE_2 |
				     MC_UPDATE_DEST |
				     MC_DEST_CURRENT |
				     MC_UPDATE_ARG1 |
				     MC_ARG1_CURRENT_COLOR | 
				     MC_ARG1_REPLICATE_ALPHA |
				     MC_ARG1_DONT_INVERT |
				     MC_UPDATE_ARG2 |
				     MC_ARG2_ONE |
				     MC_ARG2_DONT_REPLICATE_ALPHA |
				     MC_ARG2_DONT_INVERT |
				     MC_UPDATE_OP |
				     MC_OP_DISABLE );
				     

   imesa->Setup[I810_CTXREG_MA0] = ( GFX_OP_MAP_ALPHA_STAGES |
				     MA_STAGE_0 |
				     MA_UPDATE_ARG1 |
				     MA_ARG1_CURRENT_ALPHA |
				     MA_ARG1_DONT_INVERT |
				     MA_UPDATE_ARG2 |
				     MA_ARG2_CURRENT_ALPHA |
				     MA_ARG2_DONT_INVERT |
				     MA_UPDATE_OP |
				     MA_OP_ARG1 );


   imesa->Setup[I810_CTXREG_MA1] = ( GFX_OP_MAP_ALPHA_STAGES |
				     MA_STAGE_1 |
				     MA_UPDATE_ARG1 |
				     MA_ARG1_CURRENT_ALPHA |
				     MA_ARG1_DONT_INVERT |
				     MA_UPDATE_ARG2 |
				     MA_ARG2_CURRENT_ALPHA |
				     MA_ARG2_DONT_INVERT |
				     MA_UPDATE_OP |
				     MA_OP_ARG1 );


   imesa->Setup[I810_CTXREG_MA2] = ( GFX_OP_MAP_ALPHA_STAGES |
				     MA_STAGE_2 |
				     MA_UPDATE_ARG1 |
				     MA_ARG1_CURRENT_ALPHA |
				     MA_ARG1_DONT_INVERT |
				     MA_UPDATE_ARG2 |
				     MA_ARG2_CURRENT_ALPHA |
				     MA_ARG2_DONT_INVERT |
				     MA_UPDATE_OP |
				     MA_OP_ARG1 );


   imesa->Setup[I810_CTXREG_SDM] = ( GFX_OP_SRC_DEST_MONO |
				     SDM_UPDATE_MONO_ENABLE |
				     0 |
				     SDM_UPDATE_SRC_BLEND | 
				     SDM_SRC_ONE |
				     SDM_UPDATE_DST_BLEND |
				     SDM_DST_ZERO );

   /* Use for colormask:
    */
   imesa->Setup[I810_CTXREG_CF0] = GFX_OP_COLOR_FACTOR;
   imesa->Setup[I810_CTXREG_CF1] = 0xffffffff;

   imesa->Setup[I810_CTXREG_ZA] = (GFX_OP_ZBIAS_ALPHAFUNC |
				   ZA_UPDATE_ALPHAFUNC |
				   ZA_ALPHA_ALWAYS |
				   ZA_UPDATE_ZBIAS |
				   0 |
				   ZA_UPDATE_ALPHAREF |
				   0x0);

   imesa->Setup[I810_CTXREG_FOG] = (GFX_OP_FOG_COLOR | 
				    (0xffffff & ~FOG_RESERVED_MASK));

   /* Choose a pipe
    */
   imesa->Setup[I810_CTXREG_B1] = ( GFX_OP_BOOL_1 |
				    B1_UPDATE_SPEC_SETUP_ENABLE |
				    0 |
				    B1_UPDATE_ALPHA_SETUP_ENABLE |
				    B1_ALPHA_SETUP_ENABLE |
				    B1_UPDATE_CI_KEY_ENABLE |
				    0 |
				    B1_UPDATE_CHROMAKEY_ENABLE |
				    0 |
				    B1_UPDATE_Z_BIAS_ENABLE |
				    0 |
				    B1_UPDATE_SPEC_ENABLE |
				    0 |
				    B1_UPDATE_FOG_ENABLE |
				    0 |
				    B1_UPDATE_ALPHA_TEST_ENABLE |
				    0 |
				    B1_UPDATE_BLEND_ENABLE |
				    0 |
				    B1_UPDATE_Z_TEST_ENABLE |
				    0 );

   imesa->Setup[I810_CTXREG_B2] = ( GFX_OP_BOOL_2 |
				    B2_UPDATE_MAP_CACHE_ENABLE |
				    B2_MAP_CACHE_ENABLE |
				    B2_UPDATE_ALPHA_DITHER_ENABLE |
				    0 |
				    B2_UPDATE_FOG_DITHER_ENABLE |
				    0 |
				    B2_UPDATE_SPEC_DITHER_ENABLE |
				    0 |
				    B2_UPDATE_RGB_DITHER_ENABLE |
				    B2_RGB_DITHER_ENABLE |
				    B2_UPDATE_FB_WRITE_ENABLE |
				    B2_FB_WRITE_ENABLE |
				    B2_UPDATE_ZB_WRITE_ENABLE |
				    B2_ZB_WRITE_ENABLE );

   imesa->Setup[I810_CTXREG_LCS] = ( GFX_OP_LINEWIDTH_CULL_SHADE_MODE |
				     LCS_UPDATE_ZMODE |
				     LCS_Z_LESS |
				     LCS_UPDATE_LINEWIDTH |
				     LCS_LINEWIDTH_1_0 |
				     LCS_UPDATE_ALPHA_INTERP |
				     LCS_ALPHA_INTERP |
				     LCS_UPDATE_FOG_INTERP |
				     0 |
				     LCS_UPDATE_SPEC_INTERP |
				     0 |
				     LCS_UPDATE_RGB_INTERP |
				     LCS_RGB_INTERP |
				     LCS_UPDATE_CULL_MODE |
				     LCS_CULL_DISABLE);

   imesa->LcsCullMode = LCS_CULL_CW;
   
   imesa->Setup[I810_CTXREG_PV] = ( GFX_OP_PV_RULE |
				    PV_UPDATE_PIXRULE |
				    PV_PIXRULE_ENABLE |
				    PV_UPDATE_LINELIST |
				    PV_LINELIST_PV0 |
				    PV_UPDATE_TRIFAN |
				    PV_TRIFAN_PV0 |
				    PV_UPDATE_TRISTRIP |
				    PV_TRISTRIP_PV0 );


   imesa->Setup[I810_CTXREG_ST0] = GFX_OP_STIPPLE;
   imesa->Setup[I810_CTXREG_ST1] = 0;

   imesa->Setup[I810_CTXREG_AA] = ( GFX_OP_ANTIALIAS |
				    AA_UPDATE_EDGEFLAG |
				    AA_ENABLE_EDGEFLAG | /* ? */
				    AA_UPDATE_POLYWIDTH |
				    AA_POLYWIDTH_05 |
				    AA_UPDATE_LINEWIDTH |
				    AA_LINEWIDTH_05 |
				    AA_UPDATE_BB_EXPANSION |
				    0 |
				    AA_UPDATE_AA_ENABLE |
				    0 );

   memset(imesa->BufferSetup, 0, sizeof(imesa->BufferSetup));
   imesa->BufferSetup[I810_DESTREG_DI0] = CMD_OP_DESTBUFFER_INFO;

   if (imesa->glCtx->Color.DriverDrawBuffer == GL_BACK_LEFT) {
      imesa->drawMap = i810Screen->back.map;
      imesa->readMap = i810Screen->back.map;
      imesa->BufferSetup[I810_DESTREG_DI1] = (i810Screen->backOffset | 
					      i810Screen->backPitchBits);
   } else {
      imesa->drawMap = (char *)imesa->driScreen->pFB;
      imesa->readMap = (char *)imesa->driScreen->pFB;
      imesa->BufferSetup[I810_DESTREG_DI1] = (i810Screen->fbOffset | 
					      i810Screen->backPitchBits);
   }

   imesa->BufferSetup[I810_DESTREG_DV0] = GFX_OP_DESTBUFFER_VARS;
   imesa->BufferSetup[I810_DESTREG_DV1] = (DV_HORG_BIAS_OGL |
					   DV_VORG_BIAS_OGL |
					   i810Screen->fbFormat);

   imesa->BufferSetup[I810_DESTREG_DR0] = GFX_OP_DRAWRECT_INFO;
   imesa->BufferSetup[I810_DESTREG_DR1] = DR1_RECT_CLIP_ENABLE;
}


#define INTERESTED (~(NEW_MODELVIEW|NEW_PROJECTION|\
                      NEW_TEXTURE_MATRIX|\
                      NEW_USER_CLIP|NEW_CLIENT_STATE))


void i810DDUpdateState( GLcontext *ctx )
{
   i810ContextPtr imesa = I810_CONTEXT( ctx );

   /* Have to do this here to detect texture fallbacks in time:
    */
   if (I810_CONTEXT(ctx)->new_state & I810_NEW_TEXTURE)
      i810DDUpdateHwState( ctx );


   if (ctx->NewState & INTERESTED) {
      i810DDChooseRenderState(ctx);  
      i810ChooseRasterSetupFunc(ctx);
   }

   if (0) 
      fprintf(stderr, "IndirectTriangles %x Fallback %x\n", 
	      imesa->IndirectTriangles, imesa->Fallback);
   
   if (!imesa->Fallback)
   {
      ctx->IndirectTriangles &= ~DD_SW_RASTERIZE;
      ctx->IndirectTriangles |= imesa->IndirectTriangles;

      ctx->Driver.PointsFunc=imesa->PointsFunc;
      ctx->Driver.LineFunc=imesa->LineFunc;
      ctx->Driver.TriangleFunc=imesa->TriangleFunc;
      ctx->Driver.QuadFunc=imesa->QuadFunc;
   }
}


void i810DDInitStateFuncs(GLcontext *ctx)
{
   ctx->Driver.UpdateState = i810DDUpdateState;
   ctx->Driver.Enable = i810DDEnable;
   ctx->Driver.AlphaFunc = i810DDAlphaFunc;
   ctx->Driver.BlendEquation = i810DDBlendEquation;
   ctx->Driver.BlendFunc = i810DDBlendFunc;
   ctx->Driver.BlendFuncSeparate = i810DDBlendFuncSeparate;
   ctx->Driver.DepthFunc = i810DDDepthFunc;
   ctx->Driver.DepthMask = i810DDDepthMask;
   ctx->Driver.Fogfv = i810DDFogfv;
   ctx->Driver.Scissor = i810DDScissor;
   ctx->Driver.CullFace = i810DDCullFaceFrontFace;
   ctx->Driver.FrontFace = i810DDCullFaceFrontFace;
   ctx->Driver.ColorMask = i810DDColorMask;
   ctx->Driver.ReducedPrimitiveChange = i810DDReducedPrimitiveChange;
   ctx->Driver.RenderStart = i810DDUpdateHwState; 
   ctx->Driver.RenderFinish = 0; 
   ctx->Driver.PolygonStipple = i810DDPolygonStipple;
   ctx->Driver.LineStipple = 0;
   ctx->Driver.LineWidth = i810DDLineWidth;
   ctx->Driver.LogicOpcode = i810DDLogicOp;
   ctx->Driver.SetReadBuffer = i810DDSetReadBuffer;
   ctx->Driver.SetDrawBuffer = i810DDSetDrawBuffer;
   ctx->Driver.Color = i810DDSetColor;
   ctx->Driver.ClearColor = i810DDClearColor;
   ctx->Driver.Dither = i810DDDither;
   ctx->Driver.Index = 0;
   ctx->Driver.ClearIndex = 0;
   ctx->Driver.IndexMask = 0;

   if (IS_I815(I810_CONTEXT(ctx))) {
      ctx->Driver.LightModelfv = i810DDLightModelfv_i815;
   } else {
      ctx->Driver.LightModelfv = i810DDLightModelfv;
   }
}
