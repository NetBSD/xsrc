/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/fxglidew.h,v 1.3 2000/12/08 19:36:23 alanh Exp $ */
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

typedef struct tfxMesaContext *fxMesaContext;


/*
 * These are glide extension definitions.  These are not
 * defined in glide.h.  They should really be defined in
 * g3ext.h, but they are not.
 */
#if 0
FX_ENTRY void FX_CALL
grStencilFunc(GrCmpFnc_t fnc, GrStencil_t ref, GrStencil_t mask);

FX_ENTRY void FX_CALL grStencilMask(GrStencil_t write_mask);

FX_ENTRY void FX_CALL
grStencilOp(GrStencilOp_t stencil_fail,
            GrStencilOp_t depth_fail, GrStencilOp_t depth_pass);

FX_ENTRY void FX_CALL
grBufferClearExt(GrColor_t color,
                 GrAlpha_t alpha, FxU32 depth, GrStencil_t stencil);

FX_ENTRY void FX_CALL
grColorMaskExt(FxBool r, FxBool g, FxBool b, FxBool a);
#endif


typedef void (*grStencilFunc_t) (GrCmpFnc_t fnc, GrStencil_t ref,
                                 GrStencil_t mask);
typedef void (*grStencilMask_t) (GrStencil_t write_mask);
typedef void (*grStencilOp_t) (GrStencilOp_t stencil_fail,
                               GrStencilOp_t depth_fail,
                               GrStencilOp_t depth_pass);
typedef void (*grBufferClearExt_t) (GrColor_t color, GrAlpha_t alpha,
                                    FxU32 depth, GrStencil_t stencil);
typedef void (*grColorMaskExt_t) (FxBool r, FxBool g, FxBool b, FxBool a);

/*
 * "COMBINE" extension for Napalm
 */
typedef void (*grColorCombineExt_t)(GrCCUColor_t a, GrCombineMode_t a_mode,
                                    GrCCUColor_t b, GrCombineMode_t b_mode,
                                    GrCCUColor_t c, FxBool c_invert,
                                    GrCCUColor_t d, FxBool d_invert,
                                    FxU32 shift, FxBool invert);
typedef void (*grTexColorCombineExt_t)(FxU32 tmu,
                                       GrTCCUColor_t a, GrCombineMode_t a_mode,
                                       GrCCUColor_t b, GrCombineMode_t b_mode,
                                       GrCCUColor_t c, FxBool c_invert,
                                       GrCCUColor_t d, FxBool d_invert,
                                       FxU32 shift, FxBool invert);
typedef void (*grAlphaCombineExt_t)(GrCCUColor_t a, GrCombineMode_t a_mode,
                                    GrCCUColor_t b, GrCombineMode_t b_mode,
                                    GrCCUColor_t c, FxBool c_invert,
                                    GrCCUColor_t d, FxBool d_invert,
                                    FxU32 shift, FxBool invert);
typedef void (*grTexAlphaCombineExt_t)(FxU32 tmu,
                                       GrTACUColor_t a, GrCombineMode_t a_mode,
                                       GrTACUColor_t b, GrCombineMode_t b_mode,
                                       GrTACUColor_t c, FxBool c_invert,
                                       GrTACUColor_t d, FxBool d_invert,
                                       FxU32 shift, FxBool invert);
typedef void (*grAlphaBlendFunctionExt_t)(GrAlphaBlendFnc_t rgb_sf,
                                          GrAlphaBlendFnc_t rgb_df,
                                          GrAlphaBlendOp_t rgb_op,
                                          GrAlphaBlendFnc_t alpha_sf,
                                          GrAlphaBlendFnc_t alpha_df,
                                          GrAlphaBlendOp_t alpha_op);
typedef void (*grConstantColorValueExt_t)(FxU32 tmu, GrColor_t value);



/*
 * These are functions to compress and decompress images.
 * The types of the first and second parameters are not exactly
 * right.  The texus library declares them to be "char *", not
 * "void *".  However, "void *" is more correct, and more convenient.
 */
typedef void (*txImgQuantize_t)  (void *dst, void *src, 
                                  int w, int h, 
                                  FxU32 format, FxU32 dither);
typedef void (*txImgDeQuantize_t)(void *dst, void *src, 
                                  int w, int h);
/*
 * These next three declarations should probably be taken from
 * texus.h.  However, there are duplicate declarations in g3ext.h
 * and texus.h which make it hard to include them both.
 */
typedef void (*TxErrorCallbackFnc_t)( const char *string, FxBool fatal );
typedef void (*txErrorSetCallback_t)(TxErrorCallbackFnc_t  fnc,
                                     TxErrorCallbackFnc_t *old_fnc);

extern grStencilFunc_t grStencilFuncPtr;
extern grStencilMask_t grStencilMaskPtr;
extern grStencilOp_t grStencilOpPtr;
extern grBufferClearExt_t grBufferClearExtPtr;
extern grColorMaskExt_t grColorMaskExtPtr;
extern txImgQuantize_t  txImgQuantizePtr;
extern txImgDeQuantize_t txImgDequantizeFXT1Ptr;
extern txErrorSetCallback_t txErrorSetCallbackPtr;
extern grColorCombineExt_t grColorCombineExtPtr;
extern grTexColorCombineExt_t grTexColorCombineExtPtr;
extern grAlphaCombineExt_t grAlphaCombineExtPtr;
extern grTexAlphaCombineExt_t grTexAlphaCombineExtPtr;
extern grAlphaBlendFunctionExt_t grAlphaBlendFunctionExtPtr;
extern grConstantColorValueExt_t grConstantColorValueExtPtr;


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
GrVertex_12;

/* optimised vertex, packed argb, single texture, 8 dwords = 1 cacheline */
typedef struct
{
    GLfloat x, y;                 /* X and Y in screen space */
    GLfloat ooz;                  /* 65535/Z (used for Z-buffering) */
    GLfloat oow;                  /* 1/W (used for W-buffering, texturing) */
    FxU32 argb;                   /* R, G, B, A [0..255.0] */
    GrTmuVertex tmuvtx;           /* only 1 TMU used to keep vertex size down */
}
GrVertex_8;

/* vertex structure, padded to 16 dwords */
typedef union
{
    GrVertex_8    v_8;
    GrVertex_12   v_12;
    GLfloat       f[16];
    GLuint        u[16];
}
GrVertex_t;

/* keep the compiler happy for now */
typedef GrVertex_12 GrVertex;

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
#define PACK_4F_ARGB(dest, a, r, g, b) { 					\
    			        const GLuint cr = (int)r;		        \
    				const GLuint cg = (int)g;		        \
    				const GLuint ca = (int)a;		        \
    				const GLuint cb = (int)b;		        \
    				dest = ca << 24 | cr << 16 | cg << 8 | cb;	\
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

extern FxI32 FX_grGetInteger(fxMesaContext fxMesa, FxU32 pname);

extern const char *FX_grGetString(fxMesaContext fxMesa, FxU32 pname);


#define FX_grTexDownloadTable(fxMesa, TMU, type, data)	\
  do { 							\
    BEGIN_BOARD_LOCK(fxMesa); 				\
    grTexDownloadTable(type,data); 			\
    END_BOARD_LOCK(fxMesa); 				\
  } while (0);

#define FX_grTexDownloadTable_NoLock(TMU, type, data) \
  grTexDownloadTable(type, data)


#define FX_grFlush(fxMesa)	\
  do {				\
    BEGIN_BOARD_LOCK(fxMesa);	\
    grFlush();			\
    END_BOARD_LOCK(fxMesa);	\
  } while (0)

#define FX_grFinish(fxMesa)	\
  do {				\
    BEGIN_BOARD_LOCK(fxMesa); \
    grFinish();			\
    END_BOARD_LOCK(fxMesa);	\
  } while (0)


#define FX_grLfbWriteRegion(fxMesa,dst_buffer,dst_x,dst_y,src_format,src_width,src_height,src_stride,src_data)		\
  do {				\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grLfbWriteRegion(dst_buffer,dst_x,dst_y,src_format,src_width,src_height,FXFALSE,src_stride,src_data);	\
    END_BOARD_LOCK(fxMesa);		\
  } while(0)


#define FX_grLfbReadRegion(fxMesa,src_buffer,src_x,src_y,src_width,src_height,dst_stride,dst_data)			\
  do {				\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grLfbReadRegion(src_buffer,src_x,src_y,src_width,src_height,dst_stride,dst_data);				\
    END_BOARD_LOCK(fxMesa);		\
  } while (0);


#define FX_grDrawTriangle_NoLock(a,b,c) grDrawTriangle(a,b,c)
#define FX_grDrawTriangle(fxMesa, a,b,c)	\
  do {						\
    BEGIN_CLIP_LOOP(fxMesa);			\
    FX_grDrawTriangle_NoLock(a,b,c);		\
    END_CLIP_LOOP(fxMesa);			\
  } while (0)

/*
 * For Lod/LodLog2 conversion.
 */
#define FX_largeLodLog2(info)		(info).largeLodLog2

#define FX_smallLodLog2(info)		(info).smallLodLog2

#define FX_aspectRatioLog2(info)	(info).aspectRatioLog2

#define FX_lodToValue(val)		((int)(GR_LOD_256-val))

#define FX_largeLodValue(info)		((int)(GR_LOD_256-(info).largeLodLog2))
#define FX_largeLodValue_NoLock FX_largeLodValue

#define FX_smallLodValue(info)		((int)(GR_LOD_256-(info).smallLodLog2))
#define FX_smallLodValue_NoLock FX_smallLodValue

#define FX_valueToLod(val)		((GrLOD_t)(GR_LOD_256-val))



extern void FX_grHints_NoLock(GrHint_t hintType, FxU32 hintMask);
extern void FX_grHints(fxMesaContext fxMesa, GrHint_t hintType, FxU32 hintMask);


extern void FX_grAADrawLine(fxMesaContext fxMesa, GrVertex * a, GrVertex * b);

extern void FX_grAADrawPoint(fxMesaContext fxMesa, GrVertex * a);


extern void FX_setupGrVertexLayout(fxMesaContext fxMesa);


extern FX_GrContext_t FX_grSstWinOpen(fxMesaContext fxMesa,
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

extern void FX_grDrawPolygonVertexList(fxMesaContext fxMesa,
                                       int n, GrVertex * v);

#define FX_grDitherMode(fxMesa, m)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grDitherMode(m);			\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)

#define FX_grRenderBuffer(fxMesa, b)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grRenderBuffer(b);			\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)

#define FX_grRenderBuffer_NoLock(b)  grRenderBuffer(b)

#define FX_grBufferClear(fxMesa, c, a, d)	\
  do {						\
    BEGIN_CLIP_LOOP(fxMesa);			\
    grBufferClear(c, a, d);			\
    END_CLIP_LOOP(fxMesa);			\
  } while (0)

#define FX_grBufferClearExt_NoLock(c, a, d, s)  (*grBufferClearExtPtr)(c, a, d, s)

#define FX_grBufferClearExt(fxMesa, c, a, d, s)	\
  do {						\
    BEGIN_CLIP_LOOP(fxMesa);			\
    (*grBufferClearExtPtr)(c, a, d, s);		\
    END_CLIP_LOOP(fxMesa);			\
  } while (0)

#define FX_grBufferClear_NoLock(c, a, d)  grBufferClear(c, a, d)


#define FX_grEnable(fxMesa, m)           \
  do {				         \
    BEGIN_BOARD_LOCK(fxMesa);	         \
    grEnable(m);                         \
    END_BOARD_LOCK(fxMesa);	         \
  } while (0)

#define FX_grEnable_NoLock(m) grEnable(m)

#define FX_grDisable(fxMesa, m)          \
  do {				         \
    BEGIN_BOARD_LOCK(fxMesa);	         \
    grDisable(m);                        \
    END_BOARD_LOCK(fxMesa);	         \
  } while (0)

#define FX_grDisable_NoLock(m) grDisable(m)


#define FX_grStencilFunc(fxMesa, fnc, ref, mask)	\
  do {							\
    BEGIN_BOARD_LOCK(fxMesa);				\
    (*grStencilFuncPtr)((fnc), (ref), (mask));		\
    END_BOARD_LOCK(fxMesa);				\
  } while (0)

#define FX_grStencilFunc_NoLock(f, r, m)  (*grStencilFuncPtr)(f, r, m)

#define FX_grStencilMask(fxMesa, write_mask)	\
  do {						\
    BEGIN_BOARD_LOCK(fxMesa);			\
    (*grStencilMaskPtr)(write_mask);		\
    END_BOARD_LOCK(fxMesa);			\
  } while (0)

#define FX_grStencilMask_NoLock(m) (*grStencilMaskPtr)(m)

#define FX_grStencilOp(fxMesa, stencil_fail, depth_fail, depth_pass)	\
  do {									\
    BEGIN_BOARD_LOCK(fxMesa);						\
    (*grStencilOpPtr)((stencil_fail), (depth_fail), (depth_pass));	\
    END_BOARD_LOCK(fxMesa);						\
  } while (0)

#define FX_grStencilOp_NoLock(sf, df, dp)  (*grStencilOpPtr)(sf, df, dp)

#define FX_grDepthMask(fxMesa, m)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grDepthMask(m);			\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)

#define FX_grDepthMask_NoLock(m)  grDepthMask(m)


extern void FX_grColorMask(GLcontext *ctx, GLboolean r, GLboolean g,
                           GLboolean b, GLboolean a);

extern void FX_grColorMask_NoLock(GLcontext *ctx, GLboolean r, GLboolean g,
                                  GLboolean b, GLboolean a);

extern void FX_grColorMaskv(GLcontext *ctx, const GLboolean rgba[4]);

extern void FX_grColorMaskv_NoLock(GLcontext *ctx, const GLboolean rgba[4]);


extern FxBool FX_grLfbLock(fxMesaContext fxMesa,
                           GrLock_t type, GrBuffer_t buffer,
                           GrLfbWriteMode_t writeMode,
                           GrOriginLocation_t origin, FxBool pixelPipeline,
                           GrLfbInfo_t * info);

#define FX_grLfbUnlock(fxMesa, t, b)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grLfbUnlock(t, b);			\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)

#define FX_grConstantColorValue(fxMesa, v)	\
  do {						\
    BEGIN_BOARD_LOCK(fxMesa);			\
    grConstantColorValue(v);			\
    END_BOARD_LOCK(fxMesa);			\
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
    BEGIN_BOARD_LOCK(fxMesa);				\
    grAlphaBlendFunction(rs, rd, as, ad);		\
    END_BOARD_LOCK(fxMesa);				\
  } while (0)

#define FX_grAlphaCombine(func, fact, loc, oth, inv)	\
  do {							\
    BEGIN_BOARD_LOCK(fxMesa);				\
    grAlphaCombine(func, fact, loc, oth, inv);		\
    END_BOARD_LOCK(fxMesa);				\
  } while (0)

#define FX_grAlphaCombine_NoLock grAlphaCombine

#define FX_grAlphaTestFunction(fxMesa, f)	\
  do {						\
    BEGIN_BOARD_LOCK(fxMesa);			\
    grAlphaTestFunction(f);			\
    END_BOARD_LOCK(fxMesa);			\
  } while (0)

#define FX_grAlphaTestReferenceValue(fxMesa, v)	\
  do {						\
    BEGIN_BOARD_LOCK(fxMesa);			\
    grAlphaTestReferenceValue(v);		\
    END_BOARD_LOCK(fxMesa);			\
  } while (0)

#define FX_grClipWindow(fxMesa, minx, miny, maxx, maxy)	\
  do {							\
    BEGIN_BOARD_LOCK(fxMesa);				\
    grClipWindow(minx, miny, maxx, maxy);		\
    END_BOARD_LOCK(fxMesa);				\
  } while (0)

#define FX_grClipWindow_NoLock grClipWindow

#define FX_grColorCombine(fxMesa, func, fact, loc, oth, inv)	\
  do {								\
    BEGIN_BOARD_LOCK(fxMesa);					\
    grColorCombine(func, fact, loc, oth, inv);			\
    END_BOARD_LOCK(fxMesa);					\
  } while (0)

#define FX_grColorCombine_NoLock grColorCombine

#define FX_grCullMode(fxMesa, m)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grCullMode(m);			\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)

#define FX_grDepthBiasLevel(fxMesa, lev)	\
  do {						\
    BEGIN_BOARD_LOCK(fxMesa);			\
    grDepthBiasLevel(lev);			\
    END_BOARD_LOCK(fxMesa);			\
  } while (0)

#define FX_grDepthBufferFunction(fxMesa, func)	\
  do {						\
    BEGIN_BOARD_LOCK(fxMesa);			\
    grDepthBufferFunction(func);		\
    END_BOARD_LOCK(fxMesa);			\
  } while (0)

#define FX_grFogColorValue(fxMesa, c)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grFogColorValue(c);			\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)

#define FX_grFogMode(fxMesa, m)	\
  do {				\
    BEGIN_BOARD_LOCK(fxMesa);	\
    grFogMode(m);		\
    END_BOARD_LOCK(fxMesa);	\
  } while (0)

#define FX_grFogTable(fxMesa, t)\
  do {				\
    BEGIN_BOARD_LOCK(fxMesa);	\
    grFogTable(t);		\
    END_BOARD_LOCK(fxMesa);	\
  } while (0)

#define FX_grTexClampMode(fxMesa, t, sc, tc)	\
  do {						\
    BEGIN_BOARD_LOCK(fxMesa);			\
    grTexClampMode(t, sc, tc);			\
    END_BOARD_LOCK(fxMesa);			\
  } while (0)

#define FX_grTexClampMode_NoLock grTexClampMode

#define FX_grTexCombine(fxMesa, t, rfunc, rfact, afunc, afact, rinv, ainv)	\
  do {									\
    BEGIN_BOARD_LOCK(fxMesa);						\
    grTexCombine(t, rfunc, rfact, afunc, afact, rinv, ainv);		\
    END_BOARD_LOCK(fxMesa);						\
  } while (0)

#define FX_grTexCombine_NoLock grTexCombine

#define FX_grTexDownloadMipMapLevel(fxMesa, t, sa, tlod, llod, ar, f, eo, d)	\
  do {									\
    BEGIN_BOARD_LOCK(fxMesa);						\
    grTexDownloadMipMapLevel(t, sa, tlod, llod, ar, f, eo, d);		\
    END_BOARD_LOCK(fxMesa);						\
  } while (0)

#define FX_grTexDownloadMipMapLevel_NoLock grTexDownloadMipMapLevel

#define FX_grTexDownloadMipMapLevelPartial(fxMesa, t, sa, tlod, llod, ar, f, eo, d, s, e);	\
  do {									    \
    BEGIN_BOARD_LOCK(fxMesa);						    \
    grTexDownloadMipMapLevelPartial(t, sa, tlod, llod, ar, f, eo, d, s, e); \
    END_BOARD_LOCK(fxMesa);						    \
  } while (0)

#define FX_grTexFilterMode(fxMesa, t, minf, magf)	\
  do {							\
    BEGIN_BOARD_LOCK(fxMesa);				\
    grTexFilterMode(t, minf, magf);			\
    END_BOARD_LOCK(fxMesa);				\
  } while (0)

#define FX_grTexFilterMode_NoLock grTexFilterMode

extern FxU32 FX_grTexMinAddress(fxMesaContext fxMesa, GrChipID_t tmu);
extern FxU32 FX_grTexMaxAddress(fxMesaContext fxMesa, GrChipID_t tmu);

#define FX_grTexMipMapMode(t, m, lod)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grTexMipMapMode(t, m, lod);		\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)

#define FX_grTexMipMapMode_NoLock grTexMipMapMode

#define FX_grTexSource(t, sa, eo, i)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grTexSource(t, sa, eo, i);		\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)

#define FX_grTexSource_NoLock grTexSource

extern FxU32 FX_grTexTextureMemRequired(fxMesaContext fxMesa,
                                        FxU32 evenOdd, GrTexInfo * info);

#define FX_grTexTextureMemRequired_NoLock grTexTextureMemRequired

#define FX_grGlideGetState(fxMesa, s)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grGlideGetState(s);			\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)
#define FX_grGlideGetState_NoLock(s) grGlideGetState(s);

#define FX_grDRIBufferSwap(fxMesa, i)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grDRIBufferSwap(i);			\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)

#define FX_grSstSelect(fxMesa, b)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grSstSelect(b);			\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)

#define FX_grSstSelect_NoLock grSstSelect

#define FX_grGlideSetState(fxMesa, s)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grGlideSetState(s);			\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)
#define FX_grGlideSetState_NoLock(s) grGlideSetState(s);

#define FX_grDepthBufferMode(fxMesa, m)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grDepthBufferMode(m);		\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)

#define FX_grLfbWriteColorFormat(fxMesa, f)	\
  do {						\
    BEGIN_BOARD_LOCK(fxMesa);			\
    grLfbWriteColorFormat(f);			\
    END_BOARD_LOCK(fxMesa);			\
  } while (0)

#define FX_grDrawVertexArray(fxMesa, m, c, p)	\
  do {						\
    BEGIN_CLIP_LOOP(fxMesa);			\
    grDrawVertexArray(m, c, p);			\
    END_CLIP_LOOP(fxMesa);			\
  } while (0)

#define FX_grGlideShutdown(fxMesa)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grGlideShutdown();			\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)

#define FX_grTexLodBiasValue_NoLock(t, v) grTexLodBiasValue(t, v)

#define FX_grTexLodBiasValue(t, v)	\
  do {					\
    BEGIN_BOARD_LOCK(fxMesa);		\
    grTexLodBiasValue(t, v);		\
    END_BOARD_LOCK(fxMesa);		\
  } while (0)

#define FX_grGlideInit_NoLock grGlideInit
#define FX_grSstWinOpen_NoLock grSstWinOpen

extern int FX_getFogTableSize(fxMesaContext fxMesa);

extern int FX_getGrStateSize(fxMesaContext fxMesa);

#endif /* __FX_GLIDE_WARPER__ */
