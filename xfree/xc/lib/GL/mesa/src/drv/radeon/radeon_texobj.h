/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_texobj.h,v 1.3 2001/04/10 16:07:53 dawes Exp $ */
/**************************************************************************

Copyright 2000, 2001 ATI Technologies Inc., Ontario, Canada, and
                     VA Linux Systems Inc., Fremont, California.

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
ATI, VA LINUX SYSTEMS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
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

#ifndef __RADEON_TEXOBJ_H__
#define __RADEON_TEXOBJ_H__

#include "radeon_sarea.h"
#include "mm.h"

#define TEX_0 1
#define TEX_1 2

typedef struct radeon_tex_obj radeonTexObj, *radeonTexObjPtr;

/* Texture object in locally shared texture space.
 */
struct radeon_tex_obj {
   radeonTexObjPtr next, prev;

   struct gl_texture_object *tObj;	/* Mesa texture object */

   PMemBlock memBlock;			/* Memory block containing texture */
   GLuint bufAddr;			/* Offset to start of locally
					   shared texture block */

   GLuint dirty_images;			/* Flags for whether or not
					   images need to be uploaded to
					   local or AGP texture space */

   GLint bound;				/* Texture unit currently bound to */
   GLint heap;				/* Texture heap currently stored in */

   drmRadeonTexImage image[RADEON_MAX_TEXTURE_LEVELS];

   GLint totalSize;			/* Total size of the texture
					   including all mipmap levels */

   GLuint pp_txfilter;		        /* Hardware register values */
   GLuint pp_txformat;
   GLuint pp_txoffset;
   GLuint pp_border_color;
};

#endif /* __RADEON_TEXOBJ_H__ */
