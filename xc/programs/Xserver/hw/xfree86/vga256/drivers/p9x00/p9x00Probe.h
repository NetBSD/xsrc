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
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/p9x00/p9x00Probe.h,v 1.1.2.1 1998/08/25 10:54:23 hohndel Exp $ */

#ifndef P9X00PROBE_H
#define P9X00PROBE_H

#include "p9x00Includes.h"
#include "p9x00Access.h"

typedef struct {
  int chiptype;
  int bustype;
  int dactype;
  int clocktype;
  int memtype;
  CARD32 pci_busnum;
  CARD32 pci_cardnum;
  CARD32 pci_func;
  CARD32 physicalbase;
  CARD16 iobase;
  int chiprev;
  int regionsize;
  int memsize;
  int membanks;
  int samsize;
  int dacsize;
  Bool has9130;
  CARD8 p9000sequencermask;
  CARD8 p9000sequencerset;
} p9x00config_type;

#define P9X_V_NONE      0
#define P9X_V_P9000     1
#define P9X_V_P9100     2
#define P9X_V_PCIBUS    3
#define P9X_V_VLBUS     4
#define P9X_V_IBM525    5
#define P9X_V_BT484     6
#define P9X_V_BT485     7
#define P9X_V_BT485A    8
#define P9X_V_BT489     9
#define P9X_V_ATT504   10
#define P9X_V_ATT505   11
#define P9X_V_ATT511   12
#define P9X_V_PIXEL    13
#define P9X_V_OSCCLOCK 14
#define P9X_V_ICDCLOCK 15
#define P9X_V_HALFSAM  16
#define P9X_V_FULLSAM  17
#define P9X_V_VRAM128  18
#define P9X_V_VRAM256  19

#ifndef P9X00PROBE_C

  extern p9x00config_type   p9x00config;
  
#else  /* P9X00PROBE_C */

  p9x00config_type   p9x00config;
  
#endif /* P9X00PROBE_C */

#define P9X_CFG_CHIP  p9x00config.chiptype
#define P9X_CFG_BUS   p9x00config.bustype
#define P9X_CFG_DAC   p9x00config.dactype
#define P9X_CFG_CLK   p9x00config.clocktype
#define P9X_CFG_MEM   p9x00config.memtype
#define P9X_CFG_PBUS  p9x00config.pci_busnum
#define P9X_CFG_PDEV  p9x00config.pci_cardnum
#define P9X_CFG_PFUN  p9x00config.pci_func
#define P9X_CFG_BASE  p9x00config.physicalbase
#define P9X_CFG_IO    p9x00config.iobase
#define P9X_CFG_REV   p9x00config.chiprev
#define P9X_CFG_ALL   p9x00config.regionsize
#define P9X_CFG_SIZE  p9x00config.memsize
#define P9X_CFG_DW    p9x00config.dacsize
#define P9X_CFG_BANKS p9x00config.membanks
#define P9X_CFG_SAM   p9x00config.samsize
#define P9X_CFG_VCP   p9x00config.has9130

#define P9X_CFG_SEQ_MASK p9x00config.p9000sequencermask
#define P9X_CFG_SEQ_SET  p9x00config.p9000sequencerset

static Bool p9x00IsMemCorrect(int);
static void p9x00TestMem(void);
static void p9x00TestDAC(void);
static void p9x00TestClk(void);
static void p9x00TestVCP(void);

Bool p9x00Probe(void);
char *p9x00Ident(int);
  
#endif /* P9X00PROBE_H */
