/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/fxddtex.c,v 1.3 2000/12/08 21:34:20 alanh Exp $ */
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


#include "fxdrv.h"
#include "fxddtex.h"
#include "fxtexman.h"
#include "fxsetup.h"
#include "image.h"
#include "texutil.h"


void
fxPrintTextureData(tfxTexInfo * ti)
{
    fprintf(stderr, "Texture Data:\n");
    if (ti->tObj) {
        fprintf(stderr, "\tName: %d\n", ti->tObj->Name);
        fprintf(stderr, "\tBaseLevel: %d\n", ti->tObj->BaseLevel);
        fprintf(stderr, "\tSize: %d x %d\n",
                ti->tObj->Image[ti->tObj->BaseLevel]->Width,
                ti->tObj->Image[ti->tObj->BaseLevel]->Height);
    }
    else
        fprintf(stderr, "\tName: UNNAMED\n");
    fprintf(stderr, "\tLast used: %d\n", ti->lastTimeUsed);
    fprintf(stderr, "\tTMU: %ld\n", (unsigned long)ti->whichTMU);
    fprintf(stderr, "\t%s\n", (ti->isInTM) ? "In TMU" : "Not in TMU");
    if (ti->tm[0])
        fprintf(stderr, "\tMem0: %x-%x\n", (unsigned) ti->tm[0]->startAddr,
                (unsigned) ti->tm[0]->endAddr);
    if (ti->tm[1])
        fprintf(stderr, "\tMem1: %x-%x\n", (unsigned) ti->tm[1]->startAddr,
                (unsigned) ti->tm[1]->endAddr);
    fprintf(stderr, "\tMipmaps: %d-%d\n", ti->minLevel, ti->maxLevel);
    fprintf(stderr, "\tFilters: min %d min %d\n",
            (int) ti->minFilt, (int) ti->maxFilt);
    fprintf(stderr, "\tClamps: s %d t %d\n", (int) ti->sClamp,
            (int) ti->tClamp);
    fprintf(stderr, "\tScales: s %f t %f\n", ti->sScale, ti->tScale);
    fprintf(stderr, "\tInt Scales: s %d t %d\n",
            ti->int_sScale / 0x800000, ti->int_tScale / 0x800000);
    fprintf(stderr, "\t%s\n",
            (ti->fixedPalette) ? "Fixed palette" : "Non fixed palette");
    fprintf(stderr, "\t%s\n",
            (ti->validated) ? "Validated" : "Not validated");
}


/************************************************************************/
/*************************** Texture Mapping ****************************/
/************************************************************************/

static void
fxTexInvalidate(GLcontext * ctx, struct gl_texture_object *tObj)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    tfxTexInfo *ti;

    ti = fxTMGetTexInfo(tObj);
    if (ti->isInTM)
        fxTMMoveOutTM(fxMesa, tObj); /* TO DO: SLOW but easy to write */

    ti->validated = GL_FALSE;
    fxMesa->new_state |= FX_NEW_TEXTURING;
    ctx->Driver.RenderStart = fxSetupFXUnits;
}

static tfxTexInfo *
fxAllocTexObjData(fxMesaContext fxMesa)
{
    tfxTexInfo *ti;
    int i;

    if (!(ti = CALLOC(sizeof(tfxTexInfo)))) {
        gl_problem(NULL, "fx Driver: out of memory !\n");
        return NULL;
    }

    ti->validated = GL_FALSE;
    ti->isInTM = GL_FALSE;

    ti->whichTMU = FX_TMU_NONE;

    ti->tm[FX_TMU0] = NULL;
    ti->tm[FX_TMU1] = NULL;

    ti->minFilt = GR_TEXTUREFILTER_POINT_SAMPLED;
    ti->maxFilt = GR_TEXTUREFILTER_BILINEAR;

    ti->sClamp = GR_TEXTURECLAMP_WRAP;
    ti->tClamp = GR_TEXTURECLAMP_WRAP;

    ti->mmMode = GR_MIPMAP_NEAREST;
    ti->LODblend = FXFALSE;

    for (i = 0; i < MAX_TEXTURE_LEVELS; i++) {
        ti->mipmapLevel[i].data = NULL;
    }

    return ti;
}


/*
 * Called via glBindTexture.
 */
void
fxDDTexBind(GLcontext * ctx, GLenum target, struct gl_texture_object *tObj)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    tfxTexInfo *ti;

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDTexBind(%d,%x)\n", tObj->Name,
                tObj->DriverData);
    }

    if (target != GL_TEXTURE_2D)
        return;

    if (!tObj->DriverData) {
        tObj->DriverData = fxAllocTexObjData(fxMesa);
    }

    ti = fxTMGetTexInfo(tObj);

    fxMesa->texBindNumber++;
    ti->lastTimeUsed = fxMesa->texBindNumber;

    fxMesa->new_state |= FX_NEW_TEXTURING;
    ctx->Driver.RenderStart = fxSetupFXUnits;
}

void
fxDDTexEnv(GLcontext * ctx, GLenum target, GLenum pname,
           const GLfloat * param)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        if (param)
            fprintf(stderr, "fxmesa: texenv(%x,%x)\n", pname,
                    (GLint) (*param));
        else
            fprintf(stderr, "fxmesa: texenv(%x)\n", pname);
    }

    /* apply any lod biasing right now */
    if (pname == GL_TEXTURE_LOD_BIAS_EXT) {
        FX_grTexLodBiasValue(GR_TMU0, *param);
        if (fxMesa->haveTwoTMUs) {
            FX_grTexLodBiasValue(GR_TMU1, *param);
        }
    }

    /* invalidate currently bound texture(s) */
    {
       int i;
       for (i = 0; i < ctx->Const.MaxTextureUnits; i++) {
          struct gl_texture_object *tObj = ctx->Texture.Unit[i].CurrentD[2];
          if (!tObj->DriverData) {
             tObj->DriverData = fxAllocTexObjData(fxMesa);
          }
          fxTexInvalidate(ctx, tObj);
       }
    }

    fxMesa->new_state |= FX_NEW_TEXTURING;
    ctx->Driver.RenderStart = fxSetupFXUnits;
}

void
fxDDTexParam(GLcontext * ctx, GLenum target, struct gl_texture_object *tObj,
             GLenum pname, const GLfloat * params)
{
    fxMesaContext fxMesa = (fxMesaContext) ctx->DriverCtx;
    GLenum param = (GLenum) (GLint) params[0];
    tfxTexInfo *ti;

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDTexParam(%d,%x,%x,%x)\n", tObj->Name,
                tObj->DriverData, pname, param);
    }

    if (target != GL_TEXTURE_2D)
        return;

    if (!tObj->DriverData)
        tObj->DriverData = fxAllocTexObjData(fxMesa);

    ti = fxTMGetTexInfo(tObj);

    switch (pname) {

    case GL_TEXTURE_MIN_FILTER:
        switch (param) {
        case GL_NEAREST:
            ti->mmMode = GR_MIPMAP_DISABLE;
            ti->minFilt = GR_TEXTUREFILTER_POINT_SAMPLED;
            ti->LODblend = FXFALSE;
            break;
        case GL_LINEAR:
            ti->mmMode = GR_MIPMAP_DISABLE;
            ti->minFilt = GR_TEXTUREFILTER_BILINEAR;
            ti->LODblend = FXFALSE;
            break;
        case GL_NEAREST_MIPMAP_NEAREST:
            ti->mmMode = GR_MIPMAP_NEAREST;
            ti->minFilt = GR_TEXTUREFILTER_POINT_SAMPLED;
            ti->LODblend = FXFALSE;
            break;
        case GL_LINEAR_MIPMAP_NEAREST:
            ti->mmMode = GR_MIPMAP_NEAREST;
            ti->minFilt = GR_TEXTUREFILTER_BILINEAR;
            ti->LODblend = FXFALSE;
            break;
        case GL_NEAREST_MIPMAP_LINEAR:
            if (fxMesa->haveTwoTMUs) {
                ti->mmMode = GR_MIPMAP_NEAREST;
                ti->LODblend = FXTRUE;
            }
            else {
                ti->mmMode = GR_MIPMAP_NEAREST_DITHER;
                ti->LODblend = FXFALSE;
            }
            ti->minFilt = GR_TEXTUREFILTER_POINT_SAMPLED;
            break;
        case GL_LINEAR_MIPMAP_LINEAR:
            if (fxMesa->haveTwoTMUs) {
                ti->mmMode = GR_MIPMAP_NEAREST;
                ti->LODblend = FXTRUE;
            }
            else {
                ti->mmMode = GR_MIPMAP_NEAREST_DITHER;
                ti->LODblend = FXFALSE;
            }
            ti->minFilt = GR_TEXTUREFILTER_BILINEAR;
            break;
        default:
            break;
        }
        fxTexInvalidate(ctx, tObj);
        break;

    case GL_TEXTURE_MAG_FILTER:
        switch (param) {
        case GL_NEAREST:
            ti->maxFilt = GR_TEXTUREFILTER_POINT_SAMPLED;
            break;
        case GL_LINEAR:
            ti->maxFilt = GR_TEXTUREFILTER_BILINEAR;
            break;
        default:
            break;
        }
        fxTexInvalidate(ctx, tObj);
        break;

    case GL_TEXTURE_WRAP_S:
        switch (param) {
        case GL_CLAMP:
            ti->sClamp = GR_TEXTURECLAMP_CLAMP;
            break;
        case GL_REPEAT:
            ti->sClamp = GR_TEXTURECLAMP_WRAP;
            break;
        default:
            break;
        }
        fxMesa->new_state |= FX_NEW_TEXTURING;
        ctx->Driver.RenderStart = fxSetupFXUnits;
        break;

    case GL_TEXTURE_WRAP_T:
        switch (param) {
        case GL_CLAMP:
            ti->tClamp = GR_TEXTURECLAMP_CLAMP;
            break;
        case GL_REPEAT:
            ti->tClamp = GR_TEXTURECLAMP_WRAP;
            break;
        default:
            break;
        }
        fxMesa->new_state |= FX_NEW_TEXTURING;
        ctx->Driver.RenderStart = fxSetupFXUnits;
        break;

    case GL_TEXTURE_BORDER_COLOR:
        /* TO DO */
        break;

    case GL_TEXTURE_MIN_LOD:
        /* TO DO */
        break;
    case GL_TEXTURE_MAX_LOD:
        /* TO DO */
        break;
    case GL_TEXTURE_BASE_LEVEL:
        fxTexInvalidate(ctx, tObj);
        break;
    case GL_TEXTURE_MAX_LEVEL:
        fxTexInvalidate(ctx, tObj);
        break;

    default:
        break;
    }
}


/*
 * Called via glDeleteTextures to delete a texture object.
 * Here, we delete the Glide data associated with the texture.
 */
void
fxDDTexDel(GLcontext * ctx, struct gl_texture_object *tObj)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    fxTMFreeTexture(fxMesa, tObj);
    ctx->NewState |= NEW_TEXTURING;
}


/*
 * Return true if texture is resident, false otherwise.
 */
GLboolean
fxDDIsTextureResident(GLcontext *ctx, struct gl_texture_object *tObj)
{
    tfxTexInfo *ti = fxTMGetTexInfo(tObj);
    /*printf("resident %d\n", (int) (ti && ti->isInTM));*/
    return (GLboolean) (ti && ti->isInTM);
}



/*
 * Convert a gl_color_table texture palette to Glide's format.
 */
static void
convertPalette(FxU32 data[256], const struct gl_color_table *table)
{
    const GLubyte *tableUB = (const GLubyte *) table->Table;
    GLint width = table->Size;
    FxU32 r, g, b, a;
    GLint i;

    ASSERT(table->TableType == GL_UNSIGNED_BYTE);

    switch (table->Format) {
    case GL_INTENSITY:
        for (i = 0; i < width; i++) {
            r = tableUB[i];
            g = tableUB[i];
            b = tableUB[i];
            a = tableUB[i];
            data[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
        break;
    case GL_LUMINANCE:
        for (i = 0; i < width; i++) {
            r = tableUB[i];
            g = tableUB[i];
            b = tableUB[i];
            a = 255;
            data[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
        break;
    case GL_ALPHA:
        for (i = 0; i < width; i++) {
            r = g = b = 255;
            a = tableUB[i];
            data[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
        break;
    case GL_LUMINANCE_ALPHA:
        for (i = 0; i < width; i++) {
            r = g = b = tableUB[i * 2 + 0];
            a = tableUB[i * 2 + 1];
            data[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
        break;
    case GL_RGB:
        for (i = 0; i < width; i++) {
            r = tableUB[i * 3 + 0];
            g = tableUB[i * 3 + 1];
            b = tableUB[i * 3 + 2];
            a = 255;
            data[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
        break;
    case GL_RGBA:
        for (i = 0; i < width; i++) {
            r = tableUB[i * 4 + 0];
            g = tableUB[i * 4 + 1];
            b = tableUB[i * 4 + 2];
            a = tableUB[i * 4 + 3];
            data[i] = (a << 24) | (r << 16) | (g << 8) | b;
        }
        break;
    }
}



void
fxDDTexPalette(GLcontext * ctx, struct gl_texture_object *tObj)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);

    if (tObj) {
        /* per-texture palette */
        tfxTexInfo *ti;
        if (MESA_VERBOSE & VERBOSE_DRIVER) {
            fprintf(stderr, "fxmesa: fxDDTexPalette(%d,%x)\n",
                    tObj->Name, tObj->DriverData);
        }
        if (!tObj->DriverData)
            tObj->DriverData = fxAllocTexObjData(fxMesa);
        ti = fxTMGetTexInfo(tObj);
        convertPalette(ti->palette.data, &tObj->Palette);
        fxTexInvalidate(ctx, tObj);
    }
    else {
        /* global texture palette */
        if (MESA_VERBOSE & VERBOSE_DRIVER) {
            fprintf(stderr, "fxmesa: fxDDTexPalette(global)\n");
        }
        convertPalette(fxMesa->glbPalette.data, &ctx->Texture.Palette);
        fxMesa->new_state |= FX_NEW_TEXTURING;
        ctx->Driver.RenderStart = fxSetupFXUnits;
    }
}


/*
 * Enable/disable the shared texture palette feature.
 */
void
fxDDTexUseGlbPalette(GLcontext * ctx, GLboolean state)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);

    if (MESA_VERBOSE & VERBOSE_DRIVER) {
        fprintf(stderr, "fxmesa: fxDDTexUseGlbPalette(%d)\n", state);
    }

    if (state) {
        FX_grTexDownloadTable(fxMesa, GR_TMU0, GR_TEXTABLE_PALETTE_6666_EXT,
                              &(fxMesa->glbPalette));
        if (fxMesa->haveTwoTMUs)
            FX_grTexDownloadTable(fxMesa, GR_TMU1, GR_TEXTABLE_PALETTE_6666_EXT,
                                  &(fxMesa->glbPalette));
    }
    else {
        if ((ctx->Texture.Unit[0].Current == ctx->Texture.Unit[0].CurrentD[2])
            && (ctx->Texture.Unit[0].Current != NULL)) {
            struct gl_texture_object *tObj = ctx->Texture.Unit[0].Current;

            if (!tObj->DriverData)
                tObj->DriverData = fxAllocTexObjData(fxMesa);

            fxTexInvalidate(ctx, tObj);
        }
    }
}


static int
logbase2(int n)
{
    GLint i = 1;
    GLint log2 = 0;

    if (n < 0) {
        return -1;
    }

    while (n > i) {
        i *= 2;
        log2++;
    }
    if (i != n) {
        return -1;
    }
    else {
        return log2;
    }
}

/* Need different versions for different cpus.
 */
#define INT_TRICK(pow2) (0x800000 * (pow2))

/*
 * Compute various texture image parameters.
 * Input:  w, h - source texture width and height
 * Output:  lodlevel - Glide lod level token
 *          aspectratio - Glide aspect ratio token
 *          sscale - S scale factor used during triangle setup
 *          tscale - T scale factor used during triangle setup
 *          i_sscale - integer S scale used during triangle setup
 *          i_tscale - integer T scale used during triangle setup
 *          wscale - OpenGL -> Glide image width scale factor
 *          hscale - OpenGL -> Glide image height scale factor
 */
void
fxTexGetInfo(const GLcontext *ctx, int w, int h,
             GrLOD_t *lodlevel, GrAspectRatio_t *aspectratio,
             float *sscale, float *tscale,
             int *i_sscale, int *i_tscale,
             int *wscale, int *hscale)
{
    int logw, logh, ar, lod, is, it, ws, hs;
    float s, t;

    ASSERT(w >= 1);
    ASSERT(h >= 1);

    logw = logbase2(w);
    logh = logbase2(h);
    ar = logw - logh;  /* aspect ratio = difference in log dimensions */

    /* Hardware only allows a maximum aspect ratio of 8x1, so handle
       |ar| > 3 by scaling the image and using an 8x1 aspect ratio */
    if (ar >= 0) {
        ASSERT(width >= height);
        lod = logw;
#if 1
        s = 256.0;
        is = INT_TRICK(8);
#else
        s = ctx->Const.MaxTextureSize;
        is = INT_TRICK(ctx->Const.MaxTextureLevels - 1);
#endif
        ws = 1;
        if (ar < 3) {
#if 1
            t = 256 >> ar;
            it = INT_TRICK(8 - ar);
#else
            t = ctx->Const.MaxTextureSize >> ar;
            it = INT_TRICK(ctx->Const.MaxTextureLevels - 1 - ar);
#endif
            hs = 1;
        }
        else {
            t = 32.0;
            it = INT_TRICK(5);
            hs = 1 << (ar - 3);
        }
    }
    else {
        ASSERT(width < height);
        lod = logh;
#if 1
        t = 256.0;
        it = INT_TRICK(8);
#else
        t = ctx->Const.MaxTextureSize;
        it = INT_TRICK(ctx->Const.MaxTextureLevels - 1);
#endif
        hs = 1;
        if (-ar < 3) {
#if 1
            s = 256 >> -ar;
            is = INT_TRICK(8 + ar);
#else
            s = ctx->Const.MaxTextureSize >> - ar;
            is = INT_TRICK(ctx->Const.MaxTextureLevels - 1 + ar);
#endif
            ws = 1;
        }
        else {
            s = 32.0;
            is = INT_TRICK(5);
            ws = 1 << (-ar - 3);
        }
    }
    if (ar < -3)
        ar = -3;
    if (ar > 3)
        ar = 3;

    if (lodlevel)
        *lodlevel = (GrLOD_t) lod;
    if (aspectratio)
        *aspectratio = (GrAspectRatio_t) ar;
    if (sscale)
        *sscale = s;
    if (tscale)
        *tscale = t;
    if (wscale)
        *wscale = ws;
    if (hscale)
        *hscale = hs;
    if (i_sscale)
        *i_sscale = is;
    if (i_tscale)
        *i_tscale = it;
}

/*
 * Given an OpenGL internal texture format, return the corresponding
 * Glide internal texture format and base texture format.
 * If allow32bpp is true, we'll return 32-bit texel formats when
 * appropriate.
 */
void
fxTexGetFormat(GLenum glformat, GrTextureFormat_t *glideFormat,
               GLint *glFormat, MesaIntTexFormat *mesaFormat,
               GLint *texelSize, GLboolean allow32bpp)
{
    switch (glformat) {
    case 1:
    case GL_LUMINANCE:
    case GL_LUMINANCE4:
    case GL_LUMINANCE8:
    case GL_LUMINANCE12:
    case GL_LUMINANCE16:
        if (glideFormat)
            *glideFormat = GR_TEXFMT_INTENSITY_8;
        if (glFormat)
            *glFormat = GL_LUMINANCE;
        if (mesaFormat)
            *mesaFormat = MESA_L8;
        if (texelSize)
            *texelSize = 1;
        break;
    case 2:
    case GL_LUMINANCE_ALPHA:
    case GL_LUMINANCE4_ALPHA4:
    case GL_LUMINANCE6_ALPHA2:
    case GL_LUMINANCE8_ALPHA8:
    case GL_LUMINANCE12_ALPHA4:
    case GL_LUMINANCE12_ALPHA12:
    case GL_LUMINANCE16_ALPHA16:
        if (glideFormat)
            *glideFormat = GR_TEXFMT_ALPHA_INTENSITY_88;
        if (glFormat)
            *glFormat = GL_LUMINANCE_ALPHA;
        if (mesaFormat)
            *mesaFormat = MESA_A8_L8;
        if (texelSize)
            *texelSize = 2;
        break;
    case GL_INTENSITY:
    case GL_INTENSITY4:
    case GL_INTENSITY8:
    case GL_INTENSITY12:
    case GL_INTENSITY16:
        if (glideFormat)
            *glideFormat = GR_TEXFMT_ALPHA_8;
        if (glFormat)
            *glFormat = GL_INTENSITY;
        if (mesaFormat)
            *mesaFormat = MESA_I8;
        if (texelSize)
            *texelSize = 1;
       break;
    case GL_ALPHA:
    case GL_ALPHA4:
    case GL_ALPHA8:
    case GL_ALPHA12:
    case GL_ALPHA16:
        if (glideFormat)
            *glideFormat = GR_TEXFMT_ALPHA_8;
        if (glFormat)
            *glFormat = GL_ALPHA;
        if (mesaFormat)
            *mesaFormat = MESA_A8;
        if (texelSize)
            *texelSize = 1;
        break;
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
        if (glideFormat)
            *glideFormat = GR_TEXFMT_RGB_565;
        if (glFormat)
            *glFormat = GL_RGB;
        if (mesaFormat)
            *mesaFormat = MESA_R5_G6_B5;
        if (texelSize)
            *texelSize = 2;
        break;
    case 3:
    case GL_RGB:
    case GL_RGB8:
    case GL_RGB10:
    case GL_RGB12:
    case GL_RGB16:
        if (allow32bpp) {
            if (glideFormat)
                *glideFormat = GR_TEXFMT_ARGB_8888;
            if (glFormat)
                *glFormat = GL_RGB;
            if (mesaFormat)
                *mesaFormat = MESA_FF_R8_G8_B8;
            if (texelSize)
                *texelSize = 4;
        }
        else {
            if (glideFormat)
                *glideFormat = GR_TEXFMT_RGB_565;
            if (glFormat)
                *glFormat = GL_RGB;
            if (mesaFormat)
                *mesaFormat = MESA_R5_G6_B5;
            if (texelSize)
                *texelSize = 2;
        }
        break;
    case GL_RGBA2:
    case GL_RGBA4:
        if (glideFormat)
            *glideFormat = GR_TEXFMT_ARGB_4444;
        if (glFormat)
            *glFormat = GL_RGBA;
        if (mesaFormat)
            *mesaFormat = MESA_A4_R4_G4_B4;
        if (texelSize)
            *texelSize = 2;
        break;
    case 4:
    case GL_RGBA:
    case GL_RGBA8:
    case GL_RGB10_A2:
    case GL_RGBA12:
    case GL_RGBA16:
        if (allow32bpp) {
            if (glideFormat)
                *glideFormat = GR_TEXFMT_ARGB_8888;
            if (glFormat)
                *glFormat = GL_RGBA;
            if (mesaFormat)
                *mesaFormat = MESA_A8_R8_G8_B8;
            if (texelSize)
                *texelSize = 4;
        }
        else {
            if (glideFormat)
                *glideFormat = GR_TEXFMT_ARGB_4444;
            if (glFormat)
                *glFormat = GL_RGBA;
            if (mesaFormat)
                *mesaFormat = MESA_A4_R4_G4_B4;
            if (texelSize)
                *texelSize = 2;
        }
        break;
    case GL_RGB5_A1:
        if (glideFormat)
            *glideFormat = GR_TEXFMT_ARGB_1555;
        if (glFormat)
            *glFormat = GL_RGBA;
        if (mesaFormat)
            *mesaFormat = MESA_A1_R5_G5_B5;
        if (texelSize)
           *texelSize = 2;
        break;
    case GL_COLOR_INDEX:
    case GL_COLOR_INDEX1_EXT:
    case GL_COLOR_INDEX2_EXT:
    case GL_COLOR_INDEX4_EXT:
    case GL_COLOR_INDEX8_EXT:
    case GL_COLOR_INDEX12_EXT:
    case GL_COLOR_INDEX16_EXT:
        if (glideFormat)
            *glideFormat = GR_TEXFMT_P_8;
        if (glFormat)
            *glFormat = GL_RGBA;  /* XXX why is this RGBA? */
        if (mesaFormat)
            *mesaFormat = MESA_C8;
        if (texelSize)
            *texelSize = 1;
        break;
    case GL_COMPRESSED_RGB_FXT1_3DFX:
        if (glideFormat)
            *glideFormat = GR_TEXFMT_ARGB_CMP_FXT1;
        if (glFormat)
            *glFormat = GL_COMPRESSED_RGB_FXT1_3DFX;
        if (mesaFormat)
            *mesaFormat = MESA_A8_R8_G8_B8;
        if (texelSize)
            *texelSize = 4;
        break;
    case GL_COMPRESSED_RGBA_FXT1_3DFX:
        if (glideFormat)
            *glideFormat = GR_TEXFMT_ARGB_CMP_FXT1;
        if (glFormat)
            *glFormat = GL_COMPRESSED_RGBA_FXT1_3DFX;
        if (mesaFormat)
            *mesaFormat = MESA_A8_R8_G8_B8;
        if (texelSize)
            *texelSize = 4;
        break;
    default:
        gl_problem(NULL, "bad texture format in fxTexGetFormat()\n");
        break;
    }
}


/**********************************************************************/
/**** NEW TEXTURE IMAGE FUNCTIONS                                  ****/
/**********************************************************************/

static FxBool TexusFatalError = FXFALSE;
static FxBool TexusError = FXFALSE;

#define TX_DITHER_NONE                                  0x00000000

static void
fxTexusError(const char *string, FxBool fatal)
{
    gl_problem(NULL, string);
   /*
    * Just propagate the fatal value up.
    */
    TexusError = FXTRUE;
    TexusFatalError = fatal;
}

GLboolean
fxDDTexImage2D(GLcontext * ctx, GLenum target, GLint level,
               GLenum format, GLenum type, const GLvoid * pixels,
               const struct gl_pixelstore_attrib * packing,
               struct gl_texture_object * texObj,
               struct gl_texture_image * texImage,
               GLboolean * retainInternalCopy)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    const GLboolean allow32bpt = fxMesa->haveHwStencil;
    GrTextureFormat_t gldformat;
    tfxTexInfo *ti;
    tfxMipMapLevel *mml;
    GLint dstWidth, dstHeight, wScale, hScale, texelSize, dstStride;
    MesaIntTexFormat intFormat;
    GLboolean isCompressedFormat;
    GLint texsize;
    void *uncompressedImage;

    isCompressedFormat = texImage->IsCompressed;
    if (target != GL_TEXTURE_2D || texImage->Border > 0)
        return GL_FALSE;

    if (!texObj->DriverData)
        texObj->DriverData = fxAllocTexObjData(fxMesa);

    ti = fxTMGetTexInfo(texObj);
    mml = &ti->mipmapLevel[level];

    /* Determine the appropriate GL internal texel format, Mesa internal
     * texel format, and texelSize (bytes) given the user's internal
     * texture format hint.
     */
    fxTexGetFormat(texImage->IntFormat, &gldformat, NULL, &intFormat,
                   &texelSize, allow32bpt);

    /* Determine width and height scale factors for texture.
     * Remember, Glide is limited to 8:1 aspect ratios.
     */
    fxTexGetInfo(ctx,
                 texImage->Width, texImage->Height,
                 NULL,       /* lod level          */
                 NULL,       /* aspect ratio       */
                 NULL, NULL, /* sscale, tscale     */
                 NULL, NULL, /* i_sscale, i_tscale */
                 &wScale, &hScale);
    dstWidth = texImage->Width * wScale;
    dstHeight = texImage->Height * hScale;
    if (isCompressedFormat) {
        texsize = fxDDCompressedImageSize(ctx,
                                          texImage->IntFormat,
                                          2,
                                          texImage->Width,
                                          texImage->Height,
                                          1);
    } else {
        texsize = dstWidth * dstHeight * texelSize;
    }
   /*
    * If the image is not compressed, this doesn't
    * matter, but it might as well have a sensible
    * value, and it might save a failure later on.
    */
    texImage->CompressedSize = texsize;
    /* housekeeping */
    _mesa_set_teximage_component_sizes(intFormat, texImage);

    /*
     * allocate new storage for texture image, if needed.
     * This conditional wants to set uncompressedImage to
     * point to the uncompressed image, and mml->data to
     * the texture data.  If the image is uncompressed,
     * these are identical.  If the image is not compressed,
     * these are different.
     */
    if (!mml->data || mml->glideFormat != gldformat ||
        mml->width != dstWidth || mml->height != dstHeight ||
        texsize != mml->dataSize ) {
        if (mml->data) {
            FREE(mml->data);
        }
        uncompressedImage
            = (void *)MALLOC(dstWidth * dstHeight * texelSize);
        if (!uncompressedImage) {
            return(GL_FALSE);
        }
        if (isCompressedFormat) {
            mml->data = MALLOC(texsize);
            if (!mml->data) {
                FREE(uncompressedImage);
                return GL_FALSE;
            }
        } else {
            mml->data = uncompressedImage;
        }
        mml->texelSize = texelSize;
        mml->glideFormat = gldformat;
        mml->width = dstWidth;
        mml->height = dstHeight;
        mml->dataSize = texsize;
        fxTexInvalidate(ctx, texObj);
    } else {
       /*
        * Here we don't have to allocate anything, but we
        * do have to point uncompressedImage to the uncompressed
        * data.
        */
        if (isCompressedFormat) {
            uncompressedImage
                = (void *)MALLOC(dstWidth * dstHeight * texelSize);
            if (!uncompressedImage) {
                return GL_FALSE;
            }
        } else {
            uncompressedImage = mml->data;
        }
    }

    dstStride = dstWidth * texelSize;

    /* store the texture image into uncompressedImage */
    if (!_mesa_convert_teximage(intFormat,
                                dstWidth, dstHeight,
                                uncompressedImage,
                                dstStride,
                                texImage->Width, texImage->Height,
                                format, type, pixels, packing)) {
        /*printf("convert failed\n");*/
        return GL_FALSE;  /* not necessarily an error */
    }
   /*
    * Now compress it if necessary.
    */
    if (isCompressedFormat) {
        TxErrorCallbackFnc_t oldErrorCallback;
        (*txErrorSetCallbackPtr)(fxTexusError, &oldErrorCallback);
        (*txImgQuantizePtr)((char *)mml->data,
                            (char *)uncompressedImage,
                            texImage->Width,
                            texImage->Height,
                            gldformat,
                            TX_DITHER_NONE);
        (*txErrorSetCallbackPtr)(oldErrorCallback, NULL);
        if (uncompressedImage != mml->data) {
            /*
             * We do not need this any more, errors or no.
             */
            FREE(uncompressedImage);
        }
        TexusError = FXFALSE;
        if (TexusFatalError) {
            FREE(mml->data);
            mml->data = (unsigned short *)0;
            TexusFatalError = FXFALSE;
            return(GL_FALSE);
        }
    }
    if (ti->validated && ti->isInTM) {
        fxTMReloadMipMapLevel(ctx, texObj, level);
    }
    else {
        fxTexInvalidate(ctx, texObj);
    }

    *retainInternalCopy = GL_FALSE;
    return GL_TRUE;
}


GLboolean
fxDDTexSubImage2D(GLcontext * ctx, GLenum target, GLint level,
                  GLint xoffset, GLint yoffset,
                  GLsizei width, GLsizei height,
                  GLenum format, GLenum type, const GLvoid * pixels,
                  const struct gl_pixelstore_attrib * packing,
                  struct gl_texture_object * texObj,
                  struct gl_texture_image * texImage)
{
    tfxTexInfo *ti;
    GLint wscale, hscale, dstStride = 0;
    tfxMipMapLevel *mml;
    GLboolean result;
    void *uncompressedImage = (void *)0;
    FxU32 uncompressedSize;
    TxErrorCallbackFnc_t oldErrorCallback;

    if (target != GL_TEXTURE_2D)
        return GL_FALSE;

    if (!texObj->DriverData)
        return GL_FALSE;

    ti = fxTMGetTexInfo(texObj);
    mml = &ti->mipmapLevel[level];

    fxTexGetInfo(ctx, texImage->Width, texImage->Height, NULL, NULL,
                 NULL, NULL, NULL, NULL, &wscale, &hscale);

    /*
     * Must have an existing texture image!
     */
    assert(mml->data);

    switch (mml->glideFormat) {
    case GR_TEXFMT_INTENSITY_8:
        dstStride = mml->width;
        result = _mesa_convert_texsubimage(MESA_I8, xoffset, yoffset,
                                           mml->width, mml->height, mml->data,
                                           dstStride, width, height,
                                           texImage->Width, texImage->Height,
                                           format, type, pixels, packing);
        break;
    case GR_TEXFMT_ALPHA_8:
        dstStride = mml->width;
        result = _mesa_convert_texsubimage(MESA_A8, xoffset, yoffset,
                                           mml->width, mml->height, mml->data,
                                           dstStride, width, height,
                                           texImage->Width, texImage->Height,
                                           format, type, pixels, packing);
        break;
    case GR_TEXFMT_P_8:
        dstStride = mml->width;
        result = _mesa_convert_texsubimage(MESA_C8, xoffset, yoffset,
                                           mml->width, mml->height, mml->data,
                                           dstStride, width, height,
                                           texImage->Width, texImage->Height,
                                           format, type, pixels, packing);
        break;
    case GR_TEXFMT_ALPHA_INTENSITY_88:
        dstStride = mml->width * 2;
        result = _mesa_convert_texsubimage(MESA_A8_L8, xoffset, yoffset,
                                           mml->width, mml->height, mml->data,
                                           dstStride, width, height,
                                           texImage->Width, texImage->Height,
                                           format, type, pixels, packing);
        break;
    case GR_TEXFMT_RGB_565:
        dstStride = mml->width * 2;
        result = _mesa_convert_texsubimage(MESA_R5_G6_B5, xoffset, yoffset,
                                           mml->width, mml->height, mml->data,
                                           dstStride, width, height,
                                           texImage->Width, texImage->Height,
                                           format, type, pixels, packing);
        break;
    case GR_TEXFMT_ARGB_4444:
        dstStride = mml->width * 2;
        result = _mesa_convert_texsubimage(MESA_A4_R4_G4_B4, xoffset, yoffset,
                                           mml->width, mml->height, mml->data,
                                           dstStride, width, height,
                                           texImage->Width, texImage->Height,
                                           format, type, pixels, packing);
        break;
    case GR_TEXFMT_ARGB_CMP_FXT1:
        /*
         * There are some special legality constraints for compressed
         * textures.
         */
        if ((xoffset != texImage->Border)
            || (yoffset != texImage->Border)) {
            gl_error( ctx,
                      GL_INVALID_OPERATION,
                      "glTexSubImage2D(offset)" );
            return GL_FALSE;
        }
        if ((width != texImage->Width)
            || (height != texImage->Height)) {
            gl_error( ctx,
                      GL_INVALID_VALUE,
                      "glTexSubImage2D(image size)" );
            return GL_FALSE;
        }
        /*
         * The width and height have to be multiples of
         * 8 and 4 respectively.
         */
        width  = (mml->width  + 0x7) &~ 0x7;
        height = (mml->height + 0x3) &~ 0x3;
        /*
         * A texel is 8888 for this format. 
         */
        uncompressedSize = mml->width * mml->height * 4;
        uncompressedImage = (void *)MALLOC(uncompressedSize);
        /*
         * Convert the data.
         */
        dstStride = mml->width * 4;
        result = _mesa_convert_texsubimage(MESA_A8_R8_G8_B8, xoffset, yoffset,
                                           mml->width, mml->height, uncompressedImage,
                                           dstStride, width, height,
                                           texImage->Width, texImage->Height,
                                           format, type, pixels, packing);
        if (!result) {
            FREE(uncompressedImage);
            return GL_FALSE;
        }
        /*
         * Now that we have converted the data, then compress it.
         */
        (*txErrorSetCallbackPtr)(fxTexusError, &oldErrorCallback);
        (*txImgQuantizePtr)((char *)mml->data,
                            (char *)uncompressedImage,
                            mml->width,
                            mml->height,
                            mml->glideFormat,
                            TX_DITHER_NONE);
        (*txErrorSetCallbackPtr)(oldErrorCallback, NULL);
        result = TexusFatalError;
        TexusFatalError = TexusError = FXFALSE;
        /*
         * We don't need this any more.
         */
        FREE(uncompressedImage);
        break;
    case GR_TEXFMT_ARGB_8888:
        {
            MesaIntTexFormat intFormat;
            if (texImage->Format == GL_RGB) {
                /* An RGB image padded out to 4 bytes/texel */
                intFormat = MESA_FF_R8_G8_B8;
            }
            else {
                intFormat = MESA_A8_R8_G8_B8;
            }
            dstStride = mml->width * 4;
            result = _mesa_convert_texsubimage(intFormat, xoffset, yoffset,
                                            mml->width, mml->height, mml->data,
                                            dstStride, width, height,
                                            texImage->Width, texImage->Height,
                                            format, type, pixels, packing);
        }
        break;
    case GR_TEXFMT_ARGB_1555:
        dstStride = mml->width * 2;
        result = _mesa_convert_texsubimage(MESA_A1_R5_G5_B5, xoffset, yoffset,
                                           mml->width, mml->height, mml->data,
                                           dstStride, width, height,
                                           texImage->Width, texImage->Height,
                                           format, type, pixels, packing);
        break;
    default:
        gl_problem(NULL, "tdfx driver: fxTexBuildSubImageMap() bad format");
        result = GL_FALSE;
    }

    if (!result) {
        return GL_FALSE;
    }

    if (ti->validated && ti->isInTM)
        /* Don't use this, it's very broken.  Download whole image for now.*/
#if 0
        fxTMReloadSubMipMapLevel(ctx, texObj, level, yoffset, height);
#else
        fxTMReloadMipMapLevel(ctx, texObj, level);
#endif
    else
        fxTexInvalidate(ctx, texObj);

    return GL_TRUE;
}


/**********************************************************************/
/**** COMPRESSED TEXTURE IMAGE FUNCTIONS                           ****/
/**********************************************************************/

GLboolean
fxDDCompressedTexImage2D( GLcontext *ctx, GLenum target,
                          GLint level, GLsizei imageSize,
                          const GLvoid *data,
                          struct gl_texture_object *texObj,
                          struct gl_texture_image *texImage,
                          GLboolean *retainInternalCopy)
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    const GLboolean allow32bpt = fxMesa->haveHwStencil;
    GrTextureFormat_t gldformat;
    tfxTexInfo *ti;
    tfxMipMapLevel *mml;
    GLint dstWidth, dstHeight, wScale, hScale, texelSize;
    MesaIntTexFormat intFormat;
    GLboolean isCompressedFormat;
    GLsizei texsize;

    if (target != GL_TEXTURE_2D || texImage->Border > 0)
        return GL_FALSE;

    if (!texObj->DriverData)
        texObj->DriverData = fxAllocTexObjData(fxMesa);

    ti = fxTMGetTexInfo(texObj);
    mml = &ti->mipmapLevel[level];

    isCompressedFormat = fxDDIsCompressedGlideFormatMacro(texImage->IntFormat);
    if (!isCompressedFormat) {
        gl_error( ctx, GL_INVALID_ENUM, "glCompressedTexImage2D(format)" );
        return GL_FALSE;
    } 
    /* Determine the apporpriate GL internal texel format, Mesa internal
     * texel format, and texelSize (bytes) given the user's internal
     * texture format hint.
     */
    fxTexGetFormat(texImage->IntFormat, &gldformat, NULL, &intFormat,
                   &texelSize, allow32bpt);

    /* Determine width and height scale factors for texture.
     * Remember, Glide is limited to 8:1 aspect ratios.
     */
    fxTexGetInfo(ctx,
                 texImage->Width, texImage->Height,
                 NULL,       /* lod level          */
                 NULL,       /* aspect ratio       */
                 NULL, NULL, /* sscale, tscale     */
                 NULL, NULL, /* i_sscale, i_tscale */
                 &wScale, &hScale);
    dstWidth = texImage->Width * wScale;
    dstHeight = texImage->Height * hScale;
    /* housekeeping */
    _mesa_set_teximage_component_sizes(intFormat, texImage);

    texsize = fxDDCompressedImageSize(ctx,
                                      texImage->IntFormat,
                                      2,
                                      texImage->Width,
                                      texImage->Height,
                                      1);
    if (texsize != imageSize) {
        gl_error(ctx,
                 GL_INVALID_VALUE,
                 "glCompressedTexImage2D(texsize)");
        return GL_FALSE;
    }
    /* allocate new storage for texture image, if needed */
    if (!mml->data || mml->glideFormat != gldformat ||
        mml->width != dstWidth || mml->height != dstHeight ||
        texsize != mml->dataSize) {
        if (mml->data) {
            FREE(mml->data);
        }
        mml->data = MALLOC(texsize);
        if (!mml->data) {
            return GL_FALSE;
        }
        mml->texelSize = texelSize;
        mml->glideFormat = gldformat;
        mml->width = dstWidth;
        mml->height = dstHeight;
        fxTexInvalidate(ctx, texObj);
    }

    MEMCPY(mml->data, data, imageSize);
    if (ti->validated && ti->isInTM) {
        fxTMReloadMipMapLevel(ctx, texObj, level);
    }
    else {
        fxTexInvalidate(ctx, texObj);
    }

    *retainInternalCopy = GL_FALSE;
    return GL_TRUE;
}

GLboolean
fxDDCompressedTexSubImage2D( GLcontext *ctx, GLenum target,
                             GLint level, GLint xoffset,
                             GLint yoffset, GLsizei width,
                             GLint height, GLenum format,
                             GLsizei imageSize, const GLvoid *data,
                             struct gl_texture_object *texObj,
                             struct gl_texture_image *texImage )
{
    tfxTexInfo *ti;
    tfxMipMapLevel *mml;
   /*
    * We punt if we are not replacing the entire image.  This
    * is allowed by the spec.
    */
    if ((xoffset != 0) && (yoffset != 0)
        && (width != texImage->Width)
        && (height != texImage->Height)) {
        return(GL_FALSE);
    }
    ti = fxTMGetTexInfo(texObj);
    mml = &ti->mipmapLevel[level];
    if (imageSize != mml->dataSize) {
        return(GL_FALSE);
    }
    MEMCPY(data, mml->data, imageSize);
    return(GL_TRUE);
}

#if	0
static void
PrintTexture(int w, int h, int c, const GLubyte * data)
{
    int i, j;
    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            if (c == 2)
                printf("%02x %02x  ", data[0], data[1]);
            else if (c == 3)
                printf("%02x %02x %02x  ", data[0], data[1], data[2]);
            data += c;
        }
        printf("\n");
    }
}
#endif


GLboolean
fxDDTestProxyTexImage(GLcontext *ctx, GLenum target,
                      GLint level, GLint internalFormat,
                      GLenum format, GLenum type,
                      GLint width, GLint height,
                      GLint depth, GLint border )
{
    fxMesaContext fxMesa = FX_CONTEXT(ctx);
    struct gl_shared_state *mesaShared = fxMesa->glCtx->Shared;
    struct TdfxSharedState *shared = (struct TdfxSharedState *) mesaShared->DriverData;

    switch (target) {
    case GL_PROXY_TEXTURE_1D:
        return GL_TRUE;  /* software rendering */
    case GL_PROXY_TEXTURE_2D:
        {
            struct gl_texture_object *tObj;
            tfxTexInfo *ti;
            int memNeeded;

            tObj = ctx->Texture.Proxy2D;
            if (!tObj->DriverData)
                tObj->DriverData = fxAllocTexObjData(fxMesa);
            ti = fxTMGetTexInfo(tObj);

            /* assign the parameters to test against */
            tObj->Image[level]->Width = width;
            tObj->Image[level]->Height = height;
            tObj->Image[level]->Border = border;
            tObj->Image[level]->IntFormat = internalFormat;
            if (level == 0) {
               /* don't use mipmap levels > 0 */
               tObj->MinFilter = tObj->MagFilter = GL_NEAREST;
            }
            else {
               /* test with all mipmap levels */
               tObj->MinFilter = GL_LINEAR_MIPMAP_LINEAR;
               tObj->MagFilter = GL_NEAREST;
            }
            ti->validated = GL_FALSE;
            fxTexValidate(ctx, tObj);

            /*
            printf("small lodlog2 0x%x\n", ti->info.smallLodLog2);
            printf("large lodlog2 0x%x\n", ti->info.largeLodLog2);
            printf("aspect ratio 0x%x\n", ti->info.aspectRatioLog2);
            printf("glide format 0x%x\n", ti->info.format);
            printf("data %p\n", ti->info.data);
            printf("lodblend %d\n", (int) ti->LODblend);
            */

            /* determine where texture will reside */
            if (ti->LODblend && !shared->umaTexMemory) {
                /* XXX GR_MIPMAPLEVELMASK_BOTH might not be right, but works */
                memNeeded = FX_grTexTextureMemRequired_NoLock(
                                        GR_MIPMAPLEVELMASK_BOTH, &(ti->info));
            }
            else {
                /* XXX GR_MIPMAPLEVELMASK_BOTH might not be right, but works */
                memNeeded = FX_grTexTextureMemRequired_NoLock(
                                        GR_MIPMAPLEVELMASK_BOTH, &(ti->info));
            }
            /*
            printf("Proxy test %d > %d\n", memNeeded, shared->totalTexMem[0]);
            */
            if (memNeeded > shared->totalTexMem[0])
                return GL_FALSE;
            else
                return GL_TRUE;
        }
    case GL_PROXY_TEXTURE_3D:
        return GL_TRUE;  /* software rendering */
    default:
        return GL_TRUE;  /* never happens, silence compiler */
    }
}


/*
 * Return a texture image to Mesa.  This is either to satisfy
 * a glGetTexImage() call or to prepare for software texturing.
 */
GLvoid *
fxDDGetTexImage(GLcontext * ctx, GLenum target, GLint level,
                const struct gl_texture_object *texObj,
                GLenum * formatOut, GLenum * typeOut,
                GLboolean * freeImageOut)
{
    tfxTexInfo *ti;
    tfxMipMapLevel *mml;

    if (target != GL_TEXTURE_2D)
        return NULL;

    if (!texObj->DriverData)
        return NULL;

    ti = fxTMGetTexInfo(texObj);
    mml = &ti->mipmapLevel[level];
    if (mml->data) {
        MesaIntTexFormat mesaFormat;
        GLenum glFormat;
        struct gl_texture_image *texImage = texObj->Image[level];
        GLint srcStride;
        void *uncompressedImage = NULL;

        GLubyte *data =
            (GLubyte *) MALLOC(texImage->Width * texImage->Height * 4);
        if (!data)
            return NULL;

        uncompressedImage = (void *)mml->data;
        switch (mml->glideFormat) {
        case GR_TEXFMT_INTENSITY_8:
            mesaFormat = MESA_I8;
            glFormat = GL_INTENSITY;
            srcStride = mml->width;
            break;
        case GR_TEXFMT_ALPHA_INTENSITY_88:
            mesaFormat = MESA_A8_L8;
            glFormat = GL_LUMINANCE_ALPHA;
            srcStride = mml->width;
            break;
        case GR_TEXFMT_ALPHA_8:
            if (texImage->Format == GL_INTENSITY) {
                mesaFormat = MESA_I8;
                glFormat = GL_INTENSITY;
            }
            else {
                mesaFormat = MESA_A8;
                glFormat = GL_ALPHA;
            }
            srcStride = mml->width;
            break;
        case GR_TEXFMT_RGB_565:
            mesaFormat = MESA_R5_G6_B5;
            glFormat = GL_RGB;
            srcStride = mml->width * 2;
            break;
        case GR_TEXFMT_ARGB_8888:
            mesaFormat = MESA_A8_R8_G8_B8;
            glFormat = GL_RGBA;
            srcStride = mml->width * 4;
            break;
        case GR_TEXFMT_ARGB_4444:
            mesaFormat = MESA_A4_R4_G4_B4;
            glFormat = GL_RGBA;
            srcStride = mml->width * 2;
            break;
        case GR_TEXFMT_ARGB_1555:
            mesaFormat = MESA_A1_R5_G5_B5;
            glFormat = GL_RGBA;
            srcStride = mml->width * 2;
            break;
        case GR_TEXFMT_P_8:
            mesaFormat = MESA_C8;
            glFormat = GL_COLOR_INDEX;
            srcStride = mml->width;
            break;
        case GR_TEXFMT_ARGB_CMP_FXT1:
            mesaFormat = MESA_A8_R8_G8_B8;
            glFormat = GL_RGBA;
            srcStride = mml->width * 4;
           /*
            * Allocate data for the uncompressed image,
            * decompress the image.  The data will be deallocated
            * after it is converted to the mesa format.
            */
            uncompressedImage = MALLOC(mml->width * mml->height * 4);
            if (!uncompressedImage) {
                gl_problem(NULL, "can't get memory in fxDDGetTexImage");
                return(NULL);
            }
            (*txImgDequantizeFXT1Ptr)((FxU32 *)uncompressedImage,
                                      (FxU32 *)mml->data,
                                      mml->width,
                                      mml->height);
            break;
        default:
            gl_problem(NULL, "Bad glideFormat in fxDDGetTexImage");
            return NULL;
        }
        _mesa_unconvert_teximage(mesaFormat, mml->width, mml->height,
                                 uncompressedImage, srcStride, texImage->Width,
                                 texImage->Height, glFormat, data);
        if (uncompressedImage != mml->data) {
            FREE(uncompressedImage);
        }
        *formatOut = glFormat;
        *typeOut = GL_UNSIGNED_BYTE;
        *freeImageOut = GL_TRUE;
        return data;
    }
    else {
        return NULL;
    }
}

/*
 * This is called from _mesa_GetCompressedTexImage.  We just
 * copy out the compressed data.
 */
void
fxDDGetCompressedTexImage( GLcontext *ctx, GLenum target,
                           GLint lod, void *image,
                           const struct gl_texture_object *texObj,
                           struct gl_texture_image *texImage )
{
    tfxTexInfo *ti;
    tfxMipMapLevel *mml;

    if (target != GL_TEXTURE_2D)
        return;

    if (!texObj->DriverData)
        return;

    ti = fxTMGetTexInfo(texObj);
    mml = &ti->mipmapLevel[lod];
    if (mml->data) {
        MEMCPY(image, mml->data, mml->dataSize);
    }
}

/*
 * Calculate a specific texture format given a generic
 * texture format.
 */
GLint
fxDDSpecificCompressedTexFormat(GLcontext *ctx,
                                GLint      internalFormat,
                                GLint      numDimensions,
                                GLint     *levelp,
                                GLsizei   *widthp,
                                GLsizei   *heightp,
                                GLsizei   *depthp,
                                GLint     *borderp,
                                GLenum    *formatp,
                                GLenum    *typep)
{
    if (numDimensions != 2) {
        return internalFormat;
    }
   /*
    * If we don't have pointers to the functions, then
    * we drop back to uncompressed format.  The logic
    * in Mesa proper handles this for us.
    *
    * This is just to ease the transition to a Glide with
    * the texus2 library.
    */
    if (!txImgQuantizePtr || !txImgDequantizeFXT1Ptr) {
        return(internalFormat);
    }
    switch (internalFormat) {
   /*
    * GL_S3_s3tc uses negative level values.
    */
    case GL_RGB_S3TC:
    case GL_RGB4_S3TC:
        {
            GLint level;
            if (levelp) {
                level = *levelp;
                if (level < 0) {
                    level = -level;
                    *levelp = level;
                }
            }
        }
        return GL_COMPRESSED_RGB_FXT1_3DFX;
    case GL_RGBA_S3TC:
    case GL_RGBA4_S3TC:
        {
            GLint level;
            if (levelp) {
                level = *levelp;
                if (level < 0) {
                    level = -level;
                    *levelp = level;
                }
            }
        }
        return GL_COMPRESSED_RGBA_FXT1_3DFX;
    case GL_COMPRESSED_RGB_ARB:
        return GL_COMPRESSED_RGB_FXT1_3DFX;
    case GL_COMPRESSED_RGBA_ARB:
        return GL_COMPRESSED_RGBA_FXT1_3DFX;
    }
    return internalFormat;
}

/*
 * Calculate a specific texture format given a generic
 * texture format.
 */
GLint
fxDDBaseCompressedTexFormat(GLcontext *ctx,
                            GLint      internalFormat)
{
    switch (internalFormat) {
    case GL_COMPRESSED_RGB_FXT1_3DFX:
        return(GL_RGB);
    case GL_COMPRESSED_RGBA_FXT1_3DFX:
        return(GL_RGBA);
    }
    return -1;
}

/*
 * Tell us if an image is compressed.  The real work is done
 * in a macro, but we need to have a function to create a
 * function pointer.
 */
GLboolean
fxDDIsCompressedFormat(GLcontext *ctx, GLint internalFormat)
{
    return(fxDDIsCompressedFormatMacro(internalFormat));
}


/*
 * Calculate the image size of a compressed texture.
 *
 * The current compressed format, the FXT1 family, all
 * map 8x32 texel blocks into 128 bits.
 *
 * We return 0 if we can't calculate the size.
 *
 * Glide would report this out to us, but we don't have
 * exactly the right parameters.
 */
GLsizei
fxDDCompressedImageSize(GLcontext *ctx,
                        GLenum intFormat,
                        GLuint numDimensions,
                        GLuint width, 
                        GLuint height,
                        GLuint depth)
{
    if (numDimensions != 2) {
        return 0;
    }
    switch (intFormat) {
    case GL_COMPRESSED_RGB_FXT1_3DFX:
    case GL_COMPRESSED_RGBA_FXT1_3DFX:
       /*
        * Round height and width to multiples of 4 and 8,
        * divide the resulting product by 32 to get the number
        * of blocks, and multiply by 32 = 128/8 to get the.
        * number of bytes required.  That is to say, just
        * return the product.  Remember that we are returning
        * bytes, not texels, so we have shrunk the texture
        * by a factor of the texel size.
        */
        width = (width   + 0x7) &~ 0x7;
        height = (height + 0x3) &~ 0x3;
        return(width * height);
    }
    return(0);
}
