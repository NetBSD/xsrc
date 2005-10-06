/*
 * GX and Turbo GX framebuffer - defines.
 *
 * Copyright (C) 2000 Jakub Jelinek (jakub@redhat.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JAKUB JELINEK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/suncg6/cg6.h,v 1.3 2001/05/04 19:05:45 dawes Exp $ */

#ifndef CG6_H
#define CG6_H

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"
#include "xf86RamDac.h"
#include "Xmd.h"
#include "gcstruct.h"
#include "cg6_regs.h"
#include "xf86sbusBus.h"
#include "xaa.h"

/* Various offsets in virtual (ie. mmap()) spaces Linux and Solaris support. */
#define CG6_FBC_VOFF	0x70000000
#define CG6_TEC_VOFF	0x70001000
#define CG6_BTREGS_VOFF	0x70002000
#define CG6_FHC_VOFF	0x70004000
#define CG6_THC_VOFF	0x70005000
#define CG6_ROM_VOFF	0x70006000
#define CG6_RAM_VOFF	0x70016000
#define CG6_DHC_VOFF	0x80000000

typedef struct {
	unsigned int fg, bg;			/* FG/BG colors for stipple */
	unsigned int patalign;                  /* X/Y alignment of bits */
        unsigned int alu;			/* Transparent/Opaque + rop */
        unsigned int bits[32];                  /* The stipple bits themselves */
} Cg6StippleRec, *Cg6StipplePtr;

typedef struct {
	int type;
	Cg6StipplePtr stipple;
} Cg6PrivGCRec, *Cg6PrivGCPtr;

typedef struct {
	unsigned char	*fb;
	Cg6FbcPtr	fbc;
	Cg6ThcPtr	thc;
	int		vclipmax;
	int		width;
	int		height;
	int		maxheight;
	int		vidmem;
	
	sbusDevicePtr	psdp;
	Bool		HWCursor;
	Bool		NoAccel;
	CloseScreenProcPtr CloseScreen;
	xf86CursorInfoPtr CursorInfoRec;
	unsigned int	CursorXY;
	int		CursorBg, CursorFg;
	Bool		CursorEnabled;
	OptionInfoPtr	Options;
	unsigned char *buffers[1];
	CARD32	scanline[1024];
	int		words_in_scanline, scan_x, scan_y, scan_xe;
        int		clipxa, clipxe;
	XAAInfoRecPtr	pXAA;
} Cg6Rec, *Cg6Ptr;

extern int  Cg6ScreenPrivateIndex;
extern int  Cg6GCPrivateIndex;
extern int  Cg6WindowPrivateIndex;

#define GET_CG6_FROM_SCRN(p)    ((Cg6Ptr)((p)->driverPrivate))

#define Cg6GetScreenPrivate(s)						\
((Cg6Ptr) (s)->devPrivates[Cg6ScreenPrivateIndex].ptr)

#define Cg6GetGCPrivate(g)						\
((Cg6PrivGCPtr) (g)->devPrivates [Cg6GCPrivateIndex].ptr)

#define Cg6GetWindowPrivate(w)						\
((Cg6StipplePtr) (w)->devPrivates[Cg6WindowPrivateIndex].ptr)
                            
#define Cg6SetWindowPrivate(w,p) 					\
((w)->devPrivates[Cg6WindowPrivateIndex].ptr = (pointer) p)


/* rasterops */
#define GX_ROP_CLEAR	    0x0
#define GX_ROP_INVERT	    0x1
#define GX_ROP_NOOP	    0x2
#define GX_ROP_SET	    0x3

#define GX_ROP_00_0(rop)    ((rop) << 0)
#define GX_ROP_00_1(rop)    ((rop) << 2)
#define GX_ROP_01_0(rop)    ((rop) << 4)
#define GX_ROP_01_1(rop)    ((rop) << 6)
#define GX_ROP_10_0(rop)    ((rop) << 8)
#define GX_ROP_10_1(rop)    ((rop) << 10)
#define GX_ROP_11_0(rop)    ((rop) << 12)
#define GX_ROP_11_1(rop)    ((rop) << 14)
#define GX_PLOT_PLOT	    0x00000000
#define GX_PLOT_UNPLOT	    0x00020000
#define GX_RAST_BOOL	    0x00000000
#define GX_RAST_LINEAR	    0x00040000
#define GX_ATTR_UNSUPP	    0x00400000
#define GX_ATTR_SUPP	    0x00800000
#define GX_POLYG_OVERLAP    0x01000000
#define GX_POLYG_NONOVERLAP 0x02000000
#define GX_PATTERN_ZEROS    0x04000000
#define GX_PATTERN_ONES	    0x08000000
#define GX_PATTERN_MASK	    0x0c000000
#define GX_PIXEL_ZEROS	    0x10000000
#define GX_PIXEL_ONES	    0x20000000
#define GX_PIXEL_MASK	    0x30000000
#define GX_PLANE_ZEROS	    0x40000000
#define GX_PLANE_ONES	    0x80000000
#define GX_PLANE_MASK	    0xc0000000
/* rops for bit blit / copy area
   with:
       Plane Mask - use plane mask reg.
       Pixel Mask - use all ones.
       Patt  Mask - use all ones.
*/

#define POLY_O		GX_POLYG_OVERLAP
#define POLY_N		GX_POLYG_NONOVERLAP

#define ROP_STANDARD	(GX_PLANE_MASK |\
			GX_PIXEL_ONES |\
			GX_ATTR_SUPP |\
			GX_RAST_BOOL |\
			GX_PLOT_PLOT)

/* fg = don't care  bg = don't care */

#define ROP_BLIT(O,I)	(ROP_STANDARD | \
			GX_PATTERN_ONES |\
			GX_ROP_11_1(I) |\
			GX_ROP_11_0(O) |\
			GX_ROP_10_1(I) |\
			GX_ROP_10_0(O) |\
			GX_ROP_01_1(I) |\
			GX_ROP_01_0(O) |\
			GX_ROP_00_1(I) |\
			GX_ROP_00_0(O))

/* fg = fgPixel	    bg = don't care */

#define ROP_FILL(O,I)	(ROP_STANDARD | \
			GX_PATTERN_ONES |\
			GX_ROP_11_1(I) |\
			GX_ROP_11_0(I) |\
			GX_ROP_10_1(I) |\
			GX_ROP_10_0(I) | \
			GX_ROP_01_1(O) |\
			GX_ROP_01_0(O) |\
			GX_ROP_00_1(O) |\
			GX_ROP_00_0(O))

/* fg = fgPixel	    bg = don't care */
 
#define ROP_STIP(O,I)   (ROP_STANDARD |\
			GX_ROP_11_1(I) |\
			GX_ROP_11_0(GX_ROP_NOOP) |\
			GX_ROP_10_1(I) |\
			GX_ROP_10_0(GX_ROP_NOOP) | \
			GX_ROP_01_1(O) |\
			GX_ROP_01_0(GX_ROP_NOOP) |\
			GX_ROP_00_1(O) |\
			GX_ROP_00_0(GX_ROP_NOOP))

/* fg = fgPixel	    bg = bgPixel */
			    
#define ROP_OSTP(O,I)   (ROP_STANDARD |\
			GX_ROP_11_1(I) |\
			GX_ROP_11_0(I) |\
			GX_ROP_10_1(I) |\
			GX_ROP_10_0(O) |\
			GX_ROP_01_1(O) |\
			GX_ROP_01_0(I) |\
			GX_ROP_00_1(O) |\
			GX_ROP_00_0(O))

#define GX_ROP_USE_PIXELMASK	0x30000000

#define GX_BLT_INPROGRESS	0x20000000

#define GX_INPROGRESS		0x10000000
#define GX_FULL			0x20000000

/* modes */
#define GX_INDEX(n)	    ((n) << 4)
#define GX_INDEX_ALL	    0x00000030
#define GX_INDEX_MOD	    0x00000040
#define GX_BDISP_0	    0x00000080
#define GX_BDISP_1	    0x00000100
#define GX_BDISP_ALL	    0x00000180
#define GX_BREAD_0	    0x00000200
#define GX_BREAD_1	    0x00000400
#define GX_BREAD_ALL	    0x00000600
#define GX_BWRITE1_ENABLE   0x00000800
#define GX_BWRITE1_DISABLE  0x00001000
#define GX_BWRITE1_ALL	    0x00001800
#define GX_BWRITE0_ENABLE   0x00002000
#define GX_BWRITE0_DISABLE  0x00004000
#define GX_BWRITE0_ALL	    0x00006000
#define GX_DRAW_RENDER	    0x00008000
#define GX_DRAW_PICK	    0x00010000
#define GX_DRAW_ALL	    0x00018000
#define GX_MODE_COLOR8	    0x00020000
#define GX_MODE_COLOR1	    0x00040000
#define GX_MODE_HRMONO	    0x00060000
#define GX_MODE_ALL	    0x00060000
#define GX_VBLANK	    0x00080000
#define GX_BLIT_NOSRC	    0x00100000
#define GX_BLIT_SRC	    0x00200000
#define GX_BLIT_ALL	    0x00300000

int CG6AccelInit(ScrnInfoPtr);
Bool Cg6DGAInit(ScreenPtr pScreen);

#endif /* CG6_H */
