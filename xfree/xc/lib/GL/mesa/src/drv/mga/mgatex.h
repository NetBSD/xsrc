/*
 * GLX Hardware Device Driver for Matrox Millenium G200
 * Copyright (C) 1999 Wittawat Yamwong
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
 * WITTAWAT YAMWONG, OR ANY OTHER CONTRIBUTORS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 *    John Carmack <johnc@idsoftware.com>
 *    Keith Whitwell <keithw@precisioninsight.com>
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/mga/mgatex.h,v 1.4 2000/09/24 13:51:07 alanh Exp $ */

#ifndef MGATEX_INC
#define MGATEX_INC

#include "mgacontext.h"

typedef struct mga_texture_object_s *mgaTextureObjectPtr;


/* Called before a primitive is rendered to make sure the texture
 * state is properly setup.  Texture residence is checked later
 * when we grab the lock.
 */
void mgaUpdateTextureState( GLcontext *ctx );


/* Driver functions which are called directly from mesa */

void mgaConvertTexture( GLuint *dest, int texelBytes,
			struct gl_texture_image *image,
			int x, int y, int width, int height );



int mgaUploadTexImages( mgaContextPtr mmesa, mgaTextureObjectPtr t );

void mgaDestroyTexObj( mgaContextPtr mmesa, mgaTextureObjectPtr t );

void mgaAgeTextures( mgaContextPtr mmesa, int heap );

void mgaDDInitTextureFuncs( GLcontext *ctx );


#endif
