/* $XFree86: xc/lib/GL/mesa/src/drv/gamma/gamma_init.h,v 1.3 2000/06/17 00:02:56 martin Exp $ */
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
 *   Kevin E. Martin <kevin@precisioninsight.com>
 *
 */

#ifndef _GAMMA_INIT_H_
#define _GAMMA_INIT_H_

#ifdef GLX_DIRECT_RENDERING

#include "dri_tmm.h"
#include "dri_mesaint.h"
#include "gamma_region.h"
#include "gamma_regs.h"
#include "gamma_macros.h"
#include "gamma_texture.h"

typedef struct {
    int           regionCount;       /* Count of register regions */
    gammaRegion  *regions;           /* Vector of mapped region info */
    drmBufMapPtr  bufs;              /* Map of DMA buffers */
    __DRIscreenPrivate *driScrnPriv; /* Back pointer to DRI screen */
} gammaScreenPrivate;

typedef union {
    unsigned int i;
    float        f;
} dmaBufRec, *dmaBuf;

/* Flags for context */
#define GAMMA_FRONT_BUFFER    0x00000001
#define GAMMA_BACK_BUFFER     0x00000002
#define GAMMA_DEPTH_BUFFER    0x00000004
#define GAMMA_STENCIL_BUFFER  0x00000008
#define GAMMA_ACCUM_BUFFER    0x00000010

/* These are the minimum requirements and should probably be increased */
#define MAX_MODELVIEW_STACK    16
#define MAX_PROJECTION_STACK    2
#define MAX_TEXTURE_STACK       2

typedef struct {
    drmContext          hHWContext;

    dmaBuf              buf;           /* DMA buffer for regular cmds */
    int                 bufIndex;
    int                 bufSize;
    int                 bufCount;

    dmaBuf              WCbuf;         /* DMA buffer for window changed cmds */
    int                 WCbufIndex;
    int                 WCbufSize;
    int                 WCbufCount;

    gammaScreenPrivate *gammaScrnPriv;

    int                 x, y, w, h; /* Probably should be in drawable priv */
    int                 FrameCount; /* Probably should be in drawable priv */
    int                 NotClipped; /* Probably should be in drawable priv */
    int                 WindowChanged; /* Probably should be in drawabl... */
    int                 Flags;
    int                 EnabledFlags;
    int                 DepthSize;
    int                 Begin;

    float               ClearColor[4];
    float               ClearDepth;
    int                 MatrixMode;
    int                 DepthMode;
    float               zNear, zFar;
    int                 LBReadMode;
    int                 FBReadMode;
    int                 FBWindowBase;
    int                 LBWindowBase;
    int                 ColorDDAMode;
    int                 GeometryMode;
    int                 AlphaTestMode;
    int                 AlphaBlendMode;
    int                 AB_FBReadMode;
    int                 AB_FBReadMode_Save;
    int                 DeltaMode;
    int			ColorMaterialMode;
    int			MaterialMode;
    int			LightingMode;
    int			Light0Mode;
    int			Light1Mode;
    int			ScissorMode;
    int                 Window; /* GID part probably should be in draw priv */

    gammaTexObj        *curTexObj;
    gammaTexObj        *curTexObj1D;
    gammaTexObj        *curTexObj2D;
    int                 Texture1DEnabled;
    int                 Texture2DEnabled;

    driTMMPtr           tmm;

    float               ModelView[16];
    float               Proj[16];
    float               ModelViewProj[16];
    float               Texture[16];

    float               ModelViewStack[(MAX_MODELVIEW_STACK-1)*16];
    int                 ModelViewCount;
    float               ProjStack[(MAX_PROJECTION_STACK-1)*16];
    int                 ProjCount;
    float               TextureStack[(MAX_TEXTURE_STACK-1)*16];
    int                 TextureCount;
} gammaContextPrivate;

extern GLboolean  gammaMapAllRegions(__DRIscreenPrivate *driScrnPriv);
extern void       gammaUnmapAllRegions(__DRIscreenPrivate *driScrnPriv);
extern void       gammaSetMatrix(GLfloat m[16]);
extern void       gammaMultMatrix(GLfloat m[16]);
extern void       gammaLoadHWMatrix(void);
extern void       gammaInitHW(gammaContextPrivate *gcp);

extern float                IdentityMatrix[16];
extern __DRIcontextPrivate *nullCC;
extern __DRIcontextPrivate *gCC;
extern gammaContextPrivate *gCCPriv;

#endif

#endif /* _GAMMA_INIT_H_ */
