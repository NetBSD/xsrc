/*
 * Copyright 2008 Ben Skeggs
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
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xorg-config.h"
#include "xf86xv.h"
#include <X11/extensions/Xv.h>
#include "exa.h"
#include "damage.h"
#include "dixstruct.h"
#include "fourcc.h"

#include "nv_include.h"
#include "nvc0_accel.h"

extern Atom xvSyncToVBlank, xvSetDefaults;

static Bool
nvc0_xv_check_image_put(PixmapPtr ppix)
{
	switch (ppix->drawable.bitsPerPixel) {
	case 32:
	case 24:
	case 16:
	case 15:
		break;
	default:
		return FALSE;
	}

	if (!nv50_style_tiled_pixmap(ppix))
		return FALSE;

	return TRUE;
}

int
nvc0_xv_image_put(ScrnInfoPtr pScrn,
		  struct nouveau_bo *src, int packed_y, int uv,
		  int id, int src_pitch, BoxPtr dstBox,
		  int x1, int y1, int x2, int y2,
		  uint16_t width, uint16_t height,
		  uint16_t src_w, uint16_t src_h,
		  uint16_t drw_w, uint16_t drw_h,
		  RegionPtr clipBoxes, PixmapPtr ppix,
		  NVPortPrivPtr pPriv)
{
	NVPtr pNv = NVPTR(pScrn);
	struct nouveau_bo *dst = nouveau_pixmap_bo(ppix);
	struct nouveau_pushbuf_refn refs[] = {
		{ pNv->scratch, NOUVEAU_BO_VRAM | NOUVEAU_BO_RDWR },
		{ src, NOUVEAU_BO_VRAM | NOUVEAU_BO_RD },
		{ dst, NOUVEAU_BO_VRAM | NOUVEAU_BO_WR },
	};
	struct nouveau_pushbuf *push = pNv->pushbuf;
	uint32_t mode = 0xd0005000 | (src->config.nvc0.tile_mode << 18);
	float X1, X2, Y1, Y2;
	BoxPtr pbox;
	int nbox;

	if (!nvc0_xv_check_image_put(ppix))
		return BadMatch;

	if (!PUSH_SPACE(push, 256))
		return BadImplementation;

	BEGIN_NVC0(push, NVC0_3D(RT_ADDRESS_HIGH(0)), 8);
	PUSH_DATA (push, dst->offset >> 32);
	PUSH_DATA (push, dst->offset);
	PUSH_DATA (push, ppix->drawable.width);
	PUSH_DATA (push, ppix->drawable.height);
	switch (ppix->drawable.bitsPerPixel) {
	case 32: PUSH_DATA (push, NV50_SURFACE_FORMAT_BGRA8_UNORM); break;
	case 24: PUSH_DATA (push, NV50_SURFACE_FORMAT_BGRX8_UNORM); break;
	case 16: PUSH_DATA (push, NV50_SURFACE_FORMAT_B5G6R5_UNORM); break;
	case 15: PUSH_DATA (push, NV50_SURFACE_FORMAT_BGR5_X1_UNORM); break;
	}
	PUSH_DATA (push, dst->config.nvc0.tile_mode);
	PUSH_DATA (push, 1);
	PUSH_DATA (push, 0);

	BEGIN_NVC0(push, NVC0_3D(BLEND_ENABLE(0)), 1);
	PUSH_DATA (push, 0);

	PUSH_DATAu(push, pNv->scratch, TIC_OFFSET, 16);
	if (id == FOURCC_YV12 || id == FOURCC_I420) {
	PUSH_DATA (push, NV50TIC_0_0_MAPA_C0 | NV50TIC_0_0_TYPEA_UNORM |
			 NV50TIC_0_0_MAPB_ZERO | NV50TIC_0_0_TYPEB_UNORM |
			 NV50TIC_0_0_MAPG_ZERO | NV50TIC_0_0_TYPEG_UNORM |
			 NV50TIC_0_0_MAPR_ZERO | NV50TIC_0_0_TYPER_UNORM |
			 NV50TIC_0_0_FMT_8);
	PUSH_DATA (push, ((src->offset + packed_y)));
	PUSH_DATA (push, ((src->offset + packed_y) >> 32) | mode);
	PUSH_DATA (push, 0x00300000);
	PUSH_DATA (push, width);
	PUSH_DATA (push, (1 << NV50TIC_0_5_DEPTH_SHIFT) | height);
	PUSH_DATA (push, 0x03000000);
	PUSH_DATA (push, 0x00000000);
	PUSH_DATA (push, NV50TIC_0_0_MAPA_C1 | NV50TIC_0_0_TYPEA_UNORM |
			 NV50TIC_0_0_MAPB_C0 | NV50TIC_0_0_TYPEB_UNORM |
			 NV50TIC_0_0_MAPG_ZERO | NV50TIC_0_0_TYPEG_UNORM |
			 NV50TIC_0_0_MAPR_ZERO | NV50TIC_0_0_TYPER_UNORM |
			 NV50TIC_0_0_FMT_8_8);
	PUSH_DATA (push, ((src->offset + uv)));
	PUSH_DATA (push, ((src->offset + uv) >> 32) | mode);
	PUSH_DATA (push, 0x00300000);
	PUSH_DATA (push, width >> 1);
	PUSH_DATA (push, (1 << NV50TIC_0_5_DEPTH_SHIFT) | (height >> 1));
	PUSH_DATA (push, 0x03000000);
	PUSH_DATA (push, 0x00000000);
	} else {
	if (id == FOURCC_UYVY) {
	PUSH_DATA (push, NV50TIC_0_0_MAPA_C1 | NV50TIC_0_0_TYPEA_UNORM |
			 NV50TIC_0_0_MAPB_ZERO | NV50TIC_0_0_TYPEB_UNORM |
			 NV50TIC_0_0_MAPG_ZERO | NV50TIC_0_0_TYPEG_UNORM |
			 NV50TIC_0_0_MAPR_ZERO | NV50TIC_0_0_TYPER_UNORM |
			 NV50TIC_0_0_FMT_8_8);
	} else {
	PUSH_DATA (push, NV50TIC_0_0_MAPA_C0 | NV50TIC_0_0_TYPEA_UNORM |
			 NV50TIC_0_0_MAPB_ZERO | NV50TIC_0_0_TYPEB_UNORM |
			 NV50TIC_0_0_MAPG_ZERO | NV50TIC_0_0_TYPEG_UNORM |
			 NV50TIC_0_0_MAPR_ZERO | NV50TIC_0_0_TYPER_UNORM |
			 NV50TIC_0_0_FMT_8_8);
	}
	PUSH_DATA (push, ((src->offset + packed_y)));
	PUSH_DATA (push, ((src->offset + packed_y) >> 32) | mode);
	PUSH_DATA (push, 0x00300000);
	PUSH_DATA (push, width);
	PUSH_DATA (push, (1 << NV50TIC_0_5_DEPTH_SHIFT) | height);
	PUSH_DATA (push, 0x03000000);
	PUSH_DATA (push, 0x00000000);
	if (id == FOURCC_UYVY) {
	PUSH_DATA (push, NV50TIC_0_0_MAPA_C2 | NV50TIC_0_0_TYPEA_UNORM |
			 NV50TIC_0_0_MAPB_C0 | NV50TIC_0_0_TYPEB_UNORM |
			 NV50TIC_0_0_MAPG_ZERO | NV50TIC_0_0_TYPEG_UNORM |
			 NV50TIC_0_0_MAPR_ZERO | NV50TIC_0_0_TYPER_UNORM |
			 NV50TIC_0_0_FMT_8_8_8_8);
	} else {
	PUSH_DATA (push, NV50TIC_0_0_MAPA_C3 | NV50TIC_0_0_TYPEA_UNORM |
			 NV50TIC_0_0_MAPB_C1 | NV50TIC_0_0_TYPEB_UNORM |
			 NV50TIC_0_0_MAPG_ZERO | NV50TIC_0_0_TYPEG_UNORM |
			 NV50TIC_0_0_MAPR_ZERO | NV50TIC_0_0_TYPER_UNORM |
			 NV50TIC_0_0_FMT_8_8_8_8);
	}
	PUSH_DATA (push, ((src->offset + packed_y)));
	PUSH_DATA (push, ((src->offset + packed_y) >> 32) | mode);
	PUSH_DATA (push, 0x00300000);
	PUSH_DATA (push, (width >> 1));
	PUSH_DATA (push, (1 << NV50TIC_0_5_DEPTH_SHIFT) | height);
	PUSH_DATA (push, 0x03000000);
	PUSH_DATA (push, 0x00000000);
	}

	PUSH_DATAu(push, pNv->scratch, TSC_OFFSET, 16);
	PUSH_DATA (push, NV50TSC_1_0_WRAPS_CLAMP_TO_EDGE |
			 NV50TSC_1_0_WRAPT_CLAMP_TO_EDGE |
			 NV50TSC_1_0_WRAPR_CLAMP_TO_EDGE);
	PUSH_DATA (push, NV50TSC_1_1_MAGF_LINEAR |
			 NV50TSC_1_1_MINF_LINEAR |
			 NV50TSC_1_1_MIPF_NONE);
	PUSH_DATA (push, 0x00000000);
	PUSH_DATA (push, 0x00000000);
	PUSH_DATA (push, 0x00000000);
	PUSH_DATA (push, 0x00000000);
	PUSH_DATA (push, 0x00000000);
	PUSH_DATA (push, 0x00000000);
	PUSH_DATA (push, NV50TSC_1_0_WRAPS_CLAMP_TO_EDGE |
			 NV50TSC_1_0_WRAPT_CLAMP_TO_EDGE |
			 NV50TSC_1_0_WRAPR_CLAMP_TO_EDGE);
	PUSH_DATA (push, NV50TSC_1_1_MAGF_LINEAR |
			 NV50TSC_1_1_MINF_LINEAR |
			 NV50TSC_1_1_MIPF_NONE);
	PUSH_DATA (push, 0x00000000);
	PUSH_DATA (push, 0x00000000);
	PUSH_DATA (push, 0x00000000);
	PUSH_DATA (push, 0x00000000);
	PUSH_DATA (push, 0x00000000);
	PUSH_DATA (push, 0x00000000);

	BEGIN_NVC0(push, NVC0_3D(SP_START_ID(5)), 1);
	PUSH_DATA (push, PFP_NV12);

	BEGIN_NVC0(push, NVC0_3D(TSC_FLUSH), 1);
	PUSH_DATA (push, 0);
	BEGIN_NVC0(push, NVC0_3D(TIC_FLUSH), 1);
	PUSH_DATA (push, 0);
	BEGIN_NVC0(push, NVC0_3D(TEX_CACHE_CTL), 1);
	PUSH_DATA (push, 0);

	PUSH_DATAu(push, pNv->scratch, PVP_DATA, 11);
	PUSH_DATAf(push, 1.0);
	PUSH_DATAf(push, 0.0);
	PUSH_DATAf(push, 0.0);
	PUSH_DATAf(push, 0.0);
	PUSH_DATAf(push, 1.0);
	PUSH_DATAf(push, 0.0);
	PUSH_DATAf(push, 0.0);
	PUSH_DATAf(push, 0.0);
	PUSH_DATAf(push, 1.0);
	PUSH_DATAf(push, 1.0 / width);
	PUSH_DATAf(push, 1.0 / height);

	if (pPriv->SyncToVBlank) {
		NVC0SyncToVBlank(ppix, dstBox);
	}

	/* These are fixed point values in the 16.16 format. */
	X1 = (float)(x1>>16)+(float)(x1&0xFFFF)/(float)0x10000;
	Y1 = (float)(y1>>16)+(float)(y1&0xFFFF)/(float)0x10000;
	X2 = (float)(x2>>16)+(float)(x2&0xFFFF)/(float)0x10000;
	Y2 = (float)(y2>>16)+(float)(y2&0xFFFF)/(float)0x10000;

	pbox = REGION_RECTS(clipBoxes);
	nbox = REGION_NUM_RECTS(clipBoxes);
	while(nbox--) {
		float tx1=X1+(float)(pbox->x1 - dstBox->x1)*(X2-X1)/(float)(drw_w);
		float tx2=X1+(float)(pbox->x2 - dstBox->x1)*(src_w)/(float)(drw_w);
		float ty1=Y1+(float)(pbox->y1 - dstBox->y1)*(Y2-Y1)/(float)(drw_h);
		float ty2=Y1+(float)(pbox->y2 - dstBox->y1)*(src_h)/(float)(drw_h);
		int sx1=pbox->x1;
		int sx2=pbox->x2;
		int sy1=pbox->y1;
		int sy2=pbox->y2;

		if (nouveau_pushbuf_space(push, 64, 0, 0) ||
		    nouveau_pushbuf_refn (push, refs, 3))
			return BadImplementation;

		BEGIN_NVC0(push, NVC0_3D(SCISSOR_HORIZ(0)), 2);
		PUSH_DATA (push, sx2 << NVC0_3D_SCISSOR_HORIZ_MAX__SHIFT | sx1);
		PUSH_DATA (push, sy2 << NVC0_3D_SCISSOR_VERT_MAX__SHIFT | sy1 );

		BEGIN_NVC0(push, NVC0_3D(VERTEX_BEGIN_GL), 1);
		PUSH_DATA (push, NVC0_3D_VERTEX_BEGIN_GL_PRIMITIVE_TRIANGLES);
		PUSH_VTX1s(push, tx1, ty1, sx1, sy1);
		PUSH_VTX1s(push, tx2+(tx2-tx1), ty1, sx2+(sx2-sx1), sy1);
		PUSH_VTX1s(push, tx1, ty2+(ty2-ty1), sx1, sy2+(sy2-sy1));
		BEGIN_NVC0(push, NVC0_3D(VERTEX_END_GL), 1);
		PUSH_DATA (push, 0);

		pbox++;
	}

	PUSH_KICK(push);
	return Success;
}

void
nvc0_xv_csc_update(NVPtr pNv, float yco, float *off, float *uco, float *vco)
{
	struct nouveau_pushbuf *push = pNv->pushbuf;

	if (nouveau_pushbuf_space(push, 64, 0, 0) ||
	    nouveau_pushbuf_refn (push, &(struct nouveau_pushbuf_refn) {
					pNv->scratch, NOUVEAU_BO_WR |
					NOUVEAU_BO_VRAM }, 1))
		return;

	BEGIN_NVC0(push, NVC0_3D(CB_SIZE), 3);
	PUSH_DATA (push, 256);
	PUSH_DATA (push, (pNv->scratch->offset + PFP_DATA) >> 32);
	PUSH_DATA (push, (pNv->scratch->offset + PFP_DATA));
	BEGIN_NVC0(push, NVC0_3D(CB_POS), 11);
	PUSH_DATA (push, 0);
	PUSH_DATAf(push, yco);
	PUSH_DATAf(push, off[0]);
	PUSH_DATAf(push, off[1]);
	PUSH_DATAf(push, off[2]);
	PUSH_DATAf(push, uco[0]);
	PUSH_DATAf(push, uco[1]);
	PUSH_DATAf(push, uco[2]);
	PUSH_DATAf(push, vco[0]);
	PUSH_DATAf(push, vco[1]);
	PUSH_DATAf(push, vco[2]);
}
