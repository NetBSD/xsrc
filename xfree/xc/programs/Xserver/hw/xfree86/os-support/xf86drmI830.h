/**************************************************************************

Copyright 2001 VA Linux Systems Inc., Fremont, California.

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

/* $XFree86: xc/programs/Xserver/hw/xfree86/os-support/xf86drmI830.h,v 1.2 2001/10/04 18:32:29 alanh Exp $ */

/* Author: Jeff Hartmann <jhartmann@valinux.com> 
 */

#ifndef _I830_XF86DRM_H_
#define _I830_XF86DRM_H_

/* WARNING: These defines must be the same as what the Xserver uses.
 * if you change them, you must change the defines in the Xserver.
 */

#ifndef _I830_DEFINES_
#define _I830_DEFINES_

#define I830_DMA_BUF_ORDER		12
#define I830_DMA_BUF_SZ 		(1<<I830_DMA_BUF_ORDER)
#define I830_DMA_BUF_NR 		256
#define I830_NR_SAREA_CLIPRECTS 	8

/* Each region is a minimum of 64k, and there are at most 64 of them.
 */
#define I830_NR_TEX_REGIONS 64
#define I830_LOG_MIN_TEX_REGION_SIZE 16

/* if defining I830_ENABLE_4_TEXTURES, do it in i830_3d_reg.h, too */
#if !defined(I830_ENABLE_4_TEXTURES)
#define I830_TEXTURE_COUNT	2
#define I830_TEXBLEND_COUNT	2	/* always same as TEXTURE_COUNT? */
#else /* defined(I830_ENABLE_4_TEXTURES) */
#define I830_TEXTURE_COUNT	4
#define I830_TEXBLEND_COUNT	4	/* always same as TEXTURE_COUNT? */
#endif /* I830_ENABLE_4_TEXTURES */

#define I830_TEXBLEND_SIZE	12	/* (4 args + op) * 2 + COLOR_FACTOR */

#define I830_UPLOAD_CTX			0x1
#define I830_UPLOAD_BUFFERS		0x2
#define I830_UPLOAD_CLIPRECTS		0x4
#define I830_UPLOAD_TEX0_IMAGE		0x100 /* handled clientside */
#define I830_UPLOAD_TEX0_CUBE		0x200 /* handled clientside */
#define I830_UPLOAD_TEX1_IMAGE		0x400 /* handled clientside */
#define I830_UPLOAD_TEX1_CUBE		0x800 /* handled clientside */
#define I830_UPLOAD_TEX2_IMAGE		0x1000 /* handled clientside */
#define I830_UPLOAD_TEX2_CUBE		0x2000 /* handled clientside */
#define I830_UPLOAD_TEX3_IMAGE		0x4000 /* handled clientside */
#define I830_UPLOAD_TEX3_CUBE		0x8000 /* handled clientside */
#define I830_UPLOAD_TEX_N_IMAGE(n)	(0x100 << (n * 2))
#define I830_UPLOAD_TEX_N_CUBE(n)	(0x200 << (n * 2))
#define I830_UPLOAD_TEXIMAGE_MASK	0xff00
#define I830_UPLOAD_TEX0			0x10000
#define I830_UPLOAD_TEX1			0x20000
#define I830_UPLOAD_TEX2			0x40000
#define I830_UPLOAD_TEX3			0x80000
#define I830_UPLOAD_TEX_N(n)		(0x10000 << (n))
#define I830_UPLOAD_TEX_MASK		0xf0000
#define I830_UPLOAD_TEXBLEND0		0x100000
#define I830_UPLOAD_TEXBLEND1		0x200000
#define I830_UPLOAD_TEXBLEND2		0x400000
#define I830_UPLOAD_TEXBLEND3		0x800000
#define I830_UPLOAD_TEXBLEND_N(n)	(0x100000 << (n))
#define I830_UPLOAD_TEXBLEND_MASK	0xf00000
#define I830_UPLOAD_TEX_PALETTE_N(n)    (0x1000000 << (n))
#define I830_UPLOAD_TEX_PALETTE_SHARED	0x4000000

/* Indices into buf.Setup where various bits of state are mirrored per
 * context and per buffer.  These can be fired at the card as a unit,
 * or in a piecewise fashion as required.
 */

/* Destbuffer state 
 *    - backbuffer linear offset and pitch -- invarient in the current dri
 *    - zbuffer linear offset and pitch -- also invarient
 *    - drawing origin in back and depth buffers.
 *
 * Keep the depth/back buffer state here to acommodate private buffers
 * in the future.
 */

#define I830_DESTREG_CBUFADDR 0
/* Invarient */
#define I830_DESTREG_DBUFADDR 1
#define I830_DESTREG_DV0 2
#define I830_DESTREG_DV1 3
#define I830_DESTREG_SENABLE 4
#define I830_DESTREG_SR0 5
#define I830_DESTREG_SR1 6
#define I830_DESTREG_SR2 7
#define I830_DESTREG_DR0 8
#define I830_DESTREG_DR1 9
#define I830_DESTREG_DR2 10
#define I830_DESTREG_DR3 11
#define I830_DESTREG_DR4 12
#define I830_DEST_SETUP_SIZE 13

/* Context state
 */
#define I830_CTXREG_STATE1		0
#define I830_CTXREG_STATE2		1
#define I830_CTXREG_STATE3		2
#define I830_CTXREG_STATE4		3
#define I830_CTXREG_STATE5		4
#define I830_CTXREG_IALPHAB		5
#define I830_CTXREG_STENCILTST		6
#define I830_CTXREG_ENABLES_1		7
#define I830_CTXREG_ENABLES_2		8
#define I830_CTXREG_AA			9
#define I830_CTXREG_FOGCOLOR		10
#define I830_CTXREG_BLENDCOLR0		11
#define I830_CTXREG_BLENDCOLR		12 /* Dword 1 of 2 dword command */
#define I830_CTXREG_VF			13
#define I830_CTXREG_VF2			14
#define I830_CTXREG_MCSB0		15
#define I830_CTXREG_MCSB1		16
#define I830_CTX_SETUP_SIZE		17

/* Texture state (per tex unit)
 */

#define I830_TEXREG_MI0	0	/* GFX_OP_MAP_INFO (6 dwords) */
#define I830_TEXREG_MI1	1
#define I830_TEXREG_MI2	2
#define I830_TEXREG_MI3	3
#define I830_TEXREG_MI4	4
#define I830_TEXREG_MI5	5
#define I830_TEXREG_MF	6	/* GFX_OP_MAP_FILTER */
#define I830_TEXREG_MLC	7	/* GFX_OP_MAP_LOD_CTL */
#define I830_TEXREG_MLL	8	/* GFX_OP_MAP_LOD_LIMITS */
#define I830_TEXREG_MCS	9	/* GFX_OP_MAP_COORD_SETS */
#define I830_TEX_SETUP_SIZE 10


#define I830_FRONT   0x1
#define I830_BACK    0x2
#define I830_DEPTH   0x4
#endif /* _I830_DEFINES_ */

typedef struct _drmI830Init {
   unsigned int start;
   unsigned int end;
   unsigned int size;
   unsigned int mmio_offset;
   unsigned int buffers_offset;
   int sarea_off;
   unsigned int front_offset;
   unsigned int back_offset;
   unsigned int depth_offset;
   unsigned int w;
   unsigned int h;
   unsigned int pitch;
   unsigned int pitch_bits;
   unsigned int cpp;
} drmI830Init;

Bool drmI830CleanupDma(int driSubFD);
Bool drmI830InitDma(int driSubFD, drmI830Init *info );

#endif /* _I830_DRM_H_ */
