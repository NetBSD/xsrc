/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/ati/atiio.h,v 1.1.2.1 1998/02/01 16:41:56 robin Exp $ */
/*
 * Copyright 1997,1998 by Marc Aurele La France (TSI @ UQV), tsi@ualberta.ca
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of Marc Aurele La France not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Marc Aurele La France makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as-is" without express or implied warranty.
 *
 * MARC AURELE LA FRANCE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.  IN NO
 * EVENT SHALL MARC AURELE LA FRANCE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef ___ATIIO_H___
#define ___ATIIO_H___ 1

#include "atiregs.h"
#include "compiler.h"
#include "misc.h"

/* The following are port numbers that are determined by ATIProbe */
extern CARD16 ATIIOPortVGAWonder;

extern CARD16 ATIIOPortCRTC_H_TOTAL_DISP, ATIIOPortCRTC_H_SYNC_STRT_WID,
              ATIIOPortCRTC_V_TOTAL_DISP, ATIIOPortCRTC_V_SYNC_STRT_WID,
              ATIIOPortCRTC_OFF_PITCH,ATIIOPortCRTC_INT_CNTL,
              ATIIOPortCRTC_GEN_CNTL, ATIIOPortDSP_CONFIG, ATIIOPortDSP_ON_OFF,
              ATIIOPortOVR_CLR,
              ATIIOPortOVR_WID_LEFT_RIGHT, ATIIOPortOVR_WID_TOP_BOTTOM,
              ATIIOPortCLOCK_CNTL, ATIIOPortBUS_CNTL, ATIIOPortMEM_INFO,
              ATIIOPortMEM_VGA_WP_SEL, ATIIOPortMEM_VGA_RP_SEL,
              ATIIOPortDAC_REGS, ATIIOPortDAC_CNTL,
              ATIIOPortGEN_TEST_CNTL, ATIIOPortCONFIG_CNTL;

/* These port numbers are determined by ATISave & ATIRestore */
extern CARD16 ATIIOPortDAC_MASK, ATIIOPortDAC_DATA,
              ATIIOPortDAC_READ, ATIIOPortDAC_WRITE;

/* I/O decoding definitions */
#define SPARSE_IO	0
#define BLOCK_IO	1
extern CARD16 ATIIOBase;
extern CARD8 ATIIODecoding;
#define ATIIOPort(_PortTag)							\
        (((ATIIODecoding == SPARSE_IO) ?					\
          (((_PortTag) & SPARSE_IO_SELECT) | ((_PortTag) & IO_BYTE_SELECT)) :	\
          (((_PortTag) & BLOCK_IO_SELECT)  | ((_PortTag) & IO_BYTE_SELECT))) |	\
         ATIIOBase)

extern CARD8 ATIB2Reg;		/* The B2 mirror */
extern CARD8 ATIVGAOffset;	/* Low index for ATIIOPortVGAWonder */

extern void ATISetVGAIOBase FunctionPrototype((const CARD8));
extern void ATIModifyExtReg FunctionPrototype((const CARD8, int, const CARD8,
                                               CARD8));

/* Odds and ends to ease reading and writting of registers */
#define GetReg(_Register, _Index)                               \
        (                                                       \
                outb(_Register, _Index),                        \
                inb(_Register + 1)                              \
        )
#define PutReg(_Register, _Index, _Value)                       \
        outw(_Register, ((_Value) << 8) | (_Index))
#define ATIGetExtReg(_Index)                                    \
        GetReg(ATIIOPortVGAWonder, _Index)
#define ATIPutExtReg(_Index, _Value)                            \
        PutReg(ATIIOPortVGAWonder, _Index, _Value)

extern void ATIAccessMach64PLLReg FunctionPrototype((const CARD8, const Bool));

#define ATIGetMach64PLLReg(_Index)                              \
        (                                                       \
                ATIAccessMach64PLLReg(_Index, FALSE),           \
                inb(ATIIOPortCLOCK_CNTL + 2)                    \
        )
#define ATIPutMach64PLLReg(_Index, _Value)                      \
        (                                                       \
                ATIAccessMach64PLLReg(_Index, TRUE),            \
                outb(ATIIOPortCLOCK_CNTL + 2, _Value)           \
        )

/* Wait until "n" queue entries are free */
#define ibm8514WaitQueue(_n)						\
	{								\
		while (inw(GP_STAT) & (0x0100U >> (_n)));		\
	}
#define ATIWaitQueue(_n)						\
	{								\
		while (inw(EXT_FIFO_STATUS) & (0x10000U >> (_n)));	\
	}

/* Wait until GP is idle and queue is empty */
#define WaitIdleEmpty()							\
	{								\
		while (inw(GP_STAT) & (GPBUSY | 1));			\
	}
#define ProbeWaitIdleEmpty()						\
	{								\
		int i;							\
		for (i = 0; i < 100000; i++)				\
			if (!(inw(GP_STAT) & (GPBUSY | 1)))		\
				break;					\
	}

/* Wait until GP has data available */
#define WaitDataReady()							\
	{								\
		while (!(inw(GP_STAT) & DATARDY));			\
	}


#endif /* ___ATIIO_H___ */
