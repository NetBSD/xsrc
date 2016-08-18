/* $OpenBSD: wsfb_driver.c,v 1.18 2003/04/02 16:42:13 jason Exp $ */
/* $NetBSD: igs.h,v 1.7 2016/08/18 09:32:26 mrg Exp $ */
/*
 * Copyright (c) 2001 Matthieu Herrb
 *		 2009 Michael Lorenz
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    - Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    - Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * Based on fbdev.c written by:
 *
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *	     Michel Dänzer, <michdaen@iiic.ethz.ch>
 */
 
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <dev/wscons/wsconsio.h>

/* all driver need this */
#include "xorg-server.h"
#include "xf86.h"
#include "xf86_OSproc.h"
#include "compiler.h"

#include "xf86RamDac.h"
#include "exa.h"

#ifndef IGS_H
#define IGS_H

/* private data */
typedef struct {
	int			fd; /* file descriptor of open device */
	struct wsdisplay_fbinfo info; /* frame buffer characteristics */
	int			linebytes; /* number of bytes per row */
	volatile uint8_t	*reg;
	unsigned char*		fbstart;
	unsigned char*		fbmem;
	size_t			fbmem_len;
	unsigned long		fb_paddr;
	Bool			shadowFB;
	Bool			HWCursor;
	Bool			no_accel;
	CloseScreenProcPtr	CloseScreen;
	CreateScreenResourcesProcPtr CreateScreenResources;
	EntityInfoPtr		pEnt;

	struct wsdisplay_cursor cursor;
	int			maskoffset;
	int			Chipset;
	xf86CursorInfoPtr	CursorInfoRec;
	ExaDriverPtr 		pExa;
	int			shift, srcoff, srcpitch;
	uint32_t		cmd;
#ifdef XFreeXDGA
	/* DGA info */
	DGAModePtr		pDGAMode;
	int			nDGAMode;
#endif
	OptionInfoPtr		Options;
	uint8_t			mapfmt;
} IgsRec, *IgsPtr;

#define IGSPTR(p) ((IgsPtr)((p)->driverPrivate))

Bool IgsSetupCursor(ScreenPtr);

/* register definitions, from NetBSD's igsfbreg.h */

/* right now we only care about the extended VGA registers */
#define IGS_EXT_IDX		0x3ce
#define IGS_EXT_PORT		0x3cf

/* [3..0] -> [19..16] of start addr if IGS_EXT_START_ADDR_ON is set */
#define   IGS_EXT_START_ADDR		0x10
#define     IGS_EXT_START_ADDR_ON		0x10

/*
 * COPREN   - enable direct access to coprocessor registers
 * COPASELB - select IGS_COP_BASE_B for COP address
 */
#define   IGS_EXT_BIU_MISC_CTL		0x33
#define     IGS_EXT_BIU_LINEAREN		0x01
#define     IGS_EXT_BIU_LIN2MEM			0x02
#define     IGS_EXT_BIU_COPREN			0x04
#define     IGS_EXT_BIU_COPASELB		0x08
#define     IGS_EXT_BIU_SEGON			0x10
#define     IGS_EXT_BIU_SEG2MEM			0x20

/*
 * Linear Address registers
 *   PCI: don't write directly, just use nomral PCI configuration
 *   ISA: only bits [23..20] are programmable, the rest MBZ
 */
#define   IGS_EXT_LINA_LO		0x34	/* [3..0] -> [23..20] */
#define   IGS_EXT_LINA_HI		0x35	/* [7..0] -> [31..24] */

/* Hardware cursor on-screen location and hot spot */
#define   IGS_EXT_SPRITE_HSTART_LO	0x50
#define   IGS_EXT_SPRITE_HSTART_HI	0x51	/* bits [2..0] */
#define   IGS_EXT_SPRITE_HPRESET	0x52	/* bits [5..0] */

#define   IGS_EXT_SPRITE_VSTART_LO	0x53
#define   IGS_EXT_SPRITE_VSTART_HI	0x54	/* bits [2..0] */
#define   IGS_EXT_SPRITE_VPRESET	0x55	/* bits [5..0] */

/* Hardware cursor control */
#define   IGS_EXT_SPRITE_CTL		0x56
#define     IGS_EXT_SPRITE_VISIBLE		0x01
#define     IGS_EXT_SPRITE_64x64		0x02
#define     IGS_EXT_SPRITE_DAC_PEL		0x04
	  /* bits unrelated to sprite control */
#define     IGS_EXT_COP_RESET			0x08

/* chip ID */
#define   IGS_EXT_CHIP_ID0		0x91 /* 0xa4 for 2010 */
#define   IGS_EXT_CHIP_ID1		0x92 /* 0x08 for 2010 */
#define   IGS_EXT_CHIP_REV		0x92
/* DAC registers */

/* IGS_EXT_SPRITE_CTL/IGS_EXT_SPRITE_DAC_PEL (3cf/56[2]) == 0 */
#define IGS_PEL_MASK		IGS_REG_(0x3c6)

/* IGS_EXT_SPRITE_CTL/IGS_EXT_SPRITE_DAC_PEL 3cf/56[2] == 1 */
#define IGS_DAC_CMD		IGS_REG_(0x3c6)


/*
 * Palette Read/Write: write palette index to the index port.
 * Read/write R/G/B in three consecutive accesses to data port.
 * After third access to data the index is autoincremented and you can
 * proceed with reading/writing data port for the next entry.
 *
 * When IGS_EXT_SPRITE_DAC_PEL bit in sprite control is set, these
 * registers are used to access sprite (i.e. cursor) 2-color palette.
 * (NB: apparently, in this mode index autoincrement doesn't work).
 */
#define IGS_DAC_PEL_READ_IDX	IGS_REG_(0x3c7)
#define IGS_DAC_PEL_WRITE_IDX	IGS_REG_(0x3c8)
#define IGS_DAC_PEL_DATA	IGS_REG_(0x3c9)

/* blitter registers */

#define IGS_MEM_MMIO_SELECT	0x00800000 /* memory mapped i/o */
#define IGS_MEM_BE_SELECT	0x00400000 /* endian select */

#define IGS_COP_BASE_A	0xaf000		/* COPASELB == 0 */
#define IGS_COP_BASE_B	0xbf000		/* COPASELB == 1 */

/*
 * NB: Loaded width values should be 1 less than the actual width!
 */

/*
 * Coprocessor control.
 */
#define IGS_COP_CTL_REG		0x011
#define   IGS_COP_CTL_HBRDYZ		0x01
#define   IGS_COP_CTL_HFEMPTZ		0x02
#define   IGS_COP_CTL_CMDFF		0x04
#define   IGS_COP_CTL_SOP		0x08 /* rw */
#define   IGS_COP_CTL_OPS		0x10
#define   IGS_COP_CTL_TER		0x20 /* rw */
#define   IGS_COP_CTL_HBACKZ		0x40
#define   IGS_COP_CTL_BUSY		0x80


/*
 * Source(s) and destination widths.
 * 16 bit registers.  Only bits [11..0] are used.
 */
#define IGS_COP_SRC_MAP_WIDTH_REG  0x018
#define IGS_COP_SRC2_MAP_WIDTH_REG 0x118
#define IGS_COP_DST_MAP_WIDTH_REG  0x218


/*
 * Bitmap depth.
 */
#define IGS_COP_MAP_FMT_REG	0x01c
#define   IGS_COP_MAP_8BPP		0x00
#define   IGS_COP_MAP_16BPP		0x01
#define   IGS_COP_MAP_24BPP		0x02
#define   IGS_COP_MAP_32BPP		0x03


/*
 * Binary operations are defined below.  S - source, D - destination,
 * N - not; a - and, o - or, x - xor.
 *
 * For ternary operations, foreground mix function is one of 256
 * ternary raster operations defined by Win32 API; background mix is
 * ignored.
 */
#define IGS_COP_FG_MIX_REG	0x048
#define IGS_COP_BG_MIX_REG	0x049

#define   IGS_COP_MIX_0			0x0
#define   IGS_COP_MIX_SaD		0x1
#define   IGS_COP_MIX_SaND		0x2
#define   IGS_COP_MIX_S			0x3
#define   IGS_COP_MIX_NSaD		0x4
#define   IGS_COP_MIX_D			0x5
#define   IGS_COP_MIX_SxD		0x6
#define   IGS_COP_MIX_SoD		0x7
#define   IGS_COP_MIX_NSaND		0x8
#define   IGS_COP_MIX_SxND		0x9
#define   IGS_COP_MIX_ND		0xa
#define   IGS_COP_MIX_SoND		0xb
#define   IGS_COP_MIX_NS		0xc
#define   IGS_COP_MIX_NSoD		0xd
#define   IGS_COP_MIX_NSoND		0xe
#define   IGS_COP_MIX_1			0xf

#define   IGS_PLANE_MASK_REG		0x50
/*
 * Foreground/background colours (24 bit).
 * Selected by bits in IGS_COP_PIXEL_OP_3_REG.
 */
#define IGS_COP_FG_REG		0x058
#define IGS_COP_BG_REG		0x05C


/*
 * Horizontal/vertical dimensions of pixel blit function.
 * 16 bit registers.  Only [11..0] are used.
 */
#define IGS_COP_WIDTH_REG	0x060
#define IGS_COP_HEIGHT_REG	0x062


/*
 * Only bits [21..0] are used.
 */
#define IGS_COP_SRC_BASE_REG	0x070 /* only for 24bpp Src Color Tiling */
#define IGS_COP_SRC_START_REG	0x170
#define IGS_COP_SRC2_START_REG	0x174
#define IGS_COP_DST_START_REG	0x178

/*
 * Destination phase angle for 24bpp.
 */
#define IGS_COP_DST_X_PHASE_REG	0x078
#define   IGS_COP_DST_X_PHASE_MASK	0x07


/*
 * Pixel operation: Direction and draw mode.
 * When an octant bit is set, that axis is traversed backwards.
 */
#define IGS_COP_PIXEL_OP_REG	0x07c

#define   IGS_COP_OCTANT_Y_NEG		0x00000002 /* 0: top down, 1: bottom up */
#define   IGS_COP_OCTANT_X_NEG		0x00000004 /* 0: l2r, 1: r2l */

#define   IGS_COP_DRAW_ALL		0x00000000
#define   IGS_COP_DRAW_FIRST_NULL	0x00000010
#define   IGS_COP_DRAW_LAST_NULL	0x00000020


/*
 * Pixel operation: Pattern operation.
 */

#define   IGS_COP_PPM_TEXT		0x00001000
#define   IGS_COP_PPM_TILE		0x00002000
#define   IGS_COP_PPM_LINE		0x00003000
#define   IGS_COP_PPM_TRANSPARENT	0x00004000/* "or" with one of the above */

#define   IGS_COP_PPM_FIXED_FG		0x00008000
#define   IGS_COP_PPM_SRC_COLOR_TILE	0x00009000


/*
 * Pixel operation: Host CPU access (host blit) to graphics engine.
 */
#define   IGS_COP_HBLTR			0x00010000 /* enable read from engine */
#define   IGS_COP_HBLTW			0x00020000 /* enable write to engine  */


/*
 * Pixel operation: Operation function of graphic engine.
 */
#define   IGS_COP_OP_STROKE		0x04000000 /* short stroke */
#define   IGS_COP_OP_LINE		0x05000000 /* bresenham line draw */
#define   IGS_COP_OP_PXBLT		0x08000000 /* pixel blit */
#define   IGS_COP_OP_PXBLT_INV		0x09000000 /* invert pixel blit */
#define   IGS_COP_OP_PXBLT_3		0x0a000000 /* ternary pixel blit */

/* select fg/bg source: 0 - fg/bg color reg, 1 - src1 map */
#define   IGS_COP_OP_FG_FROM_SRC	0x20000000
#define   IGS_COP_OP_BG_FROM_SRC	0x80000000

#define IGS_CLOCK_REF	14318 /*24576*/	/* KHz */

#define IGS_SCALE(p)	((p) ? (2 * (p)) : 1)

#define IGS_CLOCK(m,n,p) \
	((IGS_CLOCK_REF * ((m) + 1)) / (((n) + 1) * IGS_SCALE(p)))

#define IGS_MAX_CLOCK	260000

#define IGS_MIN_VCO	115000

static __inline uint8_t
igs_idx_read(u_int idxport, uint8_t idx)
{
	outb(idxport, idx);
	return inb(idxport + 1);
}

static __inline void
igs_idx_write(u_int idxport, uint8_t idx, uint8_t val)
{
	outb(idxport, idx);
	outb(idxport + 1, val);
}

/* sugar for extended registers */
#define igs_ext_read(x)	(igs_idx_read(IGS_EXT_IDX,(x)))
#define igs_ext_write(x, v)	(igs_idx_write((IGS_EXT_IDX,(x),(v)))

Bool IgsInitAccel(ScreenPtr);

#endif
