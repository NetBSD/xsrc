/* $XFree86: xc/lib/GL/mesa/dri/dri_xmesaapi.h,v 1.5 2000/12/07 20:26:04 dawes Exp $ */
/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
Copyright 2000 VA Linux Systems, Inc.
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
 *   Kevin E. Martin <martin@valinux.com>
 *   Brian E. Paul <brianp@valinux.com>
 */


#ifndef _DRI_XMESAAPI_H_
#define _DRI_XMESAAPI_H_

#include "glxclient.h"
#include "dri_mesa.h"
#include "../src/types.h"


extern GLboolean XMesaInitDriver( __DRIscreenPrivate *driScrnPriv );

extern void XMesaResetDriver( __DRIscreenPrivate *driScrnPriv );

/* Driver creates a GLvisual and returns pointer to it.
 */
extern GLvisual *XMesaCreateVisual(Display *dpy,
                                   __DRIscreenPrivate *driScrnPriv,
                                   const XVisualInfo *visinfo,
                                   const __GLXvisualConfig *config);

/* Driver creates its private context struct and hooks it into
 * the driContextPriv->driverPrivate field.  Return true/false for
 * success/error.
 */
extern GLboolean XMesaCreateContext( Display *dpy, GLvisual *mesaVis,
                                     __DRIcontextPrivate *driContextPriv );

/* Driver frees data attached to driContextPriv->driverPrivate pointer.
 */
extern void XMesaDestroyContext( __DRIcontextPrivate *driContextPriv );

/* Driver creates a GLframebuffer struct indicating which ancillary
 * buffers are in hardware or software.
 */
extern GLframebuffer *XMesaCreateWindowBuffer( Display *dpy,
                                           __DRIscreenPrivate *driScrnPriv,
                                           __DRIdrawablePrivate *driDrawPriv,
                                           GLvisual *mesaVis);

extern GLframebuffer *XMesaCreatePixmapBuffer( Display *dpy,
                                           __DRIscreenPrivate *driScrnPriv,
                                           __DRIdrawablePrivate *driDrawPriv,
                                           GLvisual *mesaVis);

extern GLboolean XMesaMakeCurrent( __DRIcontextPrivate *driContextPriv,
                                   __DRIdrawablePrivate *driDrawPriv,
                                   __DRIdrawablePrivate *driReadPriv );

extern GLboolean XMesaUnbindContext( __DRIcontextPrivate *driContextPriv );

extern void XMesaSwapBuffers( __DRIdrawablePrivate *driDrawPriv );

extern GLboolean XMesaOpenFullScreen(__DRIcontextPrivate *driContextPriv);
extern GLboolean XMesaCloseFullScreen(__DRIcontextPrivate *driContextPriv);



#define XMESA_VALIDATE_DRAWABLE_INFO(dpy, psp, pdp)                     \
do {                                                                    \
    while (*(pdp->pStamp) != pdp->lastStamp) {                          \
	DRM_UNLOCK(psp->fd, &psp->pSAREA->lock,                         \
		   pdp->driContextPriv->hHWContext);                    \
                                                                        \
	DRM_SPINLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);     \
	DRI_MESA_VALIDATE_DRAWABLE_INFO(dpy, psp->myNum, pdp);          \
	DRM_SPINUNLOCK(&psp->pSAREA->drawable_lock, psp->drawLockID);   \
                                                                        \
	DRM_LIGHT_LOCK(psp->fd, &psp->pSAREA->lock,                     \
		       pdp->driContextPriv->hHWContext);                \
    }                                                                   \
} while (0)



typedef struct __MesaAPIRec __MesaAPI;

struct __MesaAPIRec {
    /* XMESA Functions */
    GLboolean    (*InitDriver)(__DRIscreenPrivate *driScrnPriv);
    void         (*ResetDriver)(__DRIscreenPrivate *driScrnPriv);
    GLvisual *   (*CreateVisual)(Display *display,
                                 __DRIscreenPrivate *driScrnPriv,
                                 const XVisualInfo *visinfo,
                                 const __GLXvisualConfig *config);
    GLboolean    (*CreateContext)(Display *dpy, GLvisual *mesaVis,
                                  __DRIcontextPrivate *driContextPriv);
    void         (*DestroyContext)(__DRIcontextPrivate *driContextPriv);
    GLframebuffer * (*CreateWindowBuffer)(Display *dpy,
                                          __DRIscreenPrivate *driScrnPriv,
                                          __DRIdrawablePrivate *driDrawPriv,
                                          GLvisual *mesaVis);
    GLframebuffer * (*CreatePixmapBuffer)(Display *dpy,
                                          __DRIscreenPrivate *driScrnPriv,
                                          __DRIdrawablePrivate *driDrawPriv,
                                          GLvisual *mesaVis);
    void         (*SwapBuffers)(__DRIdrawablePrivate *driDrawPriv);
    GLboolean    (*MakeCurrent)(__DRIcontextPrivate *driContextPriv,
                                __DRIdrawablePrivate *driDrawPriv,
                                __DRIdrawablePrivate *driReadPriv);
    GLboolean    (*UnbindContext)(__DRIcontextPrivate *driContextPriv);
    GLboolean    (*OpenFullScreen)(__DRIcontextPrivate *driContextPriv);
    GLboolean    (*CloseFullScreen)(__DRIcontextPrivate *driContextPriv);
};



#endif /* _DRI_XMESAAPI_H_ */
