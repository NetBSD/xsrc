/* $XFree86: xc/lib/GL/mesa/src/drv/gamma/gamma_gl.c,v 1.10 2001/08/18 02:51:04 dawes Exp $ */
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
 *   Brian Paul <brian@precisioninsight.com>
 *   Alan Hourihane <alanh@fairlite.demon.co.uk>
 */

#ifdef GLX_DIRECT_RENDERING

#include <Xarch.h>
#include <math.h>
#include "gamma_gl.h"
#include "gamma_init.h"
#include "gamma_dlist.h"
#include "glint_dri.h"

/* Here for now, will change get.c to move them into macros.h later */

#define FLOAT_TO_BOOL(X)        ( (X)==0.0F ? GL_FALSE : GL_TRUE )
#define INT_TO_BOOL(I)          ( (I)==0 ? GL_FALSE : GL_TRUE )
#define ENUM_TO_BOOL(E)         ( (E)==0 ? GL_FALSE : GL_TRUE )

void gamma_error( GLenum error, const char *s )
{
   GLboolean debug;

#ifdef DEBUG
   debug = GL_TRUE;
#else
   if (getenv("MESA_DEBUG")) {
      debug = GL_TRUE;
   }
   else {
      debug = GL_FALSE;
   }
#endif

   if (debug) {
      const char *errstr;

      switch (error) {
	 case GL_NO_ERROR:
	    errstr = "GL_NO_ERROR";
	    break;
	 case GL_INVALID_VALUE:
	    errstr = "GL_INVALID_VALUE";
	    break;
	 case GL_INVALID_ENUM:
	    errstr = "GL_INVALID_ENUM";
	    break;
	 case GL_INVALID_OPERATION:
	    errstr = "GL_INVALID_OPERATION";
	    break;
	 case GL_STACK_OVERFLOW:
	    errstr = "GL_STACK_OVERFLOW";
	    break;
	 case GL_STACK_UNDERFLOW:
	    errstr = "GL_STACK_UNDERFLOW";
	    break;
	 case GL_OUT_OF_MEMORY:
	    errstr = "GL_OUT_OF_MEMORY";
	    break;
	 default:
	    errstr = "unknown";
	    break;
      }
      fprintf( stderr, "Mesa user error: %s in %s\n", errstr, s );
   }

   if (gCCPriv->ErrorValue==GL_NO_ERROR) {
      gCCPriv->ErrorValue = error;
   }

   /* _gamma_Error( );  use hwLog or something later */
}


void _gamma_Accum(GLenum op, GLfloat value)
{
    DEBUG_GLCMDS(("Accum: %d %f\n", (int)op, value));

    switch (op) {
    case GL_ACCUM:
	break;
    case GL_LOAD:
	break;
    case GL_ADD:
	break;
    case GL_MULT:
	break;
    case GL_RETURN:
	break;
    default:
	gCCPriv->ErrorValue = GL_INVALID_ENUM;
	return;
    }
}

void _gamma_AlphaFunc(GLenum func, GLclampf ref)
{
    unsigned char r = ref * 255.0;

    DEBUG_GLCMDS(("AlphaFunc: %d %f\n", (int)func, (float)ref));

    gCCPriv->AlphaTestMode &= ~(AT_CompareMask | AT_RefValueMask);

    switch (func) {
    case GL_NEVER:
	gCCPriv->AlphaTestMode |= AT_Never;
	break;
    case GL_LESS:
	gCCPriv->AlphaTestMode |= AT_Less;
	break;
    case GL_EQUAL:
	gCCPriv->AlphaTestMode |= AT_Equal;
	break;
    case GL_LEQUAL:
	gCCPriv->AlphaTestMode |= AT_LessEqual;
	break;
    case GL_GREATER:
	gCCPriv->AlphaTestMode |= AT_Greater;
	break;
    case GL_NOTEQUAL:
	gCCPriv->AlphaTestMode |= AT_NotEqual;
	break;
    case GL_GEQUAL:
	gCCPriv->AlphaTestMode |= AT_GreaterEqual;
	break;
    case GL_ALWAYS:
	gCCPriv->AlphaTestMode |= AT_Always;
	break;
    default:
	/* ERROR!! */
	break;
    }

    gCCPriv->AlphaTestMode |= r << 4;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, AlphaTestMode, gCCPriv->AlphaTestMode);
}

GLboolean _gamma_AreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences)
{
    DEBUG_GLCMDS(("AreTexturesResident: %d\n", (int)n));
#ifdef DEBUG_VERBOSE_EXTRA
    {
	int t;
	for (t = 0; t < n; t++)
	    printf("\t%d\n", (int)textures[t]);
    }
#endif

    return GL_TRUE;
}

void _gamma_ArrayElement(GLint i)
{
    DEBUG_GLCMDS(("ArrayElement: %d\n", (int)i));
}

void _gamma_Begin(GLenum mode)
{
    DEBUG_GLCMDS(("Begin: %04x\n", (int)mode));

    if ((gCCPriv->Begin & B_PrimType_Mask) != B_PrimType_Null) {
        DEBUG_ERROR(("Begin: Error\n"));
	return;
    }

    gCCPriv->Begin &= ~B_PrimType_Mask;
    switch (mode) {
    case GL_POINTS:
	gCCPriv->Begin |= B_PrimType_Points;
	break;
    case GL_LINES:
	gCCPriv->Begin |= B_PrimType_Lines;
	break;
    case GL_LINE_LOOP:
	gCCPriv->Begin |= B_PrimType_LineLoop;
	break;
    case GL_LINE_STRIP:
	gCCPriv->Begin |= B_PrimType_LineStrip;
	break;
    case GL_TRIANGLES:
	gCCPriv->Begin |= B_PrimType_Triangles;
	break;
    case GL_TRIANGLE_STRIP:
	gCCPriv->Begin |= B_PrimType_TriangleStrip;
	break;
    case GL_TRIANGLE_FAN:
	gCCPriv->Begin |= B_PrimType_TriangleFan;
	break;
    case GL_QUADS:
	gCCPriv->Begin |= B_PrimType_Quads;
	break;
    case GL_QUAD_STRIP:
	gCCPriv->Begin |= B_PrimType_QuadStrip;
	break;
    case GL_POLYGON:
	gCCPriv->Begin |= B_PrimType_Polygon;
	break;
    default:
	/* ERROR!! */
	break;
    }

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, Begin, gCCPriv->Begin);
}

void _gamma_BindTexture(GLenum target, GLuint texture)
{
    unsigned long addrs[MIPMAP_LEVELS];
    int i;

    DEBUG_GLCMDS(("BindTexture: %04x %d\n",
		  (int)target, (unsigned int)texture));

    /* Disable all of the units in the previous bind */
    gCCPriv->curTexObj->TextureAddressMode &= ~TextureAddressModeEnable;
    gCCPriv->curTexObj->TextureReadMode    &= ~TextureReadModeEnable;
    gCCPriv->curTexObj->TextureColorMode   &= ~TextureColorModeEnable;
    gCCPriv->curTexObj->TextureFilterMode  &= ~TextureFilterModeEnable;

    /* Find the texture (create it, if necessary) */
    gCCPriv->curTexObj = gammaTOFind(texture);

    /* Make the new texture images resident */
    if (driTMMMakeImagesResident(gCCPriv->tmm, MIPMAP_LEVELS,
				  gCCPriv->curTexObj->image, addrs) < 0) {
    	DEBUG_ERROR(("BindTexture: unable\n"));
	/* NOT_DONE: Handle error */
    }

    for (i = 0; i < MIPMAP_LEVELS; i++)
	gCCPriv->curTexObj->TextureBaseAddr[i] = addrs[i] << 5;

    /* Set the target */
    gCCPriv->curTexObj->TextureAddressMode &= ~TAM_TexMapType_Mask;
    gCCPriv->curTexObj->TextureReadMode    &= ~TRM_TexMapType_Mask;
    switch (target) {
    case GL_TEXTURE_1D:
	gCCPriv->curTexObj1D = gCCPriv->curTexObj;
	gCCPriv->curTexObj->TextureAddressMode |= TAM_TexMapType_1D;
	gCCPriv->curTexObj->TextureReadMode    |= TRM_TexMapType_1D;
	break;
    case GL_TEXTURE_2D:
	gCCPriv->curTexObj2D = gCCPriv->curTexObj;
	gCCPriv->curTexObj->TextureAddressMode |= TAM_TexMapType_2D;
	gCCPriv->curTexObj->TextureReadMode    |= TRM_TexMapType_2D;
	break;
    default:
	break;
    }

    /* Enable the units if texturing is enabled */
    if (target == GL_TEXTURE_1D && gCCPriv->Texture1DEnabled) {
	gCCPriv->curTexObj->TextureAddressMode |= TextureAddressModeEnable;
	gCCPriv->curTexObj->TextureReadMode    |= TextureReadModeEnable;
	gCCPriv->curTexObj->TextureColorMode   |= TextureColorModeEnable;
	gCCPriv->curTexObj->TextureFilterMode  |= TextureFilterModeEnable;
    } else if (target == GL_TEXTURE_2D && gCCPriv->Texture2DEnabled) {
	gCCPriv->curTexObj->TextureAddressMode |= TextureAddressModeEnable;
	gCCPriv->curTexObj->TextureReadMode    |= TextureReadModeEnable;
	gCCPriv->curTexObj->TextureColorMode   |= TextureColorModeEnable;
	gCCPriv->curTexObj->TextureFilterMode  |= TextureFilterModeEnable;
    }

    /* Restore the units */
    CHECK_DMA_BUFFER(gCC, gCCPriv, 18);
    WRITE(gCCPriv->buf, TextureAddressMode,
	  gCCPriv->curTexObj->TextureAddressMode);
    WRITE(gCCPriv->buf, TextureReadMode,
	  gCCPriv->curTexObj->TextureReadMode);
    WRITE(gCCPriv->buf, TextureColorMode,
	  gCCPriv->curTexObj->TextureColorMode);
    WRITE(gCCPriv->buf, TextureFilterMode,
	  gCCPriv->curTexObj->TextureFilterMode);
    WRITE(gCCPriv->buf, TextureFormat,
	  gCCPriv->curTexObj->TextureFormat);
    WRITE(gCCPriv->buf, TxBaseAddr0,  gCCPriv->curTexObj->TextureBaseAddr[ 0]);
    WRITE(gCCPriv->buf, TxBaseAddr1,  gCCPriv->curTexObj->TextureBaseAddr[ 1]);
    WRITE(gCCPriv->buf, TxBaseAddr2,  gCCPriv->curTexObj->TextureBaseAddr[ 2]);
    WRITE(gCCPriv->buf, TxBaseAddr3,  gCCPriv->curTexObj->TextureBaseAddr[ 3]);
    WRITE(gCCPriv->buf, TxBaseAddr4,  gCCPriv->curTexObj->TextureBaseAddr[ 4]);
    WRITE(gCCPriv->buf, TxBaseAddr5,  gCCPriv->curTexObj->TextureBaseAddr[ 5]);
    WRITE(gCCPriv->buf, TxBaseAddr6,  gCCPriv->curTexObj->TextureBaseAddr[ 6]);
    WRITE(gCCPriv->buf, TxBaseAddr7,  gCCPriv->curTexObj->TextureBaseAddr[ 7]);
    WRITE(gCCPriv->buf, TxBaseAddr8,  gCCPriv->curTexObj->TextureBaseAddr[ 8]);
    WRITE(gCCPriv->buf, TxBaseAddr9,  gCCPriv->curTexObj->TextureBaseAddr[ 9]);
    WRITE(gCCPriv->buf, TxBaseAddr10, gCCPriv->curTexObj->TextureBaseAddr[10]);
    WRITE(gCCPriv->buf, TxBaseAddr11, gCCPriv->curTexObj->TextureBaseAddr[11]);
    WRITE(gCCPriv->buf, TextureCacheControl, (TCC_Enable | TCC_Invalidate));
}

void _gamma_Bitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
    DEBUG_GLCMDS(("Bitmap: \n"));
}

void _gamma_BlendFunc(GLenum sfactor, GLenum dfactor)
{
    DEBUG_GLCMDS(("BlendFunc: %04x %04x\n", (int)sfactor, (int)dfactor));

    gCCPriv->AB_FBReadMode_Save = 0;
    gCCPriv->AlphaBlendMode &= ~(AB_SrcBlendMask | AB_DstBlendMask);

    switch (sfactor) {
    case GL_ZERO:
	gCCPriv->AlphaBlendMode |= AB_Src_Zero;
	gCCPriv->AB_FBReadMode_Save |= (FBReadDstEnable);
	break;
    case GL_ONE:
	gCCPriv->AlphaBlendMode |= AB_Src_One;
	gCCPriv->AB_FBReadMode_Save |= (FBReadDstEnable);
	break;
    case GL_DST_COLOR:
	gCCPriv->AlphaBlendMode |= AB_Src_DstColor;
	gCCPriv->AB_FBReadMode_Save |= (FBReadDstEnable);
	break;
    case GL_ONE_MINUS_DST_COLOR:
	gCCPriv->AlphaBlendMode |= AB_Src_OneMinusDstColor;
	gCCPriv->AB_FBReadMode_Save |= (FBReadDstEnable);
	break;
    case GL_SRC_ALPHA:
	gCCPriv->AlphaBlendMode |= AB_Src_SrcAlpha;
	gCCPriv->AB_FBReadMode_Save |= (FBReadDstEnable);
	break;
    case GL_ONE_MINUS_SRC_ALPHA:
	gCCPriv->AlphaBlendMode |= AB_Src_OneMinusSrcAlpha;
	gCCPriv->AB_FBReadMode_Save |= (FBReadDstEnable);
	break;
    case GL_DST_ALPHA:
	gCCPriv->AlphaBlendMode |= AB_Src_DstAlpha;
	gCCPriv->AB_FBReadMode_Save |= (FBReadSrcEnable | FBReadDstEnable);
	break;
    case GL_ONE_MINUS_DST_ALPHA:
	gCCPriv->AlphaBlendMode |= AB_Src_OneMinusDstAlpha;
	gCCPriv->AB_FBReadMode_Save |= (FBReadSrcEnable | FBReadDstEnable);
	break;
    case GL_SRC_ALPHA_SATURATE:
	gCCPriv->AlphaBlendMode |= AB_Src_SrcAlphaSaturate;
	gCCPriv->AB_FBReadMode_Save |= (FBReadDstEnable);
	break;
    default:
	/* ERROR!! */
	break;
    }

    switch (dfactor) {
    case GL_ZERO:
	gCCPriv->AlphaBlendMode |= AB_Dst_Zero;
	gCCPriv->AB_FBReadMode_Save |= (FBReadDstEnable);
	break;
    case GL_ONE:
	gCCPriv->AlphaBlendMode |= AB_Dst_One;
	gCCPriv->AB_FBReadMode_Save |= (FBReadDstEnable);
	break;
    case GL_SRC_COLOR:
	gCCPriv->AlphaBlendMode |= AB_Dst_SrcColor;
	gCCPriv->AB_FBReadMode_Save |= (FBReadDstEnable);
	break;
    case GL_ONE_MINUS_SRC_COLOR:
	gCCPriv->AlphaBlendMode |= AB_Dst_OneMinusSrcColor;
	gCCPriv->AB_FBReadMode_Save |= (FBReadDstEnable);
	break;
    case GL_SRC_ALPHA:
	gCCPriv->AlphaBlendMode |= AB_Dst_SrcAlpha;
	gCCPriv->AB_FBReadMode_Save |= (FBReadDstEnable);
	break;
    case GL_ONE_MINUS_SRC_ALPHA:
	gCCPriv->AlphaBlendMode |= AB_Dst_OneMinusSrcAlpha;
	gCCPriv->AB_FBReadMode_Save |= (FBReadDstEnable);
	break;
    case GL_DST_ALPHA:
	gCCPriv->AlphaBlendMode |= AB_Dst_DstAlpha;
	gCCPriv->AB_FBReadMode_Save |= (FBReadSrcEnable | FBReadDstEnable);
	break;
    case GL_ONE_MINUS_DST_ALPHA:
	gCCPriv->AlphaBlendMode |= AB_Dst_OneMinusDstAlpha;
	gCCPriv->AB_FBReadMode_Save |= (FBReadSrcEnable | FBReadDstEnable);
	break;
    default:
	/* ERROR!! */
	break;
    }

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, AlphaBlendMode, gCCPriv->AlphaBlendMode);

    if (gCCPriv->AlphaBlendMode & AlphaBlendModeEnable) {
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	gCCPriv->AB_FBReadMode = gCCPriv->AB_FBReadMode_Save;
	WRITE(gCCPriv->buf, FBReadMode, (gCCPriv->FBReadMode |
					 gCCPriv->AB_FBReadMode));
    }
}

#if 0 /* Now in dlist.c */
void _gamma_CallList(GLuint list)
{
    DEBUG_GLCMDS(("CallList: %d\n", (unsigned int)list));
}

void _gamma_CallLists(GLsizei n, GLenum type, const GLvoid *lists)
{
    DEBUG_GLCMDS(("CallLists: %d %04x\n", (int)n, (int)type));
}
#endif

void _gamma_Clear(GLbitfield mask)
{
    int temp;
    unsigned int depth = 0;
    int do_clear = 0;
    GLINTDRIPtr gDRIPriv = (GLINTDRIPtr)gCC->driScreenPriv->pDevPriv;
#ifdef DO_VALIDATE
    __DRIscreenPrivate *driScrnPriv = gCC->driScreenPriv;
#endif

    DEBUG_GLCMDS(("Clear: %04x\n", (int)mask));

#ifdef TURN_OFF_CLEARS
    {
	static int done_first_clear = 0;
	if (done_first_clear)
	    return;
	done_first_clear = 1;
    }
#endif

#ifdef DO_VALIDATE
    /* Flush any partially filled buffers */
    FLUSH_DMA_BUFFER(gCC,gCCPriv);

    DRM_SPINLOCK(&driScrnPriv->pSAREA->drawable_lock,
		 driScrnPriv->drawLockID);
    VALIDATE_DRAWABLE_INFO_NO_LOCK(gCC,gCCPriv);
#endif

    if ((mask & GL_DEPTH_BUFFER_BIT) &&
	(gCCPriv->Flags & GAMMA_DEPTH_BUFFER)) {
	double d = (((double)gCCPriv->ClearDepth-gCCPriv->zNear)/
		    (gCCPriv->zFar-gCCPriv->zNear));

	if (d > 1.0) d = 1.0;
	else if (d < 0.0) d = 0.0;

	switch (gCCPriv->DepthSize) {
	case 16:
	    depth = d * 65535.0; /* 2^16-1 */
	    break;
	case 24:
	    depth = d * 16777215.0; /* 2^24-1 */
	    break;
	case 32:
	    depth = d * 4294967295.0; /* 2^32-1 */
	    break;
	}

#ifdef TURN_OFF_CLEARS
	depth = 0;
#endif

	/* Turn off writes the FB */
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, FBWriteMode, FBWriteModeDisable);

	/*
	 * Turn Rectangle2DControl off when the window is not clipped
	 * (i.e., the GID tests are not necessary).  This dramatically
	 * increases the performance of the depth clears.
	 */
	if (!gCCPriv->NotClipped) {
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITE(gCCPriv->buf, Rectangle2DControl, 1);
	}

	CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	WRITE(gCCPriv->buf, DepthMode, (DepthModeEnable |
					DM_Always |
					DM_SourceDepthRegister |
					DM_WriteMask));
	WRITE(gCCPriv->buf, GLINTDepth, depth);

	/* Increment the frame count */
	gCCPriv->FrameCount++;
#ifdef FAST_CLEAR_4
	gCCPriv->FrameCount &= 0x0f;
#else
	gCCPriv->FrameCount &= 0xff;
#endif

	temp = (gCCPriv->LBReadMode & LBPartialProdMask) | LBWindowOriginBot;
	/* UGH - move this later ! */
	if (gDRIPriv->numMultiDevices == 2) temp |= LBScanLineInt2;

	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, LBReadMode, temp);

	/* Force FCP to be written */
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, GLINTWindow, (WindowEnable |
					  W_PassIfEqual |
					  (gCCPriv->Window & W_GIDMask) |
					  W_DepthFCP |
					  W_LBUpdateFromRegisters |
					  W_OverrideWriteFiltering |
					  (gCCPriv->FrameCount << 9)));

	/* Clear part of the depth and FCP buffers */
	{
	    int y = gCCPriv->y;
	    int h = gCCPriv->h;
#ifndef TURN_OFF_FCP
	    float hsub = h;

	    if (gCCPriv->WindowChanged) {
		gCCPriv->WindowChanged = GL_FALSE;
	    } else {
#ifdef FAST_CLEAR_4
		hsub /= 16;
#else
		hsub /= 256;
#endif

		/* Handle the case where the height < # of FCPs */
		if (hsub < 1.0) {
		    if (gCCPriv->FrameCount > h)
			gCCPriv->FrameCount = 0;
		    h = 1;
		    y += gCCPriv->FrameCount;
		} else {
		    h = (gCCPriv->FrameCount+1)*hsub;
		    h -= (int)(gCCPriv->FrameCount*hsub);
		    y += gCCPriv->FrameCount*hsub;
		}
	    }
#endif

	    if (h) {
		CHECK_DMA_BUFFER(gCC, gCCPriv, 8);
		WRITE(gCCPriv->buf, StartXDom,   gCCPriv->x<<16);
		WRITE(gCCPriv->buf, StartY,      y<<16);
		WRITE(gCCPriv->buf, StartXSub,  (gCCPriv->x+gCCPriv->w)<<16);
		WRITE(gCCPriv->buf, GLINTCount,  h);
		WRITE(gCCPriv->buf, dY,          1<<16);
		WRITE(gCCPriv->buf, dXDom,       0<<16);
		WRITE(gCCPriv->buf, dXSub,       0<<16);
		WRITE(gCCPriv->buf, Render,      0x00000040); /* NOT_DONE */
	    }
	}

	/* Restore modes */
	CHECK_DMA_BUFFER(gCC, gCCPriv, 5);
	WRITE(gCCPriv->buf, FBWriteMode, FBWriteModeEnable);
	WRITE(gCCPriv->buf, DepthMode, gCCPriv->DepthMode);
	WRITE(gCCPriv->buf, LBReadMode, gCCPriv->LBReadMode);
	WRITE(gCCPriv->buf, GLINTWindow, gCCPriv->Window);
	WRITE(gCCPriv->buf, FastClearDepth, depth);

	/* Turn on Depth FCP */
	if (gCCPriv->Window & W_DepthFCP) {
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITE(gCCPriv->buf, WindowOr, (gCCPriv->FrameCount << 9));
	}

	/* Turn off GID clipping if window is not clipped */
	if (gCCPriv->NotClipped) {
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITE(gCCPriv->buf, Rectangle2DControl, 0);
	}
    }

    if (mask & GL_COLOR_BUFFER_BIT) {
	CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	WRITE(gCCPriv->buf, ConstantColor,
	      (((GLuint)(gCCPriv->ClearColor[3]*255.0) << 24) |
	       ((GLuint)(gCCPriv->ClearColor[2]*255.0) << 16) |
	       ((GLuint)(gCCPriv->ClearColor[1]*255.0) << 8)  |
	       ((GLuint)(gCCPriv->ClearColor[0]*255.0))));
	WRITE(gCCPriv->buf, FBBlockColor,
	      (((GLuint)(gCCPriv->ClearColor[3]*255.0) << 24) |
	       ((GLuint)(gCCPriv->ClearColor[0]*255.0) << 16) |
	       ((GLuint)(gCCPriv->ClearColor[1]*255.0) << 8)  |
	       ((GLuint)(gCCPriv->ClearColor[2]*255.0))));
	WRITE(gCCPriv->buf, ColorDDAMode, (ColorDDAEnable |
					   ColorDDAFlat));
	do_clear = 1;
    } else {
	/* Turn off writes the FB */
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, FBWriteMode, FBWriteModeDisable);
    }

    if (do_clear) {
	/* Turn on GID clipping if window is not clipped */
	if (!gCCPriv->NotClipped) {
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITE(gCCPriv->buf, Rectangle2DControl, 1);
	}

	CHECK_DMA_BUFFER(gCC, gCCPriv, 6);
	WRITE(gCCPriv->buf, DepthMode, 0);
	WRITE(gCCPriv->buf, AlphaBlendMode, 0);
	WRITE(gCCPriv->buf, Rectangle2DMode, (((gCCPriv->h & 0xfff)<<12) |
					      (gCCPriv->w & 0xfff)));
	WRITE(gCCPriv->buf, DrawRectangle2D, (((gCCPriv->y & 0xffff)<<16) |
					      (gCCPriv->x & 0xffff)));
	WRITE(gCCPriv->buf, DepthMode, gCCPriv->DepthMode);
	WRITE(gCCPriv->buf, AlphaBlendMode, gCCPriv->AlphaBlendMode);

	/* Turn off GID clipping if window is not clipped */
	if (gCCPriv->NotClipped) {
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITE(gCCPriv->buf, Rectangle2DControl, 0);
	}
    }

    if (mask & GL_COLOR_BUFFER_BIT) {
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, ColorDDAMode, gCCPriv->ColorDDAMode);
    } else {
	/* Turn on writes the FB */
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, FBWriteMode, FBWriteModeEnable);
    }

#ifdef DO_VALIDATE
    PROCESS_DMA_BUFFER_TOP_HALF(gCCPriv);

    DRM_SPINUNLOCK(&driScrnPriv->pSAREA->drawable_lock,
		   driScrnPriv->drawLockID);
    VALIDATE_DRAWABLE_INFO_NO_LOCK_POST(gCC,gCCPriv);

    PROCESS_DMA_BUFFER_BOTTOM_HALF(gCCPriv);
#endif

#if 0
    FLUSH_DMA_BUFFER(gCC,gCCPriv);
#endif
}

void _gamma_ClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    DEBUG_GLCMDS(("ClearAccum: %f %f %f %f\n", red, green, blue, alpha));
}

void _gamma_ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    DEBUG_GLCMDS(("ClearColor: %f %f %f %f\n",
		  (float)red, (float)green, (float)blue, (float)alpha));

    gCCPriv->ClearColor[0] = red;
    gCCPriv->ClearColor[1] = green;
    gCCPriv->ClearColor[2] = blue;
    gCCPriv->ClearColor[3] = alpha;
}

void _gamma_ClearDepth(GLclampd depth)
{
    DEBUG_GLCMDS(("ClearDepth: %f\n", (float)depth));

    gCCPriv->ClearDepth = depth;
}

void _gamma_ClearIndex(GLfloat c)
{
    DEBUG_GLCMDS(("ClearIndex: %f\n", c));
}

void _gamma_ClearStencil(GLint s)
{
    DEBUG_GLCMDS(("ClearStencil: %d\n", (int)s));
}

void _gamma_ClipPlane(GLenum plane, const GLdouble *equation)
{
    DEBUG_GLCMDS(("ClipPlane: %04x %f %f %f %f\n", (int)plane,
		  equation[0], equation[1], equation[2], equation[3]));
}

void _gamma_Color3b(GLbyte red, GLbyte green, GLbyte blue)
{
    DEBUG_GLCMDS(("Color3b: %d %d %d\n", red, green, blue));
  
    _gamma_Color3f((GLfloat)red,(GLfloat)green,(GLfloat)blue);
}

void _gamma_Color3bv(const GLbyte *v)
{
    DEBUG_GLCMDS(("Color3bv: %d %d %d\n", v[0], v[1], v[2]));

    _gamma_Color3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_Color3d(GLdouble red, GLdouble green, GLdouble blue)
{
    DEBUG_GLCMDS(("Color3d: %f %f %f\n", red, green, blue));

    _gamma_Color3f((GLfloat)red,(GLfloat)green,(GLfloat)blue);
}

void _gamma_Color3dv(const GLdouble *v)
{
    DEBUG_GLCMDS(("Color3dv: %f %f %f\n", v[0], v[1], v[2]));

    _gamma_Color3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_Color3f(GLfloat red, GLfloat green, GLfloat blue)
{
    DEBUG_GLCMDS(("Color3f: %f %f %f\n", red, green, blue));

    gCCPriv->Current.Color[0] = red;
    gCCPriv->Current.Color[1] = green;
    gCCPriv->Current.Color[2] = blue;
    gCCPriv->Current.Color[3] = 1.0f;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
    WRITEF(gCCPriv->buf, Cb,  blue);
    WRITEF(gCCPriv->buf, Cg,  green);
    WRITEF(gCCPriv->buf, Cr3, red);
}

void _gamma_Color3fv(const GLfloat *v)
{
    DEBUG_GLCMDS(("Color3fv: %f %f %f\n", v[0], v[1], v[2]));

    _gamma_Color3f(v[0],v[1],v[2]);
}

void _gamma_Color3i(GLint red, GLint green, GLint blue)
{
    DEBUG_GLCMDS(("Color3i: %d %d %d\n", (int)red, (int)green, (int)blue));

    _gamma_Color3f((GLfloat)red,(GLfloat)green,(GLfloat)blue);
}

void _gamma_Color3iv(const GLint *v)
{
    DEBUG_GLCMDS(("Color3iv: %d %d %d\n", (int)v[0], (int)v[1], (int)v[2]));

    _gamma_Color3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_Color3s(GLshort red, GLshort green, GLshort blue)
{
    DEBUG_GLCMDS(("Color3s: %d %d %d\n", red, green, blue));

    _gamma_Color3f((GLfloat)red,(GLfloat)green,(GLfloat)blue);
}

void _gamma_Color3sv(const GLshort *v)
{
    DEBUG_GLCMDS(("Color3sv: %d %d %d\n", v[0], v[1], v[2]));

    _gamma_Color3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_Color3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    GLuint c;

    DEBUG_GLCMDS(("Color3ub: %d %d %d\n", red, green, blue));

    c = (blue << 16) | (green << 8) | red;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, PackedColor3,  c);
}

void _gamma_Color3ubv(const GLubyte *v)
{
    GLuint c;

    DEBUG_GLCMDS(("Color3ubv: %d %d %d\n", v[0], v[1], v[2]));

    c = (v[2] << 16) | (v[1] << 8) | v[0];

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, PackedColor3,  c);
}

void _gamma_Color3ui(GLuint red, GLuint green, GLuint blue)
{
    DEBUG_GLCMDS(("Color3ui: %d %d %d\n",
		  (unsigned int)red, (unsigned int)green, (unsigned int)blue));

    _gamma_Color3f((GLfloat)red,(GLfloat)green,(GLfloat)blue);
}

void _gamma_Color3uiv(const GLuint *v)
{
    DEBUG_GLCMDS(("Color3uiv: %d %d %d\n",
		  (unsigned int)v[0], (unsigned int)v[1], (unsigned int)v[2]));

    _gamma_Color3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_Color3us(GLushort red, GLushort green, GLushort blue)
{
    DEBUG_GLCMDS(("Color3us: %d %d %d\n", red, green, blue));

    _gamma_Color3f((GLfloat)red,(GLfloat)green,(GLfloat)blue);
}

void _gamma_Color3usv(const GLushort *v)
{
    DEBUG_GLCMDS(("Color3usv: %d %d %d\n", v[0], v[1], v[2]));

    _gamma_Color3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_Color4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    GLfloat r,g,b,a;

    DEBUG_GLCMDS(("Color4b: %d %d %d %d\n", red, green, blue, alpha));

    r = BYTE_TO_FLOAT(red);
    g = BYTE_TO_FLOAT(green);
    b = BYTE_TO_FLOAT(blue);
    a = BYTE_TO_FLOAT(alpha);

    _gamma_Color4f(r,g,b,a);
}

void _gamma_Color4bv(const GLbyte *v)
{
    GLfloat p[4];

    DEBUG_GLCMDS(("Color4bv: %d %d %d %d\n", v[0], v[1], v[2], v[3]));

    p[0] = BYTE_TO_FLOAT(v[0]);
    p[1] = BYTE_TO_FLOAT(v[1]);
    p[2] = BYTE_TO_FLOAT(v[2]);
    p[3] = BYTE_TO_FLOAT(v[3]);

    _gamma_Color4fv(p);
}

void _gamma_Color4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    DEBUG_GLCMDS(("Color4d: %f %f %f %f\n", red, green, blue, alpha));

    _gamma_Color4f((GLfloat)red,(GLfloat)green,(GLfloat)blue,(GLfloat)alpha);
}

void _gamma_Color4dv(const GLdouble *v)
{
    DEBUG_GLCMDS(("Color4dv: %f %f %f %f\n", v[0], v[1], v[2], v[3]));

    _gamma_Color4f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2],(GLfloat)v[3]);
}

void _gamma_Color4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    DEBUG_GLCMDS(("Color4f: %f %f %f %f\n", red, green, blue, alpha));

    gCCPriv->Current.Color[0] = red;
    gCCPriv->Current.Color[1] = green;
    gCCPriv->Current.Color[2] = blue;
    gCCPriv->Current.Color[3] = alpha;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 4);
    WRITEF(gCCPriv->buf, Ca,  alpha);
    WRITEF(gCCPriv->buf, Cb,  blue);
    WRITEF(gCCPriv->buf, Cg,  green);
    WRITEF(gCCPriv->buf, Cr4, red);
}

void _gamma_Color4fv(const GLfloat *v)
{
    DEBUG_GLCMDS(("Color4fv: %f %f %f %f\n", v[0], v[1], v[2], v[3]));

    gCCPriv->Current.Color[0] = v[0];
    gCCPriv->Current.Color[1] = v[1];
    gCCPriv->Current.Color[2] = v[2];
    gCCPriv->Current.Color[3] = v[3];

    CHECK_DMA_BUFFER(gCC, gCCPriv, 4);
    WRITEF(gCCPriv->buf, Ca,  v[3]);
    WRITEF(gCCPriv->buf, Cb,  v[2]);
    WRITEF(gCCPriv->buf, Cg,  v[1]);
    WRITEF(gCCPriv->buf, Cr4, v[0]);
}

void _gamma_Color4i(GLint red, GLint green, GLint blue, GLint alpha)
{
    GLfloat r,g,b,a;

    DEBUG_GLCMDS(("Color4i: %d %d %d %d\n", (int)red, (int)green, (int)blue,
		  (int)alpha));

    r = INT_TO_FLOAT(red);
    g = INT_TO_FLOAT(green);
    b = INT_TO_FLOAT(blue);
    a = INT_TO_FLOAT(alpha);

    _gamma_Color4f(r,g,b,a);
}

void _gamma_Color4iv(const GLint *v)
{
    GLfloat p[4];

    DEBUG_GLCMDS(("Color4iv: %d %d %d %d\n", (int)v[0], (int)v[1], (int)v[2],
		  (int)v[3]));

    p[0] = INT_TO_FLOAT(v[0]);
    p[1] = INT_TO_FLOAT(v[1]);
    p[2] = INT_TO_FLOAT(v[2]);
    p[3] = INT_TO_FLOAT(v[3]);

    _gamma_Color4fv(p);
}

void _gamma_Color4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    GLfloat r,g,b,a;

    DEBUG_GLCMDS(("Color4s: %d %d %d %d\n", red, green, blue, alpha));

    r = SHORT_TO_FLOAT(red);
    g = SHORT_TO_FLOAT(green);
    b = SHORT_TO_FLOAT(blue);
    a = SHORT_TO_FLOAT(alpha);

    _gamma_Color4f(r,g,b,a);
}

void _gamma_Color4sv(const GLshort *v)
{
    GLfloat p[4];

    DEBUG_GLCMDS(("Color4sv: %d %d %d %d\n", v[0], v[1], v[2], v[3]));

    p[0] = SHORT_TO_FLOAT(v[0]);
    p[1] = SHORT_TO_FLOAT(v[1]);
    p[2] = SHORT_TO_FLOAT(v[2]);
    p[3] = SHORT_TO_FLOAT(v[3]);

    _gamma_Color4fv(p);
}

void _gamma_Color4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    GLuint c;

    DEBUG_GLCMDS(("Color4ub: %d %d %d %d\n", red, green, blue, alpha));

    c = (alpha << 24) | (blue << 16) | (green << 8) | red;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, PackedColor4,  c);
}

void _gamma_Color4ubv(const GLubyte *v)
{
    GLuint c;

    DEBUG_GLCMDS(("Color4ubv: %d %d %d %d\n", v[0], v[1], v[2], v[3]));

    c = (v[3] << 24) | (v[2] << 16) | (v[1] << 8) | v[0];

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, PackedColor4,  c);
}

void _gamma_Color4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    GLfloat r,g,b,a;

    DEBUG_GLCMDS(("Color4ui: %d %d %d %d\n",
		  (unsigned int)red, (unsigned int)green,
		  (unsigned int)blue, (unsigned int)alpha));

    r = UINT_TO_FLOAT(red);
    g = UINT_TO_FLOAT(green);
    b = UINT_TO_FLOAT(blue);
    a = UINT_TO_FLOAT(alpha);

    _gamma_Color4f(r,g,b,a);
}

void _gamma_Color4uiv(const GLuint *v)
{
    GLfloat p[4];

    DEBUG_GLCMDS(("Color4uiv: %d %d %d %d\n",
		  (unsigned int)v[0], (unsigned int)v[1],
		  (unsigned int)v[2], (unsigned int)v[3]));

    p[0] = UINT_TO_FLOAT(v[0]);
    p[1] = UINT_TO_FLOAT(v[1]);
    p[2] = UINT_TO_FLOAT(v[2]);
    p[3] = UINT_TO_FLOAT(v[3]);

    _gamma_Color4fv(p);
}

void _gamma_Color4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    GLfloat r,g,b,a;

    DEBUG_GLCMDS(("Color4us: %d %d %d %d\n", red, green, blue, alpha));

    r = USHORT_TO_FLOAT(red);
    g = USHORT_TO_FLOAT(green);
    b = USHORT_TO_FLOAT(blue);
    a = USHORT_TO_FLOAT(alpha);

    _gamma_Color4f(r,g,b,a);
}

void _gamma_Color4usv(const GLushort *v)
{
    GLfloat p[4];

    DEBUG_GLCMDS(("Color4usv: %d %d %d %d\n", v[0], v[1], v[2], v[3]));

    p[0] = USHORT_TO_FLOAT(v[0]);
    p[1] = USHORT_TO_FLOAT(v[1]);
    p[2] = USHORT_TO_FLOAT(v[2]);
    p[3] = USHORT_TO_FLOAT(v[3]);

    _gamma_Color4fv(p);
}

void _gamma_ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    DEBUG_GLCMDS(("ColorMask: %d %d %d %d\n", red, green, blue, alpha));
}

void _gamma_ColorMaterial(GLenum face, GLenum mode)
{
    DEBUG_GLCMDS(("ColorMaterial: %04x %04x\n", (int)face, (int)mode));

    gCCPriv->ColorMaterialMode &= ~ColorMaterialModeMask;

    switch (face) {
    case GL_FRONT:
	gCCPriv->ColorMaterialMode |= ColorMaterialModeFront;
	break;
    case GL_BACK:
	gCCPriv->ColorMaterialMode |= ColorMaterialModeBack;
	break;
    case GL_FRONT_AND_BACK:
	gCCPriv->ColorMaterialMode |= ColorMaterialModeFrontAndBack;
	break;
    }

    switch (mode) {
    case GL_AMBIENT:
	gCCPriv->ColorMaterialMode |= ColorMaterialModeAmbient;
	break;
    case GL_EMISSION:
	gCCPriv->ColorMaterialMode |= ColorMaterialModeEmission;
	break;
    case GL_DIFFUSE:
	gCCPriv->ColorMaterialMode |= ColorMaterialModeDiffuse;
	break;
    case GL_SPECULAR:
	gCCPriv->ColorMaterialMode |= ColorMaterialModeSpecular;
	break;
    case GL_AMBIENT_AND_DIFFUSE:
	gCCPriv->ColorMaterialMode |= ColorMaterialModeAmbAndDiff;
	break;
    }

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, ColorMaterialMode, gCCPriv->ColorMaterialMode);
}

void _gamma_ColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    DEBUG_GLCMDS(("ColorPointer: %d %04x %d\n",
		  (int)size, (int)type, (int)stride));

   if (size<3 || size>4) {
      gamma_error( GL_INVALID_VALUE, "glColorPointer(size)" );
      return;
   }
   if (stride<0) {
      gamma_error( GL_INVALID_VALUE, "glColorPointer(stride)" );
      return;
   }
   switch (type) {
      case GL_BYTE:
         gCCPriv->Array.ColorStrideB = stride ? stride : size*sizeof(GLbyte);
         break;
      case GL_UNSIGNED_BYTE:
         gCCPriv->Array.ColorStrideB = stride ? stride : size*sizeof(GLubyte);
         break;
      case GL_SHORT:
         gCCPriv->Array.ColorStrideB = stride ? stride : size*sizeof(GLshort);
         break;
      case GL_UNSIGNED_SHORT:
         gCCPriv->Array.ColorStrideB = stride ? stride : size*sizeof(GLushort);
         break;
      case GL_INT:
         gCCPriv->Array.ColorStrideB = stride ? stride : size*sizeof(GLint);
         break;
      case GL_UNSIGNED_INT:
         gCCPriv->Array.ColorStrideB = stride ? stride : size*sizeof(GLuint);
         break;
      case GL_FLOAT:
         gCCPriv->Array.ColorStrideB = stride ? stride : size*sizeof(GLfloat);
         break;
      case GL_DOUBLE:
         gCCPriv->Array.ColorStrideB = stride ? stride : size*sizeof(GLdouble);
         break;
      default:
         gamma_error( GL_INVALID_ENUM, "glColorPointer(type)" );
         return;
   }
   gCCPriv->Array.ColorSize = size;
   gCCPriv->Array.ColorType = type;
   gCCPriv->Array.ColorStride = stride;
   gCCPriv->Array.ColorPtr = (void *) pointer;
}

void _gamma_CopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    DEBUG_GLCMDS(("CopyPixels: %d %d %d %d %04x\n", (int)x, (int)y,
		  (int)width, (int)height, (int)type));
}

void _gamma_CopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)
{
    DEBUG_GLCMDS(("CopyTexImage1D: %04x %d %04x %d %d %d %d\n",
		  (int)target, (int)level, (int)internalformat,
		  (int)x, (int)y, (int)width, (int)border));
}

void _gamma_CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    DEBUG_GLCMDS(("CopyTexImage2D: %04x %d %04x %d %d %d %d %d\n",
		  (int)target, (int)level, (int)internalformat,
		  (int)x, (int)y, (int)width, (int)height, (int)border));
}

void _gamma_CopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    DEBUG_GLCMDS(("CopyTexSubImage1D: %04x %d %d %d %d %d\n",
		  (int)target, (int)level,
		  (int)xoffset, (int)x, (int)y, (int)width));
}

void _gamma_CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    DEBUG_GLCMDS(("CopyTexSubImage2D: %04x %d %d %d %d %d %d %d\n",
		  (int)target, (int)level, (int)xoffset, (int)yoffset,
		  (int)x, (int)y, (int)width, (int)height));
}

void _gamma_CullFace(GLenum mode)
{
    DEBUG_GLCMDS(("CullFace: %04x\n", (int)mode));

    gCCPriv->GeometryMode &= ~GM_PolyCullMask;

    switch (mode) {
    case GL_FRONT:
	gCCPriv->GeometryMode |= GM_PolyCullFront;
	break;
    case GL_BACK:
	gCCPriv->GeometryMode |= GM_PolyCullBack;
	break;
    case GL_FRONT_AND_BACK:
	gCCPriv->GeometryMode |= GM_PolyCullBoth;
	break;
    default:
	/* ERROR!! */
	break;
    }

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, GeometryMode, gCCPriv->GeometryMode);
}

#if 0 /* Now in dlist.c */
void _gamma_DeleteLists(GLuint list, GLsizei range)
{
    DEBUG_GLCMDS(("DeleteLists: %d %d\n", (unsigned int)list, (int)range));
}
#endif

void _gamma_DeleteTextures(GLsizei n, const GLuint *textures)
{
    int i;

    DEBUG_GLCMDS(("DeleteTextures: %d\n", (int)n));
#ifdef DEBUG_VERBOSE_EXTRA
    {
	int t;
	for (t = 0; t < n; t++)
	    printf("\t%d\n", (unsigned int)textures[t]);
    }
#endif

    for (i = 0; i < n; i++) {
	gammaTexObj *t = gammaTOFind(textures[i]);
	if (driTMMDeleteImages(gCCPriv->tmm, MIPMAP_LEVELS, t->image) < 0) {
    	    DEBUG_ERROR(("DeleteTextures: unable\n"));
	    /* NOT_DONE: Handle error */
	}
	gammaTODelete(textures[i]);
    }

    gCCPriv->curTexObj = gammaTOFind(0);
    gCCPriv->curTexObj1D = gCCPriv->curTexObj;
    gCCPriv->curTexObj2D = gCCPriv->curTexObj;
}

void _gamma_DepthFunc(GLenum func)
{
    DEBUG_GLCMDS(("DepthFunc: %04x\n", (int)func));

    gCCPriv->DepthMode &= ~DM_CompareMask;

    switch (func) {
    case GL_NEVER:
	gCCPriv->DepthMode |= DM_Never;
	break;
    case GL_LESS:
	gCCPriv->DepthMode |= DM_Less;
	break;
    case GL_EQUAL:
	gCCPriv->DepthMode |= DM_Equal;
	break;
    case GL_LEQUAL:
	gCCPriv->DepthMode |= DM_LessEqual;
	break;
    case GL_GREATER:
	gCCPriv->DepthMode |= DM_Greater;
	break;
    case GL_NOTEQUAL:
	gCCPriv->DepthMode |= DM_NotEqual;
	break;
    case GL_GEQUAL:
	gCCPriv->DepthMode |= DM_GreaterEqual;
	break;
    case GL_ALWAYS:
	gCCPriv->DepthMode |= DM_Always;
	break;
    default:
	/* ERROR!! */
	break;
    }

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, DepthMode, gCCPriv->DepthMode);
}

void _gamma_DepthMask(GLboolean flag)
{
    DEBUG_GLCMDS(("DepthMask: %d\n", flag));

    if (flag) {
	gCCPriv->DepthMode |=  DM_WriteMask;
    } else {
	gCCPriv->DepthMode &= ~DM_WriteMask;
    }

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, DepthMode, gCCPriv->DepthMode);
}

void _gamma_DepthRange(GLclampd zNear, GLclampd zFar)
{
    GLfloat sz, oz;

    DEBUG_GLCMDS(("DepthRange: %f %f\n", (float)zNear, (float)zFar));

    gCCPriv->zNear = zNear;
    gCCPriv->zFar = zFar;

    oz = (zFar+zNear)/2.0;
    sz = (zFar-zNear)/2.0;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
    WRITEF(gCCPriv->buf, ViewPortScaleZ,  sz);
    WRITEF(gCCPriv->buf, ViewPortOffsetZ, oz);
}

void _gamma_Disable(GLenum cap)
{
    DEBUG_GLCMDS(("Disable %04x\n", (int)cap));

    switch (cap) {
    case GL_CULL_FACE:
	gCCPriv->GeometryMode &= ~GM_PolyCullEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, GeometryMode, gCCPriv->GeometryMode);
	break;
    case GL_DEPTH_TEST:
	if (gCCPriv->Flags & GAMMA_DEPTH_BUFFER) {
	    gCCPriv->EnabledFlags &= ~GAMMA_DEPTH_BUFFER;
	    gCCPriv->DepthMode &= ~DepthModeEnable;
	    gCCPriv->LBReadMode &= ~LBReadDstEnable;
	    gCCPriv->DeltaMode &= ~DM_DepthEnable;
	    gCCPriv->Window &= ~W_DepthFCP;

	    /* Turn depth mode off */
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	    WRITE(gCCPriv->buf, DepthMode,      gCCPriv->DepthMode);
	    WRITE(gCCPriv->buf, DeltaMode,      gCCPriv->DeltaMode);
	    WRITE(gCCPriv->buf, LBReadModeAnd, ~LBReadDstEnable);
	    WRITE(gCCPriv->buf, WindowAnd,     ~W_DepthFCP);
	}
	break;
    case GL_ALPHA_TEST:
	/* Do I need to verify that alpha is enabled? */
	gCCPriv->AlphaTestMode &= ~AlphaTestModeEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	WRITE(gCCPriv->buf, AlphaTestMode, gCCPriv->AlphaTestMode);
	WRITE(gCCPriv->buf, RouterMode, R_Order_DepthTexture);
	break;
    case GL_BLEND:
	/* Do I need to verify that alpha is enabled? */
	gCCPriv->AlphaBlendMode &= ~AlphaBlendModeEnable;
	gCCPriv->AB_FBReadMode   =  0;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	WRITE(gCCPriv->buf, AlphaBlendMode, gCCPriv->AlphaBlendMode);
	WRITE(gCCPriv->buf, FBReadMode, (gCCPriv->FBReadMode |
					 gCCPriv->AB_FBReadMode));
	break;
    case GL_COLOR_MATERIAL:
	gCCPriv->ColorMaterialMode &= ~ColorMaterialModeEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, ColorMaterialMode, gCCPriv->ColorMaterialMode);
	break;
    case GL_FOG:
	gCCPriv->Begin &= ~B_FogEnable;
	gCCPriv->GeometryMode &= ~GM_FogEnable;
	gCCPriv->DeltaMode &= ~DM_FogEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	WRITE(gCCPriv->buf, GeometryMode, gCCPriv->GeometryMode);
	WRITE(gCCPriv->buf, DeltaMode, gCCPriv->DeltaMode);
	WRITE(gCCPriv->buf, FogMode, FogModeDisable);
	break;
    case GL_LIGHTING:
	gCCPriv->LightingMode &= ~LightingModeEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, LightingMode, gCCPriv->LightingMode);
	break;
    case GL_LIGHT0:
	gCCPriv->Light0Mode &= ~LNM_On;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, Light0Mode, gCCPriv->Light0Mode);
	break;
    case GL_LIGHT1:
	gCCPriv->Light1Mode &= ~LNM_On;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, Light1Mode, gCCPriv->Light1Mode);
	break;
    case GL_LOGIC_OP:
	gCCPriv->LogicalOpMode &= ~LogicalOpModeEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, LogicalOpMode, gCCPriv->LogicalOpMode);
	break;
    case GL_NORMALIZE:
	gCCPriv->NormalizeMode &= ~NormalizeModeEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	WRITE(gCCPriv->buf, NormalizeMode, gCCPriv->NormalizeMode);
	WRITE(gCCPriv->buf, TransformModeOr, 0xc);
	break;
    case GL_SCISSOR_TEST:
	gCCPriv->ScissorMode &= ~UserScissorEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, ScissorMode, gCCPriv->ScissorMode);
	break;
    case GL_TEXTURE_2D:
	gCCPriv->Texture2DEnabled = GL_FALSE;
	gCCPriv->Begin                         &= ~B_TextureEnable;
	gCCPriv->GeometryMode                  &= ~GM_TextureEnable;
	gCCPriv->DeltaMode                     &= ~DM_TextureEnable;
	gCCPriv->curTexObj->TextureAddressMode &= ~TextureAddressModeEnable;
	gCCPriv->curTexObj->TextureReadMode    &= ~TextureReadModeEnable;
	gCCPriv->curTexObj->TextureColorMode   &= ~TextureColorModeEnable;
	gCCPriv->curTexObj->TextureFilterMode  &= ~TextureFilterModeEnable;

	CHECK_DMA_BUFFER(gCC, gCCPriv, 6);
	WRITE(gCCPriv->buf, GeometryModeAnd, ~GM_TextureEnable);
	WRITE(gCCPriv->buf, DeltaModeAnd, ~DM_TextureEnable);
	WRITE(gCCPriv->buf, TextureAddressMode,
	      gCCPriv->curTexObj->TextureAddressMode);
	WRITE(gCCPriv->buf, TextureReadMode,
	      gCCPriv->curTexObj->TextureReadMode);
	WRITE(gCCPriv->buf, TextureColorMode,
	      gCCPriv->curTexObj->TextureColorMode);
	WRITE(gCCPriv->buf, TextureFilterMode,
	      gCCPriv->curTexObj->TextureFilterMode);
	break;
    default:
	break;
    }
}

void _gamma_DisableClientState(GLenum array)
{
    DEBUG_GLCMDS(("DisableClientState: %04x\n", (int)array));

    switch (array) {
      case GL_VERTEX_ARRAY:
         gCCPriv->Array.VertexEnabled = GL_FALSE;
         break;
      case GL_NORMAL_ARRAY:
         gCCPriv->Array.NormalEnabled = GL_FALSE;
         break;
      case GL_COLOR_ARRAY:
         gCCPriv->Array.ColorEnabled = GL_FALSE;
         break;
      case GL_INDEX_ARRAY:
         gCCPriv->Array.IndexEnabled = GL_FALSE;
         break;
      case GL_TEXTURE_COORD_ARRAY:
         gCCPriv->Array.TexCoordEnabled = GL_FALSE;
         break;
      case GL_EDGE_FLAG_ARRAY:
         gCCPriv->Array.EdgeFlagEnabled = GL_FALSE;
         break;
      default:
         gamma_error( GL_INVALID_ENUM, "glEnable/DisableClientState" );
   }
}

void _gamma_DrawArrays(GLenum mode, GLint first, GLsizei count)
{
    DEBUG_GLCMDS(("DrawArrays: %04x %d %d\n", (int)mode, (int)first,
		  (int)count));
}

void _gamma_DrawBuffer(GLenum mode)
{
    DEBUG_GLCMDS(("DrawBuffer: %04x\n", (int)mode));
}

void _gamma_DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{
    DEBUG_GLCMDS(("DrawElements: %04x %d %04x\n", (int)mode, (int)count,
		  (int)type));
}

void _gamma_DrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image)
{
    DEBUG_GLCMDS(("DrawPixels: %d %d %04x %04x\n", (int)width, (int)height,
		  (int)format, (int)type));
}

void _gamma_EdgeFlag(GLboolean flag)
{
    DEBUG_GLCMDS(("EdgeFlag: %d\n", flag));
}

void _gamma_EdgeFlagPointer(GLsizei stride, const GLvoid *pointer)
{
    DEBUG_GLCMDS(("EdgeFlagPointer: %d\n", (int)stride));
}

void _gamma_EdgeFlagv(const GLboolean *flag)
{
    DEBUG_GLCMDS(("EdgeFlagv: \n"));
}

void _gamma_Enable(GLenum cap)
{
    DEBUG_GLCMDS(("Enable %04x\n", (int)cap));

    switch (cap) {
    case GL_CULL_FACE:
	gCCPriv->GeometryMode |= GM_PolyCullEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, GeometryMode, gCCPriv->GeometryMode);
	break;
    case GL_DEPTH_TEST:
	if (gCCPriv->Flags & GAMMA_DEPTH_BUFFER) {
	    gCCPriv->EnabledFlags |= GAMMA_DEPTH_BUFFER;
#ifndef TURN_OFF_DEPTH
	    gCCPriv->DepthMode |= DepthModeEnable;
	    gCCPriv->LBReadMode |= LBReadDstEnable;
	    gCCPriv->DeltaMode |= DM_DepthEnable;
#ifndef TURN_OFF_FCP
	    gCCPriv->Window |= W_DepthFCP;
#endif

	    /* Turn depth mode on */
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	    WRITE(gCCPriv->buf, DepthMode,     gCCPriv->DepthMode);
	    WRITE(gCCPriv->buf, DeltaMode,     gCCPriv->DeltaMode);
	    WRITE(gCCPriv->buf, LBReadModeOr,  LBReadDstEnable);
#ifndef TURN_OFF_FCP
	    WRITE(gCCPriv->buf, WindowOr,     (W_DepthFCP |
					       (gCCPriv->FrameCount << 9)));
#else
	    WRITE(gCCPriv->buf, WindowOr,     (gCCPriv->FrameCount << 9));
#endif
#endif
	}
	break;
    case GL_ALPHA_TEST:
	gCCPriv->AlphaTestMode |= AlphaTestModeEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	WRITE(gCCPriv->buf, AlphaTestMode, gCCPriv->AlphaTestMode);
	WRITE(gCCPriv->buf, RouterMode, R_Order_TextureDepth);
	break;
    case GL_BLEND:
#ifndef TURN_OFF_BLEND
	gCCPriv->AlphaBlendMode |= AlphaBlendModeEnable;
	gCCPriv->AB_FBReadMode   = gCCPriv->AB_FBReadMode_Save;
#endif
	CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	WRITE(gCCPriv->buf, AlphaBlendMode, gCCPriv->AlphaBlendMode);
	WRITE(gCCPriv->buf, FBReadMode, (gCCPriv->FBReadMode |
					 gCCPriv->AB_FBReadMode));
	break;
    case GL_COLOR_MATERIAL:
	gCCPriv->ColorMaterialMode |= ColorMaterialModeEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, ColorMaterialMode, gCCPriv->ColorMaterialMode);
	break;
    case GL_FOG:
	gCCPriv->Begin |= B_FogEnable;
	gCCPriv->GeometryMode |= GM_FogEnable;
	gCCPriv->DeltaMode |= DM_FogEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	WRITE(gCCPriv->buf, GeometryMode, gCCPriv->GeometryMode);
	WRITE(gCCPriv->buf, DeltaMode, gCCPriv->DeltaMode);
	WRITE(gCCPriv->buf, FogMode, FogModeEnable);
	break;
    case GL_LIGHTING:
	gCCPriv->LightingMode |= LightingModeEnable | 16<<6;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, LightingMode, gCCPriv->LightingMode);
	break;
    case GL_LIGHT0:
	gCCPriv->Light0Mode |= LNM_On;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, Light0Mode, gCCPriv->Light0Mode);
	break;
    case GL_LIGHT1:
	gCCPriv->Light1Mode |= LNM_On;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, Light1Mode, gCCPriv->Light1Mode);
	break;
    case GL_LOGIC_OP:
	gCCPriv->LogicalOpMode |= LogicalOpModeEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, LogicalOpMode, gCCPriv->LogicalOpMode);
	break;
    case GL_NORMALIZE:
	gCCPriv->NormalizeMode |= NormalizeModeEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	WRITE(gCCPriv->buf, NormalizeMode, gCCPriv->NormalizeMode);
	WRITE(gCCPriv->buf, TransformModeAnd, 0xffffff3f);
	break;
    case GL_SCISSOR_TEST:
	gCCPriv->ScissorMode |= UserScissorEnable;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, ScissorMode, gCCPriv->ScissorMode);
	break;
    case GL_TEXTURE_2D:
	gCCPriv->Texture2DEnabled = GL_TRUE;
#ifndef TURN_OFF_TEXTURES
	gCCPriv->Begin                         |= B_TextureEnable;
#endif
	gCCPriv->GeometryMode                  |= GM_TextureEnable;
	gCCPriv->DeltaMode                     |= DM_TextureEnable;
	gCCPriv->curTexObj->TextureAddressMode |= TextureAddressModeEnable;
	gCCPriv->curTexObj->TextureReadMode    |= TextureReadModeEnable;
	gCCPriv->curTexObj->TextureColorMode   |= TextureColorModeEnable;
	gCCPriv->curTexObj->TextureFilterMode  |= TextureFilterModeEnable;

	CHECK_DMA_BUFFER(gCC, gCCPriv, 6);
	WRITE(gCCPriv->buf, GeometryModeOr, GM_TextureEnable);
	WRITE(gCCPriv->buf, DeltaModeOr, DM_TextureEnable);
	WRITE(gCCPriv->buf, TextureAddressMode,
	      gCCPriv->curTexObj->TextureAddressMode);
	WRITE(gCCPriv->buf, TextureReadMode,
	      gCCPriv->curTexObj->TextureReadMode);
	WRITE(gCCPriv->buf, TextureColorMode,
	      gCCPriv->curTexObj->TextureColorMode);
	WRITE(gCCPriv->buf, TextureFilterMode,
	      gCCPriv->curTexObj->TextureFilterMode);
	break;
    default:
	break;
    }
}

void _gamma_EnableClientState(GLenum array)
{
    DEBUG_GLCMDS(("EnableClientState: %04x\n", (int)array));

    switch (array) {
      case GL_VERTEX_ARRAY:
         gCCPriv->Array.VertexEnabled = GL_TRUE;
         break;
      case GL_NORMAL_ARRAY:
         gCCPriv->Array.NormalEnabled = GL_TRUE;
         break;
      case GL_COLOR_ARRAY:
         gCCPriv->Array.ColorEnabled = GL_TRUE;
         break;
      case GL_INDEX_ARRAY:
         gCCPriv->Array.IndexEnabled = GL_TRUE;
         break;
      case GL_TEXTURE_COORD_ARRAY:
         gCCPriv->Array.TexCoordEnabled = GL_TRUE;
         break;
      case GL_EDGE_FLAG_ARRAY:
         gCCPriv->Array.EdgeFlagEnabled = GL_TRUE;
         break;
      default:
         gamma_error( GL_INVALID_ENUM, "glEnable/DisableClientState" );
   }
}

void _gamma_End(void)
{
    DEBUG_GLCMDS(("End\n"));

    if ((gCCPriv->Begin & B_PrimType_Mask) == B_PrimType_Null) {
	/* ERROR!!! */
	return;
    }

    /* No longer inside of Begin/End */
    gCCPriv->Begin &= ~B_PrimType_Mask;
    gCCPriv->Begin |=  B_PrimType_Null;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, End, 0);
#if 0
    /* To force creation of smaller buffers */
    FLUSH_DMA_BUFFER(gCC,gCCPriv);
#endif
}

#if 0 /* Now in dlist.c */
void _gamma_EndList(void)
{
    DEBUG_GLCMDS(("EndList\n"));
}
#endif

void _gamma_EvalCoord1d(GLdouble u)
{
    DEBUG_GLCMDS(("EvalCoord1d: %f\n", u));
}

void _gamma_EvalCoord1dv(const GLdouble *u)
{
    DEBUG_GLCMDS(("EvalCoord1dv: %f\n", *u));
}

void _gamma_EvalCoord1f(GLfloat u)
{
    DEBUG_GLCMDS(("EvalCoord1f: %f\n", u));
}

void _gamma_EvalCoord1fv(const GLfloat *u)
{
    DEBUG_GLCMDS(("EvalCoord1fv: %f\n", *u));
}

void _gamma_EvalCoord2d(GLdouble u, GLdouble v)
{
    DEBUG_GLCMDS(("EvalCoord2d: %f %f\n", u, v));
}

void _gamma_EvalCoord2dv(const GLdouble *u)
{
    DEBUG_GLCMDS(("EvalCoord2dv: %f %f\n", u[0], u[1]));
}

void _gamma_EvalCoord2f(GLfloat u, GLfloat v)
{
    DEBUG_GLCMDS(("EvalCoord2f: %f %f\n", u, v));
}

void _gamma_EvalCoord2fv(const GLfloat *u)
{
    DEBUG_GLCMDS(("EvalCoord1fv: %f %f\n", u[0], u[1]));
}

void _gamma_EvalMesh1(GLenum mode, GLint i1, GLint i2)
{
    DEBUG_GLCMDS(("EvalMesh1: %d %d %d\n", mode, i1, i2));
}

void _gamma_EvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
}

void _gamma_EvalPoint1(GLint i)
{
}

void _gamma_EvalPoint2(GLint i, GLint j)
{
}

void _gamma_FeedbackBuffer(GLsizei size, GLenum type, GLfloat *buffer)
{
    DEBUG_GLCMDS(("FeedbackBuffer: %d %04x\n", (int)size, (int)type));
}

void _gamma_Finish(void)
{
    DEBUG_GLCMDS(("Finish\n"));

    FLUSH_DMA_BUFFER(gCC,gCCPriv);
}

void _gamma_Flush(void)
{
    DEBUG_GLCMDS(("Flush\n"));

    FLUSH_DMA_BUFFER(gCC,gCCPriv);
}

void _gamma_Fogf(GLenum pname, GLfloat param)
{
    DEBUG_GLCMDS(("Fogf: %04x %f\n", (int)pname, param));

    switch (pname) {
    case GL_FOG_DENSITY:
    	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITEF(gCCPriv->buf, FogDensity, param);
	break;
    }
}

void _gamma_Fogfv(GLenum pname, const GLfloat *params)
{
    int color;

    DEBUG_GLCMDS(("Fogfv: %04x %f\n", (int)pname, *params));

    switch (pname) {
    case GL_FOG_COLOR:
	color = (FLOAT_TO_UBYTE(params[3])<<24) |
		(FLOAT_TO_UBYTE(params[2])<<16) |
		(FLOAT_TO_UBYTE(params[1])<<8) |
		(FLOAT_TO_UBYTE(params[0]));
    	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, FogColor, color);
	break;
    }
}

void _gamma_Fogi(GLenum pname, GLint param)
{
    DEBUG_GLCMDS(("Fogi: %04x %d\n", (int)pname, (int)param));

    gCCPriv->GeometryMode &= ~GM_FogMask;

    switch (pname) {
    case GL_FOG_MODE:
	switch (param) {
	case GL_EXP:
	    gCCPriv->GeometryMode |= GM_FogExp;
	    break;
	case GL_EXP2:
	    gCCPriv->GeometryMode |= GM_FogExpSquared;
	    break;
	case GL_LINEAR:
	    gCCPriv->GeometryMode |= GM_FogLinear;
	    break;
	}
 	break;
    }

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, GeometryMode, gCCPriv->GeometryMode);
}

void _gamma_Fogiv(GLenum pname, const GLint *params)
{
    DEBUG_GLCMDS(("Fogiv: %04x %d\n", (int)pname, (int)*params));
}

void _gamma_FrontFace(GLenum mode)
{
    DEBUG_GLCMDS(("FrontFace: %04x\n", (int)mode));

    gCCPriv->GeometryMode &= ~GM_FFMask;

    if (mode == GL_CCW)
	gCCPriv->GeometryMode |= GM_FrontFaceCCW;
    else
	gCCPriv->GeometryMode |= GM_FrontFaceCW;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, GeometryMode, gCCPriv->GeometryMode);
}

void _gamma_Frustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
#define FRUSTUM_X() ((GLfloat)((2.0*zNear)/(right-left)))
#define FRUSTUM_Y() ((GLfloat)((2.0*zNear)/(top-bottom)))

#define FRUSTUM_A() ((GLfloat)(      (right+left)/(right-left)))
#define FRUSTUM_B() ((GLfloat)(      (top+bottom)/(top-bottom)))
#define FRUSTUM_C() ((GLfloat)(    -((zFar+zNear)/(zFar-zNear))))
#define FRUSTUM_D() ((GLfloat)(-((2.0*zFar*zNear)/(zFar-zNear))))

    GLfloat m[16];
    int i;

    DEBUG_GLCMDS(("Frustum: %f %f %f %f %f %f\n",
		  left, right, bottom, top, zNear, zFar));

    for (i = 0; i < 16; i++) m[i] = 0.0;

    m[0]  = FRUSTUM_X();
    m[5]  = FRUSTUM_Y();
    m[8]  = FRUSTUM_A();
    m[9]  = FRUSTUM_B();
    m[10] = FRUSTUM_C();
    m[11] = -1.0;
    m[14] = FRUSTUM_D();

    gammaMultMatrix(m);
    gammaLoadHWMatrix();
}

#if 0 /* Now in dlist.c */
GLuint _gamma_GenLists(GLsizei range)
{
    DEBUG_GLCMDS(("GenLists: %d\n", (int)range));

    return GL_TRUE;
}
#endif

void _gamma_GenTextures(GLsizei n, GLuint *textures)
{
    DEBUG_GLCMDS(("GenTextures: %d\n", (int)n));
}

void _gamma_GetBooleanv(GLenum val, GLboolean *b)
{
    int loop;

    DEBUG_GLCMDS(("GetBooleanv: %04x\n", (int)val));

    switch (val) {
    case GL_CURRENT_COLOR:
	b[0] = FLOAT_TO_BOOL(gCCPriv->Current.Color[0]);
	b[1] = FLOAT_TO_BOOL(gCCPriv->Current.Color[1]);
	b[2] = FLOAT_TO_BOOL(gCCPriv->Current.Color[2]);
	b[3] = FLOAT_TO_BOOL(gCCPriv->Current.Color[3]);
	break;
    case GL_CURRENT_NORMAL:
	b[0] = FLOAT_TO_BOOL(gCCPriv->Current.Normal[0]);
	b[1] = FLOAT_TO_BOOL(gCCPriv->Current.Normal[1]);
	b[2] = FLOAT_TO_BOOL(gCCPriv->Current.Normal[2]);
	break;
    case GL_MAX_TEXTURE_SIZE:
	*b = FLOAT_TO_BOOL(GAMMA_MAX_TEXTURE_SIZE);
	break;
    case GL_MODELVIEW_MATRIX:
	for (loop = 0; loop < 16; loop++)
	    b[loop] = FLOAT_TO_BOOL(gCCPriv->ModelView[loop]);
	break;
    case GL_CURRENT_TEXTURE_COORDS:
	b[0] = FLOAT_TO_BOOL(gCCPriv->Current.TexCoord[0]);
	b[1] = FLOAT_TO_BOOL(gCCPriv->Current.TexCoord[1]);
	b[2] = FLOAT_TO_BOOL(gCCPriv->Current.TexCoord[2]);
	b[3] = FLOAT_TO_BOOL(gCCPriv->Current.TexCoord[3]);
	break;
    }
}

void _gamma_GetClipPlane(GLenum plane, GLdouble *equation)
{
    DEBUG_GLCMDS(("GetClipPlane: %04x %f %f %f %f\n", (int)plane,
		  equation[0], equation[1], equation[2], equation[3]));
}

void _gamma_GetDoublev(GLenum val, GLdouble *d)
{
    int loop;

    DEBUG_GLCMDS(("GetDoublev: %04x\n", (int)val));

    switch (val) {
    case GL_CURRENT_COLOR:
	d[0] = (GLdouble)gCCPriv->Current.Color[0];
	d[1] = (GLdouble)gCCPriv->Current.Color[1];
	d[2] = (GLdouble)gCCPriv->Current.Color[2];
	d[3] = (GLdouble)gCCPriv->Current.Color[3];
	break;
    case GL_CURRENT_NORMAL:
	d[0] = (GLdouble)gCCPriv->Current.Normal[0];
	d[1] = (GLdouble)gCCPriv->Current.Normal[1];
	d[2] = (GLdouble)gCCPriv->Current.Normal[2];
	break;
    case GL_MAX_TEXTURE_SIZE:
	*d = (GLdouble)INT_TO_FLOAT(GAMMA_MAX_TEXTURE_SIZE);
	break;
    case GL_MODELVIEW_MATRIX:
	for (loop = 0; loop < 16; loop++)
	    d[loop] = (GLdouble)gCCPriv->ModelView[loop];
	break;
    case GL_CURRENT_TEXTURE_COORDS:
	d[0] = (GLdouble)gCCPriv->Current.TexCoord[0];
	d[1] = (GLdouble)gCCPriv->Current.TexCoord[1];
	d[2] = (GLdouble)gCCPriv->Current.TexCoord[2];
	d[3] = (GLdouble)gCCPriv->Current.TexCoord[3];
	break;
    }
}

GLenum _gamma_GetError(void)
{
    DEBUG_GLCMDS(("GetError\n"));

    return gCCPriv->ErrorValue;
}

void _gamma_GetFloatv(GLenum val, GLfloat *f)
{
    int loop;

    DEBUG_GLCMDS(("GetFloatv: %04x\n", (int)val));

    switch (val) {
    case GL_CURRENT_COLOR:
	f[0] = gCCPriv->Current.Color[0];
	f[1] = gCCPriv->Current.Color[1];
	f[2] = gCCPriv->Current.Color[2];
	f[3] = gCCPriv->Current.Color[3];
	break;
    case GL_CURRENT_NORMAL:
	f[0] = gCCPriv->Current.Normal[0];
	f[1] = gCCPriv->Current.Normal[1];
	f[2] = gCCPriv->Current.Normal[2];
	break;
    case GL_MAX_TEXTURE_SIZE:
	*f = INT_TO_FLOAT(GAMMA_MAX_TEXTURE_SIZE);
	break;
    case GL_MODELVIEW_MATRIX:
	for (loop = 0; loop < 16; loop++)
	    f[loop] = gCCPriv->ModelView[loop];
	break;
    case GL_CURRENT_TEXTURE_COORDS:
	f[0] = gCCPriv->Current.TexCoord[0];
	f[1] = gCCPriv->Current.TexCoord[1];
	f[2] = gCCPriv->Current.TexCoord[2];
	f[3] = gCCPriv->Current.TexCoord[3];
	break;
    default:
	break;
    }
}

void _gamma_GetIntegerv(GLenum val, GLint *i)
{
    int loop;

    DEBUG_GLCMDS(("GetIntegerv: %04x\n", (int)val));

    switch (val) {
    case GL_CURRENT_COLOR:
	i[0] = FLOAT_TO_INT(gCCPriv->Current.Color[0]);
	i[1] = FLOAT_TO_INT(gCCPriv->Current.Color[1]);
	i[2] = FLOAT_TO_INT(gCCPriv->Current.Color[2]);
	i[3] = FLOAT_TO_INT(gCCPriv->Current.Color[3]);
	break;
    case GL_CURRENT_NORMAL:
	i[0] = FLOAT_TO_INT(gCCPriv->Current.Normal[0]);
	i[1] = FLOAT_TO_INT(gCCPriv->Current.Normal[1]);
	i[2] = FLOAT_TO_INT(gCCPriv->Current.Normal[2]);
	break;
    case GL_MAX_TEXTURE_SIZE:
	*i = GAMMA_MAX_TEXTURE_SIZE;
	break;
    case GL_MODELVIEW_MATRIX:
	for (loop = 0; loop < 16; loop++)
	    i[loop] = FLOAT_TO_INT(gCCPriv->ModelView[loop]);
	break;
    case GL_CURRENT_TEXTURE_COORDS:
	i[0] = FLOAT_TO_INT(gCCPriv->Current.TexCoord[0]);
	i[1] = FLOAT_TO_INT(gCCPriv->Current.TexCoord[1]);
	i[2] = FLOAT_TO_INT(gCCPriv->Current.TexCoord[2]);
	i[3] = FLOAT_TO_INT(gCCPriv->Current.TexCoord[3]);
	break;
    }
}

void _gamma_GetLightfv(GLenum light, GLenum pname, GLfloat *params)
{
    DEBUG_GLCMDS(("GetLightfv: %04x %04x %f\n",
		  (int)light, (int)pname, *params));
}

void _gamma_GetLightiv(GLenum light, GLenum pname, GLint *params)
{
    DEBUG_GLCMDS(("GetLightiv: %04x %04x %d\n",
		  (int)light, (int)pname, *params));
}

void _gamma_GetMapdv(GLenum target, GLenum query, GLdouble *v)
{
}

void _gamma_GetMapfv(GLenum target, GLenum query, GLfloat *v)
{
}

void _gamma_GetMapiv(GLenum target, GLenum query, GLint *v)
{
}

void _gamma_GetMaterialfv(GLenum face, GLenum pname, GLfloat *params)
{
}

void _gamma_GetMaterialiv(GLenum face, GLenum pname, GLint *params)
{
}

void _gamma_GetPixelMapfv(GLenum map, GLfloat *values)
{
}

void _gamma_GetPixelMapuiv(GLenum map, GLuint *values)
{
}

void _gamma_GetPixelMapusv(GLenum map, GLushort *values)
{
}

void _gamma_GetPointerv(GLenum pname, void **params)
{
    DEBUG_GLCMDS(("GetPointerv: %04x\n", (int)pname));
}

void _gamma_GetPolygonStipple(GLubyte *mask)
{
}

const GLubyte *_gamma_GetString(GLenum name)
{
    static unsigned char vendor[] = "Precision Insight, Inc.";
    static unsigned char renderer[] = "DRI Glint-Gamma 20000605";
    static unsigned char version[] = "1.1";
    static unsigned char ext[] = "";

    switch (name) {
    case GL_VENDOR:
	return vendor;
    case GL_RENDERER:
	return renderer;
    case GL_VERSION:
	return version;
    case GL_EXTENSIONS:
	return ext;
    }

    return NULL;
}

void _gamma_GetTexEnvfv(GLenum target, GLenum pname, GLfloat *params)
{
}

void _gamma_GetTexEnviv(GLenum target, GLenum pname, GLint *params)
{
}

void _gamma_GetTexGendv(GLenum coord, GLenum pname, GLdouble *params)
{
}

void _gamma_GetTexGenfv(GLenum coord, GLenum pname, GLfloat *params)
{
}

void _gamma_GetTexGeniv(GLenum coord, GLenum pname, GLint *params)
{
}

void _gamma_GetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid *texels)
{
}

void _gamma_GetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
}

void _gamma_GetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params)
{
}

void _gamma_GetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
}

void _gamma_GetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
}

void _gamma_Hint(GLenum target, GLenum mode)
{
    DEBUG_GLCMDS(("Hint: %04x %04x\n", (int)target, (int)mode));
}

void _gamma_IndexMask(GLuint mask)
{
    DEBUG_GLCMDS(("Hint: %d\n", (unsigned int)mask));
}

void _gamma_IndexPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    DEBUG_GLCMDS(("IndexPointer: %04x %d\n", (int)type, (int)stride));
}

void _gamma_Indexd(GLdouble c)
{
    DEBUG_GLCMDS(("Indexd: %f\n", c));
}

void _gamma_Indexdv(const GLdouble *c)
{
    DEBUG_GLCMDS(("Indexdv: %f\n", *c));
}

void _gamma_Indexf(GLfloat c)
{
    DEBUG_GLCMDS(("Indexf: %f\n", c));
}

void _gamma_Indexfv(const GLfloat *c)
{
    DEBUG_GLCMDS(("Indexdv: %f\n", *c));
}

void _gamma_Indexi(GLint c)
{
    DEBUG_GLCMDS(("Indexi: %d\n", (int)c));
}

void _gamma_Indexiv(const GLint *c)
{
    DEBUG_GLCMDS(("Indexiv: %d\n", (int)*c));
}

void _gamma_Indexs(GLshort c)
{
    DEBUG_GLCMDS(("Indexs: %d\n", c));
}

void _gamma_Indexsv(const GLshort *c)
{
    DEBUG_GLCMDS(("Indexsv: %d\n", *c));
}

void _gamma_Indexub(GLubyte c)
{
    DEBUG_GLCMDS(("Indexub: %d\n", c));
}

void _gamma_Indexubv(const GLubyte *c)
{
    DEBUG_GLCMDS(("Indexubv: %d\n", *c));
}

void _gamma_InitNames(void)
{
    DEBUG_GLCMDS(("InitNames\n"));
}

void _gamma_InterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer)
{
    DEBUG_GLCMDS(("InterleavedArrays: %04x %d\n", (int)format, (int)stride));
}

GLboolean _gamma_IsEnabled(GLenum cap)
{
    DEBUG_GLCMDS(("IsEnabled: %04x\n", (int)cap));

    switch (cap) {
    case GL_LIGHTING:
	return ((gCCPriv->LightingMode & LightingModeEnable)?GL_TRUE:GL_FALSE);
        break;
    }

    return GL_TRUE;
}

#if 0 /* Now in dlist.c */
GLboolean _gamma_IsList(GLuint list)
{
    DEBUG_GLCMDS(("IsList: %04x\n", (unsigned int)list));

    return GL_TRUE;
}
#endif

GLboolean _gamma_IsTexture(GLuint texture)
{
    DEBUG_GLCMDS(("IsTexture: %04x\n", (unsigned int)texture));

    return GL_TRUE;
}

void _gamma_LightModelf(GLenum pname, GLfloat param)
{
    DEBUG_GLCMDS(("LightModelf: %04x %f\n",
		  (int)pname, param));
}

void _gamma_LightModelfv(GLenum pname, const GLfloat *params)
{
    DEBUG_GLCMDS(("LightModelfv: %04x %f\n",
		  (int)pname, *params));

    switch (pname) {
    case GL_LIGHT_MODEL_AMBIENT:
	CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	/* We don't do alpha */
	WRITEF(gCCPriv->buf, SceneAmbientColorBlue, params[2]);
	WRITEF(gCCPriv->buf, SceneAmbientColorGreen, params[1]);
	WRITEF(gCCPriv->buf, SceneAmbientColorRed, params[0]);
	break;
    case GL_LIGHT_MODEL_LOCAL_VIEWER:
	if (params[0] != 0.0)
	    gCCPriv->LightingMode |= LightingModeLocalViewer;
	else
	    gCCPriv->LightingMode &= ~LightingModeLocalViewer;
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, LightingMode, gCCPriv->LightingMode);
	break;
    case GL_LIGHT_MODEL_TWO_SIDE:
	if (params[0] == 1.0f) {
	    gCCPriv->LightingMode |= LightingModeTwoSides;
	    gCCPriv->MaterialMode |= MaterialModeTwoSides;
	} else {
	    gCCPriv->LightingMode &= ~LightingModeTwoSides;
	    gCCPriv->MaterialMode &= ~MaterialModeTwoSides;
	}
	CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	WRITE(gCCPriv->buf, LightingMode, gCCPriv->LightingMode);
	WRITE(gCCPriv->buf, MaterialMode, gCCPriv->MaterialMode);
	break;
    }
}

void _gamma_LightModeli(GLenum pname, GLint param)
{
}

void _gamma_LightModeliv(GLenum pname, const GLint *params)
{
}

void _gamma_Lightf(GLenum light, GLenum pname, GLfloat param)
{
    DEBUG_GLCMDS(("Lightf: %04x %04x %f\n",
		  (int)light, (int)pname, param));
}

void _gamma_Lightfv(GLenum light, GLenum pname, const GLfloat *params)
{
    GLfloat l,x,y,z,w;
    DEBUG_GLCMDS(("Lightfv: %04x %04x %f\n",
		  (int)light, (int)pname, *params));

    switch(light) {
    case GL_LIGHT0:
	switch (pname) {
	case GL_AMBIENT:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	    /* We don't do alpha */
	    WRITEF(gCCPriv->buf, Light0AmbientIntensityBlue, params[2]);
	    WRITEF(gCCPriv->buf, Light0AmbientIntensityGreen, params[1]);
	    WRITEF(gCCPriv->buf, Light0AmbientIntensityRed, params[0]);
	    break;
	case GL_DIFFUSE:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	    /* We don't do alpha */
	    WRITEF(gCCPriv->buf, Light0DiffuseIntensityBlue, params[2]);
	    WRITEF(gCCPriv->buf, Light0DiffuseIntensityGreen, params[1]);
	    WRITEF(gCCPriv->buf, Light0DiffuseIntensityRed, params[0]);
	    break;
	case GL_SPECULAR:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	    /* We don't do alpha */
	    WRITEF(gCCPriv->buf, Light0SpecularIntensityBlue, params[2]);
	    WRITEF(gCCPriv->buf, Light0SpecularIntensityGreen, params[1]);
	    WRITEF(gCCPriv->buf, Light0SpecularIntensityRed, params[0]);
	    break;
	case GL_POSITION:
    	    /* Normalize <x,y,z> */
	    x = params[0]; y = params[1]; z = params[2]; w = params[3];
	    l = sqrt(x*x + y*y + z*z + w*w);
	    w /= l;
	    x /= l;
	    y /= l;
	    z /= l;
	    if (params[3] != 0.0) {
		gCCPriv->Light0Mode |= Light0ModeAttenuation;
		gCCPriv->Light0Mode |= Light0ModeLocal;
	    } else {
		gCCPriv->Light0Mode &= ~Light0ModeAttenuation;
		gCCPriv->Light0Mode &= ~Light0ModeLocal;
	    }
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 5);
	    WRITE(gCCPriv->buf, Light0Mode, gCCPriv->Light0Mode);
	    WRITEF(gCCPriv->buf, Light0PositionW, w);
	    WRITEF(gCCPriv->buf, Light0PositionZ, z);
	    WRITEF(gCCPriv->buf, Light0PositionY, y);
	    WRITEF(gCCPriv->buf, Light0PositionX, x);
	    break;
	case GL_SPOT_DIRECTION:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	    /* WRITEF(gCCPriv->buf, Light0SpotlightDirectionW, params[3]); */
	    WRITEF(gCCPriv->buf, Light0SpotlightDirectionZ, params[2]);
	    WRITEF(gCCPriv->buf, Light0SpotlightDirectionY, params[1]);
	    WRITEF(gCCPriv->buf, Light0SpotlightDirectionX, params[0]);
	    break;
	case GL_SPOT_EXPONENT:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITEF(gCCPriv->buf, Light0SpotlightExponent, params[0]);
	    break;
	case GL_SPOT_CUTOFF:
	    if (params[0] != 180.0) 
		gCCPriv->Light0Mode |= Light0ModeSpotLight;
	    else
		gCCPriv->Light0Mode &= ~Light0ModeSpotLight;
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	    WRITE(gCCPriv->buf, Light0Mode, gCCPriv->Light0Mode);
	    WRITEF(gCCPriv->buf, Light0CosSpotlightCutoffAngle, cos(params[0]*DEG2RAD));
	    break;
	case GL_CONSTANT_ATTENUATION:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITEF(gCCPriv->buf, Light0ConstantAttenuation, params[0]);
	    break;
	case GL_LINEAR_ATTENUATION:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITEF(gCCPriv->buf, Light0LinearAttenuation, params[0]);
	    break;
	case GL_QUADRATIC_ATTENUATION:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITEF(gCCPriv->buf, Light0QuadraticAttenuation, params[0]);
	    break;
	}
	break;
    case GL_LIGHT1:
	switch (pname) {
	case GL_AMBIENT:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	    /* We don't do alpha */
	    WRITEF(gCCPriv->buf, Light1AmbientIntensityBlue, params[2]);
	    WRITEF(gCCPriv->buf, Light1AmbientIntensityGreen, params[1]);
	    WRITEF(gCCPriv->buf, Light1AmbientIntensityRed, params[0]);
	    break;
	case GL_DIFFUSE:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	    /* We don't do alpha */
	    WRITEF(gCCPriv->buf, Light1DiffuseIntensityBlue, params[2]);
	    WRITEF(gCCPriv->buf, Light1DiffuseIntensityGreen, params[1]);
	    WRITEF(gCCPriv->buf, Light1DiffuseIntensityRed, params[0]);
	    break;
	case GL_SPECULAR:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	    /* We don't do alpha */
	    WRITEF(gCCPriv->buf, Light1SpecularIntensityBlue, params[2]);
	    WRITEF(gCCPriv->buf, Light1SpecularIntensityGreen, params[1]);
	    WRITEF(gCCPriv->buf, Light1SpecularIntensityRed, params[0]);
	    break;
	case GL_POSITION:
    	    /* Normalize <x,y,z> */
	    x = params[0]; y = params[1]; z = params[2];
	    l = sqrt(x*x + y*y + z*z);
	    x /= l;
	    y /= l;
	    z /= l;
	    if (params[3] != 0.0) {
		gCCPriv->Light1Mode |= Light1ModeAttenuation;
		gCCPriv->Light1Mode |= Light1ModeLocal;
	    } else {
		gCCPriv->Light0Mode &= ~Light0ModeAttenuation;
		gCCPriv->Light0Mode &= ~Light0ModeLocal;
	    }
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 5);
	    WRITE(gCCPriv->buf, Light1Mode, gCCPriv->Light1Mode);
	    WRITEF(gCCPriv->buf, Light1PositionW, params[3]);
	    WRITEF(gCCPriv->buf, Light1PositionZ, z);
	    WRITEF(gCCPriv->buf, Light1PositionY, y);
	    WRITEF(gCCPriv->buf, Light1PositionX, x);
	    break;
	case GL_SPOT_DIRECTION:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	    /* WRITEF(gCCPriv->buf, Light1SpotlightDirectionW, params[3]); */
	    WRITEF(gCCPriv->buf, Light1SpotlightDirectionZ, params[2]);
	    WRITEF(gCCPriv->buf, Light1SpotlightDirectionY, params[1]);
	    WRITEF(gCCPriv->buf, Light1SpotlightDirectionX, params[0]);
	    break;
	case GL_SPOT_EXPONENT:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITEF(gCCPriv->buf, Light1SpotlightExponent, params[0]);
	    break;
	case GL_SPOT_CUTOFF:
	    if (params[0] != 180.0) 
		gCCPriv->Light1Mode |= Light1ModeSpotLight;
	    else
		gCCPriv->Light1Mode &= ~Light1ModeSpotLight;
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	    WRITE(gCCPriv->buf, Light1Mode, gCCPriv->Light1Mode);
	    WRITEF(gCCPriv->buf, Light1CosSpotlightCutoffAngle, cos(params[0]*DEG2RAD));
	    break;
	case GL_CONSTANT_ATTENUATION:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITEF(gCCPriv->buf, Light1ConstantAttenuation, params[0]);
	    break;
	case GL_LINEAR_ATTENUATION:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITEF(gCCPriv->buf, Light1LinearAttenuation, params[0]);
	    break;
	case GL_QUADRATIC_ATTENUATION:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITEF(gCCPriv->buf, Light1QuadraticAttenuation, params[0]);
	    break;
	}
    }
}

void _gamma_Lighti(GLenum light, GLenum pname, GLint param)
{
}

void _gamma_Lightiv(GLenum light, GLenum pname, const GLint *params)
{
}

void _gamma_LineStipple(GLint factor, GLushort pattern)
{
    DEBUG_GLCMDS(("LineStipple: %d %d\n", (int)factor, pattern));
}

void _gamma_LineWidth(GLfloat width)
{
    DEBUG_GLCMDS(("LineWidth: %f\n", width));
}

#if 0 /* Now in dlist.c */
void _gamma_ListBase(GLuint base)
{
    DEBUG_GLCMDS(("ListBase: %d\n", (unsigned int)base));
}
#endif

void _gamma_LoadIdentity(void)
{
    DEBUG_GLCMDS(("LoadIdentity: %04x\n", gCCPriv->MatrixMode));

    gammaSetMatrix(IdentityMatrix);
    gammaLoadHWMatrix();
}

void _gamma_LoadMatrixd(const GLdouble *m)
{
    GLfloat f[16];
    int i;

    DEBUG_GLCMDS(("LoadMatrixd: %04x\n", gCCPriv->MatrixMode));

    for (i = 0; i < 16; i++) f[i] = m[i];
    gammaSetMatrix(f);
    gammaLoadHWMatrix();
}

void _gamma_LoadMatrixf(const GLfloat *m)
{
    DEBUG_GLCMDS(("LoadMatrixf: %04x\n", gCCPriv->MatrixMode));

    gammaSetMatrix((GLfloat *)m);
    gammaLoadHWMatrix();
}

void _gamma_LoadName(GLuint name)
{
    DEBUG_GLCMDS(("LoadName: %d\n", (unsigned int)name));
}

void _gamma_LogicOp(GLenum opcode)
{
    DEBUG_GLCMDS(("LogicOp: %04x\n", (int)opcode));

    gCCPriv->LogicalOpMode &= ~LogicalOpModeMask;

    gCCPriv->LogicalOpMode |= (opcode & 0x0f) << 1;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, LogicalOpMode, gCCPriv->LogicalOpMode);
}

void _gamma_Map1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *pnts)
{
    DEBUG_GLCMDS(("Map1d: %04x %f %f %d %d\n", (int)target, u1, u2,
		  (int)stride, (int)order));
}

void _gamma_Map1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *pnts)
{
    DEBUG_GLCMDS(("Map1f: %04x %f %f %d %d\n", (int)target, u1, u2,
		  (int)stride, (int)order));
}

void _gamma_Map2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustr, GLint uord, GLdouble v1, GLdouble v2, GLint vstr, GLint vord, const GLdouble *pnts)
{
    DEBUG_GLCMDS(("Map2d: %04x %f %f %d %d %f %f %d %d\n",
		   (int)target,
		   u1, u2, (int)ustr, (int)uord,
		   v1, v2, (int)vstr, (int)vord));
}

void _gamma_Map2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustr, GLint uord, GLfloat v1, GLfloat v2, GLint vstr, GLint vord, const GLfloat *pnts)
{
    DEBUG_GLCMDS(("Map2f: %04x %f %f %d %d %f %f %d %d\n",
		   (int)target,
		   u1, u2, (int)ustr, (int)uord,
		   v1, v2, (int)vstr, (int)vord));
}

void _gamma_MapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
    DEBUG_GLCMDS(("MapGrid1d: %d %f %f\n", (int)un, u1, u2));
}

void _gamma_MapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
    DEBUG_GLCMDS(("MapGrid1f: %d %f %f\n", (int)un, u1, u2));
}

void _gamma_MapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    DEBUG_GLCMDS(("MapGrid2d: %d %f %f %d %f %f\n",
		  (int)un, u1, u2,
		  (int)vn, v1, v2));
}

void _gamma_MapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    DEBUG_GLCMDS(("MapGrid2f: %d %f %f %d %f %f\n",
		  (int)un, u1, u2,
		  (int)vn, v1, v2));
}

void _gamma_Materialf(GLenum face, GLenum pname, GLfloat param)
{
    DEBUG_GLCMDS(("Materialf: %04x %04x %f\n", (int)face, (int)pname, param));

    _gamma_Materialfv(face, pname, &param);
}

void _gamma_Materialfv(GLenum face, GLenum pname, const GLfloat *params)
{
    DEBUG_GLCMDS(("Materialfv: %04x %04x %f\n",
		  (int)face, (int)pname, *params));

    gCCPriv->MaterialMode |= MaterialModeEnable;
    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, MaterialMode, gCCPriv->MaterialMode);

    if ((face == GL_FRONT) || (face == GL_FRONT_AND_BACK)) {
	switch (pname) {
	case GL_AMBIENT:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	    WRITEF(gCCPriv->buf, FrontAmbientColorBlue, params[2]);
	    WRITEF(gCCPriv->buf, FrontAmbientColorGreen, params[1]);
	    WRITEF(gCCPriv->buf, FrontAmbientColorRed, params[0]);
	    break;
	case GL_DIFFUSE:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 4);
	    WRITEF(gCCPriv->buf, FrontAlpha, params[3]);
	    WRITEF(gCCPriv->buf, FrontDiffuseColorBlue, params[2]);
	    WRITEF(gCCPriv->buf, FrontDiffuseColorGreen, params[1]);
	    WRITEF(gCCPriv->buf, FrontDiffuseColorRed, params[0]);
	    break;
	case GL_SPECULAR:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	    WRITEF(gCCPriv->buf, FrontSpecularColorBlue, params[2]);
	    WRITEF(gCCPriv->buf, FrontSpecularColorGreen, params[1]);
	    WRITEF(gCCPriv->buf, FrontSpecularColorRed, params[0]);
	    break;
	case GL_EMISSION:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	    WRITEF(gCCPriv->buf, FrontEmissiveColorBlue, params[2]);
	    WRITEF(gCCPriv->buf, FrontEmissiveColorGreen, params[1]);
	    WRITEF(gCCPriv->buf, FrontEmissiveColorRed, params[0]);
	    break;
	case GL_SHININESS:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITEF(gCCPriv->buf, FrontSpecularExponent, params[0]);
	    break;
	case GL_AMBIENT_AND_DIFFUSE:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 7);
	    WRITEF(gCCPriv->buf, FrontAmbientColorBlue, params[2]);
	    WRITEF(gCCPriv->buf, FrontAmbientColorGreen, params[1]);
	    WRITEF(gCCPriv->buf, FrontAmbientColorRed, params[0]);
	    WRITEF(gCCPriv->buf, FrontAlpha, params[3]);
	    WRITEF(gCCPriv->buf, FrontDiffuseColorBlue, params[2]);
	    WRITEF(gCCPriv->buf, FrontDiffuseColorGreen, params[1]);
	    WRITEF(gCCPriv->buf, FrontDiffuseColorRed, params[0]);
	    break;
	case GL_COLOR_INDEXES:
	    /* NOT_DONE */
	    break;
	}
    }

    if ((face == GL_BACK) || (face == GL_FRONT_AND_BACK)) {
	switch (pname) {
	case GL_AMBIENT:
	    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	WRITEF(gCCPriv->buf, BackAmbientColorBlue, params[2]);
	WRITEF(gCCPriv->buf, BackAmbientColorGreen, params[1]);
	WRITEF(gCCPriv->buf, BackAmbientColorRed, params[0]);
	break;
    case GL_DIFFUSE:
	CHECK_DMA_BUFFER(gCC, gCCPriv, 4);
	WRITEF(gCCPriv->buf, BackAlpha, params[3]);
	WRITEF(gCCPriv->buf, BackDiffuseColorBlue, params[2]);
	WRITEF(gCCPriv->buf, BackDiffuseColorGreen, params[1]);
	WRITEF(gCCPriv->buf, BackDiffuseColorRed, params[0]);
	break;
    case GL_SPECULAR:
	CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	WRITEF(gCCPriv->buf, BackSpecularColorBlue, params[2]);
	WRITEF(gCCPriv->buf, BackSpecularColorGreen, params[1]);
	WRITEF(gCCPriv->buf, BackSpecularColorRed, params[0]);
	break;
    case GL_EMISSION:
	CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
	WRITEF(gCCPriv->buf, BackEmissiveColorBlue, params[2]);
	WRITEF(gCCPriv->buf, BackEmissiveColorGreen, params[1]);
	WRITEF(gCCPriv->buf, BackEmissiveColorRed, params[0]);
	break;
    case GL_SHININESS:
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITEF(gCCPriv->buf, BackSpecularExponent, params[0]);
	break;
    case GL_AMBIENT_AND_DIFFUSE:
	CHECK_DMA_BUFFER(gCC, gCCPriv, 7);
	WRITEF(gCCPriv->buf, BackAmbientColorBlue, params[2]);
	WRITEF(gCCPriv->buf, BackAmbientColorGreen, params[1]);
	WRITEF(gCCPriv->buf, BackAmbientColorRed, params[0]);
	WRITEF(gCCPriv->buf, BackAlpha, params[3]);
	WRITEF(gCCPriv->buf, BackDiffuseColorBlue, params[2]);
	WRITEF(gCCPriv->buf, BackDiffuseColorGreen, params[1]);
	WRITEF(gCCPriv->buf, BackDiffuseColorRed, params[0]);
	break;
    case GL_COLOR_INDEXES:
	/* NOT_DONE */
	break;
    }
    }
}

void _gamma_Materiali(GLenum face, GLenum pname, GLint param)
{
    DEBUG_GLCMDS(("Materiali: %04x %04x %d\n",
		  (int)face, (int)pname, (int)param));
}

void _gamma_Materialiv(GLenum face, GLenum pname, const GLint *params)
{
    DEBUG_GLCMDS(("Materialiv: %04x %04x %d\n",
		  (int)face, (int)pname, (int)*params));
}

void _gamma_MatrixMode(GLenum mode)
{
    DEBUG_GLCMDS(("MatrixMode: %04x\n", (int)mode));

    switch (mode) {
    case GL_TEXTURE:
	/* Enable the Texture transform */
	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	WRITE(gCCPriv->buf, TransformModeOr, XM_XformTexture);
    case GL_MODELVIEW:
    case GL_PROJECTION:
        gCCPriv->MatrixMode = mode;
	break;
    default:
	/* ERROR!!! */
	break;
    }
}

void _gamma_MultMatrixd(const GLdouble *m)
{
    GLfloat f[16];
    int i;

    DEBUG_GLCMDS(("MatrixMultd\n"));

    for (i = 0; i < 16; i++) f[i] = m[i];
    gammaMultMatrix(f);
    gammaLoadHWMatrix();
}

void _gamma_MultMatrixf(const GLfloat *m)
{
    DEBUG_GLCMDS(("MatrixMultf\n"));

    gammaMultMatrix((GLfloat *)m);
    gammaLoadHWMatrix();
}

#if 0 /* Now in dlist.c */
void _gamma_NewList(GLuint list, GLenum mode)
{
    DEBUG_GLCMDS(("NewList: %d %04x\n", (unsigned int)list, (int)mode));
}
#endif

void _gamma_Normal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
    DEBUG_GLCMDS(("Normal3b: %d %d %d\n", nx, ny, nz));

    _gamma_Normal3f((GLfloat)nx,(GLfloat)ny,(GLfloat)nz);
}

void _gamma_Normal3bv(const GLbyte *v)
{
    DEBUG_GLCMDS(("Normal3bv: %d %d %d\n", v[0], v[1], v[2]));

    _gamma_Normal3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_Normal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
    DEBUG_GLCMDS(("Normal3d: %f %f %f\n", nx, ny, nz));

    _gamma_Normal3f((GLfloat)nx,(GLfloat)ny,(GLfloat)nz);
}

void _gamma_Normal3dv(const GLdouble *v)
{
    DEBUG_GLCMDS(("Normal3dv: %f %f %f\n", v[0], v[1], v[2]));

    _gamma_Normal3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_Normal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    DEBUG_GLCMDS(("Normal3f: %f %f %f\n", nx, ny, nz));

    gCCPriv->Current.Normal[0] = nx;
    gCCPriv->Current.Normal[1] = ny;
    gCCPriv->Current.Normal[2] = nz;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
    WRITEF(gCCPriv->buf, Nz, nz);
    WRITEF(gCCPriv->buf, Ny, ny);
    WRITEF(gCCPriv->buf, Nx, nx);
}

void _gamma_Normal3fv(const GLfloat *v)
{
    DEBUG_GLCMDS(("Normal3fv: %f %f %f\n", v[0], v[1], v[2]));

    _gamma_Normal3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_Normal3i(GLint nx, GLint ny, GLint nz)
{
    DEBUG_GLCMDS(("Normal3i: %d %d %d\n", (int)nx, (int)ny, (int)nz));

    _gamma_Normal3f((GLfloat)nx,(GLfloat)ny,(GLfloat)nz);
}

void _gamma_Normal3iv(const GLint *v)
{
    DEBUG_GLCMDS(("Normal3iv: %d %d %d\n", (int)v[0], (int)v[1], (int)v[2]));

    _gamma_Normal3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_Normal3s(GLshort nx, GLshort ny, GLshort nz)
{
    DEBUG_GLCMDS(("Normal3s: %d %d %d\n", nx, ny, nz));

    _gamma_Normal3f((GLfloat)nx,(GLfloat)ny,(GLfloat)nz);
}

void _gamma_Normal3sv(const GLshort *v)
{
    DEBUG_GLCMDS(("Normal3sv: %d %d %d\n", v[0], v[1], v[2]));

    _gamma_Normal3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_NormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
    DEBUG_GLCMDS(("NormalPointer: %04x %d\n", (int)type, (int)stride));

   if (stride<0) {
      gamma_error( GL_INVALID_VALUE, "glNormalPointer(stride)" );
      return;
   }
   switch (type) {
      case GL_BYTE:
         gCCPriv->Array.NormalStrideB = stride ? stride : 3*sizeof(GLbyte);
         break;
      case GL_SHORT:
         gCCPriv->Array.NormalStrideB = stride ? stride : 3*sizeof(GLshort);
         break;
      case GL_INT:
         gCCPriv->Array.NormalStrideB = stride ? stride : 3*sizeof(GLint);
         break;
      case GL_FLOAT:
         gCCPriv->Array.NormalStrideB = stride ? stride : 3*sizeof(GLfloat);
         break;
      case GL_DOUBLE:
         gCCPriv->Array.NormalStrideB = stride ? stride : 3*sizeof(GLdouble);
         break;
      default:
         gamma_error( GL_INVALID_ENUM, "glNormalPointer(type)" );
         return;
   }
   gCCPriv->Array.NormalType = type;
   gCCPriv->Array.NormalStride = stride;
   gCCPriv->Array.NormalPtr = (void *) pointer;
}

void _gamma_Ortho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
#define ORTHO_X() ((GLfloat)( 2.0/(right-left)))
#define ORTHO_Y() ((GLfloat)( 2.0/(top-bottom)))
#define ORTHO_Z() ((GLfloat)(-2.0/(zFar-zNear)))

#define ORTHO_TX() ((GLfloat)(-((right+left)/(right-left))))
#define ORTHO_TY() ((GLfloat)(-((top+bottom)/(top-bottom))))
#define ORTHO_TZ() ((GLfloat)(-((zFar+zNear)/(zFar-zNear))))

    GLfloat m[16];
    int i;

    DEBUG_GLCMDS(("Ortho: %f %f %f %f %f %f\n",
		  left, right, bottom, top, zNear, zFar));

    for (i = 0; i < 16; i++) m[i] = 0.0;

    m[0]  = ORTHO_X();
    m[5]  = ORTHO_Y();
    m[10] = ORTHO_Z();
    m[12] = ORTHO_TX();
    m[13] = ORTHO_TY();
    m[14] = ORTHO_TZ();
    m[15] = 1.0;

    gammaMultMatrix(m);
    gammaLoadHWMatrix();
}

void _gamma_PassThrough(GLfloat token)
{
    DEBUG_GLCMDS(("PassThrough: %f\n", token));
}

void _gamma_PixelMapfv(GLenum map, GLint mapsize, const GLfloat *values)
{
    DEBUG_GLCMDS(("PixelMapfv: %04x %d\n", (int)map, (int)mapsize));
}

void _gamma_PixelMapuiv(GLenum map, GLint mapsize, const GLuint *values)
{
    DEBUG_GLCMDS(("PixelMapiv: %04x %d\n", (int)map, (int)mapsize));
}

void _gamma_PixelMapusv(GLenum map, GLint mapsize, const GLushort *values)
{
    DEBUG_GLCMDS(("PixelMapusv: %04x %d\n", (int)map, (int)mapsize));
}

void _gamma_PixelStoref(GLenum pname, GLfloat param)
{
    DEBUG_GLCMDS(("PixelStoref: %04x %f\n", (int)pname, param));
}

void _gamma_PixelStorei(GLenum pname, GLint param)
{
    DEBUG_GLCMDS(("PixelStorei: %04x %d\n", (int)pname, (int)param));
}

void _gamma_PixelTransferf(GLenum pname, GLfloat param)
{
    DEBUG_GLCMDS(("PixelTransferf: %04x %f\n", (int)pname, param));
}

void _gamma_PixelTransferi(GLenum pname, GLint param)
{
    DEBUG_GLCMDS(("PixelTransferi: %04x %d\n", (int)pname, (int)param));
}

void _gamma_PixelZoom(GLfloat xfactor, GLfloat yfactor)
{
    DEBUG_GLCMDS(("PixelZoom: %f %f\n", xfactor, yfactor));
}

void _gamma_PointSize(GLfloat size)
{
    unsigned char s = size;

    DEBUG_GLCMDS(("PointSize: %f\n", size));

    /* NOT_DONE: Needs to handle AAPoints also */
    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, PointSize, s);
}

void _gamma_PolygonMode(GLenum face, GLenum mode)
{
    DEBUG_GLCMDS(("PolygonMode: %04x %04x\n", (int)face, (int)mode));

    gCCPriv->GeometryMode &= ~GM_FB_PolyMask;

    switch (mode) {
    case GL_FILL:
	switch (face) {
	case GL_FRONT:
	    gCCPriv->GeometryMode |= GM_FrontPolyFill;
	    break;
	case GL_BACK:
	    gCCPriv->GeometryMode |= GM_BackPolyFill;
	    break;
	case GL_FRONT_AND_BACK:
	    gCCPriv->GeometryMode |= GM_FrontPolyFill;
	    gCCPriv->GeometryMode |= GM_BackPolyFill;
	    break;
	}
	break;
    case GL_LINE:
	switch (face) {
	case GL_FRONT:
	    gCCPriv->GeometryMode |= GM_FrontPolyLine;
	    break;
	case GL_BACK:
	    gCCPriv->GeometryMode |= GM_BackPolyLine;
	    break;
	case GL_FRONT_AND_BACK:
	    gCCPriv->GeometryMode |= GM_FrontPolyLine;
	    gCCPriv->GeometryMode |= GM_BackPolyLine;
	    break;
	}
	break;
    case GL_POINT:
	switch (face) {
	case GL_FRONT:
	    gCCPriv->GeometryMode |= GM_FrontPolyPoint;
	    break;
	case GL_BACK:
	    gCCPriv->GeometryMode |= GM_BackPolyPoint;
	    break;
	case GL_FRONT_AND_BACK:
	    gCCPriv->GeometryMode |= GM_FrontPolyPoint;
	    gCCPriv->GeometryMode |= GM_BackPolyPoint;
	    break;
	}
	break;
    }

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, GeometryMode, gCCPriv->GeometryMode);
}

void _gamma_PolygonOffset(GLfloat factor, GLfloat units)
{
    DEBUG_GLCMDS(("PolygonOffset: %f %f\n", factor, units));
}

void _gamma_PolygonStipple(const GLubyte *mask)
{
    DEBUG_GLCMDS(("PolygonStipple: \n"));
}

void _gamma_PopAttrib(void)
{
    DEBUG_GLCMDS(("PopAttrib\n"));
}

void _gamma_PopClientAttrib(void)
{
    DEBUG_GLCMDS(("PopClientAttrib\n"));
}

void _gamma_PopMatrix(void)
{
    DEBUG_GLCMDS(("PopMatrix: %04x\n", gCCPriv->MatrixMode));

    switch (gCCPriv->MatrixMode) {
    case GL_TEXTURE:
	if (gCCPriv->TextureCount == 0) {
	    /* ERROR!!! */
	} else {
	    gCCPriv->TextureCount--;
	    memcpy(gCCPriv->Texture,
		   &gCCPriv->TextureStack[gCCPriv->TextureCount*16],
		   16*sizeof(*gCCPriv->Texture));
	    gammaLoadHWMatrix();
	}
	break;
    case GL_MODELVIEW:
	if (gCCPriv->ModelViewCount == 0) {
	    /* ERROR!!! */
	} else {
	    gCCPriv->ModelViewCount--;
	    memcpy(gCCPriv->ModelView,
		   &gCCPriv->ModelViewStack[gCCPriv->ModelViewCount*16],
		   16*sizeof(*gCCPriv->ModelView));
	    gammaLoadHWMatrix();
	}
	break;
    case GL_PROJECTION:
	if (gCCPriv->ProjCount == 0) {
	    /* ERROR!!! */
	} else {
	    gCCPriv->ProjCount--;
	    memcpy(gCCPriv->Proj,
		   &gCCPriv->ProjStack[gCCPriv->ProjCount*16],
		   16*sizeof(*gCCPriv->Proj));
	    gammaLoadHWMatrix();
	}
	break;
    default:
	/* ERROR!!! */
	break;
    }
}

void _gamma_PopName(void)
{
    DEBUG_GLCMDS(("PopName\n"));
}

void _gamma_PrioritizeTextures(GLsizei n, const GLuint *textures, const GLclampf *priorities)
{
    DEBUG_GLCMDS(("PrioritizeTextures: %d\n", (int)n));
}

void _gamma_PushAttrib(GLbitfield mask)
{
    DEBUG_GLCMDS(("PushAttrib: %04x\n", (int)mask));
}

void _gamma_PushClientAttrib(GLuint mask)
{
    DEBUG_GLCMDS(("PushClientAttrib: %04x\n", (unsigned int)mask));
}

void _gamma_PushMatrix(void)
{
    DEBUG_GLCMDS(("PushMatrix: %04x\n", gCCPriv->MatrixMode));

    switch (gCCPriv->MatrixMode) {
    case GL_TEXTURE:
	if (gCCPriv->TextureCount >= MAX_TEXTURE_STACK-1) {
	    /* ERROR!!! */
	} else {
	    memcpy(&gCCPriv->TextureStack[gCCPriv->TextureCount*16],
		   gCCPriv->Texture,
		   16*sizeof(*gCCPriv->Texture));
	    gCCPriv->TextureCount++;
	}
	break;
    case GL_MODELVIEW:
	if (gCCPriv->ModelViewCount >= MAX_MODELVIEW_STACK-1) {
	    /* ERROR!!! */
	} else {
	    memcpy(&gCCPriv->ModelViewStack[gCCPriv->ModelViewCount*16],
		   gCCPriv->ModelView,
		   16*sizeof(*gCCPriv->ModelView));
	    gCCPriv->ModelViewCount++;
	}
	break;
    case GL_PROJECTION:
	if (gCCPriv->ProjCount >= MAX_PROJECTION_STACK-1) {
	    /* ERROR!!! */
	} else {
	    memcpy(&gCCPriv->ProjStack[gCCPriv->ProjCount*16],
		   gCCPriv->Proj,
		   16*sizeof(*gCCPriv->Proj));
	    gCCPriv->ProjCount++;
	}
	break;
    default:
	/* ERROR!!! */
	break;
    }
}

void _gamma_PushName(GLuint name)
{
    DEBUG_GLCMDS(("PushName: %d\n", (int)name));
}

void _gamma_RasterPos2d(GLdouble x, GLdouble y)
{
    DEBUG_GLCMDS(("RasterPos2d: %f %f\n", x, y));

    _gamma_RasterPos2f((GLfloat)x,(GLfloat)y);
}

void _gamma_RasterPos2dv(const GLdouble *v)
{
    _gamma_RasterPos2f((GLfloat)v[0],(GLfloat)v[1]);
}

void _gamma_RasterPos2f(GLfloat x, GLfloat y)
{
    DEBUG_GLCMDS(("RasterPos2f: %f %f\n", x, y));

    gCCPriv->Current.RasterPos[0] = x;
    gCCPriv->Current.RasterPos[1] = y;
    gCCPriv->Current.RasterPos[2] = 0.0f;
    gCCPriv->Current.RasterPos[3] = 1.0f;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
    WRITEF(gCCPriv->buf, RPy,  y);
    WRITEF(gCCPriv->buf, RPx2, x);
}

void _gamma_RasterPos2fv(const GLfloat *v)
{
    _gamma_RasterPos2f(v[0],v[1]);
}

void _gamma_RasterPos2i(GLint x, GLint y)
{
    _gamma_RasterPos2f((GLfloat)x,(GLfloat)y);
}

void _gamma_RasterPos2iv(const GLint *v)
{
}

void _gamma_RasterPos2s(GLshort x, GLshort y)
{
}

void _gamma_RasterPos2sv(const GLshort *v)
{
}

void _gamma_RasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
}

void _gamma_RasterPos3dv(const GLdouble *v)
{
}

void _gamma_RasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
}

void _gamma_RasterPos3fv(const GLfloat *v)
{
}

void _gamma_RasterPos3i(GLint x, GLint y, GLint z)
{
}

void _gamma_RasterPos3iv(const GLint *v)
{
}

void _gamma_RasterPos3s(GLshort x, GLshort y, GLshort z)
{
}

void _gamma_RasterPos3sv(const GLshort *v)
{
}

void _gamma_RasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
}

void _gamma_RasterPos4dv(const GLdouble *v)
{
}

void _gamma_RasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
}

void _gamma_RasterPos4fv(const GLfloat *v)
{
}

void _gamma_RasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
}

void _gamma_RasterPos4iv(const GLint *v)
{
}

void _gamma_RasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
}

void _gamma_RasterPos4sv(const GLshort *v)
{
}

void _gamma_ReadBuffer(GLenum mode)
{
    DEBUG_GLCMDS(("ReadBuffer: %04x\n", (int)mode));
}

void _gamma_ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)
{
    DEBUG_GLCMDS(("ReadPixels: %d %d %d %d %04x %04x\n", (int)x, (int)y,
		  (int)width, (int)height, (int)format, (int)type));
}

void _gamma_Rectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    DEBUG_GLCMDS(("Rectd: %f %f %f %f\n", x1, y1, x2, y2));
}

void _gamma_Rectdv(const GLdouble *v1, const GLdouble *v2)
{
    DEBUG_GLCMDS(("Rectdv: %f %f %f %f\n", v1[0], v1[1], v2[0], v2[1]));
}

void _gamma_Rectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    DEBUG_GLCMDS(("Rectf: %f %f %f %f\n", x1, y1, x2, y2));
 
    /* This should be done with Gamma's Rectangle engine - later */

    _gamma_Begin(GL_POLYGON);
    _gamma_Vertex2f(x1,y1);
    _gamma_Vertex2f(x2,y1);
    _gamma_Vertex2f(x2,y2);
    _gamma_Vertex2f(x1,y2);
    _gamma_End();
}

void _gamma_Rectfv(const GLfloat *v1, const GLfloat *v2)
{
    DEBUG_GLCMDS(("Rectfv: %f %f %f %f\n", v1[0], v1[1], v2[0], v2[1]));
}

void _gamma_Recti(GLint x1, GLint y1, GLint x2, GLint y2)
{
    DEBUG_GLCMDS(("Recti: %d %d %d %d\n", (int)x1, (int)y1, (int)x2, (int)y2));
}

void _gamma_Rectiv(const GLint *v1, const GLint *v2)
{
    DEBUG_GLCMDS(("Rectiv: %d %d %d %d\n",
		  (int)v1[0], (int)v1[1], (int)v2[0], (int)v2[1]));
}

void _gamma_Rects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    DEBUG_GLCMDS(("Rects: %d %d %d %d\n", x1, y1, x2, y2));
}

void _gamma_Rectsv(const GLshort *v1, const GLshort *v2)
{
    DEBUG_GLCMDS(("Rectsv: %d %d %d %d\n", v1[0], v1[1], v2[0], v2[1]));
}

GLint _gamma_RenderMode(GLenum mode)
{
    DEBUG_GLCMDS(("RenderMode: %04x\n", (int)mode));

    return GL_TRUE;
}

void _gamma_Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat m[16];
    GLfloat l, c, s;

    DEBUG_GLCMDS(("Rotated: %f %f %f %f\n", angle, x, y, z));

    /* Normalize <x,y,z> */
    l = sqrt(x*x + y*y + z*z);
    x /= l;
    y /= l;
    z /= l;

    c = cos(angle * M_PI/180.0);
    s = sin(angle * M_PI/180.0);

    m[0]  = x * x * (1 - c) + c;
    m[1]  = y * x * (1 - c) + z * s;
    m[2]  = x * z * (1 - c) - y * s;
    m[3]  = 0.0;
    m[4]  = x * y * (1 - c) - z * s;
    m[5]  = y * y * (1 - c) + c;
    m[6]  = y * z * (1 - c) + x * s;
    m[7]  = 0.0;
    m[8]  = x * z * (1 - c) + y * s;
    m[9]  = y * z * (1 - c) - x * s;
    m[10] = z * z * (1 - c) + c;
    m[11] = 0.0;
    m[12] = 0.0;
    m[13] = 0.0;
    m[14] = 0.0;
    m[15] = 1.0;

    gammaMultMatrix(m);
    gammaLoadHWMatrix();
}

void _gamma_Rotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat m[16];
    GLfloat l, c, s;

    DEBUG_GLCMDS(("Rotatef: %f %f %f %f\n", angle, x, y, z));

    /* Normalize <x,y,z> */
    l = sqrt(x*x + y*y + z*z);
    x /= l;
    y /= l;
    z /= l;

    c = cos((double)angle * M_PI/180.0);
    s = sin((double)angle * M_PI/180.0);

    m[0]  = x * x * (1 - c) + c;
    m[1]  = y * x * (1 - c) + z * s;
    m[2]  = x * z * (1 - c) - y * s;
    m[3]  = 0.0;
    m[4]  = x * y * (1 - c) - z * s;
    m[5]  = y * y * (1 - c) + c;
    m[6]  = y * z * (1 - c) + x * s;
    m[7]  = 0.0;
    m[8]  = x * z * (1 - c) + y * s;
    m[9]  = y * z * (1 - c) - x * s;
    m[10] = z * z * (1 - c) + c;
    m[11] = 0.0;
    m[12] = 0.0;
    m[13] = 0.0;
    m[14] = 0.0;
    m[15] = 1.0;

    gammaMultMatrix(m);
    gammaLoadHWMatrix();
}

void _gamma_Scaled(GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat m[16];
    int i;

    DEBUG_GLCMDS(("scaled: %f %f %f\n", x, y, z));

    for (i = 0; i < 16; i++) m[i] = 0.0;

    m[0]  = x;
    m[5]  = y;
    m[10] = z;
    m[15] = 1.0;

    gammaMultMatrix(m);
    gammaLoadHWMatrix();
}

void _gamma_Scalef(GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat m[16];
    int i;

    DEBUG_GLCMDS(("scalef: %f %f %f\n", x, y, z));

    for (i = 0; i < 16; i++) m[i] = 0.0;

    m[0]  = x;
    m[5]  = y;
    m[10] = z;
    m[15] = 1.0;

    gammaMultMatrix(m);
    gammaLoadHWMatrix();
}

void _gamma_Scissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    GLint x1, y1, x2, y2;

    DEBUG_GLCMDS(("Scissor: %d %d %d %d\n",
		  (int)x, (int)y, (int)width, (int)height));

    x1 = gCC->driDrawablePriv->x + x;
    y1 = gCC->driScreenPriv->fbHeight -
	(gCC->driDrawablePriv->y +
	 gCC->driDrawablePriv->h) + y;
    x2 = x1 + width;
    y2 = y1 + height;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
    WRITE(gCCPriv->buf, ScissorMinXY,  (y1&0xffff)<<16 | (x1&0xffff));
    WRITE(gCCPriv->buf, ScissorMaxXY,  (y2&0xffff)<<16 | (x2&0xffff));
}

void _gamma_SelectBuffer(GLsizei numnames, GLuint *buffer)
{
    DEBUG_GLCMDS(("SelectBuffer: %d\n", (int)numnames));
}

void _gamma_ShadeModel(GLenum mode)
{
    DEBUG_GLCMDS(("ShadeModel: %04x\n", (int)mode));

    gCCPriv->GeometryMode &= ~GM_ShadingMask;
    gCCPriv->ColorDDAMode &= ~ColorDDAShadingMask;

    switch (mode) {
    case GL_FLAT:
	gCCPriv->ColorDDAMode |= ColorDDAFlat;
	gCCPriv->GeometryMode |= GM_FlatShading;
	break;
    case GL_SMOOTH:
	gCCPriv->ColorDDAMode |= ColorDDAGouraud;
	gCCPriv->GeometryMode |= GM_GouraudShading;
	break;
    default:
	/* ERROR!!! */
	break;
    }

    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
    WRITE(gCCPriv->buf, GeometryMode, gCCPriv->GeometryMode);
    WRITE(gCCPriv->buf, ColorDDAMode, gCCPriv->ColorDDAMode);
}

void _gamma_StencilFunc(GLenum func, GLint ref, GLuint mask)
{
    DEBUG_GLCMDS(("StencilFunc: %04x %d %d\n",
		  (int)func, (int)ref, (unsigned int)mask));
}

void _gamma_StencilMask(GLuint mask)
{
    DEBUG_GLCMDS(("StencilMask: %d\n", (unsigned int)mask));
}

void _gamma_StencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    DEBUG_GLCMDS(("StencilOp: %04x %04x %04x\n",
		  (int)fail, (int)zfail, (int)zpass));
}

void _gamma_TexCoord1d(GLdouble s)
{
    DEBUG_GLCMDS(("TexCoord1d: %f\n", s));

    _gamma_TexCoord1f((GLfloat)s);
}

void _gamma_TexCoord1dv(const GLdouble *v)
{
    DEBUG_GLCMDS(("TexCoord1dv: %f\n", *v));

    _gamma_TexCoord1fv((GLfloat*)v);
}

void _gamma_TexCoord1f(GLfloat s)
{
    DEBUG_GLCMDS(("TexCoord1f: %f\n", s));

    gCCPriv->Current.TexCoord[0] = s;
    gCCPriv->Current.TexCoord[1] = 0.0f;
    gCCPriv->Current.TexCoord[2] = 0.0f;
    gCCPriv->Current.TexCoord[3] = 1.0f;

    WRITEF(gCCPriv->buf, Ts1, s);
}

void _gamma_TexCoord1fv(const GLfloat *v)
{
    DEBUG_GLCMDS(("TexCoord1fv: %f\n", *v));

    gCCPriv->Current.TexCoord[0] = v[0];
    gCCPriv->Current.TexCoord[1] = 0.0f;
    gCCPriv->Current.TexCoord[2] = 0.0f;
    gCCPriv->Current.TexCoord[3] = 1.0f;

    WRITEF(gCCPriv->buf, Ts1, v[0]);
}

void _gamma_TexCoord1i(GLint s)
{
    GLfloat x;

    DEBUG_GLCMDS(("TexCoord1i: %d\n", (int)s));

    x = INT_TO_FLOAT(s);

    _gamma_TexCoord1f(x);
}

void _gamma_TexCoord1iv(const GLint *v)
{
    GLfloat p[1];

    DEBUG_GLCMDS(("TexCoord1iv: %d\n", (int)*v));

    p[0] = INT_TO_FLOAT(v[0]);

    _gamma_TexCoord1fv(p);
}

void _gamma_TexCoord1s(GLshort s)
{
    GLfloat x;

    DEBUG_GLCMDS(("TexCoord1s: %d\n", s));

    x = SHORT_TO_FLOAT(s);

    _gamma_TexCoord1f(x);
}

void _gamma_TexCoord1sv(const GLshort *v)
{
    GLfloat p[1];

    DEBUG_GLCMDS(("TexCoord1sv: %d\n", *v));

    p[0] = SHORT_TO_FLOAT(v[0]);

    _gamma_TexCoord1fv(p);
}

void _gamma_TexCoord2d(GLdouble s, GLdouble t)
{
    DEBUG_GLCMDS(("TexCoord2d: %f %f\n", s, t));

    _gamma_TexCoord2f((GLfloat)s,(GLfloat)t);
}

void _gamma_TexCoord2dv(const GLdouble *v)
{
    DEBUG_GLCMDS(("TexCoord2dv: %f %f\n", v[0], v[1]));

    _gamma_TexCoord2fv((GLfloat*)v);
}

void _gamma_TexCoord2f(GLfloat s, GLfloat t)
{
    DEBUG_GLCMDS(("TexCoord2f: %f %f\n", s, t));

    gCCPriv->Current.TexCoord[0] = s;
    gCCPriv->Current.TexCoord[1] = t;
    gCCPriv->Current.TexCoord[2] = 0.0f;
    gCCPriv->Current.TexCoord[3] = 1.0f;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
    WRITEF(gCCPriv->buf, Tt2, t);
    WRITEF(gCCPriv->buf, Ts2, s);
}

void _gamma_TexCoord2fv(const GLfloat *v)
{
    DEBUG_GLCMDS(("TexCoord2fv: %f %f\n", v[0], v[1]));

    gCCPriv->Current.TexCoord[0] = v[0];
    gCCPriv->Current.TexCoord[1] = v[1];
    gCCPriv->Current.TexCoord[2] = 0.0f;
    gCCPriv->Current.TexCoord[3] = 1.0f;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
    WRITEF(gCCPriv->buf, Tt2, v[1]);
    WRITEF(gCCPriv->buf, Ts2, v[0]);
}

void _gamma_TexCoord2i(GLint s, GLint t)
{
    GLfloat x,y;

    DEBUG_GLCMDS(("TexCoord2i: %d %d\n", (int)s, (int)t));

    x = INT_TO_FLOAT(s);
    y = INT_TO_FLOAT(t);

    _gamma_TexCoord2f(x,y);
}

void _gamma_TexCoord2iv(const GLint *v)
{
    GLfloat p[2];

    DEBUG_GLCMDS(("TexCoord2iv: %d %d\n", (int)v[0], (int)v[1]));

    p[0] = INT_TO_FLOAT(v[0]);
    p[1] = INT_TO_FLOAT(v[1]);

    _gamma_TexCoord2fv(p);
}

void _gamma_TexCoord2s(GLshort s, GLshort t)
{
    GLfloat x,y;

    DEBUG_GLCMDS(("TexCoord2s: %d %d\n", s, t));

    x = SHORT_TO_FLOAT(s);
    y = SHORT_TO_FLOAT(t);

    _gamma_TexCoord2f(x,y);
}

void _gamma_TexCoord2sv(const GLshort *v)
{
    GLfloat p[2];

    DEBUG_GLCMDS(("TexCoord2sv: %d %d\n", v[0], v[1]));

    p[0] = SHORT_TO_FLOAT(v[0]);
    p[1] = SHORT_TO_FLOAT(v[1]);

    _gamma_TexCoord2fv(p);
}

void _gamma_TexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
    DEBUG_GLCMDS(("TexCoord3d: %f %f %f\n", s, t, r));

    _gamma_TexCoord3f((GLfloat)s,(GLfloat)t,(GLfloat)r);
}

void _gamma_TexCoord3dv(const GLdouble *v)
{
    DEBUG_GLCMDS(("TexCoord3dv: %f %f %f\n", v[0], v[1], v[2]));

    _gamma_TexCoord3fv((GLfloat*)v);
}

void _gamma_TexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
    DEBUG_GLCMDS(("TexCoord3f: %f %f %f\n", s, t, r));

    _gamma_TexCoord4f(s,t,r,1.0f);
}

void _gamma_TexCoord3fv(const GLfloat *v)
{
    GLfloat p[4];

    DEBUG_GLCMDS(("TexCoord3fv: %f %f %f\n", v[0], v[1], v[2]));

    p[0] = v[0];
    p[1] = v[1];
    p[2] = v[2];
    p[3] = 1.0f;

    _gamma_TexCoord4fv(p);
}

void _gamma_TexCoord3i(GLint s, GLint t, GLint r)
{
    GLfloat x,y,z;

    DEBUG_GLCMDS(("TexCoord3i: %d %d %d\n", (int)s, (int)t, (int)r));

    x = INT_TO_FLOAT(s);
    y = INT_TO_FLOAT(t);
    z = INT_TO_FLOAT(r);

    _gamma_TexCoord4f(x,y,z,1.0f);
}

void _gamma_TexCoord3iv(const GLint *v)
{
    GLfloat p[4];

    DEBUG_GLCMDS(("TexCoord3iv: %d %d %d\n", (int)v[0], (int)v[1], (int)v[2]));

    p[0] = INT_TO_FLOAT(v[0]);
    p[1] = INT_TO_FLOAT(v[1]);
    p[2] = INT_TO_FLOAT(v[2]);
    p[3] = 1.0f;

    _gamma_TexCoord4fv(p);
}

void _gamma_TexCoord3s(GLshort s, GLshort t, GLshort r)
{
    GLfloat x,y,z;

    DEBUG_GLCMDS(("TexCoord3s: %d %d %d\n", s, t, r));
  
    x = SHORT_TO_FLOAT(s);
    y = SHORT_TO_FLOAT(t);
    z = SHORT_TO_FLOAT(r);

    _gamma_TexCoord4f(x,y,z,1.0f);
}

void _gamma_TexCoord3sv(const GLshort *v)
{
    GLfloat p[4];

    DEBUG_GLCMDS(("TexCoord3sv: %d %d %d\n", v[0], v[1], v[2]));

    p[0] = SHORT_TO_FLOAT(v[0]);
    p[1] = SHORT_TO_FLOAT(v[1]);
    p[2] = SHORT_TO_FLOAT(v[2]);
    p[3] = 1.0f;

    _gamma_TexCoord4fv(p);
}

void _gamma_TexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    DEBUG_GLCMDS(("TexCoord4d: %f %f %f %f\n", s, t, r, q));

    _gamma_TexCoord4f((GLfloat)s,(GLfloat)t,(GLfloat)r,(GLfloat)q);
}

void _gamma_TexCoord4dv(const GLdouble *v)
{
    DEBUG_GLCMDS(("TexCoord4dv: %f %f %f %f\n", v[0], v[1], v[2], v[3]));

    _gamma_TexCoord4fv((GLfloat*)v);
}

void _gamma_TexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    DEBUG_GLCMDS(("TexCoord4f: %f %f %f %f\n", s, t, r, q));

    gCCPriv->Current.TexCoord[0] = s;
    gCCPriv->Current.TexCoord[1] = t;
    gCCPriv->Current.TexCoord[2] = t;
    gCCPriv->Current.TexCoord[3] = q;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 4);
    WRITEF(gCCPriv->buf, Tq4, q);
    WRITEF(gCCPriv->buf, Tr4, r);
    WRITEF(gCCPriv->buf, Tt4, t);
    WRITEF(gCCPriv->buf, Ts4, s);
}

void _gamma_TexCoord4fv(const GLfloat *v)
{
    DEBUG_GLCMDS(("TexCoord4fv: %f %f %f %f\n", v[0], v[1], v[2], v[3]));

    gCCPriv->Current.TexCoord[0] = v[0];
    gCCPriv->Current.TexCoord[1] = v[1];
    gCCPriv->Current.TexCoord[2] = v[2];
    gCCPriv->Current.TexCoord[3] = v[3];

    CHECK_DMA_BUFFER(gCC, gCCPriv, 4);
    WRITEF(gCCPriv->buf, Tq4, v[3]);
    WRITEF(gCCPriv->buf, Tr4, v[2]);
    WRITEF(gCCPriv->buf, Tt4, v[1]);
    WRITEF(gCCPriv->buf, Ts4, v[0]);
}

void _gamma_TexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
    GLfloat x,y,z,a;

    DEBUG_GLCMDS(("TexCoord4i: %d %d %d %d\n", (int)s, (int)t, (int)r, (int)q));

    x = INT_TO_FLOAT(s);
    y = INT_TO_FLOAT(t);
    z = INT_TO_FLOAT(r);
    a = INT_TO_FLOAT(q);

    _gamma_TexCoord4f(x,y,z,a);
}

void _gamma_TexCoord4iv(const GLint *v)
{
    GLfloat p[4];

    DEBUG_GLCMDS(("TexCoord4iv: %d %d %d %d\n",
		  (int)v[0], (int)v[1], (int)v[2], (int)v[3]));

    p[0] = INT_TO_FLOAT(v[0]);
    p[1] = INT_TO_FLOAT(v[1]);
    p[2] = INT_TO_FLOAT(v[2]);
    p[3] = INT_TO_FLOAT(v[3]);

    _gamma_TexCoord4fv(p);
}

void _gamma_TexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
    GLfloat x,y,z,a;

    DEBUG_GLCMDS(("TexCoord4s: %d %d %d %d\n", s, t, r, q));

    x = SHORT_TO_FLOAT(s);
    y = SHORT_TO_FLOAT(t);
    z = SHORT_TO_FLOAT(r);
    a = SHORT_TO_FLOAT(q);

    _gamma_TexCoord4f(s,t,r,q);
}

void _gamma_TexCoord4sv(const GLshort *v)
{
    GLfloat p[4];

    DEBUG_GLCMDS(("TexCoord4sv: %d %d %d %d\n", v[0], v[1], v[2], v[3]));

    p[0] = SHORT_TO_FLOAT(v[0]);
    p[1] = SHORT_TO_FLOAT(v[1]);
    p[2] = SHORT_TO_FLOAT(v[2]);
    p[3] = SHORT_TO_FLOAT(v[3]);

    _gamma_TexCoord4fv(p);
}

void _gamma_TexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    DEBUG_GLCMDS(("TexCoordPointer: %d %04x %d\n",
		  (int)size, (int)type, (int)stride));

   if (size<1 || size>4) {
      gamma_error( GL_INVALID_VALUE, "glTexCoordPointer(size)" );
      return;
   }
   if (stride<0) {
      gamma_error( GL_INVALID_VALUE, "glTexCoordPointer(stride)" );
      return;
   }
   switch (type) {
      case GL_SHORT:
         gCCPriv->Array.TexCoordStrideB = stride ? stride : size*sizeof(GLshort);
         break;
      case GL_INT:
         gCCPriv->Array.TexCoordStrideB = stride ? stride : size*sizeof(GLint);
         break;
      case GL_FLOAT:
         gCCPriv->Array.TexCoordStrideB = stride ? stride : size*sizeof(GLfloat);
         break;
      case GL_DOUBLE:
         gCCPriv->Array.TexCoordStrideB = stride ? stride : size*sizeof(GLdouble);
         break;
      default:
         gamma_error( GL_INVALID_ENUM, "glTexCoordPointer(type)" );
         return;
   }
   gCCPriv->Array.TexCoordSize = size;
   gCCPriv->Array.TexCoordType = type;
   gCCPriv->Array.TexCoordStride = stride;
   gCCPriv->Array.TexCoordPtr = (void *) pointer;
}

void _gamma_TexEnvf(GLenum target, GLenum pname, GLfloat param)
{
    DEBUG_GLCMDS(("TexEnvf: %04x %04x %f\n", (int)target, (int)pname, param));

    if (target != GL_TEXTURE_ENV || pname != GL_TEXTURE_ENV_MODE) {
	/* ERROR !! */
    }

    gCCPriv->curTexObj->TextureColorMode &= ~TCM_ApplicationMask;

    switch ((int)param) {
    case GL_MODULATE:
	gCCPriv->curTexObj->TextureColorMode |= TCM_Modulate;
	break;
    case GL_DECAL:
	gCCPriv->curTexObj->TextureColorMode |= TCM_Decal;
	break;
    case GL_BLEND:
	gCCPriv->curTexObj->TextureColorMode |= TCM_Blend;
	break;
    case GL_REPLACE:
	gCCPriv->curTexObj->TextureColorMode |= TCM_Replace;
	break;
    default:
	break;
    }

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, TextureColorMode,
	  gCCPriv->curTexObj->TextureColorMode);
}

void _gamma_TexEnvfv(GLenum target, GLenum pname, const GLfloat *params)
{
    DEBUG_GLCMDS(("TexEnvfv: %04x %04x %f\n",
		  (int)target, (int)pname, *params));

    if (target != GL_TEXTURE_ENV) {
	/* ERROR !! */
    }

    switch (pname) {
    case GL_TEXTURE_ENV_MODE:
	gCCPriv->curTexObj->TextureColorMode &= ~TCM_ApplicationMask;
	switch ((int)params[0]) {
	case GL_MODULATE:
	    gCCPriv->curTexObj->TextureColorMode |= TCM_Modulate;
	    break;
	case GL_DECAL:
	    gCCPriv->curTexObj->TextureColorMode |= TCM_Decal;
	    break;
	case GL_BLEND:
	    gCCPriv->curTexObj->TextureColorMode |= TCM_Blend;
	    break;
	case GL_REPLACE:
	    gCCPriv->curTexObj->TextureColorMode |= TCM_Replace;
	    break;
	}
    }

    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
    WRITE(gCCPriv->buf, TextureColorMode,
	  gCCPriv->curTexObj->TextureColorMode);
}

void _gamma_TexEnvi(GLenum target, GLenum pname, GLint param)
{
    DEBUG_GLCMDS(("TexEnvi: %04x %04x %d\n",
		  (int)target, (int)pname, (int)param));
}

void _gamma_TexEnviv(GLenum target, GLenum pname, const GLint *params)
{
    DEBUG_GLCMDS(("TexEnviv: %04x %04x %d\n",
		  (int)target, (int)pname, (int)*params));
}

void _gamma_TexGend(GLenum coord, GLenum pname, GLdouble param)
{
    DEBUG_GLCMDS(("TexGend: %04x %04x %f\n", (int)coord, (int)pname, param));
}

void _gamma_TexGendv(GLenum coord, GLenum pname, const GLdouble *params)
{
    DEBUG_GLCMDS(("TexGendv: %04x %04x %f\n", (int)coord, (int)pname, *params));
}

void _gamma_TexGenf(GLenum coord, GLenum pname, GLfloat param)
{
    DEBUG_GLCMDS(("TexGenf: %04x %04x %f\n", (int)coord, (int)pname, param));
}

void _gamma_TexGenfv(GLenum coord, GLenum pname, const GLfloat *params)
{
    DEBUG_GLCMDS(("TexGenfv: %04x %04x %f\n", (int)coord, (int)pname, *params));
}

void _gamma_TexGeni(GLenum coord, GLenum pname, GLint param)
{
    DEBUG_GLCMDS(("TexGeni: %04x %04x %d\n",
		  (int)coord, (int)pname, (int)param));
}

void _gamma_TexGeniv(GLenum coord, GLenum pname, const GLint *params)
{
    DEBUG_GLCMDS(("TexGeniv: %04x %04x %d\n",
		  (int)coord, (int)pname, (int)*params));
}

void _gamma_TexImage1D(GLenum target, GLint level, GLint components, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid *image)
{
    DEBUG_GLCMDS(("TexImage1D: %04x %d %d %d %d %04x %04x\n",
		  (int)target, (int)level, (int)components,
		  (int)width, (int)border, (int)format, (int)type));
}

void _gamma_TexImage2D(GLenum target, GLint level, GLint components, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *image)
{
    unsigned long addrs[MIPMAP_LEVELS];
    int l2w, l2h, l2d;
    gammaTexObj *t;
    int i;

    DEBUG_GLCMDS(("TexImage2D: %04x %d %d %d %d %d %04x %04x\n",
		  (int)target, (int)level, (int)components,
		  (int)width, (int)height, (int)border,
		  (int)format, (int)type));

    if (target == GL_TEXTURE_1D) {
	/* NOT_DONE */
	DEBUG_ERROR(("TexImage2D: 1D requested - ERROR\n"));
    } else if (target == GL_TEXTURE_2D) {
	/* NOT_DONE: The follow are not currently supported... */
	if (border != 0 /* || format != GL_RGBA */ || type != GL_UNSIGNED_BYTE ||
	    (components != 3 && components != 4)) {
	    DEBUG_ERROR(("TexImage2D: 2D op not supported - ERROR\n"));
	    return;
	}

	if (width > 2048 || height > 2048) return; /* ERROR !! */
	if (level < 0 || level > 11) return; /* ERROR !! */

	/* Calculate the log2width, log2height and log2depth */
	CALC_LOG2(l2w, width);
	CALC_LOG2(l2h, height);
	l2d = 5;

	t = gCCPriv->curTexObj2D;

	/* Set the texture params for level 0 only */
	if (level == 0) {
	    t->TextureAddressMode &= ~(TAM_WidthMask |
				       TAM_HeightMask);
	    t->TextureAddressMode |= (l2w << 9);
	    t->TextureAddressMode |= (l2h << 13);

	    /* NOT_DONE: Support patch mode */
	    t->TextureReadMode &= ~(TRM_WidthMask |
				    TRM_HeightMask |
				    TRM_DepthMask |
				    TRM_Border |
				    TRM_Patch);
	    t->TextureReadMode |= (l2w << 1);
	    t->TextureReadMode |= (l2h << 5);
	    t->TextureReadMode |= (l2d << 9);

	    t->TextureColorMode &= ~(TCM_BaseFormatMask);

	    switch (components) {
	    case 3:
		t->TextureColorMode |= TCM_BaseFormat_RGB;
		break;
	    case 4:
		t->TextureColorMode |= TCM_BaseFormat_RGBA;
		break;
	    }

#if X_BYTE_ORDER == X_LITTLE_ENDIAN
	    t->TextureFormat = (TF_LittleEndian |
#else
	    t->TextureFormat = (TF_BigEndian |
#endif
				TF_ColorOrder_BGR |
				TF_Compnents_4 |
				TF_OutputFmt_Texel);
	}

	/* Remove the old image */
	if (t->image[level])
	    driTMMDeleteImage(gCCPriv->tmm, t->image[level]);

	/* Insert the new image */
	t->image[level] = driTMMInsertImage(gCCPriv->tmm,
					    width, height, 1<<l2d,
					    image, NULL);

	if (!t->image[level]) {
	    /* NOT_DONE: Handle error */
	    DEBUG_ERROR(("TexImage2D: unable1\n"));
	}

	/* Make the new image resident (and all of the other mipmaps) */
	if (driTMMMakeImagesResident(gCCPriv->tmm, MIPMAP_LEVELS,
				      t->image, addrs) < 0) {
	    /* NOT_DONE: Handle error */
	    DEBUG_ERROR(("TexImage2D: unable2\n"));
	}

	for (i = 0; i < MIPMAP_LEVELS; i++)
	    t->TextureBaseAddr[i] = addrs[i] << 5;

	CHECK_DMA_BUFFER(gCC, gCCPriv, 4);
	WRITE(gCCPriv->buf, TextureAddressMode,
	      gCCPriv->curTexObj2D->TextureAddressMode);
	WRITE(gCCPriv->buf, TextureReadMode,
	      gCCPriv->curTexObj2D->TextureReadMode);
	WRITE(gCCPriv->buf, TextureColorMode,
	      gCCPriv->curTexObj2D->TextureColorMode);
	WRITE(gCCPriv->buf, TextureFormat,
	      gCCPriv->curTexObj2D->TextureFormat);

	CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	switch (level) {
	case 0:
	    WRITE(gCCPriv->buf, TxBaseAddr0,
		  gCCPriv->curTexObj2D->TextureBaseAddr[0]);
	    break;
	case 1:
	    WRITE(gCCPriv->buf, TxBaseAddr1,
		  gCCPriv->curTexObj2D->TextureBaseAddr[1]);
	    break;
	case 2:
	    WRITE(gCCPriv->buf, TxBaseAddr2,
		  gCCPriv->curTexObj2D->TextureBaseAddr[2]);
	    break;
	case 3:
	    WRITE(gCCPriv->buf, TxBaseAddr3,
		  gCCPriv->curTexObj2D->TextureBaseAddr[3]);
	    break;
	case 4:
	    WRITE(gCCPriv->buf, TxBaseAddr4,
		  gCCPriv->curTexObj2D->TextureBaseAddr[4]);
	    break;
	case 5:
	    WRITE(gCCPriv->buf, TxBaseAddr5,
		  gCCPriv->curTexObj2D->TextureBaseAddr[5]);
	    break;
	case 6:
	    WRITE(gCCPriv->buf, TxBaseAddr6,
		  gCCPriv->curTexObj2D->TextureBaseAddr[6]);
	    break;
	case 7:
	    WRITE(gCCPriv->buf, TxBaseAddr7,
		  gCCPriv->curTexObj2D->TextureBaseAddr[7]);
	    break;
	case 8:
	    WRITE(gCCPriv->buf, TxBaseAddr8,
		  gCCPriv->curTexObj2D->TextureBaseAddr[8]);
	    break;
	case 9:
	    WRITE(gCCPriv->buf, TxBaseAddr9,
		  gCCPriv->curTexObj2D->TextureBaseAddr[9]);
	    break;
	case 10:
	    WRITE(gCCPriv->buf, TxBaseAddr10,
		  gCCPriv->curTexObj2D->TextureBaseAddr[10]);
	    break;
	case 11:
	    WRITE(gCCPriv->buf, TxBaseAddr11,
		  gCCPriv->curTexObj2D->TextureBaseAddr[11]);
	    break;
	}
    } else {
	/* ERROR !! */
    }
}

void _gamma_TexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    DEBUG_GLCMDS(("TexParameterf: %04x %04x %f\n",
		  (int)target, (int)pname, param));

    if (target == GL_TEXTURE_1D) {
	/* NOT_DONE */
    } else if (target == GL_TEXTURE_2D) {
	switch (pname) {
	case GL_TEXTURE_MAG_FILTER:
	    gCCPriv->curTexObj2D->TextureReadMode &= ~TRM_Mag_Mask;
	    switch ((int)param) {
	    case GL_NEAREST:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Mag_Nearest;
		break;
	    case GL_LINEAR:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Mag_Linear;
		break;
	    default:
		break;
	    }

	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITE(gCCPriv->buf, TextureReadMode,
		  gCCPriv->curTexObj2D->TextureReadMode);
	    break;
	case GL_TEXTURE_MIN_FILTER:
	    gCCPriv->curTexObj2D->TextureReadMode    &= ~TRM_Min_Mask;
	    gCCPriv->curTexObj2D->TextureReadMode    |= TRM_MipMapEnable;
	    gCCPriv->curTexObj2D->TextureAddressMode |= TAM_LODEnable;
	    switch ((int)param) {
	    case GL_NEAREST:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Min_Nearest;
		gCCPriv->curTexObj2D->TextureReadMode    &= ~TRM_MipMapEnable;
		gCCPriv->curTexObj2D->TextureAddressMode &= ~TAM_LODEnable;
		break;
	    case GL_LINEAR:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Min_Linear;
		gCCPriv->curTexObj2D->TextureReadMode    &= ~TRM_MipMapEnable;
		gCCPriv->curTexObj2D->TextureAddressMode &= ~TAM_LODEnable;
		break;
	    case GL_NEAREST_MIPMAP_NEAREST:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Min_NearestMMNearest;
		break;
	    case GL_LINEAR_MIPMAP_NEAREST:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Min_NearestMMLinear;
		break;
	    case GL_NEAREST_MIPMAP_LINEAR:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Min_LinearMMNearest;
		break;
	    case GL_LINEAR_MIPMAP_LINEAR:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Min_LinearMMLinear;
		break;
	    default:
		break;
	    }

	    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	    WRITE(gCCPriv->buf, TextureReadMode,
		  gCCPriv->curTexObj2D->TextureReadMode);
	    WRITE(gCCPriv->buf, TextureAddressMode,
		  gCCPriv->curTexObj2D->TextureAddressMode);
	    break;
	case GL_TEXTURE_WRAP_S:
	    gCCPriv->curTexObj2D->TextureAddressMode &= ~TAM_SWrap_Mask;
	    gCCPriv->curTexObj2D->TextureReadMode    &= ~TRM_UWrap_Mask;
	    switch ((int)param) {
	    case GL_CLAMP:
		gCCPriv->curTexObj2D->TextureAddressMode |= TAM_SWrap_Clamp;
		gCCPriv->curTexObj2D->TextureReadMode    |= TRM_UWrap_Clamp;
		break;
	    case GL_REPEAT:
		gCCPriv->curTexObj2D->TextureAddressMode |= TAM_SWrap_Repeat;
		gCCPriv->curTexObj2D->TextureReadMode    |= TRM_UWrap_Repeat;
		break;
	    default:
		break;
	    }

	    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	    WRITE(gCCPriv->buf, TextureReadMode,
		  gCCPriv->curTexObj2D->TextureReadMode);
	    WRITE(gCCPriv->buf, TextureAddressMode,
		  gCCPriv->curTexObj2D->TextureAddressMode);
	    break;
	case GL_TEXTURE_WRAP_T:
	    gCCPriv->curTexObj2D->TextureAddressMode &= ~TAM_TWrap_Mask;
	    gCCPriv->curTexObj2D->TextureReadMode    &= ~TRM_VWrap_Mask;
	    switch ((int)param) {
	    case GL_CLAMP:
		gCCPriv->curTexObj2D->TextureAddressMode |= TAM_TWrap_Clamp;
		gCCPriv->curTexObj2D->TextureReadMode    |= TRM_VWrap_Clamp;
		break;
	    case GL_REPEAT:
		gCCPriv->curTexObj2D->TextureAddressMode |= TAM_TWrap_Repeat;
		gCCPriv->curTexObj2D->TextureReadMode    |= TRM_VWrap_Repeat;
		break;
	    default:
		break;
	    }

	    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	    WRITE(gCCPriv->buf, TextureReadMode,
		  gCCPriv->curTexObj2D->TextureReadMode);
	    WRITE(gCCPriv->buf, TextureAddressMode,
		  gCCPriv->curTexObj2D->TextureAddressMode);
	    break;
	default:
	    break;
	}
    } else {
	/* ERROR !! */
    }
}

void _gamma_TexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
    DEBUG_GLCMDS(("TexParameterfv: %04x %04x %f\n",
		  (int)target, (int)pname, *params));

    if (target == GL_TEXTURE_1D) {
	/* NOT_DONE */
    } else if (target == GL_TEXTURE_2D) {
	switch (pname) {
	case GL_TEXTURE_MAG_FILTER:
	    gCCPriv->curTexObj2D->TextureReadMode &= ~TRM_Mag_Mask;
	    switch ((int)params[0]) {
	    case GL_NEAREST:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Mag_Nearest;
		break;
	    case GL_LINEAR:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Mag_Linear;
		break;
	    default:
		break;
	    }

	    CHECK_DMA_BUFFER(gCC, gCCPriv, 1);
	    WRITE(gCCPriv->buf, TextureReadMode,
		  gCCPriv->curTexObj2D->TextureReadMode);
	    break;
	case GL_TEXTURE_MIN_FILTER:
	    gCCPriv->curTexObj2D->TextureReadMode    &= ~TRM_Min_Mask;
	    gCCPriv->curTexObj2D->TextureReadMode    |= TRM_MipMapEnable;
	    gCCPriv->curTexObj2D->TextureAddressMode |= TAM_LODEnable;
	    switch ((int)params[0]) {
	    case GL_NEAREST:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Min_Nearest;
		gCCPriv->curTexObj2D->TextureReadMode    &= ~TRM_MipMapEnable;
		gCCPriv->curTexObj2D->TextureAddressMode &= ~TAM_LODEnable;
		break;
	    case GL_LINEAR:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Min_Linear;
		gCCPriv->curTexObj2D->TextureReadMode    &= ~TRM_MipMapEnable;
		gCCPriv->curTexObj2D->TextureAddressMode &= ~TAM_LODEnable;
		break;
	    case GL_NEAREST_MIPMAP_NEAREST:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Min_NearestMMNearest;
		break;
	    case GL_LINEAR_MIPMAP_NEAREST:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Min_NearestMMLinear;
		break;
	    case GL_NEAREST_MIPMAP_LINEAR:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Min_LinearMMNearest;
		break;
	    case GL_LINEAR_MIPMAP_LINEAR:
		gCCPriv->curTexObj2D->TextureReadMode |=
		    TRM_Min_LinearMMLinear;
		break;
	    default:
		break;
	    }

	    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	    WRITE(gCCPriv->buf, TextureReadMode,
		  gCCPriv->curTexObj2D->TextureReadMode);
	    WRITE(gCCPriv->buf, TextureAddressMode,
		  gCCPriv->curTexObj2D->TextureAddressMode);
	    break;
	case GL_TEXTURE_WRAP_S:
	    gCCPriv->curTexObj2D->TextureAddressMode &= ~TAM_SWrap_Mask;
	    gCCPriv->curTexObj2D->TextureReadMode    &= ~TRM_UWrap_Mask;
	    switch ((int)params[0]) {
	    case GL_CLAMP:
		gCCPriv->curTexObj2D->TextureAddressMode |= TAM_SWrap_Clamp;
		gCCPriv->curTexObj2D->TextureReadMode    |= TRM_UWrap_Clamp;
		break;
	    case GL_REPEAT:
		gCCPriv->curTexObj2D->TextureAddressMode |= TAM_SWrap_Repeat;
		gCCPriv->curTexObj2D->TextureReadMode    |= TRM_UWrap_Repeat;
		break;
	    default:
		break;
	    }

	    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	    WRITE(gCCPriv->buf, TextureReadMode,
		  gCCPriv->curTexObj2D->TextureReadMode);
	    WRITE(gCCPriv->buf, TextureAddressMode,
		  gCCPriv->curTexObj2D->TextureAddressMode);
	    break;
	case GL_TEXTURE_WRAP_T:
	    gCCPriv->curTexObj2D->TextureAddressMode &= ~TAM_TWrap_Mask;
	    gCCPriv->curTexObj2D->TextureReadMode    &= ~TRM_VWrap_Mask;
	    switch ((int)params[0]) {
	    case GL_CLAMP:
		gCCPriv->curTexObj2D->TextureAddressMode |= TAM_TWrap_Clamp;
		gCCPriv->curTexObj2D->TextureReadMode    |= TRM_VWrap_Clamp;
		break;
	    case GL_REPEAT:
		gCCPriv->curTexObj2D->TextureAddressMode |= TAM_TWrap_Repeat;
		gCCPriv->curTexObj2D->TextureReadMode    |= TRM_VWrap_Repeat;
		break;
	    default:
		break;
	    }

	    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
	    WRITE(gCCPriv->buf, TextureReadMode,
		  gCCPriv->curTexObj2D->TextureReadMode);
	    WRITE(gCCPriv->buf, TextureAddressMode,
		  gCCPriv->curTexObj2D->TextureAddressMode);
	    break;
	default:
	    break;
	}
    } else {
	/* ERROR !! */
    }
}

void _gamma_TexParameteri(GLenum target, GLenum pname, GLint param)
{
    DEBUG_GLCMDS(("TexParameteri: %04x %04x %d\n",
		  (int)target, (int)pname, (int)param));
}

void _gamma_TexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
    DEBUG_GLCMDS(("TexParameteriv: %04x %04x %d\n",
		  (int)target, (int)pname, (int)*params));
}

void _gamma_TexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid *image)
{
    DEBUG_GLCMDS(("TexSubImage1D: %04x %d %d %d %04x %04x\n",
		  (int)target, (int)level,
		  (int)xoffset, (int)width, (int)format, (int)type));
}

void _gamma_TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid *image)
{
    DEBUG_GLCMDS(("TexSubImage2D: %04x %d %d %d %d %d %04x %04x\n",
		  (int)target, (int)level,
		  (int)xoffset, (int)yoffset, (int)width, (int)height,
		  (int)format, (int)type));

    if (target == GL_TEXTURE_1D) {
	/* NOT_DONE */
    } else if (target == GL_TEXTURE_2D) {
	/* NOT_DONE: The follow are not currently supported... */
	if (format != GL_RGBA || type != GL_UNSIGNED_BYTE) {
	    return;
	}

	if (width > 2048 || height > 2048) return; /* ERROR !! */
	if (level < 0 || level > 11) return; /* ERROR !! */

	/*
	** NOT_DONE: This should convert the image into the internal
	** format for the image that it is replacing.
	*/

	if (driTMMSubImage(gCCPriv->tmm, gCCPriv->curTexObj2D->image[level],
			    xoffset, yoffset, width, height, image) < 0) {
	    /* NOT_DONE: Handle error */
	}
    } else {
	/* ERROR !! */
    }
}

void _gamma_Translated(GLdouble x, GLdouble y, GLdouble z)
{
    GLfloat m[16];
    int i;

    DEBUG_GLCMDS(("Translated: %f %f %f\n", x, y, z));

    for (i = 0; i < 16; i++)
	if (i % 5 == 0)
	    m[i] = 1.0;
	else
	    m[i] = 0.0;

    m[12] = x;
    m[13] = y;
    m[14] = z;

    gammaMultMatrix(m);
    gammaLoadHWMatrix();
}

void _gamma_Translatef(GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat m[16];
    int i;

    DEBUG_GLCMDS(("Translatef: %f %f %f\n", x, y, z));

    for (i = 0; i < 16; i++)
	if (i % 5 == 0)
	    m[i] = 1.0;
	else
	    m[i] = 0.0;

    m[12] = x;
    m[13] = y;
    m[14] = z;

    gammaMultMatrix(m);
    gammaLoadHWMatrix();
}

void _gamma_Vertex2d(GLdouble x, GLdouble y)
{
    DEBUG_GLCMDS(("Vertex2d: %f %f\n", x, y));

    _gamma_Vertex2f((GLfloat)x,(GLfloat)y);
}

void _gamma_Vertex2dv(const GLdouble *v)
{
    DEBUG_GLCMDS(("Vertex2dv: %f %f\n", v[0], v[1]));

    _gamma_Vertex2f((GLfloat)v[0],(GLfloat)v[1]);
}

void _gamma_Vertex2f(GLfloat x, GLfloat y)
{
    DEBUG_GLCMDS(("Vertex2f: %f %f\n", x, y));

    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
    WRITEF(gCCPriv->buf, Vy,  y);
    WRITEF(gCCPriv->buf, Vx2, x);
}

void _gamma_Vertex2fv(const GLfloat *v)
{
    DEBUG_GLCMDS(("Vertex2fv: %f %f\n", v[0], v[1]));

    _gamma_Vertex2f(v[0],v[1]);
}

void _gamma_Vertex2i(GLint x, GLint y)
{
    DEBUG_GLCMDS(("Vertex2i: %d %d\n", (int)x, (int)y));

    _gamma_Vertex2f((GLfloat)x,(GLfloat)y);
}

void _gamma_Vertex2iv(const GLint *v)
{
    DEBUG_GLCMDS(("Vertex2iv: %d %d\n", (int)v[0], (int)v[1]));

    _gamma_Vertex2f((GLfloat)v[0],(GLfloat)v[1]);
}

void _gamma_Vertex2s(GLshort x, GLshort y)
{
    DEBUG_GLCMDS(("Vertex2s: %d %d\n", x, y));

    _gamma_Vertex2f((GLfloat)x,(GLfloat)y);
}

void _gamma_Vertex2sv(const GLshort *v)
{
    DEBUG_GLCMDS(("Vertex2sv: %d %d\n", v[0], v[1]));

    _gamma_Vertex2f((GLfloat)v[0],(GLfloat)v[1]);
}

void _gamma_Vertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    DEBUG_GLCMDS(("Vertex3d: %f %f %f\n", x, y, z));

    _gamma_Vertex3f((GLfloat)x,(GLfloat)y,(GLfloat)z);
}

void _gamma_Vertex3dv(const GLdouble *v)
{
    DEBUG_GLCMDS(("Vertex2fv: %f %f %f\n", v[0], v[1], v[2]));

    _gamma_Vertex3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_Vertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    DEBUG_GLCMDS(("Vertex3f: %f %f %f\n", x, y, z));

    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
    WRITEF(gCCPriv->buf, Vz,  z);
    WRITEF(gCCPriv->buf, Vy,  y);
    WRITEF(gCCPriv->buf, Vx3, x);
}

void _gamma_Vertex3fv(const GLfloat *v)
{
    DEBUG_GLCMDS(("Vertex3fv: %f %f %f\n", v[0], v[1], v[2]));

    _gamma_Vertex3f(v[0],v[1],v[2]);
}

void _gamma_Vertex3i(GLint x, GLint y, GLint z)
{
    DEBUG_GLCMDS(("Vertex3i: %d %d %d\n", (int)x, (int)y, (int)z));

    _gamma_Vertex3f((GLfloat)x,(GLfloat)y,(GLfloat)z);
}

void _gamma_Vertex3iv(const GLint *v)
{
    DEBUG_GLCMDS(("Vertex3iv: %d %d %d\n", (int)v[0], (int)v[1], (int)v[2]));

    _gamma_Vertex3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_Vertex3s(GLshort x, GLshort y, GLshort z)
{
    DEBUG_GLCMDS(("Vertex3s: %d %d %d\n", x, y, z));

    _gamma_Vertex3f((GLfloat)x,(GLfloat)y,(GLfloat)z);
}

void _gamma_Vertex3sv(const GLshort *v)
{
    DEBUG_GLCMDS(("Vertex3sv: %d %d %d\n", v[0], v[1], v[2]));

    _gamma_Vertex3f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2]);
}

void _gamma_Vertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    DEBUG_GLCMDS(("Vertex4d: %f %f %f %f\n", x, y, z, w));

    _gamma_Vertex4f((GLfloat)x,(GLfloat)y,(GLfloat)z,(GLfloat)w);
}

void _gamma_Vertex4dv(const GLdouble *v)
{
    DEBUG_GLCMDS(("Vertex4dv: %f %f %f %f\n", v[0], v[1], v[2], v[3]));

    _gamma_Vertex4f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2],(GLfloat)v[3]);
}

void _gamma_Vertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    DEBUG_GLCMDS(("Vertex4f: %f %f %f %f\n", x, y, z, w));

    CHECK_DMA_BUFFER(gCC, gCCPriv, 4);
    WRITEF(gCCPriv->buf, Vw,  w);
    WRITEF(gCCPriv->buf, Vz,  z);
    WRITEF(gCCPriv->buf, Vy,  y);
    WRITEF(gCCPriv->buf, Vx4, x);
}

void _gamma_Vertex4fv(const GLfloat *v)
{
    DEBUG_GLCMDS(("Vertex4fv: %f %f %f %f\n", v[0], v[1], v[2], v[3]));

    _gamma_Vertex4f(v[0],v[1],v[2],v[3]);
}

void _gamma_Vertex4i(GLint x, GLint y, GLint z, GLint w)
{
    DEBUG_GLCMDS(("Vertex4i: %d %d %d %d\n", (int)x, (int)y, (int)z, (int)w));

    _gamma_Vertex4f((GLfloat)x,(GLfloat)y,(GLfloat)z,(GLfloat)w);
}

void _gamma_Vertex4iv(const GLint *v)
{
    DEBUG_GLCMDS(("Vertex4iv: %d %d %d %d\n",
		  (int)v[0], (int)v[1], (int)v[2], (int)v[3]));

    _gamma_Vertex4f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2],(GLfloat)v[3]);
}

void _gamma_Vertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    DEBUG_GLCMDS(("Vertex4s: %d %d %d %d\n", x, y, z, w));

    _gamma_Vertex4f((GLfloat)x,(GLfloat)y,(GLfloat)z,(GLfloat)w);
}

void _gamma_Vertex4sv(const GLshort *v)
{
    DEBUG_GLCMDS(("Vertex4sv: %d %d %d %d\n", v[0], v[1], v[2], v[3]));

    _gamma_Vertex4f((GLfloat)v[0],(GLfloat)v[1],(GLfloat)v[2],(GLfloat)v[3]);
}

void _gamma_VertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
    DEBUG_GLCMDS(("VertexPointer: %d %04x %d\n",
		  (int)size, (int)type, (int)stride));

   if (size<2 || size>4) {
      gamma_error( GL_INVALID_VALUE, "glVertexPointer(size)" );
      return;
   }
   if (stride<0) {
      gamma_error( GL_INVALID_VALUE, "glVertexPointer(stride)" );
      return;
   }
   switch (type) {
      case GL_SHORT:
         gCCPriv->Array.VertexStrideB = stride ? stride : size*sizeof(GLshort);
         break;
      case GL_INT:
         gCCPriv->Array.VertexStrideB = stride ? stride : size*sizeof(GLint);
         break;
      case GL_FLOAT:
         gCCPriv->Array.VertexStrideB = stride ? stride : size*sizeof(GLfloat);
         break;
      case GL_DOUBLE:
         gCCPriv->Array.VertexStrideB = stride ? stride : size*sizeof(GLdouble);
         break;
      default:
         gamma_error( GL_INVALID_ENUM, "glVertexPointer(type)" );
         return;
   }
   gCCPriv->Array.VertexSize = size;
   gCCPriv->Array.VertexType = type;
   gCCPriv->Array.VertexStride = stride;
   gCCPriv->Array.VertexPtr = (void *) pointer;
}

void _gamma_Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    GLfloat sx, sy, ox, oy;

    DEBUG_GLCMDS(("Viewport: %d %d %d %d\n",
		  (int)x, (int)y, (int)width, (int)height));

    gCCPriv->x = gCC->driDrawablePriv->x + x;
    gCCPriv->y = gCC->driScreenPriv->fbHeight -
	(gCC->driDrawablePriv->y +
	 gCC->driDrawablePriv->h) + y;
    gCCPriv->w = width;
    gCCPriv->h = height;

    x = gCCPriv->x;
    y = gCCPriv->y;

    sx = width/2.0f;
    sy = height/2.0f;
    ox = x + sx;
    oy = y + sy;

    CHECK_DMA_BUFFER(gCC, gCCPriv, 4);
    WRITEF(gCCPriv->buf, ViewPortOffsetX, ox);
    WRITEF(gCCPriv->buf, ViewPortOffsetY, oy);
    WRITEF(gCCPriv->buf, ViewPortScaleX,  sx);
    WRITEF(gCCPriv->buf, ViewPortScaleY,  sy);
#if 1 /* Err - this shouldn't be needed, but something isn't flushing */
    FLUSH_DMA_BUFFER(gCC,gCCPriv);
#endif
}


static GLint
generic_noop(void)
{
   DEBUG_GLCMDS(("OOPS GENERIC NOOP CALLED\n"));

   return 0;
}


static void
init_no_op_table(struct _glapi_table *table)
{
   /* Check to be sure the dispatcher's table is at least as big as Mesa's. */
   const GLuint size = sizeof(struct _glapi_table) / sizeof(void *);
   assert(_glapi_get_dispatch_table_size() >= size);

   {
      const GLuint n = _glapi_get_dispatch_table_size();
      GLuint i;
      void **dispatch = (void **) table;
      for (i = 0; i < n; i++) {
         dispatch[i] = (void *) generic_noop;
      }
   }
}


void
_gamma_init_exec(struct _glapi_table *dispatch)
{
   init_no_op_table(dispatch);

   /* 1.0 */
   dispatch->Accum = _gamma_Accum;
   dispatch->AlphaFunc = _gamma_AlphaFunc;
   dispatch->Begin = _gamma_Begin;
   dispatch->Bitmap = _gamma_Bitmap;
   dispatch->BlendFunc = _gamma_BlendFunc;
   dispatch->CallList = _gamma_CallList;
   dispatch->CallLists = _gamma_CallLists;
   dispatch->Clear = _gamma_Clear;
   dispatch->ClearAccum = _gamma_ClearAccum;
   dispatch->ClearColor = _gamma_ClearColor;
   dispatch->ClearDepth = _gamma_ClearDepth;
   dispatch->ClearIndex = _gamma_ClearIndex;
   dispatch->ClearStencil = _gamma_ClearStencil;
   dispatch->ClipPlane = _gamma_ClipPlane;
   dispatch->Color3b = _gamma_Color3b;
   dispatch->Color3bv = _gamma_Color3bv;
   dispatch->Color3d = _gamma_Color3d;
   dispatch->Color3dv = _gamma_Color3dv;
   dispatch->Color3f = _gamma_Color3f;
   dispatch->Color3fv = _gamma_Color3fv;
   dispatch->Color3i = _gamma_Color3i;
   dispatch->Color3iv = _gamma_Color3iv;
   dispatch->Color3s = _gamma_Color3s;
   dispatch->Color3sv = _gamma_Color3sv;
   dispatch->Color3ub = _gamma_Color3ub;
   dispatch->Color3ubv = _gamma_Color3ubv;
   dispatch->Color3ui = _gamma_Color3ui;
   dispatch->Color3uiv = _gamma_Color3uiv;
   dispatch->Color3us = _gamma_Color3us;
   dispatch->Color3usv = _gamma_Color3usv;
   dispatch->Color4b = _gamma_Color4b;
   dispatch->Color4bv = _gamma_Color4bv;
   dispatch->Color4d = _gamma_Color4d;
   dispatch->Color4dv = _gamma_Color4dv;
   dispatch->Color4f = _gamma_Color4f;
   dispatch->Color4fv = _gamma_Color4fv;
   dispatch->Color4i = _gamma_Color4i;
   dispatch->Color4iv = _gamma_Color4iv;
   dispatch->Color4s = _gamma_Color4s;
   dispatch->Color4sv = _gamma_Color4sv;
   dispatch->Color4ub = _gamma_Color4ub;
   dispatch->Color4ubv = _gamma_Color4ubv;
   dispatch->Color4ui = _gamma_Color4ui;
   dispatch->Color4uiv = _gamma_Color4uiv;
   dispatch->Color4us = _gamma_Color4us;
   dispatch->Color4usv = _gamma_Color4usv;
   dispatch->ColorMask = _gamma_ColorMask;
   dispatch->ColorMaterial = _gamma_ColorMaterial;
   dispatch->CopyPixels = _gamma_CopyPixels;
   dispatch->CullFace = _gamma_CullFace;
   dispatch->DeleteLists = _gamma_DeleteLists;
   dispatch->DepthFunc = _gamma_DepthFunc;
   dispatch->DepthMask = _gamma_DepthMask;
   dispatch->DepthRange = _gamma_DepthRange;
   dispatch->Disable = _gamma_Disable;
   dispatch->DrawBuffer = _gamma_DrawBuffer;
   dispatch->DrawPixels = _gamma_DrawPixels;
   dispatch->EdgeFlag = _gamma_EdgeFlag;
   dispatch->EdgeFlagv = _gamma_EdgeFlagv;
   dispatch->Enable = _gamma_Enable;
   dispatch->End = _gamma_End;
   dispatch->EndList = _gamma_EndList;
   dispatch->EvalCoord1d = _gamma_EvalCoord1d;
   dispatch->EvalCoord1dv = _gamma_EvalCoord1dv;
   dispatch->EvalCoord1f = _gamma_EvalCoord1f;
   dispatch->EvalCoord1fv = _gamma_EvalCoord1fv;
   dispatch->EvalCoord2d = _gamma_EvalCoord2d;
   dispatch->EvalCoord2dv = _gamma_EvalCoord2dv;
   dispatch->EvalCoord2f = _gamma_EvalCoord2f;
   dispatch->EvalCoord2fv = _gamma_EvalCoord2fv;
   dispatch->EvalMesh1 = _gamma_EvalMesh1;
   dispatch->EvalMesh2 = _gamma_EvalMesh2;
   dispatch->EvalPoint1 = _gamma_EvalPoint1;
   dispatch->EvalPoint2 = _gamma_EvalPoint2;
   dispatch->FeedbackBuffer = _gamma_FeedbackBuffer;
   dispatch->Finish = _gamma_Finish;
   dispatch->Flush = _gamma_Flush;
   dispatch->Fogf = _gamma_Fogf;
   dispatch->Fogfv = _gamma_Fogfv;
   dispatch->Fogi = _gamma_Fogi;
   dispatch->Fogiv = _gamma_Fogiv;
   dispatch->FrontFace = _gamma_FrontFace;
   dispatch->Frustum = _gamma_Frustum;
   dispatch->GenLists = _gamma_GenLists;
   dispatch->GetBooleanv = _gamma_GetBooleanv;
   dispatch->GetClipPlane = _gamma_GetClipPlane;
   dispatch->GetDoublev = _gamma_GetDoublev;
   dispatch->GetError = _gamma_GetError;
   dispatch->GetFloatv = _gamma_GetFloatv;
   dispatch->GetIntegerv = _gamma_GetIntegerv;
   dispatch->GetLightfv = _gamma_GetLightfv;
   dispatch->GetLightiv = _gamma_GetLightiv;
   dispatch->GetMapdv = _gamma_GetMapdv;
   dispatch->GetMapfv = _gamma_GetMapfv;
   dispatch->GetMapiv = _gamma_GetMapiv;
   dispatch->GetMaterialfv = _gamma_GetMaterialfv;
   dispatch->GetMaterialiv = _gamma_GetMaterialiv;
   dispatch->GetPixelMapfv = _gamma_GetPixelMapfv;
   dispatch->GetPixelMapuiv = _gamma_GetPixelMapuiv;
   dispatch->GetPixelMapusv = _gamma_GetPixelMapusv;
   dispatch->GetPolygonStipple = _gamma_GetPolygonStipple;
   dispatch->GetString = _gamma_GetString;
   dispatch->GetTexEnvfv = _gamma_GetTexEnvfv;
   dispatch->GetTexEnviv = _gamma_GetTexEnviv;
   dispatch->GetTexGendv = _gamma_GetTexGendv;
   dispatch->GetTexGenfv = _gamma_GetTexGenfv;
   dispatch->GetTexGeniv = _gamma_GetTexGeniv;
   dispatch->GetTexImage = _gamma_GetTexImage;
   dispatch->GetTexLevelParameterfv = _gamma_GetTexLevelParameterfv;
   dispatch->GetTexLevelParameteriv = _gamma_GetTexLevelParameteriv;
   dispatch->GetTexParameterfv = _gamma_GetTexParameterfv;
   dispatch->GetTexParameteriv = _gamma_GetTexParameteriv;
   dispatch->Hint = _gamma_Hint;
   dispatch->IndexMask = _gamma_IndexMask;
   dispatch->Indexd = _gamma_Indexd;
   dispatch->Indexdv = _gamma_Indexdv;
   dispatch->Indexf = _gamma_Indexf;
   dispatch->Indexfv = _gamma_Indexfv;
   dispatch->Indexi = _gamma_Indexi;
   dispatch->Indexiv = _gamma_Indexiv;
   dispatch->Indexs = _gamma_Indexs;
   dispatch->Indexsv = _gamma_Indexsv;
   dispatch->InitNames = _gamma_InitNames;
   dispatch->IsEnabled = _gamma_IsEnabled;
   dispatch->IsList = _gamma_IsList;
   dispatch->LightModelf = _gamma_LightModelf;
   dispatch->LightModelfv = _gamma_LightModelfv;
   dispatch->LightModeli = _gamma_LightModeli;
   dispatch->LightModeliv = _gamma_LightModeliv;
   dispatch->Lightf = _gamma_Lightf;
   dispatch->Lightfv = _gamma_Lightfv;
   dispatch->Lighti = _gamma_Lighti;
   dispatch->Lightiv = _gamma_Lightiv;
   dispatch->LineStipple = _gamma_LineStipple;
   dispatch->LineWidth = _gamma_LineWidth;
   dispatch->ListBase = _gamma_ListBase;
   dispatch->LoadIdentity = _gamma_LoadIdentity;
   dispatch->LoadMatrixd = _gamma_LoadMatrixd;
   dispatch->LoadMatrixf = _gamma_LoadMatrixf;
   dispatch->LoadName = _gamma_LoadName;
   dispatch->LogicOp = _gamma_LogicOp;
   dispatch->Map1d = _gamma_Map1d;
   dispatch->Map1f = _gamma_Map1f;
   dispatch->Map2d = _gamma_Map2d;
   dispatch->Map2f = _gamma_Map2f;
   dispatch->MapGrid1d = _gamma_MapGrid1d;
   dispatch->MapGrid1f = _gamma_MapGrid1f;
   dispatch->MapGrid2d = _gamma_MapGrid2d;
   dispatch->MapGrid2f = _gamma_MapGrid2f;
   dispatch->Materialf = _gamma_Materialf;
   dispatch->Materialfv = _gamma_Materialfv;
   dispatch->Materiali = _gamma_Materiali;
   dispatch->Materialiv = _gamma_Materialiv;
   dispatch->MatrixMode = _gamma_MatrixMode;
   dispatch->MultMatrixd = _gamma_MultMatrixd;
   dispatch->MultMatrixf = _gamma_MultMatrixf;
   dispatch->NewList = _gamma_NewList;
   dispatch->Normal3b = _gamma_Normal3b;
   dispatch->Normal3bv = _gamma_Normal3bv;
   dispatch->Normal3d = _gamma_Normal3d;
   dispatch->Normal3dv = _gamma_Normal3dv;
   dispatch->Normal3f = _gamma_Normal3f;
   dispatch->Normal3fv = _gamma_Normal3fv;
   dispatch->Normal3i = _gamma_Normal3i;
   dispatch->Normal3iv = _gamma_Normal3iv;
   dispatch->Normal3s = _gamma_Normal3s;
   dispatch->Normal3sv = _gamma_Normal3sv;
   dispatch->Ortho = _gamma_Ortho;
   dispatch->PassThrough = _gamma_PassThrough;
   dispatch->PixelMapfv = _gamma_PixelMapfv;
   dispatch->PixelMapuiv = _gamma_PixelMapuiv;
   dispatch->PixelMapusv = _gamma_PixelMapusv;
   dispatch->PixelStoref = _gamma_PixelStoref;
   dispatch->PixelStorei = _gamma_PixelStorei;
   dispatch->PixelTransferf = _gamma_PixelTransferf;
   dispatch->PixelTransferi = _gamma_PixelTransferi;
   dispatch->PixelZoom = _gamma_PixelZoom;
   dispatch->PointSize = _gamma_PointSize;
   dispatch->PolygonMode = _gamma_PolygonMode;
   dispatch->PolygonOffset = _gamma_PolygonOffset;
   dispatch->PolygonStipple = _gamma_PolygonStipple;
   dispatch->PopAttrib = _gamma_PopAttrib;
   dispatch->PopMatrix = _gamma_PopMatrix;
   dispatch->PopName = _gamma_PopName;
   dispatch->PushAttrib = _gamma_PushAttrib;
   dispatch->PushMatrix = _gamma_PushMatrix;
   dispatch->PushName = _gamma_PushName;
   dispatch->RasterPos2d = _gamma_RasterPos2d;
   dispatch->RasterPos2dv = _gamma_RasterPos2dv;
   dispatch->RasterPos2f = _gamma_RasterPos2f;
   dispatch->RasterPos2fv = _gamma_RasterPos2fv;
   dispatch->RasterPos2i = _gamma_RasterPos2i;
   dispatch->RasterPos2iv = _gamma_RasterPos2iv;
   dispatch->RasterPos2s = _gamma_RasterPos2s;
   dispatch->RasterPos2sv = _gamma_RasterPos2sv;
   dispatch->RasterPos3d = _gamma_RasterPos3d;
   dispatch->RasterPos3dv = _gamma_RasterPos3dv;
   dispatch->RasterPos3f = _gamma_RasterPos3f;
   dispatch->RasterPos3fv = _gamma_RasterPos3fv;
   dispatch->RasterPos3i = _gamma_RasterPos3i;
   dispatch->RasterPos3iv = _gamma_RasterPos3iv;
   dispatch->RasterPos3s = _gamma_RasterPos3s;
   dispatch->RasterPos3sv = _gamma_RasterPos3sv;
   dispatch->RasterPos4d = _gamma_RasterPos4d;
   dispatch->RasterPos4dv = _gamma_RasterPos4dv;
   dispatch->RasterPos4f = _gamma_RasterPos4f;
   dispatch->RasterPos4fv = _gamma_RasterPos4fv;
   dispatch->RasterPos4i = _gamma_RasterPos4i;
   dispatch->RasterPos4iv = _gamma_RasterPos4iv;
   dispatch->RasterPos4s = _gamma_RasterPos4s;
   dispatch->RasterPos4sv = _gamma_RasterPos4sv;
   dispatch->ReadBuffer = _gamma_ReadBuffer;
   dispatch->ReadPixels = _gamma_ReadPixels;
   dispatch->Rectd = _gamma_Rectd;
   dispatch->Rectdv = _gamma_Rectdv;
   dispatch->Rectf = _gamma_Rectf;
   dispatch->Rectfv = _gamma_Rectfv;
   dispatch->Recti = _gamma_Recti;
   dispatch->Rectiv = _gamma_Rectiv;
   dispatch->Rects = _gamma_Rects;
   dispatch->Rectsv = _gamma_Rectsv;
   dispatch->RenderMode = _gamma_RenderMode;
   dispatch->Rotated = _gamma_Rotated;
   dispatch->Rotatef = _gamma_Rotatef;
   dispatch->Scaled = _gamma_Scaled;
   dispatch->Scalef = _gamma_Scalef;
   dispatch->Scissor = _gamma_Scissor;
   dispatch->SelectBuffer = _gamma_SelectBuffer;
   dispatch->ShadeModel = _gamma_ShadeModel;
   dispatch->StencilFunc = _gamma_StencilFunc;
   dispatch->StencilMask = _gamma_StencilMask;
   dispatch->StencilOp = _gamma_StencilOp;
   dispatch->TexCoord1d = _gamma_TexCoord1d;
   dispatch->TexCoord1dv = _gamma_TexCoord1dv;
   dispatch->TexCoord1f = _gamma_TexCoord1f;
   dispatch->TexCoord1fv = _gamma_TexCoord1fv;
   dispatch->TexCoord1i = _gamma_TexCoord1i;
   dispatch->TexCoord1iv = _gamma_TexCoord1iv;
   dispatch->TexCoord1s = _gamma_TexCoord1s;
   dispatch->TexCoord1sv = _gamma_TexCoord1sv;
   dispatch->TexCoord2d = _gamma_TexCoord2d;
   dispatch->TexCoord2dv = _gamma_TexCoord2dv;
   dispatch->TexCoord2f = _gamma_TexCoord2f;
   dispatch->TexCoord2fv = _gamma_TexCoord2fv;
   dispatch->TexCoord2i = _gamma_TexCoord2i;
   dispatch->TexCoord2iv = _gamma_TexCoord2iv;
   dispatch->TexCoord2s = _gamma_TexCoord2s;
   dispatch->TexCoord2sv = _gamma_TexCoord2sv;
   dispatch->TexCoord3d = _gamma_TexCoord3d;
   dispatch->TexCoord3dv = _gamma_TexCoord3dv;
   dispatch->TexCoord3f = _gamma_TexCoord3f;
   dispatch->TexCoord3fv = _gamma_TexCoord3fv;
   dispatch->TexCoord3i = _gamma_TexCoord3i;
   dispatch->TexCoord3iv = _gamma_TexCoord3iv;
   dispatch->TexCoord3s = _gamma_TexCoord3s;
   dispatch->TexCoord3sv = _gamma_TexCoord3sv;
   dispatch->TexCoord4d = _gamma_TexCoord4d;
   dispatch->TexCoord4dv = _gamma_TexCoord4dv;
   dispatch->TexCoord4f = _gamma_TexCoord4f;
   dispatch->TexCoord4fv = _gamma_TexCoord4fv;
   dispatch->TexCoord4i = _gamma_TexCoord4i;
   dispatch->TexCoord4iv = _gamma_TexCoord4iv;
   dispatch->TexCoord4s = _gamma_TexCoord4s;
   dispatch->TexCoord4sv = _gamma_TexCoord4sv;
   dispatch->TexEnvf = _gamma_TexEnvf;
   dispatch->TexEnvfv = _gamma_TexEnvfv;
   dispatch->TexEnvi = _gamma_TexEnvi;
   dispatch->TexEnviv = _gamma_TexEnviv;
   dispatch->TexGend = _gamma_TexGend;
   dispatch->TexGendv = _gamma_TexGendv;
   dispatch->TexGenf = _gamma_TexGenf;
   dispatch->TexGenfv = _gamma_TexGenfv;
   dispatch->TexGeni = _gamma_TexGeni;
   dispatch->TexGeniv = _gamma_TexGeniv;
   dispatch->TexImage1D = _gamma_TexImage1D;
   dispatch->TexImage2D = _gamma_TexImage2D;
   dispatch->TexParameterf = _gamma_TexParameterf;
   dispatch->TexParameterfv = _gamma_TexParameterfv;
   dispatch->TexParameteri = _gamma_TexParameteri;
   dispatch->TexParameteriv = _gamma_TexParameteriv;
   dispatch->Translated = _gamma_Translated;
   dispatch->Translatef = _gamma_Translatef;
   dispatch->Vertex2d = _gamma_Vertex2d;
   dispatch->Vertex2dv = _gamma_Vertex2dv;
   dispatch->Vertex2f = _gamma_Vertex2f;
   dispatch->Vertex2fv = _gamma_Vertex2fv;
   dispatch->Vertex2i = _gamma_Vertex2i;
   dispatch->Vertex2iv = _gamma_Vertex2iv;
   dispatch->Vertex2s = _gamma_Vertex2s;
   dispatch->Vertex2sv = _gamma_Vertex2sv;
   dispatch->Vertex3d = _gamma_Vertex3d;
   dispatch->Vertex3dv = _gamma_Vertex3dv;
   dispatch->Vertex3f = _gamma_Vertex3f;
   dispatch->Vertex3fv = _gamma_Vertex3fv;
   dispatch->Vertex3i = _gamma_Vertex3i;
   dispatch->Vertex3iv = _gamma_Vertex3iv;
   dispatch->Vertex3s = _gamma_Vertex3s;
   dispatch->Vertex3sv = _gamma_Vertex3sv;
   dispatch->Vertex4d = _gamma_Vertex4d;
   dispatch->Vertex4dv = _gamma_Vertex4dv;
   dispatch->Vertex4f = _gamma_Vertex4f;
   dispatch->Vertex4fv = _gamma_Vertex4fv;
   dispatch->Vertex4i = _gamma_Vertex4i;
   dispatch->Vertex4iv = _gamma_Vertex4iv;
   dispatch->Vertex4s = _gamma_Vertex4s;
   dispatch->Vertex4sv = _gamma_Vertex4sv;
   dispatch->Viewport = _gamma_Viewport;

   /* 1.1 */
   dispatch->AreTexturesResident = _gamma_AreTexturesResident;
   dispatch->ArrayElement = gl_save_ArrayElement; /*_gamma_ArrayElement;*/
   dispatch->BindTexture = _gamma_BindTexture;
   dispatch->ColorPointer = _gamma_ColorPointer;
   dispatch->CopyTexImage1D = _gamma_CopyTexImage1D;
   dispatch->CopyTexImage2D = _gamma_CopyTexImage2D;
   dispatch->CopyTexSubImage1D = _gamma_CopyTexSubImage1D;
   dispatch->CopyTexSubImage2D = _gamma_CopyTexSubImage2D;
   dispatch->DeleteTextures = _gamma_DeleteTextures;
   dispatch->DisableClientState = _gamma_DisableClientState;
   dispatch->DrawArrays = _gamma_DrawArrays;
   dispatch->DrawElements = _gamma_DrawElements;
   dispatch->EdgeFlagPointer = _gamma_EdgeFlagPointer;
   dispatch->EnableClientState = _gamma_EnableClientState;
   dispatch->GenTextures = _gamma_GenTextures;
   dispatch->GetPointerv = _gamma_GetPointerv;
   dispatch->IndexPointer = _gamma_IndexPointer;
   dispatch->Indexub = _gamma_Indexub;
   dispatch->Indexubv = _gamma_Indexubv;
   dispatch->InterleavedArrays = _gamma_InterleavedArrays;
   dispatch->IsTexture = _gamma_IsTexture;
   dispatch->NormalPointer = _gamma_NormalPointer;
   dispatch->PopClientAttrib = _gamma_PopClientAttrib;
   dispatch->PrioritizeTextures = _gamma_PrioritizeTextures;
   dispatch->PushClientAttrib = _gamma_PushClientAttrib;
   dispatch->TexCoordPointer = _gamma_TexCoordPointer;
   dispatch->TexSubImage1D = _gamma_TexSubImage1D;
   dispatch->TexSubImage2D = _gamma_TexSubImage2D;
   dispatch->VertexPointer = _gamma_VertexPointer;
}

void
_gamma_init_save(struct _glapi_table *table)
{
   init_no_op_table(table);

   table->Accum = gl_save_Accum;
   table->AlphaFunc = gl_save_AlphaFunc;
   table->AreTexturesResident = _gamma_AreTexturesResident; /* NOT SAVED */
   table->ArrayElement = gl_save_ArrayElement;
   table->Begin = gl_save_Begin;
   table->BindTexture = gl_save_BindTexture;
   table->Bitmap = gl_save_Bitmap;
   table->BlendFunc = gl_save_BlendFunc;
   table->CallList = gl_save_CallList;
   table->CallLists = gl_save_CallLists;
   table->Clear = gl_save_Clear;
   table->ClearAccum = gl_save_ClearAccum;
   table->ClearColor = gl_save_ClearColor;
   table->ClearDepth = gl_save_ClearDepth;
   table->ClearIndex = gl_save_ClearIndex;
   table->ClearStencil = gl_save_ClearStencil;
   table->ClipPlane = gl_save_ClipPlane;
   table->Color3f = gl_save_Color3f;
   table->Color3fv = gl_save_Color3fv;
   table->Color4f = gl_save_Color4f;
   table->Color4fv = gl_save_Color4fv;
   table->Color4ub = gl_save_Color4ub;
   table->Color4ubv = gl_save_Color4ubv;
   table->ColorMask = gl_save_ColorMask;
   table->ColorMaterial = gl_save_ColorMaterial;
   table->ColorPointer = _gamma_ColorPointer; /* NOT SAVED */
   table->CopyPixels = gl_save_CopyPixels;
   table->CopyTexImage1D = gl_save_CopyTexImage1D;
   table->CopyTexImage2D = gl_save_CopyTexImage2D;
   table->CopyTexSubImage1D = gl_save_CopyTexSubImage1D;
   table->CopyTexSubImage2D = gl_save_CopyTexSubImage2D;
   table->CullFace = gl_save_CullFace;
   table->DeleteLists = _gamma_DeleteLists;   /* NOT SAVED */
   table->DeleteTextures = _gamma_DeleteTextures;  /* NOT SAVED */
   table->DepthFunc = gl_save_DepthFunc;
   table->DepthMask = gl_save_DepthMask;
   table->DepthRange = gl_save_DepthRange;
   table->Disable = gl_save_Disable;
   table->DisableClientState = _gamma_DisableClientState;  /* NOT SAVED */
   table->DrawArrays = gl_save_DrawArrays;
   table->DrawBuffer = gl_save_DrawBuffer;
   table->DrawElements = gl_save_DrawElements;
   table->DrawPixels = gl_save_DrawPixels;
   table->EdgeFlag = gl_save_EdgeFlag;
   table->EdgeFlagPointer = _gamma_EdgeFlagPointer; /* NOT SAVED */
   table->Enable = gl_save_Enable;
   table->EnableClientState = _gamma_EnableClientState;   /* NOT SAVED */
   table->End = gl_save_End;
   table->EndList = _gamma_EndList;   /* NOT SAVED */
   table->EvalCoord1f = gl_save_EvalCoord1f;
   table->EvalCoord2f = gl_save_EvalCoord2f;
   table->EvalMesh1 = gl_save_EvalMesh1;
   table->EvalMesh2 = gl_save_EvalMesh2;
   table->EvalPoint1 = gl_save_EvalPoint1;
   table->EvalPoint2 = gl_save_EvalPoint2;
   table->FeedbackBuffer = _gamma_FeedbackBuffer;   /* NOT SAVED */
   table->Finish = _gamma_Finish;   /* NOT SAVED */
   table->Flush = _gamma_Flush;   /* NOT SAVED */
   table->Fogfv = gl_save_Fogfv;
   table->FrontFace = gl_save_FrontFace;
   table->Frustum = gl_save_Frustum;
   table->GenLists = _gamma_GenLists;   /* NOT SAVED */
   table->GenTextures = _gamma_GenTextures;   /* NOT SAVED */

   /* NONE OF THESE COMMANDS ARE COMPILED INTO DISPLAY LISTS */
   table->GetBooleanv = _gamma_GetBooleanv;
   table->GetClipPlane = _gamma_GetClipPlane;
   table->GetDoublev = _gamma_GetDoublev;
   table->GetError = _gamma_GetError;
   table->GetFloatv = _gamma_GetFloatv;
   table->GetIntegerv = _gamma_GetIntegerv;
   table->GetString = _gamma_GetString;
   table->GetLightfv = _gamma_GetLightfv;
   table->GetLightiv = _gamma_GetLightiv;
   table->GetMapdv = _gamma_GetMapdv;
   table->GetMapfv = _gamma_GetMapfv;
   table->GetMapiv = _gamma_GetMapiv;
   table->GetMaterialfv = _gamma_GetMaterialfv;
   table->GetMaterialiv = _gamma_GetMaterialiv;
   table->GetPixelMapfv = _gamma_GetPixelMapfv;
   table->GetPixelMapuiv = _gamma_GetPixelMapuiv;
   table->GetPixelMapusv = _gamma_GetPixelMapusv;
   table->GetPointerv = _gamma_GetPointerv;
   table->GetPolygonStipple = _gamma_GetPolygonStipple;
   table->GetTexEnvfv = _gamma_GetTexEnvfv;
   table->GetTexEnviv = _gamma_GetTexEnviv;
   table->GetTexGendv = _gamma_GetTexGendv;
   table->GetTexGenfv = _gamma_GetTexGenfv;
   table->GetTexGeniv = _gamma_GetTexGeniv;
   table->GetTexImage = _gamma_GetTexImage;
   table->GetTexLevelParameterfv = _gamma_GetTexLevelParameterfv;
   table->GetTexLevelParameteriv = _gamma_GetTexLevelParameteriv;
   table->GetTexParameterfv = _gamma_GetTexParameterfv;
   table->GetTexParameteriv = _gamma_GetTexParameteriv;

   table->Hint = gl_save_Hint;
   table->IndexMask = gl_save_IndexMask;
   table->Indexf = gl_save_Indexf;
   table->Indexi = gl_save_Indexi;
   table->IndexPointer = _gamma_IndexPointer; /* NOT SAVED */
   table->InitNames = gl_save_InitNames;
   table->InterleavedArrays = _gamma_InterleavedArrays; /* NOT SAVED */
   table->IsEnabled = _gamma_IsEnabled;   /* NOT SAVED */
   table->IsTexture = _gamma_IsTexture;   /* NOT SAVED */
   table->IsList = _gamma_IsList;   /* NOT SAVED */
   table->LightModelfv = gl_save_LightModelfv;
   table->Lightfv = gl_save_Lightfv;
   table->LineStipple = gl_save_LineStipple;
   table->LineWidth = gl_save_LineWidth;
   table->ListBase = gl_save_ListBase;
   table->LoadIdentity = gl_save_LoadIdentity;
   table->LoadMatrixf = gl_save_LoadMatrixf;
   table->LoadName = gl_save_LoadName;
   table->LogicOp = gl_save_LogicOp;
   table->Map1f = gl_save_Map1f;
   table->Map2f = gl_save_Map2f;
   table->MapGrid1f = gl_save_MapGrid1f;
   table->MapGrid2f = gl_save_MapGrid2f;
   table->Materialfv = gl_save_Materialfv;
   table->MatrixMode = gl_save_MatrixMode;
   table->MultMatrixf = gl_save_MultMatrixf;
   table->NewList = gl_save_NewList;
   table->Normal3f = gl_save_Normal3f;
   table->Normal3fv = gl_save_Normal3fv;
   table->NormalPointer = _gamma_NormalPointer;  /* NOT SAVED */
   table->Ortho = gl_save_Ortho;
   table->PassThrough = gl_save_PassThrough;
   table->PixelMapfv = gl_save_PixelMapfv;
   table->PixelStorei = _gamma_PixelStorei;   /* NOT SAVED */
   table->PixelTransferf = gl_save_PixelTransferf;
   table->PixelZoom = gl_save_PixelZoom;
   table->PointSize = gl_save_PointSize;
   table->PolygonMode = gl_save_PolygonMode;
   table->PolygonOffset = gl_save_PolygonOffset;
   table->PolygonStipple = gl_save_PolygonStipple;
   table->PopAttrib = gl_save_PopAttrib;
   table->PopClientAttrib = _gamma_PopClientAttrib;  /* NOT SAVED */
   table->PopMatrix = gl_save_PopMatrix;
   table->PopName = gl_save_PopName;
   table->PrioritizeTextures = gl_save_PrioritizeTextures;
   table->PushAttrib = gl_save_PushAttrib;
   table->PushClientAttrib = _gamma_PushClientAttrib;  /* NOT SAVED */
   table->PushMatrix = gl_save_PushMatrix;
   table->PushName = gl_save_PushName;
   table->RasterPos4f = gl_save_RasterPos4f;
   table->ReadBuffer = gl_save_ReadBuffer;
   table->ReadPixels = _gamma_ReadPixels;   /* NOT SAVED */
   table->Rectf = gl_save_Rectf;
   table->RenderMode = _gamma_RenderMode;   /* NOT SAVED */
   table->Rotatef = gl_save_Rotatef;
   table->Scalef = gl_save_Scalef;
   table->Scissor = gl_save_Scissor;
   table->SelectBuffer = _gamma_SelectBuffer;   /* NOT SAVED */
   table->ShadeModel = gl_save_ShadeModel;
   table->StencilFunc = gl_save_StencilFunc;
   table->StencilMask = gl_save_StencilMask;
   table->StencilOp = gl_save_StencilOp;
   table->TexCoord2f = gl_save_TexCoord2f;
   table->TexCoord2fv = gl_save_TexCoord2fv;
   table->TexCoord3fv = gl_save_TexCoord3fv;
   table->TexCoord4f = gl_save_TexCoord4f;
   table->TexCoordPointer = _gamma_TexCoordPointer;  /* NOT SAVED */
   table->TexEnvfv = gl_save_TexEnvfv;
   table->TexGenfv = gl_save_TexGenfv;
   table->TexImage1D = gl_save_TexImage1D;
   table->TexImage2D = gl_save_TexImage2D;
   table->TexSubImage1D = gl_save_TexSubImage1D;
   table->TexSubImage2D = gl_save_TexSubImage2D;
   table->TexParameterfv = gl_save_TexParameterfv;
   table->Translatef = gl_save_Translatef;
   table->Vertex2f = gl_save_Vertex2f;
   table->Vertex3f = gl_save_Vertex3f;
   table->Vertex4f = gl_save_Vertex4f;
   table->Vertex3fv = gl_save_Vertex3fv;
   table->VertexPointer = _gamma_VertexPointer;  /* NOT SAVED */
   table->Viewport = gl_save_Viewport;
}
#endif
