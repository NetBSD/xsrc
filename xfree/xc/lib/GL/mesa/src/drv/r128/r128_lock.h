/* $XFree86: xc/lib/GL/mesa/src/drv/r128/r128_lock.h,v 1.3 2000/12/07 15:43:37 tsi Exp $ */
/**************************************************************************

Copyright 1999, 2000 ATI Technologies Inc. and Precision Insight, Inc.,
                                               Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
on the rights to use, copy, modify, merge, publish, distribute, sub
license, and/or sell copies of the Software, and to permit persons to whom
the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice (including the next
paragraph) shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
ATI, PRECISION INSIGHT AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/

/*
 * Authors:
 *   Kevin E. Martin <martin@valinux.com>
 *   Gareth Hughes <gareth@valinux.com>
 *
 */

#ifndef _R128_LOCK_H_
#define _R128_LOCK_H_

#ifdef GLX_DIRECT_RENDERING

extern void r128GetLock( r128ContextPtr r128ctx, GLuint flags );


/* Turn DEBUG_LOCKING on to find locking conflicts (see r128_init.h) */
#ifdef DEBUG_LOCKING
extern char *prevLockFile;
extern int   prevLockLine;

#define DEBUG_LOCK()                                                    \
    do {                                                                \
	prevLockFile = (__FILE__);                                      \
	prevLockLine = (__LINE__);                                      \
    } while (0)

#define DEBUG_RESET()                                                   \
    do {                                                                \
	prevLockFile = 0;                                               \
	prevLockLine = 0;                                               \
    } while (0)

#define DEBUG_CHECK_LOCK()                                              \
    do {                                                                \
	if (prevLockFile) {                                             \
	    fprintf(stderr,                                             \
		    "LOCK SET!\n\tPrevious %s:%d\n\tCurrent: %s:%d\n",  \
		    prevLockFile, prevLockLine, __FILE__, __LINE__);    \
	    exit(1);                                                    \
	}                                                               \
    } while (0)

#else

#define DEBUG_LOCK()
#define DEBUG_RESET()
#define DEBUG_CHECK_LOCK()

#endif

/*
 * !!! We may want to separate locks from locks with validation.  This
 * could be used to improve performance for those things commands that
 * do not do any drawing !!!
 */

/* Lock the hardware and validate our state.
 */
#define LOCK_HARDWARE( r128ctx )					\
   do {									\
      char __ret = 0;							\
      DEBUG_CHECK_LOCK();						\
      DRM_CAS( r128ctx->driHwLock, r128ctx->hHWContext,			\
	       (DRM_LOCK_HELD | r128ctx->hHWContext), __ret );		\
      if ( __ret )							\
	 r128GetLock( r128ctx, 0 );					\
      DEBUG_LOCK();							\
   } while (0)

/* Unlock the hardware.
 */
#define UNLOCK_HARDWARE( r128ctx )					\
   do {									\
      DRM_UNLOCK( r128ctx->driFd,					\
		  r128ctx->driHwLock,					\
		  r128ctx->hHWContext );				\
      DEBUG_RESET();							\
   } while (0)

#endif
#endif /* _R128_LOCK_H_ */
