#ifndef _GLX_drawable_h_
#define _GLX_drawable_h_

/* $XFree86: xc/programs/Xserver/GL/glx/glxdrawable.h,v 1.2 1999/06/14 07:31:26 dawes Exp $ */
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

typedef struct {

    DrawablePtr pDraw;
    __GLXvisualConfig *pGlxVisual;
    __GLXscreenInfo *pGlxScreen;
    ScreenPtr pScreen;
    Bool idExists;
    int refcnt;

} __GLXpixmap;

struct __GLXdrawablePrivateRec {
    /*
    ** list of drawable private structs
    */
    struct __GLXdrawablePrivateRec *last;
    struct __GLXdrawablePrivateRec *next;

    DrawablePtr pDraw;
    XID drawId;
    __GLXpixmap *pGlxPixmap;

    /*
    ** Either DRAWABLE_PIXMAP or DRAWABLE_WINDOW, copied from pDraw above.
    ** Needed by the resource freer because pDraw might already have been
    ** freed.
    */
    int type;

    /*
    ** Configuration of the visual to which this drawable was created.
    */
    __GLXvisualConfig *pGlxVisual;

    /*
    ** cached drawable size and origin
    */
    GLint xorigin, yorigin;
    GLint width, height;

    /*
    ** list of contexts bound to this drawable
    */
    struct __GLXcontextRec *glxc;

    /*
    ** "methods" that the drawble should be able to respond to.
    */
    void (*freeBuffers)(struct __GLXdrawablePrivateRec *);
    void (*updatePalette)(struct __GLXdrawablePrivateRec *);
    GLboolean (*swapBuffers)(struct __GLXdrawablePrivateRec *);

    /*
    ** The GL drawable (information shared between GLX and the GL core
    */
    __GLdrawablePrivate glPriv;

    /*
    ** reference count
    */
    int refCount;
};

#endif /* !__GLX_drawable_h__ */
