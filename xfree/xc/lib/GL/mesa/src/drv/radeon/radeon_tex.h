/* $XFree86: xc/lib/GL/mesa/src/drv/radeon/radeon_tex.h,v 1.2 2001/03/21 16:14:25 dawes Exp $ */
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

#ifndef __RADEON_TEX_H__
#define __RADEON_TEX_H__

#ifdef GLX_DIRECT_RENDERING

extern void radeonUpdateTextureState( GLcontext *ctx );

extern int radeonUploadTexImages( radeonContextPtr rmesa, radeonTexObjPtr t );

extern void radeonAgeTextures( radeonContextPtr rmesa, int heap );
extern void radeonDestroyTexObj( radeonContextPtr rmesa, radeonTexObjPtr t );
extern void radeonSwapOutTexObj( radeonContextPtr rmesa, radeonTexObjPtr t );

extern void radeonPrintLocalLRU( radeonContextPtr rmesa, int heap );
extern void radeonPrintGlobalLRU( radeonContextPtr rmesa, int heap );
extern void radeonUpdateTexLRU(radeonContextPtr rmesa, radeonTexObjPtr t );

extern void radeonDDInitTextureFuncs( GLcontext *ctx );


/* ================================================================
 * Color conversion macros:
 */

#define RADEONPACKCOLOR332( r, g, b )					\
   (((r) & 0xe0) | (((g) & 0xe0) >> 3) | (((b) & 0xc0) >> 6))

#define RADEONPACKCOLOR1555( r, g, b, a )				\
   ((((r) & 0xf8) << 7) | (((g) & 0xf8) << 2) | (((b) & 0xf8) >> 3) |	\
    ((a) ? 0x8000 : 0))

#define RADEONPACKCOLOR565( r, g, b )					\
   ((((r) & 0xf8) << 8) | (((g) & 0xfc) << 3) | (((b) & 0xf8) >> 3))

#define RADEONPACKCOLOR88( i, a )					\
   (((a) << 8) | (i))

#define RADEONPACKCOLOR888( r, g, b )					\
   (((r) << 16) | ((g) << 8) | (b))

#define RADEONPACKCOLOR8888( r, g, b, a )				\
   (((a) << 24) | ((r) << 16) | ((g) << 8) | (b))

#define RADEONPACKCOLOR4444( r, g, b, a )				\
   ((((a) & 0xf0) << 8) | (((r) & 0xf0) << 4) | ((g) & 0xf0) | ((b) >> 4))

static __inline__ CARD32 radeonPackColor( GLuint cpp,
					  GLubyte r, GLubyte g,
					  GLubyte b, GLubyte a )
{
   switch ( cpp ) {
   case 2:
      return RADEONPACKCOLOR565( r, g, b );
   case 4:
      return RADEONPACKCOLOR8888( r, g, b, a );
   default:
      return 0;
   }
}

#endif
#endif /* __RADEON_TEX_H__ */
