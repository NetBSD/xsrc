#ifndef _GLX_context_h_
#define _GLX_context_h_

/* $XFree86: xc/programs/Xserver/GL/glx/glxcontext.h,v 1.2 1999/06/14 07:31:26 dawes Exp $ */
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

typedef struct __GLXcontextRec __GLXcontext;

#include "GL/internal/glcore.h"

struct __GLXcontextRec {
    /*
    ** list of context structs
    */
    struct __GLXcontextRec *last;
    struct __GLXcontextRec *next;

    /*
    ** list of contexts bound to the same drawable
    */
    struct __GLXcontextRec *nextPriv;

    /*
    ** Opaque pointer the context object created by the GL that the
    ** server is bound with.  Never dereferenced by this code, but used
    ** as a handle to feed to the routines in the screen info struct.
    */
    __GLinterface *gc;

    /*
    ** mode struct for this context
    */
    __GLcontextModes *modes;

    /*
    ** Pointer to screen info data for this context.  This is set
    ** when the context is created.
    */
    ScreenPtr pScreen;
    __GLXscreenInfo *pGlxScreen;

    /*
    ** This context is created with respect to this visual.
    */
    VisualRec *pVisual;
    __GLXvisualConfig *pGlxVisual;

    /*
    ** The XID of this context.
    */
    XID id;

    /*
    ** The XID of the shareList context.
    */
    XID share_id;

    /*
    ** Visual id.
    */
    VisualID vid;

    /*
    ** screen number.
    */
    GLint screen;

    /*
    ** Whether this context's ID still exists.
    */
    GLboolean idExists;
    
    /*
    ** Whether this context is current for some client.
    */
    GLboolean isCurrent;
    
    /*
    ** Whether this context is a direct rendering context.
    */
    GLboolean isDirect;

    /*
    ** Window pending state
    */
    GLuint pendingState;

    /*
    ** This flag keeps track of whether there are unflushed GL commands.
    */
    GLboolean hasUnflushedCommands;
    
    /*
    ** Current rendering mode for this context.
    */
    GLenum renderMode;
    
    /*
    ** Buffers for feedback and selection.
    */
    GLfloat *feedbackBuf;
    GLint feedbackBufSize;	/* number of elements allocated */
    GLuint *selectBuf;
    GLint selectBufSize;	/* number of elements allocated */

    /*
    ** Set only if current drawable is a glx pixmap.
    */
    __GLXpixmap *pGlxPixmap;

    /*
    ** The drawable private this context is bound to
    */
    __GLXdrawablePrivate *glxPriv;
};

/* pending state defines */
#define __GLX_PENDING_RESIZE	0x1
#define	__GLX_PENDING_DESTROY	0x2
#define __GLX_PENDING_SWAP	0x4

#endif /* !__GLX_context_h__ */
