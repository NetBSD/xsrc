/* $XFree86: xc/lib/GL/mesa/src/drv/gamma/gamma_init.h,v 1.4 2001/02/07 13:26:16 alanh Exp $ */
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

#define GAMMA_MAX_TEXTURE_SIZE    2048

/* These are the minimum requirements and should probably be increased */
#define MAX_MODELVIEW_STACK    16
#define MAX_PROJECTION_STACK    2
#define MAX_TEXTURE_STACK       2

struct gamma_current_attrib {
/*   GLubyte ByteColor[4];	old */		/* Current RGBA color */
   GLuint Index;				/* Current color index */
   GLfloat Color[4];                            /* Current RGBA color */ 
   GLfloat Normal[3];				/* Current normal vector */
   GLfloat TexCoord[4];				/* points into MultiTexCoord */
   GLfloat RasterPos[4];			/* Current raster position */
   GLfloat RasterDistance;			/* Current raster distance */
   GLfloat RasterColor[4];			/* Current raster color */
   GLuint RasterIndex;				/* Current raster index */
   GLfloat *RasterTexCoord;			/* Current raster texcoord*/
   GLboolean RasterPosValid;			/* Raster po valid flag */
   GLboolean EdgeFlag;				/* Current edge flag */
};

struct gamma_array_attrib {
	GLint VertexSize;
	GLenum VertexType;
	GLsizei VertexStride;		/* user-specified stride */
	GLsizei VertexStrideB;		/* actual stride in bytes */
	void *VertexPtr;
	GLboolean VertexEnabled;

	GLenum NormalType;
	GLsizei NormalStride;		/* user-specified stride */
	GLsizei NormalStrideB;		/* actual stride in bytes */
	void *NormalPtr;
	GLboolean NormalEnabled;

	GLint ColorSize;
	GLenum ColorType;
	GLsizei ColorStride;		/* user-specified stride */
	GLsizei ColorStrideB;		/* actual stride in bytes */
	void *ColorPtr;
	GLboolean ColorEnabled;

	GLenum IndexType;
	GLsizei IndexStride;		/* user-specified stride */
	GLsizei IndexStrideB;		/* actual stride in bytes */
	void *IndexPtr;
	GLboolean IndexEnabled;

	GLsizei EdgeFlagStride;		/* user-specified stride */
	GLsizei EdgeFlagStrideB;	/* actual stride in bytes */
	GLboolean *EdgeFlagPtr;
	GLboolean EdgeFlagEnabled;

	GLint TexCoordSize;
	GLenum TexCoordType;
	GLsizei TexCoordStride;	/* user-specified stride */
	GLsizei TexCoordStrideB;	/* actual stride in bytes */
	void *TexCoordPtr;
	GLboolean TexCoordEnabled;
	GLint TexCoordInterleaveFactor;
};

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
    GLenum              ErrorValue;

    struct _glapi_table	*Exec;
    struct _glapi_table	*Save;
    struct _glapi_table	*API;

    struct _mesa_HashTable      *DisplayList;

    struct gl_list_attrib	List;
    struct gl_pixelstore_attrib Unpack; /* Pixel unpacking */

    struct gamma_array_attrib      Array; /* From Mesa v3.0 */
    struct gamma_current_attrib    Current; /* From Mesa v3.0 */

    /* Display lists */
    GLuint CallDepth;		/* Current recursion calling depth */
    GLboolean ExecuteFlag;	/* Execute GL commands? */
    GLboolean CompileFlag;	/* Compile GL commands into display list? */
    Node *CurrentListPtr;	/* Head of list being compiled */
    GLuint CurrentListNum;	/* Number of the list being compiled */
    Node *CurrentBlock;		/* Pointer to current block of nodes */
    GLuint CurrentPos;		/* Index into current block of nodes */

    float               ClearColor[4];
    float               ClearDepth;
    int                 MatrixMode;
    int                 DepthMode;
    int			TransformMode;
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
    int			NormalizeMode;
    int			LightingMode;
    int			Light0Mode;
    int			Light1Mode;
    int			Light2Mode;
    int			Light3Mode;
    int			Light4Mode;
    int			Light5Mode;
    int			Light6Mode;
    int			Light7Mode;
    int			Light8Mode;
    int			Light9Mode;
    int			Light10Mode;
    int			Light11Mode;
    int			Light12Mode;
    int			Light13Mode;
    int			Light14Mode;
    int			Light15Mode;
    int			LogicalOpMode;
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
