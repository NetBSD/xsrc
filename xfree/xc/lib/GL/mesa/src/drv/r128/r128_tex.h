/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_tex.h,v 1.3 2000/12/12 17:17:08 dawes Exp $ */
/**************************************************************************

Copyright 1999, 2000 ATI Technologies Inc. and Precision Insight, Inc.,
                                               Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, PRECISION INSIGHT AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#ifndef _R128_TEX_H_
#define _R128_TEX_H_

#ifdef GLX_DIRECT_RENDERING

extern void r128UpdateTextureState( GLcontext *ctx );

extern int r128UploadTexImages( r128ContextPtr r128ctx, r128TexObjPtr t );

extern void r128AgeTextures( r128ContextPtr r128ctx, int heap );
extern void r128DestroyTexObj( r128ContextPtr r128ctx, r128TexObjPtr t );

extern void r128PrintLocalLRU( r128ContextPtr r128ctx, int heap );
extern void r128PrintGlobalLRU( r128ContextPtr r128ctx, int heap );

extern void r128DDInitTextureFuncs( GLcontext *ctx );


#define R128PACKCOLOR332(r, g, b)                                             \
    (((r) & 0xe0) | (((g) & 0xe0) >> 3) | (((b) & 0xc0) >> 6))

#define R128PACKCOLOR1555(r, g, b, a)                                         \
    ((((r) & 0xf8) << 7) | (((g) & 0xf8) << 2) | (((b) & 0xf8) >> 3) |        \
     ((a) ? 0x8000 : 0))

#define R128PACKCOLOR565(r, g, b)                                             \
    ((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | (((b) & 0xf8) >> 3))

#define R128PACKCOLOR888(r, g, b)                                             \
    (((r) << 16) | ((g) << 8) | (b))

#define R128PACKCOLOR8888(r, g, b, a)                                         \
    (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

#define R128PACKCOLOR4444(r, g, b, a)                                         \
    ((((a) & 0xf0) << 8) | (((r) & 0xf0) << 4) | ((g) & 0xf0) | ((b) >> 4))

static __inline__ CARD32 r128PackColor( GLuint bpp,
					GLubyte r, GLubyte g,
					GLubyte b, GLubyte a )
{
    switch ( bpp ) {
    case 16: return R128PACKCOLOR565( r, g, b );
    case 32: return R128PACKCOLOR8888( r, g, b, a );
    default: return 0;
    }
}

#endif
#endif /* _R128_TEX_H_ */
