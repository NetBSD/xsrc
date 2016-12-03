/*
 * Copyright 1997-2001 by Alan Hourihane, Wigan, England.
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
 *           Dirk Hohndel, <hohndel@suse.de>
 *           Stefan Dirsch, <sndirsch@suse.de>
 *           Mark Vojkovich, <mvojkovi@ucsd.edu>
 *           Michel Dänzer, <michdaen@iiic.ethz.ch>
 *
 * this work is sponsored by S.u.S.E. GmbH, Fuerth, Elsa GmbH, Aachen and
 * Siemens Nixdorf Informationssysteme
 * 
 * Permedia 2 accelerated options.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <X11/Xarch.h>
#include "xf86.h"
#include "xf86_OSproc.h"

#include "xf86Pci.h"

#include "glint_regs.h"
#include "glint.h"

#if DEBUG
# define TRACE_ENTER(str)       ErrorF("pm2_common: " str " %d\n",pScrn->scrnIndex)
# define TRACE_EXIT(str)        ErrorF("pm2_common: " str " done\n")
# define TRACE(str)             ErrorF("pm2_common trace: " str "\n")
#else
# define TRACE_ENTER(str)
# define TRACE_EXIT(str)
# define TRACE(str)
#endif

void
Permedia2InitializeEngine(ScrnInfoPtr pScrn)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);

    /* Initialize the Accelerator Engine to defaults */

    TRACE_ENTER("Permedia2InitializeEngine");

    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	ScissorMode);
    GLINT_SLOW_WRITE_REG(UNIT_ENABLE,	FBWriteMode);
    GLINT_SLOW_WRITE_REG(0, 		dXSub);
    GLINT_SLOW_WRITE_REG(GWIN_DisableLBUpdate,   GLINTWindow);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	DitherMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AlphaBlendMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	ColorDDAMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TextureColorMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TextureAddressMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	PMTextureReadMode);
    GLINT_SLOW_WRITE_REG(pGlint->pprod,	LBReadMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AlphaBlendMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	TexelLUTMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	YUVMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	DepthMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	RouterMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FogMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AntialiasMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AlphaTestMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	StencilMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	AreaStippleMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LogicalOpMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	DepthMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	StatisticMode);
    GLINT_SLOW_WRITE_REG(0x400,		FilterMode);
    GLINT_SLOW_WRITE_REG(0xffffffff,	FBHardwareWriteMask);
    GLINT_SLOW_WRITE_REG(0xffffffff,	FBSoftwareWriteMask);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	RasterizerMode);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	GLINTDepth);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBSourceOffset);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBPixelOffset);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LBSourceOffset);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	WindowOrigin);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBWindowBase);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	FBSourceBase);
    GLINT_SLOW_WRITE_REG(UNIT_DISABLE,	LBWindowBase);

#if X_BYTE_ORDER == X_BIG_ENDIAN
    pGlint->RasterizerSwap = 1;
#else
    pGlint->RasterizerSwap = 0;
#endif

    switch (pScrn->bitsPerPixel) {
	case 8:
	    pGlint->PixelWidth = 0x0; /* 8 Bits */
	    pGlint->TexMapFormat = pGlint->pprod;
#if X_BYTE_ORDER == X_BIG_ENDIAN
	    pGlint->RasterizerSwap |= 3<<15;	/* Swap host data */
#endif
	    break;
	case 16:
	    pGlint->PixelWidth = 0x1; /* 16 Bits */
	    pGlint->TexMapFormat = pGlint->pprod | 1<<19;
#if X_BYTE_ORDER == X_BIG_ENDIAN
	    pGlint->RasterizerSwap |= 2<<15;	/* Swap host data */
#endif
	    break;
	case 24:
 	    pGlint->PixelWidth = 0x4; /* 24 Bits */
	    pGlint->TexMapFormat = pGlint->pprod | 2<<19;
	    break;
	case 32:
	    pGlint->PixelWidth = 0x2; /* 32 Bits */
	    pGlint->TexMapFormat = pGlint->pprod | 2<<19;
  	    break;
    }
    pGlint->ClippingOn = FALSE;
    pGlint->startxdom = 0;
    pGlint->startxsub = 0;
    pGlint->starty = 0;
    pGlint->count = 0;
    pGlint->dy = 1<<16;
    pGlint->dxdom = 0;
    pGlint->x = 0;
    pGlint->y = 0;
    pGlint->h = 0;
    pGlint->w = 0;
    pGlint->ROP = 0xFF;
    GLINT_SLOW_WRITE_REG(pGlint->PixelWidth, FBReadPixel);
    GLINT_SLOW_WRITE_REG(pGlint->TexMapFormat, PMTextureMapFormat);
    GLINT_SLOW_WRITE_REG(0, RectangleSize);
    GLINT_SLOW_WRITE_REG(0, RectangleOrigin);
    GLINT_SLOW_WRITE_REG(0, dXDom);
    GLINT_SLOW_WRITE_REG(1<<16, dY);
    GLINT_SLOW_WRITE_REG(0, StartXDom);
    GLINT_SLOW_WRITE_REG(0, StartXSub);
    GLINT_SLOW_WRITE_REG(0, StartY);
    GLINT_SLOW_WRITE_REG(0, GLINTCount);

    TRACE_EXIT("Permedia2InitializeEngine");
}

void
Permedia2Sync(ScrnInfoPtr pScrn)
{
    GLINTPtr pGlint = GLINTPTR(pScrn);

    CHECKCLIPPING;

    while (GLINT_READ_REG(DMACount) != 0);
    GLINT_WAIT(2);
    GLINT_WRITE_REG(0x400, FilterMode);
    GLINT_WRITE_REG(0, GlintSync);
    do {
   	while(GLINT_READ_REG(OutFIFOWords) == 0);
    } while (GLINT_READ_REG(OutputFIFO) != Sync_tag);
}

