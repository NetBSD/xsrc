/**************************************************************************

Copyright 1998-1999 Precision Insight, Inc., Cedar Park, Texas.
All Rights Reserved.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sub license, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice (including the
next paragraph) shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

**************************************************************************/
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/i810/i810_io.c,v 1.3 2000/05/11 18:14:34 tsi Exp $ */

/*
 * Authors:
 *   Daryll Strauss <daryll@precisioninsight.com>
 *
 */

#include "xf86.h"
#include "xf86_ansic.h"
#include "xf86_OSproc.h"
#include "compiler.h"

#include "i810.h"

static void
I810WriteControlPIO(I810Ptr pI810, int addr, CARD8 index, CARD8 val) {
  outb(addr, index);
  outb(addr+1, val);
}

static CARD8
I810ReadControlPIO(I810Ptr pI810, int addr, CARD8 index) {
  outb(addr, index);
  return inb(addr+1);
}

static void
I810WriteStandardPIO(I810Ptr pI810, int addr, CARD8 val) {
  outb(addr, val);
}

static CARD8
I810ReadStandardPIO(I810Ptr pI810, int addr) {
  return inb(addr);
}

void I810SetPIOAccess(I810Ptr pI810) {
  pI810->writeControl=I810WriteControlPIO;
  pI810->readControl=I810ReadControlPIO;
  pI810->writeStandard=I810WriteStandardPIO;
  pI810->readStandard=I810ReadStandardPIO;
}

static void
I810WriteControlMMIO(I810Ptr pI810, int addr, CARD8 index, CARD8 val) {
  moutb(addr, index);
  moutb(addr+1, val);
}

static CARD8
I810ReadControlMMIO(I810Ptr pI810, int addr, CARD8 index) {
  moutb(addr, index);
  return minb(addr+1);
}

static void
I810WriteStandardMMIO(I810Ptr pI810, int addr, CARD8 val) {
  moutb(addr, val);
}

static CARD8
I810ReadStandardMMIO(I810Ptr pI810, int addr) {
  return minb(addr);
}

void I810SetMMIOAccess(I810Ptr pI810) {
  pI810->writeControl=I810WriteControlMMIO;
  pI810->readControl=I810ReadControlMMIO;
  pI810->writeStandard=I810WriteStandardMMIO;
  pI810->readStandard=I810ReadStandardMMIO;
}

