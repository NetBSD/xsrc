/* -*- mode: c; c-basic-offset: 3 -*-
 *
 * Copyright 2000 VA Linux Systems Inc., Fremont, California.
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEMS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
/* $XFree86: xc/lib/GL/mesa/src/drv/tdfx/tdfx_texman.h,v 1.1 2001/03/21 16:14:28 dawes Exp $ */

/*
 * Original rewrite:
 *	Gareth Hughes <gareth@valinux.com>, 29 Sep - 1 Oct 2000
 *
 * Authors:
 *	Gareth Hughes <gareth@valinux.com>
 *	Brian Paul <brianp@valinux.com>
 *
 */

#ifndef __TDFX_TEXMAN_H__
#define __TDFX_TEXMAN_H__

#include "tdfx_lock.h"

extern void tdfxTMInit( tdfxContextPtr fxMesa );
extern void tdfxTMClose( tdfxContextPtr fxMesa );

extern void tdfxTMDownloadTextureLocked( tdfxContextPtr fxMesa,
					 struct gl_texture_object *tObj );
extern void tdfxTMReloadMipMapLevelLocked( GLcontext *ctx,
					   struct gl_texture_object *tObj,
					   GLint level );
extern void tdfxTMMoveInTMLocked( tdfxContextPtr fxMesa,
				  struct gl_texture_object *tObj,
				  FxU32 targetTMU );
extern void tdfxTMMoveOutTMLocked( tdfxContextPtr fxMesa,
				   struct gl_texture_object *tObj );
extern void tdfxTMRestoreTexturesLocked( tdfxContextPtr fxMesa );

extern void tdfxTMFreeTextureLocked( tdfxContextPtr fxMesa,
				     struct gl_texture_object *tObj );


#define tdfxTMMoveInTM( fxMesa, tObj, targetTMU )			\
do {									\
   LOCK_HARDWARE( fxMesa );						\
   tdfxTMMoveInTMLocked( fxMesa, tObj, targetTMU );			\
   UNLOCK_HARDWARE( fxMesa );						\
} while (0)

#define tdfxTMMoveOutTM( fxMesa, tObj )					\
do {									\
   LOCK_HARDWARE( fxMesa );						\
   tdfxTMMoveOutTMLocked( fxMesa, tObj );				\
   UNLOCK_HARDWARE( fxMesa );						\
} while (0)

#endif
