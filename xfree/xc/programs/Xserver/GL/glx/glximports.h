#ifndef _glximports_h_
#define _glximports_h_

/* $XFree86: xc/programs/Xserver/GL/glx/glximports.h,v 1.2 1999/06/14 07:31:30 dawes Exp $ */
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

extern void *__glXImpMalloc(__GLcontext *gc, size_t size);
extern void *__glXImpCalloc(__GLcontext *gc, size_t nElem, size_t eSize);
extern void *__glXImpRealloc(__GLcontext *gc, void *addr, size_t newSize);
extern void  __glXImpFree(__GLcontext *gc, void *addr);

extern void  __glXImpWarning(__GLcontext *gc, char *msg);
extern void  __glXImpFatal(__GLcontext *gc, char *msg);

extern char *__glXImpGetenv(__GLcontext *gc, const char *var);
extern int   __glXImpSprintf(__GLcontext *gc, char *str, const char *fmt, ...);
extern void *__glXImpFopen(__GLcontext *gc, const char *path, 
			   const char *mode);
extern int   __glXImpFclose(__GLcontext *gc, void *stream);
extern int   __glXImpFprintf(__GLcontext *gc, void *stream, 
			     const char *fmt, ...);

extern __GLdrawablePrivate *__glXImpGetDrawablePrivate(__GLcontext *gc);


#endif /* _glximports_h_ */

