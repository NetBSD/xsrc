/*
 * Copyright 1999 Mark Vojkovich
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

#include "nv_include.h"
#include "nv_type.h"
#include "shadowfb.h"
#include "servermd.h"

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

void
NVRefreshArea(ScrnInfoPtr pScrn, int num, BoxPtr pbox)
{
	NVPtr pNv = NVPTR(pScrn);
	int x1, y1, x2, y2, width, height, cpp, FBPitch;
	unsigned char *src, *dst;
   
	cpp = pScrn->bitsPerPixel >> 3;
	FBPitch = pScrn->displayWidth * cpp;

	nouveau_bo_map(pNv->scanout, NOUVEAU_BO_WR, pNv->client);
	while(num--) {
		x1 = MAX(pbox->x1, 0);
		y1 = MAX(pbox->y1, 0);
		x2 = MIN(pbox->x2, pScrn->virtualX);
		y2 = MIN(pbox->y2, pScrn->virtualY);
		width = (x2 - x1) * cpp;
		height = y2 - y1;

		if (width > 0 && height > 0) {
			src = pNv->ShadowPtr + (y1 * pNv->ShadowPitch) + (x1 * cpp);
			dst = pNv->scanout->map + (y1 * FBPitch) + (x1 * cpp);

			while(height--) {
				memcpy(dst, src, width);
				dst += FBPitch;
				src += pNv->ShadowPitch;
			}
		}

		pbox++;
	}
} 
