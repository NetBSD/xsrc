/*
 * Copyright 1996-1997  Joerg Knura (knura@imst.de)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL 
 * JOERG KNURA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF 
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 * SOFTWARE.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/p9x00/p9x00VGARegs.h,v 1.1.2.2 1998/10/31 14:41:02 hohndel Exp $ */

/* VGA Registers - many of these are taken from libvga.h in svgalib */ 
#define CRT_IC        0x3D4
#define CRT_DC        0x3D5
#define IS1_RC        0x3DA
#define CRT_C         24
#define CRT           0   
#define ATT_C         21
#define ATT_IW        0x3C0
#define ATT_          CRT+CRT_C
#define ATT_R         0x3C1
#define GRA_C         9
#define GRA           ATT_+ATT_C
#define GRA_I         0x3CE         /* Graphics Controller Index */
#define GRA_D         0x3CF         /* Graphics Controller Data Register */
#define SEQ_C         5
#define SEQ           GRA+GRA_C
#define MIS           SEQ+SEQ_C

/* Sequencer Registers (see section 3.5 of the W5186 databook) */
#define SEQ_INDEX_REG        0x3C4  /* For setting the index */
#define SEQ_PORT             0x3C5
#define SEQ_MISC_INDEX       0x11
#define SEQ_CLKMODE_INDEX    0x01
#define SEQ_OUTPUT_CTL_INDEX 0x12   /* Vendor specific */       

/* VGA bank register */
#define VGA_BANK_REG 0x3CD

/* W5[12]86 Misc Output Register - See 3.4.3 of W5186 DataBook
 * Used for selecting which clock to use */
#define MISC_OUT_REG        0x3c2  /* Write to this port */
#define MISC_IN_REG         0x3cc  /* Read from this port */

/* VGA-related parameters */
#define FONT_BASE  0xA0000       /* The location of font data */
#ifndef Lynx
#define FONT_SIZE  0x2000        /* The size of font data */
#else
#define FONT_SIZE  0x4000        /* The size of font data */
#define TEXT_SIZE  0x4000        /* The size of font data */
#endif

/* Sync Polarities as stored in the VGA Sequencer Control Register */
#define SP_NEGATIVE 1     /*sync polarity is negative */
#define SP_POSITIVE 0     /*sync polarity is positive */

