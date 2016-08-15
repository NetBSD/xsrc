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

#include "xf86xv.h"
#include <X11/extensions/Xv.h>
#include "exa.h"
#include "damage.h"
#include "dixstruct.h"
#include "fourcc.h"

#include "nv_include.h"
#include "nv_dma.h"
#include "nv50_accel.h"

extern Atom xvSyncToVBlank, xvSetDefaults;
extern Atom xvBrightness, xvContrast, xvHue, xvSaturation;
extern Atom xvITURBT709;

static Bool
nv50_xv_check_image_put(PixmapPtr ppix)
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
nv50_xv_image_put(ScrnInfoPtr pScrn,
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
	struct nouveau_pushbuf *push = pNv->pushbuf;
	struct nouveau_pushbuf_refn refs[] = {
		{ pNv->scratch, NOUVEAU_BO_VRAM | NOUVEAU_BO_RDWR },
		{ src, NOUVEAU_BO_VRAM | NOUVEAU_BO_RD },
		{ dst, NOUVEAU_BO_VRAM | NOUVEAU_BO_WR },
	};
	uint32_t mode = 0xd0005000 | (src->config.nv50.tile_mode << 18);
	float X1, X2, Y1, Y2;
	BoxPtr pbox;
	int nbox;

	if (!nv50_xv_check_image_put(ppix))
		return BadMatch;

	if (!PUSH_SPACE(push, 256))
		return BadImplementation;

	BEGIN_NV04(push, NV50_3D(RT_ADDRESS_HIGH(0)), 5);
	PUSH_DATA (push, dst->offset >> 32);
	PUSH_DATA (push, dst->offset);
	switch (ppix->drawable.bitsPerPixel) {
	case 32: PUSH_DATA (push, NV50_SURFACE_FORMAT_BGRA8_UNORM); break;
	case 24: PUSH_DATA (push, NV50_SURFACE_FORMAT_BGRX8_UNORM); break;
	case 16: PUSH_DATA (push, NV50_SURFACE_FORMAT_B5G6R5_UNORM); break;
	case 15: PUSH_DATA (push, NV50_SURFACE_FORMAT_BGR5_X1_UNORM); break;
	}
	PUSH_DATA (push, dst->config.nv50.tile_mode);
	PUSH_DATA (push, 0);
	BEGIN_NV04(push, NV50_3D(RT_HORIZ(0)), 2);
	PUSH_DATA (push, ppix->drawable.width);
	PUSH_DATA (push, ppix->drawable.height);
	BEGIN_NV04(push, NV50_3D(RT_ARRAY_MODE), 1);
	PUSH_DATA (push, 1);

	BEGIN_NV04(push, NV50_3D(BLEND_ENABLE(0)), 1);
	PUSH_DATA (push, 0);

	PUSH_DATAu(push, pNv->scratch, TIC_OFFSET, 16);
	if (id == FOURCC_YV12 || id == FOURCC_I420) {
	PUSH_DATA (push, NV50TIC_0_0_MAPA_C0 | NV50TIC_0_0_TYPEA_UNORM |
			 NV50TIC_0_0_MAPB_ZERO | NV50TIC_0_0_TYPEB_UNORM |
			 NV50TIC_0_0_MAPG_ZERO | NV50TIC_0_0_TYPEG_UNORM |
			 NV50TIC_0_0_MAPR_ZERO | NV50TIC_0_0_TYPER_UNORM |
			 NV50TIC_0_0_FMT_8);
	PUSH_DATA (push, (src->offset + packed_y));
	PUSH_DATA (push, (src->offset + packed_y) >> 32 | mode);
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
	PUSH_DATA (push, (src->offset + uv));
	PUSH_DATA (push, (src->offset + uv) >> 32 | mode);
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
	PUSH_DATA (push, (src->offset + packed_y));
	PUSH_DATA (push, (src->offset + packed_y) >> 32 | mode);
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
	PUSH_DATA (push, (src->offset + packed_y));
	PUSH_DATA (push, (src->offset + packed_y) >> 32 | mode);
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

	BEGIN_NV04(push, NV50_3D(FP_START_ID), 1);
	PUSH_DATA (push, PFP_NV12);

	BEGIN_NV04(push, NV50_3D(TIC_FLUSH), 1);
	PUSH_DATA (push, 0);

	BEGIN_NV04(push, NV50_3D(BIND_TIC(2)), 1);
	PUSH_DATA (push, 1);
	BEGIN_NV04(push, NV50_3D(BIND_TIC(2)), 1);
	PUSH_DATA (push, 0x203);

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

	if (pPriv->SyncToVBlank)
		NV50SyncToVBlank(ppix, dstBox);

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

		/* NV50_3D_SCISSOR_VERT_T_SHIFT is wrong, because it was deducted with
		* origin lying at the bottom left. This will be changed to _MIN_ and _MAX_
		* later, because it is origin dependent.
		*/
		BEGIN_NV04(push, NV50_3D(SCISSOR_HORIZ(0)), 2);
		PUSH_DATA (push, sx2 << NV50_3D_SCISSOR_HORIZ_MAX__SHIFT | sx1);
		PUSH_DATA (push, sy2 << NV50_3D_SCISSOR_VERT_MAX__SHIFT | sy1 );
		BEGIN_NV04(push, NV50_3D(VERTEX_BEGIN_GL), 1);
		PUSH_DATA (push, NV50_3D_VERTEX_BEGIN_GL_PRIMITIVE_TRIANGLES);
		PUSH_VTX1s(push, tx1, ty1, sx1, sy1);
		PUSH_VTX1s(push, tx2+(tx2-tx1), ty1, sx2+(sx2-sx1), sy1);
		PUSH_VTX1s(push, tx1, ty2+(ty2-ty1), sx1, sy2+(sy2-sy1));
		BEGIN_NV04(push, NV50_3D(VERTEX_END_GL), 1);
		PUSH_DATA (push, 0);

		pbox++;
	}

	PUSH_KICK(push);
	return Success;
}

void
nv50_xv_video_stop(ScrnInfoPtr pScrn, pointer data, Bool exit)
{
}

/* Reference color space transform data */
struct REF_TRANSFORM {
    float   RefLuma;
    float   RefRCb;
    float   RefRCr;
    float   RefGCb;
    float   RefGCr;
    float   RefBCb;
    float   RefBCr;
} trans[] = {
	{ 1.1643, 0.0, 1.5960, -0.3918, -0.8129, 2.0172, 0.0 }, /* BT.601 */
	{ 1.1643, 0.0, 1.7927, -0.2132, -0.5329, 2.1124, 0.0 }  /* BT.709 */
};

#define RTFSaturation(a)   (1.0 + ((a)*1.0)/1000.0)
#define RTFBrightness(a)   (((a)*1.0)/2000.0)
#define RTFContrast(a)   (1.0 + ((a)*1.0)/1000.0)
#define RTFHue(a)   (((a)*3.1416)/1000.0)

void
nv50_xv_csc_update(ScrnInfoPtr pScrn, NVPortPrivPtr pPriv)
{
	NVPtr pNv = NVPTR(pScrn);
	struct nouveau_pushbuf *push = pNv->pushbuf;
	const float Loff = -0.0627;
	const float Coff = -0.502;
	float yco, off[3], uco[3], vco[3];
	float uvcosf, uvsinf;
	float bright, cont;
	int ref = pPriv->iturbt_709;

	cont = RTFContrast(pPriv->contrast);
	bright = RTFBrightness(pPriv->brightness);
	uvcosf = RTFSaturation(pPriv->saturation) * cos(RTFHue(pPriv->hue));
	uvsinf = RTFSaturation(pPriv->saturation) * sin(RTFHue(pPriv->hue));

	yco = trans[ref].RefLuma * cont;
	uco[0] = -trans[ref].RefRCr * uvsinf;
	uco[1] = trans[ref].RefGCb * uvcosf - trans[ref].RefGCr * uvsinf;
	uco[2] = trans[ref].RefBCb * uvcosf;
	vco[0] = trans[ref].RefRCr * uvcosf;
	vco[1] = trans[ref].RefGCb * uvsinf + trans[ref].RefGCr * uvcosf;
	vco[2] = trans[ref].RefBCb * uvsinf;
	off[0] = Loff * yco + Coff * (uco[0] + vco[0]) + bright;
	off[1] = Loff * yco + Coff * (uco[1] + vco[1]) + bright;
	off[2] = Loff * yco + Coff * (uco[2] + vco[2]) + bright;

	if (pNv->Architecture >= NV_FERMI) {
		nvc0_xv_csc_update(pNv, yco, off, uco, vco);
		return;
	}

	if (nouveau_pushbuf_space(push, 64, 0, 0) ||
	    nouveau_pushbuf_refn (push, &(struct nouveau_pushbuf_refn) {
					pNv->scratch, NOUVEAU_BO_WR |
					NOUVEAU_BO_VRAM }, 1))
		return;

	PUSH_DATAu(push, pNv->scratch, PFP_DATA, 10);
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

void
nv50_xv_set_port_defaults(ScrnInfoPtr pScrn, NVPortPrivPtr pPriv)
{
	pPriv->videoStatus	= 0;
	pPriv->grabbedByV4L	= FALSE;
	pPriv->blitter		= FALSE;
	pPriv->texture		= TRUE;
	pPriv->doubleBuffer	= FALSE;
	pPriv->SyncToVBlank	= TRUE;
	pPriv->brightness	= 0;
	pPriv->contrast		= 0;
	pPriv->saturation	= 0;
	pPriv->hue		= 0;
	pPriv->iturbt_709	= 0;
	pPriv->max_image_dim    = 8192;
}

int
nv50_xv_port_attribute_set(ScrnInfoPtr pScrn, Atom attribute,
			   INT32 value, pointer data)
{
	NVPortPrivPtr pPriv = (NVPortPrivPtr)data;

	if (attribute == xvSyncToVBlank) {
		if (value < 0 || value > 1)
			return BadValue;
		pPriv->SyncToVBlank = value;
	} else
	if (attribute == xvBrightness) {
		if (value < -1000 || value > 1000)
			return BadValue;
		pPriv->brightness = value;
	} else
	if (attribute == xvContrast) {
		if (value < -1000 || value > 1000)
			return BadValue;
		pPriv->contrast = value;
	} else
	if (attribute == xvSaturation) {
		if (value < -1000 || value > 1000)
			return BadValue;
		pPriv->saturation = value;
	} else
	if (attribute == xvHue) {
		if (value < -1000 || value > 1000)
			return BadValue;
		pPriv->hue = value;
	} else
	if (attribute == xvITURBT709) {
		if (value < 0 || value > 1)
			return BadValue;
		pPriv->iturbt_709 = value;
	} else
	if (attribute == xvSetDefaults) {
		nv50_xv_set_port_defaults(pScrn, pPriv);
	} else
		return BadMatch;

	nv50_xv_csc_update(pScrn, pPriv);
	return Success;
}

int
nv50_xv_port_attribute_get(ScrnInfoPtr pScrn, Atom attribute,
			   INT32 *value, pointer data)
{
	NVPortPrivPtr pPriv = (NVPortPrivPtr)data;

	if (attribute == xvSyncToVBlank)
		*value = (pPriv->SyncToVBlank) ? 1 : 0;
	else
	if (attribute == xvBrightness)
		*value = pPriv->brightness;
	else
	if (attribute == xvContrast)
		*value = pPriv->contrast;
	else
	if (attribute == xvSaturation)
		*value = pPriv->saturation;
	else
	if (attribute == xvHue)
		*value = pPriv->hue;
	else
	if (attribute == xvITURBT709)
		*value = pPriv->iturbt_709;
	else
		return BadMatch;

	return Success;
}


