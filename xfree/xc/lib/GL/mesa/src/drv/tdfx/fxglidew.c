/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/fxglidew.c,v 1.2 2000/12/08 19:36:23 alanh Exp $ */
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


#include <stdlib.h>
#include <string.h>
#include "glide.h"
#include "fxdrv.h"
#include "fxglidew.h"


grStencilFunc_t grStencilFuncPtr = NULL;
grStencilMask_t grStencilMaskPtr = NULL;
grStencilOp_t grStencilOpPtr = NULL;
grBufferClearExt_t grBufferClearExtPtr = NULL;
grColorMaskExt_t grColorMaskExtPtr = NULL;
txImgQuantize_t  txImgQuantizePtr = NULL;
txImgDeQuantize_t txImgDequantizeFXT1Ptr = NULL;
txErrorSetCallback_t txErrorSetCallbackPtr = NULL;
grColorCombineExt_t grColorCombineExtPtr = NULL;
grTexColorCombineExt_t grTexColorCombineExtPtr = NULL;
grAlphaCombineExt_t grAlphaCombineExtPtr = NULL;
grTexAlphaCombineExt_t grTexAlphaCombineExtPtr = NULL;
grAlphaBlendFunctionExt_t grAlphaBlendFunctionExtPtr = NULL;
grConstantColorValueExt_t grConstantColorValueExtPtr = NULL;


FxI32
FX_grGetInteger_NoLock(FxU32 pname)
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
            grGet(grname, 4, &result);
            return result;
        }
    case FX_ZDEPTH_MAX:
        {
            FxI32 zvals[2];
            grGet(GR_ZDEPTH_MIN_MAX, 8, zvals);
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
FX_grGetInteger(fxMesaContext fxMesa, FxU32 pname)
{
    int result;
    BEGIN_BOARD_LOCK(fxMesa);
    result = FX_grGetInteger_NoLock(pname);
    END_BOARD_LOCK(fxMesa);
    return result;
}


const char *
FX_grGetString(fxMesaContext fxMesa, FxU32 pname)
{
    const char *s;
    BEGIN_BOARD_LOCK(fxMesa);
    s = grGetString(pname);
    END_BOARD_LOCK(fxMesa);
    return s;
}



/* Wrapper for grColorMask() and grColorMaskExt().
 */
void
FX_grColorMask(GLcontext *ctx, GLboolean r, GLboolean g,
               GLboolean b, GLboolean a)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    BEGIN_BOARD_LOCK(fxMesa);
    if (ctx->Visual->RedBits == 8) {
        /* 32bpp mode */
        ASSERT(grColorMaskExtPtr);
        (*grColorMaskExtPtr)(r, g, b, a);
    }
    else {
        /* 16 bpp mode */
        /* we never have an alpha buffer */
        grColorMask(r || g || b, GL_FALSE);
    }
    END_BOARD_LOCK(fxMesa);
}


void
FX_grColorMask_NoLock(GLcontext *ctx, GLboolean r, GLboolean g,
                      GLboolean b, GLboolean a)
{
    if (ctx->Visual->RedBits == 8) {
        /* 32bpp mode */
        ASSERT(grColorMaskExtPtr);
        (*grColorMaskExtPtr)(r, g, b, a);
    }
    else {
        /* 16 bpp mode */
        /* we never have an alpha buffer */
        grColorMask(r || g || b, GL_FALSE);
    }
}


/* As above, but pass the mask as an array
 */
void
FX_grColorMaskv(GLcontext *ctx, const GLboolean rgba[4])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    BEGIN_BOARD_LOCK(fxMesa);
    if (ctx->Visual->RedBits == 8) {
        /* 32bpp mode */
        ASSERT(grColorMaskExtPtr);
        (*grColorMaskExtPtr)(rgba[RCOMP], rgba[GCOMP],
                             rgba[BCOMP], rgba[ACOMP]);
    }
    else {
        /* 16 bpp mode */
        /* we never have an alpha buffer */
        grColorMask(rgba[RCOMP] || rgba[GCOMP] || rgba[BCOMP], GL_FALSE);
    }
    END_BOARD_LOCK(fxMesa);
}

void
FX_grColorMaskv_NoLock(GLcontext *ctx, const GLboolean rgba[4])
{
    if (ctx->Visual->RedBits == 8) {
        /* 32bpp mode */
        ASSERT(grColorMaskExtPtr);
        (*grColorMaskExtPtr)(rgba[RCOMP], rgba[GCOMP],
                             rgba[BCOMP], rgba[ACOMP]);
    }
    else {
        /* 16 bpp mode */
        /* we never have an alpha buffer */
        grColorMask(rgba[RCOMP] || rgba[GCOMP] || rgba[BCOMP], GL_FALSE);
    }
}



FxBool
FX_grLfbLock(fxMesaContext fxMesa, GrLock_t type, GrBuffer_t buffer,
             GrLfbWriteMode_t writeMode, GrOriginLocation_t origin,
             FxBool pixelPipeline, GrLfbInfo_t * info)
{
    FxBool result;

    BEGIN_BOARD_LOCK(fxMesa);
    result = grLfbLock(type, buffer, writeMode, origin, pixelPipeline, info);
    END_BOARD_LOCK(fxMesa);
    return result;
}

FxU32
FX_grTexTextureMemRequired(fxMesaContext fxMesa, FxU32 evenOdd, GrTexInfo * info)
{
    FxU32 result;

    BEGIN_BOARD_LOCK(fxMesa);
    result = grTexTextureMemRequired(evenOdd, info);
    END_BOARD_LOCK(fxMesa);
    return result;
}

FxU32
FX_grTexMinAddress(fxMesaContext fxMesa, GrChipID_t tmu)
{
    FxU32 result;

    BEGIN_BOARD_LOCK(fxMesa);
    result = grTexMinAddress(tmu);
    END_BOARD_LOCK(fxMesa);
    return result;
}

extern FxU32
FX_grTexMaxAddress(fxMesaContext fxMesa, GrChipID_t tmu)
{
    FxU32 result;

    BEGIN_BOARD_LOCK(fxMesa);
    result = grTexMaxAddress(tmu);
    END_BOARD_LOCK(fxMesa);
    return result;
}


int
FX_getFogTableSize(fxMesaContext fxMesa)
{
    int result;
    BEGIN_BOARD_LOCK(fxMesa);
    grGet(GR_FOG_TABLE_ENTRIES, sizeof(int), (void *) &result);
    END_BOARD_LOCK(fxMesa);
    return result;
}

int
FX_getGrStateSize(fxMesaContext fxMesa)
{
    int result;
    BEGIN_BOARD_LOCK(fxMesa);
    grGet(GR_GLIDE_STATE_SIZE, sizeof(int), (void *) &result);
    END_BOARD_LOCK(fxMesa);
    return result;
}

void
FX_grAADrawLine(fxMesaContext fxMesa, GrVertex * a, GrVertex * b)
{
    /* ToDo */
    BEGIN_CLIP_LOOP(fxMesa);
    grDrawLine(a, b);
    END_CLIP_LOOP(fxMesa);
}

void
FX_grAADrawPoint(fxMesaContext fxMesa, GrVertex * a)
{
    BEGIN_CLIP_LOOP(fxMesa);
    grDrawPoint(a);
    END_CLIP_LOOP(fxMesa);
}

void
FX_grDrawPolygonVertexList(fxMesaContext fxMesa, int n, GrVertex * verts)
{
    BEGIN_CLIP_LOOP(fxMesa);
    grDrawVertexArrayContiguous(GR_POLYGON, n, verts, sizeof(GrVertex));
    END_CLIP_LOOP(fxMesa);
}

#if FX_USE_PARGB
void
FX_setupGrVertexLayout(fxMesaContext fxMesa)
{
    BEGIN_BOARD_LOCK(fxMesa);
    grReset(GR_VERTEX_PARAMETER);

    grCoordinateSpace(GR_WINDOW_COORDS);
    grVertexLayout(GR_PARAM_XY, GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_PARGB, GR_VERTEX_PARGB_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_Q, GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_Z, GR_VERTEX_OOZ_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_ST0, GR_VERTEX_SOW_TMU0_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_Q0, GR_VERTEX_OOW_TMU0_OFFSET << 2, GR_PARAM_DISABLE);
    grVertexLayout(GR_PARAM_ST1, GR_VERTEX_SOW_TMU1_OFFSET << 2, GR_PARAM_DISABLE);
    grVertexLayout(GR_PARAM_Q1, GR_VERTEX_OOW_TMU1_OFFSET << 2, GR_PARAM_DISABLE);
    END_BOARD_LOCK(fxMesa);
}
#else /* FX_USE_PARGB */
void
FX_setupGrVertexLayout(fxMesaContext fxMesa)
{
    BEGIN_BOARD_LOCK(fxMesa);
    grReset(GR_VERTEX_PARAMETER);

    grCoordinateSpace(GR_WINDOW_COORDS);
    grVertexLayout(GR_PARAM_XY, GR_VERTEX_X_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_RGB, GR_VERTEX_R_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_A, GR_VERTEX_A_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_Q, GR_VERTEX_OOW_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_Z, GR_VERTEX_OOZ_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_ST0, GR_VERTEX_SOW_TMU0_OFFSET << 2, GR_PARAM_ENABLE);
    grVertexLayout(GR_PARAM_Q0, GR_VERTEX_OOW_TMU0_OFFSET << 2, GR_PARAM_DISABLE);
    grVertexLayout(GR_PARAM_ST1, GR_VERTEX_SOW_TMU1_OFFSET << 2, GR_PARAM_DISABLE);
    grVertexLayout(GR_PARAM_Q1, GR_VERTEX_OOW_TMU1_OFFSET << 2, GR_PARAM_DISABLE);
    END_BOARD_LOCK(fxMesa);
}
#endif /* FX_USE_PARGB */

void
FX_grHints_NoLock(GrHint_t hintType, FxU32 hintMask)
{
    switch (hintType) {
    case GR_HINT_STWHINT:
        {
            if (hintMask & GR_STWHINT_W_DIFF_TMU0)
                grVertexLayout(GR_PARAM_Q0, GR_VERTEX_OOW_TMU0_OFFSET << 2,
                               GR_PARAM_ENABLE);
            else
                grVertexLayout(GR_PARAM_Q0, GR_VERTEX_OOW_TMU0_OFFSET << 2,
                               GR_PARAM_DISABLE);

            if (hintMask & GR_STWHINT_ST_DIFF_TMU1)
                grVertexLayout(GR_PARAM_ST1, GR_VERTEX_SOW_TMU1_OFFSET << 2,
                               GR_PARAM_ENABLE);
            else
                grVertexLayout(GR_PARAM_ST1, GR_VERTEX_SOW_TMU1_OFFSET << 2,
                               GR_PARAM_DISABLE);

            if (hintMask & GR_STWHINT_W_DIFF_TMU1)
                grVertexLayout(GR_PARAM_Q1, GR_VERTEX_OOW_TMU1_OFFSET << 2,
                               GR_PARAM_ENABLE);
            else
                grVertexLayout(GR_PARAM_Q1, GR_VERTEX_OOW_TMU1_OFFSET << 2,
                               GR_PARAM_DISABLE);

        }
    }
}

void
FX_grHints(fxMesaContext fxMesa, GrHint_t hintType, FxU32 hintMask)
{
    BEGIN_BOARD_LOCK(fxMesa);
    FX_grHints_NoLock(hintType, hintMask);
    END_BOARD_LOCK(fxMesa);
}

/* It appears to me that this function is needed either way. */
FX_GrContext_t
FX_grSstWinOpen(fxMesaContext fxMesa,
                FxU32 hWnd,
                GrScreenResolution_t screen_resolution,
                GrScreenRefresh_t refresh_rate,
                GrColorFormat_t color_format,
                GrOriginLocation_t origin_location,
                int nColBuffers, int nAuxBuffers)
{
    FX_GrContext_t i;
    BEGIN_BOARD_LOCK(fxMesa);
    i = grSstWinOpen(hWnd,
                     screen_resolution,
                     refresh_rate,
                     color_format, origin_location, nColBuffers, nAuxBuffers);

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
    END_BOARD_LOCK(fxMesa);
    return i;
}

