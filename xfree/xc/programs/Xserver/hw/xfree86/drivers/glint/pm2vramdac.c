/*
 * Copyright 1998 by Alan Hourihane, Wigan, England.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Alan Hourihane not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Alan Hourihane makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ALAN HOURIHANE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ALAN HOURIHANE BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 * Authors:  Alan Hourihane, <alanh@fairlite.demon.co.uk>
 *
 * Permedia2vOutIndReg() and Permedia2vInIndReg() are used to access 
 * the indirect Permedia2v RAMDAC registers only.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/glint/pm2vramdac.c,v 1.4 1999/02/12 22:52:05 hohndel Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"

#include "xf86PciInfo.h"
#include "xf86Pci.h"

#include "glint_regs.h"
#include "glint.h"

void
Permedia2vOutIndReg(ScrnInfoPtr pScrn,
		     CARD32 reg, unsigned char mask, unsigned char data)
{
  GLINTPtr pGlint = GLINTPTR(pScrn);
  unsigned char tmp = 0x00;

  GLINT_SLOW_WRITE_REG((reg>>8) & 0xff, PM2VDACIndexRegHigh);
  GLINT_SLOW_WRITE_REG(reg&0xff, PM2VDACIndexRegLow);

  if (mask != 0x00)
    tmp = GLINT_READ_REG (PM2VDACIndexData) & mask;

  GLINT_SLOW_WRITE_REG (tmp | data, PM2VDACIndexData);
}

unsigned char
Permedia2vInIndReg (ScrnInfoPtr pScrn, CARD32 reg)
{
  GLINTPtr pGlint = GLINTPTR(pScrn);
  unsigned char ret;

  GLINT_SLOW_WRITE_REG (reg&0xff, PM2VDACIndexRegLow);
  GLINT_SLOW_WRITE_REG((reg>>8) & 0xff, PM2VDACIndexRegHigh);
  ret = GLINT_READ_REG (PM2VDACIndexData);

  return (ret);
}
