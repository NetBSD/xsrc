/* $XFree86: xc/programs/Xserver/hw/xfree86/accel/i128/i128accel.c,v 3.4.2.8 1998/02/20 14:27:55 robin Exp $ */

/*
 * Copyright 1997 by Robin Cutshaw <robin@XFree86.Org>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Robin Cutshaw not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Robin Cutshaw makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * ROBIN CUTSHAW DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL ROBIN CUTSHAW BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "cfb.h"
#include "gc.h"
#include "gcstruct.h"

#include "xf86.h"
#include "xf86xaa.h"
#include "xf86local.h"

#include "i128.h"
#include "i128reg.h"

#define ENG_PIPELINE_READY() { while (eng_cur[BUSY] & BUSY_BUSY) ; }
#define ENG_DONE() { while (eng_cur[FLOW] & (FLOW_DEB | FLOW_MCB | FLOW_PRV)) ;}

extern struct i128io i128io;
extern struct i128mem i128mem;
extern int i128DisplayWidth;
extern int i128DisplayOffset;
extern int i128DeviceType;
extern int i128MemoryType;
static volatile CARD32 *eng_a;
static volatile CARD32 *eng_b;
static volatile CARD32 *eng_cur;
static volatile i128dlpu *i128dl1p,  *i128dl2p;
static CARD32 i128dl1o, i128dl2o;
static CARD32 i128blitdir, i128cmd, i128rop, i128clptl, i128clpbr;
static i128dlpu dlpb[256];

/* pre-shift rops and just or in as needed */

CARD32 i128alu[16] =
{
   CR_CLEAR<<8,
   CR_AND<<8,
   CR_AND_REV<<8,
   CR_COPY<<8,
   CR_AND_INV<<8,
   CR_NOOP<<8,
   CR_XOR<<8,
   CR_OR<<8,
   CR_NOR<<8,
   CR_EQUIV<<8,
   CR_INVERT<<8,
   CR_OR_REV<<8,
   CR_COPY_INV<<8,
   CR_OR_INV<<8,
   CR_NAND<<8,
   CR_SET<<8
};
                        /*  8bpp   16bpp  32bpp unused */
static int min_size[]   = { 0x62,  0x32,  0x1A, 0x00 };
static int max_size[]   = { 0x80,  0x40,  0x20, 0x00 };
static int split_size[] = { 0x20,  0x10,  0x08, 0x00 };

unsigned char byte_reversed[256] =
{
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};


void
i128EngineDone()
{
	ENG_DONE();
}


void
i128BitBlit(int x1, int y1, int x2, int y2, int w, int h)
{
	ENG_PIPELINE_READY();

	eng_cur[CMD] = i128cmd;
	/*eng_cur[XY3_DIR] = i128blitdir;*/

	if (i128blitdir & DIR_RL_TB) {
		x1 += w; x1--;
		x2 += w; x2--;
	}
	if (i128blitdir & DIR_LR_BT) {
		y1 += h; y1--;
		y2 += h; y2--;
	}



	if (i128DeviceType == I128_DEVICE_ID1) {
		int bppi;
		int origx2 = x2;
		int origy2 = y2;

		static int first_time_through = 1;

		/* The I128-1 has a nasty bitblit bug
		 * that occurs when dest is exactly 8 pages wide
		 */
		
		bppi = (eng_cur[BUF_CTRL] & BC_PSIZ_MSK) >> 24;

		if ((w >= min_size[bppi]) && (w <= max_size[bppi])) {
			if (first_time_through) {
            			ErrorF("%s: Using I128-1 workarounds.\n",
					i128InfoRec.name);
				first_time_through = 0;
			}

			bppi = split_size[bppi];
#if 1
			/* split method */

			eng_cur[XY2_WH] = (bppi<<16) | h;
			eng_cur[XY0_SRC] = (x1<<16) | y1;		MB;
			eng_cur[XY1_DST] = (x2<<16) | y2;		MB;

			ENG_PIPELINE_READY();

			w -= bppi;

			if (i128blitdir & DIR_RL_TB) {
				/* right to left blit */
				x1 -= bppi;
				x2 -= bppi;
			} else {
				/* left to right blit */
				x1 += bppi;
				x2 += bppi;
			}
#else
			/* clip method */
			eng_cur[CLPTL] = (origx2<<16) | origy2;
			eng_cur[CLPBR] = ((origx2+w)<<16) | (origy2+h);
			w += bppi;
#endif
		}
	}

	eng_cur[XY2_WH] = (w<<16) | h;
	eng_cur[XY0_SRC] = (x1<<16) | y1;				MB;
	eng_cur[XY1_DST] = (x2<<16) | y2;				MB;
}

void
i128SetupForScreenToScreenCopy(int xdir, int ydir, int rop, unsigned planemask,
	int transparency_color)
{

	ENG_PIPELINE_READY();

	eng_cur[MASK] = planemask;

	eng_cur[CLPTL] = 0x00000000;
	eng_cur[CLPBR] = (4095<<16) | 2047;

	if (transparency_color != -1)
		eng_cur[BACK] = transparency_color;


	if (xdir == -1) {
		if (ydir == -1) i128blitdir = DIR_RL_BT;
		else            i128blitdir = DIR_RL_TB;
	} else {
		if (ydir == -1) i128blitdir = DIR_LR_BT;
		else            i128blitdir = DIR_LR_TB;
	}
	eng_cur[XY3_DIR] = i128blitdir;

	i128rop = i128alu[rop];
	i128cmd = (transparency_color != -1 ? (CS_TRNSP<<16) : 0) |
		  i128rop | CO_BITBLT;
	eng_cur[CMD] = i128cmd;
}

void
i128SubsequentScreenToScreenCopy(int x1, int y1, int x2, int y2, int w, int h)
{
	i128BitBlit(x1, y1, x2, y2, w, h);
}

void
i128SetupForFillRectSolid(int color, int rop, unsigned planemask)
{

	ENG_PIPELINE_READY();
#if 0
ErrorF("SFFRS color 0x%x rop 0x%x (i128rop 0x%x) pmask 0x%x\n", color, rop, i128alu[rop]>>8, planemask);
#endif

	if (planemask == -1)
		eng_cur[MASK] = -1;
	else switch (xf86bpp) {
		case 8:
			eng_cur[MASK] = planemask |
					(planemask<<8) |
					(planemask<<16) |
					(planemask<<24);
			break;
		case 16:
			eng_cur[MASK] = planemask | (planemask<<16);
			break;
		case 24:
		case 32:
		default:
			eng_cur[MASK] = planemask;
			break;
	}

	eng_cur[FORE] = color;

	i128clptl = eng_cur[CLPTL] = 0x00000000;
	i128clpbr = eng_cur[CLPBR] = (4095<<16) | 2047 ;

	eng_cur[XY3_DIR] = i128blitdir = DIR_LR_TB;

	i128rop = i128alu[rop];
	i128cmd = (CS_SOLID<<16) | i128rop | CO_BITBLT;
	eng_cur[CMD] = i128cmd;
}

void
i128SubsequentFillRectSolid(int x, int y, int w, int h)
{
#if 0
ErrorF("SFRS %d,%d %d,%d\n", x, y, w, h);
#endif
	i128BitBlit(0, 0, x, y, w, h);
}

void
i128SubsequentTwoPointLine(int x1, int y1, int x2, int y2, int bias)
{
	ENG_PIPELINE_READY();
#if 0
ErrorF("STPL i128rop 0x%x  %d,%d %d,%d   clip %d,%d %d,%d\n", i128rop, x1, y1, x2, y2, i128clptl>>16, i128clptl&0xffff, (i128clpbr>>16)&0xffff, i128clpbr&0xffff);
#endif

	eng_cur[CMD] =
			((bias&0x0100) ? (CP_NLST<<24) : 0) |
			(CC_CLPRECI<<21) |
			(CS_SOLID<<16) |
			i128rop |
			CO_LINE;

	eng_cur[CLPTL] = i128clptl;
	eng_cur[CLPBR] = i128clpbr;

	eng_cur[XY0_SRC] = (x1<<16) | y1;				MB;
	eng_cur[XY1_DST] = (x2<<16) | y2;				MB;

}

void
i128SetClippingRectangle(int x1, int y1, int x2, int y2)
{
	int tmp;
#if 0
ErrorF("SCR  %d,%d %d,%d\n", x1, y1, x2, y2);
#endif

	if (x1 > x2) { tmp = x2; x2 = x1; x1 = tmp; }
	if (y1 > y2) { tmp = y2; y2 = y1; y1 = tmp; }

	i128clptl = (x1<<16) | y1;
	i128clpbr = (x2<<16) | y2;
}


void
i128FillRectSolid(DrawablePtr pDraw, GCPtr pGC, int nBox, register BoxPtr pBoxI)
{
	register int w, h, planemask;

	ENG_PIPELINE_READY();
#if 0
ErrorF("FRS color 0x%x rop 0x%x (i128rop 0x%x) pmask 0x%x\n", pGC->fgPixel, pGC->alu, i128alu[pGC->alu]>>8, pGC->planemask);
#endif

	planemask = pGC->planemask;

	if (planemask != -1) {
		if (xf86bpp == 8) {
			planemask |= (planemask<<8)  |
				     (planemask<<16) |
				     (planemask<<24);
		} else if (xf86bpp == 16)
			planemask |= planemask<<16;
	}

	eng_cur[MASK] = planemask;
	eng_cur[FORE] = pGC->fgPixel;
	eng_cur[CMD] = (CS_SOLID<<16) | i128alu[pGC->alu] | CO_BITBLT;
	eng_cur[CLPTL] = 0x00000000;
	eng_cur[CLPBR] = (4095<<16) | 2047;

	eng_cur[XY3_DIR] = DIR_LR_TB;
	eng_cur[XY0_SRC] = 0x00000000;

	while (nBox > 0) {
		w = pBoxI->x2 - pBoxI->x1;
		h = pBoxI->y2 - pBoxI->y1;
		if (w > 0 && h > 0) {
			eng_cur[XY2_WH] = (w<<16) | h;			MB;
			eng_cur[XY1_DST] = (pBoxI->x1<<16) | pBoxI->y1;	MB;
			ENG_PIPELINE_READY();
		}
		pBoxI++;
		nBox--;
	}

	ENG_DONE();
}


void
i128T2RFillRectSolid(DrawablePtr pDraw, GCPtr pGC, int nBox, BoxPtr pBoxI)
{
	register int planemask;
	register CARD32 *idp, *sdp;
	register i128dlpu *dp;
	static i128dlpu staticdp[2] = {
		{
		(MASK*4)&0xff,			/* aad */
		(FORE*4)&0xff,			/* bad */
		(CMD*4)&0xff,			/* cad */
		0x0c |
		(((MASK*4)&0x100)>>4) |
		(((FORE*4)&0x100)>>3) |
		(((CMD*4)&0x100)>>2),		/* control */
		0x00000000,			/* rad */
		0x00000000,			/* rbd */
		0x00000000,			/* rcd */
		},
		{
		(CLPTL*4)&0xff,			/* aad */
		(CLPBR*4)&0xff,			/* bad */
		0x00,				/* cad */
		0x08 |
		(((CLPTL*4)&0x100)>>4) |
		(((CLPBR*4)&0x100)>>3),		/* control */
		0x00000000,			/* rad */
		(4095<<16) | 2047,		/* rbd */
		0x00000000,			/* rcd */
		}
	};

	ENG_PIPELINE_READY();

	planemask = pGC->planemask;

	if (planemask != -1) {
		if (xf86bpp == 8) {
			planemask |= (planemask<<8)  |
				     (planemask<<16) |
				     (planemask<<24);
		} else if (xf86bpp == 16)
			planemask |= planemask<<16;
	}

	if (nBox == 1) {
		register int w, h;

		eng_cur[MASK] = planemask;
		eng_cur[FORE] = pGC->fgPixel;
		eng_cur[CMD] = (CS_SOLID<<16) | i128alu[pGC->alu] | CO_BITBLT;
		eng_cur[CLPTL] = 0x00000000;
		eng_cur[CLPBR] = (4095<<16) | 2047;

		eng_cur[XY3_DIR] = DIR_LR_TB;
		eng_cur[XY0_SRC] = 0x00000000;

		w = pBoxI->x2 - pBoxI->x1;
		h = pBoxI->y2 - pBoxI->y1;
		if (w > 0 && h > 0) {
			eng_cur[XY2_WH] = (w<<16) | h;			MB;
			eng_cur[XY1_DST] = (pBoxI->x1<<16) | pBoxI->y1;	MB;
		}
		ENG_DONE();
		return;
	}

	staticdp[0].f0.rad = planemask;
	staticdp[0].f0.rbd = pGC->fgPixel;
	staticdp[0].f0.rcd = (CS_SOLID<<16) | i128alu[pGC->alu] | CO_BITBLT;

	/* copy and trigger the transfer */

	idp = (CARD32 *)i128dl2p;
	sdp = (CARD32 *)staticdp;
	*idp++ = *sdp++;
	*idp++ = *sdp++;
	*idp++ = *sdp++;
	*idp++ = *sdp++;
	*idp++ = *sdp++;
	*idp++ = *sdp++;
	*idp++ = *sdp++;
	*idp++ = *sdp++;

	while (eng_cur[DL_ADR] & 0x40000000) ;

	eng_cur[DL_ADR] = i128dl2o;					MB;
	eng_cur[DL_CNTRL] = i128dl2o + 32;				MB;

	while (nBox > 0) {
		register int w, h, n, ndp, dlpslotsleft;

		dp = dlpb;
		ndp = 0;

		/* fill in up to 16 slots in the dlp1 buffer */

		dlpslotsleft = 16;
		while ((nBox > 0) && (dlpslotsleft > 0)) {
			w = pBoxI->x2 - pBoxI->x1;
			h = pBoxI->y2 - pBoxI->y1;
			if (w > 0 && h > 0) {
				dp->f1.xy0 = 0x0000000;
				dp->f1.xy2 = (w<<16) | h;
				dp->f1.xy3 = DIR_LR_TB; /* == 0 */
				dp->f1.xy1 = (pBoxI->x1<<16) | pBoxI->y1;
				dp++; ndp++; dlpslotsleft--;
			}
			pBoxI++;
			nBox--;
		}
		if (ndp == 0)  /* no valid rects, we're done */
			break;

		/* copy the words into the frame buffer dlp1 cache */

		idp = (CARD32 *)i128dl1p;
		sdp = (CARD32 *)dlpb;
		n = ndp;

		while (n-- > 0) {
			*idp++ = *sdp++;
			*idp++ = *sdp++;
			*idp++ = *sdp++;
			*idp++ = *sdp++;
		}

		/* trigger the dlp op */

		while (eng_cur[DL_ADR] & 0x40000000) ;

		eng_cur[DL_ADR] = i128dl1o;				MB;
		eng_cur[DL_CNTRL] = (i128dl1o + 16*ndp) | 0x20000000;	MB;

		/* we do our own multi-threading here */

		dp = dlpb;
		ndp = 0;

		/* fill in up to 128 slots in the dlp2 buffer */

		dlpslotsleft = 128;
		while ((nBox > 0) && (dlpslotsleft > 0)) {
			w = pBoxI->x2 - pBoxI->x1;
			h = pBoxI->y2 - pBoxI->y1;
			if (w > 0 && h > 0) {
				dp->f1.xy0 = 0x0000000;
				dp->f1.xy2 = (w<<16) | h;
				dp->f1.xy3 = DIR_LR_TB; /* == 0 */
				dp->f1.xy1 = (pBoxI->x1<<16) | pBoxI->y1;
				dp++; ndp++; dlpslotsleft--;
			}
			pBoxI++;
			nBox--;
		}
		if (ndp == 0)  /* no valid rects, we're done */
			break;

		/* copy the words into the frame buffer dlp2 cache */

		idp = (CARD32 *)i128dl2p;
		sdp = (CARD32 *)dlpb;
		n = ndp;

		while (n-- > 0) {
			*idp++ = *sdp++;
			*idp++ = *sdp++;
			*idp++ = *sdp++;
			*idp++ = *sdp++;
		}

		/* trigger the dlp op */

		while (eng_cur[DL_ADR] & 0x40000000) ;

		eng_cur[DL_ADR] = i128dl2o;				MB;
		eng_cur[DL_CNTRL] = (i128dl2o + 16*ndp) | 0x20000000;	MB;
	}

	while (!(eng_cur[DL_CNTRL] & 0x00000400)) ;

	ENG_DONE();
}


void
i128ScreenToScreenBitBlt(int nbox, DDXPointPtr pptSrc, BoxPtr pbox,
			 int xdir, int ydir, int alu, unsigned planemask)
{
        i128SetupForScreenToScreenCopy(xdir, ydir, alu, planemask, -1);
        for (; nbox; pbox++, pptSrc++, nbox--)
            i128SubsequentScreenToScreenCopy(pptSrc->x, pptSrc->y,
                pbox->x1, pbox->y1, pbox->x2 - pbox->x1, pbox->y2 - pbox->y1);
	ENG_DONE();
}


void
i128AccelInit()
{
	CARD32 buf_ctrl;

	xf86AccelInfoRec.Flags = BACKGROUND_OPERATIONS |
				 HARDWARE_CLIP_LINE |
				 USE_TWO_POINT_LINE |
				 TWO_POINT_LINE_NOT_LAST |
				 PIXMAP_CACHE
#ifdef DELAYED_SYNC
				 | DELAYED_SYNC
#endif
				 ;

	if (i128DeviceType == I128_DEVICE_ID3)
		xf86AccelInfoRec.Flags |= ONLY_LEFT_TO_RIGHT_BITBLT;

	xf86AccelInfoRec.ServerInfoRec = &i128InfoRec;

	xf86AccelInfoRec.Sync = i128EngineDone;

	xf86AccelInfoRec.SetupForScreenToScreenCopy =
		i128SetupForScreenToScreenCopy;
	xf86AccelInfoRec.SubsequentScreenToScreenCopy =
		i128SubsequentScreenToScreenCopy;

	xf86GCInfoRec.CopyAreaFlags = 0;

	xf86AccelInfoRec.SetupForFillRectSolid =
		i128SetupForFillRectSolid;
	xf86AccelInfoRec.SubsequentFillRectSolid =
		i128SubsequentFillRectSolid;

	xf86AccelInfoRec.SubsequentTwoPointLine =
		i128SubsequentTwoPointLine;

	xf86AccelInfoRec.SetClippingRectangle =
		i128SetClippingRectangle;

	xf86GCInfoRec.PolyFillRectSolidFlags = 0;

	xf86AccelInfoRec.PixmapCacheMemoryStart = i128InfoRec.virtualY *
		i128DisplayWidth * i128InfoRec.bitsPerPixel / 8;

	xf86AccelInfoRec.PixmapCacheMemoryEnd =
		(i128InfoRec.videoRam * 1024) - 1024;

	/*/
	 * If there is sufficient memory, we create two blocks of
	 * 128 diplay list instruction words (16 bytes each).
	/*/

	if ((xf86AccelInfoRec.PixmapCacheMemoryEnd -
	     xf86AccelInfoRec.PixmapCacheMemoryStart) > 16*256) {
		xf86AccelInfoRec.PixmapCacheMemoryEnd -= 16*256;
		i128dl1o = xf86AccelInfoRec.PixmapCacheMemoryEnd;
		i128dl1p = (i128dlpu *)&i128mem.mw0_ad[i128dl1o];
		i128dl2o = i128dl1o + 16*128;
		i128dl2p = (i128dlpu *)&i128mem.mw0_ad[i128dl2o];
	} else {
		i128dl1o = i128dl2o = 0;
		i128dl1p = i128dl2p = (i128dlpu *)0;
	}

	xf86GCInfoRec.PolyFillRectSolid = xf86PolyFillRect;
	if ((i128DeviceType == I128_DEVICE_ID3) && (i128dl1o != 0))
		xf86AccelInfoRec.FillRectSolid = i128T2RFillRectSolid;
	else
		xf86AccelInfoRec.FillRectSolid = i128FillRectSolid;


	i128InfoRec.displayWidth = i128DisplayWidth;

	eng_a = i128mem.rbase_a;
	eng_b = i128mem.rbase_b;
	eng_cur = eng_a;

	switch (xf86bpp) {
		case 8:  buf_ctrl = BC_PSIZ_8B;  break;
		case 16: buf_ctrl = BC_PSIZ_16B; break;
		case 24:
		case 32: buf_ctrl = BC_PSIZ_32B; break;
		default: buf_ctrl = 0;           break; /* error */
	}
	if (i128DeviceType == I128_DEVICE_ID3) {
		if (i128MemoryType == I128_MEMORY_SGRAM)
			buf_ctrl |= BC_MDM_PLN;
		else
			buf_ctrl |= BC_BLK_ENA;
	}
	eng_cur[BUF_CTRL] = buf_ctrl;

	eng_cur[DE_PGE] = 0x00;
	eng_cur[DE_SORG] = i128DisplayOffset;
	eng_cur[DE_DORG] = i128DisplayOffset;
	eng_cur[DE_MSRC] = 0x00;
	eng_cur[DE_WKEY] = 0x00;
	eng_cur[DE_SPTCH] = i128mem.rbase_g[DB_PTCH];
	eng_cur[DE_DPTCH] = i128mem.rbase_g[DB_PTCH];
	eng_cur[RMSK] = 0x00000000;
	eng_cur[XY4_ZM] = ZOOM_NONE;
	eng_cur[LPAT] = 0xffffffff;  /* for lines */
	eng_cur[PCTRL] = 0x00000000; /* for lines */
	eng_cur[CLPTL] = 0x00000000;
	eng_cur[CLPBR] = (4095<<16) | 2047 ;
}
