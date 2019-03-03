/*
 * Copyright 2007-2008 Maarten Maathuis
 * Copyright 2008 Stephane Marchesin
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86xv.h"
#include <X11/extensions/Xv.h>
#include "exa.h"
#include "damage.h"
#include "dixstruct.h"
#include "fourcc.h"

#include "nv_include.h"
#include "nv_dma.h"

#include "hwdefs/nv30-40_3d.xml.h"
#include "nv04_accel.h"

extern Atom xvSyncToVBlank, xvSetDefaults;

#define SWIZZLE(ts0x,ts0y,ts0z,ts0w,ts1x,ts1y,ts1z,ts1w)			\
	(									\
	NV30_3D_TEX_SWIZZLE_S0_X_##ts0x | NV30_3D_TEX_SWIZZLE_S0_Y_##ts0y	|	\
	NV30_3D_TEX_SWIZZLE_S0_Z_##ts0z | NV30_3D_TEX_SWIZZLE_S0_W_##ts0w	|	\
	NV30_3D_TEX_SWIZZLE_S1_X_##ts1x | NV30_3D_TEX_SWIZZLE_S1_Y_##ts1y |	\
	NV30_3D_TEX_SWIZZLE_S1_Z_##ts1z | NV30_3D_TEX_SWIZZLE_S1_W_##ts1w		\
	)

/*
 * Texture 0 : filter table
 * Texture 1 : Y data
 * Texture 2 : UV data
 */
static Bool
NV30VideoTexture(ScrnInfoPtr pScrn, struct nouveau_bo *src, int offset,
		 uint16_t width, uint16_t height, uint16_t src_pitch, int unit)
{
	NVPtr pNv = NVPTR(pScrn);
	unsigned reloc = NOUVEAU_BO_VRAM | NOUVEAU_BO_GART | NOUVEAU_BO_RD;
	struct nouveau_pushbuf *push = pNv->pushbuf;
	uint32_t card_fmt = 0;
	uint32_t card_swz = 0;
	int h = log2i(height);
	int w = log2i(width);

	switch (unit) {
	case 0:
		card_fmt = NV30_3D_TEX_FORMAT_FORMAT_A8R8G8B8;
		card_swz = SWIZZLE(S1, S1, S1, S1, X, Y, Z, W);
		break;
	case 1:
		card_fmt = NV30_3D_TEX_FORMAT_FORMAT_I8_RECT;
		card_swz = SWIZZLE(S1, S1, S1, S1, X, X, X, X);
		break;
	case 2:
		card_fmt = NV30_3D_TEX_FORMAT_FORMAT_A8L8_RECT;
#if X_BYTE_ORDER == X_BIG_ENDIAN
		card_swz = SWIZZLE(S1, S1, S1, S1, Z, W, X, Y);
#else
		card_swz = SWIZZLE(S1, S1, S1, S1, W, Z, Y, X);
#endif
		break;
	}

	BEGIN_NV04(push, NV30_3D(TEX_OFFSET(unit)), 8);
	PUSH_MTHDl(push, NV30_3D(TEX_OFFSET(unit)), src, offset, reloc);
	if (unit == 0) {
		PUSH_MTHD (push, NV30_3D(TEX_FORMAT(unit)), src,
				 NV30_3D_TEX_FORMAT_DIMS_1D |
				 card_fmt | (1 << 16) | 8 /* no idea */ |
				 (w << NV30_3D_TEX_FORMAT_BASE_SIZE_U__SHIFT) |
				 (h << NV30_3D_TEX_FORMAT_BASE_SIZE_V__SHIFT),
				 reloc | NOUVEAU_BO_OR,
				 NV30_3D_TEX_FORMAT_DMA0,
				 NV30_3D_TEX_FORMAT_DMA1);
		PUSH_DATA (push, NV30_3D_TEX_WRAP_S_REPEAT |
				 NV30_3D_TEX_WRAP_T_CLAMP_TO_EDGE |
				 NV30_3D_TEX_WRAP_R_CLAMP_TO_EDGE);
	} else {
		PUSH_MTHD (push, NV30_3D(TEX_FORMAT(unit)), src,
				 NV30_3D_TEX_FORMAT_DIMS_2D |
				 card_fmt | (1 << 16) | 8 /* no idea */ |
				 (w << NV30_3D_TEX_FORMAT_BASE_SIZE_U__SHIFT) |
				 (h << NV30_3D_TEX_FORMAT_BASE_SIZE_V__SHIFT),
				 reloc | NOUVEAU_BO_OR,
				 NV30_3D_TEX_FORMAT_DMA0,
				 NV30_3D_TEX_FORMAT_DMA1);
		PUSH_DATA (push, NV30_3D_TEX_WRAP_S_CLAMP_TO_EDGE |
				 NV30_3D_TEX_WRAP_T_CLAMP_TO_EDGE |
				 NV30_3D_TEX_WRAP_R_CLAMP_TO_EDGE);
	}

	PUSH_DATA (push, NV30_3D_TEX_ENABLE_ENABLE);
	PUSH_DATA (push, (src_pitch << NV30_3D_TEX_SWIZZLE_RECT_PITCH__SHIFT) |
			 card_swz);
	if (unit == 0)
		PUSH_DATA (push, NV30_3D_TEX_FILTER_SIGNED_ALPHA |
				 NV30_3D_TEX_FILTER_SIGNED_RED |
				 NV30_3D_TEX_FILTER_SIGNED_GREEN |
				 NV30_3D_TEX_FILTER_SIGNED_BLUE |
				 NV30_3D_TEX_FILTER_MIN_LINEAR |
				 NV30_3D_TEX_FILTER_MAG_LINEAR | 0x2000);
	else
		PUSH_DATA (push, NV30_3D_TEX_FILTER_MIN_LINEAR |
				 NV30_3D_TEX_FILTER_MAG_LINEAR | 0x2000);
	PUSH_DATA (push, (width << NV30_3D_TEX_NPOT_SIZE_W__SHIFT) | height);
	PUSH_DATA (push, 0); /* border ARGB */
	BEGIN_NV04(push, NV30_3D(TEX_MATRIX_ENABLE(unit)), 1);
	PUSH_DATA (push, 0);

	return TRUE;
}

static Bool
NV30GetSurfaceFormat(PixmapPtr ppix, int *fmt_ret)
{
	switch (ppix->drawable.bitsPerPixel) {
		case 32:
			*fmt_ret = NV30_3D_RT_FORMAT_COLOR_A8R8G8B8;
			break;
		case 24:
			*fmt_ret = NV30_3D_RT_FORMAT_COLOR_X8R8G8B8;
			break;
		case 16:
			*fmt_ret = NV30_3D_RT_FORMAT_COLOR_R5G6B5;
			break;
		case 8:
			*fmt_ret = NV30_3D_RT_FORMAT_COLOR_B8;
			break;
		default:
			return FALSE;
	}

	return TRUE;
}

void
NV30StopTexturedVideo(ScrnInfoPtr pScrn, pointer data, Bool Exit)
{
}

#define VERTEX_OUT(sx,sy,dx,dy) do {                                           \
	BEGIN_NV04(push, NV30_3D(VTX_ATTR_2F_X(8)), 4);                        \
	PUSH_DATAf(push, (sx)); PUSH_DATAf(push, (sy));                        \
	PUSH_DATAf(push, (sx)/2.0); PUSH_DATAf(push, (sy)/2.0);                \
	BEGIN_NV04(push, NV30_3D(VTX_ATTR_2I(0)), 1);                          \
	PUSH_DATA (push, (((dy)&0xffff)<<16)|((dx)&0xffff));                   \
} while(0)

int
NV30PutTextureImage(ScrnInfoPtr pScrn, struct nouveau_bo *src, int src_offset,
		    int src_offset2, int id, int src_pitch, BoxPtr dstBox,
		    int x1, int y1, int x2, int y2,
		    uint16_t width, uint16_t height,
		    uint16_t src_w, uint16_t src_h,
		    uint16_t drw_w, uint16_t drw_h,
		    RegionPtr clipBoxes,
		    PixmapPtr ppix,
		    NVPortPrivPtr pPriv)
{
	NVPtr pNv = NVPTR(pScrn);
	struct nouveau_pushbuf *push = pNv->pushbuf;
	struct nouveau_bo *bo = nouveau_pixmap_bo(ppix);
	Bool bicubic = pPriv->bicubic;
	float X1, X2, Y1, Y2;
	BoxPtr pbox;
	int nbox;
	int dst_format = 0;

	if (drw_w > 4096 || drw_h > 4096) {
		xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
			"XV: Draw size too large.\n");
		return BadAlloc;
	}

	if (!NV30GetSurfaceFormat(ppix, &dst_format)) {
		ErrorF("No surface format, bad.\n");
		return BadImplementation;
	}

	pbox = REGION_RECTS(clipBoxes);
	nbox = REGION_NUM_RECTS(clipBoxes);

	if (!PUSH_SPACE(push, 128))
		return FALSE;
	PUSH_RESET(push);

	BEGIN_NV04(push, NV30_3D(BLEND_FUNC_ENABLE), 1);
	PUSH_DATA (push, 0);
	BEGIN_NV04(push, NV30_3D(RT_FORMAT), 3);
	PUSH_DATA (push, NV30_3D_RT_FORMAT_TYPE_LINEAR |
			 NV30_3D_RT_FORMAT_ZETA_Z24S8 |
			 dst_format);
	PUSH_DATA (push, (exaGetPixmapPitch(ppix) << 16) |
			  exaGetPixmapPitch(ppix));
	PUSH_MTHDl(push, NV30_3D(COLOR0_OFFSET), bo, 0,
			 NOUVEAU_BO_VRAM | NOUVEAU_BO_WR);

	if (pNv->dev->chipset == 0x30) {
		int x = 0;
		int y = 0;
		int w = ppix->drawable.x + ppix->drawable.width;
		int h = ppix->drawable.y + ppix->drawable.height;

		BEGIN_NV04(push, NV30_3D(VIEWPORT_HORIZ), 2);
		PUSH_DATA (push, (w<<16)|x);
		PUSH_DATA (push, (h<<16)|y);
		BEGIN_NV04(push, NV30_3D(VIEWPORT_CLIP_HORIZ(0)), 2);
		PUSH_DATA (push, (w-1+x)<<16);
		PUSH_DATA (push, (h-1+y)<<16);
		BEGIN_NV04(push, NV30_3D(VIEWPORT_TX_ORIGIN), 1);
		PUSH_DATA (push, (y<<16)|x);
	}

	BEGIN_NV04(push, NV30_3D(TEX_UNITS_ENABLE), 1);
	PUSH_DATA (push, NV30_3D_TEX_UNITS_ENABLE_TX0 |
			 NV30_3D_TEX_UNITS_ENABLE_TX1);

	if (!NV30VideoTexture(pScrn, pNv->scratch, XV_TABLE, XV_TABLE_SIZE,
				     1, 0, 0) ||
	    !NV30VideoTexture(pScrn, src, src_offset, src_w, src_h,
		    	      src_pitch, 1))
		return BadImplementation;

	/* We've got NV12 format, which means half width and half height
	 * texture of chroma channels.
	 */
	if (!NV30VideoTexture(pScrn, src, src_offset2, src_w/2, src_h/2,
			      src_pitch, 2)) {
		PUSH_RESET(push);
		return BadImplementation;
	}

	BEGIN_NV04(push, NV30_3D(TEX_ENABLE(3)), 1);
	PUSH_DATA (push, 0x0);

	if (drw_w / 2 < src_w || drw_h / 2 < src_h)
		bicubic = FALSE;

	BEGIN_NV04(push, NV30_3D(FP_ACTIVE_PROGRAM), 1);
	PUSH_MTHD (push, NV30_3D(FP_ACTIVE_PROGRAM), pNv->scratch,
			 bicubic ? PFP_NV12_BICUBIC : PFP_NV12_BILINEAR,
			 NOUVEAU_BO_VRAM | NOUVEAU_BO_RD | NOUVEAU_BO_LOW |
			 NOUVEAU_BO_OR,
			 NV30_3D_FP_ACTIVE_PROGRAM_DMA0,
			 NV30_3D_FP_ACTIVE_PROGRAM_DMA1);
	BEGIN_NV04(push, NV30_3D(FP_REG_CONTROL), 1);
	PUSH_DATA (push, 0x0001000f);
	BEGIN_NV04(push, NV30_3D(FP_CONTROL), 1);
	PUSH_DATA (push, 0x00000001);
	BEGIN_NV04(push, NV30_3D(RC_ENABLE), 1);
	PUSH_DATA (push, 0x00000000);

	nouveau_pushbuf_bufctx(push, pNv->bufctx);
	if (nouveau_pushbuf_validate(push)) {
		nouveau_pushbuf_bufctx(push, NULL);
		return BadAlloc;
	}

	/* Before rendering we wait for vblank in the non-composited case. */
	if (pPriv->SyncToVBlank)
		NV11SyncToVBlank(ppix, dstBox);

	/* These are fixed point values in the 16.16 format. */
	X1 = (float)(x1>>16)+(float)(x1&0xFFFF)/(float)0x10000;
	Y1 = (float)(y1>>16)+(float)(y1&0xFFFF)/(float)0x10000;
	X2 = (float)(x2>>16)+(float)(x2&0xFFFF)/(float)0x10000;
	Y2 = (float)(y2>>16)+(float)(y2&0xFFFF)/(float)0x10000;

	BEGIN_NV04(push, NV30_3D(VERTEX_BEGIN_END), 1);
	PUSH_DATA (push, NV30_3D_VERTEX_BEGIN_END_TRIANGLES);
	while (nbox--) {
		float tx1=X1+(float)(pbox->x1 - dstBox->x1)*(X2-X1)/(float)(drw_w);
		float tx2=X1+(float)(pbox->x2 - dstBox->x1)*(src_w)/(float)(drw_w);
		float ty1=Y1+(float)(pbox->y1 - dstBox->y1)*(Y2-Y1)/(float)(drw_h);
		float ty2=Y1+(float)(pbox->y2 - dstBox->y1)*(src_h)/(float)(drw_h);
		int sx1=pbox->x1;
		int sx2=pbox->x2;
		int sy1=pbox->y1;
		int sy2=pbox->y2;

		if (!PUSH_SPACE(push, 64)) {
			nouveau_pushbuf_bufctx(push, NULL);
			return BadImplementation;
		}

		BEGIN_NV04(push, NV30_3D(SCISSOR_HORIZ), 2);
		PUSH_DATA (push, (sx2 << 16) | 0);
		PUSH_DATA (push, (sy2 << 16) | 0);

		VERTEX_OUT(tx1, ty1, sx1, sy1);
		VERTEX_OUT(tx2+(tx2-tx1), ty1, sx2+(sx2-sx1), sy1);
		VERTEX_OUT(tx1, ty2+(ty2-ty1), sx1, sy2+(sy2-sy1));

		pbox++;
	}
	BEGIN_NV04(push, NV30_3D(VERTEX_BEGIN_END), 1);
	PUSH_DATA (push, NV30_3D_VERTEX_BEGIN_END_STOP);

	if (pNv->dev->chipset == 0x30) {
		BEGIN_NV04(push, NV30_3D(VIEWPORT_HORIZ), 2);
		PUSH_DATA (push, 4096 << 16);
		PUSH_DATA (push, 4096 << 16);
		BEGIN_NV04(push, NV30_3D(VIEWPORT_CLIP_HORIZ(0)), 2);
		PUSH_DATA (push, 4095 << 16);
		PUSH_DATA (push, 4095 << 16);
		BEGIN_NV04(push, NV30_3D(VIEWPORT_TX_ORIGIN), 1);
		PUSH_DATA (push, 0);
	}

	nouveau_pushbuf_bufctx(push, NULL);
	PUSH_KICK(push);
	return Success;
}

/**
 * NV30SetTexturePortAttribute
 * sets the attribute "attribute" of port "data" to value "value"
 * supported attributes:
 * Sync to vblank.
 * 
 * @param pScrenInfo
 * @param attribute attribute to set
 * @param value value to which attribute is to be set
 * @param data port from which the attribute is to be set
 * 
 * @return Success, if setting is successful
 * BadValue/BadMatch, if value/attribute are invalid
 */
int
NV30SetTexturePortAttribute(ScrnInfoPtr pScrn, Atom attribute,
                       INT32 value, pointer data)
{
        NVPortPrivPtr pPriv = (NVPortPrivPtr)data;

        if (attribute == xvSyncToVBlank) {
                if ((value < 0) || (value > 1))
                        return BadValue;
                pPriv->SyncToVBlank = value;
        } else
        if (attribute == xvSetDefaults) {
                pPriv->SyncToVBlank = TRUE;
        } else
                return BadMatch;

        return Success;
}

/**
 * NV30GetTexturePortAttribute
 * reads the value of attribute "attribute" from port "data" into INT32 "*value"
 * Sync to vblank.
 * 
 * @param pScrn unused
 * @param attribute attribute to be read
 * @param value value of attribute will be stored here
 * @param data port from which attribute will be read
 * @return Success, if queried attribute exists
 */
int
NV30GetTexturePortAttribute(ScrnInfoPtr pScrn, Atom attribute,
                       INT32 *value, pointer data)
{
        NVPortPrivPtr pPriv = (NVPortPrivPtr)data;

        if(attribute == xvSyncToVBlank)
                *value = (pPriv->SyncToVBlank) ? 1 : 0;
        else
                return BadMatch;

        return Success;
}

