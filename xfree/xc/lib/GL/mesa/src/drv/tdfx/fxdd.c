/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/fxdd.c,v 1.3 2000/12/08 19:36:23 alanh Exp $ */
/*
 * Mesa 3-D graphics library
 * Version:  3.3
 *
 * Copyright (C) 1999-2000  Brian Paul   All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * BRIAN PAUL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Original Mesa / 3Dfx device driver (C) 1999 David Bucciarelli, by the
 * terms stated above.
 *
 * Thank you for your contribution, David!
 *
 * Please make note of the above copyright/license statement.  If you
 * contributed code or bug fixes to this code under the previous (GNU
 * Library) license and object to the new license, your code will be
 * removed at your request.  Please see the Mesa docs/COPYRIGHT file
 * for more information.
 *
 * Additional Mesa/3Dfx driver developers:
 *   Daryll Strauss <daryll@precisioninsight.com>
 *   Keith Whitwell <keith@precisioninsight.com>
 *
 * See fxapi.h for more revision/author details.
 */


#include <dlfcn.h>
#include "image.h"
#include "types.h"
#include "fxdrv.h"
#include "fxsetup.h"
#include "fxpipeline.h"
#include "fxddtex.h"
#include "fxtexman.h"
#include "enums.h"
#include "extensions.h"
#include "pb.h"


/* These are used in calls to FX_grColorMaskv() */
static const GLboolean false4[4] = { GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE };
static const GLboolean true4[4] = { GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE };

#if	defined(FX_PXCONV_TABULAR)
/* These lookup table are used to extract RGB values in [0,255] from
 * 16-bit pixel values.
 */
GLubyte FX_PixelToRArray[0x10000];
GLubyte FX_PixelToGArray[0x10000];
GLubyte FX_PixelToBArray[0x10000];

/*
 * Initialize the FX_PixelTo{RGB} arrays.
 * Input: bgrOrder - if TRUE, pixels are in BGR order, else RGB order.
 */
void
fxInitPixelTables(fxMesaContext fxMesa, GLboolean bgrOrder)
{
    fxMesa->bgrOrder = bgrOrder;
   /*
    * We add a level of braces so that we can define the
    * variable pixel here.
    */
    {
        GLuint pixel = 0;
        for (pixel = 0; pixel <= 0xffff; pixel++) {
            GLuint r, g, b;
            if (bgrOrder) {
                r = (pixel & 0x001F) << 3;
                g = (pixel & 0x07E0) >> 3;
                b = (pixel & 0xF800) >> 8;
            }
            else {
                r = (pixel & 0xF800) >> 8;
                g = (pixel & 0x07E0) >> 3;
                b = (pixel & 0x001F) << 3;
            }
            r = r * 255 / 0xF8;     /* fill in low-order bits */
            g = g * 255 / 0xFC;
            b = b * 255 / 0xF8;
            FX_PixelToRArray[pixel] = r;
            FX_PixelToGArray[pixel] = g;
            FX_PixelToBArray[pixel] = b;
        }
    }
}
#endif	/* FX_PXCONV_TABULAR */

/**********************************************************************/
/*****                 Miscellaneous functions                    *****/
/**********************************************************************/


/* Return buffer size information */
static void
fxDDBufferSize(GLcontext * ctx, GLuint * width, GLuint * height)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDBufferSize(...) Start\n");
    }

    *width = fxMesa->width;
    *height = fxMesa->height;

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDBufferSize(...) End\n");
    }
}


/* Set current drawing color */
static void
fxDDSetColor(GLcontext * ctx, GLubyte red, GLubyte green,
             GLubyte blue, GLubyte alpha)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GLubyte col[4];
    ASSIGN_4V(col, red, green, blue, alpha);
    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDSetColor(%d,%d,%d,%d)\n", red, green,
                blue, alpha);
    }
    fxMesa->color = FXCOLOR4(col);
}


/* Implements glClearColor() */
static void
fxDDClearColor(GLcontext * ctx, GLubyte red, GLubyte green,
               GLubyte blue, GLubyte alpha)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GLubyte col[4];
    ASSIGN_4V(col, red, green, blue, 255);
    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDClearColor(%d,%d,%d,%d)\n", red, green,
                blue, alpha);
    }
    fxMesa->clearC = FXCOLOR4(col);
    fxMesa->clearA = alpha;
}


/* Clear the color and/or depth buffers */
static GLbitfield
fxDDClear(GLcontext * ctx, GLbitfield mask, GLboolean all,
          GLint x, GLint y, GLint width, GLint height)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    const GLuint colorMask = *((GLuint *) & ctx->Color.ColorMask);
    const FxU32 clearD = (FxU32) (ctx->Depth.Clear * fxMesa->depthClear);
    const FxU32 clearS = (FxU32) (ctx->Stencil.Clear);
    GLbitfield softwareMask = mask & (DD_ACCUM_BIT);
    GLuint stencil_size =
        fxMesa->haveHwStencil ? fxMesa->glVis->StencilBits : 0;

    /* we can't clear accum buffers */
    mask &= ~(DD_ACCUM_BIT);

    if ((mask & DD_STENCIL_BIT) && !fxMesa->haveHwStencil) {
        /* software stencil buffer */
        mask &= ~(DD_STENCIL_BIT);
        softwareMask |= DD_STENCIL_BIT;
    }

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDClear(%d,%d,%d,%d)\n", (int) x, (int) y,
                (int) width, (int) height);
    }

    if (colorMask != 0xffffffff) {
        /* do masked color buffer clears in software */
        softwareMask |= (mask & (DD_FRONT_LEFT_BIT | DD_BACK_LEFT_BIT));
        mask &= ~(DD_FRONT_LEFT_BIT | DD_BACK_LEFT_BIT);
    }

    if (fxMesa->haveHwStencil) {
        /*
         * If we want to clear stencil, it must be enabled
         * in the HW, even if the stencil test is not enabled
         * in the OGL state.
         */
        if (mask & DD_STENCIL_BIT) {
            FX_grStencilMask_NoLock(0xFF /* ctx->Stencil.WriteMask*/ );
            /* set stencil ref value = desired clear value */
            FX_grStencilFunc_NoLock(GR_CMP_ALWAYS, ctx->Stencil.Clear, 0xff);
            FX_grStencilOp_NoLock(GR_STENCILOP_REPLACE,
                           GR_STENCILOP_REPLACE, GR_STENCILOP_REPLACE);
            FX_grEnable_NoLock(GR_STENCIL_MODE_EXT);
        }
        else {
            FX_grDisable_NoLock(GR_STENCIL_MODE_EXT);
        }
    }

    BEGIN_CLIP_LOOP(fxMesa)

    /*
     * This could probably be done fancier but doing each possible case
     * explicitly is less error prone.
     */
    switch (mask & ~DD_STENCIL_BIT) {
    case DD_BACK_LEFT_BIT | DD_DEPTH_BIT:
        /* back buffer & depth */
        FX_grDepthMask_NoLock(FXTRUE);
        FX_grRenderBuffer_NoLock(GR_BUFFER_BACKBUFFER);
        if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->clearC, fxMesa->clearA, clearD,
                                       clearS);
        else
            FX_grBufferClear_NoLock(fxMesa->clearC, fxMesa->clearA, clearD);
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
            FX_grBufferClearExt_NoLock(fxMesa->clearC, fxMesa->clearA, clearD,
                                clearS);
        else
            FX_grBufferClear_NoLock(fxMesa->clearC, fxMesa->clearA, clearD);
        /* clear front */
        FX_grColorMaskv_NoLock(ctx, true4);
        FX_grRenderBuffer_NoLock(GR_BUFFER_FRONTBUFFER);
        if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->clearC, fxMesa->clearA, clearD,
                                clearS);
        else
            FX_grBufferClear_NoLock(fxMesa->clearC, fxMesa->clearA, clearD);
        if (!ctx->Depth.Mask || !ctx->Depth.Test) {
            FX_grDepthMask_NoLock(FXFALSE);
        }
        break;
    case DD_BACK_LEFT_BIT:
        /* back buffer only */
        FX_grDepthMask_NoLock(FXFALSE);
        FX_grRenderBuffer_NoLock(GR_BUFFER_BACKBUFFER);
        if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->clearC, fxMesa->clearA, clearD,
                                clearS);
        else
            FX_grBufferClear_NoLock(fxMesa->clearC, fxMesa->clearA, clearD);
        if (ctx->Depth.Mask && ctx->Depth.Test) {
            FX_grDepthMask_NoLock(FXTRUE);
        }
        break;
    case DD_FRONT_LEFT_BIT:
        /* front buffer only */
        FX_grDepthMask_NoLock(FXFALSE);
        FX_grRenderBuffer_NoLock(GR_BUFFER_FRONTBUFFER);
        if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->clearC, fxMesa->clearA, clearD,
                                clearS);
        else
            FX_grBufferClear_NoLock(fxMesa->clearC, fxMesa->clearA, clearD);
        if (ctx->Depth.Mask && ctx->Depth.Test) {
            FX_grDepthMask_NoLock(FXTRUE);
        }
        break;
    case DD_FRONT_LEFT_BIT | DD_BACK_LEFT_BIT:
        /* front and back */
        FX_grDepthMask_NoLock(FXFALSE);
        FX_grRenderBuffer_NoLock(GR_BUFFER_BACKBUFFER);
        if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->clearC, fxMesa->clearA, clearD,
                                clearS);
        else
            FX_grBufferClear_NoLock(fxMesa->clearC, fxMesa->clearA, clearD);
        FX_grRenderBuffer_NoLock(GR_BUFFER_FRONTBUFFER);
        if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->clearC, fxMesa->clearA, clearD,
                                clearS);
        else
            FX_grBufferClear_NoLock(fxMesa->clearC, fxMesa->clearA, clearD);
        if (ctx->Depth.Mask && ctx->Depth.Test) {
            FX_grDepthMask_NoLock(FXTRUE);
        }
        break;
    case DD_FRONT_LEFT_BIT | DD_BACK_LEFT_BIT | DD_DEPTH_BIT:
        /* clear front */
        FX_grDepthMask_NoLock(FXFALSE);
        FX_grRenderBuffer_NoLock(GR_BUFFER_FRONTBUFFER);
        if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->clearC, fxMesa->clearA, clearD,
                                clearS);
        else
            FX_grBufferClear_NoLock(fxMesa->clearC, fxMesa->clearA, clearD);
        /* clear back and depth */
        FX_grDepthMask_NoLock(FXTRUE);
        FX_grRenderBuffer_NoLock(GR_BUFFER_BACKBUFFER);
        if (stencil_size > 0)
            FX_grBufferClearExt_NoLock(fxMesa->clearC, fxMesa->clearA, clearD,
                                clearS);
        else
            FX_grBufferClear_NoLock(fxMesa->clearC, fxMesa->clearA, clearD);
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
            FX_grBufferClearExt_NoLock(fxMesa->clearC, fxMesa->clearA, clearD,
                                clearS);
        else
            FX_grBufferClear_NoLock(fxMesa->clearC, fxMesa->clearA, clearD);
        FX_grColorMaskv_NoLock(ctx, true4);
        if (ctx->Color.DrawDestMask & FRONT_LEFT_BIT)
            FX_grRenderBuffer_NoLock(GR_BUFFER_FRONTBUFFER);
        if (!ctx->Depth.Test || !ctx->Depth.Mask)
           FX_grDepthMask_NoLock(FXFALSE);
        break;
    default:
         /* clear no color buffers or depth buffer but might clear stencil */
	 if ((stencil_size > 0) && (mask & DD_STENCIL_BIT)) {
            FX_grDepthMask_NoLock(FXFALSE);
            FX_grColorMaskv_NoLock(ctx, false4);
            FX_grBufferClearExt_NoLock(fxMesa->clearC,
                                       fxMesa->clearA,
				       clearD,
                                       (FxU32) clearS);
            if (ctx->Depth.Mask && ctx->Depth.Test) {
               FX_grDepthMask_NoLock(FXTRUE);
            }
            FX_grColorMaskv_NoLock(ctx, true4);
         }
         break;
    }

    END_CLIP_LOOP(fxMesa);

    if (fxMesa->haveHwStencil) {
        if (ctx->Stencil.Enabled) {
            /* restore stencil state to as it was before the clear */
            GrStencil_t sfail = fxConvertGLStencilOp(ctx->Stencil.FailFunc);
            GrStencil_t zfail = fxConvertGLStencilOp(ctx->Stencil.ZFailFunc);
            GrStencil_t zpass = fxConvertGLStencilOp(ctx->Stencil.ZPassFunc);
            FX_grStencilOp_NoLock(sfail, zfail, zpass);
            FX_grStencilMask_NoLock(ctx->Stencil.WriteMask);
            FX_grStencilFunc_NoLock(ctx->Stencil.Function - GL_NEVER,
                             ctx->Stencil.Ref, ctx->Stencil.ValueMask);
            FX_grEnable_NoLock(GR_STENCIL_MODE_EXT);
        }
        else {
            FX_grDisable_NoLock(GR_STENCIL_MODE_EXT);
        }
    }

    return softwareMask;
}


/* Set the buffer used for drawing */
/* XXX support for separate read/draw buffers hasn't been tested */
static GLboolean
fxDDSetDrawBuffer(GLcontext * ctx, GLenum mode)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDSetBuffer(%x)\n", (int) mode);
    }

    if (mode == GL_FRONT_LEFT) {
        fxMesa->currentFB = GR_BUFFER_FRONTBUFFER;
        FX_grRenderBuffer(fxMesa, fxMesa->currentFB);
        return GL_TRUE;
    }
    else if (mode == GL_BACK_LEFT) {
        fxMesa->currentFB = GR_BUFFER_BACKBUFFER;
        FX_grRenderBuffer(fxMesa, fxMesa->currentFB);
        return GL_TRUE;
    }
    else if (mode == GL_NONE) {
        FX_grColorMaskv(ctx, false4);
        return GL_TRUE;
    }
    else {
        return GL_FALSE;
    }
}


/* Set the buffer used for reading */
/* XXX support for separate read/draw buffers hasn't been tested */
static void
fxDDSetReadBuffer(GLcontext * ctx, GLframebuffer * buffer, GLenum mode)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    (void) buffer;

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDSetBuffer(%x)\n", (int) mode);
    }

    if (mode == GL_FRONT_LEFT) {
        fxMesa->currentFB = GR_BUFFER_FRONTBUFFER;
        FX_grRenderBuffer(fxMesa, fxMesa->currentFB);
    }
    else if (mode == GL_BACK_LEFT) {
        fxMesa->currentFB = GR_BUFFER_BACKBUFFER;
        FX_grRenderBuffer(fxMesa, fxMesa->currentFB);
    }
}


/*
 * These functions just set new-state flags.  The exact state
 * values will be evaluated later.
 */
static void
fxDDStencilFunc(GLcontext * ctx, GLenum func, GLint ref, GLuint mask)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    (void) func;
    (void) ref;
    (void) mask;
    fxMesa->new_state |= FX_NEW_STENCIL;
    ctx->Driver.RenderStart = fxSetupFXUnits;
}

static void
fxDDStencilMask(GLcontext * ctx, GLuint mask)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    (void) mask;
    fxMesa->new_state |= FX_NEW_STENCIL;
    ctx->Driver.RenderStart = fxSetupFXUnits;
}

static void
fxDDStencilOp(GLcontext * ctx, GLenum sfail, GLenum zfail, GLenum zpass)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    (void) sfail;
    (void) zfail;
    (void) zpass;
    fxMesa->new_state |= FX_NEW_STENCIL;
    ctx->Driver.RenderStart = fxSetupFXUnits;
}

static void
fxDDDepthFunc(GLcontext * ctx, GLenum func)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    (void) func;
    fxMesa->new_state |= FX_NEW_DEPTH;
    ctx->Driver.RenderStart = fxSetupFXUnits;
}

static void
fxDDDepthMask(GLcontext * ctx, GLboolean mask)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    (void) mask;
    fxMesa->new_state |= FX_NEW_DEPTH;
    ctx->Driver.RenderStart = fxSetupFXUnits;
}



/*
 * Return the current value of the occlusion test flag and
 * reset the flag (hardware counters) to false.
 */
#if 0
static GLboolean
get_occlusion_result(GLcontext *ctx)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GLboolean result;

    BEGIN_BOARD_LOCK(fxMesa);

    if (ctx->Depth.OcclusionTest) {
        if (ctx->OcclusionResult) {
            result = GL_TRUE;  /* result of software rendering */
        }
        else {
           FxI32 zfail, in;
           zfail = FX_grGetInteger_NoLock(GR_STATS_PIXELS_DEPTHFUNC_FAIL);
           in = FX_grGetInteger_NoLock(GR_STATS_PIXELS_IN);
           if (in == zfail)
               result = GL_FALSE; /* geom was completely occluded */
           else
               result = GL_TRUE;  /* all or part of geom was visible */
        }
    }
    else {
        result = ctx->OcclusionResultSaved;
    }

    /* reset results now */
    grReset(GR_STATS_PIXELS);
    ctx->OcclusionResult = GL_FALSE;
    ctx->OcclusionResultSaved = GL_FALSE;

    END_BOARD_LOCK(fxMesa);

    return result;
}
#else

static GLboolean get_occlusion_result( GLcontext *ctx )
{
   fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
   GLboolean result;

   LOCK_HARDWARE( fxMesa );

   printf("start state: %d %d\n", ctx->Depth.OcclusionTest, ctx->OcclusionResult);

   if (ctx->Depth.OcclusionTest) {
      if (ctx->OcclusionResult) {
	 result = GL_TRUE;  /* result of software rendering */
         printf("res = true 1\n");
      }
      else {
	 FxI32 zfail, in;
	 zfail = FX_grGetInteger_NoLock(GR_STATS_PIXELS_DEPTHFUNC_FAIL);
	 in = FX_grGetInteger_NoLock(GR_STATS_PIXELS_IN);
	 if (in == zfail) {
	    result = GL_FALSE; /* geom was completely occluded */
         printf("res = false 2\n");
         }
	 else {
	    result = GL_TRUE;  /* all or part of geom was visible */
         printf("res = true 2\n");

         }
      }
   }
   else {
      result = ctx->OcclusionResultSaved;
         printf("res = saved\n");
   }

   /* reset results now */
   grReset(GR_STATS_PIXELS);
   ctx->OcclusionResult = GL_FALSE;
   ctx->OcclusionResultSaved = GL_FALSE;

   UNLOCK_HARDWARE( fxMesa );

   printf("result = %d\n", result);
   return result;
}

#endif

/*
 * We're only implementing this function to handle the
 * GL_OCCLUSTION_TEST_RESULT_HP case.  It's special because it
 * has a side-effect: resetting the occlustion result flag.
 */
static GLboolean
fxDDGetBooleanv(GLcontext *ctx, GLenum pname, GLboolean *result)
{
    if (pname == GL_OCCLUSION_TEST_RESULT_HP) {
        *result = get_occlusion_result(ctx);
        return GL_TRUE;
    }
    return GL_FALSE;
}


static GLboolean
fxDDGetIntegerv(GLcontext *ctx, GLenum pname, GLint *result)
{
    if (pname == GL_OCCLUSION_TEST_RESULT_HP) {
        *result = (GLint) get_occlusion_result(ctx);
        return GL_TRUE;
    }
    return GL_FALSE;
}


static GLboolean
fxDDGetFloatv(GLcontext *ctx, GLenum pname, GLfloat *result)
{
    if (pname == GL_OCCLUSION_TEST_RESULT_HP) {
        *result = (GLfloat) get_occlusion_result(ctx);
        return GL_TRUE;
    }
    return GL_FALSE;
}


static GLboolean
fxDDGetDoublev(GLcontext *ctx, GLenum pname, GLdouble *result)
{
    if (pname == GL_OCCLUSION_TEST_RESULT_HP) {
        *result = (GLdouble) get_occlusion_result(ctx);
        return GL_TRUE;
    }
    return GL_FALSE;
}


/* test if window coord (px,py) is visible */
static GLboolean
inClipRects(fxMesaContext fxMesa, int px, int py)
{
    int i;
    for (i = 0; i < fxMesa->numClipRects; i++) {
        if ((px >= fxMesa->pClipRects[i].x1) &&
            (px < fxMesa->pClipRects[i].x2) &&
            (py >= fxMesa->pClipRects[i].y1) &&
            (py < fxMesa->pClipRects[i].y2)) return GL_TRUE;
    }
    return GL_FALSE;
}



static GLboolean
bitmap_R5G6B5(GLcontext * ctx, GLint px, GLint py,
              GLsizei width, GLsizei height,
              const struct gl_pixelstore_attrib *unpack,
              const GLubyte * bitmap)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GrLfbInfo_t info;
    FxU16 color;
    const struct gl_pixelstore_attrib *finalUnpack;
    struct gl_pixelstore_attrib scissoredUnpack;

    /* check if there's any raster operations enabled which we can't handle */
    if (ctx->RasterMask & (ALPHATEST_BIT |
                           BLEND_BIT |
                           DEPTH_BIT |
                           FOG_BIT |
                           LOGIC_OP_BIT |
                           SCISSOR_BIT |
                           STENCIL_BIT |
                           MASKING_BIT |
                           ALPHABUF_BIT | MULTI_DRAW_BIT)) return GL_FALSE;

    if (ctx->Scissor.Enabled) {
        /* This is a bit tricky, but by carefully adjusting the px, py,
         * width, height, skipPixels and skipRows values we can do
         * scissoring without special code in the rendering loop.
         */

        /* we'll construct a new pixelstore struct */
        finalUnpack = &scissoredUnpack;
        scissoredUnpack = *unpack;
        if (scissoredUnpack.RowLength == 0)
            scissoredUnpack.RowLength = width;

        /* clip left */
        if (px < ctx->Scissor.X) {
            scissoredUnpack.SkipPixels += (ctx->Scissor.X - px);
            width -= (ctx->Scissor.X - px);
            px = ctx->Scissor.X;
        }
        /* clip right */
        if (px + width >= ctx->Scissor.X + ctx->Scissor.Width) {
            width -= (px + width - (ctx->Scissor.X + ctx->Scissor.Width));
        }
        /* clip bottom */
        if (py < ctx->Scissor.Y) {
            scissoredUnpack.SkipRows += (ctx->Scissor.Y - py);
            height -= (ctx->Scissor.Y - py);
            py = ctx->Scissor.Y;
        }
        /* clip top */
        if (py + height >= ctx->Scissor.Y + ctx->Scissor.Height) {
            height -= (py + height - (ctx->Scissor.Y + ctx->Scissor.Height));
        }

        if (width <= 0 || height <= 0)
            return GL_TRUE;     /* totally scissored away */
    }
    else {
        finalUnpack = unpack;
    }

    /* compute pixel value */
    {
        GLint r = (GLint) (ctx->Current.RasterColor[0] * 255.0f);
        GLint g = (GLint) (ctx->Current.RasterColor[1] * 255.0f);
        GLint b = (GLint) (ctx->Current.RasterColor[2] * 255.0f);
        /*GLint a = (GLint)(ctx->Current.RasterColor[3]*255.0f); */
        if (fxMesa->bgrOrder) {
            color = (FxU16)
                (((FxU16) 0xf8 & b) << (11 - 3)) |
                (((FxU16) 0xfc & g) << (5 - 3 + 1)) |
                (((FxU16) 0xf8 & r) >> 3);
        }
        else
            color = (FxU16)
                (((FxU16) 0xf8 & r) << (11 - 3)) |
                (((FxU16) 0xfc & g) << (5 - 3 + 1)) |
                (((FxU16) 0xf8 & b) >> 3);
    }

    info.size = sizeof(info);
    if (!FX_grLfbLock(fxMesa,
                      GR_LFB_WRITE_ONLY,
                      fxMesa->currentFB,
                      GR_LFBWRITEMODE_565,
                      GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
#ifndef FX_SILENT
        fprintf(stderr, "fx Driver: error locking the linear frame buffer\n");
#endif
        return GL_TRUE;
    }

    {
        const GLint winX = fxMesa->x_offset;
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        /* The dest stride depends on the hardware and whether we're drawing
         * to the front or back buffer.  This compile-time test seems to do
         * the job for now.
         */
        const GLint dstStride = (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
            ? (fxMesa->screen_width) : (info.strideInBytes / 2);
        GLint row;
        /* compute dest address of bottom-left pixel in bitmap */
        GLushort *dst = (GLushort *) info.lfbPtr
            + (winY - py) * dstStride + (winX + px);

        for (row = 0; row < height; row++) {
            const GLubyte *src =
                (const GLubyte *) _mesa_image_address(finalUnpack,
                                                      bitmap, width, height,
                                                      GL_COLOR_INDEX,
                                                      GL_BITMAP, 0, row, 0);
            if (finalUnpack->LsbFirst) {
                /* least significan bit first */
                GLubyte mask = 1U << (finalUnpack->SkipPixels & 0x7);
                GLint col;
                for (col = 0; col < width; col++) {
                    if (*src & mask) {
                        if (inClipRects(fxMesa, winX + px + col, winY - py - row))
                            dst[col] = color;
                    }
                    if (mask == 128U) {
                        src++;
                        mask = 1U;
                    }
                    else {
                        mask = mask << 1;
                    }
                }
                if (mask != 1)
                    src++;
            }
            else {
                /* most significan bit first */
                GLubyte mask = 128U >> (finalUnpack->SkipPixels & 0x7);
                GLint col;
                for (col = 0; col < width; col++) {
                    if (*src & mask) {
                        if (inClipRects(fxMesa, winX + px + col, winY - py - row))
                            dst[col] = color;
                    }
                    if (mask == 1U) {
                        src++;
                        mask = 128U;
                    }
                    else {
                        mask = mask >> 1;
                    }
                }
                if (mask != 128)
                    src++;
            }
            dst -= dstStride;
        }
    }

    FX_grLfbUnlock(fxMesa, GR_LFB_WRITE_ONLY, fxMesa->currentFB);
    return GL_TRUE;
}


static GLboolean
bitmap_R8G8B8A8(GLcontext * ctx, GLint px, GLint py,
                GLsizei width, GLsizei height,
                const struct gl_pixelstore_attrib *unpack,
                const GLubyte * bitmap)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GrLfbInfo_t info;
    GLuint color;
    const struct gl_pixelstore_attrib *finalUnpack;
    struct gl_pixelstore_attrib scissoredUnpack;

    /* check if there's any raster operations enabled which we can't handle */
    if (ctx->RasterMask & (ALPHATEST_BIT |
                           BLEND_BIT |
                           DEPTH_BIT |
                           FOG_BIT |
                           LOGIC_OP_BIT |
                           SCISSOR_BIT |
                           STENCIL_BIT |
                           MASKING_BIT |
                           ALPHABUF_BIT | MULTI_DRAW_BIT)) return GL_FALSE;

    if (ctx->Scissor.Enabled) {
        /* This is a bit tricky, but by carefully adjusting the px, py,
         * width, height, skipPixels and skipRows values we can do
         * scissoring without special code in the rendering loop.
         */

        /* we'll construct a new pixelstore struct */
        finalUnpack = &scissoredUnpack;
        scissoredUnpack = *unpack;
        if (scissoredUnpack.RowLength == 0)
            scissoredUnpack.RowLength = width;

        /* clip left */
        if (px < ctx->Scissor.X) {
            scissoredUnpack.SkipPixels += (ctx->Scissor.X - px);
            width -= (ctx->Scissor.X - px);
            px = ctx->Scissor.X;
        }
        /* clip right */
        if (px + width >= ctx->Scissor.X + ctx->Scissor.Width) {
            width -= (px + width - (ctx->Scissor.X + ctx->Scissor.Width));
        }
        /* clip bottom */
        if (py < ctx->Scissor.Y) {
            scissoredUnpack.SkipRows += (ctx->Scissor.Y - py);
            height -= (ctx->Scissor.Y - py);
            py = ctx->Scissor.Y;
        }
        /* clip top */
        if (py + height >= ctx->Scissor.Y + ctx->Scissor.Height) {
            height -= (py + height - (ctx->Scissor.Y + ctx->Scissor.Height));
        }

        if (width <= 0 || height <= 0)
            return GL_TRUE;     /* totally scissored away */
    }
    else {
        finalUnpack = unpack;
    }

    /* compute pixel value */
    {
        GLint r = (GLint) (ctx->Current.RasterColor[0] * 255.0f);
        GLint g = (GLint) (ctx->Current.RasterColor[1] * 255.0f);
        GLint b = (GLint) (ctx->Current.RasterColor[2] * 255.0f);
        GLint a = (GLint) (ctx->Current.RasterColor[3] * 255.0f);
        color = PACK_BGRA32(r, g, b, a);
    }

    info.size = sizeof(info);
    if (!FX_grLfbLock(fxMesa, GR_LFB_WRITE_ONLY,
                      fxMesa->currentFB, GR_LFBWRITEMODE_8888,
                      GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
#ifndef FX_SILENT
        fprintf(stderr, "fx Driver: error locking the linear frame buffer\n");
#endif
        return GL_TRUE;
    }

    {
        const GLint winX = fxMesa->x_offset;
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        GLint dstStride;
        GLuint *dst;
        GLint row;

        if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT) {
            dstStride = fxMesa->screen_width;
            dst =
                (GLuint *) info.lfbPtr + (winY - py) * dstStride + (winX +
                                                                    px);
        }
        else {
            dstStride = info.strideInBytes / 4;
            dst =
                (GLuint *) info.lfbPtr + (winY - py) * dstStride + (winX +
                                                                    px);
        }

        /* compute dest address of bottom-left pixel in bitmap */
        for (row = 0; row < height; row++) {
            const GLubyte *src =
                (const GLubyte *) _mesa_image_address(finalUnpack,
                                                      bitmap, width, height,
                                                      GL_COLOR_INDEX,
                                                      GL_BITMAP, 0, row, 0);
            if (finalUnpack->LsbFirst) {
                /* least significan bit first */
                GLubyte mask = 1U << (finalUnpack->SkipPixels & 0x7);
                GLint col;
                for (col = 0; col < width; col++) {
                    if (*src & mask) {
                        if (inClipRects(fxMesa, winX + px + col, winY - py - row))
                            dst[col] = color;
                    }
                    if (mask == 128U) {
                        src++;
                        mask = 1U;
                    }
                    else {
                        mask = mask << 1;
                    }
                }
                if (mask != 1)
                    src++;
            }
            else {
                /* most significan bit first */
                GLubyte mask = 128U >> (finalUnpack->SkipPixels & 0x7);
                GLint col;
                for (col = 0; col < width; col++) {
                    if (*src & mask) {
                        if (inClipRects(fxMesa, winX + px + col, winY - py - row))
                            dst[col] = color;
                    }
                    if (mask == 1U) {
                        src++;
                        mask = 128U;
                    }
                    else {
                        mask = mask >> 1;
                    }
                }
                if (mask != 128)
                    src++;
            }
            dst -= dstStride;
        }
    }

    FX_grLfbUnlock(fxMesa, GR_LFB_WRITE_ONLY, fxMesa->currentFB);
    return GL_TRUE;
}


static GLboolean
readpixels_R5G6B5(GLcontext * ctx, GLint x, GLint y,
                  GLsizei width, GLsizei height,
                  GLenum format, GLenum type,
                  const struct gl_pixelstore_attrib *packing,
                  GLvoid * dstImage)
{
    if (ctx->Pixel.ScaleOrBiasRGBA || ctx->Pixel.MapColorFlag) {
        return GL_FALSE;        /* can't do this */
    }
    else {
        fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
        GrLfbInfo_t info;
        GLboolean result = GL_FALSE;

        BEGIN_BOARD_LOCK(fxMesa);
        info.size = sizeof(info);
        if (grLfbLock(GR_LFB_READ_ONLY,
                      fxMesa->currentFB,
                      GR_LFBWRITEMODE_ANY,
                      GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
            const GLint winX = fxMesa->x_offset;
            const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
            const GLint srcStride =
                (fxMesa->glCtx->Color.DrawBuffer ==
                 GL_FRONT) ? (fxMesa->screen_width) : (info.strideInBytes /
                                                       2);
            const GLushort *src = (const GLushort *) info.lfbPtr
                + (winY - y) * srcStride + (winX + x);
            GLubyte *dst = (GLubyte *) _mesa_image_address(packing, dstImage,
                                                           width, height,
                                                           format, type, 0, 0,
                                                           0);
            GLint dstStride =
                _mesa_image_row_stride(packing, width, format, type);

            if (format == GL_RGB && type == GL_UNSIGNED_BYTE) {
                /* convert 5R6G5B into 8R8G8B */
                GLint row, col;
                const GLint halfWidth = width >> 1;
                const GLint extraPixel = (width & 1);
                for (row = 0; row < height; row++) {
                    GLubyte *d = dst;
                    for (col = 0; col < halfWidth; col++) {
                        const GLuint pixel = ((const GLuint *) src)[col];
                        const GLint pixel0 = pixel & 0xffff;
                        const GLint pixel1 = pixel >> 16;
                        *d++ = FX_PixelToR(fxMesa, pixel0);
                        *d++ = FX_PixelToG(fxMesa, pixel0);
                        *d++ = FX_PixelToB(fxMesa, pixel0);
                        *d++ = FX_PixelToR(fxMesa, pixel1);
                        *d++ = FX_PixelToG(fxMesa, pixel1);
                        *d++ = FX_PixelToB(fxMesa, pixel1);
                    }
                    if (extraPixel) {
                        GLushort pixel = src[width - 1];
                        *d++ = FX_PixelToR(fxMesa, pixel);
                        *d++ = FX_PixelToG(fxMesa, pixel);
                        *d++ = FX_PixelToB(fxMesa, pixel);
                    }
                    dst += dstStride;
                    src -= srcStride;
                }
                result = GL_TRUE;
            }
            else if (format == GL_RGBA && type == GL_UNSIGNED_BYTE) {
                /* convert 5R6G5B into 8R8G8B8A */
                GLint row, col;
                const GLint halfWidth = width >> 1;
                const GLint extraPixel = (width & 1);
                for (row = 0; row < height; row++) {
                    GLubyte *d = dst;
                    for (col = 0; col < halfWidth; col++) {
                        const GLuint pixel = ((const GLuint *) src)[col];
                        const GLint pixel0 = pixel & 0xffff;
                        const GLint pixel1 = pixel >> 16;
                        *d++ = FX_PixelToR(fxMesa, pixel0);
                        *d++ = FX_PixelToG(fxMesa, pixel0);
                        *d++ = FX_PixelToB(fxMesa, pixel0);
                        *d++ = 255;
                        *d++ = FX_PixelToR(fxMesa, pixel1);
                        *d++ = FX_PixelToG(fxMesa, pixel1);
                        *d++ = FX_PixelToB(fxMesa, pixel1);
                        *d++ = 255;
                    }
                    if (extraPixel) {
                        const GLushort pixel = src[width - 1];
                        *d++ = FX_PixelToR(fxMesa, pixel);
                        *d++ = FX_PixelToG(fxMesa, pixel);
                        *d++ = FX_PixelToB(fxMesa, pixel);
                        *d++ = 255;
                    }
                    dst += dstStride;
                    src -= srcStride;
                }
                result = GL_TRUE;
            }
            else if (format == GL_RGB && type == GL_UNSIGNED_SHORT_5_6_5) {
                /* directly memcpy 5R6G5B pixels into client's buffer */
                const GLint widthInBytes = width * 2;
                GLint row;
                for (row = 0; row < height; row++) {
                    MEMCPY(dst, src, widthInBytes);
                    dst += dstStride;
                    src -= srcStride;
                }
                result = GL_TRUE;
            }
            else {
                result = GL_FALSE;
            }

            grLfbUnlock(GR_LFB_READ_ONLY, fxMesa->currentFB);
        }
        END_BOARD_LOCK(fxMesa);
        return result;
    }
}



static GLboolean
readpixels_R8G8B8A8(GLcontext * ctx, GLint x, GLint y,
                    GLsizei width, GLsizei height,
                    GLenum format, GLenum type,
                    const struct gl_pixelstore_attrib *packing,
                    GLvoid * dstImage)
{
    if (!(format == GL_BGRA && type == GL_UNSIGNED_INT_8_8_8_8)
     && !(format == GL_BGRA && type == GL_UNSIGNED_BYTE)
     && !(format == GL_RGBA && type == GL_UNSIGNED_BYTE)) {
        return GL_FALSE;        /* format/type not optimised */
    }

    if (ctx->Pixel.ScaleOrBiasRGBA || ctx->Pixel.MapColorFlag) {
        return GL_FALSE;        /* can't do this */
    }

    {
        fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
        GrLfbInfo_t info;
        GLboolean result = GL_FALSE;

        const GLint winX = fxMesa->x_offset;
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint scrX = winX + x;
        const GLint scrY = winY - y;

        BEGIN_BOARD_LOCK(fxMesa);
        info.size = sizeof(info);
        if (grLfbLock(GR_LFB_READ_ONLY,
                      fxMesa->currentFB,
                      GR_LFBWRITEMODE_ANY,
                      GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
            const GLint srcStride = (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
                ? (fxMesa->screen_width) : (info.strideInBytes / 4);
            const GLuint *src = (const GLuint *) info.lfbPtr
                + scrY * srcStride + scrX;
            const GLint dstStride =
                _mesa_image_row_stride(packing, width, format, type);
            const GLubyte *dst = (GLubyte *) _mesa_image_address(packing,
                dstImage, width, height, format, type, 0, 0, 0);

            if ((format == GL_BGRA && type == GL_UNSIGNED_INT_8_8_8_8)
             || (format == GL_BGRA && type == GL_UNSIGNED_BYTE)) {
                const GLint widthInBytes = width * 4;
                GLint row;
                for (row = 0; row < height; row++) {
                    MEMCPY(dst, src, widthInBytes);
                    dst += dstStride;
                    src -= srcStride;
                }
                result = GL_TRUE;
            } else
            if (format == GL_RGBA && type == GL_UNSIGNED_BYTE) {
                const GLint widthInBytes = width * 4;
                GLuint *dstp = (GLuint *)dst;
                GLint row;
                GLint i;
                for (row = 0; row < height; row++) {
                    MEMCPY(dst, src, widthInBytes);
                    dst += dstStride;
                    src -= srcStride;
                    /* Data is in memory in BGRA format */
                    /* We need to swap R & B values */
                    for (i = 0; i < width; i++, dstp++) {
                        char *dstp0 = ((char *)(dstp)) + 0;
                        char *dstp2 = ((char *)(dstp)) + 2;
                        *dstp0 ^= *dstp2;
                        *dstp2 ^= *dstp0;
                        *dstp0 ^= *dstp2;
                    }
                }
                result = GL_TRUE;
            }

            grLfbUnlock(GR_LFB_READ_ONLY, fxMesa->currentFB);
        }
        END_BOARD_LOCK(fxMesa);
        return result;
    }
}



static GLboolean
drawpixels_R8G8B8A8(GLcontext * ctx, GLint x, GLint y,
                    GLsizei width, GLsizei height,
                    GLenum format, GLenum type,
                    const struct gl_pixelstore_attrib *unpack,
                    const GLvoid * pixels)
{
    if (!(format == GL_BGRA && type == GL_UNSIGNED_INT_8_8_8_8)
     && !(format == GL_BGRA && type == GL_UNSIGNED_BYTE)) {
        return GL_FALSE;        /* format/type not optimised */
    }

    if (ctx->Pixel.ZoomX!=1.0F || ctx->Pixel.ZoomY!=1.0F) {
        return GL_FALSE;        /* can't scale pixels */
    }

    if (ctx->Pixel.ScaleOrBiasRGBA || ctx->Pixel.MapColorFlag) {
        return GL_FALSE;        /* can't do this */
    }

    if (ctx->RasterMask) {
       return GL_FALSE;         /* can't do any raster ops */
    }

    {
        fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
        GrLfbInfo_t info;
        GLboolean result = GL_FALSE;

        const GLint winX = fxMesa->x_offset;
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint scrX = winX + x;
        const GLint scrY = winY - y;

        /* look for clipmasks, giveup if region obscured */
        if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT) {
            int i;
            for (i = 0; i < fxMesa->numClipRects; i++) {
                const XF86DRIClipRectPtr rect = &fxMesa->pClipRects[i];
                
                if (scrY < rect->y1 || scrY+height > rect->y2) {
                    if (scrX < rect->x1 || scrX+width > rect->x2) {
                        return GL_FALSE;    /* dst is obscured */
                    }
                }
            }
        }

        BEGIN_BOARD_LOCK(fxMesa);
        info.size = sizeof(info);
        if (grLfbLock(GR_LFB_WRITE_ONLY,
                      fxMesa->currentFB,
                      GR_LFBWRITEMODE_8888, GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
            const GLint dstStride = (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
                ? (fxMesa->screen_width * 4) : (info.strideInBytes);
            const GLubyte *dst = (const GLubyte *) info.lfbPtr
                + scrY * dstStride + scrX * 4;
            const GLint srcStride =
                _mesa_image_row_stride(unpack, width, format, type);
            const GLubyte *src = (GLubyte *) _mesa_image_address(unpack,
                pixels, width, height, format, type, 0, 0, 0);

            if ((format == GL_BGRA && type == GL_UNSIGNED_INT_8_8_8_8)
             || (format == GL_BGRA && type == GL_UNSIGNED_BYTE)) {
                const GLint widthInBytes = width * 4;
                GLint row;
                for (row = 0; row < height; row++) {
                    MEMCPY(dst, src, widthInBytes);
                    dst -= dstStride;
                    src += srcStride;
                }
                result = GL_TRUE;
            }

            grLfbUnlock(GR_LFB_WRITE_ONLY, fxMesa->currentFB);
        }
        END_BOARD_LOCK(fxMesa);
        return result;
    }
}


static GLboolean
drawpixels_R8G8B8A8_v2(GLcontext * ctx, GLint x, GLint y,
                    GLsizei width, GLsizei height,
                    GLenum format, GLenum type,
                    const struct gl_pixelstore_attrib *unpack,
                    const GLvoid * pixels)
{
    if (!(format == GL_BGRA && type == GL_UNSIGNED_INT_8_8_8_8)
     && !(format == GL_BGRA && type == GL_UNSIGNED_BYTE)) {
        return GL_FALSE;        /* format/type not optimised */
    }

    if (ctx->Pixel.ZoomX!=1.0F || ctx->Pixel.ZoomY!=1.0F) {
        return GL_FALSE;        /* can't scale pixels */
    }

    if (ctx->Pixel.ScaleOrBiasRGBA || ctx->Pixel.MapColorFlag) {
        return GL_FALSE;        /* can't do this */
    }

    if (ctx->RasterMask & (~BLEND_BIT)) {
        return GL_FALSE;        /* can't do any raster ops, except blend */
    }

    {
        fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
        GrLfbInfo_t info;
        GLboolean result = GL_FALSE;

        const GLint winX = fxMesa->x_offset;
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint scrX = winX + x;
        const GLint scrY = winY - y;

        /* look for clipmasks, giveup if region obscured */
        if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT) {
            int i;
            for (i = 0; i < fxMesa->numClipRects; i++) {
                const XF86DRIClipRectPtr rect = &fxMesa->pClipRects[i];
                
                if (scrY < rect->y1 || scrY+height > rect->y2) {
                    if (scrX < rect->x1 || scrX+width > rect->x2) {
                        return GL_FALSE;    /* dst is obscured */
                    }
                }
            }
        }

        BEGIN_BOARD_LOCK(fxMesa);
        info.size = sizeof(info);
        if (grLfbLock(GR_LFB_WRITE_ONLY,
                      fxMesa->currentFB,
                      GR_LFBWRITEMODE_8888, GR_ORIGIN_UPPER_LEFT, FXTRUE, &info)) {
            const GLint dstStride = (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
                ? (fxMesa->screen_width * 4) : (info.strideInBytes);
            const GLubyte *dst = (const GLubyte *) info.lfbPtr
                + scrY * dstStride + scrX * 4;
            const GLint srcStride =
                _mesa_image_row_stride(unpack, width, format, type);
            const GLubyte *src = (GLubyte *) _mesa_image_address(unpack,
                pixels, width, height, format, type, 0, 0, 0);

            void *grState = NULL;
            GLint grSize;

            if (grGet(GR_GLIDE_STATE_SIZE, sizeof(grSize), (void *) &grSize)) {
                if ((grState = malloc(grSize)) != 0) {
                    grGlideGetState(grState);
                }
            }

            if (ctx->RasterMask & BLEND_BIT) {
                grDisableAllEffects();
                grAlphaBlendFunction(GR_BLEND_SRC_ALPHA,
                                     GR_BLEND_ONE_MINUS_SRC_ALPHA,
                                     GR_BLEND_ONE,
                                     GR_BLEND_ZERO);
                grAlphaCombine(GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
                               GR_COMBINE_FACTOR_OTHER_ALPHA,
                               GR_COMBINE_LOCAL_NONE,
                               GR_COMBINE_OTHER_ITERATED,
                               FXFALSE);
                grColorCombine(GR_COMBINE_FUNCTION_SCALE_OTHER_ADD_LOCAL,
                               GR_COMBINE_FACTOR_OTHER_ALPHA,
                               GR_COMBINE_LOCAL_ITERATED,
                               GR_COMBINE_OTHER_ITERATED,
                               FXFALSE);
            }

            if ((format == GL_BGRA && type == GL_UNSIGNED_INT_8_8_8_8)
             || (format == GL_BGRA && type == GL_UNSIGNED_BYTE)) {
                const GLint widthInBytes = width * 4;
                GLint row;
                for (row = 0; row < height; row++) {
                    MEMCPY(dst, src, widthInBytes);
                    dst -= dstStride;
                    src += srcStride;
                }
                result = GL_TRUE;
            }

            if (grState) {
                grGlideSetState(grState);
                free(grState);
            }

            grLfbUnlock(GR_LFB_WRITE_ONLY, fxMesa->currentFB);
        }
        END_BOARD_LOCK(fxMesa);
        return result;
    }
}


static void
fxDDFinish(GLcontext *ctx)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    FX_grFinish(fxMesa);
}


static void
fxDDFlush(GLcontext *ctx)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    FX_grFlush(fxMesa);
}


static GLint
fxDDGetParameteri(const GLcontext * ctx, GLint param)
{
    switch (param) {
    case DD_HAVE_HARDWARE_FOG:
        return 1;
    default:
        fprintf(stderr,
                "fx Driver: internal error in fxDDGetParameteri(): %x\n",
                (int) param);
        return 0;
    }
}


static void
fxDDSetNearFar(GLcontext * ctx, GLfloat n, GLfloat f)
{
    FX_CONTEXT(ctx)->new_state |= FX_NEW_FOG;
    ctx->Driver.RenderStart = fxSetupFXUnits;
}


/* KW: Put the word Mesa in the render string because quakeworld
 * checks for this rather than doing a glGet(GL_MAX_TEXTURE_SIZE).
 * Why?
 */
static const GLubyte *
fxDDGetString(GLcontext * ctx, GLenum name)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;

    switch (name) {
    case GL_RENDERER:
        {
            static char buffer[100];
            char hardware[100];
            strcpy(hardware, FX_grGetString(fxMesa, GR_HARDWARE));
            if (strcmp(hardware, "Voodoo3 (tm)") == 0)
                strcpy(hardware, "Voodoo3");
	    else if (strcmp(hardware, "Voodoo4 (tm)") == 0)
	        strcpy(hardware, "Voodoo4");
	    else if (strcmp(hardware, "Voodoo5 (tm)") == 0)
	        strcpy(hardware, "Voodoo5");
            else if (strcmp(hardware, "Voodoo Banshee (tm)") == 0)
                strcpy(hardware, "VoodooBanshee");
            else {
                /* unexpected result: replace spaces with hyphens */
                int i;
                for (i = 0; hardware[i]; i++) {
                    if (hardware[i] == ' ' || hardware[i] == '\t')
                        hardware[i] = '-';
                }
            }
            /* now make the GL_RENDERER string */
            sprintf(buffer, "Mesa DRI %s 20001101", hardware);
            return buffer;
        }
    case GL_VENDOR:
        return "VA Linux Systems, Inc.";
    default:
        return NULL;
    }
}


#if 0
/* Example extension function */
static void
fxFooBarEXT(GLint i)
{
    printf("You called glFooBarEXT(%d)\n", i);
}
#endif



/*
 * Enable/Disable the extensions for this context.
 */
static void
fxDDInitExtensions(GLcontext * ctx)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;

    gl_extensions_disable(ctx, "GL_EXT_blend_logic_op");
    gl_extensions_disable(ctx, "GL_EXT_blend_minmax");
    gl_extensions_disable(ctx, "GL_EXT_blend_subtract");
    gl_extensions_disable(ctx, "GL_EXT_blend_color");
    gl_extensions_disable(ctx, "GL_EXT_blend_func_separate");
    gl_extensions_disable(ctx, "GL_INGR_blend_func_separate");
    gl_extensions_enable(ctx, "GL_HP_occlusion_test");

    if (!fxMesa->haveTwoTMUs)
        gl_extensions_disable(ctx, "GL_EXT_texture_env_add");

    if (!fxMesa->emulateTwoTMUs)
        gl_extensions_disable(ctx, "GL_ARB_multitexture");

    if (fxMesa->isNapalm) {
        gl_extensions_enable(ctx, "GL_ARB_texture_compression");
        gl_extensions_enable(ctx, "GL_3DFX_texture_compression_FXT1");
        {
            char *legacy_str = getenv("FX_GL_COMPRESS_LEGACY_TEXTURES");
            if (!legacy_str || ((legacy_str[0] == '0')
                                && (legacy_str[1] == '\0'))) {
                gl_extensions_add(ctx, ALWAYS_ENABLED, "GL_S3_s3tc", 0);
            }
        }
        gl_extensions_enable(ctx, "GL_EXT_texture_env_combine");
    }

    /* Example of hooking in an extension function.
     * For DRI-based drivers, also see __driRegisterExtensions in the
     * tdfx_xmesa.c file.
     */
#if 0
    {
        void **dispatchTable = (void **) ctx->Exec;
        const int _gloffset_FooBarEXT = 555; /* just an example number! */
        const int tabSize = _glapi_get_dispatch_table_size();
        assert(_gloffset_FooBarEXT < tabSize);
        dispatchTable[_gloffset_FooBarEXT] = (void *) fxFooBarEXT;
        /* XXX You would also need to hook into the display list dispatch
         * table.  Really, the implementation of extensions might as well
         * be in the core of Mesa since core Mesa and the device driver
         * is one big shared lib.
         */
    }
#endif
}



/*
 * Initialize the state in an fxMesaContext struct.
 */
int
fxDDInitFxMesaContext(fxMesaContext fxMesa)
{
    /* Get Glide3 extension function pointers */
    {
        void *handle = dlopen(NULL, RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
            txImgQuantizePtr = NULL;
            txImgDequantizeFXT1Ptr = NULL;
            txErrorSetCallbackPtr = NULL;
            grStencilFuncPtr = NULL;
            grStencilMaskPtr = NULL;
            grStencilOpPtr = NULL;
            grBufferClearExtPtr = NULL;
            grColorMaskExtPtr = NULL;
            grColorCombineExtPtr = NULL;
            grTexColorCombineExtPtr = NULL;
            grAlphaCombineExtPtr = NULL;
            grTexAlphaCombineExtPtr = NULL;
            grAlphaBlendFunctionExtPtr = NULL;
            grConstantColorValueExtPtr = NULL;
            return 0;
        }
        else {
           /*
            * These are not exported by Glide.
            */
            txImgQuantizePtr = dlsym(handle, "txImgQuantize");
            txImgDequantizeFXT1Ptr = dlsym(handle, "_txImgDequantizeFXT1");
            txErrorSetCallbackPtr = dlsym(handle, "txErrorSetCallback");
            grStencilFuncPtr = dlsym(handle, "grStencilFunc");
            grStencilMaskPtr = dlsym(handle, "grStencilMask");
            grStencilOpPtr = dlsym(handle, "grStencilOp");
            grBufferClearExtPtr = dlsym(handle, "grBufferClearExt");
            grColorMaskExtPtr = dlsym(handle, "grColorMaskExt");
            grColorCombineExtPtr = dlsym(handle, "grColorCombineExt");
            grTexColorCombineExtPtr = dlsym(handle, "grTexColorCombineExt");
            grAlphaCombineExtPtr = dlsym(handle, "grAlphaCombineExt");
            grTexAlphaCombineExtPtr = dlsym(handle, "grTexAlphaCombineExt");
            grAlphaBlendFunctionExtPtr = dlsym(handle, "grAlphaBlendFunctionExt");
            grConstantColorValueExtPtr = dlsym(handle, "grConstantColorValueExt");
        }
        dlclose(handle);
    }

    FX_setupGrVertexLayout(fxMesa);

    if (getenv("FX_EMULATE_SINGLE_TMU"))
        fxMesa->haveTwoTMUs = GL_FALSE;

    fxMesa->emulateTwoTMUs = fxMesa->haveTwoTMUs;

    if (!getenv("FX_DONT_FAKE_MULTITEX"))
        fxMesa->emulateTwoTMUs = GL_TRUE;

    if (getenv("FX_GLIDE_SWAPINTERVAL"))
        fxMesa->swapInterval = atoi(getenv("FX_GLIDE_SWAPINTERVAL"));
    else
        fxMesa->swapInterval = 1;

    if (getenv("MESA_FX_SWAP_PENDING"))
        fxMesa->maxPendingSwapBuffers = atoi(getenv("MESA_FX_SWAP_PENDING"));
    else
        fxMesa->maxPendingSwapBuffers = 2;

    if (getenv("MESA_FX_INFO"))
        fxMesa->verbose = GL_TRUE;
    else
        fxMesa->verbose = GL_FALSE;

#if 0
    printf("haveTwoTMUs=%d  emulateTwoTMUs=%d\n", 
           fxMesa->haveTwoTMUs, fxMesa->emulateTwoTMUs);
#endif

    fxMesa->depthClear = FX_grGetInteger(fxMesa, FX_ZDEPTH_MAX);

    fxMesa->color = 0xffffffff;
    fxMesa->clearC = 0;
    fxMesa->clearA = 0;

    fxMesa->stats.swapBuffer = 0;
    fxMesa->stats.reqTexUpload = 0;
    fxMesa->stats.texUpload = 0;
    fxMesa->stats.memTexUpload = 0;

    fxMesa->tmuSrc = FX_TMU_NONE;
    fxTMInit(fxMesa);

    /* FX units setup */
    fxMesa->unitsState.alphaTestEnabled = GL_FALSE;
    fxMesa->unitsState.alphaTestFunc = GR_CMP_ALWAYS;
    fxMesa->unitsState.alphaTestRefValue = 0;

    fxMesa->unitsState.blendEnabled = GL_FALSE;
    fxMesa->unitsState.blendSrcFuncRGB = GR_BLEND_ONE;
    fxMesa->unitsState.blendDstFuncRGB = GR_BLEND_ZERO;
    fxMesa->unitsState.blendSrcFuncAlpha = GR_BLEND_ONE;
    fxMesa->unitsState.blendDstFuncAlpha = GR_BLEND_ZERO;

    /*
       fxMesa->unitsState.depthTestEnabled = GL_FALSE;
       fxMesa->unitsState.depthMask = GL_TRUE;
       fxMesa->unitsState.depthTestFunc = GR_CMP_LESS;
     */

    FX_grColorMaskv(fxMesa->glCtx, true4);
    if (fxMesa->glVis->DBflag) {
        fxMesa->currentFB = GR_BUFFER_BACKBUFFER;
        FX_grRenderBuffer(fxMesa, GR_BUFFER_BACKBUFFER);
    }
    else {
        fxMesa->currentFB = GR_BUFFER_FRONTBUFFER;
        FX_grRenderBuffer(fxMesa, GR_BUFFER_FRONTBUFFER);
    }

    fxMesa->state = NULL;
    fxMesa->fogTable = NULL;

    fxMesa->state = malloc(FX_grGetInteger(fxMesa, FX_GLIDE_STATE_SIZE));
    fxMesa->fogTable =
       malloc(FX_grGetInteger(fxMesa, FX_FOG_TABLE_ENTRIES) * sizeof(GrFog_t));

    if (!fxMesa->state || !fxMesa->fogTable) {
        if (fxMesa->state)
            free(fxMesa->state);
        if (fxMesa->fogTable)
            free(fxMesa->fogTable);
        return 0;
    }

    if (fxMesa->glVis->DepthBits > 0)
        FX_grDepthBufferMode(fxMesa, GR_DEPTHBUFFER_ZBUFFER);

    FX_grLfbWriteColorFormat(fxMesa, GR_COLORFORMAT_ABGR);

    fxMesa->textureAlign = FX_grGetInteger(fxMesa, FX_TEXTURE_ALIGN);
    if (fxMesa->isNapalm) {
       fxMesa->glCtx->Const.MaxTextureLevels = 12;
       fxMesa->glCtx->Const.MaxTextureSize = 2048;
       fxMesa->glCtx->Const.NumCompressedTextureFormats = 1;
    }
    else {
       fxMesa->glCtx->Const.MaxTextureLevels = 9;
       fxMesa->glCtx->Const.MaxTextureSize = 256;
       fxMesa->glCtx->Const.NumCompressedTextureFormats = 0;
    }
    fxMesa->glCtx->Const.MaxTextureUnits = fxMesa->emulateTwoTMUs ? 2 : 1;
    fxMesa->glCtx->NewState |= NEW_DRVSTATE1;
    fxMesa->new_state = NEW_ALL;

    fxDDSetupInit();
    fxDDCvaInit();
    fxDDClipInit();
    fxDDTrifuncInit();
    fxDDFastPathInit();

    fxSetupDDPointers(fxMesa->glCtx);
    fxDDRenderInit(fxMesa->glCtx);
    fxDDInitExtensions(fxMesa->glCtx);

    fxDDSetNearFar(fxMesa->glCtx, 1.0, 100.0);

    FX_grGlideGetState(fxMesa, (GrState *) fxMesa->state);

    /* XXX Fix me: callback not registered when main VB is created.
     */
    if (fxMesa->glCtx->VB)
        fxDDRegisterVB(fxMesa->glCtx->VB);

    /* XXX Fix me too: need to have the 'struct dd' prepared prior to
     * creating the context... The below is broken if you try to insert
     * new stages.  
     */
    if (fxMesa->glCtx->NrPipelineStages)
        fxMesa->glCtx->NrPipelineStages =
            fxDDRegisterPipelineStages(fxMesa->glCtx->PipelineStage,
                                       fxMesa->glCtx->PipelineStage,
                                       fxMesa->glCtx->NrPipelineStages);

    /* this little bit ensures that all Glide state gets initialized */
    fxMesa->new_state = NEW_ALL;
    fxMesa->glCtx->Driver.RenderStart = fxSetupFXUnits;

#if	defined(FX_PXCONV_TABULAR)
    fxInitPixelTables(fxMesa, GL_FALSE); /* Load tables of pixel colors */
#endif	/* FX_PXCONV_TABULAR */

    /* Run the config file */
    gl_context_initialize(fxMesa->glCtx);

    return 1;
}



/* Check if the hardware supports the current context 
 *
 * Performs similar work to fxDDChooseRenderState() - should be merged.
 */
static GLboolean
fxIsInHardware(GLcontext * ctx)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;

    if (!ctx->Hint.AllowDrawMem)
        return GL_TRUE;         /* you'll take it and like it */

    if (ctx->Color.BlendEnabled
        && ctx->Color.BlendEquation != GL_FUNC_ADD_EXT) {
        return GL_FALSE;
    }

    if (ctx->Color.ColorLogicOpEnabled && ctx->Color.LogicOp != GL_COPY) {
        return GL_FALSE;
    }

    if (ctx->Light.Enabled &&
        ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR) {
        return GL_FALSE;
    }

    if (ctx->Visual->RedBits < 8 &&
        (ctx->Color.ColorMask[RCOMP] != ctx->Color.ColorMask[GCOMP] ||
         ctx->Color.ColorMask[GCOMP] != ctx->Color.ColorMask[BCOMP])) {
        /* can't individually mask R, G, B in 16bpp/Voodoo3 mode */
        return GL_FALSE;
    }
       

#if 000
    if (
        ((ctx->Color.BlendEnabled)
         && (ctx->Color.BlendEquation != GL_FUNC_ADD_EXT))
        || ((ctx->Color.ColorLogicOpEnabled)
            && (ctx->Color.LogicOp != GL_COPY))
        || (ctx->Light.Model.ColorControl == GL_SEPARATE_SPECULAR_COLOR)
        ||
        (!((ctx->Color.ColorMask[RCOMP] == ctx->Color.ColorMask[GCOMP])
           && (ctx->Color.ColorMask[GCOMP] == ctx->Color.ColorMask[BCOMP])
           && (ctx->Color.ColorMask[ACOMP] == ctx->Color.ColorMask[ACOMP])))
        ) {
        return GL_FALSE;
    }
#endif


    /* Unsupported texture/multitexture cases */

    if (fxMesa->emulateTwoTMUs) {
        if ((ctx->Enabled & (TEXTURE0_3D | TEXTURE1_3D)) ||
            /* Not very well written ... */
            ((ctx->Enabled & (TEXTURE0_1D | TEXTURE1_1D)) &&
             ((ctx->Enabled & (TEXTURE0_2D | TEXTURE1_2D)) !=
              (TEXTURE0_2D | TEXTURE1_2D)))) {
            return GL_FALSE;
        }

        if (ctx->Texture.ReallyEnabled & TEXTURE0_2D) {
#if 0
           if (ctx->Texture.Unit[0].EnvMode == GL_BLEND) {
              return GL_FALSE;
           }
#endif
            if (!fxMesa->isNapalm &&
                ctx->Texture.Unit[0].EnvMode == GL_BLEND &&
                (ctx->Texture.ReallyEnabled & TEXTURE1_2D ||
                 ctx->Texture.Unit[0].EnvColor[0] != 0 ||
                 ctx->Texture.Unit[0].EnvColor[1] != 0 ||
                 ctx->Texture.Unit[0].EnvColor[2] != 0 ||
                 ctx->Texture.Unit[0].EnvColor[3] != 1)) {
                return GL_FALSE;
            }
            if (ctx->Texture.Unit[0].Current->Image[0]->Border > 0)
                return GL_FALSE;
        }

        if (ctx->Texture.ReallyEnabled & TEXTURE1_2D) {
            if (!fxMesa->isNapalm && ctx->Texture.Unit[1].EnvMode == GL_BLEND)
                return GL_FALSE;
            if (ctx->Texture.Unit[1].Current->Image[0]->Border > 0)
                return GL_FALSE;
        }

        if (MESA_VERBOSE & (VERBOSE_DRIVER | VERBOSE_TEXTURE))
            fprintf(stderr, "fxMesa: fxIsInHardware, envmode is %s/%s\n",
                    gl_lookup_enum_by_nr(ctx->Texture.Unit[0].EnvMode),
                    gl_lookup_enum_by_nr(ctx->Texture.Unit[1].EnvMode));

        /* KW: This was wrong (I think) and I changed it... which doesn't mean
         * it is now correct...
         */
        if ((ctx->Enabled & (TEXTURE0_1D | TEXTURE0_2D | TEXTURE0_3D)) &&
            (ctx->Enabled & (TEXTURE1_1D | TEXTURE1_2D | TEXTURE1_3D))) {
            /* Can't use multipass to blend a multitextured triangle - fall
               * back to software.
             */
            if (!fxMesa->haveTwoTMUs && ctx->Color.BlendEnabled) {
                return GL_FALSE;
            }

            if ((ctx->Texture.Unit[0].EnvMode != ctx->Texture.Unit[1].EnvMode)
                && (ctx->Texture.Unit[0].EnvMode != GL_MODULATE)
                && (ctx->Texture.Unit[0].EnvMode != GL_REPLACE)) { /* q2, seems ok... */
                if (MESA_VERBOSE & VERBOSE_DRIVER)
                    fprintf(stderr,
                            "fxMesa: unsupported multitex env mode\n");
                return GL_FALSE;
            }
        }
    }
    else {
        if ((ctx->Enabled & (TEXTURE1_1D | TEXTURE1_2D | TEXTURE1_3D)) ||
            /* Not very well written ... */
            ((ctx->Enabled & TEXTURE0_1D) && (!(ctx->Enabled & TEXTURE0_2D)))
            ) {
            return GL_FALSE;
        }


        if (!fxMesa->isNapalm && (ctx->Texture.ReallyEnabled & TEXTURE0_2D) &&
            (ctx->Texture.Unit[0].EnvMode == GL_BLEND)) {
            return GL_FALSE;
        }
    }

    if (ctx->Stencil.Enabled && !fxMesa->haveHwStencil)
        return GL_FALSE;

    return GL_TRUE;
}



#define INTERESTED (~(NEW_MODELVIEW|NEW_PROJECTION|NEW_PROJECTION|NEW_TEXTURE_MATRIX|NEW_USER_CLIP|NEW_CLIENT_STATE|NEW_TEXTURE_ENABLE))

static void
fxDDUpdateDDPointers(GLcontext * ctx)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GLuint new_state = ctx->NewState;

    if (MESA_VERBOSE & (VERBOSE_DRIVER | VERBOSE_STATE))
        fprintf(stderr, "fxmesa: fxDDUpdateDDPointers(...)\n");

    if (new_state & (NEW_RASTER_OPS | NEW_TEXTURING))
        fxMesa->is_in_hardware = fxIsInHardware(ctx);

    if (fxMesa->is_in_hardware) {
        if (fxMesa->new_state)
            fxSetupFXUnits(ctx);

        if (new_state & INTERESTED) {
            fxDDChooseRenderState(ctx);
            fxMesa->RenderVBTables = fxDDChooseRenderVBTables(ctx);
            fxMesa->RenderVBClippedTab = fxMesa->RenderVBTables[0];
            fxMesa->RenderVBCulledTab = fxMesa->RenderVBTables[1];
            fxMesa->RenderVBRawTab = fxMesa->RenderVBTables[2];

            ctx->Driver.RasterSetup = fxDDChooseSetupFunction(ctx);
        }

        ctx->Driver.PointsFunc = fxMesa->PointsFunc;
        ctx->Driver.LineFunc = fxMesa->LineFunc;
        ctx->Driver.TriangleFunc = fxMesa->TriangleFunc;
        ctx->Driver.QuadFunc = fxMesa->QuadFunc;
    }
    else {
        fxMesa->render_index = FX_FALLBACK;
    }
}

static void
fxDDReducedPrimitiveChange(GLcontext * ctx, GLenum prim)
{
    if (ctx->Polygon.CullFlag) {
        if (ctx->PB->primitive != GL_POLYGON) { /* Lines or Points */
            fxMesaContext fxMesa = FX_CONTEXT(ctx);
            FX_grCullMode(fxMesa, GR_CULL_DISABLE);
            fxMesa->cullMode = GR_CULL_DISABLE;
        }
    }
}


void
fxSetupDDPointers(GLcontext * ctx)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxSetupDDPointers()\n");
    }
    ctx->Driver.UpdateState = fxDDUpdateDDPointers;
    ctx->Driver.ClearIndex = NULL;
    ctx->Driver.ClearColor = fxDDClearColor;
    ctx->Driver.Clear = fxDDClear;
    ctx->Driver.Index = NULL;
    ctx->Driver.Color = fxDDSetColor;
    ctx->Driver.SetDrawBuffer = fxDDSetDrawBuffer;
    ctx->Driver.SetReadBuffer = fxDDSetReadBuffer;
    ctx->Driver.GetBufferSize = fxDDBufferSize;
    ctx->Driver.Finish = fxDDFinish;
    ctx->Driver.Flush = fxDDFlush;
    ctx->Driver.GetString = fxDDGetString;
    ctx->Driver.NearFar = fxDDSetNearFar;
    ctx->Driver.GetParameteri = fxDDGetParameteri;
    ctx->Driver.GetBooleanv = fxDDGetBooleanv;
    ctx->Driver.GetFloatv = fxDDGetFloatv;
    ctx->Driver.GetDoublev = fxDDGetDoublev;
    ctx->Driver.GetIntegerv = fxDDGetIntegerv;

    if (ctx->Visual->RedBits == 8 &&
        ctx->Visual->GreenBits == 8 &&
        ctx->Visual->BlueBits == 8 &&
        ctx->Visual->AlphaBits == 8) {
        ctx->Driver.Bitmap = bitmap_R8G8B8A8;
        ctx->Driver.DrawPixels = drawpixels_R8G8B8A8;
        ctx->Driver.ReadPixels = readpixels_R8G8B8A8;
    }
    else if (ctx->Visual->RedBits == 5 &&
             ctx->Visual->GreenBits == 6 &&
             ctx->Visual->BlueBits == 5 &&
             ctx->Visual->AlphaBits == 0) {
        ctx->Driver.Bitmap = bitmap_R5G6B5;
        ctx->Driver.DrawPixels = NULL;
        ctx->Driver.ReadPixels = readpixels_R5G6B5;
    }
    else {
        ctx->Driver.Bitmap = NULL;
        ctx->Driver.DrawPixels = NULL;
        ctx->Driver.ReadPixels = NULL;
    }

    ctx->Driver.RenderStart = NULL;
    ctx->Driver.RenderFinish = NULL;

    ctx->Driver.TexImage2D = fxDDTexImage2D;
    ctx->Driver.TexSubImage2D = fxDDTexSubImage2D;
    ctx->Driver.TestProxyTexImage = fxDDTestProxyTexImage;
    ctx->Driver.GetTexImage = fxDDGetTexImage;
    ctx->Driver.CompressedTexImage2D = fxDDCompressedTexImage2D;
    ctx->Driver.CompressedTexSubImage2D = fxDDCompressedTexSubImage2D;
    ctx->Driver.GetCompressedTexImage = fxDDGetCompressedTexImage;
    ctx->Driver.SpecificCompressedTexFormat = fxDDSpecificCompressedTexFormat;
    ctx->Driver.BaseCompressedTexFormat = fxDDBaseCompressedTexFormat;
    ctx->Driver.IsCompressedFormat = fxDDIsCompressedFormat;
    ctx->Driver.CompressedImageSize = fxDDCompressedImageSize;
    ctx->Driver.TexEnv = fxDDTexEnv;
    ctx->Driver.TexParameter = fxDDTexParam;
    ctx->Driver.BindTexture = fxDDTexBind;
    ctx->Driver.DeleteTexture = fxDDTexDel;
    ctx->Driver.IsTextureResident = fxDDIsTextureResident;
    ctx->Driver.UpdateTexturePalette = fxDDTexPalette;

    ctx->Driver.RectFunc = NULL;

    if (fxMesa->haveHwStencil) {
        ctx->Driver.StencilFunc = fxDDStencilFunc;
        ctx->Driver.StencilMask = fxDDStencilMask;
        ctx->Driver.StencilOp = fxDDStencilOp;
    }

    ctx->Driver.AlphaFunc = fxDDAlphaFunc;
    ctx->Driver.BlendFunc = fxDDBlendFunc;
    ctx->Driver.BlendFuncSeparate = fxDDBlendFuncSeparate;
    ctx->Driver.DepthFunc = fxDDDepthFunc;
    ctx->Driver.DepthMask = fxDDDepthMask;
    ctx->Driver.ColorMask = fxDDColorMask;
    ctx->Driver.Fogfv = fxDDFogfv;
    ctx->Driver.Scissor = fxDDScissor;
    ctx->Driver.FrontFace = fxDDFrontFace;
    ctx->Driver.CullFace = fxDDCullFace;
    ctx->Driver.ShadeModel = fxDDShadeModel;
    ctx->Driver.Enable = fxDDEnable;
    ctx->Driver.ReducedPrimitiveChange = fxDDReducedPrimitiveChange;

    ctx->Driver.RegisterVB = fxDDRegisterVB;
    ctx->Driver.UnregisterVB = fxDDUnregisterVB;

    ctx->Driver.RegisterPipelineStages = fxDDRegisterPipelineStages;

    ctx->Driver.OptimizeImmediatePipeline = 0; /* nothing done yet */
    ctx->Driver.OptimizePrecalcPipeline = 0;

/*    if (getenv("MESA_USE_FAST") || getenv("FX_USE_FAST")) */
/*       ctx->Driver.OptimizePrecalcPipeline = fxDDOptimizePrecalcPipeline; */

    if (!getenv("FX_NO_FAST"))
        ctx->Driver.BuildPrecalcPipeline = fxDDBuildPrecalcPipeline;

    ctx->Driver.TriangleCaps =
        DD_TRI_CULL | DD_TRI_OFFSET | DD_TRI_LIGHT_TWOSIDE;

    fxSetupDDSpanPointers(ctx);

    FX_CONTEXT(ctx)->render_index = 1; /* force an update */
    fxDDUpdateDDPointers(ctx);
}
