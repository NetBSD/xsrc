/* $XFree86: xc/lib/GL/mesa/dri/dri_mesaint.h,v 1.9 2000/12/07 20:26:04 dawes Exp $ */
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
 *   Brian E. Paul <brian@precisioninsight.com>
 *
 */

#ifndef _DRI_MESAINT_H_
#define _DRI_MESAINT_H_

#ifdef GLX_DIRECT_RENDERING

#include <GL/glx.h>
#include "xf86dri.h"
#include "sarea.h"
#include "dri_mesa.h"
#include "dri_xmesaapi.h"


#define DRI_MESA_VALIDATE_DRAWABLE_INFO(dpy,scrn,pDrawPriv)  \
    do {                                                     \
	if (*(pDrawPriv->pStamp) != pDrawPriv->lastStamp) {  \
	    driMesaUpdateDrawableInfo(dpy,scrn,pDrawPriv);   \
	}                                                    \
    } while (0)


struct __DRIdrawablePrivateRec {
    /*
    ** Kernel drawable handle (not currently used).
    */
    drmDrawable hHWDrawable;

    /*
    ** Mesa's private frame buffer information.  This structure is opaque.
    */
    GLframebuffer *mesaBuffer;

    /*
    ** X's drawable ID associated with this private drawable.
    */
    GLXDrawable draw;

    /*
    ** Reference count for number of context's currently bound to this
    ** drawable.  Once the refcount reaches 0, the drawable can be
    ** destroyed.  This behavior will change with GLX 1.3.
    */
    int refcount;

    /*
    ** Index of this drawable's information in the SAREA.
    */
    unsigned int index;

    /*
    ** Pointer to the "drawable has changed ID" stamp in the SAREA.
    */
    unsigned int *pStamp;

    /*
    ** Last value of the stamp.  If this differs from the value stored
    ** at *pStamp, then the drawable information has been modified by
    ** the X server, and the drawable information (below) should be
    ** retrieved from the X server.
    */
    unsigned int lastStamp;

    /*
    ** Drawable information used in software fallbacks.
    */
    int x;
    int y;
    int w;
    int h;
    int numClipRects;
    XF86DRIClipRectPtr pClipRects;

    /*
    ** Information about the back and depthbuffer where different
    ** from above.
    */
    int backX;
    int backY;
    int backClipRectType;
    int numBackClipRects;
    XF86DRIClipRectPtr pBackClipRects;

    /*
    ** Pointer to context to which this drawable is currently bound.
    */
    __DRIcontextPrivate *driContextPriv;

    /*
    ** Pointer to screen on which this drawable was created.
    */
    __DRIscreenPrivate *driScreenPriv;
};

struct __DRIcontextPrivateRec {
    /*
    ** Kernel context handle used to access the device lock.
    */
    XID contextID;

    /*
    ** Kernel context handle used to access the device lock.
    */
    drmContext hHWContext;

    /*
    ** Mesa's private context information.  This structure is opaque.
    */
    GLcontext *mesaContext;

    /*
    ** Device driver's private context data.  This structure is opaque.
    */
    void *driverPrivate;

    /*
    ** This context's display pointer.
    */
    Display *display;

    /*
    ** Pointer to drawable currently bound to this context.
    */
    __DRIdrawablePrivate *driDrawablePriv;

    /*
    ** Pointer to screen on which this context was created.
    */
    __DRIscreenPrivate *driScreenPriv;
};

struct __DRIvisualPrivateRec {
    /*
    ** Mesa's private visual information.  This structure is opaque.
    */
    GLvisual *mesaVisual;

    /*
    ** X's visual ID associated with this private visual.
    */
    VisualID vid;
};

struct __DRIscreenPrivateRec {
    /*
    ** Display for this screen
    */
    Display *display;

    /*
    ** Current screen's number
    */
    int myNum;

    /*
    ** Core rendering library's visuals associated with the current
    ** screen.
    */
    __DRIvisualPrivate *visuals;
    int numVisuals;

    /*
    ** Function pointers associated with Mesa's GLX functions.
    */
    __MesaAPI MesaAPI;

    /*
    ** DDX / 2D driver version information.
    */
    int ddxMajor;
    int ddxMinor;
    int ddxPatch;

    /*
    ** DRM version information.
    */
    int drmMajor;
    int drmMinor;
    int drmPatch;

    /*
    ** ID used when the client sets the drawable lock.  The X server
    ** uses this value to detect if the client has died while holding
    ** the drawable lock.
    */
    int drawLockID;

    /*
    ** File descriptor returned when the kernel device driver is opened.
    ** It is used to:
    **   - authenticate client to kernel
    **   - map the frame buffer, SAREA, etc.
    **   - close the kernel device driver
    */
    int fd;

    /*
    ** SAREA pointer used to access:
    **   - the device lock
    **   - the device-independent per-drawable and per-context(?) information
    */
    XF86DRISAREAPtr pSAREA;

    /*
    ** Direct frame buffer access information used for software
    ** fallbacks.
    */
    unsigned char *pFB;
    int fbSize;
    int fbOrigin;
    int fbStride;
    int fbWidth;
    int fbHeight;
    int fbBPP;

    /*
    ** Device-dependent private information (stored in the SAREA).  This
    ** data is accessed by the client driver only.
    */
    void *pDevPriv;
    int devPrivSize;

    /*
    ** Dummy context to which drawables are bound when not bound to any
    ** other context. A dummy hHWContext is created for this context,
    ** and is used by the GL core when a HW lock is required but the
    ** drawable is not currently bound (e.g., potentially during a
    ** SwapBuffers request).  The dummy context is created when the
    ** first "real" context is created on this screen.
    */
    __DRIcontextPrivate dummyContextPriv;

    /*
    ** Hash table to hold the drawable information for this screen.
    */
    void *drawHash;

    /*
    ** Device-dependent private information (not stored in the SAREA).
    ** This pointer is never touched by the DRI layer.
    */
    void *private;

    /* If we're in full screen mode (via DRIOpenFullScreen), this points
       to the drawable that was bound.  Otherwise, this is NULL. */
    __DRIdrawablePrivate *fullscreen;
};


extern void driMesaUpdateDrawableInfo(Display *dpy, int scrn,
				      __DRIdrawablePrivate *pdp);

#endif
#endif /* _DRI_MESAINT_H_ */
