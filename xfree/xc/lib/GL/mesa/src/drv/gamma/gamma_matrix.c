/* $XFree86: xc/lib/GL/mesa/src/drv/gamma/gamma_matrix.c,v 1.3 2000/06/17 00:02:56 martin Exp $ */
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

#include "gamma_init.h"

float IdentityMatrix[16] =
{
    1.0, 0.0, 0.0, 0.0,
    0.0, 1.0, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.0, 0.0, 0.0, 1.0
};

static void doMultMatrix(GLfloat *p, GLfloat *a, GLfloat *b)
{
/* Column-major order multiply */
#define MATRIX_MULT(__p, __a, __b)                                     \
do {                                                                   \
    GLint __row;                                                       \
    for (__row = 0; __row < 4; __row++) {                              \
	GLfloat __t[4];                                                \
        __t[0] = __a[__row];                                           \
	__t[1] = __a[__row+4];                                         \
	__t[2] = __a[__row+8];                                         \
	__t[3] = __a[__row+12];                                        \
	__p[__row]    = (__t[0] * __b[0] +                             \
			 __t[1] * __b[1] +                             \
			 __t[2] * __b[2] +                             \
			 __t[3] * __b[3]);                             \
	__p[__row+4]  = (__t[0] * __b[4] +                             \
			 __t[1] * __b[5] +                             \
			 __t[2] * __b[6] +                             \
			 __t[3] * __b[7]);                             \
	__p[__row+8]  = (__t[0] * __b[8] +                             \
			 __t[1] * __b[9] +                             \
			 __t[2] * __b[10] +                            \
			 __t[3] * __b[11]);                            \
	__p[__row+12] = (__t[0] * __b[12] +                            \
			 __t[1] * __b[13] +                            \
			 __t[2] * __b[14] +                            \
			 __t[3] * __b[15]);                            \
    }                                                                  \
} while(0)

    if (p == a || p == b) {
	GLfloat t[16];
	int i;

	MATRIX_MULT(t,a,b);
	for (i = 0; i < 16; i++) p[i] = t[i];
    } else {
	MATRIX_MULT(p,a,b);
    }

}

void gammaSetMatrix(GLfloat m[16])
{
    int i;

    switch (gCCPriv->MatrixMode) {
    case GL_MODELVIEW:
	for (i = 0; i < 16; i++) gCCPriv->ModelView[i] = m[i];
	doMultMatrix(gCCPriv->ModelViewProj,
		     gCCPriv->Proj, gCCPriv->ModelView);
	break;

    case GL_PROJECTION:
	for (i = 0; i < 16; i++) gCCPriv->Proj[i] = m[i];
	doMultMatrix(gCCPriv->ModelViewProj, gCCPriv->Proj,
		     gCCPriv->ModelView);
	break;

    case GL_TEXTURE:
	for (i = 0; i < 16; i++) gCCPriv->Texture[i] = m[i];
	break;

    default:
	/* ERROR!!! -- how did this happen? */
	return;
    }

}

void gammaMultMatrix(GLfloat *m)
{
    switch (gCCPriv->MatrixMode) {
    case GL_MODELVIEW:
	doMultMatrix(gCCPriv->ModelView, gCCPriv->ModelView, m);
	doMultMatrix(gCCPriv->ModelViewProj,
		     gCCPriv->Proj, gCCPriv->ModelView);
	break;

    case GL_PROJECTION:
	doMultMatrix(gCCPriv->Proj, gCCPriv->Proj, m);
	doMultMatrix(gCCPriv->ModelViewProj,
		     gCCPriv->Proj, gCCPriv->ModelView);
	break;

    case GL_TEXTURE:
	doMultMatrix(gCCPriv->Texture, gCCPriv->Texture, m);
	break;

    default:
	/* ERROR!!! -- how did this happen? */
	break;
    }
}

void gammaLoadHWMatrix(void)
{
    switch (gCCPriv->MatrixMode) {
    case GL_MODELVIEW:
#if 0
	/* The ModelView matrix is not currently used */
	CHECK_DMA_BUFFER(gCC, gCCPriv, 16);
	WRITEF(gCCPriv->buf, ModelViewMatrix0,  gCCPriv->ModelView[0]);
	WRITEF(gCCPriv->buf, ModelViewMatrix1,  gCCPriv->ModelView[1]);
	WRITEF(gCCPriv->buf, ModelViewMatrix2,  gCCPriv->ModelView[2]);
	WRITEF(gCCPriv->buf, ModelViewMatrix3,  gCCPriv->ModelView[3]);
	WRITEF(gCCPriv->buf, ModelViewMatrix4,  gCCPriv->ModelView[4]);
	WRITEF(gCCPriv->buf, ModelViewMatrix5,  gCCPriv->ModelView[5]);
	WRITEF(gCCPriv->buf, ModelViewMatrix6,  gCCPriv->ModelView[6]);
	WRITEF(gCCPriv->buf, ModelViewMatrix7,  gCCPriv->ModelView[7]);
	WRITEF(gCCPriv->buf, ModelViewMatrix8,  gCCPriv->ModelView[8]);
	WRITEF(gCCPriv->buf, ModelViewMatrix9,  gCCPriv->ModelView[9]);
	WRITEF(gCCPriv->buf, ModelViewMatrix10, gCCPriv->ModelView[10]);
	WRITEF(gCCPriv->buf, ModelViewMatrix11, gCCPriv->ModelView[11]);
	WRITEF(gCCPriv->buf, ModelViewMatrix12, gCCPriv->ModelView[12]);
	WRITEF(gCCPriv->buf, ModelViewMatrix13, gCCPriv->ModelView[13]);
	WRITEF(gCCPriv->buf, ModelViewMatrix14, gCCPriv->ModelView[14]);
	WRITEF(gCCPriv->buf, ModelViewMatrix15, gCCPriv->ModelView[15]);
#endif
    case GL_PROJECTION:
	CHECK_DMA_BUFFER(gCC, gCCPriv, 16);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix0,  gCCPriv->ModelViewProj[0]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix1,  gCCPriv->ModelViewProj[1]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix2,  gCCPriv->ModelViewProj[2]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix3,  gCCPriv->ModelViewProj[3]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix4,  gCCPriv->ModelViewProj[4]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix5,  gCCPriv->ModelViewProj[5]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix6,  gCCPriv->ModelViewProj[6]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix7,  gCCPriv->ModelViewProj[7]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix8,  gCCPriv->ModelViewProj[8]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix9,  gCCPriv->ModelViewProj[9]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix10, gCCPriv->ModelViewProj[10]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix11, gCCPriv->ModelViewProj[11]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix12, gCCPriv->ModelViewProj[12]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix13, gCCPriv->ModelViewProj[13]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix14, gCCPriv->ModelViewProj[14]);
	WRITEF(gCCPriv->buf,
	       ModelViewProjectionMatrix15, gCCPriv->ModelViewProj[15]);
	break;

    case GL_TEXTURE:
	CHECK_DMA_BUFFER(gCC, gCCPriv, 16);
	WRITEF(gCCPriv->buf, TextureMatrix0,  gCCPriv->Texture[0]);
	WRITEF(gCCPriv->buf, TextureMatrix1,  gCCPriv->Texture[1]);
	WRITEF(gCCPriv->buf, TextureMatrix2,  gCCPriv->Texture[2]);
	WRITEF(gCCPriv->buf, TextureMatrix3,  gCCPriv->Texture[3]);
	WRITEF(gCCPriv->buf, TextureMatrix4,  gCCPriv->Texture[4]);
	WRITEF(gCCPriv->buf, TextureMatrix5,  gCCPriv->Texture[5]);
	WRITEF(gCCPriv->buf, TextureMatrix6,  gCCPriv->Texture[6]);
	WRITEF(gCCPriv->buf, TextureMatrix7,  gCCPriv->Texture[7]);
	WRITEF(gCCPriv->buf, TextureMatrix8,  gCCPriv->Texture[8]);
	WRITEF(gCCPriv->buf, TextureMatrix9,  gCCPriv->Texture[9]);
	WRITEF(gCCPriv->buf, TextureMatrix10, gCCPriv->Texture[10]);
	WRITEF(gCCPriv->buf, TextureMatrix11, gCCPriv->Texture[11]);
	WRITEF(gCCPriv->buf, TextureMatrix12, gCCPriv->Texture[12]);
	WRITEF(gCCPriv->buf, TextureMatrix13, gCCPriv->Texture[13]);
	WRITEF(gCCPriv->buf, TextureMatrix14, gCCPriv->Texture[14]);
	WRITEF(gCCPriv->buf, TextureMatrix15, gCCPriv->Texture[15]);
	break;

    default:
	/* ERROR!!! -- how did this happen? */
	break;
    }
}

#endif
