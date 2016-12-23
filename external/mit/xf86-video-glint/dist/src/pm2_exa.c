/*
 * Copyright 1997-2001 by Alan Hourihane, Wigan, England.
 * Copyright 2016 by Michael Lorenz 
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
 * Permedia 2 accelerated options, EXA style.
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

/*#define PM2_DEBUG*/

#ifdef PM2_DEBUG
#define STATIC static
#define ENTER xf86Msg(X_ERROR, "%s>\n", __func__);
#define DPRINTF xf86Msg
#else
#define ENTER
#define STATIC 
#define DPRINTF while (0) xf86Msg
#endif

int src_formats[] = {PICT_a8r8g8b8, PICT_x8r8g8b8,
		     PICT_a8b8g8r8, PICT_x8b8g8r8};
int tex_formats[] = {PICT_a8r8g8b8, PICT_a8b8g8r8};

static uint32_t Pm2BlendOps[] = {
    /* Clear */
    ABM_SrcZERO			| ABM_DstZERO,
    /* Src */
    ABM_SrcONE			| ABM_DstZERO,
    /* Dst */
    ABM_SrcZERO			| ABM_DstONE,
    /* Over */
    ABM_SrcONE			| ABM_DstONE_MINUS_SRC_ALPHA,
    /* OverReverse */
    ABM_SrcONE_MINUS_DST_ALPHA	| ABM_DstONE,
    /* In */
    ABM_SrcDST_ALPHA		| ABM_DstZERO,
    /* InReverse */
    ABM_SrcZERO			| ABM_DstSRC_ALPHA,
    /* Out */
    ABM_SrcONE_MINUS_DST_ALPHA	| ABM_DstZERO,
    /* OutReverse */
    ABM_SrcZERO			| ABM_DstONE_MINUS_SRC_ALPHA,
    /* Atop */
    ABM_SrcDST_ALPHA		| ABM_DstONE_MINUS_SRC_ALPHA,
    /* AtopReverse */
    ABM_SrcONE_MINUS_DST_ALPHA	| ABM_DstSRC_ALPHA,
    /* Xor */
    ABM_SrcONE_MINUS_DST_ALPHA	| ABM_DstONE_MINUS_SRC_ALPHA,
    /* Add */
    ABM_SrcONE			| ABM_DstONE
};

#define arraysize(ary)        (sizeof(ary) / sizeof(ary[0]))

STATIC void
Pm2WaitMarker(ScreenPtr pScreen, int Marker)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];

	Permedia2Sync(pScrn);
}

STATIC Bool
Pm2PrepareCopy(PixmapPtr pSrcPixmap, PixmapPtr pDstPixmap,
		int xdir, int ydir, int rop, Pixel planemask)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);

	ENTER;

	pGlint->BltScanDirection = 0;
	if (xdir == 1) pGlint->BltScanDirection |= XPositive;
	if (ydir == 1) pGlint->BltScanDirection |= YPositive;
  
	GLINT_WAIT(6);
	DO_PLANEMASK(planemask);
	GLINT_WRITE_REG(UNIT_DISABLE, AlphaBlendMode);
	GLINT_WRITE_REG(UNIT_DISABLE, DitherMode);
	GLINT_WRITE_REG(UNIT_DISABLE, TextureAddressMode);

	GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
	if ((rop == GXset) || (rop == GXclear)) {
		pGlint->FrameBufferReadMode = pGlint->pprod;
	} else
	if ((rop == GXcopy) || (rop == GXcopyInverted)) {
		pGlint->FrameBufferReadMode = pGlint->pprod |FBRM_SrcEnable;
	} else {
		pGlint->FrameBufferReadMode = pGlint->pprod | FBRM_SrcEnable |
							FBRM_DstEnable;
	}
	LOADROP(rop);
	pGlint->srcoff = exaGetPixmapOffset(pSrcPixmap);
	return TRUE;
}

STATIC void
Pm2Copy(PixmapPtr pDstPixmap,
         int srcX, int srcY, int dstX, int dstY, int w, int h)
{
	ScrnInfoPtr pScrn = xf86Screens[pDstPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);
	int align;
	int dstoff = exaGetPixmapOffset(pDstPixmap);
	int pitch = exaGetPixmapPitch(pDstPixmap);
	
	ENTER;

	/* assuming constant pitch for now */	
	srcY += pGlint->srcoff / pitch;
	dstY += dstoff / pitch;

	/* We can only use GXcopy for Packed modes */
	if ((pGlint->ROP != GXcopy) || (pScrn->bitsPerPixel != 8)) { 
		GLINT_WAIT(5);
		GLINT_WRITE_REG(pGlint->FrameBufferReadMode, FBReadMode);
        	Permedia2LoadCoord(pScrn, dstX, dstY, w, h);
        	GLINT_WRITE_REG(((srcY - dstY) & 0x0FFF) << 16 |
        			((srcX - dstX) & 0x0FFF), FBSourceDelta);
	} else {
  		align = (dstX & pGlint->bppalign) - (srcX & pGlint->bppalign);
		GLINT_WAIT(6);
		GLINT_WRITE_REG(pGlint->FrameBufferReadMode|FBRM_Packed, FBReadMode);
        	Permedia2LoadCoord(pScrn, dstX >> pGlint->BppShift, dstY, 
					(w + 7) >> pGlint->BppShift, h);
  		GLINT_WRITE_REG(align << 29 | dstX << 16 | (dstX + w), PackedDataLimits);
        	GLINT_WRITE_REG(((srcX - dstX) & 0x0FFF) << 16 |
        		       (((srcX & ~pGlint->bppalign) - (dstX & ~pGlint->bppalign)) & 0x0FFF),
        		       FBSourceDelta);
	}
	GLINT_WRITE_REG(PrimitiveRectangle | pGlint->BltScanDirection, Render);
}

STATIC void
Pm2DoneCopy(PixmapPtr pPixmap)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);

}

STATIC Bool
Pm2PrepareSolid(PixmapPtr pPixmap, int rop, Pixel planemask, Pixel color)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);

	ENTER;

	REPLICATE(color);

	GLINT_WAIT(8);
	GLINT_WRITE_REG(UNIT_DISABLE, AlphaBlendMode);
	GLINT_WRITE_REG(UNIT_DISABLE, DitherMode);
	GLINT_WRITE_REG(UNIT_DISABLE, TextureAddressMode);
	DO_PLANEMASK(planemask);
	if (rop == GXcopy) {
		GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
		GLINT_WRITE_REG(pGlint->pprod, FBReadMode);
		GLINT_WRITE_REG(color, FBBlockColor);
	} else {
		GLINT_WRITE_REG(UNIT_ENABLE, ColorDDAMode);
      		GLINT_WRITE_REG(color, ConstantColor);
		/* We can use Packed mode for filling solid non-GXcopy rasters */
		GLINT_WRITE_REG(pGlint->pprod|FBRM_DstEnable|FBRM_Packed, FBReadMode);
	}
	LOADROP(rop);

	return TRUE;
}

STATIC void
Pm2Solid(PixmapPtr pPixmap, int x1, int y1, int x2, int y2)
{
	ScrnInfoPtr pScrn = xf86Screens[pPixmap->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);
	int offset = exaGetPixmapOffset(pPixmap);
	int pitch = exaGetPixmapPitch(pPixmap);
	int w = x2 - x1, h = y2 - y1;
	int speed = 0;

	ENTER;

	y1 += offset / pitch;
	if (pGlint->ROP == GXcopy) {
		GLINT_WAIT(3);
        	Permedia2LoadCoord(pScrn, x1, y1, w, h);
		speed = FastFillEnable;
	} else {
		GLINT_WAIT(4);
		Permedia2LoadCoord(pScrn, x1 >> pGlint->BppShift, y1, 
					(w + 7) >> pGlint->BppShift, h);
  		GLINT_WRITE_REG(x1 << 16 | (x1 + w), PackedDataLimits);
  		speed = 0;
	}
	GLINT_WRITE_REG(PrimitiveRectangle | XPositive | YPositive | speed, Render);
}

/*
 * Memcpy-based UTS.
 */
STATIC Bool
Pm2UploadToScreen(PixmapPtr pDst, int x, int y, int w, int h,
    char *src, int src_pitch)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);
	int    offset	   = exaGetPixmapOffset(pDst);
	unsigned char *dst = pGlint->FbBase + offset;
	CARD32 *s;
	int    dst_pitch   = exaGetPixmapPitch(pDst);

	int bpp    = pDst->drawable.bitsPerPixel;
	int cpp    = (bpp + 7) >> 3;
	int wBytes = w * cpp;
	int xx, i, fs = pGlint->FIFOSize, chunk, adr;

	ENTER;

	if (bpp < 24) {
		/* for now */
		dst += (x * cpp) + (y * dst_pitch);

		Permedia2Sync(pScrn);

		while (h--) {
			memcpy(dst, src, wBytes);
			src += src_pitch;
			dst += dst_pitch;
		}
	} else {
		/* use host blit */
		y += offset / dst_pitch;
		adr = y * (dst_pitch >> 2) + x;
		DPRINTF(X_ERROR, "%d %d\n", x, y);
        	while (h--) {
        		s = (CARD32 *)src;
        		xx = w;
        		GLINT_WAIT(2);
        		DO_PLANEMASK(0xffffffff);
        		GLINT_WRITE_REG(adr, TextureDownloadOffset);
			while (xx > 0) {
				chunk = min(fs - 1, xx);
	        		GLINT_WAIT(chunk);
	        	      	GLINT_WRITE_REG(((chunk - 1) << 16) | (0x11 << 4) | 
					0x0d, OutputFIFO);
				GLINT_MoveDWORDS(
					(CARD32*)((char*)pGlint->IOBase + OutputFIFO + 4),
	 				s, chunk);
	 			xx -= chunk;
	 			s += chunk;
			}
			adr += (dst_pitch >> 2);
			src += src_pitch;
		}
		exaMarkSync(pDst->drawable.pScreen);
	}
	return TRUE;
}

/*
 * Memcpy-based DFS.
 */
STATIC Bool
Pm2DownloadFromScreen(PixmapPtr pSrc, int x, int y, int w, int h,
    char *dst, int dst_pitch)
{
 	ScrnInfoPtr pScrn = xf86Screens[pSrc->drawable.pScreen->myNum];
	GLINTPtr pGlint    = GLINTPTR(pScrn);
	unsigned char *src = pGlint->FbBase + exaGetPixmapOffset(pSrc);
	int    src_pitch   = exaGetPixmapPitch(pSrc);

	ENTER;
	int bpp    = pSrc->drawable.bitsPerPixel;
	int cpp    = (bpp + 7) >> 3;
	int wBytes = w * cpp;

	src += (x * cpp) + (y * src_pitch);

	Permedia2Sync(pScrn);

	while (h--) {
		memcpy(dst, src, wBytes);
		src += src_pitch;
		dst += dst_pitch;
	}

	return TRUE;
}

Bool
Pm2CheckComposite(int op, PicturePtr pSrcPicture,
                           PicturePtr pMaskPicture,
                           PicturePtr pDstPicture)
{
	int i, ok = FALSE;

	ENTER;

	i = 0;
	while ((i < arraysize(src_formats)) && (!ok)) {
		ok =  (pSrcPicture->format == src_formats[i]);
		i++;
	}

	if (!ok) {
		DPRINTF(X_ERROR, "%s: unsupported src format %x\n",
		    __func__, pSrcPicture->format);
		return FALSE;
	}

	if (pDstPicture != NULL) {
		i = 0;
		while ((i < arraysize(src_formats)) && (!ok)) {
			ok =  (pDstPicture->format == src_formats[i]);
			i++;
		}

		if (!ok) {
			DPRINTF(X_ERROR, "%s: unsupported dst format %x\n",
			    __func__, pDstPicture->format);
			return FALSE;
		}
	}

	DPRINTF(X_ERROR, "src is %x, %d\n", pSrcPicture->format, op);

	if (pMaskPicture != NULL) {
		i = 0;
		ok = FALSE;
		while ((i < arraysize(tex_formats)) && (!ok)) {
			ok =  (pMaskPicture->format == tex_formats[i]);
			i++;
		}
		if (!ok) {
			DPRINTF(X_ERROR, "%s: unsupported mask format %x\n",
			    __func__, pMaskPicture->format);
			return FALSE;
		}
		DPRINTF(X_ERROR, "mask is %x %d %d\n", pMaskPicture->format,
		    pMaskPicture->pDrawable->width,
		    pMaskPicture->pDrawable->height);
		if (op != PictOpOver)
			return FALSE;
	}
	DPRINTF(X_ERROR, "true\n");
	return TRUE;
}

Bool
Pm2PrepareComposite(int op, PicturePtr pSrcPicture,
                             PicturePtr pMaskPicture,
                             PicturePtr pDstPicture,
                             PixmapPtr  pSrc,
                             PixmapPtr  pMask,
                             PixmapPtr  pDst)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	GLINTPtr pGlint   = GLINTPTR(pScrn);

	ENTER;

	pGlint->no_source_pixmap = FALSE;
	pGlint->source_is_solid = FALSE;

	if (pSrcPicture->pSourcePict != NULL) {
		if (pSrcPicture->pSourcePict->type == SourcePictTypeSolidFill) {
			pGlint->fillcolour =
			    pSrcPicture->pSourcePict->solidFill.color;
			DPRINTF(X_ERROR, "%s: solid src %08x\n",
			    __func__, pGlint->fillcolour);
			pGlint->no_source_pixmap = TRUE;
			pGlint->source_is_solid = TRUE;
		}
	}
	if ((pMaskPicture != NULL) && (pMaskPicture->pSourcePict != NULL)) {
		if (pMaskPicture->pSourcePict->type ==
		    SourcePictTypeSolidFill) {
			pGlint->fillcolour = 
			   pMaskPicture->pSourcePict->solidFill.color;
			DPRINTF(X_ERROR, "%s: solid mask %08x\n",
			    __func__, pGlint->fillcolour);
		}
	}
	if (pMaskPicture != NULL) {
		pGlint->mskoff = exaGetPixmapOffset(pMask);
		pGlint->mskpitch = exaGetPixmapPitch(pMask);
		pGlint->mskformat = pMaskPicture->format;
	} else {
		pGlint->mskoff = 0;
		pGlint->mskpitch = 0;
		pGlint->mskformat = 0;
	}
	if (pSrc != NULL) {
		pGlint->source_is_solid = 
		   ((pSrc->drawable.width == 1) && (pSrc->drawable.height == 1));
		pGlint->srcoff = exaGetPixmapOffset(pSrc);
		pGlint->srcpitch = exaGetPixmapPitch(pSrc);
		if (pGlint->source_is_solid) {
			pGlint->fillcolour = exaGetPixmapFirstPixel(pSrc);
		}
	}
	pGlint->srcformat = pSrcPicture->format;
	pGlint->dstformat = pDstPicture->format;


	pGlint->op = op;
	/* first things first */
	if (pGlint->source_is_solid)
		goto ok;
	if (pGlint->mskpitch == 0)
		goto ok;
	DPRINTF(X_ERROR, "nope\n");
	return FALSE;
ok:
	GLINT_WAIT(10);
	GLINT_WRITE_REG(UNIT_DISABLE, ColorDDAMode);
	LOADROP(GXcopy);
	GLINT_WRITE_REG(0, QStart);
	GLINT_WRITE_REG(0, dQdx);
	GLINT_WRITE_REG(0, dQdyDom);
	GLINT_WRITE_REG(0x20, PMTextureDataFormat);
	GLINT_WRITE_REG(1 << 20, dSdx);
	GLINT_WRITE_REG(0, dSdyDom);
	GLINT_WRITE_REG(0, dTdx);
	GLINT_WRITE_REG(1 << 20, dTdyDom);
	return TRUE;
}

void
Pm2Comp_Over32Solid(ScrnInfoPtr pScrn, int maskX, int maskY,
				     int dstX, int dstY,
				     int width, int height)
{
	GLINTPtr pGlint = GLINTPTR(pScrn);
	unsigned long pp;
	uint32_t *m;
	int i, j;

	ENTER;

	/* first blit the source colour into the mask */
	GLINT_WAIT(8);
	GLINT_WRITE_REG(UNIT_DISABLE, AlphaBlendMode);
	GLINT_WRITE_REG(UNIT_DISABLE, DitherMode);
	GLINT_WRITE_REG(CFBRM_Packed| CWM_Enable, Config);
	GLINT_WRITE_REG(pGlint->fillcolour, FBBlockColor);
	DO_PLANEMASK(0x00ffffff);	/* preserve alpha */
       	Permedia2LoadCoord(pScrn, maskX, maskY + pGlint->mskoff / pGlint->mskpitch, width, height);
	GLINT_WRITE_REG(
	    PrimitiveRectangle | XPositive | YPositive | FastFillEnable,
	    Render);

	/* now do the actual rendering */
	GLINT_WAIT(15);

	/* enable alpha blending */
	GLINT_WRITE_REG(
	    ABM_SrcSRC_ALPHA | ABM_DstONE_MINUS_SRC_ALPHA |
	    ABM_ColorOrderRGB | UNIT_ENABLE, AlphaBlendMode);

	/* not sure if we need this, everything is in ARGB anyway */
	GLINT_WRITE_REG(UNIT_ENABLE | DTM_ColorOrderRGB, DitherMode);

	/* setup for drawing the dst rectangle... */
	GLINT_WRITE_REG(CFBRM_DstEnable | /*CFBRM_Packed|*/ CWM_Enable, Config);
       	Permedia2LoadCoord(pScrn, dstX, dstY, width, height);
       	/* point at the texture, 1:1 mapping etc. */
	GLINT_WRITE_REG(pGlint->mskoff >> 2, PMTextureBaseAddress);
	GLINT_WRITE_REG(1 | 0xb << 9 | 0xb << 13 | 1 << 1 | 1 << 3, PMTextureReadMode);		
	GLINT_WRITE_REG(1 | TextureModeCopy, TextureColorMode);		
	GLINT_WRITE_REG(1, TextureAddressMode);
	GLINT_WRITE_REG(maskX << 20, SStart);
	GLINT_WRITE_REG(maskY << 20, TStart);
	GLINT_WRITE_REG(PrimitiveRectangle | XPositive | YPositive | TextureEnable,
	    Render);
}

void
Pm2Comp_Op32(ScrnInfoPtr pScrn, int op, int srcX, int srcY,
				     int dstX, int dstY,
				     int width, int height)
{
	GLINTPtr pGlint = GLINTPTR(pScrn);

	ENTER;

	GLINT_WAIT(8);
	DO_PLANEMASK(0xffffffff);
	
	GLINT_WRITE_REG(
	    Pm2BlendOps[op] |
	    ABM_ColorOrderRGB | UNIT_ENABLE, AlphaBlendMode);
	GLINT_WRITE_REG(UNIT_ENABLE | DTM_ColorOrderRGB, DitherMode);
	GLINT_WRITE_REG(CFBRM_DstEnable | /*CFBRM_Packed|*/ CWM_Enable, Config);
       	Permedia2LoadCoord(pScrn, dstX, dstY, width, height);
	GLINT_WRITE_REG(pGlint->srcoff >> 2, PMTextureBaseAddress);
	GLINT_WRITE_REG(1 | 0xb << 9 | 0xb << 13 | 1 << 1 | 1 << 3, PMTextureReadMode);		
	GLINT_WRITE_REG(1 | TextureModeCopy, TextureColorMode);		
	GLINT_WRITE_REG(1, TextureAddressMode);
	GLINT_WRITE_REG(srcX << 20, SStart);
	GLINT_WRITE_REG(srcY << 20, TStart);
	GLINT_WRITE_REG(PrimitiveRectangle | XPositive | YPositive | TextureEnable,
	    Render);
}

void
Pm2Composite(PixmapPtr pDst, int srcX, int srcY,
                              int maskX, int maskY,
                              int dstX, int dstY,
                              int width, int height)
{
	ScrnInfoPtr pScrn = xf86Screens[pDst->drawable.pScreen->myNum];
	GLINTPtr pGlint   = GLINTPTR(pScrn);
	uint32_t dstoff, dstpitch;

	ENTER;
	dstoff = exaGetPixmapOffset(pDst);		
	dstpitch = exaGetPixmapPitch(pDst);
	dstY += dstoff / dstpitch;
	if (pGlint->source_is_solid) {
		switch (pGlint->op) {
			case PictOpOver:
				DPRINTF(X_ERROR, "Over %08x %08x, %d %d\n",
				    pGlint->mskformat, pGlint->dstformat, dstX, dstY);
				switch (pGlint->mskformat) {
					case PICT_a8r8g8b8:
					case PICT_a8b8g8r8:
						Pm2Comp_Over32Solid(pScrn,
						    maskX, maskY,
						    dstX, dstY,
						    width, height);
						break;
					default:
						xf86Msg(X_ERROR,
						  "unsupported mask format\n");
				}
				break;
			default:
				xf86Msg(X_ERROR, "unsupported op %d\n", pGlint->op);
		}
	} else {
		Pm2Comp_Op32(pScrn, pGlint->op, srcX, srcY, dstX, dstY, width, height);
	}
}

Bool
Pm2InitEXA(ScreenPtr pScreen)
{
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	GLINTPtr pGlint = GLINTPTR(pScrn);
	ExaDriverPtr pExa;
	int stride, lines;

	ENTER;

	pExa = exaDriverAlloc();
	if (!pExa)
		return FALSE;

	pGlint->pExa = pExa;

	pExa->exa_major = EXA_VERSION_MAJOR;
	pExa->exa_minor = EXA_VERSION_MINOR;

	pExa->memoryBase = pGlint->FbBase;
	stride = pScrn->displayWidth * (pScrn->bitsPerPixel >> 3);
	lines = min(pGlint->FbMapSize / stride, 2047);
	pExa->memorySize = lines * stride;
	pExa->offScreenBase = stride * pScrn->virtualY;
	DPRINTF(X_ERROR, "stride: %d\n", stride);
	DPRINTF(X_ERROR, "pprod: %08x\n", pGlint->pprod);
	pExa->offScreenBase = stride * pScrn->virtualY;

	/* for now, until I figure out how to do variable stride */
	pExa->pixmapOffsetAlign = stride;
	pExa->pixmapPitchAlign = stride;

	pExa->flags = EXA_OFFSCREEN_PIXMAPS;

	pExa->maxX = 2047;
	pExa->maxY = 2047;

	pExa->WaitMarker = Pm2WaitMarker;

	pExa->PrepareSolid = Pm2PrepareSolid;
	pExa->Solid = Pm2Solid;
	pExa->DoneSolid = Pm2DoneCopy;
	pExa->PrepareCopy = Pm2PrepareCopy;
	pExa->Copy = Pm2Copy;
	pExa->DoneCopy = Pm2DoneCopy;

	if (pGlint->render) {
		pExa->CheckComposite = Pm2CheckComposite;
		pExa->PrepareComposite = Pm2PrepareComposite;
		pExa->Composite = Pm2Composite;
		pExa->DoneComposite = Pm2DoneCopy;
	}

if (0) {
	pExa->UploadToScreen = Pm2UploadToScreen;
	pExa->DownloadFromScreen = Pm2DownloadFromScreen;
}
	Permedia2InitializeEngine(pScrn);

	return exaDriverInit(pScreen, pExa);
}
