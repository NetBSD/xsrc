/* $XFree86: xc/programs/Xserver/hw/xfree68/pm2/pm2im.c,v 1.1.2.1 1999/06/02 07:50:19 hohndel Exp $ */
/*
 * Copyright 1992,1993,1994,1995,1996 by Kevin E. Martin, Chapel Hill, North Carolina.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Kevin E. Martin not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Kevin E. Martin makes no
 * representations about the suitability of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 *
 * KEVIN E. MARTIN, RICKARD E. FAITH, AND TIAGO GONS DISCLAIM ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL THE AUTHORS
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY
 * DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
 * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Modified for the Mach-8 by Rickard E. Faith (faith@cs.unc.edu)
 * Modified for the Mach32 by Kevin E. Martin (martin@cs.unc.edu)
 * Modified for the Mach64 by Kevin E. Martin (martin@cs.unc.edu)
 * Modified for the Permedia2 by Michel Dänzer (michdaen@iiic.ethz.ch)
 */
/* $XConsortium: mach64im.c /main/4 1996/05/13 16:36:12 kaleb $ */

#include "X.h"
#include "misc.h"
#include "pm2_accel.h"

#if defined(__powerpc__)
#define ALIGN_DEST_TO8(d, s, n)		\
    while (d & 7) {			\
	if (n <= 0)			\
	    return;			\
	n--;				\
	*(char *)d = *(char *)s;	\
	d++; s++;			\
    }

#define DO_REST_ALIGNED(d, s, n)	\
    while (n-- > 0) {			\
	*(char *)d = *(char *)s;	\
	d++; s++;			\
    }

static inline void memcpy_double(void *d0, void *s0, long n)
{
    unsigned long d = (unsigned long)d0;
    unsigned long s = (unsigned long)s0;

    if (!((d ^ s) & 7)) {
	ALIGN_DEST_TO8(d, s, n);
	n -= 8;
	while (n >= 0) {
	    *(double *)d = *(double *)s;
	    n -= 8;
	    s += 8;
	    d += 8;
	}
	n += 8;
	DO_REST_ALIGNED(d, s, n);
    } else
	memcpy(d0, s0, n);
}

#undef MemToBus
#undef BusToMem
#define MemToBus	memcpy_double
#define BusToMem	memcpy_double

#endif

void (*pm2fbImageReadFunc)();
void (*pm2fbImageWriteFunc)();

static void pm2fbImageRead();
static void pm2fbImageWrite();
static void pm2fbImageWriteHW();

static unsigned long PMask;
static int BytesPerPixel;
static int screenStride;

void
pm2fbImageInit()
{
    PMask = (1UL << glintInfoRec.depth) - 1;
    BytesPerPixel = glintInfoRec.bitsPerPixel / 8;
    screenStride = pm2fbVirtX * BytesPerPixel;

    pm2fbImageReadFunc = pm2fbImageRead;
    pm2fbImageWriteFunc = pm2fbImageWrite;
}

static void
pm2fbImageWrite(x, y, w, h, psrc, pwidth, px, py, alu, planemask)
    int			x;
    int			y;
    int			w;
    int			h;
    char		*psrc;
    int			pwidth;
    int			px;
    int			py;
    int			alu;
    unsigned long	planemask;
{
    pointer curvm;
    int byteCount;

    if ((w <= 0) || (h <= 0))
	return;

    if (alu == GXnoop)
	return;

    if ((alu != GXcopy) || ((planemask & PMask) != PMask)) {
#ifdef DEBUG
	ErrorF("pm2fbImageWrite: using Hardware\n");
#endif
	pm2fbImageWriteHW(x, y, w, h, psrc, pwidth, px, py,
			   alu, planemask);
	return;
    }
	
    PM2_WAIT_IDLE();

/*#ifdef DEBUG
	ErrorF("pm2fbImageWrite: using CPU\n");
#endif*/

    psrc += pwidth * py + px * BytesPerPixel;
    curvm = (pointer)((unsigned char *)glintVideoMem +
	    (x + y * pm2fbVirtX) * BytesPerPixel);

    byteCount = w * BytesPerPixel;
    while(h--) {
	MemToBus((void *)curvm, psrc, byteCount);
	curvm = (pointer)((unsigned char *)curvm + screenStride);
	psrc += pwidth;
    }
}


static void
pm2fbImageWriteHW(x, y, w, h, psrc, pwidth, px, py, alu, planemask)
    int			x;
    int			y;
    int			w;
    int			h;
    char		*psrc;
    int			pwidth;
    int			px;
    int			py;
    int			alu;
    unsigned long	planemask;
{
    int wordsPerLine;
    register int count;
    register int *pword;
    int old_DP_PIX_WIDTH;

    Permedia2ImageWrite(alu, planemask, x, y, w, h);

    /*old_DP_PIX_WIDTH = regr(DP_PIX_WIDTH);
    regw(SC_LEFT_RIGHT, (((x+w-1) << 16) | (x & 0x0000ffff)));
    regw(SC_TOP_BOTTOM, (((y+h-1) << 16) | (y & 0x0000ffff)));
    regw(DP_WRITE_MASK, planemask);
    regw(DP_MIX, (alu << 16) | alu);
    regw(DP_SRC, (MONO_SRC_ONE | FRGD_SRC_HOST | BKGD_SRC_HOST));
    switch(glintInfoRec.bitsPerPixel)
    {
	case 8:
            regw(DP_PIX_WIDTH, (SRC_8BPP | HOST_8BPP | DST_8BPP));
	    break;
	case 16:
            regw(DP_PIX_WIDTH, (SRC_16BPP | HOST_16BPP | DST_16BPP));
	    break;
	case 32:
            regw(DP_PIX_WIDTH, (SRC_32BPP | HOST_32BPP | DST_32BPP));
	    break;
    }*/

    /* w = (w + 3) / 4; round up to int boundry */

    /*regw(DST_Y_X, ((x << 16) | (y & 0x0000ffff)));
    regw(DST_HEIGHT_WIDTH, (((w * 4) << 16) | (h & 0x0000ffff)));
    */
    psrc += pwidth * py + px * BytesPerPixel;
    wordsPerLine = w * BytesPerPixel;

    while (h--)
    {
	count = wordsPerLine;
	pword = (int *)psrc;

	while (count >= 16)
	{
            GLINT_WAIT(16);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
            GLINT_WRITE_REG(*pword++, FBData);
	    count -= 16;
	}

	GLINT_WAIT(count);
	switch(count)
	{
	    case 15: GLINT_WRITE_REG(*pword++, FBData);
	    case 14: GLINT_WRITE_REG(*pword++, FBData);
	    case 13: GLINT_WRITE_REG(*pword++, FBData);
	    case 12: GLINT_WRITE_REG(*pword++, FBData);
	    case 11: GLINT_WRITE_REG(*pword++, FBData);
	    case 10: GLINT_WRITE_REG(*pword++, FBData);
	    case  9: GLINT_WRITE_REG(*pword++, FBData);
	    case  8: GLINT_WRITE_REG(*pword++, FBData);
	    case  7: GLINT_WRITE_REG(*pword++, FBData);
	    case  6: GLINT_WRITE_REG(*pword++, FBData);
	    case  5: GLINT_WRITE_REG(*pword++, FBData);
	    case  4: GLINT_WRITE_REG(*pword++, FBData);
	    case  3: GLINT_WRITE_REG(*pword++, FBData);
	    case  2: GLINT_WRITE_REG(*pword++, FBData);
	    case  1: GLINT_WRITE_REG(*pword, FBData);
	    default:
		break;
	}
	psrc += pwidth;
    }

    /*GLINT_WAIT(3);
    regw(SC_LEFT_RIGHT, ((pm2fbMaxX << 16) | 0 ));
    regw(SC_TOP_BOTTOM, ((pm2fbMaxY << 16) | 0 ));
    regw(DP_PIX_WIDTH, old_DP_PIX_WIDTH);
    */

#ifdef DEBUG
        ErrorF("pm2fbImagWriteHW: Data written, waiting...");
#endif
        PM2_WAIT_IDLE();
#ifdef DEBUG
        ErrorF(" OK\n");
#endif
}


static void
pm2fbImageRead(x, y, w, h, psrc, pwidth, px, py, planemask)
    int			x;
    int			y;
    int			w;
    int			h;
    char		*psrc;
    int			pwidth;
    int			px;
    int			py;
    unsigned long	planemask;
{
    int j;
    pointer curvm;

    if ((w <= 0) || (h <= 0))
	return;

    if ((planemask & PMask) != PMask) {
#ifdef NOT_DEFINED
        pm2fbImageReadHW(x, y, w, h, psrc, pwidth, px, py, planemask);
	return;
#else
	ErrorF ("pm2fbImageRead: unsupported planemask\n");
#endif
    }

    PM2_WAIT_IDLE();

    psrc += pwidth * py + px * BytesPerPixel;
    curvm = (pointer)((unsigned char *)glintVideoMem + x * BytesPerPixel);

    for (j = y; j < y+h; j++) {
	BusToMem(psrc, (void *)((unsigned char *)curvm + j * screenStride),
		 w * BytesPerPixel);
	psrc += pwidth;
    }
}

/* This doesn't work yet either */
void
pm2fbImageStippleFunc(x, y, w, h, psrc, pwidth, px, py, fgPixel, bgPixel,
		       alu, planemask, opaque)
    int                 x;
    int                 y;
    int                 w;
    int                 h;
    char                *psrc;
    int                 pwidth;
    int                 px;
    int                 py;
    Pixel               fgPixel;
    Pixel               bgPixel;
    int                 alu;
    unsigned long       planemask;
    int 		opaque;
{
    register int *pword;
    register int *pline;
    register int count;
    int wordsPerLine;
    int old_DP_PIX_WIDTH;
    int old_DST_OFF_PITCH;

    if (alu == GXnoop || w <= 0 || h <= 0)
	return;

    /*GLINT_WAIT(12);
    old_DP_PIX_WIDTH = regr(DP_PIX_WIDTH);
    old_DST_OFF_PITCH = regr(DST_OFF_PITCH);

    regw(SC_LEFT_RIGHT, (((x+w-1) << 16) | (x & 0x0000ffff)));
    regw(SC_TOP_BOTTOM, (((y+h-1) << 16) | (y & 0x0000ffff)));

    regw(DP_WRITE_MASK, planemask);
    regw(DP_FRGD_CLR, fgPixel);
    regw(DP_BKGD_CLR, bgPixel);*/
    
    Permedia2SetupForFillRectSolidBitmask(fgPixel, bgPixel, px, alu, planemask, opaque);
    
    /*switch(glintInfoRec.bitsPerPixel)
    {
        case 8:
    	    regw(DP_PIX_WIDTH, (BYTE_ORDER_LSB_TO_MSB | SRC_1BPP | HOST_1BPP |
				DST_8BPP));
            break;
        case 16:
    	    regw(DP_PIX_WIDTH, (BYTE_ORDER_LSB_TO_MSB | SRC_1BPP | HOST_1BPP |
				DST_16BPP));
            break;
        case 32:
    	    regw(DP_PIX_WIDTH, (BYTE_ORDER_LSB_TO_MSB | SRC_1BPP | HOST_1BPP |
				DST_32BPP));
            break;
    }
    regw(DP_SRC, (MONO_SRC_HOST | FRGD_SRC_FRGD_CLR | BKGD_SRC_BKGD_CLR));
    */
    /*w = (w + (px % 32) + 31) >> 5*/; /* round up to int boundry and take into */
				   /* account the pixels skipped over */

    /*if (px)
        regw(DST_Y_X, (((x - (px % 32)) << 16) | (y & 0x0000ffff)));
    else
	regw(DST_Y_X, ((x << 16) | (y & 0x0000ffff)));
    regw(DST_HEIGHT_WIDTH, (((w * 32) << 16) | h));
    */
    
#ifdef DEBUG
        ErrorF("pm2fbImageStippleFunc: opaque = %d, rop = %d\n", opaque, alu);
#endif

    Permedia2SubsequentFillRectSolidBitmask(x, y, w, h);
    
    wordsPerLine = pwidth >> 2;
    pline = (int*)psrc + (wordsPerLine * py) + (px >> 5);

    while (h--)
    {
        count = w;
	pword = pline;

        while (count >= 16)
        {
            GLINT_WAIT(16);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
            GLINT_WRITE_REG(*pword++, BitMaskPattern);
	    count -= 16;
        }

        GLINT_WAIT(count);
        switch(count)
        {
            case 15: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case 14: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case 13: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case 12: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case 11: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case 10: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case  9: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case  8: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case  7: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case  6: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case  5: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case  4: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case  3: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case  2: GLINT_WRITE_REG(*pword++, BitMaskPattern);
            case  1: GLINT_WRITE_REG(*pword, BitMaskPattern);
            default:
                break;
        }
	pline += wordsPerLine;
    }

#ifdef DEBUG
        ErrorF("pm2fbImageStippleFunc: Data written, waiting...");
#endif
	PM2_WAIT_IDLE();
#ifdef DEBUG
        ErrorF(" OK\n");
#endif
}
