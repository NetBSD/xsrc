#ifndef _glxcmds_h_
#define _glxcmds_h_

/* $XFree86: xc/programs/Xserver/GL/glx/glxutil.h,v 1.2 1999/06/14 07:31:33 dawes Exp $ */
/*
** The contents of this file are subject to the GLX Public License Version 1.0
** (the "License"). You may not use this file except in compliance with the
** License. You may obtain a copy of the License at Silicon Graphics, Inc.,
** attn: Legal Services, 2011 N. Shoreline Blvd., Mountain View, CA 94043
** or at http://www.sgi.com/software/opensource/glx/license.html.
**
** Software distributed under the License is distributed on an "AS IS"
** basis. ALL WARRANTIES ARE DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY
** IMPLIED WARRANTIES OF MERCHANTABILITY, OF FITNESS FOR A PARTICULAR
** PURPOSE OR OF NON- INFRINGEMENT. See the License for the specific
** language governing rights and limitations under the License.
**
** The Original Software is GLX version 1.2 source code, released February,
** 1999. The developer of the Original Software is Silicon Graphics, Inc.
** Those portions of the Subject Software created by Silicon Graphics, Inc.
** are Copyright (c) 1991-9 Silicon Graphics, Inc. All Rights Reserved.
**
** $SGI$
*/

extern void __glXNop(void);

/* memory management */
extern void *__glXMalloc(size_t size);
extern void *__glXCalloc(size_t numElements, size_t elementSize);
extern void *__glXRealloc(void *addr, size_t newSize);
extern void __glXFree(void *ptr);

/* relate contexts with drawables */
extern void __glXAssociateContext(__GLXcontext *glxc, __GLXdrawablePrivate *glxPriv);
extern void __glXDeassociateContext(__GLXcontext *glxc, __GLXdrawablePrivate *glxPriv);

/* drawable operation */
extern void __glXGetDrawableSize(__GLdrawablePrivate *glPriv, 
				 GLint *x, GLint *y, 
				 GLuint *width, GLuint *height);
extern GLboolean __glXResizeDrawable(__GLdrawablePrivate *glPriv);
extern GLboolean __glXResizeDrawableBuffers(__GLXdrawablePrivate *glxPriv);
extern void __glXFormatGLModes(__GLcontextModes *modes, __GLXvisualConfig *config);

/* drawable management */
extern void __glXRefDrawablePrivate(__GLXdrawablePrivate *glxPriv);
extern void __glXUnrefDrawablePrivate(__GLXdrawablePrivate *glxPriv);
extern __GLXdrawablePrivate *__glXCreateDrawablePrivate(DrawablePtr pDraw, 
							XID glxpixmapId, 
							__GLcontextModes *modes);
extern GLboolean __glXDestroyDrawablePrivate(__GLXdrawablePrivate *glxPriv);
extern __GLXdrawablePrivate *__glXFindDrawablePrivate(XID glxpixmapId);
extern __GLXdrawablePrivate *__glXGetDrawablePrivate(DrawablePtr pDraw, 
						     XID glxpixmapId, 
						     __GLcontextModes *modes);
extern void __glXCacheDrawableSize(__GLXdrawablePrivate *glxPriv);

/* context helper routines */
extern __GLXcontext *__glXLookupContextByTag(__GLXclientState*, GLXContextTag);


#endif /* _glxcmds_h_ */

