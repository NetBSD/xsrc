/* $XFree86: xc/lib/GL/mesa/src/drv/gamma/gamma_texture.c,v 1.3 2000/06/17 00:02:56 martin Exp $ */
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

#ifdef GLX_DIRECT_RENDERING

#include <Xarch.h>
#include <X11/Xlibint.h>
#include "gamma_init.h"
#include <string.h>

extern void gammaTOInit(gammaTexObj *t);
void *texHash = NULL; /* Hash table to hold texture objects */

void gammaTOInit(gammaTexObj *t)
{
    int i;

    t->TextureAddressMode = (TextureAddressModeDisable |
			     TAM_SWrap_Repeat |
			     TAM_TWrap_Repeat |
			     TAM_Operation_3D |
			     TAM_LODEnable |
			     TAM_DY_Enable |
			     TAM_TexMapType_2D);

    t->TextureReadMode = (TextureReadModeDisable |
			  TRM_Mag_Linear |
			  TRM_Min_NearestMMLinear |
			  TRM_UWrap_Repeat |
			  TRM_VWrap_Repeat |
			  TRM_TexMapType_2D |
			  TRM_MipMapEnable |
			  TRM_PrimaryCacheEnable |
			  TRM_FBSourceAddr_None |
			  TRM_BorderClamp);

    t->TextureColorMode = (TextureColorModeDisable |
			   TCM_Modulate |
			   TCM_OpenGLType |
			   TCM_KdDDA_Disable |
			   TCM_KsDDA_Disable |
			   TCM_BaseFormat_RGBA |
			   TCM_LoadMode_None);

    t->TextureFilterMode = (TextureFilterModeDisable);

#if X_BYTE_ORDER == X_LITTLE_ENDIAN
    t->TextureFormat = (TF_LittleEndian |
#else
    t->TextureFormat = (TF_BigEndian |
#endif
			TF_16Bit_565 |
			TF_ColorOrder_RGB |
			TF_Compnents_4 |
			TF_OutputFmt_Texel);

    for (i = 0; i < MIPMAP_LEVELS; i++) {
	t->image[i] = NULL;
	t->TextureBaseAddr[i] = 0;
    }
}

gammaTexObj *gammaTOFind(GLuint ID)
{
    gammaTexObj *texObj;
    int retcode;

    /* Create the hash table */
    if (!texHash) texHash = drmHashCreate();

    retcode = drmHashLookup(texHash, ID, (void **)&texObj);
    if (retcode > 0) { /* Not Found */
	texObj = Xmalloc(sizeof(gammaTexObj));
	gammaTOInit(texObj);
	drmHashInsert(texHash, ID, texObj);
    } else if (retcode < 0) {
	/* NOT_DONE: Error!!! */
    }

    return texObj;
}

void gammaTODelete(GLuint ID)
{
    int retcode;
    gammaTexObj *texObj;

    /* Create the hash table */
    if (!texHash) texHash = drmHashCreate();

    retcode = drmHashLookup(texHash, ID, (void **)&texObj);
    if (!retcode) { /* Found */
	drmHashDelete(texHash, ID);
	Xfree(texObj);
    } else if (retcode < 0) {
	/* NOT_DONE: Error!!! */
    }
}

int gammaTOLoad(void *private, unsigned long address,
		int width, int height, int texel_width,
		const unsigned char *image)
{
    unsigned int *wdata = (unsigned int *)image;
    int offset = 0;
    int words, depthLog2;

    CALC_LOG2(depthLog2, texel_width);
    words = (width*height) << (depthLog2-5);

    /* NOT_DONE: Use patch mode, if possible */
    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
    WRITE(gCCPriv->buf, LBWindowBase, address);
    WRITE(gCCPriv->buf, TextureCacheControl, (TCC_Enable | TCC_Invalidate));
    WRITE(gCCPriv->buf, WaitForCompletion, 0);

    /* Send the old DMA buffer to the HW, and get a clean DMA buffer */
    FLUSH_DMA_BUFFER(nullCC, gCCPriv);

    while (offset < words) {
	int count = gCCPriv->bufSize; /* Buf size in words */
	count -= 3; /* 3 is for TDO(Tag+Data) & TD(Tag) */
	if (count > words-offset) count = words-offset;

	WRITE(gCCPriv->buf, TextureDownloadOffset, offset);
	gCCPriv->buf++->i = GlintTextureDataTag | ((count-1) << 16);

	if (wdata)
	    memcpy(gCCPriv->buf, wdata, count*4);
	else
	    memset(gCCPriv->buf, 0, count*4);

	gCCPriv->buf += count;
	gCCPriv->bufCount = count+3; /* 3 is for TDO(Tag+Data) & TD(Tag) */
	FLUSH_DMA_BUFFER(nullCC, gCCPriv);

	offset += count;
	wdata += count;
    }

    /* Make sure the texture has been completely written */
    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
    WRITE(gCCPriv->buf, WaitForCompletion, 0);
    WRITE(gCCPriv->buf, LBWindowBase, gCCPriv->LBWindowBase);
    FLUSH_DMA_BUFFER(nullCC, gCCPriv);

    return 0;
}

int gammaTOLoadSub(void *private, unsigned long address,
		   int width, int height, int texel_width,
		   int xoffset, int yoffset,
		   int subimage_width, int subimage_height,
		   const unsigned char *image)
{
    unsigned int *wdata = (unsigned int *)image;
    int depthLog2;
    int h;

    if (!wdata)
	return 0;

    CALC_LOG2(depthLog2, texel_width);

    /* NOT_DONE: Use patch mode, if possible */
    CHECK_DMA_BUFFER(gCC, gCCPriv, 3);
    WRITE(gCCPriv->buf, LBWindowBase, address);
    WRITE(gCCPriv->buf, TextureCacheControl, (TCC_Enable | TCC_Invalidate));
    WRITE(gCCPriv->buf, WaitForCompletion, 0);

    /* Send the old DMA buffer to the HW, and get a clean DMA buffer */
    FLUSH_DMA_BUFFER(nullCC, gCCPriv);

    /* Make sure we are in bounds */
    if (xoffset < 0)              xoffset = 0;
    if (yoffset < 0)              yoffset = 0;
    if (subimage_width  > width)  subimage_width  = width;
    if (subimage_height > height) subimage_height = height;

    /* NOT_DONE: Must handle starting in the middle of a word properly */
    for (h = yoffset; h < yoffset+subimage_height; h++) {
	int start = (h*width + xoffset) << (depthLog2-5);
	int words = subimage_width << (depthLog2-5);
	int offset = 0;

	while (offset < words) {
	    int count = gCCPriv->bufSize; /* Buf size in words */
	    count -= 3; /* 3 is for TDO(Tag+Data) & TD(Tag) */
	    if (count > words-offset) count = words-offset;

	    WRITE(gCCPriv->buf, TextureDownloadOffset, start+offset);
	    gCCPriv->buf++->i = GlintTextureDataTag | ((count-1) << 16);

	    memcpy(gCCPriv->buf, wdata, count*4);

	    gCCPriv->buf += count;
	    gCCPriv->bufCount = count+3; /* 3 is for TDO(Tag+Data) & TD(Tag) */
	    FLUSH_DMA_BUFFER(nullCC, gCCPriv);

	    offset += count;
	    wdata += count;
	}
    }

    /* Make sure the texture has been completely written */
    CHECK_DMA_BUFFER(gCC, gCCPriv, 2);
    WRITE(gCCPriv->buf, WaitForCompletion, 0);
    WRITE(gCCPriv->buf, LBWindowBase, gCCPriv->LBWindowBase);
    FLUSH_DMA_BUFFER(nullCC, gCCPriv);

    return 0;
}

#endif
