/* $XFree86: xc/lib/GL/mesa/src/drv/gamma/gamma_texture.h,v 1.2 2000/02/23 04:46:45 martin Exp $ */
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

#ifndef _GAMMA_TEXTURE_H_
#define _GAMMA_TEXTURE_H_

#ifdef GLX_DIRECT_RENDERING

#define MIPMAP_LEVELS 12

typedef struct {
    GLuint         ID;
    void          *image[MIPMAP_LEVELS];
    int            TextureAddressMode;
    int            TextureReadMode;
    int            TextureColorMode;
    int            TextureFilterMode;
    int            TextureFormat;
    unsigned long  TextureBaseAddr[MIPMAP_LEVELS];
} gammaTexObj;

extern gammaTexObj *gammaTOFind(GLuint ID);
extern void         gammaTODelete(GLuint ID);

extern int          gammaTOLoad(void *private, unsigned long address,
				int width, int height, int texel_width,
				const unsigned char *image);
extern int          gammaTOLoadSub(void *private, unsigned long address,
				   int width, int height, int texel_width,
				   int xoffset, int yoffset,
				   int subimage_width, int subimage_height,
				   const unsigned char *image);

#endif

#endif /* _GAMMA_INIT_H_ */
