#ifndef _GLX_screens_h_
#define _GLX_screens_h_

/* $XFree86: xc/programs/Xserver/GL/glx/glxscreens.h,v 1.2 1999/06/14 07:31:32 dawes Exp $ */
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

#include "GL/internal/glcore.h"

/*
** Screen dependent data.  These methods are the interface between the DIX
** and DDX layers of the GLX server extension.  The methods provide an
** interface for context management on a screen.
*/
typedef struct {
    /*
    ** Probe the screen and see if it supports GL rendering.  It will
    ** return GL_FALSE if it doesn't, GL_TRUE otherwise.
    */
    Bool (*screenProbe)(int screen);

    /*
    ** Create a context using configuration information from modes.
    ** Use imports as callbacks back to the OS. Return an opaque handle
    ** on the context (NULL if failure).
    */
    __GLinterface *(*createContext)(__GLimports *imports,
				    __GLcontextModes *modes,
				    __GLinterface *shareGC);

    /*
    ** Create a buffer using information from glxPriv.  This routine
    ** sets up any wrappers necessary to resize, swap or destroy the
    ** buffer.
    */
    void (*createBuffer)(__GLXdrawablePrivate *glxPriv);

    __GLXvisualConfig *pGlxVisual;
    void **pVisualPriv;
    GLint numVisuals;
    GLint numUsableVisuals;

    char *GLXvendor;
    char *GLXversion;
    char *GLXextensions;

    /*
    ** Things that are not statically set.
    */
    Bool (*WrappedPositionWindow)(WindowPtr pWin, int x, int y);

} __GLXscreenInfo;


extern void __glXScreenInit(GLint);

#endif /* !__GLX_screens_h__ */
