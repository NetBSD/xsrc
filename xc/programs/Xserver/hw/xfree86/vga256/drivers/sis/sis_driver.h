/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/sis/sis_driver.h,v 1.1.2.8 1999/12/21 07:43:44 hohndel Exp $ */

/*
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
 *
 * Modified for Sis by Xavier Ducoin (xavier@rd.lectra.fr)
 *
 */


#define	SIS_MAJOR_VERSION	3
#define	SIS_MINOR_VERSION	0
#define	SIS_PATCH_LEVEL		0
#define	SIS_CURRENT_VERSION	(SIS_DRV_MAJOR_VERSION << 16 | \
				 SIS_DRV_MINOR_VERSION << 8 | \
				 SIS_DRV_PATCH_LEVEL)

#define SIS86C201 0
#define SIS86C202 1
#define SIS86C205 2
#define SIS86C215 3 /* treated as SIS86C205 */
#define SIS86C225 4 /* treated as SIS86C205 */
#define SIS5597   5
#define SIS5598   6 /* treated as SIS5597 */
#define SIS6326   7
#define SIS530    8 /* 2nd generation graphics engine */
#define SIS620    9 /* treated as SIS530 */
#define SIS300   10
#define SIS630   11
#define SIS540   12

#define SIS5597_3C5_LAST	0x39
#define SIS6326_3C5_LAST	0x3C
#define SIS530_3C5_LAST		0x3F
#define SIS300_3C5_LAST		0x3D
#define DEFAULT_3C5_LAST	0x37

#define SIS300_CURSOR_CONTROL	0x8500
#define SIS300_CURSOR_COLOR0	0x8504
#define SIS300_CURSOR_COLOR1	0x8508
#define SIS300_CURSOR_H_LOC	0x850C
#define SIS300_CURSOR_V_LOC	0x8510

#define SIS300_CURSOR_ENABLE	0x40000000
#define SIS300_CURSOR_DISABLE	0x3FFFFFFF

extern int SISchipset;
extern int SISfamily;

extern Bool sisLinearSupport;	       /*linear addressing enable */

extern Bool sisUseMMIO;
extern volatile unsigned char *sisMMIOBase;
extern unsigned int sisBLTPatternAddress;
extern int sisBLTPatternOffscreenSize;
extern Bool sisAvoidImageBLT;
extern unsigned char *sisBltDataWindow;

extern Bool sisHWCursor;
extern Bool sisTurboQueue;

extern int sisAluConv[];		       /* Map Alu to SIS ROP source data  */

/* 
 * Definitions for IO access to 32 bit ports
 */
extern int sisReg32MMIO[];
extern int sis2Reg32MMIO[];



/*
 * Forward definitions for the functions that make up the driver.    See
 * the definitions of these functions for the real scoop.
 */

/* in sis_blitter.c */
extern void sisBitBlt();
extern void sisMMIOBitBlt();

/* in sis_BitBlt.c */
extern void siscfbDoBitbltCopy();
extern void siscfbFillBoxSolid();

/* in sis_solid.c */
extern void siscfbFillRectSolid();
extern void siscfbFillSolidSpansGeneral();
extern void sisMMIOFillRectSolid();
extern void sisMMIOFillSolidSpansGeneral();

/* in sis_blt16.c */
extern RegionPtr siscfb16CopyArea();
extern RegionPtr siscfb24CopyArea();
extern void siscfbCopyWindow();

/* in sis_line.c */
extern void sisMMIOLineSS();
extern void sisMMIOSegmentSS();

/* in sis_pntwin.c */
extern void sisPaintWindow();

/* in sis_FillRct.c */
extern void siscfbPolyFillRect();

/* in ct_FillSt.c */
extern void siscfbFillRectOpaqueStippled32();
extern void siscfbFillRectTransparentStippled32();
extern void sisMMIOFillRectOpaqueStippled32();
extern void sisMMIOFillRectTransparentStippled32();

/* in sis_teblt8.c */
extern void sisMMIOImageGlyphBlt();
extern void sisMMIOPolyGlyphBlt();





