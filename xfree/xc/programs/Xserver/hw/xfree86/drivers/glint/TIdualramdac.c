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
 * Modified from IBMramdac.c to support TI RAMDAC routines 
 *   by Jens Owen, <jens@precisioninsight.com>.
 *
 * glintOutTIIndReg() and glintInTIIndReg() are used to access 
 * the indirect TI RAMDAC registers only.
 */
/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/glint/TIdualramdac.c,v 1.1 2000/03/21 21:46:27 alanh Exp $ */

#include "xf86.h"
#include "xf86_OSproc.h"
#include "xf86_ansic.h"

#include "xf86PciInfo.h"
#include "xf86Pci.h"

#include "TI.h"
#include "glint_regs.h"
#include "glint.h"

#define TI_WRITE_ADDR            0x4000  
#define TI_RAMDAC_DATA           0x4008 
#define TI_PIXEL_MASK            0x4010 
#define TI_READ_ADDR             0x4018  
#define TI_CURS_COLOR_WRITE_ADDR 0x4020 
#define TI_CURS_COLOR_DATA       0x4028 
#define TI_CURS_COLOR_READ_ADDR  0x4038 
#define TI_DIRECT_CURS_CTRL      0x4048 
#define TI_INDEX_DATA            0x4050 
#define TI_CURS_RAM_DATA         0x4058 
#define TI_CURS_X_LOW            0x4060 
#define TI_CURS_X_HIGH           0x4068 
#define TI_CURS_Y_LOW            0x4070 
#define TI_CURS_Y_HIGH           0x4078 

void
DUALglintOutTIIndReg(ScrnInfoPtr pScrn,
		     CARD32 reg, unsigned char mask, unsigned char data)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    unsigned char tmp = 0x00;
    int offset;

    if ((reg & 0xf0) == 0xa0) { /* this is really a direct register write */
	offset = TI_WRITE_ADDR + ((reg & 0xf) << 3);
	if (mask != 0x00)
	    tmp = GLINT_SECONDARY_READ_REG(offset) & mask;

	GLINT_SECONDARY_SLOW_WRITE_REG(tmp | data, offset);
    }
    else { /* normal indirect access */
	GLINT_SECONDARY_SLOW_WRITE_REG(reg & 0xFF, TI_WRITE_ADDR);

	if (mask != 0x00)
	    tmp = GLINT_SECONDARY_READ_REG(TI_INDEX_DATA) & mask;

	GLINT_SECONDARY_SLOW_WRITE_REG(tmp | data, TI_INDEX_DATA);
    }
}

unsigned char
DUALglintInTIIndReg (ScrnInfoPtr pScrn, CARD32 reg)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    unsigned char ret;
    int offset;

    if ((reg & 0xf0) == 0xa0) { /* this is really a direct register write */
	offset = TI_WRITE_ADDR + ((reg & 0xf) << 3);
	ret = GLINT_SECONDARY_READ_REG(offset);
    }
    else { /* normal indirect access */
	GLINT_SECONDARY_SLOW_WRITE_REG(reg & 0xFF, TI_WRITE_ADDR);
	ret = GLINT_SECONDARY_READ_REG(TI_INDEX_DATA);
    }

    return (ret);
}

void
DUALglintTIWriteAddress (ScrnInfoPtr pScrn, CARD32 index)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    
    GLINT_SECONDARY_SLOW_WRITE_REG(index, TI_WRITE_ADDR);
}

void
DUALglintTIWriteData (ScrnInfoPtr pScrn, unsigned char data)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    
    GLINT_SECONDARY_SLOW_WRITE_REG(data, TI_RAMDAC_DATA);
}

void
DUALglintTIReadAddress (ScrnInfoPtr pScrn, CARD32 index)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    
    GLINT_SECONDARY_SLOW_WRITE_REG(0xFF, TI_PIXEL_MASK);
    GLINT_SECONDARY_SLOW_WRITE_REG(index, TI_READ_ADDR);
}

unsigned char
DUALglintTIReadData (ScrnInfoPtr pScrn)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);
    
    return(GLINT_SECONDARY_READ_REG(TI_RAMDAC_DATA));
}
