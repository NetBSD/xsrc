/******************************************************************************\

				   Copyright (c) 1999 by Silicon Motion, Inc.
							   All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that the
above copyright notice appear in all copies and that both that copyright notice
and this permission notice appear in supporting documentation, and that the name
of Silicon Motion, Inc. not be used in advertising or publicity pertaining to
distribution of the software without specific, written prior permission.
Silicon Motion, Inc. and its suppliers make no representations about the
suitability of this software for any purpose.  It is provided "as is" without
express or implied warranty.

SILICON MOTION, INC. DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT
SHALL SILICON MOTION, INC. AND/OR ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL,
INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
THIS SOFTWARE.
\******************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/smi/regsmi.h,v 1.1.2.2 1999/12/11 17:43:20 hohndel Exp $ */

#ifndef _REGSMI_H_
#define _REGSMI_H_

/* for OUT instructions */
#include "compiler.h"

/* for mmio */
#include "mmio.h"

/* SMI chipset definitions */

#define VerticalRetraceWait() \
{ \
   while ((inb(vgaIOBase + 0x0A) & 0x08) == 0x00) ; \
   while ((inb(vgaIOBase + 0x0A) & 0x08) == 0x08) ; \
   while ((inb(vgaIOBase + 0x0A) & 0x08) == 0x00) ; \
}

/* PCI data */
#define PCI_SMI_VENDOR_ID	0x126F
#define PCI_910				0x910
#define PCI_810				0x810
#define PCI_820				0x820
#define PCI_710				0x710
#define PCI_712				0x712
#define PCI_720				0x720

/* Chip tags */
#define SMI_UNKNOWN		 	0
#define SMI_910			 	1
#define SMI_810			 	2
#define SMI_820			 	3
#define SMI_710			 	4
#define SMI_712			 	5
#define SMI_720			 	6

#define IS_NEWMMIO(id)		(	((id) == SMI_820)	\
							||	((id) == SMI_720)	\
							)
#define IS_AUTOCENTER(id)	(	((id) == SMI_710)	\
							||	((id) == SMI_712)	\
							||	((id) == SMI_720)	\
							)
#define IS_128BIT(id)		(	((id) == SMI_820)	\
							||	((id) == SMI_720)	\
							)

/* VESA Approved Register Definitions */
#define	DAC_MASK			0x3C6
#define	DAC_R_INDEX			0x3C7
#define	DAC_W_INDEX			0x3C8
#define	DAC_DATA			0x3C9

/* The Mix ROPs (selected ones, not all 256)  */
#define	ROP_0				(0x00)
#define	ROP_DSon			(0x11)
#define	ROP_DSna			(0x22)
#define	ROP_Sn				(0x33)
#define	ROP_SDna			(0x44)
#define	ROP_Dn				(0x55)
#define	ROP_DSx				(0x66)
#define	ROP_DSan			(0x77)
#define	ROP_DSa				(0x88)
#define	ROP_DSxn			(0x99)
#define	ROP_D				(0xAA)
#define	ROP_DSno			(0xBB)
#define	ROP_S				(0xCC)
#define	ROP_SDno			(0xDD)
#define	ROP_DSo				(0xEE)
#define	ROP_1				(0xFF)

/* S -> P */
#define	ROP_DPon			(0x05)
#define	ROP_DPna			(0x0A)
#define	ROP_Pn				(0x0F)
#define	ROP_PDna			(0x50)
#define	ROP_DPx				(0x5A)
#define	ROP_DPan			(0x5F)
#define	ROP_DPa				(0xA0)
#define	ROP_DPxn			(0xA5)
#define	ROP_DPno			(0xAF)
#define	ROP_P				(0xF0)
#define	ROP_PDno			(0xF5)
#define	ROP_DPo				(0xFA)

typedef struct {
   unsigned char r, g, b;
}
LUTENTRY;

#define MAXLOOP 0xffffff /* timeout value for engine waits, ~6 secs */
#define MAXFIFO 8
void SMIGEReset(int from_timeout, int line, char *file);
unsigned char ReadSeqc(unsigned char index);

#define IS_IDLE()	((ReadSeqc(0x16) & 0x08) == 0x00)
#define READ_FIFO()	((ReadSeqc(0x16) & 0x10) ? MAXFIFO \
											 : ((ReadSeqc(0x16) + 1) & 0x07))

/* Wait until "v" queue entries are free */
#define	WaitQueue(v)														\
	if (smiPriv.NoPCIRetry) {												\
		do { int loop=0; mem_barrier();										\
			while ((READ_FIFO() < min(v, MAXFIFO)) && (loop++ < MAXLOOP)) ;	\
			if (loop >= MAXLOOP) SMIGEReset(1, __LINE__, __FILE__);			\
		} while (0); }

/* Wait until GP is idle and queue is empty */
#define	WaitIdleEmpty()														\
	do {																	\
		WaitCommandEmpty();													\
		WaitIdle();															\
	} while(0)

/* Wait until GP is idle */
#define WaitIdle()															\
	do { int loop=0; mem_barrier();											\
		while ((!IS_IDLE()) && (loop++ < MAXLOOP)) ;						\
		if (loop >= MAXLOOP) SMIGEReset(1, __LINE__, __FILE__);				\
	} while (0)

/* Wait until Command FIFO is empty */
#define WaitCommandEmpty()													\
	WaitQueue(MAXFIFO)

#ifndef NULL
#define NULL	0
#endif

#define RGB8_PSEUDO      (-1)
#define RGB16_565         0
#define RGB16_555         1
#define RGB24_888         2

#endif /* _REGSMI_H_ */
