/* $XFree86: xc/lib/GL/mesa/src/drv/ffb/ffb_lock.h,v 1.1 2000/06/20 05:08:38 dawes Exp $ */

#ifndef _FFB_LOCK_H
#define _FFB_LOCK_H

#include "ffb_context.h"

extern void ffbXMesaUpdateState(ffbContextPtr fmesa);
#define FFB_UPDATE_STATE(fmesa)	ffbXMesaUpdateState(fmesa)

/* Lock the hardware and validate our state. */
#define LOCK_HARDWARE(fmesa)				\
  do {							\
    int __ret=0;					\
    DRM_CAS(fmesa->driHwLock, fmesa->hHWContext,	\
	    (DRM_LOCK_HELD | fmesa->hHWContext), __ret);\
    if (__ret) {					\
        drmGetLock(fmesa->driFd, fmesa->hHWContext, 0);	\
	FFB_UPDATE_STATE(fmesa);			\
    }							\
  } while (0)


/* Unlock the hardware. */
#define UNLOCK_HARDWARE(fmesa)					\
    DRM_UNLOCK(fmesa->driFd, fmesa->driHwLock, fmesa->hHWContext);	

#endif /* !(_FFB_LOCK_H) */
