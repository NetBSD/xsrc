/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/fxddspan.c,v 1.7 2000/12/08 19:36:23 alanh Exp $ */
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


/* fxdd.c - 3Dfx VooDoo Mesa span and pixel functions */


#include "fxdrv.h"

/*
 * Examine the cliprects to generate an array of flags to indicate
 * which pixels in a span are visible.  Note: (x,y) is a screen
 * coordinate.
 */
static void
generate_vismask(const fxMesaContext fxMesa, GLint x, GLint y, GLint n,
                 GLubyte vismask[])
{
    GLboolean initialized = GL_FALSE;
    GLint i, j;

    /* Ensure we clear the visual mask */
    MEMSET(vismask, 0, n);

    /* turn on flags for all visible pixels */
    for (i = 0; i < fxMesa->numClipRects; i++) {
        const XF86DRIClipRectPtr rect = &fxMesa->pClipRects[i];

        if (y >= rect->y1 && y < rect->y2) {
            if (x >= rect->x1 && x + n <= rect->x2) {
                /* common case, whole span inside cliprect */
                MEMSET(vismask, 1, n);
                return;
            }
            if (x < rect->x2 && x + n >= rect->x1) {
                /* some of the span is inside the rect */
                GLint start, end;
                if (!initialized) {
                    MEMSET(vismask, 0, n);
                    initialized = GL_TRUE;
                }
                if (x < rect->x1)
                    start = rect->x1 - x;
                else
                    start = 0;
                if (x + n > rect->x2)
                    end = rect->x2 - x;
                else
                    end = n;
                assert(start >= 0);
                assert(end <= n);
                for (j = start; j < end; j++)
                    vismask[j] = 1;
            }
        }
    }
}

/*
 * Examine cliprects and determine if the given screen pixel is visible.
 */
static GLboolean
visible_pixel(const fxMesaContext fxMesa, int scrX, int scrY)
{
    int i;
    for (i = 0; i < fxMesa->numClipRects; i++) {
        const XF86DRIClipRectPtr rect = &fxMesa->pClipRects[i];
        if (scrX >= rect->x1 &&
            scrX < rect->x2 &&
            scrY >= rect->y1 && scrY < rect->y2) return GL_TRUE;
    }
    return GL_FALSE;
}

/*
 * 16bpp span/pixel functions
 */
static void
write_R5G6B5_rgba_span(const GLcontext * ctx, GLuint n, GLint x, GLint y,
                       const GLubyte rgba[][4], const GLubyte mask[])
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GrLfbInfo_t info;

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: write_R5G6B5_rgba_span\n");
    }
    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_WRITE_ONLY,
                  fxMesa->currentFB,
                  GR_LFBWRITEMODE_565,
                  GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        const GLint winX = fxMesa->x_offset;
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint scrX = winX + x;
        const GLint scrY = winY - y;
        const GLint srcStride = (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
            ? (fxMesa->screen_width) : (info.strideInBytes / 2);
        GLushort *data16 = (GLushort *) info.lfbPtr + scrY * srcStride + scrX;
        GLuint i;

        if (mask) {
            for (i = 0; i < n; i++) {
                if (visible_pixel(fxMesa, scrX + i, scrY) && mask[i]) {
                    GLushort pixel;
                    if (fxMesa->bgrOrder) {
                        pixel = PACK_BGR16(rgba[i][0],
                                           rgba[i][1],
                                           rgba[i][2]);
                    } else {
                        pixel = PACK_RGB16(rgba[i][0],
                                           rgba[i][1],
                                           rgba[i][2]);
                    }
                    data16[i] = pixel;
                }
            }
        } else {
            for (i = 0; i < n; i++) {
                if (visible_pixel(fxMesa, scrX + i, scrY)) {
                    GLushort pixel;

                    if (fxMesa->bgrOrder) {
                        pixel = PACK_BGR16(rgba[i][0],
                                           rgba[i][1],
                                           rgba[i][2]);
                    } else {
                        pixel = PACK_RGB16(rgba[i][0],
                                           rgba[i][1],
                                           rgba[i][2]);
                    }
                    data16[i] = pixel;
                }
            }
        }
        grLfbUnlock(GR_LFB_WRITE_ONLY, fxMesa->currentFB);
    }
    END_BOARD_LOCK(fxMesa);
}


static void
write_R5G6B5_rgb_span(const GLcontext * ctx, GLuint n, GLint x, GLint y,
                      const GLubyte rgb[][3], const GLubyte mask[])
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GrLfbInfo_t info;

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: write_R5G6B5_rgb_span\n");
    }
    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_WRITE_ONLY,
                  fxMesa->currentFB,
                  GR_LFBWRITEMODE_565,
                  GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        const GLint winX = fxMesa->x_offset;
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint scrX = winX + x;
        const GLint scrY = winY - y;
        const GLint srcStride = (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
            ? (fxMesa->screen_width) : (info.strideInBytes / 2);
        GLushort *data16 = (GLushort *) info.lfbPtr + scrY * srcStride + scrX;
        GLuint i;

        if (mask) {
            for (i = 0; i < n; i++) {
                if (visible_pixel(fxMesa, scrX + i, scrY) && mask[i]) {
                    GLushort pixel;
                    if (fxMesa->bgrOrder) {
                        pixel = PACK_BGR16(rgb[i][0],
                                           rgb[i][1],
                                           rgb[i][2]);
                    } else {
                        pixel = PACK_RGB16(rgb[i][0],
                                           rgb[i][1],
                                           rgb[i][2]);
                    }
                    data16[i] = pixel;
                }
            }
        } else {
            for (i = 0; i < n; i++) {
                if (visible_pixel(fxMesa, scrX + i, scrY)) {
                    GLushort pixel;
                    if (fxMesa->bgrOrder) {
                        pixel = PACK_BGR16(rgb[i][0],
                                           rgb[i][1],
                                           rgb[i][2]);
                    } else {
                        pixel = PACK_RGB16(rgb[i][0],
                                           rgb[i][1],
                                           rgb[i][2]);
                    }
                    data16[i] = pixel;
                }
            }
        }
        grLfbUnlock(GR_LFB_WRITE_ONLY, fxMesa->currentFB);
    }
    END_BOARD_LOCK(fxMesa);
}

static void
write_R5G6B5_mono_span(const GLcontext * ctx, GLuint n, GLint x, GLint y,
                       const GLubyte mask[])
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GrLfbInfo_t info;
    GLubyte *constantColor = (GLubyte *)(&fxMesa->color);
    GLushort pixel;

    if (fxMesa->bgrOrder) {
        pixel = PACK_BGR16(constantColor[0],
                           constantColor[1],
                           constantColor[2]);
    } else {
        pixel = PACK_RGB16(constantColor[0],
                           constantColor[1],
                           constantColor[2]);
    }

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: write_r5g6b5_mono_span\n");
    }
    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_WRITE_ONLY,
                  fxMesa->currentFB,
                  GR_LFBWRITEMODE_565,
                  GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        const GLint winX = fxMesa->x_offset;
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint scrX = winX + x;
        const GLint scrY = winY - y;
        const GLint srcStride = (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
            ? (fxMesa->screen_width) : (info.strideInBytes / 2);
        GLushort *data16 = (GLushort *) info.lfbPtr
            + scrY * srcStride + scrX;
        GLuint i;

        if (mask) {
            for (i = 0; i < n; i++) {
                if (visible_pixel(fxMesa, scrX + i, scrY) && mask[i]) {
                    data16[i] = pixel;
                }
            }
        } else {
            for (i = 0; i < n; i++) {
                if (visible_pixel(fxMesa, scrX + i, scrY)) {
                    data16[i] = pixel;
                }
            }
        }
        grLfbUnlock(GR_LFB_WRITE_ONLY, fxMesa->currentFB);
    }
    END_BOARD_LOCK(fxMesa);
}

/*
 * Read a span of 16-bit RGB pixels.  Note, we don't worry about cliprects
 * since OpenGL says obscured pixels have undefined values.
 */
static void
read_R5G6B5_span(const GLcontext * ctx, GLuint n, GLint x, GLint y,
                 GLubyte rgba[][4])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbInfo_t info;
    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_READ_ONLY,
                  fxMesa->currentFB,
                  GR_LFBWRITEMODE_ANY,
                  GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        const GLint winX = fxMesa->x_offset;
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint srcStride = (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
            ? (fxMesa->screen_width) : (info.strideInBytes / 2);
        const GLushort *data16 = (const GLushort *) info.lfbPtr
            + (winY - y) * srcStride + (winX + x);
        GLuint i, j;
        GLuint extraPixel = (n & 1);
        n -= extraPixel;
        for (i = j = 0; i < n; i += 2, j++) {
	    /* use data16[] to keep correct alignment */
            GLuint pixel0 = data16[i];
            GLuint pixel1 = data16[i+1];
            rgba[i][RCOMP] = FX_PixelToR(fxMesa, pixel0);
            rgba[i][GCOMP] = FX_PixelToG(fxMesa, pixel0);
            rgba[i][BCOMP] = FX_PixelToB(fxMesa, pixel0);
            rgba[i][ACOMP] = 255;
            rgba[i + 1][RCOMP] = FX_PixelToR(fxMesa, pixel1);
            rgba[i + 1][GCOMP] = FX_PixelToG(fxMesa, pixel1);
            rgba[i + 1][BCOMP] = FX_PixelToB(fxMesa, pixel1);
            rgba[i + 1][ACOMP] = 255;
        }
        if (extraPixel) {
            GLushort pixel = data16[n];
            rgba[n][RCOMP] = FX_PixelToR(fxMesa, pixel);
            rgba[n][GCOMP] = FX_PixelToG(fxMesa, pixel);
            rgba[n][BCOMP] = FX_PixelToB(fxMesa, pixel);
            rgba[n][ACOMP] = 255;
        }
        grLfbUnlock(GR_LFB_READ_ONLY, fxMesa->currentFB);
    }
    END_BOARD_LOCK(fxMesa);
}


static void
write_R5G6B5_pixels(const GLcontext * ctx,
                    GLuint n, const GLint x[], const GLint y[],
                    CONST GLubyte rgba[][4], const GLubyte mask[])
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GrLfbInfo_t info;

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: write_R5G6B5_pixels\n");
    }
    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_WRITE_ONLY,
                  fxMesa->currentFB,
                  GR_LFBWRITEMODE_565,
                  GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        const GLint winX = fxMesa->x_offset;
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint srcStride = (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
            ? (fxMesa->screen_width) : (info.strideInBytes / 2);
        GLuint i;

        for (i = 0; i < n; i++) {
            GLint scrX = winX + x[i];
            GLint scrY = winY - y[i];
            if (visible_pixel(fxMesa, scrX, scrY) && mask[i]) {
                GLushort *data16 = (GLushort *) info.lfbPtr
                    + scrY * srcStride + scrX;
                GLushort pixel;
                if (fxMesa->bgrOrder) {
                    pixel = PACK_BGR16(rgba[i][0],
                                       rgba[i][1],
                                       rgba[i][2]);
                } else {
                    pixel = PACK_RGB16(rgba[i][0],
                                       rgba[i][1],
                                       rgba[i][2]);
                }
                data16[0] = pixel;
            }
        }
        grLfbUnlock(GR_LFB_WRITE_ONLY, fxMesa->currentFB);
    }
    END_BOARD_LOCK(fxMesa);
}

static void
write_R5G6B5_mono_pixels(const GLcontext * ctx,
                         GLuint n, const GLint x[], const GLint y[],
                         const GLubyte mask[])
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GrLfbInfo_t info;
    GLubyte *constantColor = (GLubyte *)(&fxMesa->color);
    GLushort pixel;

    if (fxMesa->bgrOrder) {
        pixel = PACK_BGR16(constantColor[0],
                           constantColor[1],
                           constantColor[2]);
    } else {
        pixel = PACK_RGB16(constantColor[0],
                           constantColor[1],
                           constantColor[2]);
    }

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: write_R5G6B5_mono_pixels\n");
    }

    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_WRITE_ONLY,
                  fxMesa->currentFB,
                  GR_LFBWRITEMODE_565,
                  GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        const GLint winX = fxMesa->x_offset;
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint srcStride = (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
            ? (fxMesa->screen_width) : (info.strideInBytes / 2);
        GLuint i;

        for (i = 0; i < n; i++) {
            GLint scrX = winX + x[i];
            GLint scrY = winY - y[i];
            if (visible_pixel(fxMesa, scrX, scrY) && mask[i]) {
                GLushort *data16 = (GLushort *) info.lfbPtr
                    + scrY * srcStride + scrX;
                data16[0] = pixel;
            }
        }
        grLfbUnlock(GR_LFB_WRITE_ONLY, fxMesa->currentFB);
    }
    END_BOARD_LOCK(fxMesa);
}

static void
read_R5G6B5_pixels(const GLcontext * ctx,
                   GLuint n, const GLint x[], const GLint y[],
                   GLubyte rgba[][4], const GLubyte mask[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbInfo_t info;
    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_READ_ONLY,
                  fxMesa->currentFB,
                  GR_LFBWRITEMODE_ANY,
                  GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        const GLint winX = fxMesa->x_offset;
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint srcStride = (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
            ? (fxMesa->screen_width) : (info.strideInBytes / 2);
        GLuint i;
        for (i = 0; i < n; i++) {
            if (mask[i]) {
               const GLushort *data16 = (const GLushort *) info.lfbPtr
                  + (winY - y[i]) * srcStride + (winX + x[i]);
               GLushort pixel = *data16;
               rgba[i][RCOMP] = FX_PixelToR(fxMesa, pixel);
               rgba[i][GCOMP] = FX_PixelToG(fxMesa, pixel);
               rgba[i][BCOMP] = FX_PixelToB(fxMesa, pixel);
               rgba[i][ACOMP] = 255;
            }
        }
        grLfbUnlock(GR_LFB_READ_ONLY, fxMesa->currentFB);
    }
    END_BOARD_LOCK(fxMesa);
}


/*
 * 24bpp span/pixel functions
 */

static void
write_R8G8B8_rgb_span(const GLcontext *ctx, GLuint n, GLint x, GLint y,
                      const GLubyte rgb[][3], const GLubyte mask[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbWriteMode_t mode;
    GrLfbInfo_t info;

    if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
        mode = GR_LFBWRITEMODE_888;
    else
        mode = GR_LFBWRITEMODE_888;

    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_WRITE_ONLY,
                  fxMesa->currentFB,
                  mode, GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint winX = fxMesa->x_offset;
        const GLint scrX = winX + x;
        const GLint scrY = winY - y;

        if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT) {
            /*GLint dstStride = fxMesa->screen_width * 3; */
            GLint dstStride = info.strideInBytes / 1;
            GLubyte *dst = (GLubyte *) info.lfbPtr
                + (winY - y) * dstStride + (winX + x) * 1;
            GLuint *dst32 = (GLuint *) dst;
            GLubyte visMask[MAX_WIDTH];
            GLuint i;
            generate_vismask(fxMesa, scrX, scrY, n, visMask);
            for (i = 0; i < n; i++) {
                if (visMask[i] && (!mask || mask[i])) {
                    dst32[i] =
                        PACK_BGRA32(rgb[i][0], rgb[i][1], rgb[i][2], 255);
                }
            }
        }
        else {
            /* back buffer */
            GLint dstStride = info.strideInBytes;
            GLubyte *dst = (GLubyte *) info.lfbPtr
                + (winY - y) * dstStride + (winX + x) * 4;
            GLuint *dst32 = (GLuint *) dst;
            if (mask) {
                GLuint i;
                for (i = 0; i < n; i++) {
                    if (mask[i]) {
                        dst32[i] =
                            PACK_RGBA32(rgb[i][0], rgb[i][1], rgb[i][2], 255);
                    }
                }
            }
            else {
                GLuint i;
                for (i = 0; i < n; i++) {
                    dst32[i] =
                        PACK_RGBA32(rgb[i][0], rgb[i][1], rgb[i][2], 255);
                }
            }
        }
        grLfbUnlock(GR_LFB_WRITE_ONLY, fxMesa->currentFB);
    }
    END_BOARD_LOCK(fxMesa);
}



static void
write_R8G8B8_rgba_span(const GLcontext * ctx, GLuint n, GLint x, GLint y,
                       const GLubyte rgba[][4], const GLubyte mask[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbWriteMode_t mode;
    GrLfbInfo_t info;

    if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
        mode = GR_LFBWRITEMODE_8888;
    else
        mode = GR_LFBWRITEMODE_888;

    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_WRITE_ONLY,
                  fxMesa->currentFB,
                  mode, GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint winX = fxMesa->x_offset;
        const GLint scrX = winX + x;
        const GLint scrY = winY - y;

        if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT) {
            /* XXX have to do cliprect clipping! */
            GLint dstStride = fxMesa->screen_width * 4;
            GLubyte *dst = (GLubyte *) info.lfbPtr
                + (winY - y) * dstStride + (winX + x) * 4;
            GLuint *dst32 = (GLuint *) dst;
            GLubyte visMask[MAX_WIDTH];
            GLuint i;
            generate_vismask(fxMesa, scrX, scrY, n, visMask);
            for (i = 0; i < n; i++) {
                if (visMask[i] && (!mask || mask[i])) {
                    dst32[i] =
                        PACK_BGRA32(rgba[i][0], rgba[i][1], rgba[i][2],
                                    rgba[i][3]);
                }
            }
        }
        else {
            /* back buffer */
            GLint dstStride = info.strideInBytes;
            GLubyte *dst = (GLubyte *) info.lfbPtr
                + (winY - y) * dstStride + (winX + x) * 4;
            if (mask) {
                const GLuint *src32 = (const GLuint *) rgba;
                GLuint *dst32 = (GLuint *) dst;
                GLuint i;
                for (i = 0; i < n; i++) {
                    if (mask[i]) {
                        dst32[i] = src32[i];
                    }
                }
            }
            else {
                /* no mask, write all pixels */
                MEMCPY(dst, rgba, 4 * n);
            }
        }
        grLfbUnlock(GR_LFB_WRITE_ONLY, fxMesa->currentFB);
    }
    END_BOARD_LOCK(fxMesa);
}


static void
write_R8G8B8_mono_span(const GLcontext * ctx, GLuint n, GLint x, GLint y,
                       const GLubyte mask[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GLubyte rgba[MAX_WIDTH][4];
    GLuint *data = (GLuint *) rgba;
    GLuint i;

    /* XXX this is a simple-minded implementation but good enough for now */
    for (i = 0; i < n; i++) {
        data[i] = (GLuint) fxMesa->color;
    }
    write_R8G8B8_rgba_span(ctx, n, x, y, (const GLubyte(*)[4]) rgba, mask);
}


static void
read_R8G8B8_span(const GLcontext * ctx, GLuint n, GLint x, GLint y,
                 GLubyte rgba[][4])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbInfo_t info;

    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_READ_ONLY, fxMesa->currentFB, GR_LFBWRITEMODE_8888,
                  GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint winX = fxMesa->x_offset;

        if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT) {
            GLint srcStride = fxMesa->screen_width * 4;
            const GLubyte *src = (const GLubyte *) info.lfbPtr
                + (winY - y) * srcStride + (winX + x) * 4;
            GLuint i;
            for (i = 0; i < n; i++) {
                rgba[i][0] = src[i * 4 + 2];
                rgba[i][1] = src[i * 4 + 1];
                rgba[i][2] = src[i * 4 + 0];
                rgba[i][3] = src[i * 4 + 3];
            }
        }
        else {
            /* back buffer */
            GLint srcStride = info.strideInBytes / 2;
            const GLubyte *src = (const GLubyte *) info.lfbPtr
                + (winY - y) * srcStride + (winX + x) * 4;
            GLuint i;
            for (i = 0; i < n; i++) {
                rgba[i][0] = src[i * 4 + 2];
                rgba[i][1] = src[i * 4 + 1];
                rgba[i][2] = src[i * 4 + 0];
                rgba[i][3] = src[i * 4 + 3];
            }
        }
        grLfbUnlock(GR_LFB_READ_ONLY, fxMesa->currentFB);
    }
    END_BOARD_LOCK(fxMesa);
}


static void
write_R8G8B8_pixels(const GLcontext * ctx,
                    GLuint n, const GLint x[], const GLint y[],
                    CONST GLubyte rgba[][4], const GLubyte mask[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbWriteMode_t mode;
    GrLfbInfo_t info;

    if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT)
        mode = GR_LFBWRITEMODE_8888;
    else
        mode = GR_LFBWRITEMODE_888 /*565 */ ;

    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_WRITE_ONLY,
                  fxMesa->currentFB,
                  mode, GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT) {
            GLuint i;
            for (i = 0; i < n; i++) {
                const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
                const GLint winX = fxMesa->x_offset;
                const GLint scrX = winX + x[i];
                const GLint scrY = winY - y[i];
                if (mask[i] && visible_pixel(fxMesa, scrX, scrY)) {
                    GLint dstStride = fxMesa->screen_width * 4;
                    GLubyte *dst =
                        (GLubyte *) info.lfbPtr + scrY * dstStride + scrX * 4;
                    GLuint *dst32 = (GLuint *) dst;
                    *dst32 = PACK_BGRA32(rgba[i][0], rgba[i][1],
                                         rgba[i][2], rgba[i][3]);
                }
            }
        }
        else {
            /* back buffer */
            GLuint i;
            for (i = 0; i < n; i++) {
                const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
                const GLint winX = fxMesa->x_offset;
                const GLint scrX = winX + x[i];
                const GLint scrY = winY - y[i];
                if (mask[i] && visible_pixel(fxMesa, scrX, scrY)) {
                    GLint dstStride = info.strideInBytes;
                    GLubyte *dst =
                        (GLubyte *) info.lfbPtr + scrY * dstStride + scrX * 4;
                    GLuint *dst32 = (GLuint *) dst;
                    *dst32 = PACK_BGRA32(rgba[i][0], rgba[i][1],
                                         rgba[i][2], rgba[i][3]);
                }
            }
        }
        grLfbUnlock(GR_LFB_WRITE_ONLY, fxMesa->currentFB);
    }
    END_BOARD_LOCK(fxMesa);
}


static void
write_R8G8B8_mono_pixels(const GLcontext * ctx,
                         GLuint n, const GLint x[], const GLint y[],
                         const GLubyte mask[])
{
    printf("write_r8g8b8_mono_pixels\n");
}


static void
read_R8G8B8_pixels(const GLcontext * ctx,
                   GLuint n, const GLint x[], const GLint y[],
                   GLubyte rgba[][4], const GLubyte mask[])
{
    printf("read_R8G8B8_pixels %d\n", n);
}



/*
 * 32bpp span/pixel functions
 */

static void
write_R8G8B8A8_rgb_span(const GLcontext * ctx, GLuint n, GLint x, GLint y,
                        const GLubyte rgb[][3], const GLubyte mask[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbInfo_t info;

    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_WRITE_ONLY, fxMesa->currentFB, GR_LFBWRITEMODE_8888,
                  GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint winX = fxMesa->x_offset;
        const GLint scrX = winX + x;
        const GLint scrY = winY - y;
        GLubyte visMask[MAX_WIDTH];

        generate_vismask(fxMesa, scrX, scrY, n, visMask);

        if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT) {
            GLint dstStride = fxMesa->screen_width * 4;
            GLubyte *dst = (GLubyte *) info.lfbPtr
                + (winY - y) * dstStride + (winX + x) * 4;
            GLuint *dst32 = (GLuint *) dst;
            GLuint i;
            for (i = 0; i < n; i++) {
                if (visMask[i] && (!mask || mask[i])) {
                    dst32[i] =
                       PACK_BGRA32(rgb[i][0], rgb[i][1], rgb[i][2], 255);
                }
            }
        }
        else {
            /* back buffer */
            GLint dstStride = info.strideInBytes;
            GLubyte *dst = (GLubyte *) info.lfbPtr
                + (winY - y) * dstStride + (winX + x) * 4;
            GLuint *dst32 = (GLuint *) dst;
            if (mask) {
                GLuint i;
                for (i = 0; i < n; i++) {
                    if (visMask[i] && mask[i]) {
                        dst32[i] =
                            PACK_BGRA32(rgb[i][0], rgb[i][1], rgb[i][2], 255);
                    }
                }
            }
            else {
                GLuint i;
                for (i = 0; i < n; i++) {
                    if (visMask[i]) {
                        dst32[i] = PACK_BGRA32(rgb[i][0], rgb[i][1],
                                               rgb[i][2], 255);
                    }
                }
            }
        }
        grLfbUnlock(GR_LFB_WRITE_ONLY, fxMesa->currentFB);
    }
    END_BOARD_LOCK(fxMesa);
}


/*
 *XXX test of grLfbWriteRegion in 32bpp mode.  Doesn't seem to work!
 */
#if 0
static void
write_R8G8B8A8_rgb_span2(const GLcontext * ctx, GLuint n, GLint x, GLint y,
                         const GLubyte rgb[][3], const GLubyte mask[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GLint bottom = fxMesa->height + fxMesa->y_offset - 1;
    GLint x2 = fxMesa->x_offset + x;
    GLint y2 = bottom - y;

    FX_grLfbWriteRegion(fxMesa->currentFB, x2, y2, GR_LFB_SRC_FMT_888,
                        n, 1, 0, rgb);
}
#endif


static void
write_R8G8B8A8_rgba_span(const GLcontext * ctx, GLuint n, GLint x, GLint y,
                         const GLubyte rgba[][4], const GLubyte mask[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbInfo_t info;

    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_WRITE_ONLY, fxMesa->currentFB, GR_LFBWRITEMODE_8888,
                  GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint winX = fxMesa->x_offset;
        const GLint scrX = winX + x;
        const GLint scrY = winY - y;
        GLubyte visMask[MAX_WIDTH];

        generate_vismask(fxMesa, scrX, scrY, n, visMask);

        if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT) {
            GLint dstStride = fxMesa->screen_width * 4;
            GLubyte *dst = (GLubyte *) info.lfbPtr
                + (winY - y) * dstStride + (winX + x) * 4;
            GLuint *dst32 = (GLuint *) dst;
            GLuint i;
            for (i = 0; i < n; i++) {
                if (visMask[i] && (!mask || mask[i])) {
                    dst32[i] = PACK_BGRA32(rgba[i][0], rgba[i][1], rgba[i][2],
                                           rgba[i][3]);
                }
            }
        }
        else {
            /* back buffer */
            GLint dstStride = info.strideInBytes;
            GLubyte *dst = (GLubyte *) info.lfbPtr
                + (winY - y) * dstStride + (winX + x) * 4;
            GLuint *dst32 = (GLuint *) dst;
            GLubyte visMask[MAX_WIDTH];
            generate_vismask(fxMesa, scrX, scrY, n, visMask);
            if (mask) {
                GLuint i;
                for (i = 0; i < n; i++) {
                    if (visMask[i] && mask[i]) {
                        dst32[i] = PACK_BGRA32(rgba[i][0], rgba[i][1],
                                               rgba[i][2], rgba[i][3]);
                    }
                }
            }
            else {
                /* no mask, write all pixels */
                GLuint i;
                for (i = 0; i < n; i++) {
                   if (visMask[i]) {
                       dst32[i] = PACK_BGRA32(rgba[i][0], rgba[i][1],
                                              rgba[i][2], rgba[i][3]);
                   }
                }
            }
        }
        grLfbUnlock(GR_LFB_WRITE_ONLY, fxMesa->currentFB);
    }
    else {
        info.strideInBytes = -1;
    }
    END_BOARD_LOCK(fxMesa);
}


static void
write_R8G8B8A8_mono_span(const GLcontext * ctx, GLuint n, GLint x, GLint y,
                         const GLubyte mask[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GLubyte rgba[MAX_WIDTH][4];
    GLuint *data = (GLuint *) rgba;
    GLuint i;

    /* XXX this is a simple-minded implementation but good enough for now */
    for (i = 0; i < n; i++) {
        data[i] = (GLuint) fxMesa->color;
    }
    write_R8G8B8A8_rgba_span(ctx, n, x, y, (const GLubyte(*)[4]) rgba, mask);
}


static void
read_R8G8B8A8_span(const GLcontext * ctx, GLuint n, GLint x, GLint y,
                   GLubyte rgba[][4])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbInfo_t info;

    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_READ_ONLY, fxMesa->currentFB, GR_LFBWRITEMODE_ANY,
                  GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint winX = fxMesa->x_offset;

        if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT) {
            GLint srcStride = fxMesa->screen_width;
            const GLuint *src32 = (const GLuint *) info.lfbPtr
                + (winY - y) * srcStride + (winX + x);
            GLuint i;
	    MEMCPY(rgba, src32, n * 4);
            for (i = 0; i < n; i++) {
		rgba[i][0] ^= rgba[i][2];
		rgba[i][2] ^= rgba[i][0];
		rgba[i][0] ^= rgba[i][2];
            }
        }
        else {
            /* back buffer */
            GLint srcStride = info.strideInBytes / sizeof(GLuint);
            const GLuint *src32 = (const GLuint *) info.lfbPtr
                + (winY - y) * srcStride + (winX + x);
            GLuint i;
	    MEMCPY(rgba, src32, n * 4);
            for (i = 0; i < n; i++) {
		rgba[i][0] ^= rgba[i][2];
		rgba[i][2] ^= rgba[i][0];
		rgba[i][0] ^= rgba[i][2];
            }
        }
        grLfbUnlock(GR_LFB_READ_ONLY, fxMesa->currentFB);
    }
    else
        info.strideInBytes = -1;
    END_BOARD_LOCK(fxMesa);
}


static void
write_R8G8B8A8_pixels(const GLcontext * ctx,
                      GLuint n, const GLint x[], const GLint y[],
                      CONST GLubyte rgba[][4], const GLubyte mask[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbInfo_t info;

    BEGIN_BOARD_LOCK(fxMesa);
    info.size = sizeof(info);
    if (grLfbLock(GR_LFB_WRITE_ONLY, fxMesa->currentFB, GR_LFBWRITEMODE_8888,
                  GR_ORIGIN_UPPER_LEFT, FXFALSE, &info)) {
        if (fxMesa->glCtx->Color.DrawBuffer == GL_FRONT) {
            GLuint i;
            for (i = 0; i < n; i++) {
                const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
                const GLint winX = fxMesa->x_offset;
                const GLint scrX = winX + x[i];
                const GLint scrY = winY - y[i];
                if (mask[i] && visible_pixel(fxMesa, scrX, scrY)) {
                    GLint dstStride = fxMesa->screen_width * 4;
                    GLubyte *dst =
                        (GLubyte *) info.lfbPtr + scrY * dstStride + scrX * 4;
                    GLuint *dst32 = (GLuint *) dst;
                    *dst32 = PACK_BGRA32(rgba[i][0], rgba[i][1],
                                         rgba[i][2], rgba[i][3]);
                }
            }
        }
        else {
            /* back buffer */
            GLuint i;
            for (i = 0; i < n; i++) {
                const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
                const GLint winX = fxMesa->x_offset;
                const GLint scrX = winX + x[i];
                const GLint scrY = winY - y[i];
                if (mask[i] && visible_pixel(fxMesa, scrX, scrY)) {
                    GLint dstStride = info.strideInBytes;
                    GLubyte *dst =
                        (GLubyte *) info.lfbPtr + scrY * dstStride + scrX * 4;
                    GLuint *dst32 = (GLuint *) dst;
                    *dst32 = PACK_BGRA32(rgba[i][0], rgba[i][1],
                                         rgba[i][2], rgba[i][3]);
                }
            }
        }
        grLfbUnlock(GR_LFB_WRITE_ONLY, fxMesa->currentFB);
    }
    END_BOARD_LOCK(fxMesa);
}


static void
write_R8G8B8A8_mono_pixels(const GLcontext * ctx,
                           GLuint n, const GLint x[], const GLint y[],
                           const GLubyte mask[])
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GLuint i;
    GLuint color = fxMesa->color;
    for (i = 0; i < n; i++) {
        if (mask[i]) {
            write_R8G8B8A8_rgba_span(ctx, 1, x[i], y[i],
                                     (const GLubyte(*)[4]) &color, mask + i);
        }
    }
}



static void
read_R8G8B8A8_pixels(const GLcontext * ctx,
                     GLuint n, const GLint x[], const GLint y[],
                     GLubyte rgba[][4], const GLubyte mask[])
{
    GLuint i;
    for (i = 0; i < n; i++) {
        if (mask[i]) {
            read_R8G8B8A8_span(ctx, 1, x[i], y[i], rgba + i);
        }
    }
}



/*
 * Depth buffer read/write functions.
 */
/*
 * To read the frame buffer, we need to lock and unlock it.  The
 * four macros {READ,WRITE}_FB_SPAN_{LOCK,UNLOCK}
 * do this for us.
 *
 * Note that the lock must be matched with an unlock.  These
 * macros include a spare curly brace, so they must
 * be syntactically matched.
 *
 * Note, also, that you can't lock a buffer twice with different
 * modes.  That is to say, you can't lock a buffer in both read
 * and write modes.  The strideInBytes and LFB pointer will be
 * the same with read and write locks, so you can use either.
 * o The HW has different state for reads and writes, so
 *   locking it twice may give screwy results.
 * o The DRM won't let you lock twice.  It hangs.  This is probably
 *   because of the BEGIN_BOARD_LOCK IN THE *_FB_SPAN_LOCK macros,
 *   and could be eliminated with nonlocking lock routines.  But
 *   what's the point after all.
 */
#define READ_FB_SPAN_LOCK(fxMesa, info, target_buffer)              \
  BEGIN_BOARD_LOCK(fxMesa);                                         \
  (info).size=sizeof(info);                                         \
  if (grLfbLock(GR_LFB_READ_ONLY,                                   \
                target_buffer,                                      \
                GR_LFBWRITEMODE_ANY,                                \
                GR_ORIGIN_LOWER_LEFT,                               \
                FXFALSE,                                            \
                &(info))) {

#define READ_FB_SPAN_UNLOCK(fxMesa, target_buffer)                  \
    grLfbUnlock(GR_LFB_READ_ONLY, target_buffer);                   \
  } else {                                                          \
    fprintf(stderr, "fxDriver: Can't get %s (%d) read lock\n",      \
            (target_buffer == GR_BUFFER_BACKBUFFER)                 \
                ? "back buffer"                                     \
            : ((target_buffer == GR_BUFFER_AUXBUFFER)               \
                ? "depth buffer"                                    \
               : "unknown buffer"),                                 \
            target_buffer);                                         \
  }                                                                 \
  END_BOARD_LOCK(fxMesa);


#define WRITE_FB_SPAN_LOCK(fxMesa, info, target_buffer, write_mode) \
  BEGIN_BOARD_LOCK(fxMesa);                                         \
  info.size=sizeof(info);                                           \
  if (grLfbLock(GR_LFB_WRITE_ONLY,                                  \
                target_buffer,                                      \
                write_mode,                                         \
                GR_ORIGIN_LOWER_LEFT,                               \
                FXFALSE,                                            \
                &info)) {

#define WRITE_FB_SPAN_UNLOCK(fxMesa, target_buffer)                 \
    grLfbUnlock(GR_LFB_WRITE_ONLY, target_buffer);                  \
  } else {                                                          \
    fprintf(stderr, "fxDriver: Can't get %s (%d) write lock\n",     \
            (target_buffer == GR_BUFFER_BACKBUFFER)                 \
                ? "back buffer"                                     \
            : ((target_buffer == GR_BUFFER_AUXBUFFER)               \
                ? "depth buffer"                                    \
               : "unknown buffer"),                                 \
            target_buffer);                                         \
  }                                                                 \
  END_BOARD_LOCK(fxMesa);

/*
 * Because the Linear Frame Buffer is not necessarily aligned
 * with the depth buffer, we have to do some fiddling
 * around to get the right addresses.
 *
 * Perhaps a picture is in order.  The Linear Frame Buffer
 * looks like this:
 *
 *   |<----------------------info.strideInBytes------------->|
 *   |<-----physicalStrideInBytes------->|
 *   +-----------------------------------+xxxxxxxxxxxxxxxxxxx+
 *   |                                   |                   |
 *   |          Legal Memory             |  Forbidden Zone   |
 *   |                                   |                   |
 *   +-----------------------------------+xxxxxxxxxxxxxxxxxxx+
 *
 * You can only reliably read and write legal locations.  Reads
 * and writes from the Forbidden Zone will return undefined values,
 * and may cause segmentation faults.
 *
 * Now, the depth buffer may not end up in a location such each
 * scan line is an LFB line.  For example, the depth buffer may
 * look like this:
 *
 *    wrapped               ordinary.
 *   +-----------------------------------+xxxxxxxxxxxxxxxxxxx+
 *   |0000000000000000000000             |                   | back
 *   |1111111111111111111111             |                   | buffer
 *   |2222222222222222222222             |                   |
 *   |4096b align. padxx00000000000000000|  Forbidden Zone   | depth
 *   |0000              11111111111111111|                   | buffer
 *   |1111              22222222222222222|                   |
 *   |2222                               |                   |
 *   +-----------------------------------+xxxxxxxxxxxxxxxxxxx+
 * where each number is the scan line number.  We know it will
 * be aligned on 128 byte boundaries, at least.  Aligning this
 * on a scanline boundary causes the back and depth buffers to
 * thrash in the SST1 cache.  (Note that the back buffer is always
 * allocated at the beginning of LFB memory, and so it is always
 * properly aligned with the LFB stride.)
 *
 * We call the beginning of the line (which is the rightmost
 * part of the depth line in the picture above) the *ordinary* part
 * of the scanline, and the end of the line (which is the
 * leftmost part, one line below) the *wrapped* part of the scanline.
 * a.) We need to know what x value to subtract from the screen
 *     x coordinate to index into the wrapped part.
 * b.) We also need to figure out if we need to read from the ordinary
 *     part scan line, or from the wrapped part of the scan line.
 *
 * [ad a]
 * The first wrapped x coordinate is that coordinate such that
 *           depthBufferOffset&(info.strideInBytes) + x*elmentSize  {*}
 *                            > physicalStrideInBytes
 *     where depthBufferOffset is the LFB distance in bytes
 *     from the back buffer to the depth buffer.  The expression
 *           depthBufferOffset&(info.strideInBytes)
 *     is then the offset (in bytes) from the beginining of (any)
 *     depth buffer line to first element in the line.
 * Simplifying inequation {*} above we see that x is the smallest
 * value such that
 *         x*elementSize > physicalStrideInBytes                      {**}
 *                            - depthBufferOffset&(info.strideInBytes)
 * Now, we know that both the summands on the right are multiples of
 * 128, and elementSize <= 4, so if equality holds in {**}, x would
 * be a multiple of 32.  Thus we can set x to
 *         xwrapped = (physicalStrideInBytes
 *                      - depthBufferOffset&(info.strideInBytes))/elementSize
 *                      + 1
 *
 * [ad b]
 * Question b is now simple.  We read from the wrapped scan line if
 * x is greater than xwrapped.
 */
#define TILE_WIDTH_IN_BYTES		128
#define TILE_WIDTH_IN_ZOXELS(bpz)	(TILE_WIDTH_IN_BYTES/(bpz))
#define TILE_HEIGHT_IN_LINES		32
typedef struct
{
    void *lfbPtr;
    void *lfbWrapPtr;
    FxU32 LFBStrideInElts;
    GLint firstWrappedX;
}
LFBParameters;

/*
 * We need information about the back buffer.  Note that
 * this function *cannot be called* while the aux buffer
 * is locked, or the caller will hang.
 *
 * Only Glide knows the LFB address of the back and depth
 * offsets.  The upper levels of Mesa know the depth offset,
 * but that is not in LFB space, it is tiled memory space,
 * and is not useable for us.
 */
static void
GetBackBufferInfo(fxMesaContext fxMesa, GrLfbInfo_t * backBufferInfo)
{
    READ_FB_SPAN_LOCK(fxMesa, *backBufferInfo, GR_BUFFER_BACKBUFFER);
    READ_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_BACKBUFFER);
}

static void
GetFbParams(fxMesaContext fxMesa,
            GrLfbInfo_t * info,
            GrLfbInfo_t * backBufferInfo,
            LFBParameters * ReadParamsp, FxU32 elementSize)
{
    FxU32 physicalStrideInBytes, bufferOffset;
    FxU32 strideInBytes = info->strideInBytes;
    char *lfbPtr = (char *) (info->lfbPtr);

    /*
     * These two come directly from the info structure.
     */
    ReadParamsp->lfbPtr = (void *) lfbPtr;
    ReadParamsp->LFBStrideInElts = strideInBytes / elementSize;
    /*
     * Now, calculate the value of firstWrappedX.
     *
     * The physical stride is the screen width in bytes rounded up to
     * the next highest multiple of 128 bytes.  Note that this fails
     * when TILE_WIDTH_IN_BYTES is not a power of two.
     *
     * The buffer Offset is the distance between the beginning of
     * the LFB space, which is the beginning of the back buffer,
     * and the buffer we are gathering information about.
     * We want to make this routine usable for operations on the
     * back buffer, though we don't actually use it on the back
     * buffer.  Note, then, that if bufferOffset == 0, the firstWrappedX
     * is in the forbidden zone, and is therefore never reached.
     *
     * Note that if
     *     physicalStrideInBytes
     *             < bufferOffset&(info->strideInBytes-1)
     * the buffer begins in the forbidden zone.  We assert for this.
     */
    bufferOffset = (FxU32)(lfbPtr - (char *) backBufferInfo->lfbPtr);
    physicalStrideInBytes
        = (fxMesa->screen_width * elementSize + TILE_WIDTH_IN_BYTES - 1)
        & ~(TILE_WIDTH_IN_BYTES - 1);
    assert(physicalStrideInBytes > (bufferOffset & (strideInBytes - 1)));
    ReadParamsp->firstWrappedX
        = (physicalStrideInBytes
           - (bufferOffset & (strideInBytes - 1))) / elementSize;
    /*
     * This is the address of the next physical line.
     */
    ReadParamsp->lfbWrapPtr
        = (void *) ((char *) backBufferInfo->lfbPtr
                    + (bufferOffset & ~(strideInBytes - 1))
                    + (TILE_HEIGHT_IN_LINES) * strideInBytes);
}

/*
 * These macros fetch data from the frame buffer.  The type is
 * the type of data we want to fetch.  It should match the type
 * whose size was used with GetFbParams to fill in the structure
 * in *ReadParamsp.  We have a macro to read the ordinary
 * part, a second macro to read the wrapped part, and one which
 * will do either.  When we are reading a span, we will know
 * when the ordinary part ends, so there's no need to test for
 * it.  However, when reading and writing pixels, we don't
 * necessarily know.  I suppose it's a matter of taste whether
 * it's better in the macro or in the call.
 *
 * Recall that x and y are screen coordinates.
 */
#define GET_FB_DATA(ReadParamsp, type, x, y)                        \
   (((x) < (ReadParamsp)->firstWrappedX)                            \
        ? (((type *)((ReadParamsp)->lfbPtr))                        \
                 [(y) * ((ReadParamsp)->LFBStrideInElts)            \
                   + (x)])                                          \
        : (((type *)((ReadParamsp)->lfbWrapPtr))                    \
                 [((y)) * ((ReadParamsp)->LFBStrideInElts)          \
                   + ((x) - (ReadParamsp)->firstWrappedX)]))
#define GET_ORDINARY_FB_DATA(ReadParamsp, type, x, y)               \
    (((type *)((ReadParamsp)->lfbPtr))                              \
                 [(y) * ((ReadParamsp)->LFBStrideInElts)            \
                   + (x)])
#define GET_WRAPPED_FB_DATA(ReadParamsp, type, x, y)                \
    (((type *)((ReadParamsp)->lfbWrapPtr))                          \
                 [((y)) * ((ReadParamsp)->LFBStrideInElts)          \
                   + ((x) - (ReadParamsp)->firstWrappedX)])
#define PUT_FB_DATA(ReadParamsp, type, x, y, value)                        \
    (GET_FB_DATA(ReadParamsp, type, x, y) = (type)(value))
#define PUT_ORDINARY_FB_DATA(ReadParamsp, type, x, y, value)              \
    (GET_ORDINARY_FB_DATA(ReadParamsp, type, x, y) = (type)(value))
#define PUT_WRAPPED_FB_DATA(ReadParamsp, type, x, y, value)                \
    (GET_WRAPPED_FB_DATA(ReadParamsp, type, x, y) = (type)(value))

static void
fxDDWriteDepthSpan(GLcontext * ctx,
                   GLuint n, GLint x, GLint y, const GLdepth depth[],
                   const GLubyte mask[])
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GLint bottom = fxMesa->y_offset + fxMesa->height - 1;
    GLuint depth_size = fxMesa->glVis->DepthBits;
    GLuint stencil_size = fxMesa->glVis->StencilBits;
    GrLfbInfo_t info;
    GLubyte visMask[MAX_WIDTH];

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDWriteDepthSpan(...)\n");
    }

    assert((depth_size == 16) || (depth_size == 24) || (depth_size == 32));
    /*
     * Convert x and y to screen coordinates.
     */
    x += fxMesa->x_offset;
    y = bottom - y;
    if (mask) {
        GLint i;
        GLushort d16;
        GrLfbInfo_t backBufferInfo;

        switch (depth_size) {
        case 16:
            GetBackBufferInfo(fxMesa, &backBufferInfo);
           /*
            * Note that the _LOCK macro adds a curly brace,
            * and the UNLOCK macro removes it.
            */
            WRITE_FB_SPAN_LOCK(fxMesa, info, GR_BUFFER_AUXBUFFER,
                               GR_LFBWRITEMODE_ANY);
            generate_vismask(fxMesa, x, y, n, visMask);
            {
                LFBParameters ReadParams;
                int wrappedPartStart;
                GetFbParams(fxMesa, &info, &backBufferInfo,
                            &ReadParams, sizeof(GLushort));
                if (ReadParams.firstWrappedX <= x) {
                    wrappedPartStart = 0;
                }
                else if (n <= (ReadParams.firstWrappedX - x)) {
                    wrappedPartStart = n;
                }
                else {
                    wrappedPartStart = (ReadParams.firstWrappedX - x);
                }
                for (i = 0; i < wrappedPartStart; i++) {
                    if (mask[i] && visMask[i]) {
                        d16 = depth[i];
                        PUT_ORDINARY_FB_DATA(&ReadParams, GLushort, x + i, y, d16);
                    }
                }
                for (; i < n; i++) {
                    if (mask[i] && visMask[i]) {
                        d16 = depth[i];
                        PUT_WRAPPED_FB_DATA(&ReadParams, GLushort, x + i, y, d16);
                    }
                }
            }
            WRITE_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
            break;
        case 24:
        case 32:
            GetBackBufferInfo(fxMesa, &backBufferInfo);
           /*
            * Note that the _LOCK macro adds a curly brace,
            * and the UNLOCK macro removes it.
            */
            WRITE_FB_SPAN_LOCK(fxMesa, info, GR_BUFFER_AUXBUFFER,
                               GR_LFBWRITEMODE_ANY);
            generate_vismask(fxMesa, x, y, n, visMask);
            {
                LFBParameters ReadParams;
                int wrappedPartStart;
                GetFbParams(fxMesa, &info, &backBufferInfo,
                            &ReadParams, sizeof(GLuint));
                if (ReadParams.firstWrappedX <= x) {
                    wrappedPartStart = 0;
                }
                else if (n <= (ReadParams.firstWrappedX - x)) {
                    wrappedPartStart = n;
                }
                else {
                    wrappedPartStart = (ReadParams.firstWrappedX - x);
                }
                for (i = 0; i < wrappedPartStart; i++) {
                    GLuint d32;
                    if (mask[i] && visMask[i]) {
                        if (stencil_size > 0) {
                            d32 =
                                GET_ORDINARY_FB_DATA(&ReadParams, GLuint,
                                                     x + i, y);
                            d32 =
                                (d32 & 0xFF000000) | (depth[i] & 0x00FFFFFF);
                        }
                        else {
                            d32 = depth[i];
                        }
                        PUT_ORDINARY_FB_DATA(&ReadParams, GLuint, x + i, y, d32);
                    }
                }
                for (; i < n; i++) {
                    GLuint d32;
                    if (mask[i] && visMask[i]) {
                        if (stencil_size > 0) {
                            d32 =
                                GET_WRAPPED_FB_DATA(&ReadParams, GLuint,
                                                    x + i, y);
                            d32 =
                                (d32 & 0xFF000000) | (depth[i] & 0x00FFFFFF);
                        }
                        else {
                            d32 = depth[i];
                        }
                        PUT_WRAPPED_FB_DATA(&ReadParams, GLuint, x + i, y, d32);
                    }
                }
            }
            WRITE_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
            break;
        }
    }
    else {
        GLint i;
        GLuint d32;
        GLushort d16;
        GrLfbInfo_t backBufferInfo;

        switch (depth_size) {
        case 16:
            GetBackBufferInfo(fxMesa, &backBufferInfo);
           /*
            * Note that the _LOCK macro adds a curly brace,
            * and the UNLOCK macro removes it.
            */
            WRITE_FB_SPAN_LOCK(fxMesa, info,
                               GR_BUFFER_AUXBUFFER, GR_LFBWRITEMODE_ANY);
            generate_vismask(fxMesa, x, y, n, visMask);
            {
                LFBParameters ReadParams;
                GLuint wrappedPartStart;
                GetFbParams(fxMesa, &info, &backBufferInfo,
                            &ReadParams, sizeof(GLushort));
                if (ReadParams.firstWrappedX <= x) {
                    wrappedPartStart = 0;
                }
                else if (n <= (ReadParams.firstWrappedX - x)) {
                    wrappedPartStart = n;
                }
                else {
                    wrappedPartStart = (ReadParams.firstWrappedX - x);
                }
                for (i = 0; i < wrappedPartStart; i++) {
                    if (visMask[i]) {
                        d16 = depth[i];
                        PUT_ORDINARY_FB_DATA(&ReadParams,
                                             GLushort,
                                             x + i, y,
                                             d16);
                    }
                }
                for (; i < n; i++) {
                    if (visMask[i]) {
                        d16 = depth[i];
                        PUT_WRAPPED_FB_DATA(&ReadParams,
                                            GLushort,
                                            x + i, y,
                                            d16);
                    }
                }
            }
            WRITE_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
            break;
        case 24:
        case 32:
            GetBackBufferInfo(fxMesa, &backBufferInfo);
           /*
            * Note that the _LOCK macro adds a curly brace,
            * and the UNLOCK macro removes it.
            */
            WRITE_FB_SPAN_LOCK(fxMesa, info,
                               GR_BUFFER_AUXBUFFER, GR_LFBWRITEMODE_ANY);
            generate_vismask(fxMesa, x, y, n, visMask);
            {
                LFBParameters ReadParams;
                GLuint wrappedPartStart;

                GetFbParams(fxMesa, &info, &backBufferInfo,
                            &ReadParams, sizeof(GLuint));
                if (ReadParams.firstWrappedX <= x) {
                    wrappedPartStart = 0;
                }
                else if (n <= (ReadParams.firstWrappedX - x)) {
                    wrappedPartStart = n;
                }
                else {
                    wrappedPartStart = (ReadParams.firstWrappedX - x);
                }
                for (i = 0; i < wrappedPartStart; i++) {
                    if (visMask[i]) {
                        if (stencil_size > 0) {
                            d32 = GET_ORDINARY_FB_DATA(&ReadParams, GLuint, x + i, y);
                            d32 =
                                (d32 & 0xFF000000) | (depth[i] & 0x00FFFFFF);
                        }
                        else {
                            d32 = depth[i];
                        }
                        PUT_ORDINARY_FB_DATA(&ReadParams, GLuint, x + i, y, d32);
                    }
                }
                for (; i < n; i++) {
                    if (visMask[i]) {
                        if (stencil_size > 0) {
                            d32 = GET_WRAPPED_FB_DATA(&ReadParams, GLuint, x + i, y);
                            d32 =
                                (d32 & 0xFF000000) | (depth[i] & 0x00FFFFFF);
                        }
                        else {
                            d32 = depth[i];
                        }
                        PUT_WRAPPED_FB_DATA(&ReadParams, GLuint, x + i, y, d32);
                    }
                }
            }
            WRITE_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
            break;
        }
    }
}

static void
fxDDReadDepthSpan(GLcontext * ctx,
                  GLuint n, GLint x, GLint y, GLdepth depth[])
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GLint bottom = fxMesa->height + fxMesa->y_offset - 1;
    GLuint i;
    GLuint depth_size = fxMesa->glVis->DepthBits;
    GrLfbInfo_t info;

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDReadDepthSpan(...)\n");
    }

    /*
     * Convert to screen coordinates.
     */
    x += fxMesa->x_offset;
    y = bottom - y;
    switch (depth_size) {
    case 16:
        {
            LFBParameters ReadParams;
            GrLfbInfo_t backBufferInfo;
            int wrappedPartStart;
            GetBackBufferInfo(fxMesa, &backBufferInfo);
           /*
            * Note that the _LOCK macro adds a curly brace,
            * and the UNLOCK macro removes it.
            */
            READ_FB_SPAN_LOCK(fxMesa, info, GR_BUFFER_AUXBUFFER);
            GetFbParams(fxMesa, &info, &backBufferInfo,
                        &ReadParams, sizeof(GLushort));
            if (ReadParams.firstWrappedX <= x) {
                wrappedPartStart = 0;
            }
            else if (n <= (ReadParams.firstWrappedX - x)) {
                wrappedPartStart = n;
            }
            else {
                wrappedPartStart = (ReadParams.firstWrappedX - x);
            }
            /*
             * Read the line.
             */
            for (i = 0; i < wrappedPartStart; i++) {
                depth[i] =
                    GET_ORDINARY_FB_DATA(&ReadParams, GLushort, x + i, y);
            }
            for (; i < n; i++) {
                depth[i] = GET_WRAPPED_FB_DATA(&ReadParams, GLushort,
                                               x + i, y);
            }
            READ_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
            break;
        }
    case 24:
    case 32:
        {
            LFBParameters ReadParams;
            GrLfbInfo_t backBufferInfo;
            int wrappedPartStart;
            GLuint stencil_size = fxMesa->glVis->StencilBits;
            GetBackBufferInfo(fxMesa, &backBufferInfo);
           /*
            * Note that the _LOCK macro adds a curly brace,
            * and the UNLOCK macro removes it.
            */
            READ_FB_SPAN_LOCK(fxMesa, info, GR_BUFFER_AUXBUFFER);
            GetFbParams(fxMesa, &info, &backBufferInfo,
                        &ReadParams, sizeof(GLuint));
            if (ReadParams.firstWrappedX <= x) {
                wrappedPartStart = 0;
            }
            else if (n <= (ReadParams.firstWrappedX - x)) {
                wrappedPartStart = n;
            }
            else {
                wrappedPartStart = (ReadParams.firstWrappedX - x);
            }
            /*
             * Read the line.
             */
            for (i = 0; i < wrappedPartStart; i++) {
                const GLuint mask =
                    (stencil_size > 0) ? 0x00FFFFFF : 0xFFFFFFFF;
                depth[i] =
                    GET_ORDINARY_FB_DATA(&ReadParams, GLuint, x + i, y);
                depth[i] &= mask;
            }
            for (; i < n; i++) {
                const GLuint mask =
                    (stencil_size > 0) ? 0x00FFFFFF : 0xFFFFFFFF;
                depth[i] = GET_WRAPPED_FB_DATA(&ReadParams, GLuint, x + i, y);
                depth[i] &= mask;
            }
            READ_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
            break;
        }
    }
}


static void
fxDDWriteDepthPixels(GLcontext * ctx,
                     GLuint n, const GLint x[], const GLint y[],
                     const GLdepth depth[], const GLubyte mask[])
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GLint bottom = fxMesa->height + fxMesa->y_offset - 1;
    GLuint i;
    GLushort d16;
    GLuint d32;
    GLuint depth_size = fxMesa->glVis->DepthBits;
    GLuint stencil_size = fxMesa->glVis->StencilBits;
    GrLfbInfo_t info;
    int xpos;
    int ypos;
    GrLfbInfo_t backBufferInfo;

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDWriteDepthPixels(...)\n");
    }

    switch (depth_size) {
    case 16:
        GetBackBufferInfo(fxMesa, &backBufferInfo);
       /*
        * Note that the _LOCK macro adds a curly brace,
        * and the UNLOCK macro removes it.
        */
        WRITE_FB_SPAN_LOCK(fxMesa, info,
                           GR_BUFFER_AUXBUFFER, GR_LFBWRITEMODE_ANY);
        {
            LFBParameters ReadParams;
            GetFbParams(fxMesa, &info, &backBufferInfo,
                        &ReadParams, sizeof(GLushort));
            for (i = 0; i < n; i++) {
                if (mask[i] && visible_pixel(fxMesa, x[i], y[i])) {
                    xpos = x[i] + fxMesa->x_offset;
                    ypos = bottom - y[i];
                    d16 = depth[i];
                    PUT_FB_DATA(&ReadParams, GLushort, xpos, ypos, d16);
                }
            }
        }
        WRITE_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
        break;
    case 24:
    case 32:
        GetBackBufferInfo(fxMesa, &backBufferInfo);
       /*
        * Note that the _LOCK macro adds a curly brace,
        * and the UNLOCK macro removes it.
        */
        WRITE_FB_SPAN_LOCK(fxMesa, info,
                           GR_BUFFER_AUXBUFFER, GR_LFBWRITEMODE_ANY);
        {
            LFBParameters ReadParams;
            GetFbParams(fxMesa, &info, &backBufferInfo,
                        &ReadParams, sizeof(GLuint));
            for (i = 0; i < n; i++) {
                if (mask[i]) {
                    if (visible_pixel(fxMesa, x[i], y[i])) {
                        xpos = x[i] + fxMesa->x_offset;
                        ypos = bottom - y[i];
                        if (stencil_size > 0) {
                            d32 =
                                GET_FB_DATA(&ReadParams, GLuint, xpos, ypos);
                            d32 = (d32 & 0xFF000000) | (depth[i] & 0xFFFFFF);
                        }
                        else {
                            d32 = depth[i];
                        }
                        PUT_FB_DATA(&ReadParams, GLuint, xpos, ypos, d32);
                    }
                }
            }
        }
        WRITE_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
        break;
    }
}


static void
fxDDReadDepthPixels(GLcontext * ctx, GLuint n,
                    const GLint x[], const GLint y[], GLdepth depth[])
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GLint bottom = fxMesa->height + fxMesa->y_offset - 1;
    GLuint i;
    GLuint depth_size = fxMesa->glVis->DepthBits;
    GLushort d16;
    int xpos;
    int ypos;
    GrLfbInfo_t info;
    GLuint stencil_size;
    GrLfbInfo_t backBufferInfo;

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDReadDepthPixels(...)\n");
    }

    assert((depth_size == 16) || (depth_size == 24) || (depth_size == 32));
    switch (depth_size) {
    case 16:
        GetBackBufferInfo(fxMesa, &backBufferInfo);
       /*
        * Note that the _LOCK macro adds a curly brace,
        * and the UNLOCK macro removes it.
        */
        READ_FB_SPAN_LOCK(fxMesa, info, GR_BUFFER_AUXBUFFER);
        {
            LFBParameters ReadParams;
            GetFbParams(fxMesa, &info, &backBufferInfo,
                        &ReadParams, sizeof(GLushort));
            for (i = 0; i < n; i++) {
                /*
                 * Convert to screen coordinates.
                 */
                xpos = x[i] + fxMesa->x_offset;
                ypos = bottom - y[i];
                d16 = GET_FB_DATA(&ReadParams, GLushort, xpos, ypos);
                depth[i] = d16;
            }
        }
        READ_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
        break;
    case 24:
    case 32:
        GetBackBufferInfo(fxMesa, &backBufferInfo);
       /*
        * Note that the _LOCK macro adds a curly brace,
        * and the UNLOCK macro removes it.
        */
        READ_FB_SPAN_LOCK(fxMesa, info, GR_BUFFER_AUXBUFFER);
        stencil_size = fxMesa->glVis->StencilBits;
        {
            LFBParameters ReadParams;
            GetFbParams(fxMesa, &info, &backBufferInfo,
                        &ReadParams, sizeof(GLuint));
            for (i = 0; i < n; i++) {
                GLuint d32;

                /*
                 * Convert to screen coordinates.
                 */
                xpos = x[i] + fxMesa->x_offset;
                ypos = bottom - y[i];
                d32 = GET_FB_DATA(&ReadParams, GLuint, xpos, ypos);
                if (stencil_size > 0) {
                    d32 &= 0x00FFFFFF;
                }
                depth[i] = d32;
            }
        }
        READ_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
        break;
    default:
        assert(0);
    }
}

/*
 * Stencil buffer read/write functions.
 */
#define EXTRACT_S_FROM_ZS(zs) (((zs) >> 24) & 0xFF)
#define EXTRACT_Z_FROM_ZS(zs) ((zs) & 0xffffff)
#define BUILD_ZS(z, s)  (((s) << 24) | (z))

static void
write_stencil_span(GLcontext * ctx, GLuint n, GLint x, GLint y,
                   const GLstencil stencil[], const GLubyte mask[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbInfo_t info;
    GrLfbInfo_t backBufferInfo;

    GetBackBufferInfo(fxMesa, &backBufferInfo);
   /*
    * Note that the _LOCK macro adds a curly brace,
    * and the UNLOCK macro removes it.
    */
    WRITE_FB_SPAN_LOCK(fxMesa, info, GR_BUFFER_AUXBUFFER, GR_LFBWRITEMODE_ANY);
    {
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint winX = fxMesa->x_offset;
        const GLint scrX = winX + x;
        const GLint scrY = winY - y;
        LFBParameters ReadParams;
        GLubyte visMask[MAX_WIDTH];
        GLuint i;
        int wrappedPartStart;

        GetFbParams(fxMesa, &info, &backBufferInfo, &ReadParams,
                    sizeof(GLuint));
        if (ReadParams.firstWrappedX <= x) {
            wrappedPartStart = 0;
        }
        else if (n <= (ReadParams.firstWrappedX - x)) {
            wrappedPartStart = n;
        }
        else {
            wrappedPartStart = (ReadParams.firstWrappedX - x);
        }
        generate_vismask(fxMesa, scrX, scrY, n, visMask);
        for (i = 0; i < wrappedPartStart; i++) {
            if (visMask[i] && (!mask || mask[i])) {
                GLuint z = GET_ORDINARY_FB_DATA(&ReadParams, GLuint,
                                                scrX + i, scrY) & 0x00FFFFFF;
                z |= (stencil[i] & 0xFF) << 24;
                PUT_ORDINARY_FB_DATA(&ReadParams, GLuint, scrX + i, scrY, z);
            }
        }
        for (; i < n; i++) {
            if (visMask[i] && (!mask || mask[i])) {
                GLuint z = GET_WRAPPED_FB_DATA(&ReadParams, GLuint,
                                               scrX + i, scrY) & 0x00FFFFFF;
                z |= (stencil[i] & 0xFF) << 24;
                PUT_WRAPPED_FB_DATA(&ReadParams, GLuint, scrX + i, scrY, z);
            }
        }
    }
    WRITE_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
}


static void
read_stencil_span(GLcontext * ctx, GLuint n, GLint x, GLint y,
                  GLstencil stencil[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbInfo_t info;
    GrLfbInfo_t backBufferInfo;

    GetBackBufferInfo(fxMesa, &backBufferInfo);
   /*
    * Note that the _LOCK macro adds a curly brace,
    * and the UNLOCK macro removes it.
    */
    READ_FB_SPAN_LOCK(fxMesa, info, GR_BUFFER_AUXBUFFER);
    {
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint winX = fxMesa->x_offset;
        GLuint i;
        LFBParameters ReadParams;
        int wrappedPartStart;

        /*
         * Convert to screen coordinates.
         */
        x += winX;
        y = winY - y;
        GetFbParams(fxMesa, &info, &backBufferInfo, &ReadParams,
                    sizeof(GLuint));
        if (ReadParams.firstWrappedX <= x) {
            wrappedPartStart = 0;
        }
        else if (n <= (ReadParams.firstWrappedX - x)) {
            wrappedPartStart = n;
        }
        else {
            wrappedPartStart = (ReadParams.firstWrappedX - x);
        }
        for (i = 0; i < wrappedPartStart; i++) {
            stencil[i] = (GET_ORDINARY_FB_DATA(&ReadParams, GLuint,
                                               x + i, y) >> 24) & 0xFF;
        }
        for (; i < n; i++) {
            stencil[i] = (GET_WRAPPED_FB_DATA(&ReadParams, GLuint,
                                              x + i, y) >> 24) & 0xFF;
        }
    }
    READ_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
}


static void
write_stencil_pixels(GLcontext * ctx, GLuint n,
                     const GLint x[], const GLint y[],
                     const GLstencil stencil[], const GLubyte mask[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbInfo_t info;
    GrLfbInfo_t backBufferInfo;

    GetBackBufferInfo(fxMesa, &backBufferInfo);
   /*
    * Note that the _LOCK macro adds a curly brace,
    * and the UNLOCK macro removes it.
    */
    WRITE_FB_SPAN_LOCK(fxMesa, info, GR_BUFFER_AUXBUFFER, GR_LFBWRITEMODE_ANY);
    {
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint winX = fxMesa->x_offset;
        LFBParameters ReadParams;
        GLuint i;

        GetFbParams(fxMesa, &info, &backBufferInfo, &ReadParams,
                    sizeof(GLuint));
        for (i = 0; i < n; i++) {
            const GLint scrX = winX + x[i];
            const GLint scrY = winY - y[i];
            if ((!mask || mask[i]) && visible_pixel(fxMesa, scrX, scrY)) {
                GLuint z =
                    GET_FB_DATA(&ReadParams, GLuint, scrX, scrY) & 0x00FFFFFF;
                z |= (stencil[i] & 0xFF) << 24;
                PUT_FB_DATA(&ReadParams, GLuint, scrX, scrY, z);
            }
        }
    }
    WRITE_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
}


static void
read_stencil_pixels(GLcontext * ctx, GLuint n, const GLint x[],
                    const GLint y[], GLstencil stencil[])
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    GrLfbInfo_t info;
    GrLfbInfo_t backBufferInfo;

    GetBackBufferInfo(fxMesa, &backBufferInfo);
   /*
    * Note that the _LOCK macro adds a curly brace,
    * and the UNLOCK macro removes it.
    */
    READ_FB_SPAN_LOCK(fxMesa, info, GR_BUFFER_AUXBUFFER);
    {
        const GLint winY = fxMesa->y_offset + fxMesa->height - 1;
        const GLint winX = fxMesa->x_offset;
        GLuint i;
        LFBParameters ReadParams;

        GetFbParams(fxMesa, &info, &backBufferInfo, &ReadParams,
                    sizeof(GLuint));
        for (i = 0; i < n; i++) {
            const GLint scrX = winX + x[i];
            const GLint scrY = winY - y[i];
            stencil[i] =
                (GET_FB_DATA(&ReadParams, GLuint, scrX, scrY) >> 24) & 0xFF;
        }
    }
    READ_FB_SPAN_UNLOCK(fxMesa, GR_BUFFER_AUXBUFFER);
}

void
fxSetupDDSpanPointers(GLcontext * ctx)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);

    if (ctx->Visual->RedBits == 5 &&
        ctx->Visual->GreenBits == 6 &&
        ctx->Visual->BlueBits == 5 && ctx->Visual->AlphaBits == 0) {
        /* 16bpp mode */
        ctx->Driver.WriteRGBASpan = write_R5G6B5_rgba_span;
        ctx->Driver.WriteRGBSpan = write_R5G6B5_rgb_span;
        ctx->Driver.WriteMonoRGBASpan = write_R5G6B5_mono_span;
        ctx->Driver.WriteRGBAPixels = write_R5G6B5_pixels;
        ctx->Driver.WriteMonoRGBAPixels = write_R5G6B5_mono_pixels;
        ctx->Driver.ReadRGBASpan = read_R5G6B5_span;
        ctx->Driver.ReadRGBAPixels = read_R5G6B5_pixels;
    }
    else if (ctx->Visual->RedBits == 8 &&
             ctx->Visual->GreenBits == 8 &&
             ctx->Visual->BlueBits == 8 && ctx->Visual->AlphaBits == 0) {
        /* 24bpp mode */
        ctx->Driver.WriteRGBASpan = write_R8G8B8_rgba_span;
        ctx->Driver.WriteRGBSpan = write_R8G8B8_rgb_span;
        ctx->Driver.WriteMonoRGBASpan = write_R8G8B8_mono_span;
        ctx->Driver.WriteRGBAPixels = write_R8G8B8_pixels;
        ctx->Driver.WriteMonoRGBAPixels = write_R8G8B8_mono_pixels;
        ctx->Driver.ReadRGBASpan = read_R8G8B8_span;
        ctx->Driver.ReadRGBAPixels = read_R8G8B8_pixels;
    }
    else if (ctx->Visual->RedBits == 8 &&
             ctx->Visual->GreenBits == 8 &&
             ctx->Visual->BlueBits == 8 && ctx->Visual->AlphaBits == 8) {
        /* 32bpp mode */
        ctx->Driver.WriteRGBASpan = write_R8G8B8A8_rgba_span;
        ctx->Driver.WriteRGBSpan = write_R8G8B8A8_rgb_span;
        ctx->Driver.WriteMonoRGBASpan = write_R8G8B8A8_mono_span;
        ctx->Driver.WriteRGBAPixels = write_R8G8B8A8_pixels;
        ctx->Driver.WriteMonoRGBAPixels = write_R8G8B8A8_mono_pixels;
        ctx->Driver.ReadRGBASpan = read_R8G8B8A8_span;
        ctx->Driver.ReadRGBAPixels = read_R8G8B8A8_pixels;
    }
    else {
        abort();
    }

    if (fxMesa->haveHwStencil) {
        ctx->Driver.WriteStencilSpan = write_stencil_span;
        ctx->Driver.ReadStencilSpan = read_stencil_span;
        ctx->Driver.WriteStencilPixels = write_stencil_pixels;
        ctx->Driver.ReadStencilPixels = read_stencil_pixels;
    }

    ctx->Driver.WriteDepthSpan = fxDDWriteDepthSpan;
    ctx->Driver.WriteDepthPixels = fxDDWriteDepthPixels;
    ctx->Driver.ReadDepthSpan = fxDDReadDepthSpan;
    ctx->Driver.ReadDepthPixels = fxDDReadDepthPixels;

    ctx->Driver.WriteCI8Span = NULL;
    ctx->Driver.WriteCI32Span = NULL;
    ctx->Driver.WriteMonoCISpan = NULL;
    ctx->Driver.WriteCI32Pixels = NULL;
    ctx->Driver.WriteMonoCIPixels = NULL;
    ctx->Driver.ReadCI32Span = NULL;
    ctx->Driver.ReadCI32Pixels = NULL;
}
