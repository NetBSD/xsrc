/*
 * Modified 1996 by Egbert Eich <Egbert.Eich@Physik.TH-Darmstadt.DE>
 * Modified 1996 by David Bateman <dbateman@ee.uts.edu.au>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the authors not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/chips/ct_driver.h,v 3.4 1996/10/17 15:20:47 dawes Exp $ */

/*#define DEBUG
#define CT_HW_DEBUG */
#define CT_DEBUG_WAIT 500000

extern Bool ctLinearSupport;	       /*linear addressing enable */
extern Bool ctAccelSupport;	       /*acceleration enable */
extern Bool ctisHiQV32;		       /*New architecture used in 65550 and 65554 */
extern Bool ctHDepth;		       /*Chip has 16/24bpp */
extern Bool ctDSTN;
extern Bool ctLCD;
extern Bool ctCRT;
extern Bool ctHWCursor;

extern unsigned int ctCursorAddress;   /* The address in video ram of cursor */

/* The adress in video ram of the tile pattern.  */
extern unsigned int ctBLTPatternAddress;
extern Bool ctUseMMIO;
extern Bool ctAvoidImageBLT;
extern unsigned char *ctMMIOBase;
extern unsigned char *ctBltDataWindow;

extern int ctAluConv[];		       /* Map Alu to Chips ROP source data  */
extern int ctAluConv2[];       	       /* Map Alu to Chips ROP pattern data */

extern unsigned long ctFrameBufferSize;		/* Frame buffer size */

/* Byte reversal functions */
extern unsigned char byte_reversed[];
extern unsigned int byte_reversed3[];

/* 
 * Definitions for IO access to 32 bit ports
 */
extern int ctReg32MMIO[];
extern int ctReg32HiQV[];
#define DR(x) ctReg32MMIO[x]
#define BR(x) ctReg32HiQV[x]

/*
 * Forward definitions for the functions that make up the driver.    See
 * the definitions of these functions for the real scoop.
 */

/* in ct_blitter.c */
extern void ctBitBlt();
extern void ctMMIOBitBlt();
extern void ctHiQVBitBlt();

/* in ct_BitBlt.c */
extern void ctcfbDoBitbltCopy();
extern void ctcfbFillBoxSolid();
extern void ctcfbCopyPlane1to8();

/* in ct_solid.c */
extern void ctcfbFillRectSolid();
extern void ctcfbFillSolidSpansGeneral();
extern void ctMMIOFillRectSolid();
extern void ctMMIOFillSolidSpansGeneral();
extern void ctHiQVFillRectSolid();
extern void ctHiQVFillSolidSpansGeneral();

/* in ct_blt16.c */
extern RegionPtr ctcfb16CopyArea();
extern RegionPtr ctcfb24CopyArea();

/* in ct_pci.c */
extern int ctPCIMemBase();
extern int ctPCIIOBase();

/* in ct_FillRct.c */
extern void ctcfbPolyFillRect();

/* in ct_FillRct.c */
extern void ctcfbFillRectOpaqueStippled32();
extern void ctcfbFillRectTransparentStippled32();
extern void ctMMIOFillRectOpaqueStippled32();
extern void ctMMIOFillRectTransparentStippled32();
extern void ctHiQVFillRectOpaqueStippled32();
extern void ctHiQVFillRectTransparentStippled32();

/* in ct_line.c */
extern void ctMMIOLineSS();
extern void ctMMIOSegmentSS();
extern void ctHiQVLineSS();
extern void ctHiQVSegmentSS();

/* in ct_teblt8.c */
extern void ctTransferText();
extern void ctTransferText24();
extern void ctcfbImageGlyphBlt();
extern void ctcfbPolyGlyphBlt();
extern void ctMMIOImageGlyphBlt();
extern void ctMMIOPolyGlyphBlt();
extern void ctHiQVImageGlyphBlt();
extern void ctHiQVPolyGlyphBlt();

/* in ct_colexp.c */
extern void ctcfbColorExpandStippleFill();
extern void ctMMIOColorExpandStippleFill();
extern void ctHiQVColorExpandStippleFill();
extern void ctcfbBLTWriteBitmap();
extern void ctMMIOBLTWriteBitmap();
extern void ctHiQVBLTWriteBitmap();

#define MMIOmeml(x) *(unsigned int *)(ctMMIOBase + x)
#define MMIOmemw(x) *(unsigned short *)(ctMMIOBase + x)

/* To aid debugging of 32 bit register access we make the following defines */
#if defined(DEBUG) & defined(CT_HW_DEBUG)
extern void ctHWDebug();
#define HW_DEBUG(x) ctHWDebug(x)
#else
#define HW_DEBUG(x)
#endif

