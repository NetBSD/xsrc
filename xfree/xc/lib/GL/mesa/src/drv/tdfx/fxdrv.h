/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/fxdrv.h,v 1.2 2000/12/08 19:36:23 alanh Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Daryll Strauss <daryll@precisioninsight.com>
 *   Brian Paul <brianp@valinux.com>
 */


#ifndef _FXDRV_H_
#define _FXDRV_H_

#ifdef GLX_DIRECT_RENDERING

#include <sys/time.h>
#include "dri_tmm.h"
#include "dri_mesaint.h"
#include "dri_mesa.h"
#include "dri_xmesaapi.h"
#ifdef XFree86Server
#include "GL/xf86glx.h"
#else
#include "glheader.h"
#endif
#if defined(__linux__)
#include <signal.h>
#endif
#include "clip.h"
#include "context.h"
#include "fxglidew.h"
#include "macros.h"
#include "matrix.h"
#include "mem.h"
#include "texture.h"
#include "types.h"
#include "vb.h"
#include "vbrender.h"
#include "xform.h"

typedef struct {
    drmHandle handle;
    drmSize size;
    drmAddress map;
} tdfxRegion, *tdfxRegionPtr;

typedef struct {
    tdfxRegion regs;
    int deviceID;
    int width;
    int height;
    int mem;
    int cpp;
    int stride;
    int fifoOffset;
    int fifoSize;
    int fbOffset;
    int backOffset;
    int depthOffset;
    int textureOffset;
    int textureSize;
    __DRIscreenPrivate *driScrnPriv;
} tdfxScreenPrivate;

typedef struct {
    volatile int fifoPtr;
    volatile int fifoRead;
    volatile int fifoOwner;
    volatile int ctxOwner;
    volatile int texOwner;
} TDFXSAREAPriv;


extern void fx_sanity_triangle(fxMesaContext fxMesa,
                               GrVertex *, GrVertex *, GrVertex *);
#if defined(MESA_DEBUG) && 0
#define grDrawTriangle fx_sanity_triangle
#endif


/* Define some shorter names for these things.
 */
#define XCOORD   GR_VERTEX_X_OFFSET
#define YCOORD   GR_VERTEX_Y_OFFSET
#define ZCOORD   GR_VERTEX_OOZ_OFFSET
#define OOWCOORD GR_VERTEX_OOW_OFFSET

#define RCOORD   GR_VERTEX_R_OFFSET
#define GCOORD   GR_VERTEX_G_OFFSET
#define BCOORD   GR_VERTEX_B_OFFSET
#define ACOORD   GR_VERTEX_A_OFFSET

#define S0COORD  GR_VERTEX_SOW_TMU0_OFFSET
#define T0COORD  GR_VERTEX_TOW_TMU0_OFFSET
#define S1COORD  GR_VERTEX_SOW_TMU1_OFFSET
#define T1COORD  GR_VERTEX_TOW_TMU1_OFFSET


#define CLIP_XCOORD 0           /* normal place */
#define CLIP_YCOROD 1           /* normal place */
#define CLIP_ZCOORD 2           /* GR_VERTEX_Z_OFFSET */
#define CLIP_WCOORD 3           /* GR_VERTEX_R_OFFSET */
#define CLIP_GCOORD 4           /* normal place */
#define CLIP_BCOORD 5           /* normal place */
#define CLIP_RCOORD 6           /* GR_VERTEX_OOZ_OFFSET */
#define CLIP_ACOORD 7           /* normal place */


/* Should have size == 16 * sizeof(float).
 */
typedef struct
{
    GLfloat f[15];              /* Same layout as GrVertex */
    GLubyte mask;               /* Unsued  */
    GLubyte usermask;           /* Unused */
}
fxVertex;


#ifdef __i386__
#define FXCOLOR4( c )  (* (int *)c)
#else
#define FXCOLOR4( c ) (      \
  ( ((unsigned int)(c[3]))<<24 ) | \
  ( ((unsigned int)(c[2]))<<16 ) | \
  ( ((unsigned int)(c[1]))<<8 )  | \
  (  (unsigned int)(c[0])) )
#endif


#define FX_VB_COLOR(fxm, color)				\
  do {							\
    if (sizeof(GLint) == 4*sizeof(GLubyte)) {		\
      if (fxm->constColor != *(GLuint*)color) {		\
	fxm->constColor = *(GLuint*)color;		\
	FX_grConstantColorValue(fxm, FXCOLOR4(color));	\
      }							\
    } else {						\
      FX_grConstantColorValue(fxm, FXCOLOR4(color));	\
    }							\
  } while (0)

#define GOURAUD(x) {					\
  GLubyte *col = VB->ColorPtr->data[(x)];		\
  gWin[(x)].v.r=UBYTE_COLOR_TO_FLOAT_255_COLOR(col[0]);	\
  gWin[(x)].v.g=UBYTE_COLOR_TO_FLOAT_255_COLOR(col[1]);	\
  gWin[(x)].v.b=UBYTE_COLOR_TO_FLOAT_255_COLOR(col[2]);	\
  gWin[(x)].v.a=UBYTE_COLOR_TO_FLOAT_255_COLOR(col[3]);	\
}

#if FX_USE_PARGB
#define GOURAUD2(v, c) {			                \
  GLubyte *col = c;     			                \
  PACK_4F_ARGB(GET_PARGB(v), col[3], col[0], col[1], col[2]);   \
}
#else
#define GOURAUD2(v, c) {			\
  GLubyte *col = c;  				\
  v->r=UBYTE_COLOR_TO_FLOAT_255_COLOR(col[0]);	\
  v->g=UBYTE_COLOR_TO_FLOAT_255_COLOR(col[1]);	\
  v->b=UBYTE_COLOR_TO_FLOAT_255_COLOR(col[2]);	\
  v->a=UBYTE_COLOR_TO_FLOAT_255_COLOR(col[3]);	\
}
#endif /* FX_USE_PARGB */


/* Mergable items first
 */
#define SETUP_RGBA 0x1
#define SETUP_TMU0 0x2
#define SETUP_TMU1 0x4
#define SETUP_XY   0x8
#define SETUP_Z    0x10
#define SETUP_W    0x20

#define MAX_MERGABLE 0x8


#define FX_NUM_TMU 2

#define FX_TMU0      GR_TMU0
#define FX_TMU1      GR_TMU1
#define FX_TMU_SPLIT 98
#define FX_TMU_BOTH  99
#define FX_TMU_NONE  100

/* Used for fxMesa->lastUnitsMode */

#define FX_UM_NONE                  0x00000000

#define FX_UM_E0_REPLACE            0x00000001
#define FX_UM_E0_MODULATE           0x00000002
#define FX_UM_E0_DECAL              0x00000004
#define FX_UM_E0_BLEND              0x00000008
#define FX_UM_E0_ADD		    0x00000010

#define FX_UM_E1_REPLACE            0x00000020
#define FX_UM_E1_MODULATE           0x00000040
#define FX_UM_E1_DECAL              0x00000080
#define FX_UM_E1_BLEND              0x00000100
#define FX_UM_E1_ADD		    0x00000200

#define FX_UM_E_ENVMODE             0x000003ff

#define FX_UM_E0_ALPHA              0x00001000
#define FX_UM_E0_LUMINANCE          0x00002000
#define FX_UM_E0_LUMINANCE_ALPHA    0x00004000
#define FX_UM_E0_INTENSITY          0x00008000
#define FX_UM_E0_RGB                0x00010000
#define FX_UM_E0_RGBA               0x00020000

#define FX_UM_E1_ALPHA              0x00040000
#define FX_UM_E1_LUMINANCE          0x00080000
#define FX_UM_E1_LUMINANCE_ALPHA    0x00100000
#define FX_UM_E1_INTENSITY          0x00200000
#define FX_UM_E1_RGB                0x00400000
#define FX_UM_E1_RGBA               0x00800000

#define FX_UM_E_IFMT                0x00fff000

#define FX_UM_COLOR_ITERATED        0x01000000
#define FX_UM_COLOR_CONSTANT        0x02000000
#define FX_UM_ALPHA_ITERATED        0x04000000
#define FX_UM_ALPHA_CONSTANT        0x08000000


#define PACK_BGRA32(R, G, B, A)  \
    ( (((GLuint) (R)) << 16) | \
      (((GLuint) (G)) <<  8) | \
      (((GLuint) (B))      ) | \
      (((GLuint) (A)) << 24) )

#define PACK_RGBA32(R, G, B, A)  \
    ( (((GLuint) (R))      ) | \
      (((GLuint) (G)) <<  8) | \
      (((GLuint) (B)) << 16) | \
      (((GLuint) (A)) << 24) )

/*
 * The first two macros are to pack 8 bit color
 * channel values into a 565 format.
 */
#define PACK_RGB16(R, G, B)         \
    ((((GLuint) (R) & 0xF8) << 8) | \
     (((GLuint) (G) & 0xFC) << 3) | \
      (((GLuint) (B) & 0xFF)         >> 3))
#define PACK_BGR16(R, G, B)         \
    ((((GLuint) (B) & 0xF8) << 8) | \
     (((GLuint) (G) & 0xFC) << 3) | \
     (((GLuint) (R) & 0xFF) >> 3))
/*
 * The second two macros pack 8 bit color channel values
 * into 1555 values.
 */
#define PACK_RGBA16(R, G, B, A)       \
    (((((GLuint) (A) & 0xFF) > 0) << 15)| \
     (((GLuint) (R)  & 0xF8)      << 7) | \
     (((GLuint) (G)  & 0xF8)      << 2) | \
     (((GLuint) (B)  & 0xF8)      >> 3))
#define PACK_BGRA16(R, G, B, A) \
    (((((GLuint) (A) & 0xFF) > 0) << 15)| \
      (((GLuint) (B) & 0xF8)     << 7)  | \
      (((GLuint) (G) & 0xF8)     << 2)  | \
      (((GLuint) (R) & 0xF8)     >> 3))

typedef void (*tfxRenderVBFunc) (GLcontext *);

/*
  Memory range from startAddr to endAddr-1
*/
typedef struct MemRange_t
{
    struct MemRange_t *next;
    FxU32 startAddr, endAddr;
}
MemRange;


typedef struct
{
    GLsizei width, height;              /* image size */
    GLint texelSize;                    /* How many bytes to a texel */
    GrTextureFormat_t glideFormat;      /* Glide image format */
    unsigned short *data;               /* Glide-formated texture image */
    FxU32           dataSize;           /* Count of the data size */
}
tfxMipMapLevel;


typedef struct tfxTexInfo_t
{
    struct tfxTexInfo *next;
    struct gl_texture_object *tObj;

    GLuint lastTimeUsed;
    FxU32 whichTMU;
    GLboolean isInTM;

    GrAspectRatio_t aspectRatio;
    tfxMipMapLevel mipmapLevel[MAX_TEXTURE_LEVELS];

    MemRange *tm[FX_NUM_TMU];

    GLint minLevel, maxLevel;
    GLint baseLevelInternalFormat;

    GrTexInfo info;

    GrTextureFilterMode_t minFilt;
    GrTextureFilterMode_t maxFilt;
    FxBool LODblend;

    GrTextureClampMode_t sClamp;
    GrTextureClampMode_t tClamp;

    GrMipMapMode_t mmMode;

    GLfloat sScale, tScale;
    GLint int_sScale, int_tScale; /* x86 floating point trick for
                                   * multiplication by powers of 2.  
                                   * Used in fxfasttmp.h
                                   */
    GuTexPalette palette;

    GLboolean fixedPalette;
    GLboolean validated;
}
tfxTexInfo;


typedef struct
{
    GLuint swapBuffer;
    GLuint reqTexUpload;
    GLuint texUpload;
    GLuint memTexUpload;
    GLuint texSwaps;
}
tfxStats;


typedef void (*tfxTriViewClipFunc) (struct vertex_buffer * VB,
                                    GLuint v[], GLubyte mask);

typedef void (*tfxTriClipFunc) (struct vertex_buffer * VB,
                                GLuint v[], GLuint mask);


typedef void (*tfxLineClipFunc) (struct vertex_buffer * VB,
                                 GLuint v1, GLuint v2, GLubyte mask);


extern tfxTriViewClipFunc fxTriViewClipTab[0x8];
extern tfxTriClipFunc fxTriClipStrideTab[0x8];
extern tfxLineClipFunc fxLineClipTab[0x8];

typedef struct
{
    /* Alpha test */
    GLboolean alphaTestEnabled;
    GrCmpFnc_t alphaTestFunc;
    GrAlpha_t alphaTestRefValue;

    /* Blend function */
    GLboolean blendEnabled;
    GrAlphaBlendFnc_t blendSrcFuncRGB;
    GrAlphaBlendFnc_t blendDstFuncRGB;
    GrAlphaBlendFnc_t blendSrcFuncAlpha;
    GrAlphaBlendFnc_t blendDstFuncAlpha;
}
tfxUnitsState;



/* Flags for render_index.
 */
#define FX_OFFSET             0x1
#define FX_TWOSIDE            0x2
#define FX_FRONT_BACK         0x4
#define FX_FLAT               0x8
#define FX_ANTIALIAS          0x10
#define FX_FALLBACK           0x20


/* Flags for fxMesa->new_state
 */
#define FX_NEW_TEXTURING      0x1
#define FX_NEW_BLEND          0x2
#define FX_NEW_ALPHA          0x4
#define FX_NEW_DEPTH          0x8
#define FX_NEW_FOG            0x10
#define FX_NEW_SCISSOR        0x20
#define FX_NEW_COLOR_MASK     0x40
#define FX_NEW_CULL           0x80
#define FX_NEW_STENCIL        0x100

/* FX struct stored in VB->driver_data.
 */
struct tfxMesaVertexBuffer
{
    GLuint size;                   /* Number of vertexes */
    GLvector1ui clipped_elements;  /* Array [size] of GLuints */
    fxVertex *verts;               /* Array of [size] fxVertex */
    fxVertex *last_vert;           /* Points into verts array */
#if defined(FX_GLIDE3)
    GrVertex **triangle_b;         /* Triangle buffer */
    GrVertex **strips_b;           /* Strips buffer */
#endif
};

#define FX_DRIVER_DATA(vb) ((struct tfxMesaVertexBuffer *)((vb)->driver_data))
#define FX_CONTEXT(ctx) ((fxMesaContext)((ctx)->DriverCtx))
#define FX_TEXTURE_DATA(t) fxTMGetTexInfo((t)->Current)

#if !defined(FX_PXCONV_TABULAR) \
    && !defined(FX_PXCONV_APPROXIMATION) \
    && !defined(FX_PXCONV_EXACT)
#define	FX_PXCONV_TABULAR
#endif
/* These lookup table are used to extract RGB values in [0,255] from
 * 16-bit pixel values.
 *
 * In general, we want to convert 5 or 6 bit numbers to 8
 * bit numbers.
 * o In the FX_PXCONV_TABULAR case, we do the numerically
 *   correct calculation at initialization time, and store
 *   the results in three large tables.
 * o In the FX_PXCONV_APPROXIMATION method we approximate
 *   the numerically correct value by using the upper bits
 *   of the 5 or 6 bit value.  That is,
 *      8bitvalue = 5bitvalue << 3 | (5bitvalue >> 2)
 * o In the FX_PXCONV_EXACT method, we calculate the
 *   exact value at runtime every time, using a floating
 *   point calculation.
 */
#define FX_PXCONV_INT_FIELD(v, w, s) (((v) >> (s)) & ((1 << (w)) - 1))
#if	defined(FX_PXCONV_TABULAR)
/* These lookup table are used to extract RGB values in [0,255] from
 * 16-bit pixel values.
 */
extern GLubyte FX_PixelToRArray[0x10000];
extern GLubyte FX_PixelToGArray[0x10000];
extern GLubyte FX_PixelToBArray[0x10000];
#define FX_PixelToB(fxMesa, v) (FX_PixelToBArray[(v)])
#define FX_PixelToR(fxMesa, v) (FX_PixelToRArray[(v)])
#define FX_PixelToG(fxMesa, v) (FX_PixelToGArray[(v)])
#elif	defined(FX_PXCONV_APPROXIMATION)
#define FX_PixelToR(fxMesa, v) \
    ((fxMesa)->bgrOrder \
     ? (FX_PXCONV_INT_FIELD(v, 5,  0) << 3) | FX_PXCONV_INT_FIELD(v, 3,  2) \
     : (FX_PXCONV_INT_FIELD(v, 5, 11) << 3) | FX_PXCONV_INT_FIELD(v, 3, 13))
#define FX_PixelToG(fxMesa, v) \
     ((FX_PXCONV_INT_FIELD(v, 6, 5) << 2) | FX_PXCONV_INT_FIELD(v, 2, 7))
#define FX_PixelToB(fxMesa, v) \
    ((fxMesa)->bgrOrder \
     ? (FX_PXCONV_INT_FIELD(v, 5, 11) << 3) | FX_PXCONV_INT_FIELD(v, 3, 13)\
     : (FX_PXCONV_INT_FIELD(v, 5,  0) << 3) | FX_PXCONV_INT_FIELD(v, 3,  2))
#elif	defined(FX_PXCONV_EXACT)
#define FX_PixelToR(fxMesa, v) \
    ((((fxMesa)->bgrOrder \
        ? FX_PXCONV_INT_FIELD(v, 5,  0) \
        : FX_PXCONV_INT_FIELD(v, 5, 11)) * 8 * 255) / 0xF8)
#define FX_PixelToG(fxMesa, v) \
     ((FX_PXCONV_INT_FIELD(v, 6, 5) * 4 * 255) / 0xFC)
#define FX_PixelToB(fxMesa, v) \
    ((((fxMesa)->bgrOrder \
        ? FX_PXCONV_INT_FIELD(v, 5, 11) \
        : FX_PXCONV_INT_FIELD(v, 5,  0)) * 8 * 255) / 0xF8)
#else
#error	Need to define pixel a conversion method.
#endif


/*
 * This is state which may be shared by several tdfx contexts.
 * It hangs off of Mesa's gl_shared_state object (ctx->Shared->DriverData).
 */
struct TdfxSharedState
{
    GLboolean umaTexMemory;
    GLuint totalTexMem[FX_NUM_TMU]; /* constant */
    GLuint freeTexMem[FX_NUM_TMU]; /* changes as we go */
    MemRange *tmPool;
    MemRange *tmFree[FX_NUM_TMU];
};


/*
 * This is the tdfx context struct.
 */
struct tfxMesaContext
{
    /*
     * Set once and never changed:
     */
    GLcontext *glCtx;           /* the core Mesa context */
    GLvisual *glVis;            /* describes the color buffer */

    GLboolean initDone;         /* has this context been initialized? */
    GLint board;                /* the board used for this context */
    int screen_width;
    int screen_height;

    FX_GrContext_t glideContext; /* returned by grSstWinOpen() */
    void *state;                 /* Glide state buffer */

    GLint textureAlign;
    GLboolean bgrOrder;
    GLboolean verbose;
    GLboolean haveTwoTMUs;      /* True if we really have 2 tmu's  */
    GLboolean emulateTwoTMUs;   /* True if we present 2 tmu's to mesa.  */
    GLboolean haveHwStencil;
    GLboolean isNapalm;
    GLint swapInterval;
    GLint maxPendingSwapBuffers;

    /* stuff added for DRI */
    __DRIcontextPrivate *driContextPriv;
    drmContext hHWContext;
    int numClipRects;
    XF86DRIClipRectPtr pClipRects;
    tdfxScreenPrivate *tdfxScrnPriv;

    /*
     * Changes during execution:
     */
    int width, height;   /* size of window */
    int x_offset;        /* distance from window left to screen left */
    int y_offset;        /* distance from window top to screen top */
    int y_delta;         /* distance from window bottom to screen bottom */
    int needClip;        /* need to loop over cliprects? */
    int clipMinX;        /* if !needClip, bounds of the single clip rect */
    int clipMaxX;        /* "" */
    int clipMinY;        /* "" */
    int clipMaxY;        /* "" */


    GrBuffer_t currentFB;  /* front buffer or back buffer */

    GLuint depthClear;     /* glClear depth value */
    GrColor_t clearC;      /* glClear color value */
    GrAlpha_t clearA;      /* glClear alpha value */
    GLuint constColor;
    GrColor_t color;
    GrCullMode_t cullMode;

    tfxUnitsState unitsState;
    tfxUnitsState restoreUnitsState; /* saved during multipass */

    GuTexPalette glbPalette;         /* global texture palette */

    GLuint tmu_source[FX_NUM_TMU];
    GLuint tex_dest[MAX_TEXTURE_UNITS];
    GLuint setupindex;
    GLuint partial_setup_index;
    GLuint setupdone;
    GLuint mergeindex;
    GLuint mergeinputs;
    GLuint render_index;
    GLuint last_tri_caps;
    GLuint stw_hint_state;      /* for grHints */
    GLuint is_in_hardware;
    GLuint new_state;
    GLuint using_fast_path, passes, multipass;
    GLuint texBindNumber;
    GLint tmuSrc;

    tfxLineClipFunc clip_line;
    tfxTriClipFunc clip_tri_stride;
    tfxTriViewClipFunc view_clip_tri;

    GLenum fogTableMode;
    GLfloat fogDensity;
    GLfloat fogStart, fogEnd;
    GrFog_t *fogTable;

    /* Acc. functions */

    points_func PointsFunc;
    line_func LineFunc;
    triangle_func TriangleFunc;
    quad_func QuadFunc;

    render_func **RenderVBTables;
    render_func *RenderVBClippedTab;
    render_func *RenderVBCulledTab;
    render_func *RenderVBRawTab;

    tfxStats stats;
};


typedef void (*tfxSetupFunc) (struct vertex_buffer *, GLuint, GLuint);


extern void fxPrintSetupFlags(const char *msg, GLuint flags);
extern void fxSetupDDPointers(GLcontext *);

extern void fxDDSetupInit(void);
extern void fxDDCvaInit(void);
extern void fxDDTrifuncInit(void);
extern void fxDDFastPathInit(void);

extern void fxDDRenderInitGlide3(GLcontext * ctx);

extern void fxDDChooseRenderState(GLcontext * ctx);

extern void fxRenderClippedLine(struct vertex_buffer *VB,
                                GLuint v1, GLuint v2);

extern void fxRenderClippedTriangle(struct vertex_buffer *VB,
                                    GLuint n, GLuint vlist[]);


extern tfxSetupFunc fxDDChooseSetupFunction(GLcontext *);

extern points_func fxDDChoosePointsFunction(GLcontext *);
extern line_func fxDDChooseLineFunction(GLcontext *);
extern triangle_func fxDDChooseTriangleFunction(GLcontext *);
extern quad_func fxDDChooseQuadFunction(GLcontext *);
extern render_func **fxDDChooseRenderVBTables(GLcontext *);

extern void fxDDRenderInit(GLcontext *);
extern void fxDDClipInit(void);

extern void fxSetupDDSpanPointers(GLcontext *);


extern void fxDDRegisterVB(struct vertex_buffer *VB);
extern void fxDDUnregisterVB(struct vertex_buffer *VB);
extern void fxDDResizeVB(struct vertex_buffer *VB, GLuint size);

extern void fxDDMergeAndRender(struct vertex_buffer *VB);

extern void fxDDCheckPartialRasterSetup(GLcontext * ctx,
                                        struct gl_pipeline_stage *d);

extern void fxDDPartialRasterSetup(struct vertex_buffer *VB);

extern void fxDDDoRasterSetup(struct vertex_buffer *VB);

extern void fxDDRenderElementsDirect(struct vertex_buffer *VB);
extern void fxDDRenderVBIndirectDirect(struct vertex_buffer *VB);

extern void fxDDFastPath(struct vertex_buffer *VB);

extern void fxPrintRenderState(const char *msg, GLuint state);
extern void fxPrintHintState(const char *msg, GLuint state);

extern void fxDDDoRenderVB(struct vertex_buffer *VB);

extern int fxDDInitFxMesaContext(fxMesaContext fxMesa);


extern void fxSetScissorValues(GLcontext * ctx);
extern void fxTMMoveInTM_NoLock(fxMesaContext fxMesa,
                                struct gl_texture_object *tObj, FxU32 where);
extern void fxInitPixelTables(fxMesaContext fxMesa, GLboolean bgrOrder);



extern GLboolean tdfxMapAllRegions(__DRIscreenPrivate * driScrnPriv);
extern void tdfxUnmapAllRegions(__DRIscreenPrivate * driScrnPriv);
extern GLboolean tdfxInitHW(__DRIdrawablePrivate * driDrawPrivate,
                            fxMesaContext cPriv);

extern void XMesaUpdateState(fxMesaContext fxMesa);


/* This is the private interface between Glide and DRI */
#if 0
extern void grDRIOpen(char *pFB, tdfxScreenPrivate *sPriv,
                      volatile int *fifoPtr, volatile int *fifoRead);
#endif
extern void grDRIPosition(int x, int y, int w, int h,
                          int numClip, XF86DRIClipRectPtr pClip);
extern void grDRILostContext(void);
extern void grDRIImportFifo(int fifoPtr, int fifoRead);
extern void grDRIInvalidateAll(void);
extern void grDRIResetSAREA(void);
extern void grDRIBufferSwap(FxU32 swapInterval);
extern void grDRISwapClipRects(FxU32 swapInterval,
                               int numClip,
                               const XF86DRIClipRectPtr pClip);



/* You can turn this on to find locking conflicts.
#define DEBUG_LOCKING 
*/

#ifdef DEBUG_LOCKING
extern char *prevLockFile;
extern int prevLockLine;
#define DEBUG_LOCK() \
  do { \
    prevLockFile=(__FILE__); \
    prevLockLine=(__LINE__); \
  } while (0)
#define DEBUG_RESET() \
  do { \
    prevLockFile=0; \
    prevLockLine=0; \
  } while (0)
#define DEBUG_CHECK_LOCK() \
  do { \
    if (prevLockFile) { \
      fprintf(stderr, "LOCK SET!\n\tPrevious %s:%d\n\tCurrent: %s:%d\n", \
	prevLockFile, prevLockLine, __FILE__, __LINE__); \
      exit(1); \
    } \
  } while (0)
#else
#define DEBUG_LOCK()
#define DEBUG_RESET()
#define DEBUG_CHECK_LOCK()
#endif /* DEBUG_LOCKING */


/* !!! We may want to separate locks from locks with validation.
   This could be used to improve performance for those things
   commands that do not do any drawing !!! */

#define DRM_LIGHT_LOCK_RETURN(fd,lock,context,__ret)                   \
	do {                                                           \
		DRM_CAS(lock,context,DRM_LOCK_HELD|context,__ret);     \
                if (__ret) drmGetLock(fd,context,0);                   \
        } while(0)

#define LOCK_HARDWARE(fxMesa) XMesaUpdateState(fxMesa)

/* Unlock the hardware using the global current context */
#define UNLOCK_HARDWARE(fxMesa)					\
  do {								\
    __DRIcontextPrivate *cPriv = fxMesa->driContextPriv;	\
    __DRIdrawablePrivate *dPriv = cPriv->driDrawablePriv;	\
    __DRIscreenPrivate *sPriv = dPriv->driScreenPriv;		\
    DRM_UNLOCK(sPriv->fd, &sPriv->pSAREA->lock,			\
	       dPriv->driContextPriv->hHWContext);		\
    DEBUG_RESET();						\
  } while (0)

#define BEGIN_BOARD_LOCK(fxMesa) LOCK_HARDWARE(fxMesa)
#define END_BOARD_LOCK(fxMesa) UNLOCK_HARDWARE(fxMesa)

/*
  This pair of macros makes a loop over the drawing operations
  so it is not self contained and doesn't have the nice single 
  statement semantics of most macros
*/
#define BEGIN_CLIP_LOOP(fxMesa)					\
  do {								\
    __DRIcontextPrivate *cPriv = fxMesa->driContextPriv;	\
    __DRIdrawablePrivate *dPriv = cPriv->driDrawablePriv;	\
    int _nc;							\
    LOCK_HARDWARE(fxMesa);					\
    _nc = dPriv->numClipRects;					\
    while (_nc--) {						\
      if (fxMesa->needClip) {					\
        fxMesa->clipMinX = dPriv->pClipRects[_nc].x1;		\
        fxMesa->clipMaxX = dPriv->pClipRects[_nc].x2;		\
        fxMesa->clipMinY = dPriv->pClipRects[_nc].y1;		\
        fxMesa->clipMaxY = dPriv->pClipRects[_nc].y2;		\
        fxSetScissorValues(fxMesa->glCtx);			\
      }

#define END_CLIP_LOOP(fxMesa)	\
    }				\
    UNLOCK_HARDWARE(fxMesa);	\
  } while (0)


#endif /* GLX_DIRECT_RENDERING */

#endif /* _FXDRV_H_ */
