#ifndef _XDRI_P_H
#define _XDRI_P_H

/* Direct rendering includes
 */
#include "dri_mesaint.h"


/*
 * Direct rendering macros:
 */
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



#define DRI_DRAWABLE_ARG , __DRIdrawablePrivate *driDrawPriv
#define DRI_DRAWABLE_PARM , driDrawPriv

#define DRI_CTX_ARG , __DRIcontextPrivate *driContextPriv


extern void XMesaDriSwapBuffers( XMesaBuffer b );

#endif
