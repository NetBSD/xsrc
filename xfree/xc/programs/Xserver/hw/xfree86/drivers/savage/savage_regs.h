/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/savage/savage_regs.h,v 1.4 2000/12/07 20:26:22 dawes Exp $ */

#ifndef _SAVAGE_REGS_H
#define _SAVAGE_REGS_H

#include "savage_driver.h"

#define S3_SAVAGE3D_SERIES(chip)  ((chip>=S3_SAVAGE3D) && (chip<=S3_SAVAGE_MX))

#define S3_SAVAGE4_SERIES(chip)   ((chip==S3_SAVAGE4) || (chip==S3_PROSAVAGE))

#define S3_SAVAGE_SERIES(chip)    ((chip>=S3_SAVAGE3D) && (chip<=S3_SAVAGE2000))


/* Chip tags.  These are used to group the adapters into 
 * related families.
 */

enum S3CHIPTAGS {
    S3_UNKNOWN = 0,
    S3_SAVAGE3D,
    S3_SAVAGE_MX,
    S3_SAVAGE4,
    S3_PROSAVAGE,
    S3_SAVAGE2000,
    S3_LAST
};

#define BIOS_BSIZE			1024
#define BIOS_BASE			0xc0000

#define SAVAGE_NEWMMIO_REGBASE_S3	0x1000000  /* 16MB */
#define SAVAGE_NEWMMIO_REGBASE_S4	0x0000000 
#define SAVAGE_NEWMMIO_REGSIZE		0x0080000	/* 512kb */
#define SAVAGE_NEWMMIO_VGABASE		0x8000

#define BASE_FREQ			14.31818	

#define FIFO_CONTROL_REG		0x8200
#define MIU_CONTROL_REG			0x8204
#define STREAMS_TIMEOUT_REG		0x8208
#define MISC_TIMEOUT_REG		0x820c

#define PSTREAM_CONTROL_REG		0x8180
#define COL_CHROMA_KEY_CONTROL_REG 	0x8184
#define SSTREAM_CONTROL_REG		0x8190
#define CHROMA_KEY_UPPER_BOUND_REG 	0x8194
#define SSTREAM_STRETCH_REG		0x8198
#define BLEND_CONTROL_REG		0x81a0
#define PSTREAM_FBADDR0_REG		0x81c0
#define PSTREAM_FBADDR1_REG		0x81c4
#define PSTREAM_STRIDE_REG		0x81c8
#define DOUBLE_BUFFER_REG		0x81cc
#define SSTREAM_FBADDR0_REG		0x81d0
#define SSTREAM_FBADDR1_REG		0x81d4
#define SSTREAM_STRIDE_REG		0x81d8
#define OPAQUE_OVERLAY_CONTROL_REG 	0x81dc
#define K1_VSCALE_REG			0x81e0
#define K2_VSCALE_REG			0x81e4
#define DDA_VERT_REG			0x81e8
#define STREAMS_FIFO_REG		0x81ec
#define PSTREAM_START_REG		0x81f0
#define PSTREAM_WINDOW_SIZE_REG		0x81f4
#define SSTREAM_START_REG		0x81f8
#define SSTREAM_WINDOW_SIZE_REG		0x81fC

#define SUBSYS_STAT_REG			0x8504

#define SRC_BASE			0xa4d4
#define DEST_BASE			0xa4d8
#define CLIP_L_R			0xa4dc
#define CLIP_T_B			0xa4e0
#define DEST_SRC_STR			0xa4e4
#define MONO_PAT_0			0xa4e8
#define MONO_PAT_1			0xa4ec


/* Constants for CR69. */

#define CRT_ACTIVE	0x01
#define LCD_ACTIVE	0x02
#define TV_ACTIVE	0x04
#define CRT_ATTACHED	0x10
#define LCD_ATTACHED	0x20
#define TV_ATTACHED	0x40


/*
 * reads from SUBSYS_STAT
 */
#define STATUS_WORD0            (INREG(0x48C00))
#define ALT_STATUS_WORD0        (INREG(0x48C60))
#define MAXLOOP			0xffffff
#define IN_SUBSYS_STAT()	(INREG(SUBSYS_STAT_REG))

#define MAXFIFO		0x7f00

#define VerticalRetraceWait() \
{ \
	VGAOUT8(vgaCRIndex, 0x17); \
	if (VGAIN8(vgaCRReg) & 0x80) { \
		while ((VGAIN8(vgaIOBase + 0x0a) & 0x08) == 0x00) ; \
		while ((VGAIN8(vgaIOBase + 0x0a) & 0x08) == 0x08) ; \
		while ((VGAIN8(vgaIOBase + 0x0a) & 0x08) == 0x00) ; \
	} \
}

#endif /* _SAVAGE_REGS_H */
