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


#ifndef __FX_GLIDE_WARPER__
#define __FX_GLIDE_WARPER__

#include <glide.h>
#include <g3ext.h>

typedef struct tdfx_context tdfxContextRec;
typedef struct tdfx_context *tdfxContextPtr;

/*
 * General context:
 */
typedef GrContext_t FX_GrContext_t;

#define FX_FOG_TABLE_ENTRIES            GR_FOG_TABLE_ENTRIES
#define FX_GLIDE_STATE_SIZE             GR_GLIDE_STATE_SIZE
#define FX_LFB_PIXEL_PIPE               GR_LFB_PIXEL_PIPE
#define FX_PENDING_BUFFERSWAPS          GR_PENDING_BUFFERSWAPS
#define FX_TEXTURE_ALIGN		GR_TEXTURE_ALIGN

#define FX_ZDEPTH_MAX 0x100


#define GR_ASPECT_1x1 GR_ASPECT_LOG2_1x1
#define GR_ASPECT_2x1 GR_ASPECT_LOG2_2x1
#define GR_ASPECT_4x1 GR_ASPECT_LOG2_4x1
#define GR_ASPECT_8x1 GR_ASPECT_LOG2_8x1
#define GR_ASPECT_1x2 GR_ASPECT_LOG2_1x2
#define GR_ASPECT_1x4 GR_ASPECT_LOG2_1x4
#define GR_ASPECT_1x8 GR_ASPECT_LOG2_1x8

#define GR_LOD_256	GR_LOD_LOG2_256
#define GR_LOD_128	GR_LOD_LOG2_128
#define GR_LOD_64	GR_LOD_LOG2_64
#define GR_LOD_32	GR_LOD_LOG2_32
#define GR_LOD_16	GR_LOD_LOG2_16
#define GR_LOD_8	GR_LOD_LOG2_8
#define GR_LOD_4	GR_LOD_LOG2_4
#define GR_LOD_2	GR_LOD_LOG2_2
#define GR_LOD_1	GR_LOD_LOG2_1

#define GR_FOG_WITH_TABLE GR_FOG_WITH_TABLE_ON_Q


typedef FxU32 GrHint_t;
#define GR_HINTTYPE_MIN                 0
#define GR_HINT_STWHINT                 0

typedef FxU32 GrSTWHint_t;
#define GR_STWHINT_W_DIFF_FBI   FXBIT(0)
#define GR_STWHINT_W_DIFF_TMU0  FXBIT(1)
#define GR_STWHINT_ST_DIFF_TMU0 FXBIT(2)
#define GR_STWHINT_W_DIFF_TMU1  FXBIT(3)
#define GR_STWHINT_ST_DIFF_TMU1 FXBIT(4)
#define GR_STWHINT_W_DIFF_TMU2  FXBIT(5)
#define GR_STWHINT_ST_DIFF_TMU2 FXBIT(6)

#define GR_CONTROL_ACTIVATE 		1
#define GR_CONTROL_DEACTIVATE		0

#define GrState				void

/*
** move the vertex layout defintion to application
*/
typedef struct
{
    GLfloat sow;                  /* s texture ordinate (s over w) */
    GLfloat tow;                  /* t texture ordinate (t over w) */
    GLfloat oow;                  /* 1/w (used mipmapping - really 0xfff/w) */
}
GrTmuVertex;


#if FX_USE_PARGB

/* standard vertex, packed argb, double texture, 12 dwords */
typedef struct
{
    GLfloat x, y;                 /* X and Y in screen space */
    GLfloat ooz;                  /* 65535/Z (used for Z-buffering) */
    GLfloat oow;                  /* 1/W (used for W-buffering, texturing) */
    FxU32 argb;                   /* R, G, B, A [0..255.0] */
    GrTmuVertex tmuvtx[GLIDE_NUM_TMU];
    GLfloat z;                    /* Z is ignored */
}
GrVertex;

/* optimised vertex, packed argb, single texture, 8 dwords = 1 cacheline */
typedef struct
{
    GLfloat x, y;                 /* X and Y in screen space */
    GLfloat ooz;                  /* 65535/Z (used for Z-buffering) */
    GLfloat oow;                  /* 1/W (used for W-buffering, texturing) */
    FxU32 argb;                   /* R, G, B, A [0..255.0] */
    GrTmuVertex tmuvtx;           /* only 1 TMU used to keep vertex size down */
}
GrVertex_Fast;

/* following offsets work for both vertex layouts */
#define GR_VERTEX_X_OFFSET              0
#define GR_VERTEX_Y_OFFSET              1
#define GR_VERTEX_OOZ_OFFSET            2
#define GR_VERTEX_OOW_OFFSET            3
#define GR_VERTEX_PARGB_OFFSET          4
#define GR_VERTEX_SOW_TMU0_OFFSET       5
#define GR_VERTEX_TOW_TMU0_OFFSET       6
#define GR_VERTEX_OOW_TMU0_OFFSET       7
#define GR_VERTEX_SOW_TMU1_OFFSET       8
#define GR_VERTEX_TOW_TMU1_OFFSET       9
#define GR_VERTEX_OOW_TMU1_OFFSET       10
#define GR_VERTEX_Z_OFFSET		11

#if GLIDE_ENDIAN == GLIDE_ENDIAN_BIG
#define GET_PA(v)		((FxU8*)(v))[GR_VERTEX_PARGB_OFFSET*4+3]
#define GET_PR(v)		((FxU8*)(v))[GR_VERTEX_PARGB_OFFSET*4+2]
#define GET_PG(v)		((FxU8*)(v))[GR_VERTEX_PARGB_OFFSET*4+1]
#define GET_PB(v)		((FxU8*)(v))[GR_VERTEX_PARGB_OFFSET*4+0]
#else
#define GET_PA(v)		((FxU8*)(v))[GR_VERTEX_PARGB_OFFSET*4+0]
#define GET_PR(v)		((FxU8*)(v))[GR_VERTEX_PARGB_OFFSET*4+1]
#define GET_PG(v)		((FxU8*)(v))[GR_VERTEX_PARGB_OFFSET*4+2]
#define GET_PB(v)		((FxU8*)(v))[GR_VERTEX_PARGB_OFFSET*4+3]
#endif

#define GET_PARGB(v)	        ((FxU32*)(v))[GR_VERTEX_PARGB_OFFSET]
#define PACK_4F_ARGB(dest, a, r, g, b) {		\
   const GLuint cr = (int)r;				\
   const GLuint cg = (int)g;				\
   const GLuint ca = (int)a;				\
   const GLuint cb = (int)b;				\
   dest = ca << 24 | cr << 16 | cg << 8 | cb;		\
}

#else /* FX_USE_PARGB */

typedef struct
{
    float x, y, z;              /* X, Y, and Z of scrn space -- Z is ignored */
    float r, g, b;              /* R, G, B, ([0..255.0]) */
    float ooz;                  /* 65535/Z (used for Z-buffering) */
    float a;                    /* Alpha [0..255.0] */
    float oow;                  /* 1/W (used for W-buffering, texturing) */
    GrTmuVertex tmuvtx[GLIDE_NUM_TMU];
}
GrVertex;

#define GR_VERTEX_X_OFFSET              0
#define GR_VERTEX_Y_OFFSET              1
#define GR_VERTEX_Z_OFFSET              2
#define GR_VERTEX_R_OFFSET              3
#define GR_VERTEX_G_OFFSET              4
#define GR_VERTEX_B_OFFSET              5
#define GR_VERTEX_OOZ_OFFSET            6
#define GR_VERTEX_A_OFFSET              7
#define GR_VERTEX_OOW_OFFSET            8
#define GR_VERTEX_SOW_TMU0_OFFSET       9
#define GR_VERTEX_TOW_TMU0_OFFSET       10
#define GR_VERTEX_OOW_TMU0_OFFSET       11
#define GR_VERTEX_SOW_TMU1_OFFSET       12
#define GR_VERTEX_TOW_TMU1_OFFSET       13
#define GR_VERTEX_OOW_TMU1_OFFSET       14
#endif /* FX_USE_PARGB */



extern FxI32 FX_grGetInteger_NoLock(FxU32 pname);

extern FxI32 FX_grGetInteger(tdfxContextPtr fxMesa, FxU32 pname);

extern const char *FX_grGetString(tdfxContextPtr fxMesa, FxU32 pname);


#define FX_grTexDownloadTable(fxMesa, type, data)	\
  do { 							\
    LOCK_HARDWARE(fxMesa); 				\
    grTexDownloadTable(type,data); 			\
    UNLOCK_HARDWARE(fxMesa); 				\
  } while (0);

#define FX_grTexDownloadTable_NoLock(type, data) \
  grTexDownloadTable(type, data)


#define FX_grFlush(fxMesa)	\
  do {				\
    LOCK_HARDWARE(fxMesa);	\
    grFlush();			\
    UNLOCK_HARDWARE(fxMesa);	\
  } while (0)

#define FX_grFinish(fxMesa)	\
  do {				\
    LOCK_HARDWARE(fxMesa); \
    grFinish();			\
    UNLOCK_HARDWARE(fxMesa);	\
  } while (0)


#define FX_grLfbWriteRegion(fxMesa,dst_buffer,dst_x,dst_y,src_format,src_width,src_height,src_stride,src_data)		\
  do {				\
    LOCK_HARDWARE(fxMesa);		\
    grLfbWriteRegion(dst_buffer,dst_x,dst_y,src_format,src_width,src_height,FXFALSE,src_stride,src_data);	\
    UNLOCK_HARDWARE(fxMesa);		\
  } while(0)


#define FX_grLfbReadRegion(fxMesa,src_buffer,src_x,src_y,src_width,src_height,dst_stride,dst_data)			\
  do {				\
    LOCK_HARDWARE(fxMesa);		\
    grLfbReadRegion(src_buffer,src_x,src_y,src_width,src_height,dst_stride,dst_data);				\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0);


#define FX_grDrawTriangle_NoLock(a,b,c) grDrawTriangle(a,b,c)
#define FX_grDrawTriangle(fxMesa, a,b,c)	\
  do {						\
    BEGIN_CLIP_LOOP(fxMesa);			\
    FX_grDrawTriangle_NoLock(a,b,c);		\
    END_CLIP_LOOP(fxMesa);			\
  } while (0)



extern void FX_grHints_NoLock(GrHint_t hintType, FxU32 hintMask);
extern void FX_grHints(tdfxContextPtr fxMesa, GrHint_t hintType, FxU32 hintMask);


extern void FX_grAADrawLine(tdfxContextPtr fxMesa, GrVertex * a, GrVertex * b);

extern void FX_grAADrawPoint(tdfxContextPtr fxMesa, GrVertex * a);


extern void FX_setupGrVertexLayout(tdfxContextPtr fxMesa);


extern FX_GrContext_t FX_grSstWinOpen(tdfxContextPtr fxMesa,
                                      FxU32 hWnd,
                                      GrScreenResolution_t screen_resolution,
                                      GrScreenRefresh_t refresh_rate,
                                      GrColorFormat_t color_format,
                                      GrOriginLocation_t origin_location,
                                      int nColBuffers, int nAuxBuffers);


#define FX_grDrawLine_NoLock(v1, v2) grDrawLine(v1, v2)
#define FX_grDrawLine(fxMesa, v1, v2)	\
  do {					\
    BEGIN_CLIP_LOOP(fxMesa);		\
    FX_grDrawLine_NoLock(v1, v2);	\
    END_CLIP_LOOP(fxMesa);		\
  } while (0)

#define FX_grDrawPoint_NoLock(p) grDrawPoint(p)
#define FX_grDrawPoint(fxMesa, p)	\
  do {					\
    BEGIN_CLIP_LOOP(fxMesa);		\
    FX_grDrawPoint_NoLock(p);		\
    END_CLIP_LOOP(fxMesa);		\
  } while (0)

extern void FX_grDrawPolygonVertexList(tdfxContextPtr fxMesa,
                                       int n, GrVertex * v);

#define FX_grDitherMode(fxMesa, m)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grDitherMode(m);			\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)

#define FX_grRenderBuffer(fxMesa, b)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grRenderBuffer(b);			\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)

#define FX_grRenderBuffer_NoLock(b)  grRenderBuffer(b)

#define FX_grBufferClear(fxMesa, c, a, d)	\
  do {						\
    BEGIN_CLIP_LOOP(fxMesa);			\
    grBufferClear(c, a, d);			\
    END_CLIP_LOOP(fxMesa);			\
  } while (0)

#define FX_grBufferClear_NoLock(c, a, d)  grBufferClear(c, a, d)


#define FX_grBufferClearExt_NoLock(c, a, d, s)  grBufferClearExtProc(c, a, d, s)

#define FX_grBufferClearExt(fxMesa, c, a, d, s)	\
  do {						\
    BEGIN_CLIP_LOOP(fxMesa);			\
    grBufferClearExtProc(c, a, d, s);		\
    END_CLIP_LOOP(fxMesa);			\
  } while (0)

#define FX_grEnable(fxMesa, m)           \
  do {				         \
    LOCK_HARDWARE(fxMesa);	         \
    grEnable(m);                         \
    UNLOCK_HARDWARE(fxMesa);	         \
  } while (0)

#define FX_grEnable_NoLock(m) grEnable(m)

#define FX_grDisable(fxMesa, m)          \
  do {				         \
    LOCK_HARDWARE(fxMesa);	         \
    grDisable(m);                        \
    UNLOCK_HARDWARE(fxMesa);	         \
  } while (0)

#define FX_grDisable_NoLock(m) grDisable(m)


#define FX_grStencilFunc(fxMesa, fnc, ref, mask)	\
  do {							\
    LOCK_HARDWARE(fxMesa);				\
    grStencilFuncProc((fnc), (ref), (mask));		\
    UNLOCK_HARDWARE(fxMesa);				\
  } while (0)

#define FX_grStencilFunc_NoLock(f, r, m)  grStencilFuncProc(f, r, m)

#define FX_grStencilMask(fxMesa, write_mask)	\
  do {						\
    LOCK_HARDWARE(fxMesa);			\
    grStencilMaskProc(write_mask);		\
    UNLOCK_HARDWARE(fxMesa);			\
  } while (0)

#define FX_grStencilMask_NoLock(m) grStencilMaskProc(m)

#define FX_grStencilOp(fxMesa, stencil_fail, depth_fail, depth_pass)	\
  do {									\
    LOCK_HARDWARE(fxMesa);						\
    grStencilOpProc((stencil_fail), (depth_fail), (depth_pass));	\
    UNLOCK_HARDWARE(fxMesa);						\
  } while (0)

#define FX_grStencilOp_NoLock(sf, df, dp)  grStencilOpProc(sf, df, dp)

#define FX_grDepthMask(fxMesa, m)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grDepthMask(m);			\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)

#define FX_grDepthMask_NoLock(m)  grDepthMask(m)


extern void FX_grColorMask(GLcontext *ctx, GLboolean r, GLboolean g,
                           GLboolean b, GLboolean a);

extern void FX_grColorMask_NoLock(GLcontext *ctx, GLboolean r, GLboolean g,
                                  GLboolean b, GLboolean a);

extern void FX_grColorMaskv(GLcontext *ctx, const GLboolean rgba[4]);

extern void FX_grColorMaskv_NoLock(GLcontext *ctx, const GLboolean rgba[4]);


extern FxBool FX_grLfbLock(tdfxContextPtr fxMesa,
                           GrLock_t type, GrBuffer_t buffer,
                           GrLfbWriteMode_t writeMode,
                           GrOriginLocation_t origin, FxBool pixelPipeline,
                           GrLfbInfo_t * info);

#define FX_grLfbUnlock(fxMesa, t, b)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grLfbUnlock(t, b);			\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)

#define FX_grConstantColorValue(fxMesa, v)	\
  do {						\
    LOCK_HARDWARE(fxMesa);			\
    grConstantColorValue(v);			\
    UNLOCK_HARDWARE(fxMesa);			\
  } while (0)

#define FX_grConstantColorValue_NoLock grConstantColorValue

#define FX_grAADrawTriangle(fxMesa, a, b, c, ab, bc, ca)	\
  do {								\
    BEGIN_CLIP_LOOP(fxMesa);					\
    grAADrawTriangle(a, b, c, ab, bc, ca);			\
    END_CLIP_LOOP(fxMesa);					\
  } while (0)

#define FX_grAlphaBlendFunction(rs, rd, as, ad)		\
  do {							\
    LOCK_HARDWARE(fxMesa);				\
    grAlphaBlendFunction(rs, rd, as, ad);		\
    UNLOCK_HARDWARE(fxMesa);				\
  } while (0)

#define FX_grAlphaCombine(func, fact, loc, oth, inv)	\
  do {							\
    LOCK_HARDWARE(fxMesa);				\
    grAlphaCombine(func, fact, loc, oth, inv);		\
    UNLOCK_HARDWARE(fxMesa);				\
  } while (0)

#define FX_grAlphaCombine_NoLock grAlphaCombine

#define FX_grAlphaTestFunction(fxMesa, f)	\
  do {						\
    LOCK_HARDWARE(fxMesa);			\
    grAlphaTestFunction(f);			\
    UNLOCK_HARDWARE(fxMesa);			\
  } while (0)

#define FX_grAlphaTestReferenceValue(fxMesa, v)	\
  do {						\
    LOCK_HARDWARE(fxMesa);			\
    grAlphaTestReferenceValue(v);		\
    UNLOCK_HARDWARE(fxMesa);			\
  } while (0)

#define FX_grClipWindow(fxMesa, minx, miny, maxx, maxy)	\
  do {							\
    LOCK_HARDWARE(fxMesa);				\
    grClipWindow(minx, miny, maxx, maxy);		\
    UNLOCK_HARDWARE(fxMesa);				\
  } while (0)

#define FX_grClipWindow_NoLock grClipWindow

#define FX_grColorCombine(fxMesa, func, fact, loc, oth, inv)	\
  do {								\
    LOCK_HARDWARE(fxMesa);					\
    grColorCombine(func, fact, loc, oth, inv);			\
    UNLOCK_HARDWARE(fxMesa);					\
  } while (0)

#define FX_grColorCombine_NoLock grColorCombine

#define FX_grCullMode(fxMesa, m)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grCullMode(m);			\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)

#define FX_grDepthBiasLevel(fxMesa, lev)	\
  do {						\
    LOCK_HARDWARE(fxMesa);			\
    grDepthBiasLevel(lev);			\
    UNLOCK_HARDWARE(fxMesa);			\
  } while (0)

#define FX_grDepthBufferFunction(fxMesa, func)	\
  do {						\
    LOCK_HARDWARE(fxMesa);			\
    grDepthBufferFunction(func);		\
    UNLOCK_HARDWARE(fxMesa);			\
  } while (0)

#define FX_grFogColorValue(fxMesa, c)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grFogColorValue(c);			\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)

#define FX_grFogMode(fxMesa, m)	\
  do {				\
    LOCK_HARDWARE(fxMesa);	\
    grFogMode(m);		\
    UNLOCK_HARDWARE(fxMesa);	\
  } while (0)

#define FX_grFogTable(fxMesa, t)\
  do {				\
    LOCK_HARDWARE(fxMesa);	\
    grFogTable(t);		\
    UNLOCK_HARDWARE(fxMesa);	\
  } while (0)

#define FX_grTexClampMode(fxMesa, t, sc, tc)	\
  do {						\
    LOCK_HARDWARE(fxMesa);			\
    grTexClampMode(t, sc, tc);			\
    UNLOCK_HARDWARE(fxMesa);			\
  } while (0)

#define FX_grTexClampMode_NoLock grTexClampMode

#define FX_grTexCombine(fxMesa, t, rfunc, rfact, afunc, afact, rinv, ainv)	\
  do {									\
    LOCK_HARDWARE(fxMesa);						\
    grTexCombine(t, rfunc, rfact, afunc, afact, rinv, ainv);		\
    UNLOCK_HARDWARE(fxMesa);						\
  } while (0)

#define FX_grTexCombine_NoLock grTexCombine

#define FX_grTexDownloadMipMapLevel(fxMesa, t, sa, tlod, llod, ar, f, eo, d)	\
  do {									\
    LOCK_HARDWARE(fxMesa);						\
    grTexDownloadMipMapLevel(t, sa, tlod, llod, ar, f, eo, d);		\
    UNLOCK_HARDWARE(fxMesa);						\
  } while (0)

#define FX_grTexDownloadMipMapLevelPartial(fxMesa, t, sa, tlod, llod, ar, f, eo, d, s, e);	\
  do {									    \
    LOCK_HARDWARE(fxMesa);						    \
    grTexDownloadMipMapLevelPartial(t, sa, tlod, llod, ar, f, eo, d, s, e); \
    UNLOCK_HARDWARE(fxMesa);						    \
  } while (0)

#define FX_grTexFilterMode(fxMesa, t, minf, magf)	\
  do {							\
    LOCK_HARDWARE(fxMesa);				\
    grTexFilterMode(t, minf, magf);			\
    UNLOCK_HARDWARE(fxMesa);				\
  } while (0)

#define FX_grTexFilterMode_NoLock grTexFilterMode

extern FxU32 FX_grTexMinAddress(tdfxContextPtr fxMesa, GrChipID_t tmu);
extern FxU32 FX_grTexMaxAddress(tdfxContextPtr fxMesa, GrChipID_t tmu);

#define FX_grTexMipMapMode(t, m, lod)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grTexMipMapMode(t, m, lod);		\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)

#define FX_grTexMipMapMode_NoLock grTexMipMapMode

#define FX_grTexSource(t, sa, eo, i)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grTexSource(t, sa, eo, i);		\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)

#define FX_grTexSource_NoLock grTexSource

extern FxU32 FX_grTexTextureMemRequired(tdfxContextPtr fxMesa,
                                        FxU32 evenOdd, GrTexInfo * info);

#define FX_grTexTextureMemRequired_NoLock grTexTextureMemRequired

#define FX_grGlideGetState(fxMesa, s)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grGlideGetState(s);			\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)
#define FX_grGlideGetState_NoLock(s) grGlideGetState(s);

#define FX_grDRIBufferSwap(fxMesa, i)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grDRIBufferSwap(i);			\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)

#define FX_grSstSelect(fxMesa, b)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grSstSelect(b);			\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)

#define FX_grSstSelect_NoLock grSstSelect

#define FX_grGlideSetState(fxMesa, s)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grGlideSetState(s);			\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)
#define FX_grGlideSetState_NoLock(s) grGlideSetState(s);

#define FX_grDepthBufferMode(fxMesa, m)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grDepthBufferMode(m);		\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)

#define FX_grLfbWriteColorFormat(fxMesa, f)	\
  do {						\
    LOCK_HARDWARE(fxMesa);			\
    grLfbWriteColorFormat(f);			\
    UNLOCK_HARDWARE(fxMesa);			\
  } while (0)

#define FX_grDrawVertexArray(fxMesa, m, c, p)	\
  do {						\
    BEGIN_CLIP_LOOP(fxMesa);			\
    grDrawVertexArray(m, c, p);			\
    END_CLIP_LOOP(fxMesa);			\
  } while (0)

#define FX_grGlideShutdown(fxMesa)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grGlideShutdown();			\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)

#define FX_grTexLodBiasValue_NoLock(t, v) grTexLodBiasValue(t, v)

#define FX_grTexLodBiasValue(t, v)	\
  do {					\
    LOCK_HARDWARE(fxMesa);		\
    grTexLodBiasValue(t, v);		\
    UNLOCK_HARDWARE(fxMesa);		\
  } while (0)

#define FX_grGlideInit_NoLock grGlideInit
#define FX_grSstWinOpen_NoLock grSstWinOpen

extern int FX_getFogTableSize(tdfxContextPtr fxMesa);

extern int FX_getGrStateSize(tdfxContextPtr fxMesa);

#endif /* __FX_GLIDE_WARPER__ */
