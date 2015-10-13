/*
 * Copyright 2007 Arthur Huillet
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

#include "hwdefs/nv_object.xml.h"
#include "hwdefs/nv01_2d.xml.h"
#include "nv04_accel.h"

#define FOURCC_RGB 0x0000003

#define VSYNC_POSSIBLE (pNv->dev->chipset >= 0x11)

extern Atom xvSetDefaults, xvSyncToVBlank;

Bool
NVPutBlitImage(ScrnInfoPtr pScrn, struct nouveau_bo *src, int src_offset,
	       int id, int src_pitch, BoxPtr dstBox,
	       int x1, int y1, int x2, int y2,
	       short width, short height,
	       short src_w, short src_h,
	       short drw_w, short drw_h,
	       RegionPtr clipBoxes, PixmapPtr ppix)
{
	NVPtr	  pNv   = NVPTR(pScrn);
	NVPortPrivPtr  pPriv = GET_BLIT_PRIVATE(pNv);
	BoxPtr	 pbox;
	int	    nbox;
	CARD32	 dsdx, dtdy;
	CARD32	 dst_size, dst_point;
	CARD32	 src_point, src_format;
	struct nouveau_pushbuf *push = pNv->pushbuf;
	struct nouveau_bo *bo = nouveau_pixmap_bo(ppix);
	struct nv04_fifo *fifo = pNv->channel->data;
	int dst_format;

	if (!NVAccelGetCtxSurf2DFormatFromPixmap(ppix, &dst_format))
		return BadImplementation;

	pbox = REGION_RECTS(clipBoxes);
	nbox = REGION_NUM_RECTS(clipBoxes);

	dsdx = (src_w << 20) / drw_w;
	dtdy = (src_h << 20) / drw_h;

	dst_size  = ((dstBox->y2 - dstBox->y1) << 16) |
		     (dstBox->x2 - dstBox->x1);
	dst_point = (dstBox->y1 << 16) | dstBox->x1;

	src_point = ((y1 << 4) & 0xffff0000) | (x1 >> 12);

	switch(id) {
	case FOURCC_RGB:
		src_format = NV03_SIFM_COLOR_FORMAT_X8R8G8B8;
		break;
	case FOURCC_UYVY:
		src_format = NV03_SIFM_COLOR_FORMAT_YB8V8YA8U8;
		break;
	default:
		src_format = NV03_SIFM_COLOR_FORMAT_V8YB8U8YA8;
		break;
	}

	if (!PUSH_SPACE(push, 128))
		return BadImplementation;
	PUSH_RESET(push);

	BEGIN_NV04(push, NV04_SF2D(FORMAT), 4);
	PUSH_DATA (push, dst_format);
	PUSH_DATA (push, (exaGetPixmapPitch(ppix) << 16) |
			  exaGetPixmapPitch(ppix));
	PUSH_MTHDl(push, NV04_SF2D(OFFSET_SOURCE), bo, 0,
			 NOUVEAU_BO_VRAM | NOUVEAU_BO_WR);
	PUSH_MTHDl(push, NV04_SF2D(OFFSET_DESTIN), bo, 0,
			 NOUVEAU_BO_VRAM | NOUVEAU_BO_WR);

	BEGIN_NV04(push, NV01_SUBC(MISC, OBJECT), 1);
	PUSH_DATA (push, pNv->NvScaledImage->handle);
	BEGIN_NV04(push, NV03_SIFM(DMA_IMAGE), 1);
	PUSH_MTHDo(push, NV03_SIFM(DMA_IMAGE), src, NOUVEAU_BO_RD |
			 NOUVEAU_BO_VRAM | NOUVEAU_BO_GART,
			 fifo->vram, fifo->gart);
	if (pNv->dev->chipset >= 0x05) {
		BEGIN_NV04(push, NV03_SIFM(COLOR_FORMAT), 2);
		PUSH_DATA (push, src_format);
		PUSH_DATA (push, NV03_SIFM_OPERATION_SRCCOPY);
	} else {
		BEGIN_NV04(push, NV03_SIFM(COLOR_FORMAT), 1);
		PUSH_DATA (push, src_format);
	}

	nouveau_pushbuf_bufctx(push, pNv->bufctx);
	if (nouveau_pushbuf_validate(push)) {
		nouveau_pushbuf_bufctx(push, NULL);
		return BadAlloc;
	}

	if (pPriv->SyncToVBlank)
		NV11SyncToVBlank(ppix, dstBox);

	while (nbox--) {
		if (!PUSH_SPACE(push, 16)) {
			nouveau_pushbuf_bufctx(push, NULL);
			return BadImplementation;
		}

		BEGIN_NV04(push, NV04_RECT(COLOR1_A), 1);
		PUSH_DATA (push, 0);

		BEGIN_NV04(push, NV03_SIFM(CLIP_POINT), 6);
		PUSH_DATA (push, (pbox->y1 << 16) | pbox->x1);
		PUSH_DATA (push, (pbox->y2 - pbox->y1) << 16 |
				 (pbox->x2 - pbox->x1));
		PUSH_DATA (push, dst_point);
		PUSH_DATA (push, dst_size);
		PUSH_DATA (push, dsdx);
		PUSH_DATA (push, dtdy);
		BEGIN_NV04(push, NV03_SIFM(SIZE), 4);
		PUSH_DATA (push, (height << 16) | width);
		PUSH_DATA (push, NV03_SIFM_FORMAT_FILTER_BILINEAR |
				 NV03_SIFM_FORMAT_ORIGIN_CENTER | src_pitch);
		PUSH_RELOC(push, src, src_offset, NOUVEAU_BO_LOW, 0, 0);
		PUSH_DATA (push, src_point);

		pbox++;
	}

	nouveau_pushbuf_bufctx(push, NULL);
	PUSH_KICK(push);
	exaMarkSync(pScrn->pScreen);

	pPriv->videoStatus = FREE_TIMER;
	pPriv->videoTime = currentTime.milliseconds + FREE_DELAY;
	extern void NVVideoTimerCallback(ScrnInfoPtr, Time);
	pNv->VideoTimerCallback = NVVideoTimerCallback;
	return Success;
}


/**
 * NVSetBlitPortAttribute
 * sets the attribute "attribute" of port "data" to value "value"
 * supported attributes:
 * - xvSyncToVBlank (values: 0,1)
 * - xvSetDefaults (values: NA; SyncToVBlank will be set, if hardware supports it)
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
NVSetBlitPortAttribute(ScrnInfoPtr pScrn, Atom attribute,
		       INT32 value, pointer data)
{
	NVPortPrivPtr pPriv = (NVPortPrivPtr)data;
	NVPtr	   pNv = NVPTR(pScrn);

	if ((attribute == xvSyncToVBlank) && VSYNC_POSSIBLE) {
		if ((value < 0) || (value > 1))
			return BadValue;
		pPriv->SyncToVBlank = value;
	} else
	if (attribute == xvSetDefaults) {
		pPriv->SyncToVBlank = VSYNC_POSSIBLE;
	} else
		return BadMatch;

	return Success;
}

/**
 * NVGetBlitPortAttribute
 * reads the value of attribute "attribute" from port "data" into INT32 "*value"
 * currently only one attribute supported: xvSyncToVBlank
 * 
 * @param pScrn unused
 * @param attribute attribute to be read
 * @param value value of attribute will be stored here
 * @param data port from which attribute will be read
 * @return Success, if queried attribute exists
 */
int
NVGetBlitPortAttribute(ScrnInfoPtr pScrn, Atom attribute,
		       INT32 *value, pointer data)
{
	NVPortPrivPtr pPriv = (NVPortPrivPtr)data;

	if(attribute == xvSyncToVBlank)
		*value = (pPriv->SyncToVBlank) ? 1 : 0;
	else
		return BadMatch;

	return Success;
}

/**
 * NVStopBlitVideo
 */
void
NVStopBlitVideo(ScrnInfoPtr pScrn, pointer data, Bool Exit)
{
}

