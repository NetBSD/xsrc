/* $XFree86: xc/programs/Xserver/GL/glx/glximports.c,v 1.4 1999/07/18 08:34:23 dawes Exp $ */
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

#include "glxserver.h"
#include "glxcontext.h"
#include "glximports.h"

void *__glXImpMalloc(__GLcontext *gc, size_t size)
{
    void *addr;

    if (size == 0) {
	return NULL;
    }
    addr = xalloc(size);
    if (addr == NULL) {
	/* XXX: handle out of memory error */
	return NULL;
    }
    return addr;
}

void *__glXImpCalloc(__GLcontext *gc, size_t numElements, size_t elementSize)
{
    void *addr;
    size_t size;

    if ((numElements == 0) || (elementSize == 0)) {
	return NULL;
    }
    size = numElements * elementSize;
    addr = xalloc(size);
    if (addr == NULL) {
	/* XXX: handle out of memory error */
	return NULL;
    }
    /* zero out memory */
    __glXMemset(addr, 0, size);

    return addr;
}

void __glXImpFree(__GLcontext *gc, void *addr)
{
    if (addr) {
	xfree(addr);
    }
}

void *__glXImpRealloc(__GLcontext *gc, void *addr, size_t newSize)
{
    void *newAddr;

    if (addr) {
	if (newSize == 0) {
	    xfree(addr);
	    return NULL;
	}
	newAddr = xrealloc(addr, newSize);
    } else {
	if (newSize == 0) {
	    return NULL;
	}
	newAddr = xalloc(newSize);
    }
    if (newAddr == NULL) {
	return NULL;	/* XXX: out of memory error */
    }

    return newAddr;
}

void __glXImpWarning(__GLcontext *gc, char *msg)
{
    ErrorF((char *)msg);
}

void __glXImpFatal(__GLcontext *gc, char *msg)
{
    ErrorF((char *)msg);
    __glXAbort();
}

char *__glXImpGetenv(__GLcontext *gc, const char *var)
{
    return __glXGetenv(var);
}

int __glXImpSprintf(__GLcontext *gc, char *str, const char *fmt, ...)
{
    va_list ap;
    int ret;

    /* have to deal with var args */
    va_start(ap, fmt);
    ret = __glXVsprintf(str, fmt, ap);
    va_end(ap);

    return ret;
}

void *__glXImpFopen(__GLcontext *gc, const char *path, const char *mode)
{
    return (void *) __glXFopen(path, mode);
}

int __glXImpFclose(__GLcontext *gc, void *stream)
{
    return __glXFclose((FILE *)stream);
}

int __glXImpFprintf(__GLcontext *gc, void *stream, const char *fmt, ...)
{
    va_list ap;
    int ret;

    /* have to deal with var args */
    va_start(ap, fmt);
    ret = __glXVfprintf((FILE *)stream, fmt, ap);
    va_end(ap);

    return ret;
}


__GLdrawablePrivate *__glXImpGetDrawablePrivate(__GLcontext *gc)
{
    __GLinterface *glci = (__GLinterface *) gc;
    __GLXcontext *glrc = (__GLXcontext *) glci->imports.other;

    return &glrc->glxPriv->glPriv;
}
