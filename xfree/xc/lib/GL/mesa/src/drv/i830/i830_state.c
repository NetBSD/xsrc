/**************************************************************************

Copyright 2001 VA Linux Systems Inc., Fremont, California.

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

/* $XFree86: xc/lib/GL/mesa/src/drv/i830/i830_state.c,v 1.1 2001/10/04 18:28:21 alanh Exp $ */

/*
 * Author:
 *   Jeff Hartmann <jhartmann@valinux.com>
 *
 * Heavily based on the I810 driver, which was written by:
 *   Keith Whitwell <keithw@valinux.com>
 */

#include <stdio.h>

#include "types.h"
#include "enums.h"
#include "pb.h"
#include "dd.h"

#include "mm.h"

#include "i830_drv.h"
#include "i830_tris.h"
#include "i830_ioctl.h"

/* Need to add other formats */
static __inline__ GLuint i830PackColor(GLuint format, 
				       GLubyte r, GLubyte g, 
				       GLubyte b, GLubyte a)
{

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   switch (format) {
   case DV_PF_555:
      return I830PACKCOLOR1555(r,g,b,a);
   case DV_PF_565:
      return I830PACKCOLOR565(r,g,b);
   case DV_PF_8888:
      return I830PACKCOLOR8888(r,g,b,a);
   default:
      fprintf(stderr, "unknown format %d\n", (int)format);
      return 0;
   }
}

static void i830DDPointSize(GLcontext *ctx, GLfloat size)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   GLint point_size = FloatToInt(size);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
     fprintf(stderr, "%s\n", __FUNCTION__);

   FLUSH_BATCH(imesa);
   CLAMP_SELF(point_size, 1, 256);
   imesa->dirty |= I830_UPLOAD_CTX;
   imesa->Setup[I830_CTXREG_STATE5] &= ~FIXED_POINT_WIDTH_MASK;
   imesa->Setup[I830_CTXREG_STATE5] |= (ENABLE_FIXED_POINT_WIDTH |
				       FIXED_POINT_WIDTH(point_size));
}

static void i830DDStencilFunc(GLcontext *ctx, GLenum func, GLint ref,
				 GLuint mask)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   GLuint v_mask, w_mask;
   int test = 0;


   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s : func: %s, ref : 0x%x, mask: 0x%x\n", __FUNCTION__,
	      gl_lookup_enum_by_nr(func), ref, mask);

   FLUSH_BATCH(imesa);

   v_mask = ctx->Stencil.ValueMask & 0xff;
   w_mask = ctx->Stencil.WriteMask & 0xff;

   switch(func) {
   case GL_NEVER: test = COMPAREFUNC_NEVER; break;
   case GL_LESS: test = COMPAREFUNC_LESS; break;
   case GL_EQUAL: test = COMPAREFUNC_EQUAL; break;
   case GL_LEQUAL: test = COMPAREFUNC_LEQUAL; break;
   case GL_GREATER: test = COMPAREFUNC_GREATER; break;
   case GL_NOTEQUAL: test = COMPAREFUNC_NOTEQUAL; break;
   case GL_GEQUAL: test = COMPAREFUNC_GEQUAL; break;
   case GL_ALWAYS: test = COMPAREFUNC_ALWAYS; break;
   default: break;
   }

   imesa->dirty |= I830_UPLOAD_CTX;
   imesa->Setup[I830_CTXREG_STATE4] &= ~MODE4_ENABLE_STENCIL_MASK;
   imesa->Setup[I830_CTXREG_STATE4] |= (ENABLE_STENCIL_TEST_MASK |
				    ENABLE_STENCIL_WRITE_MASK |
				    STENCIL_TEST_MASK(v_mask) |
				    STENCIL_WRITE_MASK(w_mask));
   imesa->Setup[I830_CTXREG_STENCILTST] &= ~(STENCIL_REF_VALUE_MASK |
					 ENABLE_STENCIL_TEST_FUNC_MASK);
   imesa->Setup[I830_CTXREG_STENCILTST] |= (ENABLE_STENCIL_REF_VALUE |
					ENABLE_STENCIL_TEST_FUNC |
					STENCIL_REF_VALUE(ref) |
					STENCIL_TEST_FUNC(test));

   if(I830_DEBUG&DEBUG_VERBOSE_STATE)
      fprintf(stderr, "%s : state4 : 0x%x, stentst : 0x%x\n", __FUNCTION__,
	      imesa->Setup[I830_CTXREG_STATE4],
	      imesa->Setup[I830_CTXREG_STENCILTST]);
}

static void i830DDStencilMask(GLcontext *ctx, GLuint mask)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   GLuint v_mask, w_mask;


   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s : mask 0x%x\n", __FUNCTION__, mask);

   FLUSH_BATCH(imesa);

   v_mask = ctx->Stencil.ValueMask & 0xff;
   w_mask = ctx->Stencil.WriteMask & 0xff;

   imesa->dirty |= I830_UPLOAD_CTX;

   imesa->Setup[I830_CTXREG_STATE4] &= ~MODE4_ENABLE_STENCIL_MASK;
   imesa->Setup[I830_CTXREG_STATE4] |= (ENABLE_STENCIL_TEST_MASK |
				    ENABLE_STENCIL_WRITE_MASK |
				    STENCIL_TEST_MASK(v_mask) |
				    STENCIL_WRITE_MASK(w_mask));
   if(I830_DEBUG&DEBUG_VERBOSE_STATE)
      fprintf(stderr, "%s : state4 : 0x%x\n", __FUNCTION__,
	      imesa->Setup[I830_CTXREG_STATE4]);
}

static void i830DDStencilOp(GLcontext *ctx, GLenum fail, GLenum zfail,
			       GLenum zpass)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   int fop, dfop, dpop;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s: fail : %s, zfail: %s, zpass : %s\n", __FUNCTION__,
	      gl_lookup_enum_by_nr(fail),
	      gl_lookup_enum_by_nr(zfail),
	      gl_lookup_enum_by_nr(zpass));

   FLUSH_BATCH(imesa);

   fop = 0; dfop = 0; dpop = 0;

   switch(fail) {
   case GL_KEEP: fop = STENCILOP_KEEP; break;
   case GL_ZERO: fop = STENCILOP_ZERO; break;
   case GL_REPLACE: fop = STENCILOP_REPLACE; break;
   case GL_INCR: fop = STENCILOP_INCR; break;
   case GL_DECR: fop = STENCILOP_DECR; break;
   case GL_INVERT: fop = STENCILOP_INVERT; break;
   default: break;
   }
   switch(zfail) {
   case GL_KEEP: dfop = STENCILOP_KEEP; break;
   case GL_ZERO: dfop = STENCILOP_ZERO; break;
   case GL_REPLACE: dfop = STENCILOP_REPLACE; break;
   case GL_INCR: dfop = STENCILOP_INCR; break;
   case GL_DECR: dfop = STENCILOP_DECR; break;
   case GL_INVERT: dfop = STENCILOP_INVERT; break;
   default: break;
   }
   switch(zpass) {
   case GL_KEEP: dpop = STENCILOP_KEEP; break;
   case GL_ZERO: dpop = STENCILOP_ZERO; break;
   case GL_REPLACE: dpop = STENCILOP_REPLACE; break;
   case GL_INCR: dpop = STENCILOP_INCR; break;
   case GL_DECR: dpop = STENCILOP_DECR; break;
   case GL_INVERT: dpop = STENCILOP_INVERT; break;
   default: break;
   }

   imesa->dirty |= I830_UPLOAD_CTX;
   imesa->Setup[I830_CTXREG_STENCILTST] &= ~(STENCIL_OPS_MASK);
   imesa->Setup[I830_CTXREG_STENCILTST] |= (ENABLE_STENCIL_PARMS |
					   STENCIL_FAIL_OP(fop) |
					   STENCIL_PASS_DEPTH_FAIL_OP(dfop) |
					   STENCIL_PASS_DEPTH_PASS_OP(dpop));
   if(I830_DEBUG&DEBUG_VERBOSE_STATE)
      fprintf(stderr, "%s : stentst : 0x%x\n", __FUNCTION__,
	      imesa->Setup[I830_CTXREG_STENCILTST]);

}

static void i830DDAlphaFunc(GLcontext *ctx, GLenum func, GLclampf ref)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   int test = 0;
   GLubyte tmp_ref;
   
   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s %s\n", __FUNCTION__, gl_lookup_enum_by_nr(func));

   FLOAT_COLOR_TO_UBYTE_COLOR(tmp_ref, ref);

   FLUSH_BATCH(imesa);

   switch(func) {
   case GL_NEVER:
      test = COMPAREFUNC_NEVER;
      break;
   case GL_LESS:
      test = COMPAREFUNC_LESS;
      break;
   case GL_LEQUAL:
      test = COMPAREFUNC_LEQUAL;
      break;
   case GL_GREATER:
      test = COMPAREFUNC_GREATER;
      break;
   case GL_GEQUAL:
      test = COMPAREFUNC_GEQUAL;
      break;
   case GL_NOTEQUAL:
      test = COMPAREFUNC_NOTEQUAL;
      break;
   case GL_EQUAL:
      test = COMPAREFUNC_EQUAL;
      break;
   case GL_ALWAYS:
      test = COMPAREFUNC_ALWAYS;
      break;
   default: return;
   }
   imesa->dirty |= I830_UPLOAD_CTX;
   imesa->Setup[I830_CTXREG_STATE2] &= ~ALPHA_TEST_REF_MASK;
   imesa->Setup[I830_CTXREG_STATE2] |= (ENABLE_ALPHA_TEST_FUNC |
				    ENABLE_ALPHA_REF_VALUE |
				    ALPHA_TEST_FUNC(test) |
				    ALPHA_REF_VALUE(tmp_ref));
}

/* This function makes sure that the proper enables are
 * set for LogicOp, Independant Alpha Blend, and Blending.
 * It needs to be called from numerous places where we
 * could change the LogicOp or Independant Alpha Blend without subsequent
 * calls to glEnable.
 */
static void i830EvalLogicOpBlendState(GLcontext *ctx)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);

   FLUSH_BATCH(imesa);
   imesa->dirty |= I830_UPLOAD_CTX;

   if(ctx->Color.ColorLogicOpEnabled || ctx->Color.IndexLogicOpEnabled) {
     imesa->Setup[I830_CTXREG_ENABLES_1] &= ~(ENABLE_COLOR_BLEND |
					      ENABLE_LOGIC_OP_MASK);
     imesa->Setup[I830_CTXREG_ENABLES_1] |= (DISABLE_COLOR_BLEND |
					     ENABLE_LOGIC_OP);
     imesa->Setup[I830_CTXREG_IALPHAB] &= ~ENABLE_INDPT_ALPHA_BLEND;
     imesa->Setup[I830_CTXREG_IALPHAB] |= DISABLE_INDPT_ALPHA_BLEND;
   } else if(ctx->Color.BlendEnabled) {
     imesa->Setup[I830_CTXREG_ENABLES_1] &= ~(ENABLE_COLOR_BLEND |
					      ENABLE_LOGIC_OP_MASK);
     imesa->Setup[I830_CTXREG_ENABLES_1] |= (ENABLE_COLOR_BLEND |
					     DISABLE_LOGIC_OP);
     imesa->Setup[I830_CTXREG_IALPHAB] &= ~ENABLE_INDPT_ALPHA_BLEND;
     if(imesa->Setup[I830_CTXREG_IALPHAB] & SRC_DST_ABLEND_MASK) {
       imesa->Setup[I830_CTXREG_IALPHAB] |= ENABLE_INDPT_ALPHA_BLEND;
     } else {
       imesa->Setup[I830_CTXREG_IALPHAB] |= DISABLE_INDPT_ALPHA_BLEND;
     }
   } else {
     imesa->Setup[I830_CTXREG_ENABLES_1] &= ~(ENABLE_COLOR_BLEND |
					      ENABLE_LOGIC_OP_MASK);
     imesa->Setup[I830_CTXREG_ENABLES_1] |= (DISABLE_COLOR_BLEND |
					     DISABLE_LOGIC_OP);
     imesa->Setup[I830_CTXREG_IALPHAB] &= ~ENABLE_INDPT_ALPHA_BLEND;
     imesa->Setup[I830_CTXREG_IALPHAB] |= DISABLE_INDPT_ALPHA_BLEND;
   }
}

static void i830DDBlendEquation(GLcontext *ctx, GLenum mode) 
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   int func = ENABLE_ALPHA_BLENDFUNC;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
     fprintf(stderr, "%s %s\n", __FUNCTION__,
	     gl_lookup_enum_by_nr(mode));

   i830EvalLogicOpBlendState(ctx);

   FLUSH_BATCH(imesa);

   switch(mode) {
   case GL_FUNC_ADD_EXT: func |= BLENDFUNC_ADD; break;
   case GL_MIN_EXT: func |= BLENDFUNC_MIN; break;
   case GL_MAX_EXT: func |= BLENDFUNC_MAX; break;
   case GL_FUNC_SUBTRACT_EXT: func |= BLENDFUNC_SUB; break;
   case GL_FUNC_REVERSE_SUBTRACT_EXT: func |= BLENDFUNC_RVRSE_SUB; break;
   default: return;
   }
   
   imesa->dirty |= I830_UPLOAD_CTX;
   imesa->Setup[I830_CTXREG_STATE1] &= ~BLENDFUNC_MASK;
   imesa->Setup[I830_CTXREG_STATE1] |= func;
}

static void i830DDBlendConstColor(GLcontext *ctx, GLfloat red,
				     GLfloat green, GLfloat blue,
				     GLfloat alpha)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   GLubyte r, g, b, a;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   FLOAT_COLOR_TO_UBYTE_COLOR(r, red);
   FLOAT_COLOR_TO_UBYTE_COLOR(g, green);
   FLOAT_COLOR_TO_UBYTE_COLOR(b, blue);
   FLOAT_COLOR_TO_UBYTE_COLOR(a, alpha);

   imesa->dirty |= I830_UPLOAD_CTX;
   imesa->Setup[I830_CTXREG_BLENDCOLR] = ((a << 24) |
					 (r << 16) |
					 (g << 8) |
					 b);
}

static void i830DDBlendFuncSeparate(GLcontext *ctx, GLenum sfactorRGB, 
				       GLenum dfactorRGB, GLenum sfactorA,
				       GLenum dfactorA )
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   int funcA = (ENABLE_SRC_ABLEND_FACTOR|ENABLE_DST_ABLEND_FACTOR);
   int funcRGB = (ENABLE_SRC_BLND_FACTOR|ENABLE_DST_BLND_FACTOR);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   FLUSH_BATCH(imesa);

   switch(sfactorA) {
   case GL_ZERO: funcA |= SRC_ABLEND_FACT(BLENDFACT_ZERO); break;
   case GL_SRC_ALPHA: funcA |= SRC_ABLEND_FACT(BLENDFACT_SRC_ALPHA); break;
   case GL_ONE: funcA |= SRC_ABLEND_FACT(BLENDFACT_ONE); break;
   case GL_DST_COLOR: funcA |= SRC_ABLEND_FACT(BLENDFACT_DST_COLR); break;
   case GL_ONE_MINUS_DST_COLOR: 
      		      funcA |= SRC_ABLEND_FACT(BLENDFACT_INV_DST_COLR); break;
   case GL_ONE_MINUS_SRC_ALPHA:
		      funcA |= SRC_ABLEND_FACT(BLENDFACT_INV_SRC_ALPHA); break;
   case GL_DST_ALPHA: funcA |= SRC_ABLEND_FACT(BLENDFACT_DST_ALPHA); break;
   case GL_ONE_MINUS_DST_ALPHA:
		      funcA |= SRC_ABLEND_FACT(BLENDFACT_INV_DST_ALPHA); break;
   case GL_SRC_ALPHA_SATURATE: 
		      funcA |= SRC_ABLEND_FACT(BLENDFACT_SRC_ALPHA_SATURATE);
		      break;
   case GL_CONSTANT_COLOR_EXT:
		      funcA |= SRC_ABLEND_FACT(BLENDFACT_CONST_COLOR); break;
   case GL_ONE_MINUS_CONSTANT_COLOR_EXT:
		      funcA |= SRC_ABLEND_FACT(BLENDFACT_INV_CONST_COLOR); break;
   case GL_CONSTANT_ALPHA_EXT:
		      funcA |= SRC_ABLEND_FACT(BLENDFACT_CONST_ALPHA); break;
   case GL_ONE_MINUS_CONSTANT_ALPHA_EXT:
		      funcA |= SRC_ABLEND_FACT(BLENDFACT_INV_CONST_ALPHA);
		      break;
   default: return;
   }

   switch(dfactorA) {
   case GL_SRC_ALPHA: funcA |= DST_ABLEND_FACT(BLENDFACT_SRC_ALPHA); break;
   case GL_ONE_MINUS_SRC_ALPHA: 
		      funcA |= DST_ABLEND_FACT(BLENDFACT_INV_SRC_ALPHA); break;
   case GL_ZERO: funcA |= DST_ABLEND_FACT(BLENDFACT_ZERO); break;
   case GL_ONE: funcA |= DST_ABLEND_FACT(BLENDFACT_ONE); break;
   case GL_SRC_COLOR: funcA |= DST_ABLEND_FACT(BLENDFACT_SRC_COLR); break;
   case GL_ONE_MINUS_SRC_COLOR: 
		      funcA |= DST_ABLEND_FACT(BLENDFACT_INV_SRC_COLR); break;
   case GL_DST_ALPHA: funcA |= DST_ABLEND_FACT(BLENDFACT_DST_ALPHA); break;
   case GL_ONE_MINUS_DST_ALPHA: 
		      funcA |= DST_ABLEND_FACT(BLENDFACT_INV_DST_ALPHA); break;
   case GL_CONSTANT_COLOR_EXT:
		      funcA |= DST_ABLEND_FACT(BLENDFACT_CONST_COLOR); break;
   case GL_ONE_MINUS_CONSTANT_COLOR_EXT:
		      funcA |= DST_ABLEND_FACT(BLENDFACT_INV_CONST_COLOR);
		      break;
   case GL_CONSTANT_ALPHA_EXT:
		      funcA |= DST_ABLEND_FACT(BLENDFACT_CONST_ALPHA); break;
   case GL_ONE_MINUS_CONSTANT_ALPHA_EXT:
		      funcA |= DST_ABLEND_FACT(BLENDFACT_INV_CONST_ALPHA); 
		      break;
   default: return;
   }
   
   switch(sfactorRGB) {
   case GL_ZERO: funcRGB |= SRC_BLND_FACT(BLENDFACT_ZERO); break;
   case GL_SRC_ALPHA: funcRGB |= SRC_BLND_FACT(BLENDFACT_SRC_ALPHA); break;
   case GL_ONE: funcRGB |= SRC_BLND_FACT(BLENDFACT_ONE); break;
   case GL_DST_COLOR: funcRGB |= SRC_BLND_FACT(BLENDFACT_DST_COLR); break;
   case GL_ONE_MINUS_DST_COLOR: 
      		      funcRGB |= SRC_BLND_FACT(BLENDFACT_INV_DST_COLR); break;
   case GL_ONE_MINUS_SRC_ALPHA:
		      funcRGB |= SRC_BLND_FACT(BLENDFACT_INV_SRC_ALPHA); break;
   case GL_DST_ALPHA: funcRGB |= SRC_BLND_FACT(BLENDFACT_DST_ALPHA); break;
   case GL_ONE_MINUS_DST_ALPHA:
		      funcRGB |= SRC_BLND_FACT(BLENDFACT_INV_DST_ALPHA); break;
   case GL_SRC_ALPHA_SATURATE: 
		      funcRGB |= SRC_BLND_FACT(BLENDFACT_SRC_ALPHA_SATURATE);
		      break;
   case GL_CONSTANT_COLOR_EXT:
		      funcRGB |= SRC_BLND_FACT(BLENDFACT_CONST_COLOR); break;
   case GL_ONE_MINUS_CONSTANT_COLOR_EXT:
		      funcRGB |= SRC_BLND_FACT(BLENDFACT_INV_CONST_COLOR);
		      break;
   case GL_CONSTANT_ALPHA_EXT:
		      funcRGB |= SRC_BLND_FACT(BLENDFACT_CONST_ALPHA); break;
   case GL_ONE_MINUS_CONSTANT_ALPHA_EXT:
		      funcRGB |= SRC_BLND_FACT(BLENDFACT_INV_CONST_ALPHA);
		      break;
   default: return;
   }
   
   switch(dfactorRGB) {
   case GL_SRC_ALPHA: funcRGB |= DST_BLND_FACT(BLENDFACT_SRC_ALPHA); break;
   case GL_ONE_MINUS_SRC_ALPHA: 
		      funcRGB |= DST_BLND_FACT(BLENDFACT_INV_SRC_ALPHA); break;
   case GL_ZERO: funcRGB |= DST_BLND_FACT(BLENDFACT_ZERO); break;
   case GL_ONE: funcRGB |= DST_BLND_FACT(BLENDFACT_ONE); break;
   case GL_SRC_COLOR: funcRGB |= DST_BLND_FACT(BLENDFACT_SRC_COLR); break;
   case GL_ONE_MINUS_SRC_COLOR: 
		      funcRGB |= DST_BLND_FACT(BLENDFACT_INV_SRC_COLR); break;
   case GL_DST_ALPHA: funcRGB |= DST_BLND_FACT(BLENDFACT_DST_ALPHA); break;
   case GL_ONE_MINUS_DST_ALPHA: 
		      funcRGB |= DST_BLND_FACT(BLENDFACT_INV_DST_ALPHA); break;
   case GL_CONSTANT_COLOR_EXT:
		      funcRGB |= DST_BLND_FACT(BLENDFACT_CONST_COLOR); break;
   case GL_ONE_MINUS_CONSTANT_COLOR_EXT:
		      funcRGB |= DST_BLND_FACT(BLENDFACT_INV_CONST_COLOR);
		      break;
   case GL_CONSTANT_ALPHA_EXT:
		      funcRGB |= DST_BLND_FACT(BLENDFACT_CONST_ALPHA); break;
   case GL_ONE_MINUS_CONSTANT_ALPHA_EXT:
		      funcRGB |= DST_BLND_FACT(BLENDFACT_INV_CONST_ALPHA); 
		      break;
   default: return;
   }

   imesa->dirty |= I830_UPLOAD_CTX;
   imesa->Setup[I830_CTXREG_IALPHAB] &= ~SRC_DST_ABLEND_MASK;
   imesa->Setup[I830_CTXREG_IALPHAB] |= funcA;
   imesa->Setup[I830_CTXREG_STATE1] &= ~SRC_DST_BLND_MASK;
   imesa->Setup[I830_CTXREG_STATE1] |= funcRGB;
   /* Insure Independant Alpha Blend is really enabled if need be */
   i830EvalLogicOpBlendState(ctx);
}

static void i830DDBlendFunc(GLcontext *ctx, GLenum sfactor, GLenum dfactor)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   int func = (ENABLE_SRC_BLND_FACTOR|ENABLE_DST_BLND_FACTOR);

   FLUSH_BATCH(imesa);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s %s %s\n", __FUNCTION__,
	      gl_lookup_enum_by_nr(sfactor),
	      gl_lookup_enum_by_nr(dfactor));

   switch(sfactor) {
   case GL_ZERO: func |= SRC_BLND_FACT(BLENDFACT_ZERO); break;
   case GL_SRC_ALPHA: func |= SRC_BLND_FACT(BLENDFACT_SRC_ALPHA); break;
   case GL_ONE: func |= SRC_BLND_FACT(BLENDFACT_ONE); break;
   case GL_DST_COLOR: func |= SRC_BLND_FACT(BLENDFACT_DST_COLR); break;
   case GL_ONE_MINUS_DST_COLOR: 
      		      func |= SRC_BLND_FACT(BLENDFACT_INV_DST_COLR); break;
   case GL_ONE_MINUS_SRC_ALPHA:
		      func |= SRC_BLND_FACT(BLENDFACT_INV_SRC_ALPHA); break;
   case GL_DST_ALPHA: func |= SRC_BLND_FACT(BLENDFACT_DST_ALPHA); break;
   case GL_ONE_MINUS_DST_ALPHA:
		      func |= SRC_BLND_FACT(BLENDFACT_INV_DST_ALPHA); break;
   case GL_SRC_ALPHA_SATURATE: 
		      func |= SRC_BLND_FACT(BLENDFACT_SRC_ALPHA_SATURATE);
		      break;
   case GL_CONSTANT_COLOR_EXT:
		      func |= SRC_BLND_FACT(BLENDFACT_CONST_COLOR); break;
   case GL_ONE_MINUS_CONSTANT_COLOR_EXT:
		      func |= SRC_BLND_FACT(BLENDFACT_INV_CONST_COLOR);
		      break;
   case GL_CONSTANT_ALPHA_EXT:
		      func |= SRC_BLND_FACT(BLENDFACT_CONST_ALPHA); break;
   case GL_ONE_MINUS_CONSTANT_ALPHA_EXT:
		      func |= SRC_BLND_FACT(BLENDFACT_INV_CONST_ALPHA);
		      break;
   default: return;
   }

   switch(dfactor) {
   case GL_SRC_ALPHA: func |= DST_BLND_FACT(BLENDFACT_SRC_ALPHA); break;
   case GL_ONE_MINUS_SRC_ALPHA: 
		      func |= DST_BLND_FACT(BLENDFACT_INV_SRC_ALPHA); break;
   case GL_ZERO: func |= DST_BLND_FACT(BLENDFACT_ZERO); break;
   case GL_ONE: func |= DST_BLND_FACT(BLENDFACT_ONE); break;
   case GL_SRC_COLOR: func |= DST_BLND_FACT(BLENDFACT_SRC_COLR); break;
   case GL_ONE_MINUS_SRC_COLOR: 
		      func |= DST_BLND_FACT(BLENDFACT_INV_SRC_COLR); break;
   case GL_DST_ALPHA: func |= DST_BLND_FACT(BLENDFACT_DST_ALPHA); break;
   case GL_ONE_MINUS_DST_ALPHA: 
		      func |= DST_BLND_FACT(BLENDFACT_INV_DST_ALPHA); break;
   case GL_CONSTANT_COLOR_EXT:
		      func |= DST_BLND_FACT(BLENDFACT_CONST_COLOR); break;
   case GL_ONE_MINUS_CONSTANT_COLOR_EXT:
		      func |= DST_BLND_FACT(BLENDFACT_INV_CONST_COLOR);
		      break;
   case GL_CONSTANT_ALPHA_EXT:
		      func |= DST_BLND_FACT(BLENDFACT_CONST_ALPHA); break;
   case GL_ONE_MINUS_CONSTANT_ALPHA_EXT:
		      func |= DST_BLND_FACT(BLENDFACT_INV_CONST_ALPHA); 
		      break;
   default: return;
   }

   imesa->dirty |= I830_UPLOAD_CTX;
   imesa->Setup[I830_CTXREG_IALPHAB] &= ~SRC_DST_ABLEND_MASK;
   imesa->Setup[I830_CTXREG_STATE1] &= ~SRC_DST_BLND_MASK;
   imesa->Setup[I830_CTXREG_STATE1] |= func;
   /* Insure Independant Alpha Blend is really disabled if need be */
   i830EvalLogicOpBlendState(ctx);

   if(0) fprintf(stderr, "STATE1 : 0x%x\n", imesa->Setup[I830_CTXREG_STATE1]);
}

static void i830DDDepthFunc(GLcontext *ctx, GLenum func)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   int test = 0;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   FLUSH_BATCH(imesa);

   switch(func) {
   case GL_NEVER:
      test = COMPAREFUNC_NEVER;
      break;
   case GL_LESS:
      test = COMPAREFUNC_LESS;
      break;
   case GL_LEQUAL:
      test = COMPAREFUNC_LEQUAL;
      break;
   case GL_GREATER:
      test = COMPAREFUNC_GREATER;
      break;
   case GL_GEQUAL:
      test = COMPAREFUNC_GEQUAL;
      break;
   case GL_NOTEQUAL:
      test = COMPAREFUNC_NOTEQUAL;
      break;
   case GL_EQUAL:
      test = COMPAREFUNC_EQUAL;
      break;
   case GL_ALWAYS:
      test = COMPAREFUNC_ALWAYS;
      break;
   default: return;
   }

   imesa->dirty |= I830_UPLOAD_CTX;
   imesa->Setup[I830_CTXREG_STATE3] &= ~DEPTH_TEST_FUNC_MASK;
   imesa->Setup[I830_CTXREG_STATE3] |= (ENABLE_DEPTH_TEST_FUNC |
				       DEPTH_TEST_FUNC(test));
}

static void i830DDDepthMask(GLcontext *ctx, GLboolean flag)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s flag (%d)\n", __FUNCTION__, flag);

   FLUSH_BATCH(imesa);

   imesa->dirty |= I830_UPLOAD_CTX;
   imesa->Setup[I830_CTXREG_ENABLES_2] &= ~ENABLE_DIS_DEPTH_WRITE_MASK;

   if (flag)
     imesa->Setup[I830_CTXREG_ENABLES_2] |= ENABLE_DEPTH_WRITE;
   else
     imesa->Setup[I830_CTXREG_ENABLES_2] |= DISABLE_DEPTH_WRITE;
}

/* =============================================================
 * Hardware clipping
 */

static void i830DDScissor( GLcontext *ctx, GLint x, GLint y,
			   GLsizei w, GLsizei h )
{ 
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   int x1 = x;
   int y1 = imesa->driDrawable->h - (y + h);
   int x2 = x + w - 1;
   int y2 = y1 + h - 1;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "[%s] x(%d) y(%d) w(%d) h(%d)\n", __FUNCTION__,
	      x, y, w, h);

   if(x1 < 0) x1 = 0;
   if(y1 < 0) y1 = 0;
   if(x2 < 0) x2 = 0;
   if(y2 < 0) y2 = 0;

   FLUSH_BATCH(imesa);
   imesa->dirty |= I830_UPLOAD_BUFFERS;
   imesa->BufferSetup[I830_DESTREG_SR1] = (y1 << 16) | (x1 & 0xffff);
   imesa->BufferSetup[I830_DESTREG_SR2] = (y2 << 16) | (x2 & 0xffff);
}

static void i830DDLogicOp( GLcontext *ctx, GLenum opcode )
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   int tmp = 0;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);


   FLUSH_BATCH( imesa );

   switch(opcode) {
   case GL_CLEAR: tmp = LOGICOP_CLEAR; break;
   case GL_AND: tmp = LOGICOP_AND; break;
   case GL_AND_REVERSE: tmp = LOGICOP_AND_RVRSE; break;
   case GL_COPY: tmp = LOGICOP_COPY; break;
   case GL_COPY_INVERTED: tmp = LOGICOP_COPY_INV; break;
   case GL_AND_INVERTED: tmp = LOGICOP_AND_INV; break;
   case GL_NOOP: tmp = LOGICOP_NOOP; break;
   case GL_XOR: tmp = LOGICOP_XOR; break;
   case GL_OR: tmp = LOGICOP_OR; break;
   case GL_OR_INVERTED: tmp = LOGICOP_OR_INV; break;
   case GL_NOR: tmp = LOGICOP_NOR; break;
   case GL_EQUIV: tmp = LOGICOP_EQUIV; break;
   case GL_INVERT: tmp = LOGICOP_INV; break;
   case GL_OR_REVERSE: tmp = LOGICOP_OR_RVRSE; break;
   case GL_NAND: tmp = LOGICOP_NAND; break;
   case GL_SET: tmp = LOGICOP_SET; break;
   default: return;
   }

   imesa->dirty |= I830_UPLOAD_CTX;
   imesa->Setup[I830_CTXREG_STATE4] &= ~LOGICOP_MASK;
   imesa->Setup[I830_CTXREG_STATE4] |= LOGIC_OP_FUNC(tmp);
   if(0) fprintf(stderr, "Logicop : 0x%x, state4 : 0x%x\n", tmp, imesa->Setup[I830_CTXREG_STATE4]);
   /* Insure all the enables are correct */
   i830EvalLogicOpBlendState(ctx);
}

static GLboolean i830DDSetDrawBuffer(GLcontext *ctx, GLenum mode )
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   FLUSH_BATCH(imesa);

   imesa->Fallback &= ~I830_FALLBACK_DRAW_BUFFER;
   
   if(mode == GL_FRONT_LEFT) {
      imesa->readMap = (char *)imesa->driScreen->pFB;
      imesa->drawMap = (char *)imesa->driScreen->pFB;
      imesa->BufferSetup[I830_DESTREG_CBUFADDR] = 
		imesa->i830Screen->fbOffset;
      imesa->dirty |= I830_UPLOAD_BUFFERS;
      i830XMesaSetFrontClipRects( imesa );
      return GL_TRUE;
   } else if(mode == GL_BACK_LEFT) {
      imesa->readMap = imesa->i830Screen->back.map;
      imesa->drawMap = (char *)imesa->i830Screen->back.map;
      imesa->BufferSetup[I830_DESTREG_CBUFADDR] = 
		imesa->i830Screen->backOffset;
      imesa->dirty |= I830_UPLOAD_BUFFERS;
      i830XMesaSetBackClipRects( imesa );
      return GL_TRUE;
   }

   imesa->Fallback |= I830_FALLBACK_DRAW_BUFFER;
   return GL_FALSE;
}

static void i830DDSetReadBuffer(GLcontext *ctx, GLframebuffer *colorBuffer,
				GLenum mode )
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   if(mode == GL_FRONT_LEFT) {
      imesa->readMap = (char *)imesa->driScreen->pFB;
      imesa->Fallback &= ~I830_FALLBACK_READ_BUFFER;
   } else if(mode == GL_BACK_LEFT) {
      imesa->readMap = imesa->i830Screen->back.map;
      imesa->Fallback &= ~I830_FALLBACK_READ_BUFFER;
   } else {
      imesa->Fallback |= I830_FALLBACK_READ_BUFFER;
   }
}

static void i830DDSetColor(GLcontext *ctx, 
			   GLubyte r, GLubyte g,
			   GLubyte b, GLubyte a )
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s r(%d) g(%d) b(%d) a(%d)\n", __FUNCTION__,
	      r, g, b, a);

   imesa->MonoColor = i830PackColor( imesa->i830Screen->fbFormat, r, g, b, a );
   if(I830_DEBUG&DEBUG_VERBOSE_STATE)
     fprintf(stderr, "[%s] MonoColor = 0x%08x\n", __FUNCTION__,
	     imesa->MonoColor);
}


static void i830DDClearColor(GLcontext *ctx, 
			     GLubyte r, GLubyte g,
			     GLubyte b, GLubyte a )
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s r(%d) g(%d) b(%d) a(%d)\n", __FUNCTION__,
	      r, g, b, a);

   imesa->clear_red = r;
   imesa->clear_green = g;
   imesa->clear_blue = b;
   imesa->clear_alpha = a;

   imesa->ClearColor = i830PackColor( imesa->i830Screen->fbFormat, r, g, b, a );
   if(I830_DEBUG&DEBUG_VERBOSE_STATE)
     fprintf(stderr, "[%s] ClearColor = 0x%08x\n", __FUNCTION__,
	     imesa->ClearColor);

}

static void i830DDCullFaceFrontFace(GLcontext *ctx, GLenum unused)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);
   GLuint mode = CULLMODE_BOTH;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   FLUSH_BATCH(imesa);

   if(ctx->Polygon.CullFaceMode != GL_FRONT_AND_BACK) {
      mode = CULLMODE_CW;

      if (ctx->Polygon.CullFaceMode == GL_FRONT)
	 mode ^= (CULLMODE_CW ^ CULLMODE_CCW);
      if (ctx->Polygon.FrontFace != GL_CCW)
	 mode ^= (CULLMODE_CW ^ CULLMODE_CCW);
   }

   imesa->LcsCullMode = mode;

   if(ctx->Polygon.CullFlag && ctx->PB->primitive == GL_POLYGON) {
      imesa->dirty |= I830_UPLOAD_CTX;
      imesa->Setup[I830_CTXREG_STATE3] &= ~CULLMODE_MASK;
      imesa->Setup[I830_CTXREG_STATE3] |= ENABLE_CULL_MODE | mode;
   }
}

static void i830DDReducedPrimitiveChange( GLcontext *ctx, GLenum prim )
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s %s\n", __FUNCTION__, gl_lookup_enum_by_nr(prim));

   FLUSH_BATCH(imesa);
   imesa->dirty |= I830_UPLOAD_CTX;
   imesa->Setup[I830_CTXREG_STATE3] &= ~CULLMODE_MASK;
   imesa->Setup[I830_CTXREG_AA] &= ~AA_LINE_ENABLE;
   imesa->vertex_prim = PRIM3D_TRILIST;

   switch(ctx->PB->primitive) {
   case GL_POLYGON:
      if (ctx->Polygon.CullFlag) 
	 imesa->Setup[I830_CTXREG_STATE3] |= (ENABLE_CULL_MODE |
					      imesa->LcsCullMode);
      else 
	 imesa->Setup[I830_CTXREG_STATE3] |= (ENABLE_CULL_MODE |
					      CULLMODE_NONE);
      break;

   case GL_LINE:
   case GL_LINES:
      imesa->vertex_prim = PRIM3D_LINELIST;

      if(ctx->Line.SmoothFlag)
	imesa->Setup[I830_CTXREG_AA] |= AA_LINE_ENABLE;
      imesa->Setup[I830_CTXREG_STATE3] |= CULLMODE_NONE;
      break;

   case GL_POINT:
   case GL_POINTS:
      imesa->vertex_prim = PRIM3D_POINTLIST;
      imesa->Setup[I830_CTXREG_STATE3] |= CULLMODE_NONE;
   default:
      imesa->Setup[I830_CTXREG_STATE3] |= CULLMODE_NONE;
      break;
   }

   if(I830_DEBUG&DEBUG_VERBOSE_STATE)
     fprintf(stderr, "[%s] AA(0x%08x) STATE3(0x%08x) vertex_prim 0x%x\n",
	     __FUNCTION__,
	     imesa->Setup[I830_CTXREG_AA],
	     imesa->Setup[I830_CTXREG_STATE3],
	     imesa->vertex_prim);

}

static void i830DDLineWidth( GLcontext *ctx, GLfloat widthf )
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );
   int width;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   width = FloatToInt(widthf * 2);
   CLAMP_SELF(width, 1, 15);
   imesa->Setup[I830_CTXREG_STATE5] &= ~FIXED_LINE_WIDTH_MASK;
   imesa->Setup[I830_CTXREG_STATE5] |= (ENABLE_FIXED_LINE_WIDTH |
				       FIXED_LINE_WIDTH(width));

   imesa->dirty |= I830_UPLOAD_CTX;
}

/* =============================================================
 * Color masks
 */

/* This only deals with ColorMask for rendering, clears need to update 
 * a planemask for the clearing blit.  This is to be done.
 */

static GLboolean i830DDColorMask(GLcontext *ctx,
				    GLboolean r, GLboolean g,
				    GLboolean b, GLboolean a )
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );
   GLuint tmp = 0;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
     fprintf(stderr, "%s r(%d) g(%d) b(%d) a(%d)\n", __FUNCTION__, r, g, b, a);

   imesa->mask_red = !r;
   imesa->mask_green = !g;
   imesa->mask_blue = !b;
   imesa->mask_alpha = !a;

   tmp = (imesa->Setup[I830_CTXREG_ENABLES_2] & ~WRITEMASK_MASK) |
	ENABLE_COLOR_MASK |
	ENABLE_COLOR_WRITE |
	((!r) << WRITEMASK_RED_SHIFT) |
	((!g) << WRITEMASK_GREEN_SHIFT) |
	((!b) << WRITEMASK_BLUE_SHIFT) |
	((!a) << WRITEMASK_ALPHA_SHIFT);

   if(tmp != imesa->Setup[I830_CTXREG_ENABLES_2]) {
      FLUSH_BATCH(imesa);
      imesa->dirty |= I830_UPLOAD_CTX;
      imesa->Setup[I830_CTXREG_ENABLES_2] = tmp;

      if(I830_DEBUG&DEBUG_VERBOSE_STATE)
	fprintf(stderr, "[%s] enables 2 = 0x%08x\n", __FUNCTION__, tmp);
   }

   /* Always return false so s/w fallbacks are correct */
   return GL_FALSE;
}

static void i830DDLightModelfv(GLcontext *ctx, GLenum pname, 
			      const GLfloat *param)
{
   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   if(pname == GL_LIGHT_MODEL_COLOR_CONTROL) {
      i830ContextPtr imesa = I830_CONTEXT( ctx );
      FLUSH_BATCH(imesa);
      imesa->dirty |= I830_UPLOAD_CTX;
      imesa->Setup[I830_CTXREG_ENABLES_1] &= ~ENABLE_SPEC_ADD_MASK;

      if(ctx->Texture.ReallyEnabled &&
	 ctx->Light.Enabled &&
	 ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)
	 imesa->Setup[I830_CTXREG_ENABLES_1] |= ENABLE_SPEC_ADD;
      else
	 imesa->Setup[I830_CTXREG_ENABLES_1] |= DISABLE_SPEC_ADD;

      if(I830_DEBUG&DEBUG_VERBOSE_STATE)
	fprintf(stderr, "[%s] Enables_1 = 0x%08x\n", __FUNCTION__, 
		imesa->Setup[I830_CTXREG_ENABLES_1]);

   }
}

/* =============================================================
 * Fog
 */
static void i830DDFogfv(GLcontext *ctx, GLenum pname, const GLfloat *param)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   if(pname == GL_FOG_COLOR) {
      GLuint color = (((GLubyte)(ctx->Fog.Color[0]*255.0F) << 16) |
		      ((GLubyte)(ctx->Fog.Color[1]*255.0F) << 8) |
		      ((GLubyte)(ctx->Fog.Color[2]*255.0F) << 0));

      imesa->dirty |= I830_UPLOAD_CTX;
      imesa->Setup[I830_CTXREG_FOGCOLOR] = (STATE3D_FOG_COLOR_CMD | color);

      if(I830_DEBUG&DEBUG_VERBOSE_STATE)
	fprintf(stderr, "[%s] FogColor = 0x%08x\n", __FUNCTION__, 
		imesa->Setup[I830_CTXREG_FOGCOLOR]);

   }
}


/* =============================================================
 */

static void i830DDEnable(GLcontext *ctx, GLenum cap, GLboolean state)
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s cap(%s) state(%d)\n", __FUNCTION__,
	      gl_lookup_enum_by_nr(cap), state);

   switch(cap) {
   case GL_LIGHTING:
      FLUSH_BATCH(imesa);
      imesa->dirty |= I830_UPLOAD_CTX;
      imesa->Setup[I830_CTXREG_ENABLES_1] &= ~ENABLE_SPEC_ADD_MASK;

      if (ctx->Texture.ReallyEnabled &&
	  ctx->Light.Enabled &&
	  ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)
	 imesa->Setup[I830_CTXREG_ENABLES_1] |= ENABLE_SPEC_ADD;
      else
	 imesa->Setup[I830_CTXREG_ENABLES_1] |= DISABLE_SPEC_ADD;

      break;
   case GL_ALPHA_TEST:
      FLUSH_BATCH(imesa);
      imesa->dirty |= I830_UPLOAD_CTX;
      imesa->Setup[I830_CTXREG_ENABLES_1] &= ~ENABLE_DIS_ALPHA_TEST_MASK;
      if(state)
	imesa->Setup[I830_CTXREG_ENABLES_1] |= ENABLE_ALPHA_TEST;
      else
	imesa->Setup[I830_CTXREG_ENABLES_1] |= DISABLE_ALPHA_TEST;
      break;

   case GL_BLEND:
   case GL_COLOR_LOGIC_OP:
   case GL_INDEX_LOGIC_OP:
      i830EvalLogicOpBlendState(ctx);
      break;
   case GL_DITHER:
      {
	 unsigned int temp;

	 FLUSH_BATCH(imesa);
	 temp = imesa->Setup[I830_CTXREG_ENABLES_2];

	 temp &= ~ENABLE_DITHER;

	 if(state) temp |= ENABLE_DITHER;
	 else temp |= DISABLE_DITHER;

	 if(temp != imesa->Setup[I830_CTXREG_ENABLES_2]) {
	    imesa->dirty |= I830_UPLOAD_CTX;
	    imesa->Setup[I830_CTXREG_ENABLES_2] = temp;
	 }
      }
      break;

   case GL_DEPTH_TEST:
      FLUSH_BATCH(imesa);
      imesa->dirty |= I830_UPLOAD_CTX;
      imesa->Setup[I830_CTXREG_ENABLES_1] &= ~ENABLE_DIS_DEPTH_TEST_MASK;
      
      if(state)
	imesa->Setup[I830_CTXREG_ENABLES_1] |= ENABLE_DEPTH_TEST;
      else
	imesa->Setup[I830_CTXREG_ENABLES_1] |= DISABLE_DEPTH_TEST;
      break;

   case GL_SCISSOR_TEST:
      FLUSH_BATCH(imesa);

      if(state)
	imesa->BufferSetup[I830_DESTREG_SENABLE] = (STATE3D_SCISSOR_ENABLE_CMD |
						  ENABLE_SCISSOR_RECT);
      else
	imesa->BufferSetup[I830_DESTREG_SENABLE] = (STATE3D_SCISSOR_ENABLE_CMD |
						  DISABLE_SCISSOR_RECT);
      imesa->dirty |= I830_UPLOAD_BUFFERS;
      break;

   case GL_POLYGON_STIPPLE:
      if(ctx->PB->primitive == GL_POLYGON) {
	 FLUSH_BATCH(imesa);
	 /* Need a fallback here */
      }
      break;

   case GL_LINE_SMOOTH:
      if (ctx->PB->primitive == GL_LINE) {
	 FLUSH_BATCH(imesa);
	 if(0) fprintf(stderr, "Line smooth hit\n");
	 imesa->dirty |= I830_UPLOAD_CTX;
	 imesa->Setup[I830_CTXREG_AA] &= ~AA_LINE_ENABLE;
	 /* imesa->Setup[I830_CTXREG_LCS] &= ~LCS_LINEWIDTH_0_5; */
	 if (state) {
	    imesa->Setup[I830_CTXREG_AA] |= AA_LINE_ENABLE;
	    /* imesa->Setup[I830_CTXREG_LCS] |= LCS_LINEWIDTH_0_5; */
	 } else {
	    imesa->Setup[I830_CTXREG_AA] |= AA_LINE_DISABLE;
	    /* imesa->Setup[I830_CTXREG_LCS] |= LCS_LINEWIDTH_0_5; */
	 }
      }
      break;

   case GL_POINT_SMOOTH:
      if (ctx->PB->primitive == GL_POINT) {
	 FLUSH_BATCH(imesa);
	 if(0) fprintf(stderr, "Point smooth hit\n");
      }
      break;

   case GL_POLYGON_SMOOTH:
      if (ctx->PB->primitive == GL_POLYGON) {
	 FLUSH_BATCH(imesa);
	 if(0) fprintf(stderr, "Polygon Smooth hit\n");
      }
      break;

   case GL_FOG:
      FLUSH_BATCH(imesa);
      imesa->dirty |= I830_UPLOAD_CTX;
      imesa->Setup[I830_CTXREG_ENABLES_1] &= ~ENABLE_DIS_FOG_MASK;
      if(state)
	imesa->Setup[I830_CTXREG_ENABLES_1] |= I830_ENABLE_FOG;
      else
	imesa->Setup[I830_CTXREG_ENABLES_1] |= I830_DISABLE_FOG;
      break;

   case GL_CULL_FACE:
      if (ctx->PB->primitive == GL_POLYGON) {
	 FLUSH_BATCH(imesa);
	 imesa->dirty |= I830_UPLOAD_CTX;
	 imesa->Setup[I830_CTXREG_STATE3] &= ~CULLMODE_MASK;
	 if (state)
	    imesa->Setup[I830_CTXREG_STATE3] |= (ENABLE_CULL_MODE |
						imesa->LcsCullMode);
	 else
	    imesa->Setup[I830_CTXREG_STATE3] |= (ENABLE_CULL_MODE |
						CULLMODE_NONE);
      }
      break;
   case GL_TEXTURE_1D:      
   case GL_TEXTURE_3D:      
      FLUSH_BATCH(imesa);
      imesa->new_state |= I830_NEW_TEXTURE;
      break;
   case GL_TEXTURE_2D:
      FLUSH_BATCH(imesa);
      imesa->new_state |= I830_NEW_TEXTURE;

      imesa->dirty |= I830_UPLOAD_CTX;
      imesa->Setup[I830_CTXREG_ENABLES_1] &= ~ENABLE_SPEC_ADD_MASK;

      if (ctx->Texture.ReallyEnabled &&
	  ctx->Light.Enabled &&
	  ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)
	 imesa->Setup[I830_CTXREG_ENABLES_1] |= ENABLE_SPEC_ADD;
      else
	 imesa->Setup[I830_CTXREG_ENABLES_1] |= DISABLE_SPEC_ADD;
      break;

   case GL_STENCIL_TEST:
      FLUSH_BATCH(imesa);
      if(imesa->hw_stencil) {
	 imesa->dirty |= I830_UPLOAD_CTX;
	 imesa->Setup[I830_CTXREG_ENABLES_1] &= ~ENABLE_STENCIL_TEST;

	 if(state) {
	    if(0) fprintf(stderr, "Enabling stencil test\n");
	    imesa->Setup[I830_CTXREG_ENABLES_1] |= ENABLE_STENCIL_TEST;
	 } else {
	    if(0) fprintf(stderr, "Disabling stencil test\n");
	    imesa->Setup[I830_CTXREG_ENABLES_1] |= DISABLE_STENCIL_TEST;
	 }

	 if(I830_DEBUG&DEBUG_VERBOSE_STATE)
	   fprintf(stderr, "%s : state4 : 0x%x, stentst : 0x%x,"
		   " enables_1 : 0x%x\n", __FUNCTION__,
		   imesa->Setup[I830_CTXREG_STATE4],
		   imesa->Setup[I830_CTXREG_STENCILTST],
		   imesa->Setup[I830_CTXREG_ENABLES_1]);

      } else if (state) {
	 imesa->Fallback |= I830_FALLBACK_STENCIL;
      } else {
	 imesa->Fallback &= ~I830_FALLBACK_STENCIL;
      }
	
   default:
      ; 
   }    
}



/* =============================================================
 */


void i830DDUpdateHwState( GLcontext *ctx )
{
   i830ContextPtr imesa = I830_CONTEXT(ctx);

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   if(imesa->new_state & I830_NEW_TEXTURE) {
      FLUSH_BATCH(imesa);
      i830UpdateTextureState( ctx );
   }

   imesa->new_state = 0;
}


void i830EmitDrawingRectangle( i830ContextPtr imesa )
{
   __DRIdrawablePrivate *dPriv = imesa->driDrawable;
   i830ScreenPrivate *i830Screen = imesa->i830Screen;
   int x0 = imesa->drawX;
   int y0 = imesa->drawY;
   int x1 = x0 + dPriv->w;
   int y1 = y0 + dPriv->h;

   /* Don't set drawing rectangle */
   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s x0(%d) x1(%d) y0(%d) y1(%d)\n", __FUNCTION__,
	      x0, x1, y0, y1);

   /* Coordinate origin of the window - may be offscreen.
    */
   imesa->BufferSetup[I830_DESTREG_DR4] = ((y0<<16) | 
					   (((unsigned)x0)&0xFFFF));
  
   /* Clip to screen.
    */
   if (x0 < 0) x0 = 0;
   if (y0 < 0) y0 = 0;
   if (x1 > i830Screen->width-1) x1 = i830Screen->width-1;
   if (y1 > i830Screen->height-1) y1 = i830Screen->height-1;


   /* Onscreen drawing rectangle.
    */
   imesa->BufferSetup[I830_DESTREG_DR2] = ((y0<<16) | x0);
   imesa->BufferSetup[I830_DESTREG_DR3] = (((y1+1)<<16) | (x1+1));
   imesa->dirty |= I830_UPLOAD_BUFFERS;

   if(I830_DEBUG&DEBUG_VERBOSE_STATE)
      fprintf(stderr, "[%s] DR2(0x%08x) DR3(0x%08x) DR4(0x%08x)\n",
	      __FUNCTION__,
	      imesa->BufferSetup[I830_DESTREG_DR2],
	      imesa->BufferSetup[I830_DESTREG_DR3],
	      imesa->BufferSetup[I830_DESTREG_DR4]);
}



static void i830DDPrintDirty( const char *msg, GLuint state )
{
   fprintf(stderr, "%s (0x%x): %s%s%s%s%s\n",	   
	   msg,
	   (unsigned int) state,
	   (state & I830_UPLOAD_TEX0_IMAGE)  ? "upload-tex0, " : "",
	   (state & I830_UPLOAD_TEX1_IMAGE)  ? "upload-tex1, " : "",
	   (state & I830_UPLOAD_CTX)        ? "upload-ctx, " : "",
	   (state & I830_UPLOAD_BUFFERS)    ? "upload-bufs, " : "",
	   (state & I830_UPLOAD_CLIPRECTS)  ? "upload-cliprects, " : ""
	   );
}


/* Push the state into the sarea and/or texture memory.
 */
void i830EmitHwStateLocked( i830ContextPtr imesa )
{
   int i;
   if (I830_DEBUG & DEBUG_VERBOSE_API)
      i830DDPrintDirty( "\n\n\ni830EmitHwStateLocked", imesa->dirty );

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   if (imesa->dirty & ~I830_UPLOAD_CLIPRECTS) {
      if ((imesa->dirty & I830_UPLOAD_TEX0_IMAGE) && imesa->CurrentTexObj[0])
	 i830UploadTexImages(imesa, imesa->CurrentTexObj[0]);
   
      if ((imesa->dirty & I830_UPLOAD_TEX1_IMAGE) && imesa->CurrentTexObj[1])
	 i830UploadTexImages(imesa, imesa->CurrentTexObj[1]);
  
      if (imesa->dirty & I830_UPLOAD_CTX)
	 memcpy( imesa->sarea->ContextState, 
		 imesa->Setup, 
		 sizeof(imesa->Setup) );

      for(i = 0; i < I830_TEXTURE_COUNT; i++) {
	 if ((imesa->dirty & I830_UPLOAD_TEX_N(i)) && imesa->CurrentTexObj[i]) {
	    imesa->sarea->dirty |= I830_UPLOAD_TEX_N(i);
	    memcpy(imesa->sarea->TexState[i],
	       imesa->CurrentTexObj[i]->Setup,
	       sizeof(imesa->sarea->TexState[i]));
	 }
      }

      /* Need to figure out if texturing state, or enable changed. */

      for(i = 0; i < I830_TEXBLEND_COUNT; i++) {
	 if (imesa->dirty & I830_UPLOAD_TEXBLEND_N(i)) {
	    imesa->sarea->dirty |= I830_UPLOAD_TEXBLEND_N(i);
	    memcpy(imesa->sarea->TexBlendState[i],
	       imesa->TexBlend[i],
	       imesa->TexBlendWordsUsed[i] * 4);
	    imesa->sarea->TexBlendStateWordsUsed[i] =
	       imesa->TexBlendWordsUsed[i];
	 }
      }

      if (imesa->dirty & I830_UPLOAD_BUFFERS) 
	 memcpy( imesa->sarea->BufferState, 
		 imesa->BufferSetup, 
		 sizeof(imesa->BufferSetup) );

      if (imesa->dirty & I830_UPLOAD_TEX_PALETTE_SHARED) {
	 memcpy( imesa->sarea->Palette[0],
		 imesa->palette,
		 sizeof(imesa->sarea->Palette[0]));
      } else {
	 i830TextureObjectPtr p;

	 if (imesa->dirty & I830_UPLOAD_TEX_PALETTE_N(0)) {
	    p = imesa->CurrentTexObj[0];
	    memcpy( imesa->sarea->Palette[0],
		    p->palette,
		    sizeof(imesa->sarea->Palette[0]));
	 }
	 if (imesa->dirty & I830_UPLOAD_TEX_PALETTE_N(1)) {
	    p = imesa->CurrentTexObj[1];
	    memcpy( imesa->sarea->Palette[1],
		    p->palette,
		    sizeof(imesa->sarea->Palette[1]));
	 }
      }

      imesa->sarea->dirty |= (imesa->dirty & 
			      ~(I830_UPLOAD_TEX_MASK | 
				I830_UPLOAD_TEXBLEND_MASK));
      imesa->dirty &= I830_UPLOAD_CLIPRECTS;
   }
}

void i830DDInitState( i830ContextPtr imesa )
{
   i830ScreenPrivate *i830Screen = imesa->i830Screen;
   int i, j;

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   imesa->clear_red = 0;
   imesa->clear_green = 0;
   imesa->clear_blue = 0;
   imesa->clear_alpha = 0;

   imesa->mask_red = GL_FALSE;
   imesa->mask_green = GL_FALSE;
   imesa->mask_blue = GL_FALSE;
   imesa->mask_alpha = GL_FALSE;

   /* Zero all texture state */
   for(i = 0; i < I830_TEXBLEND_COUNT; i++) {
      for(j = 0; j < I830_TEXBLEND_SIZE; j++) {
	 imesa->TexBlend[i][j] = 0;
	 imesa->Init_TexBlend[i][j] = 0;
      }
      imesa->TexBlendWordsUsed[i] = 0;
      imesa->Init_TexBlendWordsUsed[i] = 0;
      imesa->TexBlendColorPipeNum[i] = 0;
      imesa->Init_TexBlendColorPipeNum[i] = 0;
   }

   /* Set default blend state */
   imesa->TexBlend[0][0] = (STATE3D_MAP_BLEND_OP_CMD(0) |
			    TEXPIPE_COLOR |
			    ENABLE_TEXOUTPUT_WRT_SEL |
			    TEXOP_OUTPUT_CURRENT |
			    DISABLE_TEX_CNTRL_STAGE |
			    TEXOP_SCALE_1X |
			    TEXOP_MODIFY_PARMS |
			    TEXOP_LAST_STAGE |
			    TEXBLENDOP_ARG1);
   imesa->TexBlend[0][1] = (STATE3D_MAP_BLEND_OP_CMD(0) |
			    TEXPIPE_ALPHA |
			    ENABLE_TEXOUTPUT_WRT_SEL |
			    TEXOP_OUTPUT_CURRENT |
			    TEXOP_SCALE_1X |
			    TEXOP_MODIFY_PARMS |
			    TEXBLENDOP_ARG1);
   imesa->TexBlend[0][2] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
			    TEXPIPE_COLOR |
			    TEXBLEND_ARG1 |
			    TEXBLENDARG_MODIFY_PARMS |
			    TEXBLENDARG_DIFFUSE);
   imesa->TexBlend[0][3] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
			    TEXPIPE_ALPHA |
			    TEXBLEND_ARG1 |
			    TEXBLENDARG_MODIFY_PARMS |
			    TEXBLENDARG_DIFFUSE);

   imesa->TexBlendWordsUsed[0] = 4;
   imesa->TexBlendColorPipeNum[0] = 0;

   imesa->Init_TexBlend[0][0] = (STATE3D_MAP_BLEND_OP_CMD(0) |
			    TEXPIPE_COLOR |
			    ENABLE_TEXOUTPUT_WRT_SEL |
			    TEXOP_OUTPUT_CURRENT |
			    DISABLE_TEX_CNTRL_STAGE |
			    TEXOP_SCALE_1X |
			    TEXOP_MODIFY_PARMS |
			    TEXOP_LAST_STAGE |
			    TEXBLENDOP_ARG1);
   imesa->Init_TexBlend[0][1] = (STATE3D_MAP_BLEND_OP_CMD(0) |
			    TEXPIPE_ALPHA |
			    ENABLE_TEXOUTPUT_WRT_SEL |
			    TEXOP_OUTPUT_CURRENT |
			    TEXOP_SCALE_1X |
			    TEXOP_MODIFY_PARMS |
			    TEXBLENDOP_ARG1);
   imesa->Init_TexBlend[0][2] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
			    TEXPIPE_COLOR |
			    TEXBLEND_ARG1 |
			    TEXBLENDARG_MODIFY_PARMS |
			    TEXBLENDARG_CURRENT);
   imesa->Init_TexBlend[0][3] = (STATE3D_MAP_BLEND_ARG_CMD(0) |
			    TEXPIPE_ALPHA |
			    TEXBLEND_ARG1 |
			    TEXBLENDARG_MODIFY_PARMS |
			    TEXBLENDARG_CURRENT);
   imesa->Init_TexBlendWordsUsed[0] = 4;
   imesa->Init_TexBlendColorPipeNum[0] = 0;

   memset(imesa->Setup, 0, sizeof(imesa->Setup));

   imesa->Setup[I830_CTXREG_VF] = VRTX_FORMAT_NTEX(1);

   imesa->Setup[I830_CTXREG_VF2] = (STATE3D_VERTEX_FORMAT_2_CMD |
				    VRTX_TEX_SET_0_FMT(TEXCOORDFMT_2D) |
				    VRTX_TEX_SET_1_FMT(TEXCOORDFMT_2D) |
				    VRTX_TEX_SET_2_FMT(TEXCOORDFMT_2D) |
				    VRTX_TEX_SET_3_FMT(TEXCOORDFMT_2D));

   imesa->Setup[I830_CTXREG_AA] = (STATE3D_AA_CMD |
				   AA_LINE_ECAAR_WIDTH_ENABLE |
				   AA_LINE_ECAAR_WIDTH_1_0 |
				   AA_LINE_REGION_WIDTH_ENABLE |
				   AA_LINE_REGION_WIDTH_1_0 | 
				   AA_LINE_DISABLE);

   imesa->Setup[I830_CTXREG_ENABLES_1] = (STATE3D_ENABLES_1_CMD |
					  DISABLE_LOGIC_OP |
					  DISABLE_STENCIL_TEST |
					  DISABLE_DEPTH_BIAS |
					  DISABLE_SPEC_ADD |
					  I830_DISABLE_FOG |
					  DISABLE_ALPHA_TEST |
					  DISABLE_COLOR_BLEND |
					  DISABLE_DEPTH_TEST);

   if(imesa->hw_stencil) {
      imesa->Setup[I830_CTXREG_ENABLES_2] = (STATE3D_ENABLES_2_CMD |
					     ENABLE_STENCIL_WRITE |
					     ENABLE_TEX_CACHE |
					     ENABLE_DITHER |
					     ENABLE_COLOR_MASK |
					     /* set no color comps disabled */
					     ENABLE_COLOR_WRITE |
					     ENABLE_DEPTH_WRITE);
   } else {
      imesa->Setup[I830_CTXREG_ENABLES_2] = (STATE3D_ENABLES_2_CMD |
					     DISABLE_STENCIL_WRITE |
					     ENABLE_TEX_CACHE |
					     ENABLE_DITHER |
					     ENABLE_COLOR_MASK |
					     /* set no color comps disabled */
					     ENABLE_COLOR_WRITE |
					     ENABLE_DEPTH_WRITE);
   }

   imesa->Setup[I830_CTXREG_STATE1] = (STATE3D_MODES_1_CMD |
				       ENABLE_COLR_BLND_FUNC |
				       BLENDFUNC_ADD |
				       ENABLE_SRC_BLND_FACTOR |
				       SRC_BLND_FACT(BLENDFACT_ONE) | 
				       ENABLE_DST_BLND_FACTOR |
				       DST_BLND_FACT(BLENDFACT_ZERO) );

   imesa->Setup[I830_CTXREG_STATE2] = (STATE3D_MODES_2_CMD |
				       ENABLE_GLOBAL_DEPTH_BIAS | 
				       GLOBAL_DEPTH_BIAS(0) |
				       ENABLE_ALPHA_TEST_FUNC | 
				       ALPHA_TEST_FUNC(COMPAREFUNC_ALWAYS) |
				       ALPHA_REF_VALUE(0) );

   imesa->Setup[I830_CTXREG_STATE3] = (STATE3D_MODES_3_CMD |
				       ENABLE_DEPTH_TEST_FUNC |
				       DEPTH_TEST_FUNC(COMPAREFUNC_LESS) |
				       ENABLE_ALPHA_SHADE_MODE |
				       ALPHA_SHADE_MODE(SHADE_MODE_LINEAR) |
				       ENABLE_FOG_SHADE_MODE |
				       FOG_SHADE_MODE(SHADE_MODE_LINEAR) |
				       ENABLE_SPEC_SHADE_MODE |
				       SPEC_SHADE_MODE(SHADE_MODE_LINEAR) |
				       ENABLE_COLOR_SHADE_MODE |
				       COLOR_SHADE_MODE(SHADE_MODE_LINEAR) |
				       ENABLE_CULL_MODE |
				       CULLMODE_NONE);

   imesa->Setup[I830_CTXREG_STATE4] = (STATE3D_MODES_4_CMD |
				       ENABLE_LOGIC_OP_FUNC |
				       LOGIC_OP_FUNC(LOGICOP_COPY) |
				       ENABLE_STENCIL_TEST_MASK |
				       STENCIL_TEST_MASK(0xff) |
				       ENABLE_STENCIL_WRITE_MASK |
				       STENCIL_WRITE_MASK(0xff));

   imesa->Setup[I830_CTXREG_STENCILTST] = (STATE3D_STENCIL_TEST_CMD |
				  ENABLE_STENCIL_PARMS |
				  STENCIL_FAIL_OP(STENCILOP_KEEP) |
				  STENCIL_PASS_DEPTH_FAIL_OP(STENCILOP_KEEP) |
				  STENCIL_PASS_DEPTH_PASS_OP(STENCILOP_KEEP) |
				  ENABLE_STENCIL_TEST_FUNC |
				  STENCIL_TEST_FUNC(COMPAREFUNC_ALWAYS) |
				  ENABLE_STENCIL_REF_VALUE |
				  STENCIL_REF_VALUE(0) );

   imesa->Setup[I830_CTXREG_STATE5] = (STATE3D_MODES_5_CMD |
				       FLUSH_TEXTURE_CACHE |
				       ENABLE_SPRITE_POINT_TEX |
				       SPRITE_POINT_TEX_OFF |
				       ENABLE_FIXED_LINE_WIDTH |
				       FIXED_LINE_WIDTH(0x2) | /* 1.0 */
				       ENABLE_FIXED_POINT_WIDTH |
				       FIXED_POINT_WIDTH(1) );

   imesa->Setup[I830_CTXREG_IALPHAB] = (STATE3D_INDPT_ALPHA_BLEND_CMD |
					DISABLE_INDPT_ALPHA_BLEND |
					ENABLE_ALPHA_BLENDFUNC |
					ABLENDFUNC_ADD);

   imesa->Setup[I830_CTXREG_FOGCOLOR] = (STATE3D_FOG_COLOR_CMD |
					 FOG_COLOR_RED(0) |
					 FOG_COLOR_GREEN(0) |
					 FOG_COLOR_BLUE(0));

   imesa->Setup[I830_CTXREG_BLENDCOLR0] = (STATE3D_CONST_BLEND_COLOR_CMD);

   imesa->Setup[I830_CTXREG_BLENDCOLR] = 0;

   imesa->Setup[I830_CTXREG_MCSB0] = STATE3D_MAP_COORD_SETBIND_CMD;
   imesa->Setup[I830_CTXREG_MCSB1] = (TEXBIND_SET3(TEXCOORDSRC_VTXSET_3) |
				      TEXBIND_SET2(TEXCOORDSRC_VTXSET_2) |
				      TEXBIND_SET1(TEXCOORDSRC_VTXSET_1) |
				      TEXBIND_SET0(TEXCOORDSRC_VTXSET_0));

   imesa->LcsCullMode = CULLMODE_CW; /* GL default */

   memset(imesa->BufferSetup, 0, sizeof(imesa->BufferSetup));


   if (imesa->glCtx->Color.DriverDrawBuffer == GL_BACK_LEFT) {
      imesa->drawMap = i830Screen->back.map;
      imesa->readMap = i830Screen->back.map;
      imesa->BufferSetup[I830_DESTREG_CBUFADDR] = i830Screen->backOffset;
      imesa->BufferSetup[I830_DESTREG_DBUFADDR] = 0;
   } else {
      imesa->drawMap = (char *)imesa->driScreen->pFB;
      imesa->readMap = (char *)imesa->driScreen->pFB;
      imesa->BufferSetup[I830_DESTREG_CBUFADDR] = i830Screen->fbOffset;
      imesa->BufferSetup[I830_DESTREG_DBUFADDR] = 0;      
   }

   imesa->BufferSetup[I830_DESTREG_DV0] = STATE3D_DST_BUF_VARS_CMD;

   switch (i830Screen->fbFormat) {
   case DV_PF_555:
   case DV_PF_565:
      imesa->BufferSetup[I830_DESTREG_DV1] = (DSTORG_HORT_BIAS(0x8) | /* .5 */
					      DSTORG_VERT_BIAS(0x8) | /* .5 */
					      i830Screen->fbFormat |
					      DEPTH_IS_Z |
					      DEPTH_FRMT_16_FIXED);
      break;
   case DV_PF_8888:
      imesa->BufferSetup[I830_DESTREG_DV1] = (DSTORG_HORT_BIAS(0x8) | /* .5 */
					      DSTORG_VERT_BIAS(0x8) | /* .5 */
					      i830Screen->fbFormat |
					      DEPTH_IS_Z |
					      DEPTH_FRMT_24_FIXED_8_OTHER);
      break;
   }
   imesa->BufferSetup[I830_DESTREG_SENABLE] = (STATE3D_SCISSOR_ENABLE_CMD |
					       DISABLE_SCISSOR_RECT);
   imesa->BufferSetup[I830_DESTREG_SR0] = STATE3D_SCISSOR_RECT_0_CMD;
   imesa->BufferSetup[I830_DESTREG_SR1] = 0;
   imesa->BufferSetup[I830_DESTREG_SR2] = 0;

   imesa->BufferSetup[I830_DESTREG_DR0] = STATE3D_DRAW_RECT_CMD;
   imesa->BufferSetup[I830_DESTREG_DR1] = 0;
   imesa->BufferSetup[I830_DESTREG_DR2] = 0;
   imesa->BufferSetup[I830_DESTREG_DR3] = (((i830Screen->height)<<16) | 
					  (i830Screen->width));
   imesa->BufferSetup[I830_DESTREG_DR4] = 0;

   memcpy( imesa->Init_Setup,
	   imesa->Setup, 
	   sizeof(imesa->Setup) );
   memcpy( imesa->Init_BufferSetup,
	   imesa->BufferSetup, 
	   sizeof(imesa->BufferSetup) );

}

#define INTERESTED (~(NEW_MODELVIEW|NEW_PROJECTION|\
                      NEW_TEXTURE_MATRIX|\
                      NEW_USER_CLIP|NEW_CLIENT_STATE))

void i830DDUpdateState( GLcontext *ctx )
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   /* Have to do this here to detect texture fallbacks in time */
   if (imesa->new_state & I830_NEW_TEXTURE)
      i830DDUpdateHwState( ctx );


   if (ctx->NewState & INTERESTED) {
      i830DDChooseRenderState(ctx);  
      i830ChooseRasterSetupFunc(ctx);
   }

   if (0) 
      fprintf(stderr, "IndirectTriangles %x Fallback %x\n", 
	      imesa->IndirectTriangles, imesa->Fallback);
   
   if (!imesa->Fallback) {
      ctx->IndirectTriangles &= ~DD_SW_RASTERIZE;
      ctx->IndirectTriangles |= imesa->IndirectTriangles;

      ctx->Driver.PointsFunc=imesa->PointsFunc;
      ctx->Driver.LineFunc=imesa->LineFunc;
      ctx->Driver.TriangleFunc=imesa->TriangleFunc;
      ctx->Driver.QuadFunc=imesa->QuadFunc;
   }
}


void i830DDInitStateFuncs(GLcontext *ctx)
{
   i830ContextPtr imesa = I830_CONTEXT( ctx );

   if(I830_DEBUG&DEBUG_VERBOSE_TRACE)
      fprintf(stderr, "%s\n", __FUNCTION__);

   ctx->Driver.UpdateState = i830DDUpdateState;
   ctx->Driver.Enable = i830DDEnable;
   ctx->Driver.AlphaFunc = i830DDAlphaFunc;
   ctx->Driver.BlendEquation = i830DDBlendEquation;
   ctx->Driver.BlendFunc = i830DDBlendFunc;
   ctx->Driver.BlendFuncSeparate = i830DDBlendFuncSeparate;
   ctx->Driver.BlendConstColor = i830DDBlendConstColor;
   ctx->Driver.DepthFunc = i830DDDepthFunc;
   ctx->Driver.DepthMask = i830DDDepthMask;
   ctx->Driver.Fogfv = i830DDFogfv;
   ctx->Driver.Scissor = i830DDScissor;
   ctx->Driver.CullFace = i830DDCullFaceFrontFace;
   ctx->Driver.FrontFace = i830DDCullFaceFrontFace;
   ctx->Driver.ColorMask = i830DDColorMask;
   ctx->Driver.ReducedPrimitiveChange = i830DDReducedPrimitiveChange;
   ctx->Driver.RenderStart = i830DDUpdateHwState; 
   ctx->Driver.RenderFinish = 0;

   ctx->Driver.LineStipple = 0;
   ctx->Driver.LineWidth = i830DDLineWidth;
   ctx->Driver.LogicOpcode = i830DDLogicOp;
   ctx->Driver.SetReadBuffer = i830DDSetReadBuffer;
   ctx->Driver.SetDrawBuffer = i830DDSetDrawBuffer;
   ctx->Driver.Color = i830DDSetColor;
   ctx->Driver.ClearColor = i830DDClearColor;
   ctx->Driver.Dither = NULL;
   ctx->Driver.Index = 0;
   ctx->Driver.ClearIndex = 0;
   ctx->Driver.IndexMask = 0;

   if(imesa->hw_stencil) {
      ctx->Driver.StencilFunc = i830DDStencilFunc;
      ctx->Driver.StencilMask = i830DDStencilMask;
      ctx->Driver.StencilOp = i830DDStencilOp;
   }

   ctx->Driver.LightModelfv = i830DDLightModelfv;
   ctx->Driver.PointSize = i830DDPointSize;
}
