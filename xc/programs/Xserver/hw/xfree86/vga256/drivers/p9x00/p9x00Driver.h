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
/* $XFree86: xc/programs/Xserver/hw/xfree86/vga256/drivers/p9x00/p9x00Driver.h,v 1.1.2.1 1998/08/25 10:54:16 hohndel Exp $ */

#ifndef P9X00DRIVER_H
#define P9X00DRIVER_H
/*
 * Driver data structures.
 */
typedef struct {
  vgaHWRec std;
  Bool vga_mode;
  int BpPix;
  CARD32 syscfg;
  CARD32 memctrl;
  CARD32 timingctrl;
  CARD32 hrzsr;
  CARD32 hrzbr;
  CARD32 hrzbf;
  CARD32 hrzt;
  CARD32 vrtsr;
  CARD32 vrtbr;
  CARD32 vrtbf;
  CARD32 vrtt;
  int hpol;
  int vpol; 
  CARD32 refreshperiod;
  CARD32 syncpol;
  CARD32 memspeed;
  long dotfreq;
  long icdfreq;
  Bool dacdivides;
  Bool InvertSCLK;
  CARD32 byteclipmax;
  CARD32 clipmax;
} vgap9x00Rec, *vgap9x00Ptr;

#endif
